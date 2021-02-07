/* XGServerWindows - methods for window/screen handling

   Copyright (C) 1999-2020 Free Software Foundation, Inc.

   Written by:  Adam Fedor <fedor@gnu.org>
   Date: Nov 1999

   This file is part of GNUstep

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the
   Free Software Foundation, 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "config.h"
#include <math.h>
#include <Foundation/NSString.h>
#include <Foundation/NSArray.h>
#include <Foundation/NSDebug.h>
#include <Foundation/NSValue.h>
#include <Foundation/NSProcessInfo.h>
#include <Foundation/NSUserDefaults.h>
#include <Foundation/NSAutoreleasePool.h>
#include <Foundation/NSDebug.h>
#include <Foundation/NSException.h>
#include <Foundation/NSThread.h>
#include <AppKit/DPSOperators.h>
#include <AppKit/NSApplication.h>
#include <AppKit/NSCursor.h>
#include <AppKit/NSGraphics.h>
#include <AppKit/NSWindow.h>
#include <AppKit/NSImage.h>
#include <AppKit/NSBitmapImageRep.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_WRASTER_H
#include "wraster.h"
#else
#include "x11/wraster.h"
#endif

// For X_HAVE_UTF8_STRING
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#if HAVE_XCURSOR
#include <X11/Xcursor/Xcursor.h>
#endif
#ifdef HAVE_XSHAPE
#include <X11/extensions/shape.h>
#endif
#if HAVE_XFIXES
#include <X11/extensions/Xfixes.h>
#endif
#ifdef HAVE_XRANDR
#include <X11/extensions/Xrandr.h>
#endif

#include "x11/XGDragView.h"
#include "x11/XGInputServer.h"

#define	ROOT generic.appRootWindow


static BOOL handlesWindowDecorations = YES;
static int _wmAppIcon = -1;

#define WINDOW_WITH_TAG(windowNumber) (gswindow_device_t *)NSMapGet(windowtags, (void *)(uintptr_t)windowNumber)

/* Current mouse grab window */
static gswindow_device_t *grab_window = NULL;

/* Keep track of windows */
static NSMapTable *windowmaps = NULL;
static NSMapTable *windowtags = NULL;

/* Track used window numbers */
static int		last_win_num = 0;

@interface NSCursor (BackendPrivate)
- (void *)_cid;
@end

@interface NSBitmapImageRep (GSPrivate)
- (NSBitmapImageRep *) _convertToFormatBitsPerSample: (NSInteger)bps
                                     samplesPerPixel: (NSInteger)spp
                                            hasAlpha: (BOOL)alpha
                                            isPlanar: (BOOL)isPlanar
                                      colorSpaceName: (NSString*)colorSpaceName
                                        bitmapFormat: (NSBitmapFormat)bitmapFormat
                                         bytesPerRow: (NSInteger)rowBytes
                                        bitsPerPixel: (NSInteger)pixelBits;
@end

static NSBitmapImageRep *getStandardBitmap(NSImage *image)
{
  NSBitmapImageRep *rep;

  if (image == nil)
    {
      return nil;
    }

/*
  We should rather convert the image to a bitmap representation here via
  the following code, but this is currently not supported by the libart backend

{
  NSSize size = [image size];

  [image lockFocus];
  rep = [[NSBitmapImageRep alloc] initWithFocusedViewRect:
            NSMakeRect(0, 0, size.width, size.height)];
  AUTORELEASE(rep);
  [image unlockFocus];
}
*/

  rep = (NSBitmapImageRep *)[image bestRepresentationForDevice: nil];
  if (!rep || ![rep respondsToSelector: @selector(samplesPerPixel)])
    {
      return nil;
    }
  else
    {
      // Convert into something usable by the backend
      return [rep _convertToFormatBitsPerSample: 8
                                samplesPerPixel: [rep hasAlpha] ? 4 : 3
                                       hasAlpha: [rep hasAlpha]
                                       isPlanar: NO
                                 colorSpaceName: NSCalibratedRGBColorSpace
                                   bitmapFormat: [rep bitmapFormat]
                                    bytesPerRow: 0
                                   bitsPerPixel: 0];
    }
}


void __objc_xgcontextwindow_linking (void)
{
}

/*
 * The next two functions derived from WindowMaker by Alfredo K. Kojima
 */
static unsigned char*
PropGetCheckProperty(Display *dpy, Window window, Atom hint, Atom type,
		     int format, int count, int *retCount)
{
  Atom type_ret;
  int fmt_ret;
  unsigned long nitems_ret;
  unsigned long bytes_after_ret;
  unsigned char *data;
  int tmp;

  if (count <= 0)
    {
      tmp = 0xffffff;
    }
  else
    {
      tmp = count;
    }

  if (XGetWindowProperty(dpy, window, hint, 0, tmp, False, type,
			 &type_ret, &fmt_ret, &nitems_ret, &bytes_after_ret,
			 (unsigned char **)&data)!=Success || !data)
    {
      return NULL;
    }

  if ((type != AnyPropertyType && type != type_ret)
    || (count > 0 && nitems_ret != (unsigned long)count)
    || (bytes_after_ret != 0)
    || (format != 0 && format != fmt_ret))
    {
      NSLog(@"XGetWindowProperty type %lu type_ret %lu count %d count_ret %lu format %d format_ret %d bytes_after_ret %lu",
            type, type_ret, count, nitems_ret, format, fmt_ret, bytes_after_ret);
      XFree(data);
      return NULL;
    }

  if (retCount)
    {
      *retCount = nitems_ret;
    }

  return data;
}

static void
setNormalHints(Display *d, gswindow_device_t *w)
{
  if (w->siz_hints.flags & (USPosition | PPosition))
    NSDebugLLog(@"XGTrace", @"Hint posn %lu: %d, %d",
      w->number, w->siz_hints.x, w->siz_hints.y);
  if (w->siz_hints.flags & (USSize | PSize))
    NSDebugLLog(@"XGTrace", @"Hint size %lu: %d, %d",
      w->number, w->siz_hints.width, w->siz_hints.height);
  if (w->siz_hints.flags & PMinSize)
    NSDebugLLog(@"XGTrace", @"Hint mins %lu: %d, %d",
      w->number, w->siz_hints.min_width, w->siz_hints.min_height);
  if (w->siz_hints.flags & PMaxSize)
    NSDebugLLog(@"XGTrace", @"Hint maxs %lu: %d, %d",
      w->number, w->siz_hints.max_width, w->siz_hints.max_height);
  if (w->siz_hints.flags & PResizeInc)
    NSDebugLLog(@"XGTrace", @"Hint incr %lu: %d, %d",
      w->number, w->siz_hints.width_inc, w->siz_hints.height_inc);
  if (handlesWindowDecorations
    && !(w->win_attrs.window_style & NSResizableWindowMask))
    {
      /* Some silly window managers (*cough* metacity *cough*) ignore
	 our "non-resizable" hints unless we set the min and max
	 sizes equal to the current size, hence the ugly code here.  */
      CARD32 oldFlags;
      int old_w0, old_h0, old_w1, old_h1;

      old_w0 = w->siz_hints.min_width;
      old_h0 = w->siz_hints.max_width;
      old_w1 = w->siz_hints.min_height;
      old_h1 = w->siz_hints.max_height;
      oldFlags = w->siz_hints.flags;

      w->siz_hints.flags |= PMinSize | PMaxSize;
      w->siz_hints.min_width = w->siz_hints.max_width = w->xframe.size.width;
      w->siz_hints.min_height = w->siz_hints.max_height = w->xframe.size.height;
      XSetWMNormalHints(d, w->ident, &w->siz_hints);

      w->siz_hints.min_width = old_w0;
      w->siz_hints.max_width = old_h0;
      w->siz_hints.min_height = old_w1;
      w->siz_hints.max_height = old_h1;
      w->siz_hints.flags = oldFlags;
    }
  else
    XSetWMNormalHints(d, w->ident, &w->siz_hints);
}

/*
 * Setting Motif Hints for Window Managers (Nicola Pero, July 2000)
 */

/*
 * Motif window hints to communicate to a window manager
 * that we want a window to have a titlebar/resize button/etc.
 */

/* Motif window hints struct */
typedef struct {
  unsigned long flags;
  unsigned long functions;
  unsigned long decorations;
  unsigned long input_mode;
  unsigned long status;
} MwmHints;

/* Number of entries in the struct */
#define PROP_MWM_HINTS_ELEMENTS 5

/* Now for each field in the struct, meaningful stuff to put in: */

/* flags */
#define MWM_HINTS_FUNCTIONS   (1L << 0)
#define MWM_HINTS_DECORATIONS (1L << 1)
#define MWM_HINTS_INPUT_MODE  (1L << 2)
#define MWM_HINTS_STATUS      (1L << 3)

/* functions */
#define MWM_FUNC_ALL          (1L << 0)
#define MWM_FUNC_RESIZE       (1L << 1)
#define MWM_FUNC_MOVE         (1L << 2)
#define MWM_FUNC_MINIMIZE     (1L << 3)
#define MWM_FUNC_MAXIMIZE     (1L << 4)
#define MWM_FUNC_CLOSE        (1L << 5)

/* decorations */
#define MWM_DECOR_ALL         (1L << 0)
#define MWM_DECOR_BORDER      (1L << 1)
#define MWM_DECOR_RESIZEH     (1L << 2)
#define MWM_DECOR_TITLE       (1L << 3)
#define MWM_DECOR_MENU        (1L << 4)
#define MWM_DECOR_MINIMIZE    (1L << 5)
#define MWM_DECOR_MAXIMIZE    (1L << 6)



/* Now the code */

/* Set the style `styleMask' for the XWindow `window' using motif
 * window hints.  This makes an X call, please make sure you do it
 * only once.
 */
static void setWindowHintsForStyle (Display *dpy, Window window,
                                    unsigned int styleMask, Atom mwhints_atom)
{
  MwmHints *hints;
  BOOL needToFreeHints = YES;
  Atom type_ret;
  int format_ret, success;
  unsigned long nitems_ret;
  unsigned long bytes_after_ret;

  /* Get the already-set window hints */
  success = XGetWindowProperty (dpy, window, mwhints_atom, 0,
		      sizeof (MwmHints) / sizeof (unsigned long),
		      False, AnyPropertyType, &type_ret, &format_ret,
		      &nitems_ret, &bytes_after_ret,
		      (unsigned char **)&hints);

  /* If no window hints were set, create new hints to 0 */
  if (success != Success || type_ret == None)
    {
      needToFreeHints = NO;
      hints = alloca (sizeof (MwmHints));
      memset (hints, 0, sizeof (MwmHints));
    }

  /* Remove the hints we want to change */
  hints->flags &= ~MWM_HINTS_DECORATIONS;
  hints->flags &= ~MWM_HINTS_FUNCTIONS;
  hints->decorations = 0;
  hints->functions = 0;

  /* Now add to the hints from the styleMask */
  if (styleMask == NSBorderlessWindowMask
      || !handlesWindowDecorations)
    {
      hints->flags |= MWM_HINTS_DECORATIONS;
      hints->flags |= MWM_HINTS_FUNCTIONS;
      hints->decorations = 0;
      hints->functions = 0;
    }
  else
    {
      /* These need to be on all windows except mini and icon windows,
	 where they are specifically set to 0 (see below) */
      hints->flags |= MWM_HINTS_DECORATIONS;
      hints->decorations |= (MWM_DECOR_TITLE | MWM_DECOR_BORDER);
      if (styleMask & NSTitledWindowMask)
	{
	  // Without this, iceWM does not let you move the window!
	  // [idem below]
	  hints->flags |= MWM_HINTS_FUNCTIONS;
	  hints->functions |= MWM_FUNC_MOVE;
	}
      if (styleMask & NSClosableWindowMask)
	{
	  hints->flags |= MWM_HINTS_FUNCTIONS;
	  hints->functions |= MWM_FUNC_CLOSE;
	  hints->functions |= MWM_FUNC_MOVE;
	}
      if (styleMask & NSMiniaturizableWindowMask)
	{
	  hints->flags |= MWM_HINTS_DECORATIONS;
	  hints->flags |= MWM_HINTS_FUNCTIONS;
	  hints->decorations |= MWM_DECOR_MINIMIZE;
	  hints->functions |= MWM_FUNC_MINIMIZE;
	  hints->functions |= MWM_FUNC_MOVE;
	}
      if (styleMask & NSResizableWindowMask)
	{
	  hints->flags |= MWM_HINTS_DECORATIONS;
	  hints->flags |= MWM_HINTS_FUNCTIONS;
	  hints->decorations |= MWM_DECOR_RESIZEH;
	  hints->decorations |= MWM_DECOR_MAXIMIZE;
	  hints->functions |= MWM_FUNC_RESIZE;
	  hints->functions |= MWM_FUNC_MAXIMIZE;
	  hints->functions |= MWM_FUNC_MOVE;
        }
      if (styleMask & NSIconWindowMask)
	{
	  // FIXME
	  hints->flags |= MWM_HINTS_DECORATIONS;
	  hints->flags |= MWM_HINTS_FUNCTIONS;
	  hints->decorations = 0;
	  hints->functions = 0;
	}
      if (styleMask & NSMiniWindowMask)
	{
	  // FIXME
	  hints->flags |= MWM_HINTS_DECORATIONS;
	  hints->flags |= MWM_HINTS_FUNCTIONS;
	  hints->decorations = 0;
	  hints->functions = 0;
	}
    }

  /* Set the hints */
  XChangeProperty (dpy, window, mwhints_atom, mwhints_atom, 32,
		   PropModeReplace, (unsigned char *)hints,
		   sizeof (MwmHints) / sizeof (unsigned long));

  /* Free the hints if allocated by the X server for us */
  if (needToFreeHints == YES)
    XFree (hints);
}

/*
 * End of motif hints for window manager code
 */

BOOL AtomPresentAndPointsToItself(Display *dpy, Atom atom, Atom type)
{
  int count;
  BOOL result = NO;
  Window root = DefaultRootWindow(dpy);
  Window *win = (Window*)PropGetCheckProperty(dpy, root, atom,
                                              type, 32, -1, &count);
  if (win != 0)
    {
      Window *win1 = (Window*)PropGetCheckProperty(dpy, *win, atom,
                                                   type, 32, -1, &count);
      // If the two are not identical, the flag on the root window, was
      // a left over from an old window manager.
      if (win1 && *win1 == *win)
        {
          result= YES;
        }
      if (win1)
        {
          XFree(win1);
        }
      XFree(win);
    }

  return result;
}

@interface XGServer (WindowOps)
- (gswindow_device_t *) _rootWindow;
- (void) styleoffsets: (float *) l : (float *) r : (float *) t : (float *) b
                     : (unsigned int) style : (Window) win;
- (void) _setSupportedWMProtocols: (gswindow_device_t *) window;
@end

@implementation XGServer (WindowOps)

- (BOOL) handlesWindowDecorations
{
  return handlesWindowDecorations;
}


/*
 * Where a window has been reparented by the wm, we use this method to
 * locate the window given knowledge of its border window.
 */
+ (gswindow_device_t *) _windowForXParent: (Window)xWindow
{
  NSMapEnumerator	enumerator;
  void		*key;
  gswindow_device_t	*d;

  enumerator = NSEnumerateMapTable(windowmaps);
  while (NSNextMapEnumeratorPair(&enumerator, &key, (void**)&d) == YES)
    {
      if (d->root != d->parent && d->parent == xWindow)
	{
	  return d;
	}
    }
  return 0;
}

+ (gswindow_device_t *) _windowForXWindow: (Window)xWindow
{
  return NSMapGet(windowmaps, (void *)xWindow);
}

+ (gswindow_device_t *) _windowWithTag: (int)windowNumber
{
  return WINDOW_WITH_TAG(windowNumber);
}

/*
 * Convert a window frame in OpenStep absolute screen coordinates to
 * a frame in X absolute screen coordinates by flipping an applying
 * offsets to allow for the X window decorations.
 * The result is the rectangle of the window we can actually draw
 * to (in the X coordinate system).
 */
- (NSRect) _OSFrameToXFrame: (NSRect)o for: (void*)window
{
  gswindow_device_t	*win = (gswindow_device_t*)window;
  unsigned int		style = win->win_attrs.window_style;
  NSRect	x;
  float	t, b, l, r;

  [self styleoffsets: &l : &r : &t : &b : style : win->ident];

  x.size.width = o.size.width - l - r;
  x.size.height = o.size.height - t - b;
  x.origin.x = o.origin.x + l;
  x.origin.y = o.origin.y + o.size.height - t;
  x.origin.y = xScreenSize.height - x.origin.y;
  NSDebugLLog(@"Frame", @"O2X %lu, %x, %@, %@", win->number, style,
    NSStringFromRect(o), NSStringFromRect(x));
  return x;
}

/*
 * Convert a window frame in OpenStep absolute screen coordinates to
 * a frame suitable for setting X hints for a window manager.
 * NB. Hints use the coordinates of the parent decoration window,
 * but the size of the actual window.
 */
- (NSRect) _OSFrameToXHints: (NSRect)o for: (void*)window
{
  gswindow_device_t	*win = (gswindow_device_t*)window;
  unsigned int		style = win->win_attrs.window_style;
  NSRect	x;
  float	t, b, l, r;

  [self styleoffsets: &l : &r : &t : &b : style : win->ident];

  x.size.width = o.size.width - l - r;
  x.size.height = o.size.height - t - b;
  x.origin.x = o.origin.x;
  x.origin.y = o.origin.y + o.size.height;
  x.origin.y = xScreenSize.height - x.origin.y;
  NSDebugLLog(@"Frame", @"O2H %lu, %x, %@, %@", win->number, style,
    NSStringFromRect(o), NSStringFromRect(x));
  return x;
}

/*
 * Convert a rectangle in X  coordinates relative to the X-window
 * to a rectangle in OpenStep coordinates (base coordinates of the NSWindow).
 */
- (NSRect) _XWinRectToOSWinRect: (NSRect)x for: (void*)window
{
  gswindow_device_t	*win = (gswindow_device_t*)window;
  unsigned int		style = win->win_attrs.window_style;
  NSRect	o;
  float	t, b, l, r;

  [self styleoffsets: &l : &r : &t : &b : style : win->ident];
  o.size.width = x.size.width;
  o.size.height = x.size.height;
  o.origin.x = x.origin.x + l;
  o.origin.y = NSHeight(win->xframe) - (x.origin.y + x.size.height);
  o.origin.y = o.origin.y + b;
  NSDebugLLog(@"Frame", @"XW2OW %@ %@",
    NSStringFromRect(x), NSStringFromRect(o));
  return o;
}

/*
 * Convert a window frame in X absolute screen coordinates to a frame
 * in OpenStep absolute screen coordinates by flipping an applying
 * offsets to allow for the X window decorations.
 */
- (NSRect) _XFrameToOSFrame: (NSRect)x for: (void*)window
{
  gswindow_device_t	*win = (gswindow_device_t*)window;
  unsigned int		style = win->win_attrs.window_style;
  NSRect	o;
  float	t, b, l, r;

  [self styleoffsets: &l : &r : &t : &b : style : win->ident];
  o = x;
  o.origin.y = xScreenSize.height - x.origin.y;
  o.origin.y = o.origin.y - x.size.height - b;
  o.origin.x -= l;
  o.size.width += l + r;
  o.size.height += t + b;

  NSDebugLLog(@"Frame", @"X2O %lu, %x, %@, %@", win->number, style,
    NSStringFromRect(x), NSStringFromRect(o));
  return o;
}

