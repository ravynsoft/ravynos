/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1999-2005, 2008-2020 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <time.h>
#include <signal.h>

#include <sudoers.h>
#include "sudo_auth.h"
#include <insults.h>
#include <timestamp.h>

static sudo_auth auth_switch[] = {
/* Standalone entries first */
#ifdef HAVE_AIXAUTH
    AUTH_ENTRY("aixauth", FLAG_STANDALONE, sudo_aix_init, NULL, sudo_aix_verify, NULL, sudo_aix_cleanup, NULL, NULL)
#endif
#ifdef HAVE_PAM
    AUTH_ENTRY("pam", FLAG_STANDALONE, sudo_pam_init, NULL, sudo_pam_verify, sudo_pam_approval, sudo_pam_cleanup, sudo_pam_begin_session, sudo_pam_end_session)
#endif
#ifdef HAVE_SECURID
    AUTH_ENTRY("SecurId", FLAG_STANDALONE, sudo_securid_init, sudo_securid_setup, sudo_securid_verify, NULL, NULL, NULL, NULL)
#endif
#ifdef HAVE_SIA_SES_INIT
    AUTH_ENTRY("sia", FLAG_STANDALONE, NULL, sudo_sia_setup, sudo_sia_verify, NULL, sudo_sia_cleanup, sudo_sia_begin_session, NULL)
#endif
#ifdef HAVE_FWTK
    AUTH_ENTRY("fwtk", FLAG_STANDALONE, sudo_fwtk_init, NULL, sudo_fwtk_verify, NULL, sudo_fwtk_cleanup, NULL, NULL)
#endif
#ifdef HAVE_BSD_AUTH_H
    AUTH_ENTRY("bsdauth", FLAG_STANDALONE, bsdauth_init, NULL, bsdauth_verify, bsdauth_approval, bsdauth_cleanup, NULL, NULL)
#endif

/* Non-standalone entries */
#ifndef WITHOUT_PASSWD
    AUTH_ENTRY("passwd", 0, sudo_passwd_init, NULL, sudo_passwd_verify, NULL, sudo_passwd_cleanup, NULL, NULL)
#endif
#if defined(HAVE_GETPRPWNAM) && !defined(WITHOUT_PASSWD)
    AUTH_ENTRY("secureware", 0, sudo_secureware_init, NULL, sudo_secureware_verify, NULL, sudo_secureware_cleanup, NULL, NULL)
#endif
#ifdef HAVE_AFS
    AUTH_ENTRY("afs", 0, NULL, NULL, sudo_afs_verify, NULL, NULL, NULL, NULL)
#endif
#ifdef HAVE_DCE
    AUTH_ENTRY("dce", 0, NULL, NULL, sudo_dce_verify, NULL, NULL, NULL, NULL)
#endif
#ifdef HAVE_KERB5
    AUTH_ENTRY("kerb5", 0, sudo_krb5_init, sudo_krb5_setup, sudo_krb5_verify, NULL, sudo_krb5_cleanup, NULL, NULL)
#endif
#ifdef HAVE_SKEY
    AUTH_ENTRY("S/Key", 0, NULL, sudo_rfc1938_setup, sudo_rfc1938_verify, NULL, NULL, NULL, NULL)
#endif
#ifdef HAVE_OPIE
    AUTH_ENTRY("OPIE", 0, NULL, sudo_rfc1938_setup, sudo_rfc1938_verify, NULL, NULL, NULL, NULL)
#endif
    AUTH_ENTRY(NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL)
};

static bool standalone;

/*
 * Initialize sudoers authentication method(s).
 * Returns AUTH_SUCCESS on success and AUTH_ERROR on error.
 */
