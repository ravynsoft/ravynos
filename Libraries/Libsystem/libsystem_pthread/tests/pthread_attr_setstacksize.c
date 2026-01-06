#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <pthread.h>

#include "darwintest_defaults.h"

#define STACK_ALLOWANCE (1024ULL * 6)

static void *
pthread_attr_setstacksize_func(void *arg)
{
    if ((size_t)arg < 1024ULL * 32) {
        /*
         * We can't use darwintest because it requires a bigger stack than
         * this, so cheat and use the return value for the test.
         */
#ifndef __arm64__
        if ((size_t)arg != pthread_get_stacksize_np(pthread_self())) {
            return NULL;
        }
#endif
        return (void*)pthread_attr_setstacksize_func;
    }

#if defined(__arm64__)
    // Because of <rdar://problem/19941744>, the kext adds additional size to the stack on arm64.
    T_EXPECTFAIL;
#endif
    T_EXPECT_EQ((size_t)arg, pthread_get_stacksize_np(pthread_self()), "[stacksize=%zu] pthread_self stack size matches", (size_t)arg);

    size_t stacksize = (size_t)arg - STACK_ALLOWANCE;
    char *buf = alloca(stacksize);

    memset_s(buf, sizeof(buf), 0, sizeof(buf) - 1);

    return (void*)pthread_attr_setstacksize_func;
}

T_DECL(pthread_attr_setstacksize, "pthread_attr_setstacksize")
{
    size_t stacksizes[] = {PTHREAD_STACK_MIN, 1024ULL * 16, 1024ULL * 32, 1024ULL * 1024};
    for (int i = 0; (size_t)i < sizeof(stacksizes)/sizeof(stacksizes[0]); i++){
        pthread_t t = NULL;
        pthread_attr_t  attr;
        size_t stacksize = stacksizes[i];

        T_ASSERT_POSIX_ZERO(pthread_attr_init(&attr), "[stacksize=%zu] pthread_attr_init", stacksize);
        T_ASSERT_POSIX_ZERO(pthread_attr_setstacksize(&attr, stacksize), "[stacksize=%zu] pthread_attr_stacksize", stacksize);

        T_ASSERT_POSIX_ZERO(pthread_create(&t, &attr, pthread_attr_setstacksize_func, (void*)stacksize), "[stacksize=%zu] pthread_create", stacksize);
        T_ASSERT_NOTNULL(t, "[stacksize=%zu] pthread pointer not null", stacksize);

        T_EXPECT_POSIX_ZERO(pthread_attr_destroy(&attr), "[stacksize=%zu] pthread_attr_destroy", stacksize);

#if defined(__arm64__)
        // Because of <rdar://problem/19941744>, the kext adds additional size to the stack on arm64.
        T_EXPECTFAIL;
#endif
        T_EXPECT_EQ(stacksize, pthread_get_stacksize_np(t), "[stacksize=%zu] pthread stack size matches", stacksize);

        void *out = NULL;
        T_ASSERT_POSIX_ZERO(pthread_join(t, &out), "[stacksize=%zu] pthread_join", stacksize);
        T_EXPECT_EQ_PTR(out, (void*)pthread_attr_setstacksize_func, "[stacksize=%zu] pthread_join returns correct value", stacksize);
    }
}
