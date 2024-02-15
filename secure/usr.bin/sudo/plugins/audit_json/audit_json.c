/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2020-2021 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <config.h>

#include <sys/stat.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif /* HAVE_STDBOOL_H */
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <time.h>

#include <pathnames.h>
#include <sudo_compat.h>
#include <sudo_conf.h>
#include <sudo_debug.h>
#include <sudo_dso.h>
#include <sudo_fatal.h>
#include <sudo_gettext.h>
#include <sudo_json.h>
#include <sudo_plugin.h>
#include <sudo_util.h>

static int audit_debug_instance = SUDO_DEBUG_INSTANCE_INITIALIZER;
static sudo_conv_t audit_conv;
static sudo_printf_t audit_printf;

static struct audit_state {
    int submit_optind;
    char uuid_str[37];
    bool accepted;
    FILE *log_fp;
    char *logfile;
    char * const * settings;
    char * const * user_info;
    char * const * submit_argv;
    char * const * submit_envp;
} state = { -1 };

/* Filter out entries in settings[] that are not really options. */
const char * const settings_filter[] = {
    "debug_flags",
    "max_groups",
    "network_addrs",
    "plugin_dir",
    "plugin_path",
    "progname",
    NULL
};

static int
audit_json_open(unsigned int version, sudo_conv_t conversation,
    sudo_printf_t plugin_printf, char * const settings[],
    char * const user_info[], int submit_optind, char * const submit_argv[],
    char * const submit_envp[], char * const plugin_options[],
    const char **errstr)
{
    struct sudo_conf_debug_file_list debug_files =
	TAILQ_HEAD_INITIALIZER(debug_files);
    struct sudo_debug_file *debug_file;
    const char *cp, *plugin_path = NULL;
    unsigned char uuid[16];
    char * const *cur;
    mode_t oldmask;
    int fd, ret = -1;
    debug_decl_vars(audit_json_open, SUDO_DEBUG_PLUGIN);

    audit_conv = conversation;
    audit_printf = plugin_printf;

    /*
     * Stash initial values.
     */
    state.submit_optind = submit_optind;
    state.settings = settings;
    state.user_info = user_info;
    state.submit_argv = submit_argv;
    state.submit_envp = submit_envp;

    /* Initialize the debug subsystem.  */
    for (cur = settings; (cp = *cur) != NULL; cur++) {
        if (strncmp(cp, "debug_flags=", sizeof("debug_flags=") - 1) == 0) {
            cp += sizeof("debug_flags=") - 1;
            if (sudo_debug_parse_flags(&debug_files, cp) == -1)
                goto oom;
            continue;
        }
        if (strncmp(cp, "plugin_path=", sizeof("plugin_path=") - 1) == 0) {
            plugin_path = cp + sizeof("plugin_path=") - 1;
            continue;
        }
    }
    if (plugin_path != NULL && !TAILQ_EMPTY(&debug_files)) {
	audit_debug_instance =
	    sudo_debug_register(plugin_path, NULL, NULL, &debug_files, -1);
	if (audit_debug_instance == SUDO_DEBUG_INSTANCE_ERROR) {
	    *errstr = U_("unable to initialize debugging");
	    goto bad;
	}
	sudo_debug_enter(__func__, __FILE__, __LINE__, sudo_debug_subsys);
    }

    /* Create a UUID for this command for use with audit records. */
    sudo_uuid_create(uuid);
    if (sudo_uuid_to_string(uuid, state.uuid_str, sizeof(state.uuid_str)) == NULL) {
	*errstr = U_("unable to generate UUID");
	goto bad;
    }

    /* Parse plugin_options to check for logfile option. */
    if (plugin_options != NULL) {
	for (cur = plugin_options; (cp = *cur) != NULL; cur++) {
	    if (strncmp(cp, "logfile=", sizeof("logfile=") - 1) == 0) {
		state.logfile = strdup(cp + sizeof("logfile=") - 1);
		if (state.logfile == NULL)
		    goto oom;
	    }
	}
    }
    if (state.logfile == NULL) {
	if (asprintf(&state.logfile, "%s/sudo_audit.json", _PATH_SUDO_LOGDIR) == -1)
	    goto oom;
    }

    /* open log file */
    /* TODO: support pipe */
    oldmask = umask(S_IRWXG|S_IRWXO);
    fd = open(state.logfile, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR);
    (void)umask(oldmask);
    if (fd == -1 || (state.log_fp = fdopen(fd, "w")) == NULL) {
	*errstr = U_("unable to open audit system");
	if (fd != -1)
	    close(fd);
	goto bad;
    }

    ret = 1;
    goto done;

oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    *errstr = U_("unable to allocate memory");

bad:
    if (state.log_fp != NULL) {
	fclose(state.log_fp);
	state.log_fp = NULL;
    }

done:
    while ((debug_file = TAILQ_FIRST(&debug_files))) {
	TAILQ_REMOVE(&debug_files, debug_file, entries);
	free(debug_file->debug_file);
	free(debug_file->debug_flags);
	free(debug_file);
    }

    debug_return_int(ret);
}

