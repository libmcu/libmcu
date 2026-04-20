/*
 * SPDX-FileCopyrightText: 2026 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/metrics_reporter.h"
#include "libmcu/metrics.h"
#include "libmcu/metrics_overrides.h"
#include "libmcu/metricfs.h"

#include <errno.h>
#include <stdbool.h>

#if !defined(METRICS_REPORT_INTERVAL_SEC)
#define METRICS_REPORT_INTERVAL_SEC	3600U
#endif

static inline bool has_backlog(const struct metricfs *fs)
{
	return fs && metricfs_count(fs) > 0;
}

static int report_once(void *buf, size_t bufsize,
		struct metricfs *mfs, void *ctx)
{
	size_t len = 0;
	bool from_store = false;
	int err;

	if (buf == NULL || bufsize == 0) {
		return -EINVAL;
	}

	metrics_report_prepare(ctx);

	if (has_backlog(mfs)) {
		err = metricfs_peek_first(mfs, buf, bufsize, NULL);
		if (err > (int)bufsize) {
			return -ENOBUFS;
		} else if (err > 0) {
			len = (size_t)err;
			from_store = true;
		}
	}

	if (!from_store) {
		len = metrics_collect(buf, bufsize);
		if (len > bufsize) {
			return -ENOBUFS;
		} else if (len == 0) {
			return has_backlog(mfs) ? -EAGAIN : 0;
		}
	}

	err = metrics_report_transmit(buf, len, ctx);

	if (err == 0) {
		if (from_store) {
			err = metricfs_del_first(mfs, NULL);
			if (err != 0) {
				return err;
			}
		} else {
			metrics_reset();
		}
	} else if (!from_store && mfs) {
		if (metricfs_write(mfs, buf, len, NULL) == 0) {
			metrics_reset();
		}
	}

	if (has_backlog(mfs)) {
		return -EAGAIN;
	}

	return err;
}

int metrics_report(void *buf, size_t bufsize, struct metricfs *mfs, void *ctx)
{
	return report_once(buf, bufsize, mfs, ctx);
}

static uint64_t last_report_time;
static bool periodic_initialized;

void metrics_report_periodic_reset(void)
{
	last_report_time = 0;
	periodic_initialized = false;
}

int metrics_report_periodic(void *buf, size_t bufsize,
		struct metricfs *mfs, void *ctx)
{
	uint64_t now = metrics_get_unix_timestamp();

	if (periodic_initialized && now != 0) {
		if (now - last_report_time < METRICS_REPORT_INTERVAL_SEC
				&& !has_backlog(mfs)) {
			return -EALREADY;
		}
	}

	int err = report_once(buf, bufsize, mfs, ctx);

	if (err == 0 || err == -EAGAIN) {
		last_report_time = now;
		periodic_initialized = true;
	}

	return err;
}
