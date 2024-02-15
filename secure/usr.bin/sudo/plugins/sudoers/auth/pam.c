/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1999-2005, 2007-2020 Todd C. Miller <Todd.Miller@sudo.ws>
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
 *
 * Sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and Air Force Research Laboratory, Air Force
 * Materiel Command, USAF, under agreement number F39502-99-1-0512.
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#ifdef HAVE_PAM

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>

#ifdef HAVE_PAM_PAM_APPL_H
# include <pam/pam_appl.h>
#else
# include <security/pam_appl.h>
#endif

#ifdef __hpux
# include <nl_types.h>
#endif

#ifdef HAVE_LIBINTL_H
# if defined(__LINUX_PAM__)
#  define PAM_TEXT_DOMAIN	"Linux-PAM"
# elif defined(__sun__)
#  define PAM_TEXT_DOMAIN	"SUNW_OST_SYSOSPAM"
# endif
#endif

/* We don't want to translate the strings in the calls to dgt(). */
#ifdef PAM_TEXT_DOMAIN
# define dgt(d, t)	dgettext(d, t)
#endif

#include <sudoers.h>
#include "sudo_auth.h"

/* Only OpenPAM and Linux PAM use const qualifiers. */
#ifdef PAM_SUN_CODEBASE
# define PAM_CONST
#else
# define PAM_CONST	const
#endif

/* Ambiguity in spec: is it an array of pointers or a pointer to an array? */
#ifdef PAM_SUN_CODEBASE
# define PAM_MSG_GET(msg, n) (*(msg) + (n))
#else
# define PAM_MSG_GET(msg, n) ((msg)[(n)])
#endif

#ifndef PAM_DATA_SILENT
#define PAM_DATA_SILENT	0
#endif

struct sudo_pam_closure {
    const struct sudoers_context *ctx;
    struct sudo_conv_callback *callback;
};

struct conv_filter {
    char *msg;
    size_t msglen;
};

static int converse(int, PAM_CONST struct pam_message **,
		    struct pam_response **, void *);
static struct sudo_pam_closure pam_closure;
static struct pam_conv pam_conv = { converse, &pam_closure };
static const char *def_prompt = PASSPROMPT;
static bool getpass_error;
static bool noninteractive;
static pam_handle_t *pamh;
static struct conv_filter *conv_filter;

static void
conv_filter_init(const struct sudoers_context *ctx)
{
    debug_decl(conv_filter_init, SUDOERS_DEBUG_AUTH);

#ifdef __hpux
    /*
     * HP-UX displays last login information as part of either account
     * management (in trusted mode) or session management (regular mode).
     * Filter those out in the conversation function unless running a shell.
     */
    if (!ISSET(ctx->mode, MODE_SHELL|MODE_LOGIN_SHELL)) {
	int i, nfilt = 0, maxfilters = 0;
	struct conv_filter *newfilt;
	nl_catd catd;
	char *msg;

	/*
	 * Messages from PAM account management when trusted mode is enabled:
	 *  1 Last   successful login for %s: %s
	 *  2 Last   successful login for %s: %s on %s
	 *  3 Last unsuccessful login for %s: %s
	 *  4 Last unsuccessful login for %s: %s on %s
	 */
	if ((catd = catopen("pam_comsec", NL_CAT_LOCALE)) != -1) {
	    maxfilters += 4;
	    newfilt = reallocarray(conv_filter, maxfilters + 1,
		sizeof(*conv_filter));
	    if (newfilt != NULL) {
		conv_filter = newfilt;
		for (i = 1; i < 5; i++) {
		    if ((msg = catgets(catd, 1, i, NULL)) == NULL)
			break;
		    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
			"adding \"%s\" to conversation filter", msg);
		    if ((conv_filter[nfilt].msg = strdup(msg)) == NULL)
			break;
		    conv_filter[nfilt].msglen = strcspn(msg, "%");
		    nfilt++;
		}
	    }
	}
	/*
	 * Messages from PAM session management when trusted mode is disabled:
	 *  3 Last successful login:       %s %s %s %s
	 *  4 Last authentication failure: %s %s %s %s
	 */
	if ((catd = catopen("pam_hpsec", NL_CAT_LOCALE)) != -1) {
	    maxfilters += 2;
	    newfilt = reallocarray(conv_filter, maxfilters + 1,
		sizeof(*conv_filter));
	    if (newfilt != NULL) {
		conv_filter = newfilt;
		for (i = 3; i < 5; i++) {
		    if ((msg = catgets(catd, 1, i, NULL)) == NULL)
			break;
		    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
			"adding \"%s\" to conversation filter", msg);
		    if ((conv_filter[nfilt].msg = strdup(msg)) == NULL)
			break;
		    conv_filter[nfilt].msglen = strcspn(msg, "%");
		    nfilt++;
		}
	    }
	}
	if (conv_filter != NULL) {
	    conv_filter[nfilt].msg = NULL;
	    conv_filter[nfilt].msglen = 0;
	}
    }
