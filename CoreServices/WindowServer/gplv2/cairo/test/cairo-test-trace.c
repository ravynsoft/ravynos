/*
 * Copyright © 2009 Chris Wilson
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
 * Authors: Chris Wilson <chris@chris-wilson.co.uk>
 */

/*
 * The basic idea is that we feed the trace to multiple backends in parallel
 * and compare the output at the end of each context (based on the premise
 * that contexts demarcate expose events, or their logical equivalents) with
 * that of the image[1] backend. Each backend is executed in a separate
 * process, for robustness and to isolate the global cairo state, with the
 * image data residing in shared memory and synchronising over a socket.
 *
 * [1] Should be reference implementation, currently the image backend is
 *     considered to be the reference for all other backends.
 */

/* XXX Can't directly compare fills using spans versus trapezoidation,
 *     i.e. xlib vs image. Gah, kinda renders this whole scheme moot.
 *     How about reference platforms?
 *     E.g. accelerated xlib driver vs Xvfb?
 *
 *     boilerplate->create_reference_surface()?
 *     boilerplate->reference->create_surface()?
 *     So for each backend spawn two processes, a reference and xlib
 *     (obviously minimising the number of reference processes when possible)
 */

/*
 * XXX Handle show-page as well as cairo_destroy()? Though arguably that is
 *     only relevant for paginated backends which is currently outside the
 *     scope of this test.
 */

#include "cairo-test.h"
#include "buffer-diff.h"

#include "cairo-boilerplate-getopt.h"
#include <cairo-script-interpreter.h>
#include "cairo-missing.h"

#if CAIRO_HAS_SCRIPT_SURFACE
#include <cairo-script.h>
#endif

/* For basename */
#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif
#include <ctype.h> /* isspace() */

#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/un.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>

#if CAIRO_HAS_REAL_PTHREAD
#include <pthread.h>
#endif

#if defined(HAVE_POLL_H)
#include <poll.h>
#elif defined(HAVE_SYS_POLL_H)
#include <sys/poll.h>
#else
#error No poll.h equivalent found
#endif

#if HAVE_FCFINI
#include <fontconfig/fontconfig.h>
#endif

#ifndef MAP_NORESERVE
#define MAP_NORESERVE 0
#endif

#define DEBUG 0

#define ignore_image_differences 0 /* XXX make me a cmdline option! */
#define write_results 1
#define write_traces 1

#define DATA_SIZE (256 << 20)
#define SHM_PATH_XXX "/.shmem-cairo-trace"

typedef struct _test_trace {
    /* Options from command-line */
    cairo_bool_t list_only;
    char **names;
    unsigned int num_names;
    char **exclude_names;
    unsigned int num_exclude_names;

    /* Stuff used internally */
    const cairo_boilerplate_target_t **targets;
    int num_targets;
} test_trace_t;

typedef struct _test_runner {
    const char *name;
    cairo_surface_t *surface;
    void *closure;
    uint8_t *base;
    const char *trace;
    pid_t pid;
    int sk;
    cairo_bool_t is_recording;

    cairo_script_interpreter_t *csi;
    struct context_closure {
	struct context_closure *next;
	unsigned long id;
	unsigned long start_line;
	unsigned long end_line;
	cairo_t *context;
	cairo_surface_t *surface;
    } *contexts;

    unsigned long context_id;
} test_runner_t;

struct slave {
    pid_t pid;
    int fd;
    unsigned long image_serial;
    unsigned long image_ready;
    unsigned long start_line;
    unsigned long end_line;
    cairo_surface_t *image;
    long width, height;
    cairo_surface_t *difference;
    buffer_diff_result_t result;
    const cairo_boilerplate_target_t *target;
    const struct slave *reference;
    cairo_bool_t is_recording;
};

struct request_image {
    unsigned long id;
    unsigned long start_line;
    unsigned long end_line;
    cairo_format_t format;
    long width;
    long height;
    long stride;
};

struct surface_tag {
    long width, height;
};
static const cairo_user_data_key_t surface_tag;

#define TARGET_NAME(T)  ((T) ? (T)->name : "recording")

#if CAIRO_HAS_REAL_PTHREAD
#define tr_die(t) t->is_recording ? pthread_exit(NULL) : exit(1)
#else
#define tr_die(t) exit(1)
#endif

static cairo_bool_t
writen (int fd, const void *ptr, int len)
{
#if 0
    const uint8_t *data = ptr;
    while (len) {
	int ret = write (fd, data, len);
	if (ret < 0) {
	    switch (errno) {
	    case EAGAIN:
	    case EINTR:
		continue;
	    default:
		return FALSE;
	    }
	} else if (ret == 0) {
	    return FALSE;
	} else {
	    data += ret;
	    len -= ret;
	}
    }
    return TRUE;
#else
    int ret = send (fd, ptr, len, 0);
    return ret == len;
#endif
}

static cairo_bool_t
readn (int fd, void *ptr, int len)
{
#if 0
    uint8_t *data = ptr;
    while (len) {
	int ret = read (fd, data, len);
	if (ret < 0) {
	    switch (errno) {
	    case EAGAIN:
	    case EINTR:
		continue;
	    default:
		return FALSE;
	    }
	} else if (ret == 0) {
	    return FALSE;
	} else {
	    data += ret;
	    len -= ret;
	}
    }
    return TRUE;
#else
    int ret = recv (fd, ptr, len, MSG_WAITALL);
    return ret == len;
#endif
}

