#ifndef LIBMCU_CLI_H
#define LIBMCU_CLI_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>
#include "cli_command.h"

typedef struct {
	size_t (*read)(void *buf, size_t bufsize);
	size_t (*write)(const void *data, size_t data_size);
} cli_io_t;

typedef struct {
	const cli_io_t *io;
	const cli_cmd_t *cmds;
	size_t cmds_count;
	char cmdbuf[CLI_COMMAND_MAXLEN + 1/*linefeed*/ + 1/*null*/];
	size_t cmdbuf_index;
} cli_t;

void cli_init(cli_t *cli,
		const cli_io_t *io, const cli_cmd_t *cmds, size_t cmdcnt);
void cli_run(cli_t *cli);
void cli_step(cli_t *cli);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_CLI_H */
