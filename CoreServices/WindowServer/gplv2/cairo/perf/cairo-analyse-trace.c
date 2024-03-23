/* -*- Mode: c; c-basic-offset: 4; indent-tabs-mode: t; tab-width: 8; -*- */
/*
 * Copyright © 2006 Mozilla Corporation
 * Copyright © 2006 Red Hat, Inc.
 * Copyright © 2009 Chris Wilson
 * Copyright © 2011 Intel Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * the authors not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. The authors make no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE AUTHORS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: Vladimir Vukicevic <vladimir@pobox.com>
 *	    Carl Worth <cworth@cworth.org>
 *	    Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "config.h"

#include "cairo-perf.h"
#include "cairo-stats.h"

#include "cairo-boilerplate-getopt.h"
#include <cairo-script-interpreter.h>
#include "cairo-missing.h"

/* rudely reuse bits of the library... */
#include "../src/cairo-error-private.h"

/* For basename */
#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif
#include <ctype.h> /* isspace() */

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _MSC_VER
#include "dirent-win32.h"

static char *
basename_no_ext (char *path)
{
    static char name[_MAX_FNAME + 1];

    _splitpath (path, NULL, NULL, name, NULL);

    name[_MAX_FNAME] = '\0';

    return name;
}


#else
#include <dirent.h>

static char *
basename_no_ext (char *path)
{
    char *dot, *name;

    name = basename (path);

    dot = strchr (name, '.');
    if (dot)
	*dot = '\0';

    return name;
}

#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <signal.h>

#if HAVE_FCFINI
#include <fontconfig/fontconfig.h>
#endif

struct trace {
    const cairo_boilerplate_target_t *target;
    void            *closure;
    cairo_surface_t *surface;
};

cairo_bool_t
cairo_perf_can_run (cairo_perf_t *perf,
		    const char	 *name,
		    cairo_bool_t *is_explicit)
{
    unsigned int i;
    char *copy, *dot;
    cairo_bool_t ret;

    if (is_explicit)
	*is_explicit = FALSE;

    if (perf->exact_names) {
	if (is_explicit)
	    *is_explicit = TRUE;
	return TRUE;
    }

    if (perf->num_names == 0 && perf->num_exclude_names == 0)
	return TRUE;

    copy = xstrdup (name);
    dot = strchr (copy, '.');
    if (dot != NULL)
	*dot = '\0';

    if (perf->num_names) {
	ret = TRUE;
	for (i = 0; i < perf->num_names; i++)
	    if (strstr (copy, perf->names[i])) {
		if (is_explicit)
		    *is_explicit = strcmp (copy, perf->names[i]) == 0;
		goto check_exclude;
	    }

	ret = FALSE;
	goto done;
    }

check_exclude:
    if (perf->num_exclude_names) {
	ret = FALSE;
	for (i = 0; i < perf->num_exclude_names; i++)
	    if (strstr (copy, perf->exclude_names[i])) {
		if (is_explicit)
		    *is_explicit = strcmp (copy, perf->exclude_names[i]) == 0;
		goto done;
	    }

	ret = TRUE;
	goto done;
    }

done:
    free (copy);

    return ret;
}

static cairo_surface_t *
surface_create (void		 *closure,
		cairo_content_t  content,
		double		  width,
		double		  height,
		long		  uid)
{
    struct trace *args = closure;
    return cairo_surface_create_similar (args->surface, content, width, height);
}

static int user_interrupt;

static void
interrupt (int sig)
{
    if (user_interrupt) {
	signal (sig, SIG_DFL);
	raise (sig);
    }

    user_interrupt = 1;
}

static void
describe (cairo_perf_t *perf,
          void *closure)
{
    char *description = NULL;

    if (perf->has_described_backend)
	    return;
    perf->has_described_backend = TRUE;

    if (perf->target->describe)
        description = perf->target->describe (closure);

    if (description == NULL)
        return;

    free (description);
}

static void
execute (cairo_perf_t	 *perf,
	 struct trace	 *args,
	 const char	 *trace)
{
    char *trace_cpy, *name;
    const cairo_script_interpreter_hooks_t hooks = {
	.closure = args,
	.surface_create = surface_create,
    };

    trace_cpy = xstrdup (trace);
    name = basename_no_ext (trace_cpy);

    if (perf->list_only) {
	printf ("%s\n", name);
	free (trace_cpy);
	return;
    }

    describe (perf, args->closure);

    {
	cairo_script_interpreter_t *csi;
	cairo_status_t status;
	unsigned int line_no;

	csi = cairo_script_interpreter_create ();
	cairo_script_interpreter_install_hooks (csi, &hooks);

	cairo_script_interpreter_run (csi, trace);

	cairo_script_interpreter_finish (csi);

	line_no = cairo_script_interpreter_get_line_number (csi);
	status = cairo_script_interpreter_destroy (csi);
	if (status) {
	    /* XXXX cairo_status_to_string is just wrong! */
	    fprintf (stderr, "Error during replay, line %d: %s\n",
		     line_no, cairo_status_to_string (status));
	}
    }
    user_interrupt = 0;

    free (trace_cpy);
}

