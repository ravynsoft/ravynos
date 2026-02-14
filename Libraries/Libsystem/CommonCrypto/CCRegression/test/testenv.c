/*
 *  testenv.c
 *  corecrypto
 *
 *  Created on 09/13/2012
 *
 *  Copyright (c) 2012,2014,2015 Apple Inc. All rights reserved.
 *
 */

#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <stdbool.h>
#if defined(_WIN32)
static int optind = 1;
#else
#include <unistd.h>
#include <dlfcn.h>
#endif

#include "testmore.h"
#include "testenv.h"

static int tests_printall(void);

static int
tests_summary(int verbose) {
    int failed_tests = 0;
    int todo_tests = 0;
    int actual_tests = 0;
    int planned_tests = 0;
    int warning_tests = 0;
    uint64_t duration_tests = 0;
    
    // First compute the totals to help decide if we need to print headers or not.
    for (int i = 0; testlist[i].name; ++i) {
        if (testlist[i].executed) {
            failed_tests += testlist[i].failed_tests;
            todo_tests += testlist[i].todo_tests;
            actual_tests += testlist[i].actual_tests;
            planned_tests += testlist[i].planned_tests;
            warning_tests += testlist[i].warning_tests;
            duration_tests += testlist[i].duration;
        }
    }
    
    fprintf(stdout, "\n[SUMMARY]\n");
    
    // -v makes the summary verbose as well.
    if (verbose || failed_tests || actual_tests != planned_tests || todo_tests || warning_tests) {
        fprintf(stdout, "Test name                                                failed  warning  todo  ran  planned\n");
        fprintf(stdout, "============================================================================================\n");
    }
    for (int i = 0; testlist[i].name; ++i) {
        if (testlist[i].executed) {
            const char *token = NULL;
            if (testlist[i].failed_tests) {
                token = "FAIL";
            } else if (testlist[i].actual_tests != testlist[i].planned_tests
                       || (testlist[i].todo_tests)
                       || (testlist[i].warning_tests)) {
                token = "WARN";
            } else if (verbose) {
                token = "PASS";
            }
            if (token) {
                fprintf(stdout, "[%s] %-49s %6d  %6d %6d %6d %6d\n", token,
                        testlist[i].name,
                        testlist[i].failed_tests, testlist[i].warning_tests,
                        testlist[i].todo_tests,
                        testlist[i].actual_tests, testlist[i].planned_tests);
            }
        }
    }
    if (verbose || failed_tests || warning_tests || todo_tests || actual_tests != planned_tests) {
        fprintf(stdout, "============================================================================================\n");
    }
    else {
        fprintf(stdout, "Test name                                                failed  warning  todo  ran  planned\n");
    }
    fprintf(stdout, "Totals (%6llus)                                         %6d  %6d %6d %6d %6d\n", duration_tests/1000, failed_tests, warning_tests, todo_tests, actual_tests, planned_tests);
    return failed_tests;
}

#if defined(_WIN32)
//rdar://problem/26799042
static int tests_run_index(int i, int argc, char * const *argv)
{
    fprintf(stderr, "\n[BEGIN] %s\n", testlist[i].name);
    
    run_one_test(&testlist[i], argc, argv);
    if(testlist[i].failed_tests) {
        fprintf(stderr, "[FAIL] %s\n", testlist[i].name);
    } else {
        fprintf(stderr, "duration: %llu ms\n", testlist[i].duration);
        fprintf(stderr, "[PASS] %s\n", testlist[i].name);
    }
    
    return 0;
}
#else
static void usage(const char *progname)
{
    fprintf(stderr, "usage: %s [-L][-v][-s seed][-w][testname [testargs] ...]\n", progname);
    fprintf(stderr, "\t-v verbose\n");
    fprintf(stderr, "\t-s <seed> to provide a specific seed (ex 8686b151ec2aa17c4ec41a59e496d2ff)\n");
    fprintf(stderr, "\t-w sleep(100)\n");
    fprintf(stderr, "\t-L list supported tests by test names\n");
    fprintf(stderr, "Here is the list of supported tests:\n");
    tests_printall();
    exit(1);
}

static int tests_run_index(int i, int argc, char * const *argv)
{
    int verbose = 0;
    int ch;
    
    while ((ch = getopt(argc, argv, "v")) != -1)
    {
        switch  (ch)
        {
            case 'v':
                verbose++;
                break;
            default:
                usage(argv[0]);
        }
    }
    
    fprintf(stderr, "\n[BEGIN] %s\n", testlist[i].name);
    
    run_one_test(&testlist[i], argc, argv);
    if(testlist[i].failed_tests) {
        fprintf(stderr, "[FAIL] %s\n", testlist[i].name);
    } else {
        fprintf(stderr, "duration: %llu ms\n", testlist[i].duration);
        fprintf(stderr, "[PASS] %s\n", testlist[i].name);
    }
    
    return 0;
}

