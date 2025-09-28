/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2010-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>

#include <sudoers.h>
#include <sudoers_version.h>
#include <timestamp.h>
#include <interfaces.h>
#include "auth/sudo_auth.h"

static char **command_info;

/*
 * Command execution args to be filled in: argv, envp and command info.
 */
struct sudoers_exec_args {
    char ***argv;
    char ***envp;
    char ***info;
};

static unsigned int sudo_version;
static const char *interfaces_string;
sudo_conv_t sudo_conv;
sudo_printf_t sudo_printf;
struct sudo_plugin_event * (*plugin_event_alloc)(void);
static const char *path_sudoers = _PATH_SUDOERS;
static bool session_opened;

extern sudo_dso_public struct policy_plugin sudoers_policy;

static int
parse_bool(const char *line, int varlen, unsigned int *flags, unsigned int fval)
{
    debug_decl(parse_bool, SUDOERS_DEBUG_PLUGIN);

    switch (sudo_strtobool(line + varlen + 1)) {
    case true:
	SET(*flags, fval);
	debug_return_int(true);
    case false:
	CLR(*flags, fval);
	debug_return_int(false);
    default:
	sudo_warnx(U_("invalid %.*s set by sudo front-end"),
	    varlen, line);
	debug_return_int(-1);
    }
}

#define RUN_VALID_FLAGS	(MODE_ASKPASS|MODE_PRESERVE_ENV|MODE_RESET_HOME|MODE_IMPLIED_SHELL|MODE_LOGIN_SHELL|MODE_NONINTERACTIVE|MODE_IGNORE_TICKET|MODE_UPDATE_TICKET|MODE_PRESERVE_GROUPS|MODE_SHELL|MODE_RUN|MODE_POLICY_INTERCEPTED)
#define EDIT_VALID_FLAGS	(MODE_ASKPASS|MODE_NONINTERACTIVE|MODE_IGNORE_TICKET|MODE_UPDATE_TICKET|MODE_EDIT)
#define LIST_VALID_FLAGS	(MODE_ASKPASS|MODE_NONINTERACTIVE|MODE_IGNORE_TICKET|MODE_UPDATE_TICKET|MODE_LIST|MODE_CHECK)
#define VALIDATE_VALID_FLAGS	(MODE_ASKPASS|MODE_NONINTERACTIVE|MODE_IGNORE_TICKET|MODE_UPDATE_TICKET|MODE_VALIDATE)
#define INVALIDATE_VALID_FLAGS	(MODE_ASKPASS|MODE_NONINTERACTIVE|MODE_IGNORE_TICKET|MODE_UPDATE_TICKET|MODE_INVALIDATE)

/*
 * Deserialize args, settings and user_info arrays.
 * Fills in struct sudoers_user_context and other common sudoers state.
 */
unsigned int
sudoers_policy_deserialize_info(struct sudoers_context *ctx, void *v,
    struct defaults_list *defaults)
{
    const char *p, *errstr, *groups = NULL;
    struct sudoers_open_info *info = v;
    unsigned int flags = MODE_UPDATE_TICKET;
    const char *host = NULL;
    const char *remhost = NULL;
    unsigned char uuid[16];
    char * const *cur;
    debug_decl(sudoers_policy_deserialize_info, SUDOERS_DEBUG_PLUGIN);

#define MATCHES(s, v)	\
    (strncmp((s), (v), sizeof(v) - 1) == 0)

#define INVALID(v) do {	\
    sudo_warnx(U_("invalid %.*s set by sudo front-end"), \
	(int)(sizeof(v) - 2), (v)); \
} while (0)

