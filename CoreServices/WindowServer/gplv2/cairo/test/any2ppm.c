/*
 * Copyright © 2008 Chris Wilson
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Chris Wilson not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Chris Wilson makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * CHRIS WILSON DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL CHRIS WILSON BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 *
 * Contributor(s):
 *	Carlos Garcia Campos <carlosgc@gnome.org>
 *
 * Adapted from pdf2png.c:
 * Copyright © 2005 Red Hat, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Red Hat, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Red Hat, Inc. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * RED HAT, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL RED HAT, INC. BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Kristian Høgsberg <krh@redhat.com>
 */

#include "config.h"

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cairo.h>

#if CAIRO_HAS_INTERPRETER
#include <cairo-script-interpreter.h>
#endif

#if CAIRO_CAN_TEST_PDF_SURFACE
#include <poppler.h>
#endif

#if CAIRO_CAN_TEST_SVG_SURFACE
#define RSVG_DISABLE_DEPRECATION_WARNINGS
#include <librsvg/rsvg.h>
#ifndef RSVG_CAIRO_H
#include <librsvg/rsvg-cairo.h>
#endif
#endif

#if CAIRO_HAS_SPECTRE
#include <libspectre/spectre.h>
#endif

#include <errno.h>

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

#if HAVE_UNISTD_H && HAVE_SIGNAL_H && HAVE_SYS_STAT_H && HAVE_SYS_SOCKET_H && (HAVE_POLL_H || HAVE_SYS_POLL_H) && HAVE_SYS_UN_H
#include <signal.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

#if defined(HAVE_POLL_H)
#include <poll.h>
#elif defined(HAVE_SYS_POLL_H)
#include <sys/poll.h>
#endif

#define SOCKET_PATH "./.any2ppm"
#define TIMEOUT 60000 /* 60 seconds */

#if HAVE_FORK
#define CAN_RUN_AS_DAEMON 1
#endif
#endif

#define ARRAY_LENGTH(__array) ((int) (sizeof (__array) / sizeof (__array[0])))

static int
_cairo_writen (int fd, char *buf, int len)
{
    while (len) {
	int ret;

	ret = write (fd, buf, len);
	if (ret == -1) {
	    int err = errno;
	    switch (err) {
	    case EINTR:
	    case EAGAIN:
		continue;
	    default:
		return 0;
	    }
	}
	len -= ret;
	buf += ret;
    }

    return 1;
}

static int
_cairo_write (int fd,
	char *buf, int maxlen, int buflen,
	const unsigned char *src, int srclen)
{
    if (buflen < 0)
	return buflen;

    while (srclen) {
	int len;

	len = buflen + srclen;
	if (len > maxlen)
	    len = maxlen;
	len -= buflen;

	memcpy (buf + buflen, src, len);
	buflen += len;
	srclen -= len;
	src += len;

	if (buflen == maxlen) {
	    if (! _cairo_writen (fd, buf, buflen))
		return -1;

	    buflen = 0;
	}
    }

    return buflen;
}