static void
usage (const char *argv0)
{
    fprintf (stderr,
"Usage: %s [-l] [-i iterations] [-x exclude-file] [test-names ... | traces ...]\n"
"\n"
"Run the cairo trace analysis suite over the given tests (all by default)\n"
"The command-line arguments are interpreted as follows:\n"
"\n"
"  -i	iterations; specify the number of iterations per test case\n"
"  -l	list only; just list selected test case names without executing\n"
"  -x	exclude; specify a file to read a list of traces to exclude\n"
"\n"
"If test names are given they are used as sub-string matches so a command\n"
"such as \"%s firefox\" can be used to run all firefox traces.\n"
"Alternatively, you can specify a list of filenames to execute.\n",
	     argv0, argv0);
}

static cairo_bool_t
read_excludes (cairo_perf_t *perf,
	       const char   *filename)
{
    FILE *file;
    char *line = NULL;
    size_t line_size = 0;
    char *s, *t;

    file = fopen (filename, "r");
    if (file == NULL)
	return FALSE;

    while (getline (&line, &line_size, file) != -1) {
	/* terminate the line at a comment marker '#' */
	s = strchr (line, '#');
	if (s)
	    *s = '\0';

	/* whitespace delimits */
	s = line;
	while (*s != '\0' && isspace ((unsigned char)*s))
	    s++;

	t = s;
	while (*t != '\0' && ! isspace ((unsigned char)*t))
	    t++;

	if (s != t) {
	    int i = perf->num_exclude_names;
	    perf->exclude_names = xrealloc (perf->exclude_names,
					    sizeof (char *) * (i+1));
	    perf->exclude_names[i] = strndup (s, t-s);
	    perf->num_exclude_names++;
	}
    }
    free (line);

    fclose (file);

    return TRUE;
}

static void
parse_options (cairo_perf_t *perf,
	       int	     argc,
	       char	    *argv[])
{
    char *end;
    int c;

    perf->list_only = FALSE;
    perf->names = NULL;
    perf->num_names = 0;
    perf->exclude_names = NULL;
    perf->num_exclude_names = 0;

    while (1) {
	c = _cairo_getopt (argc, argv, "i:lx:");
	if (c == -1)
	    break;

	switch (c) {
	case 'i':
	    perf->exact_iterations = TRUE;
	    perf->iterations = strtoul (optarg, &end, 10);
	    if (*end != '\0') {
		fprintf (stderr, "Invalid argument for -i (not an integer): %s\n",
			 optarg);
		exit (1);
	    }
	    break;
	case 'l':
	    perf->list_only = TRUE;
	    break;
	case 'x':
	    if (! read_excludes (perf, optarg)) {
		fprintf (stderr, "Invalid argument for -x (not readable file): %s\n",
			 optarg);
		exit (1);
	    }
	    break;
	default:
	    fprintf (stderr, "Internal error: unhandled option: %c\n", c);
	    /* fall-through */
	case '?':
	    usage (argv[0]);
	    exit (1);
	}
    }

    if (optind < argc) {
	perf->names = &argv[optind];
	perf->num_names = argc - optind;
    }
}

static void
cairo_perf_fini (cairo_perf_t *perf)
{
    cairo_boilerplate_free_targets (perf->targets);
    cairo_boilerplate_fini ();

    cairo_debug_reset_static_data ();
#if HAVE_FCFINI
    FcFini ();
#endif
}

static cairo_bool_t
have_trace_filenames (cairo_perf_t *perf)
{
    unsigned int i;

    if (perf->num_names == 0)
	return FALSE;

#if HAVE_UNISTD_H
    for (i = 0; i < perf->num_names; i++)
	if (access (perf->names[i], R_OK) == 0)
	    return TRUE;
#endif

    return FALSE;
}

static cairo_status_t
print (void *closure, const unsigned char *data, unsigned int length)
{
    fwrite (data, length, 1, closure);
    return CAIRO_STATUS_SUCCESS;
}

