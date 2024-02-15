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

#include "python_plugin_common.h"

struct AuditPluginContext
{
    struct PluginContext base_ctx;
    struct audit_plugin *plugin;
};

#define BASE_CTX(audit_ctx) (&(audit_ctx->base_ctx))

#define PY_AUDIT_PLUGIN_VERSION SUDO_API_MKVERSION(1, 0)

#define CALLBACK_PLUGINFUNC(func_name) audit_ctx->plugin->func_name

// This also verifies compile time that the name matches the sudo plugin API.
#define CALLBACK_PYNAME(func_name) ((void)CALLBACK_PLUGINFUNC(func_name), #func_name)

#define MARK_CALLBACK_OPTIONAL(function_name) \
    do { \
        python_plugin_mark_callback_optional(plugin_ctx, CALLBACK_PYNAME(function_name), \
            (void **)&CALLBACK_PLUGINFUNC(function_name)); \
    } while(0)

sudo_dso_public struct audit_plugin *python_audit_clone(void);

static int
_call_plugin_open(struct AuditPluginContext *audit_ctx, int submit_optind, char * const submit_argv[])
{
    debug_decl(_call_plugin_open, PYTHON_DEBUG_CALLBACKS);

    struct PluginContext *plugin_ctx = BASE_CTX(audit_ctx);
    if (!PyObject_HasAttrString(plugin_ctx->py_instance, CALLBACK_PYNAME(open))) {
        debug_return_int(SUDO_RC_OK);
    }

    int rc = SUDO_RC_ERROR;
    PyObject *py_submit_argv = py_str_array_to_tuple(submit_argv);

    if (py_submit_argv != NULL) {
        rc = python_plugin_api_rc_call(plugin_ctx, CALLBACK_PYNAME(open),
                                       Py_BuildValue("(iO)", submit_optind, py_submit_argv));
    }

    Py_XDECREF(py_submit_argv);
    debug_return_int(rc);
}

static int
python_plugin_audit_open(struct AuditPluginContext *audit_ctx,
        unsigned int version, sudo_conv_t conversation,
        sudo_printf_t sudo_printf, char * const settings[],
        char * const user_info[], int submit_optind,
        char * const submit_argv[], char * const submit_envp[],
        char * const plugin_options[], const char **errstr)
{
    debug_decl(python_plugin_audit_open, PYTHON_DEBUG_CALLBACKS);
    (void) version;

    int rc = python_plugin_register_logging(conversation, sudo_printf, settings);
    if (rc != SUDO_RC_OK) {
        debug_return_int(rc);
    }

    struct PluginContext *plugin_ctx = BASE_CTX(audit_ctx);

    rc = python_plugin_init(plugin_ctx, plugin_options, version);
    if (rc != SUDO_RC_OK) {
        debug_return_int(rc);
    }

    rc = python_plugin_construct(plugin_ctx, PY_AUDIT_PLUGIN_VERSION, settings,
                                 user_info, submit_envp, plugin_options);
    CALLBACK_SET_ERROR(plugin_ctx, errstr);
    if (rc != SUDO_RC_OK) {
        debug_return_int(rc);
    }

    // skip plugin callbacks which are not mandatory
    MARK_CALLBACK_OPTIONAL(accept);
    MARK_CALLBACK_OPTIONAL(reject);
    MARK_CALLBACK_OPTIONAL(error);

    plugin_ctx->call_close = 1;
    rc = _call_plugin_open(audit_ctx, submit_optind, submit_argv);
    CALLBACK_SET_ERROR(plugin_ctx, errstr);

    if (PyErr_Occurred()) {
        py_log_last_error("Error during calling audit open");
    }

    debug_return_int(rc);
}

static void
python_plugin_audit_close(struct AuditPluginContext *audit_ctx, int status_type, int status)
{
    debug_decl(python_plugin_audit_close, PYTHON_DEBUG_CALLBACKS);

    python_plugin_close(BASE_CTX(audit_ctx), CALLBACK_PYNAME(close),
                        Py_BuildValue("(ii)", status_type, status));

    debug_return;
}

static int
python_plugin_audit_accept(struct AuditPluginContext *audit_ctx,
    const char *plugin_name, unsigned int plugin_type,
    char * const command_info[], char * const run_argv[],
    char * const run_envp[], const char **errstr)
{
    debug_decl(python_plugin_audit_accept, PYTHON_DEBUG_CALLBACKS);

    struct PluginContext *plugin_ctx = BASE_CTX(audit_ctx);
    PyThreadState_Swap(plugin_ctx->py_interpreter);

    PyObject *py_command_info = NULL, *py_run_argv = NULL, *py_run_envp = NULL;
    int rc = SUDO_RC_ERROR;

    py_run_argv = py_str_array_to_tuple(run_argv);
    if (py_run_argv == NULL)
        goto cleanup;

    py_command_info = py_str_array_to_tuple(command_info);
    if (py_command_info == NULL)
        goto cleanup;

    py_run_envp = py_str_array_to_tuple(run_envp);
    if (py_run_envp == NULL)
        goto cleanup;

    PyObject *py_args = Py_BuildValue("(ziOOO)", plugin_name, plugin_type, py_command_info, py_run_argv, py_run_envp);
    rc = python_plugin_api_rc_call(plugin_ctx, CALLBACK_PYNAME(accept), py_args);
    CALLBACK_SET_ERROR(plugin_ctx, errstr);

cleanup:
    Py_CLEAR(py_command_info);
    Py_CLEAR(py_run_argv);
    Py_CLEAR(py_run_envp);

    debug_return_int(rc);
}