static int tests_named_index(const char *testcase)
{
    int i;
    
    for (i = 0; testlist[i].name; ++i) {
        if (strcmp(testlist[i].name, testcase) == 0) {
            return i;
        }
    }
    
    return -1;
}

#endif

static int tests_printall(void)
{
    for (int i = 0; testlist[i].name; ++i) {
        fprintf(stdout, "%s\n", testlist[i].name);
    }
    
    return 0;
}

static int tests_run_all(int argc, char * const *argv)
{
    int curroptind = optind;
    int i;
    
    for (i = 0; testlist[i].name; ++i) {
        tests_run_index(i, argc, argv);
        optind = curroptind;
    }
    
    return 0;
}

static int tests_rng(const char *seed) {
    (void)seed;
    return 0;
}

#if CC_XNU_KERNEL_AVAILABLE
static off_t fsize(const char *fname)
{
    struct stat st;
    return (stat(fname, &st) == 0)? st.st_size:-1;
}

#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonCryptorSPI.h>

static void print_lib_path(void)
{
    Dl_info dl_info;
    if( dladdr((void *)cc_clear, &dl_info) != 0){
        fprintf(stderr, "corecrypto loaded: %s\nDYLIB size %lld bytes\n\n", dl_info.dli_fname, fsize(dl_info.dli_fname));
    }
    
    if( dladdr((void *)CCCryptorGCMFinalize, &dl_info) != 0){
        fprintf(stderr, "CommonCrypto loaded: %s\nDYLIB size %lld bytes\n\n", dl_info.dli_fname, fsize(dl_info.dli_fname));
    }
}
#else
static void print_lib_path(void){

}
#endif

static int tests_init(char* seed) {
    print_lib_path();
    printf("\n[TEST] === CommonCrypto ===\n");
    return tests_rng(seed);
}

static int tests_end(void) {
    //commoncrypto does not have a global rng
    //if (test_rng.drbg_state !=NULL) ccrng_test_done(&test_rng);
    return 0;
}

#if defined(_WIN32)
int
tests_begin(int argc, char * const *argv)
{
    char *seed = NULL; //seed for test drbg
    int list = 0;
    int retval;
    int verbose = 0;
    
    printf("Command-line options are currently not supported on Windows.\n");
    //rdar://problem/26799042 corecrypto coresponding radar
    
    if (tests_init(seed) != 0) return -1;
    tests_run_all(argc, argv);
    
    if (list) {
        tests_printall();
        retval = 0;
    }
    else {
        retval = tests_summary(verbose);
    }
    /* Cleanups */
    tests_end();
    
    return retval;
}
#else

int
tests_begin(int argc, char * const *argv)
{
    char *seed=NULL; //seed for test drbg
    int list=0;
    int retval;
    int verbose = 0;
    
    const char *testcase = NULL;
    bool initialized = false;
    int testix = -1;
    int ch;
    
    for (;;) {
        
        while (!testcase && (ch = getopt(argc, argv, "Lvws:")) != -1)
        {
            switch  (ch)
            {
                case 's': // seed provided
                    seed = optarg;
                    break;
                case 'w': // wait
                    sleep(100);
                    break;
                case 'v': // verbose
                    verbose=1;
                    break;
                case 'L': // List test for test discovery
                    list=1;
                    break;
                case '?':
                default:
                    printf("invalid option %c\n",ch);
                    usage(argv[0]);
            }
        }
        
        if (optind < argc) {
            testix = tests_named_index(argv[optind]);
            if(testix<0) {
                printf("invalid test %s\n",argv[optind]);
                usage(argv[0]);
                
            }
            argc -= optind;
            argv += optind;
        }
        
        if (testix < 0) {
            if (!initialized && !list) {
                //initialized = true;
                if (tests_init(seed)!=0) return -1;
                tests_run_all(argc, argv);
            }
            break;
        } else if (!list) {
            if (!initialized) {
                if (tests_init(seed)!=0) return -1;
                initialized = true;
            }
            tests_run_index(testix, argc, argv);
            testix = -1;
        }
    }
    
    if (list) {
        tests_printall();
        retval=0;
    } else {
        retval=tests_summary(verbose);
    }
    /* Cleanups */
    tests_end();
    
    return retval;
}
#endif
