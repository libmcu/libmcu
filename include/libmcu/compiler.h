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

#define USED				__attribute__((used))
#define UNUSED				__attribute__((unused))
#define unused(x)			(void)(x)

#define barrier()			__asm__ __volatile__("" ::: "memory")
#define ACCESS_ONCE(x)			(*(volatile typeof(x) *)&(x))
#define WRITE_ONCE(ptr, val)
#define READ_ONCE(x)

#define NO_OPTIMIZE			__attribute__((optimize("O0")))

#define stringify(x)			#x
#define def2str(x)			stringify(x)

/** Align down */
#define BASE(x, unit)			((x) & ~((typeof(x))(unit) - 1UL))
/** Align up */
#define ALIGN(x, unit)			BASE((x) + ((typeof(x))(unit) - 1UL), unit)

#define container_of(ptr, type, member) \
	((type *)(void *)((char *)(ptr) - offsetof(type, member)))

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_COMPILER_H */
