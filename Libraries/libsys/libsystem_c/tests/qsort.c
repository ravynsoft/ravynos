/*
 * Randomized test for qsort() routine.
 */

#include <sys/cdefs.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <darwintest.h>
#include <darwintest_utils.h>

#include "freebsd_qsort.h"

#define BUFLEN (32 * 1024)

static int tests = 10;

T_DECL(qsort_random, "qsort random test", T_META_CHECK_LEAKS(NO))
{
	char testvector[BUFLEN];
	char sresvector[BUFLEN];
    size_t i;

    while (--tests >= 0) {
        size_t elmsize = (tests % 32) + 1;
        size_t elmcount = sizeof(testvector) / elmsize;
        T_LOG("%d: size = %zu, count = %zu", tests, elmsize, elmcount);

        /* Populate test vectors */
        arc4random_buf(testvector, sizeof(testvector));
        memcpy(sresvector, testvector, sizeof(testvector));

        /* Sort using qsort(3) */
        qsort_r(testvector, elmcount, elmsize, (void*)elmsize, szsorthelp);
        /* Sort using reference slow sorting routine */
        szsort(sresvector, elmcount, elmsize);

        /* Compare results */
        for (i = 0; i < elmcount; i++){
            if (memcmp(testvector + (i * elmsize), sresvector + (i * elmsize), elmsize) != 0) {
                T_LOG("testvector =");
                dt_print_hex_dump((uint8_t*)testvector, sizeof(testvector));
                T_LOG("sresvector =");
                dt_print_hex_dump((uint8_t*)sresvector, sizeof(sresvector));
                T_ASSERT_FAIL("%d: item at index %zd should match", tests, i);
                break;
            }
        }
        if (i == elmcount) {
            T_PASS("%d: Sorted successfully.", tests);
        }
    }
}
