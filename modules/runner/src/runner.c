/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/runner.h"
#include <string.h>

static struct {
	const struct runner *runners;
	size_t nr_runners;

	runner_change_cb_t pre_change_cb;
	void *pre_change_cb_ctx;
	runner_change_cb_t post_change_cb;
	void *post_change_cb_ctx;

	const struct runner *current;
} m;

static const struct runner *find_runner_by_type(const runner_t runner_type)
{
	for (size_t i = 0; i < m.nr_runners; i++) {
		if (m.runners[i].type == runner_type) {
			return &m.runners[i];
		}
	}

	return NULL;
}

static const struct runner *get_current(void)
{
	return m.current;
}

static runner_t get_current_type(void)
{
	return get_current()->type;
}

static void set_current(const struct runner *runner)
{
	m.current = runner;
}

runner_t runner_type(const struct runner *runner)
{
	return runner->type;
}

const struct runner *runner(void)
{
	return get_current();
}

int runner_change(const runner_t new_runner_type)
{
	if (new_runner_type == get_current_type()) {
		return -EALREADY;
	}

	const struct runner *old_runner = get_current();
	const struct runner *new_runner = find_runner_by_type(new_runner_type);

	if (new_runner == NULL) {
		return -EINVAL;
	}

	if (m.pre_change_cb) {
		if (!m.pre_change_cb(new_runner_type, m.pre_change_cb_ctx)) {
			return -EFAULT;
		}
	}

	old_runner->api->terminate();
	set_current(new_runner);

	if (m.post_change_cb) {
		m.post_change_cb(new_runner_type, m.post_change_cb_ctx);
	}

	return new_runner->api->prepare();
}

void runner_start(const runner_t runner_type)
{
	set_current(find_runner_by_type(runner_type));

	if (get_current()->api->prepare) {
		get_current()->api->prepare();
	}
}

int runner_register_change_cb(runner_change_cb_t pre_cb, void *pre_ctx,
		runner_change_cb_t post_cb, void *post_ctx)
{
	m.pre_change_cb = pre_cb;
	m.pre_change_cb_ctx = pre_ctx;
	m.post_change_cb = post_cb;
	m.post_change_cb_ctx = post_ctx;

	return 0;
}

int runner_init(const struct runner *runners, size_t nr_runners)
{
	if (!runners || nr_runners == 0) {
		return -ENOENT;
	}

	runner_t duplicate_check[nr_runners];

	memset(duplicate_check, 0, sizeof(duplicate_check));

	for (size_t i = 0; i < nr_runners; i++) {
		const struct runner *r = &runners[i];
		duplicate_check[i] = r->type;
		for (size_t j = 0; j < i; j++) {
			if (duplicate_check[j] == r->type) {
				return -EINVAL;
			}
		}
	}

	m.runners = runners;
	m.nr_runners = nr_runners;

	return 0;
}
