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

#include "sudo_python_module.h"

PyTypeObject *sudo_type_ConvMessage;

static PyObject *
_sudo_ConvMessage__Init(PyObject *py_self, PyObject *py_args, PyObject *py_kwargs)
{
    debug_decl(_sudo_ConvMessage__Init, PYTHON_DEBUG_C_CALLS);

    py_debug_python_call("ConvMessage", "__init__", py_args, py_kwargs, PYTHON_DEBUG_C_CALLS);

    PyObject *py_empty = PyTuple_New(0);

    struct sudo_conv_message conv_message = { 0, 0, NULL };

    static const char *keywords[] = { "self", "msg_type", "msg", "timeout", NULL };
    if (!PyArg_ParseTupleAndKeywords(py_args ? py_args : py_empty, py_kwargs, "Ois|i:sudo.ConvMessage", (char **)keywords,
                                     &py_self, &(conv_message.msg_type), &(conv_message.msg),
                                     &(conv_message.timeout)))
        goto cleanup;

    sudo_debug_printf(SUDO_DEBUG_TRACE, "Parsed arguments: self='%p' msg_type='%d' timeout='%d' msg='%s'",
                      (void *)py_self, conv_message.msg_type, conv_message.timeout, conv_message.msg);

    py_object_set_attr_number(py_self, "msg_type", conv_message.msg_type);
    if (PyErr_Occurred())
        goto cleanup;

    py_object_set_attr_number(py_self, "timeout", conv_message.timeout);
    if (PyErr_Occurred()) // -V547
        goto cleanup;

    py_object_set_attr_string(py_self, "msg", conv_message.msg);
    if (PyErr_Occurred()) // -V547
        goto cleanup;

cleanup:
    Py_CLEAR(py_empty);

    if (PyErr_Occurred())
        debug_return_ptr(NULL);

    debug_return_ptr_pynone;
}


static PyMethodDef _sudo_ConvMessage_class_methods[] =
{
    {"__init__", (PyCFunction)_sudo_ConvMessage__Init,
                 METH_VARARGS | METH_KEYWORDS,
                 "Conversation message (same as C type sudo_conv_message)"},
    {NULL, NULL, 0, NULL}
};


int
sudo_module_register_conv_message(PyObject *py_module)
{
    debug_decl(sudo_module_register_conv_message, PYTHON_DEBUG_INTERNAL);
    int rc = SUDO_RC_ERROR;
    PyObject *py_class = NULL;

    py_class = sudo_module_create_class("sudo.ConvMessage", _sudo_ConvMessage_class_methods, NULL);
    if (py_class == NULL)
        goto cleanup;

    if (PyModule_AddObject(py_module, "ConvMessage", py_class) < 0) {
        goto cleanup;
    }

    // PyModule_AddObject steals the reference to py_class on success
    Py_INCREF(py_class);
    rc = SUDO_RC_OK;

    Py_CLEAR(sudo_type_ConvMessage);
    sudo_type_ConvMessage = (PyTypeObject *)py_class;
    Py_INCREF(sudo_type_ConvMessage);

cleanup:
    Py_CLEAR(py_class);
    debug_return_int(rc);
}

int
sudo_module_ConvMessage_to_c(PyObject *py_conv_message, struct sudo_conv_message *conv_message)
{
    debug_decl(sudo_module_ConvMessage_to_c, PYTHON_DEBUG_C_CALLS);

    conv_message->msg_type = (int)py_object_get_optional_attr_number(py_conv_message, "msg_type");
    if (PyErr_Occurred())
        debug_return_int(SUDO_RC_ERROR);

    conv_message->timeout = (int)py_object_get_optional_attr_number(py_conv_message, "timeout");
    if (PyErr_Occurred()) // -V547
        debug_return_int(SUDO_RC_ERROR);

    conv_message->msg = py_object_get_optional_attr_string(py_conv_message, "msg");
    if (PyErr_Occurred()) // -V547
        debug_return_int(SUDO_RC_ERROR);

    debug_return_int(SUDO_RC_OK);
}

int
sudo_module_ConvMessages_to_c(PyObject *py_tuple, Py_ssize_t *num_msgs, struct sudo_conv_message **msgs)
{
    debug_decl(sudo_module_ConvMessages_to_c, PYTHON_DEBUG_C_CALLS);

    *num_msgs = PyTuple_Size(py_tuple);
    *msgs = NULL;

    if (*num_msgs <= 0) {
        *num_msgs = 0;
        PyErr_Format(sudo_exc_SudoException, "Expected at least one ConvMessage");
        debug_return_int(SUDO_RC_ERROR);
    }

    *msgs = calloc((size_t)*num_msgs, sizeof(struct sudo_conv_message));
    if (*msgs == NULL) {
        debug_return_int(SUDO_RC_ERROR);
    }

    for (Py_ssize_t i = 0; i < *num_msgs; ++i) {
        PyObject *py_msg = py_tuple_get(py_tuple, i, sudo_type_ConvMessage);
        if (py_msg == NULL || sudo_module_ConvMessage_to_c(py_msg, &(*msgs)[i]) < 0) {
            debug_return_int(SUDO_RC_ERROR);
        }
    }

    debug_return_int(SUDO_RC_OK);
}

