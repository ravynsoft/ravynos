/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1999-2005, 2007-2018 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifdef HAVE_AIXAUTH

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <usersec.h>

#include <sudoers.h>
#include "sudo_auth.h"

/*
 * For a description of the AIX authentication API, see
 * http://publib16.boulder.ibm.com/doc_link/en_US/a_doc_lib/libs/basetrf1/authenticate.htm
 */

#ifdef HAVE_PAM
# define AIX_AUTH_UNKNOWN	0
# define AIX_AUTH_STD		1
# define AIX_AUTH_PAM		2

static int
sudo_aix_authtype(void)
{
    size_t linesize = 0;
    ssize_t len;
    char *cp, *line = NULL;
    bool in_stanza = false;
    int authtype = AIX_AUTH_UNKNOWN;
    FILE *fp;
    debug_decl(sudo_aix_authtype, SUDOERS_DEBUG_AUTH);

    if ((fp = fopen("/etc/security/login.cfg", "r")) == NULL)
	debug_return_int(AIX_AUTH_UNKNOWN);

    while ((len = getdelim(&line, &linesize, '\n', fp)) != -1) {
	/* First remove comments. */
	if ((cp = strchr(line, '#')) != NULL) {
	    *cp = '\0';
	    len = (ssize_t)(cp - line);
	}

	/* Next remove trailing newlines and whitespace. */
	while (len > 0 && isspace((unsigned char)line[len - 1]))
	    line[--len] = '\0';

	/* Skip blank lines. */
	if (len == 0)
	    continue;

	/* Match start of the usw stanza. */
	if (!in_stanza) {
	    if (strncmp(line, "usw:", 4) == 0)
		in_stanza = true;
	    continue;
	}

	/* Check for end of the usw stanza. */
	if (!isblank((unsigned char)line[0])) {
	    in_stanza = false;
	    break;
	}

	/* Skip leading blanks. */
	cp = line;
	do {
	    cp++;
	} while (isblank((unsigned char)*cp));

	/* Match "auth_type = (PAM_AUTH|STD_AUTH)". */
	if (strncmp(cp, "auth_type", 9) != 0)
	    continue;
	cp += 9;
	while (isblank((unsigned char)*cp))
	    cp++;
	if (*cp++ != '=')
	    continue;
	while (isblank((unsigned char)*cp))
	    cp++;
	if (strcmp(cp, "PAM_AUTH") == 0) {
	    authtype = AIX_AUTH_PAM;
	    break;
	}
	if (strcmp(cp, "STD_AUTH") == 0) {
	    authtype = AIX_AUTH_STD;
	    break;
	}
    }
    free(line);
    fclose(fp);

    debug_return_int(authtype);
}
#endif /* HAVE_PAM */

int
sudo_aix_init(const struct sudoers_context *ctx, struct passwd *pw,
    sudo_auth *auth)
{
    debug_decl(sudo_aix_init, SUDOERS_DEBUG_AUTH);

#ifdef HAVE_PAM
    /* Check auth_type in /etc/security/login.cfg. */
    if (sudo_aix_authtype() == AIX_AUTH_PAM) {
	if (sudo_pam_init_quiet(ctx, pw, auth) == AUTH_SUCCESS) {
	    /* Fail AIX authentication so we can use PAM instead. */
	    debug_return_int(AUTH_FAILURE);
	}
    }
#endif
    debug_return_int(AUTH_SUCCESS);
}

/* Ignore AIX password incorrect message */
static bool
sudo_aix_valid_message(const char *message)
{
    const char *cp;
    const char badpass_msgid[] = "3004-300";
    debug_decl(sudo_aix_valid_message, SUDOERS_DEBUG_AUTH);

    if (message == NULL || message[0] == '\0')
	debug_return_bool(false);

    /* Match "3004-300: You entered an invalid login name or password" */
    for (cp = message; *cp != '\0'; cp++) {
	if (isdigit((unsigned char)*cp)) {
	    if (strncmp(cp, badpass_msgid, strlen(badpass_msgid)) == 0)
		debug_return_bool(false);
	    break;
	}
    }
    debug_return_bool(true);
}

/* 
 * Change the user's password.  If root changes the user's password
 * the ADMCHG flag is set on the account (and the user must change
 * it again) so we run passwd(1) as the user.  This does mean that
 * the user will need to re-enter their original password again,
 * unlike with su(1).  We may consider using pwdadm(1) as root to
 * change the password and then clear the flag in the future.
 */
