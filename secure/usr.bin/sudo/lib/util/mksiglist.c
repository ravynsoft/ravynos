/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2010-2012, 2015, 2021 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <stdlib.h>
#include <signal.h>

#include <sudo_compat.h>

sudo_dso_public int main(int argc, char *argv[]);

int
main(int argc, char *argv[])
{
    unsigned int i;

#include "mksiglist.h"

    /*
     * For portability we must not use %zu below.
     * This program is compiled with the host C compiler,
     * so it cannot use any of the functions in libsudo_util.
     */
    puts("const char *const sudo_sys_siglist[] = {");
    for (i = 0; i < NSIG; i++) {
	if (sudo_sys_siglist[i] != NULL) {
	    printf("    \"%s\",\n", sudo_sys_siglist[i]);
	} else {
	    printf("    \"Signal %u\",\n", i);
	}
    }
    puts("};");

    return EXIT_SUCCESS;
}
