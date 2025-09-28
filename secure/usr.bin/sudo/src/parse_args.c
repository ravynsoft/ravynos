/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1993-1996, 1998-2023 Todd C. Miller <Todd.Miller@sudo.ws>
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

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#ifdef HAVE_GETOPT_LONG
# include <getopt.h>
# else
# include <compat/getopt.h>
#endif /* HAVE_GETOPT_LONG */

#include <sudo_usage.h>
#include <sudo.h>
#include <sudo_lbuf.h>

unsigned int tgetpass_flags;

/*
 * Local functions.
 */
sudo_noreturn static void help(void);
sudo_noreturn static void usage_excl(void);
sudo_noreturn static void usage_excl_ticket(void);

/*
 * Mapping of command line options to name/value settings.
 * Do not reorder, indexes must match ARG_ defines in sudo.h.
 */
static struct sudo_settings sudo_settings[] = {
    { "bsdauth_type" },
    { "login_class" },
    { "preserve_environment" },
    { "runas_group" },
    { "set_home" },
    { "run_shell" },
    { "login_shell" },
    { "ignore_ticket" },
    { "update_ticket" },
    { "prompt" },
    { "selinux_role" },
    { "selinux_type" },
    { "runas_user" },
    { "progname" },
    { "implied_shell" },
    { "preserve_groups" },
    { "noninteractive" },
    { "sudoedit" },
    { "closefrom" },
    { "network_addrs" },
    { "max_groups" },
    { "plugin_dir" },
    { "remote_host" },
    { "timeout" },
    { "cmnd_chroot" },
    { "cmnd_cwd" },
    { "askpass" },
    { "intercept_setid" },
    { "intercept_ptrace" },
    { "apparmor_profile" },
    { NULL }
};

struct environment {
    char **envp;		/* pointer to the new environment */
    size_t env_size;		/* size of new_environ in char **'s */
    size_t env_len;		/* number of slots used, not counting NULL */
};

/*
 * Default flags allowed when running a command.
 */
#define DEFAULT_VALID_FLAGS	(MODE_BACKGROUND|MODE_PRESERVE_ENV|MODE_RESET_HOME|MODE_LOGIN_SHELL|MODE_NONINTERACTIVE|MODE_PRESERVE_GROUPS|MODE_SHELL)
#define EDIT_VALID_FLAGS	MODE_NONINTERACTIVE
#define LIST_VALID_FLAGS	(MODE_NONINTERACTIVE|MODE_LONG_LIST)
#define VALIDATE_VALID_FLAGS	MODE_NONINTERACTIVE

/* Option number for the --host long option due to ambiguity of the -h flag. */
#define OPT_HOSTNAME	256

/*
 * Available command line options, both short and long.
 * Note that we must disable arg permutation to support setting environment
 * variables and to better support the optional arg of the -h flag.
 * There is a more limited set of options for sudoedit (the sudo-specific
 * long options are listed first).
 */
