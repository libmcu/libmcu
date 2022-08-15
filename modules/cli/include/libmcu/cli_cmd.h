#ifndef LIBMCU_CLI_CMD_H
#define LIBMCU_CLI_CMD_H

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(CLI_CMD_MAXLEN)
#define CLI_CMD_MAXLEN				62
#endif
#if !defined(CLI_CMD_ARGS_MAXLEN)
#define CLI_CMD_ARGS_MAXLEN			4
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

struct cli_cmd {
	char const *name;
	cli_cmd_func_t func;
	char const *desc;
};

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_CLI_CMD_H */
