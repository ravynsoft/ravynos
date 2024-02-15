/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2023 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <fcntl.h>
#include <unistd.h>

#include <sudoers.h>

/*
 * Pivot to a new root directory, storing the old root and old cwd
 * in state.  Changes current working directory to the new root.
 * Returns true on success, else false.
 */
bool
pivot_root(const char *new_root, struct sudoers_pivot *state)
{
    debug_decl(pivot_root, SUDOERS_DEBUG_UTIL);

    state->saved_root = open("/", O_RDONLY);
    state->saved_cwd = open(".", O_RDONLY);
    if (state->saved_root == -1 || state->saved_cwd == -1 || chroot(new_root) == -1) {
	if (state->saved_root != -1) {
	    close(state->saved_root);
	    state->saved_root = -1;
	}
	if (state->saved_cwd != -1) {
	    close(state->saved_cwd);
	    state->saved_cwd = -1;
	}
	debug_return_bool(false);
    }
    debug_return_bool(chdir("/") == 0);
}

/*
 * Pivot back to the stored root directory and restore the old cwd.
 * Returns true on success, else false.
 */
bool
unpivot_root(struct sudoers_pivot *state)
{
    bool ret = true;
    debug_decl(unpivot_root, SUDOERS_DEBUG_UTIL);

    /* Order is important: restore old root, *then* change cwd. */
    if (state->saved_root != -1) {
	if (fchdir(state->saved_root) == -1 || chroot(".") == -1) {
	    sudo_warn("%s", U_("unable to restore root directory"));
	    ret = false;
	}
	close(state->saved_root);
	state->saved_root = -1;
    }
    if (state->saved_cwd != -1) {
	if (fchdir(state->saved_cwd) == -1) {
	    sudo_warn("%s", U_("unable to restore current working directory"));
	    ret = false;
	}
	close(state->saved_cwd);
	state->saved_cwd = -1;
    }

    debug_return_bool(ret);
}
