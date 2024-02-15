/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2009-2022 Todd C. Miller <Todd.Miller@sudo.ws>
 * Copyright (c) 2008 Dan Walsh <dwalsh@redhat.com>
 *
 * Borrowed heavily from newrole source code
 * Authors:
 *	Anthony Colatrella
 *	Tim Fraser
 *	Steve Grubb <sgrubb@redhat.com>
 *	Darrel Goeddel <DGoeddel@trustedcs.com>
 *	Michael Thompson <mcthomps@us.ibm.com>
 *	Dan Walsh <dwalsh@redhat.com>
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

#ifdef HAVE_SELINUX

#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

#include <selinux/selinux.h>           /* for is_selinux_enabled() */
#include <selinux/context.h>           /* for context-mangling functions */
#include <selinux/get_default_type.h>
#include <selinux/get_context_list.h>

#ifdef HAVE_LINUX_AUDIT
# include <libaudit.h>
#endif

#include <sudo.h>
#include <sudo_exec.h>

static struct selinux_state {
    char * old_context;
    char * new_context;
    char * tty_con_raw;
    char * new_tty_con_raw;
    const char *ttyn;
    int ttyfd;
    int enforcing;
} se_state;

int
selinux_audit_role_change(void)
{
#ifdef HAVE_LINUX_AUDIT
    int au_fd, rc = -1;
    char *message;
    debug_decl(selinux_audit_role_change, SUDO_DEBUG_SELINUX);

    au_fd = audit_open();
    if (au_fd == -1) {
        /* Kernel may not have audit support. */
        if (errno != EINVAL && errno != EPROTONOSUPPORT && errno != EAFNOSUPPORT
)
            sudo_fatal("%s", U_("unable to open audit system"));
    } else {
	/* audit role change using the same format as newrole(1) */
	rc = asprintf(&message, "newrole: old-context=%s new-context=%s",
	    se_state.old_context, se_state.new_context ? se_state.new_context : "?");
	if (rc == -1)
	    sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	rc = audit_log_user_message(au_fd, AUDIT_USER_ROLE_CHANGE,
	    message, NULL, NULL, se_state.ttyn, se_state.new_context ? 1 : 0);
	if (rc <= 0)
	    sudo_warn("%s", U_("unable to send audit message"));
	free(message);
	close(au_fd);
    }

    debug_return_int(rc);
#else
    return 0;
#endif /* HAVE_LINUX_AUDIT */
}

/*
 * This function attempts to revert the relabeling done to the tty.
 * fd		   - referencing the opened ttyn
 * ttyn		   - name of tty to restore
 *
 * Returns 0 on success and -1 on failure.
 */
int
selinux_restore_tty(void)
{
    int ret = -1;
    char * chk_tty_con_raw = NULL;
    debug_decl(selinux_restore_tty, SUDO_DEBUG_SELINUX);

    if (se_state.ttyfd == -1 || se_state.new_tty_con_raw == NULL) {
	sudo_debug_printf(SUDO_DEBUG_INFO, "%s: no tty, skip relabel",
	    __func__);
	debug_return_int(0);
    }

    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: %s -> %s",
	__func__, se_state.new_tty_con_raw, se_state.tty_con_raw);

    /* Verify that the tty still has the context set by sudo. */
    if (fgetfilecon_raw(se_state.ttyfd, &chk_tty_con_raw) == -1) {
	sudo_warn(U_("unable to fgetfilecon %s"), se_state.ttyn);
	goto skip_relabel;
    }

    if (strcmp(chk_tty_con_raw, se_state.new_tty_con_raw) != 0) {
	sudo_warnx(U_("%s changed labels"), se_state.ttyn);
	sudo_debug_printf(SUDO_DEBUG_INFO,
	    "%s: not restoring tty label, expected %s, have %s",
	    __func__, se_state.new_tty_con_raw, chk_tty_con_raw);
	goto skip_relabel;
    }

    if (fsetfilecon_raw(se_state.ttyfd, se_state.tty_con_raw) == -1) {
	sudo_warn(U_("unable to restore context for %s"), se_state.ttyn);
	goto skip_relabel;
    }

    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: successfully set tty label to %s",
	__func__, se_state.tty_con_raw);
    ret = 0;

