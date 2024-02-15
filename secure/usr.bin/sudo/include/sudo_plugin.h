/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2009-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef SUDO_PLUGIN_H
#define SUDO_PLUGIN_H

/* API version major/minor */
#define SUDO_API_VERSION_MAJOR 1
#define SUDO_API_VERSION_MINOR 21
#define SUDO_API_MKVERSION(x, y) (((x) << 16) | (y))
#define SUDO_API_VERSION SUDO_API_MKVERSION(SUDO_API_VERSION_MAJOR, SUDO_API_VERSION_MINOR)

/* Getters and setters for plugin API versions */
#define SUDO_API_VERSION_GET_MAJOR(v) ((v) >> 16)
#define SUDO_API_VERSION_GET_MINOR(v) ((v) & 0xffffU)
#define SUDO_API_VERSION_SET_MAJOR(vp, n) do { \
    *(vp) = (*(vp) & 0x0000ffffU) | ((n) << 16); \
} while(0)
#define SUDO_API_VERSION_SET_MINOR(vp, n) do { \
    *(vp) = (*(vp) & 0xffff0000U) | (n); \
} while(0)

/* "plugin type" for the sudo front end, as passed to an audit plugin */
#define SUDO_FRONT_END		0

/* Conversation function types and defines */
struct sudo_conv_message {
#define SUDO_CONV_PROMPT_ECHO_OFF   0x0001  /* do not echo user input */
#define SUDO_CONV_PROMPT_ECHO_ON    0x0002  /* echo user input */
#define SUDO_CONV_ERROR_MSG	    0x0003  /* error message */
#define SUDO_CONV_INFO_MSG	    0x0004  /* informational message */
#define SUDO_CONV_PROMPT_MASK	    0x0005  /* mask user input */
#define SUDO_CONV_PROMPT_ECHO_OK    0x1000  /* flag: allow echo if no tty */
#define SUDO_CONV_PREFER_TTY	    0x2000  /* flag: use tty if possible */
    int msg_type;
    int timeout;
    const char *msg;
};

/*
 * Maximum length of a reply (not including the trailing NUL) when
 * conversing with the user.  In practical terms, this is the longest
 * password sudo will support.  This means that a buffer of size
 * SUDO_CONV_REPL_MAX+1 is guaranteed to be able to hold any reply
 * from the conversation function.
 */
#define SUDO_CONV_REPL_MAX	1023

struct sudo_conv_reply {
    char *reply;
};

/* Conversation callback API version major/minor */
#define SUDO_CONV_CALLBACK_VERSION_MAJOR	1
#define SUDO_CONV_CALLBACK_VERSION_MINOR	0
#define SUDO_CONV_CALLBACK_VERSION SUDO_API_MKVERSION(SUDO_CONV_CALLBACK_VERSION_MAJOR, SUDO_CONV_CALLBACK_VERSION_MINOR)

/*
 * Callback struct to be passed to the conversation function.
 * Can be used to perform operations on suspend/resume such
 * as dropping/acquiring locks.
 */
typedef int (*sudo_conv_callback_fn_t)(int signo, void *closure);
struct sudo_conv_callback {
    unsigned int version;
    void *closure;
    sudo_conv_callback_fn_t on_suspend;
    sudo_conv_callback_fn_t on_resume;
};

typedef int (*sudo_conv_t)(int num_msgs, const struct sudo_conv_message msgs[],
	struct sudo_conv_reply replies[], struct sudo_conv_callback *callback);
typedef int (*sudo_printf_t)(int msg_type, const char * restrict fmt, ...);

/*
 * Hooks allow a plugin to hook into specific sudo and/or libc functions.
 */

#if defined(__GNUC__) && ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || __GNUC__ > 4)
# pragma GCC diagnostic ignored "-Wstrict-prototypes"
#endif

/* Hook functions typedefs. */
typedef int (*sudo_hook_fn_t)();
typedef int (*sudo_hook_fn_setenv_t)(const char *name, const char *value, int overwrite, void *closure);
typedef int (*sudo_hook_fn_putenv_t)(char *string, void *closure);
typedef int (*sudo_hook_fn_getenv_t)(const char *name, char **value, void *closure);
typedef int (*sudo_hook_fn_unsetenv_t)(const char *name, void *closure);

/* Hook structure definition. */
struct sudo_hook {
    unsigned int hook_version;
    unsigned int hook_type;
    sudo_hook_fn_t hook_fn;
    void *closure;
};

