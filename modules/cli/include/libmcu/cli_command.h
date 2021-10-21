#ifndef LIBMCU_CLI_CMD_H
#define LIBMCU_CLI_CMD_H

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum {
	CLI_CMD_SUCCESS				= 0,
	CLI_CMD_EXIT,
	CLI_CMD_NOT_FOUND,
	CLI_CMD_INVALID_PARAM,
	CLI_CMD_BLANK,
	CLI_CMD_ERROR,
} cli_cmd_error_t;

typedef cli_cmd_error_t (*cli_cmd_func_t)(int argc, const char *argv[],
		const void *env);

typedef struct {
	const char *name;
	cli_cmd_func_t run;
	const char *desc;
} cli_cmd_t;

const cli_cmd_t *cli_get_command_list(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_CLI_CMD_H */
