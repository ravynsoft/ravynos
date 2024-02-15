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

#ifndef SUDO_PLUGIN_PYHELPERS_H
#define	SUDO_PLUGIN_PYHELPERS_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

/* Python may be built with 32-bit time_t support on some platforms. */
#undef SIZEOF_TIME_T

#include <config.h>
#include <sudo_compat.h>
#include <sudo_plugin.h>

#include "pyhelpers_cpychecker.h"

#include "sudo_python_debug.h"

enum SudoPluginFunctionReturnCode {
    SUDO_RC_OK = 1,
    SUDO_RC_ACCEPT = 1,
    SUDO_RC_REJECT = 0,
    SUDO_RC_ERROR = -1,
    SUDO_RC_USAGE_ERROR = -2,
};

#define INTERPRETER_MAX 32

struct PythonContext
{
    sudo_printf_t sudo_log;
    sudo_conv_t sudo_conv;
    PyThreadState *py_main_interpreter;
    size_t interpreter_count;
    PyThreadState *py_subinterpreters[INTERPRETER_MAX];
};

extern struct PythonContext py_ctx;

#define Py_TYPENAME(object) (object ? Py_TYPE(object)->tp_name : "NULL")

#define py_sudo_log(...) py_ctx.sudo_log(__VA_ARGS__)

int py_sudo_conv(int num_msgs, const struct sudo_conv_message msgs[],
                 struct sudo_conv_reply replies[], struct sudo_conv_callback *callback);

void py_log_last_error(const char *context_message);

char *py_create_string_rep(PyObject *py_object);

char *py_join_str_list(PyObject *py_str_list, const char *separator);

struct key_value_str_int
{
    const char *key;
    int value;
};

PyObject *py_dict_create_string_int(size_t count, struct key_value_str_int *key_values);

PyObject *py_from_passwd(const struct passwd *pwd);

PyObject *py_str_array_to_tuple_with_count(Py_ssize_t count, char * const strings[]);
PyObject *py_str_array_to_tuple(char * const strings[]);
char **py_str_array_from_tuple(PyObject *py_tuple);

CPYCHECKER_RETURNS_BORROWED_REF
PyObject *py_tuple_get(PyObject *py_tuple, Py_ssize_t index, PyTypeObject *expected_type);

PyObject *py_object_get_optional_attr(PyObject *py_object, const char *attr, PyObject *py_default);
long py_object_get_optional_attr_number(PyObject *py_object, const char *attr_name);
const char *py_object_get_optional_attr_string(PyObject *py_object, const char *attr_name);

void py_object_set_attr_number(PyObject *py_object, const char *attr_name, long number);
void py_object_set_attr_string(PyObject *py_object, const char *attr_name, const char *value);

PyObject *py_create_version(unsigned int version);

void py_debug_python_call(const char *class_name, const char *function_name,
                          PyObject *py_args, PyObject *py_kwargs,
                          unsigned int subsystem_id);
void py_debug_python_result(const char *class_name, const char *function_name,
                            PyObject *py_args, unsigned int subsystem_id);

void str_array_free(char ***array);

int py_get_current_execution_frame(char **file_name, long *line_number, char **function_name);

void py_ctx_reset(void);

#endif // SUDO_PLUGIN_PYHELPERS_H
