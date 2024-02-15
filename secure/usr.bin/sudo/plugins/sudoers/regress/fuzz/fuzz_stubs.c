/*
 * Copyright (c) 2021 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <config.h>

#include <sys/socket.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pwd.h>
#include <unistd.h>
#if defined(HAVE_STDINT_H)
# include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
# include <inttypes.h>
#endif
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef NEED_RESOLV_H
# include <arpa/nameser.h>
# include <resolv.h>
#endif /* NEED_RESOLV_H */
#include <netdb.h>

#include <sudoers.h>
#include <timestamp.h>
#include <interfaces.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

struct interface_list *
get_interfaces(void)
{
    static struct interface_list empty = SLIST_HEAD_INITIALIZER(interfaces);
    return &empty;
}

void
init_eventlog_config(void)
{
    return;
}

bool
pivot_root(const char *new_root, struct sudoers_pivot *state)
{
    return true;
}

bool
unpivot_root(struct sudoers_pivot *state)
{
    return true;
}

int
group_plugin_query(const char *user, const char *group, const struct passwd *pw)
{
    return false;
}

bool
set_perms(const struct sudoers_context *ctx, int perm)
{
    return true;
}

bool
restore_perms(void)
{
    return true;
}

bool
rewind_perms(void)
{
    return true;
}

bool
sudo_nss_can_continue(const struct sudo_nss *nss, int match)
{
    return true;
}