static cairo_format_t
format_for_content (cairo_content_t content)
{
    switch (content) {
    case CAIRO_CONTENT_ALPHA:
	return CAIRO_FORMAT_A8;
    case CAIRO_CONTENT_COLOR:
	return CAIRO_FORMAT_RGB24;
    default:
    case CAIRO_CONTENT_COLOR_ALPHA:
	return CAIRO_FORMAT_ARGB32;
    }
}

static void
send_recording_surface (test_runner_t *tr,
			int width, int height,
			struct context_closure *closure)
{
#if CAIRO_HAS_REAL_PTHREAD
    const struct request_image rq = {
	closure->id,
	closure->start_line,
	closure->end_line,
	-1,
	width, height,
	(long) closure->surface,
    };
    unsigned long offset;
    unsigned long serial;

    if (DEBUG > 1) {
	printf ("send-recording-surface: %lu [%lu, %lu]\n",
		closure->id,
		closure->start_line,
		closure->end_line);
    }
    writen (tr->sk, &rq, sizeof (rq));
    readn (tr->sk, &offset, sizeof (offset));

    /* signal completion */
    writen (tr->sk, &closure->id, sizeof (closure->id));

    /* wait for image check */
    serial = 0;
    readn (tr->sk, &serial, sizeof (serial));
    if (DEBUG > 1) {
	printf ("send-recording-surface: serial: %lu\n", serial);
    }
    if (serial != closure->id)
	pthread_exit (NULL);
#else
    exit (1);
#endif
}

static void *
request_image (test_runner_t *tr,
	       struct context_closure *closure,
	       cairo_format_t format,
	       int width, int height, int stride)
{
    const struct request_image rq = {
	closure->id,
	closure->start_line,
	closure->end_line,
	format, width, height, stride
    };
    unsigned long offset = -1;

    assert (format != (cairo_format_t) -1);

    writen (tr->sk, &rq, sizeof (rq));
    readn (tr->sk, &offset, sizeof (offset));
    if (offset == (unsigned long) -1)
	return NULL;

    return tr->base + offset;
}

static void
send_surface (test_runner_t *tr,
	      struct context_closure *closure)
{
    cairo_surface_t *source = closure->surface;
    cairo_surface_t *image;
    cairo_format_t format = (cairo_format_t) -1;
    cairo_t *cr;
    int width, height, stride;
    void *data;
    unsigned long serial;

    if (DEBUG > 1) {
	printf ("send-surface: '%s', is-recording? %d\n",
		tr->name, tr->is_recording);
    }

    if (cairo_surface_get_type (source) == CAIRO_SURFACE_TYPE_IMAGE) {
	width = cairo_image_surface_get_width (source);
	height = cairo_image_surface_get_height (source);
	format = cairo_image_surface_get_format (source);
    } else {
	struct surface_tag *tag;

	tag = cairo_surface_get_user_data (source, &surface_tag);
	if (tag != NULL) {
	    width = tag->width;
	    height = tag->height;
	} else {
	    double x0, x1, y0, y1;

	    /* presumably created using cairo_surface_create_similar() */
	    cr = cairo_create (source);
	    cairo_clip_extents (cr, &x0, &y0, &x1, &y1);
	    cairo_destroy (cr);

	    tag = xmalloc (sizeof (*tag));
	    width = tag->width = x1 - x0;
	    height = tag->height = y1 - y0;

	    if (cairo_surface_set_user_data (source, &surface_tag, tag, free))
		tr_die (tr);
	}
    }

    if (tr->is_recording) {
	send_recording_surface (tr, width, height, closure);
	return;
    }

    if (format == (cairo_format_t) -1)
	format = format_for_content (cairo_surface_get_content (source));

    stride = cairo_format_stride_for_width (format, width);

    data = request_image (tr, closure, format, width, height, stride);
    if (data == NULL)
	tr_die (tr);

    image = cairo_image_surface_create_for_data (data,
						 format,
						 width, height,
						 stride);
    cr = cairo_create (image);
    cairo_surface_destroy (image);

    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_surface (cr, source, 0, 0);
    cairo_paint (cr);
    cairo_destroy (cr);

    /* signal completion */
    writen (tr->sk, &closure->id, sizeof (closure->id));

    /* wait for image check */
    serial = 0;
    readn (tr->sk, &serial, sizeof (serial));
    if (serial != closure->id)
	tr_die (tr);
}

static cairo_surface_t *
_surface_create (void *closure,
		 cairo_content_t content,
		 double width, double height,
		 long uid)
{
    test_runner_t *tr = closure;
    cairo_surface_t *surface;

    surface = cairo_surface_create_similar (tr->surface,
					    content, width, height);
    if (cairo_surface_get_type (surface) != CAIRO_SURFACE_TYPE_IMAGE) {
	struct surface_tag *tag;

	tag = xmalloc (sizeof (*tag));
	tag->width = width;
	tag->height = height;
	if (cairo_surface_set_user_data (surface, &surface_tag, tag, free))
	    tr_die (tr);
    }

    return surface;
}

static cairo_t *
_context_create (void *closure, cairo_surface_t *surface)
{
    test_runner_t *tr = closure;
    struct context_closure *l;

    if (DEBUG) {
	fprintf (stderr, "%s: starting context %lu on line %d\n",
		 tr->name ? tr->name : "recording" ,
		 tr->context_id + 1,
		 cairo_script_interpreter_get_line_number (tr->csi));
    }

    l = xmalloc (sizeof (*l));
    l->next = tr->contexts;
    l->start_line = cairo_script_interpreter_get_line_number (tr->csi);
    l->end_line = l->start_line;
    l->context = cairo_create (surface);
    l->surface = cairo_surface_reference (surface);
    l->id = ++tr->context_id;
    if (l->id == 0)
	l->id = ++tr->context_id;
    tr->contexts = l;

    return l->context;
}