int
sudo_auth_init(const struct sudoers_context *ctx, struct passwd *pw,
    unsigned int mode)
{
    sudo_auth *auth;
    debug_decl(sudo_auth_init, SUDOERS_DEBUG_AUTH);

    if (auth_switch[0].name == NULL)
	debug_return_int(AUTH_SUCCESS);

    /* Initialize auth methods and unconfigure the method if necessary. */
    for (auth = auth_switch; auth->name; auth++) {
	if (ISSET(mode, MODE_NONINTERACTIVE))
	    SET(auth->flags, FLAG_NONINTERACTIVE);
	if (auth->init && !IS_DISABLED(auth)) {
	    /* Disable if it failed to init unless there was a fatal error. */
	    switch ((auth->init)(ctx, pw, auth)) {
	    case AUTH_SUCCESS:
		break;
	    case AUTH_FAILURE:
		SET(auth->flags, FLAG_DISABLED);
		break;
	    default:
		/* Assume error msg already printed. */
		debug_return_int(AUTH_ERROR);
	    }
	}
    }

    /*
     * Make sure we haven't mixed standalone and shared auth methods.
     * If there are multiple standalone methods, only use the first one.
     */
    if ((standalone = IS_STANDALONE(&auth_switch[0]))) {
	bool found = false;
	for (auth = auth_switch; auth->name; auth++) {
	    if (IS_DISABLED(auth))
		continue;
	    if (!IS_STANDALONE(auth)) {
		audit_failure(ctx, ctx->runas.argv,
		    N_("invalid authentication methods"));
		log_warningx(ctx, SLOG_SEND_MAIL,
		    N_("Invalid authentication methods compiled into sudo!  "
		    "You may not mix standalone and non-standalone authentication."));
		debug_return_int(AUTH_ERROR);
	    }
	    if (!found) {
		/* Found first standalone method. */
		found = true;
		continue;
	    }
	    /* Disable other standalone methods. */
	    SET(auth->flags, FLAG_DISABLED);
	}
    }

    /* Set FLAG_ONEANDONLY if there is only one auth method. */
    for (auth = auth_switch; auth->name; auth++) {
	/* Find first enabled auth method. */
	if (!IS_DISABLED(auth)) {
	    sudo_auth *first = auth;
	    /* Check for others. */
	    for (; auth->name; auth++) {
		if (!IS_DISABLED(auth))
		    break;
	    }
	    if (auth->name == NULL)
		SET(first->flags, FLAG_ONEANDONLY);
	    break;
	}
    }

    debug_return_int(AUTH_SUCCESS);
}

/*
 * Call all authentication approval methods, if any.
 * Returns AUTH_SUCCESS, AUTH_FAILURE or AUTH_ERROR.
 */
int
sudo_auth_approval(const struct sudoers_context *ctx, struct passwd *pw,
    unsigned int validated, bool exempt)
{
    int ret = AUTH_SUCCESS;
    sudo_auth *auth;
    debug_decl(sudo_auth_approval, SUDOERS_DEBUG_AUTH);

    /* Call approval routines. */
    for (auth = auth_switch; auth->name; auth++) {
	if (auth->approval && !IS_DISABLED(auth)) {
	    ret = (auth->approval)(ctx, pw, auth, exempt);
	    if (ret != AUTH_SUCCESS) {
		/* Assume error msg already printed. */
		log_auth_failure(ctx, validated, 0);
		break;
	    }
	}
    }
    debug_return_int(ret);
}

/*
 * Cleanup all authentication methods.
 * Returns AUTH_SUCCESS on success and AUTH_ERROR on error.
 */
int
sudo_auth_cleanup(const struct sudoers_context *ctx, struct passwd *pw,
    bool force)
{
    sudo_auth *auth;
    debug_decl(sudo_auth_cleanup, SUDOERS_DEBUG_AUTH);

    /* Call cleanup routines. */
    for (auth = auth_switch; auth->name; auth++) {
	if (auth->cleanup && !IS_DISABLED(auth)) {
	    int status = (auth->cleanup)(ctx, pw, auth, force);
	    if (status != AUTH_SUCCESS) {
		/* Assume error msg already printed. */
		debug_return_int(AUTH_ERROR);
	    }
	}
    }
    debug_return_int(AUTH_SUCCESS);
}

static void
pass_warn(void)
{
    const char *warning = def_badpass_message;
    debug_decl(pass_warn, SUDOERS_DEBUG_AUTH);

#ifdef INSULT
    if (def_insults)
	warning = INSULT;
#endif
    sudo_printf(SUDO_CONV_ERROR_MSG|SUDO_CONV_PREFER_TTY, "%s\n", warning);

    debug_return;
}

static bool
user_interrupted(void)
{
    sigset_t mask;

    return (sigpending(&mask) == 0 &&
	(sigismember(&mask, SIGINT) || sigismember(&mask, SIGQUIT)));
}

/*
 * Called when getpass is suspended so we can drop the lock.
 */
static int
getpass_suspend(int signo, void *vclosure)
{
    struct getpass_closure *closure = vclosure;

    timestamp_close(closure->cookie);
    closure->cookie = NULL;
    return 0;
}

/*
 * Called when getpass is resumed so we can reacquire the lock.
 */