static bool
add_key_value(struct json_container *jsonc, const char *str)
{
    struct json_value json_value;
    const char *cp, *errstr;
    char name[256];
    size_t len;
    debug_decl(add_key_value, SUDO_DEBUG_PLUGIN);

    if ((cp = strchr(str, '=')) == NULL) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "ignoring bad command info string \"%s\"", str);
	debug_return_bool(false);
    }
    len = (size_t)(cp - str);
    cp++;

    /* Variable name currently limited to 256 chars */
    if (len >= sizeof(name)) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "ignoring long command info name \"%.*s\"", (int)len, str);
	debug_return_bool(false);
    }
    memcpy(name, str, len);
    name[len] = '\0';

    /* Check for bool or number. */
    json_value.type = JSON_NULL;
    switch (cp[0]) {
    case '0':
	if (cp[1] == '\0') {
	    /* Only treat a plain "0" as number 0. */
	    json_value.u.number = 0;
	    json_value.type = JSON_NUMBER;
	}
	break;
    case '+': case '-':
	if (cp[1] == '0') {
	    /* Encode octal numbers as strings. */
	    break;
	}
	FALLTHROUGH;
    case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9':
	json_value.u.number = sudo_strtonum(cp, INT_MIN, INT_MAX, &errstr);
	if (errstr == NULL)
	    json_value.type = JSON_NUMBER;
	break;
    case 't':
	if (strcmp(cp, "true") == 0) {
	    json_value.type = JSON_BOOL;
	    json_value.u.boolean = true;
	}
	break;
    case 'f':
	if (strcmp(cp, "false") == 0) {
	    json_value.type = JSON_BOOL;
	    json_value.u.boolean = false;
	}
	break;
    }

    /* Default to string type. */
    if (json_value.type == JSON_NULL) {
	json_value.type = JSON_STRING;
	json_value.u.string = cp;
    }

    debug_return_bool(sudo_json_add_value(jsonc, name, &json_value));
}

static bool
add_array(struct json_container *jsonc, const char *name, char * const * array)
{
    const char *cp;
    struct json_value json_value;
    debug_decl(add_array, SUDO_DEBUG_PLUGIN);

    if (!sudo_json_open_array(jsonc, name))
	debug_return_bool(false);
    while ((cp = *array) != NULL) {
	json_value.type = JSON_STRING;
	json_value.u.string = cp;
	if (!sudo_json_add_value(jsonc, name, &json_value))
	    debug_return_bool(false);
	array++;
    }
    if (!sudo_json_close_array(jsonc))
	debug_return_bool(false);

    debug_return_bool(true);
}

static bool
filter_key_value(const char *kv, const char * const * filter)
{
    const char * const *cur;
    const char *cp;
    size_t namelen;

    if (filter != NULL) {
	namelen = strcspn(kv, "=");
	for (cur = filter; (cp = *cur) != NULL; cur++) {
	    if (strncmp(kv, cp, namelen) == 0 && cp[namelen] == '\0')
		return true;
	}
    }
    return false;
}

static bool
add_key_value_object(struct json_container *jsonc, const char *name,
	char * const * array, const char * const * filter)
{
    char * const *cur;
    const char *cp;
    bool empty = false;
    debug_decl(add_key_value_object, SUDO_DEBUG_PLUGIN);

    if (filter != NULL) {
	/* Avoid printing an empty object if everything is filtered. */
	empty = true;
	for (cur = array; (cp = *cur) != NULL; cur++) {
	    if (!filter_key_value(cp, filter)) {
		empty = false;
		break;
	    }
	}
    }
    if (!empty) {
	if (!sudo_json_open_object(jsonc, name))
	    goto bad;
	for (cur = array; (cp = *cur) != NULL; cur++) {
	    if (filter_key_value(cp, filter))
		continue;
	    if (!add_key_value(jsonc, cp))
		goto bad;
	}
	if (!sudo_json_close_object(jsonc))
	    goto bad;
    }

    debug_return_bool(true);
bad:
    debug_return_bool(false);
}

