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

struct ApprovalPluginContext
{
    struct PluginContext base_ctx;
    struct approval_plugin *plugin;
};

#define BASE_CTX(approval_ctx) (&(approval_ctx->base_ctx))

#define PY_APPROVAL_PLUGIN_VERSION SUDO_API_MKVERSION(1, 0)

#define CALLBACK_PLUGINFUNC(func_name) approval_ctx->plugin->func_name

// This also verifies compile time that the name matches the sudo plugin API.
#define CALLBACK_PYNAME(func_name) ((void)CALLBACK_PLUGINFUNC(func_name), #func_name)

sudo_dso_public struct approval_plugin *python_approval_clone(void);

static int
python_plugin_approval_open(struct ApprovalPluginContext *approval_ctx,
    unsigned int version, sudo_conv_t conversation, sudo_printf_t sudo_printf,
    char * const settings[], char * const user_info[], int submit_optind,
    char * const submit_argv[], char * const submit_envp[],
    char * const plugin_options[], const char **errstr)
{
    debug_decl(python_plugin_approval_open, PYTHON_DEBUG_CALLBACKS);
    (void) version;

    int rc = python_plugin_register_logging(conversation, sudo_printf, settings);
    if (rc != SUDO_RC_OK) {
        debug_return_int(rc);
    }

    struct PluginContext *plugin_ctx = BASE_CTX(approval_ctx);

    rc = python_plugin_init(plugin_ctx, plugin_options, version);
    if (rc != SUDO_RC_OK) {
        debug_return_int(rc);
    }

    PyObject *py_kwargs = NULL, *py_submit_optind = NULL,
             *py_submit_argv = NULL;

    if ((py_kwargs = python_plugin_construct_args(version, settings, user_info,
                                                  submit_envp, plugin_options)) == NULL ||
        (py_submit_optind = PyLong_FromLong(submit_optind)) == NULL ||
        (py_submit_argv = py_str_array_to_tuple(submit_argv)) == NULL)
    {
        py_log_last_error("Failed to construct plugin instance");
        rc = SUDO_RC_ERROR;
    } else {
        PyDict_SetItemString(py_kwargs, "submit_optind", py_submit_optind);
        PyDict_SetItemString(py_kwargs, "submit_argv", py_submit_argv);

        rc = python_plugin_construct_custom(plugin_ctx, py_kwargs);
        CALLBACK_SET_ERROR(plugin_ctx, errstr);
    }

    Py_CLEAR(py_kwargs);
    Py_CLEAR(py_submit_argv);
    Py_CLEAR(py_submit_optind);

    if (rc != SUDO_RC_OK) {
        debug_return_int(rc);
    }

    debug_return_int(rc);
}

static void
python_plugin_approval_close(struct ApprovalPluginContext *approval_ctx)
{
    debug_decl(python_plugin_approval_close, PYTHON_DEBUG_CALLBACKS);

    struct PluginContext *plugin_ctx = BASE_CTX(approval_ctx);
    PyThreadState_Swap(plugin_ctx->py_interpreter);
    python_plugin_deinit(plugin_ctx);

    debug_return;
}

static int
python_plugin_approval_check(struct ApprovalPluginContext *approval_ctx,
                             char * const command_info[], char * const run_argv[],
                             char * const run_envp[], const char **errstr)
{
    debug_decl(python_plugin_approval_check, PYTHON_DEBUG_CALLBACKS);

    struct PluginContext *plugin_ctx = BASE_CTX(approval_ctx);

    PyObject *py_command_info = NULL, *py_run_argv = NULL, *py_run_envp = NULL,
        *py_args = NULL;

    int rc = SUDO_RC_ERROR;
    if ((py_command_info = py_str_array_to_tuple(command_info)) != NULL &&
        (py_run_argv = py_str_array_to_tuple(run_argv)) != NULL &&
        (py_run_envp = py_str_array_to_tuple(run_envp)) != NULL)
    {
        py_args = Py_BuildValue("(OOO)", py_command_info, py_run_argv, py_run_envp);
    }

    // Note, py_args gets cleared by api_rc_call
    rc = python_plugin_api_rc_call(plugin_ctx, CALLBACK_PYNAME(check), py_args);
    CALLBACK_SET_ERROR(plugin_ctx, errstr);

    Py_CLEAR(py_command_info);
    Py_CLEAR(py_run_argv);
    Py_CLEAR(py_run_envp);

    debug_return_int(rc);
}

static int
python_plugin_approval_show_version(struct ApprovalPluginContext *approval_ctx, int verbose)
{
    debug_decl(python_plugin_approval_show_version, PYTHON_DEBUG_CALLBACKS);

    struct PluginContext *plugin_ctx = BASE_CTX(approval_ctx);
    PyThreadState_Swap(plugin_ctx->py_interpreter);

    debug_return_int(python_plugin_show_version(plugin_ctx,
        CALLBACK_PYNAME(show_version), verbose, PY_APPROVAL_PLUGIN_VERSION, "approval"));
}

sudo_dso_public struct approval_plugin python_approval;

// generate symbols for loading multiple approval plugins:
#define APPROVAL_SYMBOL_NAME(symbol) symbol
#include "python_plugin_approval_multi.inc"
#define APPROVAL_SYMBOL_NAME(symbol) symbol##1
#include "python_plugin_approval_multi.inc"
#define APPROVAL_SYMBOL_NAME(symbol) symbol##2
#include "python_plugin_approval_multi.inc"
#define APPROVAL_SYMBOL_NAME(symbol) symbol##3
#include "python_plugin_approval_multi.inc"
#define APPROVAL_SYMBOL_NAME(symbol) symbol##4
#include "python_plugin_approval_multi.inc"
#define APPROVAL_SYMBOL_NAME(symbol) symbol##5
#include "python_plugin_approval_multi.inc"
#define APPROVAL_SYMBOL_NAME(symbol) symbol##6
#include "python_plugin_approval_multi.inc"
#define APPROVAL_SYMBOL_NAME(symbol) symbol##7
#include "python_plugin_approval_multi.inc"

static struct approval_plugin *extra_approval_plugins[] = {
    &python_approval1,
    &python_approval2,
    &python_approval3,
    &python_approval4,
    &python_approval5,
    &python_approval6,
    &python_approval7
};

struct approval_plugin *
python_approval_clone(void)
{
    static size_t counter = 0;
    struct approval_plugin *next_plugin = NULL;

    size_t max = sizeof(extra_approval_plugins) / sizeof(*extra_approval_plugins);
    if (counter < max) {
        next_plugin = extra_approval_plugins[counter];
        ++counter;
    } else if (counter == max) {
        ++counter;
        py_sudo_log(SUDO_CONV_ERROR_MSG,
                    "sudo: loading more than %d sudo python approval plugins is not supported\n", counter);
    }

    return next_plugin;
}
