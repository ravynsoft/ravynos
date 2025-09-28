/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2018, 2021-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef SUDOERS_CVTSUDOERS_H
#define SUDOERS_CVTSUDOERS_H

#include <strlist.h>

/* Supported input/output formats. */
enum sudoers_formats {
    format_csv,
    format_json,
    format_ldif,
    format_sudoers
};

/* Flags for cvtsudoers_config.defaults */
#define CVT_DEFAULTS_GLOBAL	0x01U
#define CVT_DEFAULTS_USER	0x02U
#define CVT_DEFAULTS_RUNAS	0x04U
#define CVT_DEFAULTS_HOST	0x08U
#define CVT_DEFAULTS_CMND	0x10U
#define CVT_DEFAULTS_ALL	0xffU

/* Flags for cvtsudoers_config.suppress */
#define SUPPRESS_DEFAULTS	0x01U
#define SUPPRESS_ALIASES	0x02U
#define SUPPRESS_PRIVS		0x04U

/* cvtsudoers.conf settings */
struct cvtsudoers_config {
    unsigned int sudo_order;
    unsigned int order_increment;
    unsigned int order_padding;
    unsigned int order_max;
    unsigned int defaults;
    unsigned int suppress;
    bool store_options;
    bool expand_aliases;
    bool prune_matches;
    bool match_local;
    char *sudoers_base;
    char *input_format;
    char *output_format;
    char *filter;
    char *logfile;
    char *defstr;
    char *supstr;
    char *group_file;
    char *passwd_file;
};

/* Initial config settings for above. */
#define INITIAL_CONFIG { 1, 1, 0, 0, CVT_DEFAULTS_ALL, 0, true }

#define CONF_BOOL	0
#define CONF_UINT	1
#define CONF_STR	2

struct cvtsudoers_conf_table {
    const char *conf_str;	/* config file string */
    int type;			/* CONF_BOOL, CONF_UINT, CONF_STR */
    void *valp;			/* pointer into cvtsudoers_config */
};

struct cvtsudoers_filter {
    struct sudoers_str_list users;
    struct sudoers_str_list groups;
    struct sudoers_str_list hosts;
    struct sudoers_str_list cmnds;
};

/* cvtsudoers.c */
extern struct cvtsudoers_filter *filters;
void log_warnx(const char * restrict fmt, ...) sudo_printflike(1, 2);

/* cvtsudoers_csv.c */
bool convert_sudoers_csv(const struct sudoers_parse_tree *parse_tree, const char *output_file, struct cvtsudoers_config *conf);

/* cvtsudoers_json.c */
bool convert_sudoers_json(const struct sudoers_parse_tree *parse_tree, const char *output_file, struct cvtsudoers_config *conf);

/* cvtsudoers_ldif.c */
bool convert_sudoers_ldif(const struct sudoers_parse_tree *parse_tree, const char *output_file, struct cvtsudoers_config *conf);

/* cvtsudoers_merge.c */
struct sudoers_parse_tree *merge_sudoers(struct sudoers_parse_tree_list *parse_trees, struct sudoers_parse_tree *merged_tree);

/* cvtsudoers_pwutil.c */
struct cache_item *cvtsudoers_make_pwitem(uid_t uid, const char *name);
struct cache_item *cvtsudoers_make_gritem(gid_t gid, const char *name);
struct cache_item *cvtsudoers_make_gidlist_item(const struct passwd *pw, int unusued1,  GETGROUPS_T *unused2, char * const *unused3, unsigned int type);
struct cache_item *cvtsudoers_make_grlist_item(const struct passwd *pw, char * const *unused1);

#endif /* SUDOERS_CVTSUDOERS_H */