static bool
add_timestamp(struct json_container *jsonc, struct timespec *ts)
{
    struct json_value json_value;
    time_t secs = ts->tv_sec;
    char timebuf[1024];
    struct tm gmt;
    size_t len;
    debug_decl(add_timestamp, SUDO_DEBUG_PLUGIN);

    if (gmtime_r(&secs, &gmt) == NULL)
	debug_return_bool(false);

    sudo_json_open_object(jsonc, "timestamp");

    json_value.type = JSON_NUMBER;
    json_value.u.number = ts->tv_sec;
    sudo_json_add_value(jsonc, "seconds", &json_value);

    json_value.type = JSON_NUMBER;
    json_value.u.number = ts->tv_nsec;
    sudo_json_add_value(jsonc, "nanoseconds", &json_value);

    timebuf[sizeof(timebuf) - 1] = '\0';
    len = strftime(timebuf, sizeof(timebuf), "%Y%m%d%H%M%SZ", &gmt);
    if (len != 0 && timebuf[sizeof(timebuf) - 1] == '\0'){
	json_value.type = JSON_STRING;
	json_value.u.string = timebuf;
	sudo_json_add_value(jsonc, "iso8601", &json_value);
    }

    timebuf[sizeof(timebuf) - 1] = '\0';
    len = strftime(timebuf, sizeof(timebuf), "%a %b %e %H:%M:%S %Z %Y", &gmt);
    if (len != 0 && timebuf[sizeof(timebuf) - 1] == '\0'){
	json_value.type = JSON_STRING;
	json_value.u.string = timebuf;
	sudo_json_add_value(jsonc, "localtime", &json_value);
    }

    sudo_json_close_object(jsonc);

    debug_return_bool(true);
}

static int
audit_write_json(struct json_container * restrict jsonc)
{
    struct stat sb;
    int ret = -1;
    debug_decl(audit_write_json, SUDO_DEBUG_PLUGIN);

    if (!sudo_lock_file(fileno(state.log_fp), SUDO_LOCK)) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO|SUDO_DEBUG_LINENO,
	    "unable to lock %s", state.logfile);
	goto done;
    }

    /* Note: assumes file ends in "\n}\n" */
    if (fstat(fileno(state.log_fp), &sb) == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO|SUDO_DEBUG_LINENO,
	    "unable to stat %s", state.logfile);
	goto done;
    }
    if (sb.st_size == 0) {
	/* New file */
	putc('{', state.log_fp);
    } else if (fseeko(state.log_fp, -3, SEEK_END) == 0) {
	/* Continue file, overwrite the final "\n}\n" */
	putc(',', state.log_fp);
    } else {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO|SUDO_DEBUG_LINENO,
	    "unable to seek %s", state.logfile);
	goto done;
    }

    fputs(sudo_json_get_buf(jsonc), state.log_fp);
    fputs("\n}\n", state.log_fp);
    fflush(state.log_fp);
    (void)sudo_lock_file(fileno(state.log_fp), SUDO_UNLOCK);

    /* TODO: undo partial record on error */
    if (!ferror(state.log_fp))
	ret = true;

done:
    debug_return_int(ret);
}

