#include "libmcu/apptimer.h"

#include <pthread.h>
#include <assert.h>

#include "libmcu/llist.h"
#include "libmcu/bitops.h"
#include "libmcu/compiler.h"
#include "libmcu/logging.h"

#if !defined(APPTIMER_NR_WHEELS)
#define APPTIMER_NR_WHEELS		5
#endif
#define NR_WHEELS			APPTIMER_NR_WHEELS
#define NR_SLOTS			8

#if !defined(MIN)
#define MIN(x, y)			((x) > (y)? (y) : (x))
#endif

#define time_before(goal, chasing)	((int)(chasing) - (int)(goal)    < 0)

#define SLOTS_BITS			(fls(NR_SLOTS) - 1)
#define SLOTS_MASK			((1UL << SLOTS_BITS) - 1)
#define WHEELS_BITS			(SLOTS_BITS * NR_WHEELS)
#define MAX_WHEELS_TIMEOUT		(1UL << WHEELS_BITS)

struct apptimer {
	apptimer_timeout_t interval;
	apptimer_timeout_t goaltime;
	bool repeat;
	apptimer_callback_t callback;
	void *context;
	struct llist list;
};
LIBMCU_STATIC_ASSERT(sizeof(struct apptimer) == sizeof(apptimer_t),
		"apptimer_t must be larger or equal to struct apptimer.");

static struct {
	pthread_mutex_t wheels_lock;
	struct llist wheels[NR_WHEELS][NR_SLOTS];
	struct llist pending;
	apptimer_timeout_t time_counter;
	int active_timers;
	void (*update_alarm)(apptimer_timeout_t timeout);
} m;

static int get_wheel_index_from_timeout(apptimer_timeout_t timeout)
{
	assert(timeout != 0);
	return MIN(WHEELS_BITS - 1, fls(timeout) - 1) / SLOTS_BITS;
}

static int get_slot_index_from_timeout(apptimer_timeout_t timeout,
		int wheel)
{
	return (int)((timeout >> (SLOTS_BITS * wheel)) & SLOTS_MASK);
}

static apptimer_timeout_t get_timer_counter(void)
{
	return m.time_counter;
}

static void set_timer_counter(apptimer_timeout_t t)
{
	m.time_counter = t;
}

static apptimer_timeout_t get_time_distance(apptimer_timeout_t goal,
		apptimer_timeout_t chasing)
{
	return goal - chasing;
}

static bool is_timer_expired(const struct apptimer * const timer)
{
	return !time_before(timer->goaltime, get_timer_counter());
}

static bool is_timer_registered(const struct apptimer * const timer)
{
	return !llist_empty(&timer->list);
}

static void remove_timer_from_list(struct apptimer * const timer)
{
	if (llist_empty(&timer->list)) {
		return;
	}

	llist_del(&timer->list);
	m.active_timers--;

	llist_init(&timer->list);
}

static void insert_timer_into_pending(struct apptimer * const timer)
{
	llist_add(&timer->list, &m.pending);
}

static void insert_timer_into_wheel(struct apptimer * const timer)
{
	m.active_timers++;

	if (is_timer_expired(timer)) {
		insert_timer_into_pending(timer);
		debug("%lx: Insert timer in pending %d %d", get_timer_counter(),
				timer->goaltime, timer->interval);
		return;
	}

	apptimer_timeout_t current_time = get_timer_counter();
	apptimer_timeout_t delta = get_time_distance(timer->goaltime, current_time);
	apptimer_timeout_t split = current_time & SLOTS_MASK;
	int wheel = get_wheel_index_from_timeout(delta + split);
	int slot = get_slot_index_from_timeout(delta + split, wheel);

	llist_add(&timer->list, &m.wheels[wheel][slot]);

	debug("%lu: Insert timer(%lu) in wheel %d slot %d",
			current_time, timer->goaltime, wheel, slot);
}

static apptimer_timeout_t find_earliest_timer_wheel_timeout(void)
{
	if (m.active_timers <= 0) {
		goto out;
	}

	for (int i = 0; i < NR_WHEELS; i++) {
		for (int j = 0; j < NR_SLOTS; j++) {
			struct llist *p;
			llist_for_each(p, &m.wheels[i][j]) {
				return 1UL << (i * SLOTS_BITS);
			}
		}
	}
out:
	return MAX_WHEELS_TIMEOUT;
}

static void update_slots(int wheel, int slot, int n)
{
	DEFINE_LLIST_HEAD(tmp_lists);
	struct llist *p, *t;

	for (unsigned int i = (unsigned int)slot+1; i > 0 && n; i--, n--) {
		llist_for_each_safe(p, t, &m.wheels[wheel][i-1]) {
			llist_del(p);
			llist_add(p, &tmp_lists);
		}
	}

	llist_for_each_safe(p, t, &tmp_lists) {
		llist_del(p);
		m.active_timers--;
		insert_timer_into_wheel(llist_entry(p, struct apptimer, list));
	}
}

static void update_whole_slots(int wheel)
{
	update_slots(wheel, NR_SLOTS-1, NR_SLOTS);
}

