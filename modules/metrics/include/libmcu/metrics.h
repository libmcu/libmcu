#ifndef LIBMCU_METRICS_H
#define LIBMCU_METRICS_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

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
size_t metrics_get_encoded(void *buf, size_t bufsize);
size_t metrics_encode(void *buf, size_t bufsize,
		metric_key_t key, int32_t value);
void metrics_init(void);

#if defined(METRICS_KEY_STRING)
char const *metrics_stringify_key(metric_key_t key);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_METRICS_H */
