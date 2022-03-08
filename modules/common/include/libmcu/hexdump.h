#ifndef LIBMCU_HEXDUMP_H
#define LIBMCU_HEXDUMP_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

size_t hexdump(void *buf, size_t bufsize, void const *data, size_t datasize);
size_t hexdump_verbose(void *buf, size_t bufsize,
		void const *data, size_t datasize);
size_t hexdump_compute_output_size(size_t datasize);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_HEXDUMP_H */
