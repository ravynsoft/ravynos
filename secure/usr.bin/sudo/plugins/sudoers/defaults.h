/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1999-2005, 2008-2023
 *	Todd C. Miller <Todd.Miller@sudo.ws>
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
 *
 * Sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and Air Force Research Laboratory, Air Force
 * Materiel Command, USAF, under agreement number F39502-99-1-0512.
 */

#ifndef SUDOERS_DEFAULTS_H
#define SUDOERS_DEFAULTS_H

#include <time.h>
#include <def_data.h>
#include <sudo_queue.h>

struct list_member {
    SLIST_ENTRY(list_member) entries;
    char *value;
};

SLIST_HEAD(list_members, list_member);

enum list_ops {
    add,
    delete,
    freeall
};

/* Mapping of tuple string value to enum def_tuple. */
struct def_values {
    const char *sval;	/* string value */
    enum def_tuple nval;/* numeric value */
};

union sudo_defs_val {
    bool flag;
    int ival;
    unsigned int uival;
    enum def_tuple tuple;
    char *str;
    mode_t mode;
    struct timespec tspec;
    struct list_members list;
};

/*
 * Structure describing compile-time and run-time options.
 */
struct sudoers_context;
struct sudo_defs_types {
    const char *name;
    int type;
    const char *desc;
    struct def_values *values;
    bool (*callback)(struct sudoers_context *ctx, const char *file, int line, int column, const union sudo_defs_val *, int op);
    union sudo_defs_val sd_un;
};

/*
 * Defaults values to apply before others.
 */
struct early_default {
    int idx;
    int run_callback;
    int line;
    int column;
    char *file;
};

/*
 * Four types of defaults: strings, integers, and flags.
 * Also, T_INT, T_TIMESPEC or T_STR may be ANDed with T_BOOL to indicate that
 * a value is not required.  Flags are boolean by nature...
 */
#undef T_INT
#define T_INT		0x001
#undef T_UINT
#define T_UINT		0x002
#undef T_STR
#define T_STR		0x003
#undef T_FLAG
#define T_FLAG		0x004
#undef T_MODE
#define T_MODE		0x005
#undef T_LIST
#define T_LIST		0x006
#undef T_LOGFAC
#define T_LOGFAC	0x007
#undef T_LOGPRI
#define T_LOGPRI	0x008
#undef T_TUPLE
#define T_TUPLE		0x009
#undef T_TIMESPEC
#define T_TIMESPEC	0x010
#undef T_TIMEOUT
#define T_TIMEOUT	0x011
#undef T_RLIMIT
#define T_RLIMIT	0x012
#undef T_PLUGIN
#define T_PLUGIN	0x013
#undef T_MASK
#define T_MASK		0x0FF
#undef T_BOOL
#define T_BOOL		0x100
#undef T_PATH
#define T_PATH		0x200
#undef T_CHPATH
#define T_CHPATH	0x400
#undef T_SPACE
#define T_SPACE		0x800

/*
 * Argument to update_defaults()
 */
#define SETDEF_GENERIC	0x01
#define	SETDEF_HOST	0x02
#define	SETDEF_USER	0x04
#define	SETDEF_RUNAS	0x08
#define	SETDEF_CMND	0x10
#define SETDEF_ALL	(SETDEF_GENERIC|SETDEF_HOST|SETDEF_USER|SETDEF_RUNAS|SETDEF_CMND)

/*
 * Convenience macros
 */
#define iolog_enabled   (def_log_stdin || def_log_ttyin || def_log_stdout || def_log_stderr || def_log_ttyout)

/*
 * Prototypes
 */
struct defaults_list;
struct sudoers_parse_tree;
void dump_default(void);
bool init_defaults(void);
bool set_default(struct sudoers_context *ctx, const char *var, const char *val, int op, const char *file, int line, int column, bool quiet);
bool update_defaults(struct sudoers_context *ctx, struct sudoers_parse_tree *parse_tree, struct defaults_list *defs, int what, bool quiet);
bool check_defaults(const struct sudoers_parse_tree *parse_tree, bool quiet);
bool append_default(const char *var, const char *val, int op, char *source, struct defaults_list *defs);
bool cb_passprompt_regex(struct sudoers_context *ctx, const char *file, int line, int column, const union sudo_defs_val *sd_un, int op);

extern struct sudo_defs_types sudo_defs_table[];

#endif /* SUDOERS_DEFAULTS_H */
