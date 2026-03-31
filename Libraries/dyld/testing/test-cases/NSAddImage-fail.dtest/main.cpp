// BUILD_ONLY: MacOSX

// BUILD:  $CXX main.cpp          -o $BUILD_DIR/NSAddImage-fail.exe -Wno-deprecated-declarations -DRUN_DIR="$RUN_DIR"

// RUN:  ./NSAddImage-fail.exe return
// RUN:  ./NSAddImage-fail.exe abort



#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <libproc.h>
#include <mach-o/dyld.h>
#include <mach-o/dyld_priv.h>
#include <System/sys/reason.h>
#include <System/sys/proc_info.h>
#include <System/kern/kern_cdata.h>

#include "test_support.h"

//FIXME:

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    const char* arg = argv[1];

    if ( strcmp(arg, "return") == 0 ) {
        const struct mach_header* mh = NSAddImage("/xqz/42/libnotfound.xxx", NSADDIMAGE_OPTION_RETURN_ONLY_IF_LOADED);
        if ( mh == NULL )
            PASS("Success");
        else
            FAIL("Got mh non-existent image");
    } else if (strcmp(arg, "abort-child") == 0) {
        // run with nocr which print BEGIN/PASS/FAIL
        NSAddImage("/xqz/42/libnotfound.xxx", 0);
    } else if (strcmp(arg, "abort") == 0) {
        _process process;
        process.set_executable_path(RUN_DIR "/NSAddImage-fail.exe");
        const char* args[] = {"abort-child", NULL };
        process.set_args(args);
        const char* env[] = { "TEST_OUTPUT=None", NULL};
        process.set_env(env);
        process.set_crash_handler(^(task_t task) {
            LOG("Crash for task=%u", task);
            vm_address_t corpse_data;
            uint32_t corpse_size;
            if (task_map_corpse_info(mach_task_self(), task, &corpse_data, &corpse_size) != KERN_SUCCESS) {
                FAIL("Could not read corpse data");
            }
            kcdata_iter_t autopsyData = kcdata_iter((void*)corpse_data, corpse_size);
            if (!kcdata_iter_valid(autopsyData)) {
                FAIL("Corpse Data Invalid");
            }
            kcdata_iter_t exitReasonData = kcdata_iter_find_type(autopsyData, EXIT_REASON_SNAPSHOT);
            if (!kcdata_iter_valid(exitReasonData)) {
                FAIL("Could not find exit data");
            }
            struct exit_reason_snapshot *ers = (struct exit_reason_snapshot *)kcdata_iter_payload(exitReasonData);

            if ( ers->ers_namespace != OS_REASON_DYLD ) {
                FAIL("eri_namespace (%d) != OS_REASON_DYLD", ers->ers_namespace);
            }
            if ( ers->ers_code != DYLD_EXIT_REASON_OTHER ) {
                FAIL("eri_code (%lld) != DYLD_EXIT_REASON_OTHER", ers->ers_code);
            }
            kcdata_iter_t iter = kcdata_iter((void*)corpse_data, corpse_size);

            KCDATA_ITER_FOREACH(iter) {
                if (kcdata_iter_type(iter) == KCDATA_TYPE_NESTED_KCDATA) {
                    kcdata_iter_t nestedIter = kcdata_iter(kcdata_iter_payload(iter), kcdata_iter_size(iter));
                    if ( kcdata_iter_type(nestedIter) != KCDATA_BUFFER_BEGIN_OS_REASON ){
                        return;
                    }
                    kcdata_iter_t payloadIter = kcdata_iter_find_type(nestedIter, EXIT_REASON_USER_PAYLOAD);
                    if ( !kcdata_iter_valid(payloadIter) ) {
                        FAIL("invalid kcdata payload iterator from payload data");
                    }
                    const dyld_abort_payload* dyldInfo = (dyld_abort_payload*)kcdata_iter_payload(payloadIter);

                    if ( dyldInfo->version != 1 ) {
                        FAIL("dyld payload is not version 1");
                    }
                    PASS("Success");
                }
            }
            FAIL("Did not find EXIT_REASON_USER_PAYLOAD");
        });
        process.launch();
        dispatch_main();
    }
	return 0;
}

