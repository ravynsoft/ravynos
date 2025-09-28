/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2009-2022 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sudoers.h>
#include <sudo_eventlog.h>
#include <sudo_iolog.h>
#include <strlist.h>
#ifdef SUDOERS_LOG_CLIENT
# include <log_client.h>
#endif

static struct iolog_file iolog_files[] = {
    { false },	/* IOFD_STDIN */
    { false },	/* IOFD_STDOUT */
    { false },	/* IOFD_STDERR */
    { false },	/* IOFD_TTYIN  */
    { false },	/* IOFD_TTYOUT */
    { true, },	/* IOFD_TIMING */
};

static struct sudoers_io_operations {
    int (*open)(struct timespec *now);
    void (*close)(int exit_status, int error, const char **errstr);
    int (*log)(int event, const char *buf, unsigned int len,
	struct timespec *delay, const char **errstr);
    int (*change_winsize)(unsigned int lines, unsigned int cols,
	struct timespec *delay, const char **errstr);
    int (*suspend)(const char *signame, struct timespec *delay,
	const char **errstr);
} io_operations;

static struct log_details iolog_details;
static bool warned = false;
static bool log_passwords = true;
static int iolog_dir_fd = -1;
static struct timespec last_time;
static void *passprompt_regex_handle;
static void sudoers_io_setops(void);

/* sudoers_io is declared at the end of this file. */
extern sudo_dso_public struct io_plugin sudoers_io;

/*
 * Sudoers callback for maxseq Defaults setting.
 */
bool
cb_maxseq(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    const char *errstr;
    unsigned int value;
    debug_decl(cb_maxseq, SUDOERS_DEBUG_UTIL);

    value = (unsigned int)sudo_strtonum(sd_un->str, 0, SESSID_MAX, &errstr);
    if (errstr != NULL) {
        if (errno != ERANGE) {
            sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
                "bad maxseq: %s: %s", sd_un->str, errstr);
            debug_return_bool(false);
        }
        /* Out of range, clamp to SESSID_MAX as documented. */
        value = SESSID_MAX;
    }
    iolog_set_maxseq(value);
    debug_return_bool(true);
}

/*
 * Sudoers callback for iolog_user Defaults setting.
 */
bool
cb_iolog_user(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    const char *name = sd_un->str;
    struct passwd *pw;
    debug_decl(cb_iolog_user, SUDOERS_DEBUG_UTIL);

    /* NULL name means reset to default. */
    if (name == NULL) {
	iolog_set_owner(ROOT_UID, ROOT_GID);
    } else {
	if ((pw = sudo_getpwnam(name)) == NULL) {
	    log_warningx(ctx, SLOG_SEND_MAIL, N_("unknown user %s"), name);
	    debug_return_bool(false);
	}
	iolog_set_owner(pw->pw_uid, pw->pw_gid);
	sudo_pw_delref(pw);
    }

    debug_return_bool(true);
}

/*
 * Look up I/O log group-ID from group name.
 */
bool
cb_iolog_group(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    const char *name = sd_un->str;
    struct group *gr;
    debug_decl(cb_iolog_group, SUDOERS_DEBUG_UTIL);

    /* NULL name means reset to default. */
    if (name == NULL) {
	iolog_set_gid(ROOT_GID);
    } else {
	if ((gr = sudo_getgrnam(name)) == NULL) {
	    log_warningx(ctx, SLOG_SEND_MAIL, N_("unknown group %s"), name);
	    debug_return_bool(false);
	}
	iolog_set_gid(gr->gr_gid);
	sudo_gr_delref(gr);
    }

    debug_return_bool(true);
}

/*
 * Sudoers callback for iolog_mode Defaults setting.
 */
bool
cb_iolog_mode(struct sudoers_context *ctx, const char *file,
    int line, int column, const union sudo_defs_val *sd_un, int op)
{
    iolog_set_mode(sd_un->mode);
    return true;
}

/*
 * Make a shallow copy of a NULL-terminated argument or environment vector.
 * Only the outer array is allocated, the pointers inside are copied.
 * The caller is responsible for freeing the returned copy.
 */
static char **
copy_vector_shallow(char * const *vec)
{
    char **copy;
    size_t len;
    debug_decl(copy_vector, SUDOERS_DEBUG_UTIL);

    for (len = 0; vec[len] != NULL; len++)
	continue;

    if ((copy = reallocarray(NULL, len + 1, sizeof(char *))) != NULL) {
	for (len = 0; vec[len] != NULL; len++)
	    copy[len] = vec[len];
	copy[len] = NULL;
    }

    debug_return_ptr(copy);
}

static void
free_iolog_details(void)
{
    debug_decl(free_iolog_details, SUDOERS_DEBUG_PLUGIN);

    if (iolog_details.evlog != NULL) {
	/* We only make a shallow copy of argv and envp. */
	free(iolog_details.evlog->runargv);
	iolog_details.evlog->runargv = NULL;
	free(iolog_details.evlog->runenv);
	iolog_details.evlog->runenv = NULL;
	free(iolog_details.evlog->submitenv);
	iolog_details.evlog->submitenv = NULL;
	eventlog_free(iolog_details.evlog);
    }
    str_list_free(iolog_details.log_servers);
#if defined(HAVE_OPENSSL)
    free(iolog_details.ca_bundle);
    free(iolog_details.cert_file);
    free(iolog_details.key_file);
#endif /* HAVE_OPENSSL */

    debug_return;
}

/*
 * Convert a comma-separated list to a string list.
 */
