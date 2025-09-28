/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1996, 1998-2005, 2007-2023
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

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

/*
 * Lock the sudoers file for safe editing (ala vipw) and check for parse errors.
 */

#ifdef __TANDEM
# include <floss.h>
#endif

#include <config.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#ifndef __TANDEM
# include <sys/file.h>
#endif
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef HAVE_GETOPT_LONG
# include <getopt.h>
# else
# include <compat/getopt.h>
#endif /* HAVE_GETOPT_LONG */

#include <sudoers.h>
#include <interfaces.h>
#include <redblack.h>
#include <sudoers_version.h>
#include <sudo_conf.h>
#include <gram.h>

struct sudoersfile {
    TAILQ_ENTRY(sudoersfile) entries;
    char *opath;	/* original path we opened */
    char *dpath;	/* destination path to write to */
    char *tpath;	/* editor temporary file path */
    bool created;	/* true if visudo created a new sudoers file */
    bool modified;	/* true if the user modified the file */
    bool doedit;	/* true when editing (not just checking) sudoers */
    int fd;		/* fd of the original file (if it exists) */
    int errorline;	/* line number when there is a syntax error */
};
TAILQ_HEAD(sudoersfile_list, sudoersfile);

/*
 * Function prototypes
 */
static void quit(int);
static int whatnow(void);
static char *get_editor(int *editor_argc, char ***editor_argv);
static bool check_syntax(struct sudoers_context *ctx, const char *, bool, bool);
static bool edit_sudoers(struct sudoersfile *, char *, int, char **, int);
static bool install_sudoers(struct sudoersfile *, bool, bool);
static bool visudo_track_error(const struct sudoers_context *ctx, const char *file, int line, int column, const char * restrict fmt, va_list args);
static int print_unused(struct sudoers_parse_tree *, struct alias *, void *);
static bool reparse_sudoers(struct sudoers_context *ctx, char *, int, char **);
static int run_command(const char *, char *const *);
static void parse_sudoers_options(struct sudoers_context *ctx);
static void setup_signals(void);
static void visudo_cleanup(void);
sudo_noreturn static void export_sudoers(const char *infile, const char *outfile);
sudo_noreturn static void help(void);
sudo_noreturn static void usage(void);

/*
 * Globals
 */
static const char *path_sudoers = _PATH_SUDOERS;
static struct sudoersfile_list sudoerslist = TAILQ_HEAD_INITIALIZER(sudoerslist);
static bool checkonly;
static bool edit_includes = true;
static unsigned int errors;
static const char short_opts[] =  "cf:hIOPqsVx:";
static struct option long_opts[] = {
    { "check",		no_argument,		NULL,	'c' },
    { "export",		required_argument,	NULL,	'x' },
    { "file",		required_argument,	NULL,	'f' },
    { "help",		no_argument,		NULL,	'h' },
    { "no-includes",	no_argument,		NULL,	'I' },
    { "owner",		no_argument,		NULL,	'O' },
    { "perms",		no_argument,		NULL,	'P' },
    { "quiet",		no_argument,		NULL,	'q' },
    { "strict",		no_argument,		NULL,	's' },
    { "version",	no_argument,		NULL,	'V' },
    { NULL,		no_argument,		NULL,	'\0' },
};

sudo_dso_public int main(int argc, char *argv[]);