/* Hook API version major/minor */
#define SUDO_HOOK_VERSION_MAJOR	1
#define SUDO_HOOK_VERSION_MINOR	0
#define SUDO_HOOK_VERSION SUDO_API_MKVERSION(SUDO_HOOK_VERSION_MAJOR, SUDO_HOOK_VERSION_MINOR)

/*
 * Hook function return values.
 */
#define SUDO_HOOK_RET_ERROR	-1	/* error */
#define SUDO_HOOK_RET_NEXT	0	/* go to the next hook in the list */
#define SUDO_HOOK_RET_STOP	1	/* stop hook processing for this type */

/*
 * Hooks for setenv/unsetenv/putenv/getenv.
 * This allows the plugin to be notified when a PAM module modifies
 * the environment so it can update the copy of the environment that
 * is passed to execve().
 */
#define SUDO_HOOK_SETENV	1
#define SUDO_HOOK_UNSETENV	2
#define SUDO_HOOK_PUTENV	3
#define SUDO_HOOK_GETENV	4

/*
 * Plugin interface to sudo's main event loop.
 */
typedef void (*sudo_plugin_ev_callback_t)(int fd, int what, void *closure);

struct timespec;
struct sudo_plugin_event {
    int (*set)(struct sudo_plugin_event *pev, int fd, int events, sudo_plugin_ev_callback_t callback, void *closure);
    int (*add)(struct sudo_plugin_event *pev, struct timespec *timeout);
    int (*del)(struct sudo_plugin_event *pev);
    int (*pending)(struct sudo_plugin_event *pev, int events, struct timespec *ts);
    int (*fd)(struct sudo_plugin_event *pev);
    void (*setbase)(struct sudo_plugin_event *pev, void *base);
    void (*loopbreak)(struct sudo_plugin_event *pev);
    void (*free)(struct sudo_plugin_event *pev);
    /* actually larger... */
};

/* Sudo plugin Event types */
#define SUDO_PLUGIN_EV_TIMEOUT	0x01	/* fire after timeout */
#define SUDO_PLUGIN_EV_READ	0x02	/* fire when readable */
#define SUDO_PLUGIN_EV_WRITE	0x04	/* fire when writable */
#define SUDO_PLUGIN_EV_PERSIST	0x08	/* persist until deleted */
#define SUDO_PLUGIN_EV_SIGNAL	0x10	/* fire on signal receipt */

/* Policy plugin type and defines. */
struct passwd;
struct policy_plugin {
#define SUDO_POLICY_PLUGIN     1
    unsigned int type; /* always SUDO_POLICY_PLUGIN */
    unsigned int version; /* always SUDO_API_VERSION */
    int (*open)(unsigned int version, sudo_conv_t conversation,
	sudo_printf_t sudo_plugin_printf, char * const settings[],
	char * const user_info[], char * const user_env[],
	char * const plugin_options[], const char **errstr);
    void (*close)(int exit_status, int error); /* wait status or error */
    int (*show_version)(int verbose);
    int (*check_policy)(int argc, char * const argv[],
	char *env_add[], char **command_info[],
	char **argv_out[], char **user_env_out[], const char **errstr);
    int (*list)(int argc, char * const argv[], int verbose,
	const char *user, const char **errstr);
    int (*validate)(const char **errstr);
    void (*invalidate)(int rmcred);
    int (*init_session)(struct passwd *pwd, char **user_env_out[],
	const char **errstr);
    void (*register_hooks)(int version, int (*register_hook)(struct sudo_hook *hook));
    void (*deregister_hooks)(int version, int (*deregister_hook)(struct sudo_hook *hook));
    struct sudo_plugin_event * (*event_alloc)(void);
};

/* I/O plugin type and defines. */
struct io_plugin {
#define SUDO_IO_PLUGIN	    2
    unsigned int type; /* always SUDO_IO_PLUGIN */
    unsigned int version; /* always SUDO_API_VERSION */
    int (*open)(unsigned int version, sudo_conv_t conversation,
	sudo_printf_t sudo_plugin_printf, char * const settings[],
	char * const user_info[], char * const command_info[],
	int argc, char * const argv[], char * const user_env[],
	char * const plugin_options[], const char **errstr);
    void (*close)(int exit_status, int error); /* wait status or error */
    int (*show_version)(int verbose);
    int (*log_ttyin)(const char *buf, unsigned int len, const char **errstr);
    int (*log_ttyout)(const char *buf, unsigned int len, const char **errstr);
    int (*log_stdin)(const char *buf, unsigned int len, const char **errstr);
    int (*log_stdout)(const char *buf, unsigned int len, const char **errstr);
    int (*log_stderr)(const char *buf, unsigned int len, const char **errstr);
    void (*register_hooks)(int version,
	int (*register_hook)(struct sudo_hook *hook));
    void (*deregister_hooks)(int version,
	int (*deregister_hook)(struct sudo_hook *hook));
    int (*change_winsize)(unsigned int line, unsigned int cols,
	const char **errstr);
    int (*log_suspend)(int signo, const char **errstr);
    struct sudo_plugin_event * (*event_alloc)(void);
};

