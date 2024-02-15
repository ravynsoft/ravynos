/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2018-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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

/*
 * Convert from the sudoers file format to LDIF or JSON format.
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif /* HAVE_STRINGS_H */
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <unistd.h>
#ifdef HAVE_GETOPT_LONG
# include <getopt.h>
# else
# include <compat/getopt.h>
#endif /* HAVE_GETOPT_LONG */

#include <sudoers.h>
#include <sudoers_version.h>
#include <sudo_lbuf.h>
#include <redblack.h>
#include <cvtsudoers.h>
#include <testsudoers_pwutil.h>
#include <tsgetgrpw.h>
#include <gram.h>

/* Long-only options values. */
#define OPT_GROUP_FILE	256
#define OPT_PASSWD_FILE	257

/*
 * Globals
 */
struct cvtsudoers_filter *filters;
static FILE *logfp;
static const char short_opts[] =  "b:c:d:ef:hi:I:l:m:Mo:O:pP:s:V";
static struct option long_opts[] = {
    { "base",		required_argument,	NULL,	'b' },
    { "config",		required_argument,	NULL,	'c' },
    { "defaults",	required_argument,	NULL,	'd' },
    { "expand-aliases",	no_argument,		NULL,	'e' },
    { "output-format",	required_argument,	NULL,	'f' },
    { "help",		no_argument,		NULL,	'h' },
    { "input-format",	required_argument,	NULL,	'i' },
    { "increment",	required_argument,	NULL,	'I' },
    { "logfile",	required_argument,	NULL,	'l' },
    { "match",		required_argument,	NULL,	'm' },
    { "match-local",	no_argument,		NULL,	'M' },
    { "prune-matches",	no_argument,		NULL,	'p' },
    { "padding",	required_argument,	NULL,	'P' },
    { "order-start",	required_argument,	NULL,	'O' },
    { "output",		required_argument,	NULL,	'o' },
    { "suppress",	required_argument,	NULL,	's' },
    { "version",	no_argument,		NULL,	'V' },
    { "group-file",	required_argument,	NULL,	OPT_GROUP_FILE },
    { "passwd-file",	required_argument,	NULL,	OPT_PASSWD_FILE },
    { NULL,		no_argument,		NULL,	0 },
};

sudo_dso_public int main(int argc, char *argv[]);
static bool convert_sudoers_sudoers(struct sudoers_parse_tree *parse_tree, const char *output_file, struct cvtsudoers_config *conf);
static bool parse_sudoers(struct sudoers_context *ctx, const char *input_file, struct cvtsudoers_config *conf);
static bool parse_ldif(struct sudoers_parse_tree *parse_tree, const char *input_file, struct cvtsudoers_config *conf);
static bool cvtsudoers_parse_filter(char *expression);
static struct cvtsudoers_config *cvtsudoers_conf_read(const char *conf_file);
static void cvtsudoers_conf_free(struct cvtsudoers_config *conf);
static unsigned int cvtsudoers_parse_defaults(char *expression);
static unsigned int cvtsudoers_parse_suppression(char *expression);
static void filter_userspecs(struct sudoers_parse_tree *parse_tree, struct cvtsudoers_config *conf);
static void filter_defaults(struct sudoers_parse_tree *parse_tree, struct cvtsudoers_config *conf);
static void alias_remove_unused(struct sudoers_parse_tree *parse_tree);
static void alias_prune(struct sudoers_parse_tree *parse_tree, struct cvtsudoers_config *conf);
sudo_noreturn static void help(void);
sudo_noreturn static void usage(void);