#define CHECK(s, v) do {	\
    if ((s)[sizeof(v) - 1] == '\0') { \
	INVALID(v); \
	goto bad; \
    } \
} while (0)

    if (sudo_gettime_real(&ctx->submit_time) == -1) {
	sudo_warn("%s", U_("unable to get time of day"));
	goto bad;
    }

    /* Parse sudo.conf plugin args. */
    if (info->plugin_args != NULL) {
	for (cur = info->plugin_args; *cur != NULL; cur++) {
	    if (MATCHES(*cur, "error_recovery=")) {
		int val = sudo_strtobool(*cur + sizeof("error_recovery=") - 1);
		if (val == -1) {
		    INVALID("error_recovery=");	/* Not a fatal error. */
		} else {
		    ctx->parser_conf.recovery = val;
		}
		continue;
	    }
	    if (MATCHES(*cur, "ignore_perms=")) {
		int val = sudo_strtobool(*cur + sizeof("ignore_perms=") - 1);
		if (val == -1) {
		    INVALID("ignore_perms=");	/* Not a fatal error. */
		} else {
		    ctx->parser_conf.ignore_perms = val;
		}
		continue;
	    }
	    if (MATCHES(*cur, "sudoers_file=")) {
		CHECK(*cur, "sudoers_file=");
		path_sudoers = *cur + sizeof("sudoers_file=") - 1;
		continue;
	    }
	    if (MATCHES(*cur, "sudoers_uid=")) {
		p = *cur + sizeof("sudoers_uid=") - 1;
		ctx->parser_conf.sudoers_uid = (uid_t)sudo_strtoid(p, &errstr);
		if (errstr != NULL) {
		    sudo_warnx(U_("%s: %s"), *cur, U_(errstr));
		    goto bad;
		}
		continue;
	    }
	    if (MATCHES(*cur, "sudoers_gid=")) {
		p = *cur + sizeof("sudoers_gid=") - 1;
		ctx->parser_conf.sudoers_gid = (gid_t)sudo_strtoid(p, &errstr);
		if (errstr != NULL) {
		    sudo_warnx(U_("%s: %s"), *cur, U_(errstr));
		    goto bad;
		}
		continue;
	    }
	    if (MATCHES(*cur, "sudoers_mode=")) {
		p = *cur + sizeof("sudoers_mode=") - 1;
		ctx->parser_conf.sudoers_mode = sudo_strtomode(p, &errstr);
		if (errstr != NULL) {
		    sudo_warnx(U_("%s: %s"), *cur, U_(errstr));
		    goto bad;
		}
		continue;
	    }
	    if (MATCHES(*cur, "ldap_conf=")) {
		CHECK(*cur, "ldap_conf=");
		ctx->settings.ldap_conf = *cur + sizeof("ldap_conf=") - 1;
		continue;
	    }
	    if (MATCHES(*cur, "ldap_secret=")) {
		CHECK(*cur, "ldap_secret=");
		ctx->settings.ldap_secret = *cur + sizeof("ldap_secret=") - 1;
		continue;
	    }
	}
    }
    ctx->parser_conf.sudoers_path = path_sudoers;

    /* Parse command line settings. */
    ctx->settings.flags = 0;
    ctx->user.closefrom = -1;
    ctx->sudoedit_nfiles = 0;
    ctx->mode = 0;
    for (cur = info->settings; *cur != NULL; cur++) {
	if (MATCHES(*cur, "closefrom=")) {
	    p = *cur + sizeof("closefrom=") - 1;
	    ctx->user.closefrom = (int)sudo_strtonum(p, 3, INT_MAX, &errstr);
	    if (ctx->user.closefrom == 0) {
		sudo_warnx(U_("%s: %s"), *cur, U_(errstr));
		goto bad;
	    }
	    continue;
	}
	if (MATCHES(*cur, "cmnd_chroot=")) {
	    CHECK(*cur, "cmnd_chroot=");
	    ctx->runas.chroot = *cur + sizeof("cmnd_chroot=") - 1;
	    if (strlen(ctx->runas.chroot) >= PATH_MAX) {
		sudo_warnx(U_("path name for \"%s\" too long"), "cmnd_chroot");
		goto bad;
	    }
	    continue;
	}
	if (MATCHES(*cur, "cmnd_cwd=")) {
	    CHECK(*cur, "cmnd_cwd=");
	    ctx->runas.cwd = *cur + sizeof("cmnd_cwd=") - 1;
	    if (strlen(ctx->runas.cwd) >= PATH_MAX) {
		sudo_warnx(U_("path name for \"%s\" too long"), "cmnd_cwd");
		goto bad;
	    }
	    continue;
	}
	if (MATCHES(*cur, "runas_user=")) {
	    CHECK(*cur, "runas_user=");
	    ctx->runas.user = *cur + sizeof("runas_user=") - 1;
	    SET(ctx->settings.flags, RUNAS_USER_SPECIFIED);
	    continue;
	}
	if (MATCHES(*cur, "runas_group=")) {
	    CHECK(*cur, "runas_group=");
	    ctx->runas.group = *cur + sizeof("runas_group=") - 1;
	    SET(ctx->settings.flags, RUNAS_GROUP_SPECIFIED);
	    continue;
	}
	if (MATCHES(*cur, "prompt=")) {
	    /* Allow epmpty prompt. */
	    ctx->user.prompt = *cur + sizeof("prompt=") - 1;
	    if (!append_default("passprompt_override", NULL, true, NULL, defaults))
		goto oom;
	    continue;
	}
	if (MATCHES(*cur, "set_home=")) {
	    if (parse_bool(*cur, sizeof("set_home") - 1, &flags,
		MODE_RESET_HOME) == -1)
		goto bad;
	    continue;
	}
	if (MATCHES(*cur, "preserve_environment=")) {
	    if (parse_bool(*cur, sizeof("preserve_environment") - 1, &flags,
		MODE_PRESERVE_ENV) == -1)
		goto bad;
	    continue;
	}
	if (MATCHES(*cur, "run_shell=")) {
	    if (parse_bool(*cur, sizeof("run_shell") -1, &flags,
		MODE_SHELL) == -1)
		goto bad;
	    continue;
	}
	if (MATCHES(*cur, "login_shell=")) {
	    if (parse_bool(*cur, sizeof("login_shell") - 1, &flags,
		MODE_LOGIN_SHELL) == -1)
		goto bad;
	    continue;
	}
	if (MATCHES(*cur, "implied_shell=")) {
	    if (parse_bool(*cur, sizeof("implied_shell") - 1, &flags,
		MODE_IMPLIED_SHELL) == -1)
		goto bad;
	    continue;
	}
	if (MATCHES(*cur, "preserve_groups=")) {
	    if (parse_bool(*cur, sizeof("preserve_groups") - 1, &flags,
		MODE_PRESERVE_GROUPS) == -1)
		goto bad;
	    continue;
	}
	if (MATCHES(*cur, "ignore_ticket=")) {
	    if (parse_bool(*cur, sizeof("ignore_ticket") -1, &flags,
		MODE_IGNORE_TICKET) == -1)
		goto bad;
	    continue;
	}
	if (MATCHES(*cur, "update_ticket=")) {
	    if (parse_bool(*cur, sizeof("update_ticket") -1, &flags,
		MODE_UPDATE_TICKET) == -1)
		goto bad;
	    continue;
	}
	if (MATCHES(*cur, "noninteractive=")) {
	    if (parse_bool(*cur, sizeof("noninteractive") - 1, &flags,
		MODE_NONINTERACTIVE) == -1)
		goto bad;
	    continue;
	}
	if (MATCHES(*cur, "sudoedit=")) {
	    if (parse_bool(*cur, sizeof("sudoedit") - 1, &flags,
		MODE_EDIT) == -1)
		goto bad;
	    continue;
	}
	if (MATCHES(*cur, "login_class=")) {
	    CHECK(*cur, "login_class=");
	    ctx->runas.class = *cur + sizeof("login_class=") - 1;
	    if (!append_default("use_loginclass", NULL, true, NULL, defaults))
		goto oom;
	    continue;
	}
	if (MATCHES(*cur, "intercept_ptrace=")) {
	    if (parse_bool(*cur, sizeof("intercept_ptrace") - 1, &ctx->settings.flags,
		    HAVE_INTERCEPT_PTRACE) == -1)
		goto bad;
	    continue;
	}
	if (MATCHES(*cur, "intercept_setid=")) {
	    if (parse_bool(*cur, sizeof("intercept_setid") - 1, &ctx->settings.flags,
		    CAN_INTERCEPT_SETID) == -1)
		goto bad;
	    continue;
	}
#ifdef HAVE_SELINUX
	if (MATCHES(*cur, "selinux_role=")) {
	    CHECK(*cur, "selinux_role=");
	    free(ctx->runas.role);
	    ctx->runas.role = strdup(*cur + sizeof("selinux_role=") - 1);
	    if (ctx->runas.role == NULL)
		goto oom;
	    continue;
	}
	if (MATCHES(*cur, "selinux_type=")) {
	    CHECK(*cur, "selinux_type=");
	    free(ctx->runas.type);
	    ctx->runas.type = strdup(*cur + sizeof("selinux_type=") - 1);
	    if (ctx->runas.type == NULL)
		goto oom;
	    continue;
	}
#endif /* HAVE_SELINUX */
#ifdef HAVE_APPARMOR
	if (MATCHES(*cur, "apparmor_profile=")) {
	    CHECK(*cur, "apparmor_profile=");
	    free(ctx->runas.apparmor_profile);
	    ctx->runas.apparmor_profile = strdup(*cur + sizeof("apparmor_profile=") - 1);
	    if (ctx->runas.apparmor_profile == NULL)
		goto oom;
	    continue;
	}
#endif /* HAVE_APPARMOR */
#ifdef HAVE_BSD_AUTH_H
	if (MATCHES(*cur, "bsdauth_type=")) {
	    CHECK(*cur, "bsdauth_type=");
	    p = *cur + sizeof("bsdauth_type=") - 1;
	    bsdauth_set_style(p);
	    continue;
	}
#endif /* HAVE_BSD_AUTH_H */
	if (MATCHES(*cur, "network_addrs=")) {
	    interfaces_string = *cur + sizeof("network_addrs=") - 1;
	    if (!set_interfaces(interfaces_string)) {
		sudo_warn("%s", U_("unable to parse network address list"));
		goto bad;
	    }
	    continue;
	}
	if (MATCHES(*cur, "max_groups=")) {
	    int max_groups;
	    p = *cur + sizeof("max_groups=") - 1;
	    max_groups = (int)sudo_strtonum(p, 1, 1024, &errstr);
	    if (max_groups == 0) {
		sudo_warnx(U_("%s: %s"), *cur, U_(errstr));
		goto bad;
	    }
	    sudo_pwutil_set_max_groups(max_groups);
	    continue;
	}
	if (MATCHES(*cur, "remote_host=")) {
	    CHECK(*cur, "remote_host=");
	    remhost = *cur + sizeof("remote_host=") - 1;
	    continue;
	}
	if (MATCHES(*cur, "timeout=")) {
	    p = *cur + sizeof("timeout=") - 1;
	    ctx->user.timeout = parse_timeout(p);
	    if (ctx->user.timeout == -1) {
		if (errno == ERANGE)
		    sudo_warnx(U_("%s: %s"), p, U_("timeout value too large"));
		else
		    sudo_warnx(U_("%s: %s"), p, U_("invalid timeout value"));
		goto bad;
	    }
	    continue;
	}
	if (MATCHES(*cur, "askpass=")) {
	    if (parse_bool(*cur, sizeof("askpass") - 1, &flags,
		MODE_ASKPASS) == -1)
		goto bad;
	    continue;
	}
#ifdef ENABLE_SUDO_PLUGIN_API
	if (MATCHES(*cur, "plugin_dir=")) {
	    CHECK(*cur, "plugin_dir=");
	    ctx->settings.plugin_dir = *cur + sizeof("plugin_dir=") - 1;
	    continue;
	}
#endif
    }
    /* Ignore ticket trumps update. */
    if (ISSET(flags, MODE_IGNORE_TICKET))
	CLR(flags, MODE_UPDATE_TICKET);

    ctx->user.gid = (gid_t)-1;
    ctx->user.uid = (gid_t)-1;
    ctx->user.umask = (mode_t)-1;
    for (cur = info->user_info; *cur != NULL; cur++) {
	if (MATCHES(*cur, "user=")) {
	    CHECK(*cur, "user=");
	    free(ctx->user.name);
	    if ((ctx->user.name = strdup(*cur + sizeof("user=") - 1)) == NULL)
		goto oom;
	    continue;
	}
	if (MATCHES(*cur, "euid=")) {
	    p = *cur + sizeof("euid=") - 1;
	    ctx->user.euid = (uid_t) sudo_strtoid(p, &errstr);
	    if (errstr != NULL) {
		sudo_warnx(U_("%s: %s"), *cur, U_(errstr));
		goto bad;
	    }
	    continue;
	}
	if (MATCHES(*cur, "uid=")) {
	    p = *cur + sizeof("uid=") - 1;
	    ctx->user.uid = (uid_t) sudo_strtoid(p, &errstr);
	    if (errstr != NULL) {
		sudo_warnx(U_("%s: %s"), *cur, U_(errstr));
		goto bad;
	    }
	    continue;
	}
	if (MATCHES(*cur, "egid=")) {
	    p = *cur + sizeof("egid=") - 1;
	    ctx->user.egid = (gid_t) sudo_strtoid(p, &errstr);
	    if (errstr != NULL) {
		sudo_warnx(U_("%s: %s"), *cur, U_(errstr));
		goto bad;
	    }
	    continue;
	}
	if (MATCHES(*cur, "gid=")) {
	    p = *cur + sizeof("gid=") - 1;
	    ctx->user.gid = (gid_t) sudo_strtoid(p, &errstr);
	    if (errstr != NULL) {
		sudo_warnx(U_("%s: %s"), *cur, U_(errstr));
		goto bad;
	    }
	    continue;
	}
	if (MATCHES(*cur, "groups=")) {
	    CHECK(*cur, "groups=");
	    groups = *cur + sizeof("groups=") - 1;
	    continue;
	}
	if (MATCHES(*cur, "cwd=")) {
	    CHECK(*cur, "cwd=");
	    free(ctx->user.cwd);
	    if ((ctx->user.cwd = strdup(*cur + sizeof("cwd=") - 1)) == NULL)
		goto oom;
	    continue;
	}
	if (MATCHES(*cur, "tty=")) {
	    CHECK(*cur, "tty=");
	    free(ctx->user.ttypath);
	    if ((ctx->user.ttypath = strdup(*cur + sizeof("tty=") - 1)) == NULL)
		goto oom;
	    ctx->user.tty = ctx->user.ttypath;
	    if (strncmp(ctx->user.tty, _PATH_DEV, sizeof(_PATH_DEV) - 1) == 0)
		ctx->user.tty += sizeof(_PATH_DEV) - 1;
	    continue;
	}
	if (MATCHES(*cur, "host=")) {
	    CHECK(*cur, "host=");
	    host = *cur + sizeof("host=") - 1;
	    continue;
	}
	if (MATCHES(*cur, "lines=")) {
	    p = *cur + sizeof("lines=") - 1;
	    ctx->user.lines = (int)sudo_strtonum(p, 1, INT_MAX, &errstr);
	    if (ctx->user.lines == 0) {
		sudo_warnx(U_("%s: %s"), *cur, U_(errstr));
		goto bad;
	    }
	    continue;
	}
	if (MATCHES(*cur, "cols=")) {
	    p = *cur + sizeof("cols=") - 1;
	    ctx->user.cols = (int)sudo_strtonum(p, 1, INT_MAX, &errstr);
	    if (ctx->user.cols == 0) {
		sudo_warnx(U_("%s: %s"), *cur, U_(errstr));
		goto bad;
	    }
	    continue;
	}
	if (MATCHES(*cur, "pid=")) {
	    p = *cur + sizeof("pid=") - 1;
	    ctx->user.pid = (pid_t) sudo_strtoid(p, &errstr);
	    if (errstr != NULL) {
		sudo_warnx(U_("%s: %s"), *cur, U_(errstr));
		goto bad;
	    }
	    continue;
	}
	if (MATCHES(*cur, "ppid=")) {
	    p = *cur + sizeof("ppid=") - 1;
	    ctx->user.ppid = (pid_t) sudo_strtoid(p, &errstr);
	    if (errstr != NULL) {
		sudo_warnx(U_("%s: %s"), *cur, U_(errstr));
		goto bad;
	    }
	    continue;
	}
	if (MATCHES(*cur, "sid=")) {
	    p = *cur + sizeof("sid=") - 1;
	    ctx->user.sid = (pid_t) sudo_strtoid(p, &errstr);
	    if (errstr != NULL) {
		sudo_warnx(U_("%s: %s"), *cur, U_(errstr));
		goto bad;
	    }
	    continue;
	}
	if (MATCHES(*cur, "tcpgid=")) {
	    p = *cur + sizeof("tcpgid=") - 1;
	    ctx->user.tcpgid = (pid_t) sudo_strtoid(p, &errstr);
	    if (errstr != NULL) {
		sudo_warnx(U_("%s: %s"), *cur, U_(errstr));
		goto bad;
	    }
	    continue;
	}
	if (MATCHES(*cur, "umask=")) {
	    p = *cur + sizeof("umask=") - 1;
	    ctx->user.umask = sudo_strtomode(p, &errstr);
	    if (errstr != NULL) {
		sudo_warnx(U_("%s: %s"), *cur, U_(errstr));
		goto bad;
	    }
	    continue;
	}
    }

    /* User name, user-ID, group-ID and host name must be specified. */
    if (ctx->user.name == NULL) {
	sudo_warnx("%s", U_("user name not set by sudo front-end"));
	goto bad;
    }
    if (ctx->user.uid == (uid_t)-1) {
	sudo_warnx("%s", U_("user-ID not set by sudo front-end"));
	goto bad;
    }
    if (ctx->user.gid == (gid_t)-1) {
	sudo_warnx("%s", U_("group-ID not set by sudo front-end"));
	goto bad;
    }
    if (host == NULL) {
	sudo_warnx("%s", U_("host name not set by sudo front-end"));
	goto bad;
    }

    if (!sudoers_sethost(ctx, host, remhost)) {
	/* sudoers_sethost() will print a warning on error. */
	goto bad;
    }
    if (ctx->user.tty == NULL) {
	if ((ctx->user.tty = strdup("unknown")) == NULL)
	    goto oom;
	/* ctx->user.ttypath remains NULL */
    }

    ctx->user.pw = sudo_getpwnam(ctx->user.name);
    if (ctx->user.pw != NULL && groups != NULL) {
	/* sudo_parse_gids() will print a warning on error. */
	GETGROUPS_T *gids;
	int ngids = sudo_parse_gids(groups, &ctx->user.gid, &gids);
	if (ngids == -1)
	    goto bad;

	/* sudo_set_gidlist will adopt gids[] */
	if (sudo_set_gidlist(ctx->user.pw, ngids, gids, NULL, ENTRY_TYPE_FRONTEND) == -1) {
	    free(gids);
	    goto bad;
	}
    }

    /* umask is only set in user_info[] for API 1.10 and above. */
    if (ctx->user.umask == (mode_t)-1) {
	ctx->user.umask = umask(0);
	umask(ctx->user.umask);
    }

    /* Always reset the environment for a login shell. */
    if (ISSET(flags, MODE_LOGIN_SHELL))
	def_env_reset = true;

    /* Some systems support fexecve() which we use for digest matches. */
    ctx->runas.execfd = -1;

    /* Create a UUID to store in the event log. */
    sudo_uuid_create(uuid);
    if (sudo_uuid_to_string(uuid, ctx->uuid_str, sizeof(ctx->uuid_str)) == NULL) {
	sudo_warnx("%s", U_("unable to generate UUID"));
	goto bad;
    }

    /*
     * Set intercept defaults based on flags set above.
     * We pass -1 as the operator to indicate it is set by the front end.
     */
    if (ISSET(ctx->settings.flags, HAVE_INTERCEPT_PTRACE)) {
	if (!append_default("intercept_type", "trace", -1, NULL, defaults))
	    goto oom;
    }
    if (ISSET(ctx->settings.flags, CAN_INTERCEPT_SETID)) {
	if (!append_default("intercept_allow_setid", NULL, -1, NULL, defaults))
	    goto oom;
    }

