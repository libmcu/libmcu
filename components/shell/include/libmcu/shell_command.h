#ifndef LIBMCU_SHELL_CMD_H
#define LIBMCU_SHELL_CMD_H 202012L

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum {
	SHELL_CMD_SUCCESS			= 0,
	SHELL_CMD_EXIT,
	SHELL_CMD_NOT_FOUND,
	SHELL_CMD_INVALID_PARAM,
	SHELL_CMD_BLANK,
	SHELL_CMD_ERROR,
} shell_cmd_error_t;

typedef shell_cmd_error_t (*shell_cmd_func_t)(int argc, const char *argv[],
		const void *env);

typedef struct {
	const char *name;
	shell_cmd_func_t run;
	const char *desc;
} shell_cmd_t;

const shell_cmd_t *shell_get_command_list(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_CLI_CMD_H */