static const char *
write_ppm (cairo_surface_t *surface, int fd)
{
    char buf[4096];
    cairo_format_t format;
    const char *format_str;
    const unsigned char *data;
    int len;
    int width, height, stride;
    int i, j;

    data = cairo_image_surface_get_data (surface);
    height = cairo_image_surface_get_height (surface);
    width = cairo_image_surface_get_width (surface);
    stride = cairo_image_surface_get_stride (surface);
    format = cairo_image_surface_get_format (surface);
    if (format == CAIRO_FORMAT_ARGB32) {
	/* see if we can convert to a standard ppm type and trim a few bytes */
	const unsigned char *alpha = data;
	for (j = height; j--; alpha += stride) {
	    for (i = 0; i < width; i++) {
		if ((*(unsigned int *) (alpha+4*i) & 0xff000000) != 0xff000000)
		    goto done;
	    }
	}
	format = CAIRO_FORMAT_RGB24;
 done: ;
    }

    switch (format) {
    case CAIRO_FORMAT_ARGB32:
	/* XXX need true alpha for svg */
	format_str = "P7";
	break;
    case CAIRO_FORMAT_RGB24:
	format_str = "P6";
	break;
    case CAIRO_FORMAT_A8:
	format_str = "P5";
	break;
    case CAIRO_FORMAT_A1:
    case CAIRO_FORMAT_RGB16_565:
    case CAIRO_FORMAT_RGB30:
    case CAIRO_FORMAT_RGB96F:
    case CAIRO_FORMAT_RGBA128F:
    case CAIRO_FORMAT_INVALID:
    default:
	return "unhandled image format";
    }

    len = sprintf (buf, "%s %d %d 255\n", format_str, width, height);
    for (j = 0; j < height; j++) {
	const unsigned int *row = (unsigned int *) (data + stride * j);

	switch ((int) format) {
	case CAIRO_FORMAT_ARGB32:
	    len = _cairo_write (fd,
			  buf, sizeof (buf), len,
			  (unsigned char *) row, 4 * width);
	    break;
	case CAIRO_FORMAT_RGB24:
	    for (i = 0; i < width; i++) {
		unsigned char rgb[3];
		unsigned int p = *row++;
		rgb[0] = (p & 0xff0000) >> 16;
		rgb[1] = (p & 0x00ff00) >> 8;
		rgb[2] = (p & 0x0000ff) >> 0;
		len = _cairo_write (fd,
			      buf, sizeof (buf), len,
			      rgb, 3);
	    }
	    break;
	case CAIRO_FORMAT_A8:
	    len = _cairo_write (fd,
			  buf, sizeof (buf), len,
			  (unsigned char *) row, width);
	    break;
	}
	if (len < 0)
	    return "write failed";
    }

    if (len && ! _cairo_writen (fd, buf, len))
	return "write failed";

    return NULL;
}

#if CAIRO_HAS_INTERPRETER
static cairo_surface_t *
_create_image (void *closure,
	       cairo_content_t content,
	       double width, double height,
	       long uid)
{
    cairo_surface_t **out = closure;
    cairo_format_t format;
    switch (content) {
    case CAIRO_CONTENT_ALPHA:
	format = CAIRO_FORMAT_A8;
	break;
    case CAIRO_CONTENT_COLOR:
	format = CAIRO_FORMAT_RGB24;
	break;
    default:
    case CAIRO_CONTENT_COLOR_ALPHA:
	format = CAIRO_FORMAT_ARGB32;
	break;
    }
    *out = cairo_image_surface_create (format, width, height);
    return cairo_surface_reference (*out);
}

static const char *
_cairo_script_render_page (const char *filename,
			   cairo_surface_t **surface_out)
{
    cairo_script_interpreter_t *csi;
    cairo_surface_t *surface = NULL;
    cairo_status_t status;
    const cairo_script_interpreter_hooks_t hooks = {
	&surface,
	_create_image,
	NULL, /* surface_destroy */
	NULL, /* context_create */
	NULL, /* context_destroy */
	NULL, /* show_page */
	NULL  /* copy_page */
    };

    csi = cairo_script_interpreter_create ();
    cairo_script_interpreter_install_hooks (csi, &hooks);
    status = cairo_script_interpreter_run (csi, filename);
    if (status) {
	cairo_surface_destroy (surface);
	surface = NULL;
    }
    status = cairo_script_interpreter_destroy (csi);
    if (surface == NULL)
	return "cairo-script interpreter failed";

    if (status == CAIRO_STATUS_SUCCESS)
	status = cairo_surface_status (surface);
    if (status) {
	cairo_surface_destroy (surface);
	return cairo_status_to_string (status);
    }

    *surface_out = surface;
    return NULL;
}

static const char *
cs_convert (char **argv, int fd)
{
    const char *err;
    cairo_surface_t *surface = NULL; /* silence compiler warning */

    err = _cairo_script_render_page (argv[0], &surface);
    if (err != NULL)
	return err;

    err = write_ppm (surface, fd);
    cairo_surface_destroy (surface);

    return err;
}
#else
static const char *
cs_convert (char **argv, int fd)
{
    return "compiled without CairoScript support.";
}
#endif

