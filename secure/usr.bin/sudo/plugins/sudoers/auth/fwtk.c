/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1999-2005, 2008, 2010-2015
 *	Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifdef HAVE_FWTK

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>

#include <auth.h>
#include <firewall.h>

#include <sudoers.h>
#include "sudo_auth.h"

int
sudo_fwtk_init(const struct sudoers_context *ctx, struct passwd *pw,
    sudo_auth *auth)
{
    static Cfg *confp;			/* Configuration entry struct */
    char resp[128];			/* Response from the server */
    debug_decl(sudo_fwtk_init, SUDOERS_DEBUG_AUTH);

    /* Only initialize once. */
    if (auth->data != NULL)
	debug_return_int(AUTH_SUCCESS);

    if (IS_NONINTERACTIVE(auth))
        debug_return_int(AUTH_NONINTERACTIVE);

    if ((confp = cfg_read("sudo")) == (Cfg *)-1) {
	sudo_warnx("%s", U_("unable to read fwtk config"));
	debug_return_int(AUTH_ERROR);
    }

    if (auth_open(confp)) {
	sudo_warnx("%s", U_("unable to connect to authentication server"));
	debug_return_int(AUTH_ERROR);
    }

    /* Get welcome message from auth server */
    if (auth_recv(resp, sizeof(resp))) {
	sudo_warnx("%s", U_("lost connection to authentication server"));
	debug_return_int(AUTH_ERROR);
    }
    if (strncmp(resp, "Authsrv ready", 13) != 0) {
	sudo_warnx(U_("authentication server error:\n%s"), resp);
	debug_return_int(AUTH_ERROR);
    }
    auth->data = (void *) confp;

    debug_return_int(AUTH_SUCCESS);
}

int
sudo_fwtk_verify(const struct sudoers_context *ctx, struct passwd *pw,
    const char *prompt, sudo_auth *auth, struct sudo_conv_callback *callback)
{
    char *pass;				/* Password from the user */
    char buf[SUDO_CONV_REPL_MAX + 12];	/* General prupose buffer */
    char resp[128];			/* Response from the server */
    int error;
    debug_decl(sudo_fwtk_verify, SUDOERS_DEBUG_AUTH);

    /* Send username to authentication server. */
    (void) snprintf(buf, sizeof(buf), "authorize %s 'sudo'", pw->pw_name);
restart:
    if (auth_send(buf) || auth_recv(resp, sizeof(resp))) {
	sudo_warnx("%s", U_("lost connection to authentication server"));
	debug_return_int(AUTH_ERROR);
    }

    /* Get the password/response from the user. */
    if (strncmp(resp, "challenge ", 10) == 0) {
	(void) snprintf(buf, sizeof(buf), "%s\nResponse: ", &resp[10]);
	pass = auth_getpass(buf, SUDO_CONV_PROMPT_ECHO_OFF, callback);
	if (pass && *pass == '\0') {
	    free(pass);
	    pass = auth_getpass("Response [echo on]: ",
		SUDO_CONV_PROMPT_ECHO_ON, callback);
	}
    } else if (strncmp(resp, "chalnecho ", 10) == 0) {
	pass = auth_getpass(&resp[10], SUDO_CONV_PROMPT_ECHO_OFF, callback);
    } else if (strncmp(resp, "password", 8) == 0) {
	pass = auth_getpass(prompt, SUDO_CONV_PROMPT_ECHO_OFF, callback);
    } else if (strncmp(resp, "display ", 8) == 0) {
	sudo_printf(SUDO_CONV_INFO_MSG|SUDO_CONV_PREFER_TTY, "%s\n", &resp[8]);
	strlcpy(buf, "response noop", sizeof(buf));
	goto restart;
    } else {
	sudo_warnx("%s", resp);
	debug_return_int(AUTH_ERROR);
    }
    if (pass == NULL) {			/* ^C or error */
	debug_return_int(AUTH_INTR);
    }

    /* Send the user's response to the server */
    (void) snprintf(buf, sizeof(buf), "response '%s'", pass);
    if (auth_send(buf) || auth_recv(resp, sizeof(resp))) {
	sudo_warnx("%s", U_("lost connection to authentication server"));
	error = AUTH_ERROR;
	goto done;
    }

    if (strncmp(resp, "ok", 2) == 0) {
	error = AUTH_SUCCESS;
	goto done;
    }

    /* Main loop prints "Permission Denied" or insult. */
    if (strcmp(resp, "Permission Denied.") != 0)
	sudo_warnx("%s", resp);
    error = AUTH_FAILURE;
done:
    explicit_bzero(buf, sizeof(buf));
    freezero(pass, strlen(pass));
    debug_return_int(error);
}

int
sudo_fwtk_cleanup(const struct sudoers_context *ctx, struct passwd *pw,
    sudo_auth *auth, bool force)
{
    debug_decl(sudo_fwtk_cleanup, SUDOERS_DEBUG_AUTH);

    auth_close();
    debug_return_int(AUTH_SUCCESS);
}

#endif /* HAVE_FWTK */
