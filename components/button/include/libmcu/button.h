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
#if 0
	void (*repeat_started)(const struct button_data *btn, void *context);
	void (*repeat)(const struct button_data *btn, void *context);
	void (*double_clicked)(const struct button_data *btn, void *context);
#endif
} button_handlers_t;

void button_init(unsigned int (*get_monotonic_tick_in_ms)(void),
		void (*mydelay)(unsigned int ms));
void button_hw_init(void);
bool button_register(const button_handlers_t *handlers, int (*get_state)(void));
void button_poll(void *context);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BUTTON_H */