static void run_pending_timers(void)
{
	struct llist *p, *t;
	llist_for_each_safe(p, t, &m.pending) {
		llist_del(p);
		llist_init(p);
		m.active_timers--;

		struct apptimer *timer = llist_entry(p, struct apptimer, list);
		timer->callback(timer->context);

		if (timer->repeat) {
			timer->goaltime = get_timer_counter() + timer->interval;
			insert_timer_into_wheel(timer);
		}
	}
}

apptimer_error_t apptimer_start(apptimer_t * const timer,
		apptimer_timeout_t timeout, void *callback_context)
{
	struct apptimer *p = (struct apptimer *)timer;

	if (!p) {
		return APPTIMER_INVALID_PARAM;
	}
	if (is_timer_registered(p)) {
		return APPTIMER_ALREADY_STARTED;
	}
	if (timeout > APPTIMER_MAX_TIMEOUT) {
		return APPTIMER_TIME_LIMIT_EXCEEDED;
	}

	p->interval = timeout;
	p->goaltime = get_timer_counter() + p->interval;
	p->context = callback_context;

	pthread_mutex_lock(&m.wheels_lock);
	{
		insert_timer_into_wheel(p);

		if (m.update_alarm) {
			m.update_alarm(find_earliest_timer_wheel_timeout());
		}
	}
	pthread_mutex_unlock(&m.wheels_lock);

	return APPTIMER_SUCCESS;
}

apptimer_t *apptimer_create_static(apptimer_t * const timer, bool repeat,
		apptimer_callback_t callback)
{
	struct apptimer *p = (struct apptimer *)timer;

	if (!p || !callback) {
		return NULL;
	}

	p->repeat = repeat;
	p->callback = callback;
	llist_init(&p->list);

	return timer;
}

// TODO: Implement `apptimer_create()`
apptimer_t *apptimer_create(bool repeat, apptimer_callback_t callback)
{
	unused(repeat);
	unused(callback);
	return NULL;
}

apptimer_error_t apptimer_stop(apptimer_t * const timer)
{
	struct apptimer *p = (struct apptimer *)timer;

	pthread_mutex_lock(&m.wheels_lock);
	{
		remove_timer_from_list(p);
	}
	pthread_mutex_unlock(&m.wheels_lock);

	return APPTIMER_SUCCESS;
}

apptimer_error_t apptimer_delete(apptimer_t *timer)
{
	struct apptimer *p = (struct apptimer *)timer;

	// TODO: free if the timer created dynamically
	pthread_mutex_lock(&m.wheels_lock);
	{
		remove_timer_from_list(p);
	}
	pthread_mutex_unlock(&m.wheels_lock);

	return APPTIMER_SUCCESS;
}

int apptimer_count(void)
{
	return m.active_timers;
}

void apptimer_schedule(apptimer_timeout_t time_elapsed)
{
	if (time_elapsed > APPTIMER_MAX_TIMEOUT) {
		error("time overrun %lu / %lu",
				time_elapsed, APPTIMER_MAX_TIMEOUT);
	}

	apptimer_timeout_t previous_time = get_timer_counter();
	apptimer_timeout_t current_time = previous_time + time_elapsed;
	apptimer_timeout_t diff_time = current_time ^ previous_time;

	int farmost_wheel = get_wheel_index_from_timeout(diff_time);
	int slot = (diff_time >= MAX_WHEELS_TIMEOUT)? (int)SLOTS_MASK :
		get_slot_index_from_timeout(current_time, farmost_wheel);
	debug("schedule %lx(%lx): wheel %d slot %d",
			current_time, diff_time, farmost_wheel, slot);

	pthread_mutex_lock(&m.wheels_lock);
	{
		set_timer_counter(current_time);

		for (int wheel = 0; wheel < farmost_wheel; wheel++) {
			update_whole_slots(wheel);
		}
		update_slots(farmost_wheel, slot, (int)time_elapsed);

		run_pending_timers();

		if (m.update_alarm) {
			m.update_alarm(find_earliest_timer_wheel_timeout());
		}
	}
	pthread_mutex_unlock(&m.wheels_lock);
}

void apptimer_init(void (*update_alarm)(apptimer_timeout_t timeout))
{
	debug("slots bits %d, max timeout %lu, wheels bits %d:%lu", SLOTS_BITS,
			APPTIMER_MAX_TIMEOUT, WHEELS_BITS, MAX_WHEELS_TIMEOUT-1);

	pthread_mutex_init(&m.wheels_lock, NULL);
	m.update_alarm = update_alarm;
	m.time_counter = 0;
	m.active_timers = 0;
	llist_init(&m.pending);

	for (int i = 0; i < NR_WHEELS; i++) {
		for (int j = 0; j < NR_SLOTS; j++) {
			llist_init(&m.wheels[i][j]);
		}
	}
}

apptimer_error_t apptimer_deinit(void)
{
	pthread_mutex_lock(&m.wheels_lock);
	pthread_mutex_unlock(&m.wheels_lock);

	return APPTIMER_SUCCESS;
}
