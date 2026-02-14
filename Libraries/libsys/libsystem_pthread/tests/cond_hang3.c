#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <mach/thread_switch.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <mach/sync.h>
#include <mach/sync_policy.h>

#define LOG

typedef long pthread_lock_t;

typedef struct _ConditionLock {
	pthread_mutex_t _mutex;
	pthread_cond_t _condition;
	int _owner;
	int _last_owner;
	volatile int _isLocked;
	volatile int _state;
} ConditionLock;

typedef struct _log {
	int thread;
	const char * op;
	int tbr;
} log_t;

#define LOCK_INIT(l) ((l) = 0)
#define LOCK(v)                                                        \
	if (__is_threaded) {                                               \
		while (!_spin_lock_try((pthread_lock_t *)&v)) {                \
			syscall_thread_switch(THREAD_NULL, SWITCH_OPTION_WAIT, 1); \
		}                                                              \
	}
#define UNLOCK(v)      \
	if (__is_threaded) \
	_spin_unlock((pthread_lock_t *)&v)
#ifndef ESUCCESS
#define ESUCCESS 0
#endif

#define my_pthread_mutex_init(m) pthread_mutex_init((m), NULL)
#define my_pthread_mutex_lock(m, ptself) pthread_mutex_lock(m)
#define my_pthread_mutex_unlock(m) pthread_mutex_unlock(m)
#define my_pthread_cond_init(c) pthread_cond_init((c), NULL)
#define my_pthread_cond_wait(c, m, ptself) pthread_cond_wait((c), (m))
#define my_pthread_cond_broadcast(c) pthread_cond_broadcast(c)

static int initConditionLockWithCondition(ConditionLock *, int);
static int lockConditionLock(ConditionLock *, int);
static int lockConditionLockWhenCondition(ConditionLock *, int, int);
static int unlockConditionLockWithCondition(ConditionLock *, int, int);
static void * testThread(void *);

extern int __is_threaded;
extern int _spin_lock_try(pthread_lock_t * lockp);
extern void _spin_unlock(pthread_lock_t * lockp);
extern kern_return_t syscall_thread_switch(thread_t, int, int);

static ConditionLock * lock;
static volatile int count = 0;
#if defined(LOG)
static volatile int logcount = 0;
static log_t * tracelog;
static const size_t logsize = 4 * 1024 * 1024;
pthread_lock_t loglock;
extern int getTBR(void);

static __inline__ unsigned long long
ReadTBR()
{
	union {
		unsigned long long time64;
		unsigned long word[2];
	} now;
#if defined(__i386__)
	/* Read from Pentium and Pentium Pro 64-bit timestamp counter.
	 * The counter is set to 0 at processor reset and increments on
	 * every clock cycle. */
	__asm__ volatile("rdtsc" : : : "eax", "edx");
	__asm__ volatile("movl %%eax,%0" : "=m"(now.word[0]) : : "eax");
	__asm__ volatile("movl %%edx,%0" : "=m"(now.word[1]) : : "edx");
#elif defined(__ppc__)
	/* Read from PowerPC 64-bit time base register. The increment
	 * rate of the time base is implementation-dependent, but is
	 * 1/4th the bus clock cycle on 603/604 processors. */
	unsigned long t3;
	do {
		__asm__ volatile("mftbu %0" : "=r"(now.word[0]));
		__asm__ volatile("mftb %0" : "=r"(now.word[1]));
		__asm__ volatile("mftbu %0" : "=r"(t3));
	} while (now.word[0] != t3);
#else
	now.time64 = mach_absolute_time();
#endif
	return now.time64;
}

static void
log(int self, const char * op, ConditionLock * cl)
{
	LOCK(loglock);
	if (logcount >= logsize)
		logcount              = 0;
	tracelog[logcount].thread = self;
	tracelog[logcount].op     = op;
	tracelog[logcount++].tbr = ReadTBR();
	UNLOCK(loglock);
}
#else
#define log(a, b, c)
#endif

int
main(int argc, char * argv[])
{
	pthread_t thread[4];
	long iterations = 100000L;

	lock = (ConditionLock *)calloc(sizeof(ConditionLock), 1);
	if (initConditionLockWithCondition(lock, 0))
		abort();
#if defined(LOG)
	tracelog = (log_t *)calloc(logsize, sizeof(log_t));
	LOCK_INIT(loglock);
#endif

	pthread_create(&thread[0], NULL, testThread, (void *)1);
	pthread_create(&thread[1], NULL, testThread, (void *)2);
	pthread_create(&thread[2], NULL, testThread, (void *)3);
	pthread_create(&thread[3], NULL, testThread, (void *)4);

	while (iterations-- > 0) {
		if (lockConditionLock(lock, 0))
			abort();
		count++;
		if (unlockConditionLockWithCondition(lock, 1, 0))
			abort();
	}
	printf("completed numerous iterations without hanging. Exiting with return 0\n");
	return 0;
}

