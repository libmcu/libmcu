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

int metrics_report(void *buf, size_t bufsize, struct metricfs *mfs, void *ctx)
{
	size_t len = 0;
	bool from_store = false;
	int err;

	metrics_report_prepare(ctx);

	if (mfs && metricfs_count(mfs) > 0) {
		err = metricfs_peek_first(mfs, buf, bufsize, NULL);
		if (err > 0) {
			len = (size_t)err;
			from_store = true;
		}
	}

	if (!from_store) {
		len = metrics_collect(buf, bufsize);
		if (len == 0) {
			return 0;
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

	if (mfs && metricfs_count(mfs) > 0) {
		return -EAGAIN;
	}

	return err;
}
