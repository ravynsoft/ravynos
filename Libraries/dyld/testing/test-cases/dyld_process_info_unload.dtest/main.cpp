
// BUILD:  $CC target.c       -o $BUILD_DIR/target.exe
// BUILD:  $CC foo.c          -o $BUILD_DIR/libfoo.dylib -dynamiclib -install_name $RUN_DIR/libfoo.dylib
// BUILD:  $CXX main.cpp         -o $BUILD_DIR/dyld_process_info_unload.exe -DRUN_DIR="$RUN_DIR"
// BUILD:  $TASK_FOR_PID_ENABLE  $BUILD_DIR/dyld_process_info_unload.exe

// RUN:  $SUDO ./dyld_process_info_unload.exe $RUN_DIR/target.exe

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

#include "test_support.h"

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    _process process;
    process.set_executable_path(RUN_DIR "/target.exe");
    process.set_launch_suspended(true);
    process.set_launch_async(true);
    const char* env[] = { "TEST_OUTPUT=None", NULL};
    process.set_env(env);
    pid_t pid = process.launch();
    task_t task;
    if (task_for_pid(mach_task_self(), pid, &task) != KERN_SUCCESS) {
        FAIL("task_for_pid() failed");
    }

    dispatch_async(dispatch_get_main_queue(), ^{
        int failCount = 0;
        for (int i=0; i < 100; ++i ) {
            kern_return_t result;
            dyld_process_info info = _dyld_process_info_create(task, 0, &result);
            LOG("info=%p, result=%08X", info, result);
            if ( i == 0 )
                (void)kill(pid, SIGCONT);
            if ( info == NULL ) {
                //FIXME: Compact info will never fail, make this a FAIL()
                failCount++;
                // ideally the fail count would be zero.  But the target is dlopen/dlclosing in a tight loop, so there may never be a stable set of images.
                // The real bug driving this test case was _dyld_process_info_create() crashing when the the image list changed too fast.
                // The important thing is to not crash.  Getting NULL back is ok.
                LOG("info=%p, result=%08X", info, result);
            }
            else {
                usleep(100);
                _dyld_process_info_release(info);
            }
        }
        PASS("Success");
    });

    dispatch_main();
}