static struct sudoers_str_list *
deserialize_stringlist(const char *s)
{
    struct sudoers_str_list *strlist;
    struct sudoers_string *str;
    const char *s_end = s + strlen(s);
    const char *cp, *ep;
    debug_decl(deserialize_stringlist, SUDOERS_DEBUG_UTIL);

    if ((strlist = str_list_alloc()) == NULL)
	debug_return_ptr(NULL);

    for (cp = sudo_strsplit(s, s_end, ",", &ep); cp != NULL;
	    cp = sudo_strsplit(NULL, s_end, ",", &ep)) {
	if (cp == ep)
	    continue;
	if ((str = malloc(sizeof(*str))) == NULL)
	    goto bad;
	if ((str->str = strndup(cp, (size_t)(ep - cp))) == NULL) {
	    free(str);
	    goto bad;
	}
	unescape_string(str->str);
	STAILQ_INSERT_TAIL(strlist, str, entries);
    }
    if (STAILQ_EMPTY(strlist))
	goto bad;

    debug_return_ptr(strlist);

bad:
    str_list_free(strlist);
    debug_return_ptr(NULL);
}

/*
 * Set passprompt regex filter based on a comma-separated string.
 * Returns a passprompt regex handle pointer.
 */
static void *
set_passprompt_regex(const char *cstr)
{
    void *handle;
    char *cp, *str, *last = NULL;
    debug_decl(set_passprompt_regex, SUDOERS_DEBUG_UTIL);

    handle = iolog_pwfilt_alloc();
    str = strdup(cstr);
    if (handle == NULL || str == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto bad;
    }

    for ((cp = strtok_r(str, ",", &last)); cp != NULL;
	    (cp = strtok_r(NULL, ",", &last))) {
	unescape_string(cp);
	if (!iolog_pwfilt_add(handle, cp))
	    goto bad;
    }

    free(str);
    debug_return_ptr(handle);
bad:
    free(str);
    iolog_pwfilt_free(handle);
    debug_return_ptr(NULL);
}

/*
 * Pull out I/O log related data from user_info and command_info arrays.
 * Returns true if I/O logging is enabled, false if not and -1 on error.
 */
