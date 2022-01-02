#ifndef LIBMCU_ASSERT_H
#define LIBMCU_ASSERT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include "libmcu/compiler.h"

#undef assert
#define assert(exp) \
	do { \
		if (!(exp)) { \
			const uintptr_t *pc = (const uintptr_t *)libmcu_get_pc(); \
			const uintptr_t *lr = (const uintptr_t *)libmcu_get_lr(); \
			libmcu_assertion_failed(pc, lr); \
		} \
	} while (0)

void libmcu_assertion_failed(const uintptr_t *pc, const uintptr_t *lr);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_ASSERT_H */