#if CAIRO_CAN_TEST_PDF_SURFACE
/* adapted from pdf2png.c */
static const char *
_poppler_render_page (const char *filename,
		      const char *page_label,
		      cairo_surface_t **surface_out)
{
    PopplerDocument *document;
    PopplerPage *page;
    double width, height;
    GError *error = NULL;
    gchar *absolute, *uri;
    cairo_surface_t *surface;
    cairo_t *cr;
    cairo_status_t status;

    if (g_path_is_absolute (filename)) {
	absolute = g_strdup (filename);
    } else {
	gchar *dir = g_get_current_dir ();
	absolute = g_build_filename (dir, filename, (gchar *) 0);
	g_free (dir);
    }

    uri = g_filename_to_uri (absolute, NULL, &error);
    g_free (absolute);
    if (uri == NULL)
	return error->message; /* XXX g_error_free (error) */

    document = poppler_document_new_from_file (uri, NULL, &error);
    g_free (uri);
    if (document == NULL)
	return error->message; /* XXX g_error_free (error) */

    page = poppler_document_get_page_by_label (document, page_label);
    g_object_unref (document);
    if (page == NULL)
	return "page not found";

    poppler_page_get_size (page, &width, &height);

    surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, width, height);
    cr = cairo_create (surface);

    cairo_set_source_rgb (cr, 1., 1., 1.);
    cairo_paint (cr);
    cairo_push_group_with_content (cr, CAIRO_CONTENT_COLOR_ALPHA);

    poppler_page_render (page, cr);
    g_object_unref (page);

    cairo_pop_group_to_source (cr);
    cairo_paint (cr);

    status = cairo_status (cr);
    cairo_destroy (cr);

    if (status) {
	cairo_surface_destroy (surface);
	return  cairo_status_to_string (status);
    }

    *surface_out = surface;
    return NULL;
}

static const char *
pdf_convert (char **argv, int fd)
{
    const char *err;
    cairo_surface_t *surface = NULL; /* silence compiler warning */

    err = _poppler_render_page (argv[0], argv[1], &surface);
    if (err != NULL)
	return err;

    err = write_ppm (surface, fd);
    cairo_surface_destroy (surface);

    return err;
}
#else
static const char *
pdf_convert (char **argv, int fd)
{
    return "compiled without PDF support.";
}
#endif

#if CAIRO_CAN_TEST_SVG_SURFACE
static const char *
_rsvg_render_page (const char *filename,
		   cairo_surface_t **surface_out)
{
    RsvgHandle *handle;
    RsvgDimensionData dimensions;
    GError *error = NULL;
    cairo_surface_t *surface;
    cairo_t *cr;
    cairo_status_t status;

    handle = rsvg_handle_new_from_file (filename, &error);
    if (handle == NULL)
	return error->message; /* XXX g_error_free */

    rsvg_handle_set_dpi (handle, 72.0);
    rsvg_handle_get_dimensions (handle, &dimensions);
    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
					  dimensions.width,
					  dimensions.height);
    cr = cairo_create (surface);

    rsvg_handle_render_cairo (handle, cr);
    g_object_unref (handle);

    status = cairo_status (cr);
    cairo_destroy (cr);

    if (status) {
	cairo_surface_destroy (surface);
	return  cairo_status_to_string (status);
    }

    *surface_out = surface;
    return NULL;
}

static const char *
svg_convert (char **argv, int fd)
{
    const char *err;
    cairo_surface_t *surface = NULL; /* silence compiler warning */

    err = _rsvg_render_page (argv[0], &surface);
    if (err != NULL)
	return err;

    err = write_ppm (surface, fd);
    cairo_surface_destroy (surface);

    return err;
}
#else
static const char *
svg_convert (char **argv, int fd)
{
    return "compiled without SVG support.";
}
#endif

#if CAIRO_HAS_SPECTRE
static const char *
_spectre_render_page (const char *filename,
		      const char *page_label,
		      cairo_surface_t **surface_out)
{
    static const cairo_user_data_key_t key;

    SpectreDocument *document;
    SpectreStatus status;
    int width, height, stride;
    unsigned char *pixels;
    cairo_surface_t *surface;

    document = spectre_document_new ();
    spectre_document_load (document, filename);
    status = spectre_document_status (document);
    if (status) {
	spectre_document_free (document);
	return spectre_status_to_string (status);
    }

    if (page_label) {
	SpectrePage *page;
	SpectreRenderContext *rc;

	page = spectre_document_get_page_by_label (document, page_label);
	spectre_document_free (document);
	if (page == NULL)
	    return "page not found";

	spectre_page_get_size (page, &width, &height);
	rc = spectre_render_context_new ();
	spectre_render_context_set_page_size (rc, width, height);
	spectre_page_render (page, rc, &pixels, &stride);
	spectre_render_context_free (rc);
	status = spectre_page_status (page);
	spectre_page_free (page);
	if (status) {
	    free (pixels);
	    return spectre_status_to_string (status);
	}
    } else {
	spectre_document_get_page_size (document, &width, &height);
	spectre_document_render (document, &pixels, &stride);
	spectre_document_free (document);
    }

    surface = cairo_image_surface_create_for_data (pixels,
						   CAIRO_FORMAT_RGB24,
						   width, height,
						   stride);
    cairo_surface_set_user_data (surface, &key,
				 pixels, (cairo_destroy_func_t) free);
    *surface_out = surface;
    return NULL;
}

