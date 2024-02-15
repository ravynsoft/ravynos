/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2009-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# include <compat/stdbool.h>
#endif
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif /* HAVE_STRINGS_H */
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#ifdef HAVE_DLOPEN
# include <dlfcn.h>
#endif

#define SUDO_ERROR_WRAP	0

#include <sudo_compat.h>
#include <sudo_conf.h>
#include <sudo_debug.h>
#include <sudo_fatal.h>
#include <sudo_gettext.h>
#include <sudo_plugin.h>
#include <sudo_util.h>
#include <pathnames.h>

#ifndef _PATH_SUDO_INTERCEPT
# define _PATH_SUDO_INTERCEPT NULL
#endif
#ifndef _PATH_SUDO_NOEXEC
# define _PATH_SUDO_NOEXEC NULL
#endif

struct sudo_conf_table {
    const char *name;
    unsigned int namelen;
    int (*parser)(const char *entry, const char *conf_file, unsigned int lineno);
};

struct sudo_conf_path_table {
    const char *pname;
    unsigned int pnamelen;
    bool dynamic;
    const char *pval;
};

struct sudo_conf_settings {
    bool updated;
    bool disable_coredump;
    bool probe_interfaces;
    int group_source;
    int max_groups;
};

static int parse_debug(const char *entry, const char *conf_file, unsigned int lineno);
static int parse_path(const char *entry, const char *conf_file, unsigned int lineno);
static int parse_plugin(const char *entry, const char *conf_file, unsigned int lineno);
static int parse_variable(const char *entry, const char *conf_file, unsigned int lineno);

static struct sudo_conf_table sudo_conf_table[] = {
    { "Debug", sizeof("Debug") - 1, parse_debug },
    { "Path", sizeof("Path") - 1, parse_path },
    { "Plugin", sizeof("Plugin") - 1, parse_plugin },
    { "Set", sizeof("Set") - 1, parse_variable },
    { NULL }
};

static int set_var_disable_coredump(const char *entry, const char *conf_file, unsigned int);
static int set_var_group_source(const char *entry, const char *conf_file, unsigned int);
static int set_var_max_groups(const char *entry, const char *conf_file, unsigned int);
static int set_var_probe_interfaces(const char *entry, const char *conf_file, unsigned int);

static struct sudo_conf_table sudo_conf_var_table[] = {
    { "disable_coredump", sizeof("disable_coredump") - 1, set_var_disable_coredump },
    { "group_source", sizeof("group_source") - 1, set_var_group_source },
    { "max_groups", sizeof("max_groups") - 1, set_var_max_groups },
    { "probe_interfaces", sizeof("probe_interfaces") - 1, set_var_probe_interfaces },
    { NULL }
};

/* Indexes into path_table[] below (order is important). */
#define SUDO_CONF_PATH_ASKPASS		0
#define SUDO_CONF_PATH_SESH		1
#define SUDO_CONF_PATH_INTERCEPT	2
#define SUDO_CONF_PATH_NOEXEC		3
#define SUDO_CONF_PATH_PLUGIN_DIR	4
#define SUDO_CONF_PATH_DEVSEARCH	5

#define SUDO_CONF_PATH_INITIALIZER	{				\
    { "askpass", sizeof("askpass") - 1, false, _PATH_SUDO_ASKPASS },	\
    { "sesh", sizeof("sesh") - 1, false, _PATH_SUDO_SESH },		\
    { "intercept", sizeof("intercept") - 1, false, _PATH_SUDO_INTERCEPT },	\
    { "noexec", sizeof("noexec") - 1, false, _PATH_SUDO_NOEXEC },	\
    { "plugin_dir", sizeof("plugin_dir") - 1, false, _PATH_SUDO_PLUGIN_DIR }, \
    { "devsearch", sizeof("devsearch") - 1, false, _PATH_SUDO_DEVSEARCH }, \
    { NULL } \
}

