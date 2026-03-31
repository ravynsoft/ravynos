/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2020-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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
#include <ctype.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>

#include <sudo_compat.h>
#include <sudo_debug.h>
#include <sudo_eventlog.h>
#include <sudo_fatal.h>
#include <sudo_gettext.h>
#include <sudo_util.h>

#include <parse_json.h>

struct json_stack {
    unsigned int depth;
    unsigned int maxdepth;
    struct eventlog_json_object *frames[64];
};
#define JSON_STACK_INTIALIZER(s) { 0, nitems((s).frames) };

static char *iolog_file;

static bool
json_store_columns(struct json_item *item, struct eventlog *evlog)
{
    debug_decl(json_store_columns, SUDO_DEBUG_UTIL);

    if (item->u.number < 1 || item->u.number > INT_MAX) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "tty cols %lld: out of range", item->u.number);
	evlog->columns = 0;
	debug_return_bool(false);
    }

    evlog->columns = (int)item->u.number;
    debug_return_bool(true);
}

static bool
json_store_command(struct json_item *item, struct eventlog *evlog)
{
    debug_decl(json_store_command, SUDO_DEBUG_UTIL);

    /*
     * Note: struct eventlog must store command + args.
     *       We don't have argv yet so we append the args later.
     */
    free(evlog->command);
    evlog->command = item->u.string;
    item->u.string = NULL;
    debug_return_bool(true);
}

static bool
json_store_dumped_core(struct json_item *item, struct eventlog *evlog)
{
    debug_decl(json_store_dumped_core, SUDO_DEBUG_UTIL);

    evlog->dumped_core = item->u.boolean;
    debug_return_bool(true);
}

static bool
json_store_exit_value(struct json_item *item, struct eventlog *evlog)
{
    debug_decl(json_store_exit_value, SUDO_DEBUG_UTIL);

    if (item->u.number < 0 || item->u.number > INT_MAX) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "exit value %lld: out of range", item->u.number);
	evlog->exit_value = -1;
	debug_return_bool(false);
    }

    evlog->exit_value = (int)item->u.number;
    debug_return_bool(true);
}

static bool
json_store_iolog_file(struct json_item *item, struct eventlog *evlog)
{
    debug_decl(json_store_iolog_file, SUDO_DEBUG_UTIL);

    /* Do set evlog->iolog_file directly, it is a substring of iolog_path. */
    free(iolog_file);
    iolog_file = item->u.string;
    item->u.string = NULL;
    debug_return_bool(true);
}

static bool
json_store_iolog_path(struct json_item *item, struct eventlog *evlog)
{
    debug_decl(json_store_iolog_path, SUDO_DEBUG_UTIL);

    free(evlog->iolog_path);
    evlog->iolog_path = item->u.string;
    item->u.string = NULL;
    debug_return_bool(true);
}

static bool
json_store_lines(struct json_item *item, struct eventlog *evlog)
{
    debug_decl(json_store_lines, SUDO_DEBUG_UTIL);

    if (item->u.number < 1 || item->u.number > INT_MAX) {
	sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
	    "tty lines %lld: out of range", item->u.number);
	evlog->lines = 0;
	debug_return_bool(false);
    }

    evlog->lines = (int)item->u.number;
    debug_return_bool(true);
}

static bool
json_store_peeraddr(struct json_item *item, struct eventlog *evlog)
{
    debug_decl(json_store_peeraddr, SUDO_DEBUG_UTIL);

    free(evlog->peeraddr);
    evlog->peeraddr = item->u.string;
    item->u.string = NULL;
    debug_return_bool(true);
}

