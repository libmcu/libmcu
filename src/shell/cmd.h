#ifndef LIBMCU_SHELL_CMD_H
#define LIBMCU_SHELL_CMD_H 1

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum {
	SHELL_CMD_SUCCESS			= 0,
	SHELL_CMD_EXIT,
	SHELL_CMD_ERROR,
} shell_cmd_error_t;

typedef shell_cmd_error_t (*shell_cmd_func_t)(int argc, void *argv[],
		const void *env);

typedef struct {
	const char *name;
	shell_cmd_func_t func;
	const char *desc;
} shell_cmd_t;

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_CLI_CMD_H */
