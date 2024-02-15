/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2019-2020 Robert Manner <robert.manner@oneidentity.com>
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

#include "pyhelpers.h"

#include <pwd.h>
#include <signal.h>
#include <pathnames.h>

static int
_sudo_printf_default(int msg_type, const char * restrict fmt, ...)
{
    FILE *fp = stdout;
    FILE *ttyfp = NULL;
    va_list ap;
    int len;

    if (ISSET(msg_type, SUDO_CONV_PREFER_TTY)) {
	/* Try writing to /dev/tty first. */
	ttyfp = fopen(_PATH_TTY, "w");
    }

    switch (msg_type & 0xff) {
    case SUDO_CONV_ERROR_MSG:
	fp = stderr;
	FALLTHROUGH;
    case SUDO_CONV_INFO_MSG:
	va_start(ap, fmt);
	len = vfprintf(ttyfp ? ttyfp : fp, fmt, ap);
	va_end(ap);
	break;
    default:
	len = -1;
	errno = EINVAL;
	break;
    }

    if (ttyfp != NULL)
	fclose(ttyfp);

    return len;
}


struct PythonContext py_ctx = {
    .sudo_log = &_sudo_printf_default,
};


char *
py_join_str_list(PyObject *py_str_list, const char *separator)
{
    debug_decl(py_join_str_list, PYTHON_DEBUG_INTERNAL);

    char *result = NULL;
    PyObject *py_separator = NULL;
    PyObject *py_str = NULL;

    py_separator = PyUnicode_FromString(separator);
    if (py_separator == NULL)
        goto cleanup;

    py_str = PyObject_CallMethod(py_separator, "join", "(O)", py_str_list);
    if (py_str == NULL) {
        goto cleanup;
    }

    const char *str = PyUnicode_AsUTF8(py_str);
    if (str != NULL) {
        result = strdup(str);
    }

cleanup:
    Py_XDECREF(py_str);
    Py_XDECREF(py_separator);

    debug_return_str(result);
}

static char *
py_create_traceback_string(PyObject *py_traceback)
{
    debug_decl(py_create_traceback_string, PYTHON_DEBUG_INTERNAL);
    if (py_traceback == NULL)
        debug_return_str(strdup(""));

    char* traceback = NULL;


    PyObject *py_traceback_module = PyImport_ImportModule("traceback");
    if (py_traceback_module == NULL) {
        PyErr_Clear(); // do not care, we just won't show backtrace
    } else {
        PyObject *py_traceback_str_list = PyObject_CallMethod(py_traceback_module, "format_tb", "(O)", py_traceback);

        if (py_traceback_str_list != NULL) {
            traceback = py_join_str_list(py_traceback_str_list, "");
            Py_DECREF(py_traceback_str_list);
        }

        Py_CLEAR(py_traceback_module);
    }

    debug_return_str(traceback ? traceback : strdup(""));
}

void
py_log_last_error(const char *context_message)
{
    debug_decl(py_log_last_error, PYTHON_DEBUG_INTERNAL);
    if (!PyErr_Occurred()) {
        py_sudo_log(SUDO_CONV_ERROR_MSG, "%s\n", context_message);
        debug_return;
    }

    PyObject *py_type = NULL, *py_message = NULL, *py_traceback = NULL;
    PyErr_Fetch(&py_type, &py_message, &py_traceback);

    char *message = py_message ? py_create_string_rep(py_message) : NULL;

    py_sudo_log(SUDO_CONV_ERROR_MSG, "%s%s%s\n",
                context_message ? context_message : "",
                context_message && *context_message ? ": " : "",
                message ? message : "(NULL)");
    free(message);

    if (py_traceback != NULL) {
        char *traceback = py_create_traceback_string(py_traceback);
        py_sudo_log(SUDO_CONV_INFO_MSG, "Traceback:\n%s\n", traceback);
        free(traceback);
    }

    Py_XDECREF(py_type);
    Py_XDECREF(py_message);
    Py_XDECREF(py_traceback);
    debug_return;
}

PyObject *
py_str_array_to_tuple_with_count(Py_ssize_t count, char * const strings[])
{
    debug_decl(py_str_array_to_tuple_with_count, PYTHON_DEBUG_INTERNAL);

    PyObject *py_argv = PyTuple_New(count);
    if (py_argv == NULL)
        debug_return_ptr(NULL);

    for (int i = 0; i < count; ++i) {
        PyObject *py_arg = PyUnicode_FromString(strings[i]);
        if (py_arg == NULL || PyTuple_SetItem(py_argv, i, py_arg) != 0) {
            Py_CLEAR(py_argv);
            break;
        }
    }

    debug_return_ptr(py_argv);
}

