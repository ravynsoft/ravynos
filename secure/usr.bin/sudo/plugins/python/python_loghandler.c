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

#include "sudo_python_module.h"

#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION < 9
# define PyObject_CallNoArgs(_o)	PyObject_CallObject((_o), NULL)
#endif

static void
_debug_plugin(unsigned int log_level, const char *log_message)
{
    debug_decl_vars(python_sudo_debug, PYTHON_DEBUG_PLUGIN);

    if (sudo_debug_needed(SUDO_DEBUG_INFO)) {
        // at trace level we output the position for the python log as well
        char *func_name = NULL, *file_name = NULL;
        long line_number = -1;

        if (py_get_current_execution_frame(&file_name, &line_number, &func_name) == SUDO_RC_OK) {
            sudo_debug_printf(SUDO_DEBUG_INFO, "%s @ %s:%ld debugs:\n",
                              func_name, file_name, line_number);
        }

        free(func_name);
        free(file_name);
    }

    sudo_debug_printf(log_level, "%s\n", log_message);
}

PyObject *
python_sudo_debug(PyObject *Py_UNUSED(py_self), PyObject *py_args)
{
    debug_decl(python_sudo_debug, PYTHON_DEBUG_C_CALLS);
    py_debug_python_call("sudo", "debug", py_args, NULL, PYTHON_DEBUG_C_CALLS);

    unsigned int log_level = SUDO_DEBUG_DEBUG;
    const char *log_message = NULL;
    if (!PyArg_ParseTuple(py_args, "is:sudo.debug", &log_level, &log_message)) {
        debug_return_ptr(NULL);
    }

    _debug_plugin(log_level, log_message);

    debug_return_ptr_pynone;
}

static unsigned int
_sudo_log_level_from_python(long level)
{
    if (level >= 50)
        return SUDO_DEBUG_CRIT;
    if (level >= 40)
        return SUDO_DEBUG_ERROR;
    if (level >= 30)
        return SUDO_DEBUG_WARN;
    if (level >= 20)
        return SUDO_DEBUG_INFO;

    return SUDO_DEBUG_TRACE;
}

static PyObject *
_sudo_LogHandler__emit(PyObject *py_self, PyObject *py_args)
{
    debug_decl(_sudo_LogHandler__emit, PYTHON_DEBUG_C_CALLS);

    PyObject *py_record = NULL; // borrowed
    PyObject *py_message = NULL;

    py_debug_python_call("LogHandler", "emit", py_args, NULL, PYTHON_DEBUG_C_CALLS);

    if (!PyArg_UnpackTuple(py_args, "sudo.LogHandler.emit", 2, 2, &py_self, &py_record))
        goto cleanup;

    long python_loglevel = py_object_get_optional_attr_number(py_record, "levelno");
    if (PyErr_Occurred()) {
        PyErr_Format(sudo_exc_SudoException, "sudo.LogHandler: Failed to determine log level");
        goto cleanup;
    }

    unsigned int sudo_loglevel = _sudo_log_level_from_python(python_loglevel);

    py_message = PyObject_CallMethod(py_self, "format", "O", py_record);
    if (py_message == NULL)
        goto cleanup;

    _debug_plugin(sudo_loglevel, PyUnicode_AsUTF8(py_message));

cleanup:
    Py_CLEAR(py_message);
    if (PyErr_Occurred()) {
        debug_return_ptr(NULL);
    }

    debug_return_ptr_pynone;
}

/* The sudo.LogHandler class can be used to make the default python logger
 * use sudo's built in log system. */
static PyMethodDef _sudo_LogHandler_class_methods[] =
{
    {"emit", _sudo_LogHandler__emit, METH_VARARGS, ""},
    {NULL, NULL, 0, NULL}
};

// This function creates the sudo.LogHandler class and adds it
// to the root logger.
int
sudo_module_set_default_loghandler()
{
    debug_decl(sudo_module_set_default_loghandler, PYTHON_DEBUG_INTERNAL);

    PyObject *py_sudo, *py_logging_module = NULL, *py_logger = NULL,
	     *py_streamhandler = NULL, *py_class = NULL,
	     *py_loghandler = NULL, *py_result = NULL;

    py_sudo = PyImport_ImportModule("sudo");
    if (py_sudo == NULL)
        goto cleanup;

    py_logging_module = PyImport_ImportModule("logging");
    if (py_logging_module == NULL)
        goto cleanup;

    // Get the root logger which all loggers descend from.
    py_logger = PyObject_CallMethod(py_logging_module, "getLogger", NULL);
    if (py_logger == NULL)
        goto cleanup;

    py_streamhandler = PyObject_GetAttrString(py_logging_module, "StreamHandler");
    if (py_streamhandler == NULL)
        goto cleanup;

    // Create our own handler that is a sub-class of StreamHandler
    py_class = sudo_module_create_class("sudo.LogHandler",
        _sudo_LogHandler_class_methods, py_streamhandler);
    if (py_class == NULL)
        goto cleanup;

    // PyModule_AddObject steals a reference to py_class on success
    if (PyModule_AddObject(py_sudo, "LogHandler", py_class) < 0)
        goto cleanup;
    Py_INCREF(py_class);

    py_loghandler = PyObject_CallNoArgs(py_class);
    if (py_loghandler == NULL)
        goto cleanup;

    py_result = PyObject_CallMethod(py_logger, "addHandler", "O", py_loghandler);

cleanup:
    Py_CLEAR(py_result);
    Py_CLEAR(py_loghandler);
    Py_CLEAR(py_class);
    Py_CLEAR(py_streamhandler);
    Py_CLEAR(py_logger);
    Py_CLEAR(py_logging_module);
    Py_CLEAR(py_sudo);
    debug_return_int(PyErr_Occurred() ? SUDO_RC_ERROR : SUDO_RC_OK);
}
