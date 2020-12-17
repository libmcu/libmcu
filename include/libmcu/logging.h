#ifndef LIBMCU_LOGGING_H
#define LIBMCU_LOGGING_H 202012L

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "libmcu/logging_storage.h"

#if !defined(LOGGING_MESSAGE_MAXLEN)
/** Message itself only. `logging_data_t` type size overhead should also take
 * place. */
#define LOGGING_MESSAGE_MAXLEN				80
#endif

enum logging_type {
	LOGGING_TYPE_VERBOSE				= 0,
	LOGGING_TYPE_DEBUG,
	LOGGING_TYPE_INFO,
	LOGGING_TYPE_NOTICE,
	LOGGING_TYPE_WARN,
	LOGGING_TYPE_ERROR,
	LOGGING_TYPE_ALERT,
	LOGGING_TYPE_MAX,
};
typedef uint8_t logging_t;

void logging_init(const logging_storage_t *ops);
size_t logging_save(logging_t type, const void *pc, const void *lr, ...);
size_t logging_read(void *buf, size_t bufsize);
size_t logging_peek(void *buf, size_t bufsize);
size_t logging_consume(size_t size);
size_t logging_count(void);
const char *logging_stringify(char *buf, size_t bufsize, const void *log);
/** Change the minimum log level to be saved in the storage
 *
 * `LOGGING_TYPE_DEBUG` by default.
 *
 * @param min_log_level Refer to `logging_t` for the list to specify
 */
void logging_set_level(logging_t min_log_level);
logging_t logging_get_level(void);

/* Wrappers are below for convenience */
#if !defined(get_program_counter)
	#if defined(__GNUC__)
	#define get_program_counter()		({__label__ l; l: &&l; })
	#else
	#define get_program_counter()		((void *)0xfeedc0de)
	#endif
#endif

#define LOGGING_WRAPPER(type, ...) do { 				\
	logging_save(type, get_program_counter(),			\
			__builtin_return_address(0), __VA_ARGS__);	\
} while (0)

#define verbose(...) \
	LOGGING_WRAPPER(LOGGING_TYPE_VERBOSE, __VA_ARGS__)
#define debug(...) \
	LOGGING_WRAPPER(LOGGING_TYPE_DEBUG, __VA_ARGS__)
#define info(...) \
	LOGGING_WRAPPER(LOGGING_TYPE_INFO, __VA_ARGS__)
#define notice(...) \
	LOGGING_WRAPPER(LOGGING_TYPE_NOTICE, __VA_ARGS__)
#define warn(...) \
	LOGGING_WRAPPER(LOGGING_TYPE_WARN, __VA_ARGS__)
#define error(...) \
	LOGGING_WRAPPER(LOGGING_TYPE_ERROR, __VA_ARGS__)
#define alert(...) \
	LOGGING_WRAPPER(LOGGING_TYPE_ALERT, __VA_ARGS__)

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_LOGGING_H */