static int
python_plugin_audit_reject(struct AuditPluginContext *audit_ctx,
    const char *plugin_name, unsigned int plugin_type,
    const char *audit_msg, char * const command_info[], const char **errstr)
{
    debug_decl(python_plugin_audit_reject, PYTHON_DEBUG_CALLBACKS);

    struct PluginContext *plugin_ctx = BASE_CTX(audit_ctx);
    PyThreadState_Swap(plugin_ctx->py_interpreter);

    PyObject *py_command_info = NULL;
    int rc = SUDO_RC_ERROR;

    py_command_info = py_str_array_to_tuple(command_info);
    if (PyErr_Occurred())
        goto cleanup;

    PyObject *py_args = Py_BuildValue("(zizO)", plugin_name, plugin_type, audit_msg, py_command_info);
    rc = python_plugin_api_rc_call(plugin_ctx, CALLBACK_PYNAME(reject), py_args);

    CALLBACK_SET_ERROR(plugin_ctx, errstr);

cleanup:
    Py_CLEAR(py_command_info);
    if (PyErr_Occurred())
        py_log_last_error("Error during calling audit reject");

    debug_return_int(rc);
}

static int
python_plugin_audit_error(struct AuditPluginContext *audit_ctx,
    const char *plugin_name, unsigned int plugin_type,
    const char *audit_msg, char * const command_info[], const char **errstr)
{
    debug_decl(python_plugin_audit_error, PYTHON_DEBUG_CALLBACKS);

    struct PluginContext *plugin_ctx = BASE_CTX(audit_ctx);
    PyThreadState_Swap(plugin_ctx->py_interpreter);

    PyObject *py_command_info = NULL;
    int rc = SUDO_RC_ERROR;

    py_command_info = py_str_array_to_tuple(command_info);
    if (PyErr_Occurred())
        goto cleanup;

    PyObject *py_args = Py_BuildValue("(zizO)", plugin_name, plugin_type, audit_msg, py_command_info);
    rc = python_plugin_api_rc_call(plugin_ctx, CALLBACK_PYNAME(error), py_args);
    CALLBACK_SET_ERROR(plugin_ctx, errstr);

cleanup:
    Py_CLEAR(py_command_info);

    debug_return_int(rc);
}

static int
python_plugin_audit_show_version(struct AuditPluginContext *audit_ctx, int verbose)
{
    debug_decl(python_plugin_audit_show_version, PYTHON_DEBUG_CALLBACKS);

    struct PluginContext *plugin_ctx = BASE_CTX(audit_ctx);
    PyThreadState_Swap(plugin_ctx->py_interpreter);

    debug_return_int(python_plugin_show_version(plugin_ctx,
        CALLBACK_PYNAME(show_version), verbose, PY_AUDIT_PLUGIN_VERSION, "audit"));
}

sudo_dso_public struct audit_plugin python_audit;

// generate symbols for loading multiple audit plugins:
#define AUDIT_SYMBOL_NAME(symbol) symbol
#include "python_plugin_audit_multi.inc"
#define AUDIT_SYMBOL_NAME(symbol) symbol##1
#include "python_plugin_audit_multi.inc"
#define AUDIT_SYMBOL_NAME(symbol) symbol##2
#include "python_plugin_audit_multi.inc"
#define AUDIT_SYMBOL_NAME(symbol) symbol##3
#include "python_plugin_audit_multi.inc"
#define AUDIT_SYMBOL_NAME(symbol) symbol##4
#include "python_plugin_audit_multi.inc"
#define AUDIT_SYMBOL_NAME(symbol) symbol##5
#include "python_plugin_audit_multi.inc"
#define AUDIT_SYMBOL_NAME(symbol) symbol##6
#include "python_plugin_audit_multi.inc"
#define AUDIT_SYMBOL_NAME(symbol) symbol##7
#include "python_plugin_audit_multi.inc"

static struct audit_plugin *extra_audit_plugins[] = {
    &python_audit1,
    &python_audit2,
    &python_audit3,
    &python_audit4,
    &python_audit5,
    &python_audit6,
    &python_audit7
};

struct audit_plugin *
python_audit_clone(void)
{
    static size_t counter = 0;
    struct audit_plugin *next_plugin = NULL;

    size_t max = sizeof(extra_audit_plugins) / sizeof(*extra_audit_plugins);
    if (counter < max) {
        next_plugin = extra_audit_plugins[counter];
        ++counter;
    } else if (counter == max) {
        ++counter;
        py_sudo_log(SUDO_CONV_ERROR_MSG, "sudo: loading more than %d sudo python audit plugins is not supported\n", counter);
    }

    return next_plugin;
}
