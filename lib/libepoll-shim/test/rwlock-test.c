#include <atf-c.h>

#include <stdlib.h>

#include <rwlock.h>

// Test based on
// <https://github.com/eliben/code-for-blog/blob/master/2019/rwlocks/rwlock_test.go>

struct stress_data {
	int iterations;
	RWLock *lock;
	int *data;
	int inc;
};

static void *
stress_reader(void *arg)
{
	struct stress_data *stress_data = arg;
	struct timespec now;
	ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &now) == 0);
	unsigned int seed = (unsigned int)now.tv_nsec;

	for (int i = 0; i < stress_data->iterations; ++i) {
		usleep((useconds_t)(((rand_r(&seed) % 8) + 1) * 10));

		rwlock_lock_read(stress_data->lock);
		for (int i = 1; i < 1000; ++i) {
			ATF_REQUIRE(stress_data->data[i] ==
			    stress_data->data[i - 1] + 1);
		}
		rwlock_unlock_read(stress_data->lock);
	}

	return NULL;
}

static void *
stress_writer(void *arg)
{
	struct stress_data *stress_data = arg;
	struct timespec now;
	ATF_REQUIRE(clock_gettime(CLOCK_MONOTONIC, &now) == 0);
	unsigned int seed = (unsigned int)now.tv_nsec;

	for (int i = 0; i < stress_data->iterations; ++i) {
		usleep((useconds_t)(((rand_r(&seed) % 8) + 1) * 10));

		rwlock_lock_write(stress_data->lock);
		for (int i = 0; i < 1000; ++i) {
			stress_data->data[i] += stress_data->inc;
		}
		rwlock_downgrade(stress_data->lock);
		for (int i = 1; i < 1000; ++i) {
			ATF_REQUIRE(stress_data->data[i] ==
			    stress_data->data[i - 1] + 1);
		}
		rwlock_unlock_read(stress_data->lock);
	}

	return NULL;
}

ATF_TC_WITHOUT_HEAD(stress);
ATF_TC_BODY(stress, tc)
{
	RWLock rwlock;
	ATF_REQUIRE(rwlock_init(&rwlock) == 0);

	int data[1000];
	for (int i = 0; i < 1000; ++i) {
		data[i] = i;
	}

	pthread_t threads[1010];
	struct stress_data stress_data[1010];

	for (int i = 0; i < 1000; ++i) {
		stress_data[i] = (struct stress_data) {
			.iterations = 500,
			.lock = &rwlock,
			.data = data,
		};
		ATF_REQUIRE(pthread_create(&threads[i], NULL, /**/
				stress_reader, &stress_data[i]) == 0);
	}

	for (int i = 0; i < 10; ++i) {
		stress_data[1000 + i] = (struct stress_data) {
			.iterations = 500,
			.lock = &rwlock,
			.data = data,
			.inc = i + 1,
		};
		ATF_REQUIRE(pthread_create(&threads[1000 + i], NULL,
				stress_writer, &stress_data[1000 + i]) == 0);
	}

	for (int i = 0; i < 1010; ++i) {
		ATF_REQUIRE(pthread_join(threads[i], NULL) == 0);
	}
}

ATF_TP_ADD_TCS(tp)
{
	ATF_TP_ADD_TC(tp, stress);

	return atf_no_error();
}