#endif /* __hpux */
    debug_return;
}

/*
 * Like pam_strerror() but never returns NULL and uses strerror(errno)
 * for PAM_SYSTEM_ERR.
 */
static const char *
sudo_pam_strerror(pam_handle_t *handle, int errnum)
{
    const char *errstr;
    static char errbuf[32];

    if (errnum == PAM_SYSTEM_ERR)
	return strerror(errno);
    if ((errstr = pam_strerror(handle, errnum)) == NULL)
	(void)snprintf(errbuf, sizeof(errbuf), "PAM error %d", errnum);
    return errstr;
}

static int
sudo_pam_init2(const struct sudoers_context *ctx, struct passwd *pw,
    sudo_auth *auth, bool quiet)
{
    static int pam_status = PAM_SUCCESS;
    const char *ttypath = ctx->user.ttypath;
    const char *errstr, *pam_service;
    int rc;
    debug_decl(sudo_pam_init, SUDOERS_DEBUG_AUTH);

    /* Stash pointer to last pam status. */
    auth->data = &pam_status;

    if (pamh != NULL) {
	/* Already initialized (may happen with AIX or with sub-commands). */
	debug_return_int(AUTH_SUCCESS);
    }

    /* Stash value of noninteractive flag for conversation function. */
    noninteractive = IS_NONINTERACTIVE(auth);

    /* Store context in closure so converse() has access to it. */
    pam_closure.ctx = ctx;

    /* Initialize PAM. */
    if (ISSET(ctx->mode, MODE_ASKPASS) && def_pam_askpass_service != NULL) {
	pam_service = def_pam_askpass_service;
    } else {
	pam_service = ISSET(ctx->mode, MODE_LOGIN_SHELL) ?
	    def_pam_login_service : def_pam_service;
    }
    pam_status = pam_start(pam_service, pw->pw_name, &pam_conv, &pamh);
    if (pam_status != PAM_SUCCESS) {
	errstr = sudo_pam_strerror(NULL, pam_status);
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "pam_start(%s, %s, %p, %p): %s", pam_service, pw->pw_name,
	    &pam_conv, &pamh, errstr);
	if (!quiet)
	    log_warningx(ctx, 0, N_("unable to initialize PAM: %s"), errstr);
	debug_return_int(AUTH_ERROR);
    }

    /* Initialize conversation function message filter. */
    conv_filter_init(ctx);

    /*
     * Set PAM_RUSER to the invoking user (the "from" user).
     * Solaris 7 and below require PAM_RHOST to be set if PAM_RUSER is.
     * Note: PAM_RHOST may cause a DNS lookup on Linux in libaudit.
     */
    if (def_pam_ruser) {
	rc = pam_set_item(pamh, PAM_RUSER, ctx->user.name);
	if (rc != PAM_SUCCESS) {
	    errstr = sudo_pam_strerror(pamh, rc);
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"pam_set_item(pamh, PAM_RUSER, %s): %s", ctx->user.name, errstr);
	}
    }
    if (def_pam_rhost) {
	rc = pam_set_item(pamh, PAM_RHOST, ctx->user.host);
	if (rc != PAM_SUCCESS) {
	    errstr = sudo_pam_strerror(pamh, rc);
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"pam_set_item(pamh, PAM_RHOST, %s): %s", ctx->user.host, errstr);
	}
    }
    if (ttypath != NULL) {
	rc = pam_set_item(pamh, PAM_TTY, ttypath);
	if (rc != PAM_SUCCESS) {
	    errstr = sudo_pam_strerror(pamh, rc);
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"pam_set_item(pamh, PAM_TTY, %s): %s", ttypath, errstr);
	}
    }

    /*
     * If PAM session and setcred support is disabled we don't
     * need to keep a sudo process around to close the session.
     */
    if (!def_pam_session && !def_pam_setcred)
	auth->end_session = NULL;

    debug_return_int(AUTH_SUCCESS);
}