static char **
json_array_to_strvec(struct eventlog_json_object *array)
{
    struct json_item *item;
    size_t len = 0;
    char **ret;
    debug_decl(json_array_to_strvec, SUDO_DEBUG_UTIL);

    TAILQ_FOREACH(item, &array->items, entries) {
	/* Can only convert arrays of string. */
	if (item->type != JSON_STRING) {
	    sudo_warnx(U_("expected JSON_STRING, got %d"), item->type);
	    debug_return_ptr(NULL);
	}
	/* Prevent integer overflow. */
	if (++len == INT_MAX) {
	    sudo_warnx("%s", U_("JSON_ARRAY too large"));
	    debug_return_ptr(NULL);
	}
    }
    if ((ret = reallocarray(NULL, len + 1, sizeof(char *))) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_ptr(NULL);
    }
    len = 0;
    TAILQ_FOREACH(item, &array->items, entries) {
	ret[len++] = item->u.string;
	item->u.string = NULL;
    }
    ret[len] = NULL;

    debug_return_ptr(ret);
}

static bool
json_store_submitenv(struct json_item *item, struct eventlog *evlog)
{
    size_t i;
    debug_decl(json_store_submitenv, SUDO_DEBUG_UTIL);

    if (evlog->submitenv != NULL) {
	for (i = 0; evlog->submitenv[i] != NULL; i++)
	    free(evlog->submitenv[i]);
	free(evlog->submitenv);
    }
    evlog->submitenv = json_array_to_strvec(&item->u.child);

    debug_return_bool(evlog->submitenv != NULL);
}

static bool
json_store_runargv(struct json_item *item, struct eventlog *evlog)
{
    size_t i;
    debug_decl(json_store_runargv, SUDO_DEBUG_UTIL);

    if (evlog->runargv != NULL) {
	for (i = 0; evlog->runargv[i] != NULL; i++)
	    free(evlog->runargv[i]);
	free(evlog->runargv);
    }
    evlog->runargv = json_array_to_strvec(&item->u.child);

    debug_return_bool(evlog->runargv != NULL);
}

static bool
json_store_runenv(struct json_item *item, struct eventlog *evlog)
{
    size_t i;
    debug_decl(json_store_runenv, SUDO_DEBUG_UTIL);

    if (evlog->runenv != NULL) {
	for (i = 0; evlog->runenv[i] != NULL; i++)
	    free(evlog->runenv[i]);
	free(evlog->runenv);
    }
    evlog->runenv = json_array_to_strvec(&item->u.child);

    debug_return_bool(evlog->runenv != NULL);
}

static bool
json_store_runenv_override(struct json_item *item, struct eventlog *evlog)
{
    size_t i;
    debug_decl(json_store_runenv_override, SUDO_DEBUG_UTIL);

    if (evlog->env_add != NULL) {
	for (i = 0; evlog->env_add[i] != NULL; i++)
	    free(evlog->env_add[i]);
	free(evlog->env_add);
    }
    evlog->env_add = json_array_to_strvec(&item->u.child);

    debug_return_bool(evlog->env_add != NULL);
}

static bool
json_store_rungid(struct json_item *item, struct eventlog *evlog)
{
    debug_decl(json_store_rungid, SUDO_DEBUG_UTIL);

    evlog->rungid = (gid_t)item->u.number;
    debug_return_bool(true);
}

static bool
json_store_rungroup(struct json_item *item, struct eventlog *evlog)
{
    debug_decl(json_store_rungroup, SUDO_DEBUG_UTIL);

    free(evlog->rungroup);
    evlog->rungroup = item->u.string;
    item->u.string = NULL;
    debug_return_bool(true);
}

static bool
json_store_runuid(struct json_item *item, struct eventlog *evlog)
{
    debug_decl(json_store_runuid, SUDO_DEBUG_UTIL);

    evlog->runuid = (uid_t)item->u.number;
    debug_return_bool(true);
}

static bool
json_store_runuser(struct json_item *item, struct eventlog *evlog)
{
    debug_decl(json_store_runuser, SUDO_DEBUG_UTIL);

    free(evlog->runuser);
    evlog->runuser = item->u.string;
    item->u.string = NULL;
    debug_return_bool(true);
}

static bool
json_store_runchroot(struct json_item *item, struct eventlog *evlog)
{
    debug_decl(json_store_runchroot, SUDO_DEBUG_UTIL);

    free(evlog->runchroot);
    evlog->runchroot = item->u.string;
    item->u.string = NULL;
    debug_return_bool(true);
}

