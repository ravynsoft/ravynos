#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>

struct context {
	pthread_rwlock_t rwlock;
	long value;
	long count;
};

void mask_signals(bool masked, bool perthread)
{
	sigset_t mask;

	if (masked) {
		sigfillset(&mask);
		sigdelset(&mask, SIGINT);
	} else {
		sigemptyset(&mask);
		sigaddset(&mask, SIGWINCH);
	}

	int action = (masked ? SIG_BLOCK : SIG_UNBLOCK);
	if (perthread) {
		pthread_sigmask(action, &mask, NULL);
	} else {
		sigprocmask(action, &mask, NULL);
	}
}

void test_signal(int signo)
{
	/* nothing */
}

void *test_signal_thread(void *ptr)
{
	sigset_t mask;
	sigfillset(&mask);
	sigdelset(&mask, SIGWINCH);
	pthread_sigmask(SIG_BLOCK, &mask, NULL);

	struct context *context = ptr;
	do {
		usleep(100);
		kill(getpid(), SIGWINCH);
	} while (context->count > 0);

	return NULL;
}

void *test_thread(void *ptr) {
	int res;
	long old;
	struct context *context = ptr;

	int i = 0;
	char *str;

	mask_signals(false, true);

	do {
		bool try = i & 1;
		bool exclusive = i & 2;
		switch (i++ & 3) {
			case 0:
				str = "pthread_rwlock_rdlock";
				res = pthread_rwlock_rdlock(&context->rwlock);
				break;
			case 1:
				str = "pthread_rwlock_tryrdlock";
				res = pthread_rwlock_tryrdlock(&context->rwlock);
				break;
			case 2:
				str = "pthread_rwlock_wrlock";
				res = pthread_rwlock_wrlock(&context->rwlock);
				break;
			case 3:
				str = "pthread_rwlock_trywrlock";
				res = pthread_rwlock_trywrlock(&context->rwlock);
				break;
		}
		if (res != 0) {
			if (try && res == EBUSY) {
				continue;
			}
			fprintf(stderr, "[%ld] %s: %s\n", context->count, str, strerror(res));
			abort();
		}

		if (exclusive) {
			old = __sync_fetch_and_or(&context->value, 1);
			if ((old & 1) != 0) {
				fprintf(stderr, "[%ld] OR %lx\n", context->count, old);
				abort();
			}
		}

		old = __sync_fetch_and_and(&context->value, 0);
		if ((old & 1) != (exclusive ? 1 : 0)) {
			fprintf(stderr, "[%ld] AND %lx\n", context->count, old);
			abort();
		}
	
		res = pthread_rwlock_unlock(&context->rwlock);
		if (res) {
			fprintf(stderr, "[%ld] pthread_rwlock_unlock: %s\n", context->count, strerror(res));
			abort();
		}
	} while (__sync_fetch_and_sub(&context->count, 1) > 0);

	return NULL;
}

int main(int argc, char *argv[])
{
	struct context context = {
		.rwlock = PTHREAD_RWLOCK_INITIALIZER,
		.value = 0,
		.count = 5000000,
	};
	int i;
	int res;
	int threads = 16;
	pthread_t p[threads+1];

	mask_signals(true, false);
	signal(SIGWINCH, test_signal);

	for (i = 0; i < threads; ++i) {
		res = pthread_create(&p[i], NULL, test_thread, &context);
		assert(res == 0);
	}

	pthread_create(&p[threads], NULL, test_signal_thread, &context);
	assert(res == 0);

	for (i = 0; i < threads; ++i) {
		res = pthread_join(p[i], NULL);
		assert(res == 0);
	}
	res = pthread_join(p[threads], NULL);
	assert(res == 0);

	return 0;
}