/*
 * Convert a window frame in X absolute screen coordinates to
 * a frame suitable for setting X hints for a window manager.
 */
- (NSRect) _XFrameToXHints: (NSRect)o for: (void*)window
{
  gswindow_device_t	*win = (gswindow_device_t*)window;
  unsigned int		style = win->win_attrs.window_style;
  NSRect	x;
  float	t, b, l, r;

  [self styleoffsets: &l : &r : &t : &b : style : win->ident];

  /* WARNING: if we adjust the frame size we get problems,
   * but we do seem to need to adjust the position to allow for
   * decorations.
   */
  x.size.width = o.size.width;
  x.size.height = o.size.height;
  x.origin.x = o.origin.x - l;
  x.origin.y = o.origin.y - t;
  NSDebugLLog(@"Frame", @"X2H %lu, %x, %@, %@", win->number, style,
    NSStringFromRect(o), NSStringFromRect(x));
  return x;
}

- (void)_sendRoot: (Window)root
             type: (Atom)type
           window: (Window)window
            data0: (long)data0
            data1: (long)data1
            data2: (long)data2
            data3: (long)data3
{
  XEvent event;

  memset(&event, 0, sizeof(event));
  event.xclient.type = ClientMessage;
  event.xclient.message_type = type;
  event.xclient.format = 32;
  event.xclient.display = dpy;
  event.xclient.window = window;
  event.xclient.data.l[0] = data0;
  event.xclient.data.l[1] = data1;
  event.xclient.data.l[2] = data2;
  event.xclient.data.l[3] = data3;
  XSendEvent(dpy, root, False,
             (SubstructureNotifyMask|SubstructureRedirectMask), &event);
  XFlush(dpy);
}

/*
 * Check if the window manager supports a feature.
 */
- (BOOL) _checkWMSupports: (Atom)feature
{
  Window root;
  int	count;
  Atom *data;

  if ((generic.wm & XGWM_EWMH) == 0)
    {
      return NO;
    }

  root = DefaultRootWindow(dpy);
  data = (Atom*)PropGetCheckProperty(dpy, root, generic._NET_SUPPORTED_ATOM, XA_ATOM, 32, -1, &count);
  if (data != 0)
    {
      int i = 0;

      while (i < count && data[i] != feature)
        {
          i++;
        }
      XFree(data);

      if (i < count)
        {
            return YES;
        }
    }
  return NO;
}

static void
select_input(Display *display, Window w, BOOL ignoreMouse)
{
  long event_mask = ExposureMask
    | KeyPressMask
    | KeyReleaseMask
    | StructureNotifyMask
    | FocusChangeMask
    /* enable property notifications to detect window (de)miniaturization */
    | PropertyChangeMask
    //    | ColormapChangeMask
    | KeymapStateMask
    | VisibilityChangeMask;

  if (!ignoreMouse)
    {
      event_mask |= ButtonPressMask
        | ButtonReleaseMask
        | ButtonMotionMask
        | PointerMotionMask
        | EnterWindowMask
        | LeaveWindowMask;
    }

  XSelectInput(display, w, event_mask);
}

Bool
_get_next_prop_new_event(Display *display, XEvent *event, char *arg)
{
  XID *data = (XID*)arg;

  if (event->type == PropertyNotify &&
      event->xproperty.window == data[0] &&
      event->xproperty.atom == data[1] &&
      event->xproperty.state == PropertyNewValue)
    {
      return True;
    }
  else
    {
      return False;
    }
}

- (BOOL) _tryRequestFrameExtents: (gswindow_device_t *)window
{
  XEvent xEvent;
  XID event_data[2];
  NSDate *limit;

  if (![self _checkWMSupports: generic._NET_REQUEST_FRAME_EXTENTS_ATOM])
    {
      return NO;
    }

  [self _sendRoot: window->root
        type: generic._NET_REQUEST_FRAME_EXTENTS_ATOM
        window: window->ident
        data0: 0
        data1: 0
        data2: 0
        data3: 0];

  event_data[0] = window->ident;
  event_data[1] = generic._NET_FRAME_EXTENTS_ATOM;

  limit = [NSDate dateWithTimeIntervalSinceNow: 1.0];
  while ([limit timeIntervalSinceNow] > 0.0)
    {
      if (XCheckTypedWindowEvent(dpy, window->ident, DestroyNotify, &xEvent))
        {
          return NO;
        }
      else if (XCheckIfEvent(dpy, &xEvent, _get_next_prop_new_event,
                             (char*)(&event_data)))
        {
          return YES;
        }
      else
        {
          CREATE_AUTORELEASE_POOL(pool);

          [NSThread sleepUntilDate:
                        [NSDate dateWithTimeIntervalSinceNow: 0.01]];
          IF_NO_GC([pool release]);
        }
    }

  return NO;
}

- (unsigned long*) _getExtents: (Window)win
{
  int count;
  unsigned long *extents;

  /* If our window manager supports _NET_FRAME_EXTENTS we trust that as
   * definitive information.
   */
  extents = (unsigned long *)PropGetCheckProperty(dpy, win, generic._NET_FRAME_EXTENTS_ATOM,
                                                  XA_CARDINAL, 32, 4, &count);
  if (extents != 0)
    {
      NSDebugLLog(@"Offset", @"Offsets retrieved from _NET_FRAME_EXTENTS");
    }
  if (extents == 0)
    {
      /* If our window manager supports _KDE_NET_WM_FRAME_STRUT we assume
       * its as reliable as _NET_FRAME_EXTENTS
       */
      extents = (unsigned long *)PropGetCheckProperty(dpy, win, generic._KDE_NET_WM_FRAME_STRUT_ATOM,
                                                      XA_CARDINAL, 32, 4, &count);
      if (extents!= 0)
        {
          NSDebugLLog(@"Offset", @"Offsets retrieved from _KDE_NET_WM_FRAME_STRUT");
        }
    }
  return extents;
}

- (BOOL) _checkStyle: (unsigned)style
{
  gswindow_device_t	*window;
  gswindow_device_t	*root;
  NSRect		frame;
  XGCValues		values;
  unsigned long		valuemask;
  XClassHint		classhint;
  RContext              *context;
  XEvent		xEvent;
  unsigned long		*extents;
  Offsets		*o = generic.offsets + (style & 15);
  int			repp = 0;
  int			repx = 0;
  int			repy = 0;
  BOOL                  onScreen;
  BOOL                  reparented = NO;

  NSDebugLLog(@"Offset", @"Checking offsets for style %d\n", style);

  onScreen = [[NSUserDefaults standardUserDefaults] boolForKey:
    @"GSBackChecksOffsetsOnScreen"];

  root = [self _rootWindow];
  context = [self screenRContext];

  window = NSAllocateCollectable(sizeof(gswindow_device_t), NSScannedOption);
  memset(window, '\0', sizeof(gswindow_device_t));
  window->display = dpy;
  window->screen_id = defScreen;

  window->win_attrs.flags |= GSWindowStyleAttr;
  window->win_attrs.window_style = style;

  if (onScreen == YES)
    {
      frame = NSMakeRect(100,100,100,100);
    }
  else
    {
      frame = NSMakeRect(-200,100,100,100);
    }

  window->xframe = frame;
  window->type = NSBackingStoreNonretained;
  window->root = root->ident;
  window->parent = root->ident;
  window->depth = context->depth;
  window->xwn_attrs.border_pixel = context->black;
  window->xwn_attrs.background_pixel = context->white;
  window->xwn_attrs.colormap = context->cmap;
  window->xwn_attrs.save_under = False;
  window->xwn_attrs.override_redirect = False;

  window->ident = XCreateWindow(dpy, window->root,
				NSMinX(frame), NSMinY(frame),
				NSWidth(frame), NSHeight(frame),
				0,
				context->depth,
				CopyFromParent,
				context->visual,
				(CWColormap | CWBorderPixel | CWOverrideRedirect),
				&window->xwn_attrs);

  /*
   * Mark this as a GNUstep app with the current application name.
   */
  classhint.res_name = "CheckWindowStyle";
  classhint.res_class = "GNUstep";
  XSetClassHint(dpy, window->ident, &classhint);

  window->map_state = IsUnmapped;
  window->visibility = 2;
  window->wm_state = WithdrawnState;

  // Create an X GC for the content view set it's colors
  values.foreground = window->xwn_attrs.background_pixel;
  values.background = window->xwn_attrs.background_pixel;
  values.function = GXcopy;
  valuemask = (GCForeground | GCBackground | GCFunction);
  window->gc = XCreateGC(dpy, window->ident, valuemask, &values);

  /* Set the X event mask */
  select_input(dpy, window->ident, YES);

  /*
   * Initial attributes for any GNUstep window tell Window Maker not to
   * create an app icon for us.
   */
  window->win_attrs.flags |= GSExtraFlagsAttr;
  window->win_attrs.extra_flags |= GSNoApplicationIconFlag;

  /*
   * Prepare size/position hints, but don't set them now - ordering
   * the window in should automatically do it.
   */
  window->siz_hints.x = NSMinX(frame);
  window->siz_hints.y = NSMinY(frame);
  window->siz_hints.width = NSWidth(frame);
  window->siz_hints.height = NSHeight(frame);
  window->siz_hints.flags = USPosition|PPosition|USSize|PSize;

  // Always send GNUstepWMAttributes
/* Warning ... X-bug .. when we specify 32bit data X actually expects data
 * of type 'long' or 'unsigned long' even on machines where those types
 * hold 64bit values.
 */
  XChangeProperty(dpy, window->ident, generic._GNUSTEP_WM_ATTR_ATOM,
		      generic._GNUSTEP_WM_ATTR_ATOM, 32, PropModeReplace,
		      (unsigned char *)&window->win_attrs,
		      sizeof(GNUstepWMAttributes)/sizeof(CARD32));

  // send to the WM window style hints
  if ((generic.wm & XGWM_WINDOWMAKER) == 0)
    {
      setWindowHintsForStyle(dpy, window->ident, style, generic._MOTIF_WM_HINTS_ATOM);
    }

  // Use the globally active input mode
  window->gen_hints.flags = InputHint;
  window->gen_hints.input = False;

  /*
   * Prepare the protocols supported by the window.
   * These protocols should be set on the window when it is ordered in.
   */
  [self _setSupportedWMProtocols: window];

  window->exposedRects = [NSMutableArray new];
  window->region = XCreateRegion();
  window->buffer = 0;
  window->alpha_buffer = 0;
  window->ic = 0;

  // make sure that new window has the correct cursor
  [self _initializeCursorForXWindow: window->ident];

  /*
   * FIXME - should this be protected by a lock for thread safety?
   * generate a unique tag for this new window.
   */
  do
    {
      last_win_num++;
    }
  while (last_win_num == 0 || WINDOW_WITH_TAG(last_win_num) != 0);
  window->number = last_win_num;

  // Insert window into the mapping
  NSMapInsert(windowmaps, (void*)(uintptr_t)window->ident, window);
  NSMapInsert(windowtags, (void*)(uintptr_t)window->number, window);
  [self _setWindowOwnedByServer: window->number];

  if (![self _tryRequestFrameExtents: window])
    {
      // Only display the window, if the window manager does not support
      // _NET_REQUEST_FRAME_EXTENTS
      [self orderwindow: NSWindowAbove : 0 : window->number];

      XSync(dpy, False);
      while (XPending(dpy) > 0 || window->visibility > 1)
        {
          if (XPending(dpy) == 0)
            {
              NSDate	*until;

              /* In theory, after executing XSync() all events resulting from
               * our window creation and ordering front should be available in
               * the X event queue.
               * However, it's possible that a window manager
               * could send some events after the XSync() has been satisfied,
               * so if we have not received a visibility notification
               * we can wait for up to a second for more events.
               */
              until = [NSDate dateWithTimeIntervalSinceNow: 1.0];
              while (XPending(dpy) == 0 && [until timeIntervalSinceNow] > 0.0)
                {
                  CREATE_AUTORELEASE_POOL(pool);

                  [NSThread sleepUntilDate:
                                [NSDate dateWithTimeIntervalSinceNow: 0.01]];

                  IF_NO_GC([pool release]);
                }
              if (XPending(dpy) == 0)
                {
                  NSLog(@"Waited for a second, but the X system never"
                        @" made the window visible");
                  break;
                }
            }
          XNextEvent(dpy, &xEvent);
          NSDebugLLog(@"Offset", @"Testing ... event %d window %lu\n",
                      xEvent.type, xEvent.xany.window);
          if (xEvent.xany.window != window->ident)
            {
              continue;
            }
          switch (xEvent.type)
            {
              case VisibilityNotify:
                window->visibility = xEvent.xvisibility.state;
                break;

              case ReparentNotify:
                NSDebugLLog(@"Offset", @"%lu ReparentNotify - offset %d %d\n",
                            xEvent.xreparent.window, xEvent.xreparent.x,
                            xEvent.xreparent.y);
                repp = xEvent.xreparent.parent;
                repx = xEvent.xreparent.x;
                repy = xEvent.xreparent.y;
                reparented = YES;
                break;
            }
          if (onScreen == NO && reparented == YES)
            {
              /* If we are not testing on screen, the window will never
               * become visible, so we only wait for it to be reparented
               * and hope that the reparenting indicates completion of
               * an window decoration.
               */
              break;
            }
        }
    }

  extents = [self _getExtents: window->ident];
  if (extents != 0)
    {
      o->l = extents[0];
      o->r = extents[1];
      o->t = extents[2];
      o->b = extents[3];
      o->known = YES;
      NSDebugLLog(@"Offset", @"Extents left %lu, right %lu, top %lu, bottom %lu",
                  extents[0], extents[1], extents[2], extents[3]);
      XFree(extents);
    }
  else if (repp != 0)
    {
      NSDebugLLog(@"Offset", @"Offsets retrieved from ReparentNotify");
      window->parent = repp;
      if (repp != window->root)
        {
          Window parent = repp;
          XWindowAttributes	wattr;
          int			l;
          int			r;
          int			t;
          int			b;

          /* Get the WM offset info which we hope is the same
           * for all parented windows with the same style.
           * The coordinates in the event are insufficient to determine
           * the offsets as the new parent window may have a border,
           * so we must get the attributes of that window and use them
           * to determine our offsets.
           */
          XGetWindowAttributes(dpy, parent, &wattr);
          NSDebugLLog(@"Offset", @"Parent border,width,height %d,%d,%d\n",
                      wattr.border_width, wattr.width, wattr.height);
          l = repx + wattr.border_width;
          t = repy + wattr.border_width;

          // Some window manager e.g. KDE2 put in multiple windows,
          // so we have to find the right parent, closest to root
          /* FIXME: This section of code has caused problems with
             certain users. An X error occurs in XQueryTree and
             later a seg fault in XFree. It's 'commented' out for
             now unless you set the default 'GSDoubleParentWindows'
             or we are reparented to 0,0 (which presumably must mean
             that we have a double parent).
          */
          if (generic.flags.doubleParentWindow == YES
              || (repx == 0 && repy == 0))
            {
              Window new_parent = parent;
              Window root = window->root;

              while (new_parent && (new_parent != window->root))
                {
                  Window *children = 0;
                  unsigned int nchildren;

                  parent = new_parent;
                  repx = wattr.x;
                  repy = wattr.y;
                  NSDebugLLog(@"Offset",
                              @"QueryTree window is %lu (root %lu cwin root %lu)",
                              parent, root, window->root);
                  if (!XQueryTree(dpy, parent, &root, &new_parent,
                                  &children, &nchildren))
                    {
                      new_parent = None;
                      if (children)
                        {
                          NSLog(@"Bad pointer from failed X call?");
                          children = 0;
                        }
                    }
                  if (children)
                    {
                      XFree(children);
                    }
                  if (new_parent && new_parent != window->root)
                    {
                      XGetWindowAttributes(dpy, new_parent, &wattr);
                      l += repx + wattr.border_width;
                      t += repy + wattr.border_width;
                    }
                } /* while */
            } /* generic.flags.doubleParentWindow */

          /* Find total parent size and subtract window size and
           * top-left-corner offset to determine bottom-right-corner
           * offset.
           */
          r = wattr.width + wattr.border_width * 2;
          r -= (window->xframe.size.width + l);
          b = wattr.height + wattr.border_width * 2;
          b -= (window->xframe.size.height + t);

          if ((l >= 0) && (r >= 0) && (t >= 0) && (b >= 0))
            {
              o->l = (float)l;
              o->r = (float)r;
              o->t = (float)t;
              o->b = (float)b;
              o->known = YES;
              NSDebugLLog(@"Offset",
                          @"Style %d lrtb set to %d,%d,%d,%d\n",
                          style, l, r, t, b);
            }
          else
            {
              NSLog(@"Reparenting resulted in negative border %d, %d, %d, %d", l, r, t, b);
            }
        }
    }

  [self termwindow: window->number];
  XSync(dpy, False);
  while (XPending(dpy) > 0)
    {
      XNextEvent(dpy, &xEvent);
      NSDebugLLog(@"Offset", @"Destroying ... event %d window %lu\n",
                  xEvent.type, xEvent.xany.window);
      if (xEvent.xany.window != window->ident)
        {
          continue;
        }
    }
  if (o->known == NO)
    {
      NSLog(@"Failed to determine offsets for style %d", style);
      return NO;
    }
  return YES;
}

- (NSString *) windowManagerName
{
  if (generic.wm & XGWM_WINDOWMAKER)
    {
      return @"WindowMaker";
    }
  else if (generic.wm & XGWM_EWMH)
    {
      int count;
      Window *win = (Window*)PropGetCheckProperty(dpy, DefaultRootWindow(dpy),
                                                  generic._NET_SUPPORTING_WM_CHECK_ATOM,
                                                  XA_WINDOW, 32, -1, &count);
      char *name = (char *)PropGetCheckProperty(dpy, *win,
                                                generic._NET_WM_NAME_ATOM, generic.UTF8_STRING_ATOM,
                                                0, 0, &count);
      if (name)
        {
          NSString *wm_name = [NSString stringWithUTF8String: name];
          XFree(name);
          return wm_name;
        }
    }

  return nil;
}

- (XGWMProtocols) _checkWindowManager
{
  int wmflags;
  Window root;
  Atom	*data;
  int	count;

  root = DefaultRootWindow(dpy);
  wmflags = XGWM_UNKNOWN;

  /* Check for WindowMaker */
  data = (Atom*)PropGetCheckProperty(dpy, root, generic._WINDOWMAKER_WM_PROTOCOLS_ATOM,
                                     XA_ATOM, 32, -1, &count);
  if (data != 0)
    {
      int i = 0;

      while (i < count && data[i] != generic._WINDOWMAKER_NOTICEBOARD_ATOM)
        {
          i++;
        }
      XFree(data);

      if (i < count)
        {
          if (AtomPresentAndPointsToItself(dpy, generic._WINDOWMAKER_NOTICEBOARD_ATOM, XA_WINDOW))
            {
              wmflags |= XGWM_WINDOWMAKER;
            }
        }
      else
        {
          wmflags |= XGWM_WINDOWMAKER;
        }
    }

  /* Now check for Gnome */
  if (AtomPresentAndPointsToItself(dpy, generic._WIN_SUPPORTING_WM_CHECK_ATOM, XA_CARDINAL))
    {
      wmflags |= XGWM_GNOME;
    }

  /* Now check for NET (EWMH) compliant window manager */
  if (AtomPresentAndPointsToItself(dpy, generic._NET_SUPPORTING_WM_CHECK_ATOM, XA_WINDOW))
    {
      wmflags |= XGWM_EWMH;
    }

  NSDebugLLog(@"WM",
	      @"WM Protocols: WindowMaker=(%s) GNOME=(%s) EWMH=(%s)",
	      (wmflags & XGWM_WINDOWMAKER) ? "YES" : "NO",
	      (wmflags & XGWM_GNOME) ? "YES" : "NO",
	      (wmflags & XGWM_EWMH) ? "YES" : "NO");

  return wmflags;
}

