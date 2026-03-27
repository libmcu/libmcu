/*
 * SPDX-FileCopyrightText: 2026 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * @file mgmt_test.cpp
 *
 * Tests for the mgmt_init contract.
 *
 * Verifies the NULL-guard and boundary checks on struct mgmt_config:
 *   - null config pointer
 *   - null buf_pool
 *   - zero buf_size
 *   - zero buf_count
 *   - valid config
 *
 * No vtable dispatch tests: all vtables (transport / img / os) were removed.
 * Platform wiring (SMP transport, image/OS management) is handled internally
 * by mgmt_init() on each port; callers only supply the buffer pool.
 *
 * mgmt_init() is implemented as a lightweight stub below so the test runs
 * without any ESP-IDF or mcumgr dependencies.
 */

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"

#include <stdint.h>
#include <stddef.h>
#include <errno.h>

#include "libmcu/mgmt.h"

/* -------------------------------------------------------------------------
 * Stub mgmt_init — validates contract without platform dependencies.
 * The real implementation (mgmt_esp.c) additionally wires up the SMP
 * streamer and registers mcumgr groups.
 * ---------------------------------------------------------------------- */

int mgmt_init(const struct mgmt_config *cfg)
{
	if (!cfg || !cfg->buf_pool || cfg->buf_size == 0
			|| cfg->buf_count == 0) {
		return -EINVAL;
	}
	return 0;
}

void mgmt_deinit(void)
{
	/* stub */
}

/* -------------------------------------------------------------------------
 * Test fixtures
 * ---------------------------------------------------------------------- */

static uint8_t g_buf_pool[2][1032];

/* -------------------------------------------------------------------------
 * TEST GROUP: MgmtInit
 * ---------------------------------------------------------------------- */

TEST_GROUP(MgmtInit) {
	void setup(void) {}
	void teardown(void) {}
};

TEST(MgmtInit, nullConfig_ShouldReturnEINVAL) {
	LONGS_EQUAL(-EINVAL, mgmt_init(NULL));
}

TEST(MgmtInit, nullBufPool_ShouldReturnEINVAL) {
	const struct mgmt_config cfg = {
		.buf_pool  = NULL,
		.buf_size  = sizeof(g_buf_pool[0]),
		.buf_count = 2,
	};
	LONGS_EQUAL(-EINVAL, mgmt_init(&cfg));
}

TEST(MgmtInit, zeroBufSize_ShouldReturnEINVAL) {
	const struct mgmt_config cfg = {
		.buf_pool  = g_buf_pool,
		.buf_size  = 0,
		.buf_count = 2,
	};
	LONGS_EQUAL(-EINVAL, mgmt_init(&cfg));
}

TEST(MgmtInit, zeroBufCount_ShouldReturnEINVAL) {
	const struct mgmt_config cfg = {
		.buf_pool  = g_buf_pool,
		.buf_size  = sizeof(g_buf_pool[0]),
		.buf_count = 0,
	};
	LONGS_EQUAL(-EINVAL, mgmt_init(&cfg));
}

TEST(MgmtInit, validConfig_ShouldReturnZero) {
	const struct mgmt_config cfg = {
		.buf_pool  = g_buf_pool,
		.buf_size  = sizeof(g_buf_pool[0]),
		.buf_count = 2,
	};
	LONGS_EQUAL(0, mgmt_init(&cfg));
}