static const char sudo_short_opts[] = "+Aa:BbC:c:D:Eeg:Hh::iKklNnPp:R:r:SsT:t:U:u:Vv";
static const char edit_short_opts[] = "+Aa:BC:c:D:g:h::KkNnp:R:r:ST:t:u:V";
static struct option sudo_long_opts[] = {
    /* sudo-specific long options */
    { "background",	no_argument,		NULL,	'b' },
    { "preserve-env",	optional_argument,	NULL,	'E' },
    { "edit",		no_argument,		NULL,	'e' },
    { "set-home",	no_argument,		NULL,	'H' },
    { "login",		no_argument,		NULL,	'i' },
    { "remove-timestamp", no_argument,		NULL,	'K' },
    { "list",		no_argument,		NULL,	'l' },
    { "preserve-groups", no_argument,		NULL,	'P' },
    { "shell",		no_argument,		NULL,	's' },
    { "other-user",	required_argument,	NULL,	'U' },
    { "validate",	no_argument,		NULL,	'v' },
    /* common long options */
    { "askpass",	no_argument,		NULL,	'A' },
    { "auth-type",	required_argument,	NULL,	'a' },
    { "bell",	        no_argument,		NULL,	'B' },
    { "close-from",	required_argument,	NULL,	'C' },
    { "login-class",	required_argument,	NULL,	'c' },
    { "chdir",		required_argument,	NULL,	'D' },
    { "group",		required_argument,	NULL,	'g' },
    { "help",		no_argument,		NULL,	'h' },
    { "host",		required_argument,	NULL,	OPT_HOSTNAME },
    { "reset-timestamp", no_argument,		NULL,	'k' },
    { "no-update",	no_argument,		NULL,	'N' },
    { "non-interactive", no_argument,		NULL,	'n' },
    { "prompt",		required_argument,	NULL,	'p' },
    { "chroot",		required_argument,	NULL,	'R' },
    { "role",		required_argument,	NULL,	'r' },
    { "stdin",		no_argument,		NULL,	'S' },
    { "command-timeout",required_argument,	NULL,	'T' },
    { "type",		required_argument,	NULL,	't' },
    { "user",		required_argument,	NULL,	'u' },
    { "version",	no_argument,		NULL,	'V' },
    { NULL,		no_argument,		NULL,	'\0' },
};
static struct option *edit_long_opts = &sudo_long_opts[11];

/*
 * Insert a key=value pair into the specified environment.
 */
static void
env_insert(struct environment *e, char *pair)
{
    debug_decl(env_insert, SUDO_DEBUG_ARGS);

    /* Make sure we have at least two slots free (one for NULL). */
    if (e->env_len + 1 >= e->env_size) {
	char **tmp;

	if (e->env_size == 0)
	    e->env_size = 16;
	tmp = reallocarray(e->envp, e->env_size, 2 * sizeof(char *));
	if (tmp == NULL)
	    sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	e->envp = tmp;
	e->env_size *= 2;
    }
    e->envp[e->env_len++] = pair;
    e->envp[e->env_len] = NULL;

    debug_return;
}

/*
 * Format as var=val and insert into the specified environment.
 */
static void
env_set(struct environment *e, char *var, char *val)
{
    char *pair;
    debug_decl(env_set, SUDO_DEBUG_ARGS);

    pair = sudo_new_key_val(var, val);
    if (pair == NULL) {
	sudo_fatalx(U_("%s: %s"),
	    __func__, U_("unable to allocate memory"));
    }
    env_insert(e, pair);

    debug_return;
}

/*
 * Parse a comma-separated list of env vars and add to the
 * specified environment.
 */
static void
parse_env_list(struct environment *e, char *list)
{
    char *cp, *last, *val;
    debug_decl(parse_env_list, SUDO_DEBUG_ARGS);

    for ((cp = strtok_r(list, ",", &last)); cp != NULL;
	(cp = strtok_r(NULL, ",", &last))) {
	if (strchr(cp, '=') != NULL) {
	    sudo_warnx(U_("invalid environment variable name: %s"), cp);
	    usage();
	}
	if ((val = getenv(cp)) != NULL)
	    env_set(e, cp, val);
    }
    debug_return;
}

/*
 * Command line argument parsing.
 * Sets nargc and nargv which corresponds to the argc/argv we'll use
 * for the command to be run (if we are running one).
 */