- (gswindow_device_t *) _rootWindow
{
  int x, y;
  unsigned int width, height;
  gswindow_device_t *window;

  /* Screen number is negative to avoid conflict with windows */
  window = WINDOW_WITH_TAG(-defScreen);
  if (window)
    return window;

  window = NSAllocateCollectable(sizeof(gswindow_device_t), NSScannedOption);
  memset(window, '\0', sizeof(gswindow_device_t));

  window->display = dpy;
  window->screen_id = defScreen;
  window->ident  = RootWindow(dpy, defScreen);
  window->root   = window->ident;
  window->type   = NSBackingStoreNonretained;
  window->number = -defScreen;
  window->map_state = IsViewable;
  window->visibility = -1;
  window->wm_state = NormalState;
  window->monitor_id = 0;
  if (window->ident)
    {
      XGetGeometry(dpy, window->ident, &window->root,
                   &x, &y, &width, &height,
                   &window->border, &window->depth);
    }
  else
    {
      NSLog(@"Failed to get root window");
      x = 0;
      y = 0;
      width = 0;
      height = 0;
    }

  window->xframe = NSMakeRect(x, y, width, height);
  NSMapInsert (windowtags, (void*)(uintptr_t)window->number, window);
  NSMapInsert (windowmaps, (void*)(uintptr_t)window->ident,  window);
  return window;
}

/* Create the window and screen list if necessary, add the root window to
   the window list as window 0 */
- (void) _checkWindowlist
{
  if (windowmaps)
    return;

  windowmaps = NSCreateMapTable(NSNonOwnedPointerMapKeyCallBacks,
				 NSNonOwnedPointerMapValueCallBacks, 20);
  windowtags = NSCreateMapTable(NSIntMapKeyCallBacks,
				 NSNonOwnedPointerMapValueCallBacks, 20);
}

- (void) _setupMouse
{
  int			numButtons;
  unsigned char		mouseNumbers[7];
  unsigned char		buttons[7] = {
			  Button1,
			  Button2,
			  Button3,
			  Button4,
			  Button5,
			  6,
			  7
			};
  int			masks[5] = {
			  Button1Mask,
			  Button2Mask,
			  Button3Mask,
			  Button4Mask,
			  Button5Mask
			};
  /*
   * Get pointer information - so we know which mouse buttons we have.
   * With a two button
   */
  numButtons = XGetPointerMapping(dpy, mouseNumbers, 7);
  if (numButtons > 7)
    {
      NSDebugLLog(@"XGTrace", @"Warning - mouse/pointer seems to have more than 7 buttons "
	@"(%d) - just using one to seven", numButtons);
      numButtons = 7;
    }
  generic.lMouse = buttons[0];
  generic.lMouseMask = masks[0];
  if (numButtons >= 7)
    {
      generic.scrollLeftMouse = buttons[5];
      generic.scrollRightMouse = buttons[6];
    }

  if (numButtons >= 5)
    {
      generic.upMouse = buttons[3];
      generic.downMouse = buttons[4];
      generic.rMouse = buttons[2];
      generic.rMouseMask = masks[2];
      generic.mMouse = buttons[1];
      generic.mMouseMask = masks[1];
    }
  else if (numButtons == 3)
    {
// FIXME: Button4 and Button5 are ScrollWheel up and ScrollWheel down
//      generic.rMouse = buttons[numButtons-1];
//      generic.rMouseMask = masks[numButtons-1];
      generic.upMouse = 0;
      generic.downMouse = 0;
      generic.rMouse = buttons[2];
      generic.rMouseMask = masks[2];
      generic.mMouse = buttons[1];
      generic.mMouseMask = masks[1];
    }
  else if (numButtons == 2)
    {
      generic.upMouse = 0;
      generic.downMouse = 0;
      generic.rMouse = buttons[1];
      generic.rMouseMask = masks[1];
      generic.mMouse = 0;
      generic.mMouseMask = 0;
    }
  else if (numButtons == 1)
    {
      generic.upMouse = 0;
      generic.downMouse = 0;
      generic.rMouse = 0;
      generic.rMouseMask = 0;
      generic.mMouse = 0;
      generic.mMouseMask = 0;
    }
  else
    {
      NSLog(@"Warning - mouse/pointer seems to have NO buttons");
    }
}

- (void) _setSupportedWMProtocols: (gswindow_device_t *) window
{
  NSWindow *nswin = GSWindowWithNumber(window->number);

  window->numProtocols = 0;
  if (!nswin || [nswin canBecomeKeyWindow])
    {
      window->protocols[window->numProtocols++] = generic.WM_TAKE_FOCUS_ATOM;
    }
  if ((window->win_attrs.window_style & NSClosableWindowMask) != 0)
    window->protocols[window->numProtocols++] = generic.WM_DELETE_WINDOW_ATOM;
  // Add ping protocol for EWMH
  if ((generic.wm & XGWM_EWMH) != 0)
    {
      window->protocols[window->numProtocols++] = generic._NET_WM_PING_ATOM;
#ifdef HAVE_X11_EXTENSIONS_SYNC_H
      window->protocols[window->numProtocols++] = generic._NET_WM_SYNC_REQUEST_ATOM;
#endif
    }
  if ((generic.wm & XGWM_WINDOWMAKER) != 0)
    {
      if ((window->win_attrs.window_style & NSMiniaturizableWindowMask) != 0)
        {
          window->protocols[window->numProtocols++] = generic._GNUSTEP_WM_MINIATURIZE_WINDOW_ATOM;
        }
      window->protocols[window->numProtocols++] = generic._GNUSTEP_WM_HIDE_APP_ATOM;
    }
  NSAssert1(window->numProtocols <= GSMaxWMProtocols,
	    @"Too many protocols (%d > GSMaxWMProtocols)",
	    window->numProtocols);
  XSetWMProtocols(dpy, window->ident, window->protocols, window->numProtocols);
}

- (void) _setupRootWindow
{
  NSProcessInfo		*pInfo = [NSProcessInfo processInfo];
  NSArray		*args;
  unsigned int		i;
  unsigned int		argc;
  char			**argv;
  XClassHint		classhint;
  XTextProperty		windowName;
  NSUserDefaults	*defs;
  const char *host_name = [[pInfo hostName] UTF8String];

  /*
   * Initialize time of last events to be the start of time - not
   * the current time!
   */
  generic.lastClick = 1;
  generic.lastMotion = 1;
  generic.lastTime = 1;

  /*
   * Set up standard atoms.
   */
#ifdef HAVE_XINTERNATOMS
   XInternAtoms(dpy, atom_names, sizeof(atom_names)/sizeof(char*),
                False, generic.atoms);
#else
   {
     int atomCount;

     for (atomCount = 0; atomCount < sizeof(atom_names)/sizeof(char*); atomCount++)
       generic.atoms[atomCount] = XInternAtom(dpy, atom_names[atomCount], False);
   }
#endif

  [self _setupMouse];
  [self _checkWindowlist];

  /*
   * determine window manager in use.
   */
  generic.wm = [self _checkWindowManager];

  if (generic.rootName == 0)
    {
      const char *str = [[pInfo processName] UTF8String];
      int len = strlen(str) +1;

      generic.rootName = malloc(len);
      strncpy(generic.rootName, str, len);
    }

  /*
   * Now check user defaults.
   */
  defs = [NSUserDefaults standardUserDefaults];

  if ([defs objectForKey: @"GSBackHandlesWindowDecorations"])
    {
      handlesWindowDecorations
	= [defs boolForKey: @"GSBackHandlesWindowDecorations"];
    }
  else
    {
      if ([defs objectForKey: @"GSX11HandlesWindowDecorations"])
        {
	  handlesWindowDecorations
	    = [defs boolForKey: @"GSX11HandlesWindowDecorations"];
	}
    }

  generic.flags.useWindowMakerIcons = NO;
  if ((generic.wm & XGWM_WINDOWMAKER) != 0)
    {
      generic.flags.useWindowMakerIcons = YES;
      if ([defs objectForKey: @"UseWindowMakerIcons"] != nil
	&& [defs boolForKey: @"UseWindowMakerIcons"] == NO)
	{
	  generic.flags.useWindowMakerIcons = NO;
	}
    }
  generic.flags.appOwnsMiniwindow = YES;
  if ([defs objectForKey: @"GSAppOwnsMiniwindow"] != nil
    && [defs boolForKey: @"GSAppOwnsMiniwindow"] == NO)
    {
      generic.flags.appOwnsMiniwindow = NO;
    }
  generic.flags.doubleParentWindow = NO;
  if ([defs objectForKey: @"GSDoubleParentWindows"] != nil
    && [defs boolForKey: @"GSDoubleParentWindows"] == YES)
    {
      generic.flags.doubleParentWindow = YES;
    }

  /*
   * make app root window
   */
  ROOT = XCreateSimpleWindow(dpy,RootWindow(dpy,defScreen),0,0,1,1,0,0,0);

  /*
   * set hints for root window
   */
  {
    XWMHints		gen_hints;

    gen_hints.flags = WindowGroupHint | StateHint;
    gen_hints.initial_state = WithdrawnState;
    gen_hints.window_group = ROOT;
    XSetWMHints(dpy, ROOT, &gen_hints);
  }

  /*
   * Mark this as a GNUstep app with the current application name.
   */
  classhint.res_name = generic.rootName;
  classhint.res_class = "GNUstep";
  XSetClassHint(dpy, ROOT, &classhint);

  /*
   * Set app name as root window title - probably unused unless
   * the window manager wants to keep us in a menu or something like that.
   */
  XStringListToTextProperty((char**)&classhint.res_name, 1, &windowName);
  XSetWMName(dpy, ROOT, &windowName);
  XSetWMIconName(dpy, ROOT, &windowName);
  XFree(windowName.value);

  /*
   * Record the information used to start this app.
   * If we have a user default set up (eg. by the openapp script) use it.
   * otherwise use the process arguments.
   */
  args = [defs arrayForKey: @"GSLaunchCommand"];
  if (args == nil)
    {
      args = [pInfo arguments];
    }
  argc = [args count];
  argv = (char**)malloc(argc*sizeof(char*));
  for (i = 0; i < argc; i++)
    {
      argv[i] = (char*)[[args objectAtIndex: i] UTF8String];
    }
  XSetCommand(dpy, ROOT, argv, argc);
  free(argv);

  // Store the host name of the machine we a running on
  XStringListToTextProperty((char**)&host_name, 1, &windowName);
  XSetWMClientMachine(dpy, ROOT, &windowName);
  XFree(windowName.value);

  // Always send GNUstepWMAttributes
  {
    GNUstepWMAttributes	win_attrs;

    /*
     * Tell WindowMaker not to set up an app icon for us - we'll make our own.
     */
    win_attrs.flags = GSExtraFlagsAttr;
    win_attrs.extra_flags = GSNoApplicationIconFlag;
/* Warning ... X-bug .. when we specify 32bit data X actually expects data
 * of type 'long' or 'unsigned long' even on machines where those types
 * hold 64bit values.
 */
    XChangeProperty(dpy, ROOT,
                    generic._GNUSTEP_WM_ATTR_ATOM, generic._GNUSTEP_WM_ATTR_ATOM,
                    32, PropModeReplace, (unsigned char *)&win_attrs,
                    sizeof(GNUstepWMAttributes)/sizeof(CARD32));
  }

  if ((generic.wm & XGWM_EWMH) != 0)
    {
      // Store the id of our process
      long pid = [pInfo processIdentifier];

/* Warning ... X-bug .. when we specify 32bit data X actually expects data
 * of type 'long' or 'unsigned long' even on machines where those types
 * hold 64bit values.
 */
      XChangeProperty(dpy, ROOT,
		      generic._NET_WM_PID_ATOM, XA_CARDINAL,
		      32, PropModeReplace,
		      (unsigned char*)&pid, 1);
      // FIXME: Need to set WM_CLIENT_MACHINE as well.
    }

  /* We need to determine the offsets between the actual decorated window
   * and the window we draw into.
   */
  if (handlesWindowDecorations == YES)
    {
      unsigned		i;
      int		count;
      uint16_t		*offsets;

      /* Offsets for NSBorderlessWindowMask *should* always be zero.
       * We record them in the offsets block only for consistency.
       */
      generic.offsets[0].l = 0.0;
      generic.offsets[0].r = 0.0;
      generic.offsets[0].t = 0.0;
      generic.offsets[0].b = 0.0;
      generic.offsets[0].known = YES;

      /* We trust the _GNUSTEP_FRAME_OFFSETS values set on the root window
       * of the X server if present.
       * Of course, these could have changed if the window manager has
       * changed. (FIXME)
       * The GSIgnoreRootOffsets default turns off this trusting approach.
       */
      if ([defs boolForKey: @"GSIgnoreRootOffsets"] == YES)
        {
          NSLog(@"Ignoring _GNUSTEP_FRAME_OFFSETS root window property.");
          offsets = 0;
        }
      else
        {
          offsets = (uint16_t *)PropGetCheckProperty(dpy, DefaultRootWindow(dpy),
                                                     generic._GNUSTEP_FRAME_OFFSETS_ATOM,
                                                     XA_CARDINAL, 16, 60, &count);
        }

      if (offsets == 0)
        {
          BOOL	ok = YES;

          /* No offsets available on the root window ... so we test each
           * style of window to determine its offsets.
           */
          for (i = 1; i < 16; i++)
            {
              if ([self _checkStyle: i] == NO)
                {
                  ok = NO;	// test failed for this style
                }
            }

          if (ok == YES)
            {
              uint16_t	off[60];

              /* We have obtained all the offsets, so we store them to
               * the root window so that other GNUstep applications don't
               * need to test to determine offsets.
               */
              count = 0;
              for (i = 1; i < 16; i++)
                {
                  off[count++] = (uint16_t)generic.offsets[i].l;
                  off[count++] = (uint16_t)generic.offsets[i].r;
                  off[count++] = (uint16_t)generic.offsets[i].t;
                  off[count++] = (uint16_t)generic.offsets[i].b;
                }
              XChangeProperty(dpy, DefaultRootWindow(dpy),
                              generic._GNUSTEP_FRAME_OFFSETS_ATOM,
                              XA_CARDINAL, 16, PropModeReplace,
                              (unsigned char *)off, 60);
            }
        }
      else
        {
          /* Got offsets from the root window.
           * Let's copy them into our local table.
           */
          count = 0;
          for (i = 1; i < 16; i++)
            {
              generic.offsets[i].l = (float)(offsets[count++]);
              generic.offsets[i].r = (float)(offsets[count++]);
              generic.offsets[i].t = (float)(offsets[count++]);
              generic.offsets[i].b = (float)(offsets[count++]);
              generic.offsets[i].known = YES;
            }
          XFree(offsets);
        }
    }
}

/* Destroys all the windows and other window resources that belong to
   this context */
- (void) _destroyServerWindows
{
  void *key;
  gswindow_device_t *d;
  NSMapEnumerator   enumerator;
  NSMapTable        *mapcopy;

  /* Have to get a copy, since termwindow will remove them from
     the map table */
  mapcopy = NSCopyMapTableWithZone(windowtags, [self zone]);
  enumerator = NSEnumerateMapTable(mapcopy);
  while (NSNextMapEnumeratorPair(&enumerator, &key, (void**)&d) == YES)
    {
      if (d->display == dpy && d->ident != d->root)
	[self termwindow: (int)(intptr_t)key];
    }
  NSFreeMapTable(mapcopy);
}

/* Sets up a backing pixmap when a window is created or resized.  This is
   only done if the Window is buffered or retained. */
- (void) _createBuffer: (gswindow_device_t *)window
{
  if (window->depth == 0)
    window->depth = DefaultDepth(dpy, window->screen_id);
  if (NSWidth(window->xframe) == 0 && NSHeight(window->xframe) == 0)
    {
      NSDebugLLog(@"NSWindow", @"Cannot create buffer for ZeroRect frame");
      return;
    }

  window->buffer = XCreatePixmap(dpy, window->root,
				 NSWidth(window->xframe),
				 NSHeight(window->xframe),
				 window->depth);

  if (!window->buffer)
    {
      NSLog(@"DPS Windows: Unable to create backing store\n");
      return;
    }

  XFillRectangle(dpy,
		 window->buffer,
		 window->gc,
		 0, 0,
		 NSWidth(window->xframe),
		 NSHeight(window->xframe));
}

/*
 * Code to build up a NET WM icon from our application icon
 */

-(BOOL) _createNetIcon: (NSImage*)image
		result: (long**)pixeldata
		  size: (int*)size
{
  NSBitmapImageRep *rep;
  int i, j, w, h, samples;
  unsigned char *data;
  int index;
  long *iconPropertyData;
  int iconSize;

  rep = getStandardBitmap(image);
  if (rep == nil)
    {
      NSLog(@"Wrong image type for WM icon");
      return NO;
    }

  h = [rep pixelsHigh];
  w = [rep pixelsWide];
  samples = [rep samplesPerPixel];
  data = [rep bitmapData];

  iconSize = 2 + w * h;
  iconPropertyData = (long *)malloc(sizeof(long) * iconSize);
  if (iconPropertyData == NULL)
    {
      NSLog(@"No memory for WM icon");
      return NO;
    }

  memset(iconPropertyData, 0, sizeof(long)*iconSize);
  index = 0;
  iconPropertyData[index++] = w;
  iconPropertyData[index++] = h;

  for (i = 0; i < h; i++)
    {
      unsigned char *d = data;

      for (j = 0; j < w; j++)
	{
	  unsigned char A, R, G, B;

	  // red
	  R = d[0];
	  // green
	  G = d[1];
	  // blue
	  B = d[2];
	  // alpha
	  if (samples == 4)
	    {
	      A = d[3];
	    }
	  else
	    {
	      A = 255;
	    }

          iconPropertyData[index++] = A << 24 | R << 16 | G << 8 | B;
	  d += samples;
	}
      data += [rep bytesPerRow];
    }

  *pixeldata = iconPropertyData;
  *size = iconSize;
  return YES;
}

- (void) _setNetWMIconFor: (Window) window
{
  // We store the GNUstep application icon image in the window
  // and use that as our title bar icon.
  // FIXME: This code should rather use the window mini icon,
  // but currently this image is not available in the backend.
  static BOOL didCreateNetIcon = NO;
  static long *iconPropertyData = NULL;
  static int iconSize;
  NSImage *image;

  if (!didCreateNetIcon)
    {
      if (iconPropertyData != NULL)
        {
	  free(iconPropertyData);
	}

      image = [NSApp applicationIconImage];
      if (image != nil)
        {
	  didCreateNetIcon = YES;
	  [self _createNetIcon: image
			result: &iconPropertyData
			  size: &iconSize];
	}
    }

  if (iconPropertyData != 0)
    {
      XChangeProperty(dpy, window,
		      generic._NET_WM_ICON_ATOM, XA_CARDINAL,
		      32, PropModeReplace,
		      (unsigned char *)iconPropertyData, iconSize);
    }
}