static int
iolog_deserialize_info(struct log_details *details, char * const user_info[],
    char * const command_info[], char * const argv[], char * const user_env[])
{
    const struct sudoers_context *ctx = sudoers_get_context();
    struct eventlog *evlog;
    const char *runas_uid_str = "0", *runas_euid_str = NULL;
    const char *runas_gid_str = "0", *runas_egid_str = NULL;
    const char *errstr;
    char idbuf[STRLEN_MAX_UNSIGNED(uid_t) + 2];
    char * const *cur;
    struct passwd *pw;
    struct group *gr;
    id_t id;
    debug_decl(iolog_deserialize_info, SUDOERS_DEBUG_UTIL);

    if ((evlog = calloc(1, sizeof(*evlog))) == NULL)
	goto oom;
    details->evlog = evlog;

    evlog->lines = 24;
    evlog->columns = 80;
    evlog->runuid = ROOT_UID;
    evlog->rungid = 0;
    sudo_gettime_real(&evlog->submit_time);

    for (cur = user_info; *cur != NULL; cur++) {
	switch (**cur) {
	case 'c':
	    if (strncmp(*cur, "cols=", sizeof("cols=") - 1) == 0) {
		int n = (int)sudo_strtonum(*cur + sizeof("cols=") - 1, 1,
		    INT_MAX, NULL);
		if (n > 0)
		    evlog->columns = n;
		continue;
	    }
	    if (strncmp(*cur, "cwd=", sizeof("cwd=") - 1) == 0) {
		free(evlog->cwd);
		evlog->cwd = strdup(*cur + sizeof("cwd=") - 1);
		if (evlog->cwd == NULL)
		    goto oom;
		continue;
	    }
	    break;
	case 'h':
	    if (strncmp(*cur, "host=", sizeof("host=") - 1) == 0) {
		free(evlog->submithost);
		evlog->submithost = strdup(*cur + sizeof("host=") - 1);
		if (evlog->submithost == NULL)
		    goto oom;
		continue;
	    }
	    break;
	case 'l':
	    if (strncmp(*cur, "lines=", sizeof("lines=") - 1) == 0) {
		int n = (int)sudo_strtonum(*cur + sizeof("lines=") - 1, 1,
		    INT_MAX, NULL);
		if (n > 0)
		    evlog->lines = n;
		continue;
	    }
	    break;
	case 't':
	    if (strncmp(*cur, "tty=", sizeof("tty=") - 1) == 0) {
		free(evlog->ttyname);
		evlog->ttyname = strdup(*cur + sizeof("tty=") - 1);
		if (evlog->ttyname == NULL)
		    goto oom;
		continue;
	    }
	    break;
	case 'u':
	    if (strncmp(*cur, "user=", sizeof("user=") - 1) == 0) {
		free(evlog->submituser);
		evlog->submituser = strdup(*cur + sizeof("user=") - 1);
		if (evlog->submituser == NULL)
		    goto oom;
		continue;
	    }
	    break;
	}
    }

    for (cur = command_info; *cur != NULL; cur++) {
	switch (**cur) {
	case 'c':
	    if (strncmp(*cur, "command=", sizeof("command=") - 1) == 0) {
		free(evlog->command);
		evlog->command = strdup(*cur + sizeof("command=") - 1);
		if (evlog->command == NULL)
		    goto oom;
		continue;
	    }
	    if (strncmp(*cur, "chroot=", sizeof("chroot=") - 1) == 0) {
		free(evlog->runchroot);
		evlog->runchroot = strdup(*cur + sizeof("chroot=") - 1);
		if (evlog->runchroot == NULL)
		    goto oom;
		continue;
	    }
	    break;
	case 'i':
	    if (strncmp(*cur, "ignore_iolog_errors=", sizeof("ignore_iolog_errors=") - 1) == 0) {
		if (sudo_strtobool(*cur + sizeof("ignore_iolog_errors=") - 1) == true)
		    details->ignore_log_errors = true;
		continue;
	    }
	    if (strncmp(*cur, "iolog_path=", sizeof("iolog_path=") - 1) == 0) {
		free(evlog->iolog_path);
		evlog->iolog_path = strdup(*cur + sizeof("iolog_path=") - 1);
		if (evlog->iolog_path == NULL)
		    goto oom;
		continue;
	    }
	    if (strncmp(*cur, "iolog_stdin=", sizeof("iolog_stdin=") - 1) == 0) {
		if (sudo_strtobool(*cur + sizeof("iolog_stdin=") - 1) == true)
		    iolog_files[IOFD_STDIN].enabled = true;
		continue;
	    }
	    if (strncmp(*cur, "iolog_stdout=", sizeof("iolog_stdout=") - 1) == 0) {
		if (sudo_strtobool(*cur + sizeof("iolog_stdout=") - 1) == true)
		    iolog_files[IOFD_STDOUT].enabled = true;
		continue;
	    }
	    if (strncmp(*cur, "iolog_stderr=", sizeof("iolog_stderr=") - 1) == 0) {
		if (sudo_strtobool(*cur + sizeof("iolog_stderr=") - 1) == true)
		    iolog_files[IOFD_STDERR].enabled = true;
		continue;
	    }
	    if (strncmp(*cur, "iolog_ttyin=", sizeof("iolog_ttyin=") - 1) == 0) {
		if (sudo_strtobool(*cur + sizeof("iolog_ttyin=") - 1) == true)
		    iolog_files[IOFD_TTYIN].enabled = true;
		continue;
	    }
	    if (strncmp(*cur, "iolog_ttyout=", sizeof("iolog_ttyout=") - 1) == 0) {
		if (sudo_strtobool(*cur + sizeof("iolog_ttyout=") - 1) == true)
		    iolog_files[IOFD_TTYOUT].enabled = true;
		continue;
	    }
	    if (strncmp(*cur, "iolog_compress=", sizeof("iolog_compress=") - 1) == 0) {
		int val = sudo_strtobool(*cur + sizeof("iolog_compress=") - 1);
		if (val != -1) {
		    iolog_set_compress(val);
		} else {
		    sudo_debug_printf(SUDO_DEBUG_WARN,
			"%s: unable to parse %s", __func__, *cur);
		}
		continue;
	    }
	    if (strncmp(*cur, "iolog_flush=", sizeof("iolog_flush=") - 1) == 0) {
		int val = sudo_strtobool(*cur + sizeof("iolog_flush=") - 1);
		if (val != -1) {
		    iolog_set_flush(val);
		} else {
		    sudo_debug_printf(SUDO_DEBUG_WARN,
			"%s: unable to parse %s", __func__, *cur);
		}
		continue;
	    }
	    if (strncmp(*cur, "iolog_mode=", sizeof("iolog_mode=") - 1) == 0) {
		mode_t mode = sudo_strtomode(*cur + sizeof("iolog_mode=") - 1, &errstr);
		if (errstr == NULL) {
		    iolog_set_mode(mode);
		} else {
		    sudo_debug_printf(SUDO_DEBUG_WARN,
			"%s: unable to parse %s", __func__, *cur);
		}
		continue;
	    }
	    if (strncmp(*cur, "iolog_group=", sizeof("iolog_group=") - 1) == 0) {
		gr = sudo_getgrnam(*cur + sizeof("iolog_group=") - 1);
		if (gr == NULL) {
		    sudo_debug_printf(SUDO_DEBUG_WARN, "%s: unknown group %s",
			__func__, *cur + sizeof("iolog_group=") - 1);
		} else {
		    iolog_set_gid(gr->gr_gid);
		    sudo_gr_delref(gr);
		}
		continue;
	    }
	    if (strncmp(*cur, "iolog_user=", sizeof("iolog_user=") - 1) == 0) {
		pw = sudo_getpwnam(*cur + sizeof("iolog_user=") - 1);
		if (pw == NULL) {
		    sudo_debug_printf(SUDO_DEBUG_WARN, "%s: unknown user %s",
			__func__, *cur + sizeof("iolog_user=") - 1);
		} else {
		    iolog_set_owner(pw->pw_uid, pw->pw_gid);
		    sudo_pw_delref(pw);
		}
		continue;
	    }
	    break;
	case 'l':
	    if (strncmp(*cur, "log_passwords=", sizeof("log_passwords=") - 1) == 0) {
		int val = sudo_strtobool(*cur + sizeof("log_passwords=") - 1);
		if (val != -1) {
		    log_passwords = val;
		} else {
		    sudo_debug_printf(SUDO_DEBUG_WARN,
			"%s: unable to parse %s", __func__, *cur);
		}
		continue;
	    }
	    if (strncmp(*cur, "log_servers=", sizeof("log_servers=") - 1) == 0) {
		details->log_servers =
		    deserialize_stringlist(*cur + sizeof("log_servers=") - 1);
		if (!details->log_servers)
		    goto oom;
		continue;
	    }
	    if (strncmp(*cur, "log_server_timeout=", sizeof("log_server_timeout=") - 1) == 0) {
		details->server_timeout.tv_sec = (time_t)
		    sudo_strtonum(*cur + sizeof("log_server_timeout=") - 1, 1,
		    TIME_T_MAX, NULL);
		continue;
	    }
            if (strncmp(*cur, "log_server_keepalive=", sizeof("log_server_keepalive=") - 1) == 0) {
                int val = sudo_strtobool(*cur + sizeof("log_server_keepalive=") - 1);
                if (val != -1) {
                    details->keepalive = val;
                } else {
                    sudo_debug_printf(SUDO_DEBUG_WARN,
                        "%s: unable to parse %s", __func__, *cur);
                }
                continue;
            }
#if defined(HAVE_OPENSSL)
            if (strncmp(*cur, "log_server_cabundle=", sizeof("log_server_cabundle=") - 1) == 0) {
		free(details->ca_bundle);
                details->ca_bundle = strdup(*cur + sizeof("log_server_cabundle=") - 1);
		if (details->ca_bundle == NULL)
		    goto oom;
                continue;
            }
            if (strncmp(*cur, "log_server_peer_cert=", sizeof("log_server_peer_cert=") - 1) == 0) {
		free(details->cert_file);
                details->cert_file = strdup(*cur + sizeof("log_server_peer_cert=") - 1);
		if (details->cert_file == NULL)
		    goto oom;
                continue;
            }
            if (strncmp(*cur, "log_server_peer_key=", sizeof("log_server_peer_key=") - 1) == 0) {
		free(details->key_file);
                details->key_file = strdup(*cur + sizeof("log_server_peer_key=") - 1);
		if (details->key_file == NULL)
		    goto oom;
                continue;
            }
            if (strncmp(*cur, "log_server_verify=", sizeof("log_server_verify=") - 1) == 0) {
                int val = sudo_strtobool(*cur + sizeof("log_server_verify=") - 1);
                if (val != -1) {
                    details->verify_server = val;
                } else {
                    sudo_debug_printf(SUDO_DEBUG_WARN,
                        "%s: unable to parse %s", __func__, *cur);
                }
                continue;
            }
#endif /* HAVE_OPENSSL */
	    break;
	case 'm':
	    if (strncmp(*cur, "maxseq=", sizeof("maxseq=") - 1) == 0) {
		union sudo_defs_val sd_un;
		sd_un.str = *cur + sizeof("maxseq=") - 1;
		cb_maxseq(NULL, "policy", -1, -1, &sd_un, true);
		continue;
	    }
	    break;
	case 'p':
	    if (strncmp(*cur, "passprompt_regex=", sizeof("passprompt_regex=") - 1) == 0) {
		iolog_pwfilt_free(passprompt_regex_handle);
		passprompt_regex_handle =
		    set_passprompt_regex(*cur + sizeof("passprompt_regex=") - 1);
		if (passprompt_regex_handle == NULL)
		    debug_return_int(-1);
	    }
	    break;
	case 'r':
	    if (strncmp(*cur, "runas_gid=", sizeof("runas_gid=") - 1) == 0) {
		runas_gid_str = *cur + sizeof("runas_gid=") - 1;
		continue;
	    }
	    if (strncmp(*cur, "runas_egid=", sizeof("runas_egid=") - 1) == 0) {
		runas_egid_str = *cur + sizeof("runas_egid=") - 1;
		continue;
	    }
	    if (strncmp(*cur, "runas_uid=", sizeof("runas_uid=") - 1) == 0) {
		runas_uid_str = *cur + sizeof("runas_uid=") - 1;
		continue;
	    }
	    if (strncmp(*cur, "runas_euid=", sizeof("runas_euid=") - 1) == 0) {
		runas_euid_str = *cur + sizeof("runas_euid=") - 1;
		continue;
	    }
	    if (strncmp(*cur, "runcwd=", sizeof("runcwd=") - 1) == 0) {
		free(evlog->runcwd);
		evlog->runcwd = strdup(*cur + sizeof("runcwd=") - 1);
		if (evlog->runcwd == NULL)
		    goto oom;
		continue;
	    }
	    break;
	case 's':
	    if (strncmp(*cur, "source=", sizeof("source=") - 1) == 0) {
		free(evlog->source);
		evlog->source = strdup(*cur + sizeof("source=") - 1);
		if (evlog->source == NULL)
		    goto oom;
		continue;
	    }
	}
    }

    if (argv != NULL) {
	evlog->runargv = copy_vector_shallow(argv);
	if (evlog->runargv == NULL)
	    goto oom;
    }
    if (user_env != NULL) {
	evlog->runenv = copy_vector_shallow(user_env);
	if (evlog->runenv ==  NULL)
	    goto oom;
    }
    if (ctx->user.envp != NULL) {
	evlog->submitenv = copy_vector_shallow(ctx->user.envp);
	if (evlog->submitenv ==  NULL)
	    goto oom;
    }

    /*
     * Lookup runas user and group, preferring effective over real uid/gid.
     */
    if (runas_euid_str != NULL)
	runas_uid_str = runas_euid_str;
    if (runas_uid_str != NULL) {
	id = sudo_strtoid(runas_uid_str, &errstr);
	if (errstr != NULL)
	    sudo_warnx("runas uid %s: %s", runas_uid_str, U_(errstr));
	else
	    evlog->runuid = (uid_t)id;
    }
    if (runas_egid_str != NULL)
	runas_gid_str = runas_egid_str;
    if (runas_gid_str != NULL) {
	id = sudo_strtoid(runas_gid_str, &errstr);
	if (errstr != NULL)
	    sudo_warnx("runas gid %s: %s", runas_gid_str, U_(errstr));
	else
	    evlog->rungid = (gid_t)id;
    }

    pw = sudo_getpwuid(evlog->runuid);
    if (pw != NULL) {
	gid_t pw_gid = pw->pw_gid;
	free(evlog->runuser);
	evlog->runuser = strdup(pw->pw_name);
	sudo_pw_delref(pw);
	if (evlog->runuser == NULL)
	    goto oom;
	if (evlog->rungid != pw_gid) {
	    gr = sudo_getgrgid(evlog->rungid);
	    if (gr != NULL) {
		free(evlog->rungroup);
		evlog->rungroup = strdup(gr->gr_name);
		sudo_gr_delref(gr);
		if (evlog->rungroup == NULL)
		    goto oom;
	    } else {
		idbuf[0] = '#';
		strlcpy(&idbuf[1], runas_gid_str, sizeof(idbuf) - 1);
		free(evlog->rungroup);
		evlog->rungroup = strdup(idbuf);
		if (evlog->rungroup == NULL)
		    goto oom;
	    }
	}
    } else {
	idbuf[0] = '#';
	strlcpy(&idbuf[1], runas_uid_str, sizeof(idbuf) - 1);
	free(evlog->runuser);
	evlog->runuser = strdup(idbuf);
	if (evlog->runuser == NULL)
	    goto oom;
    }

    debug_return_int(
	iolog_files[IOFD_STDIN].enabled || iolog_files[IOFD_STDOUT].enabled ||
	iolog_files[IOFD_STDERR].enabled || iolog_files[IOFD_TTYIN].enabled ||
	iolog_files[IOFD_TTYOUT].enabled);
oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    debug_return_int(-1);
}

