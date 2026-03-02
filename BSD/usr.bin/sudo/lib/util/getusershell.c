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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_util.h>

#ifdef TESTSUDOERS
# include <tsgetgrpw.h>
#endif

static const char *shellfile = "/etc/shell";
static char **allowed_shells, * const *current_shell;
static const char *default_shells[] = {
    "/bin/sh",
    "/bin/ksh",
    "/bin/ksh93",
    "/bin/bash",
    "/bin/dash",
    "/bin/zsh",
    "/bin/csh",
    "/bin/tcsh",
    NULL
};

static char **
read_shells(void)
{
    size_t maxshells = 16, nshells = 0;
    size_t linesize = 0;
    char *line = NULL;
    FILE *fp;
    debug_decl(read_shells, SUDO_DEBUG_UTIL);

    if ((fp = fopen(shellfile, "r")) == NULL)
	goto bad;

    free(allowed_shells);
    allowed_shells = reallocarray(NULL, maxshells, sizeof(char *));
    if (allowed_shells == NULL)
	goto bad;

    while (sudo_parseln(&line, &linesize, NULL, fp, PARSELN_CONT_IGN) != -1) {
	if (nshells + 1 >= maxshells) {
	    char **new_shells;

	    new_shells = reallocarray(NULL, maxshells + 16, sizeof(char *));
	    if (new_shells == NULL)
		goto bad;
	    allowed_shells = new_shells;
	    maxshells += 16;
	}
	if ((allowed_shells[nshells] = strdup(line)) == NULL)
	    goto bad;
	nshells++;
    }
    allowed_shells[nshells] = NULL;

    free(line);
    fclose(fp);
    debug_return_ptr(allowed_shells);
bad:
    free(line);
    if (fp != NULL)
	fclose(fp);
    while (nshells != 0)
	free(allowed_shells[--nshells]);
    free(allowed_shells);
    allowed_shells = NULL;
    debug_return_ptr(default_shells);
}

void
sudo_setusershell(void)
{
    debug_decl(setusershell, SUDO_DEBUG_UTIL);

    current_shell = read_shells();

    debug_return;
}

void
sudo_endusershell(void)
{
    debug_decl(endusershell, SUDO_DEBUG_UTIL);

    if (allowed_shells != NULL) {
	char **shell;

	for (shell = allowed_shells; *shell != NULL; shell++)
	    free(*shell);
	free(allowed_shells);
	allowed_shells = NULL;
    }
    current_shell = NULL;

    debug_return;
}

char *
sudo_getusershell(void)
{
    debug_decl(getusershell, SUDO_DEBUG_UTIL);

    if (current_shell == NULL)
	current_shell = read_shells();

    debug_return_str(*current_shell++);
}

#ifdef TESTSUDOERS
void
testsudoers_setshellfile(const char *file)
{
    testsudoers_endusershell();
    shellfile = file;
}
#endif /* TESTSUDOERS */