static bool
json_store_runcwd(struct json_item *item, struct eventlog *evlog)
{
    debug_decl(json_store_runcwd, SUDO_DEBUG_UTIL);

    free(evlog->runcwd);
    evlog->runcwd = item->u.string;
    item->u.string = NULL;
    debug_return_bool(true);
}

static bool
json_store_signal(struct json_item *item, struct eventlog *evlog)
{
    debug_decl(json_store_signal, SUDO_DEBUG_UTIL);

    free(evlog->signal_name);
    evlog->signal_name = item->u.string;
    item->u.string = NULL;
    debug_return_bool(true);
}

static bool
json_store_source(struct json_item *item, struct eventlog *evlog)
{
    debug_decl(json_store_source, SUDO_DEBUG_UTIL);

    free(evlog->source);
    evlog->source = item->u.string;
    item->u.string = NULL;
    debug_return_bool(true);
}

static bool
json_store_submitcwd(struct json_item *item, struct eventlog *evlog)
{
    debug_decl(json_store_submitcwd, SUDO_DEBUG_UTIL);

    free(evlog->cwd);
    evlog->cwd = item->u.string;
    item->u.string = NULL;
    debug_return_bool(true);
}

static bool
json_store_submithost(struct json_item *item, struct eventlog *evlog)
{
    debug_decl(json_store_submithost, SUDO_DEBUG_UTIL);

    free(evlog->submithost);
    evlog->submithost = item->u.string;
    item->u.string = NULL;
    debug_return_bool(true);
}

static bool
json_store_submituser(struct json_item *item, struct eventlog *evlog)
{
    debug_decl(json_store_submituser, SUDO_DEBUG_UTIL);

    free(evlog->submituser);
    evlog->submituser = item->u.string;
    item->u.string = NULL;
    debug_return_bool(true);
}

static bool
json_store_submitgroup(struct json_item *item, struct eventlog *evlog)
{
    debug_decl(json_store_submitgroup, SUDO_DEBUG_UTIL);

    free(evlog->submitgroup);
    evlog->submitgroup = item->u.string;
    item->u.string = NULL;
    debug_return_bool(true);
}

static bool
json_store_timespec(struct json_item *item, struct timespec *ts)
{
    struct eventlog_json_object *object;
    debug_decl(json_store_timespec, SUDO_DEBUG_UTIL);

    object = &item->u.child;
    TAILQ_FOREACH(item, &object->items, entries) {
	if (item->type != JSON_NUMBER)
	    continue;
	if (strcmp(item->name, "seconds") == 0) {
	    ts->tv_sec = (time_t)item->u.number;
	    continue;
	}
	if (strcmp(item->name, "nanoseconds") == 0) {
	    ts->tv_nsec = (long)item->u.number;
	    continue;
	}
    }
    debug_return_bool(true);
}

static bool
json_store_iolog_offset(struct json_item *item, struct eventlog *evlog)
{
    return json_store_timespec(item, &evlog->iolog_offset);
}

static bool
json_store_run_time(struct json_item *item, struct eventlog *evlog)
{
    return json_store_timespec(item, &evlog->run_time);
}

static bool
json_store_timestamp(struct json_item *item, struct eventlog *evlog)
{
    return json_store_timespec(item, &evlog->submit_time);
}

static bool
json_store_ttyname(struct json_item *item, struct eventlog *evlog)
{
    debug_decl(json_store_ttyname, SUDO_DEBUG_UTIL);

    free(evlog->ttyname);
    evlog->ttyname = item->u.string;
    item->u.string = NULL;
    debug_return_bool(true);
}

static bool
json_store_uuid(struct json_item *item, struct eventlog *evlog)
{
    bool ret = false;
    debug_decl(json_store_uuid, SUDO_DEBUG_UTIL);

    if (strlen(item->u.string) == sizeof(evlog->uuid_str) - 1) {
	memcpy(evlog->uuid_str, item->u.string, sizeof(evlog->uuid_str));
	ret = true;
    }
    free(item->u.string);
    item->u.string = NULL;
    debug_return_bool(ret);
}