static int
sudoers_io_open_local(struct timespec *now)
{
    const struct sudoers_context *ctx = sudoers_get_context();
    struct eventlog *evlog = iolog_details.evlog;
    int i, ret = -1;
    debug_decl(sudoers_io_open_local, SUDOERS_DEBUG_PLUGIN);

    /* If no I/O log path defined we need to figure it out ourselves. */
    if (evlog->iolog_path == NULL) {
	int len;

	/* Get next session ID and convert it into a path. */
	if (!iolog_nextid(_PATH_SUDO_IO_LOGDIR, evlog->sessid)) {
	    log_warning(ctx, SLOG_SEND_MAIL,
		N_("unable to update sequence file"));
	    warned = true;
	    goto done;
	}
	len = asprintf(&evlog->iolog_path, "%s/%c%c/%c%c/%c%c",
	    _PATH_SUDO_IO_LOGDIR,
	    evlog->sessid[0], evlog->sessid[1], evlog->sessid[2],
	    evlog->sessid[3], evlog->sessid[4], evlog->sessid[5]);
	if (len == -1) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    goto done;
	}
    }

    /*
     * Create I/O log path along with any intermediate subdirs.
     * Calls mkdtemp() if iolog_path ends in XXXXXX.
     */
    if (!iolog_mkpath(evlog->iolog_path)) {
	log_warning(ctx, SLOG_SEND_MAIL, "%s", evlog->iolog_path);
	warned = true;
	goto done;
    }

    iolog_dir_fd = iolog_openat(AT_FDCWD, evlog->iolog_path, O_RDONLY);
    if (iolog_dir_fd == -1) {
	log_warning(ctx, SLOG_SEND_MAIL, "%s", evlog->iolog_path);
	warned = true;
	goto done;
    }

    /* Write log file with user and command details. */
    if (!iolog_write_info_file(iolog_dir_fd, iolog_details.evlog)) {
	log_warningx(ctx, SLOG_SEND_MAIL,
	    N_("unable to write to I/O log file: %s"), strerror(errno));
	warned = true;
	goto done;
    }

    /* Create the timing and I/O log files. */
    for (i = 0; i < IOFD_MAX; i++) {
	if (!iolog_open(&iolog_files[i], iolog_dir_fd, i, "w")) {
	    log_warning(ctx, SLOG_SEND_MAIL, N_("unable to create %s/%s"),
		evlog->iolog_path, iolog_fd_to_name(i));
	    warned = true;
	    goto done;
	}
    }

    ret = true;

