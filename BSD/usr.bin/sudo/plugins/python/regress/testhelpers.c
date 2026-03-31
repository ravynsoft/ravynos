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

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include "testhelpers.h"

struct TestData data;

/*
 * Starting with Python 3.11, backtraces may contain a line with
 * '^' characters to bring attention to the important part of the
 * line.
 */
static void
remove_underline(char *output)
{
    char *cp, *ep;

    // Remove lines that only consist of '^' and white space.
    cp = output;
    ep = output + strlen(output);
    for (;;) {
	size_t len = strspn(cp, "^ \t");
	if (len > 0 && cp[len] == '\n') {
	    /* Prune out lines that are "underlining". */
	    memmove(cp, cp + len + 1, (size_t)(ep - cp));
	    if (*cp == '\0')
		break;
	} else {
	    /* No match, move to the next line. */
	    cp = strchr(cp, '\n');
	    if (cp == NULL)
		break;
	    cp++;
	}
    }
}

static void
clean_output(char *output)
{
    // we replace some output which otherwise would be test run dependent
    str_replace_in_place(output, MAX_OUTPUT, data.tmp_dir, TEMP_PATH_TEMPLATE);

    if (data.tmp_dir2)
        str_replace_in_place(output, MAX_OUTPUT, data.tmp_dir2, TEMP_PATH_TEMPLATE "2");

    str_replace_in_place(output, MAX_OUTPUT, SRC_DIR, "SRC_DIR");

    remove_underline(output);
}

const char *
expected_path(const char *format, ...)
{
    static char expected_output_file[PATH_MAX];
    size_t dirlen = strlcpy(expected_output_file, TESTDATA_DIR, sizeof(expected_output_file));

    va_list args;
    va_start(args, format);
    vsnprintf(expected_output_file + dirlen, PATH_MAX - dirlen, format, args);
    va_end(args);

    return expected_output_file;
}

char **
create_str_array(size_t count, ...)
{
    va_list args;

    va_start(args, count);

    char **result = calloc(count, sizeof(char *));
    if (result != NULL) {
        for (size_t i = 0; i < count; ++i) {
            const char *str = va_arg(args, char *);
            if (str != NULL) {
                result[i] = strdup(str);
                if (result[i] == NULL) {
                    while (i > 0) {
                        free(result[--i]);
                    }
                    free(result);
                    result = NULL;
                    break;
                }
            }
        }
    }

    va_end(args);
    return result;
}

int
is_update(void)
{
    static int result = -1;
    if (result < 0) {
        const char *update = getenv("UPDATE_TESTDATA");
        result = (update && strcmp(update, "1") == 0) ? 1 : 0;
    }
    return result;
}

int
verify_content(char *actual_content, const char *reference_path)
{
    clean_output(actual_content);

    if (is_update()) {
        VERIFY_TRUE(fwriteall(reference_path, actual_content));
    } else {
        char expected_output[MAX_OUTPUT] = "";
        if (!freadall(reference_path, expected_output, sizeof(expected_output))) {
            printf("Error: Missing test data at '%s'\n", reference_path);
            return false;
        }
        VERIFY_STR(actual_content, expected_output);
    }

    return true;
}

int
verify_file(const char *actual_dir, const char *actual_file_name, const char *reference_path)
{
    char actual_path[PATH_MAX];
    snprintf(actual_path, sizeof(actual_path), "%s/%s", actual_dir, actual_file_name);

    char actual_str[MAX_OUTPUT];
    if (!freadall(actual_path, actual_str, sizeof(actual_str))) {
        printf("Expected that file '%s' gets created, but it was not\n", actual_path);
        return false;
    }

    int rc = verify_content(actual_str, reference_path);
    return rc;
}

int
fake_conversation(int num_msgs, const struct sudo_conv_message msgs[],
                  struct sudo_conv_reply replies[], struct sudo_conv_callback *callback)
{
    (void) callback;
    snprintf_append(data.conv_str, MAX_OUTPUT, "Question count: %d\n", num_msgs);
    for (int i = 0; i < num_msgs; ++i) {
        const struct sudo_conv_message *msg = &msgs[i];
        snprintf_append(data.conv_str, MAX_OUTPUT, "Question %d: <<%s>> (timeout: %d, msg_type=%d)\n",
                      i, msg->msg, msg->timeout, msg->msg_type);

        if (data.conv_replies[i] == NULL)
            return 1; // simulates user interruption (conversation error)

        replies[i].reply = strdup(data.conv_replies[i]);
        if (replies[i].reply == NULL)
            return 1; // memory allocation error
    }

    return 0; // simulate user answered just fine
}

int
fake_conversation_with_suspend(int num_msgs, const struct sudo_conv_message msgs[],
                               struct sudo_conv_reply replies[], struct sudo_conv_callback *callback)
{
    if (callback != NULL) {
        callback->on_suspend(SIGTSTP, callback->closure);
        callback->on_resume(SIGCONT, callback->closure);
    }

    return fake_conversation(num_msgs, msgs, replies, callback);
}