static struct evlog_json_key {
    const char *name;
    enum json_value_type type;
    bool (*setter)(struct json_item *, struct eventlog *);
} evlog_json_keys[] = {
    { "columns", JSON_NUMBER, json_store_columns },
    { "command", JSON_STRING, json_store_command },
    { "dumped_core", JSON_BOOL, json_store_dumped_core },
    { "exit_value", JSON_NUMBER, json_store_exit_value },
    { "iolog_file", JSON_STRING, json_store_iolog_file },
    { "iolog_path", JSON_STRING, json_store_iolog_path },
    { "iolog_offset", JSON_OBJECT, json_store_iolog_offset },
    { "lines", JSON_NUMBER, json_store_lines },
    { "peeraddr", JSON_STRING, json_store_peeraddr },
    { "run_time", JSON_OBJECT, json_store_run_time },
    { "runargv", JSON_ARRAY, json_store_runargv },
    { "runenv", JSON_ARRAY, json_store_runenv },
    { "runenv_override", JSON_ARRAY, json_store_runenv_override },
    { "rungid", JSON_ID, json_store_rungid },
    { "rungroup", JSON_STRING, json_store_rungroup },
    { "runuid", JSON_ID, json_store_runuid },
    { "runuser", JSON_STRING, json_store_runuser },
    { "runchroot", JSON_STRING, json_store_runchroot },
    { "runcwd", JSON_STRING, json_store_runcwd },
    { "source", JSON_STRING, json_store_source },
    { "signal", JSON_STRING, json_store_signal },
    { "submitcwd", JSON_STRING, json_store_submitcwd },
    { "submitenv", JSON_ARRAY, json_store_submitenv },
    { "submithost", JSON_STRING, json_store_submithost },
    { "submitgroup", JSON_STRING, json_store_submitgroup },
    { "submituser", JSON_STRING, json_store_submituser },
    { "timestamp", JSON_OBJECT, json_store_timestamp },
    { "ttyname", JSON_STRING, json_store_ttyname },
    { "uuid", JSON_STRING, json_store_uuid },
    { NULL }
};

static struct json_item *
new_json_item(enum json_value_type type, char *name, unsigned int lineno)
{
    struct json_item *item;
    debug_decl(new_json_item, SUDO_DEBUG_UTIL);

    if ((item = malloc(sizeof(*item))) == NULL)  {
	sudo_warnx(U_("%s: %s"), __func__,
	    U_("unable to allocate memory"));
	debug_return_ptr(NULL);
    }
    item->name = name;
    item->type = type;
    item->lineno = lineno;

    debug_return_ptr(item);
}

static char *
json_parse_string(char **strp)
{
    char *dst, *end, *ret, *src = *strp + 1;
    size_t len;
    debug_decl(json_parse_string, SUDO_DEBUG_UTIL);

    for (end = src; *end != '"' && *end != '\0'; end++) {
	if (end[0] == '\\' && end[1] == '"')
	    end++;
    }
    if (*end != '"') {
	sudo_warnx("%s", U_("missing double quote in name"));
	debug_return_str(NULL);
    }
    len = (size_t)(end - src);

    /* Copy string, flattening escaped chars. */
    dst = ret = malloc(len + 1);
    if (dst == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_str(NULL);
    }
    while (src < end) {
	int ch = *src++;
	if (ch == '\\') {
	    switch (*src) {
	    case 'b':
		ch = '\b';
		break;
	    case 'f':
		ch = '\f';
		break;
	    case 'n':
		ch = '\n';
		break;
	    case 'r':
		ch = '\r';
		break;
	    case 't':
		ch = '\t';
		break;
	    case 'u':
		/* Only currently handles 8-bit ASCII. */
		if (src[1] == '0' && src[2] == '0') {
		    ch = sudo_hexchar(&src[3]);
		    if (ch != -1) {
			src += 4;
			break;
		    }
		}
		/* Not in \u00XX format. */
		FALLTHROUGH;
	    case '"':
	    case '\\':
	    default:
		/* Note: a bare \ at the end of a string will be removed. */
		ch = *src;
		break;
	    }
	    src++;
	}
	*dst++ = (char)ch;
    }
    *dst = '\0';

    /* Trim trailing whitespace. */
    do {
	end++;
    } while (isspace((unsigned char)*end));
    *strp = end;

    debug_return_str(ret);
}