done:
    debug_return_int(ret);
}

#ifdef SUDOERS_LOG_CLIENT
static int
sudoers_io_open_remote(struct timespec *now)
{
    debug_decl(sudoers_io_open_remote, SUDOERS_DEBUG_PLUGIN);

    /* Open connection to log server, send hello and accept messages. */
    client_closure = log_server_open(&iolog_details, now, true, SEND_ACCEPT,
	NULL);
    if (client_closure != NULL)
	debug_return_int(1);

    debug_return_int(-1);
}
#endif /* SUDOERS_LOG_CLIENT */

static int
sudoers_io_open(unsigned int version, sudo_conv_t conversation,
    sudo_printf_t plugin_printf, char * const settings[],
    char * const user_info[], char * const command_info[],
    int argc, char * const argv[], char * const user_env[], char * const args[],
    const char **errstr)
{
    struct sudo_conf_debug_file_list debug_files = TAILQ_HEAD_INITIALIZER(debug_files);
    char * const *cur;
    const char *cp, *plugin_path = NULL;
    int ret = -1;
    debug_decl(sudoers_io_open, SUDOERS_DEBUG_PLUGIN);

    sudo_conv = conversation;
    sudo_printf = plugin_printf;
    if (sudoers_io.event_alloc != NULL)
	plugin_event_alloc = sudoers_io.event_alloc;

    bindtextdomain("sudoers", LOCALEDIR);

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
	goto done;

    /* If we have no command (because -V was specified) just return. */
    if (argc == 0)
	debug_return_int(true);

    /*
     * Pull iolog settings out of command_info.
     */
    ret = iolog_deserialize_info(&iolog_details, user_info, command_info,
	argv, user_env);
    if (ret != true)
	goto done;

    /* Initialize io_operations. */
    sudoers_io_setops();

    if (sudo_gettime_awake(&last_time) == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "%s: unable to get time of day", __func__);
	goto done;
    }

    /*
     * Create local I/O log file or connect to remote log server.
     */
    if ((ret = io_operations.open(&last_time)) != true)
	goto done;

    /*
     * Clear I/O log function pointers for disabled log functions.
     */
    if (!iolog_files[IOFD_STDIN].enabled)
	sudoers_io.log_stdin = NULL;
    if (!iolog_files[IOFD_STDOUT].enabled)
	sudoers_io.log_stdout = NULL;
    if (!iolog_files[IOFD_STDERR].enabled)
	sudoers_io.log_stderr = NULL;
    if (!iolog_files[IOFD_TTYIN].enabled)
	sudoers_io.log_ttyin = NULL;
    if (!iolog_files[IOFD_TTYOUT].enabled)
	sudoers_io.log_ttyout = NULL;

