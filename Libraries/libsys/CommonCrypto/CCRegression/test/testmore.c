/*
 *  testmore.c
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
#if defined(_WIN32)
#include <windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

#include "testmore.h"

static int test_num = 0;
static int test_fails = 0;
static int test_cases = 0;
static int test_todo = 0;
static int test_warning = 0;
static const char *test_plan_file;
static int test_plan_line=0;

const char *test_directive = NULL;
const char *test_reason = NULL;

void test_skip(const char *reason, int how_many, int unless)
{
    if (unless)
        return;
    
    int done;
    for (done = 0; done < how_many; ++done)
        test_ok(1, NULL, "skip", reason, __FILE__, __LINE__, NULL);
}

void test_bail_out(const char *reason, const char *file, unsigned line)
{
    printf("BAIL OUT! (%s at line %u) %s\n", file, line, reason);
    fflush(stdout);
    exit(255);
}

void test_plan_skip_all(const char *reason)
{
    if (test_num > test_cases)
    {
        test_skip(reason, test_cases - test_num, 0);
        exit(test_fails > 255 ? 255 : test_fails);
    }
}

static void test_plan_reset(void) {
    test_fails = 0;
    test_num = 0;
    test_cases = 0;
    test_plan_file = NULL;
    test_plan_line = 0;
    test_warning = 0;
}

static void test_plan_exit(void)
{
    // int status = 0;
    fflush(stdout);
    
    if (!test_num)
    {
        if (test_cases)
        {
            fprintf(stderr, "%s:%u: warning: No tests run!\n", test_plan_file, test_plan_line);
            // status = 255;
        }
        else
        {
            fprintf(stderr, "%s:%u: error: Looks like your test died before it could "
                    "output anything.\n", test_plan_file, test_plan_line);
            // status = 255;
        }
    }
    else {
        if (test_fails)
        {
            fprintf(stderr, "%s:%u: error: Looks like you failed %d tests of %d.\n",
                    test_plan_file, test_plan_line, test_fails, test_cases);
            // status = test_fails;
        }
        if (test_num < test_cases)
        {
            fprintf(stderr, "%s:%u: warning: Looks like you planned %d tests but only ran %d.\n",
                    test_plan_file, test_plan_line, test_cases, test_num);
            // status = test_fails + test_cases - test_num;
        }
        else if (test_num > test_cases)
        {
            fprintf(stderr, "%s:%u: warning: Looks like you planned %d tests but ran %d extra.\n",
                    test_plan_file, test_plan_line, test_cases, test_num - test_cases);
            // status = test_fails;
        }
    }
    
    fflush(stderr);
    test_plan_reset();
}

void test_plan_tests(int count, const char *file, unsigned line)
{
    if (test_cases)
    {
        fprintf(stderr,
                "%s:%u: error: You tried to plan twice!\n",
                file, line);
        
        fflush(stderr);
        exit(255);
    }
    else
    {
        if (!count)
        {
            fprintf(stderr, "%s:%u: warning: You said to run 0 tests!  You've got to run "
                    "something.\n", file, line);
            fflush(stderr);
            exit(255);
        }
        
        test_plan_file=file;
        test_plan_line=line;
        
        test_cases = count;
        fprintf(stderr, "%s:%u: note: 1..%d\n", file, line, test_cases);
        fflush(stdout);
    }
}

int
test_diag(const char *directive, __unused const char *reason,
          __unused const char *file, __unused unsigned line, const char *fmt, ...)
{
    int is_todo = directive && !strcmp(directive, "TODO");
    va_list args;
    
    va_start(args, fmt);
    
    if (is_todo)
    {
        fputs("# ", stdout);
        if (fmt)
            vprintf(fmt, args);
        fputs("\n", stdout);
        fflush(stdout);
    }
    else
    {
        fflush(stdout);
        fputs("# ", stderr);
        if (fmt)
            vfprintf(stderr, fmt, args);
        fputs("\n", stderr);
        fflush(stderr);
    }
    
    va_end(args);
    
    return 1;
}

int
test_ok(int passed, const char *description, const char *directive,
        const char *reason, const char *file, unsigned line,
        const char *fmt, ...)
{
    int is_todo = !passed && directive && !strcmp(directive, "TODO");
    int is_warning = !passed && directive && !strcmp(directive, "WARNING");
    int is_setup = directive && !is_todo && !strcmp(directive, "SETUP");
    
    if (is_setup)
    {
        if (!passed)
        {
            fflush(stdout);
            fprintf(stderr, "# SETUP not ok%s%s%s%s\n",
                    description ? " - " : "",
                    description ? description : "",
                    reason ? " - " : "",
                    reason ? reason : "");
        }
    }
    else
    {
        if (!test_cases)
        {
            atexit(test_plan_exit);
            fprintf(stderr, "You tried to run a test without a plan!  "
                    "Gotta have a plan. at %s line %u\n", file, line);
            fflush(stderr);
            exit(255);
        }
        
        ++test_num;
        if (!passed && !is_todo && !is_warning) {
            ++test_fails;
        }
        /* We dont need to print this unless we want to */