- (int) window: (NSRect)frame : (NSBackingStoreType)type : (unsigned int)style
	      : (int)screen
{
  gswindow_device_t	*window;
  gswindow_device_t	*root;
  XGCValues		values;
  unsigned long		valuemask;
  XClassHint		classhint;
  RContext              *context;

  NSDebugLLog(@"XGTrace", @"DPSwindow: %@ %d", NSStringFromRect(frame), (int)type);
  
  /* WindowMaker hack: Reuse the empty app icon allocated in _createWMAppiconHack
   * for the real app icon. */
  if ((style & NSIconWindowMask) == NSIconWindowMask && _wmAppIcon != -1)
    {
      int win = _wmAppIcon;
      NSDebugLLog(@"XGTrace",
                  @"WindowMaker hack: Returning window %d as app icon window", win);
      _wmAppIcon = -1;
      return win;
    }
  
  root = [self _rootWindow];
  context = [self screenRContext];

  /* Create the window structure and set the style early so we can use it to
  convert frames. */
  window = NSAllocateCollectable(sizeof(gswindow_device_t), NSScannedOption);
  memset(window, '\0', sizeof(gswindow_device_t));
  window->display = dpy;
  window->screen_id = defScreen;
  window->monitor_id = screen;

  window->win_attrs.flags |= GSWindowStyleAttr;
  if (handlesWindowDecorations)
    {
      window->win_attrs.window_style = style;
    }
  else
    {
      window->win_attrs.window_style
        = style & (NSIconWindowMask | NSMiniWindowMask);
    }

  frame = [self _OSFrameToXFrame: frame for: window];

  /* We're not allowed to create a zero rect window */
  if (NSWidth(frame) <= 0 || NSHeight(frame) <= 0)
    {
      frame.size.width = 2;
      frame.size.height = 2;
    }
  window->xframe = frame;
  window->type = type;
  window->root = root->ident;
  window->parent = root->ident;
  window->depth = context->depth;
  window->xwn_attrs.border_pixel = context->black;
  window->xwn_attrs.background_pixel = context->white;
  window->xwn_attrs.colormap = context->cmap;
  window->xwn_attrs.save_under = False;
  /*
   * Setting this to True should only be done, when we also grap the pointer.
   * It could be done for popup windows, but at this point we don't know
   * about the usage of the window.
   */
  window->xwn_attrs.override_redirect = False;

  window->ident = XCreateWindow(dpy, window->root,
				NSMinX(frame), NSMinY(frame),
				NSWidth(frame), NSHeight(frame),
				0,
				context->depth,
				CopyFromParent,
				context->visual,
                                // Don't set the CWBackPixel, as the background of the
                                // window may be different.
				(CWColormap | CWBorderPixel | CWOverrideRedirect),
				&window->xwn_attrs);

  /*
   * Mark this as a GNUstep app with the current application name.
   */
  classhint.res_name = generic.rootName;
  classhint.res_class = "GNUstep";
  XSetClassHint(dpy, window->ident, &classhint);

  window->map_state = IsUnmapped;
  window->visibility = -1;
  window->wm_state = WithdrawnState;

  // Create an X GC for the content view set it's colors
  values.foreground = window->xwn_attrs.background_pixel;
  values.background = window->xwn_attrs.background_pixel;
  values.function = GXcopy;
  valuemask = (GCForeground | GCBackground | GCFunction);
  window->gc = XCreateGC(dpy, window->ident, valuemask, &values);

  /* Set the X event mask */
  select_input(dpy, window->ident, NO);

  /*
   * Initial attributes for any GNUstep window tell Window Maker not to
   * create an app icon for us.
   */
  window->win_attrs.flags |= GSExtraFlagsAttr;
  window->win_attrs.extra_flags |= GSNoApplicationIconFlag;

  /*
   * Prepare size/position hints, but don't set them now - ordering
   * the window in should automatically do it.
   */
  frame = [self _XFrameToXHints: window->xframe for: window];
  window->siz_hints.x = NSMinX(frame);
  window->siz_hints.y = NSMinY(frame);
  window->siz_hints.width = NSWidth(frame);
  window->siz_hints.height = NSHeight(frame);
  window->siz_hints.flags = USPosition|PPosition|USSize|PSize;

  // Always send GNUstepWMAttributes
/* Warning ... X-bug .. when we specify 32bit data X actually expects data
 * of type 'long' or 'unsigned long' even on machines where those types
 * hold 64bit values.
 */
  XChangeProperty(dpy, window->ident, generic._GNUSTEP_WM_ATTR_ATOM,
		      generic._GNUSTEP_WM_ATTR_ATOM, 32, PropModeReplace,
		      (unsigned char *)&window->win_attrs,
		      sizeof(GNUstepWMAttributes)/sizeof(CARD32));

  // send to the WM window style hints
  if ((generic.wm & XGWM_WINDOWMAKER) == 0)
    {
      setWindowHintsForStyle(dpy, window->ident, style, generic._MOTIF_WM_HINTS_ATOM);
    }

  // For window managers supporting EWMH, but not Window Maker,
  // where we use a different solution, set the window icon.
  if ((generic.wm & XGWM_EWMH) != 0)
    {
      [self _setNetWMIconFor: window->ident];
    }

  // Use the globally active input mode
  window->gen_hints.flags = InputHint;
  window->gen_hints.input = False;
  // All the windows of a GNUstep application belong to one group.
  window->gen_hints.flags |= WindowGroupHint;
  window->gen_hints.window_group = ROOT;

#ifdef HAVE_X11_EXTENSIONS_SYNC_H
  /**
   * Setup net_wm_sync_request_counter
   */
  {
    XSyncValue value;
    XSyncIntToValue(&value, 0);
    window->net_wm_sync_request_counter = XSyncCreateCounter(dpy, value);
    XChangeProperty(dpy,
		    window->ident,
		    generic._NET_WM_SYNC_REQUEST_COUNTER_ATOM,
		    XA_CARDINAL,
		    32,
		    PropModeReplace,
		    (unsigned char *) &(window->net_wm_sync_request_counter),
		    1);
    window->net_wm_sync_request_counter_value_low = 0;
    window->net_wm_sync_request_counter_value_high = 0;
  }
#endif

  /*
   * Prepare the protocols supported by the window.
   * These protocols should be set on the window when it is ordered in.
   */
  [self _setSupportedWMProtocols: window];

  window->exposedRects = [NSMutableArray new];
  window->region = XCreateRegion();
  window->buffer = 0;
  window->alpha_buffer = 0;
  window->ic = 0;

  // make sure that new window has the correct cursor
  [self _initializeCursorForXWindow: window->ident];

  /*
   * FIXME - should this be protected by a lock for thread safety?
   * generate a unique tag for this new window.
   */
  do
    {
      last_win_num++;
    }
  while (last_win_num == 0 || WINDOW_WITH_TAG(last_win_num) != 0);
  window->number = last_win_num;

  // Insert window into the mapping
  NSMapInsert(windowmaps, (void*)(uintptr_t)window->ident, window);
  NSMapInsert(windowtags, (void*)(uintptr_t)window->number, window);
  [self _setWindowOwnedByServer: window->number];

  return window->number;
}

- (int) nativeWindow: (void *)winref : (NSRect*)frame : (NSBackingStoreType*)type
		    : (unsigned int*)style : (int*)screen
{
  gswindow_device_t	*window;
  gswindow_device_t	*root;
  XGCValues		values;
  unsigned long		valuemask;
  RContext              *context;
  XWindowAttributes win_attributes;
  Window windowRef;
  NSRect xframe;

  windowRef = *((Window*)winref);
  NSDebugLLog(@"XGTrace", @"nativeWindow: %lu", windowRef);
  if (!XGetWindowAttributes(dpy, windowRef, &win_attributes))
    {
      return 0;
    }

  *screen = XScreenNumberOfScreen(win_attributes.screen);
  *type = NSBackingStoreNonretained;
  *style = NSBorderlessWindowMask;
  root = [self _rootWindow];
  context = [self screenRContext];

  /* Create the window structure and set the style early so we can use it to
  convert frames. */
  window = NSAllocateCollectable(sizeof(gswindow_device_t), NSScannedOption);
  memset(window, '\0', sizeof(gswindow_device_t));
  window->display = dpy;
  window->screen_id = *screen;
  window->monitor_id = 0;
  window->ident = windowRef;
  window->root = root->ident;
  window->parent = root->ident;
  window->type = *type;
  window->win_attrs.flags |= GSWindowStyleAttr;
  window->win_attrs.window_style = *style;

  window->border = win_attributes.border_width;
  window->depth = win_attributes.depth;
  window->xframe = NSMakeRect(win_attributes.x, win_attributes.y,
			      win_attributes.width, win_attributes.height);
  window->xwn_attrs.colormap = win_attributes.colormap;
  window->xwn_attrs.save_under = win_attributes.save_under;
  window->xwn_attrs.override_redirect = win_attributes.override_redirect;
  window->map_state = win_attributes.map_state;

  window->xwn_attrs.border_pixel = context->black;
  window->xwn_attrs.background_pixel = context->white;
  window->visibility = -1;
  window->wm_state = [self _wm_state: windowRef];

  // Create an X GC for the content view set it's colors
  values.foreground = window->xwn_attrs.background_pixel;
  values.background = window->xwn_attrs.background_pixel;
  values.function = GXcopy;
  valuemask = (GCForeground | GCBackground | GCFunction);
  window->gc = XCreateGC(dpy, window->ident, valuemask, &values);

  /*
   * Initial attributes for any GNUstep window tell Window Maker not to
   * create an app icon for us.
   */
  window->win_attrs.flags |= GSExtraFlagsAttr;
  window->win_attrs.extra_flags |= GSNoApplicationIconFlag;

  /*
   * Prepare size/position hints, but don't set them now - ordering
   * the window in should automatically do it.
   */
  *frame = [self _XFrameToOSFrame: window->xframe for: window];

  // Use the globally active input mode
  window->gen_hints.flags = InputHint;
  window->gen_hints.input = False;
  // All the windows of a GNUstep application belong to one group.
  window->gen_hints.flags |= WindowGroupHint;
  window->gen_hints.window_group = ROOT;
  window->exposedRects = [NSMutableArray new];
  window->region = XCreateRegion();
  window->buffer = 0;
  window->alpha_buffer = 0;
  window->ic = 0;

  /*
   * Prepare size/position hints, but don't set them now - ordering
   * the window in should automatically do it.
   */
  xframe = [self _XFrameToXHints: window->xframe for: window];
  window->siz_hints.x = NSMinX(xframe);
  window->siz_hints.y = NSMinY(xframe);
  window->siz_hints.width = NSWidth(xframe);
  window->siz_hints.height = NSHeight(xframe);
  window->siz_hints.flags = USPosition|PPosition|USSize|PSize;

  // make sure that new window has the correct cursor
  [self _initializeCursorForXWindow: window->ident];

  /*
   * FIXME - should this be protected by a lock for thread safety?
   * generate a unique tag for this new window.
   */
  do
    {
      last_win_num++;
    }
  while (last_win_num == 0 || WINDOW_WITH_TAG(last_win_num) != 0);
  window->number = last_win_num;

  // Insert window into the mapping
  NSMapInsert(windowmaps, (void*)(uintptr_t)window->ident, window);
  NSMapInsert(windowtags, (void*)(uintptr_t)window->number, window);
  [self _setWindowOwnedByServer: window->number];
  return window->number;
}

- (void) termwindow: (int)win
{
  gswindow_device_t *window;

  window = WINDOW_WITH_TAG(win);
  if (!window)
    return;

  if (window->root == window->ident)
    {
      NSLog(@"DPStermwindow: Trying to destroy root window");
      return;
    }

  NSDebugLLog(@"XGTrace", @"DPStermwindow: %d", win);
  if (window->ic)
    {
      [inputServer ximCloseIC: window->ic];
    }
  if (window->ident)
    {
      XDestroyWindow(dpy, window->ident);
      if (window->gc)
	XFreeGC (dpy, window->gc);
      if (generic.cachedWindow != 0 &&
	  window->ident == ((gswindow_device_t*)(generic.cachedWindow))->ident)
        {
	  generic.cachedWindow = 0;
	}
      NSMapRemove(windowmaps, (void*)window->ident);
    }

  if (window->buffer)
    XFreePixmap (dpy, window->buffer);
  if (window->alpha_buffer)
    XFreePixmap (dpy, window->alpha_buffer);
  if (window->region)
    XDestroyRegion (window->region);
  RELEASE(window->exposedRects);
  NSMapRemove(windowtags, (void*)(uintptr_t)win);
  NSZoneFree(0, window);
}

/*
 * Return the offsets between the window content-view and it's frame
 * depending on the window style.
 */
- (void) styleoffsets: (float *) l : (float *) r : (float *) t : (float *) b
		     : (unsigned int) style
{
  [self styleoffsets: l : r : t : b : style : (Window) 0];
}

- (void) styleoffsets: (float *) l : (float *) r : (float *) t : (float *) b
		     : (unsigned int) style : (Window) win
{
  Offsets	*o;

  if (!handlesWindowDecorations)
    {
      /*
      If we don't handle decorations, all our windows are going to be
      border- and decorationless. In that case, -gui won't call this method,
      but we still use it internally.
      */
      *l = *r = *t = *b = 0.0;
      return;
    }

  /* First check _NET_FRAME_EXTENTS */
  if (win  && ((generic.wm & XGWM_EWMH) != 0))
    {
      unsigned long *extents = [self _getExtents: win];

      if (extents)
        {
          NSDebugLLog(@"Frame",
                      @"Window %lu, left %lu, right %lu, top %lu, bottom %lu",
                      win, extents[0], extents[1], extents[2], extents[3]);
          *l = extents[0];
          *r = extents[1];
          *t = extents[2];
          *b = extents[3];
          XFree(extents);
          return;
        }
    }

  if ((style & NSIconWindowMask) || (style & NSMiniWindowMask))
    {
      style = NSBorderlessWindowMask;
    }

  /* Next try to get the offset information that we have obtained from
     the WM. This will only work if the application has already created
     a window that has been reparented by the WM. Otherwise we have to
     guess.
  */
  o = generic.offsets + (style & 15);
  if (o->known == YES)
    {
      *l = o->l;
      *r = o->r;
      *b = o->b;
      *t = o->t;
      NSDebugLLog(@"Frame",
                  @"Window %lu, offsets %f, %f, %f, %f",
                  win, *l, *r, *t, *b);
      return;
    }

  NSLog(@"styleoffsets ... guessing offsets\n");
  if ((generic.wm & XGWM_WINDOWMAKER) != 0)
    {
      *l = *r = *t = *b = 1.0;
      if (NSResizableWindowMask & style)
        {
          *b = 9.0;
        }
      if ((style & NSTitledWindowMask) || (style & NSClosableWindowMask)
          || (style & NSMiniaturizableWindowMask))
        {
          *t = 25.0;
        }
      NSDebugLLog(@"Frame",
                  @"Window %lu, windowmaker %f, %f, %f, %f",
                  win, *l, *r, *t, *b);
    }
  else if ((generic.wm & XGWM_EWMH) != 0)
    {
      *l = *r = *t = *b = 4;
      if (NSResizableWindowMask & style)
        {
          *b = 7;
        }
      if ((style & NSTitledWindowMask) || (style & NSClosableWindowMask)
          || (style & NSMiniaturizableWindowMask))
        {
          *t = 20;
        }
      NSDebugLLog(@"Frame",
                  @"Window %lu, EWMH %f, %f, %f, %f",
                  win, *l, *r, *t, *b);
    }
  else
    {
      /* No known WM protocols */
      /*
       * FIXME
       * This should make a good guess - at the moment use no offsets.
       */
      *l = *r = *t = *b = 0.0;
      NSDebugLLog(@"Frame",
                  @"Window %lu, unknown %f, %f, %f, %f",
                  win, *l, *r, *t, *b);
    }
}

- (void) stylewindow: (unsigned int)style : (int) win
{
  gswindow_device_t	*window;

  NSAssert(handlesWindowDecorations, @"-stylewindow:: called when handlesWindowDecorations==NO");

  window = WINDOW_WITH_TAG(win);
  if (!window)
    return;

  NSDebugLLog(@"XGTrace", @"DPSstylewindow: %d : %d", style, win);
  if (window->win_attrs.window_style != style
    || (window->win_attrs.flags & GSWindowStyleAttr) == 0)
    {
      NSRect h;
      window->win_attrs.flags |= GSWindowStyleAttr;
      window->win_attrs.window_style = style;

      /* Fix up hints */
      h = [self _XFrameToXHints: window->xframe for: window];
      window->siz_hints.x = NSMinX(h);
      window->siz_hints.y = NSMinY(h);
      window->siz_hints.width = NSWidth(h);
      window->siz_hints.height = NSHeight(h);

      // send to the WM window style hints
/* Warning ... X-bug .. when we specify 32bit data X actually expects data
 * of type 'long' or 'unsigned long' even on machines where those types
 * hold 64bit values.
 */
      XChangeProperty(dpy, window->ident, generic._GNUSTEP_WM_ATTR_ATOM,
			  generic._GNUSTEP_WM_ATTR_ATOM, 32, PropModeReplace,
			  (unsigned char *)&window->win_attrs,
			  sizeof(GNUstepWMAttributes)/sizeof(CARD32));

      // send to the WM window style hints
      if ((generic.wm & XGWM_WINDOWMAKER) == 0)
	{
	  setWindowHintsForStyle(dpy, window->ident, style, generic._MOTIF_WM_HINTS_ATOM);
	}
    }
}

- (void) setbackgroundcolor: (NSColor *)color : (int)win
{
  XColor xf;
  gswindow_device_t *window;

  window = WINDOW_WITH_TAG(win);
  if (!window)
    return;


  color = [color colorUsingColorSpaceName: NSDeviceRGBColorSpace];
  xf.red   = 65535 * [color redComponent];
  xf.green = 65535 * [color greenComponent];
  xf.blue  = 65535 * [color blueComponent];
  NSDebugLLog(@"XGTrace", @"setbackgroundcolor: %@ %d", color, win);
  xf = [self xColorFromColor: xf];
  window->xwn_attrs.background_pixel = xf.pixel;
  XSetWindowBackground(dpy, window->ident, window->xwn_attrs.background_pixel);
}

- (void) windowbacking: (NSBackingStoreType)type : (int) win
{
  gswindow_device_t *window;

  window = WINDOW_WITH_TAG(win);
  if (!window)
    return;

  NSDebugLLog(@"XGTrace", @"DPSwindowbacking: %d : %d", (int)type, win);

  window->type = type;
  if ((window->gdriverProtocol & GDriverHandlesBacking))
    {
      return;
    }

  if (window->buffer && type == NSBackingStoreNonretained)
    {
      XFreePixmap (dpy, window->buffer);
      window->buffer = 0;
    }
  else if (window->buffer == 0)
    {
      [self _createBuffer: window];
    }
}