int
sudo_pam_init(const struct sudoers_context *ctx, struct passwd *pw,
    sudo_auth *auth)
{
    return sudo_pam_init2(ctx, pw, auth, false);
}

#ifdef _AIX
int
sudo_pam_init_quiet(const struct sudoers_context *ctx, struct passwd *pw,
    sudo_auth *auth)
{
    return sudo_pam_init2(ctx, pw, auth, true);
}
#endif /* _AIX */

int
sudo_pam_verify(const struct sudoers_context *ctx, struct passwd *pw,
    const char *prompt, sudo_auth *auth, struct sudo_conv_callback *callback)
{
    const char *envccname;
    const char *s;
    int *pam_status = (int *)auth->data;
    debug_decl(sudo_pam_verify, SUDOERS_DEBUG_AUTH);

    def_prompt = prompt;		/* for converse */
    getpass_error = false;		/* set by converse if user presses ^C */
    pam_closure.callback = callback;	/* passed to conversation function */

	/* Set KRB5CCNAME from the user environment if not set to propagate this
	 * information to PAM modules that may use it to authentication. */
	envccname = sudo_getenv("KRB5CCNAME");
	if (envccname == NULL && ctx->user.ccname != NULL) {
		if (sudo_setenv("KRB5CCNAME", ctx->user.ccname, true) != 0) {
			sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
			"unable to set KRB5CCNAME");
			debug_return_int(AUTH_FAILURE);
		}
	}

    /* PAM_SILENT prevents the authentication service from generating output. */
    *pam_status = pam_authenticate(pamh, PAM_SILENT);

    /* Restore def_prompt, the passed-in prompt may be freed later. */
    def_prompt = PASSPROMPT;

	/* Restore KRB5CCNAME to its original value. */
	if (envccname == NULL && sudo_unsetenv("KRB5CCNAME") != 0) {
		sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
		"unable to restore KRB5CCNAME");
		debug_return_int(AUTH_FAILURE);
	}

    if (getpass_error) {
	/* error or ^C from tgetpass() or running non-interactive */
	debug_return_int(noninteractive ? AUTH_NONINTERACTIVE : AUTH_INTR);
    }
    switch (*pam_status) {
	case PAM_SUCCESS:
	    debug_return_int(AUTH_SUCCESS);
	case PAM_AUTH_ERR:
	case PAM_AUTHINFO_UNAVAIL:
	case PAM_MAXTRIES:
	case PAM_PERM_DENIED:
	    sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
		"pam_authenticate: %d", *pam_status);
	    debug_return_int(AUTH_FAILURE);
	default:
	    s = sudo_pam_strerror(pamh, *pam_status);
	    log_warningx(ctx, 0, N_("PAM authentication error: %s"), s);
	    debug_return_int(AUTH_ERROR);
    }
}

int
sudo_pam_approval(const struct sudoers_context *ctx, struct passwd *pw,
    sudo_auth *auth, bool exempt)
{
    const char *s;
    int rc, status = AUTH_SUCCESS;
    int *pam_status = (int *) auth->data;
    debug_decl(sudo_pam_approval, SUDOERS_DEBUG_AUTH);

    if (def_pam_acct_mgmt) {
	rc = pam_acct_mgmt(pamh, PAM_SILENT);
	switch (rc) {
	    case PAM_SUCCESS:
		break;
	    case PAM_AUTH_ERR:
		log_warningx(ctx, 0, N_("account validation failure, "
		    "is your account locked?"));
		status = AUTH_ERROR;
		break;
	    case PAM_NEW_AUTHTOK_REQD:
		/* Ignore if user is exempt from password restrictions. */
		if (exempt) {
		    rc = *pam_status;
		    break;
		}
		/* New password required, try to change it. */
		log_warningx(ctx, 0, N_("Account or password is "
		    "expired, reset your password and try again"));
		rc = pam_chauthtok(pamh, PAM_CHANGE_EXPIRED_AUTHTOK);
		if (rc == PAM_SUCCESS)
		    break;
		s = pam_strerror(pamh, rc);
		log_warningx(ctx, 0,
		    N_("unable to change expired password: %s"), s);
		status = AUTH_FAILURE;
		break;
	    case PAM_AUTHTOK_EXPIRED:
		/* Ignore if user is exempt from password restrictions. */
		if (exempt) {
		    rc = *pam_status;
		    break;
		}
		/* Password expired, cannot be updated by user. */
		log_warningx(ctx, 0,
		    N_("Password expired, contact your system administrator"));
		status = AUTH_ERROR;
		break;
	    case PAM_ACCT_EXPIRED:
		log_warningx(ctx, 0,
		    N_("Account expired or PAM config lacks an \"account\" "
		    "section for sudo, contact your system administrator"));
		status = AUTH_ERROR;
		break;
	    case PAM_AUTHINFO_UNAVAIL:
	    case PAM_MAXTRIES:
	    case PAM_PERM_DENIED:
		s = sudo_pam_strerror(pamh, rc);
		log_warningx(ctx, 0, N_("PAM account management error: %s"), s);
		status = AUTH_FAILURE;
		break;
	    default:
		s = sudo_pam_strerror(pamh, rc);
		log_warningx(ctx, 0, N_("PAM account management error: %s"), s);
		status = AUTH_ERROR;
		break;
	}
	*pam_status = rc;
    }
    debug_return_int(status);
}