static int
audit_write_exit_record(int exit_status, int error)
{
    struct json_container jsonc;
    struct json_value json_value;
    struct timespec now;
    int ret = -1;
    debug_decl(audit_write_exit_record, SUDO_DEBUG_PLUGIN);

    if (sudo_gettime_real(&now) == -1) {
	sudo_warn("%s", U_("unable to read the clock"));
	goto done;
    }

    if (!sudo_json_init(&jsonc, 4, false, false, false))
	goto oom;
    if (!sudo_json_open_object(&jsonc, "exit"))
	goto oom;

    /* Write UUID */
    json_value.type = JSON_STRING;
    json_value.u.string = state.uuid_str;
    if (!sudo_json_add_value(&jsonc, "uuid", &json_value))
	goto oom;

    /* Write time stamp */
    if (!add_timestamp(&jsonc, &now))
	goto oom;

    if (error != 0) {
	/* Error executing command */
	json_value.type = JSON_STRING;
	json_value.u.string = strerror(error);
	if (!sudo_json_add_value(&jsonc, "error", &json_value))
	    goto oom;
    } else {
        if (WIFEXITED(exit_status)) {
	    /* Command exited normally. */
	    json_value.type = JSON_NUMBER;
	    json_value.u.number = WEXITSTATUS(exit_status);
	    if (!sudo_json_add_value(&jsonc, "exit_value", &json_value))
		goto oom;
        } else if (WIFSIGNALED(exit_status)) {
	    /* Command killed by signal. */
	    char signame[SIG2STR_MAX];
            int signo = WTERMSIG(exit_status);
            if (signo <= 0 || sig2str(signo, signame) == -1) {
		json_value.type = JSON_NUMBER;
		json_value.u.number = signo;
		if (!sudo_json_add_value(&jsonc, "signal", &json_value))
		    goto oom;
            } else {
		json_value.type = JSON_STRING;
		json_value.u.string = signame; // -V507
		if (!sudo_json_add_value(&jsonc, "signal", &json_value))
		    goto oom;
	    }
	    /* Core dump? */
	    json_value.type = JSON_BOOL;
	    json_value.u.boolean = WCOREDUMP(exit_status);
	    if (!sudo_json_add_value(&jsonc, "dumped_core", &json_value))
		goto oom;
	    /* Exit value */
	    json_value.type = JSON_NUMBER;
	    json_value.u.number = WTERMSIG(exit_status) | 128;
	    if (!sudo_json_add_value(&jsonc, "exit_value", &json_value))
		goto oom;
        }
    }

    if (!sudo_json_close_object(&jsonc))
	goto oom;

    ret = audit_write_json(&jsonc);
    sudo_json_free(&jsonc);
done:
    debug_return_int(ret);
oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    sudo_json_free(&jsonc);
    debug_return_int(-1);
}

static int
audit_write_record(const char *audit_str, const char *plugin_name,
    unsigned int plugin_type, const char *reason, char * const command_info[],
    char * const run_argv[], char * const run_envp[])
{
    struct json_container jsonc;
    struct json_value json_value;
    struct timespec now;
    int ret = -1;
    debug_decl(audit_write_record, SUDO_DEBUG_PLUGIN);

    if (sudo_gettime_real(&now) == -1) {
	sudo_warn("%s", U_("unable to read the clock"));
	goto done;
    }

    if (!sudo_json_init(&jsonc, 4, false, false, false))
	goto oom;
    if (!sudo_json_open_object(&jsonc, audit_str))
	goto oom;

    json_value.type = JSON_STRING;
    json_value.u.string = plugin_name;
    if (!sudo_json_add_value(&jsonc, "plugin_name", &json_value))
	goto oom;

    switch (plugin_type) {
    case SUDO_FRONT_END:
	json_value.u.string = "front-end";
	break;
    case SUDO_POLICY_PLUGIN:
	json_value.u.string = "policy";
	break;
    case SUDO_IO_PLUGIN:
	json_value.u.string = "io";
	break;
    case SUDO_APPROVAL_PLUGIN:
	json_value.u.string = "approval";
	break;
    case SUDO_AUDIT_PLUGIN:
	json_value.u.string = "audit";
	break;
    default:
	json_value.u.string = "unknown";
	break;
    }
    json_value.type = JSON_STRING;
    if (!sudo_json_add_value(&jsonc, "plugin_type", &json_value))
	goto oom;

    /* error and reject audit events usually contain a reason. */
    if (reason != NULL) {
	json_value.type = JSON_STRING;
	json_value.u.string = reason;
	if (!sudo_json_add_value(&jsonc, "reason", &json_value))
	    goto oom;
    }

    json_value.type = JSON_STRING;
    json_value.u.string = state.uuid_str;
    if (!sudo_json_add_value(&jsonc, "uuid", &json_value))
	goto oom;

    if (!add_timestamp(&jsonc, &now))
	goto oom;

    /* Write key=value objects. */
    if (state.settings != NULL) {
	if (!add_key_value_object(&jsonc, "options", state.settings, settings_filter))
	    goto oom;
    } else {
	sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
	    "missing settings list");
    }
    if (state.user_info != NULL) {
	if (!add_key_value_object(&jsonc, "user_info", state.user_info, NULL))
	    goto oom;
    } else {
	sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
	    "missing user_info list");
    }
    if (command_info != NULL) {
	if (!add_key_value_object(&jsonc, "command_info", command_info, NULL))
	    goto oom;
    }

    /* Write submit_optind before submit_argv */
    json_value.type = JSON_NUMBER;
    json_value.u.number = state.submit_optind;
    if (!sudo_json_add_value(&jsonc, "submit_optind", &json_value))
	goto oom;

    if (state.submit_argv != NULL) {
	if (!add_array(&jsonc, "submit_argv", state.submit_argv))
	    goto oom;
    } else {
	sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
	    "missing submit_argv array");
    }
    if (state.submit_envp != NULL) {
	if (!add_array(&jsonc, "submit_envp", state.submit_envp))
	    goto oom;
    } else {
	sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
	    "missing submit_envp array");
    }
    if (run_argv != NULL) {
	if (!add_array(&jsonc, "run_argv", run_argv))
	    goto oom;
    }
    if (run_envp != NULL) {
	if (!add_array(&jsonc, "run_envp", run_envp))
	    goto oom;
    }

    if (!sudo_json_close_object(&jsonc))
	goto oom;

    ret = audit_write_json(&jsonc);
    sudo_json_free(&jsonc);

