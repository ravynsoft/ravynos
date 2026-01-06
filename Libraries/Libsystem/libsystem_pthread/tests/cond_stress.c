#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef struct _ConditionLock {
	pthread_mutex_t _mutex;
	pthread_cond_t _condition;
	int _owner;
	int _last_owner;
	volatile int _state;
} ConditionLock;

typedef struct _log {
	int thread;
	const char * op;
} log_t;

static int initConditionLockWithCondition(ConditionLock *, int);
static int lockConditionLock(ConditionLock *, int);
static int lockConditionLockWhenCondition(ConditionLock *, int, int);
static int unlockConditionLockWithCondition(ConditionLock *, int, int);
static int destroyConditionLock(ConditionLock *);
static void * testThread(void *);
static void log(int, const char *, ConditionLock *);

static ConditionLock * lock;
static volatile int count    = 0;
static volatile int logcount = 0;
static log_t * tracelog;
#define TRACE_MAX_COUNT (4 * 1024 * 1024)
long iterations = 999000L;

static void
log(int self, const char * op, ConditionLock * cl)
{
	tracelog[logcount].thread = self;
	tracelog[logcount++].op = op;
	if (logcount == TRACE_MAX_COUNT)
		logcount = 0;
}

int
main(int argc, char * argv[])
{
	pthread_t thread[4];

	lock = (ConditionLock *)calloc(sizeof(ConditionLock), 1);
	if (initConditionLockWithCondition(lock, 0))
		abort();
	tracelog = (log_t *)calloc(sizeof(log_t), TRACE_MAX_COUNT);

	pthread_create(&thread[0], NULL, testThread, (void *)1);
	pthread_create(&thread[1], NULL, testThread, (void *)2);
	pthread_create(&thread[2], NULL, testThread, (void *)3);
	pthread_create(&thread[3], NULL, testThread, (void *)4);

	while (iterations > -100) {
		if (lockConditionLock(lock, 0))
			abort();
		count = 1;
		iterations--;
		if (unlockConditionLockWithCondition(lock, 1, 0))
			abort();
	}

	destroyConditionLock(lock);
	free(lock);
	free(tracelog);

	return 0;
}

static void *
testThread(void * arg)
{
	int self = (int)arg;
	while (iterations > 0) {
		if (lockConditionLockWhenCondition(lock, 1, self))
			abort();
		count = 0;
		if (unlockConditionLockWithCondition(lock, 0, self))
			abort();
	}
	return arg;
}

static int
initConditionLockWithCondition(ConditionLock * cl, int condition)
{
	int rc;

	if ((rc = pthread_mutex_init(&cl->_mutex, NULL))) {
		fprintf(stderr, "pthread_mutex_init returned %d, %s\n", rc, strerror(rc));
		return 1;
	}

	if ((rc = pthread_cond_init(&cl->_condition, NULL))) {
		fprintf(stderr, "pthread_cond_init returned %d, %s\n", rc, strerror(rc));
		return 1;
	}

	cl->_state = condition;

	return 0;
}

static int
destroyConditionLock(ConditionLock * cl)
{
	int rc;

	if ((rc = pthread_mutex_destroy(&cl->_mutex))) {
		fprintf(stderr, "pthread_mutex_destroy returned %d, %s\n", rc, strerror(rc));
		return 1;
	}
	if ((rc = pthread_cond_destroy(&cl->_condition))) {
		fprintf(stderr, "pthread_cond_destroy returned %d, %s\n", rc, strerror(rc));
		return 1;
	}
	return 0;
}

static int
lockConditionLock(ConditionLock * cl, int self)
{
	int rc;

	if ((rc = pthread_mutex_lock(&cl->_mutex))) {
		fprintf(stderr, "pthread_mutex_lock() returned %d, %s\n", rc, strerror(rc));
		return 1;
	}
	cl->_owner = self;
	log(self, "Got lock", cl);
	return 0;
}

static int
lockConditionLockWhenCondition(ConditionLock * cl, int condition, int self)
{
	int rc;

	if ((rc = pthread_mutex_lock(&cl->_mutex))) {
		fprintf(stderr, "pthread_mutex_lock() returned %d, %s\n", rc, strerror(rc));
		return 1;
	}
	log(self, "Waiting for condition", cl);
	while (cl->_state != condition) {
		if ((rc = pthread_cond_wait(&cl->_condition, &cl->_mutex))) {
			fprintf(stderr, "pthread_cond_wait() returned %d, %s\n", rc, strerror(rc));
			return 1;
		}
		if (cl->_state != condition) {
			log(self, "condition lock wakeup with wrong condition", cl);
		}
	}
	cl->_owner = self;
	log(self, "Got condition", cl);
	return 0;
}

static int
unlockConditionLockWithCondition(ConditionLock * cl, int condition, int self)
{
	int rc;

	if (cl->_owner != self) {
		fprintf(stderr, "%d: trying to unlock a lock owned by %d\n", self, cl->_owner);
		abort();
	}
	log(self, condition ? "Unlocking with condition set" : "Unlocking with condition cleared", cl);
	cl->_last_owner = cl->_owner;
	cl->_owner      = 0;
	cl->_state = condition;
	if ((rc = pthread_cond_signal(&cl->_condition))) {
		fprintf(stderr, "pthread_cond_broadcast() returned %d, %s\n", rc, strerror(rc));
		return 1;
	}
	log(self, "Sent broadcast", cl);
	if ((rc = pthread_mutex_unlock(&cl->_mutex))) {
		fprintf(stderr, "pthread_mutex_unlock() returned %d, %s\n", rc, strerror(rc));
		return 1;
	}
	return 0;
}

