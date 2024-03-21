/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2010-2016, 2022 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif /* HAVE_STDBOOL_H */
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif /* HAVE_STRINGS_H */
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <grp.h>
#include <pwd.h>

#include <pathnames.h>
#include <sudo_compat.h>
#include <sudo_plugin.h>
#include <sudo_util.h>

/*
 * Sample plugin module that allows any user who knows the password
 * ("test") to run any command as root.  Since there is no credential
 * caching the validate and invalidate functions are NULL.
 */

static struct plugin_state {
    char **envp;
    char * const *settings;
    char * const *user_info;
} plugin_state;
static sudo_conv_t sudo_conv;
static sudo_printf_t sudo_log;
static FILE *input, *output;
static uid_t runas_uid = ROOT_UID;
static gid_t runas_gid = (gid_t)-1;
static int use_sudoedit = false;

/*
 * Plugin policy open function.
 */
static int
policy_open(unsigned int version, sudo_conv_t conversation,
    sudo_printf_t sudo_plugin_printf, char * const settings[],
    char * const user_info[], char * const user_env[], char * const args[],
    const char **errstr)
{
    char * const *ui;
    struct passwd *pw;
    const char *runas_user = NULL;
    struct group *gr;
    const char *runas_group = NULL;

    if (!sudo_conv)
	sudo_conv = conversation;
    if (!sudo_log)
	sudo_log = sudo_plugin_printf;

    if (SUDO_API_VERSION_GET_MAJOR(version) != SUDO_API_VERSION_MAJOR) {
	sudo_log(SUDO_CONV_ERROR_MSG,
	    "the sample plugin requires API version %d.x\n",
	    SUDO_API_VERSION_MAJOR);
	return -1;
    }

    /* Only allow commands to be run as root. */
    for (ui = settings; *ui != NULL; ui++) {
	if (strncmp(*ui, "runas_user=", sizeof("runas_user=") - 1) == 0) {
	    runas_user = *ui + sizeof("runas_user=") - 1;
	}
	if (strncmp(*ui, "runas_group=", sizeof("runas_group=") - 1) == 0) {
	    runas_group = *ui + sizeof("runas_group=") - 1;
	}
	if (strncmp(*ui, "progname=", sizeof("progname=") - 1) == 0) {
	    initprogname(*ui + sizeof("progname=") - 1);
	}
	/* Check to see if sudo was called as sudoedit or with -e flag. */
	if (strncmp(*ui, "sudoedit=", sizeof("sudoedit=") - 1) == 0) {
	    if (strcasecmp(*ui + sizeof("sudoedit=") - 1, "true") == 0)
		use_sudoedit = true;
	}
	/* This plugin doesn't support running sudo with no arguments. */
	if (strncmp(*ui, "implied_shell=", sizeof("implied_shell=") - 1) == 0) {
	    if (strcasecmp(*ui + sizeof("implied_shell=") - 1, "true") == 0)
		return -2; /* usage error */
	}
    }
    if (runas_user != NULL) {
	if ((pw = getpwnam(runas_user)) == NULL) {
	    sudo_log(SUDO_CONV_ERROR_MSG, "unknown user %s\n", runas_user);
	    return 0;
	}
	runas_uid = pw->pw_uid;
    }
    if (runas_group != NULL) {
	if ((gr = getgrnam(runas_group)) == NULL) {
	    sudo_log(SUDO_CONV_ERROR_MSG, "unknown group %s\n", runas_group);
	    return 0;
	}
	runas_gid = gr->gr_gid;
    }

    /* Plugin state. */
    plugin_state.envp = (char **)user_env;
    plugin_state.settings = settings;
    plugin_state.user_info = user_info;

    return 1;
}

static char *
find_in_path(char *command, char **envp)
{
    struct stat sb;
    char *path = NULL;
    char *path0, **ep, *cp;
    char pathbuf[PATH_MAX], *qualified = NULL;

    if (strchr(command, '/') != NULL)
	return command;

    for (ep = plugin_state.envp; *ep != NULL; ep++) {
	if (strncmp(*ep, "PATH=", 5) == 0) {
	    path = *ep + 5;
	    break;
	}
    }
    path = path0 = strdup(path ? path : _PATH_DEFPATH);
    do {
	if ((cp = strchr(path, ':')))
	    *cp = '\0';
	snprintf(pathbuf, sizeof(pathbuf), "%s/%s", *path ? path : ".",
	    command);
	if (stat(pathbuf, &sb) == 0) {
	    if (S_ISREG(sb.st_mode) && (sb.st_mode & 0000111)) {
		qualified = pathbuf;
		break;
	    }
	}
	path = cp + 1;
    } while (cp != NULL);
    free(path0);
    return qualified ? strdup(qualified) : NULL;
}

