#ifndef LIBMCU_ASSERT_OVERRIDE_H
#define LIBMCU_ASSERT_OVERRIDE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "libmcu/compiler.h"

#define assert(exp)			(unused(exp))

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_ASSERT_OVERRIDE_H */