done:
    debug_return_int(ret);
oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    sudo_json_free(&jsonc);
    debug_return_int(-1);
}

static int
audit_json_accept(const char *plugin_name, unsigned int plugin_type,
    char * const command_info[], char * const run_argv[],
    char * const run_envp[], const char **errstr)
{
    int ret;
    debug_decl(audit_json_accept, SUDO_DEBUG_PLUGIN);

    /* Ignore the extra accept event from the sudo front-end. */
    if (plugin_type == SUDO_FRONT_END)
	debug_return_int(true);

    state.accepted = true;

    ret = audit_write_record("accept", plugin_name, plugin_type, NULL,
	command_info, run_argv, run_envp);

    debug_return_int(ret);
}

static int
audit_json_reject(const char *plugin_name, unsigned int plugin_type,
    const char *reason, char * const command_info[], const char **errstr)
{
    int ret;
    debug_decl(audit_json_reject, SUDO_DEBUG_PLUGIN);

    ret = audit_write_record("reject", plugin_name, plugin_type,
	reason, command_info, NULL, NULL);

    debug_return_int(ret);
}

static int
audit_json_error(const char *plugin_name, unsigned int plugin_type,
    const char *reason, char * const command_info[], const char **errstr)
{
    int ret;
    debug_decl(audit_json_error, SUDO_DEBUG_PLUGIN);

    ret = audit_write_record("error", plugin_name, plugin_type,
	reason, command_info, NULL, NULL);

    debug_return_int(ret);
}

static void
audit_json_close(int status_type, int status)
{
    debug_decl(audit_json_close, SUDO_DEBUG_PLUGIN);

    switch (status_type) {
    case SUDO_PLUGIN_NO_STATUS:
	break;
    case SUDO_PLUGIN_WAIT_STATUS:
	audit_write_exit_record(status, 0);
	break;
    case SUDO_PLUGIN_EXEC_ERROR:
	audit_write_exit_record(0, status);
	break;
    case SUDO_PLUGIN_SUDO_ERROR:
	audit_write_record("error", "sudo", 0, strerror(status),
	    NULL, NULL, NULL);
	break;
    default:
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "unexpected status type %d, value %d", status_type, status);
	break;
    }

    free(state.logfile);
    if (state.log_fp != NULL)
	fclose(state.log_fp);

    debug_return;
}

static int
audit_json_show_version(int verbose)
{
    debug_decl(audit_json_show_version, SUDO_DEBUG_PLUGIN);

    audit_printf(SUDO_CONV_INFO_MSG, "JSON audit plugin version %s\n",
        PACKAGE_VERSION);

    debug_return_int(true);
}

sudo_dso_public struct audit_plugin audit_json = {
    SUDO_AUDIT_PLUGIN,
    SUDO_API_VERSION,
    audit_json_open,
    audit_json_close,
    audit_json_accept,
    audit_json_reject,
    audit_json_error,
    audit_json_show_version,
    NULL, /* register_hooks */
    NULL /* deregister_hooks */
};
