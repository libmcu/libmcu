#ifndef LIBMCU_METRICS_H
#define LIBMCU_METRICS_H 202102L

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

typedef enum {
#define METRICS_DEFINE(id, key)	key = id,
#include "metrics_defaults.def"
#undef METRICS_DEFINE
} metric_key_t;

void metrics_set(metric_key_t key, int32_t val);
void metrics_increase(metric_key_t key);
void metrics_increase_by(metric_key_t key, int32_t n);
void metrics_reset(void);
void metrics_iterate(void (*callback_each)(metric_key_t keyid, int32_t value));
void metrics_init(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_METRICS_H */