int
sudo_pam_cleanup(const struct sudoers_context *ctx, struct passwd *pw,
    sudo_auth *auth, bool force)
{
    int *pam_status = (int *) auth->data;
    debug_decl(sudo_pam_cleanup, SUDOERS_DEBUG_AUTH);

    /* If successful, we can't close the session until sudo_pam_end_session() */
    if (force || *pam_status != PAM_SUCCESS || auth->end_session == NULL) {
	*pam_status = pam_end(pamh, *pam_status | PAM_DATA_SILENT);
	pamh = NULL;
    }
    debug_return_int(*pam_status == PAM_SUCCESS ? AUTH_SUCCESS : AUTH_FAILURE);
}

int
sudo_pam_begin_session(const struct sudoers_context *ctx, struct passwd *pw,
    char **user_envp[], sudo_auth *auth)
{
    int rc, status = AUTH_SUCCESS;
    int *pam_status = (int *) auth->data;
    const char *errstr;
    debug_decl(sudo_pam_begin_session, SUDOERS_DEBUG_AUTH);

    /*
     * If there is no valid user we cannot open a PAM session.
     * This is not an error as sudo can run commands with arbitrary
     * uids, it just means we are done from a session management standpoint.
     */
    if (pw == NULL) {
	if (pamh != NULL) {
	    rc = pam_end(pamh, PAM_SUCCESS | PAM_DATA_SILENT);
	    if (rc != PAM_SUCCESS) {
		errstr = sudo_pam_strerror(pamh, rc);
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "pam_end: %s", errstr);
	    }
	    pamh = NULL;
	}
	goto done;
    }

    /*
     * Update PAM_USER to reference the user we are running the command
     * as, as opposed to the user we authenticated as.
     */
    rc = pam_set_item(pamh, PAM_USER, pw->pw_name);
    if (rc != PAM_SUCCESS) {
	errstr = sudo_pam_strerror(pamh, rc);
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "pam_set_item(pamh, PAM_USER, %s): %s", pw->pw_name, errstr);
    }

    /*
     * Reinitialize credentials when changing the user.
     * We don't worry about a failure from pam_setcred() since with
     * stacked PAM auth modules a failure from one module may override
     * PAM_SUCCESS from another.  For example, given a non-local user,
     * pam_unix will fail but pam_ldap or pam_sss may succeed, but if
     * pam_unix is first in the stack, pam_setcred() will fail.
     */
    if (def_pam_setcred) {
	rc = pam_setcred(pamh, PAM_REINITIALIZE_CRED);
	if (rc != PAM_SUCCESS) {
	    errstr = sudo_pam_strerror(pamh, rc);
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"pam_setcred: %s", errstr);
	    def_pam_setcred = false;
	}
    }

    if (def_pam_session) {
	/*
	 * We use PAM_SILENT to prevent pam_lastlog from printing last login
	 * information except when explicitly running a shell.
	 */
	const bool silent = !ISSET(ctx->mode, MODE_SHELL|MODE_LOGIN_SHELL);
	rc = pam_open_session(pamh, silent ? PAM_SILENT : 0);
	switch (rc) {
	case PAM_SUCCESS:
	    break;
	case PAM_SESSION_ERR:
	    /* Treat PAM_SESSION_ERR as a non-fatal error. */
	    errstr = sudo_pam_strerror(pamh, rc);
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"pam_open_session: %s", errstr);
	    /* Avoid closing session that was not opened. */
	    def_pam_session = false;
	    break;
	default:
	    /* Unexpected session failure, treat as fatal error. */
	    *pam_status = rc;
	    errstr = sudo_pam_strerror(pamh, rc);
	    log_warningx(ctx, 0, N_("%s: %s"), "pam_open_session", errstr);
	    rc = pam_end(pamh, *pam_status | PAM_DATA_SILENT);
	    if (rc != PAM_SUCCESS) {
		errstr = sudo_pam_strerror(pamh, rc);
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "pam_end: %s", errstr);
	    }
	    pamh = NULL;
	    status = AUTH_ERROR;
	    goto done;
	}
    }

