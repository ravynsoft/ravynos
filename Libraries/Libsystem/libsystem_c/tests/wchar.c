#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <darwintest.h>

/* 
 * Test courtesy of Roel Standaert
 * Source: https://bugs.freebsd.org/bugzilla/show_bug.cgi?id=209907 
 */
T_DECL(wchar_PR_26556792, "wcsrtombs neglects to set src pointer on EILSEQ error")
{
    char out[64];
    wchar_t *in = L"Hello! \x20AC Hello!";
    const wchar_t *inptr = in;
    mbstate_t state = {{0}};

    T_ASSERT_EQ((size_t)-1, wcsrtombs(out, &inptr, sizeof(out), &state), NULL);
    T_EXPECT_EQ((long)(inptr - in), (long)7, NULL);
}


T_DECL(wchar_PR_26828480, "double free in __vfwprintf")
{
    wchar_t *test;
    int ret;

    test = (wchar_t *) malloc(512 * sizeof(wchar_t));
    ret = swprintf(test, 512, L"%a, %s\n", 3.14, (char *) NULL);

    free(test);
    T_ASSERT_GT(ret, 0, "swprintf");
}
