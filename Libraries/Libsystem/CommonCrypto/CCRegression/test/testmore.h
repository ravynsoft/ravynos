/*
 *  testmore.h
 *  corecrypto
 *
 *  Created on 09/13/2012
 *
 *  Copyright (c) 2012,2014,2015 Apple Inc. All rights reserved.
 *
 */

#ifndef _TESTMORE_H_
#define _TESTMORE_H_  1

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <corecrypto/ccn.h>

#if defined (_WIN32)
#define __unused
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define TM_UNUSED CC_UNUSED
    
    /* This is included here, because its already included by all the test case */
#include "testlist.h"
    
    /* rng to use for testing */
    extern struct ccrng_state *global_test_rng;
    unsigned int cc_rand(unsigned max);
    cc_unit cc_rand_unit(void);
    
    typedef int (*one_test_entry)(int argc, char *const *argv);
    
#define ONE_TEST_ENTRY(x) int x(int argc, char *const *argv)
    
    struct one_test_s {
        char *name;            /* test name. */
        one_test_entry entry;  /* entry point. */
        int sub_tests;         /* number of subtests. */
        int failed_tests;      /* number of failed tests. */
        int warning_tests;     /* number of tests raised a warning. */
        int todo_tests;        /* number of todo tests */
        int actual_tests;      /* number of tests attempted. */
        int planned_tests;     /* number of planned tests. */
        const char *plan_file; /* full path to file that called plan_tests() */
        int plan_line;         /* line number in plan_file at which plan_tests was called. */
        unsigned long long duration; /* test duration in msecs. */
        int executed;           /* whether the test was run */
        /* add more later: timing, etc... */
    };
    
    extern struct one_test_s testlist[];
    
    int run_one_test(struct one_test_s *test, int argc, char * const *argv);
    
    /* this test harnes rely on shadowing for TODO, SKIP and SETUP blocks */
#pragma GCC diagnostic ignored "-Wshadow"
    
#define diag_linereturn() fputs("\n", stderr);

#define cc_print(label, count, s) { \
printf("%s { %zu, ",(const char*)label, (size_t)count); \
for (size_t ix=0; ix<(size_t)count ; ix++) { \
printf("%.02x", ((const unsigned char*)s)[ix]); \
} \
printf("}\n"); \
}\

#define ok(THIS, args...) \
test_ok(!!(THIS), NULL, test_directive, test_reason, \
__FILE__, __LINE__, args)
    
#define is(THIS, THAT, args...) \
({ \
char test_string[200]={0}; \
snprintf(test_string,sizeof(test_string),args); \
__typeof__(THIS) _this = (THIS); \
__typeof__(THAT) _that = (THAT); \
test_ok((_this == _that), test_string, test_directive, test_reason, \
__FILE__, __LINE__, \
"#          got: '%d'\n" \
"#     expected: '%d'\n" , \
_this, _that); \
})
    
