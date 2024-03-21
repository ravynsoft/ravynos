/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2010, 2011, 2013 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifdef HAVE_GSS_KRB5_CCACHE_NAME
# if defined(HAVE_GSSAPI_GSSAPI_KRB5_H)
#  include <gssapi/gssapi.h>
#  include <gssapi/gssapi_krb5.h>
# elif defined(HAVE_GSSAPI_GSSAPI_H)
#  include <gssapi/gssapi.h>
# else
#  include <gssapi.h>
# endif
#endif

#include <sudo.h>
#include <sudo_dso.h>
#include <sudo_plugin.h>

#ifdef STATIC_SUDOERS_PLUGIN

extern struct policy_plugin sudoers_policy;
extern struct io_plugin sudoers_io;
extern struct audit_plugin sudoers_audit;

static struct sudo_preload_symbol sudo_rtld_default_symbols[] = {
# ifdef HAVE_GSS_KRB5_CCACHE_NAME
    { "gss_krb5_ccache_name", (void *)&gss_krb5_ccache_name},
# endif
    { (const char *)0, (void *)0 }
};

/* XXX - can we autogenerate these? */
static struct sudo_preload_symbol sudo_sudoers_plugin_symbols[] = {
    { "sudoers_policy", (void *)&sudoers_policy },
    { "sudoers_io", (void *)&sudoers_io },
    { "sudoers_audit", (void *)&sudoers_audit },
    { (const char *)0, (void *)0 }
};

/*
 * Statically compiled symbols indexed by handle.
 */
static struct sudo_preload_table sudo_preload_table[] = {
    { (char *)0, SUDO_DSO_DEFAULT, sudo_rtld_default_symbols },
    { _PATH_SUDOERS_PLUGIN, &sudo_sudoers_plugin_symbols, sudo_sudoers_plugin_symbols },
    { (char *)0, (void *)0, (struct sudo_preload_symbol *)0 }
};

void
preload_static_symbols(void)
{
    sudo_dso_preload_table(sudo_preload_table);
}

#endif /* STATIC_SUDOERS_PLUGIN */
