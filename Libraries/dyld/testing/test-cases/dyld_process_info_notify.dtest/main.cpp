
// BUILD:  $CC target.c      -o $BUILD_DIR/target.exe -DRUN_DIR="$RUN_DIR"
// BUILD:  $CC foo.c         -o $BUILD_DIR/libfoo.dylib -dynamiclib
// BUILD:  $CXX main.cpp        -o $BUILD_DIR/dyld_process_info_notify.exe -DRUN_DIR="$RUN_DIR"
// BUILD:  $TASK_FOR_PID_ENABLE $BUILD_DIR/dyld_process_info_notify.exe

// RUN_TIMEOUT: 2400
// XFAIL:  $SUDO ./dyld_process_info_notify.exe  $RUN_DIR/target.exe

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <signal.h>
#include <spawn.h>
#include <errno.h>
#include <mach/mach.h>
#include <mach/machine.h>
#include <mach-o/dyld_process_info.h>
#include <dispatch/dispatch.h>
#include <Availability.h>

#include "test_support.h"

extern char** environ;

void launchTest(bool launchSuspended, bool disconnectEarly)
{
    LOG("launchTest (%s)", launchSuspended ? "suspended" : "unsuspened");
    dispatch_queue_t queue = dispatch_queue_create("com.apple.dyld.test.dyld_process_info", NULL);
    // We do this instead of using a dispatch_semaphore to prevent priority inversions
    dispatch_block_t taskDone = dispatch_block_create(DISPATCH_BLOCK_INHERIT_QOS_CLASS, ^{});
    dispatch_block_t taskStarted = dispatch_block_create(DISPATCH_BLOCK_INHERIT_QOS_CLASS, ^{});
    pid_t pid;
    
    task_t task;
    char subTestNameBuffer[256];
    char *subTestName = &subTestNameBuffer[0];
    __block bool sawMainExecutable = false;
    __block bool sawlibSystem = false;
    __block bool gotTerminationNotice = false;
    __block bool gotEarlyNotice = false;
    __block bool gotMainNotice = false;
    __block bool gotMainNoticeBeforeAllInitialDylibs = false;
    __block bool gotFooNoticeBeforeMain = false;

    __block int libFooLoadCount = 0;
    __block int libFooUnloadCount = 0;
    __block dyld_process_info_notify handle;

    _process process;
    process.set_executable(RUN_DIR "/target.exe");
    const char* env[] = { "TEST_OUTPUT=None", NULL};
    process.set_env(env);
    process.set_launch_suspended(launchSuspended);
    if (!launchSuspended) {
        const char* args[] = {"suspend-in-main", NULL};
        _process_config_set_args(process, args);
        _process_config_set_stderr_handler(process, ^(int fd) {
            dispatch_semaphore_signal(taskStarted);
        });
        _process_config_set_exit_handler(process, ^(pid_t pid) {
            LOG("DIED (pid: %d)", pid);
        });
    }
    pid = process.launch(queue);

    if (!launchSuspended && dispatch_semaphore_wait(taskStarted, dispatch_time(DISPATCH_TIME_NOW, 5LL * NSEC_PER_SEC)) != 0) {
        FAIL("Child launch timeout");
    }
#if 1
    snprintf(&subTestNameBuffer[0], 256, "%s (arch: %d)", launchSuspended ? "launch suspended" : "launch suspend-in-main", currentArch);

    if ( task_for_pid(mach_task_self(), pid, &task) != KERN_SUCCESS ) {
        FAIL("task_for_pid()");
    }

    kern_return_t kr;
    unsigned count = 0;
    do {
        handle = _dyld_process_info_notify(task, queue,
                                          ^(bool unload, uint64_t timestamp, uint64_t machHeader, const uuid_t uuid, const char* path) {
                                            if ( strstr(path, "/target.exe") != NULL )
                                                sawMainExecutable = true;
                                            if ( strstr(path, "/libSystem") != NULL )
                                                sawlibSystem = true;
                                            if ( strstr(path, "/libfoo.dylib") != NULL ) {
                                                if ( !gotMainNotice ) {
                                                    gotFooNoticeBeforeMain = true;
                                                }
                                                if ( unload ) {
                                                    ++libFooUnloadCount;
                                                } else {
                                                    ++libFooLoadCount;
                                                }
                                                if ( disconnectEarly ) {
                                                    LOG("EARLY DISCONNECT");
                                                    gotEarlyNotice = true;
                                                    dispatch_semaphore_signal(taskDone);
                                                }
                                            }
                                          },
                                          ^{
                                            LOG("TERMINATED (pid: %d)", pid);
                                            gotTerminationNotice = true;
                                            dispatch_semaphore_signal(taskDone);
                                          },
                                          &kr);
        ++count;
        if ( handle == NULL )
            LOG("_dyld_process_info_notify() returned NULL, result=%d, count=%d", kr, count);
     } while ( (handle == NULL) && (count < 5) );

    if ( handle == NULL ) {
        FAIL("%s: did not not get handle", subTestName);
    }

    if (launchSuspended) {
        // If the process starts suspended register for main(),
        // otherwise skip since this test is a race between
        // process setup and notification registration
        _dyld_process_info_notify_main(handle, ^{
                                                LOG("target entering main()");
                                                gotMainNotice = true;
                                                if ( !sawMainExecutable || !sawlibSystem )
                                                    gotMainNoticeBeforeAllInitialDylibs = true;
                                                });
        kill(pid, SIGCONT);
        LOG("Sent SIGCONT");
    } else {
        kill(pid, SIGUSR1);
        LOG("Sent SIGUSR1");
    }

    // block waiting for notification that target has exited
    if (dispatch_semaphore_wait(taskDone, dispatch_time(DISPATCH_TIME_NOW, 10LL * NSEC_PER_SEC)) != 0) {
         FAIL("%s: did not get exit signal", subTestName);
    }

//    dispatch_release(taskDone);
//    dispatch_release(queue);
//    _dyld_process_info_notify_release(handle);

    // Do not run any tests associated with startup unless the kernel suspended us
    // before main()
    if (launchSuspended) {
        if ( !sawMainExecutable ) {
            FAIL("%s: did not get load notification of main executable", subTestName);
        }

        if ( !gotMainNotice ) {
            FAIL("%s: did not get notification of main()", subTestName);
        }

        if ( gotMainNoticeBeforeAllInitialDylibs ) {
            FAIL("%s: notification of main() arrived before all initial dylibs", subTestName);
        }

        if ( gotFooNoticeBeforeMain ) {
            FAIL("%s: notification of main() arrived after libfoo load notice", subTestName);
        }

        if ( !sawlibSystem ) {
            FAIL("%s: did not get load notification of libSystem", subTestName);
        }
    }

    if ( disconnectEarly ) {
        if ( libFooLoadCount != 1 ) {
            FAIL("%s: got %d load notifications about libFoo instead of 1", subTestName, libFooLoadCount);
        }
        if ( libFooUnloadCount != 0 ) {
            FAIL("%s: got %d unload notifications about libFoo instead of 1", subTestName, libFooUnloadCount);
        }
    }
    else {
        if ( libFooLoadCount != 3 ) {
            FAIL("%s: got %d load notifications about libFoo instead of 3", subTestName, libFooLoadCount);
        }
        if ( libFooUnloadCount != 3 ) {
            FAIL("%s: got %d unload notifications about libFoo instead of 3", subTestName, libFooUnloadCount);
        }
    }
#endif
}

