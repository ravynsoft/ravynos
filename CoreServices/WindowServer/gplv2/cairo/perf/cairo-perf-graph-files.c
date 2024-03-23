/*
 * Copyright © 2008 Chris Wilson
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of the
 * copyright holders not be used in advertising or publicity
 * pertaining to distribution of the software without specific,
 * written prior permission. The copyright holders make no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * Authors: Chris Wilson <chris@chris-wilson.co.uk>
 */

#define GLIB_DISABLE_DEPRECATION_WARNINGS

#include "cairo-perf.h"
#include "cairo-perf-graph.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef G_OS_UNIX
#include <unistd.h>
#include <fcntl.h>
#endif

#include <cairo.h>

static void
usage (const char *argv0)
{
    char const *basename = strrchr (argv0, '/');
    basename = basename ? basename+1 : argv0;
    g_printerr ("Usage: %s [options] file1 file2 [...]\n\n", basename);
    g_printerr ("Draws a graph illustrating the change in performance over a series.\n");
    exit(1);
}

enum {
    CASE_SHOWN,
    CASE_INCONSISTENT,
    CASE_BACKEND,
    CASE_CONTENT,
    CASE_NAME,
    CASE_SIZE,
    CASE_FG_COLOR,
    CASE_DATA,
    CASE_NCOLS
};

static GtkTreeStore *
cases_to_store (test_case_t *cases)
{
    GtkTreeStore *store;
    GtkTreeIter backend_iter;
    GtkTreeIter content_iter;
    const char *backend = NULL;
    const char *content = NULL;

    store = gtk_tree_store_new (CASE_NCOLS,
				G_TYPE_BOOLEAN, /* shown */
				G_TYPE_BOOLEAN, /* inconsistent */
				G_TYPE_STRING, /* backend */
				G_TYPE_STRING, /* content */
				G_TYPE_STRING, /* name */
				G_TYPE_INT, /* size */
				GDK_TYPE_COLOR, /* fg color */
				G_TYPE_POINTER); /* data */
    while (cases->backend != NULL) {
	GtkTreeIter iter;

	if (backend == NULL || strcmp (backend, cases->backend)) {
	    gtk_tree_store_append (store, &backend_iter, NULL);
	    gtk_tree_store_set (store, &backend_iter,
				CASE_SHOWN, TRUE,
				CASE_BACKEND, cases->backend,
				-1);
	    backend = cases->backend;
	    content = NULL;
	}
	if (content == NULL || strcmp (content, cases->content)) {
	    gtk_tree_store_append (store, &content_iter, &backend_iter);
	    gtk_tree_store_set (store, &content_iter,
				CASE_SHOWN, TRUE,
				CASE_BACKEND, cases->backend,
				CASE_CONTENT, cases->content,
				-1);
	    content = cases->content;
	}

	gtk_tree_store_append (store, &iter, &content_iter);
	gtk_tree_store_set (store, &iter,
			    CASE_SHOWN, TRUE,
			    CASE_BACKEND, cases->backend,
			    CASE_CONTENT, cases->content,
			    CASE_NAME, cases->name,
			    CASE_SIZE, cases->size,
			    CASE_FG_COLOR, &cases->color,
			    CASE_DATA, cases,
			    -1);
	cases++;
    }

    return store;
}

struct _app_data {
    GtkWidget *window;

    test_case_t *cases;
    cairo_perf_report_t *reports;
    int num_reports;

    GtkTreeStore *case_store;

    GIOChannel *git_io;
    GtkTextBuffer *git_buffer;

    GtkWidget *gv;
};

static void
recurse_set_shown (GtkTreeModel *model,
		   GtkTreeIter	*parent,
		   gboolean	 shown)
{
    GtkTreeIter iter;

    if (gtk_tree_model_iter_children (model, &iter, parent)) do {
	test_case_t *c;

	gtk_tree_model_get (model, &iter, CASE_DATA, &c, -1);
	if (c == NULL) {
	    recurse_set_shown (model, &iter, shown);
	} else if (shown != c->shown) {
	    c->shown = shown;
	    gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
				CASE_SHOWN, shown,
				CASE_INCONSISTENT, FALSE,
				-1);
	}
    } while (gtk_tree_model_iter_next (model, &iter));
}