int
main(int argc, char *argv[])
{
    struct sudoers_parse_tree_list parse_trees = TAILQ_HEAD_INITIALIZER(parse_trees);
    struct sudoers_context ctx = SUDOERS_CONTEXT_INITIALIZER;
    struct sudoers_parse_tree merged_tree, *parse_tree = NULL;
    struct cvtsudoers_config *conf = NULL;
    enum sudoers_formats output_format = format_ldif;
    enum sudoers_formats input_format = format_sudoers;
    const char *input_file = "-";
    const char *output_file = "-";
    const char *conf_file = NULL;
    const char *grfile = NULL, *pwfile = NULL;
    const char *cp, *errstr;
    int ch, exitcode = EXIT_FAILURE;
    bool match_local = false;
    debug_decl(main, SUDOERS_DEBUG_MAIN);

#if defined(SUDO_DEVEL) && defined(__OpenBSD__)
    {
	extern char *malloc_options;
	malloc_options = "S";
    }
#endif

    initprogname(argc > 0 ? argv[0] : "cvtsudoers");
    if (!sudoers_initlocale(setlocale(LC_ALL, ""), def_sudoers_locale))
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    sudo_warn_set_locale_func(sudoers_warn_setlocale);
    bindtextdomain("sudoers", LOCALEDIR);
    textdomain("sudoers");

    /* Initialize early, before any "goto done". */
    init_parse_tree(&merged_tree, NULL, NULL, &ctx, NULL);

    /* Read debug and plugin sections of sudo.conf. */
    if (sudo_conf_read(NULL, SUDO_CONF_DEBUG|SUDO_CONF_PLUGINS) == -1)
	goto done;

    /* Initialize the debug subsystem. */
    if (!sudoers_debug_register(getprogname(), sudo_conf_debug_files(getprogname())))
	goto done;

    /* Check for --config option first (no getopt warnings). */
    opterr = 0;
    while ((ch = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
	switch (ch) {
	case 'c':
	    conf_file = optarg;
	    break;
	}
    }

    /* Read conf file. */
    conf = cvtsudoers_conf_read(conf_file);

    /*
     * Reset getopt and handle the rest of the arguments.
     */
    opterr = 1;
    optind = 1;
#ifdef HAVE_OPTRESET
    optreset = 1;
#endif
    while ((ch = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
	switch (ch) {
	case 'b':
	    free(conf->sudoers_base);
	    conf->sudoers_base = strdup(optarg);
	    if (conf->sudoers_base == NULL) {
		sudo_fatalx(U_("%s: %s"), __func__,
		    U_("unable to allocate memory"));
	    }
	    break;
	case 'c':
	    /* handled above */
	    break;
	case 'd':
	    conf->defstr = optarg;
	    break;
	case 'e':
	    conf->expand_aliases = true;
	    break;
	case 'f':
	    free(conf->output_format);
	    conf->output_format = strdup(optarg);
	    if (conf->output_format == NULL) {
		sudo_fatalx(U_("%s: %s"), __func__,
		    U_("unable to allocate memory"));
	    }
	    break;
	case 'h':
	    help();
	    /* NOTREACHED */
	case 'i':
	    free(conf->input_format);
	    conf->input_format = strdup(optarg);
	    if (conf->input_format == NULL) {
		sudo_fatalx(U_("%s: %s"), __func__,
		    U_("unable to allocate memory"));
	    }
	    break;
	case 'I':
	    conf->order_increment =
		(unsigned int)sudo_strtonum(optarg, 1, UINT_MAX, &errstr);
	    if (errstr != NULL) {
		sudo_warnx(U_("order increment: %s: %s"), optarg, U_(errstr));
		usage();
	    }
	    break;
	case 'l':
	    conf->logfile = optarg;
	    break;
	case 'm':
	    conf->filter = optarg;
	    break;
	case 'M':
	    match_local = true;
	    break;
	case 'o':
	    output_file = optarg;
	    break;
	case 'O':
	    conf->sudo_order =
		(unsigned int)sudo_strtonum(optarg, 0, UINT_MAX, &errstr);
	    if (errstr != NULL) {
		sudo_warnx(U_("starting order: %s: %s"), optarg, U_(errstr));
		usage();
	    }
	    break;
	case 'p':
	    conf->prune_matches = true;
	    break;
	case 'P':
	    conf->order_padding =
		(unsigned int)sudo_strtonum(optarg, 1, UINT_MAX, &errstr);
	    if (errstr != NULL ) {
		sudo_warnx(U_("order padding: %s: %s"), optarg, U_(errstr));
		usage();
	    }
	    break;
	case 's':
	    conf->supstr = optarg;
	    break;
	case 'V':
	    (void) printf(_("%s version %s\n"), getprogname(),
		PACKAGE_VERSION);
	    (void) printf(_("%s grammar version %d\n"), getprogname(),
		SUDOERS_GRAMMAR_VERSION);
	    exitcode = EXIT_SUCCESS;
	    goto done;
	case OPT_GROUP_FILE:
	    grfile = optarg;
	    break;
	case OPT_PASSWD_FILE:
	    pwfile = optarg;
	    break;
	default:
	    usage();
	    /* NOTREACHED */
	}
    }
    argc -= optind;
    argv += optind;

    if (conf->logfile != NULL) {
	logfp = fopen(conf->logfile, "w");
	if (logfp == NULL)
	    sudo_fatalx(U_("unable to open log file %s"), conf->logfile);
    }

    if (conf->input_format != NULL) {
	if (strcasecmp(conf->input_format, "ldif") == 0) {
	    input_format = format_ldif;
	} else if (strcasecmp(conf->input_format, "sudoers") == 0) {
	    input_format = format_sudoers; // -V1048
	} else {
	    sudo_warnx(U_("unsupported input format %s"), conf->input_format);
	    usage();
	}
    }
    if (conf->output_format != NULL) {
	if (strcasecmp(conf->output_format, "csv") == 0) {
	    output_format = format_csv;
	    conf->store_options = true;
	} else if (strcasecmp(conf->output_format, "json") == 0) {
	    output_format = format_json;
	    conf->store_options = true;
	} else if (strcasecmp(conf->output_format, "ldif") == 0) {
	    output_format = format_ldif; // -V1048
	    conf->store_options = true;
	} else if (strcasecmp(conf->output_format, "sudoers") == 0) {
	    output_format = format_sudoers;
	    conf->store_options = false;
	} else {
	    sudo_warnx(U_("unsupported output format %s"), conf->output_format);
	    usage();
	}
    }
    if (conf->filter != NULL) {
	/* We always expand aliases when filtering (may change in future). */
	if (!cvtsudoers_parse_filter(conf->filter))
	    usage();
    }
    if (conf->defstr != NULL) {
	conf->defaults = cvtsudoers_parse_defaults(conf->defstr);
	if (conf->defaults == (unsigned int)-1)
	    usage();
    }
    if (conf->supstr != NULL) {
	conf->suppress = cvtsudoers_parse_suppression(conf->supstr);
	if (conf->suppress == (unsigned int)-1)
	    usage();
    }

    /* Apply padding to sudo_order if present. */
    if (conf->sudo_order != 0 && conf->order_padding != 0) {
	unsigned int multiplier = 1;

	do {
	    multiplier *= 10;
	} while (--conf->order_padding != 0);
	conf->sudo_order *= multiplier;
	conf->order_max = conf->sudo_order + (multiplier - 1);
	conf->order_padding = multiplier;
    }

    /* If no base DN specified, check SUDOERS_BASE. */
    if (conf->sudoers_base == NULL) {
	conf->sudoers_base = getenv("SUDOERS_BASE");
	if (conf->sudoers_base != NULL && *conf->sudoers_base != '\0') {
	    if ((conf->sudoers_base = strdup(conf->sudoers_base)) == NULL) {
		sudo_fatalx(U_("%s: %s"), __func__,
		    U_("unable to allocate memory"));
	    }
	}
    }

    /* Set pwutil backend to use the filter data. */
    if (conf->filter != NULL && !match_local) {
	sudo_pwutil_set_backend(cvtsudoers_make_pwitem, cvtsudoers_make_gritem,
	    cvtsudoers_make_gidlist_item, cvtsudoers_make_grlist_item, NULL);
    } else {
	if (grfile != NULL)
	    testsudoers_setgrfile(grfile);
	if (pwfile != NULL)
	    testsudoers_setpwfile(pwfile);
	sudo_pwutil_set_backend(
	    pwfile ? testsudoers_make_pwitem : NULL,
	    grfile ? testsudoers_make_gritem : NULL,
	    grfile ? testsudoers_make_gidlist_item : NULL,
	    grfile ? testsudoers_make_grlist_item : NULL,
	    NULL);
    }

    /* We may need the hostname to resolve %h escapes in include files. */
    if (!sudoers_sethost(&ctx, NULL, NULL))
	goto done;

    do {
	char *lhost = NULL, *shost = NULL;

	/* Input file (defaults to stdin). */
	if (argc > 0)
	    input_file = argv[0];

	/* Check for optional hostname prefix on the input file. */
	cp = strchr(input_file, ':');
	if (cp != NULL) {
	    struct stat sb;

	    if (strcmp(cp, ":-") == 0 || stat(input_file, &sb) == -1) {
		lhost = strndup(input_file, (size_t)(cp - input_file));
		if (lhost == NULL)
		    sudo_fatalx("%s", U_("unable to allocate memory"));
		input_file = cp + 1;
		cp = strchr(lhost, '.');
		if (cp == NULL) {
		    shost = lhost;
		} else {
		    shost = strndup(lhost, (size_t)(cp - lhost));
		}
	    }
	}

	if (strcmp(input_file, "-") != 0) {
	    if (strcmp(input_file, output_file) == 0) {
		sudo_fatalx(U_("%s: input and output files must be different"),
		    input_file);
	    }
	}

	parse_tree = malloc(sizeof(*parse_tree));
	if (parse_tree == NULL)
	    sudo_fatalx("%s", U_("unable to allocate memory"));
	init_parse_tree(parse_tree, lhost, shost, &ctx, NULL);
	TAILQ_INSERT_TAIL(&parse_trees, parse_tree, entries);

	/* Setup defaults data structures. */
	if (!init_defaults()) {
	    sudo_fatalx("%s",
		U_("unable to initialize sudoers default values"));
	}

	switch (input_format) {
	case format_ldif:
	    if (!parse_ldif(parse_tree, input_file, conf))
		goto done;
	    break;
	case format_sudoers:
	    if (!parse_sudoers(&ctx, input_file, conf))
		goto done;
	    reparent_parse_tree(parse_tree);
	    break;
	default:
	    sudo_fatalx("error: unhandled input %d", input_format);
	}

	/* Apply filters. */
	filter_userspecs(parse_tree, conf);
	filter_defaults(parse_tree, conf);
	if (filters != NULL) {
	    alias_remove_unused(parse_tree);
	    if (conf->prune_matches && conf->expand_aliases)
		alias_prune(parse_tree, conf);
	}

	argc--;
	argv++;
    } while (argc > 0);

    parse_tree = TAILQ_FIRST(&parse_trees);
    if (TAILQ_NEXT(parse_tree, entries)) {
	/* Multiple sudoers files, merge them all. */
	parse_tree = merge_sudoers(&parse_trees, &merged_tree);
    }

    switch (output_format) {
    case format_csv:
	exitcode = !convert_sudoers_csv(parse_tree, output_file, conf);
	break;
    case format_json:
	exitcode = !convert_sudoers_json(parse_tree, output_file, conf);
	break;
    case format_ldif:
	exitcode = !convert_sudoers_ldif(parse_tree, output_file, conf);
	break;
    case format_sudoers:
	exitcode = !convert_sudoers_sudoers(parse_tree, output_file, conf);
	break;
    default:
	sudo_fatalx("error: unhandled output format %d", output_format);
    }

done:
    sudoers_ctx_free(&ctx);
    free_parse_tree(&merged_tree);
    while ((parse_tree = TAILQ_FIRST(&parse_trees)) != NULL) {
	TAILQ_REMOVE(&parse_trees, parse_tree, entries);
	free_parse_tree(parse_tree);
	free(parse_tree);
    }
    cvtsudoers_conf_free(conf);
    sudo_debug_exit_int(__func__, __FILE__, __LINE__, sudo_debug_subsys, exitcode);
    return exitcode;
}

void
log_warnx(const char * restrict fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    if (logfp != NULL) {
	vfprintf(logfp, fmt, ap);
	fputc('\n', logfp);
    } else {
    	sudo_vwarnx_nodebug(fmt, ap);
    }
    va_end(ap);
}

/*
 * cvtsudoers configuration data.
 */
static struct cvtsudoers_config cvtsudoers_config = INITIAL_CONFIG;
static struct cvtsudoers_conf_table cvtsudoers_conf_vars[] = {
    { "order_start", CONF_UINT, &cvtsudoers_config.sudo_order },
    { "order_increment", CONF_UINT, &cvtsudoers_config.order_increment },
    { "order_padding", CONF_UINT, &cvtsudoers_config.order_padding },
    { "sudoers_base", CONF_STR, &cvtsudoers_config.sudoers_base },
    { "input_format", CONF_STR, &cvtsudoers_config.input_format },
    { "output_format", CONF_STR, &cvtsudoers_config.output_format },
    { "match", CONF_STR, &cvtsudoers_config.filter },
    { "match_local", CONF_BOOL, &cvtsudoers_config.match_local },
    { "logfile", CONF_STR, &cvtsudoers_config.logfile },
    { "defaults", CONF_STR, &cvtsudoers_config.defstr },
    { "suppress", CONF_STR, &cvtsudoers_config.supstr },
    { "group_file", CONF_STR, &cvtsudoers_config.group_file },
    { "passwd_file", CONF_STR, &cvtsudoers_config.passwd_file },
    { "expand_aliases", CONF_BOOL, &cvtsudoers_config.expand_aliases },
    { "prune_matches", CONF_BOOL, &cvtsudoers_config.prune_matches }
};

/*
 * Look up keyword in config table.
 * Returns true if found, else false.
 */
static bool
cvtsudoers_parse_keyword(const char *conf_file, const char *keyword,
    const char *value, struct cvtsudoers_conf_table *table)
{
    struct cvtsudoers_conf_table *cur;
    const char *errstr;
    debug_decl(sudo_ldap_parse_keyword, SUDOERS_DEBUG_UTIL);

    /* Look up keyword in config tables */
    for (cur = table; cur->conf_str != NULL; cur++) {
	if (strcasecmp(keyword, cur->conf_str) == 0) {
	    switch (cur->type) {
	    case CONF_BOOL:
		*(bool *)(cur->valp) = sudo_strtobool(value) == true;
		break;
	    case CONF_UINT:
		{
		    unsigned int uval =
			(unsigned int)sudo_strtonum(value, 0, UINT_MAX, &errstr);
		    if (errstr != NULL) {
			sudo_warnx(U_("%s: %s: %s: %s"),
			    conf_file, keyword, value, U_(errstr));
			continue;
		    }
		    *(unsigned int *)(cur->valp) = uval;
		}
		break;
	    case CONF_STR:
		{
		    char *cp = strdup(value);
		    if (cp == NULL) {
			sudo_fatalx(U_("%s: %s"), __func__,
			    U_("unable to allocate memory"));
		    }
		    free(*(char **)(cur->valp));
		    *(char **)(cur->valp) = cp;
		    break;
		}
	    }
	    debug_return_bool(true);
	}
    }
    debug_return_bool(false);
}

static struct cvtsudoers_config *
cvtsudoers_conf_read(const char *path)
{
    char conf_file[PATH_MAX], *line = NULL;
    size_t linesize = 0;
    FILE *fp = NULL;
    int fd = -1;
    debug_decl(cvtsudoers_conf_read, SUDOERS_DEBUG_UTIL);

    if (path != NULL) {
	/* Empty string means use the defaults. */
	if (*path == '\0')
	    debug_return_ptr(&cvtsudoers_config);
	if (strlcpy(conf_file, path, sizeof(conf_file)) >= sizeof(conf_file))
	    errno = ENAMETOOLONG;
	else 
	    fd = open(conf_file, O_RDONLY);
    } else {
	fd = sudo_open_conf_path(_PATH_CVTSUDOERS_CONF, conf_file,
	    sizeof(conf_file), NULL);
    }
    if (fd != -1)
	fp = fdopen(fd, "r");
    if (fp == NULL) {
	if (path != NULL || errno != ENOENT)
	    sudo_warn("%s", conf_file);
	debug_return_ptr(&cvtsudoers_config);
    }

    while (sudo_parseln(&line, &linesize, NULL, fp, 0) != -1) {
	char *keyword, *value;
	size_t len;

	if (*line == '\0')
	    continue;		/* skip empty line */

	/* Parse keyword = value */
	keyword = line;
	if ((value = strchr(line, '=')) == NULL || value == line)
	    continue;
	len = (size_t)(value - line);

	/* Trim whitespace after keyword and NUL-terminate. */
	while (len > 0 && isblank((unsigned char)line[len - 1]))
	    len--;
	line[len] = '\0';

	/* Trim whitespace before value. */
	do {
	    value++;
	} while (isblank((unsigned char)*value));

	/* Look up keyword in config tables */
	if (!cvtsudoers_parse_keyword(conf_file, keyword, value, cvtsudoers_conf_vars))
	    sudo_warnx(U_("%s: unknown key word %s"), conf_file, keyword);
    }
    free(line);
    fclose(fp);

    debug_return_ptr(&cvtsudoers_config);
}

static void
cvtsudoers_conf_free(struct cvtsudoers_config *conf)
{
    debug_decl(cvtsudoers_conf_free, SUDOERS_DEBUG_UTIL);

    if (conf != NULL) {
	free(conf->sudoers_base);
	free(conf->input_format);
	free(conf->output_format);
	conf->sudoers_base = NULL;
	conf->input_format = NULL;
	conf->output_format = NULL;
    }

    debug_return;
}

static unsigned int
cvtsudoers_parse_defaults(char *expression)
{
    char *last, *cp = expression;
    unsigned int flags = 0;
    debug_decl(cvtsudoers_parse_defaults, SUDOERS_DEBUG_UTIL);

    for ((cp = strtok_r(cp, ",", &last)); cp != NULL; (cp = strtok_r(NULL, ",", &last))) {
	if (strcasecmp(cp, "all") == 0) {
	    SET(flags, CVT_DEFAULTS_ALL);
	} else if (strcasecmp(cp, "global") == 0) {
	    SET(flags, CVT_DEFAULTS_GLOBAL);
	} else if (strcasecmp(cp, "user") == 0) {
	    SET(flags, CVT_DEFAULTS_USER);
	} else if (strcasecmp(cp, "runas") == 0) {
	    SET(flags, CVT_DEFAULTS_RUNAS);
	} else if (strcasecmp(cp, "host") == 0) {
	    SET(flags, CVT_DEFAULTS_HOST);
	} else if (strcasecmp(cp, "command") == 0) {
	    SET(flags, CVT_DEFAULTS_CMND);
	} else {
	    sudo_warnx(U_("invalid defaults type: %s"), cp);
	    debug_return_uint((unsigned int)-1);
	}
    }

    debug_return_uint(flags);
}

static unsigned int
cvtsudoers_parse_suppression(char *expression)
{
    char *last, *cp = expression;
    unsigned int flags = 0;
    debug_decl(cvtsudoers_parse_suppression, SUDOERS_DEBUG_UTIL);

    for ((cp = strtok_r(cp, ",", &last)); cp != NULL; (cp = strtok_r(NULL, ",", &last))) {
	if (strcasecmp(cp, "defaults") == 0) {
	    SET(flags, SUPPRESS_DEFAULTS);
	} else if (strcasecmp(cp, "aliases") == 0) {
	    SET(flags, SUPPRESS_ALIASES);
	} else if (strcasecmp(cp, "privileges") == 0 || strcasecmp(cp, "privs") == 0) {
	    SET(flags, SUPPRESS_PRIVS);
	} else {
	    sudo_warnx(U_("invalid suppression type: %s"), cp);
	    debug_return_uint((unsigned int)-1);
	}
    }

    debug_return_uint(flags);
}

static bool
cvtsudoers_parse_filter(char *expression)
{
    char *last, *cp = expression;
    debug_decl(cvtsudoers_parse_filter, SUDOERS_DEBUG_UTIL);

    if (filters == NULL) {
	if ((filters = malloc(sizeof(*filters))) == NULL) {
	    sudo_fatalx(U_("%s: %s"), __func__,
		U_("unable to allocate memory"));
	}
	STAILQ_INIT(&filters->users);
	STAILQ_INIT(&filters->groups);
	STAILQ_INIT(&filters->hosts);
	STAILQ_INIT(&filters->cmnds);
    }

    for ((cp = strtok_r(cp, ",", &last)); cp != NULL; (cp = strtok_r(NULL, ",", &last))) {
	/*
	 * Filter expression:
	 *	user=foo,group=bar,host=baz
	 */
	char *keyword;
	struct sudoers_string *s;

	if ((s = malloc(sizeof(*s))) == NULL) {
	    sudo_fatalx(U_("%s: %s"), __func__,
		U_("unable to allocate memory"));
	}

	/* Parse keyword = value */
	keyword = cp;
	if ((cp = strchr(cp, '=')) == NULL) {
	    sudo_warnx(U_("invalid filter: %s"), keyword);
	    free(s);
	    debug_return_bool(false);
	}
	*cp++ = '\0';
	s->str = cp;

	if (strcmp(keyword, "user") == 0) {
	    STAILQ_INSERT_TAIL(&filters->users, s, entries);
	} else if (strcmp(keyword, "group") == 0) {
	    STAILQ_INSERT_TAIL(&filters->groups, s, entries);
	} else if (strcmp(keyword, "host") == 0) {
	    STAILQ_INSERT_TAIL(&filters->hosts, s, entries);
	} else if (strcmp(keyword, "cmnd") == 0 || strcmp(keyword, "cmd") == 0) {
	    STAILQ_INSERT_TAIL(&filters->cmnds, s, entries);
	} else {
	    sudo_warnx(U_("invalid filter: %s"), keyword);
	    free(s);
	    debug_return_bool(false);
	}
    }

    debug_return_bool(true);
}

static bool
parse_ldif(struct sudoers_parse_tree *parse_tree, const char *input_file,
    struct cvtsudoers_config *conf)
{
    FILE *fp = stdin;
    bool ret = false;
    debug_decl(parse_ldif, SUDOERS_DEBUG_UTIL);

    /* Open LDIF file and parse it. */
    if (strcmp(input_file, "-") != 0) {
	if ((fp = fopen(input_file, "r")) == NULL)
	    sudo_warn(U_("unable to open %s"), input_file);
    }
    if (fp != NULL) {
	ret = sudoers_parse_ldif(parse_tree, fp, conf->sudoers_base,
	    conf->store_options);
	if (fp != stdin)
	    fclose(fp);
    }
    debug_return_bool(ret);
}

static bool
parse_sudoers(struct sudoers_context *ctx, const char *input_file,
    struct cvtsudoers_config *conf)
{
    debug_decl(parse_sudoers, SUDOERS_DEBUG_UTIL);

    /* Open sudoers file and parse it. */
    if (strcmp(input_file, "-") == 0) {
	sudoersin = stdin;
	input_file = "stdin";
    } else if ((sudoersin = fopen(input_file, "r")) == NULL)
	sudo_fatal(U_("unable to open %s"), input_file);
    init_parser(ctx, input_file);
    if (sudoersparse() && !parse_error) {
	sudo_warnx(U_("failed to parse %s file, unknown error"), input_file);
	parse_error = true;
    }
    debug_return_bool(!parse_error);
}

FILE *
open_sudoers(const char *file, char **outfile, bool doedit, bool *keepopen)
{
    return fopen(file, "r");
}

static bool
userlist_matches_filter(struct sudoers_parse_tree *parse_tree,
    struct member_list *users, struct cvtsudoers_config *conf)
{
    struct sudoers_string *s;
    struct member *m, *next;
    bool ret = false;
    debug_decl(userlist_matches_filter, SUDOERS_DEBUG_UTIL);

    if (filters == NULL ||
	(STAILQ_EMPTY(&filters->users) && STAILQ_EMPTY(&filters->groups)))
	debug_return_bool(true);

    TAILQ_FOREACH_REVERSE_SAFE(m, users, member_list, entries, next) {
	bool matched = false;

	if (STAILQ_EMPTY(&filters->users)) {
	    struct passwd pw;

	    /*
	     * Only groups in filter, make a fake user so userlist_matches()
	     * can do its thing.
	     */
	    memset(&pw, 0, sizeof(pw));
	    pw.pw_name = (char *)"_nobody";
	    pw.pw_uid = (uid_t)-1;
	    pw.pw_gid = (gid_t)-1;

	    if (user_matches(parse_tree, &pw, m) == ALLOW)
		matched = true;
	} else {
	    STAILQ_FOREACH(s, &filters->users, entries) {
		struct passwd *pw = NULL;

		/* An upper case filter entry may be a User_Alias */
		/* XXX - doesn't handle nested aliases */
		if (m->type == ALIAS && !conf->expand_aliases) {
		    if (strcmp(m->name, s->str) == 0) {
			matched = true;
			break;
		    }
		}

		if (s->str[0] == '#') {
		    const char *errstr;
		    uid_t uid = sudo_strtoid(s->str + 1, &errstr);
		    if (errstr == NULL)
			pw = sudo_getpwuid(uid);
		}
		if (pw == NULL)
		    pw = sudo_getpwnam(s->str);
		if (pw == NULL)
		    continue;

		if (user_matches(parse_tree, pw, m) == ALLOW)
		    matched = true;
		sudo_pw_delref(pw);

		/* Only need one user in the filter to match. */
		if (matched)
		    break;
	    }
	}

	if (matched) {
	    ret = true;
	} else if (conf->prune_matches) {
	    TAILQ_REMOVE(users, m, entries);
	    free_member(m);
	}
    }

    debug_return_bool(ret);
}

static bool
hostlist_matches_filter(struct sudoers_parse_tree *parse_tree,
    struct member_list *hostlist, struct cvtsudoers_config *conf)
{
    struct sudoers_string *s;
    struct member *m, *next;
    char *lhost, *shost;
    bool ret = false;
    char **shosts;
    size_t n = 0;
    debug_decl(hostlist_matches_filter, SUDOERS_DEBUG_UTIL);

    if (filters == NULL || STAILQ_EMPTY(&filters->hosts))
	debug_return_bool(true);

    /* Create an array of short host names. */
    STAILQ_FOREACH(s, &filters->hosts, entries) {
	n++;
    }
    shosts = reallocarray(NULL, n, sizeof(char *));
    if (shosts == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    n = 0;
    STAILQ_FOREACH(s, &filters->hosts, entries) {
	lhost = s->str;
	if ((shost = strchr(lhost, '.')) != NULL) {
	    shost = strndup(lhost, (size_t)(shost - lhost));
	    if (shost == NULL) {
		sudo_fatalx(U_("%s: %s"), __func__,
		    U_("unable to allocate memory"));
	    }
	} else {
	    shost = lhost;
	}
	shosts[n++] = shost;
    }

    TAILQ_FOREACH_REVERSE_SAFE(m, hostlist, member_list, entries, next) {
	bool matched = false;
	n = 0;
	STAILQ_FOREACH(s, &filters->hosts, entries) {
	    lhost = s->str;
	    shost = shosts[n++];

	    /* An upper case filter entry may be a Host_Alias */
	    /* XXX - doesn't handle nested aliases */
	    if (m->type == ALIAS && !conf->expand_aliases) {
		if (strcmp(m->name, s->str) == 0) {
		    matched = true;
		    break;
		}
	    }

	    /* Only need one host in the filter to match. */
	    /* XXX - can't use netgroup_tuple with NULL pw */
	    if (host_matches(parse_tree, NULL, lhost, shost, m) == ALLOW) {
		matched = true;
		break;
	    }
	}

	if (matched) {
	    ret = true;
	} else if (conf->prune_matches) {
	    TAILQ_REMOVE(hostlist, m, entries);
	    free_member(m);
	}
    }

    /* Free shosts array and its contents. */
    n = 0;
    STAILQ_FOREACH(s, &filters->hosts, entries) {
	lhost = s->str;
	shost = shosts[n++];
	if (shost != lhost)
	    free(shost);
    }
    free(shosts);

    debug_return_bool(ret);
}

static bool
cmnd_matches_filter(struct sudoers_parse_tree *parse_tree,
    struct member *m, struct cvtsudoers_config *conf)
{
    struct sudoers_context *ctx = parse_tree->ctx;
    struct sudoers_string *s;
    bool matched = false;
    debug_decl(cmnd_matches_filter, SUDOERS_DEBUG_UTIL);

    /* TODO: match on runasuserlist/runasgrouplist, notbefore/notafter etc */
    STAILQ_FOREACH(s, &filters->cmnds, entries) {
	/* An upper case filter entry may be a Cmnd_Alias */
	/* XXX - doesn't handle nested aliases */
	if (m->type == ALIAS && !conf->expand_aliases) {
	    if (strcmp(m->name, s->str) == 0) {
		matched = true;
		break;
	    }
	}

	/* Only need one command in the filter to match. */
	ctx->user.cmnd = s->str;
	ctx->user.cmnd_base = sudo_basename(ctx->user.cmnd);
	if (cmnd_matches(parse_tree, m, NULL, NULL) == ALLOW) {
	    matched = true;
	    break;
	}
    }
    ctx->user.cmnd_base = NULL;
    ctx->user.cmnd = NULL;

    debug_return_bool(matched);
}

static bool
cmndlist_matches_filter(struct sudoers_parse_tree *parse_tree,
    struct member_list *cmndlist, struct cvtsudoers_config *conf)
{
    struct member *m, *next;
    bool ret = false;
    debug_decl(cmndlist_matches_filter, SUDOERS_DEBUG_UTIL);

    if (filters == NULL || STAILQ_EMPTY(&filters->cmnds))
	debug_return_bool(true);

    TAILQ_FOREACH_REVERSE_SAFE(m, cmndlist, member_list, entries, next) {
	bool matched = cmnd_matches_filter(parse_tree, m, conf);
	if (matched) {
	    ret = true;
	} else if (conf->prune_matches) {
	    TAILQ_REMOVE(cmndlist, m, entries);
	    free_member(m);
	}
    }

    debug_return_bool(ret);
}

static bool
cmndspeclist_matches_filter(struct sudoers_parse_tree *parse_tree,
    struct cmndspec_list *cmndspecs, struct cvtsudoers_config *conf)
{
    struct cmndspec *cs, *next;
    bool ret = false;
    debug_decl(cmndspeclist_matches_filter, SUDOERS_DEBUG_UTIL);

    if (filters == NULL || STAILQ_EMPTY(&filters->cmnds))
	debug_return_bool(true);

    TAILQ_FOREACH_REVERSE_SAFE(cs, cmndspecs, cmndspec_list, entries, next) {
	bool matched = cmnd_matches_filter(parse_tree, cs->cmnd, conf);
	if (matched) {
	    ret = true;
	} else if (conf->prune_matches) {
	    /* free_cmndspec() removes cs from the list itself. */
	    free_cmndspec(cs, cmndspecs);
	}
    }

    debug_return_bool(ret);
}

/*
 * Display Defaults entries
 */
static bool
print_defaults_sudoers(struct sudoers_parse_tree *parse_tree,
    struct sudo_lbuf *lbuf, bool expand_aliases)
{
    struct defaults *def, *next;
    debug_decl(print_defaults_sudoers, SUDOERS_DEBUG_UTIL);

    TAILQ_FOREACH_SAFE(def, &parse_tree->defaults, entries, next) {
	sudoers_format_default_line(lbuf, parse_tree, def, &next,
	    expand_aliases);
    }

    debug_return_bool(!sudo_lbuf_error(lbuf));
}

static int
print_alias_sudoers(struct sudoers_parse_tree *parse_tree, struct alias *a,
    void *v)
{
    struct sudo_lbuf *lbuf = v;
    struct member *m;
    debug_decl(print_alias_sudoers, SUDOERS_DEBUG_UTIL);

    sudo_lbuf_append(lbuf, "%s %s = ", alias_type_to_string(a->type),
	a->name);
    TAILQ_FOREACH(m, &a->members, entries) {
	if (m != TAILQ_FIRST(&a->members))
	    sudo_lbuf_append(lbuf, ", ");
	sudoers_format_member(lbuf, parse_tree, m, NULL, UNSPEC);
    }
    sudo_lbuf_append(lbuf, "\n");

    debug_return_int(sudo_lbuf_error(lbuf) ? -1 : 0);
}

/*
 * Display aliases
 */
static bool
print_aliases_sudoers(struct sudoers_parse_tree *parse_tree,
    struct sudo_lbuf *lbuf)
{
    debug_decl(print_aliases_sudoers, SUDOERS_DEBUG_UTIL);

    alias_apply(parse_tree, print_alias_sudoers, lbuf);

    debug_return_bool(!sudo_lbuf_error(lbuf));
}

static FILE *output_fp;		/* global for convert_sudoers_output */

static int
convert_sudoers_output(const char * restrict buf)
{
    return fputs(buf, output_fp);
}

/*
 * Apply filters to userspecs, removing non-matching entries.
 */
static void
filter_userspecs(struct sudoers_parse_tree *parse_tree,
    struct cvtsudoers_config *conf)
{
    struct userspec *us, *next_us;
    struct privilege *priv, *next_priv;
    debug_decl(filter_userspecs, SUDOERS_DEBUG_UTIL);

    if (filters == NULL)
	debug_return;

    /*
     * Does not currently prune out non-matching entries in the user or
     * host lists.  It acts more like a grep than a true filter.
     * In the future, we may want to add a prune option.
     */
    TAILQ_FOREACH_SAFE(us, &parse_tree->userspecs, entries, next_us) {
	if (!userlist_matches_filter(parse_tree, &us->users, conf)) {
	    TAILQ_REMOVE(&parse_tree->userspecs, us, entries);
	    free_userspec(us);
	    continue;
	}
	TAILQ_FOREACH_SAFE(priv, &us->privileges, entries, next_priv) {
	    if (!hostlist_matches_filter(parse_tree, &priv->hostlist, conf) ||
		!cmndspeclist_matches_filter(parse_tree, &priv->cmndlist, conf)) {
		TAILQ_REMOVE(&us->privileges, priv, entries);
		free_privilege(priv);
	    }
	}
	if (TAILQ_EMPTY(&us->privileges)) {
	    TAILQ_REMOVE(&parse_tree->userspecs, us, entries);
	    free_userspec(us);
	    continue;
	}
    }
    debug_return;
}

/*
 * Check whether the alias described by "alias_name" is the same
 * as "name" or includes an alias called "name".
 * Returns true if matched, else false.
 */
static bool
alias_matches(struct sudoers_parse_tree *parse_tree, const char *name,
    const char *alias_name, short alias_type)
{
    struct alias *a;
    struct member *m;
    bool ret = false;
    debug_decl(alias_matches, SUDOERS_DEBUG_ALIAS);

    if (strcmp(name, alias_name) == 0)
	debug_return_bool(true);

    a = alias_get(parse_tree, alias_name, alias_type);
    if (a != NULL) {
	TAILQ_FOREACH(m, &a->members, entries) {
	    if (m->type != ALIAS)
		continue;
	    if (alias_matches(parse_tree, name, m->name, alias_type)) {
		ret = true;
		break;
	    }
	}
	alias_put(a);
    }

    debug_return_bool(ret);
}

/*
 * Check whether userspecs uses the aliases in the specified member lists.
 * If used, they are removed (and freed) from the list.
 * This does *not* check Defaults for used aliases, only userspecs.
 */
static void
alias_used_by_userspecs(struct sudoers_parse_tree *parse_tree,
    struct member_list *user_aliases, struct member_list *runas_aliases,
    struct member_list *host_aliases, struct member_list *cmnd_aliases)
{
    struct privilege *priv, *priv_next;
    struct userspec *us, *us_next;
    struct cmndspec *cs, *cs_next;
    struct member *m, *m_next;
    struct member *am, *am_next;
    debug_decl(alias_used_by_userspecs, SUDOERS_DEBUG_ALIAS);

    /* Iterate over the policy, checking for aliases. */
    TAILQ_FOREACH_SAFE(us, &parse_tree->userspecs, entries, us_next) {
	TAILQ_FOREACH_SAFE(m, &us->users, entries, m_next) {
	    if (m->type == ALIAS) {
		/* If alias is used, remove from user_aliases and free. */
		TAILQ_FOREACH_SAFE(am, user_aliases, entries, am_next) {
		    if (alias_matches(parse_tree, am->name, m->name, USERALIAS)) {
			TAILQ_REMOVE(user_aliases, am, entries);
			free_member(am);
		    }
		}
	    }
	}
	TAILQ_FOREACH_SAFE(priv, &us->privileges, entries, priv_next) {
	    TAILQ_FOREACH(m, &priv->hostlist, entries) {
		if (m->type == ALIAS) {
		    /* If alias is used, remove from host_aliases and free. */
		    TAILQ_FOREACH_SAFE(am, host_aliases, entries, am_next) {
			if (alias_matches(parse_tree, am->name, m->name, HOSTALIAS)) {
			    TAILQ_REMOVE(host_aliases, am, entries);
			    free_member(am);
			}
		    }
		}
	    }
	    TAILQ_FOREACH_SAFE(cs, &priv->cmndlist, entries, cs_next) {
		if (cs->runasuserlist != NULL) {
		    TAILQ_FOREACH_SAFE(m, cs->runasuserlist, entries, m_next) {
			if (m->type == ALIAS) {
			    /* If alias is used, remove from runas_aliases and free. */
			    TAILQ_FOREACH_SAFE(am, runas_aliases, entries, am_next) {
				if (alias_matches(parse_tree, am->name, m->name, RUNASALIAS)) {
				    TAILQ_REMOVE(runas_aliases, am, entries);
				    free_member(am);
				}
			    }
			}
		    }
		}
		if (cs->runasgrouplist != NULL) {
		    TAILQ_FOREACH_SAFE(m, cs->runasgrouplist, entries, m_next) {
			if (m->type == ALIAS) {
			    /* If alias is used, remove from runas_aliases and free. */
			    TAILQ_FOREACH_SAFE(am, runas_aliases, entries, am_next) {
				if (alias_matches(parse_tree, am->name, m->name, RUNASALIAS)) {
				    TAILQ_REMOVE(runas_aliases, am, entries);
				    free_member(am);
				}
			    }
			}
		    }
		}
		if ((m = cs->cmnd)->type == ALIAS) {
		    /* If alias is used, remove from cmnd_aliases and free. */
		    TAILQ_FOREACH_SAFE(am, cmnd_aliases, entries, am_next) {
			if (alias_matches(parse_tree, am->name, m->name, CMNDALIAS)) {
			    TAILQ_REMOVE(cmnd_aliases, am, entries);
			    free_member(am);
			}
		    }
		}
	    }
	}
    }

    debug_return;
}

/*
 * For each alias listed in members, remove and free the alias.
 * Frees the contents of members too.
 */
static void
free_aliases_by_members(struct sudoers_parse_tree *parse_tree,
    struct member_list *members, short type)
{
    struct member *m;
    struct alias *a;
    debug_decl(free_aliases_by_members, SUDOERS_DEBUG_ALIAS);

    while ((m = TAILQ_FIRST(members)) != NULL) {
        TAILQ_REMOVE(members, m, entries);
	a = alias_remove(parse_tree, m->name, type);
	alias_free(a);
	free_member(m);
    }
    debug_return;
}

/*
 * Apply filters to host/user-based Defaults, removing non-matching entries.
 */
static void
filter_defaults(struct sudoers_parse_tree *parse_tree,
    struct cvtsudoers_config *conf)
{
    struct member_list user_aliases = TAILQ_HEAD_INITIALIZER(user_aliases);
    struct member_list runas_aliases = TAILQ_HEAD_INITIALIZER(runas_aliases);
    struct member_list host_aliases = TAILQ_HEAD_INITIALIZER(host_aliases);
    struct member_list cmnd_aliases = TAILQ_HEAD_INITIALIZER(cmnd_aliases);
    struct defaults *def, *def_next;
    struct member *m, *m_next;
    short alias_type;
    debug_decl(filter_defaults, SUDOERS_DEBUG_DEFAULTS);

    if (filters == NULL && conf->defaults == CVT_DEFAULTS_ALL)
	debug_return;

    TAILQ_FOREACH_SAFE(def, &parse_tree->defaults, entries, def_next) {
	bool keep = true;

	switch (def->type) {
	case DEFAULTS:
	    if (!ISSET(conf->defaults, CVT_DEFAULTS_GLOBAL))
		keep = false;
	    alias_type = UNSPEC;
	    break;
	case DEFAULTS_USER:
	    if (!ISSET(conf->defaults, CVT_DEFAULTS_USER) ||
		    !userlist_matches_filter(parse_tree, &def->binding->members,
		    conf)) {
		keep = false;
	    }
	    alias_type = USERALIAS;
	    break;
	case DEFAULTS_RUNAS:
	    if (!ISSET(conf->defaults, CVT_DEFAULTS_RUNAS))
		keep = false;
	    alias_type = RUNASALIAS;
	    break;
	case DEFAULTS_HOST:
	    if (!ISSET(conf->defaults, CVT_DEFAULTS_HOST) ||
		    !hostlist_matches_filter(parse_tree, &def->binding->members,
		    conf)) {
		keep = false;
	    }
	    alias_type = HOSTALIAS;
	    break;
	case DEFAULTS_CMND:
	    if (!ISSET(conf->defaults, CVT_DEFAULTS_CMND) ||
		    !cmndlist_matches_filter(parse_tree, &def->binding->members,
		    conf)) {
		keep = false;
	    }
	    alias_type = CMNDALIAS;
	    break;
	default:
	    sudo_fatalx_nodebug("unexpected defaults type %d", def->type);
	    break;
	}

	if (!keep) {
	    /*
	     * Look for aliases used by the binding.
	     * Consecutive Defaults can share the same binding.
	     */
	    /* XXX - move to function */
	    if (alias_type != UNSPEC &&
		    (def_next == NULL || def->binding != def_next->binding)) {
		TAILQ_FOREACH_SAFE(m, &def->binding->members, entries, m_next) {
		    if (m->type == ALIAS) {
			TAILQ_REMOVE(&def->binding->members, m, entries);
			switch (alias_type) {
			case USERALIAS:
			    TAILQ_INSERT_TAIL(&user_aliases, m, entries);
			    break;
			case RUNASALIAS:
			    TAILQ_INSERT_TAIL(&runas_aliases, m, entries);
			    break;
			case HOSTALIAS:
			    TAILQ_INSERT_TAIL(&host_aliases, m, entries);
			    break;
			case CMNDALIAS:
			    TAILQ_INSERT_TAIL(&cmnd_aliases, m, entries);
			    break;
			default:
			    sudo_fatalx_nodebug("unexpected alias type %d",
				alias_type);
			    break;
			}
		    }
		}
	    }
	    TAILQ_REMOVE(&parse_tree->defaults, def, entries);
	    free_default(def);
	}
    }

    /* Determine unreferenced aliases and remove/free them. */
    alias_used_by_userspecs(parse_tree, &user_aliases, &runas_aliases,
	&host_aliases, &cmnd_aliases);
    free_aliases_by_members(parse_tree, &user_aliases, USERALIAS);
    free_aliases_by_members(parse_tree, &runas_aliases, RUNASALIAS);
    free_aliases_by_members(parse_tree, &host_aliases, HOSTALIAS);
    free_aliases_by_members(parse_tree, &cmnd_aliases, CMNDALIAS);

    debug_return;
}

/*
 * Remove unreferenced aliases.
 */
static void
alias_remove_unused(struct sudoers_parse_tree *parse_tree)
{
    struct rbtree *used_aliases;
    debug_decl(alias_remove_unused, SUDOERS_DEBUG_ALIAS);

    used_aliases = alloc_aliases();
    if (used_aliases == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));

    /* Move all referenced aliases to used_aliases. */
    if (!alias_find_used(parse_tree, used_aliases))
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));

    /* Only unreferenced aliases are left, swap and free the unused ones. */
    free_aliases(parse_tree->aliases);
    parse_tree->aliases = used_aliases;

    debug_return;
}