int
main(int argc, char *argv[])
{
    struct sudoers_context ctx = SUDOERS_CONTEXT_INITIALIZER;
    struct sudoersfile *sp;
    char *editor, **editor_argv;
    const char *export_path = NULL;
    int ch, oldlocale, editor_argc, exitcode = 0;
    bool use_perms, use_owner, fflag;
    debug_decl(main, SUDOERS_DEBUG_MAIN);

#if defined(SUDO_DEVEL) && defined(__OpenBSD__)
    {
	extern char *malloc_options;
	malloc_options = "S";
    }
#endif

    initprogname(argc > 0 ? argv[0] : "visudo");
    if (!sudoers_initlocale(setlocale(LC_ALL, ""), def_sudoers_locale))
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    sudo_warn_set_locale_func(sudoers_warn_setlocale);
    bindtextdomain("sudoers", LOCALEDIR); /* XXX - should have visudo domain */
    textdomain("sudoers");

    if (argc < 1)
	usage();

    /* Register callbacks. */
    sudo_fatal_callback_register(visudo_cleanup);
    set_callbacks();

    /* Read debug and plugin sections of sudo.conf. */
    if (sudo_conf_read(NULL, SUDO_CONF_DEBUG|SUDO_CONF_PLUGINS) == -1)
	return EXIT_FAILURE;

    /* Initialize the debug subsystem. */
    if (!sudoers_debug_register(getprogname(), sudo_conf_debug_files(getprogname())))
	return EXIT_FAILURE;

    /* Parse sudoers plugin options, if any. */
    parse_sudoers_options(&ctx);

    /*
     * Arg handling.
     */
    fflag = use_owner = use_perms = false;
    while ((ch = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
	switch (ch) {
	    case 'V':
		(void) printf(_("%s version %s\n"), getprogname(),
		    PACKAGE_VERSION);
		(void) printf(_("%s grammar version %d\n"), getprogname(),
		    SUDOERS_GRAMMAR_VERSION);
		goto done;
	    case 'c':
		checkonly = true;	/* check mode */
		break;
	    case 'f':
		path_sudoers = optarg;
		fflag = true;
		break;
	    case 'h':
		help();
		/* NOTREACHED */
	    case 'I':
		edit_includes = false;
		break;
	    case 'O':
		use_owner = true;	/* check/set owner */
		break;
	    case 'P':
		use_perms = true;	/* check/set perms */
		break;
	    case 's':
		ctx.parser_conf.strict = 2;
		break;
	    case 'q':
		ctx.parser_conf.verbose = 0;
		break;
	    case 'x':
		export_path = optarg;
		break;
	    default:
		usage();
	}
    }
    argc -= optind;
    argv += optind;

    /* Check for optional sudoers file argument. */
    switch (argc) {
    case 0:
	break;
    case 1:
	/* Only accept sudoers file if no -f was specified. */
	if (!fflag) {
	    path_sudoers = *argv;
	    fflag = true;
	}
	break;
    default:
	usage();
    }

    if (fflag) {
	/* Looser owner/permission checks for an uninstalled sudoers file. */
	if (!use_owner) {
	    ctx.parser_conf.sudoers_uid = (uid_t)-1;
	    ctx.parser_conf.sudoers_gid = (gid_t)-1;
	}
	if (!use_perms)
	    ctx.parser_conf.sudoers_mode |= S_IWUSR;
    } else {
	/* Check/set owner and mode for installed sudoers file. */
	use_owner = true;
	use_perms = true;
    }

    if (export_path != NULL) {
	/* Backward compatibility for the time being. */
	export_sudoers(path_sudoers, export_path);
    }

    /* Mock up a fake struct sudoers_user_context. */
    ctx.user.cmnd = ctx.user.cmnd_base = strdup("true");
    if (ctx.user.cmnd == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    if (geteuid() == 0) {
	const char *user = getenv("SUDO_USER");
	if (user != NULL && *user != '\0')
	    ctx.user.pw = sudo_getpwnam(user);
    }
    if (ctx.user.pw == NULL) {
	if ((ctx.user.pw = sudo_getpwuid(getuid())) == NULL)
	    sudo_fatalx(U_("you do not exist in the %s database"), "passwd");
    }
    if (!sudoers_sethost(&ctx, NULL, NULL))
	return EXIT_FAILURE;

    /* Hook the sudoers parser to track files with parse errors. */
    sudoers_error_hook = visudo_track_error;

    /* Setup defaults data structures. */
    if (!init_defaults())
	sudo_fatalx("%s", U_("unable to initialize sudoers default values"));

    if (checkonly) {
	exitcode = check_syntax(&ctx, path_sudoers, use_owner, use_perms)
	    ? 0 : 1;
	goto done;
    }

    /*
     * Parse the existing sudoers file(s) to highlight any existing
     * errors and to pull in editor and env_editor conf values.
     */
    ctx.parser_conf.sudoers_path = path_sudoers;
    init_parser(&ctx, NULL);
    if ((sudoersin = open_sudoers(path_sudoers, &sudoers, true, NULL)) == NULL)
	return EXIT_FAILURE;
    sudoers_setlocale(SUDOERS_LOCALE_SUDOERS, &oldlocale);
    (void) sudoersparse();
    (void) update_defaults(&ctx, &parsed_policy, NULL,
	SETDEF_GENERIC|SETDEF_HOST|SETDEF_USER, !ctx.parser_conf.verbose);
    sudoers_setlocale(oldlocale, NULL);

    editor = get_editor(&editor_argc, &editor_argv);

    /* Install signal handlers to clean up temp files if we are killed. */
    setup_signals();

    /* Edit the sudoers file(s) */
    TAILQ_FOREACH(sp, &sudoerslist, entries) {
	if (!sp->doedit)
	    continue;
	if (sp != TAILQ_FIRST(&sudoerslist)) {
	    printf(_("press return to edit %s: "), sp->opath);
	    while ((ch = getchar()) != EOF && ch != '\r' && ch != '\n')
		    continue;
	}
	edit_sudoers(sp, editor, editor_argc, editor_argv, -1);
    }

    /*
     * Check edited files for a parse error, re-edit any that fail
     * and install the edited files as needed.
     */
    if (reparse_sudoers(&ctx, editor, editor_argc, editor_argv)) {
	TAILQ_FOREACH(sp, &sudoerslist, entries) {
	    if (!install_sudoers(sp, use_owner, use_perms)) {
		if (sp->tpath != NULL) {
		    sudo_warnx(U_("contents of edit session left in %s"),
			sp->tpath);
		    free(sp->tpath);
		    sp->tpath = NULL;
		}
		exitcode = 1;
	    }
	}
    }
    free(editor);

done:
    sudoers_ctx_free(&ctx);
    sudo_debug_exit_int(__func__, __FILE__, __LINE__, sudo_debug_subsys, exitcode);
    return exitcode;
}

static bool
visudo_track_error(const struct sudoers_context *ctx, const char *file,
    int line, int column, const char * restrict fmt, va_list args)
{
    struct sudoersfile *sp;
    debug_decl(visudo_track_error, SUDOERS_DEBUG_UTIL);

    TAILQ_FOREACH(sp, &sudoerslist, entries) {
	if (sp->errorline > 0)
	    continue;		/* preserve the first error */

	if (strcmp(file, sp->opath) == 0 ||
		(sp->tpath != NULL && strcmp(file, sp->tpath) == 0)) {
	    sp->errorline = line;
	    break;
	}
    }
    errors++;

    debug_return_bool(true);
}

static char *
get_editor(int *editor_argc, char ***editor_argv)
{
    char *editor_path = NULL, **allowlist = NULL;
    const char *env_editor = NULL;
    static const char *files[] = { "+1", "sudoers" };
    size_t allowlist_len = 0;
    debug_decl(get_editor, SUDOERS_DEBUG_UTIL);

    /* Build up editor allowlist from def_editor unless env_editor is set. */
    if (!def_env_editor) {
	const char *cp, *ep;
	const char *def_editor_end = def_editor + strlen(def_editor);

	/* Count number of entries in allowlist and split into a list. */
	for (cp = sudo_strsplit(def_editor, def_editor_end, ":", &ep);
	    cp != NULL; cp = sudo_strsplit(NULL, def_editor_end, ":", &ep)) {
	    allowlist_len++;
	}
	allowlist = reallocarray(NULL, allowlist_len + 1, sizeof(char *));
	if (allowlist == NULL)
	    sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	allowlist_len = 0;
	for (cp = sudo_strsplit(def_editor, def_editor_end, ":", &ep);
	    cp != NULL; cp = sudo_strsplit(NULL, def_editor_end, ":", &ep)) {
	    allowlist[allowlist_len] = strndup(cp, (size_t)(ep - cp));
	    if (allowlist[allowlist_len] == NULL)
		sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	    allowlist_len++;
	}
	allowlist[allowlist_len] = NULL;
    }

    editor_path = find_editor(2, (char **)files, editor_argc, editor_argv,
	allowlist, &env_editor);
    if (editor_path == NULL) {
	if (def_env_editor && env_editor != NULL) {
	    /* We are honoring $EDITOR so this is a fatal error. */
	    if (errno == ENOENT) {
		sudo_warnx(U_("specified editor (%s) doesn't exist"),
		    env_editor);
	    }
	    exit(EXIT_FAILURE);
	}
	sudo_fatalx(U_("no editor found (editor path = %s)"), def_editor);
    }

    if (allowlist != NULL) {
	while (allowlist_len)
	    free(allowlist[--allowlist_len]);
	free(allowlist);
    }

    debug_return_str(editor_path);
}

/*
 * List of editors that support the "+lineno" command line syntax.
 * If an entry starts with '*' the tail end of the string is matched.
 * No other wild cards are supported.
 */
static const char *lineno_editors[] = {
    "*macs",
    "ee",
    "elvis",
    "ex",
    "joe",
    "jove",
    "mg",
    "nano",
    "nex",
    "nvi",
    "nvim",
    "o",
    "pico",
    "vi",
    "vile",
    "vim",
    "zile",
    NULL
};

/*
 * Check whether or not the specified editor matched lineno_editors[].
 * Returns true if yes, false if no.
 */
static bool
editor_supports_plus(const char *editor)
{
    const char *cp, *editor_base;
    const char **av;
    debug_decl(editor_supports_plus, SUDOERS_DEBUG_UTIL);

    editor_base = sudo_basename(editor);
    if (*editor_base == 'r')
	editor_base++;

    for (av = lineno_editors; (cp = *av) != NULL; av++) {
	/* We only handle a leading '*' wildcard. */
	if (*cp == '*') {
	    size_t blen = strlen(editor_base);
	    size_t clen = strlen(++cp);
	    if (blen >= clen) {
		if (strcmp(cp, editor_base + blen - clen) == 0)
		    break;
	    }
	} else if (strcmp(cp, editor_base) == 0)
	    break;
    }
    debug_return_bool(cp != NULL);
}

/*
 * Edit each sudoers file.
 * Returns true on success, else false.
 */
static bool
edit_sudoers(struct sudoersfile *sp, char *editor, int editor_argc,
    char **editor_argv, int lineno)
{
    int tfd;				/* sudoers temp file descriptor */
    bool modified;			/* was the file modified? */
    int ac;				/* argument count */
    char linestr[64];			/* string version of lineno */
    struct timespec ts, times[2];	/* time before and after edit */
    struct timespec orig_mtim;		/* starting mtime of sudoers file */
    off_t orig_size;			/* starting size of sudoers file */
    struct stat sb;			/* stat buffer */
    bool ret = false;			/* return value */
    debug_decl(edit_sudoers, SUDOERS_DEBUG_UTIL);

    if (fstat(sp->fd, &sb) == -1)
	sudo_fatal(U_("unable to stat %s"), sp->opath);
    orig_size = sb.st_size;
    mtim_get(&sb, orig_mtim);

    /* Create the temp file if needed and set timestamp. */
    if (sp->tpath == NULL) {
	if (asprintf(&sp->tpath, "%s.tmp", sp->dpath) == -1)
	    sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	tfd = open(sp->tpath, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU|S_IRUSR);
	if (tfd < 0)
	    sudo_fatal("%s", sp->tpath);

	/* Copy sp->opath -> sp->tpath and reset the mtime. */
	if (orig_size != 0) {
	    char buf[4096], lastch = '\0';
	    ssize_t nread;

	    (void) lseek(sp->fd, (off_t)0, SEEK_SET);
	    while ((nread = read(sp->fd, buf, sizeof(buf))) > 0) {
		if (write(tfd, buf, (size_t)nread) == -1)
		    sudo_fatal("%s", U_("write error"));
		lastch = buf[nread - 1];
	    }

	    /* Add missing newline at EOF if needed. */
	    if (lastch != '\n') {
		lastch = '\n';
		if (write(tfd, &lastch, 1) == -1)
		    sudo_fatal("%s", U_("write error"));
	    }
	}
	(void) close(tfd);
    }
    times[0].tv_sec = times[1].tv_sec = orig_mtim.tv_sec;
    times[0].tv_nsec = times[1].tv_nsec = orig_mtim.tv_nsec;
    (void) utimensat(AT_FDCWD, sp->tpath, times, 0);

    /* Disable +lineno if editor doesn't support it. */
    if (lineno > 0 && !editor_supports_plus(editor))
	lineno = -1;

    /*
     * The last 3 slots in the editor argv are: "-- +1 sudoers"
     * Replace those placeholders with the real values.
     */
    ac = editor_argc - 3;
    if (lineno > 0) {
	(void)snprintf(linestr, sizeof(linestr), "+%d", lineno);
	editor_argv[ac++] = linestr; // -V507
    }
    editor_argv[ac++] = (char *)"--";
    editor_argv[ac++] = sp->tpath;
    editor_argv[ac] = NULL;

    /*
     * Do the edit:
     *  We cannot check the editor's exit value against 0 since
     *  XPG4 specifies that vi's exit value is a function of the
     *  number of errors during editing (?!?!).
     */
    if (sudo_gettime_real(&times[0]) == -1) {
	sudo_warn("%s", U_("unable to read the clock"));
	goto done;
    }

    if (run_command(editor, editor_argv) != -1) {
	if (sudo_gettime_real(&times[1]) == -1) {
	    sudo_warn("%s", U_("unable to read the clock"));
	    goto done;
	}
	/*
	 * Check for zero length sudoers file.
	 */
	if (stat(sp->tpath, &sb) == -1) {
	    sudo_warnx(U_("unable to stat temporary file (%s), %s unchanged"),
		sp->tpath, sp->opath);
	    goto done;
	}
	if (sb.st_size == 0 && orig_size != 0) {
	    /* Avoid accidental zeroing of main sudoers file. */
	    if (sp == TAILQ_FIRST(&sudoerslist)) {
		sudo_warnx(U_("zero length temporary file (%s), %s unchanged"),
		    sp->tpath, sp->opath);
		goto done;
	    }
	}
    } else {
	sudo_warnx(U_("editor (%s) failed, %s unchanged"), editor, sp->opath);
	goto done;
    }

    /* Set modified bit if the user changed the file. */
    modified = true;
    mtim_get(&sb, ts);
    if (orig_size == sb.st_size && sudo_timespeccmp(&orig_mtim, &ts, ==)) {
	/*
	 * If mtime and size match but the user spent no measurable
	 * time in the editor we can't tell if the file was changed.
	 */
	if (sudo_timespeccmp(&times[0], &times[1], !=))
	    modified = false;
    }

    /*
     * If modified in this edit session, mark as modified.
     */
    if (modified) {
	sp->modified = modified;
    } else {
	/*
	 * If visudo created a new, zero-length file that the user did
	 * not modify, remove it as long as it is the main sudoers file.
	 * We must retain new files created via a @include directive
	 * to avoid a parse error, even if they are empty.
	 */
	if (sp->created && orig_size == 0) {
	    if (sp == TAILQ_FIRST(&sudoerslist))
		unlink(sp->dpath);
	}
	sudo_warnx(U_("%s unchanged"), sp->tpath);
    }

    ret = true;
done:
    debug_return_bool(ret);
}

/*
 * Check Defaults and Alias entries.
 * On error, visudo_track_error() will set the line number in sudoerslist.
 */
static void
check_defaults_and_aliases(struct sudoers_context *ctx)
{
    const bool quiet = !ctx->parser_conf.verbose;
    const bool strict = ctx->parser_conf.strict > 1;
    debug_decl(check_defaults_and_aliases, SUDOERS_DEBUG_UTIL);

    if (!check_defaults(&parsed_policy, quiet)) {
	parse_error = true;
    }
    if (check_aliases(&parsed_policy, strict, quiet, print_unused) != 0) {
	parse_error = true;
    }
    debug_return;
}

/*
 * Parse sudoers after editing and re-edit any ones that caused a parse error.
 */
static bool
reparse_sudoers(struct sudoers_context *ctx, char *editor, int editor_argc,
    char **editor_argv)
{
    struct sudoersfile *sp, *last;
    FILE *fp;
    int ch, oldlocale;
    debug_decl(reparse_sudoers, SUDOERS_DEBUG_UTIL);

    /*
     * Parse the edited sudoers files.
     */
    errors = 0;
    while ((sp = TAILQ_FIRST(&sudoerslist)) != NULL) {
	last = TAILQ_LAST(&sudoerslist, sudoersfile_list);
	fp = fopen(sp->tpath, "r+");
	if (fp == NULL)
	    sudo_fatalx(U_("unable to re-open temporary file (%s), %s unchanged."),
		sp->tpath, sp->opath);

	/* Clean slate for each parse */
	if (!init_defaults())
	    sudo_fatalx("%s", U_("unable to initialize sudoers default values"));
	init_parser(ctx, sp->opath);
	sp->errorline = -1;

	/* Parse the sudoers temp file(s) */
	sudoersrestart(fp);
	sudoers_setlocale(SUDOERS_LOCALE_SUDOERS, &oldlocale);
	if (sudoersparse() && !parse_error) {
	    sudo_warnx(U_("unable to parse temporary file (%s), unknown error"),
		sp->tpath);
	    parse_error = true;
	}
	fclose(sudoersin);
	if (!parse_error) {
	    parse_error = !update_defaults(ctx, &parsed_policy, NULL,
		SETDEF_GENERIC|SETDEF_HOST|SETDEF_USER, true);
	    check_defaults_and_aliases(ctx);
	}
	sudoers_setlocale(oldlocale, NULL);

	/*
	 * Got an error, prompt the user for what to do now.
	 */
	if (parse_error) {
	    switch (whatnow()) {
	    case 'Q':
		parse_error = false;	/* ignore parse error */
		break;
	    case 'x':
		visudo_cleanup();	/* discard changes */
		debug_return_bool(false);
	    case 'e':
	    default:
		/* Edit file with the parse error */
		TAILQ_FOREACH(sp, &sudoerslist, entries) {
		    if (errors == 0 || sp->errorline > 0) {
			edit_sudoers(sp, editor, editor_argc, editor_argv,
			    sp->errorline);
		    }
		}
		break;
	    }
	}

	/* If any new #include directives were added, edit them too. */
	if ((sp = TAILQ_NEXT(last, entries)) != NULL) {
	    bool modified = false;
	    do {
		printf(_("press return to edit %s: "), sp->opath);
		while ((ch = getchar()) != EOF && ch != '\r' && ch != '\n')
			continue;
		edit_sudoers(sp, editor, editor_argc, editor_argv, -1);
		if (sp->modified)
		    modified = true;
	    } while ((sp = TAILQ_NEXT(sp, entries)) != NULL);

	    /* Reparse sudoers if newly added includes were modified. */
	    if (modified)
		continue;
	}

	/* If all sudoers files parsed OK we are done. */
	if (!parse_error)
	    break;
    }

    debug_return_bool(true);
}

/*
 * Set the owner and mode on a sudoers temp file and
 * move it into place.  Returns true on success, else false.
 */
static bool
install_sudoers(struct sudoersfile *sp, bool set_owner, bool set_mode)
{
    struct stat sb;
    bool ret = false;
    debug_decl(install_sudoers, SUDOERS_DEBUG_UTIL);

    if (sp->tpath == NULL) {
	ret = true;
	goto done;
    }

    if (!sp->modified) {
	/*
	 * No changes but fix owner/mode if needed.
	 */
	(void) unlink(sp->tpath);
	if (fstat(sp->fd, &sb) == 0) {
	    if (set_owner) {
		if (sb.st_uid != sudoers_file_uid() || sb.st_gid != sudoers_file_gid()) {
		    if (chown(sp->opath, sudoers_file_uid(), sudoers_file_gid()) != 0) {
			sudo_warn(U_("unable to set (uid, gid) of %s to (%u, %u)"),
			    sp->opath, (unsigned int)sudoers_file_uid(),
			    (unsigned int)sudoers_file_gid());
		    }
		}
	    }
	    if (set_mode) {
		if ((sb.st_mode & ACCESSPERMS) != sudoers_file_mode()) {
		    if (chmod(sp->opath, sudoers_file_mode()) != 0) {
			sudo_warn(U_("unable to change mode of %s to 0%o"),
			    sp->opath, (unsigned int)sudoers_file_mode());
		    }
		}
	    }
	}
	ret = true;
	goto done;
    }

    /*
     * Change mode and ownership of temp file before moving it into place
     * to avoid a race condition.
     */
    if (!set_owner || !set_mode) {
	/* Preserve owner/perms of the existing file.  */
	if (fstat(sp->fd, &sb) == -1)
	    sudo_fatal(U_("unable to stat %s"), sp->opath);
    }
    if (set_owner) {
	if (chown(sp->tpath, sudoers_file_uid(), sudoers_file_gid()) != 0) {
	    sudo_warn(U_("unable to set (uid, gid) of %s to (%u, %u)"),
		sp->tpath, (unsigned int)sudoers_file_uid(),
		(unsigned int)sudoers_file_gid());
	    goto done;
	}
    } else {
	if (chown(sp->tpath, sb.st_uid, sb.st_gid) != 0) {
	    sudo_warn(U_("unable to set (uid, gid) of %s to (%u, %u)"),
		sp->tpath, (unsigned int)sb.st_uid, (unsigned int)sb.st_gid);
	}
    }
    if (set_mode) {
	if (chmod(sp->tpath, sudoers_file_mode()) != 0) {
	    sudo_warn(U_("unable to change mode of %s to 0%o"), sp->tpath,
		(unsigned int)sudoers_file_mode());
	    goto done;
	}
    } else {
	if (chmod(sp->tpath, sb.st_mode & ACCESSPERMS) != 0) {
	    sudo_warn(U_("unable to change mode of %s to 0%o"), sp->tpath,
		(unsigned int)(sb.st_mode & ACCESSPERMS));
	}
    }

    /*
     * Now that we know sp->tpath parses correctly, it needs to be
     * rename(2)'d to sp->dpath.  If the rename(2) fails we try using
     * mv(1) in case sp->tpath and sp->dpath are on different file systems.
     */
    if (rename(sp->tpath, sp->dpath) == 0) {
	free(sp->tpath);
	sp->tpath = NULL;
    } else {
	if (errno == EXDEV) {
	    char *av[4];
	    sudo_warnx(U_("%s and %s not on the same file system, using mv to rename"),
	      sp->tpath, sp->dpath);

	    /* Build up argument vector for the command */
	    av[0] = sudo_basename(_PATH_MV);
	    av[1] = sp->tpath;
	    av[2] = sp->dpath;
	    av[3] = NULL;

	    /* And run it... */
	    if (run_command(_PATH_MV, av) != 0) {
		sudo_warnx(U_("command failed: '%s %s %s', %s unchanged"),
		    _PATH_MV, sp->tpath, sp->dpath, sp->opath);
		goto done;
	    }
	    free(sp->tpath);
	    sp->tpath = NULL;
	} else {
	    sudo_warn(U_("error renaming %s, %s unchanged"), sp->tpath,
		sp->opath);
	    goto done;
	}
    }
    ret = true;
done:
    debug_return_bool(ret);
}

/*
 * Assuming a parse error occurred, prompt the user for what they want
 * to do now.  Returns the first letter of their choice.
 */
static int
whatnow(void)
{
    int choice, c;
    debug_decl(whatnow, SUDOERS_DEBUG_UTIL);

    for (;;) {
	(void) fputs(_("What now? "), stdout);
	choice = getchar();
	for (c = choice; c != '\r' && c != '\n' && c != EOF;)
	    c = getchar();

	switch (choice) {
	    case EOF:
		choice = 'x';
		FALLTHROUGH;
	    case 'e':
	    case 'x':
	    case 'Q':
		debug_return_int(choice);
	    default:
		(void) puts(_("Options are:\n"
		    "  (e)dit sudoers file again\n"
		    "  e(x)it without saving changes to sudoers file\n"
		    "  (Q)uit and save changes to sudoers file (DANGER!)\n"));
	}
    }
}

/*
 * Install signal handlers for visudo.
 */
static void
setup_signals(void)
{
    struct sigaction sa;
    debug_decl(setup_signals, SUDOERS_DEBUG_UTIL);

    /*
     * Setup signal handlers to cleanup nicely.
     */
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = quit;
    (void) sigaction(SIGTERM, &sa, NULL);
    (void) sigaction(SIGHUP, &sa, NULL);
    (void) sigaction(SIGINT, &sa, NULL);
    (void) sigaction(SIGQUIT, &sa, NULL);

    debug_return;
}

static int
run_command(const char *path, char *const *argv)
{
    int status;
    pid_t pid, rv;
    debug_decl(run_command, SUDOERS_DEBUG_UTIL);

    switch (pid = sudo_debug_fork()) {
	case -1:
	    sudo_fatal(U_("unable to execute %s"), path);
	case 0:
	    closefrom(STDERR_FILENO + 1);
	    execv(path, argv);
	    sudo_warn(U_("unable to run %s"), path);
	    _exit(127);
    }

    for (;;) {
	rv = waitpid(pid, &status, 0);
	if (rv == -1 && errno != EINTR)
	    break;
	if (rv != -1 && !WIFSTOPPED(status))
	    break;
    }

    if (rv != -1)
	rv = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    debug_return_int(rv);
}

static bool
check_file(struct sudoers_context *ctx, const char *path, bool check_owner,
    bool check_mode)
{
    struct stat sb;
    bool ok = true;
    debug_decl(check_file, SUDOERS_DEBUG_UTIL);

    if (stat(path, &sb) == 0) {
	if (check_owner) {
	    if (sb.st_uid != sudoers_file_uid() || sb.st_gid != sudoers_file_gid()) {
		ok = false;
		if (ctx->parser_conf.verbose) {
		    fprintf(stderr,
			_("%s: wrong owner (uid, gid) should be (%u, %u)\n"),
			path, (unsigned int)sudoers_file_uid(),
			(unsigned int)sudoers_file_gid());
		}
	    }
	}
	if (check_mode) {
	    if ((sb.st_mode & ALLPERMS) != sudoers_file_mode()) {
		ok = false;
		if (ctx->parser_conf.verbose) {
		    fprintf(stderr,
			_("%s: bad permissions, should be mode 0%o\n"),
			path, (unsigned int)sudoers_file_mode());
		}
	    }
	}
    }
    debug_return_bool(ok);
}

static bool
check_syntax(struct sudoers_context *ctx, const char *path, bool check_owner,
    bool check_mode)
{
    bool ok = false;
    int fd, oldlocale;
    char fname[PATH_MAX];
    debug_decl(check_syntax, SUDOERS_DEBUG_UTIL);

    if (strcmp(path, "-") == 0) {
	sudoersin = stdin;
	(void)strlcpy(fname, "stdin", sizeof(fname));
    } else {
	fd = sudo_open_conf_path(path, fname, sizeof(fname), NULL);
	if (fd == -1 || (sudoersin = fdopen(fd, "r")) == NULL) {
	    if (ctx->parser_conf.verbose)
		sudo_warn(U_("unable to open %s"), fname);
	    if (fd != -1)
		close(fd);
	    goto done;
	}
    }
    init_parser(ctx, fname);
    sudoers_setlocale(SUDOERS_LOCALE_SUDOERS, &oldlocale);
    if (sudoersparse() && !parse_error) {
	if (ctx->parser_conf.verbose)
	    sudo_warnx(U_("failed to parse %s file, unknown error"), fname);
	parse_error = true;
    }
    if (!parse_error) {
	parse_error = !update_defaults(ctx, &parsed_policy, NULL,
	    SETDEF_GENERIC|SETDEF_HOST|SETDEF_USER, true);
	check_defaults_and_aliases(ctx);
    }
    sudoers_setlocale(oldlocale, NULL);
    ok = !parse_error;

    if (!parse_error) {
	struct sudoersfile *sp;

	/* Parsed OK, check mode and owner. */
	if (check_file(ctx, fname, check_owner, check_mode)) {
	    if (ctx->parser_conf.verbose)
		(void) printf(_("%s: parsed OK\n"), fname);
	} else {
	    ok = false;
	}
	TAILQ_FOREACH(sp, &sudoerslist, entries) {
	    if (check_file(ctx, sp->opath, check_owner, check_mode)) {
		if (ctx->parser_conf.verbose)
		    (void) printf(_("%s: parsed OK\n"), sp->opath);
	    } else {
		ok = false;
	    }
	}
    }

done:
    debug_return_bool(ok);
}

static bool
lock_sudoers(struct sudoersfile *entry)
{
    int ch;
    debug_decl(lock_sudoers, SUDOERS_DEBUG_UTIL);

    if (!sudo_lock_file(entry->fd, SUDO_TLOCK)) {
	if (errno == EAGAIN || errno == EWOULDBLOCK) {
	    sudo_warnx(U_("%s busy, try again later"), entry->opath);
	    debug_return_bool(false);
	}
	sudo_warn(U_("unable to lock %s"), entry->opath);
	(void) fputs(_("Edit anyway? [y/N]"), stdout);
	ch = getchar();
	if (tolower(ch) != 'y')
	    debug_return_bool(false);
    }
    debug_return_bool(true);
}

/*
 * Open (and lock) a new sudoers file.
 * Returns a new struct sudoersfile on success or NULL on failure.
 */
static struct sudoersfile *
new_sudoers(const char *path, bool doedit)
{
    const char *cp, *ep, *path_end;
    struct sudoersfile *entry;
    struct stat sb;
    size_t len;
    int fd = -1;
    debug_decl(new_sudoersfile, SUDOERS_DEBUG_UTIL);

    /* We always write to the first file in the colon-separated path. */
    len = strcspn(path, ":");
    entry = calloc(1, sizeof(*entry));
    if (entry == NULL || (entry->dpath = strndup(path, len)) == NULL)
	sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));

    /* Open the first file found in the colon-separated path. */
    path_end = path + strlen(path);
    for (cp = sudo_strsplit(path, path_end, ":", &ep);
	cp != NULL; cp = sudo_strsplit(NULL, path_end, ":", &ep)) {

	char fname[PATH_MAX];
	len = (size_t)(ep - cp);
	if (len >= sizeof(fname)) {
	    errno = ENAMETOOLONG;
	    break;
	}
	memcpy(fname, cp, len);
	fname[len] = '\0';

	/* Open in write mode for file locking. */
	fd = open(fname, checkonly ? O_RDONLY : O_RDWR);
	if (fd != -1) {
	    /* Store the path we actually opened. */
	    if ((entry->opath = strdup(fname)) == NULL) {
		sudo_fatalx(U_("%s: %s"), __func__,
		    U_("unable to allocate memory"));
	    }
	    break;
	}

	/* If the file exists but we can't open it, that is a fatal error. */
	if (errno != ENOENT) {
	    sudo_warn("%s", fname);
	    goto bad;
	}
    }
    if (fd == -1) {
	if (!checkonly) {
	    /* No sudoers file, create the destination file for editing. */
	    fd = open(entry->dpath, O_RDWR|O_CREAT, sudoers_file_mode());
	    entry->created = true;
	}
	if (fd == -1) {
	    sudo_warn("%s", entry->dpath);
	    goto bad;
	}
	entry->opath = entry->dpath;
    }
    if (fstat(fd, &sb) == -1) {
	sudo_warn("%s", entry->opath);
	goto bad;
    }
    if (!S_ISREG(sb.st_mode)) {
	sudo_warnx(U_("%s is not a regular file"), entry->opath);
	goto bad;
    }
    entry->fd = fd;
    /* entry->tpath = NULL; */
    /* entry->modified = false; */
    entry->doedit = doedit;
    if (!checkonly && !lock_sudoers(entry))
	goto bad;
    debug_return_ptr(entry);
bad:
    if (fd != -1)
	close(fd);
    if (entry->opath != entry->dpath)
	free(entry->opath);
    free(entry->dpath);
    free(entry);
    debug_return_ptr(NULL);
}