- (void) titlewindow: (NSString *)window_title : (int) win
{
  gswindow_device_t *window;

  window = WINDOW_WITH_TAG(win);
  if (!window)
    return;

  NSDebugLLog(@"XGTrace", @"DPStitlewindow: %@ : %d", window_title, win);
  if (window_title && window->ident)
    {
      XTextProperty windowName;
      const char *title;
      int error = XLocaleNotSupported;

      if (handlesWindowDecorations && (generic.wm & XGWM_WINDOWMAKER) == 0 &&
	  (window->win_attrs.flags & GSExtraFlagsAttr) &&
	  (window->win_attrs.extra_flags & GSDocumentEditedFlag))
        {
	  window_title = [@"*" stringByAppendingString: window_title];
	}

#ifdef X_HAVE_UTF8_STRING
      title = [window_title UTF8String];
      error = Xutf8TextListToTextProperty(dpy, (char **)&title, 1,
					  XUTF8StringStyle,
					  &windowName);
#endif
      if (error != Success)
        {
	  title = [window_title lossyCString];
	  XStringListToTextProperty((char **)&title, 1,
				    &windowName);
	}
      XSetWMName(dpy, window->ident, &windowName);
      XSetWMIconName(dpy, window->ident, &windowName);
      XFree(windowName.value);

      {
      /* Set _NET_WM_NAME and _NET_WM_ICON_NAME */
      char *name = (char *)[window_title UTF8String];
      XChangeProperty(dpy, window->ident, generic._NET_WM_NAME_ATOM, generic.UTF8_STRING_ATOM,
		      8, PropModeReplace,
		      (unsigned char *)name, strlen(name));
      XChangeProperty(dpy, window->ident, generic._NET_WM_ICON_NAME_ATOM, generic.UTF8_STRING_ATOM,
		      8, PropModeReplace,
		      (unsigned char *)name, strlen(name));
      }
    }
}

- (void) docedited: (int)edited : (int) win
{
  gswindow_device_t *window;

  window = WINDOW_WITH_TAG(win);
  if (!window)
    return;

  NSDebugLLog(@"XGTrace", @"DPSdocedited: %d : %d", edited, win);
  window->win_attrs.flags |= GSExtraFlagsAttr;
  if (edited)
    {
      window->win_attrs.extra_flags |= GSDocumentEditedFlag;
    }
  else
    {
      window->win_attrs.extra_flags &= ~GSDocumentEditedFlag;
    }

  // send WindowMaker WM window style hints
  // Always send GNUstepWMAttributes
/* Warning ... X-bug .. when we specify 32bit data X actually expects data
 * of type 'long' or 'unsigned long' even on machines where those types
 * hold 64bit values.
 */
  XChangeProperty(dpy, window->ident,
	generic._GNUSTEP_WM_ATTR_ATOM, generic._GNUSTEP_WM_ATTR_ATOM,
	32, PropModeReplace, (unsigned char *)&window->win_attrs,
	sizeof(GNUstepWMAttributes)/sizeof(CARD32));

  /*
   * Reflect the document's edited status in the window's title when the
   * backend does not manage the window decorations
   */
  if (handlesWindowDecorations && (generic.wm & XGWM_WINDOWMAKER) == 0)
    {
      NSWindow *nswin = GSWindowWithNumber(win);
      [self titlewindow: [nswin title] : win];
    }
}

- (BOOL) appOwnsMiniwindow
{
  return generic.flags.appOwnsMiniwindow;
}

- (void) miniwindow: (int) win
{
  gswindow_device_t	*window;
  XEvent e;

  window = WINDOW_WITH_TAG(win);
  if (window == 0)
    {
      return;
    }
  NSDebugLLog(@"XGTrace", @"DPSminiwindow: %d ", win);
  /*
   * If we haven't already done so - set the icon window hint for this
   * window so that the GNUstep miniwindow is displayed (if supported).
   */
  if (generic.flags.appOwnsMiniwindow
      && (window->gen_hints.flags & IconWindowHint) == 0)
    {
      NSWindow		*nswin;

      nswin = GSWindowWithNumber(window->number);
      if (nswin != nil)
	{
	  int			iNum = [[nswin counterpart] windowNumber];
	  gswindow_device_t	*iconw = WINDOW_WITH_TAG(iNum);

	  if (iconw != 0)
	    {
	      window->gen_hints.flags |= IconWindowHint;
	      window->gen_hints.icon_window = iconw->ident;
	      XSetWMHints(dpy, window->ident, &window->gen_hints);
	    }
	}
    }

  /* First discard all existing events for thsi window ... we don't need them
   * because the window is being miniaturised, and they might confuse us when
   * we try to find the event telling us that the miniaturisation worked.
   */
  XSync(dpy, False);
  while (XCheckWindowEvent(dpy, window->ident, 0xffffffff, &e) == True) ;

  /* When the application owns the mini window, we withdraw the window itself
     during miniaturization and put up the mini window instead. However, this
     does not work for WindowMaker, which unmaps the mini window, too, when
     the actual window is withdrawn. Fortunately, miniaturizing the actual
     window does already the right thing on WindowMaker. */
  /* Note: The wm_state != IconicState check is there to avoid iconifying a
     window when -miniwindow: is called as a consequence of processing a
     GSAppKitWindowMiniaturize event. This avoids iconifying shaded windows
     under metacity, which sets _NET_WM_STATE for shaded windows to both
     _NET_WM_STATE_SHADED and _NET_WM_STATE_HIDDEN. */
  if (generic.flags.appOwnsMiniwindow && !(generic.wm & XGWM_WINDOWMAKER))
    XWithdrawWindow(dpy, window->ident, window->screen_id);
  else if (window->wm_state != IconicState)
    XIconifyWindow(dpy, window->ident, window->screen_id);
}

/* Actually this is "hide application" action.
   However, key press may be received by particular window. */
- (BOOL) hideApplication: (int)win
{
  gswindow_device_t *window;
  
  if ((generic.wm & XGWM_WINDOWMAKER) == 0)
    return NO;
    
  window = [XGServer _windowWithTag: win];
  [self _sendRoot: window->root
             type: generic._WINDOWMAKER_WM_FUNCTION_ATOM
           window: ROOT
            data0: WMFHideApplication
            data1: CurrentTime
            data2: 0
            data3: 0];
  XSync(dpy, False);
    
  return YES;
}

/**
   Make sure we have the most up-to-date window information and then
   make sure the context has our new information
*/
- (void) setWindowdevice: (int)win forContext: (NSGraphicsContext *)ctxt
{
  unsigned width, height;
  gswindow_device_t *window;
  float	t, b, l, r;

  NSDebugLLog(@"XGTrace", @"DPSwindowdevice: %d ", win);
  window = WINDOW_WITH_TAG(win);
  if (!window)
    {
      NSLog(@"Invalidparam: Invalid window number %d", win);
      return;
    }

  if (!window->ident)
    return;

  width = NSWidth(window->xframe);
  height = NSHeight(window->xframe);

  if (window->buffer
    && (window->buffer_width != width || window->buffer_height != height)
    && (window->gdriverProtocol & GDriverHandlesBacking) == 0)
    {
      [[self class] waitAllContexts];
      XFreePixmap(dpy, window->buffer);
      window->buffer = 0;
      if (window->alpha_buffer)
        XFreePixmap (dpy, window->alpha_buffer);
      window->alpha_buffer = 0;
    }

  window->buffer_width = width;
  window->buffer_height = height;

#if (BUILD_GRAPHICS == GRAPHICS_xlib)
  /* The window->gdriverProtocol flag does not work as intended;
     it doesn't get set until after _createBuffer is called,
     so it's not a relaible way to tell whether we need to create
     a backing pixmap here.

     This method was causing a serious leak with the default
     Cairo XGCairoModernSurface because we were erroneously
     creating a pixmap, and then not releasing it in -termwindow:
     because the GDriverHandlesBacking flag was set in between.

     So this #if servers as a foolproof way of ensuring we
     don't ever create window->buffer in the default configuration.
   */
  if ((window->buffer == 0) && (window->type != NSBackingStoreNonretained) &&
      ((window->gdriverProtocol & GDriverHandlesBacking) == 0))
    {
      [self _createBuffer: window];
    }
#endif

  [self styleoffsets: &l : &r : &t : &b
		    : window->win_attrs.window_style : window->ident];
  GSSetDevice(ctxt, window, l, NSHeight(window->xframe) + b);
  DPSinitmatrix(ctxt);
  //DPStranslate(ctxt, -l, -b);
  DPSinitclip(ctxt);
}

/*
Build a Pixmap of our icon so the windowmaker dock will remember our
icon when we quit.

ICCCM really only allows 1-bit pixmaps for IconPixmapHint, but this code is
only used if windowmaker is the window manager, and windowmaker can handle
real color pixmaps.
*/
static Pixmap xIconPixmap;
static Pixmap xIconMask;
static BOOL didCreatePixmaps;

Pixmap
alphaMaskForImage(Display *xdpy, Drawable draw, const unsigned char *data,
                  int w, int h, int colors, unsigned int alpha_treshold)
{
  Pixmap pix;
  // (w/8) rounded up times height
  int bitmapSize = ((w + 7) >> 3) * h;
  unsigned char *aData = calloc(1, bitmapSize);

  if (colors == 4)
    {
      int j, i;
      unsigned int ialpha;
      unsigned char *cData = aData;

      // skip R, G, B
      data += 3;

      for (j = 0; j < h; j++)
	{
	  int k = 0;

	  for (i = 0; i < w; i++, k++)
	    {
	      if (k > 7)
		{
	      	  cData++;
	      	  k = 0;
	      	}
	      ialpha = (unsigned int)(*data);
	      if (ialpha > alpha_treshold)
                {
		  *cData |= (0x01 << k);
                }
	      data += 4;
	    }
	  cData++;
	}
    }
  else
    {
      memset(aData, 0xff, bitmapSize);
    }

  pix = XCreatePixmapFromBitmapData(xdpy, draw, (char *)aData, w, h,
				    1L, 0L, 1);
  free(aData);
  return pix;
}

// Convert RGBA unpacked to ARGB packed.
// Packed ARGB values are layed out as ARGB on big endian systems
// and as BGRA on little endian systems
void
swapColors(unsigned char *image_data, NSBitmapImageRep *rep)
{
  unsigned char *target = image_data;
  unsigned char *source = [rep bitmapData];
  NSInteger width = [rep pixelsWide];
  NSInteger height = [rep pixelsHigh];
  NSInteger samples_per_pixel = [rep samplesPerPixel];
  NSInteger bytes_per_row = [rep bytesPerRow];
  unsigned char *r, *g, *b, *a;
  NSInteger     x, y;

#if GS_WORDS_BIGENDIAN
  // RGBA -> ARGB
  r = target + 1;
  g = target + 2;
  b = target + 3;
  a = target;
#else
  // RGBA -> BGRA
  r = target + 2;
  g = target + 1;
  b = target;
  a = target + 3;
#endif

  if (samples_per_pixel == 4)
    {
      for (y = 0; y < height; y++)
        {
          unsigned char *d = source;
          for (x = 0; x < width; x++)
            {
              *r = d[0];
              *g = d[1];
              *b = d[2];
              *a = d[3];
              r += 4;
              g += 4;
              b += 4;
              a += 4;
              d += samples_per_pixel;
            }
          source += bytes_per_row;
        }
    }
  else if (samples_per_pixel == 3)
    {
      for (y = 0; y < height; y++)
        {
          unsigned char *d = source;
          for (x = 0; x < width; x++)
            {
              *r = d[0];
              *g = d[1];
              *b = d[2];
              *a = 255;
              r += 4;
              g += 4;
              b += 4;
              a += 4;
              d += samples_per_pixel;
            }
          source += bytes_per_row;
        }
    }
}

- (int) _createAppIconPixmaps
{
  NSBitmapImageRep *rep;
  int              width, height, colors;
  RContext         *rcontext;
  RXImage          *rxImage;

  NSAssert(!didCreatePixmaps, @"called _createAppIconPixmap twice");

  didCreatePixmaps = YES;

  rep = getStandardBitmap([NSApp applicationIconImage]);
  if (rep == nil)
    {
      return 0;
    }

  rcontext = [self screenRContext];
  width = [rep pixelsWide];
  height = [rep pixelsHigh];
  colors = [rep samplesPerPixel];

  if (rcontext->depth != 32)
    {
      NSLog(@"Unsupported context depth %d", rcontext->depth);
      return 0;
    }

  rxImage = RCreateXImage(rcontext, rcontext->depth, width, height);
  if (rxImage->image->bytes_per_line != 4 * width)
    {
      NSLog(@"bytes_per_line %d does not match width %d", rxImage->image->bytes_per_line, width);
      RDestroyXImage(rcontext, rxImage);
      return 0;
    }

  swapColors((unsigned char *)rxImage->image->data, rep);
  
  xIconPixmap = XCreatePixmap(dpy, rcontext->drawable,
                              width, height, rcontext->depth);
  XPutImage(dpy, xIconPixmap, rcontext->copy_gc, rxImage->image,
            0, 0, 0, 0, width, height);
  RDestroyXImage(rcontext, rxImage);
  
  xIconMask = alphaMaskForImage(dpy, ROOT, [rep bitmapData], width, height, colors, 0);

  return 1;
}

- (void) orderwindow: (int)op : (int)otherWin : (int)winNum
{
  gswindow_device_t	*window;
  gswindow_device_t	*other;
  int		level;

  window = WINDOW_WITH_TAG(winNum);
  if (winNum == 0 || window == NULL)
    {
      NSLog(@"Invalidparam: Ordering invalid window %d", winNum);
      return;
    }

  if (op != NSWindowOut)
    {
      /*
       * Some window managers ignore any hints and properties until the
       * window is actually mapped, so we need to set them all up
       * immediately before mapping the window ...
       */

      setNormalHints(dpy, window);
      XSetWMHints(dpy, window->ident, &window->gen_hints);

      /*
       * If we are asked to set hints for the appicon and the window manager is
       * to control it, we must let the window manager know that this window is
       * the icon window for the app root window.
       */
      if ((window->win_attrs.window_style & NSIconWindowMask) != 0)
	{
	  XWMHints gen_hints;

	  gen_hints.flags = WindowGroupHint | StateHint | IconWindowHint;
	  gen_hints.initial_state = WithdrawnState;
	  gen_hints.window_group = ROOT;
	  gen_hints.icon_window = window->ident;

          if ((generic.wm & XGWM_WINDOWMAKER) != 0)
            {
              if (!didCreatePixmaps)
                {
                  [self _createAppIconPixmaps];
                }
              if (xIconPixmap)
                {
                  gen_hints.flags |= IconPixmapHint;
                  gen_hints.icon_pixmap = xIconPixmap;
                }
              if (xIconMask)
                {
                  gen_hints.flags |= IconMaskHint;
                  gen_hints.icon_mask = xIconMask;
                }
            }

	  XSetWMHints(dpy, ROOT, &gen_hints);
	}

      /*
       * Tell the window manager what protocols this window conforms to.
       */
      [self _setSupportedWMProtocols: window];
    }

  if (generic.flags.useWindowMakerIcons == 1)
    {
      /*
       * Icon windows are mapped/unmapped by the window manager - so we
       * mustn't do anything with them here - though we can raise the
       * application root window to let Window Maker know it should use
       * our appicon window.
       */
      if ((window->win_attrs.window_style & NSIconWindowMask) != 0)
	{
          NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
          if (op != NSWindowOut && window->map_state == IsUnmapped &&
              [[defaults objectForKey: @"autolaunch"] isEqualToString:@"YES"])
 	    {
              XEvent ev;

              // Inform WindowMaker to ignore focus events
              ev.xclient.type = ClientMessage;
              ev.xclient.message_type = generic.WM_IGNORE_FOCUS_EVENTS_ATOM;
              ev.xclient.format = 32;
              ev.xclient.data.l[0] = True;
              XSendEvent(dpy, ROOT, True, EnterWindowMask, &ev);
              // Display application icon
              XMapWindow(dpy, ROOT);
              // Inform WindowMaker to process focus events again
              ev.xclient.data.l[0] = False;
              XSendEvent(dpy, ROOT, True, EnterWindowMask, &ev);
            }
	  return;
	}
      if ((window->win_attrs.window_style & NSMiniWindowMask) != 0)
	{
	  return;
	}
    }

  NSDebugLLog(@"XGTrace", @"DPSorderwindow: %d : %d : %d",op,otherWin,winNum);
  level = window->win_attrs.window_level;
  if (otherWin > 0)
    {
      other = WINDOW_WITH_TAG(otherWin);
      if (other)
	level = other->win_attrs.window_level;
    }
  else if (otherWin == 0 && op == NSWindowAbove)
    {
      /* Don't let the window go in front of the current key/main window.  */
      /* FIXME: Don't know how to get the current main window.  */
      Window keywin;
      int revert, status;
      status = XGetInputFocus(dpy, &keywin, &revert);
      other = NULL;
      if (status == True)
	{
	  /* Alloc a temporary window structure */
	  other = GSAutoreleasedBuffer(sizeof(gswindow_device_t));
	  other->ident = keywin;
	  op = NSWindowBelow;
	}
    }
  else
    {
      other = NULL;
    }
  [self setwindowlevel: level : winNum];

  /*
   * When we are ordering a window in, we must ensure that the position
   * and size hints are set for the window - the window could have been
   * moved or resized by the window manager before it was ordered out,
   * in which case, we will have been notified of the new position, but
   * will not yet have updated the window hints, so if the window manager
   * looks at the existing hints when re-mapping the window it will
   * place the window in an old location.
   * We also set other hints and protocols supported by the window.
   */
  if (op != NSWindowOut && window->map_state != IsViewable)
    {
      XMoveWindow(dpy, window->ident, window->siz_hints.x,
      	window->siz_hints.y);
      setNormalHints(dpy, window);
      /* Set this to ignore any take focus events for this window */
      window->ignore_take_focus = YES;
    }

  switch (op)
    {
      case NSWindowBelow:
        if (other != 0)
	  {
	    XWindowChanges chg;
	    chg.sibling = other->ident;
	    chg.stack_mode = Below;
	    XReconfigureWMWindow(dpy, window->ident, window->screen_id,
	      CWSibling|CWStackMode, &chg);
	  }
	else
	  {
	    XWindowChanges chg;
	    chg.stack_mode = Below;
	    XReconfigureWMWindow(dpy, window->ident, window->screen_id,
	      CWStackMode, &chg);
	  }
	XMapWindow(dpy, window->ident);
	break;

      case NSWindowAbove:
        if (other != 0)
	  {
	    XWindowChanges chg;
	    chg.sibling = other->ident;
	    chg.stack_mode = Above;
	    XReconfigureWMWindow(dpy, window->ident, window->screen_id,
	      CWSibling|CWStackMode, &chg);
	  }
	else
	  {
	    XWindowChanges chg;
	    chg.stack_mode = Above;
	    XReconfigureWMWindow(dpy, window->ident, window->screen_id,
	      CWStackMode, &chg);
	  }
	XMapWindow(dpy, window->ident);
	break;

      case NSWindowOut:
        XWithdrawWindow (dpy, window->ident, window->screen_id);
	break;
    }
  /*
   * When we are ordering a window in, we must ensure that the position
   * and size hints are set for the window - the window could have been
   * moved or resized by the window manager before it was ordered out,
   * in which case, we will have been notified of the new position, but
   * will not yet have updated the window hints, so if the window manager
   * looks at the existing hints when re-mapping the window it will
   * place the window in an old location.
   */
  if (op != NSWindowOut && window->map_state != IsViewable)
    {
      XMoveWindow(dpy, window->ident, window->siz_hints.x,
	window->siz_hints.y);
      setNormalHints(dpy, window);
      /*
       * Do we need to setup drag types when the window is mapped or will
       * they work on the set up before mapping?
       *
       * [self _resetDragTypesForWindow: GSWindowWithNumber(window->number)];
       */
      if ((window->win_attrs.window_level != NSNormalWindowLevel) ||
	  ((window->win_attrs.window_style &
	    (NSIconWindowMask|NSMiniWindowMask)) != 0))
        {
	  /*
	   * Make any window which assumes the desktop level act as the
	   * background.
	   */
	  if (window->win_attrs.window_level == NSDesktopWindowLevel)
	    {
	      [self _sendRoot: window->root
		    type: generic._NET_WM_STATE_ATOM
		    window: window->ident
		    data0: _NET_WM_STATE_ADD
		    data1: generic._NET_WM_STATE_SKIP_TASKBAR_ATOM
		    data2: generic._NET_WM_STATE_STICKY_ATOM
		    data3: 1];
	    }
	  else
	    {
	      BOOL sticky = NO;
	      NSUserDefaults *defs = [NSUserDefaults standardUserDefaults];

	      [self _sendRoot: window->root
		    type: generic._NET_WM_STATE_ATOM
		    window: window->ident
		    data0: _NET_WM_STATE_ADD
		    data1: generic._NET_WM_STATE_SKIP_TASKBAR_ATOM
		    data2: generic._NET_WM_STATE_SKIP_PAGER_ATOM
                    data3: 1];

	      if ((window->win_attrs.window_style & NSIconWindowMask) != 0)
		{
		  sticky = [defs boolForKey: @"GSStickyAppIcons"];
		}
	      else if ((window->win_attrs.window_style & NSMiniWindowMask) != 0)
		{
		  sticky = [defs boolForKey: @"GSStickyMiniWindows"];
		}
	      if (sticky == YES)
		{
		  [self _sendRoot: window->root
			     type: generic._NET_WM_STATE_ATOM
			   window: window->ident
			    data0: _NET_WM_STATE_ADD
			    data1: generic._NET_WM_STATE_STICKY_ATOM
			    data2: 0
			    data3: 1];
		}
	    }
	}

      if (window->win_attrs.window_level == NSModalPanelWindowLevel)
        {
          [self _sendRoot: window->root
                     type: generic._NET_WM_STATE_ATOM
                   window: window->ident
                    data0: _NET_WM_STATE_ADD
                    data1: generic._NET_WM_STATE_MODAL_ATOM
                    data2: 0
                    data3: 1];
        }
    }
  XFlush(dpy);
}