skip_relabel:
    if (se_state.ttyfd != -1) {
	close(se_state.ttyfd);
	se_state.ttyfd = -1;
    }
    freecon(chk_tty_con_raw);
    debug_return_int(ret);
}

/*
 * This function attempts to relabel the tty. If this function fails, then
 * the contexts are free'd and -1 is returned. On success, 0 is returned
 * and tty_con_raw and new_tty_con_raw are set.
 *
 * This function will not fail if it can not relabel the tty when selinux is
 * in permissive mode.
 */
int
selinux_relabel_tty(const char *ttyn, int ptyfd)
{
    char * tty_con = NULL;
    char * new_tty_con = NULL;
    struct stat sb;
    int fd;
    debug_decl(relabel_tty, SUDO_DEBUG_SELINUX);

    se_state.ttyfd = ptyfd;

    /* It is perfectly legal to have no tty. */
    if (ptyfd == -1 && ttyn == NULL) {
	sudo_debug_printf(SUDO_DEBUG_INFO, "%s: no tty, skip relabel",
	    __func__);
	debug_return_int(0);
    }
    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: relabeling tty %s", __func__, ttyn);

    /* If sudo is not allocating a pty for the command, open current tty. */
    if (ptyfd == -1) {
	se_state.ttyfd = open(ttyn, O_RDWR|O_NOCTTY|O_NONBLOCK);
	if (se_state.ttyfd == -1 || fstat(se_state.ttyfd, &sb) == -1) {
	    sudo_warn(U_("unable to open %s, not relabeling tty"), ttyn);
	    goto bad;
	}
	if (!S_ISCHR(sb.st_mode)) {
	    sudo_warn(U_("%s is not a character device, not relabeling tty"),
		ttyn);
	    goto bad;
	}
	(void)fcntl(se_state.ttyfd, F_SETFL,
	    fcntl(se_state.ttyfd, F_GETFL, 0) & ~O_NONBLOCK);
    }

    if (fgetfilecon(se_state.ttyfd, &tty_con) == -1) {
	sudo_warn("%s", U_("unable to get current tty context, not relabeling tty"));
	goto bad;
    }

    if (tty_con != NULL) {
	security_class_t tclass = string_to_security_class("chr_file");
	if (tclass == 0) {
	    sudo_warn("%s", U_("unknown security class \"chr_file\", not relabeling tty"));
	    goto bad;
	}
	if (security_compute_relabel(se_state.new_context, tty_con,
	    tclass, &new_tty_con) == -1) {
	    sudo_warn("%s", U_("unable to get new tty context, not relabeling tty"));
	    goto bad;
	}
    }

    if (new_tty_con != NULL) {
	sudo_debug_printf(SUDO_DEBUG_INFO, "%s: tty context %s -> %s",
	    __func__, tty_con, new_tty_con);
	if (fsetfilecon(se_state.ttyfd, new_tty_con) == -1) {
	    sudo_warn("%s", U_("unable to set new tty context"));
	    goto bad;
	}
    }

    if (ptyfd != -1) {
	int oflags, flags = 0;

	/* Reopen pty that was relabeled, std{in,out,err} are reset later. */
	se_state.ttyfd = open(ttyn, O_RDWR|O_NOCTTY, 0);
	if (se_state.ttyfd == -1 || fstat(se_state.ttyfd, &sb) == -1) {
	    sudo_warn(U_("unable to open %s"), ttyn);
	    goto bad;
	}
	if (!S_ISCHR(sb.st_mode)) {
	    sudo_warn(U_("%s is not a character device, not relabeling tty"),
		ttyn);
	    goto bad;
	}
	/* Preserve O_NONBLOCK and the close-on-exec flags. */
	if ((oflags = fcntl(ptyfd, F_GETFL)) == -1) {
	    sudo_warn("F_GETFL");
	    goto bad;
	}
	if (ISSET(oflags, O_NONBLOCK))
	    flags |= O_NONBLOCK;
	if ((oflags = fcntl(ptyfd, F_GETFD)) == -1) {
	    sudo_warn("F_GETFD");
	    goto bad;
	}
	if (ISSET(oflags, FD_CLOEXEC))
	    flags |= O_CLOEXEC;
	if (dup3(se_state.ttyfd, ptyfd, flags) == -1) {
	    sudo_warn("dup3");
	    goto bad;
	}
    } else {
	/* Re-open tty to get new label and reset std{in,out,err} */
	close(se_state.ttyfd);
	se_state.ttyfd = open(ttyn, O_RDWR|O_NOCTTY|O_NONBLOCK);
	if (se_state.ttyfd == -1 || fstat(se_state.ttyfd, &sb) == -1) {
	    sudo_warn(U_("unable to open %s"), ttyn);
	    goto bad;
	}
	if (!S_ISCHR(sb.st_mode)) {
	    sudo_warn(U_("%s is not a character device, not relabeling tty"),
		ttyn);
	    goto bad;
	}
	(void)fcntl(se_state.ttyfd, F_SETFL,
	    fcntl(se_state.ttyfd, F_GETFL, 0) & ~O_NONBLOCK);
	for (fd = STDIN_FILENO; fd <= STDERR_FILENO; fd++) {
	    if (sudo_isatty(fd, &sb) && dup2(se_state.ttyfd, fd) == -1) {
		sudo_warn("dup2");
		goto bad;
	    }
	}
    }
    /* Retain se_state.ttyfd so we can restore label when command finishes. */
    (void)fcntl(se_state.ttyfd, F_SETFD, FD_CLOEXEC);

    se_state.ttyn = ttyn;
    if (selinux_trans_to_raw_context(tty_con, &se_state.tty_con_raw) == -1)
	goto bad;
    if (selinux_trans_to_raw_context(new_tty_con, &se_state.new_tty_con_raw) == -1)
	goto bad;
    freecon(tty_con);
    freecon(new_tty_con);
    debug_return_int(0);

bad:
    if (se_state.ttyfd != -1 && se_state.ttyfd != ptyfd) {
	close(se_state.ttyfd);
	se_state.ttyfd = -1;
    }
    freecon(se_state.tty_con_raw);
    se_state.tty_con_raw = NULL;
    freecon(se_state.new_tty_con_raw);
    se_state.new_tty_con_raw = NULL;
    freecon(tty_con);
    freecon(new_tty_con);
    debug_return_int(se_state.enforcing ? -1 : 0);
}

