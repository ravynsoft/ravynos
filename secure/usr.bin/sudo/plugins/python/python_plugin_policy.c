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

#include "python_plugin_common.h"


static struct PluginContext plugin_ctx;

extern struct policy_plugin python_policy;

#define PY_POLICY_PLUGIN_VERSION SUDO_API_MKVERSION(1, 0)

#define CALLBACK_PLUGINFUNC(func_name) python_policy.func_name
#define CALLBACK_CFUNC(func_name) python_plugin_policy_ ## func_name

// This also verifies compile time that the name matches the sudo plugin API.
#define CALLBACK_PYNAME(func_name) ((void)CALLBACK_PLUGINFUNC(func_name), #func_name)

#define MARK_CALLBACK_OPTIONAL(function_name) \
    do { \
        python_plugin_mark_callback_optional(&plugin_ctx, CALLBACK_PYNAME(function_name), \
            (void **)&CALLBACK_PLUGINFUNC(function_name)); \
    } while(0)


static int
python_plugin_policy_open(unsigned int version, sudo_conv_t conversation,
    sudo_printf_t sudo_printf, char * const settings[],
    char * const user_info[], char * const user_env[],
    char * const plugin_options[], const char **errstr)
{
    debug_decl(python_plugin_policy_open, PYTHON_DEBUG_CALLBACKS);

    if (version < SUDO_API_MKVERSION(1, 2)) {
        sudo_printf(SUDO_CONV_ERROR_MSG,
                    "Error: Python policy plugin requires at least plugin API version 1.2\n");
        debug_return_int(SUDO_RC_ERROR);
    }

    int rc = python_plugin_register_logging(conversation, sudo_printf, settings);
    if (rc != SUDO_RC_OK)
        debug_return_int(rc);

    rc = python_plugin_init(&plugin_ctx, plugin_options, version);
    if (rc != SUDO_RC_OK)
        debug_return_int(rc);

    rc = python_plugin_construct(&plugin_ctx, PY_POLICY_PLUGIN_VERSION, settings,
                                 user_info, user_env, plugin_options);
    CALLBACK_SET_ERROR(&plugin_ctx, errstr);
    if (rc != SUDO_RC_OK) {
        debug_return_int(rc);
    }

    // skip plugin callbacks which are not mandatory
    MARK_CALLBACK_OPTIONAL(list);
    MARK_CALLBACK_OPTIONAL(validate);
    MARK_CALLBACK_OPTIONAL(invalidate);
    MARK_CALLBACK_OPTIONAL(init_session);
    // check_policy, open and close are mandatory

    debug_return_int(rc);
}

static void
python_plugin_policy_close(int exit_status, int error)
{
    debug_decl(python_plugin_policy_close, PYTHON_DEBUG_CALLBACKS);
    python_plugin_close(&plugin_ctx, CALLBACK_PYNAME(close),
                        Py_BuildValue("(ii)", error == 0 ? exit_status : -1, error));
    debug_return;
}

static int 
python_plugin_policy_check(int argc, char * const argv[],
    char *env_add[], char **command_info_out[],
    char **argv_out[], char **user_env_out[], const char **errstr)
{
    debug_decl(python_plugin_policy_check, PYTHON_DEBUG_CALLBACKS);
    int rc = SUDO_RC_ERROR;

    PyThreadState_Swap(plugin_ctx.py_interpreter);

    *command_info_out = *argv_out = *user_env_out = NULL;

    PyObject *py_argv = py_str_array_to_tuple_with_count(argc, argv);

    PyObject *py_env_add = py_str_array_to_tuple(env_add);
    PyObject *py_result = NULL;

    if (py_argv == NULL || py_env_add == NULL) {
        sudo_debug_printf(SUDO_DEBUG_ERROR, "Failed to create some of the arguments for the python call "
                                           "(py_argv=%p py_env_add=%p)\n", (void *)py_argv, (void *)py_env_add);
        goto cleanup;
    }

    py_result = python_plugin_api_call(&plugin_ctx, CALLBACK_PYNAME(check_policy),
                                      Py_BuildValue("(OO)", py_argv, py_env_add));
    CALLBACK_SET_ERROR(&plugin_ctx, errstr);
    if (py_result == NULL)
        goto cleanup;

    PyObject *py_rc = NULL,
             *py_command_info_out = NULL,
             *py_argv_out = NULL,
             *py_user_env_out = NULL;
    if (PyTuple_Check(py_result))
    {
        if (!PyArg_ParseTuple(py_result, "O!|O!O!O!:python_plugin.check_policy",
                              &PyLong_Type, &py_rc,
                              &PyTuple_Type, &py_command_info_out,
                              &PyTuple_Type, &py_argv_out,
                              &PyTuple_Type, &py_user_env_out))
        {
            goto cleanup;
        }
    } else {
        py_rc = py_result;
    }

    if (py_command_info_out != NULL)
        *command_info_out = py_str_array_from_tuple(py_command_info_out);

    if (py_argv_out != NULL)
        *argv_out = py_str_array_from_tuple(py_argv_out);

    if (py_user_env_out != NULL)
        *user_env_out = py_str_array_from_tuple(py_user_env_out);

    rc = python_plugin_rc_to_int(py_rc);

cleanup:
    if (PyErr_Occurred()) {
        py_log_last_error(NULL);
        rc = SUDO_RC_ERROR;
        free(*command_info_out);
        free(*argv_out);
        free(*user_env_out);
        *command_info_out = *argv_out = *user_env_out = NULL;
    }

    Py_XDECREF(py_argv);
    Py_XDECREF(py_env_add);
    Py_XDECREF(py_result);

    if (rc == SUDO_RC_ACCEPT)
        plugin_ctx.call_close = 1;

    debug_return_int(rc);
}

