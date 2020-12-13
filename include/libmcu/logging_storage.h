#ifndef LOGGING_STORAGE_H
#define LOGGING_STORAGE_H 202012L

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

typedef struct logging_storage_ops {
	size_t (*write)(const void *data, size_t size);
	/** Read up to `bufsize` bytes from the storage
	 * @return The number of bytes read is returned on success,
	 *         0 is returned on error or when no log is there. */
	size_t (*read)(void *buf, size_t bufsize);
	/** Remove the oldest log in the storage
	 * @return The number of bytes removed. */
	size_t (*consume)(size_t size);
	size_t (*count)(void);
} logging_storage_t;

#if defined(__cplusplus)
}
#endif

#endif /* LOGGING_STORAGE_H */