PyObject *
py_str_array_to_tuple(char * const strings[])
{
    debug_decl(py_str_array_to_tuple, PYTHON_DEBUG_INTERNAL);

    // find the item count ("strings" ends with NULL terminator):
    Py_ssize_t count = 0;
    if (strings != NULL) {
        while (strings[count] != NULL)
            ++count;
    }

    debug_return_ptr(py_str_array_to_tuple_with_count(count, strings));
}

char **
py_str_array_from_tuple(PyObject *py_tuple)
{
    debug_decl(py_str_array_from_tuple, PYTHON_DEBUG_INTERNAL);

    if (!PyTuple_Check(py_tuple)) {
        PyErr_Format(PyExc_ValueError, "%s: value error, argument should be a tuple but it is '%s'",
                     __func__, Py_TYPENAME(py_tuple));
        debug_return_ptr(NULL);
    }

    Py_ssize_t tuple_size = PyTuple_Size(py_tuple);

    // we need an extra 0 at the end
    char **result = calloc((size_t)tuple_size + 1, sizeof(char *));
    if (result == NULL) {
        debug_return_ptr(NULL);
    }

    for (int i = 0; i < tuple_size; ++i) {
        PyObject *py_value = PyTuple_GetItem(py_tuple, i);
        if (py_value == NULL) {
            str_array_free(&result);
            debug_return_ptr(NULL);
        }

        // Note that it can be an "int" or something else as well
        char *value = py_create_string_rep(py_value);
        if (value == NULL) {
            // conversion error is already set
            str_array_free(&result);
            debug_return_ptr(NULL);
        }
        result[i] = value;
    }

    debug_return_ptr(result);
}

PyObject *
py_tuple_get(PyObject *py_tuple, Py_ssize_t idx, PyTypeObject *expected_type)
{
    debug_decl(py_tuple_get, PYTHON_DEBUG_INTERNAL);

    PyObject *py_item = PyTuple_GetItem(py_tuple, idx);
    if (py_item == NULL) {
        debug_return_ptr(NULL);
    }

    if (!PyObject_TypeCheck(py_item, expected_type)) {
        PyErr_Format(PyExc_ValueError, "Value error: tuple element %d should "
                                       "be a '%s' (but it is '%s')",
                     idx, expected_type->tp_name, Py_TYPENAME(py_item));
        debug_return_ptr(NULL);
    }

    debug_return_ptr(py_item);
}

PyObject *
py_create_version(unsigned int version)
{
    debug_decl(py_create_version, PYTHON_DEBUG_INTERNAL);
    debug_return_ptr(PyUnicode_FromFormat("%d.%d", SUDO_API_VERSION_GET_MAJOR(version),
                                          SUDO_API_VERSION_GET_MINOR(version)));
}

PyObject *
py_from_passwd(const struct passwd *pwd)
{
    debug_decl(py_from_passwd, PYTHON_DEBUG_INTERNAL);

    if (pwd == NULL) {
        debug_return_ptr_pynone;
    }

    // Create a tuple similar and convertible to python "struct_passwd" of "pwd" module
    debug_return_ptr(
        Py_BuildValue("(zziizzz)", pwd->pw_name, pwd->pw_passwd,
                      pwd->pw_uid, pwd->pw_gid, pwd->pw_gecos,
                      pwd->pw_dir, pwd->pw_shell)
    );
}

char *
py_create_string_rep(PyObject *py_object)
{
    debug_decl(py_create_string_rep, PYTHON_DEBUG_INTERNAL);
    char *result = NULL;

    if (py_object == NULL)
        debug_return_ptr(NULL);

    PyObject *py_string = PyObject_Str(py_object);
    if (py_string != NULL) {
        const char *bytes = PyUnicode_AsUTF8(py_string);
        if (bytes != NULL) {
	    /*
	     * Convert from old format w/ numeric value to new without it.
	     * Old: (<DEBUG.ERROR: 2>, 'ERROR level debug message')
	     * New: (DEBUG.ERROR, 'ERROR level debug message')
	     */
	    if (bytes[0] == '(' && bytes[1] == '<') {
		const char *colon = strchr(bytes + 2, ':');
		if (colon != NULL && colon[1] == ' ') {
		    const char *cp = colon + 2;
		    while (isdigit((unsigned char)*cp))
			cp++;
		    if (cp[0] == '>' && (cp[1] == ',' || cp[1] == '\0')) {
			bytes += 2;
			if (asprintf(&result, "(%.*s%s", (int)(colon - bytes),
				bytes, cp + 1) == -1) {
			    result = NULL;
			    goto done;
			}
		    }
		}
	    }
	    if (result == NULL)
		result = strdup(bytes);
        }
    }

done:
    Py_XDECREF(py_string);
    debug_return_ptr(result);
}

