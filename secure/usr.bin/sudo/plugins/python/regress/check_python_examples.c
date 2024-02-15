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

#include "testhelpers.h"
#include <unistd.h>

#include <sudo_dso.h>

#define DECL_PLUGIN(type, variable_name) \
    static struct type *variable_name = NULL; \
    static struct type variable_name ## _original

#define RESTORE_PYTHON_PLUGIN(variable_name) \
    memcpy(variable_name, &(variable_name ## _original), sizeof(variable_name ## _original))

#define SAVE_PYTHON_PLUGIN(variable_name) \
    memcpy(&(variable_name ## _original), variable_name, sizeof(variable_name ## _original))

static const char *python_plugin_so_path = NULL;
static void *python_plugin_handle = NULL;
DECL_PLUGIN(io_plugin, python_io);
DECL_PLUGIN(policy_plugin, python_policy);
DECL_PLUGIN(approval_plugin, python_approval);
DECL_PLUGIN(audit_plugin, python_audit);
DECL_PLUGIN(sudoers_group_plugin, group_plugin);

static struct passwd example_pwd;
static bool verbose;

static int _init_symbols(void);
static int _unlink_symbols(void);

static void
create_plugin_options(const char *module_name, const char *class_name, const char *extra_option)
{
    char opt_module_path[PATH_MAX + 256];
    char opt_classname[PATH_MAX + 256];
    snprintf(opt_module_path, sizeof(opt_module_path),
             "ModulePath=" SRC_DIR "/%s.py", module_name);

    snprintf(opt_classname, sizeof(opt_classname), "ClassName=%s", class_name);

    str_array_free(&data.plugin_options);
    size_t count = 3 + (extra_option != NULL);
    data.plugin_options = create_str_array(count, opt_module_path,
                                           opt_classname, extra_option, NULL);
}

static void
create_io_plugin_options(const char *log_path)
{
    char opt_logpath[PATH_MAX + 16];
    snprintf(opt_logpath, sizeof(opt_logpath), "LogPath=%s", log_path);
    create_plugin_options("example_io_plugin", "SudoIOPlugin", opt_logpath);
}

static void
create_debugging_plugin_options(void)
{
    create_plugin_options("example_debugging", "DebugDemoPlugin", NULL);
}

static void
create_audit_plugin_options(const char *extra_argument)
{
    create_plugin_options("example_audit_plugin", "SudoAuditPlugin", extra_argument);
}

static void
create_conversation_plugin_options(void)
{
    char opt_logpath[PATH_MAX + 16];
    snprintf(opt_logpath, sizeof(opt_logpath), "LogPath=%s", data.tmp_dir);
    create_plugin_options("example_conversation", "ReasonLoggerIOPlugin", opt_logpath);
}

static void
create_policy_plugin_options(void)
{
    create_plugin_options("example_policy_plugin", "SudoPolicyPlugin", NULL);
}

static int
init(void)
{
    // always start each test from clean state
    memset(&data, 0, sizeof(data));

    memset(&example_pwd, 0, sizeof(example_pwd));
    example_pwd.pw_name = (char *)"pw_name";
    example_pwd.pw_passwd = (char *)"pw_passwd";
    example_pwd.pw_gecos = (char *)"pw_gecos";
    example_pwd.pw_shell = (char *)"pw_shell";
    example_pwd.pw_dir = (char *)"pw_dir";
    example_pwd.pw_uid = (uid_t)1001;
    example_pwd.pw_gid = (gid_t)101;

    VERIFY_TRUE(asprintf(&data.tmp_dir, TEMP_PATH_TEMPLATE) >= 0);
    VERIFY_NOT_NULL(mkdtemp(data.tmp_dir));

    sudo_conf_clear_paths();

    // some default values for the plugin open:
    data.settings = create_str_array(1, NULL);
    data.user_info = create_str_array(1, NULL);
    data.command_info = create_str_array(1, NULL);
    data.plugin_argc = 0;
    data.plugin_argv = create_str_array(1, NULL);
    data.user_env = create_str_array(1, NULL);

    VERIFY_TRUE(_init_symbols());
    return true;
}

static int
cleanup(int success)
{
    if (!success) {
        printf("\nThe output of the plugin:\n%s", data.stdout_str);
        printf("\nThe error output of the plugin:\n%s", data.stderr_str);
    }

    VERIFY_TRUE(rmdir_recursive(data.tmp_dir));
    if (data.tmp_dir2) {
        VERIFY_TRUE(rmdir_recursive(data.tmp_dir2));
    }

    free(data.tmp_dir);
    free(data.tmp_dir2);

    str_array_free(&data.settings);
    str_array_free(&data.user_info);
    str_array_free(&data.command_info);
    str_array_free(&data.plugin_argv);
    str_array_free(&data.user_env);
    str_array_free(&data.plugin_options);

    return true;
}

static int
check_example_io_plugin_version_display(int is_verbose)
{
    const char *errstr = NULL;
    create_io_plugin_options(data.tmp_dir);

    VERIFY_INT(python_io->open(SUDO_API_VERSION, fake_conversation, fake_printf, data.settings,
                              data.user_info, data.command_info, data.plugin_argc, data.plugin_argv, data.user_env,
                              data.plugin_options, &errstr), SUDO_RC_OK);
    VERIFY_INT(python_io->show_version(is_verbose), SUDO_RC_OK);

    python_io->close(0, 0);  // this should not call the python plugin close as there was no command run invocation

    if (is_verbose) {
        // Note: the exact python version is environment dependent
        VERIFY_STR_CONTAINS(data.stdout_str, "Python interpreter version:");
        *strstr(data.stdout_str, "Python interpreter version:") = '\0';
        VERIFY_STDOUT(expected_path("check_example_io_plugin_version_display_full.stdout"));
    } else {
        VERIFY_STDOUT(expected_path("check_example_io_plugin_version_display.stdout"));
    }

    VERIFY_STDERR(expected_path("check_example_io_plugin_version_display.stderr"));
    VERIFY_FILE("sudo.log", expected_path("check_example_io_plugin_version_display.stored"));

    return true;
}

static int
check_example_io_plugin_command_log(void)
{
    const char *errstr = NULL;
    create_io_plugin_options(data.tmp_dir);

    str_array_free(&data.plugin_argv);
    data.plugin_argc = 2;
    data.plugin_argv = create_str_array(3, "id", "--help", NULL);

    str_array_free(&data.command_info);
    data.command_info = create_str_array(3, "command=/bin/id", "runas_uid=0", NULL);

    VERIFY_INT(python_io->open(SUDO_API_VERSION, fake_conversation, fake_printf, data.settings,
                              data.user_info, data.command_info, data.plugin_argc, data.plugin_argv,
                              data.user_env, data.plugin_options, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_io->log_stdin("some standard input", strlen("some standard input"), &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_io->log_stdout("some standard output", strlen("some standard output"), &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_io->log_stderr("some standard error", strlen("some standard error"), &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_io->log_suspend(SIGTSTP, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_io->log_suspend(SIGCONT, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_io->change_winsize(200, 100, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_io->log_ttyin("some tty input", strlen("some tty input"), &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_io->log_ttyout("some tty output", strlen("some tty output"), &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    python_io->close(1, 0);  // successful execution, command returned 1

    VERIFY_STDOUT(expected_path("check_example_io_plugin_command_log.stdout"));
    VERIFY_STDERR(expected_path("check_example_io_plugin_command_log.stderr"));
    VERIFY_FILE("sudo.log", expected_path("check_example_io_plugin_command_log.stored"));

    return true;
}

typedef struct io_plugin * (io_clone_func)(void);

static int
check_example_io_plugin_command_log_multiple(void)
{
    const char *errstr = NULL;

    // verify multiple python io plugin symbols are available
    io_clone_func *python_io_clone = (io_clone_func *)sudo_dso_findsym(python_plugin_handle, "python_io_clone");
    VERIFY_PTR_NE(python_io_clone, NULL);

    struct io_plugin *python_io2 = NULL;

    for (int i = 0; i < 7; ++i) {
        python_io2 = (*python_io_clone)();
        VERIFY_PTR_NE(python_io2, NULL);
        VERIFY_PTR_NE(python_io2, python_io);
    }

    // open the first plugin and let it log to tmp_dir
    create_io_plugin_options(data.tmp_dir);

    str_array_free(&data.plugin_argv);
    data.plugin_argc = 2;
    data.plugin_argv = create_str_array(3, "id", "--help", NULL);

    str_array_free(&data.command_info);
    data.command_info = create_str_array(3, "command=/bin/id", "runas_uid=0", NULL);

    VERIFY_INT(python_io->open(SUDO_API_VERSION, fake_conversation, fake_printf, data.settings,
                              data.user_info, data.command_info, data.plugin_argc, data.plugin_argv,
                              data.user_env, data.plugin_options, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    // For verifying the error message of no more plugin. It should be displayed only once.
    VERIFY_PTR((*python_io_clone)(), NULL);
    VERIFY_PTR((*python_io_clone)(), NULL);

    // open the second plugin with another log directory
    VERIFY_TRUE(asprintf(&data.tmp_dir2, TEMP_PATH_TEMPLATE) >= 0);
    VERIFY_NOT_NULL(mkdtemp(data.tmp_dir2));
    create_io_plugin_options(data.tmp_dir2);

    str_array_free(&data.plugin_argv);
    data.plugin_argc = 1;
    data.plugin_argv = create_str_array(2, "whoami", NULL);

    str_array_free(&data.command_info);
    data.command_info = create_str_array(3, "command=/bin/whoami", "runas_uid=1", NULL);

    VERIFY_INT(python_io2->open(SUDO_API_VERSION, fake_conversation, fake_printf, data.settings,
                              data.user_info, data.command_info, data.plugin_argc, data.plugin_argv,
                              data.user_env, data.plugin_options, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    VERIFY_INT(python_io->log_stdin("stdin for plugin 1", strlen("stdin for plugin 1"), &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_io2->log_stdin("stdin for plugin 2", strlen("stdin for plugin 2"), &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_io->log_stdout("stdout for plugin 1", strlen("stdout for plugin 1"), &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_io2->log_stdout("stdout for plugin 2", strlen("stdout for plugin 2"), &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_io->log_stderr("stderr for plugin 1", strlen("stderr for plugin 1"), &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_io2->log_stderr("stderr for plugin 2", strlen("stderr for plugin 2"), &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_io->log_suspend(SIGTSTP, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_io2->log_suspend(SIGSTOP, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_io->log_suspend(SIGCONT, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_io2->log_suspend(SIGCONT, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_io->change_winsize(20, 10, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_io2->change_winsize(30, 40, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_io->log_ttyin("tty input for plugin 1", strlen("tty input for plugin 1"), &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_io2->log_ttyin("tty input for plugin 2", strlen("tty input for plugin 2"), &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_io->log_ttyout("tty output for plugin 1", strlen("tty output for plugin 1"), &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_io2->log_ttyout("tty output for plugin 2", strlen("tty output for plugin 2"), &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    python_io->close(1, 0);  // successful execution, command returned 1
    python_io2->close(2, 0);  //                      command returned 2

    VERIFY_STDOUT(expected_path("check_example_io_plugin_command_log_multiple.stdout"));
    VERIFY_STDERR(expected_path("check_example_io_plugin_command_log_multiple.stderr"));
    VERIFY_FILE("sudo.log", expected_path("check_example_io_plugin_command_log_multiple1.stored"));
    VERIFY_TRUE(verify_file(data.tmp_dir2, "sudo.log", expected_path("check_example_io_plugin_command_log_multiple2.stored")));

    return true;
}

static int
check_example_io_plugin_failed_to_start_command(void)
{
    const char *errstr = NULL;

    create_io_plugin_options(data.tmp_dir);

    str_array_free(&data.plugin_argv);
    data.plugin_argc = 1;
    data.plugin_argv = create_str_array(2, "cmd", NULL);

    str_array_free(&data.command_info);
    data.command_info = create_str_array(3, "command=/usr/share/cmd", "runas_uid=0", NULL);

    VERIFY_INT(python_io->open(SUDO_API_VERSION, fake_conversation, fake_printf, data.settings,
                              data.user_info, data.command_info, data.plugin_argc, data.plugin_argv,
                              data.user_env, data.plugin_options, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    python_io->close(0, EPERM);  // execve returned with error

    VERIFY_STDOUT(expected_path("check_example_io_plugin_failed_to_start_command.stdout"));
    VERIFY_STDERR(expected_path("check_example_io_plugin_failed_to_start_command.stderr"));
    VERIFY_FILE("sudo.log", expected_path("check_example_io_plugin_failed_to_start_command.stored"));

    return true;
}

static int
check_example_io_plugin_fails_with_python_backtrace(void)
{
    const char *errstr = NULL;

    create_io_plugin_options("/some/not/writable/directory");

    VERIFY_INT(python_io->open(SUDO_API_VERSION, fake_conversation, fake_printf, data.settings,
                              data.user_info, data.command_info, data.plugin_argc, data.plugin_argv,
                              data.user_env, data.plugin_options, &errstr), SUDO_RC_ERROR);
    VERIFY_PTR(errstr, NULL);

    VERIFY_STDOUT(expected_path("check_example_io_plugin_fails_with_python_backtrace.stdout"));
    VERIFY_STDERR(expected_path("check_example_io_plugin_fails_with_python_backtrace.stderr"));

    python_io->close(0, 0);
    return true;
}

static int
check_io_plugin_reports_error(void)
{
    const char *errstr = NULL;
    str_array_free(&data.plugin_options);
    data.plugin_options = create_str_array(
        3,
        "ModulePath=" SRC_DIR "/regress/plugin_errorstr.py",
        "ClassName=ConstructErrorPlugin",
        NULL
    );

    VERIFY_INT(python_io->open(SUDO_API_VERSION, fake_conversation, fake_printf, data.settings,
                              data.user_info, data.command_info, data.plugin_argc, data.plugin_argv,
                              data.user_env, data.plugin_options, &errstr), SUDO_RC_ERROR);

    VERIFY_STR(errstr, "Something wrong in plugin constructor");
    errstr = NULL;

    python_io->close(0, 0);

    str_array_free(&data.plugin_options);
    data.plugin_options = create_str_array(
        3,
        "ModulePath=" SRC_DIR "/regress/plugin_errorstr.py",
        "ClassName=ErrorMsgPlugin",
        NULL
    );

    VERIFY_INT(python_io->open(SUDO_API_VERSION, fake_conversation, fake_printf, data.settings,
                              data.user_info, data.command_info, data.plugin_argc, data.plugin_argv,
                              data.user_env, data.plugin_options, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    VERIFY_INT(python_io->log_stdin("", 0, &errstr), SUDO_RC_ERROR);
    VERIFY_STR(errstr, "Something wrong in log_stdin");

    errstr = (void *)13;
    VERIFY_INT(python_io->log_stdout("", 0, &errstr), SUDO_RC_ERROR);
    VERIFY_STR(errstr, "Something wrong in log_stdout");

    errstr = NULL;
    VERIFY_INT(python_io->log_stderr("", 0, &errstr), SUDO_RC_ERROR);
    VERIFY_STR(errstr, "Something wrong in log_stderr");

    errstr = NULL;
    VERIFY_INT(python_io->log_ttyin("", 0, &errstr), SUDO_RC_ERROR);
    VERIFY_STR(errstr, "Something wrong in log_ttyin");

    errstr = NULL;
    VERIFY_INT(python_io->log_ttyout("", 0, &errstr), SUDO_RC_ERROR);
    VERIFY_STR(errstr, "Something wrong in log_ttyout");

    errstr = NULL;
    VERIFY_INT(python_io->log_suspend(SIGTSTP, &errstr), SUDO_RC_ERROR);
    VERIFY_STR(errstr, "Something wrong in log_suspend");

    errstr = NULL;
    VERIFY_INT(python_io->change_winsize(200, 100, &errstr), SUDO_RC_ERROR);
    VERIFY_STR(errstr, "Something wrong in change_winsize");

    python_io->close(0, 0);

    VERIFY_STR(data.stderr_str, "");
    VERIFY_STR(data.stdout_str, "");
    return true;
}

static int
check_example_group_plugin(void)
{
    create_plugin_options("example_group_plugin", "SudoGroupPlugin", NULL);

    VERIFY_INT(group_plugin->init(GROUP_API_VERSION, fake_printf, data.plugin_options), SUDO_RC_OK);

    VERIFY_INT(group_plugin->query("test", "mygroup", NULL), SUDO_RC_OK);
    VERIFY_INT(group_plugin->query("testuser2", "testgroup", NULL), SUDO_RC_OK);
    VERIFY_INT(group_plugin->query("testuser2", "mygroup", NULL), SUDO_RC_REJECT);
    VERIFY_INT(group_plugin->query("test", "testgroup", NULL), SUDO_RC_REJECT);

    group_plugin->cleanup();
    VERIFY_STR(data.stderr_str, "");
    VERIFY_STR(data.stdout_str, "");
    return true;
}

#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
static const char *
create_debug_config(const char *debug_spec)
{
    char *result = NULL;

    static char config_path[PATH_MAX] = "/";
    snprintf(config_path, sizeof(config_path), "%s/sudo.conf", data.tmp_dir);

    char *content = NULL;
    if (asprintf(&content, "Debug %s %s/debug.log %s\n",
                 "python_plugin.so", data.tmp_dir, debug_spec) < 0)
    {
        puts("Failed to allocate string");
        goto cleanup;
    }

    if (fwriteall(config_path, content) != true) {
        printf("Failed to write '%s'\n", config_path);
        goto cleanup;
    }

    result = config_path;

cleanup:
    free(content);

    return result;
}

static int
check_example_group_plugin_is_able_to_debug(void)
{
    const char *config_path = create_debug_config("py_calls@diag");
    VERIFY_NOT_NULL(config_path);
    VERIFY_INT(sudo_conf_read(config_path, SUDO_CONF_ALL), true);

    create_plugin_options("example_group_plugin", "SudoGroupPlugin", NULL);

    group_plugin->init(GROUP_API_VERSION, fake_printf, data.plugin_options);

    group_plugin->query("user", "group", &example_pwd);

    group_plugin->cleanup();

    VERIFY_STR(data.stderr_str, "");
    VERIFY_STR(data.stdout_str, "");

    VERIFY_LOG_LINES(expected_path("check_example_group_plugin_is_able_to_debug.log"));

    return true;
}
#endif /* FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION */

static int
check_plugin_unload(void)
{
    // You can call this test to avoid having a lot of subinterpreters
    // (each plugin->open starts one, and only plugin unlink closes)
    // It only verifies that python was shut down correctly.
    VERIFY_TRUE(Py_IsInitialized());
    VERIFY_TRUE(_unlink_symbols());
    VERIFY_FALSE(Py_IsInitialized());  // python interpreter could be stopped
    return true;
}

#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
static int
check_example_debugging(const char *debug_spec)
{
    const char *errstr = NULL;
    const char *config_path = create_debug_config(debug_spec);
    VERIFY_NOT_NULL(config_path);
    VERIFY_INT(sudo_conf_read(config_path, SUDO_CONF_ALL), true);

    create_debugging_plugin_options();

    str_array_free(&data.settings);
    char *debug_flags_setting = NULL;
    VERIFY_TRUE(asprintf(&debug_flags_setting, "debug_flags=%s/debug.log %s", data.tmp_dir, debug_spec) >= 0);

    data.settings = create_str_array(3, debug_flags_setting, "plugin_path=python_plugin.so", NULL);

    VERIFY_INT(python_io->open(SUDO_API_VERSION, fake_conversation, fake_printf, data.settings,
                              data.user_info, data.command_info, data.plugin_argc, data.plugin_argv,
                              data.user_env, data.plugin_options, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    python_io->close(0, 0);

    VERIFY_STR(data.stderr_str, "");
    VERIFY_STR(data.stdout_str, "");

    VERIFY_LOG_LINES(expected_path("check_example_debugging_%s.log", debug_spec));

    free(debug_flags_setting);
    return true;
}
#endif /* FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION */

static int
check_loading_fails(const char *name)
{
    const char *errstr = NULL;

    VERIFY_INT(python_io->open(SUDO_API_VERSION, fake_conversation, fake_printf, data.settings,
                              data.user_info, data.command_info, data.plugin_argc, data.plugin_argv,
                              data.user_env, data.plugin_options, &errstr), SUDO_RC_ERROR);
    VERIFY_PTR(errstr, NULL);
    python_io->close(0, 0);

    VERIFY_STDOUT(expected_path("check_loading_fails_%s.stdout", name));
    VERIFY_STDERR(expected_path("check_loading_fails_%s.stderr", name));

    return true;
}

static int
check_loading_fails_with_missing_path(void)
{
    str_array_free(&data.plugin_options);
    data.plugin_options = create_str_array(2, "ClassName=DebugDemoPlugin", NULL);
    return check_loading_fails("missing_path");
}

static int
check_loading_succeeds_with_missing_classname(void)
{
    str_array_free(&data.plugin_options);
    data.plugin_options = create_str_array(2, "ModulePath=" SRC_DIR "/example_debugging.py", NULL);

    const char *errstr = NULL;

    VERIFY_INT(python_io->open(SUDO_API_VERSION, fake_conversation, fake_printf, data.settings,
                              data.user_info, data.command_info, data.plugin_argc, data.plugin_argv,
                              data.user_env, data.plugin_options, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_io->show_version(1), SUDO_RC_OK);
    python_io->close(0, 0);

    VERIFY_STDOUT(expected_path("check_loading_succeeds_with_missing_classname.stdout"));
    VERIFY_STR(data.stderr_str, "");

    return true;
}

static int
check_loading_fails_with_missing_classname(void)
{
    str_array_free(&data.plugin_options);
    data.plugin_options = create_str_array(2, "ModulePath=" SRC_DIR "/regress/plugin_errorstr.py", NULL);
    return check_loading_fails("missing_classname");
}

static int
check_loading_fails_with_wrong_classname(void)
{
    create_plugin_options("example_debugging", "MispelledPluginName", NULL);
    return check_loading_fails("wrong_classname");
}

static int
check_loading_fails_with_wrong_path(void)
{
    str_array_free(&data.plugin_options);
    data.plugin_options = create_str_array(3, "ModulePath=/wrong_path.py", "ClassName=PluginName", NULL);
    return check_loading_fails("wrong_path");
}

static int
check_example_conversation_plugin_reason_log(int simulate_suspend, const char *description)
{
    const char *errstr = NULL;

    create_conversation_plugin_options();

    str_array_free(&data.plugin_argv); // have a command run
    data.plugin_argc = 1;
    data.plugin_argv = create_str_array(2, "/bin/whoami", NULL);

    data.conv_replies[0] = "my fake reason";
    data.conv_replies[1] = "my real secret reason";

    sudo_conv_t conversation = simulate_suspend ? fake_conversation_with_suspend : fake_conversation;

    VERIFY_INT(python_io->open(SUDO_API_VERSION, conversation, fake_printf, data.settings,
                              data.user_info, data.command_info, data.plugin_argc, data.plugin_argv,
                              data.user_env, data.plugin_options, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    python_io->close(0, 0);

    VERIFY_STDOUT(expected_path("check_example_conversation_plugin_reason_log_%s.stdout", description));
    VERIFY_STDERR(expected_path("check_example_conversation_plugin_reason_log_%s.stderr", description));
    VERIFY_CONV(expected_path("check_example_conversation_plugin_reason_log_%s.conversation", description));
    VERIFY_FILE("sudo_reasons.txt", expected_path("check_example_conversation_plugin_reason_log_%s.stored", description));
    return true;
}

static int
check_example_conversation_plugin_user_interrupts(void)
{
    const char *errstr = NULL;

    create_conversation_plugin_options();

    str_array_free(&data.plugin_argv); // have a command run
    data.plugin_argc = 1;
    data.plugin_argv = create_str_array(2, "/bin/whoami", NULL);

    data.conv_replies[0] = NULL; // this simulates user interrupt for the first question

    VERIFY_INT(python_io->open(SUDO_API_VERSION, fake_conversation, fake_printf, data.settings,
                              data.user_info, data.command_info, data.plugin_argc, data.plugin_argv,
                              data.user_env, data.plugin_options, &errstr), SUDO_RC_REJECT);
    VERIFY_PTR(errstr, NULL);
    python_io->close(0, 0);

    VERIFY_STDOUT(expected_path("check_example_conversation_plugin_user_interrupts.stdout"));
    VERIFY_STDERR(expected_path("check_example_conversation_plugin_user_interrupts.stderr"));
    VERIFY_CONV(expected_path("check_example_conversation_plugin_user_interrupts.conversation"));
    return true;
}

static int
check_example_policy_plugin_version_display(int is_verbose)
{
    const char *errstr = NULL;

    create_policy_plugin_options();

    VERIFY_INT(python_policy->open(SUDO_API_VERSION, fake_conversation, fake_printf, data.settings,
                                  data.user_info, data.user_env, data.plugin_options, &errstr),
               SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);
    VERIFY_INT(python_policy->show_version(is_verbose), SUDO_RC_OK);

    python_policy->close(0, 0);  // this should not call the python plugin close as there was no command run invocation

    if (is_verbose) {
        // Note: the exact python version is environment dependent
        VERIFY_STR_CONTAINS(data.stdout_str, "Python interpreter version:");
        *strstr(data.stdout_str, "Python interpreter version:") = '\0';
        VERIFY_STDOUT(expected_path("check_example_policy_plugin_version_display_full.stdout"));
    } else {
        VERIFY_STDOUT(expected_path("check_example_policy_plugin_version_display.stdout"));
    }

    VERIFY_STDERR(expected_path("check_example_policy_plugin_version_display.stderr"));

    return true;
}

static int
check_example_policy_plugin_accepted_execution(void)
{
    const char *errstr = NULL;

    create_policy_plugin_options();

    str_array_free(&data.plugin_argv);
    data.plugin_argc = 2;
    data.plugin_argv = create_str_array(3, "/bin/whoami", "--help", NULL);

    str_array_free(&data.user_env);
    data.user_env = create_str_array(3, "USER_ENV1=VALUE1", "USER_ENV2=value2", NULL);

    VERIFY_INT(python_policy->open(SUDO_API_VERSION, fake_conversation, fake_printf, data.settings,
                                  data.user_info, data.user_env, data.plugin_options, &errstr),
               SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    char **env_add = create_str_array(3, "REQUESTED_ENV1=VALUE1", "REQUESTED_ENV2=value2", NULL);

    char **argv_out, **user_env_out, **command_info_out;  // free to contain garbage

    VERIFY_INT(python_policy->check_policy(data.plugin_argc, data.plugin_argv, env_add,
                                          &command_info_out, &argv_out, &user_env_out, &errstr),
               SUDO_RC_ACCEPT);
    VERIFY_PTR(errstr, NULL);

    VERIFY_STR_SET(command_info_out, 4, "command=/bin/whoami", "runas_uid=0", "runas_gid=0", NULL);
    VERIFY_STR_SET(user_env_out, 5, "USER_ENV1=VALUE1", "USER_ENV2=value2",
                   "REQUESTED_ENV1=VALUE1", "REQUESTED_ENV2=value2", NULL);
    VERIFY_STR_SET(argv_out, 3, "/bin/whoami", "--help", NULL);

    VERIFY_INT(python_policy->init_session(&example_pwd, &user_env_out, &errstr), SUDO_RC_ACCEPT);
    VERIFY_PTR(errstr, NULL);

    // init session is able to modify the user env:
    VERIFY_STR_SET(user_env_out, 6, "USER_ENV1=VALUE1", "USER_ENV2=value2",
                   "REQUESTED_ENV1=VALUE1", "REQUESTED_ENV2=value2", "PLUGIN_EXAMPLE_ENV=1", NULL);

    python_policy->close(3, 0);  // successful execution returned exit code 3

    VERIFY_STDOUT(expected_path("check_example_policy_plugin_accepted_execution.stdout"));
    VERIFY_STDERR(expected_path("check_example_policy_plugin_accepted_execution.stderr"));

    str_array_free(&env_add);
    str_array_free(&user_env_out);
    str_array_free(&command_info_out);
    str_array_free(&argv_out);
    return true;
}

static int
check_example_policy_plugin_failed_execution(void)
{
    const char *errstr = NULL;

    create_policy_plugin_options();

    str_array_free(&data.plugin_argv);
    data.plugin_argc = 2;
    data.plugin_argv = create_str_array(3, "/bin/id", "--help", NULL);

    VERIFY_INT(python_policy->open(SUDO_API_VERSION, fake_conversation, fake_printf, data.settings,
                                  data.user_info, data.user_env, data.plugin_options, &errstr),
               SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    char **argv_out, **user_env_out, **command_info_out;  // free to contain garbage

    VERIFY_INT(python_policy->check_policy(data.plugin_argc, data.plugin_argv, NULL,
                                          &command_info_out, &argv_out, &user_env_out, &errstr),
               SUDO_RC_ACCEPT);
    VERIFY_PTR(errstr, NULL);

    // pwd is unset (user is not part of /etc/passwd)
    VERIFY_INT(python_policy->init_session(NULL, &user_env_out, &errstr), SUDO_RC_ACCEPT);
    VERIFY_PTR(errstr, NULL);

    python_policy->close(12345, ENOENT);  // failed to execute

    VERIFY_STDOUT(expected_path("check_example_policy_plugin_failed_execution.stdout"));
    VERIFY_STDERR(expected_path("check_example_policy_plugin_failed_execution.stderr"));

    str_array_free(&user_env_out);
    str_array_free(&command_info_out);
    str_array_free(&argv_out);
    return true;
}

static int
check_example_policy_plugin_denied_execution(void)
{
    const char *errstr = NULL;

    create_policy_plugin_options();

    str_array_free(&data.plugin_argv);
    data.plugin_argc = 1;
    data.plugin_argv = create_str_array(2, "/bin/passwd", NULL);

    VERIFY_INT(python_policy->open(SUDO_API_VERSION, fake_conversation, fake_printf, data.settings,
                                  data.user_info, data.user_env, data.plugin_options, &errstr),
               SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    char **argv_out, **user_env_out, **command_info_out;  // free to contain garbage

    VERIFY_INT(python_policy->check_policy(data.plugin_argc, data.plugin_argv, NULL,
                                          &command_info_out, &argv_out, &user_env_out, &errstr),
               SUDO_RC_REJECT);
    VERIFY_PTR(errstr, NULL);

    VERIFY_PTR(command_info_out, NULL);
    VERIFY_PTR(argv_out, NULL);
    VERIFY_PTR(user_env_out, NULL);

    python_policy->close(0, 0);  // there was no execution

    VERIFY_STDOUT(expected_path("check_example_policy_plugin_denied_execution.stdout"));
    VERIFY_STDERR(expected_path("check_example_policy_plugin_denied_execution.stderr"));

    return true;
}

static int
check_example_policy_plugin_list(void)
{
    const char *errstr = NULL;

    create_policy_plugin_options();

    VERIFY_INT(python_policy->open(SUDO_API_VERSION, fake_conversation, fake_printf, data.settings,
                                  data.user_info, data.user_env, data.plugin_options, &errstr),
               SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    snprintf_append(data.stdout_str, MAX_OUTPUT, "-- minimal --\n");
    VERIFY_INT(python_policy->list(data.plugin_argc, data.plugin_argv, false, NULL, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    snprintf_append(data.stdout_str, MAX_OUTPUT, "\n-- minimal (verbose) --\n");
    VERIFY_INT(python_policy->list(data.plugin_argc, data.plugin_argv, true, NULL, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    snprintf_append(data.stdout_str, MAX_OUTPUT, "\n-- with user --\n");
    VERIFY_INT(python_policy->list(data.plugin_argc, data.plugin_argv, false, "testuser", &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    snprintf_append(data.stdout_str, MAX_OUTPUT, "\n-- with user (verbose) --\n");
    VERIFY_INT(python_policy->list(data.plugin_argc, data.plugin_argv, true, "testuser", &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    snprintf_append(data.stdout_str, MAX_OUTPUT, "\n-- with allowed program --\n");
    str_array_free(&data.plugin_argv);
    data.plugin_argc = 3;
    data.plugin_argv = create_str_array(4, "/bin/id", "some", "arguments", NULL);
    VERIFY_INT(python_policy->list(data.plugin_argc, data.plugin_argv, false, NULL, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    snprintf_append(data.stdout_str, MAX_OUTPUT, "\n-- with allowed program (verbose) --\n");
    VERIFY_INT(python_policy->list(data.plugin_argc, data.plugin_argv, true, NULL, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    snprintf_append(data.stdout_str, MAX_OUTPUT, "\n-- with denied program --\n");
    str_array_free(&data.plugin_argv);
    data.plugin_argc = 1;
    data.plugin_argv = create_str_array(2, "/bin/passwd", NULL);
    VERIFY_INT(python_policy->list(data.plugin_argc, data.plugin_argv, false, NULL, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    snprintf_append(data.stdout_str, MAX_OUTPUT, "\n-- with denied program (verbose) --\n");
    VERIFY_INT(python_policy->list(data.plugin_argc, data.plugin_argv, true, NULL, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    python_policy->close(0, 0);  // there was no execution

    VERIFY_STDOUT(expected_path("check_example_policy_plugin_list.stdout"));
    VERIFY_STDERR(expected_path("check_example_policy_plugin_list.stderr"));

    return true;
}

static int
check_example_policy_plugin_validate_invalidate(void)
{
    const char *errstr = NULL;

#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
    // the plugin does not do any meaningful for these, so using log to validate instead
    const char *config_path = create_debug_config("py_calls@diag");
    VERIFY_NOT_NULL(config_path);
    VERIFY_INT(sudo_conf_read(config_path, SUDO_CONF_ALL), true);
#endif

    create_policy_plugin_options();

    VERIFY_INT(python_policy->open(SUDO_API_VERSION, fake_conversation, fake_printf, data.settings,
                                  data.user_info, data.user_env, data.plugin_options, &errstr),
               SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    VERIFY_INT(python_policy->validate(&errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    python_policy->invalidate(true);
    python_policy->invalidate(false);

    python_policy->close(0, 0); // no command execution

#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
    VERIFY_LOG_LINES(expected_path("check_example_policy_plugin_validate_invalidate.log"));
#endif
    VERIFY_STR(data.stderr_str, "");
    VERIFY_STR(data.stdout_str, "");
    return true;
}

static int
check_policy_plugin_callbacks_are_optional(void)
{
    const char *errstr = NULL;

    create_debugging_plugin_options();

    VERIFY_INT(python_policy->open(SUDO_API_VERSION, fake_conversation, fake_printf, data.settings,
                                  data.user_info, data.user_env, data.plugin_options, &errstr),
               SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    VERIFY_PTR(python_policy->list, NULL);
    VERIFY_PTR(python_policy->validate, NULL);
    VERIFY_PTR(python_policy->invalidate, NULL);
    VERIFY_PTR_NE(python_policy->check_policy, NULL); // (not optional)
    VERIFY_PTR(python_policy->init_session, NULL);

    // show_version always displays the plugin, but it is optional in the python layer
    VERIFY_PTR_NE(python_policy->show_version, NULL);
    VERIFY_INT(python_policy->show_version(1), SUDO_RC_OK);

    python_policy->close(0, 0);
    return true;
}

static int
check_policy_plugin_reports_error(void)
{
    const char *errstr = NULL;
    str_array_free(&data.plugin_options);
    data.plugin_options = create_str_array(
        3,
        "ModulePath=" SRC_DIR "/regress/plugin_errorstr.py",
        "ClassName=ConstructErrorPlugin",
        NULL
    );

    VERIFY_INT(python_policy->open(SUDO_API_VERSION, fake_conversation, fake_printf, data.settings,
                              data.user_info, data.user_env, data.plugin_options, &errstr), SUDO_RC_ERROR);
    VERIFY_STR(errstr, "Something wrong in plugin constructor");
    errstr = NULL;

    python_policy->close(0, 0);

    str_array_free(&data.plugin_options);
    data.plugin_options = create_str_array(
        3,
        "ModulePath=" SRC_DIR "/regress/plugin_errorstr.py",
        "ClassName=ErrorMsgPlugin",
        NULL
    );

    data.plugin_argc = 1;
    str_array_free(&data.plugin_argv);
    data.plugin_argv = create_str_array(2, "id", NULL);

    VERIFY_INT(python_policy->open(SUDO_API_VERSION, fake_conversation, fake_printf, data.settings,
                              data.user_info, data.user_env, data.plugin_options, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    char **command_info_out = NULL;
    char **argv_out = NULL;
    char **user_env_out = NULL;

    VERIFY_INT(python_policy->list(data.plugin_argc, data.plugin_argv, true, NULL, &errstr), SUDO_RC_ERROR);
    VERIFY_STR(errstr, "Something wrong in list");

    errstr = NULL;
    VERIFY_INT(python_policy->validate(&errstr), SUDO_RC_ERROR);
    VERIFY_STR(errstr, "Something wrong in validate");

    errstr = NULL;
    VERIFY_INT(python_policy->check_policy(data.plugin_argc, data.plugin_argv, data.user_env,
                                           &command_info_out, &argv_out, &user_env_out, &errstr),
               SUDO_RC_ERROR);
    VERIFY_STR(errstr, "Something wrong in check_policy");

    errstr = NULL;
    VERIFY_INT(python_policy->init_session(&example_pwd, &user_env_out, &errstr), SUDO_RC_ERROR);
    VERIFY_STR(errstr, "Something wrong in init_session");

    python_policy->close(0, 0);

    VERIFY_STR(data.stderr_str, "");
    VERIFY_STR(data.stdout_str, "");
    return true;
}

static int
check_io_plugin_callbacks_are_optional(void)
{
    const char *errstr = NULL;

    create_debugging_plugin_options();

    VERIFY_INT(python_io->open(SUDO_API_VERSION, fake_conversation, fake_printf, data.settings,
                              data.user_info, data.command_info, data.plugin_argc, data.plugin_argv,
                              data.user_env, data.plugin_options, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    VERIFY_PTR(python_io->log_stdin, NULL);
    VERIFY_PTR(python_io->log_stdout, NULL);
    VERIFY_PTR(python_io->log_stderr, NULL);
    VERIFY_PTR(python_io->log_ttyin, NULL);
    VERIFY_PTR(python_io->log_ttyout, NULL);
    VERIFY_PTR(python_io->change_winsize, NULL);

    // show_version always displays the plugin, but it is optional in the python layer
    VERIFY_PTR_NE(python_io->show_version, NULL);
    VERIFY_INT(python_io->show_version(1), SUDO_RC_OK);

    python_io->close(0, 0);
    return true;
}

static int
check_python_plugins_do_not_affect_each_other(void)
{
    const char *errstr = NULL;

    // We test here that one plugin is not able to effect the environment of another
    // This is important so they do not ruin or depend on each other's state.
    create_plugin_options("regress/plugin_conflict", "ConflictPlugin", "Path=path_for_first_plugin");

    VERIFY_INT(python_io->open(SUDO_API_VERSION, fake_conversation, fake_printf, data.settings,
                              data.user_info, data.command_info, data.plugin_argc, data.plugin_argv,
                              data.user_env, data.plugin_options, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    create_plugin_options("regress/plugin_conflict", "ConflictPlugin", "Path=path_for_second_plugin");
    VERIFY_INT(python_policy->open(SUDO_API_VERSION, fake_conversation, fake_printf, data.settings,
                              data.user_info, data.user_env, data.plugin_options, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    python_io->close(0, 0);
    python_policy->close(0, 0);

    VERIFY_STDOUT(expected_path("check_python_plugins_do_not_affect_each_other.stdout"));
    VERIFY_STR(data.stderr_str, "");
    return true;
}

static int
check_example_audit_plugin_receives_accept(void)
{
    create_audit_plugin_options("");
    const char *errstr = NULL;

    str_array_free(&data.plugin_argv);
    data.plugin_argv = create_str_array(6, "sudo", "-u", "user", "id", "--help", NULL);

    str_array_free(&data.user_env);
    data.user_env = create_str_array(3, "KEY1=VALUE1", "KEY2=VALUE2", NULL);

    str_array_free(&data.user_info);
    data.user_info = create_str_array(3, "user=testuser1", "uid=123", NULL);

    VERIFY_INT(python_audit->open(SUDO_API_VERSION, fake_conversation, fake_printf,
                                  data.settings, data.user_info, 3, data.plugin_argv,
                                  data.user_env, data.plugin_options, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    str_array_free(&data.command_info);
    data.command_info = create_str_array(2, "command=/sbin/id", NULL);

    str_array_free(&data.plugin_argv);
    data.plugin_argv = create_str_array(3, "id", "--help", NULL);

    VERIFY_INT(python_audit->accept("accepter plugin name", SUDO_POLICY_PLUGIN,
                                    data.command_info, data.plugin_argv,
                                    data.user_env, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    python_audit->close(SUDO_PLUGIN_WAIT_STATUS, W_EXITCODE(2, 0));  // process exited with 2

    VERIFY_STDOUT(expected_path("check_example_audit_plugin_receives_accept.stdout"));
    VERIFY_STR(data.stderr_str, "");

    return true;
}

static int
check_example_audit_plugin_receives_reject(void)
{
    create_audit_plugin_options(NULL);
    const char *errstr = NULL;

    str_array_free(&data.plugin_argv);
    data.plugin_argv = create_str_array(3, "sudo", "passwd", NULL);

    str_array_free(&data.user_info);
    data.user_info = create_str_array(3, "user=root", "uid=0", NULL);

    VERIFY_INT(python_audit->open(SUDO_API_VERSION, fake_conversation, fake_printf,
                                  data.settings, data.user_info, 1, data.plugin_argv,
                                  data.user_env, data.plugin_options, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    VERIFY_INT(python_audit->reject("rejecter plugin name", SUDO_IO_PLUGIN,
                                    "Rejected just because!", data.command_info,
                                    &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    python_audit->close(SUDO_PLUGIN_NO_STATUS, 0);  // program was not run

    VERIFY_STDOUT(expected_path("check_example_audit_plugin_receives_reject.stdout"));
    VERIFY_STR(data.stderr_str, "");

    return true;
}

static int
check_example_audit_plugin_receives_error(void)
{
    create_audit_plugin_options("");
    const char *errstr = NULL;

    str_array_free(&data.plugin_argv);
    data.plugin_argv = create_str_array(5, "sudo", "-u", "user", "id", NULL);

    VERIFY_INT(python_audit->open(SUDO_API_VERSION, fake_conversation, fake_printf,
                                  data.settings, data.user_info, 3, data.plugin_argv,
                                  data.user_env, data.plugin_options, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    str_array_free(&data.command_info);
    data.command_info = create_str_array(2, "command=/sbin/id", NULL);

    VERIFY_INT(python_audit->error("errorer plugin name", SUDO_AUDIT_PLUGIN,
                                   "Some error has happened", data.command_info,
                                   &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    python_audit->close(SUDO_PLUGIN_SUDO_ERROR, 222);

    VERIFY_STDOUT(expected_path("check_example_audit_plugin_receives_error.stdout"));
    VERIFY_STR(data.stderr_str, "");

    return true;
}

typedef struct audit_plugin * (audit_clone_func)(void);

static int
check_example_audit_plugin_workflow_multiple(void)
{
    // verify multiple python audit plugins are available
    audit_clone_func *python_audit_clone = (audit_clone_func *)sudo_dso_findsym(
                python_plugin_handle, "python_audit_clone");
    VERIFY_PTR_NE(python_audit_clone, NULL);

    struct audit_plugin *python_audit2 = NULL;

    for (int i = 0; i < 7; ++i) {
        python_audit2 = (*python_audit_clone)();
        VERIFY_PTR_NE(python_audit2, NULL);
        VERIFY_PTR_NE(python_audit2, python_audit);
    }

    const char *errstr = NULL;

    str_array_free(&data.plugin_argv);
    data.plugin_argv = create_str_array(6, "sudo", "-u", "user", "id", "--help", NULL);

    str_array_free(&data.user_env);
    data.user_env = create_str_array(3, "KEY1=VALUE1", "KEY2=VALUE2", NULL);

    str_array_free(&data.user_info);
    data.user_info = create_str_array(3, "user=default", "uid=1000", NULL);

    create_audit_plugin_options("Id=1");
    VERIFY_INT(python_audit->open(SUDO_API_VERSION, fake_conversation, fake_printf,
                                  data.settings, data.user_info, 3, data.plugin_argv,
                                  data.user_env, data.plugin_options, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    // For verifying the error message of no more plugin. It should be displayed only once.
    VERIFY_PTR((*python_audit_clone)(), NULL);
    VERIFY_PTR((*python_audit_clone)(), NULL);

    create_audit_plugin_options("Id=2");
    VERIFY_INT(python_audit2->open(SUDO_API_VERSION, fake_conversation, fake_printf,
                                  data.settings, data.user_info, 3, data.plugin_argv,
                                  data.user_env, data.plugin_options, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    str_array_free(&data.command_info);
    data.command_info = create_str_array(2, "command=/sbin/id", NULL);

    str_array_free(&data.plugin_argv);
    data.plugin_argv = create_str_array(3, "id", "--help", NULL);

    VERIFY_INT(python_audit->accept("accepter plugin name", SUDO_POLICY_PLUGIN,
                                    data.command_info, data.plugin_argv,
                                    data.user_env, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    VERIFY_INT(python_audit2->accept("accepter plugin name", SUDO_POLICY_PLUGIN,
                                    data.command_info, data.plugin_argv,
                                    data.user_env, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    python_audit->close(SUDO_PLUGIN_WAIT_STATUS, W_EXITCODE(0, 11));  // process got signal 11
    python_audit2->close(SUDO_PLUGIN_WAIT_STATUS, W_EXITCODE(0, 11));

    VERIFY_STDOUT(expected_path("check_example_audit_plugin_workflow_multiple.stdout"));
    VERIFY_STDERR(expected_path("check_example_audit_plugin_workflow_multiple.stderr"));

    return true;
}

static int
check_example_audit_plugin_version_display(void)
{
    create_audit_plugin_options(NULL);
    const char *errstr = NULL;

    str_array_free(&data.user_info);
    data.user_info = create_str_array(3, "user=root", "uid=0", NULL);

    str_array_free(&data.plugin_argv);
    data.plugin_argv = create_str_array(3, "sudo", "-V", NULL);

    VERIFY_INT(python_audit->open(SUDO_API_VERSION, fake_conversation, fake_printf,
                                  data.settings, data.user_info, 2, data.plugin_argv,
                                  data.user_env, data.plugin_options, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    VERIFY_INT(python_audit->show_version(false), SUDO_RC_OK);
    VERIFY_INT(python_audit->show_version(true), SUDO_RC_OK);

    python_audit->close(SUDO_PLUGIN_SUDO_ERROR, 222);

    VERIFY_STDOUT(expected_path("check_example_audit_plugin_version_display.stdout"));
    VERIFY_STR(data.stderr_str, "");

    return true;
}

static int
check_audit_plugin_callbacks_are_optional(void)
{
    const char *errstr = NULL;

    create_debugging_plugin_options();

    VERIFY_INT(python_audit->open(SUDO_API_VERSION, fake_conversation, fake_printf,
                                  data.settings, data.user_info, 2, data.plugin_argv,
                                  data.user_env, data.plugin_options, &errstr),
               SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    VERIFY_PTR(python_audit->accept, NULL);
    VERIFY_PTR(python_audit->reject, NULL);
    VERIFY_PTR(python_audit->error, NULL);

    // show_version always displays the plugin, but it is optional in the python layer
    VERIFY_PTR_NE(python_audit->show_version, NULL);
    VERIFY_INT(python_audit->show_version(1), SUDO_RC_OK);

    python_audit->close(SUDO_PLUGIN_NO_STATUS, 0);
    return true;
}

static int
check_audit_plugin_reports_error(void)
{
    const char *errstr = NULL;
    create_plugin_options("regress/plugin_errorstr", "ConstructErrorPlugin", NULL);

    VERIFY_INT(python_audit->open(SUDO_API_VERSION, fake_conversation, fake_printf,
                                  data.settings, data.user_info, 0, data.plugin_argv,
                                  data.user_env, data.plugin_options, &errstr), SUDO_RC_ERROR);

    VERIFY_STR(errstr, "Something wrong in plugin constructor");
    errstr = NULL;

    python_audit->close(SUDO_PLUGIN_NO_STATUS, 0);

    create_plugin_options("regress/plugin_errorstr", "ErrorMsgPlugin", NULL);

    VERIFY_INT(python_audit->open(SUDO_API_VERSION, fake_conversation, fake_printf,
                                  data.settings, data.user_info, 0, data.plugin_argv,
                                  data.user_env, data.plugin_options, &errstr), SUDO_RC_ERROR);
    VERIFY_STR(errstr, "Something wrong in open");

    errstr = NULL;
    VERIFY_INT(python_audit->accept("plugin name", SUDO_POLICY_PLUGIN,
                                    data.command_info, data.plugin_argv,
                                    data.user_env, &errstr), SUDO_RC_ERROR);
    VERIFY_STR(errstr, "Something wrong in accept");

    errstr = NULL;
    VERIFY_INT(python_audit->reject("plugin name", SUDO_POLICY_PLUGIN,
                                    "audit message", data.command_info,
                                    &errstr), SUDO_RC_ERROR);
    VERIFY_STR(errstr, "Something wrong in reject");

    errstr = NULL;
    VERIFY_INT(python_audit->error("plugin name", SUDO_POLICY_PLUGIN,
                                    "audit message", data.command_info,
                                    &errstr), SUDO_RC_ERROR);
    VERIFY_STR(errstr, "Something wrong in error");

    python_audit->close(SUDO_PLUGIN_NO_STATUS, 0);

    VERIFY_STR(data.stderr_str, "");
    VERIFY_STR(data.stdout_str, "");
    return true;
}

static int
check_example_approval_plugin(const char *date_str, const char *expected_error)
{
    const char *errstr = NULL;

    create_plugin_options("example_approval_plugin", "BusinessHoursApprovalPlugin", NULL);

    VERIFY_INT(python_approval->open(SUDO_API_VERSION, fake_conversation, fake_printf,
                                     data.settings, data.user_info, 0, data.plugin_argv,
                                     data.user_env, data.plugin_options, &errstr), SUDO_RC_OK);

    VERIFY_TRUE(mock_python_datetime_now("example_approval_plugin", date_str));

    int expected_rc = (expected_error == NULL) ? SUDO_RC_ACCEPT : SUDO_RC_REJECT;

    VERIFY_INT(python_approval->check(data.command_info, data.plugin_argv, data.user_env, &errstr),
               expected_rc);

    if (expected_error == NULL) {
        VERIFY_PTR(errstr, NULL);
        VERIFY_STR(data.stdout_str, "");
    } else {
        VERIFY_STR(errstr, expected_error);
        VERIFY_STR_CONTAINS(data.stdout_str, expected_error);  // (ends with \n)
    }
    VERIFY_STR(data.stderr_str, "");

    python_approval->close();

    return true;
}

typedef struct approval_plugin * (approval_clone_func)(void);

static int
check_multiple_approval_plugin_and_arguments(void)
{
    // verify multiple python approval plugins are available
    approval_clone_func *python_approval_clone = (approval_clone_func *)sudo_dso_findsym(
                python_plugin_handle, "python_approval_clone");
    VERIFY_PTR_NE(python_approval_clone, NULL);

    struct approval_plugin *python_approval2 = NULL;

    for (int i = 0; i < 7; ++i) {
        python_approval2 = (*python_approval_clone)();
        VERIFY_PTR_NE(python_approval2, NULL);
        VERIFY_PTR_NE(python_approval2, python_approval);
    }

    const char *errstr = NULL;
    create_plugin_options("regress/plugin_approval_test", "ApprovalTestPlugin", "Id=1");

    str_array_free(&data.plugin_argv);
    data.plugin_argv = create_str_array(6, "sudo", "-u", "user", "whoami", "--help", NULL);

    str_array_free(&data.user_env);
    data.user_env = create_str_array(3, "USER_ENV1=VALUE1", "USER_ENV2=value2", NULL);

    str_array_free(&data.user_info);
    data.user_info = create_str_array(3, "INFO1=VALUE1", "info2=value2", NULL);

    str_array_free(&data.settings);
    data.settings = create_str_array(3, "SETTING1=VALUE1", "setting2=value2", NULL);

    VERIFY_INT(python_approval->open(SUDO_API_VERSION, fake_conversation, fake_printf,
                                     data.settings, data.user_info, 3, data.plugin_argv,
                                     data.user_env, data.plugin_options, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    // For verifying the error message of no more plugin. It should be displayed only once.
    VERIFY_PTR((*python_approval_clone)(), NULL);
    VERIFY_PTR((*python_approval_clone)(), NULL);

    create_plugin_options("regress/plugin_approval_test", "ApprovalTestPlugin", "Id=2");
    VERIFY_INT(python_approval2->open(SUDO_API_VERSION, fake_conversation, fake_printf,
                                      data.settings, data.user_info, 3, data.plugin_argv,
                                      data.user_env, data.plugin_options, &errstr), SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    VERIFY_INT(python_approval->show_version(false), SUDO_RC_OK);
    VERIFY_INT(python_approval2->show_version(true), SUDO_RC_OK);

    str_array_free(&data.command_info);
    data.command_info = create_str_array(3, "CMDINFO1=value1", "CMDINFO2=VALUE2", NULL);

    str_array_free(&data.plugin_argv);
    data.plugin_argv = create_str_array(3, "whoami", "--help", NULL);

    VERIFY_INT(python_approval->check(data.command_info, data.plugin_argv, data.user_env, &errstr),
               SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    VERIFY_INT(python_approval2->check(data.command_info, data.plugin_argv, data.user_env, &errstr),
               SUDO_RC_OK);
    VERIFY_PTR(errstr, NULL);

    python_approval->close();
    python_approval2->close();

    VERIFY_STDOUT(expected_path("check_multiple_approval_plugin_and_arguments.stdout"));
    VERIFY_STDERR(expected_path("check_multiple_approval_plugin_and_arguments.stderr"));

    return true;
}


static int
_init_symbols(void)
{
    if (python_plugin_handle != NULL) {
        // symbols are already loaded, we just restore
        RESTORE_PYTHON_PLUGIN(python_io);
        RESTORE_PYTHON_PLUGIN(python_policy);
        RESTORE_PYTHON_PLUGIN(python_approval);
        RESTORE_PYTHON_PLUGIN(python_audit);
        RESTORE_PYTHON_PLUGIN(group_plugin);
        return true;
    }

    // we load the symbols
    python_plugin_handle = sudo_dso_load(python_plugin_so_path, SUDO_DSO_LAZY|SUDO_DSO_GLOBAL);
    VERIFY_PTR_NE(python_plugin_handle, NULL);

    python_io = sudo_dso_findsym(python_plugin_handle, "python_io");
    VERIFY_PTR_NE(python_io, NULL);

    group_plugin = sudo_dso_findsym(python_plugin_handle, "group_plugin");
    VERIFY_PTR_NE(group_plugin, NULL);

    python_policy = sudo_dso_findsym(python_plugin_handle, "python_policy");
    VERIFY_PTR_NE(python_policy, NULL);

    python_audit = sudo_dso_findsym(python_plugin_handle, "python_audit");
    VERIFY_PTR_NE(python_audit, NULL);

    python_approval = sudo_dso_findsym(python_plugin_handle, "python_approval");
    VERIFY_PTR_NE(python_approval, NULL);

    SAVE_PYTHON_PLUGIN(python_io);
    SAVE_PYTHON_PLUGIN(python_policy);
    SAVE_PYTHON_PLUGIN(python_approval);
    SAVE_PYTHON_PLUGIN(python_audit);
    SAVE_PYTHON_PLUGIN(group_plugin);

    return true;
}

static int
_unlink_symbols(void)
{
    python_io = NULL;
    group_plugin = NULL;
    python_policy = NULL;
    python_approval = NULL;
    python_audit = NULL;
    VERIFY_INT(sudo_dso_unload(python_plugin_handle), 0);
    python_plugin_handle = NULL;
    VERIFY_FALSE(Py_IsInitialized());
    return true;
}

int
main(int argc, char *argv[])
{
    int ch, errors = 0, ntests = 0;

    while ((ch = getopt(argc, argv, "v")) != -1) {
        switch (ch) {
        case 'v':
            verbose = true;
            break;
        default:
            fprintf(stderr, "usage: %s [-v]\n", getprogname());
            return EXIT_FAILURE;
        }
    }
    argc -= optind;
    argv += optind;

    if (argc != 1) {
        printf("Please specify the python_plugin.so as argument!\n");
        return EXIT_FAILURE;
    }
    python_plugin_so_path = argv[0];

    // Unbuffer stdout so we don't miss output during a crash.
    setvbuf(stdout, NULL, _IONBF, 0);

    RUN_TEST(check_example_io_plugin_version_display(true));
    RUN_TEST(check_example_io_plugin_version_display(false));
    RUN_TEST(check_example_io_plugin_command_log());
    RUN_TEST(check_example_io_plugin_command_log_multiple());
    RUN_TEST(check_example_io_plugin_failed_to_start_command());
    RUN_TEST(check_example_io_plugin_fails_with_python_backtrace());
    RUN_TEST(check_io_plugin_callbacks_are_optional());
    RUN_TEST(check_io_plugin_reports_error());
    RUN_TEST(check_plugin_unload());

    RUN_TEST(check_example_group_plugin());
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
    RUN_TEST(check_example_group_plugin_is_able_to_debug());
#endif
    RUN_TEST(check_plugin_unload());

    RUN_TEST(check_loading_fails_with_missing_path());
    RUN_TEST(check_loading_succeeds_with_missing_classname());
    RUN_TEST(check_loading_fails_with_missing_classname());
    RUN_TEST(check_loading_fails_with_wrong_classname());
    RUN_TEST(check_loading_fails_with_wrong_path());
    RUN_TEST(check_plugin_unload());

    RUN_TEST(check_example_conversation_plugin_reason_log(false, "without_suspend"));
    RUN_TEST(check_example_conversation_plugin_reason_log(true, "with_suspend"));
    RUN_TEST(check_example_conversation_plugin_user_interrupts());
    RUN_TEST(check_plugin_unload());

    RUN_TEST(check_example_policy_plugin_version_display(true));
    RUN_TEST(check_example_policy_plugin_version_display(false));
    RUN_TEST(check_example_policy_plugin_accepted_execution());
    RUN_TEST(check_example_policy_plugin_failed_execution());
    RUN_TEST(check_example_policy_plugin_denied_execution());
    RUN_TEST(check_example_policy_plugin_list());
    RUN_TEST(check_example_policy_plugin_validate_invalidate());
    RUN_TEST(check_policy_plugin_callbacks_are_optional());
    RUN_TEST(check_policy_plugin_reports_error());
    RUN_TEST(check_plugin_unload());

    RUN_TEST(check_example_audit_plugin_receives_accept());
    RUN_TEST(check_example_audit_plugin_receives_reject());
    RUN_TEST(check_example_audit_plugin_receives_error());
    RUN_TEST(check_example_audit_plugin_workflow_multiple());
    RUN_TEST(check_example_audit_plugin_version_display());
    RUN_TEST(check_audit_plugin_callbacks_are_optional());
    RUN_TEST(check_audit_plugin_reports_error());
    RUN_TEST(check_plugin_unload());

    // Monday, too early
    RUN_TEST(check_example_approval_plugin(
        "2020-02-10T07:55:23", "That is not allowed outside the business hours!"));
    // Monday, good time
    RUN_TEST(check_example_approval_plugin("2020-02-10T08:05:23", NULL));
    // Friday, good time
    RUN_TEST(check_example_approval_plugin("2020-02-14T17:59:23", NULL));
    // Friday, too late
    RUN_TEST(check_example_approval_plugin(
        "2020-02-10T18:05:23", "That is not allowed outside the business hours!"));
    // Saturday
    RUN_TEST(check_example_approval_plugin(
        "2020-02-15T08:05:23", "That is not allowed on the weekend!"));
    RUN_TEST(check_multiple_approval_plugin_and_arguments());

    RUN_TEST(check_python_plugins_do_not_affect_each_other());
    RUN_TEST(check_plugin_unload());

#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
    RUN_TEST(check_example_debugging("plugin@err"));
    RUN_TEST(check_example_debugging("plugin@info"));
    RUN_TEST(check_example_debugging("load@diag"));
    RUN_TEST(check_example_debugging("sudo_cb@info"));
    RUN_TEST(check_example_debugging("c_calls@diag"));
    RUN_TEST(check_example_debugging("c_calls@info"));
    RUN_TEST(check_example_debugging("py_calls@diag"));
    RUN_TEST(check_example_debugging("py_calls@info"));
    RUN_TEST(check_example_debugging("plugin@err"));
    RUN_TEST(check_plugin_unload());
#endif /* FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION */

    if (ntests != 0) {
        printf("%s: %d tests run, %d errors, %d%% success rate\n",
            getprogname(), ntests, errors, (ntests - errors) * 100 / ntests);
    }

    return errors;
}
