#ifndef LIBMCU_CLI_H
#define LIBMCU_CLI_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

typedef struct {
	size_t (*read)(void *buf, size_t bufsize);
	size_t (*write)(const void *data, size_t data_size);
} cli_io_t;

void cli_run(const cli_io_t *io);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_CLI_H */
