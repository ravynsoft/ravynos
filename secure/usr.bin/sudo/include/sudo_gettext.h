/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2011-2014 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef SUDO_GETTEXT_H
#define SUDO_GETTEXT_H

/*
 * Solaris locale.h includes libintl.h which causes problems when we
 * redefine the gettext functions.  We include it first to avoid this.
 */
#include <locale.h>

#ifdef HAVE_LIBINTL_H

# include <libintl.h>

/*
 * If DEFAULT_TEXT_DOMAIN is defined, use its value as the domain for
 * gettext() and ngettext() instead of the value set by textdomain().
 * This is used by the sudoers plugin as well as the convenience libraries.
 */
# ifdef DEFAULT_TEXT_DOMAIN
#  undef gettext
#  define gettext(String) \
    dgettext(DEFAULT_TEXT_DOMAIN, String)
#  undef ngettext
#  define ngettext(String, String_Plural, N) \
    dngettext(DEFAULT_TEXT_DOMAIN, String, String_Plural, N)
# endif

/*
 * Older versions of Solaris lack ngettext() so we have to kludge it.
 */
# ifndef HAVE_NGETTEXT
#  undef ngettext
#  define ngettext(String, String_Plural, N) \
    ((N) == 1 ? gettext(String) : gettext(String_Plural))
# endif

/* Gettext convenience macros */
# define _(String) gettext(String)
# define gettext_noop(String) String
# define N_(String) gettext_noop(String)
# define U_(String) sudo_warn_gettext(String)

#else /* !HAVE_LIBINTL_H */

/*
 * Internationalization is either unavailable or has been disabled.
 * Define away the gettext functions used by sudo.
 */
# define _(String) String 
# define N_(String) String
# define U_(String) String
# define textdomain(Domain)
# define bindtextdomain(Package, Directory)
# define ngettext(String, String_Plural, N) \
    ((N) == 1 ? (String) : (String_Plural))

#endif /* HAVE_LIBINTL_H */

#endif /* SUDO_GETTEXT_H */
