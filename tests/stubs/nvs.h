#ifndef NVS_H
#define NVS_H 1

#include <stdint.h>
#include <stddef.h>

typedef uint32_t nvs_handle_t;

typedef enum {
	NVS_READONLY,  /*!< Read only */
	NVS_READWRITE  /*!< Read and write */
} nvs_open_mode_t;

int nvs_flash_init(void);
int nvs_open(const char *name, nvs_open_mode_t open_mode, nvs_handle_t *out_handle);
void nvs_close(nvs_handle_t handle);
int nvs_set_blob(nvs_handle_t c_handle, const char *key, const void *value, size_t length);
int nvs_commit(nvs_handle_t c_handle);
int nvs_get_blob(nvs_handle_t c_handle, const char* key, void* out_value, size_t* length);

#endif /* NVS_H */