static void
_context_destroy (void *closure, void *ptr)
{
    test_runner_t *tr = closure;
    struct context_closure *l, **prev = &tr->contexts;

    while ((l = *prev) != NULL) {
	if (l->context == ptr) {
	    if (DEBUG) {
		fprintf (stderr, "%s: context %lu complete on line %d\n",
			 tr->name ? tr->name : "recording" ,
			 tr->context_id,
			 cairo_script_interpreter_get_line_number (tr->csi));
	    }
	    l->end_line =
		cairo_script_interpreter_get_line_number (tr->csi);
	    if (cairo_surface_status (l->surface) == CAIRO_STATUS_SUCCESS) {
		send_surface (tr, l);
            } else {
		fprintf (stderr, "%s: error during replay, line %lu: %s!\n",
			 tr->name,
			 l->end_line,
			 cairo_status_to_string (cairo_surface_status (l->surface)));
		tr_die (tr);
	    }

            cairo_surface_destroy (l->surface);
            *prev = l->next;
            free (l);
            return;
        }
        prev = &l->next;
    }
}

static void
execute (test_runner_t *tr)
{
    const cairo_script_interpreter_hooks_t hooks = {
	.closure = tr,
	.surface_create = _surface_create,
	.context_create = _context_create,
	.context_destroy = _context_destroy,
    };
    pid_t ack;

    tr->csi = cairo_script_interpreter_create ();
    cairo_script_interpreter_install_hooks (tr->csi, &hooks);

    ack = -1;
    readn (tr->sk, &ack, sizeof (ack));
    if (ack != tr->pid)
	tr_die (tr);

    cairo_script_interpreter_run (tr->csi, tr->trace);

    cairo_script_interpreter_finish (tr->csi);
    if (cairo_script_interpreter_destroy (tr->csi))
	tr_die (tr);
}

static int
spawn_socket (const char *socket_path, pid_t pid)
{
    struct sockaddr_un addr;
    int sk;

    sk = socket (PF_UNIX, SOCK_STREAM, 0);
    if (sk == -1)
	return -1;

    memset (&addr, 0, sizeof (addr));
    addr.sun_family = AF_UNIX;
    strcpy (addr.sun_path, socket_path);

    if (connect (sk, (struct sockaddr *) &addr, sizeof (addr)) == -1)
	return -1;

    if (! writen (sk, &pid, sizeof (pid)))
	return -1;

    return sk;
}

static void *
spawn_shm (const char *shm_path)
{
    void *base;
    int fd;

    fd = shm_open (shm_path, O_RDWR, 0);
    if (fd == -1)
	return MAP_FAILED;

    base = mmap (NULL, DATA_SIZE,
		 PROT_READ | PROT_WRITE,
#ifdef MAP_NORESERVE
		 MAP_SHARED | MAP_NORESERVE,
#else
		 MAP_SHARED,
#endif
		 fd, 0);
    close (fd);

    return base;
}

static int
spawn_target (const char *socket_path,
	      const char *shm_path,
	      const cairo_boilerplate_target_t *target,
	      const char *trace)
{
    test_runner_t tr;
    pid_t pid;

    if (DEBUG)
	printf ("Spawning slave '%s' for %s\n", target->name, trace);

    pid = fork ();
    if (pid != 0)
	return pid;

    tr.is_recording = FALSE;
    tr.pid = getpid ();

    tr.sk = spawn_socket (socket_path, tr.pid);
    if (tr.sk == -1) {
	fprintf (stderr, "%s: Failed to open socket.\n",
		 target->name);
	exit (-1);
    }

    tr.base = spawn_shm (shm_path);
    if (tr.base == MAP_FAILED) {
	fprintf (stderr, "%s: Failed to map shared memory segment.\n",
		 target->name);
	exit (-1);
    }

    tr.name = target->name;
    tr.contexts = NULL;
    tr.context_id = 0;
    tr.trace = trace;

    tr.surface = target->create_surface (NULL,
					 target->content,
					 1, 1,
					 1, 1,
					 CAIRO_BOILERPLATE_MODE_TEST,
					 &tr.closure);
    if (tr.surface == NULL) {
	fprintf (stderr,
		 "%s:  Failed to create target surface.\n",
		 target->name);
	exit (-1);
    }

    execute (&tr);

    cairo_surface_destroy (tr.surface);

    if (target->cleanup)
	target->cleanup (tr.closure);

    close (tr.sk);
    munmap (tr.base, DATA_SIZE);

    exit (0);
}

#if CAIRO_HAS_REAL_PTHREAD
static void
cleanup_recorder (void *arg)
{
    test_runner_t *tr = arg;

    cairo_surface_finish (tr->surface);
    cairo_surface_destroy (tr->surface);

    close (tr->sk);
    free (tr);
}

static void *
record (void *arg)
{
    test_runner_t *tr = arg;

    pthread_cleanup_push (cleanup_recorder, tr);
    execute (tr);
    pthread_cleanup_pop (TRUE);

    return NULL;
}

/* The recorder is special:
 * 1. It doesn't generate an image, but keeps an in-memory trace to
 *    reconstruct any surface.
 * 2. Runs in the same process, but separate thread.
 */
