/* This is a wrapper around X11 tests to make it faster to use for the simple
 * type of test cases.
 *
 * Use with the X11_TEST macro like this:
 *
 * X11_TEST(some_test) {
 *  return 0;
 * }
 *
 * int main(void) {
 *  return x11_tests_run(void);
 * }
 *
 */

#pragma once

typedef int (* x11_test_func_t)(char* display);

struct test_function {
    const char *name;     /* function name */
    const char *file;     /* file name */
    x11_test_func_t func; /* test function */
} __attribute__((aligned(16)));

/**
 * Defines a struct test_function in a custom ELF section that we can then
 * loop over in x11_tests_run() to extract the tests. This removes the
 * need of manually adding the tests to a suite or listing them somewhere.
 */
#define X11_TEST(_func) \
static int _func(char* display); \
static const struct test_function _test_##_func \
__attribute__((used)) \
__attribute__((section("test_functions_section"))) = { \
    .name = #_func, \
    .func = _func, \
    .file = __FILE__, \
}; \
static int _func(char* display)

int xvfb_wrapper(int (*f)(char* display));

int x11_tests_run(void);
