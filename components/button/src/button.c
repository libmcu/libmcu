#include "libmcu/button.h"
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include "libmcu/compiler.h"

#if !defined(BUTTON_MAX)
#define BUTTON_MAX				1
#endif
#if !defined(BUTTON_SAMPLING_PERIOD_MS)
#define BUTTON_SAMPLING_PERIOD_MS		10U
#endif
#if !defined(BUTTON_MIN_PRESS_TIME_MS)
#define BUTTON_MIN_PRESS_TIME_MS		60U
#endif
#if 0
#if !defined(BUTTON_REPEAT_DELAY_MS)
#define BUTTON_REPEAT_DELAY_MS			300U
#endif
#if !defined(BUTTON_REPEAT_RATE_MS)
#define BUTTON_REPEAT_RATE_MS			100U
#endif
#endif
LIBMCU_STATIC_ASSERT(BUTTON_MAX < 8*sizeof(unsigned int),
		"BUTTON_MAX must be less than bitmap data type size.");
LIBMCU_STATIC_ASSERT(BUTTON_MIN_PRESS_TIME_MS > BUTTON_SAMPLING_PERIOD_MS,
		"The sampling period time must be less than press hold time.");

#define MIN_PRESSED_HISTORY			\
	(BUTTON_MIN_PRESS_TIME_MS / BUTTON_SAMPLING_PERIOD_MS)
#define HISTORY_MASK				\
	((1U << (MIN_PRESSED_HISTORY + 1)) - 1) // 0b1111111
LIBMCU_STATIC_ASSERT(MIN_PRESSED_HISTORY < (8*sizeof(unsigned int) - 2),
		"The history pattern must be within the data type size.");

typedef enum {
	BUTTON_STATE_UNKNOWN			= 0,
	BUTTON_STATE_PRESSED,
	BUTTON_STATE_RELEASED,
	BUTTON_STATE_DOWN,
	BUTTON_STATE_UP,
	BUTTON_STATE_INACTIVE = BUTTON_STATE_UP,
} button_state_t;

struct button {
	struct button_data data;
	const button_handlers_t *ops;
	int (*get_state)(void);
	bool pressed;
	bool active;
};

static struct {
	unsigned int (*get_monotonic_tick_in_ms)(void);
	void (*delay)(unsigned int ms);

	pthread_mutex_t lock;
	struct button buttons[BUTTON_MAX];
} m;

static void update_button(struct button *btn)
{
	unsigned int history = ACCESS_ONCE(btn->data.history);
	history <<= 1;
	history |= (unsigned int)(btn->get_state() & 1);
	btn->data.history = history;
}

static bool is_button_pressed(const struct button *btn)
{
	unsigned int expected = (1U << MIN_PRESSED_HISTORY) - 1;   // 0b0111111
	return (btn->data.history & HISTORY_MASK) == expected;
}

static bool is_button_released(const struct button *btn)
{
	unsigned int expected = 1U << MIN_PRESSED_HISTORY;         // 0b1000000
	return (btn->data.history & HISTORY_MASK) == expected;
}

static bool is_button_up(const struct button *btn)
{
	return (btn->data.history & HISTORY_MASK) == 0;
}

static bool is_button_down(const struct button *btn)
{
	return (btn->data.history & HISTORY_MASK) == HISTORY_MASK;
}

static button_state_t scan_button(struct button *btn, void *context)
{
	if (!btn->active) {
		return BUTTON_STATE_INACTIVE;
	}

	unsigned int t = m.get_monotonic_tick_in_ms();

	update_button(btn);

	if (is_button_pressed(btn) && !btn->pressed) {
		btn->data.time_pressed = t;
		btn->pressed = true;
		if (btn->ops->pressed) {
			btn->ops->pressed(&btn->data, context);
		}
		return BUTTON_STATE_PRESSED;
	} else if (is_button_released(btn) && btn->pressed) {
		btn->data.time_released = t;
		btn->pressed = false;
		if (btn->ops->released) {
			btn->ops->released(&btn->data, context);
		}
		return BUTTON_STATE_RELEASED;
	} else if (is_button_down(btn)) {
		return BUTTON_STATE_DOWN;
	} else if (is_button_up(btn)) {
		return BUTTON_STATE_UP;
	}

	return BUTTON_STATE_UNKNOWN;
}

static unsigned int scan_buttons(unsigned int bitmap, void *context)
{
	for (int i = 0; i < BUTTON_MAX; i++) {
		if (scan_button(&m.buttons[i], context) == BUTTON_STATE_UP) {
			bitmap &= ~(1U << i);
		}
	}

	return bitmap;
}

static void button_poll_internal(void *context)
{
	// it gets zeros when all buttons are up
	unsigned int bitmap = (1U << BUTTON_MAX) - 1;

	while ((bitmap = scan_buttons(bitmap, context)) != 0) {
		m.delay(BUTTON_SAMPLING_PERIOD_MS);
	}
}

static struct button *get_unused_button(void)
{
	for (int i = 0; i < BUTTON_MAX; i++) {
		if (!m.buttons[i].active) {
			return &m.buttons[i];
		}
	}

	return NULL;
}

void button_poll(void *context)
{
	button_poll_internal(context);
}

bool button_register(const button_handlers_t *handlers, int (*get_state)(void))
{
	if (handlers == NULL || get_state == NULL) {
		return false;
	}

	bool rc = false;

	pthread_mutex_lock(&m.lock);
	{
		struct button *btn = get_unused_button();

		if (btn == NULL) {
			goto out;
		}

		btn->ops = handlers;
		btn->get_state = get_state;
		btn->pressed = false;
		memset(&btn->data, 0, sizeof(btn->data));
		btn->active = true;
	}
	pthread_mutex_unlock(&m.lock);

	rc = true;
out:
	return rc;
}

void LIBMCU_WEAK button_hw_init(void)
{
}

void button_init(unsigned int (*get_monotonic_tick_in_ms)(void),
		void (*mydelay)(unsigned int ms))
{
	assert(get_monotonic_tick_in_ms != NULL);
	assert(mydelay != NULL);

	m.get_monotonic_tick_in_ms = get_monotonic_tick_in_ms;
	m.delay = mydelay;

	memset(m.buttons, 0, sizeof(m.buttons));

	pthread_mutex_init(&m.lock, NULL);

	button_hw_init();
}
