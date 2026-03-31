#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <dispatch/dispatch.h>


__attribute__((section("__DATA,__allow_alt_plat"))) uint64_t dummy;

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    dispatch_source_t exitSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_PROC, getppid(),
                                                            DISPATCH_PROC_EXIT, dispatch_get_main_queue());
    dispatch_source_set_event_handler(exitSource, ^{
        exit(0);
    });
    dispatch_resume(exitSource);
    dispatch_async(dispatch_get_main_queue(), ^{
        kill(getppid(), SIGUSR1);
    });
    dispatch_main();
}