#if 0
        fprintf(stderr, "%s:%u: note: %sok %d%s%s%s%s%s%s\n", file, line, passed ? "" : "not ", test_num,
                description ? " - " : "",
                description ? description : "",
                directive ? " # " : "",
                directive ? directive : "",
                reason ? " " : "",
                reason ? reason : "");
#endif
    }
    
    if (passed)
        fflush(stdout);
    else
    {
        va_list args;
        
        va_start(args, fmt);
        
        if (is_todo)
        {
            /* Enable this to output TODO as warning */
#if 0
            printf("%s:%d: warning: Failed (TODO) test\n", file, line);
            if (fmt)
                vprintf(fmt, args);
#endif
            ++test_todo;
            fflush(stdout);
        }
        else if (is_warning)
        {
            /* Enable this to output warning */
            printf("%s:%d: warning: Failed test [%s]\n", file, line, description);
            if (fmt)
                vprintf(fmt, args);
            ++test_warning;
            fflush(stdout);
        }
        else
        {
            fflush(stdout);
            if (description) {
                fprintf(stderr, "%s:%d: error: Failed test [%s]\n", file, line, description);
                if (fmt)
                    vfprintf(stderr, fmt, args);
            } else {
                fprintf(stderr, "%s:%d: error: Failed test [", file, line);
                vfprintf(stderr, fmt, args);
                fprintf(stderr, "]\n");
            }
            fflush(stderr);
        }
        
        va_end(args);
    }
    
    return passed;
}

/* run one test, described by test, return info in test struct */
int run_one_test(struct one_test_s *test, int argc, char * const *argv)
{
    
    test->executed=1;
    
    if(test->entry==NULL) {
        printf("%s:%d: error: wtf?\n", __FILE__, __LINE__);
        return -1;
    }
    
#if defined(_WIN32)
    SYSTEMTIME st, end;
    
    GetSystemTime(&st);
    test->entry(argc, argv);
    GetSystemTime(&end);
    test->duration = (end.wMinute-st.wMinute)*60*1000 + (end.wSecond-st.wSecond)*1000 + (end.wMilliseconds-st.wMilliseconds);
#else
    struct timeval start, stop;
    gettimeofday(&start, NULL);
    test->entry(argc, argv);
    gettimeofday(&stop, NULL);
    /* this may overflow... */
    test->duration=(unsigned long long) (stop.tv_sec-start.tv_sec)*1000+(stop.tv_usec/1000)-(start.tv_usec/1000);
#endif
    
    test->failed_tests=test_fails;
    test->actual_tests=test_num;
    test->planned_tests=test_cases;
    test->plan_file=test_plan_file;
    test->plan_line=test_plan_line;
    test->todo_tests=test_todo;
    test->warning_tests=test_warning;
    
    
    test_plan_exit();
    
    return test->failed_tests;
}



