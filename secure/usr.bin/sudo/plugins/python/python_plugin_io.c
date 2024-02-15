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

struct IOPluginContext
{
    struct PluginContext base_ctx;
    struct io_plugin *io_plugin;
};

#define BASE_CTX(io_ctx) (&(io_ctx->base_ctx))

#define PY_IO_PLUGIN_VERSION SUDO_API_MKVERSION(1, 0)

#define CALLBACK_PLUGINFUNC(func_name) io_ctx->io_plugin->func_name

// This also verifies compile time that the name matches the sudo plugin API.
#define CALLBACK_PYNAME(func_name) ((void)CALLBACK_PLUGINFUNC(func_name), #func_name)

#define MARK_CALLBACK_OPTIONAL(function_name) \
    do { \
        python_plugin_mark_callback_optional(plugin_ctx, CALLBACK_PYNAME(function_name), \
            (void **)&CALLBACK_PLUGINFUNC(function_name)); \
    } while(0)

sudo_dso_public struct io_plugin *python_io_clone(void);

static int
_call_plugin_open(struct IOPluginContext *io_ctx, int argc, char * const argv[], char * const command_info[])
{
    debug_decl(_call_plugin_open, PYTHON_DEBUG_CALLBACKS);
    struct PluginContext *plugin_ctx = BASE_CTX(io_ctx);
    plugin_ctx->call_close = 1;

    if (!PyObject_HasAttrString(plugin_ctx->py_instance, CALLBACK_PYNAME(open))) {
        debug_return_int(SUDO_RC_OK);
    }

    int rc = SUDO_RC_ERROR;
    PyObject *py_argv = py_str_array_to_tuple_with_count(argc, argv);
    PyObject *py_command_info = py_str_array_to_tuple(command_info);

    if (py_argv != NULL && py_command_info != NULL) {
        rc = python_plugin_api_rc_call(plugin_ctx, CALLBACK_PYNAME(open),
                                       Py_BuildValue("(OO)", py_argv, py_command_info));
    }

    if (rc != SUDO_RC_OK)
        plugin_ctx->call_close = 0;

    Py_XDECREF(py_argv);
    Py_XDECREF(py_command_info);
    debug_return_int(rc);
}

static int
python_plugin_io_open(struct IOPluginContext *io_ctx,
    unsigned int version, sudo_conv_t conversation,
    sudo_printf_t sudo_printf, char * const settings[],
    char * const user_info[], char * const command_info[],
    int argc, char * const argv[], char * const user_env[],
    char * const plugin_options[], const char **errstr)
{
    debug_decl(python_plugin_io_open, PYTHON_DEBUG_CALLBACKS);

    if (version < SUDO_API_MKVERSION(1, 2)) {
        sudo_printf(SUDO_CONV_ERROR_MSG,
                    "Error: Python IO plugin requires at least plugin API version 1.2\n");
        debug_return_int(SUDO_RC_ERROR);
    }

    int rc = python_plugin_register_logging(conversation, sudo_printf, settings);
    if (rc != SUDO_RC_OK)
        debug_return_int(rc);

    struct PluginContext *plugin_ctx = BASE_CTX(io_ctx);
    rc = python_plugin_init(plugin_ctx, plugin_options, version);

    if (rc != SUDO_RC_OK)
        debug_return_int(rc);

    rc = python_plugin_construct(plugin_ctx, PY_IO_PLUGIN_VERSION,
                                 settings, user_info, user_env, plugin_options);
    CALLBACK_SET_ERROR(plugin_ctx, errstr);
    if (rc != SUDO_RC_OK) {
        debug_return_int(rc);
    }

    // skip plugin callbacks which are not mandatory
    MARK_CALLBACK_OPTIONAL(log_ttyin);
    MARK_CALLBACK_OPTIONAL(log_ttyout);
    MARK_CALLBACK_OPTIONAL(log_stdin);
    MARK_CALLBACK_OPTIONAL(log_stdout);
    MARK_CALLBACK_OPTIONAL(log_stderr);
    MARK_CALLBACK_OPTIONAL(change_winsize);
    MARK_CALLBACK_OPTIONAL(log_suspend);
    // open and close are mandatory

    if (argc > 0)  // we only call open if there is request for running sg
        rc = _call_plugin_open(io_ctx, argc, argv, command_info);

    CALLBACK_SET_ERROR(plugin_ctx, errstr);
    debug_return_int(rc);
}