#define ALPHA_THRESHOLD 158

/* Restrict the displayed part of the window to the given image.
   This only yields usefull results if the window is borderless and
   displays the image itself */
- (void) restrictWindow: (int)win toImage: (NSImage*)image
{
  gswindow_device_t	*window;
  Pixmap pixmap = 0;

  window = WINDOW_WITH_TAG(win);
  if (win == 0 || window == NULL)
    {
      NSLog(@"Invalidparam: Restricting invalid window %d", win);
      return;
    }

#ifdef HAVE_XSHAPE
  if ([[image backgroundColor] alphaComponent] * 256 <= ALPHA_THRESHOLD)
    {
      // The mask computed here is only correct for unscaled images.
      NSBitmapImageRep *rep = getStandardBitmap(image);

      if (rep != nil)
        {
	    if ([rep samplesPerPixel] == 4)
	      {
		pixmap = alphaMaskForImage(dpy, GET_XDRAWABLE(window),
                                           [rep bitmapData],
                                           [rep pixelsWide], [rep pixelsHigh],
                                           [rep samplesPerPixel],
                                           ALPHA_THRESHOLD);
	      }
	}
    }

  XShapeCombineMask(dpy, window->ident, ShapeBounding, 0, 0,
		    pixmap, ShapeSet);

  if (pixmap)
    {
      XFreePixmap(dpy, pixmap);
    }
#endif
}

/* This method is a fast implementation of move that only works
   correctly for borderless windows. Use with caution. */
- (void) movewindow: (NSPoint)loc : (int)win
{
  gswindow_device_t	*window;

  window = WINDOW_WITH_TAG(win);
  if (win == 0 || window == NULL)
    {
      NSLog(@"Invalidparam: Moving invalid window %d", win);
      return;
    }

  window->siz_hints.x = (int)loc.x;
  window->siz_hints.y = (int)(xScreenSize.height
                              - loc.y - window->siz_hints.height);
  XMoveWindow (dpy, window->ident, window->siz_hints.x, window->siz_hints.y);
  setNormalHints(dpy, window);
}

- (void) placewindow: (NSRect)rect : (int)win
{
  NSEvent *e;
  NSRect xFrame;
  NSRect xHint;
  gswindow_device_t *window;
  NSWindow *nswin;
  BOOL resize = NO;
  BOOL move = NO;

  window = WINDOW_WITH_TAG(win);
  if (win == 0 || window == NULL)
    {
      NSLog(@"Invalidparam: Placing invalid window %d", win);
      return;
    }

  NSDebugLLog(@"XGTrace", @"DPSplacewindow: %@ : %d", NSStringFromRect(rect),
	      win);

  xFrame = [self _OSFrameToXFrame: rect for: window];
  if (NSEqualRects(rect, window->osframe) == YES
      && NSEqualRects(xFrame, window->xframe) == YES)
    {
      return;
    }

  if (NSEqualSizes(rect.size, window->osframe.size) == NO)
    {
      resize = YES;
      move = YES;
    }
  else if (NSEqualPoints(rect.origin, window->osframe.origin) == NO
           || NSEqualPoints(xFrame.origin, window->xframe.origin) == NO)
    {
      move = YES;
    }

  // Cache OpenStep window frame for future comparison
  window->osframe = rect;

  /* Temporarily remove minimum and maximum window size hints to make
   * the window resizable programatically.
   */
  if (window->siz_hints.flags & (PMinSize | PMaxSize))
    {
      long flags = window->siz_hints.flags;

      window->siz_hints.flags &= ~(PMinSize | PMaxSize);
      XSetWMNormalHints(dpy, window->ident, &window->siz_hints);
      window->siz_hints.flags = flags;
    }

  xHint = [self _XFrameToXHints: xFrame for: window];
  window->siz_hints.width = (int)xHint.size.width;
  window->siz_hints.height = (int)xHint.size.height;
  window->siz_hints.x = (int)xHint.origin.x;
  window->siz_hints.y = (int)xHint.origin.y;

  NSDebugLLog(@"Moving", @"Place %lu - o:%@, x:%@", window->number,
    NSStringFromRect(rect), NSStringFromRect(xFrame));
  XMoveResizeWindow (dpy, window->ident,
    window->siz_hints.x, window->siz_hints.y,
    window->siz_hints.width, window->siz_hints.height);

  /* Update xframe right away. We optimistically assume that we'll get the
  frame we asked for. If we're right, -gui can update/redraw right away,
  and we don't have to do anything when the ConfigureNotify arrives later.
  If we're wrong, the ConfigureNotify will have the exact coordinates, and
  at that point, we'll send new GSAppKitWindow* events to -gui. */
  window->xframe = xFrame;

  /* Update the hints.  Note that we do this _after_ updating xframe since
     the hint setting code needs the new xframe to work around problems
     with min/max sizes and resizability in some window managers.  */
  setNormalHints(dpy, window);

  nswin = GSWindowWithNumber(win);
  if (resize == YES)
    {
      NSDebugLLog(@"Moving", @"Fake size %lu - %@", window->number,
	NSStringFromSize(rect.size));
      e = [NSEvent otherEventWithType: NSAppKitDefined
			     location: rect.origin
			modifierFlags: 0
			    timestamp: 0
			 windowNumber: win
			      context: GSCurrentContext()
			      subtype: GSAppKitWindowResized
				data1: rect.size.width
				data2: rect.size.height];
      [nswin sendEvent: e];
    }
  else if (move == YES)
    {
      NSDebugLLog(@"Moving", @"Fake move %lu - %@", window->number,
	NSStringFromPoint(rect.origin));
      e = [NSEvent otherEventWithType: NSAppKitDefined
			     location: NSZeroPoint
			modifierFlags: 0
			    timestamp: 0
			 windowNumber: win
			      context: GSCurrentContext()
			      subtype: GSAppKitWindowMoved
				data1: rect.origin.x
				data2: rect.origin.y];
      [nswin sendEvent: e];
    }
}

- (BOOL) findwindow: (NSPoint)loc : (int) op : (int) otherWin : (NSPoint *)floc
: (int*) winFound
{
  return NO;
}

- (NSRect) windowbounds: (int)win
{
  gswindow_device_t *window;
  int screenHeight;
  NSRect rect;
  int x, y;
  unsigned int width, height;

  window = WINDOW_WITH_TAG(win);
  if (!window)
    return NSZeroRect;

  NSDebugLLog(@"XGTrace", @"DPScurrentwindowbounds: %d", win);

  // get the current xframe of the window
  XGetGeometry(dpy, window->ident, &window->root,
    &x, &y, &width, &height, &window->border, &window->depth);
  window->xframe = NSMakeRect(x, y, width, height);

  screenHeight = [self boundsForScreen: window->monitor_id].size.height;
  rect = window->xframe;
  rect.origin.y = screenHeight - NSMaxY(window->xframe);
  return rect;
}

- (void) setwindowlevel: (int)level : (int)win
{
  gswindow_device_t *window;

  window = WINDOW_WITH_TAG(win);
  if (!window)
    return;

  NSDebugLLog(@"XGTrace", @"DPSsetwindowlevel: %d : %d", level, win);
  if ((int)(window->win_attrs.window_level) != level
    || (window->win_attrs.flags & GSWindowLevelAttr) == 0)
    {
      window->win_attrs.flags |= GSWindowLevelAttr;
      window->win_attrs.window_level = level;

      // send WindowMaker WM window style hints
      // Always send GNUstepWMAttributes
      /*
       * First change the window properties so that, if the window
       * is not mapped, we have stored the required info for when
       * the WM maps it.
       */
/* Warning ... X-bug .. when we specify 32bit data X actually expects data
 * of type 'long' or 'unsigned long' even on machines where those types
 * hold 64bit values.
 */
      XChangeProperty(dpy, window->ident,
                      generic._GNUSTEP_WM_ATTR_ATOM, generic._GNUSTEP_WM_ATTR_ATOM,
                      32, PropModeReplace, (unsigned char *)&window->win_attrs,
                      sizeof(GNUstepWMAttributes)/sizeof(CARD32));
      /*
       * Now send a message for rapid handling.
       */
      [self _sendRoot: window->root
            type: generic._GNUSTEP_WM_ATTR_ATOM
            window: window->ident
            data0: GSWindowLevelAttr
            data1: window->win_attrs.window_level
            data2: 0
            data3: 0];

      if ((generic.wm & XGWM_EWMH) != 0)
        {
          int len;
          long data[2];
          BOOL skipTaskbar = NO;

          data[0] = generic._NET_WM_WINDOW_TYPE_NORMAL_ATOM;
          data[1] = None;
          len = 1;

          if (level == NSModalPanelWindowLevel)
            {
              data[0] = generic._NET_WM_WINDOW_TYPE_NORMAL_ATOM;
              skipTaskbar = YES;
            }
          else if (level == NSMainMenuWindowLevel)
            {
              data[0] = generic._NET_WM_WINDOW_TYPE_DOCK_ATOM;
              skipTaskbar = YES;
            }
          else if (level == NSSubmenuWindowLevel
                   || level == NSTornOffMenuWindowLevel)
            {
              data[0] = generic._NET_WM_WINDOW_TYPE_MENU_ATOM;
              skipTaskbar = YES;
            }
          else if (level == NSFloatingWindowLevel)
            {
              data[0] = generic._NET_WM_WINDOW_TYPE_UTILITY_ATOM;
              skipTaskbar = YES;
            }
          else if (level == NSDockWindowLevel
                   || level == NSStatusWindowLevel)
            {
              data[0] =generic._NET_WM_WINDOW_TYPE_DOCK_ATOM;
              skipTaskbar = YES;
            }
          // Does this belong into a different category?
          else if (level == NSPopUpMenuWindowLevel)
            {
                NSWindow *nswin = GSWindowWithNumber(window->number);

                if ([[nswin className] isEqual: @"GSTTPanel"])
                  {
                    data[0] = generic._NET_WM_WINDOW_TYPE_TOOLTIP_ATOM;
                  }
                else
                  {
//              data[0] = generic._NET_WM_WINDOW_TYPE_POPUP_MENU_ATOM;
                    data[0] = generic._NET_WM_WINDOW_TYPE_DIALOG_ATOM;
                  }
              skipTaskbar = YES;
            }
          else if (level == NSDesktopWindowLevel)
            {
              data[0] = generic._NET_WM_WINDOW_TYPE_DESKTOP_ATOM;
              skipTaskbar = YES;
            }

/* Warning ... X-bug .. when we specify 32bit data X actually expects data
 * of type 'long' or 'unsigned long' even on machines where those types
 * hold 64bit values.
 */
          XChangeProperty(dpy, window->ident, generic._NET_WM_WINDOW_TYPE_ATOM,
                          XA_ATOM, 32, PropModeReplace,
                          (unsigned char *)&data, len);

/*
 * Change _NET_WM_STATE based on window level.
 * This should be based on real window type (NSMenu, NSPanel, etc).
 * This feature is only needed for window managers that cannot properly
 * handle the window type set above.
 */
         if (skipTaskbar)
            {
              [self _sendRoot: window->root
                    type: generic._NET_WM_STATE_ATOM
                    window: window->ident
                    data0: _NET_WM_STATE_ADD
                    data1: generic._NET_WM_STATE_SKIP_TASKBAR_ATOM
                    data2: generic._NET_WM_STATE_SKIP_PAGER_ATOM
                    data3: 1];
            }
          else
            {
              [self _sendRoot: window->root
                    type: generic._NET_WM_STATE_ATOM
                    window: window->ident
                    data0: _NET_WM_STATE_REMOVE
                    data1: generic._NET_WM_STATE_SKIP_TASKBAR_ATOM
                    data2: generic._NET_WM_STATE_SKIP_PAGER_ATOM
                    data3: 1];
            }
       }
      else if ((generic.wm & XGWM_GNOME) != 0)
        {
          long flag = WIN_LAYER_NORMAL;

          if (level == NSDesktopWindowLevel)
            flag = WIN_LAYER_DESKTOP;
          else if (level == NSSubmenuWindowLevel
                   || level == NSFloatingWindowLevel
                   || level == NSTornOffMenuWindowLevel)
            flag = WIN_LAYER_ONTOP;
          else if (level == NSMainMenuWindowLevel)
            flag = WIN_LAYER_MENU;
          else if (level == NSDockWindowLevel
                   || level == NSStatusWindowLevel)
            flag = WIN_LAYER_DOCK;
          else if (level == NSModalPanelWindowLevel
                   || level == NSPopUpMenuWindowLevel)
            flag = WIN_LAYER_ONTOP;
          else if (level == NSScreenSaverWindowLevel)
            flag = WIN_LAYER_ABOVE_DOCK;

          XChangeProperty(dpy, window->ident, generic._WIN_LAYER_ATOM,
                          XA_CARDINAL, 32, PropModeReplace,
                          (unsigned char *)&flag, 1);

          [self _sendRoot: window->root
                type: generic._WIN_LAYER_ATOM
                window: window->ident
                data0: flag
                data1: 0
                data2: 0
                data3: 0];
        }
    }
}

- (int) windowlevel: (int)win
{
  gswindow_device_t *window;

  window = WINDOW_WITH_TAG(win);
  /*
   * If we have previously set a level for this window - return the value set.
   */
  if (window != 0 && (window->win_attrs.flags & GSWindowLevelAttr))
    return window->win_attrs.window_level;
  return 0;
}

- (NSArray *) windowlist
{
  gswindow_device_t *rootWindow;
  Window *windowOrder;
  gswindow_device_t *tmp;
  NSMutableArray *ret;
  int c;

  rootWindow = [self _rootWindow];

  windowOrder = (Window *)PropGetCheckProperty(dpy, rootWindow->ident,
                                               generic._NET_CLIENT_LIST_STACKING_ATOM,
                                               XA_WINDOW, 32, -1, &c);
  if (windowOrder == NULL || !c)
    {
      return [super windowlist];
    }

  ret = [NSMutableArray array];

  while (c-- > 0)
    {
      tmp = [[self class] _windowForXWindow: windowOrder[c]];
      /* CLIENT_LIST_STACKING returns all windows on the server, we're only
       * interested in the ones which are ours. */
      if (tmp)
        {
          [ret addObject: [NSNumber numberWithInt: tmp->number]];
        }
    }

  XFree(windowOrder);
  return ret;
}

- (int) windowdepth: (int)win
{
  gswindow_device_t *window;

  window = WINDOW_WITH_TAG(win);
  if (!window)
    return 0;

  return window->depth;
}

- (void) setmaxsize: (NSSize)size : (int)win
{
  gswindow_device_t	*window;
  NSRect		r;

  window = WINDOW_WITH_TAG(win);
  if (window == 0)
    {
      return;
    }
  r = NSMakeRect(0, 0, size.width, size.height);
  r = [self _OSFrameToXFrame: r for: window];
  window->siz_hints.flags |= PMaxSize;
  window->siz_hints.max_width = r.size.width;
  window->siz_hints.max_height = r.size.height;
  setNormalHints(dpy, window);
}

- (void) setminsize: (NSSize)size : (int)win
{
  gswindow_device_t	*window;
  NSRect		r;

  window = WINDOW_WITH_TAG(win);
  if (window == 0)
    {
      return;
    }
  r = NSMakeRect(0, 0, size.width, size.height);
  r = [self _OSFrameToXFrame: r for: window];
  window->siz_hints.flags |= PMinSize;
  window->siz_hints.min_width = r.size.width;
  window->siz_hints.min_height = r.size.height;
  setNormalHints(dpy, window);
}

- (void) setresizeincrements: (NSSize)size : (int)win
{
  gswindow_device_t *window;

  window = WINDOW_WITH_TAG(win);
  if (window == 0)
    {
      return;
    }
  window->siz_hints.flags |= PResizeInc;
  window->siz_hints.width_inc = size.width;
  window->siz_hints.height_inc = size.height;
  setNormalHints(dpy, window);
}

// process expose event
- (void) _addExposedRectangle: (XRectangle)rectangle : (int)win : (BOOL) ignoreBacking
{
  gswindow_device_t *window;

  window = WINDOW_WITH_TAG(win);
  if (!window)
    return;

  if (!ignoreBacking && window->type != NSBackingStoreNonretained)
    {
      XGCValues values;
      unsigned long valuemask;

      // window has a backing store so just copy the exposed rect from the
      // pixmap to the X window

      NSDebugLLog (@"NSWindow", @"copy exposed area ((%d, %d), (%d, %d))",
		  rectangle.x, rectangle.y, rectangle.width, rectangle.height);

      values.function = GXcopy;
      values.plane_mask = AllPlanes;
      values.clip_mask = None;
      values.foreground = window->xwn_attrs.background_pixel;
      valuemask = (GCFunction | GCPlaneMask | GCClipMask | GCForeground);
      XChangeGC(dpy, window->gc, valuemask, &values);
      [[self class] waitAllContexts];
      if ((window->gdriverProtocol & GDriverHandlesExpose))
	{
	  /* Temporary protocol until we standardize the backing buffer */
	  NSRect rect = NSMakeRect(rectangle.x, rectangle.y,
				   rectangle.width, rectangle.height);
	  [[GSCurrentContext() class] handleExposeRect: rect
			     forDriver: window->gdriver];
	}
      else
        {
	  XCopyArea (dpy, window->buffer, window->ident, window->gc,
		   rectangle.x, rectangle.y, rectangle.width, rectangle.height,
		   rectangle.x, rectangle.y);
	}
    }
  else
    {
      NSRect	rect;

      // no backing store, so keep a list of exposed rects to be
      // processed in the _processExposedRectangles method
      // Add the rectangle to the region used in -_processExposedRectangles
      // to set the clipping path.
      XUnionRectWithRegion (&rectangle, window->region, window->region);

      // Transform the rectangle's coordinates to OS coordinates and add
      // this new rectangle to the list of exposed rectangles.
      {
 	rect = [self _XWinRectToOSWinRect: NSMakeRect(
	  rectangle.x, rectangle.y, rectangle.width, rectangle.height)
	  for: window];
	[window->exposedRects addObject: [NSValue valueWithRect: rect]];
      }
    }
}

