#ifndef LIBMCU_BASE64_H
#define LIBMCU_BASE64_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

size_t base64_encode(char *out, const void *data, size_t datasize);
size_t base64_decode(void *out, const char *in, size_t input_size);
size_t base64_decode_overwrite(char *inout, size_t input_size);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BASE64_H */