static pid_t
spawn_recorder (const char *socket_path, const char *trace, test_runner_t **out)
{
    test_runner_t *tr;
    pthread_t id;
    pthread_attr_t attr;
    pid_t pid = getpid ();

    if (DEBUG)
	printf ("Spawning recorder for %s\n", trace);

    tr = malloc (sizeof (*tr));
    if (tr == NULL)
	return -1;

    tr->is_recording = TRUE;
    tr->pid = pid;
    tr->sk = spawn_socket (socket_path, tr->pid);
    if (tr->sk == -1) {
	free (tr);
	return -1;
    }

    tr->base = NULL;
    tr->name = NULL;
    tr->contexts = NULL;
    tr->context_id = 0;
    tr->trace = trace;

    tr->surface = cairo_recording_surface_create (CAIRO_CONTENT_COLOR_ALPHA,
						  NULL);
    if (tr->surface == NULL) {
	cleanup_recorder (tr);
	return -1;
    }

    pthread_attr_init (&attr);
    pthread_attr_setdetachstate (&attr, TRUE);
    if (pthread_create (&id, &attr, record, tr) < 0) {
	pthread_attr_destroy (&attr);
	cleanup_recorder (tr);
	return -1;
    }
    pthread_attr_destroy (&attr);


    *out = tr;
    return pid;
}
#endif

/* XXX imagediff - is the extra expense worth it? */
static cairo_bool_t
matches_reference (struct slave *slave)
{
    cairo_surface_t *a, *b;

    a = slave->image;
    b = slave->reference->image;

    if (a == b)
	return TRUE;

    if (a == NULL || b == NULL)
	return FALSE;

    if (cairo_surface_status (a) || cairo_surface_status (b))
	return FALSE;

    if (cairo_surface_get_type (a) != cairo_surface_get_type (b))
	return FALSE;

    if (cairo_image_surface_get_format (a) != cairo_image_surface_get_format (b))
	return FALSE;

    if (cairo_image_surface_get_width (a) != cairo_image_surface_get_width (b))
	return FALSE;

    if (cairo_image_surface_get_height (a) != cairo_image_surface_get_height (b))
	return FALSE;

    if (cairo_image_surface_get_stride (a) != cairo_image_surface_get_stride (b))
	return FALSE;

    if (FALSE && cairo_surface_get_content (a) & CAIRO_CONTENT_COLOR) {
	cairo_surface_t *diff;
	int width, height, stride, size;
	unsigned char *data;
	cairo_status_t status;

	width = cairo_image_surface_get_width (a);
	height = cairo_image_surface_get_height (a);
	stride = cairo_image_surface_get_stride (a);
	size = height * stride * 4;
	data = malloc (size);
	if (data == NULL)
	    return FALSE;

	diff = cairo_image_surface_create_for_data (data,
						    cairo_image_surface_get_format (a),
						    width, height, stride);
	cairo_surface_set_user_data (diff, (cairo_user_data_key_t *) diff,
				     data, free);

	status = image_diff (NULL, a, b, diff, &slave->result);
	if (status) {
	    cairo_surface_destroy (diff);
	    return FALSE;
	}

	if (image_diff_is_failure (&slave->result, slave->target->error_tolerance)) {
	    slave->difference = diff;
	    return FALSE;
	} else {
	    cairo_surface_destroy (diff);
	    return TRUE;
	}
    } else {
	int width, height, stride;
	const uint8_t *aa, *bb;
	int x, y;

	width = cairo_image_surface_get_width (a);
	height = cairo_image_surface_get_height (a);
	stride = cairo_image_surface_get_stride (a);

	aa = cairo_image_surface_get_data (a);
	bb = cairo_image_surface_get_data (b);
	switch (cairo_image_surface_get_format (a)) {
	case CAIRO_FORMAT_ARGB32:
	    for (y = 0; y < height; y++) {
		const uint32_t *ua = (uint32_t *) aa;
		const uint32_t *ub = (uint32_t *) bb;
		for (x = 0; x < width; x++) {
		    if (ua[x] != ub[x]) {
			int channel;

			for (channel = 0; channel < 4; channel++) {
			    int va, vb;
			    unsigned diff;

			    va = (ua[x] >> (channel*8)) & 0xff;
			    vb = (ub[x] >> (channel*8)) & 0xff;
			    diff = abs (va - vb);
			    if (diff > slave->target->error_tolerance)
				return FALSE;
			}
		    }
		}
		aa += stride;
		bb += stride;
	    }
	    break;

	case CAIRO_FORMAT_RGB24:
	    for (y = 0; y < height; y++) {
		const uint32_t *ua = (uint32_t *) aa;
		const uint32_t *ub = (uint32_t *) bb;
		for (x = 0; x < width; x++) {
		    if ((ua[x] & 0x00ffffff) != (ub[x] & 0x00ffffff)) {
			int channel;

			for (channel = 0; channel < 3; channel++) {
			    int va, vb;
			    unsigned diff;

			    va = (ua[x] >> (channel*8)) & 0xff;
			    vb = (ub[x] >> (channel*8)) & 0xff;
			    diff = abs (va - vb);
			    if (diff > slave->target->error_tolerance)
				return FALSE;
			}
		    }
		}
		aa += stride;
		bb += stride;
	    }
	    break;

	case CAIRO_FORMAT_A8:
	    for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
		    if (aa[x] != bb[x]) {
			unsigned diff = abs (aa[x] - bb[x]);
			if (diff > slave->target->error_tolerance)
			    return FALSE;
		    }
		}
		aa += stride;
		bb += stride;
	    }
	    break;

	case CAIRO_FORMAT_A1:
	    width /= 8;
	    for (y = 0; y < height; y++) {
		if (memcmp (aa, bb, width))
		    return FALSE;
		aa += stride;
		bb += stride;
	    }
	    break;

	case CAIRO_FORMAT_RGB30:
	case CAIRO_FORMAT_RGB16_565:
        case CAIRO_FORMAT_RGB96F:
        case CAIRO_FORMAT_RGBA128F:
	case CAIRO_FORMAT_INVALID:
	    assert (0);
	}

	return TRUE;
    }
}

