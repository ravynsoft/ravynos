/* cairo-tutorial-xlib.h - a tutorial framework for cairo with xlib
 *
 * Copyright © 2005, Keith Packard
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 */

#include    <stdio.h>
#include    <stdlib.h>
#include    <ctype.h>
#include    <strings.h>
#include    <X11/Xos.h>
#include    <X11/Xlib.h>
#include    <X11/Xutil.h>
#include    <X11/Xatom.h>
#include    <cairo.h>
#include    <cairo-xlib.h>

#ifndef WIDTH
#define WIDTH 400
#endif

#ifndef HEIGHT
#define HEIGHT 400
#endif

#ifndef DEFAULT_VISUAL
#define DEFAULT_VISUAL 0
#endif

static void
Usage (char *program)
{
    fprintf (stderr, "Usage: %s\n", program);
    fprintf (stderr, "\t-display <display-name>\n");
    fprintf (stderr, "\t-geometry <geometry>\n");
    exit (1);
}

char	    *dpy_name;
VisualID    vid = DEFAULT_VISUAL;
Colormap    colormap;
Visual	    *visual;
int	    depth;
unsigned int width = WIDTH, height = HEIGHT;
Window	    win;
Pixmap	    pix;
GC	    gc;

static void
draw (cairo_t *cr, int width, int height);

static void
draw_to_pixmap (Display *dpy, Pixmap pix)
{
    cairo_surface_t *surface;
    cairo_t *cr;

    surface = cairo_xlib_surface_create (dpy, pix, visual,
					 width, height);
    cr = cairo_create (surface);

    draw (cr, width, height);

    cairo_destroy (cr);
    cairo_surface_destroy (surface);
}

static void
handle_configure (Display *dpy, XConfigureEvent *cev)
{
    width = cev->width;
    height = cev->height;

    XFreePixmap(dpy, pix);
    pix = XCreatePixmap(dpy, win, width, height, depth);
    XFillRectangle(dpy, pix, gc, 0, 0, width, height);
    draw_to_pixmap (dpy, pix);
}

static void
handle_expose (Display *dpy, XExposeEvent *eev)
{
    XCopyArea (dpy, pix, win, gc,
	       eev->x, eev->y,
	       eev->width, eev->height,
	       eev->x, eev->y);
}

int
main (argc, argv)
    int	    argc;
    char    **argv;
{
    Display *dpy;
    Window  root = 0;
    char    **init_argv = argv;
    XSetWindowAttributes    attr;
    int	    scr;
    int	    x = 0, y = 0;
    int	    geometryMask;
    int	    border_width = 1;
    XSizeHints	sizeHints;
    XWMHints	wmHints;
    XClassHint	classHints;
    XEvent	ev;
    XEvent	eev;
    int		HasExpose = 0;
    int		sync = 0;
    XTextProperty   wm_name, icon_name;
    Atom	wm_delete_window;
    unsigned long   gc_mask;
    XGCValues	    gcv;
    char	quit_string[10];
    unsigned long   window_mask;
    int		has_colormap = 0;

    wm_name.value = (unsigned char *) argv[0];
    wm_name.encoding = XA_STRING;
    wm_name.format = 8;
    wm_name.nitems = strlen (argv[0]) + 1;
    icon_name = wm_name;
    gc_mask = 0;
    while (*++argv) {
	if (!strcmp (*argv, "-display"))
	    dpy_name = *++argv;
	else if (!strcmp (*argv, "-visual"))
	    vid = strtol(*++argv, NULL, 0);
	else if (!strcmp (*argv, "-geometry"))
	    geometryMask = XParseGeometry (*++argv, &x, &y, &width, &height);
	else if (!strcmp (*argv, "-sync"))
	    sync = 1;
	else if (!strcmp (*argv, "-bw"))
	    border_width = strtol(*++argv, NULL, 0);
	else if (!strcmp (*argv, "-root"))
	    root = strtol (*++argv, NULL, 0);
	else
	    Usage (*init_argv);
    }
    sizeHints.flags = 0;
    wmHints.flags = InputHint;
    wmHints.input = True;
    classHints.res_name = init_argv[0];
    classHints.res_class = init_argv[0];
    dpy = XOpenDisplay (dpy_name);
    if (!dpy) {
	fprintf (stderr, "Error: failed to open display: %s\n",
		 XDisplayName (dpy_name));
	exit (1);
    }
    if (sync)
	XSynchronize (dpy, sync);
    scr = DefaultScreen (dpy);
    if (!root)
	root = RootWindow (dpy, scr);
    window_mask = CWBackPixel|CWBorderPixel|CWEventMask;
    if (!has_colormap)
	colormap = DefaultColormap (dpy, scr);
    else
    {
	window_mask |= CWColormap;
	attr.colormap = colormap;
    }
    visual = DefaultVisual (dpy, scr);
    depth = DefaultDepth (dpy, scr);
    if (vid)
    {
	XVisualInfo vi, *vi_ret;
	int	    n;

	vi.visualid = vid;
	vi.screen = scr;
	vi_ret = XGetVisualInfo (dpy, VisualIDMask|VisualScreenMask,
				 &vi, &n);
	if (vi_ret)
	{
	    visual = vi_ret->visual;
	    if (!has_colormap)
	    {
		colormap = XCreateColormap (dpy, root, visual, AllocNone);
		window_mask |= CWColormap;
		attr.colormap = colormap;
	    }
	    depth = vi_ret->depth;
	}
    }
    attr.background_pixel = WhitePixel (dpy, scr);
    attr.border_pixel = 0;
    attr.event_mask = ExposureMask|KeyPressMask|KeyReleaseMask|StructureNotifyMask;
    wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    win = XCreateWindow (dpy, root, x, y, width, height, border_width,
			 depth, InputOutput,
			 visual,
 			 window_mask,
			 &attr);
    pix = XCreatePixmap (dpy, win, width, height, depth);
    gcv.foreground = WhitePixel (dpy, scr);
    gc = XCreateGC (dpy, pix, GCForeground, &gcv);
    XFillRectangle(dpy, pix, gc, 0, 0, width, height);
    draw_to_pixmap (dpy, pix);
    XSetWMProperties (dpy, win,
		      &wm_name, &icon_name,
		      init_argv, argc,
 		      &sizeHints, &wmHints, 0);
    XSetWMProtocols (dpy, win, &wm_delete_window, 1);
    XMapWindow (dpy, win);
    for (;;) {
	XNextEvent (dpy, &ev);
	if (HasExpose && ev.type != Expose) {
	    HasExpose = 0;
	    handle_expose (dpy, &eev.xexpose);
	}
	switch (ev.type) {
	case ConfigureNotify:
	    handle_configure (dpy, &ev.xconfigure);
	    break;
	case Expose:
	    if (QLength(dpy)) {
		eev = ev;
		HasExpose = 1;
	    } else if (ev.xexpose.count == 0) {
		handle_expose (dpy, &ev.xexpose);
	    }
	    break;
	case KeyPress:
	    if (XLookupString ((XKeyEvent *) &ev, quit_string, sizeof (quit_string), 0, 0) == 1) {
		switch (quit_string[0]) {
		case 'q':
		    exit (0);
		case 'c':
		    XClearArea (dpy, ev.xkey.window, 0, 0, 0, 0, True);
		    break;
		}
	    }
	    break;
	case ClientMessage:
	    exit (0);
	}
    }
}