static int
getpass_resume(int signo, void *vclosure)
{
    struct getpass_closure *closure = vclosure;

    closure->cookie = timestamp_open(closure->ctx);
    if (closure->cookie == NULL)
	return -1;
    if (!timestamp_lock(closure->cookie, closure->auth_pw))
	return -1;
    return 0;
}

/*
 * Verify the specified user.
 * Returns AUTH_SUCCESS, AUTH_FAILURE or AUTH_ERROR.
 */
int
verify_user(const struct sudoers_context *ctx, struct passwd *pw, char *prompt,
    unsigned int validated, struct sudo_conv_callback *callback)
{
    struct sigaction sa, saved_sigtstp;
    int ret = AUTH_FAILURE;
    unsigned int ntries;
    sigset_t mask, omask;
    sudo_auth *auth;
    debug_decl(verify_user, SUDOERS_DEBUG_AUTH);

    /* Make sure we have at least one auth method. */
    if (auth_switch[0].name == NULL) {
	audit_failure(ctx, ctx->runas.argv, N_("no authentication methods"));
    	log_warningx(ctx, SLOG_SEND_MAIL,
	    N_("There are no authentication methods compiled into sudo!  "
	    "If you want to turn off authentication, use the "
	    "--disable-authentication configure option."));
	debug_return_int(AUTH_ERROR);
    }

    /* Enable suspend during password entry. */
    callback->on_suspend = getpass_suspend;
    callback->on_resume = getpass_resume;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = SIG_DFL;
    (void) sigaction(SIGTSTP, &sa, &saved_sigtstp);

    /*
     * We treat authentication as a critical section and block
     * keyboard-generated signals such as SIGINT and SIGQUIT
     * which might otherwise interrupt a sleep(3).
     * They are temporarily unblocked by auth_getpass().
     */
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);
    (void) sigprocmask(SIG_BLOCK, &mask, &omask);

    for (ntries = 0; ntries < def_passwd_tries; ntries++) {
	int num_methods = 0;
	char *pass = NULL;

	/* If user attempted to interrupt password verify, quit now. */
	if (user_interrupted())
	    goto done;

	if (ntries != 0)
	    pass_warn();

	/* Do any per-method setup and unconfigure the method if needed */
	for (auth = auth_switch; auth->name; auth++) {
	    if (IS_DISABLED(auth))
		continue;
	    num_methods++;
	    if (auth->setup != NULL) {
		switch ((auth->setup)(ctx, pw, &prompt, auth)) {
		case AUTH_SUCCESS:
		    if (user_interrupted())
			goto done;	/* assume error msg already printed */
		    break;
		case AUTH_FAILURE:
		    SET(auth->flags, FLAG_DISABLED);
		    break;
		case AUTH_NONINTERACTIVE:
		    /* Non-interactive mode, cannot prompt user. */
		    goto done;
		default:
		    ret = AUTH_ERROR;
		    goto done;
		}
	    }
	}
	if (num_methods == 0) {
	    audit_failure(ctx, ctx->runas.argv,
		N_("no authentication methods"));
	    log_warningx(ctx, SLOG_SEND_MAIL,
		N_("Unable to initialize authentication methods."));
	    debug_return_int(AUTH_ERROR);
	}

	/* Get the password unless the auth function will do it for us */
	if (!standalone) {
	    if (IS_NONINTERACTIVE(&auth_switch[0])) {
		ret = AUTH_NONINTERACTIVE;
		goto done;
	    }
	    pass = auth_getpass(prompt, SUDO_CONV_PROMPT_ECHO_OFF, callback);
	    if (pass == NULL)
		break;
	}

	/* Call authentication functions. */
	for (auth = auth_switch; auth->name; auth++) {
	    if (IS_DISABLED(auth))
		continue;

	    ret = auth->status = (auth->verify)(ctx, pw,
		standalone ? prompt : pass, auth, callback);
	    if (ret != AUTH_FAILURE)
		break;
	}
	if (pass != NULL)
	    freezero(pass, strlen(pass));

	if (ret != AUTH_FAILURE)
	    goto done;
    }

done:
    /* Restore signal handlers and signal mask. */
    (void) sigaction(SIGTSTP, &saved_sigtstp, NULL);
    (void) sigprocmask(SIG_SETMASK, &omask, NULL);

    switch (ret) {
	case AUTH_SUCCESS:
	    break;
	case AUTH_INTR:
	    ret = AUTH_FAILURE;
	    FALLTHROUGH;
	case AUTH_FAILURE:
	    if (ntries != 0)
		SET(validated, FLAG_BAD_PASSWORD);
	    log_auth_failure(ctx, validated, ntries);
	    break;
	case AUTH_NONINTERACTIVE:
	    SET(validated, FLAG_NO_USER_INPUT);
	    FALLTHROUGH;
	default:
	    log_auth_failure(ctx, validated, 0);
	    ret = AUTH_ERROR;
	    break;
    }

    debug_return_int(ret);
}

