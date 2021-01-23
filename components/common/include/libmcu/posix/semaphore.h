#ifndef LIBMCU_SEMAPHORE_H
#define LIBMCU_SEMAPHORE_H 202012L

#if defined(__cplusplus)
extern "C" {
#endif

typedef union {
	char _size[1];
	long _align;
} sem_t;

int sem_init(sem_t *sem, int pshared, unsigned int value);
int sem_destroy(sem_t *sem);
int sem_wait(sem_t *sem);
int sem_timedwait(sem_t *sem, unsigned int timeout_ms);
int sem_trywait(sem_t *sem);
int sem_getvalue(sem_t *sem, int *sval);
int sem_post(sem_t *sem);
int sem_post_nointr(sem_t *sem);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_SEMAPHORE_H */