#ifdef NO_ROOT_MAILER
    eventlog_set_mailuid(ctx->user.uid);
#endif

    /* Dump settings and user info (XXX - plugin args) */
    for (cur = info->settings; *cur != NULL; cur++)
	sudo_debug_printf(SUDO_DEBUG_INFO, "settings: %s", *cur);
    for (cur = info->user_info; *cur != NULL; cur++)
	sudo_debug_printf(SUDO_DEBUG_INFO, "user_info: %s", *cur);

#undef MATCHES
#undef INVALID
#undef CHECK
    debug_return_uint(flags);

oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
bad:
    debug_return_uint(MODE_ERROR);
}

/*
 * Store the execution environment and other front-end settings.
 * Builds up the command_info list and sets argv and envp.
 * Consumes iolog_path if not NULL.
 * Returns true on success, else false.
 */
bool
sudoers_policy_store_result(struct sudoers_context *ctx, bool accepted,
    char *argv[], char *envp[], mode_t cmnd_umask, char *iolog_path, void *v)
{
    struct sudoers_exec_args *exec_args = v;
    unsigned int info_len = 0;
    debug_decl(sudoers_policy_store_result, SUDOERS_DEBUG_PLUGIN);

    if (exec_args == NULL)
	debug_return_bool(true);	/* nothing to do */

    /* Free old data, if any. */
    if (command_info != NULL) {
	char **cur;
	sudoers_gc_remove(GC_VECTOR, command_info);
	for (cur = command_info; *cur != NULL; cur++)
	    free(*cur);
	free(command_info);
    }

    /* Increase the length of command_info as needed, it is *not* checked. */
    command_info = calloc(74, sizeof(char *));
    if (command_info == NULL)
	goto oom;

    if (ctx->runas.cmnd != NULL) {
	command_info[info_len] = sudo_new_key_val("command", ctx->runas.cmnd);
	if (command_info[info_len++] == NULL)
	    goto oom;
    }
    if (def_log_subcmds) {
	if ((command_info[info_len++] = strdup("log_subcmds=true")) == NULL)
	    goto oom;
    }
    if (iolog_enabled) {
	if (iolog_path)
	    command_info[info_len++] = iolog_path;	/* now owned */
	if (def_log_stdin) {
	    if ((command_info[info_len++] = strdup("iolog_stdin=true")) == NULL)
		goto oom;
	}
	if (def_log_stdout) {
	    if ((command_info[info_len++] = strdup("iolog_stdout=true")) == NULL)
		goto oom;
	}
	if (def_log_stderr) {
	    if ((command_info[info_len++] = strdup("iolog_stderr=true")) == NULL)
		goto oom;
	}
	if (def_log_ttyin) {
	    if ((command_info[info_len++] = strdup("iolog_ttyin=true")) == NULL)
		goto oom;
	}
	if (def_log_ttyout) {
	    if ((command_info[info_len++] = strdup("iolog_ttyout=true")) == NULL)
		goto oom;
	}
	if (def_compress_io) {
	    if ((command_info[info_len++] = strdup("iolog_compress=true")) == NULL)
		goto oom;
	}
	if (def_iolog_flush) {
	    if ((command_info[info_len++] = strdup("iolog_flush=true")) == NULL)
		goto oom;
	}
	if ((command_info[info_len++] = sudo_new_key_val("log_passwords",
		def_log_passwords ? "true" : "false")) == NULL)
	    goto oom;
	if (!SLIST_EMPTY(&def_passprompt_regex)) {
	    char *passprompt_regex =
		serialize_list("passprompt_regex", &def_passprompt_regex);
	    if (passprompt_regex == NULL)
		goto oom;
	    command_info[info_len++] = passprompt_regex;
	}
	if (def_maxseq != NULL) {
	    if ((command_info[info_len++] = sudo_new_key_val("maxseq", def_maxseq)) == NULL)
		goto oom;
	}
    }
    if (ISSET(ctx->mode, MODE_EDIT)) {
	if ((command_info[info_len++] = strdup("sudoedit=true")) == NULL)
	    goto oom;
	if (ctx->sudoedit_nfiles > 0) {
	    if (asprintf(&command_info[info_len++], "sudoedit_nfiles=%d",
		ctx->sudoedit_nfiles) == -1)
		goto oom;
	}
	if (!def_sudoedit_checkdir) {
	    if ((command_info[info_len++] = strdup("sudoedit_checkdir=false")) == NULL)
		goto oom;
	}
	if (def_sudoedit_follow) {
	    if ((command_info[info_len++] = strdup("sudoedit_follow=true")) == NULL)
		goto oom;
	}
    }
    if (def_runcwd && strcmp(def_runcwd, "*") != 0) {
	/* Set cwd to explicit value (sudoers or user-specified). */
	if (!expand_tilde(&def_runcwd, ctx->runas.pw->pw_name)) {
	    sudo_warnx(U_("invalid working directory: %s"), def_runcwd);
	    goto bad;
	}
	if ((command_info[info_len++] = sudo_new_key_val("cwd", def_runcwd)) == NULL)
	    goto oom;
    } else if (ISSET(ctx->mode, MODE_LOGIN_SHELL)) {
	/* Set cwd to run user's homedir. */
	if ((command_info[info_len++] = sudo_new_key_val("cwd", ctx->runas.pw->pw_dir)) == NULL)
	    goto oom;
	if ((command_info[info_len++] = strdup("cwd_optional=true")) == NULL)
	    goto oom;
    }
    if ((command_info[info_len++] = sudo_new_key_val("runas_user", ctx->runas.pw->pw_name)) == NULL)
	goto oom;
    if (ctx->runas.gr != NULL) {
	if ((command_info[info_len++] = sudo_new_key_val("runas_group", ctx->runas.gr->gr_name)) == NULL)
	    goto oom;
    }
    if (def_stay_setuid) {
	if (asprintf(&command_info[info_len++], "runas_uid=%u",
	    (unsigned int)ctx->user.uid) == -1)
	    goto oom;
	if (asprintf(&command_info[info_len++], "runas_gid=%u",
	    (unsigned int)ctx->user.gid) == -1)
	    goto oom;
	if (asprintf(&command_info[info_len++], "runas_euid=%u",
	    (unsigned int)ctx->runas.pw->pw_uid) == -1)
	    goto oom;
	if (asprintf(&command_info[info_len++], "runas_egid=%u",
	    ctx->runas.gr ? (unsigned int)ctx->runas.gr->gr_gid :
	    (unsigned int)ctx->runas.pw->pw_gid) == -1)
	    goto oom;
    } else {
	if (asprintf(&command_info[info_len++], "runas_uid=%u",
	    (unsigned int)ctx->runas.pw->pw_uid) == -1)
	    goto oom;
	if (asprintf(&command_info[info_len++], "runas_gid=%u",
	    ctx->runas.gr ? (unsigned int)ctx->runas.gr->gr_gid :
	    (unsigned int)ctx->runas.pw->pw_gid) == -1)
	    goto oom;
    }
    if (def_preserve_groups) {
	if ((command_info[info_len++] = strdup("preserve_groups=true")) == NULL)
	    goto oom;
    } else {
	int i, len;
	gid_t egid;
	size_t glsize;
	char *cp, *gid_list;
	struct gid_list *gidlist;

	/* Only use results from a group db query, not the front end. */
	gidlist = sudo_get_gidlist(ctx->runas.pw, ENTRY_TYPE_QUERIED);

	/* We reserve an extra spot in the list for the effective gid. */
	glsize = sizeof("runas_groups=") - 1 +
	    (((size_t)gidlist->ngids + 1) * (STRLEN_MAX_UNSIGNED(gid_t) + 1));
	gid_list = malloc(glsize);
	if (gid_list == NULL) {
	    sudo_gidlist_delref(gidlist);
	    goto oom;
	}
	memcpy(gid_list, "runas_groups=", sizeof("runas_groups=") - 1);
	cp = gid_list + sizeof("runas_groups=") - 1;
	glsize -= (size_t)(cp - gid_list);

	/* On BSD systems the effective gid is the first group in the list. */
	egid = ctx->runas.gr ? (unsigned int)ctx->runas.gr->gr_gid :
	    (unsigned int)ctx->runas.pw->pw_gid;
	len = snprintf(cp, glsize, "%u", (unsigned int)egid);
	if (len < 0 || (size_t)len >= glsize) {
	    sudo_warnx(U_("internal error, %s overflow"), __func__);
	    free(gid_list);
	    sudo_gidlist_delref(gidlist);
	    goto bad;
	}
	cp += len;
	glsize -= (size_t)len;
	for (i = 0; i < gidlist->ngids; i++) {
	    if (gidlist->gids[i] != egid) {
		len = snprintf(cp, glsize, ",%u",
		     (unsigned int)gidlist->gids[i]);
		if (len < 0 || (size_t)len >= glsize) {
		    sudo_warnx(U_("internal error, %s overflow"), __func__);
		    free(gid_list);
		    sudo_gidlist_delref(gidlist);
		    goto bad;
		}
		cp += len;
		glsize -= (size_t)len;
	    }
	}
	command_info[info_len++] = gid_list;
	sudo_gidlist_delref(gidlist);
    }
    if (def_closefrom >= 0) {
	if (asprintf(&command_info[info_len++], "closefrom=%d", def_closefrom) == -1)
	    goto oom;
    }
    if (def_ignore_iolog_errors) {
	if ((command_info[info_len++] = strdup("ignore_iolog_errors=true")) == NULL)
	    goto oom;
    }
    if (def_intercept) {
	if ((command_info[info_len++] = strdup("intercept=true")) == NULL)
	    goto oom;
    }
    if (def_intercept_type == trace) {
	if ((command_info[info_len++] = strdup("use_ptrace=true")) == NULL)
	    goto oom;
    }
    if (def_intercept_verify) {
	if ((command_info[info_len++] = strdup("intercept_verify=true")) == NULL)
	    goto oom;
    }
    if (def_noexec) {
	if ((command_info[info_len++] = strdup("noexec=true")) == NULL)
	    goto oom;
    }
    if (def_exec_background) {
	if ((command_info[info_len++] = strdup("exec_background=true")) == NULL)
	    goto oom;
    }
    if (def_set_utmp) {
	if ((command_info[info_len++] = strdup("set_utmp=true")) == NULL)
	    goto oom;
    }
    if (def_use_pty) {
	if ((command_info[info_len++] = strdup("use_pty=true")) == NULL)
	    goto oom;
    }
    if (def_utmp_runas) {
	if ((command_info[info_len++] = sudo_new_key_val("utmp_user", ctx->runas.pw->pw_name)) == NULL)
	    goto oom;
    }
    if (def_iolog_mode != (S_IRUSR|S_IWUSR)) {
	if (asprintf(&command_info[info_len++], "iolog_mode=0%o", (unsigned int)def_iolog_mode) == -1)
	    goto oom;
    }
    if (def_iolog_user != NULL) {
	if ((command_info[info_len++] = sudo_new_key_val("iolog_user", def_iolog_user)) == NULL)
	    goto oom;
    }
    if (def_iolog_group != NULL) {
	if ((command_info[info_len++] = sudo_new_key_val("iolog_group", def_iolog_group)) == NULL)
	    goto oom;
    }
    if (!SLIST_EMPTY(&def_log_servers)) {
	char *log_servers = serialize_list("log_servers", &def_log_servers);
	if (log_servers == NULL)
	    goto oom;
	command_info[info_len++] = log_servers;

	if (asprintf(&command_info[info_len++], "log_server_timeout=%u", def_log_server_timeout) == -1)
	    goto oom;
    }

    if ((command_info[info_len++] = sudo_new_key_val("log_server_keepalive",
	    def_log_server_keepalive ? "true" : "false")) == NULL)
        goto oom;

    if ((command_info[info_len++] = sudo_new_key_val("log_server_verify",
	    def_log_server_verify ? "true" : "false")) == NULL)
        goto oom;

    if (def_log_server_cabundle != NULL) {
        if ((command_info[info_len++] = sudo_new_key_val("log_server_cabundle", def_log_server_cabundle)) == NULL)
            goto oom;
    }
    if (def_log_server_peer_cert != NULL) {
        if ((command_info[info_len++] = sudo_new_key_val("log_server_peer_cert", def_log_server_peer_cert)) == NULL)
            goto oom;
    }
    if (def_log_server_peer_key != NULL) {
        if ((command_info[info_len++] = sudo_new_key_val("log_server_peer_key", def_log_server_peer_key)) == NULL)
            goto oom;
    }

    if (def_command_timeout > 0 || ctx->user.timeout > 0) {
	int timeout = ctx->user.timeout;
    if (timeout == 0 || (def_command_timeout > 0 && def_command_timeout < timeout))
	    timeout = def_command_timeout;
	if (asprintf(&command_info[info_len++], "timeout=%u", timeout) == -1)
	    goto oom;
    }
    if (def_runchroot != NULL && strcmp(def_runchroot, "*") != 0) {
	if (!expand_tilde(&def_runchroot, ctx->runas.pw->pw_name)) {
	    sudo_warnx(U_("invalid chroot directory: %s"), def_runchroot);
	    goto bad;
	}
        if ((command_info[info_len++] = sudo_new_key_val("chroot", def_runchroot)) == NULL)
            goto oom;
    }
    if (cmnd_umask != ACCESSPERMS) {
	if (asprintf(&command_info[info_len++], "umask=0%o", (unsigned int)cmnd_umask) == -1)
	    goto oom;
    }
    if (sudoers_override_umask()) {
	if ((command_info[info_len++] = strdup("umask_override=true")) == NULL)
	    goto oom;
    }
    if (ctx->runas.execfd != -1) {
	if (sudo_version < SUDO_API_MKVERSION(1, 9)) {
	    /* execfd only supported by plugin API 1.9 and higher */
	    close(ctx->runas.execfd);
	    ctx->runas.execfd = -1;
	} else {
	    if (asprintf(&command_info[info_len++], "execfd=%d", ctx->runas.execfd) == -1)
		goto oom;
	}
    }
    if (def_rlimit_as != NULL) {
        if ((command_info[info_len++] = sudo_new_key_val("rlimit_as", def_rlimit_as)) == NULL)
            goto oom;
    }
    if (def_rlimit_core != NULL) {
        if ((command_info[info_len++] = sudo_new_key_val("rlimit_core", def_rlimit_core)) == NULL)
            goto oom;
    }
    if (def_rlimit_cpu != NULL) {
        if ((command_info[info_len++] = sudo_new_key_val("rlimit_cpu", def_rlimit_cpu)) == NULL)
            goto oom;
    }
    if (def_rlimit_data != NULL) {
        if ((command_info[info_len++] = sudo_new_key_val("rlimit_data", def_rlimit_data)) == NULL)
            goto oom;
    }
    if (def_rlimit_fsize != NULL) {
        if ((command_info[info_len++] = sudo_new_key_val("rlimit_fsize", def_rlimit_fsize)) == NULL)
            goto oom;
    }
    if (def_rlimit_locks != NULL) {
        if ((command_info[info_len++] = sudo_new_key_val("rlimit_locks", def_rlimit_locks)) == NULL)
            goto oom;
    }
    if (def_rlimit_memlock != NULL) {
        if ((command_info[info_len++] = sudo_new_key_val("rlimit_memlock", def_rlimit_memlock)) == NULL)
            goto oom;
    }
    if (def_rlimit_nofile != NULL) {
        if ((command_info[info_len++] = sudo_new_key_val("rlimit_nofile", def_rlimit_nofile)) == NULL)
            goto oom;
    }
    if (def_rlimit_nproc != NULL) {
        if ((command_info[info_len++] = sudo_new_key_val("rlimit_nproc", def_rlimit_nproc)) == NULL)
            goto oom;
    }
    if (def_rlimit_rss != NULL) {
        if ((command_info[info_len++] = sudo_new_key_val("rlimit_rss", def_rlimit_rss)) == NULL)
            goto oom;
    }
    if (def_rlimit_stack != NULL) {
        if ((command_info[info_len++] = sudo_new_key_val("rlimit_stack", def_rlimit_stack)) == NULL)
            goto oom;
    }
    if (ctx->source != NULL) {
	command_info[info_len] = sudo_new_key_val("source", ctx->source);
	if (command_info[info_len++] == NULL)
	    goto oom;
    }
#ifdef HAVE_LOGIN_CAP_H
    if (def_use_loginclass) {
	if ((command_info[info_len++] = sudo_new_key_val("login_class", ctx->runas.class)) == NULL)
	    goto oom;
    }
#endif /* HAVE_LOGIN_CAP_H */
#ifdef HAVE_SELINUX
    if (def_selinux && ctx->runas.role != NULL) {
	if ((command_info[info_len++] = sudo_new_key_val("selinux_role", ctx->runas.role)) == NULL)
	    goto oom;
    }
    if (def_selinux && ctx->runas.type != NULL) {
	if ((command_info[info_len++] = sudo_new_key_val("selinux_type", ctx->runas.type)) == NULL)
	    goto oom;
    }
#endif /* HAVE_SELINUX */
#ifdef HAVE_APPARMOR
	if (ctx->runas.apparmor_profile != NULL) {
	    if ((command_info[info_len++] = sudo_new_key_val("apparmor_profile", ctx->runas.apparmor_profile)) == NULL)
		goto oom;
	}
#endif /* HAVE_APPARMOR */
#ifdef HAVE_PRIV_SET
    if (ctx->runas.privs != NULL) {
	if ((command_info[info_len++] = sudo_new_key_val("runas_privs", ctx->runas.privs)) == NULL)
	    goto oom;
    }
    if (ctx->runas.limitprivs != NULL) {
	if ((command_info[info_len++] = sudo_new_key_val("runas_limitprivs", ctx->runas.limitprivs)) == NULL)
	    goto oom;
    }
#endif /* HAVE_PRIV_SET */

    /* Fill in exec environment info. */
    *(exec_args->argv) = argv;
    *(exec_args->envp) = envp;
    *(exec_args->info) = command_info;

    /* Free command_info on exit. */
    sudoers_gc_add(GC_VECTOR, command_info);

    debug_return_bool(true);

oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
bad:
    free(audit_msg);
    audit_msg = NULL;
    while (info_len)
	free(command_info[--info_len]);
    free(command_info);
    command_info = NULL;
    debug_return_bool(false);
}