static void
free_json_items(struct json_item_list *items)
{
    struct json_item *item;
    debug_decl(free_json_items, SUDO_DEBUG_UTIL);

    while ((item = TAILQ_FIRST(items)) != NULL) {
	TAILQ_REMOVE(items, item, entries);
	switch (item->type) {
	case JSON_STRING:
	    free(item->u.string);
	    break;
	case JSON_ARRAY:
	case JSON_OBJECT:
	    free_json_items(&item->u.child.items);
	    break;
	case JSON_ID:
	case JSON_NUMBER:
	case JSON_BOOL:
	case JSON_NULL:
	    /* Nothing to free. */
	    break;
	default:
	    sudo_warnx("%s: internal error, invalid JSON type %d",
		__func__, item->type);
	    break;
	}
	free(item->name);
	free(item);
    }

    debug_return;
}

void
eventlog_json_free(struct eventlog_json_object *root)
{
    debug_decl(eventlog_json_free, SUDO_DEBUG_UTIL);
    if (root != NULL) {
	free_json_items(&root->items);
	free(root);
    }
    debug_return;
}

bool
eventlog_json_parse(struct eventlog_json_object *object, struct eventlog *evlog)
{
    struct json_item *item;
    bool ret = false;
    debug_decl(eventlog_json_parse, SUDO_DEBUG_UTIL);

    /* First object holds all the actual data. */
    item = TAILQ_FIRST(&object->items);
    if (item == NULL) {
	sudo_warnx("%s", U_("missing JSON_OBJECT"));
	goto done;
    }
    if (item->type != JSON_OBJECT) {
	sudo_warnx(U_("expected JSON_OBJECT, got %d"), item->type);
	goto done;
    }
    object = &item->u.child;

    TAILQ_FOREACH(item, &object->items, entries) {
	struct evlog_json_key *key;

	/* expecting key:value pairs */
	if (item->name == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
		"%s: missing object name", __func__);
	    goto done;
	}

	/* lookup name */
	for (key = evlog_json_keys; key->name != NULL; key++) {
	    if (strcmp(item->name, key->name) == 0)
		break;
	}
	if (key->name == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
		"%s: unknown key %s", __func__, item->name);
	} else if (key->type != item->type &&
		(key->type != JSON_ID || item->type != JSON_NUMBER)) {
	    sudo_debug_printf(SUDO_DEBUG_WARN|SUDO_DEBUG_LINENO,
		"%s: key mismatch %s type %d, expected %d", __func__,
		item->name, item->type, key->type);
	    goto done;
	} else {
	    /* Matched name and type. */
	    if (!key->setter(item, evlog)) {
		sudo_debug_printf(SUDO_DEBUG_ERROR|SUDO_DEBUG_LINENO,
		    "unable to store %s", key->name);
		goto done;
	    }
	}
    }

    /*
     * iolog_file must be a substring of iolog_path.
     */
    if (iolog_file != NULL && evlog->iolog_path != NULL) {
	const size_t filelen = strlen(iolog_file);
	const size_t pathlen = strlen(evlog->iolog_path);
	if (filelen <= pathlen) {
	    const char *cp = &evlog->iolog_path[pathlen - filelen];
	    if (strcmp(cp, iolog_file) == 0) {
		evlog->iolog_file = cp;
	    }
	}
    }

    ret = true;

done:
    free(iolog_file);
    iolog_file = NULL;

    debug_return_bool(ret);
}