static void
python_plugin_io_close(struct IOPluginContext *io_ctx, int exit_status, int error)
{
    debug_decl(python_plugin_io_close, PYTHON_DEBUG_CALLBACKS);
    python_plugin_close(BASE_CTX(io_ctx), CALLBACK_PYNAME(close),
                        Py_BuildValue("(ii)", error == 0 ? exit_status : -1, error));
    debug_return;
}

static int
python_plugin_io_show_version(struct IOPluginContext *io_ctx, int verbose)
{
    debug_decl(python_plugin_io_show_version, PYTHON_DEBUG_CALLBACKS);

    PyThreadState_Swap(BASE_CTX(io_ctx)->py_interpreter);

    debug_return_int(python_plugin_show_version(BASE_CTX(io_ctx), CALLBACK_PYNAME(show_version),
                                                verbose, PY_IO_PLUGIN_VERSION, "io"));
}

static int
python_plugin_io_log_ttyin(struct IOPluginContext *io_ctx, const char *buf, unsigned int len, const char **errstr)
{
    debug_decl(python_plugin_io_log_ttyin, PYTHON_DEBUG_CALLBACKS);
    struct PluginContext *plugin_ctx = BASE_CTX(io_ctx);
    PyThreadState_Swap(plugin_ctx->py_interpreter);
    int rc = python_plugin_api_rc_call(plugin_ctx, CALLBACK_PYNAME(log_ttyin),
                                       Py_BuildValue("(s#)", buf, len));
    CALLBACK_SET_ERROR(plugin_ctx, errstr);
    debug_return_int(rc);
}

static int
python_plugin_io_log_ttyout(struct IOPluginContext *io_ctx, const char *buf, unsigned int len, const char **errstr)
{
    debug_decl(python_plugin_io_log_ttyout, PYTHON_DEBUG_CALLBACKS);
    struct PluginContext *plugin_ctx = BASE_CTX(io_ctx);
    PyThreadState_Swap(plugin_ctx->py_interpreter);
    int rc = python_plugin_api_rc_call(plugin_ctx, CALLBACK_PYNAME(log_ttyout),
                                       Py_BuildValue("(s#)", buf, len));
    CALLBACK_SET_ERROR(plugin_ctx, errstr);
    debug_return_int(rc);
}

static int
python_plugin_io_log_stdin(struct IOPluginContext *io_ctx, const char *buf, unsigned int len, const char **errstr)
{
    debug_decl(python_plugin_io_log_stdin, PYTHON_DEBUG_CALLBACKS);
    struct PluginContext *plugin_ctx = BASE_CTX(io_ctx);
    PyThreadState_Swap(plugin_ctx->py_interpreter);
    int rc = python_plugin_api_rc_call(plugin_ctx, CALLBACK_PYNAME(log_stdin),
                                               Py_BuildValue("(s#)", buf, len));
    CALLBACK_SET_ERROR(plugin_ctx, errstr);
    debug_return_int(rc);
}

static int
python_plugin_io_log_stdout(struct IOPluginContext *io_ctx, const char *buf, unsigned int len, const char **errstr)
{
    debug_decl(python_plugin_io_log_stdout, PYTHON_DEBUG_CALLBACKS);
    struct PluginContext *plugin_ctx = BASE_CTX(io_ctx);
    PyThreadState_Swap(plugin_ctx->py_interpreter);
    int rc = python_plugin_api_rc_call(plugin_ctx, CALLBACK_PYNAME(log_stdout),
                                               Py_BuildValue("(s#)", buf, len));
    CALLBACK_SET_ERROR(plugin_ctx, errstr);
    debug_return_int(rc);
}