static const char *
ps_convert (char **argv, int fd)
{
    const char *err;
    cairo_surface_t *surface = NULL; /* silence compiler warning */

    err = _spectre_render_page (argv[0], argv[1], &surface);
    if (err != NULL)
	return err;

    err = write_ppm (surface, fd);
    cairo_surface_destroy (surface);

    return err;
}
#else
static const char *
ps_convert (char **argv, int fd)
{
    return "compiled without PostScript support.";
}
#endif

static const char *
convert (char **argv, int fd)
{
    static const struct converter {
	const char *type;
	const char *(*func) (char **, int);
    } converters[] = {
	{ "cs", cs_convert },
	{ "pdf", pdf_convert },
	{ "ps", ps_convert },
	{ "svg", svg_convert },
	{ NULL, NULL }
    };
    const struct converter *converter = converters;
    char *type;

    type = strrchr (argv[0], '.');
    if (type == NULL)
	return "no file extension";
    type++;

    while (converter->type) {
	if (strcmp (type, converter->type) == 0)
	    return converter->func (argv, fd);
	converter++;
    }
    return "no converter";
}

#if CAN_RUN_AS_DAEMON
static int
_getline (int fd, char **linep, size_t *lenp)
{
    char *line;
    size_t len, i;
    ssize_t ret;

    line = *linep;
    if (line == NULL) {
	line = malloc (1024);
	if (line == NULL)
	    return -1;
	line[0] = '\0';
	len = 1024;
    } else
	len = *lenp;

    /* XXX simple, but ugly! */
    i = 0;
    do {
	if (i == len - 1) {
	    char *nline;

	    nline = realloc (line, len + 1024);
	    if (nline == NULL)
		goto out;

	    line = nline;
	    len += 1024;
	}

	ret = read (fd, line + i, 1);
	if (ret == -1 || ret == 0)
	    goto out;
    } while (line[i++] != '\n');

out:
    line[i] = '\0';
    *linep = line;
    *lenp = len;
    return i-1;
}

static int
split_line (char *line, char *argv[], int max_argc)
{
    int i = 0;

    max_argc--; /* leave one spare for the trailing NULL */

    argv[i++] = line;
    while (i < max_argc && (line = strchr (line, ' ')) != NULL) {
	*line++ = '\0';
	argv[i++] = line;
    }

    /* chomp the newline */
    line = strchr (argv[i-1], '\n');
    if (line != NULL)
	*line = '\0';

    argv[i] = NULL;

    return i;
}

static int
any2ppm_daemon_exists (void)
{
    struct stat st;
    int fd;
    char buf[80];
    int pid;
    int ret;

    if (stat (SOCKET_PATH, &st) < 0)
	return 0;

    fd = open (SOCKET_PATH ".pid", O_RDONLY);
    if (fd < 0)
	return 0;

    pid = 0;
    ret = read (fd, buf, sizeof (buf) - 1);
    if (ret > 0) {
	buf[ret] = '\0';
	pid = atoi (buf);
    }
    close (fd);

    return pid > 0 && kill (pid, 0) == 0;
}

static int
write_pid_file (void)
{
    int fd;
    char buf[80];
    int ret;

    fd = open (SOCKET_PATH ".pid", O_CREAT | O_TRUNC | O_WRONLY, 0666);
    if (fd < 0)
	return 0;

    ret = sprintf (buf, "%d\n", getpid ());
    ret = write (fd, buf, ret) == ret;
    close (fd);

    return ret;
}

static int
open_devnull_to_fd (int want_fd, int flags)
{
    int error;
    int got_fd;

    close (want_fd);

    got_fd = open("/dev/null", flags | O_CREAT, 0700);
    if (got_fd == -1)
        return -1;

    error = dup2 (got_fd, want_fd);
    close (got_fd);

    return error;
}