#define isnt(THIS, THAT, TESTNAME) \
cmp_ok((THIS), !=, (THAT), (TESTNAME))
#define diag(MSG, ARGS...) \
test_diag(test_directive, test_reason, __FILE__, __LINE__, MSG, ## ARGS)
#define cmp_ok(THIS, OP, THAT, TESTNAME) \
({ \
__typeof__(THIS) _this = (THIS); \
__typeof__(THAT) _that = (THAT); \
test_ok((_this OP _that), TESTNAME, test_directive, test_reason, \
__FILE__, __LINE__, \
"#     '%d'\n" \
"#         " #OP "\n" \
"#     '%d'\n", \
_this, _that); \
})
#define eq_string(THIS, THAT, TESTNAME) \
({ \
const char *_this = (THIS); \
const char *_that = (THAT); \
test_ok(!strcmp(_this, _that), TESTNAME, test_directive, test_reason, \
__FILE__, __LINE__, \
"#     '%s'\n" \
"#         eq\n" \
"#     '%s'\n", \
_this, _that); \
})
#define eq_stringn(THIS, THISLEN, THAT, THATLEN, TESTNAME) \
({ \
__typeof__(THISLEN) _thislen = (THISLEN); \
__typeof__(THATLEN) _thatlen = (THATLEN); \
const char *_this = (THIS); \
const char *_that = (THAT); \
test_ok(_thislen == _thatlen && !strncmp(_this, _that, _thislen), \
TESTNAME, test_directive, test_reason, \
__FILE__, __LINE__, \
"#     '%.*s'\n" \
"#         eq\n" \
"#     '%.*s'\n", \
(int)_thislen, _this, (int)_thatlen, _that); \
})
#define like(THIS, REGEXP, TESTNAME) like_not_yet_implemented()
#define unlike(THIS, REGEXP, TESTNAME) unlike_not_yet_implemented()
#define is_deeply(STRUCT1, STRUCT2, TESTNAME) is_deeply_not_yet_implemented()
#define TODO switch(0) default
#define SKIP switch(0) default
#define SETUP switch(0) default
#define todo(REASON) const char *test_directive __attribute__((unused)) = "TODO", \
*test_reason __attribute__((unused)) = (REASON)
#define skip(WHY, HOW_MANY, UNLESS) if (!(UNLESS)) \
{ test_skip((WHY), (HOW_MANY), 0); break; }
#define setup(REASON) const char *test_directive = "SETUP", \
*test_reason = (REASON)
#define pass(TESTNAME) ok(1, (TESTNAME))
#define fail(TESTNAME) ok(0, (TESTNAME))
#define BAIL_OUT(WHY) test_bail_out(WHY, __FILE__, __LINE__)
#define plan_skip_all(REASON) test_plan_skip_all(REASON)
#define plan_tests(COUNT) test_plan_tests(COUNT, __FILE__, __LINE__)
    
#define ok_status(THIS, TESTNAME) \
({ \
int _this = (THIS); \
test_ok(!_this, TESTNAME, test_directive, test_reason, \
__FILE__, __LINE__, \
"#     status: %d\n",_this); \
})
    
#define ok_status_or_goto(THIS, TESTNAME,LABEL) \
({ \
int _this = (THIS); \
test_ok(!_this, TESTNAME, test_directive, test_reason, \
__FILE__, __LINE__, \
"#     status: %d\n",_this); \
if(_this) goto LABEL; \
})
    
#define ok_or_fail(THIS, args...) \
({ \
if (!ok(THIS,args)) return 0; \
})
    
#define ok_or_warning(THIS, TESTNAME) \
({ \
int _this = (THIS); \
test_ok(_this, TESTNAME, "WARNING", test_reason, \
__FILE__, __LINE__, \
"#     status: %ld\n", _this); \
})
    
#define ok_or_goto(THIS, TESTNAME, LABEL) \
({ \
int _this = (THIS); \
test_ok(_this, TESTNAME, test_directive, test_reason, \
__FILE__, __LINE__, \
"#     status: %d\n", _this);\
if(_this == 0) goto LABEL; \
})
    
#define ccn_cmp_print(_NLEN_, _P1_, _P2_) \
({ \
int _this = ccn_cmp(_NLEN_, (_P1_), (_P2_)); \
if(_this!=0) {ccn_lprint(_NLEN_,"Compare: ",(_P1_)); \
ccn_lprint(_NLEN_,"    and: ",(_P2_));} \
_this; \
})
    
#define ok_ccn_cmp(_NLEN_, _P1_, _P2_, args...) ok(ccn_cmp_print(_NLEN_, _P1_, _P2_)==0, args)
    
#define memcmp_print(_P1_, _P2_, _LEN_) \
({ \
int _this = memcmp((_P1_), (_P2_), _LEN_); \
if(_this!=0) {cc_print("Compare: ",_LEN_,(const uint8_t*)(_P1_)); \
cc_print("    and: ",_LEN_,(const uint8_t*)(_P2_));} \
_this; \
})
    
#define ok_memcmp(_P1_, _P2_, _LEN_, args...) ok(memcmp_print(_P1_, _P2_, _LEN_)==0, args)
    
#define ok_memcmp_or_fail(_P1_, _P2_, _LEN_, args...) \
({ \
if(!ok(memcmp_print(_P1_, _P2_, _LEN_)==0, args)) return 0; \
})
    
#define is(THIS, THAT, args...) \
({ \
char test_string[200]={0}; \
snprintf(test_string,sizeof(test_string),args); \
__typeof__(THIS) _this = (THIS); \
__typeof__(THAT) _that = (THAT); \
test_ok((_this == _that), test_string, test_directive, test_reason, \
__FILE__, __LINE__, \
"#          got: '%d'\n" \
"#     expected: '%d'\n" , \
_this, _that); \
})
    
#define is_or_goto(THIS, THAT, TESTNAME, LABEL) \
({ \
__typeof__(THIS) _this = (THIS); \
__typeof__(THAT) _that = (THAT); \
test_ok((_this == _that), TESTNAME, test_directive, test_reason, \
__FILE__, __LINE__, \
"#          got: '%d'\n" \
"#     expected: '%d'\n" , \
_this, _that); \
if(_this != _that) goto LABEL; \
})
    
#define is_status(THIS, THAT, TESTNAME) \
({ \
OSStatus _this = (THIS); \
OSStatus _that = (THAT); \
test_ok(_this == _that, TESTNAME, test_directive, test_reason, \
__FILE__, __LINE__, \
"#          got: %ld\n" \
"#     expected: %ld\n", \
_this, _that); \
})
    
#define entryPoint(testname,supportname) \
int testname(TM_UNUSED int argc, TM_UNUSED char *const *argv) { \
char prString[80];\
sprintf(prString, "No %s Support in this release\n", supportname);\
plan_tests(1); \
diag(prString); \
ok(1, prString); \
return 0; \
}
    
    
    extern const char *test_directive;
    extern const char *test_reason;
    
    void test_bail_out(const char *reason, const char *file, unsigned line);
    int test_diag(const char *directive, const char *reason,
                  const char *file, unsigned line, const char *fmt, ...);
    int test_ok(int passed, const char *description, const char *directive,
                const char *reason, const char *file, unsigned line, const char *fmt, ...);
    void test_plan_skip_all(const char *reason);
    void test_plan_tests(int count, const char *file, unsigned line);
    void test_skip(const char *reason, int how_many, int unless);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_TESTMORE_H_ */