done:
    if (ret != true) {
	if (iolog_dir_fd != -1) {
	    close(iolog_dir_fd);
	    iolog_dir_fd = -1;
	}
	free_iolog_details();
	sudo_freepwcache();
	sudo_freegrcache();
    }

    /* Ignore errors if they occur if the policy says so. */
    if (ret == -1 && iolog_details.ignore_log_errors)
	ret = 0;

    debug_return_int(ret);
}

static void
sudoers_io_close_local(int exit_status, int error, const char **errstr)
{
    unsigned int i;
    debug_decl(sudoers_io_close_local, SUDOERS_DEBUG_PLUGIN);

    /* Close the files. */
    for (i = 0; i < IOFD_MAX; i++) {
	if (iolog_files[i].fd.v == NULL)
	    continue;
	iolog_close(&iolog_files[i], errstr);
    }

    /* Clear write bits from I/O timing file to indicate completion. */
    if (iolog_dir_fd != -1) {
	struct stat sb;
	if (fstatat(iolog_dir_fd, "timing", &sb, 0) != -1) {
	    CLR(sb.st_mode, S_IWUSR|S_IWGRP|S_IWOTH);
	    if (fchmodat(iolog_dir_fd, "timing", sb.st_mode, 0) == -1) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
		    "%s: unable to fchmodat timing file", __func__);
	    }
	}
	close(iolog_dir_fd);
	iolog_dir_fd = -1;
    }

    debug_return;
}

#ifdef SUDOERS_LOG_CLIENT
static void
sudoers_io_close_remote(int exit_status, int error, const char **errstr)
{
    debug_decl(sudoers_io_close_remote, SUDOERS_DEBUG_PLUGIN);

    log_server_close(client_closure, exit_status, error);
    client_closure = NULL;

    debug_return;
}
#endif

static void
sudoers_io_close(int exit_status, int error)
{
    const struct sudoers_context *ctx = sudoers_get_context();
    const char *errstr = NULL;
    debug_decl(sudoers_io_close, SUDOERS_DEBUG_PLUGIN);

    if (io_operations.close != NULL)
	io_operations.close(exit_status, error, &errstr);

    if (errstr != NULL && !warned) {
	/* Only warn about I/O log file errors once. */
	log_warningx(ctx, SLOG_SEND_MAIL,
	    N_("unable to write to I/O log file: %s"), errstr);
	warned = true;
    }

    free_iolog_details();
    sudo_freepwcache();
    sudo_freegrcache();
    iolog_pwfilt_free(passprompt_regex_handle);
    passprompt_regex_handle = NULL;

    /* sudoers_debug_deregister() calls sudo_debug_exit() for us. */
    sudoers_debug_deregister();
}

static int
sudoers_io_version(int verbose)
{
    debug_decl(sudoers_io_version, SUDOERS_DEBUG_PLUGIN);

    sudo_printf(SUDO_CONV_INFO_MSG, "Sudoers I/O plugin version %s\n",
	PACKAGE_VERSION);

    debug_return_int(true);
}

/*
 * Write an I/O log entry to the local file system.
 * Returns 1 on success and -1 on error.
 * Fills in errstr on error.
 */
static int
sudoers_io_log_local(int event, const char *buf, unsigned int len,
    struct timespec *delay, const char **errstr)
{
    struct iolog_file *iol;
    char tbuf[1024];
    char *newbuf = NULL;
    int ret = -1;
    debug_decl(sudoers_io_log_local, SUDOERS_DEBUG_PLUGIN);

    if (event < 0 || event >= IOFD_MAX) {
	*errstr = NULL;
	sudo_warnx(U_("unexpected I/O event %d"), event);
	debug_return_int(-1);
    }
    iol = &iolog_files[event];
    if (!iol->enabled) {
	*errstr = NULL;
	sudo_warnx(U_("%s: internal error, I/O log file for event %d not open"),
	    __func__, event);
	debug_return_int(-1);
    }

    if (!log_passwords && passprompt_regex_handle != NULL) {
	if (!iolog_pwfilt_run(passprompt_regex_handle, event, buf, len, &newbuf))
	    debug_return_int(-1);
    }

    /* Write I/O log file entry. */
    if (iolog_write(iol, newbuf ? newbuf : buf, len, errstr) == -1)
	goto done;

    /* Write timing file entry. */
    len = (unsigned int)snprintf(tbuf, sizeof(tbuf), "%d %lld.%09ld %u\n",
	event, (long long)delay->tv_sec, delay->tv_nsec, len);
    if (len >= sizeof(tbuf)) {
	/* Not actually possible due to the size of tbuf[]. */
	*errstr = strerror(EOVERFLOW);
	goto done;
    }
    if (iolog_write(&iolog_files[IOFD_TIMING], tbuf, len, errstr) == -1)
	goto done;

    /* Success. */
    ret = 1;

done:
    free(newbuf);
    debug_return_int(ret);
}

#ifdef SUDOERS_LOG_CLIENT
/*
 * Schedule an I/O log entry to be written to the log server.
 * Returns 1 on success and -1 on error.
 * Fills in errstr on error.
 */
static int
sudoers_io_log_remote(int event, const char *buf, unsigned int len,
    struct timespec *delay, const char **errstr)
{
    int type, ret = -1;
    debug_decl(sudoers_io_log_remote, SUDOERS_DEBUG_PLUGIN);

    if (client_closure->disabled)
	debug_return_int(1);

    /* Track elapsed time for comparison with commit points. */
    sudo_timespecadd(delay, &client_closure->elapsed, &client_closure->elapsed);