/*
 * Used to open (and lock) the initial sudoers file and to also open
 * any subsequent files #included via a callback from the parser.
 */
FILE *
open_sudoers(const char *path, char **outfile, bool doedit, bool *keepopen)
{
    struct sudoersfile *entry;
    size_t len;
    FILE *fp;
    debug_decl(open_sudoers, SUDOERS_DEBUG_UTIL);

    /* Check for existing entry using the first file in path. */
    len = strcspn(path, ":");
    TAILQ_FOREACH(entry, &sudoerslist, entries) {
	if (strncmp(path, entry->dpath, len) == 0 && entry->dpath[len] == '\0')
	    break;
    }
    if (entry == NULL) {
	if (doedit && !edit_includes) {
	    /* Only edit the main sudoers file. */
	    if (strncmp(path, path_sudoers, len) != 0 ||
		    (path_sudoers[len] != '\0' && path_sudoers[len] != ':'))
		doedit = false;
	}
	if ((entry = new_sudoers(path, doedit)) == NULL)
	    debug_return_ptr(NULL);
	if ((fp = fdopen(entry->fd, "r")) == NULL)
	    sudo_fatal("%s", entry->opath);
	TAILQ_INSERT_TAIL(&sudoerslist, entry, entries);
    } else {
	/* Already exists, open .tmp version if there is one. */
	if (entry->tpath != NULL) {
	    if ((fp = fopen(entry->tpath, "r")) == NULL)
		sudo_fatal("%s", entry->tpath);
	} else {
	    if ((fp = fdopen(entry->fd, "r")) == NULL)
		sudo_fatal("%s", entry->opath);
	    rewind(fp);
	}
    }
    if (keepopen != NULL)
	*keepopen = true;
    if (outfile != NULL) {
	*outfile = sudo_rcstr_dup(entry->opath);
	if (*outfile == NULL)
	    sudo_fatalx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
    }
    debug_return_ptr(fp);
}

