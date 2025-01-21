/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_METRICFS_H
#define LIBMCU_METRICFS_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include "libmcu/kvstore.h"

#if !defined(METRICFS_ID_PREFIX)
#define METRICFS_ID_PREFIX	"metric"
#endif
#if !defined(METRICFS_ID_MAXLEN)
#define METRICFS_ID_MAXLEN	15
#endif

struct metricfs;

typedef uint16_t metricfs_id_t;

/**
 * @typedef metricfs_iterator_t
 * @brief A function pointer type for iterating over metric file system entries.
 *
 * This type defines a function pointer for a callback function used to iterate
 * over entries in the metric file system.
 *
 * @param[in] id The identifier of the metric entry.
 * @param[in] data A pointer to the data associated with the metric entry.
 * @param[in] datasize The size of the data in bytes.
 * @param[in] ctx A user-defined context pointer that can be used to pass
 *            additional information to the callback function.
 */
typedef void (*metricfs_iterator_t)(const metricfs_id_t id,
		const void *data, const size_t datasize, void *ctx);

/**
 * @brief Creates a metric file system instance.
 *
 * This function initializes and returns a new metric file system instance.
 * This function initializes and returns a new metric file system instance.
 *
 * @note This function does not open the namespace internally. The namespace
 * must be opened before using the metric file system instance.
 *
 * @param[in] kvstore A pointer to the key-value store to be used by the metric
 *            file system.
 * @param[in] prefix A string representing the prefix for the metrics.
 * @param[in] max_metrics The maximum number of metrics that can be stored in
 *            the metric file system.
 *
 * @return A pointer to the newly created metric file system instance.
 */
struct metricfs *metricfs_create(struct kvstore *kvstore,
		const char *prefix, const size_t max_metrics);

/**
 * @brief Destroys a metric file system instance.
 *
 * This function releases all resources associated with the given metric file
 * system instance.
 *
 * @param[in] fs A pointer to the metric file system instance to be destroyed.
 */
void metricfs_destroy(struct metricfs *fs);

/**
 * @brief Writes data to the metric file system.
 *
 * This function writes the given data to the metric file system.
 *
 * @param[in] fs A pointer to the metric file system instance.
 * @param[in] data A pointer to the data to be written.
 * @param[in] datasize The size of the data in bytes.
 * @param[out] id A pointer to store the generated id. NULL can be passed if id
 *             is not needed.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int metricfs_write(struct metricfs *fs,
		const void *data, const size_t datasize, metricfs_id_t *id);

/**
 * @brief Counts the number of metrics in the metric file system.
 *
 * This function returns the number of metrics currently stored in the metric
 * file system.
 *
 * @param[in] fs A pointer to the metric file system instance.
 *
 * @return The number of metrics in the metric file system.
 */
uint16_t metricfs_count(const struct metricfs *fs);

/**
 * @brief Iterates over metrics in the metric file system.
 *
 * This function iterates over the metrics stored in the metric file system and
 * calls the provided callback function for each metric.
 *
 * @param[in] fs A pointer to the metric file system instance.
 * @param[in] cb The callback function to be called for each metric.
 * @param[in] cb_ctx A user-defined context pointer to be passed to the callback
 *            function.
 * @param[out] buf A buffer to store the data of each metric during iteration.
 * @param[in] bufsize The size of the buffer in bytes.
 * @param[in] max_metrics The maximum number of metrics to iterate over.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int metricfs_iterate(struct metricfs *fs,
		metricfs_iterator_t cb, void *cb_ctx,
		void *buf, const size_t bufsize, const size_t max_metrics);

/**
 * @brief Peeks at data in the metric file system without removing it.
 *
 * This function reads the data associated with the specified id from the metric
 * file system without removing the data.
 *
 * @param[in] fs A pointer to the metric file system instance.
 * @param[in] id The id of the metric to be peeked at.
 * @param[out] buf A buffer to store the peeked data.
 * @param[in] bufsize The size of the buffer in bytes.
 *
 * @return The number of bytes read on success, or a negative error code on
 *         failure.
 */
int metricfs_peek(struct metricfs *fs,
		const metricfs_id_t id, void *buf, const size_t bufsize);

/**
 * @brief Peeks at the first metric in the metric file system without removing
 * it.
 *
 * This function reads the data of the first metric in the metric file system
 * without removing it.
 *
 * @param[in] fs A pointer to the metric file system instance.
 * @param[out] buf A buffer to store the peeked data.
 * @param[in] bufsize The size of the buffer in bytes.
 * @param[out] id A pointer to store the id of the first metric. If null, the id
 *             is ignored. Null input is acceptable.
 *
 * @return The number of bytes read on success, or a negative error code on
 *         failure.
 */
int metricfs_peek_first(struct metricfs *fs,
		void *buf, const size_t bufsize, metricfs_id_t *id);

/**
 * @brief Reads the first metric from the metric file system.
 *
 * This function reads the data of the first metric in the metric file system
 * and removes it.
 *
 * @param[in] fs A pointer to the metric file system instance.
 * @param[out] buf A buffer to store the read data.
 * @param[in] bufsize The size of the buffer in bytes.
 * @param[out] id A pointer to store the id of the first metric. If null, the id
 *             is ignored. Null input is acceptable.
 *
 * @return The number of bytes read on success, or a negative error code on
 *         failure.
 */
int metricfs_read_first(struct metricfs *fs,
		void *buf, const size_t bufsize, metricfs_id_t *id);

/**
 * @brief Deletes the first metric from the metric file system.
 *
 * This function deletes the first metric in the metric file system.
 *
 * @param[in] fs A pointer to the metric file system instance.
 * @param[out] id A pointer to store the id of the deleted metric. If null, the
 *             id is ignored. Null input is acceptable.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int metricfs_del_first(struct metricfs *fs, metricfs_id_t *id);

/**
 * @brief Clears all metrics from the metric file system.
 *
 * This function removes all metrics from the metric file system.
 *
 * @param[in] fs A pointer to the metric file system instance.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int metricfs_clear(struct metricfs *fs);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_METRICFS_H */
