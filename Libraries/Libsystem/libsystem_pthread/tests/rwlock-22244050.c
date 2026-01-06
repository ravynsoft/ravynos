#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <libkern/OSAtomic.h>

pthread_t thr;
pthread_rwlock_t lock;
OSSpinLock slock = 0;
int i = 0;

void sighandler(int sig)
{
	if (sig == SIGUSR1) {
		OSSpinLockLock(&slock);
		OSSpinLockUnlock(&slock);		
	} else {
		// ALARM
		fprintf(stderr, "FAIL (%d)\n", i);
		exit(1);
	}
}

void* thread(void *arg)
{
	pthread_rwlock_rdlock(&lock);
	pthread_rwlock_unlock(&lock);
	return NULL;
}

int main(int argc, const char *argv[])
{
	pthread_rwlock_init(&lock, NULL);
	signal(SIGUSR1, sighandler);
	signal(SIGALRM, sighandler);

	alarm(30);

	while (i++ < 10000) {
		pthread_rwlock_wrlock(&lock);
		pthread_create(&thr, NULL, thread, NULL);
		OSSpinLockLock(&slock);
		pthread_kill(thr, SIGUSR1);
		pthread_rwlock_unlock(&lock);
		OSSpinLockUnlock(&slock);
	}
	
	fprintf(stderr, "PASS\n");
	return 0;
}