#ifdef HAVE_PAM_GETENVLIST
    /*
     * Update environment based on what is stored in pamh.
     * If no authentication is done we will only have environment
     * variables if pam_env is called via session.
     */
    if (user_envp != NULL) {
	char **pam_envp = pam_getenvlist(pamh);
	if (pam_envp != NULL) {
	    /* Merge pam env with user env. */
	    if (!env_init(*user_envp) || !env_merge(ctx, pam_envp))
		status = AUTH_ERROR;
	    *user_envp = env_get();
	    free(pam_envp);
	    /* XXX - we leak any duplicates that were in pam_envp */
	}
    }
#endif /* HAVE_PAM_GETENVLIST */

done:
    debug_return_int(status);
}

int
sudo_pam_end_session(sudo_auth *auth)
{
    int rc, status = AUTH_SUCCESS;
    const char *errstr;
    debug_decl(sudo_pam_end_session, SUDOERS_DEBUG_AUTH);

    if (pamh != NULL) {
	if (def_pam_session) {
	    rc = pam_close_session(pamh, PAM_SILENT);
	    if (rc != PAM_SUCCESS) {
		errstr = sudo_pam_strerror(pamh, rc);
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "pam_close_session: %s", errstr);
	    }
	}
	if (def_pam_setcred) {
	    rc = pam_setcred(pamh, PAM_DELETE_CRED | PAM_SILENT);
	    if (rc != PAM_SUCCESS) {
		errstr = sudo_pam_strerror(pamh, rc);
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "pam_setcred: %s", errstr);
	    }
	}
	rc = pam_end(pamh, PAM_SUCCESS | PAM_DATA_SILENT);
	if (rc != PAM_SUCCESS) {
	    errstr = sudo_pam_strerror(pamh, rc);
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"pam_end: %s", errstr);
	    status = AUTH_ERROR;
	}
	pamh = NULL;
    }

    debug_return_int(status);
}

#define PROMPT_IS_PASSWORD(_p) \
    (strncmp((_p), "Password:", 9) == 0 && \
	((_p)[9] == '\0' || ((_p)[9] == ' ' && (_p)[10] == '\0')))

#ifdef PAM_TEXT_DOMAIN
# define PAM_PROMPT_IS_PASSWORD(_p) \
    (strcmp((_p), dgt(PAM_TEXT_DOMAIN, "Password:")) == 0 || \
	strcmp((_p), dgt(PAM_TEXT_DOMAIN, "Password: ")) == 0 || \
	PROMPT_IS_PASSWORD(_p))
#else
# define PAM_PROMPT_IS_PASSWORD(_p)	PROMPT_IS_PASSWORD(_p)
#endif /* PAM_TEXT_DOMAIN */

/*
 * We use the PAM prompt in preference to sudo's as long
 * as passprompt_override is not set and:
 *  a) the (translated) sudo prompt matches /^Password: ?/
 * or:
 *  b) the PAM prompt itself *doesn't* match /^Password: ?/
 *     or /^username's Password: ?/
 *
 * The intent is to use the PAM prompt for things like
 * challenge-response, otherwise use sudo's prompt.
 * There may also be cases where a localized translation
 * of "Password: " exists for PAM but not for sudo.
 */
static bool
use_pam_prompt(const char *pam_prompt)
{
    size_t user_len;
    debug_decl(use_pam_prompt, SUDOERS_DEBUG_AUTH);

    /* Always use sudo prompt if passprompt_override is set. */
    if (def_passprompt_override)
	debug_return_bool(false);

    /* If sudo prompt matches "^Password: ?$", use PAM prompt. */
    if (PROMPT_IS_PASSWORD(def_prompt))
	debug_return_bool(true);

    /* If PAM prompt matches "^Password: ?$", use sudo prompt. */
    if (PAM_PROMPT_IS_PASSWORD(pam_prompt))
	debug_return_bool(false);

    /*
     * Some PAM modules use "^username's Password: ?$" instead of
     * "^Password: ?" so check for that too.
     */
    if (pam_closure.ctx != NULL) {
	const char *user_name = pam_closure.ctx->user.name;
	user_len = strlen(user_name);
	if (strncmp(pam_prompt, user_name, user_len) == 0) {
	    const char *cp = pam_prompt + user_len;
	    if (strncmp(cp, "'s Password:", 12) == 0 &&
		(cp[12] == '\0' || (cp[12] == ' ' && cp[13] == '\0')))
		debug_return_bool(false);
	}
    }

    /* Otherwise, use the PAM prompt. */
    debug_return_bool(true);
}