static gboolean
children_consistent (GtkTreeModel *model,
		     GtkTreeIter  *parent)
{
    GtkTreeIter iter;
    gboolean first = TRUE;
    gboolean first_active;

    if (gtk_tree_model_iter_children (model, &iter, parent)) do {
	gboolean active, inconsistent;

	gtk_tree_model_get (model, &iter,
			    CASE_INCONSISTENT, &inconsistent,
			    CASE_SHOWN, &active,
			    -1);
	if (inconsistent)
	    return FALSE;

	if (first) {
	    first_active = active;
	    first = FALSE;
	} else if (active != first_active)
	    return FALSE;
    } while (gtk_tree_model_iter_next (model, &iter));

    return TRUE;
}

static void
check_consistent (GtkTreeModel *model,
		  GtkTreeIter  *child)
{
    GtkTreeIter parent;

    if (gtk_tree_model_iter_parent (model, &parent, child)) {
	gtk_tree_store_set (GTK_TREE_STORE (model), &parent,
			    CASE_INCONSISTENT,
			    ! children_consistent (model, &parent),
			    -1);
	check_consistent (model, &parent);
    }
}

static void
show_case_toggled (GtkCellRendererToggle *cell,
		   gchar		 *str,
		   struct _app_data	 *app)
{
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkTreeIter iter;
    test_case_t *c;
    gboolean active;

    active = ! gtk_cell_renderer_toggle_get_active (cell);

    model = GTK_TREE_MODEL (app->case_store);

    path = gtk_tree_path_new_from_string (str);
    gtk_tree_model_get_iter (model, &iter, path);
    gtk_tree_path_free (path);

    gtk_tree_store_set (app->case_store, &iter,
			CASE_SHOWN, active,
			CASE_INCONSISTENT, FALSE,
			-1);
    gtk_tree_model_get (model, &iter, CASE_DATA, &c, -1);
    if (c != NULL) {
	if (active == c->shown)
	    return;

	c->shown = active;
    } else {
	recurse_set_shown (model, &iter, active);
    }
    check_consistent (model, &iter);

    graph_view_update_visible ((GraphView *) app->gv);
}

#ifdef G_OS_UNIX

static gboolean
git_read (GIOChannel	   *io,
	  GIOCondition	    cond,
	  struct _app_data *app)
{
    int fd;

    fd = g_io_channel_unix_get_fd (io);
    do {
	char buf[4096];
	int len;
	GtkTextIter end;

	len = read (fd, buf, sizeof (buf));
	if (len <= 0) {
	    int err = len ? errno : 0;
	    switch (err) {
	    case EAGAIN:
	    case EINTR:
		return TRUE;
	    default:
		g_io_channel_unref (app->git_io);
		app->git_io = NULL;
		return FALSE;
	    }
	}

	gtk_text_buffer_get_end_iter (app->git_buffer, &end);
	gtk_text_buffer_insert (app->git_buffer, &end, buf, len);
    } while (TRUE);
}

static void
do_git (struct _app_data  *app,
	char		 **argv)
{
    gint output;
    GError *error = NULL;
    GtkTextIter start, stop;
    long flags;

    if (! g_spawn_async_with_pipes (NULL, argv, NULL,
				    G_SPAWN_SEARCH_PATH |
				    G_SPAWN_STDERR_TO_DEV_NULL |
				    G_SPAWN_FILE_AND_ARGV_ZERO,
				    NULL, NULL, NULL,
				    NULL, &output, NULL,
				    &error))
    {
	g_error ("spawn failed: %s", error->message);
    }

    if (app->git_io) {
	g_io_channel_shutdown (app->git_io, FALSE, NULL);
	g_io_channel_unref (app->git_io);
    }

    gtk_text_buffer_get_bounds (app->git_buffer, &start, &stop);
    gtk_text_buffer_delete (app->git_buffer, &start, &stop);

    flags = fcntl (output, F_GETFL);
    if ((flags & O_NONBLOCK) == 0)
	fcntl (output, F_SETFL, flags | O_NONBLOCK);

    app->git_io = g_io_channel_unix_new (output);
    g_io_add_watch (app->git_io, G_IO_IN | G_IO_HUP, (GIOFunc) git_read, app);
}

