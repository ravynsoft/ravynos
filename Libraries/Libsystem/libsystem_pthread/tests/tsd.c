#include <pthread.h>
#include <stdio.h>

#include "darwintest_defaults.h"

static void *ptr = NULL;

static void destructor(void *value)
{
	ptr = value;
}

static void *thread(void *param)
{
	pthread_key_t key = *(pthread_key_t *)param;
	T_ASSERT_POSIX_ZERO(pthread_setspecific(key, (void *)0x12345678), NULL);
	void *value = pthread_getspecific(key);

	T_ASSERT_POSIX_ZERO(pthread_key_create(&key, NULL), NULL);
	T_ASSERT_POSIX_ZERO(pthread_setspecific(key, (void *)0x55555555), NULL);

	return value;
}

T_DECL(tsd, "tsd",
       T_META_ALL_VALID_ARCHS(YES))
{
	pthread_key_t key;

	T_ASSERT_POSIX_ZERO(pthread_key_create(&key, destructor), NULL);
	T_LOG("key = %ld", key);

	pthread_t p = NULL;
	T_ASSERT_POSIX_ZERO(pthread_create(&p, NULL, thread, &key), NULL);

	void *value = NULL;
	T_ASSERT_POSIX_ZERO(pthread_join(p, &value), NULL);
	T_LOG("value = %p; ptr = %p\n", value, ptr);

	T_EXPECT_EQ(ptr, value, NULL);

	T_ASSERT_POSIX_ZERO(pthread_key_delete(key), NULL);
}