static bool
is_filtered(const char *msg)
{
    bool filtered = false;

    if (conv_filter != NULL) {
	struct conv_filter *filt = conv_filter;
	while (filt->msg != NULL) {
	    if (strncmp(msg, filt->msg, filt->msglen) == 0) {
		filtered = true;
		break;
	    }
	    filt++;
	}
    }
    return filtered;
}

/*
 * ``Conversation function'' for PAM <-> human interaction.
 */
static int
converse(int num_msg, PAM_CONST struct pam_message **msg,
    struct pam_response **reply_out, void *appdata_ptr)
{
    struct sudo_conv_callback *callback = NULL;
    struct sudo_pam_closure *closure = appdata_ptr;
    struct pam_response *reply;
    const char *prompt;
    char *pass;
    int n, type;
    debug_decl(converse, SUDOERS_DEBUG_AUTH);

    if (num_msg <= 0 || num_msg > PAM_MAX_NUM_MSG) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "invalid number of PAM messages: %d", num_msg);
	debug_return_int(PAM_CONV_ERR);
    }
    sudo_debug_printf(SUDO_DEBUG_DEBUG|SUDO_DEBUG_LINENO,
	"number of PAM messages: %d", num_msg);

    reply = calloc((size_t)num_msg, sizeof(struct pam_response));
    if (reply == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_int(PAM_BUF_ERR);
    }

    if (closure != NULL)
	callback = closure->callback;

    for (n = 0; n < num_msg; n++) {
	PAM_CONST struct pam_message *pm = PAM_MSG_GET(msg, n);

	type = SUDO_CONV_PROMPT_ECHO_OFF;
	switch (pm->msg_style) {
	    case PAM_PROMPT_ECHO_ON:
		type = SUDO_CONV_PROMPT_ECHO_ON;
		FALLTHROUGH;
	    case PAM_PROMPT_ECHO_OFF:
		/* Error out if the last password read was interrupted. */
		if (getpass_error)
		    goto bad;

		/* Treat non-interactive mode as a getpass error. */
		if (noninteractive) {
		    getpass_error = true;
		    goto bad;
		}

		/* Choose either the sudo prompt or the PAM one. */
		prompt = use_pam_prompt(pm->msg) ? pm->msg : def_prompt;

		/* Read the password unless interrupted. */
		pass = auth_getpass(prompt, type, callback);
		if (pass == NULL) {
		    /* Error (or ^C) reading password, don't try again. */
		    getpass_error = true;
		    goto bad;
		}
		if (strlen(pass) >= PAM_MAX_RESP_SIZE) {
		    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
			"password longer than %d", PAM_MAX_RESP_SIZE);
		    freezero(pass, strlen(pass));
		    pass = NULL;
		    goto bad;
		}
		reply[n].resp = pass;	/* auth_getpass() malloc's a copy */
		break;
	    case PAM_TEXT_INFO:
		if (pm->msg != NULL && !is_filtered(pm->msg))
		    sudo_printf(SUDO_CONV_INFO_MSG|SUDO_CONV_PREFER_TTY,
			"%s\n", pm->msg);
		break;
	    case PAM_ERROR_MSG:
		if (pm->msg != NULL)
		    sudo_printf(SUDO_CONV_ERROR_MSG|SUDO_CONV_PREFER_TTY,
			"%s\n", pm->msg);
		break;
	    default:
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "unsupported message style: %d", pm->msg_style);
		goto bad;
	}
    }

    *reply_out = reply;
    debug_return_int(PAM_SUCCESS);

bad:
    /* Zero and free allocated memory and return an error. */
    for (n = 0; n < num_msg; n++) {
	struct pam_response *pr = &reply[n];

	if (pr->resp != NULL) {
	    freezero(pr->resp, strlen(pr->resp));
	    pr->resp = NULL;
	}
    }
    free(reply);
    debug_return_int(PAM_CONV_ERR);
}

#endif /* HAVE_PAM */