static cairo_bool_t
check_images (struct slave *slaves, int num_slaves)
{
    int n;

    if (ignore_image_differences)
	return TRUE;

    for (n = 0; n < num_slaves; n++) {
	if (slaves[n].reference == NULL)
	    continue;

	if (! matches_reference (&slaves[n]))
	    return FALSE;
    }

    return TRUE;
}

static void
write_images (const char *trace, struct slave *slave, int num_slaves)
{
    while (num_slaves--) {
	if (slave->image != NULL && ! slave->is_recording) {
	    char *filename;

	    xasprintf (&filename, "%s-%s-fail.png",
		       trace, slave->target->name);
	    cairo_surface_write_to_png (slave->image, filename);
	    free (filename);

	    if (slave->difference) {
		xasprintf (&filename, "%s-%s-diff.png",
			   trace, slave->target->name);
		cairo_surface_write_to_png (slave->difference, filename);
		free (filename);
	    }
	}

	slave++;
    }
}

static void
write_result (const char *trace, struct slave *slave)
{
    static int index;
    char *filename;

    xasprintf (&filename, "%s-%s-pass-%d-%ld-%ld.png",
	       trace, slave->target->name, ++index,
	       slave->start_line, slave->end_line);
    cairo_surface_write_to_png (slave->image, filename);
    free (filename);
}

static void
write_trace (const char *trace, const char *id, struct slave *slave)
{
#if CAIRO_HAS_SCRIPT_SURFACE
    cairo_device_t *script;
    char *filename;

    assert (slave->is_recording);

    xasprintf (&filename, "%s-%s.trace", trace, id);

    script = cairo_script_create (filename);
    cairo_script_from_recording_surface (script, slave->image);
    cairo_device_destroy (script);

    free (filename);
#endif
}

static void
dump_traces (test_runner_t *tr,
	     const char *trace,
	     const char *target,
	     const char *fail)
{
#if CAIRO_HAS_SCRIPT_SURFACE
    struct context_closure *c;

    for (c = tr->contexts; c; c = c->next) {
	cairo_device_t *script;
	char *filename;

	xasprintf (&filename, "%s-%s-%s.%lu.trace",
		   trace, target, fail, c->start_line);

	script = cairo_script_create (filename);
	cairo_script_from_recording_surface (script, c->surface);
	cairo_device_destroy (script);

	free (filename);
    }
#endif
}

static unsigned long
allocate_image_for_slave (uint8_t *base,
			  unsigned long offset,
			  struct slave *slave)
{
    struct request_image rq;
    int size;
    uint8_t *data;

    assert (slave->image == NULL);

    readn (slave->fd, &rq, sizeof (rq));
    slave->image_serial = rq.id;
    slave->start_line = rq.start_line;
    slave->end_line = rq.end_line;

    slave->width = rq.width;
    slave->height = rq.height;

    if (DEBUG > 1) {
	printf ("allocate-image-for-slave: %s %lu [%lu, %lu] %ldx%ld stride=%lu => %lu, is-recording? %d\n",
		TARGET_NAME (slave->target),
		slave->image_serial,
		slave->start_line,
		slave->end_line,
		slave->width,
		slave->height,
		rq.stride,
		offset,
		slave->is_recording);
    }

    if (slave->is_recording) {
	/* special communication with recording-surface thread */
	slave->image = cairo_surface_reference ((cairo_surface_t *) rq.stride);
    } else {
	size = rq.height * rq.stride;
	size = (size + 4095) & -4096;
	data = base + offset;
	offset += size;
	assert (offset <= DATA_SIZE);

	slave->image = cairo_image_surface_create_for_data (data, rq.format,
							    rq.width, rq.height,
							    rq.stride);
    }

    return offset;
}

struct error_info {
    unsigned long context_id;
    unsigned long start_line;
    unsigned long end_line;
};

static cairo_bool_t
test_run (void *base,
	  int sk,
	  const char *trace,
	  struct slave *slaves,
	  int num_slaves,
	  struct error_info *error)
{
    struct pollfd *pfd;
    int npfd, cnt, n, i;
    int completion, err = 0;
    cairo_bool_t ret = FALSE;
    unsigned long image;

    if (DEBUG) {
	printf ("Running trace '%s' over %d slaves\n",
		trace, num_slaves);
    }

    pfd = xcalloc (num_slaves+1, sizeof (*pfd));

    pfd[0].fd = sk;
    pfd[0].events = POLLIN;
    npfd = 1;