static void
cairo_perf_trace (cairo_perf_t			   *perf,
		  const cairo_boilerplate_target_t *target,
		  const char			   *trace)
{
    struct trace args;
    cairo_surface_t *real;

    args.target = target;
    real = target->create_surface (NULL,
				   CAIRO_CONTENT_COLOR_ALPHA,
				   1, 1,
				   1, 1,
				   CAIRO_BOILERPLATE_MODE_PERF,
				   &args.closure);
    args.surface =
	    cairo_surface_create_observer (real,
					   CAIRO_SURFACE_OBSERVER_RECORD_OPERATIONS);
    cairo_surface_destroy (real);
    if (cairo_surface_status (args.surface)) {
	fprintf (stderr,
		 "Error: Failed to create target surface: %s\n",
		 target->name);
	return;
    }

    printf ("Observing '%s'...", trace);
    fflush (stdout);

    execute (perf, &args, trace);

    printf ("\n");
    cairo_device_observer_print (cairo_surface_get_device (args.surface),
				 print, stdout);
    fflush (stdout);

    cairo_surface_destroy (args.surface);

    if (target->cleanup)
	target->cleanup (args.closure);
}

static void
warn_no_traces (const char *message,
		const char *trace_dir)
{
    fprintf (stderr,
"Error: %s '%s'.\n"
"Have you cloned the cairo-traces repository and uncompressed the traces?\n"
"  git clone git://anongit.freedesktop.org/cairo-traces\n"
"  cd cairo-traces && make\n"
"Or set the env.var CAIRO_TRACE_DIR to point to your traces?\n",
	    message, trace_dir);
}

static int
cairo_perf_trace_dir (cairo_perf_t		       *perf,
		      const cairo_boilerplate_target_t *target,
		      const char		       *dirname)
{
    DIR *dir;
    struct dirent *de;
    int num_traces = 0;
    cairo_bool_t force;
    cairo_bool_t is_explicit;

    dir = opendir (dirname);
    if (dir == NULL)
	return 0;

    force = FALSE;
    if (cairo_perf_can_run (perf, dirname, &is_explicit))
	force = is_explicit;

    while ((de = readdir (dir)) != NULL) {
	char *trace;
	struct stat st;

	if (de->d_name[0] == '.')
	    continue;

	xasprintf (&trace, "%s/%s", dirname, de->d_name);
	if (stat (trace, &st) != 0)
	    goto next;

	if (S_ISDIR(st.st_mode)) {
	    num_traces += cairo_perf_trace_dir (perf, target, trace);
	} else {
	    const char *dot;

	    dot = strrchr (de->d_name, '.');
	    if (dot == NULL)
		goto next;
	    if (strcmp (dot, ".trace"))
		goto next;

	    num_traces++;
	    if (!force && ! cairo_perf_can_run (perf, de->d_name, NULL))
		goto next;

	    cairo_perf_trace (perf, target, trace);
	}
next:
	free (trace);

    }
    closedir (dir);

    return num_traces;
}

int
main (int   argc,
      char *argv[])
{
    cairo_perf_t perf;
    const char *trace_dir = "cairo-traces:/usr/src/cairo-traces:/usr/share/cairo-traces";
    unsigned int n;
    int i;

    parse_options (&perf, argc, argv);

    signal (SIGINT, interrupt);

    if (getenv ("CAIRO_TRACE_DIR") != NULL)
	trace_dir = getenv ("CAIRO_TRACE_DIR");

    perf.targets = cairo_boilerplate_get_targets (&perf.num_targets, NULL);

    /* do we have a list of filenames? */
    perf.exact_names = have_trace_filenames (&perf);

    for (i = 0; i < perf.num_targets; i++) {
	const cairo_boilerplate_target_t *target = perf.targets[i];

	if (! perf.list_only && ! target->is_measurable)
	    continue;

	perf.target = target;
	perf.has_described_backend = FALSE;

	if (perf.exact_names) {
	    for (n = 0; n < perf.num_names; n++) {
		struct stat st;

		if (stat (perf.names[n], &st) == 0) {
		    if (S_ISDIR (st.st_mode)) {
			cairo_perf_trace_dir (&perf, target, perf.names[n]);
		    } else
			cairo_perf_trace (&perf, target, perf.names[n]);
		}
	    }
	} else {
	    int num_traces = 0;
	    const char *dir;

	    dir = trace_dir;
	    do {
		char buf[1024];
		const char *end = strchr (dir, ':');
		if (end != NULL) {
		    memcpy (buf, dir, end-dir);
		    buf[end-dir] = '\0';
		    end++;

		    dir = buf;
		}

		num_traces += cairo_perf_trace_dir (&perf, target, dir);
		dir = end;
	    } while (dir != NULL);

	    if (num_traces == 0) {
		warn_no_traces ("Found no traces in", trace_dir);
		return 1;
	    }
	}

	if (perf.list_only)
	    break;
    }

    cairo_perf_fini (&perf);

    return 0;
}