/*
 * Support "visudo -x" for backwards compatibility.
 * To be removed in a future version of sudo.
 */
static void
export_sudoers(const char *infile, const char *outfile)
{
    char pathbuf[PATH_MAX];
    int fd;
    debug_decl(export_sudoers, SUDOERS_DEBUG_UTIL);

    sudo_warnx("%s",
	U_("the -x option will be removed in a future release"));
    sudo_warnx("%s",
	U_("please consider using the cvtsudoers utility instead"));

    /* Export the first sudoers file that exists in the path. */
    fd = sudo_open_conf_path(infile, pathbuf, sizeof(pathbuf), NULL);
    if (fd != -1) {
	close(fd);
	infile = pathbuf;
    }
    execlp("cvtsudoers", "cvtsudoers", "-f", "json", "-o", outfile,
	infile, (char *)0);
    sudo_fatal(U_("unable to execute %s"), "cvtsudoers");
}

/* Display unused aliases from check_aliases(). */
static int
print_unused(struct sudoers_parse_tree *parse_tree, struct alias *a, void *v)
{
    const bool quiet = *((bool *)v);

    if (!quiet) {
	fprintf(stderr, U_("Warning: %s:%d:%d: unused %s \"%s\""), a->file,
	    a->line, a->column, alias_type_to_string(a->type), a->name);
	fputc('\n', stderr);
    }
    return 0;
}