    completion = 0;
    image = 0;
    while ((cnt = poll (pfd, npfd, -1)) > 0) {
	if (pfd[0].revents) {
	    int fd;

	    while ((fd = accept (sk, NULL, NULL)) != -1) {
		pid_t pid;

		readn (fd, &pid, sizeof (pid));
		for (n = 0; n < num_slaves; n++) {
		    if (slaves[n].pid == pid) {
			slaves[n].fd = fd;
			break;
		    }
		}
		if (n == num_slaves) {
		    if (DEBUG)
			printf ("unknown slave pid\n");
		    goto out;
		}

		pfd[npfd].fd = fd;
		pfd[npfd].events = POLLIN;
		npfd++;

		if (! writen (fd, &pid, sizeof (pid)))
		    goto out;
	    }
	    cnt--;
	}

	for (n = 1; n < npfd && cnt; n++) {
	    if (! pfd[n].revents)
		continue;

	    if (pfd[n].revents & POLLHUP) {
		pfd[n].events = pfd[n].revents = 0;
		completion++;
		continue;
	    }

	    for (i = 0; i < num_slaves; i++) {
		if (slaves[i].fd == pfd[n].fd) {
		    /* Communication with the slave is done in three phases,
		     * and we do each pass synchronously.
		     *
		     * 1. The slave requests an image buffer, which we
		     * allocate and then return to the slave the offset into
		     * the shared memory segment.
		     *
		     * 2. The slave indicates that it has finished writing
		     * into the shared image buffer. The slave now waits
		     * for the server to collate all the image data - thereby
		     * throttling the slaves.
		     *
		     * 3. After all slaves have finished writing their images,
		     * we compare them all against the reference image and,
		     * if satisfied, send an acknowledgement to all slaves.
		     */
		    if (slaves[i].image_serial == 0) {
			unsigned long offset;

			image =
			    allocate_image_for_slave (base,
						      offset = image,
						      &slaves[i]);
			if (! writen (pfd[n].fd, &offset, sizeof (offset))) {
			    pfd[n].events = pfd[n].revents = 0;
			    err = 1;
			    completion++;
			    continue;
			}
		    } else {
			readn (pfd[n].fd,
			       &slaves[i].image_ready,
			       sizeof (slaves[i].image_ready));
			if (DEBUG) {
			    printf ("slave '%s' reports completion on %lu (expecting %lu)\n",
				    TARGET_NAME (slaves[i].target),
				    slaves[i].image_ready,
				    slaves[i].image_serial);
			}
			if (slaves[i].image_ready != slaves[i].image_serial) {
			    pfd[n].events = pfd[n].revents = 0;
			    err = 1;
			    completion++;
			    continue;
			}

			/* Can anyone spell 'P·E·D·A·N·T'? */
			if (! slaves[i].is_recording)
			    cairo_surface_mark_dirty (slaves[i].image);
			completion++;
		    }

		    break;
		}
	    }

	    cnt--;
	}

	if (completion >= num_slaves) {
	    if (err) {
		if (DEBUG > 1)
		    printf ("error detected\n");
		goto out;
	    }

	    if (DEBUG > 1) {
		printf ("all saves report completion\n");
	    }
	    if (slaves[0].end_line >= slaves[0].start_line &&
		! check_images (slaves, num_slaves)) {
		error->context_id = slaves[0].image_serial;
		error->start_line = slaves[0].start_line;
		error->end_line = slaves[0].end_line;

		if (DEBUG) {
		    printf ("check_images failed: %lu, [%lu, %lu]\n",
			    slaves[0].image_serial,
			    slaves[0].start_line,
			    slaves[0].end_line);
		}

		write_images (trace, slaves, num_slaves);

		if (slaves[0].is_recording)
		    write_trace (trace, "fail", &slaves[0]);

		goto out;
	    }

	    if (write_results) write_result (trace, &slaves[1]);
	    if (write_traces && slaves[0].is_recording) {
		char buf[80];
		snprintf (buf, sizeof (buf), "%ld", slaves[0].image_serial);
		write_trace (trace, buf, &slaves[0]);
	    }

	    /* ack */
	    for (i = 0; i < num_slaves; i++) {
		cairo_surface_destroy (slaves[i].image);
		slaves[i].image = NULL;

		if (DEBUG > 1) {
		    printf ("sending continuation to '%s'\n",
			    TARGET_NAME (slaves[i].target));
		}
		if (! writen (slaves[i].fd,
			      &slaves[i].image_serial,
			      sizeof (slaves[i].image_serial)))
		{
		    goto out;
		}

		slaves[i].image_serial = 0;
		slaves[i].image_ready = 0;
	    }

	    completion = 0;
	    image = 0;
	}
    }

    ret = TRUE;

out:
    if (DEBUG) {
	printf ("run complete: %d\n", ret);
    }

    for (n = 0; n < num_slaves; n++) {
	if (slaves[n].fd != -1)
	    close (slaves[n].fd);

	if (slaves[n].image == NULL)
	    continue;

	cairo_surface_destroy (slaves[n].image);
	slaves[n].image = NULL;

	cairo_surface_destroy (slaves[n].difference);
	slaves[n].difference = NULL;

	slaves[n].image_serial = 0;
	slaves[n].image_ready = 0;
    }

    free (pfd);

    return ret;
}

static int
server_socket (const char *socket_path)
{
    long flags;
    struct sockaddr_un addr;
    int sk;

    sk = socket (PF_UNIX, SOCK_STREAM, 0);
    if (sk == -1)
	return -1;

    memset (&addr, 0, sizeof (addr));
    addr.sun_family = AF_UNIX;
    strcpy (addr.sun_path, socket_path);
    if (bind (sk, (struct sockaddr *) &addr, sizeof (addr)) == -1) {
	close (sk);
	return -1;
    }

    flags = fcntl (sk, F_GETFL);
    if (flags == -1 || fcntl (sk, F_SETFL, flags | O_NONBLOCK) == -1) {
	close (sk);
	return -1;
    }

    if (listen (sk, 5) == -1) {
	close (sk);
	return -1;
    }

    return sk;
}

static int
server_shm (const char *shm_path)
{
    int fd;

    fd = shm_open (shm_path, O_RDWR | O_EXCL | O_CREAT, 0777);
    if (fd == -1)
	return -1;

    if (ftruncate (fd, DATA_SIZE) == -1) {
	close (fd);
	return -1;
    }

    return fd;
}

