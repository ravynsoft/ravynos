/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2019-2020 Todd C. Miller <Todd.Miller@sudo.ws>
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

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif /* HAVE_STDBOOL_H */
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_gettext.h>
#include <sudo_util.h>
#include <sudo_iolog.h>

/*
 * Parse a string in the form host[:port] where host can also be
 * an IPv4 address or an IPv6 address in square brackets.
 * Fills in hostp and portp which may point within str, which is modified.
 */
bool
iolog_parse_host_port(char *str, char **hostp, char **portp, bool *tlsp,
     const char *defport, const char *defport_tls)
{
    char *flags, *port, *host = str;
    bool ret = false;
    bool tls = false;
    debug_decl(iolog_parse_host_port, SUDO_DEBUG_UTIL);

    /* Check for IPv6 address like [::0] followed by optional port */
    if (*host == '[') {
	host++;
	port = strchr(host, ']');
	if (port == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"invalid IPv6 address %s", str);
	    goto done;
	}
	*port++ = '\0';
        switch (*port) {
        case ':':
            port++;
            break;
        case '\0':
            port = NULL;		/* no port specified */
            break;
        case '(':
            /* flag, handled below */
            break;
        default:
	    sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		"invalid IPv6 address %s", str);
	    goto done;
	}
    } else {
	port = strrchr(host, ':');
	if (port != NULL)
	    *port++ = '\0';
    }

    /* Check for optional tls flag at the end. */
    flags = strchr(port ? port : host, '(');
    if (flags != NULL) {
	if (strcasecmp(flags, "(tls)") == 0)
	    tls = true;
	*flags = '\0';
	if (port == flags)
	    port = NULL;
    }

    if (port == NULL)
	port = tls ? (char *)defport_tls : (char *)defport;
    else if (*port == '\0')
	goto done;

    *hostp = host;
    *portp = port;
    *tlsp = tls;

    ret = true;

done:
    debug_return_bool(ret);
}