static void
parse_sudoers_options(struct sudoers_context *ctx)
{
    struct plugin_info_list *plugins;
    debug_decl(parse_sudoers_options, SUDOERS_DEBUG_UTIL);

    plugins = sudo_conf_plugins();
    if (plugins) {
	struct plugin_info *info;

	TAILQ_FOREACH(info, plugins, entries) {
	    if (strcmp(info->symbol_name, "sudoers_policy") == 0)
		break;
	}
	if (info != NULL && info->options != NULL) {
	    char * const *cur;

#define MATCHES(s, v)	\
    (strncmp((s), (v), sizeof(v) - 1) == 0 && (s)[sizeof(v) - 1] != '\0')

	    for (cur = info->options; *cur != NULL; cur++) {
		const char *errstr, *p;
		id_t id;
		mode_t mode;

		if (MATCHES(*cur, "sudoers_file=")) {
		    path_sudoers = *cur + sizeof("sudoers_file=") - 1;
		    continue;
		}
		if (MATCHES(*cur, "sudoers_uid=")) {
		    p = *cur + sizeof("sudoers_uid=") - 1;
		    id = sudo_strtoid(p, &errstr);
		    if (errstr == NULL)
			ctx->parser_conf.sudoers_uid = (uid_t)id;
		    continue;
		}
		if (MATCHES(*cur, "sudoers_gid=")) {
		    p = *cur + sizeof("sudoers_gid=") - 1;
		    id = sudo_strtoid(p, &errstr);
		    if (errstr == NULL)
			ctx->parser_conf.sudoers_gid = (gid_t)id;
		    continue;
		}
		if (MATCHES(*cur, "sudoers_mode=")) {
		    p = *cur + sizeof("sudoers_mode=") - 1;
		    mode = sudo_strtomode(p, &errstr);
		    if (errstr == NULL)
			ctx->parser_conf.sudoers_mode = mode;
		    continue;
		}
	    }
#undef MATCHES
	}
    }
    debug_return;
}