static cairo_bool_t
_test_trace (test_trace_t *test,
	     const char *trace,
	     const char *name,
	     struct error_info *error)
{
    const char *shm_path = SHM_PATH_XXX;
    const cairo_boilerplate_target_t *target, *image;
    struct slave *slaves, *s;
    test_runner_t *recorder = NULL;
    pid_t slave;
    char socket_dir[] = "/tmp/cairo-test-trace.XXXXXX";
    char *socket_path;
    int sk, fd;
    int i, num_slaves;
    void *base;
    cairo_bool_t ret = FALSE;

    if (DEBUG)
	printf ("setting up trace '%s'\n", trace);

    /* create a socket to control the test runners */
    if (mkdtemp (socket_dir) == NULL) {
	fprintf (stderr, "Unable to create temporary name for socket\n");
	return FALSE;
    }

    xasprintf (&socket_path, "%s/socket", socket_dir);
    sk = server_socket (socket_path);
    if (sk == -1) {
	fprintf (stderr, "Unable to create socket for server\n");
	goto cleanup_paths;
    }

    /* allocate some shared memory */
    fd = server_shm (shm_path);
    if (fd == -1) {
	fprintf (stderr, "Unable to create shared memory '%s': %s\n",
		 shm_path, strerror (errno));
	goto cleanup_sk;
    }

    image = cairo_boilerplate_get_image_target (CAIRO_CONTENT_COLOR_ALPHA);
    assert (image != NULL);

    s = slaves = xcalloc (2*test->num_targets + 1, sizeof (struct slave));

#if CAIRO_HAS_REAL_PTHREAD
    /* set-up a recording-surface to reconstruct errors */
    slave = spawn_recorder (socket_path, trace, &recorder);
    if (slave < 0) {
        fprintf (stderr, "Unable to create recording surface\n");
        goto cleanup_sk;
    }

    s->pid = slave;
    s->is_recording = TRUE;
    s->target = NULL;
    s->fd = -1;
    s->reference = NULL;
    s++;
#endif

    /* spawn slave processes to run the trace */
    for (i = 0; i < test->num_targets; i++) {
	const cairo_boilerplate_target_t *reference;
	struct slave *master;

	target = test->targets[i];

	if (DEBUG)
	    printf ("setting up target[%d]? '%s' (image? %d, measurable? %d)\n",
		    i, target->name, target == image, target->is_measurable);

	if (target == image || ! target->is_measurable)
	    continue;

	/* find a matching slave to use as a reference for this target */
	if (target->reference_target != NULL) {
	    reference =
		cairo_boilerplate_get_target_by_name (target->reference_target,
						      target->content);
	    assert (reference != NULL);
	} else {
	    reference = image;
	}
	for (master = slaves; master < s; master++) {
	    if (master->target == reference)
		break;
	}

	if (master == s) {
	    /* no match found, spawn a slave to render the reference image */
	    slave = spawn_target (socket_path, shm_path, reference, trace);
	    if (slave < 0)
		continue;

	    s->pid = slave;
	    s->target = reference;
	    s->fd = -1;
	    s->reference = NULL;
	    s++;
	}

	slave = spawn_target (socket_path, shm_path, target, trace);
	if (slave < 0)
	    continue;

	s->pid = slave;
	s->target = target;
	s->fd = -1;
	s->reference = master;
	s++;
    }
    num_slaves = s - slaves;
    if (num_slaves == 1) {
	fprintf (stderr, "No targets to test\n");
	goto cleanup;
    }

    base = mmap (NULL, DATA_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (base == MAP_FAILED) {
	fprintf (stderr, "Unable to mmap shared memory\n");
	goto cleanup;
    }
    ret = test_run (base, sk, name, slaves, num_slaves, error);
    munmap (base, DATA_SIZE);

cleanup:
    close (fd);
    while (s-- > slaves) {
	int status;

	if (s->fd != -1)
	    close (s->fd);

	cairo_surface_destroy (s->image);
	cairo_surface_destroy (s->difference);

	if (s->is_recording) /* in-process */
	    continue;

	kill (s->pid, SIGKILL);
	waitpid (s->pid, &status, 0);
	if (WIFSIGNALED (status) && WTERMSIG(status) != SIGKILL) {
	    fprintf (stderr, "%s crashed\n", s->target->name);
	    if (recorder)
		dump_traces (recorder, trace, s->target->name, "crash");
	}
    }
    free (slaves);
    shm_unlink (shm_path);
cleanup_sk:
    close (sk);

cleanup_paths:
    remove (socket_path);
    remove (socket_dir);

    free (socket_path);
    return ret;
}

static void
test_trace (test_trace_t *test, const char *trace)
{
    char *trace_cpy, *name, *dot;

    trace_cpy = xstrdup (trace);
    name = basename (trace_cpy);
    dot = strchr (name, '.');
    if (dot)
	*dot = '\0';

    if (test->list_only) {
	printf ("%s\n", name);
    } else {
	struct error_info error = {0};
	cairo_bool_t ret;

	printf ("%s: ", name);
	fflush (stdout);

	ret = _test_trace (test, trace, name, &error);
	if (ret) {
	    printf ("PASS\n");
	} else {
	    if (error.context_id) {
		printf ("FAIL (context %lu, lines [%lu, %lu])\n",
			error.context_id,
			error.start_line,
			error.end_line);
	    } else {
		printf ("FAIL\n");
	    }
	}
    }

    free (trace_cpy);
}

static cairo_bool_t
read_excludes (test_trace_t *test, const char *filename)
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
	    int i = test->num_exclude_names;
	    test->exclude_names = xrealloc (test->exclude_names,
					    sizeof (char *) * (i+1));
	    test->exclude_names[i] = strndup (s, t-s);
	    test->num_exclude_names++;
	}
    }
    free (line);

    fclose (file);

    return TRUE;
}