- (void) flushwindowrect: (NSRect)rect : (int)win
{
  int xi, yi, width, height;
  XGCValues values;
  unsigned long valuemask;
  gswindow_device_t *window;
  float	l, r, t, b;

  window = WINDOW_WITH_TAG(win);
  if (win == 0 || window == NULL)
    {
      NSLog(@"Invalidparam: Placing invalid window %d", win);
      return;
    }

  NSDebugLLog(@"XGFlush", @"DPSflushwindowrect: %@ : %d",
	      NSStringFromRect(rect), win);
  if (window->type == NSBackingStoreNonretained)
    {
      XFlush(dpy);
      return;
    }

  values.function = GXcopy;
  values.plane_mask = AllPlanes;
  values.clip_mask = None;
  valuemask = (GCFunction | GCPlaneMask | GCClipMask);
  XChangeGC(dpy, window->gc, valuemask, &values);

  [self styleoffsets: &l : &r : &t : &b
		    : window->win_attrs.window_style : window->ident];
  xi = rect.origin.x = NSMinX(rect) - l;
  yi = rect.origin.y = NSHeight(window->xframe) + b - NSMaxY(rect);
  width = NSWidth(rect);
  height = NSHeight(rect);

  if (width > 0 || height > 0)
    {
      [[self class] waitAllContexts];
      if ((window->gdriverProtocol & GDriverHandlesBacking))
	{
	  NSDebugLLog (@"XGFlush",
	       @"expose X rect ((%d, %d), (%d, %d))", xi, yi, width, height);
	  /* Temporary protocol until we standardize the backing buffer */
	  [[GSCurrentContext() class] handleExposeRect: rect
			     forDriver: window->gdriver];
	}
      else
        {
	  NSDebugLLog (@"XGFlush",
	       @"copy X rect ((%d, %d), (%d, %d))", xi, yi, width, height);

	  XCopyArea (dpy, window->buffer, window->ident, window->gc,
		     xi, yi, width, height, xi, yi);
	}
    }

#ifdef HAVE_X11_EXTENSIONS_SYNC_H
  if (window->net_wm_sync_request_counter_value_low != 0
      || window->net_wm_sync_request_counter_value_high != 0)
    {
      XSyncValue value;
      XSyncIntsToValue(&value,
		       window->net_wm_sync_request_counter_value_low,
		       window->net_wm_sync_request_counter_value_high);
      XSyncSetCounter(dpy, window->net_wm_sync_request_counter, value);
      window->net_wm_sync_request_counter_value_low = 0;
      window->net_wm_sync_request_counter_value_high = 0;
    }
#endif

  XFlush(dpy);
}

// handle X expose events
- (void) _processExposedRectangles: (int)win
{
  int n;
  gswindow_device_t *window;
  NSWindow *gui_win;

  window = WINDOW_WITH_TAG(win);
  if (!window)
    return;

  // Set the clipping path to the exposed rectangles
  // so that further drawing will not affect the non-exposed region
  XSetRegion (dpy, window->gc, window->region);

  // We should determine the views that need to be redisplayed. Until we
  // fully support scalation and rotation of views redisplay everything.
  // FIXME: It seems wierd to trigger a front-end method from here...

  // We simply invalidate the
  // corresponding rectangle of the top most view of the window.

  gui_win = GSWindowWithNumber(win);

  n = [window->exposedRects count];
  if (n > 0)
    {
      NSView *v;
      NSValue *val[n];
      int i;

      v = [[gui_win contentView] superview];

      [window->exposedRects getObjects: val];
      for (i = 0; i < n; ++i)
	{
	  NSRect rect = [val[i] rectValue];
	  [v setNeedsDisplayInRect: rect];
	}
    }

  // Restore the exposed rectangles and the region
  [window->exposedRects removeAllObjects];
  XDestroyRegion (window->region);
  window->region = XCreateRegion();
  XSetClipMask (dpy, window->gc, None);
}

- (BOOL) capturemouse: (int)win
{
  int ret;
  gswindow_device_t *window;

  window = WINDOW_WITH_TAG(win);
  if (!window)
    return NO;

  ret = XGrabPointer(dpy, window->ident, False,
		     PointerMotionMask | ButtonReleaseMask | ButtonPressMask,
		     GrabModeAsync, GrabModeAsync, None, None, [self lastTime]);

  if (ret != GrabSuccess)
    NSDebugLLog(@"XGTrace", @"Failed to grab pointer %d\n", win);
  else
    {
      grab_window = window;
      NSDebugLLog(@"XGTrace", @"Grabbed pointer %d\n", win);
    }
  return (ret == GrabSuccess) ? YES : NO;
}

- (void) releasemouse
{
  NSDebugLLog(@"XGTrace", @"Released pointer\n");
  XUngrabPointer(dpy, [self lastTime]);
  grab_window = NULL;
}

- (void) setMouseLocation: (NSPoint)mouseLocation onScreen: (int)aScreen
{
  int height;
  int destX, destY;

  height = [self boundsForScreen: aScreen].size.height;
  destY = height - mouseLocation.y;
  destX = mouseLocation.x;

  XWarpPointer(dpy, None, [self xDisplayRootWindow],
               0, 0, 0, 0, destX, destY);
}

- (void) setinputfocus: (int)win
{
  gswindow_device_t *window = WINDOW_WITH_TAG(win);

  if (win == 0 || window == 0)
    {
      NSDebugLLog(@"Focus", @"Setting focus to unknown win %d", win);
      return;
    }

  NSDebugLLog(@"XGTrace", @"DPSsetinputfocus: %d", win);
  /*
   * If we have an outstanding request to set focus to this window,
   * we don't want to do it again.
   */
  if (win == generic.desiredFocusWindow && generic.focusRequestNumber != 0)
    {
      NSDebugLLog(@"Focus", @"Focus already set on %lu", window->number);
      return;
    }

  if ((generic.wm & XGWM_EWMH) != 0)
    {
      Time last = [self lastTime];

      NSDebugLLog(@"Focus", @"Setting user time for %lu to %lu", window->ident, last);
      XChangeProperty(dpy, window->ident, generic._NET_WM_USER_TIME_ATOM, XA_CARDINAL, 32,
                      PropModeReplace, (unsigned char *)&last, 1);
    }

  NSDebugLLog(@"Focus", @"Setting focus to %lu", window->number);
  generic.desiredFocusWindow = win;
  generic.focusRequestNumber = XNextRequest(dpy);
  XSetInputFocus(dpy, window->ident, RevertToParent, [self lastTime]);
  [inputServer ximFocusICWindow: window];
}

/*
 * Instruct window manager that the specified window is 'key', 'main', or
 * just a normal window.
 */
- (void) setinputstate: (int)st : (int)win
{
  if (!handlesWindowDecorations)
    return;

  NSDebugLLog(@"XGTrace", @"DPSsetinputstate: %d : %d", st, win);
  if ((generic.wm & XGWM_WINDOWMAKER) != 0)
    {
      gswindow_device_t *window = WINDOW_WITH_TAG(win);

      if (win == 0 || window == 0)
        {
          return;
        }

      [self _sendRoot: window->root
            type: generic._GNUSTEP_TITLEBAR_STATE_ATOM
            window: window->ident
            data0: st
            data1: 0
            data2: 0
            data3: 0];
    }
#if 0
  else if ((generic.wm & XGWM_EWMH) != 0)
    {
      if ((st == GSTitleBarKey) || (st == GSTitleBarMain))
        {
          gswindow_device_t *window = WINDOW_WITH_TAG(win);

          if (win == 0 || window == 0)
            {
              return;
            }

          /*
           * Work around "focus stealing prevention" by first asking for focus
           * before we try to take it.
           */
          [self _sendRoot: window->root
                type: generic._NET_ACTIVE_WINDOW_ATOM
                window: window->ident
                data0: 1
                data1: [self lastTime]
                data2: 0
                data3: 0];
        }
    }
#endif
}

/** Sets the transparancy value for the whole window */
- (void) setalpha: (float)alpha : (int) win
{
  gswindow_device_t *window = WINDOW_WITH_TAG(win);

  if (win == 0 || window == 0)
    {
      NSDebugLLog(@"XGTrace", @"Setting alpha to unknown win %d", win);
      return;
    }

  NSDebugLLog(@"XGTrace", @"setalpha: %d", win);

  if (alpha == 1.0)
    {
      XDeleteProperty(window->display, window->ident, generic._NET_WM_WINDOW_OPACITY_ATOM);
    }
  else
    {
      unsigned int opacity;

      opacity = (unsigned int)(alpha * 0xffffffffU);
      XChangeProperty(window->display, window->ident, generic._NET_WM_WINDOW_OPACITY_ATOM,
		      XA_CARDINAL, 32, PropModeReplace,
		      (unsigned char*)&opacity, 1L);
      if (window->parent != window->root)
        {
          XChangeProperty(window->display, window->parent, generic._NET_WM_WINDOW_OPACITY_ATOM,
                          XA_CARDINAL, 32, PropModeReplace,
                          (unsigned char*)&opacity, 1);
        }

      // GDK uses an event to set opacity, but most window manager still wait
      // for property changes. What is the official stanard?
    }
}

- (float) getAlpha: (int)win
{
  gswindow_device_t *window = WINDOW_WITH_TAG(win);
  int c;
  unsigned int *num;
  float alpha = 0.0;

  if (win == 0 || window == 0)
    {
      NSDebugLLog(@"XGTrace", @"Setting alpha to unknown win %d", win);
      return alpha;
    }

  num = (unsigned int*)PropGetCheckProperty(dpy, window->ident,
                                            generic._NET_WM_WINDOW_OPACITY_ATOM, XA_CARDINAL,
                                            32, 1, &c);

  if (num)
    {
      if (*num)
        alpha = (float)*num / 0xffffffffU;
      XFree(num);
    }

  return alpha;
}

- (void *) serverDevice
{
  return dpy;
}

- (void *) windowDevice: (int)win
{
  Window ptrloc;
  gswindow_device_t *window;

  window = WINDOW_WITH_TAG(win);
  if (window != NULL)
    ptrloc = window->ident;
  else
    ptrloc = 0;
  return (void *)ptrloc;
}

/* Cursor Ops */

static char xgps_blank_cursor_bits [] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static Cursor xgps_blank_cursor = None;
static BOOL   cursor_hidden = NO;

- (Cursor) _blankCursor
{
  if (xgps_blank_cursor == None)
    {
      Pixmap shape, mask;
      XColor black, white;
      Drawable drw = [self xDisplayRootWindow];

      shape = XCreatePixmapFromBitmapData(dpy, drw,
					  xgps_blank_cursor_bits,
					  16, 16, 1, 0, 1);
      mask = XCreatePixmapFromBitmapData(dpy, drw,
					 xgps_blank_cursor_bits,
					 16, 16, 1, 0, 1);
      black.red = black.green = black.blue = 0;
      black = [self xColorFromColor: black];
      white.red = white.green = white.blue = 65535;
      white = [self xColorFromColor: white];

      xgps_blank_cursor = XCreatePixmapCursor(dpy, shape, mask,
					      &white, &black,  0, 0);
      XFreePixmap(dpy, shape);
      XFreePixmap(dpy, mask);
    }
  return xgps_blank_cursor;
}

/*
  set the cursor for a newly created window.
*/

- (void) _initializeCursorForXWindow: (Window) win
{
  if (cursor_hidden)
    {
      XDefineCursor (dpy, win, [self _blankCursor]);
    }
  else
    {
      Cursor cid = (Cursor)[[NSCursor currentCursor] _cid];

      XDefineCursor (dpy, win, cid);
    }
}


/*
  set cursor on all XWindows we own.  if `set' is NO
  the cursor is unset on all windows.
  Normally the cursor `c' correspond to the [NSCursor currentCursor]
  The only exception should be when the cursor is hidden.
  In that case `c' will be a blank cursor.
*/

- (void) _DPSsetcursor: (Cursor)c : (BOOL)set
{
  void	*key;
  NSMapEnumerator enumerator;
  gswindow_device_t  *d;
  Window root;

  NSDebugLLog (@"NSCursor", @"_DPSsetcursor: cursor = %lu, set = %d", c, set);

  root = DefaultRootWindow(dpy);
  enumerator = NSEnumerateMapTable (windowmaps);
  while (NSNextMapEnumeratorPair (&enumerator, &key, (void**)&d) == YES)
    {
      Window win = (Window)key;

      if (win == root)
	continue;

      if (set)
        XDefineCursor(dpy, win, c);
      else
        XUndefineCursor(dpy, win);
    }
}

#if !HAVE_XCURSOR

Pixmap
xgps_cursor_image(Display *xdpy, Drawable draw, const unsigned char *data,
		  int w, int h, int colors, XColor *fg, XColor *bg)
{
  int j, i, min, max;
  Pixmap pix;
  int bitmapSize = ((w + 7) >> 3) * h; // w/8 rounded up multiplied by h
  char *aData = calloc(1, bitmapSize);
  char *cData = aData;

  min = 1 << 16;
  max = 0;
  if (colors == 4 || colors == 3)
    {
      int k;
      for (j = 0; j < h; j++)
	{
	  k = 0;
	  for (i = 0; i < w; i++, k++)
	    {
	      /* colors is in the range 0..65535
		 and value is the percieved brightness, obtained by
		 averaging 0.3 red + 0.59 green + 0.11 blue
	      */
	      int color = ((77 * data[0]) + (151 * data[1]) + (28 * data[2]));

	      if (k > 7)
		{
		  cData++;
		  k = 0;
		}
	      if (color > (1 << 15))
		{
		  *cData |= (0x01 << k);
		}
	      if (color < min)
		{
		  min = color;
		  bg->red = (int)data[0] * 256;
		  bg->green = (int)data[1] * 256;
		  bg->blue = (int)data[2] * 256;
		}
	      else if (color > max)
		{
		  max = color;
		  fg->red = (int)data[0] * 256;
		  fg->green = (int)data[1] * 256;
		  fg->blue = (int)data[2] * 256;
		}
	      data += 3;
	      if (colors == 4)
		{
		  data++;
		}
	    }
	  cData++;
	}
    }
  else
    {
      for (j = 0; j < bitmapSize; j++)
	{
	  if ((unsigned short)((char)*data++) > 128)
	    {
	      *cData |= (0x01 << j);
	    }
	  cData++;
	}
    }

  pix = XCreatePixmapFromBitmapData(xdpy, draw, (char *)aData, w, h,
				    1L, 0L, 1);
  free(aData);
  return pix;
}

#endif

- (void) hidecursor
{
  if (cursor_hidden)
    return;

  [self _DPSsetcursor: [self _blankCursor] : YES];
  cursor_hidden = YES;
}

- (void) showcursor
{
  if (cursor_hidden)
    {
      /* This just resets the cursor to the parent window's cursor.
	 I'm not even sure it's needed */
      [self _DPSsetcursor: None : NO];
      /* Reset the current cursor */
      [[NSCursor currentCursor] set];
    }
  cursor_hidden = NO;
}

- (void) standardcursor: (int)style : (void **)cid
{
  Cursor cursor = None;

  switch (style)
    {
    case GSArrowCursor:
      cursor = XCreateFontCursor(dpy, XC_left_ptr);
      break;
    case GSIBeamCursor:
      cursor = XCreateFontCursor(dpy, XC_xterm);
      break;
    case GSOpenHandCursor:
      cursor = XCreateFontCursor(dpy, XC_hand1);
      break;
    case GSPointingHandCursor:
      cursor = XCreateFontCursor(dpy, XC_hand2);
      break;
    case GSCrosshairCursor:
      cursor = XCreateFontCursor(dpy, XC_crosshair);
      break;
    case GSResizeDownCursor:
      cursor = XCreateFontCursor(dpy, XC_bottom_side);
      break;
    case GSResizeLeftCursor:
      cursor = XCreateFontCursor(dpy, XC_left_side);
      break;
    case GSResizeLeftRightCursor:
      cursor = XCreateFontCursor(dpy, XC_sb_h_double_arrow);
      break;
    case GSResizeRightCursor:
      cursor = XCreateFontCursor(dpy, XC_right_side);
      break;
    case GSResizeUpCursor:
      cursor = XCreateFontCursor(dpy, XC_top_side);
      break;
    case GSResizeUpDownCursor:
      cursor = XCreateFontCursor(dpy, XC_sb_v_double_arrow);
      break;
    default:
      return;
    }
  if (cid)
    *cid = (void *)cursor;
}

- (void) imagecursor: (NSPoint)hotp : (NSImage *)image : (void **)cid
{
  Cursor cursor;
  NSBitmapImageRep *rep;
  int w, h;
  int colors;

  rep = getStandardBitmap(image);
  if (rep == nil)
    {
      /* FIXME: We might create a blank cursor here? */
      NSLog(@"Could not convert cursor bitmap data");
      *cid = NULL;
      return;
    }

  if (hotp.x >= [rep pixelsWide])
    hotp.x = [rep pixelsWide]-1;

  if (hotp.y >= [rep pixelsHigh])
    hotp.y = [rep pixelsHigh]-1;

  w = [rep pixelsWide];
  h = [rep pixelsHigh];
  colors = [rep samplesPerPixel];

  if (w <= 0 || h <= 0)
    {
      *cid = NULL;
      return;
    }

#if HAVE_XCURSOR
  // FIXME: Standardize getStandardBitmap() so it always returns
  // alpha, and document the format.
  if (colors != 4)
    {
      *cid = NULL;
      return;
    }

  {
    XcursorImage *xcursorImage;
    xcursorImage = XcursorImageCreate(w, h);
    xcursorImage->xhot = hotp.x;
    xcursorImage->yhot = hotp.y;

    // Copy the data from the image rep to the Xcursor structure
    swapColors((unsigned char *)xcursorImage->pixels, rep);
    
    cursor = XcursorImageLoadCursor(dpy, xcursorImage);
    XcursorImageDestroy(xcursorImage);
  }
#else // !HAVE_XCURSOR
  {
    Pixmap source, mask;
    unsigned int maxw, maxh;
    XColor fg, bg;
    const unsigned char *data = [rep bitmapData];

    /* FIXME: Handle this better or return an error? */
    XQueryBestCursor(dpy, ROOT, w, h, &maxw, &maxh);
    if ((unsigned int)w > maxw)
      w = maxw;
    if ((unsigned int)h > maxh)
      h = maxh;

    source = xgps_cursor_image(dpy, ROOT, data, w, h, colors, &fg, &bg);
    mask = alphaMaskForImage(dpy, ROOT, data, w, h, colors, ALPHA_THRESHOLD);
    bg = [self xColorFromColor: bg];
    fg = [self xColorFromColor: fg];

    cursor = XCreatePixmapCursor(dpy, source, mask, &fg, &bg,
				 (int)hotp.x, (int)hotp.y);
    XFreePixmap(dpy, source);
    XFreePixmap(dpy, mask);
  }
#endif

  if (cid)
    *cid = (void *)cursor;
}