static int
check_passwd(void)
{
    struct sudo_conv_message msg;
    struct sudo_conv_reply repl;

    /* Prompt user for password via conversation function. */
    memset(&msg, 0, sizeof(msg));
    msg.msg_type = SUDO_CONV_PROMPT_ECHO_OFF;
    msg.msg = "Password: ";
    memset(&repl, 0, sizeof(repl));
    sudo_conv(1, &msg, &repl, NULL);
    if (repl.reply == NULL) {
	sudo_log(SUDO_CONV_ERROR_MSG, "missing password\n");
	return false;
    }
    if (strcmp(repl.reply, "test") != 0) {
	sudo_log(SUDO_CONV_ERROR_MSG, "incorrect password\n");
	return false;
    }
    return true;
}

static char **
build_command_info(const char *command)
{
    char **command_info;
    int i = 0;

    /* Setup command info. */
    command_info = calloc(32, sizeof(char *));
    if (command_info == NULL)
	goto oom;
    if ((command_info[i] = sudo_new_key_val("command", command)) == NULL)
	goto oom;
    i++;
    if (asprintf(&command_info[i], "runas_euid=%ld", (long)runas_uid) == -1)
	goto oom;
    i++;
    if (asprintf(&command_info[i++], "runas_uid=%ld", (long)runas_uid) == -1)
	goto oom;
    i++;
    if (runas_gid != (gid_t)-1) {
	if (asprintf(&command_info[i++], "runas_gid=%ld", (long)runas_gid) == -1)
	    goto oom;
	i++;
	if (asprintf(&command_info[i++], "runas_egid=%ld", (long)runas_gid) == -1)
	    goto oom;
	i++;
    }
#ifdef USE_TIMEOUT
    if ((command_info[i] = strdup("timeout=30")) == NULL)
	goto oom;
    i++;
#endif
    if (use_sudoedit) {
	if ((command_info[i] = strdup("sudoedit=true")) == NULL)
	    goto oom;
    }
    return command_info;
oom:
    while (i > 0) {
	free(command_info[i--]);
    }
    free(command_info);
    return NULL;
}

static char *
find_editor(int nfiles, char * const files[], char **argv_out[])
{
    char *cp, *last, **ep, **nargv, *editor_path;
    char *editor = NULL;
    int ac, i, nargc, wasblank;

    /* Lookup EDITOR in user's environment. */
    for (ep = plugin_state.envp; *ep != NULL; ep++) {
	if (strncmp(*ep, "EDITOR=", 7) == 0) {
	    editor = *ep + 7;
	    break;
	}
    }
    editor = strdup(editor ? editor : _PATH_VI);
    if (editor == NULL) {
	sudo_log(SUDO_CONV_ERROR_MSG, "unable to allocate memory\n");
	return NULL;
    }

    /*
     * Split editor into an argument vector; editor is reused (do not free).
     * The EDITOR environment variables may contain command
     * line args so look for those and alloc space for them too.
     */
    nargc = 1;
    for (wasblank = 0, cp = editor; *cp != '\0'; cp++) {
	if (isblank((unsigned char) *cp))
	    wasblank = 1;
	else if (wasblank) {
	    wasblank = 0;
	    nargc++;
	}
    }
    /* If we can't find the editor in the user's PATH, give up. */
    cp = strtok_r(editor, " \t", &last);
    if (cp == NULL ||
	(editor_path = find_in_path(editor, plugin_state.envp)) == NULL) {
	free(editor);
	return NULL;
    }
    if (editor_path != editor)
	free(editor);
    nargv = reallocarray(NULL, (size_t)nargc + 1 + (size_t)nfiles + 1,
	sizeof(char *));
    if (nargv == NULL) {
	sudo_log(SUDO_CONV_ERROR_MSG, "unable to allocate memory\n");
	free(editor_path);
	return NULL;
    }
    for (ac = 0; cp != NULL && ac < nargc; ac++) {
	nargv[ac] = cp;
	cp = strtok_r(NULL, " \t", &last);
    }
    nargv[ac++] = (char *)"--";
    for (i = 0; i < nfiles; )
	nargv[ac++] = files[i++];
    nargv[ac] = NULL;

    *argv_out = nargv;
    return editor_path;
}

/*
 * Plugin policy check function.
 * Simple example that prompts for a password, hard-coded to "test".
 */
static int 
policy_check(int argc, char * const argv[],
    char *env_add[], char **command_info_out[],
    char **argv_out[], char **user_env_out[], const char **errstr)
{
    char *command;

    if (!argc || argv[0] == NULL) {
	sudo_log(SUDO_CONV_ERROR_MSG, "no command specified\n");
	return false;
    }

    if (!check_passwd())
	return false;

    command = find_in_path(argv[0], plugin_state.envp);
    if (command == NULL) {
	sudo_log(SUDO_CONV_ERROR_MSG, "%s: command not found\n", argv[0]);
	return false;
    }

    /* If "sudo vi" is run, auto-convert to sudoedit.  */
    if (strcmp(command, _PATH_VI) == 0)
	use_sudoedit = true;

    if (use_sudoedit) {
	/* Rebuild argv using editor */
	free(command);
	command = find_editor(argc - 1, argv + 1, argv_out);
	if (command == NULL) {
	    sudo_log(SUDO_CONV_ERROR_MSG, "unable to find valid editor\n");
	    return -1;
	}
	use_sudoedit = true;
    } else {
	/* No changes needd to argv */
	*argv_out = (char **)argv;
    }

    /* No changes to envp */
    *user_env_out = plugin_state.envp;

    /* Setup command info. */
    *command_info_out = build_command_info(command);
    free(command);
    if (*command_info_out == NULL) {
	sudo_log(SUDO_CONV_ERROR_MSG, "out of memory\n");
	return -1;
    }

    return true;
}