#endif

static void
gv_report_selected (GraphView	     *gv,
		    int 	      i,
		    struct _app_data *app)
{
    cairo_perf_report_t *report;
    char *hyphen;

    if (i == -1)
	return;

    report = &app->reports[i];
    hyphen = strchr (report->configuration, '-');
    if (hyphen != NULL) {
	int len = hyphen - report->configuration;
	char *id = g_malloc (len + 1);
	char *argv[5];

	memcpy (id, report->configuration, len);
	id[len] = '\0';

	argv[0] = (char *) "git";
	argv[1] = (char *) "git";
	argv[2] = (char *) "show";
	argv[3] = id;
	argv[4] = NULL;

#ifdef G_OS_UNIX
	do_git (app, argv);
#else
        g_print ("id: %s\n", id);
#endif
	g_free (id);
    }
}

static GtkWidget *
window_create (test_case_t	   *cases,
	       cairo_perf_report_t *reports,
	       int		    num_reports)
{
    GtkWidget *window, *table, *w;
    GtkWidget *tv, *sw;
    GtkTreeStore *store;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;
    struct _app_data *data;


    data = g_new0 (struct _app_data, 1);
    data->cases = cases;
    data->reports = reports;
    data->num_reports = num_reports;

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "Cairo Performance Graph");
    g_object_set_data_full (G_OBJECT (window),
			    "app-data", data, (GDestroyNotify)g_free);

    data->window = window;

    table = gtk_table_new (2, 2, FALSE);

    /* legend & show/hide lines (categorised) */
    tv = gtk_tree_view_new ();
    store = cases_to_store (cases);
    data->case_store = store;
    gtk_tree_view_set_model (GTK_TREE_VIEW (tv), GTK_TREE_MODEL (store));

    renderer = gtk_cell_renderer_toggle_new ();
    column = gtk_tree_view_column_new_with_attributes (NULL,
	    renderer,
	    "active", CASE_SHOWN,
	    "inconsistent", CASE_INCONSISTENT,
	    NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tv), column);
    g_signal_connect (renderer, "toggled",
		      G_CALLBACK (show_case_toggled), data);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Backend",
	    renderer,
	    "text", CASE_BACKEND,
	    NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tv), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Content",
	    renderer,
	    "text", CASE_CONTENT,
	    NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tv), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Test",
	    renderer,
	    "text", CASE_NAME,
	    "foreground-gdk", CASE_FG_COLOR,
	    NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tv), column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Size",
	    renderer,
	    "text", CASE_SIZE,
	    NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (tv), column);

    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (tv), TRUE);
    g_object_unref (store);

    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				    GTK_POLICY_NEVER,
				    GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (sw), tv);
    gtk_widget_show (tv);
    gtk_table_attach (GTK_TABLE (table), sw,
		      0, 1, 0, 2,
		      GTK_FILL, GTK_FILL,
		      4, 4);
    gtk_widget_show (sw);

    /* the performance chart */
    w = graph_view_new ();
    data->gv = w;
    g_signal_connect (w, "report-selected",
		      G_CALLBACK (gv_report_selected), data);
    graph_view_set_reports ((GraphView *)w, cases, reports, num_reports);
    gtk_table_attach (GTK_TABLE (table), w,
		      1, 2, 0, 1,
		      GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND,
		      4, 4);
    gtk_widget_show (w);

    /* interesting information - presumably the commit log */
    w = gtk_text_view_new ();
    data->git_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (w));
    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				    GTK_POLICY_NEVER,
				    GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (sw), w);
    gtk_widget_show (w);
    gtk_table_attach (GTK_TABLE (table), sw,
		      1, 2, 1, 2,
		      GTK_FILL, GTK_FILL | GTK_EXPAND,
		      4, 4);
    gtk_widget_show (sw);

    gtk_container_add (GTK_CONTAINER (window), table);
    gtk_widget_show (table);

    return window;
}