static void *
testThread(void * arg)
{
	int self = (int)arg;
	while (1) {
		if (lockConditionLockWhenCondition(lock, 1, self))
			abort();
		count--;
		if (!count) {
			if (unlockConditionLockWithCondition(lock, 0, self))
				abort();
		} else {
			if (unlockConditionLockWithCondition(lock, 1, self))
				abort();
		}
	}
	return arg;
}

static int
initConditionLockWithCondition(ConditionLock * cl, int condition)
{
	int rc;

	if ((rc = my_pthread_mutex_init(&cl->_mutex))) {
		fprintf(stderr, "pthread_mutex_init returned %d, %s\n", rc, strerror(rc));
		return 1;
	}

	if ((rc = my_pthread_cond_init(&cl->_condition))) {
		fprintf(stderr, "pthread_cond_init returned %d, %s\n", rc, strerror(rc));
		return 1;
	}

	cl->_isLocked = 0;
	cl->_state    = condition;

	return 0;
}

static int
lockConditionLock(ConditionLock * cl, int self)
{
	int rc;

	if ((rc = my_pthread_mutex_lock(&cl->_mutex, self))) {
		fprintf(stderr, "pthread_mutex_lock() returned %d, %s\n", rc, strerror(rc));
		return 1;
	}
	log(self, "Waiting for lock", cl);
	while (cl->_isLocked) {
		if ((rc = my_pthread_cond_wait(&cl->_condition, &cl->_mutex, self))) {
			fprintf(stderr, "pthread_cond_wait() returned %d, %s\n", rc, strerror(rc));
			if (rc != EINVAL) {
				return 1;
			}
		}
		if (cl->_isLocked) {
			log(self, "lock wakeup with lock held", cl);
		}
	}
	cl->_isLocked = 1;
	cl->_owner = self;
	log(self, "Got lock", cl);
	if ((rc = my_pthread_mutex_unlock(&cl->_mutex))) {
		fprintf(stderr, "pthread_mutex_unlock() %d, %s\n", rc, strerror(rc));
		return 1;
	}
	return 0;
}

static int
lockConditionLockWhenCondition(ConditionLock * cl, int condition, int self)
{
	int rc;

	if ((rc = my_pthread_mutex_lock(&cl->_mutex, self))) {
		fprintf(stderr, "pthread_mutex_lock() returned %d, %s\n", rc, strerror(rc));
		return 1;
	}
	log(self, "Waiting for condition", cl);
	while (cl->_isLocked || cl->_state != condition) {
		if ((rc = my_pthread_cond_wait(&cl->_condition, &cl->_mutex, self))) {
			fprintf(stderr, "pthread_cond_wait() returned %d, %s\n", rc, strerror(rc));
			if (rc != EINVAL) {
				return 1;
			}
		}
		if (cl->_isLocked) {
			log(self, "condition lock wakeup with lock held", cl);
		}
		if (cl->_state != condition) {
			log(self, "condition lock wakeup with wrong condition", cl);
		}
	}
	cl->_isLocked = 1;
	cl->_owner = self;
	log(self, "Got condition", cl);
	if ((rc = my_pthread_mutex_unlock(&cl->_mutex))) {
		fprintf(stderr, "pthread_mutex_unlock() returned %d, %s\n", rc, strerror(rc));
		return 1;
	}
	return 0;
}

static int
unlockConditionLockWithCondition(ConditionLock * cl, int condition, int self)
{
	int rc;

	if ((rc = my_pthread_mutex_lock(&cl->_mutex, self))) {
		fprintf(stderr, "pthread_mutex_lock() returned %d, %s\n", rc, strerror(rc));
		return 1;
	}
	if (cl->_owner != self) {
		fprintf(stderr, "%d: trying to unlock a lock owned by %d\n", self, cl->_owner);
		abort();
	}
	log(self, condition ? "Unlocking with condition set" : "Unlocking with condition cleared", cl);
	cl->_isLocked   = 0;
	cl->_last_owner = cl->_owner;
	cl->_owner      = 0;
	cl->_state = condition;
	if ((rc = my_pthread_cond_broadcast(&cl->_condition))) {
		fprintf(stderr, "pthread_cond_broadcast() returned %d, %s\n", rc, strerror(rc));
		return 1;
	}
	log(self, "Sent broadcast", cl);
	if ((rc = my_pthread_mutex_unlock(&cl->_mutex))) {
		fprintf(stderr, "pthread_mutex_unlock() returned %d, %s\n", rc, strerror(rc));
		return 1;
	}
	return 0;
}