/*
 * getgroups(2) on macOS is flakey with respect to non-local groups. 
 * Even with _DARWIN_UNLIMITED_GETGROUPS set we may not get all groups./
 * See bug #946 for details.
 */
#ifdef __APPLE__
# define GROUP_SOURCE_DEFAULT	GROUP_SOURCE_DYNAMIC
#else
# define GROUP_SOURCE_DEFAULT	GROUP_SOURCE_ADAPTIVE
#endif

#define SUDO_CONF_SETTINGS_INITIALIZER	{				\
    false,			/* updated */				\
    true,			/* disable_coredump */			\
    true,			/* probe_interfaces */			\
    GROUP_SOURCE_DEFAULT,	/* group_source */			\
    -1				/* max_groups */			\
}

static struct sudo_conf_data {
    struct sudo_conf_settings settings;
    struct sudo_conf_debug_list debugging;
    struct plugin_info_list plugins;
    struct sudo_conf_path_table path_table[7];
} sudo_conf_data = {
    SUDO_CONF_SETTINGS_INITIALIZER,
    TAILQ_HEAD_INITIALIZER(sudo_conf_data.debugging),
    TAILQ_HEAD_INITIALIZER(sudo_conf_data.plugins),
    SUDO_CONF_PATH_INITIALIZER
};

/*
 * "Set variable_name value"
 */
static int
parse_variable(const char *entry, const char *conf_file, unsigned int lineno)
{
    struct sudo_conf_table *var;
    int ret;
    debug_decl(parse_variable, SUDO_DEBUG_UTIL);

    for (var = sudo_conf_var_table; var->name != NULL; var++) {
	if (strncmp(entry, var->name, var->namelen) == 0 &&
	    isblank((unsigned char)entry[var->namelen])) {
	    entry += var->namelen + 1;
	    while (isblank((unsigned char)*entry))
		entry++;
	    ret = var->parser(entry, conf_file, lineno);
	    sudo_debug_printf(ret ? SUDO_DEBUG_INFO : SUDO_DEBUG_ERROR,
		"%s: %s:%u: Set %s %s", __func__, conf_file,
		lineno, var->name, entry);
	    debug_return_int(ret);
	}
    }
    sudo_debug_printf(SUDO_DEBUG_WARN, "%s: %s:%u: unknown setting %s",
	__func__, conf_file, lineno, entry);
    debug_return_int(false);
}

/*
 * "Path name /path/to/file"
 * If path is missing it will be set to the NULL pointer.
 */
static int
parse_path(const char *entry, const char *conf_file, unsigned int lineno)
{
    const char *entry_end = entry + strlen(entry);
    const char *ep, *name, *path;
    struct sudo_conf_path_table *cur;
    size_t namelen;
    debug_decl(parse_path, SUDO_DEBUG_UTIL);

    /* Parse name. */
    name = sudo_strsplit(entry, entry_end, " \t", &ep);
    if (name == NULL)
	goto bad;
    namelen = (size_t)(ep - name);

    /* Parse path (if present). */
    path = sudo_strsplit(NULL, entry_end, " \t", &ep);

    /* Match supported paths, ignoring unknown paths. */
    for (cur = sudo_conf_data.path_table; cur->pname != NULL; cur++) {
	if (namelen == cur->pnamelen &&
	    strncasecmp(name, cur->pname, cur->pnamelen) == 0) {
	    char *pval = NULL;
	    if (path != NULL) {
		if ((pval = strdup(path)) == NULL) {
		    sudo_warnx(U_("%s: %s"), __func__,
			U_("unable to allocate memory"));
		    debug_return_int(-1);
		}
	    }
	    if (cur->dynamic)
		free((char *)cur->pval);
	    cur->pval = pval;
	    cur->dynamic = true;
	    sudo_debug_printf(SUDO_DEBUG_INFO, "%s: %s:%u: Path %s %s",
		__func__, conf_file, lineno, cur->pname,
		pval ? pval : "(none)");
	    debug_return_int(true);
	}
    }
    sudo_debug_printf(SUDO_DEBUG_WARN, "%s: %s:%u: unknown path %s",
	__func__, conf_file, lineno, entry);
    debug_return_int(false);
bad:
    sudo_warnx(U_("invalid Path value \"%s\" in %s, line %u"),
	entry, conf_file, lineno);
    debug_return_int(false);
}

