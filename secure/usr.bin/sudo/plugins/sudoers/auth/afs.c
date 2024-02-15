/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1999, 2001-2005, 2007, 2010-2012, 2014-2015
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

#ifdef HAVE_AFS

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>

#include <afs/stds.h>
#include <afs/kautils.h>

#include <sudoers.h>
#include "sudo_auth.h"
#include <timestamp.h>

int
sudo_afs_verify(const struct sudoers_context *ctx, struct passwd *pw,
    const char *pass, sudo_auth *auth, struct sudo_conv_callback *callback)
{
    struct ktc_encryptionKey afs_key;
    struct ktc_token afs_token;
    debug_decl(sudo_afs_verify, SUDOERS_DEBUG_AUTH);

    if (IS_NONINTERACTIVE(auth))
	debug_return_int(AUTH_NONINTERACTIVE);

    /* Display lecture if needed and we haven't already done so. */
    display_lecture(callback);

    /* Try to just check the password */
    ka_StringToKey(pass, NULL, &afs_key);
    if (ka_GetAdminToken(pw->pw_name,		/* name */
			 NULL,			/* instance */
			 NULL,			/* realm */
			 &afs_key,		/* key (contains password) */
			 0,			/* lifetime */
			 &afs_token,		/* token */
			 0) == 0)		/* new */
	debug_return_int(AUTH_SUCCESS);

    /* Fall back on old method XXX - needed? */
    setpag();
    if (ka_UserAuthenticateGeneral(KA_USERAUTH_VERSION+KA_USERAUTH_DOSETPAG,
				   pw->pw_name,	/* name */
				   NULL,	/* instance */
				   NULL,	/* realm */
				   pass,	/* password */
				   0,		/* lifetime */
				   NULL,	/* expiration ptr (unused) */
				   0,		/* spare */
				   NULL) == 0)	/* reason */
	debug_return_int(AUTH_SUCCESS);

    debug_return_int(AUTH_FAILURE);
}

#endif HAVE_AFS