#if 0
static void validateMaxNotifies(struct task_and_pid tp)
{
    dispatch_queue_t serviceQueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0);
    dyld_process_info_notify handles[10];
    // This loop goes through 10 iterations
    // i = 0..7 Should succeed
    // i = 8 Should fail,  but trigger a release that frees up a slot
    // i = 9 Should succeed
    for (int i=0; i < 10; ++i) {
        kern_return_t kr;
        handles[i] = _dyld_process_info_notify(tp.task, serviceQueue,
                                          ^(bool unload, uint64_t timestamp, uint64_t machHeader, const uuid_t uuid, const char* path) {
                                            LOG("unload=%d, 0x%012llX <%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X> %s",
                                                unload, machHeader, uuid[0],  uuid[1],  uuid[2],  uuid[3],  uuid[4],  uuid[5],  uuid[6],  uuid[7],
                                                uuid[8],  uuid[9],  uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15], path);
                                          },
                                          ^{
                                            LOG("target exited");
                                          },
                                          &kr);
        if ( handles[i] == NULL ) {
            if ( i == 8 ) {
                // expected failure, because only 8 simultaneous connections allowed
                // release one and try again
                _dyld_process_info_notify_release(handles[4]);
                handles[4] = NULL;
            }
            else {
                LOG("_dyld_process_info_notify() returned NULL and kern_result=%d, on count=%d", kr, i);
                killTest(tp);
                exit(0);
            }
        }
    }
    // release all
    for (int i=0; i < 10; ++i) {
        if ( handles[i] != NULL ) {
            _dyld_process_info_notify_release(handles[i]);
        }
    }
    dispatch_release(serviceQueue);
}
#endif

static void testSelfAttach(void) {
    LOG("7");
    __block bool dylibLoadNotified = false;
    kern_return_t kr = KERN_SUCCESS;
    dispatch_queue_t queue = dispatch_queue_create("com.apple.dyld.test.dyld_process_info.self-attach", NULL);
    LOG("7.5");
    dyld_process_info_notify handle = _dyld_process_info_notify(mach_task_self(), queue,
                                       ^(bool unload, uint64_t timestamp, uint64_t machHeader, const uuid_t uuid, const char* path) {
                                           if ( strstr(path, "/libfoo.dylib") != NULL ) {
                                               dylibLoadNotified = true;
                                           }
                                       },
                                       ^{},
                                       &kr);
    LOG("8");
    if ( handle == NULL ) {
        LOG("_dyld_process_info_notify() returned NULL, result=%d", kr);
    }
    LOG("8.5");
    void* h = dlopen(RUN_DIR "/libfoo.dylib", 0);
    LOG("8.75");
    dlclose(h);
    if (!dylibLoadNotified) {
        FAIL("testSelfAttach");
    }
    LOG("9");
}

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {

    // test 1) launch test program suspended in same arch as this program
    launchTest(true, false);

    // test 2) launch test program in same arch as this program where it sleeps itself
    launchTest(false, false);
//        validateMaxNotifies(child);

    // test 3) launch test program where we disconnect from it after first dlopen
    launchTest(true, true);
//        monitor("disconnect", child, true, false);

    // test 4) attempt to monitor the monitoring process
//    testSelfAttach();
    PASS("Success");

}
