/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1996, 1998-2005, 2010-2012, 2014-2015
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
/*
 *  The code below basically comes from the examples supplied on
 *  the OSF DCE 1.0.3 manpages for the sec_login routines, with
 *  enough additional polishing to make the routine work with the
 *  rest of sudo.
 *
 *  This code is known to work on HP 700 and 800 series systems
 *  running HP-UX 9.X and 10.X, with either HP's version 1.2.1 of DCE.
 *  (aka, OSF DCE 1.0.3) or with HP's version 1.4 of DCE (aka, OSF
 *  DCE 1.1).
 */

#include <config.h>

#ifdef HAVE_DCE

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>

#include <dce/rpc.h>
#include <dce/sec_login.h>
#include <dce/dce_error.h> /* required to call dce_error_inq_text routine */

#include <sudoers.h>
#include "sudo_auth.h"
#include <timestamp.h>

static int check_dce_status(error_status_t, char *);

int
sudo_dce_verify(const struct sudoers_context *ctx, struct passwd *pw,
    const char *plain_pw, sudo_auth *auth, struct sudo_conv_callback *callback)
{
    struct passwd		temp_pw;
    sec_passwd_rec_t		password_rec;
    sec_login_handle_t		login_context;
    boolean32			reset_passwd;
    sec_login_auth_src_t	auth_src;
    error_status_t		status;
    debug_decl(sudo_dce_verify, SUDOERS_DEBUG_AUTH);

    if (IS_NONINTERACTIVE(auth))
	debug_return_int(AUTH_NONINTERACTIVE);

    /* Display lecture if needed and we haven't already done so. */
    display_lecture(callback);

    /*
     * Create the local context of the DCE principal necessary
     * to perform authenticated network operations.  The network
     * identity set up by this operation cannot be used until it
     * is validated via sec_login_validate_identity().
     */
    if (sec_login_setup_identity((unsigned_char_p_t) pw->pw_name,
	sec_login_no_flags, &login_context, &status)) {

	if (check_dce_status(status, "sec_login_setup_identity(1):"))
	    debug_return_int(AUTH_FAILURE);

	password_rec.key.key_type = sec_passwd_plain;
	password_rec.key.tagged_union.plain = (idl_char *) plain_pw;
	password_rec.pepper = NULL;
	password_rec.version_number = sec_passwd_c_version_none;

	/* Validate the login context with the password */
	if (sec_login_validate_identity(login_context, &password_rec,
	    &reset_passwd, &auth_src, &status)) {

	    if (check_dce_status(status, "sec_login_validate_identity(1):"))
		debug_return_int(AUTH_FAILURE);

	    /*
	     * Certify that the DCE Security Server used to set
	     * up and validate a login context is legitimate.  Makes
	     * sure that we didn't get spoofed by another DCE server.
	     */
	    if (!sec_login_certify_identity(login_context, &status)) {
		sudo_printf(SUDO_CONV_ERROR_MSG|SUDO_CONV_PREFER_TTY,
		    "Whoa! Bogus authentication server!\n");
		(void) check_dce_status(status,"sec_login_certify_identity(1):");
		debug_return_int(AUTH_FAILURE);
	    }
	    if (check_dce_status(status, "sec_login_certify_identity(2):"))
		debug_return_int(AUTH_FAILURE);

	    /*
	     * Sets the network credentials to those specified
	     * by the now validated login context.
	     */
	    sec_login_set_context(login_context, &status);
	    if (check_dce_status(status, "sec_login_set_context:"))
		debug_return_int(AUTH_FAILURE);

	    /*
	     * Oops, your credentials were no good. Possibly
	     * caused by clock times out of adjustment between
	     * DCE client and DCE security server...
	     */
	    if (auth_src != sec_login_auth_src_network) {
		    sudo_printf(SUDO_CONV_ERROR_MSG|SUDO_CONV_PREFER_TTY,
			"You have no network credentials.\n");
		    debug_return_int(AUTH_FAILURE);
	    }
	    /* Check if the password has aged and is thus no good */
	    if (reset_passwd) {
		    sudo_printf(SUDO_CONV_ERROR_MSG|SUDO_CONV_PREFER_TTY,
			"Your DCE password needs resetting.\n");
		    debug_return_int(AUTH_FAILURE);
	    }

	    /*
	     * We should be a valid user by this point.  Pull the
	     * user's password structure from the DCE security
	     * server just to make sure.  If we get it with no
	     * problems, then we really are legitimate...
	     */
	    sec_login_get_pwent(login_context, (sec_login_passwd_t) &temp_pw,
		&status);
	    if (check_dce_status(status, "sec_login_get_pwent:"))
		debug_return_int(AUTH_FAILURE);

	    /*
	     * If we get to here, then the pwent above properly fetched
	     * the password structure from the DCE registry, so the user
	     * must be valid.  We don't really care what the user's
	     * registry password is, just that the user could be
	     * validated.  In fact, if we tried to compare the local
	     * password to the DCE entry at this point, the operation
	     * would fail if the hidden password feature is turned on,
	     * because the password field would contain an asterisk.
	     * Also go ahead and destroy the user's DCE login context
	     * before we leave here (and don't bother checking the
	     * status), in order to clean up credentials files in
	     * /opt/dcelocal/var/security/creds.  By doing this, we are
	     * assuming that the user will not need DCE authentication
	     * later in the program, only local authentication.  If this
	     * is not true, then the login_context will have to be
	     * returned to the calling program, and the context purged
	     * somewhere later in the program.
	     */
	    sec_login_purge_context(&login_context, &status);
	    debug_return_int(AUTH_SUCCESS);
	} else {
	    if(check_dce_status(status, "sec_login_validate_identity(2):"))
		debug_return_int(AUTH_FAILURE);
	    sec_login_purge_context(&login_context, &status);
	    if(check_dce_status(status, "sec_login_purge_context:"))
		debug_return_int(AUTH_FAILURE);
	}
    }
    (void) check_dce_status(status, "sec_login_setup_identity(2):");
    debug_return_int(AUTH_FAILURE);
}

/* Returns 0 for DCE "ok" status, 1 otherwise */
static int
check_dce_status(error_status_t input_status, char *comment)
{
    int error_stat;
    unsigned char error_string[dce_c_error_string_len];
    debug_decl(check_dce_status, SUDOERS_DEBUG_AUTH);

    if (input_status == rpc_s_ok)
	debug_return_int(0);
    dce_error_inq_text(input_status, error_string, &error_stat);
    sudo_printf(SUDO_CONV_ERROR_MSG|SUDO_CONV_PREFER_TTY,
	"%s %s\n", comment, error_string);
    debug_return_int(1);
}

#endif /* HAVE_DCE */
