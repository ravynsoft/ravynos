
// BUILD:  $CC foo.c -dynamiclib -install_name /cant/find/me.dylib -o $BUILD_DIR/libmissing.dylib
// BUILD:  $CC foo.c -dynamiclib $BUILD_DIR/libmissing.dylib -install_name $RUN_DIR/libMissingDylib.dylib -o $BUILD_DIR/libMissingDylib.dylib
// BUILD:  $CC emptyMain.c $BUILD_DIR/libMissingDylib.dylib  -o $BUILD_DIR/prog_missing_dylib.exe
// BUILD:  $CC defSymbol.c -dynamiclib -install_name $RUN_DIR/libMissingSymbols.dylib -o $BUILD_DIR/libMissingSymbols.dylib
// BUILD:  $CC defSymbol.c -dynamiclib -install_name $RUN_DIR/libMissingSymbols.dylib -o $BUILD_DIR/libHasSymbols.dylib -DHAS_SYMBOL
// BUILD:  $CC useSymbol.c $BUILD_DIR/libHasSymbols.dylib -o $BUILD_DIR/prog_missing_symbol.exe
// BUILD:  $CXX main.cpp -o $BUILD_DIR/dyld_abort_tests.exe

// NO_CRASH_LOG: prog_missing_dylib.exe
// NO_CRASH_LOG: prog_missing_symbol.exe

// RUN:  ./dyld_abort_tests.exe 

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <signal.h>
#include <spawn.h>
#include <errno.h>
#include <mach/mach.h>
#include <mach/machine.h>
#include <err.h>
#include <System/sys/reason.h>
#include <System/sys/proc_info.h>
#include <System/kern/kern_cdata.h>
#include <libproc.h>
#include <mach-o/dyld_priv.h>

#include "test_support.h"


void runTest(const char* prog, uint64_t dyldReason, const char* expectedDylibPath, const char* expectedSymbol) {
    _process process;
    process.set_executable_path(prog);
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
        if ( ers->ers_code != dyldReason ) {
            FAIL("eri_code (%llu) != dyldReason (%lld)", ers->ers_code, dyldReason);
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

                if ( (dyldInfo->flags & 1) == 0 ) {
                    FAIL("dyld flags should have low bit  set to indicate process terminated during launch");
                }

                if ( expectedDylibPath != NULL ) {
                    if ( dyldInfo->targetDylibPathOffset != 0 ) {
                        const char* targetDylib = (char*)dyldInfo + dyldInfo->targetDylibPathOffset;
                        if ( strstr(targetDylib, expectedDylibPath) == NULL ) {
                            FAIL("dylib path (%s) not what expected (%s)", targetDylib, expectedDylibPath);
                        }
                    } else {
                        FAIL("dylib path (%s) not provided by dyld", expectedDylibPath);
                    }
                }

                if ( expectedSymbol != NULL ) {
                    if ( dyldInfo->targetDylibPathOffset != 0 ) {
                        const char* missingSymbol = (char*)dyldInfo + dyldInfo->symbolOffset;
                        if ( strcmp(expectedSymbol, missingSymbol) != 0 ) {
                            FAIL("symbol (%s) not what expected (%s)", missingSymbol, expectedSymbol);
                        }
                    } else {
                        FAIL("symbol (%s) not provided by dyld", expectedSymbol);
                    }
                }
                PASS("Success");
            }
        }
        FAIL("Did not find EXIT_REASON_USER_PAYLOAD");
    });
    process.launch();
}


int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
    // test launch program with missing library
    runTest("./prog_missing_dylib.exe", DYLD_EXIT_REASON_DYLIB_MISSING, "/cant/find/me.dylib", NULL);
//    runTest("./prog_missing_symbol.exe", DYLD_EXIT_REASON_SYMBOL_MISSING, "libMissingSymbols.dylib", "_slipperySymbol");
    PASS("Success");
}