/*
 * "Debug program /path/to/log flags,..."
 */
static int
parse_debug(const char *entry, const char *conf_file, unsigned int lineno)
{
    struct sudo_conf_debug *debug_spec;
    struct sudo_debug_file *debug_file = NULL;
    const char *ep, *path, *progname, *flags;
    const char *entry_end = entry + strlen(entry);
    size_t pathlen, prognamelen;
    debug_decl(parse_debug, SUDO_DEBUG_UTIL);

    /* Parse progname. */
    progname = sudo_strsplit(entry, entry_end, " \t", &ep);
    if (progname == NULL)
	debug_return_int(false);	/* not enough fields */
    prognamelen = (size_t)(ep - progname);

    /* Parse path. */
    path = sudo_strsplit(NULL, entry_end, " \t", &ep);
    if (path == NULL)
	debug_return_int(false);	/* not enough fields */
    pathlen = (size_t)(ep - path);

    /* Remainder is flags (freeform). */
    flags = sudo_strsplit(NULL, entry_end, " \t", &ep);
    if (flags == NULL)
	debug_return_int(false);	/* not enough fields */

    /* If progname already exists, use it, else alloc a new one. */
    TAILQ_FOREACH(debug_spec, &sudo_conf_data.debugging, entries) {
	if (strncmp(debug_spec->progname, progname, prognamelen) == 0 &&
	    debug_spec->progname[prognamelen] == '\0')
	    break;
    }
    if (debug_spec == NULL) {
	debug_spec = malloc(sizeof(*debug_spec));
	if (debug_spec == NULL)
	    goto oom;
	debug_spec->progname = strndup(progname, prognamelen);
	if (debug_spec->progname == NULL) {
	    free(debug_spec);
	    debug_spec = NULL;
	    goto oom;
	}
	TAILQ_INIT(&debug_spec->debug_files);
	TAILQ_INSERT_TAIL(&sudo_conf_data.debugging, debug_spec, entries);
    }
    debug_file = calloc(1, sizeof(*debug_file));
    if (debug_file == NULL)
	goto oom;
    debug_file->debug_file = strndup(path, pathlen);
    if (debug_file->debug_file == NULL)
	goto oom;
    debug_file->debug_flags = strdup(flags);
    if (debug_file->debug_flags == NULL)
	goto oom;
    TAILQ_INSERT_TAIL(&debug_spec->debug_files, debug_file, entries);

    debug_return_int(true);
oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    if (debug_file != NULL) {
	free(debug_file->debug_file);
	free(debug_file->debug_flags);
	free(debug_file);
    }
    debug_return_int(-1);
}

/*
 * "Plugin symbol /path/to/log args..."
 */
