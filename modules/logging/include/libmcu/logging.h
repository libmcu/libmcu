/*
 * SPDX-FileCopyrightText: 2020 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_LOGGING_H
#define LIBMCU_LOGGING_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "libmcu/logging_backend.h"
#include "libmcu/compiler.h"

#if !defined(LOGGING_MESSAGE_MAXLEN)
/** Message itself only. `logging_data_t` type size overhead should also take
 * place. */
#define LOGGING_MESSAGE_MAXLEN			80
#endif
#if !defined(LOGGING_TAGS_MAXNUM)
#define LOGGING_TAGS_MAXNUM			8
#endif
#if !defined(LOGGING_TAG)
#define FILENAME_TAG				\
	(__builtin_strrchr(__FILE__, '/')?	\
		__builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#define LOGGING_TAG				FILENAME_TAG
#endif

#define logging_set_level(level)	\
	logging_set_level_tag(LOGGING_TAG, level)
#define logging_get_level()		\
	logging_get_level_tag(LOGGING_TAG)

enum logging_type {
	LOGGING_TYPE_DEBUG			= 0,
	LOGGING_TYPE_INFO,
	LOGGING_TYPE_WARN,
	LOGGING_TYPE_ERROR,
	LOGGING_TYPE_NONE,
	LOGGING_TYPE_MAX,
};
typedef uint8_t logging_t;

struct logging_context {
	const char *tag;
	const void *pc;
	const void *lr;
};

typedef uint32_t (*logging_time_func_t)(void);

#if !defined(get_program_counter)
#define get_program_counter()		libmcu_get_pc()
#endif

#define LOGGING_WRAPPER(type, ...) do { 		\
	const struct logging_context _logctx = {	\
		.tag = LOGGING_TAG,			\
		.pc = get_program_counter(),		\
		.lr = __builtin_return_address(0),	\
	};						\
	logging_write(type, &_logctx, __VA_ARGS__);	\
} while (0)

#define debug(...) \
	LOGGING_WRAPPER(LOGGING_TYPE_DEBUG, __VA_ARGS__)
#define info(...) \
	LOGGING_WRAPPER(LOGGING_TYPE_INFO, __VA_ARGS__)
#define warn(...) \
	LOGGING_WRAPPER(LOGGING_TYPE_WARN, __VA_ARGS__)
#define error(...) \
	LOGGING_WRAPPER(LOGGING_TYPE_ERROR, __VA_ARGS__)

void logging_init(logging_time_func_t time_func);

int logging_add_backend(const struct logging_backend *backend);
int logging_remove_backend(const struct logging_backend *backend);

size_t logging_write_with_backend(logging_t type,
		const struct logging_backend *backend,
		const struct logging_context *ctx, ...);
size_t logging_write(logging_t type, const struct logging_context *ctx, ...);
size_t logging_read(const struct logging_backend *backend,
		void *buf, size_t bufsize);
size_t logging_peek(const struct logging_backend *backend,
		void *buf, size_t bufsize);
size_t logging_consume(const struct logging_backend *backend,
		size_t consume_size);
size_t logging_count(const struct logging_backend *backend);

size_t logging_stringify(char *buf, size_t bufsize, const void *log);

/**
 * @brief Change the minimum log level to be saved for the tag
 *
 * A log gets saved only when the global log level is lower than the tag log
 * level.
 *
 * @param tag module tag
 * @param min_log_level one of @ref logging_t
 */
void logging_set_level_tag(const char *tag, logging_t min_log_level);
logging_t logging_get_level_tag(const char *tag);
void logging_set_level_global(logging_t min_log_level);
logging_t logging_get_level_global(void);

size_t logging_count_tags(void);
void logging_iterate_tag(void (*callback_each)(const char *tag,
			logging_t min_log_level));

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_LOGGING_H */
