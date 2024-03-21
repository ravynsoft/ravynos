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

#define EXC_VAR(exception_name) sudo_exc_ ## exception_name
#define TYPE_VAR(type_name) &sudo_type_ ## type_name

// exceptions:
PyObject *sudo_exc_SudoException;
PyObject *sudo_exc_PluginException;
PyObject *sudo_exc_PluginError;
PyObject *sudo_exc_PluginReject;
static PyObject *sudo_exc_ConversationInterrupted;

// the methods exposed in the "sudo" python module
// "args" is a tuple (~= const list) containing all the unnamed arguments
// "kwargs" is a dict of the keyword arguments or NULL if there are none
static PyObject *python_sudo_log_info(PyObject *py_self, PyObject *py_args, PyObject *py_kwargs);
static PyObject *python_sudo_log_error(PyObject *py_self, PyObject *py_args, PyObject *py_kwargs);
static PyObject *python_sudo_conversation(PyObject *py_self, PyObject *py_args, PyObject *py_kwargs);
static PyObject *python_sudo_options_as_dict(PyObject *py_self, PyObject *py_args);
static PyObject *python_sudo_options_from_dict(PyObject *py_self, PyObject *py_args);

// Called on module teardown.
static void sudo_module_free(void *self);

static PyMethodDef sudo_methods[] = {
    {"debug",  (PyCFunction)python_sudo_debug, METH_VARARGS, "Debug messages which can be saved to file in sudo.conf."},
    {"log_info",  (PyCFunction)python_sudo_log_info, METH_VARARGS | METH_KEYWORDS, "Display informational messages."},
    {"log_error",  (PyCFunction)python_sudo_log_error, METH_VARARGS | METH_KEYWORDS, "Display error messages."},
    {"conv", (PyCFunction)python_sudo_conversation, METH_VARARGS | METH_KEYWORDS, "Interact with the user"},
    {"options_as_dict", python_sudo_options_as_dict, METH_VARARGS, "Convert a string tuple in key=value format to a dictionary."},
    {"options_from_dict", python_sudo_options_from_dict, METH_VARARGS, "Convert a dictionary to a tuple of strings in key=value format."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef sudo_module = {
    PyModuleDef_HEAD_INIT,
    "sudo",   /* name of module */
    NULL,     /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    sudo_methods,
    NULL, /* slots */
    NULL, /* traverse */
    NULL, /* clear */
    sudo_module_free
};

CPYCHECKER_NEGATIVE_RESULT_SETS_EXCEPTION
static int
_parse_log_function_args(PyObject *py_args, PyObject *py_kwargs, char **args_joined, const char ** end)
{
    debug_decl(python_sudo_log, PYTHON_DEBUG_INTERNAL);

    int rc = SUDO_RC_ERROR;
    PyObject *py_empty = NULL;

    const char *sep = NULL;
    py_empty = PyTuple_New(0);
    if (py_empty == NULL)
        goto cleanup;

    static const char *keywords[] = { "sep", "end", NULL };
    if (py_kwargs != NULL && !PyArg_ParseTupleAndKeywords(py_empty, py_kwargs, "|zz:sudo.log", (char **)keywords, &sep, end))
        goto cleanup;

    if (sep == NULL)
        sep = " ";

    if (*end == NULL)
        *end = "\n";

    // this is to mimic the behaviour of python "print" / "log"
    *args_joined = py_join_str_list(py_args, sep);
    if (!PyErr_Occurred())  // == (*args_joined != NULL), but cpychecker does not understand that
        rc = SUDO_RC_OK;

cleanup:
    Py_CLEAR(py_empty);
    debug_return_int(rc);
}

static PyObject *
python_sudo_log(int msg_type, PyObject *Py_UNUSED(py_self), PyObject *py_args, PyObject *py_kwargs)
{
    debug_decl(python_sudo_log, PYTHON_DEBUG_C_CALLS);
    py_debug_python_call("sudo", "log", py_args, py_kwargs, PYTHON_DEBUG_C_CALLS);

    int rc = SUDO_RC_ERROR;

    char *args_joined = NULL;
    const char *end = NULL;
    if (_parse_log_function_args(py_args, py_kwargs, &args_joined, &end) != SUDO_RC_OK)
        goto cleanup;

    rc = py_ctx.sudo_log(msg_type, "%s%s", args_joined, end);
    if (rc < 0) {
        PyErr_Format(sudo_exc_SudoException, "sudo.log: Error displaying message");
        goto cleanup;
    }

cleanup:
    free(args_joined);

    PyObject *py_result = PyErr_Occurred() ? NULL : PyLong_FromLong(rc);

    py_debug_python_result("sudo", "log", py_result, PYTHON_DEBUG_C_CALLS);
    debug_return_ptr(py_result);
}

static PyObject *
python_sudo_options_as_dict(PyObject *py_self, PyObject *py_args)
{
    (void) py_self;

    debug_decl(python_sudo_options_as_dict, PYTHON_DEBUG_C_CALLS);
    py_debug_python_call("sudo", "options_as_dict", py_args, NULL, PYTHON_DEBUG_C_CALLS);

    PyObject *py_config_tuple = NULL,
             *py_result = NULL,
             *py_config_tuple_iterator = NULL,
             *py_config = NULL,
             *py_splitted = NULL,
             *py_separator = NULL;

    if (!PyArg_ParseTuple(py_args, "O:sudo.options_as_dict", &py_config_tuple))
        goto cleanup;

    py_config_tuple_iterator = PyObject_GetIter(py_config_tuple);
    if (py_config_tuple_iterator == NULL)
        goto cleanup;

    py_result = PyDict_New();
    if (py_result == NULL)
        goto cleanup;

    py_separator = PyUnicode_FromString("=");
    if (py_separator == NULL)
        goto cleanup;

    while ((py_config = PyIter_Next(py_config_tuple_iterator)) != NULL) {
        py_splitted = PyUnicode_Split(py_config, py_separator, 1);
        if (py_splitted == NULL)
            goto cleanup;

        PyObject *py_key = PyList_GetItem(py_splitted, 0); // borrowed ref
        if (py_key == NULL)
            goto cleanup;

        PyObject *py_value = PyList_GetItem(py_splitted, 1);
        if (py_value == NULL) {  // skip values without a key
            Py_CLEAR(py_config);
            Py_CLEAR(py_splitted);
            PyErr_Clear();
            continue;
        }

        if (PyDict_SetItem(py_result, py_key, py_value) != 0) {
            goto cleanup;
        }

        Py_CLEAR(py_config);
        Py_CLEAR(py_splitted);
    }

cleanup:
    Py_CLEAR(py_config_tuple_iterator);
    Py_CLEAR(py_config);
    Py_CLEAR(py_splitted);
    Py_CLEAR(py_separator);

    if (PyErr_Occurred()) {
        Py_CLEAR(py_result);
    }

    py_debug_python_result("sudo", "options_as_dict", py_result, PYTHON_DEBUG_C_CALLS);
    debug_return_ptr(py_result);
}

static PyObject *
python_sudo_options_from_dict(PyObject *py_self, PyObject *py_args)
{
    (void) py_self;
    debug_decl(python_sudo_options_from_dict, PYTHON_DEBUG_C_CALLS);
    py_debug_python_call("sudo", "options_from_dict", py_args, NULL, PYTHON_DEBUG_C_CALLS);

    PyObject *py_config_dict = NULL,
             *py_result = NULL;

    if (!PyArg_ParseTuple(py_args, "O!:sudo.options_from_dict", &PyDict_Type, &py_config_dict))
        goto cleanup;

    Py_ssize_t dict_size = PyDict_Size(py_config_dict);
    py_result = PyTuple_New(dict_size);
    if (py_result == NULL)
        goto cleanup;

    PyObject *py_key = NULL, *py_value = NULL; // -> borrowed references
    Py_ssize_t i, pos = 0;
    for (i = 0; PyDict_Next(py_config_dict, &pos, &py_key, &py_value); i++) {
        PyObject *py_config = PyUnicode_FromFormat("%S%s%S", py_key, "=", py_value);
        if (py_config == NULL)
            goto cleanup;

        /* Dictionaries are sparse so we cannot use pos as an index. */
        if (PyTuple_SetItem(py_result, i, py_config) != 0) { // this steals a reference, even on error
            goto cleanup;
        }
    }

cleanup:
    if (PyErr_Occurred()) {
        Py_CLEAR(py_result);
    }

    py_debug_python_result("sudo", "options_from_dict", py_result, PYTHON_DEBUG_C_CALLS);
    debug_return_ptr(py_result);
}

static PyObject *
python_sudo_log_info(PyObject *py_self, PyObject *py_args, PyObject *py_kwargs)
{
    return python_sudo_log(SUDO_CONV_INFO_MSG, py_self, py_args, py_kwargs);
}

static PyObject *
python_sudo_log_error(PyObject *py_self, PyObject *py_args, PyObject *py_kwargs)
{
    return python_sudo_log(SUDO_CONV_ERROR_MSG, py_self, py_args, py_kwargs);
}

CPYCHECKER_NEGATIVE_RESULT_SETS_EXCEPTION
static int py_expect_arg_callable(PyObject *py_callable,
    const char *func_name, const char *arg_name)
{
    debug_decl(py_expect_arg_callable, PYTHON_DEBUG_INTERNAL);

    if (!PyCallable_Check(py_callable)) {
        PyErr_Format(PyExc_ValueError, "%s: %s argument must be python callable (got %s) ",
                     func_name, arg_name, Py_TYPENAME(py_callable));
        debug_return_int(-1);
    }

    debug_return_int(0);
}

struct py_conv_callback_closure
{
    PyObject *py_on_suspend;
    PyObject *py_on_resume;
};

static int
_call_conversation_callback(PyObject *py_callback, int signo)
{
    debug_decl(_call_conversation_callback, PYTHON_DEBUG_INTERNAL);

    if (py_callback == NULL || py_callback == Py_None)
        debug_return_int(0); // nothing to do

    PyObject *py_result = PyObject_CallFunction(py_callback, "(i)", signo);

    int rc = -1;

    // We treat sudo.RC_OK (1) and None (no exception occurred) as success as well to avoid confusion
    if (py_result && (py_result == Py_None || PyLong_AsLong(py_result) >= 0))
        rc = 0;

    Py_CLEAR(py_result);

    if (rc != 0)
        py_log_last_error("Error during conversation callback");

    debug_return_int(rc);
}

static int
python_sudo_conversation_suspend_cb(int signo, struct py_conv_callback_closure *closure)
{
    return _call_conversation_callback(closure->py_on_suspend, signo);
}

static int
python_sudo_conversation_resume_cb(int signo, struct py_conv_callback_closure *closure)
{
    return _call_conversation_callback(closure->py_on_resume, signo);
}

static PyObject *
python_sudo_conversation(PyObject *Py_UNUSED(self), PyObject *py_args, PyObject *py_kwargs)
{
    debug_decl(python_sudo_conversation, PYTHON_DEBUG_C_CALLS);
    py_debug_python_call("sudo", "conv", py_args, py_kwargs, PYTHON_DEBUG_C_CALLS);

    PyObject *py_result = NULL, *py_empty = NULL;
    Py_ssize_t num_msgs = 0;
    struct sudo_conv_message *msgs = NULL;
    struct sudo_conv_reply *replies = NULL;

    // Note, they are both borrowed references of py_kwargs
    struct py_conv_callback_closure callback_closure = { NULL, NULL };

    struct sudo_conv_callback callback = {
        SUDO_CONV_CALLBACK_VERSION,
        &callback_closure,
        (sudo_conv_callback_fn_t)python_sudo_conversation_suspend_cb,
        (sudo_conv_callback_fn_t)python_sudo_conversation_resume_cb
    };

    py_empty = PyTuple_New(0);
    if (py_empty == NULL)
        goto cleanup;

    static const char *keywords[] = { "on_suspend", "on_resume", NULL };
    if (py_kwargs != NULL && !PyArg_ParseTupleAndKeywords(py_empty, py_kwargs, "|OO:sudo.conv", (char **)keywords,
                                                  &callback_closure.py_on_suspend,
                                                  &callback_closure.py_on_resume))
        goto cleanup;

    if (callback_closure.py_on_suspend != NULL &&
        py_expect_arg_callable(callback_closure.py_on_suspend, "sudo.conv", "on_suspend") < 0) {
        goto cleanup;
    }

    if (callback_closure.py_on_resume != NULL &&
        py_expect_arg_callable(callback_closure.py_on_resume, "sudo.conv", "on_resume") < 0) {
        goto cleanup;
    }

    /* sudo_module_ConvMessages_to_c() returns error if no messages. */
    if (sudo_module_ConvMessages_to_c(py_args, &num_msgs, &msgs) < 0) {
        goto cleanup;
    }

    replies = calloc((size_t)num_msgs, sizeof(struct sudo_conv_reply));
    if (replies == NULL)
        goto cleanup;
    py_result = PyTuple_New(num_msgs);
    if (py_result == NULL)
        goto cleanup;

    if (py_ctx.sudo_conv == NULL) {
        PyErr_Format(sudo_exc_SudoException, "%s: conversation is unavailable",
                     __func__);
        goto cleanup;
    }

    int rc = py_sudo_conv((int)num_msgs, msgs, replies, &callback);
    if (rc != 0) {
        PyErr_Format(sudo_exc_ConversationInterrupted,
                     "%s: conversation was interrupted", __func__, rc);
        goto cleanup;
    }

    for (Py_ssize_t i = 0; i < num_msgs; ++i) {
        char *reply = replies[i].reply;
        if (reply != NULL) {
            PyObject *py_reply = PyUnicode_FromString(reply);
            if (py_reply == NULL) {
                goto cleanup;
            }

            if (PyTuple_SetItem(py_result, i, py_reply) != 0) {  // this steals a reference even on error
                PyErr_Format(sudo_exc_SudoException, "%s: failed to set tuple item", __func__);
                goto cleanup;
            }

            sudo_debug_printf(SUDO_DEBUG_DIAG, "user reply for conversation: '%s'\n", reply);
        }
    }

cleanup:
    Py_CLEAR(py_empty);
    if (replies != NULL) {
        for (int i = 0; i < num_msgs; ++i)
            free(replies[i].reply);
    }
    free(msgs);
    free(replies);

    if (PyErr_Occurred()) {
        Py_CLEAR(py_result);  // we return NULL
    }

    py_debug_python_result("sudo", "conv", py_result, PYTHON_DEBUG_C_CALLS);

    debug_return_ptr(py_result);
}

/*
 * Create a python class.
 * Class name must be a full name including module, eg. "sudo.MyFavouriteClass".
 * The resulting class object can be added to a module using PyModule_AddObject.
 */
PyObject *
sudo_module_create_class(const char *class_name, PyMethodDef *class_methods,
                         PyObject *base_class)
{
    debug_decl(sudo_module_create_class, PYTHON_DEBUG_INTERNAL);

    PyObject *py_base_classes = NULL, *py_class = NULL, *py_member_dict = NULL;

    if (base_class == NULL) {
        py_base_classes = PyTuple_New(0);
    } else {
        py_base_classes = Py_BuildValue("(O)", base_class);
    }

    if (py_base_classes == NULL)
        goto cleanup;

    py_member_dict = PyDict_New();
    if (py_member_dict == NULL)
        goto cleanup;

    for (PyMethodDef *py_def = class_methods; py_def->ml_name != NULL; ++py_def) {
        PyObject *py_func = PyCFunction_New(py_def, NULL);
        if (py_func == NULL) {
            goto cleanup;
        }

        // this wrapping makes the function get the 'self' as argument
        PyObject *py_method = PyInstanceMethod_New(py_func);
        if (py_method == NULL) {
            Py_DECREF(py_func);
            goto cleanup;
        }

        int rc = PyDict_SetItemString(py_member_dict, py_def->ml_name, py_method);

        Py_XDECREF(py_func);
        Py_XDECREF(py_method);

        if (rc != 0)
            goto cleanup;
    }

    py_class = PyObject_CallFunction((PyObject *)&PyType_Type, "(sOO)",
                                     class_name,
                                     py_base_classes,
                                     py_member_dict);

cleanup:
    Py_CLEAR(py_base_classes);
    Py_CLEAR(py_member_dict);

    debug_return_ptr(py_class);
}

CPYCHECKER_STEALS_REFERENCE_TO_ARG(3)
static void
sudo_module_register_enum(PyObject *py_module, const char *enum_name, PyObject *py_constants_dict)
{
    // pseudo code:
    // return enum.IntEnum('MyEnum', {'DEFINITION_NAME': DEFINITION_VALUE, ...})

    debug_decl(sudo_module_register_enum, PYTHON_DEBUG_INTERNAL);

    if (py_constants_dict == NULL)
        return;

    PyObject *py_enum_class = NULL;
    PyObject *py_enum_module = PyImport_ImportModule("enum");
    if (py_enum_module == NULL) {
	Py_CLEAR(py_constants_dict);
	debug_return;
    }

    py_enum_class = PyObject_CallMethod(py_enum_module,
					"IntEnum", "sO", enum_name,
					py_constants_dict);

    Py_CLEAR(py_constants_dict);
    Py_CLEAR(py_enum_module);

    if (py_enum_class == NULL) {
        debug_return;
    }

    // PyModule_AddObject steals the reference to py_enum_class on success
    if (PyModule_AddObject(py_module, enum_name, py_enum_class) < 0) {
        Py_CLEAR(py_enum_class);
    }

    debug_return;
}

PyMODINIT_FUNC
sudo_module_init(void)
{
    debug_decl(sudo_module_init, PYTHON_DEBUG_C_CALLS);

    PyObject *py_module = PyModule_Create(&sudo_module);

    if (py_module == NULL)
        debug_return_ptr(NULL);

    // Note: "PyModule_AddObject()" decrements the refcount only on success

    // exceptions
    #define MODULE_ADD_EXCEPTION(exception_name, base_exception) \
        do { \
            EXC_VAR(exception_name) = PyErr_NewException("sudo." # exception_name, base_exception, NULL); \
            if (EXC_VAR(exception_name) == NULL || PyModule_AddObject(py_module, # exception_name, EXC_VAR(exception_name)) < 0) { \
                Py_CLEAR(EXC_VAR(exception_name)); \
                goto cleanup; \
            } \
            Py_INCREF(EXC_VAR(exception_name)); \
        } while(0);

    MODULE_ADD_EXCEPTION(SudoException, NULL);

    MODULE_ADD_EXCEPTION(PluginException, NULL);
    MODULE_ADD_EXCEPTION(PluginError, EXC_VAR(PluginException));
    MODULE_ADD_EXCEPTION(PluginReject, EXC_VAR(PluginException));

    MODULE_ADD_EXCEPTION(ConversationInterrupted, EXC_VAR(SudoException));

    #define MODULE_REGISTER_ENUM(name, key_values) \
        sudo_module_register_enum(py_module, name, py_dict_create_string_int(\
            sizeof(key_values) / sizeof(struct key_value_str_int), key_values))

    // constants
    struct key_value_str_int constants_rc[] = {
        {"OK", SUDO_RC_OK},
        {"ACCEPT", SUDO_RC_ACCEPT},
        {"REJECT", SUDO_RC_REJECT},
        {"ERROR", SUDO_RC_ERROR},
        {"USAGE_ERROR", SUDO_RC_USAGE_ERROR}
    };
    MODULE_REGISTER_ENUM("RC", constants_rc);

    struct key_value_str_int constants_conv[] = {
        {"PROMPT_ECHO_OFF", SUDO_CONV_PROMPT_ECHO_OFF},
        {"PROMPT_ECHO_ON", SUDO_CONV_PROMPT_ECHO_ON},
        {"INFO_MSG", SUDO_CONV_INFO_MSG},
        {"PROMPT_MASK", SUDO_CONV_PROMPT_MASK},
        {"PROMPT_ECHO_OK", SUDO_CONV_PROMPT_ECHO_OK},
        {"PREFER_TTY", SUDO_CONV_PREFER_TTY}
    };
    MODULE_REGISTER_ENUM("CONV", constants_conv);

    struct key_value_str_int constants_debug[] = {
        {"CRIT", SUDO_DEBUG_CRIT},
        {"ERROR", SUDO_DEBUG_ERROR},
        {"WARN", SUDO_DEBUG_WARN},
        {"NOTICE", SUDO_DEBUG_NOTICE},
        {"DIAG", SUDO_DEBUG_DIAG},
        {"INFO", SUDO_DEBUG_INFO},
        {"TRACE", SUDO_DEBUG_TRACE},
        {"DEBUG", SUDO_DEBUG_DEBUG}
    };
    MODULE_REGISTER_ENUM("DEBUG", constants_debug);

    struct key_value_str_int constants_exit_reason[] = {
        {"NO_STATUS", SUDO_PLUGIN_NO_STATUS},
        {"WAIT_STATUS", SUDO_PLUGIN_WAIT_STATUS},
        {"EXEC_ERROR", SUDO_PLUGIN_EXEC_ERROR},
        {"SUDO_ERROR", SUDO_PLUGIN_SUDO_ERROR}
    };
    MODULE_REGISTER_ENUM("EXIT_REASON", constants_exit_reason);

    struct key_value_str_int constants_plugin_types[] = {
        {"POLICY", SUDO_POLICY_PLUGIN},
        {"AUDIT", SUDO_AUDIT_PLUGIN},
        {"IO", SUDO_IO_PLUGIN},
        {"APPROVAL", SUDO_APPROVAL_PLUGIN},
        {"SUDO", SUDO_FRONT_END}
    };
    MODULE_REGISTER_ENUM("PLUGIN_TYPE", constants_plugin_types);

    // classes
    if (sudo_module_register_conv_message(py_module) != SUDO_RC_OK)
        goto cleanup;

    if (sudo_module_register_baseplugin(py_module) != SUDO_RC_OK)
        goto cleanup;

cleanup:
    if (PyErr_Occurred()) {
        Py_CLEAR(py_module);
        Py_CLEAR(sudo_exc_SudoException);
        Py_CLEAR(sudo_exc_PluginError);
        Py_CLEAR(sudo_exc_PluginReject);
        Py_CLEAR(sudo_exc_ConversationInterrupted);
    }

    debug_return_ptr(py_module);
}

static void
sudo_module_free(void *self)
{
    debug_decl(sudo_module_free, PYTHON_DEBUG_C_CALLS);

    // Free exceptions
    Py_CLEAR(sudo_exc_SudoException);
    Py_CLEAR(sudo_exc_PluginError);
    Py_CLEAR(sudo_exc_PluginReject);
    Py_CLEAR(sudo_exc_ConversationInterrupted);

    // Free base plugin
    Py_CLEAR(sudo_type_Plugin);

    // Free conversation message type
    Py_CLEAR(sudo_type_ConvMessage);

    debug_return;
}