static bool
sudo_aix_change_password(const struct sudoers_context *ctx, const char *user)
{
    struct sigaction sa, savechld;
    pid_t child, pid;
    bool ret = false;
    sigset_t mask;
    int status;
    debug_decl(sudo_aix_change_password, SUDOERS_DEBUG_AUTH);

    /* Set SIGCHLD handler to default since we call waitpid() below. */
    memset(&sa, 0, sizeof(sa));                
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;     
    sa.sa_handler = SIG_DFL;
    (void) sigaction(SIGCHLD, &sa, &savechld);

    switch (child = sudo_debug_fork()) {
    case -1:
	/* error */
	sudo_warn("%s", U_("unable to fork"));
	break;
    case 0:
	/* child, run passwd(1) */
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGQUIT);
	(void) sigprocmask(SIG_UNBLOCK, &mask, NULL);
	set_perms(ctx, PERM_USER);
	execl("/usr/bin/passwd", "passwd", user, (char *)NULL);
	sudo_warn("passwd");
	_exit(127);
	/* NOTREACHED */
    default:
	/* parent */
	break;
    }

    /* Wait for passwd(1) to complete. */
    do {
	pid = waitpid(child, &status, 0);
    } while (pid == -1 && errno == EINTR);
    sudo_debug_printf(SUDO_DEBUG_INFO|SUDO_DEBUG_LINENO,
	"child (%d) exit value %d", (int)child, status);
    if (pid != -1 && WIFEXITED(status) && WEXITSTATUS(status) == 0)
	ret = true;

    /* Restore saved SIGCHLD handler. */
    (void) sigaction(SIGCHLD, &savechld, NULL);

    debug_return_bool(ret);
}

int
sudo_aix_verify(const struct sudoers_context *ctx, struct passwd *pw,
    const char *prompt, sudo_auth *auth, struct sudo_conv_callback *callback)
{
    char *pass, *message = NULL;
    int result = 1, reenter = 0;
    int ret = AUTH_SUCCESS;
    debug_decl(sudo_aix_verify, SUDOERS_DEBUG_AUTH);

    if (IS_NONINTERACTIVE(auth))
        debug_return_int(AUTH_NONINTERACTIVE);

    do {
	pass = auth_getpass(prompt, SUDO_CONV_PROMPT_ECHO_OFF, callback);
	if (pass == NULL)
	    break;
	free(message);
	message = NULL;
	result = authenticate(pw->pw_name, pass, &reenter, &message);
	freezero(pass, strlen(pass));
	prompt = message;
    } while (reenter);

    if (result != 0) {
	/* Display error message, if any. */
	if (sudo_aix_valid_message(message))
	    sudo_printf(SUDO_CONV_ERROR_MSG|SUDO_CONV_PREFER_TTY,
		"%s", message);
	ret = pass ? AUTH_FAILURE : AUTH_INTR;
    }
    free(message);
    message = NULL;

    /* Check if password expired and allow user to change it if possible. */
    if (ret == AUTH_SUCCESS) {
	result = passwdexpired(pw->pw_name, &message);
	if (message != NULL && message[0] != '\0') {
	    int msg_type = SUDO_CONV_PREFER_TTY;
	    msg_type |= result ? SUDO_CONV_ERROR_MSG : SUDO_CONV_INFO_MSG,
	    sudo_printf(msg_type, "%s", message);
	    free(message);
	    message = NULL;
	}
	switch (result) {
	case 0:
	    /* password not expired. */
	    break;
	case 1:
	    /* password expired, user must change it */
	    if (!sudo_aix_change_password(ctx, pw->pw_name)) {
		sudo_warnx(U_("unable to change password for %s"), pw->pw_name);
		ret = AUTH_ERROR;
	    }
	    break;
	case 2:
	    /* password expired, only admin can change it */
	    ret = AUTH_ERROR;
	    break;
	default:
	    /* error (-1) */
	    sudo_warn("passwdexpired");
	    ret = AUTH_ERROR;
	    break;
	}
    }

    debug_return_int(ret);
}

int
sudo_aix_cleanup(const struct sudoers_context *ctx, struct passwd *pw,
    sudo_auth *auth, bool force)
{
    debug_decl(sudo_aix_cleanup, SUDOERS_DEBUG_AUTH);

    /* Unset AUTHSTATE as it may not be correct for the runas user. */
    if (sudo_unsetenv("AUTHSTATE") == -1)
	debug_return_int(AUTH_FAILURE);

    debug_return_int(AUTH_SUCCESS);
}

#endif /* HAVE_AIXAUTH */
