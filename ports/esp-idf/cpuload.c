/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SEC_TO_USEC(sec)		((sec) * 1000 * 1000)
#define MIN_TO_USEC(min)		((min) * 60 * 1000 * 1000)
#define MIN_TO_SEC(min)			((min) * 60)

enum {
	CPULOAD_PERIOD_SEC,
	CPULOAD_PERIOD_MIN,
	CPULOAD_PERIOD_5MIN,
	CPULOAD_PERIOD_MAX,
};

struct cpuload_entry {
	uint32_t idle_time_elapsed;
	uint32_t running_time_elapsed;
	uint32_t total_elapsed;
	uint8_t cpuload;
};

struct cpuload {
	struct cpuload_entry entry[CPULOAD_PERIOD_MAX];
	TaskHandle_t prev_task;
	uint64_t t0;
};

/* NOTE: Keep cpuload as 100% at the beginning to avoid side effects. */
static struct cpuload cores[SOC_CPU_CORES_NUM] = {
	[0] = {
		.entry = {
			[CPULOAD_PERIOD_SEC] = { .cpuload = 100 },
			[CPULOAD_PERIOD_MIN] = { .cpuload = 100 },
			[CPULOAD_PERIOD_5MIN] = { .cpuload = 100 },
		},
	},
	[1] = {
		.entry = {
			[CPULOAD_PERIOD_SEC] = { .cpuload = 100 },
			[CPULOAD_PERIOD_MIN] = { .cpuload = 100 },
			[CPULOAD_PERIOD_5MIN] = { .cpuload = 100 },
		},
	},
};

static void increase_idle_time(struct cpuload *core, uint32_t elapsed)
{
	for (int i = 0; i < CPULOAD_PERIOD_MAX; i++) {
		core->entry[i].idle_time_elapsed += elapsed;
		core->entry[i].total_elapsed += elapsed;
	}
}

static void increase_running_time(struct cpuload *core, uint32_t elapsed)
{
	for (int i = 0; i < CPULOAD_PERIOD_MAX; i++) {
		core->entry[i].running_time_elapsed += elapsed;
		core->entry[i].total_elapsed += elapsed;
	}
}

static void clear_time(struct cpuload_entry *entry)
{
	entry->idle_time_elapsed = 0;
	entry->running_time_elapsed = 0;
	entry->total_elapsed = 0;
}

static uint8_t calculate_cpuload_pct(struct cpuload_entry *entry)
{
	return (uint8_t)(entry->running_time_elapsed * 100 /
			(entry->running_time_elapsed +
					entry->idle_time_elapsed));
}

static void update_cpuload(struct cpuload *core, const uint64_t t1,
		TaskHandle_t current)
{
	uint32_t period[CPULOAD_PERIOD_MAX] = {
		SEC_TO_USEC(1),
		MIN_TO_USEC(1),
		MIN_TO_USEC(5),
	};

	for (int i = 0; i < CPULOAD_PERIOD_MAX; i++) {
		struct cpuload_entry *entry = &core->entry[i];

		if (entry->total_elapsed >= period[i]) {
			entry->cpuload = calculate_cpuload_pct(entry);
			clear_time(entry);
		}
	}

	core->t0 = t1;
	core->prev_task = current;
}

void on_task_switch_in(void)
{
	const uint64_t t1 = (uint64_t)esp_timer_get_time();
	const TaskHandle_t current = xTaskGetCurrentTaskHandle();
	const TaskHandle_t idle_task = xTaskGetIdleTaskHandle();
	struct cpuload *core = &cores[xPortGetCoreID()];
	uint32_t elapsed = (uint32_t)(t1 - core->t0);

	/* NOTE: count at least 1 even if the task has run for much shorter time
	 * as millisecond unit timer used here. For fine granularity, introduce
	 * high-resolution timer. */
	if (elapsed == 0) {
		elapsed = 1;
	}

	if (current == idle_task) {
		if (core->prev_task == idle_task) { /* idle to idle */
			increase_idle_time(core, elapsed);
		} else { /* active to idle */
			increase_running_time(core, elapsed);
		}
	} else {
		if (core->prev_task == idle_task) { /* idle to active */
			increase_idle_time(core, elapsed);
		} else { /* active to active */
			increase_running_time(core, elapsed);
		}
	}

	update_cpuload(core, t1, current);
}

uint8_t board_cpuload(int core_id, uint32_t period_sec)
{
	if (period_sec >= MIN_TO_SEC(5)) {
		return cores[core_id].entry[CPULOAD_PERIOD_5MIN].cpuload;
	} else if (period_sec >= MIN_TO_SEC(1)) {
		return cores[core_id].entry[CPULOAD_PERIOD_MIN].cpuload;
	}

	return cores[core_id].entry[CPULOAD_PERIOD_SEC].cpuload;
}