unsigned int
parse_args(int argc, char **argv, const char *shell, int *old_optind,
    int *nargc, char ***nargv, struct sudo_settings **settingsp,
    char ***env_addp, const char **list_userp)
{
    const char *progname, *short_opts = sudo_short_opts;
    struct option *long_opts = sudo_long_opts;
    struct environment extra_env;
    const char *list_user = NULL;
    unsigned int mode = 0;	/* what mode is sudo to be run in? */
    unsigned int flags = 0;	/* mode flags */
    unsigned int valid_flags = DEFAULT_VALID_FLAGS;
    int ch, i;
    char *cp;
    debug_decl(parse_args, SUDO_DEBUG_ARGS);

    /* Is someone trying something funny? */
    if (argc <= 0)
	usage();

    /* The plugin API includes the program name (either sudo or sudoedit). */
    progname = getprogname();
    sudo_settings[ARG_PROGNAME].value = progname;

    /* First, check to see if we were invoked as "sudoedit". */
    if (strcmp(progname, "sudoedit") == 0) {
	mode = MODE_EDIT;
	sudo_settings[ARG_SUDOEDIT].value = "true";
	valid_flags = EDIT_VALID_FLAGS;
	short_opts = edit_short_opts;
	long_opts = edit_long_opts;
    }

    /* Load local IP addresses and masks. */
    if (get_net_ifs(&cp) > 0)
	sudo_settings[ARG_NET_ADDRS].value = cp;

    /* Set max_groups from sudo.conf. */
    i = sudo_conf_max_groups();
    if (i != -1) {
	if (asprintf(&cp, "%d", i) == -1)
	    sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	sudo_settings[ARG_MAX_GROUPS].value = cp;
    }

    /* Returns true if the last option string was "-h" */
#define got_host_flag	(optind > 1 && argv[optind - 1][0] == '-' && \
	    argv[optind - 1][1] == 'h' && argv[optind - 1][2] == '\0')

    /* Returns true if the last option string was "--" */
#define got_end_of_args	(optind > 1 && argv[optind - 1][0] == '-' && \
	    argv[optind - 1][1] == '-' && argv[optind - 1][2] == '\0')

    /* Returns true if next option is an environment variable */