- (void) recolorcursor: (NSColor *)fg : (NSColor *)bg : (void*) cid
{
  XColor xf, xb;
  Cursor cursor;

  cursor = (Cursor)cid;
  if (cursor == None)
    {
      NSLog(@"Invalidparam: Invalid cursor");
      return;
    }

  fg = [fg colorUsingColorSpaceName: NSDeviceRGBColorSpace];
  bg = [bg colorUsingColorSpaceName: NSDeviceRGBColorSpace];
  if (fg == nil || bg == nil)
    {
      return;
    }

  xf.red   = 65535 * [fg redComponent];
  xf.green = 65535 * [fg greenComponent];
  xf.blue  = 65535 * [fg blueComponent];
  xb.red   = 65535 * [bg redComponent];
  xb.green = 65535 * [bg greenComponent];
  xb.blue  = 65535 * [bg blueComponent];
  xf = [self xColorFromColor: xf];
  xb = [self xColorFromColor: xb];

  XRecolorCursor(dpy, cursor, &xf, &xb);
}

- (void) setcursor: (void*) cid
{
  Cursor cursor;

  cursor = (Cursor)cid;
  if (cursor == None)
    {
      NSLog(@"Invalidparam: Invalid cursor");
      return;
    }

  [self _DPSsetcursor: cursor : YES];
}

- (void) freecursor: (void*) cid
{
  Cursor cursor;

  cursor = (Cursor)cid;
  if (cursor == None)
    {
      NSLog(@"Invalidparam: Invalid cursor");
      return;
    }

  XFreeCursor(dpy, cursor);
}

/*******************************************************************************
 * Screens, monitors
 *******************************************************************************/

static NSWindowDepth
_computeDepth(int class, int bpp)
{
  int		spp = 0;
  int		bitValue = 0;
  int		bps = 0;
  NSWindowDepth	depth = 0;

  switch (class)
    {
      case GrayScale:
      case StaticGray:
	bitValue = _GSGrayBitValue;
	spp = 1;
	break;
      case PseudoColor:
      case StaticColor:
	bitValue = _GSCustomBitValue;
	spp = 1;
	break;
      case DirectColor:
      case TrueColor:
	bitValue = _GSRGBBitValue;
	spp = 3;
	break;
      default:
        NSLog(@"Unknown visual class %d in computeDepth", class);
        return 0;
	break;
    }

  bps = (bpp/spp);
  depth = (bitValue | bps);

  return depth;
}

static NSSize
_screenSize(Display *dpy, int screen)
{
  int          x, y;
  unsigned int width, height, border_width, depth;
  Window       root_window;
  NSSize       scrSize;

  XGetGeometry(dpy, RootWindow(dpy, screen), &root_window,
               &x, &y, &width, &height, &border_width, &depth);
  scrSize = NSMakeSize(width, height);
  
  if (scrSize.height == 0)
    scrSize.height = DisplayHeight(dpy, screen);
  if (scrSize.width == 0)
    scrSize.width = DisplayWidth(dpy, screen);

  return scrSize;
}
  

/* This method assumes that we deal with one X11 screen - `defScreen`.
   Basically it means that we have DISPLAY variable set to `:0.0`.
   Where both digits have artbitrary values, but it defines once on
   every application run.
   AppKit and X11 use the same term "screen" with different meaning:
   AppKit Screen - single monitor/display device.
   X11 Screen - it's a plane where X Server can draw on.
   XRandR - Xorg extension that helps to manipulate monitor layout providing
   X11 Screen. 
   We map XRandR monitors (outputs) to NSScreen. */
- (NSArray *)screenList
{
  xScreenSize = _screenSize(dpy, defScreen);

  monitorsCount = 0;
  if (monitors != NULL) {
    NSZoneFree([self zone], monitors);
    monitors = NULL;
  }
  
#ifdef HAVE_XRANDR
  Window              root = [self xDisplayRootWindow];
  XRRScreenResources *screen_res = XRRGetScreenResources(dpy, root);

  if (screen_res != NULL)
    {
      monitorsCount = screen_res->noutput;
      if (monitorsCount != 0)
        {
          int             i;
          int             mi;
          NSMutableArray *tmpScreens = [NSMutableArray arrayWithCapacity: monitorsCount];
          RROutput        primary_output = XRRGetOutputPrimary(dpy, root);

          monitors = NSZoneMalloc([self zone], monitorsCount * sizeof(MonitorDevice));

          for (i = 0, mi = 0; i < screen_res->noutput; i++)
            {
              XRROutputInfo *output_info;
              
              output_info = XRRGetOutputInfo(dpy, screen_res, screen_res->outputs[i]);
              if (output_info->crtc)
                {
                  XRRCrtcInfo *crtc_info;
                  
                  crtc_info = XRRGetCrtcInfo(dpy, screen_res, output_info->crtc);
                  
                  monitors[mi].screen_id = defScreen;
                  monitors[mi].depth = [self windowDepthForScreen: mi];
                  monitors[mi].resolution = [self resolutionForScreen: defScreen];
                  /* Transform coordinates from Xlib (flipped) to OpenStep (unflippped). 
                     Windows and screens should have the same coordinate system. */
                  monitors[mi].frame =
                    NSMakeRect(crtc_info->x,
                               xScreenSize.height - crtc_info->height - crtc_info->y,
                               crtc_info->width,
                               crtc_info->height);
                  /* Add monitor ID (index in monitors array).
                     Put primary monitor ID at index 0 since NSScreen get this as main 
                     screen if application has no key window. */
                  if (screen_res->outputs[i] == primary_output)
                    {
                      [tmpScreens insertObject: [NSNumber numberWithInt: mi] atIndex: 0];
                    }
                  else
                    {
                      [tmpScreens addObject: [NSNumber numberWithInt: mi]];
                    }
                  XRRFreeCrtcInfo(crtc_info);
                  mi++;
                }
              XRRFreeOutputInfo(output_info);
            }

          monitorsCount = mi;
          if (monitorsCount != 0)
            {
              XRRFreeScreenResources(screen_res);
              return [NSArray arrayWithArray: tmpScreens];
            }
          else
            {
              NSZoneFree([self zone], monitors);
              monitors = NULL;
            }
        }
      XRRFreeScreenResources(screen_res);
    }
#endif

  /* It is assumed that there is always only one screen per application. 
     We only need to know its number and it was saved in _initXContext as
     `defScreen`. */
  monitorsCount = 1;
  monitors = NSZoneMalloc([self zone], sizeof(MonitorDevice));
  monitors[0].screen_id = defScreen;
  monitors[0].depth = [self windowDepthForScreen: 0];
  monitors[0].resolution = [self resolutionForScreen: defScreen];
  monitors[0].frame = NSMakeRect(0, 0, xScreenSize.width, xScreenSize.height);
  return [NSArray arrayWithObject: [NSNumber numberWithInt: defScreen]];
}

// `screen` is a monitor index not X11 screen
- (NSWindowDepth) windowDepthForScreen: (int)screen
{
  Screen	*x_screen;
  int		 class = 0, bpp = 0;

  if (screen < 0 || screen >= monitorsCount)
    {
      return 0;
    }
  
  x_screen = XScreenOfDisplay(dpy, monitors[screen].screen_id);
  
  if (x_screen == NULL)
    {
      return 0;
    }

  bpp = x_screen->root_depth;
  class = x_screen->root_visual->class;

  return _computeDepth(class, bpp);
}

// `screen` is a monitor index not X11 screen
- (const NSWindowDepth *) availableDepthsForScreen: (int)screen
{
  Screen	*x_screen;
  int		 class = 0;
  int		 index = 0;
  int		 ndepths = 0;
  NSZone	*defaultZone = NSDefaultMallocZone();
  NSWindowDepth	*depths = 0;

  if (dpy == NULL || screen < 0 || screen >= monitorsCount)
    {
      return NULL;
    }

  x_screen = XScreenOfDisplay(dpy, monitors[screen].screen_id);
  
  if (x_screen == NULL)
    {
      return NULL;
    }

  // Allocate the memory for the array and fill it in.
  ndepths = x_screen->ndepths;
  class = x_screen->root_visual->class;
  depths = NSZoneMalloc(defaultZone, sizeof(NSWindowDepth)*(ndepths + 1));
  for (index = 0; index < ndepths; index++)
    {
      int depth = x_screen->depths[index].depth;
      depths[index] = _computeDepth(class, depth);
    }
  depths[index] = 0; // terminate with a zero.

  return depths;
}

// `screen_num` is a Xlib screen number.
- (NSSize) resolutionForScreen: (int)screen_num
{
  // NOTE:
  // -gui now trusts the return value of resolutionForScreen:,
  // so if it is not {72, 72} then the entire UI will be scaled.
  //
  // I commented out the implementation below because it may not
  // be safe to use the DPI value we get from the X server.
  // (i.e. I don't know if it will be a "fake" DPI like 72 or 96,
  //  or a real measurement reported from the monitor's firmware
  //  (possibly incorrect?))
  // More research needs to be done.

  return NSMakeSize(72, 72);

  /*
  int res_x, res_y;

  if (screen < 0 || screen_num >= ScreenCount(dpy))
    {
      NSLog(@"Invalidparam: no screen %d", screen);
      return NSMakeSize(0,0);
    }
  // This does not take virtual displays into account!!
  res_x = DisplayWidth(dpy, screen) /
      (DisplayWidthMM(dpy, screen) / 25.4);
  res_y = DisplayHeight(dpy, screen) /
      (DisplayHeightMM(dpy, screen) / 25.4);

  return NSMakeSize(res_x, res_y);
  */
}

// `screen` is a monitor index not X11 screen
- (NSRect) boundsForScreen: (int)screen
{
  NSRect boundsRect = NSZeroRect;
  
  if (screen < 0 || screen >= monitorsCount)
    {
      NSLog(@"Invalidparam: no screen %d", screen);
      return NSZeroRect;
    }
  
  if (monitors != NULL)
    {
      boundsRect = monitors[screen].frame;
    }
  
  return boundsRect;
}

- (NSImage *) iconTileImage
{
  Window win;
  Window *pwin;
  int count;
  unsigned char *tile;
  NSImage *iconTileImage;
  NSBitmapImageRep *imageRep;
  unsigned int width, height;

  if (((generic.wm & XGWM_WINDOWMAKER) == 0)
      || generic.flags.useWindowMakerIcons == NO)
    return [super iconTileImage];

  win = DefaultRootWindow(dpy);
  pwin = (Window *)PropGetCheckProperty(dpy, win, generic._WINDOWMAKER_NOTICEBOARD_ATOM, XA_WINDOW,
					32, -1, &count);
  if (pwin == NULL)
    return [super iconTileImage];

  tile = PropGetCheckProperty(dpy, *pwin, generic._WINDOWMAKER_ICON_TILE_ATOM, generic._RGBA_IMAGE_ATOM,
			      8, -1, &count);
  XFree(pwin);
  if (tile == NULL || count < 4)
    return [super iconTileImage];

  width = (tile[0] << 8) + tile[1];
  height = (tile[2] << 8) + tile[3];

  if (count > 4 + width * height * 4)
    return [super iconTileImage];

  iconTileImage = [[NSImage alloc] init];
  imageRep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes: NULL
                                                     pixelsWide: width
                                                     pixelsHigh: height
                                                  bitsPerSample: 8
                                                samplesPerPixel: 4
                                                       hasAlpha: YES
                                                       isPlanar: NO
                                                 colorSpaceName: NSDeviceRGBColorSpace
                                                    bytesPerRow: width * 4
                                                   bitsPerPixel: 32];
  memcpy([imageRep bitmapData], &tile[4], width * height * 4);
  XFree(tile);
  [iconTileImage addRepresentation:imageRep];
  RELEASE(imageRep);

  return AUTORELEASE(iconTileImage);
}

- (NSSize) iconSize
{
  XIconSize *xiconsize;
  int count_return;
  int status;
  NSSize iconSize;

  status = XGetIconSizes(dpy,
                         DefaultRootWindow(dpy),
			 &xiconsize,
			 &count_return);
  if (status)
    {
      if ((generic.wm & XGWM_WINDOWMAKER) != 0)
        {
	  /* must add 4 pixels for the border which windowmaker leaves out
	     but gnustep draws over. */
          iconSize = NSMakeSize(xiconsize[0].max_width + 4,
				xiconsize[0].max_height + 4);
	}
      else
        {
          iconSize = NSMakeSize(xiconsize[0].max_width,
				xiconsize[0].max_height);
	}

      XFree(xiconsize);
      return iconSize;
    }

  return [super iconSize];
}

- (unsigned int) numberOfDesktops: (int)screen
{
  int c;
  unsigned int *num;
  unsigned int number = 0;

  num = (unsigned int*)PropGetCheckProperty(dpy, RootWindow(dpy, screen),
                                            generic._NET_NUMBER_OF_DESKTOPS_ATOM, XA_CARDINAL,
                                            32, 1, &c);

  if (num)
    {
      number = *num;
      XFree(num);
    }
  return number;
}

- (NSArray *) namesOfDesktops: (int)screen
{
  int c;
  char *names;

  names = (char *)PropGetCheckProperty(dpy, RootWindow(dpy, screen),
                                       generic._NET_DESKTOP_NAMES_ATOM, generic.UTF8_STRING_ATOM,
                                       0, 0, &c);
  if (names)
  {
    NSMutableArray *array = [[NSMutableArray alloc] init];
    char *p = names;

    while (p < names + c - 1)
      {
        [array addObject: [NSString stringWithUTF8String: p]];
        p += strlen(p) + 1;
      }
    XFree(names);
    return AUTORELEASE(array);
  }

  return nil;
}

- (unsigned int) desktopNumberForScreen: (int)screen
{
  int c;
  unsigned int *num;
  unsigned int number = 0;

  num = (unsigned int*)PropGetCheckProperty(dpy, RootWindow(dpy, screen),
                                            generic._NET_CURRENT_DESKTOP_ATOM, XA_CARDINAL,
                                            32, 1, &c);

  if (num)
    {
      number = *num;
      XFree(num);
    }
  return number;
}

- (void) setDesktopNumber: (unsigned int)workspace forScreen: (int)screen
{
  Window root = RootWindow(dpy, screen);

  [self _sendRoot: root
        type: generic._NET_CURRENT_DESKTOP_ATOM
        window: root
        data0: workspace
        data1: [self lastTime]
        data2: 0
        data3: 0];
}

- (unsigned int) desktopNumberForWindow: (int)win
{
  gswindow_device_t	*window;
  int c;
  unsigned int *num;
  unsigned int number = 0;

  window = WINDOW_WITH_TAG(win);
  if (!window)
    return 0;

  num = (unsigned int*)PropGetCheckProperty(dpy, window->ident,
                                            generic._NET_WM_DESKTOP_ATOM, XA_CARDINAL,
                                            32, 1, &c);

  if (num)
    {
      number = *num;
      XFree(num);
    }
  return number;
}

- (void) setDesktopNumber: (unsigned int)workspace forWindow: (int)win
{
  gswindow_device_t	*window;

  window = WINDOW_WITH_TAG(win);
  if (!window)
    return;

  [self _sendRoot: window->root
        type: generic._NET_WM_DESKTOP_ATOM
        window: window->ident
        data0: workspace
        data1: 1
        data2: 0
        data3: 0];
}

- (void) setShadow: (BOOL)hasShadow : (int)win
{
  gswindow_device_t	*window;
  unsigned long shadow;

  window = WINDOW_WITH_TAG(win);
  if (!window)
    return;

  if (hasShadow)
    {
      // FIXME: What size?
      shadow = (unsigned int)(0.1 * 0xffffffff);

      XChangeProperty(window->display, window->ident, generic._NET_WM_WINDOW_SHADOW_ATOM,
                      XA_CARDINAL, 32, PropModeReplace,
                      (unsigned char*)&shadow, 1L);
      if (window->parent != window->root)
        {
          XChangeProperty(window->display, window->parent, generic._NET_WM_WINDOW_SHADOW_ATOM,
                          XA_CARDINAL, 32, PropModeReplace,
                          (unsigned char*)&shadow, 1);
        }
    }
  else
    {
      XDeleteProperty(window->display, window->ident, generic._NET_WM_WINDOW_SHADOW_ATOM);
      if (window->parent != window->root)
        {
          XDeleteProperty(window->display, window->parent, generic._NET_WM_WINDOW_SHADOW_ATOM);
        }
    }
}

- (BOOL) hasShadow: (int)win
{
  gswindow_device_t	*window;
  int c;
  unsigned int *num;
  BOOL hasShadow = NO;

  window = WINDOW_WITH_TAG(win);
  if (!window)
    return hasShadow;

  num = (unsigned int*)PropGetCheckProperty(dpy, window->ident,
                                            generic._NET_WM_WINDOW_SHADOW_ATOM, XA_CARDINAL,
                                            32, 1, &c);

  if (num)
    {
      if (*num)
        hasShadow = YES;
      XFree(num);
    }
  return hasShadow;
}

/*
 * Check whether the window is miniaturized according to the ICCCM window
 * state property.
 */
- (int) _wm_state: (Window)win
{
  long *data;
  long state;

  data = (long *)PropGetCheckProperty(dpy, win, generic.WM_STATE_ATOM,
				      generic.WM_STATE_ATOM, 32, -1, NULL);

  if (data)
    {
      state = *data;
      XFree(data);
    }
  else
    state = WithdrawnState;

  return state;
}

/*
 * Check whether the EWMH window state includes the _NET_WM_STATE_HIDDEN
 * state. On EWMH, a window is iconified if it is iconic state and the
 * _NET_WM_STATE_HIDDEN is present.
 */
- (BOOL) _ewmh_isHidden: (Window)win
{
  Atom *data;
  int count;
  int i;

  data = (Atom *)PropGetCheckProperty(dpy, win,
                                      generic._NET_WM_STATE_ATOM,
                                      XA_ATOM, 32, -1, &count);

  if (!data)
    // if we don't have any information, default to "No"
    return NO;

  for (i = 0; i < count; i++)
    {
      if (data[i] == generic._NET_WM_STATE_HIDDEN_ATOM)
        {
          XFree(data);
          return YES;
        }
    }

  XFree(data);
  return NO;
}

- (void) setParentWindow: (int)parentWin
          forChildWindow: (int)childWin
{
  gswindow_device_t	*cwindow;
  gswindow_device_t	*pwindow;
  Window p;

  cwindow = WINDOW_WITH_TAG(childWin);
  if (!cwindow)
    return;

  pwindow = WINDOW_WITH_TAG(parentWin);
  if (!pwindow)
    p = None;
  else
    p = pwindow->ident;

  XSetTransientForHint(dpy, cwindow->ident, p);
}

- (void) setIgnoreMouse: (BOOL)ignoreMouse : (int)win
{
#if HAVE_XFIXES
  gswindow_device_t *window;
  XserverRegion region;
  int error;
  int xFixesEventBase;

  if (!XFixesQueryExtension(dpy, &xFixesEventBase, &error))
    {
      return;
    }

  window = WINDOW_WITH_TAG(win);
  if (!window)
    {
      return;
    }

  if (ignoreMouse)
    {
      region = XFixesCreateRegion(dpy, NULL, 0);
    }
  else
    {
      region = None;
    }


  XFixesSetWindowShapeRegion(dpy,
                             window->ident,
                             ShapeInput,
                             0, 0, region);
  if (region != None)
    {
      XFixesDestroyRegion(dpy, region);
    }
#endif
}

@end