static bool
json_insert_bool(struct json_item_list *items, char *name, bool value,
    unsigned int lineno)
{
    struct json_item *item;
    debug_decl(json_insert_bool, SUDO_DEBUG_UTIL);

    if ((item = new_json_item(JSON_BOOL, name, lineno)) == NULL)
	debug_return_bool(false);
    item->u.boolean = value;
    TAILQ_INSERT_TAIL(items, item, entries);

    debug_return_bool(true);
}

static bool
json_insert_null(struct json_item_list *items, char *name, unsigned int lineno)
{
    struct json_item *item;
    debug_decl(json_insert_null, SUDO_DEBUG_UTIL);

    if ((item = new_json_item(JSON_NULL, name, lineno)) == NULL)
	debug_return_bool(false);
    TAILQ_INSERT_TAIL(items, item, entries);

    debug_return_bool(true);
}

static bool
json_insert_num(struct json_item_list *items, char *name, long long value,
    unsigned int lineno)
{
    struct json_item *item;
    debug_decl(json_insert_num, SUDO_DEBUG_UTIL);

    if ((item = new_json_item(JSON_NUMBER, name, lineno)) == NULL)
	debug_return_bool(false);
    item->u.number = value;
    TAILQ_INSERT_TAIL(items, item, entries);

    debug_return_bool(true);
}

static bool
json_insert_str(struct json_item_list *items, char *name, char **strp,
    unsigned int lineno)
{
    struct json_item *item;
    debug_decl(json_insert_str, SUDO_DEBUG_UTIL);

    if ((item = new_json_item(JSON_STRING, name, lineno)) == NULL)
	debug_return_bool(false);
    item->u.string = json_parse_string(strp);
    if (item->u.string == NULL) {
	free(item);
	debug_return_bool(false);
    }
    TAILQ_INSERT_TAIL(items, item, entries);

    debug_return_bool(true);
}

static struct eventlog_json_object *
json_stack_push(struct json_stack *stack, struct json_item_list *items,
    struct eventlog_json_object *frame, enum json_value_type type, char *name,
    unsigned int lineno)
{
    struct json_item *item;
    debug_decl(json_stack_push, SUDO_DEBUG_UTIL);

    /* We limit the stack size rather than expanding it. */
    if (stack->depth >= stack->maxdepth) {
	sudo_warnx(U_("json stack exhausted (max %u frames)"), stack->maxdepth);
	debug_return_ptr(NULL);
    }

    /* Allocate a new item and insert it into the list. */
    if ((item = new_json_item(type, name, lineno)) == NULL)
	debug_return_ptr(NULL);
    TAILQ_INIT(&item->u.child.items);
    item->u.child.parent = item;
    TAILQ_INSERT_TAIL(items, item, entries);

    /* Push the current frame onto the stack (depth check performed above). */
    stack->frames[stack->depth++] = frame;

    /* Return the new frame */
    debug_return_ptr(&item->u.child);
}

/* Only expect a value if a name is defined or we are in an array. */
#define expect_value (name != NULL || (frame->parent != NULL && frame->parent->type == JSON_ARRAY))

