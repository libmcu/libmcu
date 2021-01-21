#ifndef LIBMCU_BUTTON_H
#define LIBMCU_BUTTON_H 202101L

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>

struct button_data {
	unsigned int history;
	unsigned int time_pressed;
	unsigned int time_released;
};

typedef struct {
	void (*pressed)(const struct button_data *btn, void *context);
	void (*released)(const struct button_data *btn, void *context);
	void (*repeat_started)(const struct button_data *btn, void *context);
	void (*repeat)(const struct button_data *btn, void *context);
	void (*double_clicked)(const struct button_data *btn, void *context);
} button_handlers_t;

void button_init(unsigned int (*get_time_ms)(void),
		void (*delayf)(unsigned int ms));
bool button_register(const button_handlers_t *handlers, int (*get_state)(void));
void button_poll(void *context);
void button_sleep(void *context);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BUTTON_H */