static int
parse_plugin(const char *entry, const char *conf_file, unsigned int lineno)
{
    struct plugin_info *info = NULL;
    const char *ep, *path, *symbol;
    const char *entry_end = entry + strlen(entry);
    char **options = NULL;
    size_t pathlen, symlen;
    unsigned int nopts = 0;
    debug_decl(parse_plugin, SUDO_DEBUG_UTIL);

    /* Parse symbol. */
    symbol = sudo_strsplit(entry, entry_end, " \t", &ep);
    if (symbol == NULL)
	debug_return_int(false);	/* not enough fields */
    symlen = (size_t)(ep - symbol);

    /* Parse path. */
    path = sudo_strsplit(NULL, entry_end, " \t", &ep);
    if (path == NULL)
	debug_return_int(false);	/* not enough fields */
    pathlen = (size_t)(ep - path);

    /* Split options into an array if present. */
    while (isblank((unsigned char)*ep))
	ep++;
    if (*ep != '\0') {
	/* Count number of options and allocate array. */
	const char *cp, *opt = ep;

	/* Count and allocate options array. */
	for (nopts = 0, cp = sudo_strsplit(opt, entry_end, " \t", &ep);
	    cp != NULL; cp = sudo_strsplit(NULL, entry_end, " \t", &ep)) {
	    nopts++;
	}
	options = reallocarray(NULL, nopts + 1, sizeof(*options));
	if (options == NULL)
	    goto oom;

	/* Fill in options array. */
	for (nopts = 0, cp = sudo_strsplit(opt, entry_end, " \t", &ep);
	    cp != NULL; cp = sudo_strsplit(NULL, entry_end, " \t", &ep)) {
	    options[nopts] = strndup(cp, (size_t)(ep - cp));
	    if (options[nopts] == NULL)
		goto oom;
	    nopts++;
	}
	options[nopts] = NULL;
    }

    info = calloc(1, sizeof(*info));
    if (info == NULL)
	    goto oom;
    info->symbol_name = strndup(symbol, symlen);
    if (info->symbol_name == NULL)
	    goto oom;
    info->path = strndup(path, pathlen);
    if (info->path == NULL)
	    goto oom;
    info->options = options;
    info->lineno = lineno;
    TAILQ_INSERT_TAIL(&sudo_conf_data.plugins, info, entries);

    debug_return_int(true);
oom:
    sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    if (options != NULL) {
	while (nopts)
	    free(options[--nopts]);
	free(options);
    }
    if (info != NULL) {
	free(info->symbol_name);
	free(info->path);
	free(info);
    }
    debug_return_int(-1);
}

static int
set_var_disable_coredump(const char *strval, const char *conf_file,
    unsigned int lineno)
{
    int val = sudo_strtobool(strval);
    debug_decl(set_var_disable_coredump, SUDO_DEBUG_UTIL);

    if (val == -1) {
	sudo_warnx(U_("invalid value for %s \"%s\" in %s, line %u"),
	    "disable_coredump", strval, conf_file, lineno);
	debug_return_int(false);
    }
    sudo_conf_data.settings.disable_coredump = val;
    debug_return_int(true);
}

static int
set_var_group_source(const char *strval, const char *conf_file,
    unsigned int lineno)
{
    debug_decl(set_var_group_source, SUDO_DEBUG_UTIL);

    if (strcasecmp(strval, "adaptive") == 0) {
	sudo_conf_data.settings.group_source = GROUP_SOURCE_ADAPTIVE;
    } else if (strcasecmp(strval, "static") == 0) {
	sudo_conf_data.settings.group_source = GROUP_SOURCE_STATIC;
    } else if (strcasecmp(strval, "dynamic") == 0) {
	sudo_conf_data.settings.group_source = GROUP_SOURCE_DYNAMIC;
    } else {
	sudo_warnx(U_("unsupported group source \"%s\" in %s, line %u"), strval,
	    conf_file, lineno);
	debug_return_int(false);
    }
    debug_return_int(true);
}

static int
set_var_max_groups(const char *strval, const char *conf_file,
    unsigned int lineno)
{
    int max_groups;
    debug_decl(set_var_max_groups, SUDO_DEBUG_UTIL);

    max_groups = (int)sudo_strtonum(strval, 1, 1024, NULL);
    if (max_groups <= 0) {
	sudo_warnx(U_("invalid max groups \"%s\" in %s, line %u"), strval,
	    conf_file, lineno);
	debug_return_int(false);
    }
    sudo_conf_data.settings.max_groups = max_groups;
    debug_return_int(true);
}

