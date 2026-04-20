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

static inline bool has_backlog(const struct metricfs *fs)
{
	return fs && metricfs_count(fs) > 0;
}

int metrics_report(void *buf, size_t bufsize, struct metricfs *mfs, void *ctx)
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
		if (err > 0) {
			len = (size_t)err;
			from_store = true;
		}
	}

	if (!from_store) {
		len = metrics_collect(buf, bufsize);
		if (len == 0) {
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
