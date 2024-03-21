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

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#ifdef HAVE_GETSPNAM
# include <shadow.h>
#endif /* HAVE_GETSPNAM */
#ifdef HAVE_GETPRPWNAM
# ifdef __hpux
#  undef MAXINT
#  include <hpsecurity.h>
# else
#  include <sys/security.h>
# endif /* __hpux */
# include <prot.h>
#endif /* HAVE_GETPRPWNAM */

#include <sudoers.h>

/*
 * Exported for auth/secureware.c
 */
#if defined(HAVE_GETPRPWNAM) && defined(__alpha)
int crypt_type = INT_MAX;
#endif /* HAVE_GETPRPWNAM && __alpha */

/*
 * Return a copy of the encrypted password for the user described by pw.
 * If shadow passwords are in use, look in the shadow file.
 */
char *
sudo_getepw(const struct passwd *pw)
{
    char *epw = NULL;
    debug_decl(sudo_getepw, SUDOERS_DEBUG_AUTH);

    /* If there is a function to check for shadow enabled, use it... */
#ifdef HAVE_ISCOMSEC
    if (!iscomsec())
	goto done;
#endif /* HAVE_ISCOMSEC */

#ifdef HAVE_GETPWNAM_SHADOW
    {
	struct passwd *spw;

	/* On OpenBSD we need to closed the non-shadow passwd db first. */
	endpwent();
	if ((spw = getpwnam_shadow(pw->pw_name)) != NULL)
	    epw = spw->pw_passwd;
	setpassent(1);
    }
#endif /* HAVE_GETPWNAM_SHADOW */
#ifdef HAVE_GETPRPWNAM
    {
	struct pr_passwd *spw;

	if ((spw = getprpwnam(pw->pw_name)) && spw->ufld.fd_encrypt) {
# ifdef __alpha
	    crypt_type = spw->ufld.fd_oldcrypt;
# endif /* __alpha */
	    epw = spw->ufld.fd_encrypt;
	}
    }
#endif /* HAVE_GETPRPWNAM */
#ifdef HAVE_GETSPNAM
    {
	struct spwd *spw;

	if ((spw = getspnam(pw->pw_name)) && spw->sp_pwdp)
	    epw = spw->sp_pwdp;
    }
#endif /* HAVE_GETSPNAM */

#if defined(HAVE_ISCOMSEC)
done:
#endif
    /* If no shadow password, fall back on regular password. */
    debug_return_str(strdup(epw ? epw : pw->pw_passwd));
}

void
sudo_setspent(void)
{
    debug_decl(sudo_setspent, SUDOERS_DEBUG_AUTH);

#ifdef HAVE_GETPRPWNAM
    setprpwent();
#endif
#ifdef HAVE_GETSPNAM
    setspent();
#endif
    debug_return;
}

void
sudo_endspent(void)
{
    debug_decl(sudo_endspent, SUDOERS_DEBUG_AUTH);

#ifdef HAVE_GETPRPWNAM
    endprpwent();
#endif
#ifdef HAVE_GETSPNAM
    endspent();
#endif
    debug_return;
}
