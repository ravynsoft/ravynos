/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1998-2005, 2010-2015 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifdef HAVE_GETPRPWNAM

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#ifdef __hpux
#  undef MAXINT
#  include <hpsecurity.h>
#else
#  include <sys/security.h>
#endif /* __hpux */
#include <prot.h>

#include <sudoers.h>
#include "sudo_auth.h"

#ifdef __alpha
extern int crypt_type;
#endif

int
sudo_secureware_init(const struct sudoers_context *ctx, struct passwd *pw,
    sudo_auth *auth)
{
    debug_decl(sudo_secureware_init, SUDOERS_DEBUG_AUTH);

    /* Only initialize once. */
    if (auth->data != NULL)
	debug_return_int(AUTH_SUCCESS);

#ifdef __alpha
    if (crypt_type == INT_MAX)
	debug_return_int(AUTH_FAILURE);			/* no shadow */
#endif

    sudo_setspent();
    auth->data = sudo_getepw(pw);
    sudo_endspent();
    debug_return_int(auth->data ? AUTH_SUCCESS : AUTH_ERROR);
}

int
sudo_secureware_verify(const struct sudoers_context *ctx, struct passwd *pw,
    const char *pass, sudo_auth *auth, struct sudo_conv_callback *callback)
{
    char *pw_epasswd = auth->data;
    char *epass = NULL;
    debug_decl(sudo_secureware_verify, SUDOERS_DEBUG_AUTH);

    /* An empty plain-text password must match an empty encrypted password. */
    if (pass[0] == '\0')
	debug_return_int(pw_epasswd[0] ? AUTH_FAILURE : AUTH_SUCCESS);

#if defined(__alpha)
# ifdef HAVE_DISPCRYPT
	epass = dispcrypt(pass, pw_epasswd, crypt_type);
# else
	if (crypt_type == AUTH_CRYPT_BIGCRYPT)
	    epass = bigcrypt(pass, pw_epasswd);
	else if (crypt_type == AUTH_CRYPT_CRYPT16)
	    epass = crypt(pass, pw_epasswd);
# endif /* HAVE_DISPCRYPT */
#elif defined(HAVE_BIGCRYPT)
    epass = bigcrypt(pass, pw_epasswd);
#endif /* __alpha */

    if (epass != NULL && strcmp(pw_epasswd, epass) == 0)
	debug_return_int(AUTH_SUCCESS);
    debug_return_int(AUTH_FAILURE);
}

int
sudo_secureware_cleanup(const struct sudoers_context *ctx, struct passwd *pw,
    sudo_auth *auth, bool force)
{
    char *pw_epasswd = auth->data;
    debug_decl(sudo_secureware_cleanup, SUDOERS_DEBUG_AUTH);

    if (pw_epasswd != NULL)
	freezero(pw_epasswd, strlen(pw_epasswd));
    debug_return_int(AUTH_SUCCESS);
}

#endif /* HAVE_GETPRPWNAM */