/*
 * Prune out non-matching entries from user and host aliases.
 */
static int
alias_prune_helper(struct sudoers_parse_tree *parse_tree, struct alias *a,
    void *v)
{
    struct cvtsudoers_config *conf = v;

    /* XXX - misuse of these functions */
    switch (a->type) {
    case USERALIAS:
	userlist_matches_filter(parse_tree, &a->members, conf);
	break;
    case HOSTALIAS:
	hostlist_matches_filter(parse_tree, &a->members, conf);
	break;
    default:
	break;
    }

    return 0;
}

/*
 * Prune out non-matching entries from within aliases.
 */
static void
alias_prune(struct sudoers_parse_tree *parse_tree,
    struct cvtsudoers_config *conf)
{
    debug_decl(alias_prune, SUDOERS_DEBUG_ALIAS);

    alias_apply(parse_tree, alias_prune_helper, conf);

    debug_return;
}

/*
 * Convert back to sudoers.
 */
static bool
convert_sudoers_sudoers(struct sudoers_parse_tree *parse_tree,
    const char *output_file, struct cvtsudoers_config *conf)
{
    bool ret = true;
    struct sudo_lbuf lbuf;
    debug_decl(convert_sudoers_sudoers, SUDOERS_DEBUG_UTIL);

    if (strcmp(output_file, "-") == 0) {
	output_fp = stdout;
    } else {
	if ((output_fp = fopen(output_file, "w")) == NULL)
	    sudo_fatal(U_("unable to open %s"), output_file);
    }

    /* Wrap lines at 80 columns with a 4 character indent. */
    sudo_lbuf_init(&lbuf, convert_sudoers_output, 4, "\\", 80);

    /* Print Defaults */
    if (!ISSET(conf->suppress, SUPPRESS_DEFAULTS)) {
	if (!print_defaults_sudoers(parse_tree, &lbuf, conf->expand_aliases))
	    goto done;
	if (lbuf.len > 0) {
	    sudo_lbuf_print(&lbuf);
	    sudo_lbuf_append(&lbuf, "\n");
	}
    }

    /* Print Aliases */
    if (!conf->expand_aliases && !ISSET(conf->suppress, SUPPRESS_ALIASES)) {
	if (!print_aliases_sudoers(parse_tree, &lbuf))
	    goto done;
	if (lbuf.len > 1) {
	    sudo_lbuf_print(&lbuf);
	    sudo_lbuf_append(&lbuf, "\n");
	}
    }

    /* Print User_Specs, separated by blank lines. */
    if (!ISSET(conf->suppress, SUPPRESS_PRIVS)) {
	if (!sudoers_format_userspecs(&lbuf, parse_tree, "\n",
	    conf->expand_aliases, true)) {
	    goto done;
	}
	if (lbuf.len > 1) {
	    sudo_lbuf_print(&lbuf);
	}
    }

done:
    if (sudo_lbuf_error(&lbuf)) {
	if (errno == ENOMEM)
	    sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	ret = false;
    }
    sudo_lbuf_destroy(&lbuf);

    (void)fflush(output_fp);
    if (ferror(output_fp)) {
	sudo_warn(U_("unable to write to %s"), output_file);
	ret = false;
    }
    if (output_fp != stdout)
	fclose(output_fp);

    debug_return_bool(ret);
}

