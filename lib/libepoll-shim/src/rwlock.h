#ifndef RWLOCK_H_
#define RWLOCK_H_

#include <errno.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <pthread.h>
#include <semaphore.h>

typedef struct {
	pthread_mutex_t mutex;
	sem_t writer_wait;
	sem_t reader_wait;
	atomic_int_fast32_t num_pending;
	atomic_int_fast32_t readers_departing;
} RWLock;

errno_t rwlock_init(RWLock *rwlock);
void rwlock_terminate(RWLock *rwlock);

void rwlock_lock_read(RWLock *rwlock);
void rwlock_unlock_read(RWLock *rwlock);
void rwlock_lock_write(RWLock *rwlock);
void rwlock_unlock_write(RWLock *rwlock);
void rwlock_downgrade(RWLock *rwlock);

#endif
