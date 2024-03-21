/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2013-2022 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sudo_compat.h>
#include <sudo_conf.h>
#include <sudo_debug.h>
#include <sudo_util.h>

static void sudo_conf_dump(void);

sudo_dso_public int main(int argc, char *argv[]);

/* Awful hack for macOS where the default group source is dynamic. */
#ifdef __APPLE__
# undef GROUP_SOURCE_ADAPTIVE
# define GROUP_SOURCE_ADAPTIVE GROUP_SOURCE_DYNAMIC
#endif

sudo_noreturn static void
usage(void)
{
    fprintf(stderr, "usage: %s [-v] conf_file\n", getprogname());
    exit(EXIT_FAILURE);
}

/*
 * Simple test driver for sudo_conf().
 * Parses the given configuration file and dumps the resulting
 * sudo_conf_data struct to the standard output.
 */
int
main(int argc, char *argv[])
{
    int ch;

    initprogname(argc > 0 ? argv[0] : "conf_test");

    while ((ch = getopt(argc, argv, "v")) != -1) {
	switch (ch) {
	case 'v':
	    /* ignore */
	    break;
	default:
	    usage();
	    /* NOTREACHED */
	}
    }
    argc -= optind;
    argv += optind;

    if (argc != 1)
	usage();

    sudo_conf_clear_paths();
    if (sudo_conf_read(argv[0], SUDO_CONF_ALL) == -1)
	return EXIT_FAILURE;
    sudo_conf_dump();

    return EXIT_SUCCESS;
}

static void
sudo_conf_dump(void)
{
    struct plugin_info_list *plugins = sudo_conf_plugins();
    struct sudo_conf_debug_list *debug_list = sudo_conf_debugging();
    struct sudo_conf_debug *debug_spec;
    struct sudo_debug_file *debug_file;
    struct plugin_info *info;

    printf("Set disable_coredump %s\n",
	sudo_conf_disable_coredump() ? "true" : "false");
    printf("Set group_source %s\n",
	sudo_conf_group_source() == GROUP_SOURCE_ADAPTIVE ? "adaptive" :
	sudo_conf_group_source() == GROUP_SOURCE_STATIC ? "static" : "dynamic");
    printf("Set max_groups %d\n", sudo_conf_max_groups());
    printf("Set probe_interfaces %s\n",
	sudo_conf_probe_interfaces() ? "true" : "false");
    if (sudo_conf_askpass_path() != NULL)
	printf("Path askpass %s\n", sudo_conf_askpass_path());
    if (sudo_conf_sesh_path() != NULL)
	printf("Path sesh %s\n", sudo_conf_sesh_path());
    if (sudo_conf_intercept_path() != NULL)
	printf("Path intercept %s\n", sudo_conf_intercept_path());
    if (sudo_conf_noexec_path() != NULL)
	printf("Path noexec %s\n", sudo_conf_noexec_path());
    if (sudo_conf_plugin_dir_path() != NULL)
	printf("Path plugin_dir %s\n", sudo_conf_plugin_dir_path());
    TAILQ_FOREACH(info, plugins, entries) {
	printf("Plugin %s %s", info->symbol_name, info->path);
	if (info->options) {
	    char * const * op;
	    for (op = info->options; *op != NULL; op++)
		printf(" %s", *op);
	}
	putchar('\n');
    }
    TAILQ_FOREACH(debug_spec, debug_list, entries) {
	TAILQ_FOREACH(debug_file, &debug_spec->debug_files, entries) {
	    printf("Debug %s %s %s\n", debug_spec->progname,
		debug_file->debug_file, debug_file->debug_flags);
	}
    }
}