bool
sudoers_tty_present(struct sudoers_context *ctx)
{
    debug_decl(sudoers_tty_present, SUDOERS_DEBUG_PLUGIN);
    
    if (ctx->user.tcpgid == 0 && ctx->user.ttypath == NULL) {
	/* No job control or terminal, check /dev/tty. */
	int fd = open(_PATH_TTY, O_RDWR);
	if (fd == -1)
	    debug_return_bool(false);
	close(fd);
    }
    debug_return_bool(true);
}

static int
sudoers_policy_open(unsigned int version, sudo_conv_t conversation,
    sudo_printf_t plugin_printf, char * const settings[],
    char * const user_info[], char * const envp[], char * const args[],
    const char **errstr)
{
    struct sudo_conf_debug_file_list debug_files = TAILQ_HEAD_INITIALIZER(debug_files);
    struct sudoers_open_info info;
    const char *cp, *plugin_path = NULL;
    char * const *cur;
    int ret;
    debug_decl(sudoers_policy_open, SUDOERS_DEBUG_PLUGIN);

    sudo_version = version;
    sudo_conv = conversation;
    sudo_printf = plugin_printf;
    if (sudoers_policy.event_alloc != NULL)
	plugin_event_alloc = sudoers_policy.event_alloc;

    /* Plugin args are only specified for API version 1.2 and higher. */
    if (sudo_version < SUDO_API_MKVERSION(1, 2))
	args = NULL;

    /* Initialize the debug subsystem.  */
    for (cur = settings; (cp = *cur) != NULL; cur++) {
	if (strncmp(cp, "debug_flags=", sizeof("debug_flags=") - 1) == 0) {
	    cp += sizeof("debug_flags=") - 1;
	    if (!sudoers_debug_parse_flags(&debug_files, cp))
		debug_return_int(-1);
	    continue;
	}
	if (strncmp(cp, "plugin_path=", sizeof("plugin_path=") - 1) == 0) {
	    plugin_path = cp + sizeof("plugin_path=") - 1;
	    continue;
	}
    }
    if (!sudoers_debug_register(plugin_path, &debug_files))
	debug_return_int(-1);

    /* Call the sudoers init function. */
    info.settings = settings;
    info.user_info = user_info;
    info.plugin_args = args;
    ret = sudoers_init(&info, log_parse_error, envp);

    /* The audit functions set audit_msg on failure. */
    if (ret != 1 && audit_msg != NULL) {
	if (sudo_version >= SUDO_API_MKVERSION(1, 15))
	    *errstr = audit_msg;
    }

    debug_return_int(ret);
}