#define is_envar (optind < argc && argv[optind][0] != '/' && \
	    argv[optind][0] != '=' && strchr(argv[optind], '=') != NULL)

    /* Space for environment variables is lazy allocated. */
    memset(&extra_env, 0, sizeof(extra_env));

    for (;;) {
	/*
	 * Some trickiness is required to allow environment variables
	 * to be interspersed with command line options.
	 */
	if ((ch = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
	    switch (ch) {
		case 'A':
		    SET(tgetpass_flags, TGP_ASKPASS);
		    sudo_settings[ARG_ASKPASS].value = "true";
		    break;
#ifdef HAVE_BSD_AUTH_H
		case 'a':
		    assert(optarg != NULL);
		    if (*optarg == '\0')
			usage();
		    if (sudo_settings[ARG_BSDAUTH_TYPE].value != NULL)
			usage();
		    sudo_settings[ARG_BSDAUTH_TYPE].value = optarg;
		    break;
#endif
		case 'b':
		    SET(flags, MODE_BACKGROUND);
		    break;
		case 'B':
		    SET(tgetpass_flags, TGP_BELL);
		    break;
		case 'C':
		    assert(optarg != NULL);
		    if (sudo_strtonum(optarg, 3, INT_MAX, NULL) == 0) {
			sudo_warnx("%s",
			    U_("the argument to -C must be a number greater than or equal to 3"));
			usage();
		    }
		    if (sudo_settings[ARG_CLOSEFROM].value != NULL)
			usage();
		    sudo_settings[ARG_CLOSEFROM].value = optarg;
		    break;
#ifdef HAVE_LOGIN_CAP_H
		case 'c':
		    assert(optarg != NULL);
		    if (*optarg == '\0')
			usage();
		    if (sudo_settings[ARG_LOGIN_CLASS].value != NULL)
			usage();
		    sudo_settings[ARG_LOGIN_CLASS].value = optarg;
		    break;
#endif
		case 'D':
		    assert(optarg != NULL);
		    if (*optarg == '\0')
			usage();
		    if (sudo_settings[ARG_CWD].value != NULL)
			usage();
		    sudo_settings[ARG_CWD].value = optarg;
		    break;
		case 'E':
		    /*
		     * Optional argument is a comma-separated list of
		     * environment variables to preserve.
		     * If not present, preserve everything.
		     */
		    if (optarg == NULL) {
			sudo_settings[ARG_PRESERVE_ENVIRONMENT].value = "true";
			SET(flags, MODE_PRESERVE_ENV);
		    } else {
			parse_env_list(&extra_env, optarg);
		    }
		    break;
		case 'e':
		    if (mode && mode != MODE_EDIT)
			usage_excl();
		    mode = MODE_EDIT;
		    sudo_settings[ARG_SUDOEDIT].value = "true";
		    valid_flags = EDIT_VALID_FLAGS;
		    break;
		case 'g':
		    assert(optarg != NULL);
		    if (*optarg == '\0')
			usage();
		    if (sudo_settings[ARG_RUNAS_GROUP].value != NULL)
			usage();
		    sudo_settings[ARG_RUNAS_GROUP].value = optarg;
		    break;
		case 'H':
		    sudo_settings[ARG_SET_HOME].value = "true";
		    SET(flags, MODE_RESET_HOME);
		    break;
		case 'h':
		    if (optarg == NULL) {
			/*
			 * Optional args support -hhostname, not -h hostname.
			 * If we see a non-option after the -h flag, treat as
			 * remote host and bump optind to skip over it.
			 */
			if (got_host_flag && argv[optind] != NULL &&
			    argv[optind][0] != '-' && !is_envar) {
			    if (sudo_settings[ARG_REMOTE_HOST].value != NULL)
				usage();
			    sudo_settings[ARG_REMOTE_HOST].value = argv[optind++];
			    continue;
			}
			if (mode && mode != MODE_HELP) {
			    if (strcmp(progname, "sudoedit") != 0)
				usage_excl();
			}
			mode = MODE_HELP;
			valid_flags = 0;
			break;
		    }
		    FALLTHROUGH;
		case OPT_HOSTNAME:
		    assert(optarg != NULL);
		    if (*optarg == '\0')
			usage();
		    if (sudo_settings[ARG_REMOTE_HOST].value != NULL)
			usage();
		    sudo_settings[ARG_REMOTE_HOST].value = optarg;
		    break;
		case 'i':
		    sudo_settings[ARG_LOGIN_SHELL].value = "true";
		    SET(flags, MODE_LOGIN_SHELL);
		    break;
		case 'K':
		    if (mode && mode != MODE_KILL)
			usage_excl();
		    mode = MODE_KILL;
		    valid_flags = 0;
		    FALLTHROUGH;
		case 'k':
		    if (sudo_settings[ARG_UPDATE_TICKET].value != NULL)
			usage_excl_ticket();
		    sudo_settings[ARG_IGNORE_TICKET].value = "true";
		    break;
		case 'l':
		    if (mode) {
			if (mode == MODE_LIST)
			    SET(flags, MODE_LONG_LIST);
			else
			    usage_excl();
		    }
		    mode = MODE_LIST;
		    valid_flags = LIST_VALID_FLAGS;
		    break;
		case 'N':
		    if (sudo_settings[ARG_IGNORE_TICKET].value != NULL)
			usage_excl_ticket();
		    sudo_settings[ARG_UPDATE_TICKET].value = "false";
		    break;
		case 'n':
		    SET(flags, MODE_NONINTERACTIVE);
		    sudo_settings[ARG_NONINTERACTIVE].value = "true";
		    break;
		case 'P':
		    sudo_settings[ARG_PRESERVE_GROUPS].value = "true";
		    SET(flags, MODE_PRESERVE_GROUPS);
		    break;
		case 'p':
		    /* An empty prompt is allowed. */
		    assert(optarg != NULL);
		    if (sudo_settings[ARG_PROMPT].value != NULL)
			usage();
		    sudo_settings[ARG_PROMPT].value = optarg;
		    break;
		case 'R':
		    assert(optarg != NULL);
		    if (*optarg == '\0')
			usage();
		    if (sudo_settings[ARG_CHROOT].value != NULL)
			usage();
		    sudo_settings[ARG_CHROOT].value = optarg;
		    break;
#ifdef HAVE_SELINUX
		case 'r':
		    assert(optarg != NULL);
		    if (*optarg == '\0')
			usage();
		    if (sudo_settings[ARG_SELINUX_ROLE].value != NULL)
			usage();
		    sudo_settings[ARG_SELINUX_ROLE].value = optarg;
		    break;
		case 't':
		    assert(optarg != NULL);
		    if (*optarg == '\0')
			usage();
		    if (sudo_settings[ARG_SELINUX_TYPE].value != NULL)
			usage();
		    sudo_settings[ARG_SELINUX_TYPE].value = optarg;
		    break;
#endif
		case 'T':
		    /* Plugin determines whether empty timeout is allowed. */
		    assert(optarg != NULL);
		    if (sudo_settings[ARG_TIMEOUT].value != NULL)
			usage();
		    sudo_settings[ARG_TIMEOUT].value = optarg;
		    break;
		case 'S':
		    SET(tgetpass_flags, TGP_STDIN);
		    break;
		case 's':
		    sudo_settings[ARG_USER_SHELL].value = "true";
		    SET(flags, MODE_SHELL);
		    break;
		case 'U':
		    assert(optarg != NULL);
		    if (list_user != NULL || *optarg == '\0')
			usage();
		    list_user = optarg;
		    break;
		case 'u':
		    assert(optarg != NULL);
		    if (*optarg == '\0')
			usage();
		    if (sudo_settings[ARG_RUNAS_USER].value != NULL)
			usage();
		    sudo_settings[ARG_RUNAS_USER].value = optarg;
		    break;
		case 'v':
		    if (mode && mode != MODE_VALIDATE)
			usage_excl();
		    mode = MODE_VALIDATE;
		    valid_flags = VALIDATE_VALID_FLAGS;
		    break;
		case 'V':
		    if (mode && mode != MODE_VERSION) {
			if (strcmp(progname, "sudoedit") != 0)
			    usage_excl();
		    }
		    mode = MODE_VERSION;
		    valid_flags = 0;
		    break;
		default:
		    usage();
	    }
	} else if (!got_end_of_args && is_envar) {
	    /* Insert key=value pair, crank optind and resume getopt. */
	    env_insert(&extra_env, argv[optind]);
	    optind++;
	} else {
	    /* Not an option or an environment variable -- we're done. */
	    break;
	}
    }

    argc -= optind;
    argv += optind;
    *old_optind = optind;

    if (!mode) {
	/* Defer -k mode setting until we know whether it is a flag or not */
	if (sudo_settings[ARG_IGNORE_TICKET].value != NULL) {
	    if (argc == 0 && !ISSET(flags, MODE_SHELL|MODE_LOGIN_SHELL)) {
		mode = MODE_INVALIDATE;	/* -k by itself */
		sudo_settings[ARG_IGNORE_TICKET].value = NULL;
		valid_flags = 0;
	    }
	}
	if (!mode)
	    mode = MODE_RUN;		/* running a command */
    }

    if (argc > 0 && mode == MODE_LIST)
	mode = MODE_CHECK;

    if (ISSET(flags, MODE_LOGIN_SHELL)) {
	if (ISSET(flags, MODE_SHELL)) {
	    sudo_warnx("%s",
		U_("you may not specify both the -i and -s options"));
	    usage();
	}
	if (ISSET(flags, MODE_PRESERVE_ENV)) {
	    sudo_warnx("%s",
		U_("you may not specify both the -i and -E options"));
	    usage();
	}
	SET(flags, MODE_SHELL);
    }
    if ((flags & valid_flags) != flags)
	usage();
    if (mode == MODE_EDIT &&
       (ISSET(flags, MODE_PRESERVE_ENV) || extra_env.env_len != 0)) {
	if (ISSET(mode, MODE_PRESERVE_ENV))
	    sudo_warnx("%s", U_("the -E option is not valid in edit mode"));
	if (extra_env.env_len != 0)
	    sudo_warnx("%s",
		U_("you may not specify environment variables in edit mode"));
	usage();
    }
    if ((sudo_settings[ARG_RUNAS_USER].value != NULL ||
	 sudo_settings[ARG_RUNAS_GROUP].value != NULL) &&
	!ISSET(mode, MODE_EDIT | MODE_RUN | MODE_CHECK | MODE_VALIDATE)) {
	usage();
    }
    if (list_user != NULL && mode != MODE_LIST && mode != MODE_CHECK) {
	sudo_warnx("%s",
	    U_("the -U option may only be used with the -l option"));
	usage();
    }
    if (ISSET(tgetpass_flags, TGP_STDIN) && ISSET(tgetpass_flags, TGP_ASKPASS)) {
	sudo_warnx("%s", U_("the -A and -S options may not be used together"));
	usage();
    }
    if ((argc == 0 && mode == MODE_EDIT) ||
	(argc > 0 && !ISSET(mode, MODE_RUN | MODE_EDIT | MODE_CHECK)))
	usage();
    if (argc == 0 && mode == MODE_RUN && !ISSET(flags, MODE_SHELL)) {
	SET(flags, (MODE_IMPLIED_SHELL | MODE_SHELL));
	sudo_settings[ARG_IMPLIED_SHELL].value = "true";
    }
#ifdef ENABLE_SUDO_PLUGIN_API
    sudo_settings[ARG_PLUGIN_DIR].value = sudo_conf_plugin_dir_path();
#endif
    if (exec_ptrace_intercept_supported())
	sudo_settings[ARG_INTERCEPT_SETID].value = "true";
    if (exec_ptrace_subcmds_supported())
	sudo_settings[ARG_INTERCEPT_PTRACE].value = "true";

    if (mode == MODE_HELP)
	help();

    /*
     * For shell mode we need to rewrite argv
     * TODO: move this to the policy plugin and make escaping configurable
     */
    if (ISSET(flags, MODE_SHELL|MODE_LOGIN_SHELL) && ISSET(mode, MODE_RUN)) {
	char **av, *cmnd = NULL;
	int ac = 1;

	if (argc != 0) {
	    /* shell -c "command" */
	    char *src, *dst;
	    size_t size = 0;

	    for (av = argv; *av != NULL; av++)
		size += strlen(*av) + 1;
	    if (size == 0 || (cmnd = reallocarray(NULL, size, 2)) == NULL)
		sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    if (!gc_add(GC_PTR, cmnd))
		exit(EXIT_FAILURE);

	    for (dst = cmnd, av = argv; *av != NULL; av++) {
		for (src = *av; *src != '\0'; src++) {
		    /* quote potential meta characters */
		    if (!isalnum((unsigned char)*src) && *src != '_' && *src != '-' && *src != '$')
			*dst++ = '\\';
		    *dst++ = *src;
		}
		*dst++ = ' ';
	    }
	    if (cmnd != dst)
		dst--;  /* replace last space with a NUL */
	    *dst = '\0';

	    ac += 2; /* -c cmnd */
	}

	av = reallocarray(NULL, (size_t)ac + 1, sizeof(char *));
	if (av == NULL)
	    sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	if (!gc_add(GC_PTR, av))
	    exit(EXIT_FAILURE);

	av[0] = (char *)shell;	/* plugin may override shell */
	if (cmnd != NULL) {
	    av[1] = (char *)"-c";
	    av[2] = cmnd;
	}
	av[ac] = NULL;

	argv = av;
	argc = ac;
    }

    /*
     * For sudoedit we need to rewrite argv
     */
    if (mode == MODE_EDIT) {
#if defined(HAVE_SETRESUID) || defined(HAVE_SETREUID) || defined(HAVE_SETEUID)
	char **av;
	int ac;

	av = reallocarray(NULL, (size_t)argc + 2, sizeof(char *));
	if (av == NULL)
	    sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	if (!gc_add(GC_PTR, av))
	    exit(EXIT_FAILURE);

	/* Must have the command in argv[0]. */
	av[0] = (char *)"sudoedit";
	for (ac = 0; argv[ac] != NULL; ac++) {
	    av[ac + 1] = argv[ac];
	}
	av[++ac] = NULL;

	argv = av;
	argc = ac;
#else
	sudo_fatalx("%s", U_("sudoedit is not supported on this platform"));
#endif
    }

    *settingsp = sudo_settings;
    *env_addp = extra_env.envp;
    *nargc = argc;
    *nargv = argv;
    *list_userp = list_user;
    debug_return_uint(mode | flags);
}

