/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2022 Will Shand <wss2ec@virginia.edu>
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

#ifdef HAVE_APPARMOR

# include <stdio.h>
# include <stdlib.h>
# include <sys/apparmor.h>

# include <sudo.h>
# include <sudo_debug.h>

/**
 * @brief Check whether AppArmor is enabled.
 *
 * @return 1 if AppArmor is enabled, 0 otherwise.
 */
int
apparmor_is_enabled(void)
{
    int ret;
    FILE *fd;
    debug_decl(apparmor_is_enabled, SUDO_DEBUG_APPARMOR);

    /*
     * Check whether AppArmor is enabled by reading
     * /sys/module/apparmor/parameters/enabled
     *
     * When this file exists and its contents are equal to "Y", AppArmor
     * is enabled. This is a little more reliable than using
     * aa_is_enabled(2), which performs an additional check on securityfs
     * that will fail in settings where securityfs isn't available
     * (e.g. inside a container).
     */

    fd = fopen("/sys/module/apparmor/parameters/enabled", "r");
    if (fd == NULL)
        debug_return_int(0);

    ret = (fgetc(fd) == 'Y');

    fclose(fd);
    debug_return_int(ret);
}

/**
 * @brief Prepare to transition into a new AppArmor profile.
 *
 * @param new_profile The AppArmor profile to transition into on the
 *                    next exec.
 *
 * @return 0 on success, and a nonzero value on failure.
 */
int
apparmor_prepare(const char *new_profile)
{
    int ret;
    char *mode, *old_profile;
    debug_decl(apparmor_prepare, SUDO_DEBUG_APPARMOR);

    /* Determine the current AppArmor confinement status */
    if ((ret = aa_getcon(&old_profile, &mode)) == -1) {
        sudo_warn("%s", U_("failed to determine AppArmor confinement"));
        old_profile = NULL;
        goto done;
    }

    /* Tell AppArmor to transition into the new profile on the
     * next exec */
    if ((ret = aa_change_onexec(new_profile)) != 0) {
        sudo_warn(U_("unable to change AppArmor profile to %s"), new_profile);
        goto done;
    }

    if (mode == NULL) {
        sudo_debug_printf(SUDO_DEBUG_INFO,
	    "%s: changing AppArmor profile: %s -> %s", __func__,
	    old_profile, new_profile ? new_profile : "?");
    } else {
        sudo_debug_printf(SUDO_DEBUG_INFO,
	    "%s: changing AppArmor profile: %s (%s) -> %s", __func__,
	    old_profile, mode, new_profile ? new_profile : "?");
    }

done:
    /*
     * The profile string returned by aa_getcon must be free'd, while the
     * mode string must _not_ be free'd.
     */
    if (old_profile != NULL)
        free(old_profile);

    debug_return_int(ret);
}

#endif /* HAVE_APPARMOR */
