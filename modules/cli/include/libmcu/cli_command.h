#ifndef LIBMCU_CLI_CMD_H
#define LIBMCU_CLI_CMD_H

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(CLI_COMMAND_MAXLEN)
#define CLI_COMMAND_MAXLEN			62
#endif
#if !defined(CLI_ARGS_MAXLEN)
#define CLI_ARGS_MAXLEN				4
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
	cli_cmd_func_t func;
	const char *desc;
} cli_cmd_t;

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_CLI_CMD_H */