static int
set_var_probe_interfaces(const char *strval, const char *conf_file,
    unsigned int lineno)
{
    int val = sudo_strtobool(strval);
    debug_decl(set_var_probe_interfaces, SUDO_DEBUG_UTIL);

    if (val == -1) {
	sudo_warnx(U_("invalid value for %s \"%s\" in %s, line %u"),
	    "probe_interfaces", strval, conf_file, lineno);
	debug_return_int(false);
    }
    sudo_conf_data.settings.probe_interfaces = val;
    debug_return_int(true);
}

const char *
sudo_conf_askpass_path_v1(void)
{
    return sudo_conf_data.path_table[SUDO_CONF_PATH_ASKPASS].pval;
}

const char *
sudo_conf_sesh_path_v1(void)
{
    return sudo_conf_data.path_table[SUDO_CONF_PATH_SESH].pval;
}

const char *
sudo_conf_intercept_path_v1(void)
{
    return sudo_conf_data.path_table[SUDO_CONF_PATH_INTERCEPT].pval;
}

const char *
sudo_conf_noexec_path_v1(void)
{
    return sudo_conf_data.path_table[SUDO_CONF_PATH_NOEXEC].pval;
}

const char *
sudo_conf_plugin_dir_path_v1(void)
{
    return sudo_conf_data.path_table[SUDO_CONF_PATH_PLUGIN_DIR].pval;
}

const char *
sudo_conf_devsearch_path_v1(void)
{
    return sudo_conf_data.path_table[SUDO_CONF_PATH_DEVSEARCH].pval;
}

int
sudo_conf_group_source_v1(void)
{
    return sudo_conf_data.settings.group_source;
}

int
sudo_conf_max_groups_v1(void)
{
    return sudo_conf_data.settings.max_groups;
}

struct plugin_info_list *
sudo_conf_plugins_v1(void)
{
    return &sudo_conf_data.plugins;
}

struct sudo_conf_debug_list *
sudo_conf_debugging_v1(void)
{
    return &sudo_conf_data.debugging;
}

/* Return the debug files list for a program, or NULL if none. */
struct sudo_conf_debug_file_list *
sudo_conf_debug_files_v1(const char *progname)
{
    struct sudo_conf_debug *debug_spec;
    const char *progbase;
    debug_decl(sudo_conf_debug_files, SUDO_DEBUG_UTIL);

    /* Determine basename if program is fully qualified (like for plugins). */
    progbase = progname[0] == '/' ? sudo_basename(progname) : progname;

    /* Convert sudoedit -> sudo. */
    if (strcmp(progbase, "sudoedit") == 0)
	progbase = "sudo";

    TAILQ_FOREACH(debug_spec, &sudo_conf_data.debugging, entries) {
	const char *prog = progbase;

	if (debug_spec->progname[0] == '/') {
	    /* Match fully-qualified name, if possible. */
	    prog = progname;
	}
	if (strcmp(debug_spec->progname, prog) == 0)
	    debug_return_ptr(&debug_spec->debug_files);

#ifdef RTLD_MEMBER
	/* Handle names like sudoers.a(sudoers.so) for AIX. */
	const char *cp = strchr(prog, '(');
	const char *ep = strchr(prog, ')');
	if (cp != NULL && ep != NULL) {
	    /* Match on the program name without the member. */
	    size_t len = (size_t)(cp - prog);
	    if (strncmp(debug_spec->progname, prog, len) == 0 &&
		    debug_spec->progname[len] == '\0') {
		debug_return_ptr(&debug_spec->debug_files);
	    }

	    /* Match on the member itself. */
	    cp++;
	    len = (size_t)(ep - cp);
	    if (strncmp(debug_spec->progname, cp, len) == 0 &&
		    debug_spec->progname[len] == '\0') {
		debug_return_ptr(&debug_spec->debug_files);
	    }
	}
#endif
    }
    debug_return_ptr(NULL);
}

bool
sudo_conf_developer_mode_v1(void)
{
    /* Developer mode was removed in sudo 1.9.13. */
    return false;
}