static void
_py_debug_python_function(const char *class_name, const char *function_name, const char *message,
                          PyObject *py_args, PyObject *py_kwargs, unsigned int subsystem_id)
{
    debug_decl_vars(_py_debug_python_function, subsystem_id);

    if (sudo_debug_needed(SUDO_DEBUG_DIAG)) {
        char *args_str = NULL;
        char *kwargs_str = NULL;
        if (py_args != NULL) {
            /* Sort by key for consistent output on Python < 3.6 */
	    PyObject *py_args_sorted = NULL;
	    if (PyDict_Check(py_args)) {
		py_args_sorted = PyDict_Items(py_args);
		if (py_args_sorted != NULL) {
		    if (PyList_Sort(py_args_sorted) == 0) {
			py_args = py_args_sorted;
		    }
		}
	    }
            args_str = py_create_string_rep(py_args);
	    if (args_str != NULL && strncmp(args_str, "RC.", 3) == 0) {
		/* Strip leading RC. to match python 3.10 behavior. */
		memmove(args_str, args_str + 3, strlen(args_str + 3) + 1);
	    }
	    Py_XDECREF(py_args_sorted);
        }
        if (py_kwargs != NULL) {
            /* Sort by key for consistent output on Python < 3.6 */
            PyObject *py_kwargs_sorted = NULL;
	    if (PyDict_Check(py_kwargs)) {
		py_kwargs_sorted = PyDict_Items(py_kwargs);
		if (py_kwargs_sorted != NULL) {
		    if (PyList_Sort(py_kwargs_sorted) == 0) {
			py_kwargs = py_kwargs_sorted;
		    }
		}
	    }
            kwargs_str = py_create_string_rep(py_kwargs);
	    Py_XDECREF(py_kwargs_sorted);
        }

        sudo_debug_printf(SUDO_DEBUG_DIAG, "%s.%s %s: %s%s%s\n", class_name,
                          function_name, message, args_str ? args_str : "()",
                          kwargs_str ? " " : "", kwargs_str ? kwargs_str : "");
        free(args_str);
        free(kwargs_str);
    }
}

void
py_debug_python_call(const char *class_name, const char *function_name,
                     PyObject *py_args, PyObject *py_kwargs,
                     unsigned int subsystem_id)
{
    debug_decl_vars(py_debug_python_call, subsystem_id);

    if (subsystem_id == PYTHON_DEBUG_C_CALLS && sudo_debug_needed(SUDO_DEBUG_INFO)) {
        // at this level we also output the callee python script
        char *callee_func_name = NULL, *callee_file_name = NULL;
        long callee_line_number = -1;

        if (py_get_current_execution_frame(&callee_file_name, &callee_line_number, &callee_func_name) == SUDO_RC_OK) {
            sudo_debug_printf(SUDO_DEBUG_INFO, "%s @ %s:%ld calls C function:\n",
                              callee_func_name, callee_file_name, callee_line_number);
        }

        free(callee_func_name);
        free(callee_file_name);
    }

    _py_debug_python_function(class_name, function_name, "was called with arguments",
                              py_args, py_kwargs, subsystem_id);
}

void
py_debug_python_result(const char *class_name, const char *function_name,
                       PyObject *py_result, unsigned int subsystem_id)
{
    if (py_result == NULL) {
        debug_decl_vars(py_debug_python_result, subsystem_id);
        sudo_debug_printf(SUDO_CONV_ERROR_MSG, "%s.%s call failed\n",
                          class_name, function_name);
    } else {
        _py_debug_python_function(class_name, function_name, "returned result",
                                  py_result, NULL, subsystem_id);
    }
}

void
str_array_free(char ***array)
{
    debug_decl(str_array_free, PYTHON_DEBUG_INTERNAL);

    if (*array == NULL)
        debug_return;

    for (char **item_ptr = *array; *item_ptr != NULL; ++item_ptr)
        free(*item_ptr);

    free(*array);
    *array = NULL;

    debug_return;
}