/*
 * Display usage message.
 * The actual usage strings are in sudo_usage.h for configure substitution.
 */
static void
display_usage(FILE *fp)
{
    const char * const **uvecs = sudo_usage;
    const char * const *uvec;
    size_t i;
    int indent;

    /*
     * Use usage vectors appropriate to the progname.
     */
    if (strcmp(getprogname(), "sudoedit") == 0)
	uvecs = sudoedit_usage;

    indent = (int)strlen(getprogname()) + 8;
    while ((uvec = *uvecs) != NULL) {
	(void)fprintf(fp, "usage: %s %s\n", getprogname(), uvec[0]);
	for (i = 1; uvec[i] != NULL; i++) {
	    (void)fprintf(fp, "%*s%s\n", indent, "", uvec[i]);
	}
	uvecs++;
    }
}

/*
 * Display usage message and exit.
 */
sudo_noreturn void
usage(void)
{
    display_usage(stderr);
    exit(EXIT_FAILURE);
}

/*
 * Tell which options are mutually exclusive and exit.
 */
static void
usage_excl(void)
{
    debug_decl(usage_excl, SUDO_DEBUG_ARGS);

    sudo_warnx("%s",
	U_("Only one of the -e, -h, -i, -K, -l, -s, -v or -V options may be specified"));
    usage();
}

