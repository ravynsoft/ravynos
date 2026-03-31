/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2020 Todd C. Miller <Todd.Miller@sudo.ws>
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
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif /* HAVE_STDBOOL_H */
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sudo_compat.h>
#include <sudo_conf.h>
#include <sudo_debug.h>
#include <sudo_fatal.h>
#include <sudo_gettext.h>
#include <sudo_plugin.h>
#include <sudo_util.h>

static int approval_debug_instance = SUDO_DEBUG_INSTANCE_INITIALIZER;
sudo_printf_t sudo_printf;

static int
sample_approval_open(unsigned int version, sudo_conv_t conversation,
    sudo_printf_t plugin_printf, char * const settings[],
    char * const user_info[], int submit_optind, char * const submit_argv[],
    char * const submit_envp[], char * const plugin_options[],
    const char **errstr)
{
    struct sudo_conf_debug_file_list debug_files =
	TAILQ_HEAD_INITIALIZER(debug_files);
    struct sudo_debug_file *debug_file;
    const char *cp, *plugin_path = NULL;
    char * const *cur;
    int ret = -1;
    debug_decl_vars(sample_approval_open, SUDO_DEBUG_PLUGIN);

    sudo_printf = plugin_printf;

    /* Initialize the debug subsystem.  */
    for (cur = settings; (cp = *cur) != NULL; cur++) {
        if (strncmp(cp, "debug_flags=", sizeof("debug_flags=") - 1) == 0) {
            cp += sizeof("debug_flags=") - 1;
            if (sudo_debug_parse_flags(&debug_files, cp) == -1)
                goto oom;
            continue;
        }
        if (strncmp(cp, "plugin_path=", sizeof("plugin_path=") - 1) == 0) {
            plugin_path = cp + sizeof("plugin_path=") - 1;
            continue;
        }
    }
    if (plugin_path != NULL && !TAILQ_EMPTY(&debug_files)) {
	approval_debug_instance =
	    sudo_debug_register(plugin_path, NULL, NULL, &debug_files, -1);
	if (approval_debug_instance == SUDO_DEBUG_INSTANCE_ERROR) {
	    *errstr = U_("unable to initialize debugging");
	    goto done;
	}
	sudo_debug_enter(__func__, __FILE__, __LINE__, sudo_debug_subsys);
    }

    ret = 1;
    goto done;

oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    *errstr = U_("unable to allocate memory");

done:
    while ((debug_file = TAILQ_FIRST(&debug_files)) != NULL) {
	TAILQ_REMOVE(&debug_files, debug_file, entries);
	free(debug_file->debug_file);
	free(debug_file->debug_flags);
	free(debug_file);
    }

    debug_return_int(ret);
}

static void
sample_approval_close(void)
{
    debug_decl(sample_approval_close, SUDO_DEBUG_PLUGIN);

    /* Nothing here, we could store a NULL pointer instead. */

    debug_return;
}

static int
sample_approval_check(char * const command_info[], char * const run_argv[],
    char * const run_envp[], const char **errstr)
{
    struct tm tm;
    time_t now;
    int ret = 0;
    debug_decl(sample_approval_check, SUDO_DEBUG_PLUGIN);

    /*
     * Only approve requests that are within business hours,
     * which are 9am - 5pm local time.  Does not check holidays.
     */
    if (time(&now) == -1 || localtime_r(&now, &tm) == NULL)
	goto done;
    if (tm.tm_wday < 1 || tm.tm_wday > 5) {
	/* bad weekday */
	goto done;
    }
    if (tm.tm_hour < 9 || tm.tm_hour > 17 ||
	    (tm.tm_hour == 17 && tm.tm_min > 0)) {
	/* bad hour */
	goto done;
    }
    ret = 1;

done:
    if (ret == 0) {
	*errstr = U_("You are not allowed to use sudo outside business hours");
	sudo_printf(SUDO_CONV_ERROR_MSG, "%s\n", *errstr);
    }

    debug_return_int(ret);
}

static int
sample_approval_show_version(int verbose)
{
    debug_decl(approval_show_version, SUDO_DEBUG_PLUGIN);

    sudo_printf(SUDO_CONV_INFO_MSG, "sample approval plugin version %s\n",
        PACKAGE_VERSION);

    debug_return_int(true);
}

sudo_dso_public struct approval_plugin sample_approval = {
    SUDO_APPROVAL_PLUGIN,
    SUDO_API_VERSION,
    sample_approval_open,
    sample_approval_close,
    sample_approval_check,
    sample_approval_show_version
};
