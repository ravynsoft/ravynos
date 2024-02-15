/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2010-2020 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef SUDO_PLUGIN_INT_H
#define SUDO_PLUGIN_INT_H

/*
 * All plugin structures start with a type and a version.
 */
struct generic_plugin {
    unsigned int type;
    unsigned int version;
    /* the rest depends on the type... */
};

typedef int (*sudo_conv_1_7_t)(int num_msgs,
    const struct sudo_conv_message msgs[], struct sudo_conv_reply replies[]);

/*
 * Backwards-compatible structures for API bumps.
 */
struct policy_plugin_1_0 {
    unsigned int type;
    unsigned int version;
    int (*open)(unsigned int version, sudo_conv_1_7_t conversation,
	sudo_printf_t sudo_plugin_printf, char * const settings[],
	char * const user_info[], char * const user_env[]);
    void (*close)(int exit_status, int error); /* wait status or error */
    int (*show_version)(int verbose);
    int (*check_policy)(int argc, char * const argv[],
	char *env_add[], char **command_info[],
	char **argv_out[], char **user_env_out[]);
    int (*list)(int argc, char * const argv[], int verbose,
	const char *user);
    int (*validate)(void);
    void (*invalidate)(int rmcred);
    int (*init_session)(struct passwd *pwd);
};
struct io_plugin_1_0 {
    unsigned int type;
    unsigned int version;
    int (*open)(unsigned int version, sudo_conv_1_7_t conversation,
        sudo_printf_t sudo_plugin_printf, char * const settings[],
        char * const user_info[], int argc, char * const argv[],
        char * const user_env[]);
    void (*close)(int exit_status, int error);
    int (*show_version)(int verbose);
    int (*log_ttyin)(const char *buf, unsigned int len);
    int (*log_ttyout)(const char *buf, unsigned int len);
    int (*log_stdin)(const char *buf, unsigned int len);
    int (*log_stdout)(const char *buf, unsigned int len);
    int (*log_stderr)(const char *buf, unsigned int len);
};
struct io_plugin_1_1 {
    unsigned int type;
    unsigned int version;
    int (*open)(unsigned int version, sudo_conv_1_7_t conversation,
	sudo_printf_t sudo_plugin_printf, char * const settings[],
	char * const user_info[], char * const command_info[],
	int argc, char * const argv[], char * const user_env[]);
    void (*close)(int exit_status, int error); /* wait status or error */
    int (*show_version)(int verbose);
    int (*log_ttyin)(const char *buf, unsigned int len);
    int (*log_ttyout)(const char *buf, unsigned int len);
    int (*log_stdin)(const char *buf, unsigned int len);
    int (*log_stdout)(const char *buf, unsigned int len);
    int (*log_stderr)(const char *buf, unsigned int len);
};

/*
 * Sudo plugin internals.
 */
struct plugin_container {
    TAILQ_ENTRY(plugin_container) entries;
    struct sudo_conf_debug_file_list *debug_files;
    char *name;
    char *path;
    char **options;
    void *handle;
    int debug_instance;
    union {
	struct generic_plugin *generic;
	struct policy_plugin *policy;
	struct policy_plugin_1_0 *policy_1_0;
	struct io_plugin *io;
	struct io_plugin_1_0 *io_1_0;
	struct io_plugin_1_1 *io_1_1;
	struct audit_plugin *audit;
	struct approval_plugin *approval;
    } u;
};
TAILQ_HEAD(plugin_container_list, plugin_container);

/*
 * Private implementation of struct sudo_plugin_event.
 */
struct sudo_plugin_event_int {
    struct sudo_event private;		/* must be first */
    int debug_instance;			/* plugin's debug instance */
    void *closure;			/* actual user closure */
    sudo_ev_callback_t callback;	/* actual user callback */
    struct sudo_plugin_event public;	/* user-visible portion */
};

extern struct plugin_container policy_plugin;
extern struct plugin_container_list io_plugins;
extern struct plugin_container_list audit_plugins;
extern struct plugin_container_list approval_plugins;

int sudo_conversation(int num_msgs, const struct sudo_conv_message msgs[],
    struct sudo_conv_reply replies[], struct sudo_conv_callback *callback);
int sudo_conversation_1_7(int num_msgs, const struct sudo_conv_message msgs[],
    struct sudo_conv_reply replies[]);
int sudo_conversation_printf(int msg_type, const char * restrict fmt, ...);

bool sudo_load_plugins(void);

#endif /* SUDO_PLUGIN_INT_H */
