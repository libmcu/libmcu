#ifndef COMPILER_H
#define COMPILER_H

#if defined(TEST)
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

#endif /* COMPILER_H */
