#include "libmcu/logging.h"

#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#if defined(__STDC_HOSTED__)
#include <stdio.h>
#endif

#include "libmcu/compiler.h"

#define LOGGING_MAGIC				0xA5A5U

typedef uint16_t logging_magic_t;

typedef struct {
	time_t timestamp;
	uintptr_t pc;
	uintptr_t lr;
	logging_magic_t magic;
	logging_t type;
	uint8_t message_length;
	uint8_t message[];
} LIBMCU_PACKED logging_data_t;
LIBMCU_STATIC_ASSERT(sizeof(logging_t) == sizeof(uint8_t),
		"The size of logging_t must be the same of uint8_t.");
LIBMCU_STATIC_ASSERT(LOGGING_TYPE_MAX <= (1U << (sizeof(logging_t) * 8)) - 1,
		"TYPE_MAX must not exceed its data type size.");
LIBMCU_STATIC_ASSERT(LOGGING_MESSAGE_MAXLEN
		< (1U << (sizeof(((logging_data_t *)0)->message_length) * 8)),
		"MESSAGE_MAXLEN must not exceed its data type size.");

static struct {
	const logging_storage_t *storage;
	logging_t min_save_level;
} m;

static inline const char *stringify_type(logging_t type)
{
	switch (type) {
	case LOGGING_TYPE_VERBOSE:
		return "VERBOSE";
	case LOGGING_TYPE_DEBUG:
		return "DEBUG";
	case LOGGING_TYPE_INFO:
		return "INFO";
	case LOGGING_TYPE_NOTICE:
		return "NOTICE";
	case LOGGING_TYPE_WARN:
		return "WARN";
	case LOGGING_TYPE_ERROR:
		return "ERROR";
	case LOGGING_TYPE_ALERT:
		return "ALERT";
	default:
		break;
	}
	return "UNKNOWN";
}

static inline logging_magic_t compute_magic(const logging_data_t *entry)
{
	return (logging_magic_t)(entry->pc ^ entry->lr ^ LOGGING_MAGIC);
}

static inline bool is_logging_type_enabled(const logging_t type)
{
	if (type < m.min_save_level) {
		return false;
	}
	return true;
}

static inline bool is_logging_type_valid(const logging_t type)
{
	if (type >= LOGGING_TYPE_MAX) {
		return false;
	}
	return true;
}

#if 0
static inline bool is_log_valid(const logging_data_t *entry)
{
	if (compute_magic(entry) != entry->magic) {
		return false;
	}
	if (!is_logging_type_valid(entry->type)) {
		return false;
	}
	if (entry->message_length > LOGGING_MESSAGE_MAXLEN) {
		return false;
	}
	return true;
}
#endif

static inline size_t get_log_size(const logging_data_t *entry)
{
	size_t size = sizeof(*entry);

	if (entry && entry->message_length) {
		size += entry->message_length;
	}

	return size;
}

static inline size_t logging_peek_internal(void *buf, size_t bufsize)
{
	if (!buf || bufsize < sizeof(logging_data_t)) {
		return 0;
	}
	return m.storage->read(buf, bufsize);
}

static inline size_t logging_consume_internal(size_t size)
{
	if (!size) {
		return 0;
	}
	return m.storage->consume(size);
}

#define pack_message(ptr, lr) do { \
	va_list ap; \
	const char *fmt; \
	int len = 0; \
	va_start(ap, lr); \
	fmt = va_arg(ap, char *); \
	if (fmt) { \
		len = vsnprintf((char *)ptr->message, LOGGING_MESSAGE_MAXLEN, \
				fmt, ap); \
	} \
	va_end(ap); \
	if (len > 0 && len <= LOGGING_MESSAGE_MAXLEN) { \
		ptr->message_length = (typeof(ptr->message_length))len; \
	} \
} while (0)

static inline void pack_log(logging_data_t *entry, logging_t type,
		const void *pc, const void *lr)
{
	*entry = (logging_data_t) {
		.timestamp = time(NULL),
		.type = type,
		.pc = (uintptr_t)pc,
		.lr = (uintptr_t)lr,
		.magic = LOGGING_MAGIC,
		.message_length = 0,
	};
	entry->magic = compute_magic(entry);
}

size_t logging_save(logging_t type, const void *pc, const void *lr, ...)
{
	size_t bytes_written = 0;

	if (!is_logging_type_valid(type)) {
		goto out; // ERROR_NOT_SUPPORTED_TYPE;
	}
	if (!is_logging_type_enabled(type)) {
		goto out; // ERROR_MIN_SAVE_LEVEL;
	}

	uint8_t buf[LOGGING_MESSAGE_MAXLEN + sizeof(logging_data_t)];
	logging_data_t *log = (logging_data_t *)buf;
	pack_log(log, type, pc, lr);
	pack_message(log, lr);
	// TODO: logging_encode(log)

	bytes_written = m.storage->write(log, get_log_size(log));
out:
	return bytes_written;
}

size_t logging_peek(void *buf, size_t bufsize)
{
	return logging_peek_internal(buf, bufsize);
}

size_t logging_consume(size_t size)
{
	return logging_consume_internal(size);
}

size_t logging_read(void *buf, size_t bufsize)
{
	size_t size_read = logging_peek_internal(buf, bufsize);
	logging_consume_internal(size_read);
	return size_read;
}

size_t logging_count(void)
{
	return m.storage->count();
}

void logging_set_level(logging_t min_log_level)
{
	if (min_log_level < LOGGING_TYPE_MAX) {
		m.min_save_level = min_log_level;
	}
}

logging_t logging_get_level(void)
{
	return m.min_save_level;
}

const char *logging_stringify(char *buf, size_t bufsize, const void *log)
{
	const logging_data_t *p = (const logging_data_t *)log;
	int len = snprintf(buf, bufsize-1, "%lu: [%s] <%p,%p> ",
			(unsigned long)p->timestamp, stringify_type(p->type),
			(void *)p->pc, (void *)p->lr);
	buf[bufsize] = '\0';

	if (len > 0) {
		size_t msglen = (bufsize - (size_t)len - 1) < p->message_length?
			(bufsize - (size_t)len - 1) : p->message_length;
		memcpy(&buf[len], p->message, msglen);
		buf[msglen + (size_t)len] = '\0';
	}

	return buf;
}

void logging_init(const logging_storage_t *ops)
{
	m.storage = ops;
	logging_set_level(LOGGING_TYPE_DEBUG);
}