static void
sudoers_policy_close(int exit_status, int error_code)
{
    const struct sudoers_context *ctx = sudoers_get_context();
    debug_decl(sudoers_policy_close, SUDOERS_DEBUG_PLUGIN);

    if (session_opened) {
	/* Close the session we opened in sudoers_policy_init_session(). */
	(void)sudo_auth_end_session();

	if (error_code) {
	    errno = error_code;
	    sudo_warn(U_("unable to execute %s"), ctx->runas.cmnd);
	} else {
	    log_exit_status(ctx, exit_status);
	}
    }

    /* Deregister the callback for sudo_fatal()/sudo_fatalx(). */
    sudo_fatal_callback_deregister(sudoers_cleanup);

    /* Free sudoers sources, ctx->user.and passwd/group caches. */
    sudoers_cleanup();

    /* command_info was freed by the g/c code. */
    command_info = NULL;

    /* Free error message passed back to front-end, if any. */
    free(audit_msg);
    audit_msg = NULL;

    /* sudoers_debug_deregister() calls sudo_debug_exit() for us. */
    sudoers_debug_deregister();
}

/*
 * The init_session function is called before executing the command
 * and before uid/gid changes occur.
 * Returns 1 on success, 0 on failure and -1 on error.
 */
static int
sudoers_policy_init_session(struct passwd *pwd, char **user_env[],
    const char **errstr)
{
    const struct sudoers_context *ctx = sudoers_get_context();
    int ret;
    debug_decl(sudoers_policy_init_session, SUDOERS_DEBUG_PLUGIN);

    /* user_env is only specified for API version 1.2 and higher. */
    if (sudo_version < SUDO_API_MKVERSION(1, 2))
	user_env = NULL;

    ret = sudo_auth_begin_session(ctx, pwd, user_env);

    if (ret == 1) {
	session_opened = true;
    } else if (audit_msg != NULL) {
	/* The audit functions set audit_msg on failure. */
	if (sudo_version >= SUDO_API_MKVERSION(1, 15))
	    *errstr = audit_msg;
    }
    debug_return_int(ret);
}

