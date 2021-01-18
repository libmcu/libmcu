#ifndef LIBMCU_SHELL_H
#define LIBMCU_SHELL_H 202012L

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

typedef struct {
	size_t (*read)(void *buf, size_t bufsize);
	size_t (*write)(const void *data, size_t data_size);
} shell_io_t;

void shell_run(const shell_io_t *io);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_SHELL_H */
