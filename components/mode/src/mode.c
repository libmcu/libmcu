#include "libmcu/mode.h"

#include <pthread.h>
#include <stdlib.h>

#include "libmcu/list.h"
#include "libmcu/logging.h"

struct callback_obj {
	mode_transition_callback_t callback;
	struct list list;
};

static runmode_t current_mode = UNKNOWN_MODE;
static pthread_mutex_t mode_lock;
static struct list callbacks_list_head;

static inline const char *mode_stringify(runmode_t mode)
{
	switch (mode) {
	case PROVISIONING_MODE:
		return "provisioning";
	case REPORT_MODE:
		return "report";
	case UNKNOWN_MODE:
	default:
		break;
	}

	return "unknown";
}

void mode_set(runmode_t mode)
{
	debug("change mode from %s to %s",
			mode_stringify(current_mode),
			mode_stringify(mode));

	if ((mode >= UNKNOWN_MODE)
			|| (mode == current_mode)) {
		return;
	}

	pthread_mutex_lock(&mode_lock);
	{
		current_mode = mode;

		struct list *p;
		list_for_each(p, &callbacks_list_head) {
			list_entry(p, struct callback_obj, list)->callback((void *)mode);
		}
	}
	pthread_mutex_unlock(&mode_lock);
}

runmode_t mode_get(void)
{
	return current_mode;
}

static inline bool is_registered(const mode_transition_callback_t callback)
{
	struct list *p;
	bool rc = false;

	pthread_mutex_lock(&mode_lock);
	list_for_each(p, &callbacks_list_head) {
		if (list_entry(p, struct callback_obj, list)->callback == callback) {
			rc = true;
			break;
		}
	}
	pthread_mutex_unlock(&mode_lock);

	return rc;
}

int mode_register_transition_callback(const mode_transition_callback_t callback)
{
	if (is_registered(callback)) {
		return 0;
	}

	struct callback_obj *p;

	if (!(p = (struct callback_obj *)malloc(sizeof(*p)))) {
		return -1;
	}

	p->callback = callback;
	pthread_mutex_lock(&mode_lock);
	list_add(&p->list, &callbacks_list_head);
	pthread_mutex_unlock(&mode_lock);

	return 0;
}

int mode_unregister_transition_callback(const mode_transition_callback_t callback)
{
	pthread_mutex_lock(&mode_lock);

	struct list *p, *n;
	list_for_each_safe(p, n, &callbacks_list_head) {
		struct callback_obj *entry = list_entry(p, struct callback_obj, list);
		if (entry->callback == callback) {
			list_del(&entry->list, &callbacks_list_head);
			free(entry);
			break;
		}
	}

	pthread_mutex_unlock(&mode_lock);

	return 0;
}

void mode_init(void)
{
	list_init(&callbacks_list_head);
	pthread_mutex_init(&mode_lock, NULL);

	current_mode = UNKNOWN_MODE;
}