bool
sudo_conf_disable_coredump_v1(void)
{
    return sudo_conf_data.settings.disable_coredump;
}

bool
sudo_conf_probe_interfaces_v1(void)
{
    return sudo_conf_data.settings.probe_interfaces;
}

/*
 * Free dynamically allocated parts of sudo_conf_data and
 * reset to initial values.
 */
static void
sudo_conf_init(int conf_types)
{
    struct sudo_conf_debug *debug_spec;
    struct sudo_debug_file *debug_file;
    struct plugin_info *plugin_info;
    size_t i;
    debug_decl(sudo_conf_init, SUDO_DEBUG_UTIL);

    /* Free and reset paths. */
    if (ISSET(conf_types, SUDO_CONF_PATHS)) {
	const struct sudo_conf_path_table path_table[] = SUDO_CONF_PATH_INITIALIZER;
	sudo_conf_clear_paths();
	memcpy(sudo_conf_data.path_table, path_table,
	    sizeof(sudo_conf_data.path_table));
    }

    /* Free and reset debug settings. */
    if (ISSET(conf_types, SUDO_CONF_DEBUG)) {
	while ((debug_spec = TAILQ_FIRST(&sudo_conf_data.debugging))) {
	    TAILQ_REMOVE(&sudo_conf_data.debugging, debug_spec, entries);
	    free(debug_spec->progname);
	    while ((debug_file = TAILQ_FIRST(&debug_spec->debug_files))) {
		TAILQ_REMOVE(&debug_spec->debug_files, debug_file, entries);
		free(debug_file->debug_file);
		free(debug_file->debug_flags);
		free(debug_file);
	    }
	    free(debug_spec);
	}
    }

    /* Free and reset plugins. */
    if (ISSET(conf_types, SUDO_CONF_PLUGINS)) {
	while ((plugin_info = TAILQ_FIRST(&sudo_conf_data.plugins))) {
	    TAILQ_REMOVE(&sudo_conf_data.plugins, plugin_info, entries);
	    free(plugin_info->symbol_name);
	    free(plugin_info->path);
	    if (plugin_info->options != NULL) {
		for (i = 0; plugin_info->options[i] != NULL; i++)
		    free(plugin_info->options[i]);
		free(plugin_info->options);
	    }
	    free(plugin_info);
	}
    }

    /* Set initial values. */
    if (ISSET(conf_types, SUDO_CONF_SETTINGS)) {
	const struct sudo_conf_settings settings = SUDO_CONF_SETTINGS_INITIALIZER;
	sudo_conf_data.settings = settings;
    }

    debug_return;
}

/*
 * Read in /etc/sudo.conf and populates sudo_conf_data.
 */