/*
 * Determine the new security context based on the old context and the
 * specified role and type.
 * Returns 0 on success, and -1 on failure.
 */
static int
get_exec_context(const char *role, const char *type)
{
    char *new_context = NULL;
    context_t context = NULL;
    char *typebuf = NULL;
    int ret = -1;
    debug_decl(get_exec_context, SUDO_DEBUG_SELINUX);

    if (role == NULL) {
	sudo_warnx(U_("you must specify a role for type %s"), type);
	errno = EINVAL;
	goto done;
    }
    if (type == NULL) {
	if (get_default_type(role, &typebuf)) {
	    sudo_warnx(U_("unable to get default type for role %s"), role);
	    errno = EINVAL;
	    goto done;
	}
	type = typebuf;
    }

    /*
     * Expand old_context into a context_t so that we can extract and modify
     * its components easily.
     */
    if ((context = context_new(se_state.old_context)) == NULL) {
	sudo_warn("%s", U_("failed to get new context"));
	goto done;
    }

    /*
     * Replace the role and type in "context" with the role and
     * type we will be running the command as.
     */
    if (context_role_set(context, role)) {
	sudo_warn(U_("failed to set new role %s"), role);
	goto done;
    }
    if (context_type_set(context, type)) {
	sudo_warn(U_("failed to set new type %s"), type);
	goto done;
    }

    /*
     * Convert "context" back into a string and verify it.
     */
    if ((new_context = strdup(context_str(context))) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto done;
    }
    if (security_check_context(new_context) == -1) {
	sudo_warnx(U_("%s is not a valid context"), new_context);
	errno = EINVAL;
	goto done;
    }

    se_state.new_context = new_context;
    new_context = NULL;
    ret = 0;

done:
    free(typebuf);
    context_free(context);
    freecon(new_context);
    debug_return_int(ret);
}