int
py_get_current_execution_frame(char **file_name, long *line_number, char **function_name)
{
    *file_name = NULL;
    *line_number = (long)-1;
    *function_name = NULL;

    PyObject *py_err_type = NULL, *py_err_value = NULL, *py_err_traceback = NULL;
    PyErr_Fetch(&py_err_type, &py_err_value, &py_err_traceback);

    PyObject *py_frame = NULL, *py_f_code = NULL,
              *py_filename = NULL, *py_function_name = NULL;

    PyObject *py_getframe = PySys_GetObject("_getframe");
    if (py_getframe == NULL)
        goto cleanup;

    py_frame = PyObject_CallFunction(py_getframe, "i", 0);
    if (py_frame == NULL)
        goto cleanup;

    *line_number = py_object_get_optional_attr_number(py_frame, "f_lineno");

    py_f_code = py_object_get_optional_attr(py_frame, "f_code", NULL);
    if (py_f_code != NULL) {
        py_filename = py_object_get_optional_attr(py_f_code, "co_filename", NULL);
        if (py_filename != NULL)
            *file_name = strdup(PyUnicode_AsUTF8(py_filename));

        py_function_name = py_object_get_optional_attr(py_f_code, "co_name", NULL);
        if (py_function_name != NULL)
            *function_name = strdup(PyUnicode_AsUTF8(py_function_name));
    }

cleanup:
    Py_CLEAR(py_frame);
    Py_CLEAR(py_f_code);
    Py_CLEAR(py_filename);
    Py_CLEAR(py_function_name);

    // we hide every error happening inside this function
    PyErr_Restore(py_err_type, py_err_value, py_err_traceback);

    return (*file_name && *function_name && (*line_number >= 0)) ?
                SUDO_RC_OK : SUDO_RC_ERROR;
}

void
py_ctx_reset()
{
    memset(&py_ctx, 0, sizeof(py_ctx));
    py_ctx.sudo_log = &_sudo_printf_default;
}

int
py_sudo_conv(int num_msgs, const struct sudo_conv_message msgs[],
             struct sudo_conv_reply replies[], struct sudo_conv_callback *callback)
{
    /* Enable suspend during password entry. */
    struct sigaction sa, saved_sigtstp;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = SIG_DFL;
    (void) sigaction(SIGTSTP, &sa, &saved_sigtstp);

    int rc = SUDO_RC_ERROR;
    if (py_ctx.sudo_conv != NULL)
        rc = py_ctx.sudo_conv((int)num_msgs, msgs, replies, callback);

    /* Restore signal handlers and signal mask. */
    (void) sigaction(SIGTSTP, &saved_sigtstp, NULL);

    return rc;
}

PyObject *
py_object_get_optional_attr(PyObject *py_object, const char *attr, PyObject *py_default)
{
    if (PyObject_HasAttrString(py_object, attr)) {
        return PyObject_GetAttrString(py_object, attr);
    }
    Py_XINCREF(py_default);  // whatever we return will have its refcount incremented
    return py_default;
}

const char *
py_object_get_optional_attr_string(PyObject *py_object, const char *attr_name)
{
    PyObject *py_value = py_object_get_optional_attr(py_object, attr_name, NULL);
    if (py_value == NULL)
        return NULL;

    const char *value = PyUnicode_AsUTF8(py_value);
    Py_CLEAR(py_value); // Note, the object still has reference to the attribute
    return value;
}

long
py_object_get_optional_attr_number(PyObject *py_object, const char *attr_name)
{
    PyObject *py_value = py_object_get_optional_attr(py_object, attr_name, NULL);
    if (py_value == NULL)
        return -1;

    long value = PyLong_AsLong(py_value);
    Py_CLEAR(py_value);
    return value;
}

void
py_object_set_attr_number(PyObject *py_object, const char *attr_name, long number)
{
    PyObject *py_number = PyLong_FromLong(number);
    if (py_number == NULL)
        return;

    PyObject_SetAttrString(py_object, attr_name, py_number);
    Py_CLEAR(py_number);
}

void
py_object_set_attr_string(PyObject *py_object, const char *attr_name, const char *value)
{
    PyObject *py_value = PyUnicode_FromString(value);
    if (py_value == NULL)
        return;

    PyObject_SetAttrString(py_object, attr_name, py_value);
    Py_CLEAR(py_value);
}

PyObject *
py_dict_create_string_int(size_t count, struct key_value_str_int *key_values)
{
    debug_decl(py_dict_create_string_int, PYTHON_DEBUG_INTERNAL);

    PyObject *py_value = NULL;
    PyObject *py_dict = PyDict_New();
    if (py_dict == NULL)
        goto cleanup;

    for (size_t i = 0; i < count; ++i) {
        py_value = PyLong_FromLong(key_values[i].value);
        if (py_value == NULL)
            goto cleanup;

        if (PyDict_SetItemString(py_dict, key_values[i].key, py_value) < 0)
            goto cleanup;

        Py_CLEAR(py_value);
    }

cleanup:
    if (PyErr_Occurred()) {
        Py_CLEAR(py_dict);
    }
    Py_CLEAR(py_value);

    debug_return_ptr(py_dict);
}
