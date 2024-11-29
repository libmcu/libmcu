#include "CppUTestExt/MockSupport.h"
#include "libmcu/assert.h"

void libmcu_assertion_failed(const uintptr_t *pc, const uintptr_t *lr) {
	mock().actualCall("libmcu_assertion_failed")
		.withParameter("pc", pc)
		.withParameter("lr", lr);
}