struct eventlog_json_object *
eventlog_json_read(FILE *fp, const char *filename)
{
    struct eventlog_json_object *frame, *root;
    struct json_stack stack = JSON_STACK_INTIALIZER(stack);
    unsigned int lineno = 0;
    char *name = NULL;
    char *cp, *line = NULL;
    size_t len, linesize = 0;
    ssize_t linelen;
    bool saw_comma = false;
    long long num;
    char ch;
    debug_decl(eventlog_json_read, SUDO_DEBUG_UTIL);

    root = malloc(sizeof(*root));
    if (root == NULL)
	goto bad;

    root->parent = NULL;
    TAILQ_INIT(&root->items);

    frame = root;
    while ((linelen = getdelim(&line, &linesize, '\n', fp)) != -1) {
	char *ep = line + linelen - 1;
	cp = line;

	lineno++;

	/* Trim trailing whitespace. */
	while (ep > cp && isspace((unsigned char)*ep))
	    ep--;
	ep[1] = '\0';

	for (;;) {
	    const char *errstr;

	    /* Trim leading whitespace, skip blank lines. */
	    while (isspace((unsigned char)*cp))
		cp++;

	    /* Check for comma separator and strip it out. */
	    if (*cp == ',') {
		saw_comma = true;
		cp++;
		while (isspace((unsigned char)*cp))
		    cp++;
	    }

	    /* End of line? */
	    if (*cp == '\0')
		break;

	    switch (*cp) {
	    case '{':
		if (name == NULL && frame->parent != NULL) {
		    sudo_warnx("%s:%u:%td: %s", filename, lineno, cp - line, 
			U_("objects must consist of name:value pairs"));
		    goto bad;
		}
		if (!saw_comma && !TAILQ_EMPTY(&frame->items)) {
		    sudo_warnx("%s:%u:%td: %s", filename, lineno, cp - line, 
			U_("missing separator between values"));
		    goto bad;
		}
		cp++;
		saw_comma = false;
		frame = json_stack_push(&stack, &frame->items, frame,
		    JSON_OBJECT, name, lineno);
		if (frame == NULL)
		    goto bad;
		name = NULL;
		break;
	    case '}':
		if (stack.depth == 0 || frame->parent == NULL ||
			frame->parent->type != JSON_OBJECT) {
		    sudo_warnx("%s:%u:%td: %s", filename, lineno, cp - line, 
			U_("unmatched close brace"));
		    goto bad;
		}
		cp++;
		frame = stack.frames[--stack.depth];
		saw_comma = false;
		break;
	    case '[':
		if (frame->parent == NULL) {
		    /* Must have an enclosing object. */
		    sudo_warnx("%s:%u:%td: %s", filename, lineno, cp - line, 
			U_("unexpected array"));
		    goto bad;
		}
		if (!saw_comma && !TAILQ_EMPTY(&frame->items)) {
		    sudo_warnx("%s:%u:%td: %s", filename, lineno, cp - line, 
			U_("missing separator between values"));
		    goto bad;
		}
		cp++;
		saw_comma = false;
		frame = json_stack_push(&stack, &frame->items, frame,
		    JSON_ARRAY, name, lineno);
		if (frame == NULL)
		    goto bad;
		name = NULL;
		break;
	    case ']':
		if (stack.depth == 0 || frame->parent == NULL ||
			frame->parent->type != JSON_ARRAY) {
		    sudo_warnx("%s:%u:%td: %s", filename, lineno, cp - line, 
			U_("unmatched close bracket"));
		    goto bad;
		}
		cp++;
		frame = stack.frames[--stack.depth];
		saw_comma = false;
		break;
	    case '"':
		if (frame->parent == NULL) {
		    /* Must have an enclosing object. */
		    sudo_warnx("%s:%u:%td: %s", filename, lineno, cp - line, 
			U_("unexpected string"));
		    goto bad;
		}

		if (!expect_value) {
		    /* Parse "name": */
		    if ((name = json_parse_string(&cp)) == NULL)
			goto bad;
		    /* TODO: allow colon on next line? */
		    if (*cp != ':') {
			sudo_warnx("%s:%u:%td: %s", filename, lineno, cp - line, 
			    U_("missing colon after name"));
			goto bad;
		    }
		    cp++;
		} else {
		    if (!saw_comma && !TAILQ_EMPTY(&frame->items)) {
			sudo_warnx("%s:%u:%td: %s", filename, lineno, cp - line, 
			    U_("missing separator between values"));
			goto bad;
		    }
		    saw_comma = false;
		    if (!json_insert_str(&frame->items, name, &cp, lineno))
			goto bad;
		    name = NULL;
		}
		break;
	    case 't':
		if (strncmp(cp, "true", sizeof("true") - 1) != 0)
		    goto parse_error;
		if (!expect_value) {
		    sudo_warnx("%s:%u:%td: %s", filename, lineno, cp - line, 
			U_("unexpected boolean"));
		    goto bad;
		}
		cp += sizeof("true") - 1;
		if (*cp != ',' && !isspace((unsigned char)*cp) && *cp != '\0')
		    goto parse_error;
		if (!saw_comma && !TAILQ_EMPTY(&frame->items)) {
		    sudo_warnx("%s:%u:%td: %s", filename, lineno, cp - line, 
			U_("missing separator between values"));
		    goto bad;
		}
		saw_comma = false;

		if (!json_insert_bool(&frame->items, name, true, lineno))
		    goto bad;
		name = NULL;
		break;
	    case 'f':
		if (strncmp(cp, "false", sizeof("false") - 1) != 0)
		    goto parse_error;
		if (!expect_value) {
		    sudo_warnx("%s:%u:%td: %s", filename, lineno, cp - line, 
			U_("unexpected boolean"));
		    goto bad;
		}
		cp += sizeof("false") - 1;
		if (*cp != ',' && !isspace((unsigned char)*cp) && *cp != '\0')
		    goto parse_error;
		if (!saw_comma && !TAILQ_EMPTY(&frame->items)) {
		    sudo_warnx("%s:%u:%td: %s", filename, lineno, cp - line, 
			U_("missing separator between values"));
		    goto bad;
		}
		saw_comma = false;

		if (!json_insert_bool(&frame->items, name, false, lineno))
		    goto bad;
		name = NULL;
		break;
	    case 'n':
		if (strncmp(cp, "null", sizeof("null") - 1) != 0)
		    goto parse_error;
		if (!expect_value) {
		    sudo_warnx("%s:%u:%td: %s", filename, lineno, cp - line, 
			U_("unexpected null"));
		    goto bad;
		}
		cp += sizeof("null") - 1;
		if (*cp != ',' && !isspace((unsigned char)*cp) && *cp != '\0')
		    goto parse_error;
		if (!saw_comma && !TAILQ_EMPTY(&frame->items)) {
		    sudo_warnx("%s:%u:%td: %s", filename, lineno, cp - line, 
			U_("missing separator between values"));
		    goto bad;
		}
		saw_comma = false;

		if (!json_insert_null(&frame->items, name, lineno))
		    goto bad;
		name = NULL;
		break;
	    case '+': case '-': case '0': case '1': case '2': case '3':
	    case '4': case '5': case '6': case '7': case '8': case '9':
		if (!expect_value) {
		    sudo_warnx("%s:%u:%td: %s", filename, lineno, cp - line, 
			U_("unexpected number"));
		    goto bad;
		}
		/* XXX - strtonumx() would be simpler here. */
		len = strcspn(cp, " \f\n\r\t\v,");
		ch = cp[len];
		cp[len] = '\0';
		if (!saw_comma && !TAILQ_EMPTY(&frame->items)) {
		    sudo_warnx("%s:%u:%td: %s", filename, lineno, cp - line, 
			U_("missing separator between values"));
		    goto bad;
		}
		saw_comma = false;
		num = sudo_strtonum(cp, LLONG_MIN, LLONG_MAX, &errstr);
		if (errstr != NULL) {
		    sudo_warnx("%s:%u:%td: %s: %s", filename, lineno, cp - line,
			cp, U_(errstr));
		    goto bad;
		}
		cp += len;
		*cp = ch;

		if (!json_insert_num(&frame->items, name, num, lineno))
		    goto bad;
		name = NULL;
		break;
	    default:
		goto parse_error;
	    }
	}
    }
    if (stack.depth != 0) {
	frame = stack.frames[stack.depth - 1];
	if (frame->parent == NULL || frame->parent->type == JSON_OBJECT) {
	    sudo_warnx("%s:%u:%td: %s", filename, lineno, cp - line,
		U_("unmatched close brace"));
	} else {
	    sudo_warnx("%s:%u:%td: %s", filename, lineno, cp - line,
		U_("unmatched close bracket"));
	}
	goto bad;
    }

    goto done;

parse_error:
    sudo_warnx("%s:%u:%td: %s", filename, lineno, cp - line, U_("parse error"));
bad:
    eventlog_json_free(root);
    root = NULL;
done:
    free(line);
    free(name);

    debug_return_ptr(root);
}