static int
python_plugin_io_log_stderr(struct IOPluginContext *io_ctx, const char *buf, unsigned int len, const char **errstr)
{
    debug_decl(python_plugin_io_log_stderr, PYTHON_DEBUG_CALLBACKS);
    struct PluginContext *plugin_ctx = BASE_CTX(io_ctx);
    PyThreadState_Swap(plugin_ctx->py_interpreter);
    int rc = python_plugin_api_rc_call(plugin_ctx, CALLBACK_PYNAME(log_stderr),
                                               Py_BuildValue("(s#)", buf, len));
    CALLBACK_SET_ERROR(plugin_ctx, errstr);
    debug_return_int(rc);
}

static int
python_plugin_io_change_winsize(struct IOPluginContext *io_ctx, unsigned int line, unsigned int cols, const char **errstr)
{
    debug_decl(python_plugin_io_change_winsize, PYTHON_DEBUG_CALLBACKS);
    struct PluginContext *plugin_ctx = BASE_CTX(io_ctx);
    PyThreadState_Swap(plugin_ctx->py_interpreter);
    int rc = python_plugin_api_rc_call(plugin_ctx, CALLBACK_PYNAME(change_winsize),
                                       Py_BuildValue("(ii)", line, cols));
    CALLBACK_SET_ERROR(plugin_ctx, errstr);
    debug_return_int(rc);
}

static int
python_plugin_io_log_suspend(struct IOPluginContext *io_ctx, int signo, const char **errstr)
{
    debug_decl(python_plugin_io_log_suspend, PYTHON_DEBUG_CALLBACKS);
    struct PluginContext *plugin_ctx = BASE_CTX(io_ctx);
    PyThreadState_Swap(plugin_ctx->py_interpreter);
    int rc = python_plugin_api_rc_call(plugin_ctx, CALLBACK_PYNAME(log_suspend),
                                       Py_BuildValue("(i)", signo));
    CALLBACK_SET_ERROR(plugin_ctx, errstr);
    debug_return_int(rc);
}

// generate symbols for loading multiple io plugins:
sudo_dso_public struct io_plugin python_io;
#define IO_SYMBOL_NAME(symbol) symbol
#include "python_plugin_io_multi.inc"
#define IO_SYMBOL_NAME(symbol) symbol##1
#include "python_plugin_io_multi.inc"
#define IO_SYMBOL_NAME(symbol) symbol##2
#include "python_plugin_io_multi.inc"
#define IO_SYMBOL_NAME(symbol) symbol##3
#include "python_plugin_io_multi.inc"
#define IO_SYMBOL_NAME(symbol) symbol##4
#include "python_plugin_io_multi.inc"
#define IO_SYMBOL_NAME(symbol) symbol##5
#include "python_plugin_io_multi.inc"
#define IO_SYMBOL_NAME(symbol) symbol##6
#include "python_plugin_io_multi.inc"
#define IO_SYMBOL_NAME(symbol) symbol##7
#include "python_plugin_io_multi.inc"

static struct io_plugin *extra_io_plugins[] = {
    &python_io1,
    &python_io2,
    &python_io3,
    &python_io4,
    &python_io5,
    &python_io6,
    &python_io7
};

struct io_plugin *
python_io_clone(void)
{
    static size_t counter = 0;
    struct io_plugin *next_plugin = NULL;

    size_t max = sizeof(extra_io_plugins) / sizeof(*extra_io_plugins);
    if (counter < max) {
        next_plugin = extra_io_plugins[counter];
        ++counter;
    } else if (counter == max) {
        ++counter;
        py_sudo_log(SUDO_CONV_ERROR_MSG, "sudo: loading more than %d sudo python IO plugins is not supported\n", counter);
    }

    return next_plugin;
}