#if 0
static int
my_pthread_mutex_init(my_pthread_mutex_t *mutex)
{
        kern_return_t kern_res;
        LOCK_INIT(mutex->lock);
        mutex->owner = (pthread_t)NULL;
        mutex->waiters = 0;
        mutex->cond_lock = 0;
        kern_res = semaphore_create(mach_task_self(),
                                &mutex->sem,
                                SYNC_POLICY_FIFO,
                                0);
        if (kern_res != KERN_SUCCESS)
        {
                return (ENOMEM);
        } else
        {
                return (ESUCCESS);
        }
}

static int
my_pthread_mutex_lock(my_pthread_mutex_t *mutex, int self)
{
        kern_return_t kern_res;

        LOCK(mutex->lock);
#if 0
        if (mutex->waiters || mutex->owner != (pthread_t)NULL)
#else
        while (mutex->owner != (pthread_t)NULL)
#endif
        {
                mutex->waiters++;
                log(self, "going in to sem_wait", 0);
                UNLOCK(mutex->lock);
                kern_res = semaphore_wait(mutex->sem);
                LOCK(mutex->lock);
                mutex->waiters--;
                log(self, "woke up from sem_wait", 0);
                if (mutex->cond_lock) {
                        log(self, "clearing cond_lock", 0);
                        mutex->cond_lock = 0;
#if 0
#else
                        break;
#endif
                }
        }
        mutex->owner = (pthread_t)0x12141968;
        UNLOCK(mutex->lock);
        return (ESUCCESS);
}

static int
my_pthread_mutex_unlock(my_pthread_mutex_t *mutex)
{
        kern_return_t kern_res;
        int waiters;
        
        LOCK(mutex->lock);
        mutex->owner = (pthread_t)NULL;
        waiters = mutex->waiters;
        UNLOCK(mutex->lock);
        if (waiters)
        {
            kern_res = semaphore_signal(mutex->sem);
        }
        return (ESUCCESS);
}

/*
 * Initialize a condition variable.  Note: 'attr' is ignored.
 */
static int       
my_pthread_cond_init(my_pthread_cond_t *cond)
{
        kern_return_t kern_res;
        LOCK_INIT(cond->lock);
        cond->waiters = 0;
        kern_res = semaphore_create(mach_task_self(),
                                   &cond->sem,
                                   SYNC_POLICY_FIFO,
                                   0);
        if (kern_res != KERN_SUCCESS)
        {
                return (ENOMEM);
        }
        return (ESUCCESS);
}

/*
 * Signal a condition variable, waking up all threads waiting for it.
 */
static int       
my_pthread_cond_broadcast(my_pthread_cond_t *cond)
{
        kern_return_t kern_res;
        int waiters;
        
        LOCK(cond->lock);
        waiters = cond->waiters;
        if (cond->waiters == 0)
        { /* Avoid kernel call since there are no waiters... */
                UNLOCK(cond->lock);
                return (ESUCCESS);
        }
        UNLOCK(cond->lock);
#if 0
        kern_res = semaphore_signal(cond->sem);
#endif
        kern_res = semaphore_signal_all(cond->sem);
        if (kern_res == KERN_SUCCESS)
        {
                return (ESUCCESS);
        } else
        {
                return (EINVAL);
        }
}

static int       
my_pthread_cond_wait(my_pthread_cond_t *cond, my_pthread_mutex_t *mutex, int self)
{
        int res;
        kern_return_t kern_res;

        LOCK(cond->lock);
        cond->waiters++;
        UNLOCK(cond->lock);
        LOCK(mutex->lock);
        mutex->cond_lock = 1;
        log(self, "going in to sem_wait_signal", 0);
        UNLOCK(mutex->lock);
        kern_res = semaphore_wait_signal(cond->sem, mutex->sem);
        LOCK(cond->lock);
        cond->waiters--;
        log(self, "woke up from sem_wait_signal", 0);
        UNLOCK(cond->lock);
        if ((res = my_pthread_mutex_lock(mutex, self)) != ESUCCESS)
        {
                return (res);
        }
        if (kern_res == KERN_SUCCESS)
        {
                return (ESUCCESS);
        } else
        {
                return (EINVAL);
        }
}

#endif
