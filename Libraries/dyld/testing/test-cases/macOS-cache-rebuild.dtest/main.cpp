// BUILD_ONLY: MacOSX

// BUILD:  $CXX main.cpp          -o $BUILD_DIR/rebuild-dyld-cache.exe

// FIXME: This test will not make sense when remove update_dyld_shared_cache, and the functionality will be subsummed by root testing
// ./rebuild-dyld-cache.exe


#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <signal.h>
#include <spawn.h>
#include <errno.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "test_support.h"

extern char** environ;

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    _process process;
    process.set_executable_path("/usr/bin/update_dyld_shared_cache");
    const char* env[] = { "TEST_OUTPUT=None", NULL};
    process.set_env(env);
    const char* args[] = { "-cache_dir", "/tmp/", NULL };
    process.set_args(args);
    process.set_exit_handler(^(pid_t pid) {
        int childStatus;
        (void)wait4(pid, &childStatus, 0, NULL);
        if (WIFEXITED(childStatus) == 0)
            FAIL("update_dyld_shared_cache did not exit");
        else if (WEXITSTATUS(childStatus) != 0)
            FAIL("update_dyld_shared_cache failed");
        else
            PASS("Success");
    });

    process.launch();
}