/* Differ audit plugin close status types. */
#define SUDO_PLUGIN_NO_STATUS		0
#define SUDO_PLUGIN_WAIT_STATUS		1
#define SUDO_PLUGIN_EXEC_ERROR		2
#define SUDO_PLUGIN_SUDO_ERROR		3

/* Audit plugin type and defines */
struct audit_plugin {
#define SUDO_AUDIT_PLUGIN	    3
    unsigned int type; /* always SUDO_AUDIT_PLUGIN */
    unsigned int version; /* always SUDO_API_VERSION */
    int (*open)(unsigned int version, sudo_conv_t conversation,
	sudo_printf_t sudo_plugin_printf, char * const settings[],
	char * const user_info[], int submit_optind,
	char * const submit_argv[], char * const submit_envp[],
	char * const plugin_options[], const char **errstr);
    void (*close)(int status_type, int status);
    int (*accept)(const char *plugin_name, unsigned int plugin_type,
	char * const command_info[], char * const run_argv[],
	char * const run_envp[], const char **errstr);
    int (*reject)(const char *plugin_name, unsigned int plugin_type,
	const char *audit_msg, char * const command_info[],
	const char **errstr);
    int (*error)(const char *plugin_name, unsigned int plugin_type,
	const char *audit_msg, char * const command_info[],
	const char **errstr);
    int (*show_version)(int verbose);
    void (*register_hooks)(int version, int (*register_hook)(struct sudo_hook *hook));
    void (*deregister_hooks)(int version, int (*deregister_hook)(struct sudo_hook *hook));
    struct sudo_plugin_event * (*event_alloc)(void);
};

/* Approval plugin type and defines */
struct approval_plugin {
#define SUDO_APPROVAL_PLUGIN	    4
    unsigned int type; /* always SUDO_APPROVAL_PLUGIN */
    unsigned int version; /* always SUDO_API_VERSION */
    int (*open)(unsigned int version, sudo_conv_t conversation,
	sudo_printf_t sudo_plugin_printf, char * const settings[],
	char * const user_info[], int submit_optind,
	char * const submit_argv[], char * const submit_envp[],
	char * const plugin_options[], const char **errstr);
    void (*close)(void);
    int (*check)(char * const command_info[], char * const run_argv[],
	char * const run_envp[], const char **errstr);
    int (*show_version)(int verbose);
};

/* Sudoers group plugin version major/minor */
#define GROUP_API_VERSION_MAJOR 1
#define GROUP_API_VERSION_MINOR 0
#define GROUP_API_VERSION SUDO_API_MKVERSION(GROUP_API_VERSION_MAJOR, GROUP_API_VERSION_MINOR)

/* Getters and setters for group version (for source compat only) */
#define GROUP_API_VERSION_GET_MAJOR(v) SUDO_API_VERSION_GET_MAJOR(v)
#define GROUP_API_VERSION_GET_MINOR(v) SUDO_API_VERSION_GET_MINOR(v)
#define GROUP_API_VERSION_SET_MAJOR(vp, n) SUDO_API_VERSION_SET_MAJOR(vp, n)
#define GROUP_API_VERSION_SET_MINOR(vp, n) SUDO_API_VERSION_SET_MINOR(vp, n)

/*
 * version: for compatibility checking
 * group_init: return 1 on success, 0 if unconfigured, -1 on error.
 * group_cleanup: called to clean up resources used by provider
 * user_in_group: returns 1 if user is in group, 0 if not.
 *                note that pwd may be NULL if the user is not in passwd.
 */
struct sudoers_group_plugin {
    unsigned int version;
    int (*init)(int version, sudo_printf_t sudo_plugin_printf,
	char *const argv[]);
    void (*cleanup)(void);
    int (*query)(const char *user, const char *group, const struct passwd *pwd);
};

#endif /* SUDO_PLUGIN_H */
