#ifndef LIBMCU_BUTTON_H
#define LIBMCU_BUTTON_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>

struct button_data {
	unsigned int history;
	unsigned int time_pressed;
	unsigned int time_released;
};

struct button_handlers {
	void (*pressed)(const struct button_data *btn, void *context);
	void (*released)(const struct button_data *btn, void *context);
	union {
		void (*holding)(const struct button_data *btn, void *context);
		void (*repeat_started)(const struct button_data *btn,
				void *context);
	};
#if 0
	void (*repeat)(const struct button_data *btn, void *context);
	void (*double_clicked)(const struct button_data *btn, void *context);
#endif
};

void button_init(unsigned int (*get_monotonic_time_in_ms)(void),
		void (*mydelay)(unsigned int ms));
bool button_register(const struct button_handlers *handlers,
		int (*get_button_state)(void));
void button_poll(void *context);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BUTTON_H */