/*
 * Tell which options are mutually exclusive and exit.
 */
static void
usage_excl_ticket(void)
{
    debug_decl(usage_excl_ticket, SUDO_DEBUG_ARGS);

    sudo_warnx("%s",
	U_("Only one of the -K, -k or -N options may be specified"));
    usage();
}

static int
help_out(const char * restrict buf)
{
    return fputs(buf, stdout);
}

sudo_noreturn static void
help(void)
{
    struct sudo_lbuf lbuf;
    const int indent = 32;
    const char *pname = getprogname();
    bool sudoedit = false;
    debug_decl(help, SUDO_DEBUG_ARGS);

    if (strcmp(pname, "sudoedit") == 0) {
	sudoedit = true;
	(void)printf(_("%s - edit files as another user\n\n"), pname);
    } else {
	(void)printf(_("%s - execute a command as another user\n\n"), pname);
    }
    display_usage(stdout);

    sudo_lbuf_init(&lbuf, help_out, indent, NULL, 80);
    sudo_lbuf_append(&lbuf, "%s", _("\nOptions:\n"));
    sudo_lbuf_append(&lbuf, "  -A, --askpass                 %s\n",
	_("use a helper program for password prompting"));
#ifdef HAVE_BSD_AUTH_H
    sudo_lbuf_append(&lbuf, "  -a, --auth-type=type          %s\n",
	_("use specified BSD authentication type"));
#endif
    if (!sudoedit) {
	sudo_lbuf_append(&lbuf, "  -b, --background              %s\n",
	    _("run command in the background"));
    }
    sudo_lbuf_append(&lbuf, "  -B, --bell                    %s\n",
	_("ring bell when prompting"));
    sudo_lbuf_append(&lbuf, "  -C, --close-from=num          %s\n",
	_("close all file descriptors >= num"));
#ifdef HAVE_LOGIN_CAP_H
    sudo_lbuf_append(&lbuf, "  -c, --login-class=class       %s\n",
	_("run command with the specified BSD login class"));
#endif
    sudo_lbuf_append(&lbuf, "  -D, --chdir=directory         %s\n",
	_("change the working directory before running command"));
    if (!sudoedit) {
	sudo_lbuf_append(&lbuf, "  -E, --preserve-env            %s\n",
	    _("preserve user environment when running command"));
	sudo_lbuf_append(&lbuf, "      --preserve-env=list       %s\n",
	    _("preserve specific environment variables"));
	sudo_lbuf_append(&lbuf, "  -e, --edit                    %s\n",
	    _("edit files instead of running a command"));
    }
    sudo_lbuf_append(&lbuf, "  -g, --group=group             %s\n",
	_("run command as the specified group name or ID"));
    if (!sudoedit) {
	sudo_lbuf_append(&lbuf, "  -H, --set-home                %s\n",
	    _("set HOME variable to target user's home dir"));
    }
    sudo_lbuf_append(&lbuf, "  -h, --help                    %s\n",
	_("display help message and exit"));
    sudo_lbuf_append(&lbuf, "  -h, --host=host               %s\n",
	_("run command on host (if supported by plugin)"));
    if (!sudoedit) {
	sudo_lbuf_append(&lbuf, "  -i, --login                   %s\n",
	    _("run login shell as the target user; a command may also be specified"));
	sudo_lbuf_append(&lbuf, "  -K, --remove-timestamp        %s\n",
	    _("remove timestamp file completely"));
    }
    sudo_lbuf_append(&lbuf, "  -k, --reset-timestamp         %s\n",
	_("invalidate timestamp file"));
    if (!sudoedit) {
	sudo_lbuf_append(&lbuf, "  -l, --list                    %s\n",
	    _("list user's privileges or check a specific command; use twice for longer format"));
    }
    sudo_lbuf_append(&lbuf, "  -n, --non-interactive         %s\n",
	_("non-interactive mode, no prompts are used"));
    if (!sudoedit) {
	sudo_lbuf_append(&lbuf, "  -P, --preserve-groups         %s\n",
	    _("preserve group vector instead of setting to target's"));
    }
    sudo_lbuf_append(&lbuf, "  -p, --prompt=prompt           %s\n",
	_("use the specified password prompt"));
    sudo_lbuf_append(&lbuf, "  -R, --chroot=directory        %s\n",
	_("change the root directory before running command"));
#ifdef HAVE_SELINUX
    sudo_lbuf_append(&lbuf, "  -r, --role=role               %s\n",
	_("create SELinux security context with specified role"));
#endif
    sudo_lbuf_append(&lbuf, "  -S, --stdin                   %s\n",
	_("read password from standard input"));
    if (!sudoedit) {
	sudo_lbuf_append(&lbuf, "  -s, --shell                   %s\n",
	    _("run shell as the target user; a command may also be specified"));
    }
#ifdef HAVE_SELINUX
    sudo_lbuf_append(&lbuf, "  -t, --type=type               %s\n",
	_("create SELinux security context with specified type"));
#endif
    sudo_lbuf_append(&lbuf, "  -T, --command-timeout=timeout %s\n",
	_("terminate command after the specified time limit"));
    if (!sudoedit) {
	sudo_lbuf_append(&lbuf, "  -U, --other-user=user         %s\n",
	    _("in list mode, display privileges for user"));
    }
    sudo_lbuf_append(&lbuf, "  -u, --user=user               %s\n",
	_("run command (or edit file) as specified user name or ID"));
    sudo_lbuf_append(&lbuf, "  -V, --version                 %s\n",
	_("display version information and exit"));
    if (!sudoedit) {
	sudo_lbuf_append(&lbuf, "  -v, --validate                %s\n",
	    _("update user's timestamp without running a command"));
    }
    sudo_lbuf_append(&lbuf, "  --                            %s\n",
	_("stop processing command line arguments"));
    sudo_lbuf_print(&lbuf);
    sudo_lbuf_destroy(&lbuf);
    sudo_debug_exit_int(__func__, __FILE__, __LINE__, sudo_debug_subsys, 0);
    exit(EXIT_SUCCESS);
}