static int
sudoers_policy_check(int argc, char * const argv[], char *env_add[],
    char **command_infop[], char **argv_out[], char **user_env_out[],
    const char **errstr)
{
    const struct sudoers_context *ctx = sudoers_get_context();
    unsigned int valid_flags = RUN_VALID_FLAGS;
    unsigned int flags = MODE_RUN;
    struct sudoers_exec_args exec_args;
    int ret;
    debug_decl(sudoers_policy_check, SUDOERS_DEBUG_PLUGIN);

    if (ISSET(ctx->mode, MODE_EDIT)) {
	valid_flags = EDIT_VALID_FLAGS;
	flags = 0;
    }
    if (!sudoers_set_mode(flags, valid_flags)) {
	sudo_warnx(U_("%s: invalid mode flags from sudo front end: 0x%x"),
	    __func__, ctx->mode);
	debug_return_int(-1);
    }

    exec_args.argv = argv_out;
    exec_args.envp = user_env_out;
    exec_args.info = command_infop;

    ret = sudoers_check_cmnd(argc, argv, env_add, &exec_args);
#ifndef NO_LEAKS
    if (ret == true && sudo_version >= SUDO_API_MKVERSION(1, 3)) {
	/* Unset close function if we don't need it to avoid extra process. */
	if (!iolog_enabled && !def_use_pty && !def_log_exit_status &&
		SLIST_EMPTY(&def_log_servers) && !sudo_auth_needs_end_session())
	    sudoers_policy.close = NULL;
    }
#endif

    /* The audit functions set audit_msg on failure. */
    if (ret != 1 && audit_msg != NULL) {
	if (sudo_version >= SUDO_API_MKVERSION(1, 15))
	    *errstr = audit_msg;
    }
    debug_return_int(ret);
}

