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

#ifndef SUDO_PYTHON_MODULE_H
#define SUDO_PYTHON_MODULE_H

#include "pyhelpers.h"

extern PyObject *sudo_exc_SudoException;  // Base exception for the sudo module problems

// This is for the python plugins to report error messages for us
extern PyObject *sudo_exc_PluginException;  // base exception of the following:
extern PyObject *sudo_exc_PluginReject;  // a reject with message
extern PyObject *sudo_exc_PluginError;   // an error with message

extern PyTypeObject *sudo_type_Plugin;
extern PyTypeObject *sudo_type_ConvMessage;

PyObject *sudo_module_create_class(const char *class_name, PyMethodDef *class_methods,
                                   PyObject *base_class);

CPYCHECKER_NEGATIVE_RESULT_SETS_EXCEPTION
int sudo_module_register_conv_message(PyObject *py_module);

CPYCHECKER_NEGATIVE_RESULT_SETS_EXCEPTION
int sudo_module_ConvMessage_to_c(PyObject *py_conv_message, struct sudo_conv_message *conv_message);

CPYCHECKER_NEGATIVE_RESULT_SETS_EXCEPTION
int sudo_module_ConvMessages_to_c(PyObject *py_tuple, Py_ssize_t *num_msgs, struct sudo_conv_message **msgs);

CPYCHECKER_NEGATIVE_RESULT_SETS_EXCEPTION
int sudo_module_register_baseplugin(PyObject *py_module);

CPYCHECKER_NEGATIVE_RESULT_SETS_EXCEPTION
int sudo_module_set_default_loghandler(void);

PyObject *python_sudo_debug(PyObject *py_self, PyObject *py_args);

PyMODINIT_FUNC sudo_module_init(void);

#endif // SUDO_PYTHON_MODULE_H
