#include "fake_storage.h"
#include "libmcu/logging.h"

#include <time.h>
#include <string.h>
#include <stdio.h>

static void (*print_string)(const char *str);

static const char *type_to_str(uint8_t type)
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

static inline const char *convert_to_string(char *buf, size_t bufsize, const void *data)
{
	const struct {
		time_t timestamp;
		uintptr_t pc;
		uintptr_t lr;
		uint16_t magic;
		uint8_t type;
		uint8_t message_length;
		uint8_t message[];
	} __attribute__((packed)) *p = data;

	int len = snprintf(buf, bufsize - 1, "%lu: [%s] <%#zx> ",
			p->timestamp, type_to_str(p->type), p->pc);
	buf[bufsize] = '\0';

	if (len > 0) {
		size_t msglen = (bufsize-(size_t)len-1) < p->message_length?
			(bufsize-(size_t)len-1) : p->message_length;
		memcpy(&buf[len], p->message, msglen);
		buf[msglen + (size_t)len] = '\0';
	}

	return buf;
}

static size_t fake_write(const void *data, size_t data_size)
{
#define BUFSIZE		128
	char strbuf[BUFSIZE];
	print_string(convert_to_string(strbuf, BUFSIZE, data));
	return data_size;
}

static size_t fake_read(void *buf, size_t bufsize)
{
	(void)buf;
	(void)bufsize;
	return 0;
}

static size_t fake_consume(size_t size)
{
	(void)size;
	return 0;
}

static size_t fake_count(void)
{
	return 0;
}

const logging_storage_t *logging_fake_storage_init(
		void (*_print_string)(const char *s))
{
	print_string = _print_string;

	static const logging_storage_t ops = {
		.write = fake_write,
		.read = fake_read,
		.consume = fake_consume,
		.count = fake_count,
	};

	return &ops;
}
