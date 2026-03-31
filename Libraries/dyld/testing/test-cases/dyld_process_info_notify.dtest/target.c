#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <signal.h>
#include <unistd.h>
#include <mach/mach.h>
#include <dispatch/dispatch.h>

void performDylibOperations(void) {
    for (int i=0; i < 3; ++i) {
        void* h = dlopen(RUN_DIR "/libfoo.dylib", 0);
        dlclose(h);
    }
    fprintf(stderr, "Done (pid: %d)\n", getpid());
    exit(0);
}

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    signal(SIGUSR1, SIG_IGN);
    dispatch_source_t signalSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_PROC, getppid(),
                                                            DISPATCH_PROC_EXIT, dispatch_get_main_queue());
    dispatch_source_set_event_handler(signalSource, ^{
        exit(0);
    });
    dispatch_resume(signalSource);

    if ( (argc > 1) && (strcmp(argv[1], "suspend-in-main") == 0) ) {
        dispatch_source_t signalSourceSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL,
                                                                      SIGUSR1, 0, dispatch_get_main_queue());
        dispatch_source_set_event_handler(signalSourceSource, ^{
            performDylibOperations();
        });
        dispatch_resume(signalSourceSource);
        dispatch_async(dispatch_get_main_queue(), ^{
            fprintf(stderr, "Ready (pid: %d)\n", getpid());
        });
    } else {
        performDylibOperations();
    }

    dispatch_main();
}