    switch (event) {
    case IO_EVENT_STDIN:
	type = CLIENT_MESSAGE__TYPE_STDIN_BUF;
	break;
    case IO_EVENT_STDOUT:
	type = CLIENT_MESSAGE__TYPE_STDOUT_BUF;
	break;
    case IO_EVENT_STDERR:
	type = CLIENT_MESSAGE__TYPE_STDERR_BUF;
	break;
    case IO_EVENT_TTYIN:
	type = CLIENT_MESSAGE__TYPE_TTYIN_BUF;
	break;
    case IO_EVENT_TTYOUT:
	type = CLIENT_MESSAGE__TYPE_TTYOUT_BUF;
	break;
    default:
	sudo_warnx(U_("unexpected I/O event %d"), event);
	goto done;
    }
    if (fmt_io_buf(client_closure, type, buf, len, delay)) {
	ret = client_closure->write_ev->add(client_closure->write_ev,
	    &iolog_details.server_timeout);
	if (ret == -1)
	    sudo_warn("%s", U_("unable to add event to queue"));
    }

done:
    debug_return_int(ret);
}
#endif /* SUDOERS_LOG_CLIENT */

/*
 * Generic I/O logging function.  Called by the I/O logging entry points.
 * Returns 1 on success and -1 on error.
 */
static int
sudoers_io_log(const char *buf, unsigned int len, int event, const char **errstr)
{
    const struct sudoers_context *ctx = sudoers_get_context();
    struct timespec now, delay;
    const char *ioerror = NULL;
    int ret = -1;
    debug_decl(sudoers_io_log, SUDOERS_DEBUG_PLUGIN);

    if (sudo_gettime_awake(&now) == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "%s: unable to get time of day", __func__);
	ioerror = N_("unable to read the clock");
	goto bad;
    }
    sudo_timespecsub(&now, &last_time, &delay);

    ret = io_operations.log(event, buf, len, &delay, &ioerror);

    last_time.tv_sec = now.tv_sec;
    last_time.tv_nsec = now.tv_nsec;

bad:
    if (ret == -1) {
	if (ioerror != NULL) {
	    char *cp;

	    if (asprintf(&cp, N_("unable to write to I/O log file: %s"),
		    ioerror) != -1) {
		*errstr = cp;
	    }
	    if (!warned) {
		/* Only warn about I/O log file errors once. */
		log_warningx(ctx, SLOG_SEND_MAIL,
		    N_("unable to write to I/O log file: %s"), ioerror);
		warned = true;
	    }
	}

	/* Ignore errors if they occur if the policy says so. */
	if (iolog_details.ignore_log_errors)
	    ret = 1;
    }

    debug_return_int(ret);
}

static int
sudoers_io_log_stdin(const char *buf, unsigned int len, const char **errstr)
{
    return sudoers_io_log(buf, len, IO_EVENT_STDIN, errstr);
}

static int
sudoers_io_log_stdout(const char *buf, unsigned int len, const char **errstr)
{
    return sudoers_io_log(buf, len, IO_EVENT_STDOUT, errstr);
}

static int
sudoers_io_log_stderr(const char *buf, unsigned int len, const char **errstr)
{
    return sudoers_io_log(buf, len, IO_EVENT_STDERR, errstr);
}

static int
sudoers_io_log_ttyin(const char *buf, unsigned int len, const char **errstr)
{
    return sudoers_io_log(buf, len, IO_EVENT_TTYIN, errstr);
}

static int
sudoers_io_log_ttyout(const char *buf, unsigned int len, const char **errstr)
{
    return sudoers_io_log(buf, len, IO_EVENT_TTYOUT, errstr);
}

static int
sudoers_io_change_winsize_local(unsigned int lines, unsigned int cols,
    struct timespec *delay, const char **errstr)
{
    char tbuf[1024];
    int len, ret = -1;
    debug_decl(sudoers_io_change_winsize_local, SUDOERS_DEBUG_PLUGIN);

    /* Write window change event to the timing file. */
    len = snprintf(tbuf, sizeof(tbuf), "%d %lld.%09ld %u %u\n",
	IO_EVENT_WINSIZE, (long long)delay->tv_sec, delay->tv_nsec,
	lines, cols);
    if (len < 0 || len >= ssizeof(tbuf)) {
	/* Not actually possible due to the size of tbuf[]. */
	*errstr = strerror(EOVERFLOW);
	goto done;
    }
    if (iolog_write(&iolog_files[IOFD_TIMING], tbuf, (size_t)len, errstr) == -1)
	goto done;

    /* Success. */
    ret = 1;

done:
    debug_return_int(ret);
}

#ifdef SUDOERS_LOG_CLIENT
static int
sudoers_io_change_winsize_remote(unsigned int lines, unsigned int cols,
    struct timespec *delay, const char **errstr)
{
    int ret = -1;
    debug_decl(sudoers_io_change_winsize_remote, SUDOERS_DEBUG_PLUGIN);

    if (client_closure->disabled)
	debug_return_int(1);

    /* Track elapsed time for comparison with commit points. */
    sudo_timespecadd(delay, &client_closure->elapsed, &client_closure->elapsed);

    if (fmt_winsize(client_closure, lines, cols, delay)) {
	ret = client_closure->write_ev->add(client_closure->write_ev,
	    &iolog_details.server_timeout);
	if (ret == -1)
	    sudo_warn("%s", U_("unable to add event to queue"));
    }

    debug_return_int(ret);
}
#endif /* SUDOERS_LOG_CLIENT */

static int
sudoers_io_change_winsize(unsigned int lines, unsigned int cols, const char **errstr)
{
    const struct sudoers_context *ctx = sudoers_get_context();
    struct timespec now, delay;
    const char *ioerror = NULL;
    int ret = -1;
    debug_decl(sudoers_io_change_winsize, SUDOERS_DEBUG_PLUGIN);

    if (sudo_gettime_awake(&now) == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "%s: unable to get time of day", __func__);
	ioerror = N_("unable to read the clock");
	goto bad;
    }
    sudo_timespecsub(&now, &last_time, &delay);

    ret = io_operations.change_winsize(lines, cols, &delay, &ioerror);

    last_time.tv_sec = now.tv_sec;
    last_time.tv_nsec = now.tv_nsec;

