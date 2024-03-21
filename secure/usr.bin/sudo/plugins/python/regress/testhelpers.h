/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2020 Robert Manner <robert.manner@oneidentity.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef PYTHON_TESTHELPERS
#define PYTHON_TESTHELPERS

#include "iohelpers.h"

#include "../pyhelpers.h"

#include <sudo_conf.h>

// just for the IDE
#ifndef SRC_DIR
#define SRC_DIR ""
#endif
#define TESTDATA_DIR SRC_DIR "/regress/testdata/"

#define TEMP_PATH_TEMPLATE "/tmp/sudo_check_python_exampleXXXXXX"

extern struct TestData {
    char *tmp_dir;
    char *tmp_dir2;
    char stdout_str[MAX_OUTPUT];
    char stderr_str[MAX_OUTPUT];

    char conv_str[MAX_OUTPUT];
    const char *conv_replies[8];

    // some example test data used by multiple test cases:
    char ** settings;
    char ** user_info;
    char ** command_info;
    char ** plugin_argv;
    int plugin_argc;
    char ** user_env;
    char ** plugin_options;
} data;

const char * expected_path(const char *format, ...);

char ** create_str_array(size_t count, ...);

#define RUN_TEST(testcase) \
    do { \
        int success = 1; \
        ntests++; \
        if (verbose) { \
            printf("Running test " #testcase " ... \n"); \
        } \
        if (!init()) { \
            printf("FAILED: initialization of testcase %s at %s:%d\n", #testcase, __FILE__, __LINE__); \
            success = 0; \
        } else \
        if (!testcase) { \
            printf("FAILED: testcase %s at %s:%d\n", #testcase, __FILE__, __LINE__); \
            success = 0; \
        } \
        if (!cleanup(success)) { \
            printf("FAILED: deinitialization of testcase %s at %s:%d\n", #testcase, __FILE__, __LINE__); \
            success = 0; \
        } \
        if (!success) { \
            errors++; \
        } \
    } while(false)

#define VERIFY_PRINT_MSG(fmt, actual_str, actual, expected_str, expected, expected_to_be_message) \
    printf("Expectation failed at %s:%d:\n  actual is <<" fmt ">>: %s\n  %s <<" fmt ">>: %s\n", \
           __FILE__, __LINE__, actual, actual_str, expected_to_be_message, expected, expected_str)

#define VERIFY_CUSTOM(fmt, type, actual, expected, invert) \
    do { \
        type actual_value = (type)(actual); \
        int failed = (actual_value != expected); \
        if (invert) \
            failed = !failed; \
        if (failed) { \
            VERIFY_PRINT_MSG(fmt, #actual, actual_value, #expected, expected, invert ? "not expected to be" : "expected to be"); \
            return false; \
        } \
    } while(false)

#define VERIFY_EQ(fmt, type, actual, expected) VERIFY_CUSTOM(fmt, type, actual, expected, false)
#define VERIFY_NE(fmt, type, actual, not_expected) VERIFY_CUSTOM(fmt, type, actual, not_expected, true)

#define VERIFY_INT(actual, expected) VERIFY_EQ("%d", int, actual, expected)

#define VERIFY_PTR(actual, expected) VERIFY_EQ("%p", const void *, (const void *)actual, (const void *)expected)
#define VERIFY_PTR_NE(actual, not_expected) VERIFY_NE("%p", const void *, (const void *)actual, (const void *)not_expected)

#define VERIFY_TRUE(actual) VERIFY_NE("%d", int, actual, 0)
#define VERIFY_FALSE(actual) VERIFY_INT(actual, false)

#define VERIFY_NOT_NULL(actual) VERIFY_NE("%p", const void *, actual, NULL)

#define VERIFY_STR(actual, expected) \
    do { \
        const char *actual_str = actual; \
        if (!actual_str || strcmp(actual_str, expected) != 0) { \
            VERIFY_PRINT_MSG("%s", #actual, actual_str ? actual_str : "(null)", #expected, expected, "expected to be"); \
            return false; \
        } \
    } while(false)

#define VERIFY_STR_CONTAINS(actual, expected) \
    do { \
        const char *actual_str = actual; \
        if (!actual_str || strstr(actual_str, expected) == NULL) { \
            VERIFY_PRINT_MSG("%s", #actual, actual_str ? actual_str : "(null)", #expected, expected, "expected to contain the string"); \
            return false; \
        } \
    } while(false)

int is_update(void);

int verify_content(char *actual_content, const char *reference_path);

#define VERIFY_CONTENT(actual_output, reference_path) \
    VERIFY_TRUE(verify_content(actual_output, reference_path))

#define VERIFY_STDOUT(reference_path) \
    VERIFY_CONTENT(data.stdout_str, reference_path)

#define VERIFY_STDERR(reference_path) \
    VERIFY_CONTENT(data.stderr_str, reference_path)

#define VERIFY_CONV(reference_name) \
    VERIFY_CONTENT(data.conv_str, reference_name)

int verify_file(const char *actual_dir, const char *actual_file_name, const char *reference_path);

#define VERIFY_FILE(actual_file_name, reference_path) \
    VERIFY_TRUE(verify_file(data.tmp_dir, actual_file_name, reference_path))

int fake_conversation(int num_msgs, const struct sudo_conv_message msgs[],
                      struct sudo_conv_reply replies[], struct sudo_conv_callback *callback);

int fake_conversation_with_suspend(int num_msgs, const struct sudo_conv_message msgs[],
                                   struct sudo_conv_reply replies[], struct sudo_conv_callback *callback);

int fake_printf(int msg_type, const char * restrict fmt, ...);

int verify_log_lines(const char *reference_path);

int mock_python_datetime_now(const char *plugin_name, const char *date_str);

#define VERIFY_LOG_LINES(reference_path) \
    VERIFY_TRUE(verify_log_lines(reference_path))

int verify_str_set(char **actual_set, char **expected_set, const char *actual_variable_name);

#define VERIFY_STR_SET(actual_set, ...) \
    do { \
        char **expected_set = create_str_array(__VA_ARGS__); \
        VERIFY_TRUE(verify_str_set(actual_set, expected_set, #actual_set)); \
        str_array_free(&expected_set); \
    } while(false)

#endif // PYTHON_TESTHELPERS