int
sudo_conf_read_v1(const char *path, int conf_types)
{
    FILE *fp = NULL;
    int fd = -1, ret = false;
    char *prev_locale, *line = NULL;
    unsigned int conf_lineno = 0;
    char conf_file[PATH_MAX];
    size_t linesize = 0;
    debug_decl(sudo_conf_read, SUDO_DEBUG_UTIL);

    if ((prev_locale = setlocale(LC_ALL, NULL)) == NULL) {
	sudo_warn("setlocale(LC_ALL, NULL)");
	debug_return_int(-1);
    }
    if ((prev_locale = strdup(prev_locale)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_int(-1);
    }

    /* Parse sudo.conf in the "C" locale. */
    if (prev_locale[0] != 'C' || prev_locale[1] != '\0')
        setlocale(LC_ALL, "C");

    if (path != NULL) {
	/* Caller specified a single file, which must exist. */
	if (strlcpy(conf_file, path, sizeof(conf_file)) >= sizeof(conf_file)) {
	    errno = ENAMETOOLONG;
	    sudo_warn("%s", path);
	    goto done;
	}
	fd = open(conf_file, O_RDONLY);
	if (fd == -1) {
	    sudo_warn(U_("unable to open %s"), conf_file);
	    goto done;
	}
    } else {
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
	struct stat sb;
	int error;

	/* _PATH_SUDO_CONF is a colon-separated list of path. */
	fd = sudo_open_conf_path(_PATH_SUDO_CONF, conf_file,
	    sizeof(conf_file), NULL);
	error = sudo_secure_fd(fd, S_IFREG, ROOT_UID, (gid_t)-1, &sb);
	switch (error) {
	case SUDO_PATH_SECURE:
	    /* OK! */
	    break;
	case SUDO_PATH_MISSING:
	    /* Root should always be able to read sudo.conf. */
	    if (errno != ENOENT && geteuid() == ROOT_UID)
		sudo_warn(U_("unable to open %s"), conf_file);
	    goto done;
	case SUDO_PATH_BAD_TYPE:
	    sudo_warnx(U_("%s is not a regular file"), conf_file);
	    goto done;
	case SUDO_PATH_WRONG_OWNER:
	    sudo_warnx(U_("%s is owned by uid %u, should be %u"),
		conf_file, (unsigned int) sb.st_uid, ROOT_UID);
	    goto done;
	case SUDO_PATH_WORLD_WRITABLE:
	    sudo_warnx(U_("%s is world writable"), conf_file);
	    goto done;
	case SUDO_PATH_GROUP_WRITABLE:
	    sudo_warnx(U_("%s is group writable"), conf_file);
	    goto done;
	default:
	    sudo_warnx("%s: internal error, unexpected error %d",
		__func__, error);
	    goto done;
	}
#else
	/* No default sudo.conf when fuzzing. */
	goto done;
#endif /* FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION */
    }

    if ((fp = fdopen(fd, "r")) == NULL) {
	sudo_warn(U_("unable to open %s"), conf_file);
	goto done;
    }

    /* Reset to initial values if necessary. */
    if (sudo_conf_data.settings.updated)
	sudo_conf_init(conf_types);

    while (sudo_parseln(&line, &linesize, &conf_lineno, fp, 0) != -1) {
	struct sudo_conf_table *cur;
	size_t i;
	char *cp;

	if (*(cp = line) == '\0')
	    continue;		/* empty line or comment */

	for (i = 0, cur = sudo_conf_table; cur->name != NULL; i++, cur++) {
	    if (strncasecmp(cp, cur->name, cur->namelen) == 0 &&
		isblank((unsigned char)cp[cur->namelen])) {
		if (ISSET(conf_types, (1 << i))) {
		    cp += cur->namelen;
		    while (isblank((unsigned char)*cp))
			cp++;
		    ret = cur->parser(cp, conf_file, conf_lineno);
		    switch (ret) {
		    case true:
			sudo_conf_data.settings.updated = true;
			break;
		    case false:
			break;
		    default:
			goto done;
		    }
		}
		break;
	    }
	}
	if (cur->name == NULL) {
	    sudo_debug_printf(SUDO_DEBUG_WARN,
		"%s: %s:%u: unsupported entry: %s", __func__, conf_file,
		conf_lineno, line);
	}
    }
    ret = true;

done:
    if (fp != NULL)
	fclose(fp);
    else if (fd != -1)
	close(fd);
    free(line);

    /* Restore locale if needed. */
    if (prev_locale[0] != 'C' || prev_locale[1] != '\0')
        setlocale(LC_ALL, prev_locale);
    free(prev_locale);
    debug_return_int(ret);
}

/*
 * Used by the sudo_conf regress test to clear compile-time path settings.
 */
void
sudo_conf_clear_paths_v1(void)
{
    struct sudo_conf_path_table *cur;
    debug_decl(sudo_conf_clear_paths, SUDO_DEBUG_UTIL);

    for (cur = sudo_conf_data.path_table; cur->pname != NULL; cur++) {
	if (cur->dynamic)
	    free((char *)cur->pval);
	cur->pval = NULL;
	cur->dynamic = false;
    }
}
