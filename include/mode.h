#ifndef RUNMODE_H
#define RUNMODE_H 1

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum {
	PROVISIONING_MODE	= 0,
	REPORT_MODE,
	UNKNOWN_MODE,
} runmode_t;

typedef void (*mode_transition_callback_t)(void *param);

void mode_set(runmode_t mode);
runmode_t mode_get(void);
int mode_register_transition_callback(const mode_transition_callback_t callback);
int mode_unregister_transition_callback(const mode_transition_callback_t callback);
void mode_init(void);

#if defined(__cplusplus)
}
#endif

#endif /* RUNMODE_H */
