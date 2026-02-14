
#include <pthread.h>
#include <mach/mach.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread_spis.h>

#include "darwintest_defaults.h"

#define CHILD_STACK_COUNT 1024
static uint64_t child_stack[CHILD_STACK_COUNT];

static void*
pthread_runner(void* __unused arg)
{
    T_PASS("mach -> pthread conversion successful");
    T_END;
}

static void *
mach_bootstrap(void * __unused arg)
{
    pthread_t thread;
    pthread_create_from_mach_thread(&thread, NULL, pthread_runner, NULL);
    while (1) {
        swtch_pri(0); // mach_yield
    }
}

T_DECL(pthread_create_from_mach_thread, "pthread_create_from_mach_thread",
       T_META_ALL_VALID_ARCHS(YES),
       // Having leaks running will suppress the behavior we are testing
       T_META_CHECK_LEAKS(false),
       T_META_ENVVAR("MallocStackLogging=1")
    )
{
    T_PASS("MallocStackLogging: %s", getenv("MallocStackLogging"));

    // Create a mach_thread to start with
    mach_port_t task = mach_task_self();

    thread_state_flavor_t flavor;
    mach_msg_type_number_t count;

    uintptr_t start_addr = (uintptr_t)&mach_bootstrap;
    // Force alignment to 16-bytes
    uintptr_t stack_top = ((uintptr_t)&child_stack[CHILD_STACK_COUNT]) & ~0xf;

#if defined(__x86_64__)
    T_PASS("x86_64");
    flavor = x86_THREAD_STATE64;
    count = x86_THREAD_STATE64_COUNT;
    x86_thread_state64_t state = {
        .__rip = start_addr,
        // Must be 16-byte-off-by-8 aligned <rdar://problem/15886599>
        .__rsp = stack_top - 8,
    };
#elif defined(__arm64__)
    T_PASS("arm64");
    flavor = ARM_THREAD_STATE64;
    count = ARM_THREAD_STATE64_COUNT;
    arm_thread_state64_t state = { };
    arm_thread_state64_set_pc_fptr(state, &mach_bootstrap);
    arm_thread_state64_set_sp(state, stack_top);
    (void)start_addr;
#elif defined(__arm__)
    T_PASS("arm (32)");
    flavor = ARM_THREAD_STATE;
    count = ARM_THREAD_STATE_COUNT;
    arm_thread_state_t state = {
        .__pc = start_addr,
        .__sp = stack_top,
        .__cpsr = 0x20,
    };
#else
#error Unknown architecture
#endif

    thread_state_t state_ptr = (thread_state_t)&state;
    thread_t task_thread;
    T_PASS("Launching Thread");

    kern_return_t ret = thread_create_running(task, flavor, state_ptr, count, &task_thread);
    T_ASSERT_MACH_SUCCESS(ret, "mach thread created");
    // Wait forever
    sigset_t empty;
    T_QUIET; T_ASSERT_POSIX_ZERO(sigemptyset(&empty), NULL);
    while (sigsuspend(&empty)) {
        continue;
    }
    T_FAIL("Didn't wait forever?");
}