static int
sudoers_policy_validate(const char **errstr)
{
    const struct sudoers_context *ctx = sudoers_get_context();
    int ret;
    debug_decl(sudoers_policy_validate, SUDOERS_DEBUG_PLUGIN);

    if (!sudoers_set_mode(MODE_VALIDATE, VALIDATE_VALID_FLAGS)) {
	sudo_warnx(U_("%s: invalid mode flags from sudo front end: 0x%x"),
	    __func__, ctx->mode);
	debug_return_int(-1);
    }

    ret = sudoers_validate_user();

    /* The audit functions set audit_msg on failure. */
    if (ret != 1 && audit_msg != NULL) {
	if (sudo_version >= SUDO_API_MKVERSION(1, 15))
	    *errstr = audit_msg;
    }
    debug_return_int(ret);
}

static void
sudoers_policy_invalidate(int unlinkit)
{
    const struct sudoers_context *ctx = sudoers_get_context();
    debug_decl(sudoers_policy_invalidate, SUDOERS_DEBUG_PLUGIN);

    if (!sudoers_set_mode(MODE_INVALIDATE, INVALIDATE_VALID_FLAGS)) {
	sudo_warnx(U_("%s: invalid mode flags from sudo front end: 0x%x"),
	    __func__, ctx->mode);
    } else {
	timestamp_remove(ctx, unlinkit);
    }

    debug_return;
}

