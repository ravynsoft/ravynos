/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1999-2005, 2010-2015 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <string.h>
#include <unistd.h>
#include <pwd.h>

#include <sudoers.h>
#include "sudo_auth.h"

#define DESLEN			13
#define HAS_AGEINFO(p, l)	(l == 18 && p[DESLEN] == ',')

int
sudo_passwd_init(const struct sudoers_context *ctx, struct passwd *pw,
    sudo_auth *auth)
{
    debug_decl(sudo_passwd_init, SUDOERS_DEBUG_AUTH);

    /* Only initialize once. */
    if (auth->data != NULL)
	debug_return_int(AUTH_SUCCESS);

#ifdef HAVE_SKEYACCESS
    if (skeyaccess(pw, ctx->user.tty, NULL, NULL) == 0)
	debug_return_int(AUTH_FAILURE);
#endif
    sudo_setspent();
    auth->data = sudo_getepw(pw);
    sudo_endspent();
    debug_return_int(auth->data ? AUTH_SUCCESS : AUTH_ERROR);
}

#ifdef HAVE_CRYPT
int
sudo_passwd_verify(const struct sudoers_context *ctx, struct passwd *pw,
    const char *pass, sudo_auth *auth, struct sudo_conv_callback *callback)
{
    char des_pass[9], *epass;
    char *pw_epasswd = auth->data;
    size_t pw_len;
    int ret;
    debug_decl(sudo_passwd_verify, SUDOERS_DEBUG_AUTH);

    /* An empty plain-text password must match an empty encrypted password. */
    if (pass[0] == '\0')
	debug_return_int(pw_epasswd[0] ? AUTH_FAILURE : AUTH_SUCCESS);

    /*
     * Truncate to 8 chars if standard DES since not all crypt()'s do this.
     */
    pw_len = strlen(pw_epasswd);
    if (pw_len == DESLEN || HAS_AGEINFO(pw_epasswd, pw_len)) {
	(void)strlcpy(des_pass, pass, sizeof(des_pass));
	pass = des_pass;
    }

    /*
     * Normal UN*X password check.
     * HP-UX may add aging info (separated by a ',') at the end so
     * only compare the first DESLEN characters in that case.
     */
    epass = (char *) crypt(pass, pw_epasswd);
    ret = AUTH_FAILURE;
    if (epass != NULL) {
	if (HAS_AGEINFO(pw_epasswd, pw_len) && strlen(epass) == DESLEN) {
	    if (strncmp(pw_epasswd, epass, DESLEN) == 0)
		ret = AUTH_SUCCESS;
	} else {
	    if (strcmp(pw_epasswd, epass) == 0)
		ret = AUTH_SUCCESS;
	}
    }

    explicit_bzero(des_pass, sizeof(des_pass));

    debug_return_int(ret);
}
#else
int
sudo_passwd_verify(const struct sudoers_context *ctx, struct passwd *pw,
    const char *pass, sudo_auth *auth, struct sudo_conv_callback *callback)
{
    char *pw_passwd = auth->data;
    int ret;
    debug_decl(sudo_passwd_verify, SUDOERS_DEBUG_AUTH);

    /* Simple string compare for systems without crypt(). */
    if (strcmp(pass, pw_passwd) == 0)
	ret = AUTH_SUCCESS;
    else
	ret = AUTH_FAILURE;

    debug_return_int(ret);
}
#endif

int
sudo_passwd_cleanup(const struct sudoers_context *ctx, struct passwd *pw,
    sudo_auth *auth, bool force)
{
    debug_decl(sudo_passwd_cleanup, SUDOERS_DEBUG_AUTH);

    if (auth->data != NULL) {
	/* Zero out encrypted password before freeing. */
	size_t len = strlen((char *)auth->data);
	freezero(auth->data, len);
	auth->data = NULL;
    }

    debug_return_int(AUTH_SUCCESS);
}
