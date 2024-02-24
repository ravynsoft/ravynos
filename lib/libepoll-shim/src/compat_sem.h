#ifndef COMPAT_SEM_H
#define COMPAT_SEM_H

#ifdef COMPAT_ENABLE_SEM
#include <semaphore.h>

#include <dispatch/dispatch.h>

#define sem_t dispatch_semaphore_t

#define sem_init(sem, pshared, value) \
	(*(sem) = dispatch_semaphore_create(value))
#define sem_post(sem) dispatch_semaphore_signal(*(sem))
#define sem_wait(sem) dispatch_semaphore_wait(*(sem), DISPATCH_TIME_FOREVER)
#define sem_destroy(sem) dispatch_release(*(sem))
#endif

#endif
