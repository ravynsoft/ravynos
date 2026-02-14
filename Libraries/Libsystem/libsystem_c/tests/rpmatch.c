#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <locale.h>

#include <darwintest.h>

static char *expectations[] = {
    "inclonclusive", /* -1 */
    "negative", /* 0 */
    "affirmative" /* 1 */
};

static size_t
dumb_strescape(char * dst, const char * in, size_t len) {
    size_t count = 0;

    while (*in && count + 3 < len) {
        switch (*in) {
          case '\"':
            *dst++ = '\\';
            *dst++ = '\"';
            count += 2;
            break;
          case '\'':
            *dst++ = '\\';
            *dst++ = '\"';
            count += 2;
            break;
          case '\\':
            *dst++ = '\\';
            *dst++ = '\\';
            count += 2;
            break;
          case '\a':
            *dst++ = '\\';
            *dst++ = 'a';
            count += 2;
            break;
          case '\b':
            *dst++ = '\\';
            *dst++ = 'b';
            count += 2;
            break;
          case '\n':
            *dst++ = '\\';
            *dst++ = 'n';
            count += 2;
            break;
          case '\t':
            *dst++ = '\\';
            *dst++ = 't';
            count += 2;
            break;
          /* There are many more special cases */
          default:
            if (iscntrl(*in)) {
                count += (size_t)snprintf(dst, len - count, "\\%03o", *in);
            }
            else {
                *dst++ = *in;
                count++;
            }
        }
        in++;
    }
    *dst++ = '\0';
    return count;
}

static void
rpmatch_testcase(const char * response, int expectation) {
    const char * expect_msg = expectations[expectation+1];

    char escaped_response[16];
    dumb_strescape(escaped_response, response, sizeof(escaped_response)/sizeof(char));

    /* darwintest should escape special characters in message strings */
    T_EXPECT_EQ(rpmatch(response), expectation, "'%s' is %s in the %s locale", escaped_response, expect_msg, setlocale(LC_ALL, NULL));
}

T_DECL(rpmatch, "Ensure rpmatch responds to locales")
{
    setlocale(LC_ALL, "C");

    /* Check several single character variants */
    rpmatch_testcase("y", 1);
    rpmatch_testcase("Y", 1);
    rpmatch_testcase("j", -1); /* becomes afirmative in german */
    rpmatch_testcase("J", -1);
    rpmatch_testcase("x", -1);
    rpmatch_testcase(" ", -1);
    rpmatch_testcase("", -1);
    rpmatch_testcase("n", 0);
    rpmatch_testcase("N", 0);

    /* A few full words */
    rpmatch_testcase("yes", 1);
    rpmatch_testcase("ja", -1);
    rpmatch_testcase("no", 0);

    /* Check each variant with a newline */
    rpmatch_testcase("y\n", 1);
    rpmatch_testcase("Y\n", 1);
    rpmatch_testcase("j\n", -1);
    rpmatch_testcase("J\n", -1);
    rpmatch_testcase("x\n", -1);
    rpmatch_testcase(" \n", -1);
    rpmatch_testcase("\n", -1);
    rpmatch_testcase("n\n", 0);
    rpmatch_testcase("N\n", 0);

    rpmatch_testcase("yes\n", 1);
    rpmatch_testcase("ja\n", -1);
    rpmatch_testcase("no\n", 0);

    /* Do it all again in a german locale */
    setlocale(LC_ALL, "de_DE.ISO8859-1");

    if (strcmp(setlocale(LC_ALL, NULL), "de_DE.ISO8859-1") != 0) {
        T_LOG("This system does not have a de_DE.ISO8859-1 locale");
        return;
    }

    /* Check several single character variants */
    rpmatch_testcase("y", 1);
    rpmatch_testcase("Y", 1);
    rpmatch_testcase("j", 1); /* now afirmative */
    rpmatch_testcase("J", 1);
    rpmatch_testcase("x", -1);
    rpmatch_testcase(" ", -1);
    rpmatch_testcase("", -1);
    rpmatch_testcase("n", 0);
    rpmatch_testcase("N", 0);

    /* A few full words */
    rpmatch_testcase("yes", 1);
    rpmatch_testcase("ja", 1);
    rpmatch_testcase("xx", -1);
    rpmatch_testcase("no", 0);

    /* Check each variant with a newline */
    rpmatch_testcase("y\n", 1);
    rpmatch_testcase("Y\n", 1);
    rpmatch_testcase("j\n", 1);
    rpmatch_testcase("J\n", 1);
    rpmatch_testcase("x\n", -1);
    rpmatch_testcase(" \n", -1);
    rpmatch_testcase("\n", -1);
    rpmatch_testcase("n\n", 0);
    rpmatch_testcase("N\n", 0);

    rpmatch_testcase("yes\n", 1);
    rpmatch_testcase("ja\n", 1);
    rpmatch_testcase("xx\n", -1);
    rpmatch_testcase("no\n", 0);
}