static int
python_plugin_policy_list(int argc, char * const argv[], int verbose, const char *list_user, const char **errstr)
{
    debug_decl(python_plugin_policy_list, PYTHON_DEBUG_CALLBACKS);

    PyThreadState_Swap(plugin_ctx.py_interpreter);

    PyObject *py_argv = py_str_array_to_tuple_with_count(argc, argv);
    if (py_argv == NULL) {
        sudo_debug_printf(SUDO_DEBUG_ERROR, "%s: Failed to create argv argument for the python call\n", __func__);
        debug_return_int(SUDO_RC_ERROR);
    }

    int rc = python_plugin_api_rc_call(&plugin_ctx, CALLBACK_PYNAME(list),
        Py_BuildValue("(Oiz)", py_argv, verbose, list_user));

    Py_XDECREF(py_argv);

    CALLBACK_SET_ERROR(&plugin_ctx, errstr);
    debug_return_int(rc);
}

static int
python_plugin_policy_version(int verbose)
{
    debug_decl(python_plugin_policy_version, PYTHON_DEBUG_CALLBACKS);

    PyThreadState_Swap(plugin_ctx.py_interpreter);

    debug_return_int(python_plugin_show_version(&plugin_ctx, CALLBACK_PYNAME(show_version),
                                                verbose, PY_POLICY_PLUGIN_VERSION, "policy"));
}

static int
python_plugin_policy_validate(const char **errstr)
{
    debug_decl(python_plugin_policy_validate, PYTHON_DEBUG_CALLBACKS);
    PyThreadState_Swap(plugin_ctx.py_interpreter);
    int rc = python_plugin_api_rc_call(&plugin_ctx, CALLBACK_PYNAME(validate), NULL);
    CALLBACK_SET_ERROR(&plugin_ctx, errstr);
    debug_return_int(rc);
}

static void
python_plugin_policy_invalidate(int unlinkit)
{
    debug_decl(python_plugin_policy_invalidate, PYTHON_DEBUG_CALLBACKS);
    PyThreadState_Swap(plugin_ctx.py_interpreter);
    python_plugin_api_rc_call(&plugin_ctx, CALLBACK_PYNAME(invalidate),
                              Py_BuildValue("(i)", unlinkit));
    debug_return;
}

static int
python_plugin_policy_init_session(struct passwd *pwd, char **user_env[], const char **errstr)
{
    debug_decl(python_plugin_policy_init_session, PYTHON_DEBUG_CALLBACKS);
    int rc = SUDO_RC_ERROR;
    PyThreadState_Swap(plugin_ctx.py_interpreter);
    PyObject *py_pwd = NULL, *py_user_env = NULL, *py_result = NULL;

    py_pwd = py_from_passwd(pwd);
    if (py_pwd == NULL)
        goto cleanup;

    py_user_env = py_str_array_to_tuple(*user_env);
    if (py_user_env == NULL)
        goto cleanup;

    py_result = python_plugin_api_call(&plugin_ctx, CALLBACK_PYNAME(init_session),
        Py_BuildValue("(OO)", py_pwd, py_user_env));
    CALLBACK_SET_ERROR(&plugin_ctx, errstr);
    if (py_result == NULL)
        goto cleanup;

    PyObject *py_user_env_out = NULL, *py_rc = NULL;
    if (PyTuple_Check(py_result)) {
        if (!PyArg_ParseTuple(py_result, "O!|O!:python_plugin.init_session",
                         &PyLong_Type, &py_rc,
                         &PyTuple_Type, &py_user_env_out)) {
            goto cleanup;
        }
    } else {
        py_rc = py_result;
    }

    if (py_user_env_out != NULL) {
        str_array_free(user_env);
        *user_env = py_str_array_from_tuple(py_user_env_out);
        if (*user_env == NULL)
            goto cleanup;
    }

    rc = python_plugin_rc_to_int(py_rc);

cleanup:
    Py_XDECREF(py_pwd);
    Py_XDECREF(py_user_env);
    Py_XDECREF(py_result);

    debug_return_int(rc);
}

sudo_dso_public struct policy_plugin python_policy = {
    SUDO_POLICY_PLUGIN,
    SUDO_API_VERSION,
    CALLBACK_CFUNC(open),
    CALLBACK_CFUNC(close),
    CALLBACK_CFUNC(version),
    CALLBACK_CFUNC(check),
    CALLBACK_CFUNC(list),
    CALLBACK_CFUNC(validate),
    CALLBACK_CFUNC(invalidate),
    CALLBACK_CFUNC(init_session),
    NULL, /* register_hooks */
    NULL, /* deregister_hooks */
    NULL  /* event_alloc */
};