static void
display_usage(FILE *fp)
{
    (void) fprintf(fp, "usage: %s [-ehMpV] [-b dn] "
	"[-c conf_file ] [-d deftypes] [-f output_format] [-i input_format] "
	"[-I increment] [-m filter] [-o output_file] [-O start_point] "
	"[-P padding] [-s sections] [input_file]\n", getprogname());
}

sudo_noreturn static void
usage(void)
{
    display_usage(stderr);
    exit(EXIT_FAILURE);
}

sudo_noreturn static void
help(void)
{
    (void) printf(_("%s - convert between sudoers file formats\n\n"), getprogname());
    display_usage(stdout);
    (void) puts(_("\nOptions:\n"
	"  -b, --base=dn              the base DN for sudo LDAP queries\n"
	"  -c, --config=conf_file     the path to the configuration file\n"
	"  -d, --defaults=deftypes    only convert Defaults of the specified types\n"
	"  -e, --expand-aliases       expand aliases when converting\n"
	"  -f, --output-format=format set output format: JSON, LDIF or sudoers\n"
	"  -i, --input-format=format  set input format: LDIF or sudoers\n"
	"  -I, --increment=num        amount to increase each sudoOrder by\n"
	"  -h, --help                 display help message and exit\n"
	"  -m, --match=filter         only convert entries that match the filter\n"
	"  -M, --match-local          match filter uses passwd and group databases\n"
	"  -o, --output=output_file   write converted sudoers to output_file\n"
	"  -O, --order-start=num      starting point for first sudoOrder\n"
	"  -p, --prune-matches        prune non-matching users, groups and hosts\n"
	"  -P, --padding=num          base padding for sudoOrder increment\n"
	"  -s, --suppress=sections    suppress output of certain sections\n"
	"  -V, --version              display version information and exit"));
    exit(EXIT_SUCCESS);
}