static void
usage (const char *argv0)
{
    fprintf (stderr,
"Usage: %s [-l] [-x exclude-file] [test-names ... | traces ...]\n"
"\n"
"Run the cairo test suite over the given traces (all by default).\n"
"The command-line arguments are interpreted as follows:\n"
"\n"
"  -l	list only; just list selected test case names without executing\n"
"  -x	exclude; specify a file to read a list of traces to exclude\n"
"\n"
"If test names are given they are used as sub-string matches so a command\n"
"such as \"%s firefox\" can be used to run all firefox traces.\n"
"Alternatively, you can specify a list of filenames to execute.\n",
	     argv0, argv0);
}

static void
parse_options (test_trace_t *test, int argc, char *argv[])
{
    int c;

    test->list_only = FALSE;
    test->names = NULL;
    test->num_names = 0;
    test->exclude_names = NULL;
    test->num_exclude_names = 0;

    while (1) {
	c = _cairo_getopt (argc, argv, "lx:");
	if (c == -1)
	    break;

	switch (c) {
	case 'l':
	    test->list_only = TRUE;
	    break;
	case 'x':
	    if (! read_excludes (test, optarg)) {
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
	test->names = &argv[optind];
	test->num_names = argc - optind;
    }
}

static void
test_reset (test_trace_t *test)
{
    /* XXX leaking fonts again via recording-surface? */
#if 0
    cairo_debug_reset_static_data ();
#if HAVE_FCFINI
    FcFini ();
#endif
#endif
}

static void
test_fini (test_trace_t *test)
{
    test_reset (test);

    cairo_boilerplate_free_targets (test->targets);
    free (test->exclude_names);
}

static cairo_bool_t
test_has_filenames (test_trace_t *test)
{
    unsigned int i;

    if (test->num_names == 0)
	return FALSE;

    for (i = 0; i < test->num_names; i++)
	if (access (test->names[i], R_OK) == 0)
	    return TRUE;

    return FALSE;
}

static cairo_bool_t
test_can_run (test_trace_t *test, const char *name)
{
    unsigned int i;
    char *copy, *dot;
    cairo_bool_t ret;

    if (test->num_names == 0 && test->num_exclude_names == 0)
	return TRUE;

    copy = xstrdup (name);
    dot = strrchr (copy, '.');
    if (dot != NULL)
	*dot = '\0';

    if (test->num_names) {
	ret = TRUE;
	for (i = 0; i < test->num_names; i++)
	    if (strstr (copy, test->names[i]))
		goto check_exclude;

	ret = FALSE;
	goto done;
    }

check_exclude:
    if (test->num_exclude_names) {
	ret = FALSE;
	for (i = 0; i < test->num_exclude_names; i++)
	    if (strstr (copy, test->exclude_names[i]))
		goto done;

	ret = TRUE;
	goto done;
    }

done:
    free (copy);

    return ret;
}

static void
warn_no_traces (const char *message, const char *trace_dir)
{
    fprintf (stderr,
"Error: %s '%s'.\n"
"Have you cloned the cairo-traces repository and uncompressed the traces?\n"
"  git clone git://anongit.freedesktop.org/cairo-traces\n"
"  cd cairo-traces && make\n"
"Or set the env.var CAIRO_TRACE_DIR to point to your traces?\n",
	    message, trace_dir);
}

static void
interrupt (int sig)
{
    shm_unlink (SHM_PATH_XXX);

    signal (sig, SIG_DFL);
    raise (sig);
}

int
main (int argc, char *argv[])
{
    test_trace_t test;
    const char *trace_dir = "cairo-traces";
    unsigned int n;

    signal (SIGPIPE, SIG_IGN);
    signal (SIGINT, interrupt);

    parse_options (&test, argc, argv);

    shm_unlink (SHM_PATH_XXX);

    if (getenv ("CAIRO_TRACE_DIR") != NULL)
	trace_dir = getenv ("CAIRO_TRACE_DIR");

    test.targets = cairo_boilerplate_get_targets (&test.num_targets, NULL);

    if (test_has_filenames (&test)) {
	for (n = 0; n < test.num_names; n++) {
	    if (access (test.names[n], R_OK) == 0) {
		test_trace (&test, test.names[n]);
		test_reset (&test);
	    }
	}
    } else {
	DIR *dir;
	struct dirent *de;
	int num_traces = 0;

	dir = opendir (trace_dir);
	if (dir == NULL) {
	    warn_no_traces ("Failed to open directory", trace_dir);
	    test_fini (&test);
	    return 1;
	}

	while ((de = readdir (dir)) != NULL) {
	    char *trace;
	    const char *dot;

	    dot = strrchr (de->d_name, '.');
	    if (dot == NULL)
		continue;
	    if (strcmp (dot, ".trace"))
		continue;

	    num_traces++;
	    if (! test_can_run (&test, de->d_name))
		continue;

	    xasprintf (&trace, "%s/%s", trace_dir, de->d_name);
	    test_trace (&test, trace);
	    test_reset (&test);

	    free (trace);

	}
	closedir (dir);

	if (num_traces == 0) {
	    warn_no_traces ("Found no traces in", trace_dir);
	    test_fini (&test);
	    return 1;
	}
    }

    test_fini (&test);

    return 0;
}

void
cairo_test_logv (const cairo_test_context_t *ctx,
		 const char *fmt, va_list va)
{
#if 0
    vfprintf (stderr, fmt, va);
#endif
}

void
cairo_test_log (const cairo_test_context_t *ctx, const char *fmt, ...)
{
#if 0
    va_list va;

    va_start (va, fmt);
    vfprintf (stderr, fmt, va);
    va_end (va);
#endif
}
