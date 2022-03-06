#ifndef LIBMCU_METRICS_H
#define LIBMCU_METRICS_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#if !defined(METRICS_USER_DEFINES)
#define METRICS_USER_DEFINES		"metrics.def"
#endif

enum {
#define METRICS_DEFINE(id, key)		key = id,
#include METRICS_USER_DEFINES
#undef METRICS_DEFINE
};

typedef uint8_t metric_key_t;

void metrics_set(metric_key_t key, int32_t val);
int32_t metrics_get(metric_key_t key);
void metrics_increase(metric_key_t key);
void metrics_increase_by(metric_key_t key, int32_t n);
void metrics_reset(void);
void metrics_iterate(void (*callback_each)(metric_key_t key, int32_t value,
					   void *ctx), void *ctx);
size_t metrics_collect(void *buf, size_t bufsize);
size_t metrics_count(void);

/**
 * @brief It creates an encoding header.
 *
 * This function is called internally in `metrics_collect()`.
 *
 * @param[in] buf buffer
 * @param[in] bufsize buffer size
 * @param[in] nr_total the number of metrics declared in metrics.def
 * @param[in] nr_updated the number of metrics with non-zero value
 * @return the number of bytes written
 */
size_t metrics_encode_header(void *buf, size_t bufsize,
		uint32_t nr_total, uint32_t nr_updated);
size_t metrics_encode_each(void *buf, size_t bufsize,
		metric_key_t key, int32_t value);

void metrics_lock_init(void);
void metrics_lock(void);
void metrics_unlock(void);

void metrics_init(bool force);

#if defined(METRICS_KEY_STRING)
char const *metrics_stringify_key(metric_key_t key);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_METRICS_H */