int
fake_printf(int msg_type, const char * restrict fmt, ...)
{
    int rc = -1;
    va_list args;
    va_start(args, fmt);

    char *output = NULL;
    switch(msg_type) {
    case SUDO_CONV_INFO_MSG:
        output = data.stdout_str;
        break;
    case SUDO_CONV_ERROR_MSG:
        output = data.stderr_str;
        break;
    default:
        break;
    }

    if (output)
        rc = vsnprintf_append(output, MAX_OUTPUT, fmt, args);

    va_end(args);
    return rc;
}

int
verify_log_lines(const char *reference_path)
{
    char stored_path[PATH_MAX];
    snprintf(stored_path, sizeof(stored_path), "%s/%s", data.tmp_dir, "debug.log");

    FILE *file = fopen(stored_path, "rb");
    if (file == NULL) {
        printf("Failed to open file '%s'\n", stored_path);
        return false;
    }

    char line[1024] = "";
    char stored_str[MAX_OUTPUT] = "";
    while (fgets(line, sizeof(line), file) != NULL) {
        char *line_data = strstr(line, "] "); // this skips the timestamp and pid at the beginning
        VERIFY_NOT_NULL(line_data); // malformed log line
        line_data += 2;

        char *line_end = strstr(line_data, " object at "); // this skips checking the pointer hex
        if (line_end) {
            snprintf(line_end, sizeof(line) - (size_t)(line_end - line),
		" object>\n");
	}

	if (strncmp(line_data, "handle @ /", sizeof("handle @ /") - 1) == 0) {
            char *start = line_data + sizeof("handle @ ") - 1;

            // normalize path to logging/__init__.py
            char *logging = strstr(start, "logging/");
            if (logging != NULL) {
                memmove(start, logging, strlen(logging) + 1);
            }

            // remove line number
            char *colon = strchr(start, ':');
            if (colon != NULL) {
                size_t len = strspn(colon + 1, "0123456789");
                if (len != 0)
                    memmove(colon, colon + len + 1, strlen(colon + len + 1) + 1);
            }
        } else if (strncmp(line_data, "LogHandler.emit was called ", 27) == 0) {
            // LogHandler.emit argument details vary based on python version
            line_data[26] = '\n';
            line_data[27] = '\0';
        } else {
	    // Python 3.11 uses 0 instead of the symbolic REJECT in backtraces
	    char *cp = strstr(line_data, ": REJECT");
	    if (cp != NULL) {
		// Convert ": REJECT" to ": 0" + rest of line
		memcpy(cp, ": 0", 3);
		memmove(cp + 3, cp + 8, strlen(cp + 8) + 1);
	    } else {
		// Python 3.12 may use <RC.REJECT: 0> instead of 0
		cp = strstr(line_data, "<RC.REJECT: 0>");
		if (cp != NULL) {
		    *cp = '0';
		    memmove(cp + 1, cp + 14, strlen(cp + 14) + 1);
		}
	    }

	}

        VERIFY_TRUE(strlcat(stored_str, line_data, sizeof(stored_str)) < sizeof(stored_str));  // we have enough space in buffer
    }

    clean_output(stored_str);

    VERIFY_TRUE(verify_content(stored_str, reference_path));
    return true;
}

int
verify_str_set(char **actual_set, char **expected_set, const char *actual_variable_name)
{
    VERIFY_NOT_NULL(actual_set);
    VERIFY_NOT_NULL(expected_set);

    int actual_len = str_array_count(actual_set);
    int expected_len = str_array_count(expected_set);

    int matches = false;
    if (actual_len == expected_len) {
        int actual_pos = 0;
        for (; actual_pos < actual_len; ++actual_pos) {
            char *actual_item = actual_set[actual_pos];

            int expected_pos = 0;
            for (; expected_pos < expected_len; ++expected_pos) {
                if (strcmp(actual_item, expected_set[expected_pos]) == 0)
                    break;
            }

            if (expected_pos == expected_len) {
                // matching item was not found
                break;
            }
        }

        matches = (actual_pos == actual_len);
    }

    if (!matches) {
        char actual_set_str[MAX_OUTPUT] = "";
        char expected_set_str[MAX_OUTPUT] = "";
        str_array_snprint(actual_set_str, MAX_OUTPUT, actual_set, actual_len);
        str_array_snprint(expected_set_str, MAX_OUTPUT, expected_set, expected_len);

        VERIFY_PRINT_MSG("%s", actual_variable_name, actual_set_str, "expected",
                         expected_set_str, "expected to contain the same elements as");
        return false;
    }

    return true;
}

int
mock_python_datetime_now(const char *plugin_name, const char *date_str)
{
    char *cmd = NULL;
    int len;
    len = asprintf(&cmd,
                   "import %s\n"                      // the plugin has its own submodule
                   "from datetime import datetime\n"  // store the real datetime
                   "import time\n"
                   "from unittest.mock import Mock\n"
                   "%s.datetime = Mock()\n"           // replace plugin's datetime
                   "%s.datetime.now = lambda: datetime.strptime('%s', '%%Y-%%m-%%dT%%H:%%M:%%S')\n",
             plugin_name, plugin_name, plugin_name, date_str);
    if (len == -1)
	return false;
    VERIFY_PTR_NE(cmd, NULL);
    VERIFY_INT(PyRun_SimpleString(cmd), 0);
    free(cmd);
    return true;
}