/*
 * Determine the exec and tty contexts the command will run in.
 * Returns 0 on success and -1 on failure.
 */
int
selinux_getexeccon(const char *role, const char *type)
{
    int ret = -1;
    debug_decl(selinux_getexeccon, SUDO_DEBUG_SELINUX);

    /* Store the caller's SID in old_context. */
    if (getprevcon(&se_state.old_context)) {
	sudo_warn("%s", U_("failed to get old context"));
	goto done;
    }

    se_state.enforcing = security_getenforce();
    if (se_state.enforcing == -1) {
	sudo_warn("%s", U_("unable to determine enforcing mode."));
	goto done;
    }

    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: old context %s", __func__,
	se_state.old_context);
    ret = get_exec_context(role, type);
    if (ret == -1) {
	/* Audit role change failure (success is logged later). */
	selinux_audit_role_change();
	goto done;
    }
    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: new context %s", __func__,
	se_state.new_context);

done:
    debug_return_int(ret);
}

int
selinux_setexeccon(void)
{
    debug_decl(selinux_setexeccon, SUDO_DEBUG_SELINUX);

    if (setexeccon(se_state.new_context)) {
	sudo_warn(U_("unable to set exec context to %s"), se_state.new_context);
	if (se_state.enforcing)
	    debug_return_int(-1);
    }

#ifdef HAVE_SETKEYCREATECON
    if (setkeycreatecon(se_state.new_context)) {
	sudo_warn(U_("unable to set key creation context to %s"), se_state.new_context);
	if (se_state.enforcing)
	    debug_return_int(-1);
    }
#endif /* HAVE_SETKEYCREATECON */

    debug_return_int(0);
}

void
selinux_execve(int fd, const char *path, char *const argv[], char *envp[],
    const char *rundir, unsigned int flags)
{
    char **nargv;
    const char *sesh;
    int argc, len, nargc, serrno;
    debug_decl(selinux_execve, SUDO_DEBUG_SELINUX);

    sesh = sudo_conf_sesh_path();
    if (sesh == NULL) {
	sudo_warnx("internal error: sesh path not set");
	errno = EINVAL;
	debug_return;
    }

    /* Set SELinux exec and keycreate contexts. */
    if (selinux_setexeccon() == -1)
	debug_return;

    /*
     * Build new argv with sesh as argv[0].
     */
    for (argc = 0; argv[argc] != NULL; argc++)
	continue;
    if (argc == 0) {
	errno = EINVAL;
	debug_return;
    }
    nargv = reallocarray(NULL, 5 + argc + 1, sizeof(char *));
    if (nargv == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return;
    }
    if (*argv[0] == '-')
	nargv[0] = (char *)"-sesh";
    else
	nargv[0] = (char *)"sesh";
    nargc = 1;
    if (ISSET(flags, CD_RBAC_SET_CWD)) {
	const char *prefix = ISSET(flags, CD_CWD_OPTIONAL) ? "+" : "";
	if (rundir == NULL) {
	    sudo_warnx("internal error: sesh rundir not set");
	    errno = EINVAL;
	    debug_return;
	}
	len = asprintf(&nargv[nargc++], "--directory=%s%s", prefix, rundir);
	if (len == -1) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    debug_return;
	}
    }
    if (fd != -1) {
	len = asprintf(&nargv[nargc++], "--execfd=%d", fd);
	if (len == -1) {
	    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    debug_return;
	}
    }
    if (ISSET(flags, CD_NOEXEC)) {
	CLR(flags, CD_NOEXEC);
	nargv[nargc++] = (char *)"--noexec";
    }
    nargv[nargc++] = (char *)"--";
    nargv[nargc++] = (char *)path;
    memcpy(&nargv[nargc], &argv[1], argc * sizeof(char *)); /* copies NULL */

    sudo_execve(-1, sesh, nargv, envp, -1, flags);
    serrno = errno;
    free(nargv);
    errno = serrno;
    debug_return;
}

#endif /* HAVE_SELINUX */