static int
sudoers_policy_list(int argc, char * const argv[], int verbose,
    const char *list_user, const char **errstr)
{
    const struct sudoers_context *ctx = sudoers_get_context();
    int ret;
    debug_decl(sudoers_policy_list, SUDOERS_DEBUG_PLUGIN);

    if (!sudoers_set_mode(argc ? MODE_CHECK : MODE_LIST, LIST_VALID_FLAGS)) {
	sudo_warnx(U_("%s: invalid mode flags from sudo front end: 0x%x"),
	    __func__, ctx->mode);
	debug_return_int(-1);
    }

    ret = sudoers_list(argc, argv, list_user, verbose);

    /* The audit functions set audit_msg on failure. */
    if (ret != 1 && audit_msg != NULL) {
	if (sudo_version >= SUDO_API_MKVERSION(1, 15))
	    *errstr = audit_msg;
    }
    debug_return_int(ret);
}

static int
sudoers_policy_version(int verbose)
{
#ifdef HAVE_LDAP
    const struct sudoers_context *ctx = sudoers_get_context();
#endif
    debug_decl(sudoers_policy_version, SUDOERS_DEBUG_PLUGIN);

    sudo_printf(SUDO_CONV_INFO_MSG, _("Sudoers policy plugin version %s\n"),
	PACKAGE_VERSION);
    sudo_printf(SUDO_CONV_INFO_MSG, _("Sudoers file grammar version %d\n"),
	SUDOERS_GRAMMAR_VERSION);

    if (verbose) {
	sudo_printf(SUDO_CONV_INFO_MSG, _("\nSudoers path: %s\n"), path_sudoers);
#ifdef HAVE_LDAP
# ifdef _PATH_NSSWITCH_CONF
	sudo_printf(SUDO_CONV_INFO_MSG, _("nsswitch path: %s\n"), _PATH_NSSWITCH_CONF);
# endif
	if (ctx->settings.ldap_conf != NULL)
	    sudo_printf(SUDO_CONV_INFO_MSG, _("ldap.conf path: %s\n"), ctx->settings.ldap_conf);
	if (ctx->settings.ldap_secret != NULL)
	    sudo_printf(SUDO_CONV_INFO_MSG, _("ldap.secret path: %s\n"), ctx->settings.ldap_secret);
#endif
	dump_auth_methods();
	dump_defaults();
	sudo_printf(SUDO_CONV_INFO_MSG, "\n");
	if (interfaces_string != NULL) {
	    dump_interfaces(interfaces_string);
	    sudo_printf(SUDO_CONV_INFO_MSG, "\n");
	}
    }
    debug_return_int(true);
}

static struct sudo_hook sudoers_hooks[] = {
    { SUDO_HOOK_VERSION, SUDO_HOOK_SETENV, sudoers_hook_setenv, NULL },
    { SUDO_HOOK_VERSION, SUDO_HOOK_UNSETENV, sudoers_hook_unsetenv, NULL },
    { SUDO_HOOK_VERSION, SUDO_HOOK_GETENV, sudoers_hook_getenv, NULL },
    { SUDO_HOOK_VERSION, SUDO_HOOK_PUTENV, sudoers_hook_putenv, NULL },
    { 0, 0, NULL, NULL }
};

/*
 * Register environment function hooks.
 * Note that we have not registered sudoers with the debug subsystem yet.
 */
static void
sudoers_policy_register_hooks(int version, int (*register_hook)(struct sudo_hook *hook))
{
    struct sudo_hook *hook;

    for (hook = sudoers_hooks; hook->hook_fn != NULL; hook++) {
	if (register_hook(hook) != 0) {
	    sudo_warn_nodebug(
		U_("unable to register hook of type %d (version %d.%d)"),
		hook->hook_type, SUDO_API_VERSION_GET_MAJOR(hook->hook_version),
		SUDO_API_VERSION_GET_MINOR(hook->hook_version));
	}
    }
}

/*
 * De-register environment function hooks.
 */
static void
sudoers_policy_deregister_hooks(int version, int (*deregister_hook)(struct sudo_hook *hook))
{
    struct sudo_hook *hook;

    for (hook = sudoers_hooks; hook->hook_fn != NULL; hook++) {
	if (deregister_hook(hook) != 0) {
	    sudo_warn_nodebug(
		U_("unable to deregister hook of type %d (version %d.%d)"),
		hook->hook_type, SUDO_API_VERSION_GET_MAJOR(hook->hook_version),
		SUDO_API_VERSION_GET_MINOR(hook->hook_version));
	}
    }
}

sudo_dso_public struct policy_plugin sudoers_policy = {
    SUDO_POLICY_PLUGIN,
    SUDO_API_VERSION,
    sudoers_policy_open,
    sudoers_policy_close,
    sudoers_policy_version,
    sudoers_policy_check,
    sudoers_policy_list,
    sudoers_policy_validate,
    sudoers_policy_invalidate,
    sudoers_policy_init_session,
    sudoers_policy_register_hooks,
    sudoers_policy_deregister_hooks,
    NULL /* event_alloc() filled in by sudo */
};
