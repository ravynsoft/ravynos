#include "rwlock.h"

#include <errno.h>

/*
 * Inspired by:
 * <https://eli.thegreenplace.net/2019/implementing-reader-writer-locks/>
 */

#define MAX_READERS (1 << 30)

static int
sem_wait_nointr(sem_t *sem)
{
	int rc;
	int cs;
	(void)pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);

	do {
		rc = sem_wait(sem);
	} while (rc < 0 && errno == EINTR);

	(void)pthread_setcancelstate(cs, NULL);

	return rc;
}

errno_t
rwlock_init(RWLock *rwlock)
{
	errno_t ec;

	*rwlock = (RWLock) {};

	if ((ec = pthread_mutex_init(&rwlock->mutex, NULL)) != 0) {
		goto out_mutex;
	}

	if (sem_init(&rwlock->writer_wait, 0, 0) < 0) {
		ec = errno;
		goto out_writer_wait;
	}

	if (sem_init(&rwlock->reader_wait, 0, 0) < 0) {
		ec = errno;
		goto out_reader_wait;
	}

	return 0;

	(void)sem_destroy(&rwlock->reader_wait);
out_reader_wait:
	(void)sem_destroy(&rwlock->writer_wait);
out_writer_wait:
	(void)pthread_mutex_destroy(&rwlock->mutex);
out_mutex:
	return ec;
}

void
rwlock_terminate(RWLock *rwlock)
{
	(void)sem_destroy(&rwlock->reader_wait);
	(void)sem_destroy(&rwlock->writer_wait);
	(void)pthread_mutex_destroy(&rwlock->mutex);
}

void
rwlock_lock_read(RWLock *rwlock)
{
	if (atomic_fetch_add(&rwlock->num_pending, 1) + 1 < 0) {
		(void)sem_wait_nointr(&rwlock->reader_wait);
	}
}

void
rwlock_unlock_read(RWLock *rwlock)
{
	if (atomic_fetch_sub(&rwlock->num_pending, 1) - 1 < 0) {
		if (atomic_fetch_sub(&rwlock->readers_departing, 1) - 1 == 0) {
			(void)sem_post(&rwlock->writer_wait);
		}
	}
}

void
rwlock_lock_write(RWLock *rwlock)
{
	(void)pthread_mutex_lock(&rwlock->mutex);
	int_fast32_t r = atomic_fetch_sub(&rwlock->num_pending, MAX_READERS);
	if (r != 0 &&
	    atomic_fetch_add(&rwlock->readers_departing, r) + r != 0) {
		(void)sem_wait_nointr(&rwlock->writer_wait);
	}
}

static inline void
rwlock_unlock_common(RWLock *rwlock, int keep_reading)
{
	int_fast32_t r = atomic_fetch_add(&rwlock->num_pending,
			     MAX_READERS + keep_reading) +
	    MAX_READERS + keep_reading;
	for (int_fast32_t i = keep_reading; i < r; ++i) {
		(void)sem_post(&rwlock->reader_wait);
	}
	(void)pthread_mutex_unlock(&rwlock->mutex);
}

void
rwlock_unlock_write(RWLock *rwlock)
{
	rwlock_unlock_common(rwlock, 0);
}

void
rwlock_downgrade(RWLock *rwlock)
{
	rwlock_unlock_common(rwlock, 1);
}
