/*
 * SPDX-FileCopyrightText: 2026 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_METRICS_REPORTER_H
#define LIBMCU_METRICS_REPORTER_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

struct metricfs;

/**
 * @brief Runs a single metrics report cycle.
 *
 * Each invocation performs the following sequence:
 * 1. Calls metrics_report_prepare() to let the application refresh metric
 *    values (e.g. uptime, CPU load).
 * 2. If @p mfs is non-NULL and contains unsent entries, peeks the oldest one
 *    for transmission.
 * 3. Otherwise collects current in-memory metrics via metrics_collect().
 * 4. Calls metrics_report_transmit() with the payload.
 * 5. On success: resets in-memory metrics or deletes the metricfs entry.
 * 6. On failure with @p mfs non-NULL: persists the payload to metricfs and
 *    resets in-memory metrics.  If persistence also fails, in-memory metrics
 *    are kept so the next cycle can retry.
 * 7. On failure with @p mfs NULL: does not reset (allows retry on next call).
 *
 * Timing, scheduling, and retry strategy are the caller's responsibility.
 *
 * @param[in] buf     Encoding buffer (caller-owned, no internal allocation).
 * @param[in] bufsize Size of the buffer in bytes.
 * @param[in] mfs     metricfs instance for persistent storage on failure.
 *                    Pass NULL to disable persistence.
 * @param[in] ctx     User context forwarded to metrics_report_transmit() and
 *                    metrics_report_prepare().
 *
 * @return 0 on success.
 * @return -EAGAIN when unsent data remains in metricfs.
 * @return negative errno on error.
 */
int metrics_report(void *buf, size_t bufsize, struct metricfs *mfs, void *ctx);

/**
 * @brief Convenience wrapper that rate-limits metrics_report() calls.
 *
 * Calls metrics_report() only when at least @c METRICS_REPORT_INTERVAL_SEC
 * seconds (default 3600) have elapsed since the last successful report, or
 * when unsent data remains in @p mfs.  On the very first call the report
 * always runs.
 *
 * Time is obtained via metrics_get_unix_timestamp().
 *
 * @param[in] buf     Encoding buffer (caller-owned).
 * @param[in] bufsize Size of the buffer in bytes.
 * @param[in] mfs     metricfs instance, or NULL.
 * @param[in] ctx     User context forwarded to hooks.
 *
 * @return 0 when skipped or succeeded.
 * @return -EAGAIN when unsent data remains.
 * @return negative errno on error.
 */
int metrics_report_periodic(void *buf, size_t bufsize,
		struct metricfs *mfs, void *ctx);

/**
 * @brief Resets the internal periodic timer state.
 *
 * Intended for testing.  After this call the next metrics_report_periodic()
 * invocation will run unconditionally.
 */
void metrics_report_periodic_reset(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_METRICS_REPORTER_H */