static void
name_to_color (const char *name,
	       GdkColor   *color)
{
    gint v = g_str_hash (name);

    color->red = ((v >>  0) & 0xff) / 384. * 0xffff;
    color->green = ((v >>  8) & 0xff) / 384. * 0xffff;
    color->blue = ((v >> 16) & 0xff) / 384. * 0xffff;
}

static test_case_t *
test_cases_from_reports (cairo_perf_report_t *reports,
			 int		      num_reports)
{
    test_case_t *cases, *c;
    test_report_t **tests;
    int i, j;
    int num_tests;

    num_tests = 0;
    for (i = 0; i < num_reports; i++) {
	for (j = 0; reports[i].tests[j].name != NULL; j++)
	    ;
	if (j > num_tests)
	    num_tests = j;
    }

    cases = xcalloc (num_tests+1, sizeof (test_case_t));
    tests = xmalloc (num_reports * sizeof (test_report_t *));
    for (i = 0; i < num_reports; i++)
	tests[i] = reports[i].tests;

    c = cases;
    while (1) {
	int seen_non_null;
	test_report_t *min_test;

	/* We expect iterations values of 0 when multiple raw reports
	 * for the same test have been condensed into the stats of the
	 * first. So we just skip these later reports that have no
	 * stats. */
	seen_non_null = 0;
	for (i = 0; i < num_reports; i++) {
	    while (tests[i]->name && tests[i]->stats.iterations == 0)
		tests[i]++;
	    if (tests[i]->name)
		seen_non_null++;
	}

	if (seen_non_null < 2)
	    break;

	/* Find the minimum of all current tests, (we have to do this
	 * in case some reports don't have a particular test). */
	for (i = 0; i < num_reports; i++) {
	    if (tests[i]->name) {
		min_test = tests[i];
		break;
	    }
	}
	for (++i; i < num_reports; i++) {
	    if (tests[i]->name &&
		test_report_cmp_backend_then_name (tests[i], min_test) < 0)
	    {
		min_test = tests[i];
	    }
	}

	c->min_test = min_test;
	c->backend = min_test->backend;
	c->content = min_test->content;
	c->name = min_test->name;
	c->size = min_test->size;
	c->baseline = min_test->stats.min_ticks;
	c->min = c->max = 1.;
	c->shown = TRUE;
	name_to_color (c->name, &c->color);

	for (i = 0; i < num_reports; i++) {
	    if (tests[i]->name &&
		test_report_cmp_backend_then_name (tests[i], min_test) == 0)
	    {
		tests[i]++;
		break;
	    }
	}

	for (++i; i < num_reports; i++) {
	    if (tests[i]->name &&
		test_report_cmp_backend_then_name (tests[i], min_test) == 0)
	    {
		double v = tests[i]->stats.min_ticks / c->baseline;
		if (v < c->min)
		    c->min = v;
		if (v > c->max)
		    c->max = v;
		tests[i]++;
	    }
	}

	c++;
    }
    free (tests);

    return cases;
}
int
main (int   argc,
      char *argv[])
{
    cairo_perf_report_t *reports;
    test_case_t *cases;
    test_report_t *t;
    int i;
    GtkWidget *window;

    gtk_init (&argc, &argv);

    if (argc < 3)
	usage (argv[0]);

    reports = xmalloc ((argc-1) * sizeof (cairo_perf_report_t));
    for (i = 1; i < argc; i++ )
	cairo_perf_report_load (&reports[i-1], argv[i], i, NULL);

    cases = test_cases_from_reports (reports, argc-1);

    window = window_create (cases, reports, argc-1);
    g_signal_connect (window, "delete-event",
		      G_CALLBACK (gtk_main_quit), NULL);
    gtk_widget_show (window);

    gtk_main ();

    /* Pointless memory cleanup, (would be a great place for talloc) */
    free (cases);
    for (i = 0; i < argc-1; i++) {
	for (t = reports[i].tests; t->name; t++) {
	    free (t->samples);
	    free (t->backend);
	    free (t->name);
	}
	free (reports[i].tests);
	free (reports[i].configuration);
    }
    free (reports);

    return 0;
}