static int
daemonize (void)
{
    void (*oldhup) (int);

    /* Let the parent go. */
    switch (fork ()) {
    case -1: return -1;
    case 0: break;
    default: _exit (0);
    }

    /* Become session leader. */
    if (setsid () == -1)
	return -1;

    /* Refork to yield session leadership. */
    oldhup = signal (SIGHUP, SIG_IGN);

    switch (fork ()) {		/* refork to yield session leadership. */
    case -1: return -1;
    case 0: break;
    default: _exit (0);
    }

    signal (SIGHUP, oldhup);

    /* Establish stdio. */
    if (open_devnull_to_fd (0, O_RDONLY) == -1)
	return -1;
    if (open_devnull_to_fd (1, O_WRONLY | O_APPEND) == -1)
	return -1;
    if (dup2 (1, 2) == -1)
	return -1;

    return 0;
}

static const char *
any2ppm_daemon (void)
{
    int timeout = TIMEOUT;
    struct pollfd pfd;
    int sk, fd;
    long flags;
    struct sockaddr_un addr;
    char *line = NULL;
    size_t len = 0;

#ifdef SIGPIPE
    signal (SIGPIPE, SIG_IGN);
#endif

    /* XXX racy! */
    if (getenv ("ANY2PPM_FORCE") == NULL && any2ppm_daemon_exists ())
	return "any2ppm daemon already running";

    unlink (SOCKET_PATH);

    sk = socket (PF_UNIX, SOCK_STREAM, 0);
    if (sk == -1)
	return "unable to create socket";

    memset (&addr, 0, sizeof (addr));
    addr.sun_family = AF_UNIX;
    strcpy (addr.sun_path, SOCKET_PATH);
    if (bind (sk, (struct sockaddr *) &addr, sizeof (addr)) == -1) {
	close (sk);
	return "unable to bind socket";
    }

    flags = fcntl (sk, F_GETFL);
    if (flags == -1 || fcntl (sk, F_SETFL, flags | O_NONBLOCK) == -1) {
	close (sk);
	return "unable to set socket to non-blocking";
    }

    if (listen (sk, 5) == -1) {
	close (sk);
	return "unable to listen on socket";
    }

    /* ready for client connection - detach from parent/terminal */
    if (getenv ("ANY2PPM_NODAEMON") == NULL && daemonize () == -1) {
	close (sk);
	return "unable to detach from parent";
    }

    if (! write_pid_file ()) {
	close (sk);
	return "unable to write pid file";
    }

    if (getenv ("ANY2PPM_TIMEOUT") != NULL) {
	timeout = atoi (getenv ("ANY2PPM_TIMEOUT"));
	if (timeout == 0)
	    timeout = -1;
	if (timeout > 0)
	    timeout *= 1000; /* convert env (in seconds) to milliseconds */
    }

    pfd.fd = sk;
    pfd.events = POLLIN;
    pfd.revents = 0; /* valgrind */
    while (poll (&pfd, 1, timeout) > 0) {
	while ((fd = accept (sk, NULL, NULL)) != -1) {
	    if (_getline (fd, &line, &len) != -1) {
		char *argv[10];

		if (split_line (line, argv, ARRAY_LENGTH (argv)) > 0) {
		    const char *err;

		    err = convert (argv, fd);
		    if (err != NULL) {
			FILE *file = fopen (".any2ppm.errors", "a");
			if (file != NULL) {
			    fprintf (file,
				     "Failed to convert '%s': %s\n",
				     argv[0], err);
			    fclose (file);
			}
		    }
		}
	    }
	    close (fd);
	}
    }
    close (sk);
    unlink (SOCKET_PATH);
    unlink (SOCKET_PATH ".pid");

    free (line);
    return NULL;
}
#else
static const char *
any2ppm_daemon (void)
{
    return "daemon not compiled in.";
}
#endif

int
main (int argc, char **argv)
{
    const char *err;

#if CAIRO_CAN_TEST_PDF_SURFACE || CAIRO_CAN_TEST_SVG_SURFACE
#if GLIB_MAJOR_VERSION <= 2 && GLIB_MINOR_VERSION <= 34
    g_type_init ();
#endif
#endif

#if defined(_WIN32) && !defined (__CYGWIN__)
    _setmode (1, _O_BINARY);
#endif

    if (argc == 1)
	err = any2ppm_daemon ();
    else
	err = convert (argv + 1, 1);
    if (err != NULL) {
	fprintf (stderr, "Failed to run converter: %s\n", err);
	return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
