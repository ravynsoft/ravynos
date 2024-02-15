/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2013-2018 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sudoers.h>
#include <sudo_digest.h>

unsigned char *
sudo_filedigest(int fd, const char *file, unsigned int digest_type,
    size_t *digest_len)
{
    unsigned char *file_digest = NULL;
    unsigned char buf[32 * 1024];
    struct sudo_digest *dig = NULL;
    FILE *fp = NULL;
    size_t nread;
    int fd2;
    debug_decl(sudo_filedigest, SUDOERS_DEBUG_UTIL);

    *digest_len = sudo_digest_getlen(digest_type);
    if (*digest_len == 0) {
	sudo_warnx(U_("unsupported digest type %u for %s"), digest_type, file);
	debug_return_ptr(NULL);
    }

    if ((fd2 = dup(fd)) == -1) {
	sudo_debug_printf(SUDO_DEBUG_INFO, "unable to dup %s: %s",
	    file, strerror(errno));
	debug_return_ptr(NULL);
    }
    if ((fp = fdopen(fd2, "r")) == NULL) {
	sudo_debug_printf(SUDO_DEBUG_INFO, "unable to fdopen %s: %s",
	    file, strerror(errno));
	close(fd2);
	goto bad;
    }
    if ((file_digest = malloc(*digest_len)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto bad;
    }
    if ((dig = sudo_digest_alloc(digest_type)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	goto bad;
    }
    while ((nread = fread(buf, 1, sizeof(buf), fp)) != 0) {
	sudo_digest_update(dig, buf, nread);
    }
    if (ferror(fp)) {
	sudo_warnx(U_("%s: read error"), file);
	goto bad;
    }
    sudo_digest_final(dig, file_digest);
    sudo_digest_free(dig);
    fclose(fp);

    debug_return_ptr(file_digest);
bad:
    sudo_digest_free(dig);
    free(file_digest);
    if (fp != NULL)
	fclose(fp);
    debug_return_ptr(NULL);
}