static int
policy_list(int argc, char * const argv[], int verbose, const char *list_user,
    const char **errstr)
{
    /*
     * List user's capabilities.
     */
    sudo_log(SUDO_CONV_INFO_MSG, "Validated users may run any command\n");
    return true;
}

static int
policy_version(int verbose)
{
    sudo_log(SUDO_CONV_INFO_MSG, "Sample policy plugin version %s\n",
	PACKAGE_VERSION);
    return true;
}

static void
policy_close(int exit_status, int error)
{
    /*
     * The policy might log the command exit status here.
     * In this example, we just print a message.
     */
    if (error) {
	sudo_log(SUDO_CONV_ERROR_MSG, "Command error: %s\n", strerror(error));
    } else {
        if (WIFEXITED(exit_status)) {
	    sudo_log(SUDO_CONV_INFO_MSG, "Command exited with status %d\n",
		WEXITSTATUS(exit_status));
        } else if (WIFSIGNALED(exit_status)) {
	    sudo_log(SUDO_CONV_INFO_MSG, "Command killed by signal %d\n",
		WTERMSIG(exit_status));
	}
    }
}

static int
io_open(unsigned int version, sudo_conv_t conversation,
    sudo_printf_t sudo_plugin_printf, char * const settings[],
    char * const user_info[], char * const command_info[],
    int argc, char * const argv[], char * const user_env[], char * const args[],
    const char **errstr)
{
    int fd;
    char path[PATH_MAX];

    if (!sudo_conv)
	sudo_conv = conversation;
    if (!sudo_log)
	sudo_log = sudo_plugin_printf;

    /* Open input and output files. */
    snprintf(path, sizeof(path), "/var/tmp/sample-%u.output",
	(unsigned int)getpid());
    fd = open(path, O_WRONLY|O_CREAT|O_EXCL, 0644);
    if (fd == -1)
	return false;
    output = fdopen(fd, "w");

    snprintf(path, sizeof(path), "/var/tmp/sample-%u.input",
	(unsigned int)getpid());
    fd = open(path, O_WRONLY|O_CREAT|O_EXCL, 0644);
    if (fd == -1)
	return false;
    input = fdopen(fd, "w");

    return true;
}

static void
io_close(int exit_status, int error)
{
    fclose(input);
    fclose(output);
}

static int
io_version(int verbose)
{
    sudo_log(SUDO_CONV_INFO_MSG, "Sample I/O plugin version %s\n",
	PACKAGE_VERSION);
    return true;
}

static int
io_log_input(const char *buf, unsigned int len, const char **errstr)
{
    ignore_result(fwrite(buf, len, 1, input));
    return true;
}

static int
io_log_output(const char *buf, unsigned int len, const char **errstr)
{
    const char *cp, *ep;
    bool ret = true;

    ignore_result(fwrite(buf, len, 1, output));
    /*
     * If we find the string "honk!" in the buffer, reject it.
     * In practice we'd want to be able to detect the word
     * broken across two buffers.
     */
    for (cp = buf, ep = buf + len; cp < ep; cp++) {
	if (cp + 5 < ep && memcmp(cp, "honk!", 5) == 0) {
	    ret = false;
	    break;
	}
    }
    return ret;
}

sudo_dso_public struct policy_plugin sample_policy = {
    SUDO_POLICY_PLUGIN,
    SUDO_API_VERSION,
    policy_open,
    policy_close,
    policy_version,
    policy_check,
    policy_list,
    NULL, /* validate */
    NULL, /* invalidate */
    NULL, /* init_session */
    NULL, /* register_hooks */
    NULL, /* deregister_hooks */
    NULL /* event_alloc() filled in by sudo */
};

/*
 * Note: This plugin does not differentiate between tty and pipe I/O.
 *       It all gets logged to the same file.
 */
sudo_dso_public struct io_plugin sample_io = {
    SUDO_IO_PLUGIN,
    SUDO_API_VERSION,
    io_open,
    io_close,
    io_version,
    io_log_input,	/* tty input */
    io_log_output,	/* tty output */
    io_log_input,	/* command stdin if not tty */
    io_log_output,	/* command stdout if not tty */
    io_log_output,	/* command stderr if not tty */
    NULL, /* register_hooks */
    NULL, /* deregister_hooks */
    NULL, /* change_winsize */
    NULL, /* log_suspend */
    NULL /* event_alloc() filled in by sudo */
};
