/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2011-2017, 2019-2021 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef SUDO_CONF_H
#define SUDO_CONF_H

#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif

#include <sudo_queue.h>

/* Flags for sudo_conf_read() */
#define SUDO_CONF_DEBUG		0x01
#define SUDO_CONF_PATHS		0x02
#define SUDO_CONF_PLUGINS	0x04
#define SUDO_CONF_SETTINGS	0x08
#define SUDO_CONF_ALL		0x0f

/* Values of sudo_conf_group_source() */
#define GROUP_SOURCE_ADAPTIVE	0
#define GROUP_SOURCE_STATIC	1
#define GROUP_SOURCE_DYNAMIC	2

struct sudo_debug_file;
TAILQ_HEAD(sudo_conf_debug_file_list, sudo_debug_file);

struct plugin_info {
    TAILQ_ENTRY(plugin_info) entries;
    char *path;
    char *symbol_name;
    char **options;
    unsigned int lineno;
};
TAILQ_HEAD(plugin_info_list, plugin_info);

struct sudo_conf_debug {
    TAILQ_ENTRY(sudo_conf_debug) entries;
    struct sudo_conf_debug_file_list debug_files;
    char *progname;
};
TAILQ_HEAD(sudo_conf_debug_list, sudo_conf_debug);

/* Read main sudo.conf file. */
sudo_dso_public int sudo_conf_read_v1(const char *conf_file, int conf_types);
#define sudo_conf_read(_a, _b) sudo_conf_read_v1((_a), (_b))

/* Accessor functions. */
sudo_dso_public const char *sudo_conf_askpass_path_v1(void);
sudo_dso_public const char *sudo_conf_sesh_path_v1(void);
sudo_dso_public const char *sudo_conf_intercept_path_v1(void);
sudo_dso_public const char *sudo_conf_noexec_path_v1(void);
sudo_dso_public const char *sudo_conf_plugin_dir_path_v1(void);
sudo_dso_public const char *sudo_conf_devsearch_path_v1(void);
sudo_dso_public struct sudo_conf_debug_list *sudo_conf_debugging_v1(void);
sudo_dso_public struct sudo_conf_debug_file_list *sudo_conf_debug_files_v1(const char *progname);
sudo_dso_public struct plugin_info_list *sudo_conf_plugins_v1(void);
sudo_dso_public bool sudo_conf_disable_coredump_v1(void);
sudo_dso_public bool sudo_conf_developer_mode_v1(void);
sudo_dso_public bool sudo_conf_probe_interfaces_v1(void);
sudo_dso_public int sudo_conf_group_source_v1(void);
sudo_dso_public int sudo_conf_max_groups_v1(void);
sudo_dso_public void sudo_conf_clear_paths_v1(void);
#define sudo_conf_askpass_path() sudo_conf_askpass_path_v1()
#define sudo_conf_sesh_path() sudo_conf_sesh_path_v1()
#define sudo_conf_intercept_path() sudo_conf_intercept_path_v1()
#define sudo_conf_noexec_path() sudo_conf_noexec_path_v1()
#define sudo_conf_plugin_dir_path() sudo_conf_plugin_dir_path_v1()
#define sudo_conf_devsearch_path() sudo_conf_devsearch_path_v1()
#define sudo_conf_debugging() sudo_conf_debugging_v1()
#define sudo_conf_debug_files(_a) sudo_conf_debug_files_v1((_a))
#define sudo_conf_plugins() sudo_conf_plugins_v1()
#define sudo_conf_disable_coredump() sudo_conf_disable_coredump_v1()
#define sudo_conf_developer_mode() sudo_conf_developer_mode_v1()
#define sudo_conf_probe_interfaces() sudo_conf_probe_interfaces_v1()
#define sudo_conf_group_source() sudo_conf_group_source_v1()
#define sudo_conf_max_groups() sudo_conf_max_groups_v1()
#define sudo_conf_clear_paths() sudo_conf_clear_paths_v1()

#endif /* SUDO_CONF_H */
