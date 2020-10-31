#ifndef ESP_TIMER
#define ESP_TIMER

#include <stdint.h>

#define ESP_OK			0

typedef int32_t esp_err_t;
typedef void * esp_timer_handle_t;
typedef struct {
	void (*callback)(void *);
} esp_timer_create_args_t;

esp_err_t esp_timer_init(void);
esp_err_t esp_timer_deinit(void);
esp_err_t esp_timer_create(const esp_timer_create_args_t *args,
                           esp_timer_handle_t *out_handle);
esp_err_t esp_timer_delete(esp_timer_handle_t timer);
esp_err_t esp_timer_start_periodic(esp_timer_handle_t timer, uint64_t period_us);
esp_err_t esp_timer_start_once(esp_timer_handle_t timer, uint64_t timeout_us);
esp_err_t esp_timer_stop(esp_timer_handle_t timer);

#endif