/*
 * Call authentication method begin session hooks.
 * Returns true on success, false on failure and -1 on error.
 */
int
sudo_auth_begin_session(const struct sudoers_context *ctx, struct passwd *pw,
    char **user_env[])
{
    sudo_auth *auth;
    int ret = true;
    debug_decl(sudo_auth_begin_session, SUDOERS_DEBUG_AUTH);

    for (auth = auth_switch; auth->name; auth++) {
	if (auth->begin_session && !IS_DISABLED(auth)) {
	    int status = (auth->begin_session)(ctx, pw, user_env, auth);
	    switch (status) {
	    case AUTH_SUCCESS:
		break;
	    case AUTH_FAILURE:
		ret = false;
		break;
	    default:
		/* Assume error msg already printed. */
		ret = -1;
		break;
	    }
	}
    }
    debug_return_int(ret);
}

bool
sudo_auth_needs_end_session(void)
{
    sudo_auth *auth;
    bool needed = false;
    debug_decl(sudo_auth_needs_end_session, SUDOERS_DEBUG_AUTH);

    for (auth = auth_switch; auth->name; auth++) {
	if (auth->end_session && !IS_DISABLED(auth)) {
	    needed = true;
	    break;
	}
    }
    debug_return_bool(needed);
}

/*
 * Call authentication method end session hooks.
 * Returns true on success, false on failure and -1 on error.
 */
int
sudo_auth_end_session(void)
{
    sudo_auth *auth;
    int ret = true;
    int status;
    debug_decl(sudo_auth_end_session, SUDOERS_DEBUG_AUTH);

    for (auth = auth_switch; auth->name; auth++) {
	if (auth->end_session && !IS_DISABLED(auth)) {
	    status = (auth->end_session)(auth);
	    switch (status) {
	    case AUTH_SUCCESS:
		break;
	    case AUTH_FAILURE:
		ret = false;
		break;
	    default:
		/* Assume error msg already printed. */
		ret = -1;
		break;
	    }
	}
    }
    debug_return_int(ret);
}

/*
 * Prompts the user for a password using the conversation function.
 * Returns the plaintext password or NULL.
 * The user is responsible for freeing the returned value.
 */
char *
auth_getpass(const char *prompt, int type, struct sudo_conv_callback *callback)
{
    struct sudo_conv_message msg;
    struct sudo_conv_reply repl;
    sigset_t mask, omask;
    debug_decl(auth_getpass, SUDOERS_DEBUG_AUTH);

    /* Display lecture if needed and we haven't already done so. */
    display_lecture(callback);

    /* Mask user input if pwfeedback set and echo is off. */
    if (type == SUDO_CONV_PROMPT_ECHO_OFF && def_pwfeedback)
	type = SUDO_CONV_PROMPT_MASK;

    /* If visiblepw set, do not error out if there is no tty. */
    if (def_visiblepw)
	type |= SUDO_CONV_PROMPT_ECHO_OK;

    /* Unblock SIGINT and SIGQUIT during password entry. */
    /* XXX - do in tgetpass() itself instead? */
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);
    (void) sigprocmask(SIG_UNBLOCK, &mask, &omask);

    /* Call conversation function. */
    memset(&msg, 0, sizeof(msg));
    msg.msg_type = type;
    msg.timeout = (int)def_passwd_timeout.tv_sec;
    msg.msg = prompt;
    memset(&repl, 0, sizeof(repl));
    sudo_conv(1, &msg, &repl, callback);
    /* XXX - check for ENOTTY? */

    /* Restore previous signal mask. */
    (void) sigprocmask(SIG_SETMASK, &omask, NULL);

    debug_return_str_masked(repl.reply);
}

void
dump_auth_methods(void)
{
    sudo_auth *auth;
    debug_decl(dump_auth_methods, SUDOERS_DEBUG_AUTH);

    sudo_printf(SUDO_CONV_INFO_MSG, _("Authentication methods:"));
    for (auth = auth_switch; auth->name; auth++)
	sudo_printf(SUDO_CONV_INFO_MSG, " '%s'", auth->name);
    sudo_printf(SUDO_CONV_INFO_MSG, "\n");

    debug_return;
}