bad:
    if (ret == -1) {
	if (ioerror != NULL && !warned) {
	    char *cp;

	    if (asprintf(&cp, N_("unable to write to I/O log file: %s"),
		    ioerror) != -1) {
		*errstr = cp;
	    }
	    if (!warned) {
		/* Only warn about I/O log file errors once. */
		log_warningx(ctx, SLOG_SEND_MAIL,
		    N_("unable to write to I/O log file: %s"), ioerror);
		warned = true;
	    }
	}

	/* Ignore errors if they occur if the policy says so. */
	if (iolog_details.ignore_log_errors)
	    ret = 1;
    }

    debug_return_int(ret);
}

static int
sudoers_io_suspend_local(const char *signame, struct timespec *delay,
    const char **errstr)
{
    unsigned int len;
    char tbuf[1024];
    int ret = -1;
    debug_decl(sudoers_io_suspend_local, SUDOERS_DEBUG_PLUGIN);

    /* Write suspend event to the timing file. */
    len = (unsigned int)snprintf(tbuf, sizeof(tbuf), "%d %lld.%09ld %s\n",
	IO_EVENT_SUSPEND, (long long)delay->tv_sec, delay->tv_nsec, signame);
    if (len >= sizeof(tbuf)) {
	/* Not actually possible due to the size of tbuf[]. */
	*errstr = strerror(EOVERFLOW);
	goto done;
    }
    if (iolog_write(&iolog_files[IOFD_TIMING], tbuf, len, errstr) == -1)
	goto done;

    /* Success. */
    ret = 1;

done:
    debug_return_int(ret);
}

#ifdef SUDOERS_LOG_CLIENT
static int
sudoers_io_suspend_remote(const char *signame, struct timespec *delay,
    const char **errstr)
{
    int ret = -1;
    debug_decl(sudoers_io_suspend_remote, SUDOERS_DEBUG_PLUGIN);

    if (client_closure->disabled)
	debug_return_int(1);

    /* Track elapsed time for comparison with commit points. */
    sudo_timespecadd(delay, &client_closure->elapsed, &client_closure->elapsed);

    if (fmt_suspend(client_closure, signame, delay)) {
	ret = client_closure->write_ev->add(client_closure->write_ev,
	    &iolog_details.server_timeout);
	if (ret == -1)
	    sudo_warn("%s", U_("unable to add event to queue"));
    }

    debug_return_int(ret);
}
#endif /* SUDOERS_LOG_CLIENT */

static int
sudoers_io_suspend(int signo, const char **errstr)
{
    const struct sudoers_context *ctx = sudoers_get_context();
    struct timespec now, delay;
    char signame[SIG2STR_MAX];
    const char *ioerror = NULL;
    int ret = -1;
    debug_decl(sudoers_io_suspend, SUDOERS_DEBUG_PLUGIN);

    if (signo <= 0 || sig2str(signo, signame) == -1) {
	sudo_warnx(U_("%s: internal error, invalid signal %d"),
	    __func__, signo);
	debug_return_int(-1);
    }

    if (sudo_gettime_awake(&now) == -1) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_ERRNO,
	    "%s: unable to get time of day", __func__);
	ioerror = N_("unable to read the clock");
	goto bad;
    }
    sudo_timespecsub(&now, &last_time, &delay);

    /* Write suspend event to the timing file. */
    ret = io_operations.suspend(signame, &delay, &ioerror);

    last_time.tv_sec = now.tv_sec;
    last_time.tv_nsec = now.tv_nsec;

bad:
    if (ret == -1) {
	if (ioerror != NULL && !warned) {
	    char *cp;

	    if (asprintf(&cp, N_("unable to write to I/O log file: %s"),
		    ioerror) != -1) {
		*errstr = cp;
	    }
	    if (!warned) {
		/* Only warn about I/O log file errors once. */
		log_warningx(ctx, SLOG_SEND_MAIL,
		    N_("unable to write to I/O log file: %s"), ioerror);
		warned = true;
	    }
	}

	/* Ignore errors if they occur if the policy says so. */
	if (iolog_details.ignore_log_errors)
	    ret = 1;
    }

    debug_return_int(ret);
}

/*
 * Fill in the contents of io_operations, either local or remote.
 */
static void
sudoers_io_setops(void)
{
    debug_decl(sudoers_io_setops, SUDOERS_DEBUG_PLUGIN);

#ifdef SUDOERS_LOG_CLIENT
    if (plugin_event_alloc != NULL && iolog_details.log_servers != NULL) {
	io_operations.open = sudoers_io_open_remote;
	io_operations.close = sudoers_io_close_remote;
	io_operations.log = sudoers_io_log_remote;
	io_operations.change_winsize = sudoers_io_change_winsize_remote;
	io_operations.suspend = sudoers_io_suspend_remote;
    } else
#endif /* SUDOERS_LOG_CLIENT */
    {
	io_operations.open = sudoers_io_open_local;
	io_operations.close = sudoers_io_close_local;
	io_operations.log = sudoers_io_log_local;
	io_operations.change_winsize = sudoers_io_change_winsize_local;
	io_operations.suspend = sudoers_io_suspend_local;
    }

    debug_return;
}

sudo_dso_public struct io_plugin sudoers_io = {
    SUDO_IO_PLUGIN,
    SUDO_API_VERSION,
    sudoers_io_open,
    sudoers_io_close,
    sudoers_io_version,
    sudoers_io_log_ttyin,
    sudoers_io_log_ttyout,
    sudoers_io_log_stdin,
    sudoers_io_log_stdout,
    sudoers_io_log_stderr,
    NULL, /* register_hooks */
    NULL, /* deregister_hooks */
    sudoers_io_change_winsize,
    sudoers_io_suspend,
    NULL /* event_alloc() filled in by sudo */
};