/*
 * Unlink any sudoers temp files that remain.
 */
static void
visudo_cleanup(void)
{
    struct sudoersfile *sp;

    TAILQ_FOREACH(sp, &sudoerslist, entries) {
	if (sp->tpath != NULL)
	    (void) unlink(sp->tpath);
    }
}

/*
 * Unlink sudoers temp files (if any) and exit.
 */
static void
quit(int signo)
{
    struct iovec iov[4];

    visudo_cleanup();

#define	emsg	 " exiting due to signal: "
    iov[0].iov_base = (char *)getprogname();
    iov[0].iov_len = strlen(iov[0].iov_base);
    iov[1].iov_base = (char *)emsg;
    iov[1].iov_len = sizeof(emsg) - 1;
    iov[2].iov_base = strsignal(signo);
    iov[2].iov_len = strlen(iov[2].iov_base);
    iov[3].iov_base = (char *)"\n";
    iov[3].iov_len = 1;
    ignore_result(writev(STDERR_FILENO, iov, 4));
    _exit(signo);
}

#define VISUDO_USAGE	"usage: %s [-chqsV] [[-f] sudoers ]\n"

sudo_noreturn static void
usage(void)
{
    (void) fprintf(stderr, VISUDO_USAGE, getprogname());
    exit(EXIT_FAILURE);
}

sudo_noreturn static void
help(void)
{
    (void) printf(_("%s - safely edit the sudoers file\n\n"), getprogname());
    (void) printf(VISUDO_USAGE, getprogname());
    (void) puts(_("\nOptions:\n"
	"  -c, --check              check-only mode\n"
	"  -f, --file=sudoers       specify sudoers file location\n"
	"  -h, --help               display help message and exit\n"
	"  -I, --no-includes        do not edit include files\n"
	"  -q, --quiet              less verbose (quiet) syntax error messages\n"
	"  -s, --strict             strict syntax checking\n"
	"  -V, --version            display version information and exit\n"));
    exit(EXIT_SUCCESS);
}
