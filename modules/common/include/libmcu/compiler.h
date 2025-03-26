/*
 * SPDX-FileCopyrightText: 2020 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_COMPILER_H
#define LIBMCU_COMPILER_H

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(UNITTEST)
#define STATIC
#else
#define STATIC				static
#endif

#define unused(x)			(void)(x)

#if defined(__GNUC__) || defined(__clang__)
#define LIBMCU_UNUSED			__attribute__((unused))
#define LIBMCU_USED			__attribute__((used))
#define LIBMCU_ALWAYS_INLINE		__attribute__((always_inline))
#define LIBMCU_WEAK			__attribute__((weak))
#define LIBMCU_NORETURN			__attribute__((noreturn))
#define LIBMCU_PACKED			__attribute__((packed))
#define LIBMCU_NO_INSTRUMENT		__attribute__((no_instrument_function))
#define NO_OPTIMIZE			__attribute__((optimize("O0")))
#elif defined(_MSC_VER)
#define LIBMCU_UNUSED
#define LIBMCU_USED
#define LIBMCU_ALWAYS_INLINE		__forceinline
#define LIBMCU_WEAK
#define LIBMCU_NORETURN			__declspec(noreturn)
#define LIBMCU_PACKED
#define LIBMCU_NO_INSTRUMENT
#define NO_OPTIMIZE			__pragma(optimize("", off))
#else
#define LIBMCU_UNUSED
#define LIBMCU_USED
#define LIBMCU_ALWAYS_INLINE
#define LIBMCU_WEAK
#define LIBMCU_NORETURN
#define LIBMCU_PACKED
#define LIBMCU_NO_INSTRUMENT
#define NO_OPTIMIZE
#endif

#if !defined(static_assert)
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#define static_assert _Static_assert
#elif defined(__cplusplus) && __cplusplus >= 201103L
#include <cassert>
#else
#define static_assert_paste(a, b)	a ## b
#define static_assert_expand(a,b)	static_assert_paste(a, b)
#define static_assert(expr, msg)	\
	enum { static_assert_expand(ASSERT_line_,__LINE__) = 1 / (expr) }
#endif
#endif

#define CONST_CAST(t, v)		\
	(((union { const t cval; t val; }*)&(v))->val)

#define stringify(x)			#x
#define def2str(x)			stringify(x)

#define LIBMCU_CONCAT(a, b)		(a ## b)

/** Align down */
#define BASE(x, unit)			((x) & ~((__typeof__(x))(unit) - 1UL))
/** Align up */
#define ALIGN(x, unit)			BASE((x) + ((__typeof__(x))(unit) - 1UL), unit)

#define container_of(ptr, type, member) \
	((type *)(void *)((char *)(ptr) - offsetof(type, member)))

#define libmcu_get_lr()			__builtin_return_address(0)
#if defined(__clang__) || defined(__CC_ARM) || defined(__ICCARM__)
#define libmcu_get_pc()			((void *)0xfeedc0de)
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wreturn-local-addr"
LIBMCU_NO_INSTRUMENT
static inline void *libmcu_get_pc(void)
{
	__label__ l;
l:
	return &&l;
}
#pragma GCC diagnostic pop
#else
#error "Unsupported compiler"
#endif

#if !defined(LIBMCU_NOINIT)
#if defined(__APPLE__) && defined(__MACH__)
#define LIBMCU_NOINIT		__attribute__((section("__NOINIT,__noinit")))
#elif defined(_MSC_VER)
#pragma section("BSS", read, write)
#define LIBMCU_NOINIT		__declspec(allocate("BSS"))
#else
#define LIBMCU_NOINIT		__attribute__((section(".noinit")))
#endif
#endif

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_COMPILER_H */
