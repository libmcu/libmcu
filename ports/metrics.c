#include "libmcu/metrics.h"
#include <pthread.h>
#include "cbor/cbor.h"
#include "cbor/encoder.h"

static pthread_mutex_t lock;
static cbor_writer_t writer;

void metrics_lock_init(void)
{
	pthread_mutex_init(&lock, NULL);
}

void metrics_lock(void)
{
	pthread_mutex_lock(&lock);
}

void metrics_unlock(void)
{
	pthread_mutex_unlock(&lock);
}

size_t metrics_encode_header(void *buf, size_t bufsize,
		uint32_t nr_total, uint32_t nr_updated)
{
	cbor_writer_init(&writer, buf, bufsize);
	cbor_encode_map(&writer, nr_updated);

	return cbor_writer_len(&writer);
}

size_t metrics_encode_each(void *buf, size_t bufsize,
		metric_key_t key, int32_t value)
{
	if (value == 0) {
		return 0;
	}

	size_t len = cbor_writer_len(&writer);

	cbor_encode_unsigned_integer(&writer, (uint64_t)key);

	if (value >= 0) {
		cbor_encode_unsigned_integer(&writer, (uint64_t)value);
	} else {
		cbor_encode_negative_integer(&writer, value);
	}

	return cbor_writer_len(&writer) - len;
}
