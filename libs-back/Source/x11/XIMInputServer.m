/* XIMInputServer - XIM Keyboard input handling

   Copyright (C) 2002 Free Software Foundation, Inc.

   Author: Christian Gillot <cgillot@neo-rousseaux.org>
   Date: Nov 2001
   Author: Adam Fedor <fedor@gnu.org>
   Date: Jan 2002

   This file is part of the GNUstep GUI Library.

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

#include <Foundation/NSUserDefaults.h>
#include <Foundation/NSData.h>
#include <Foundation/NSDebug.h>
#include <Foundation/NSException.h>
#include <GNUstepBase/Unicode.h>
#include <AppKit/NSWindow.h>
#include <AppKit/NSView.h>

#include "x11/XGInputServer.h"
#include <X11/Xlocale.h>

#define NoneStyle           (XIMPreeditNone     | XIMStatusNone)
#define RootWindowStyle	    (XIMPreeditNothing	| XIMStatusNothing)
#define OffTheSpotStyle	    (XIMPreeditArea	| XIMStatusArea)
#define OverTheSpotStyle    (XIMPreeditPosition	| XIMStatusArea)
#define OnTheSpotStyle	    (XIMPreeditCallbacks| XIMStatusCallbacks)


@interface XIMInputServer (XIMPrivate)
- (BOOL) ximInit: (Display *)dpy;
- (void) ximClose;
- (int) ximStyleInit;
- (XIC) ximCreateIC: (Window)w;
- (unsigned long) ximXicGetMask: (XIC)xic;
@end

#define BUF_LEN 255

@implementation XIMInputServer

- (id) initWithDelegate: (id)aDelegate
		   name: (NSString *)name
{
  Display *dpy = [XGServer xDisplay];
  return [self initWithDelegate: aDelegate display: dpy name: name];
}

- (id) initWithDelegate: (id)aDelegate
		display: (Display *)dpy
		   name: (NSString *)name
{
  char *locale;
  delegate = aDelegate;
  ASSIGN(server_name, name);

  locale = setlocale(LC_CTYPE, "");

  if (XSupportsLocale() != True) 
    {
      NSLog(@"Xlib does not support locale setting %s", locale);
      /* FIXME: Should we reset the locale or just hope that X 
	 can deal with it? */
    }

#ifdef USE_XIM
  if ([self ximInit: dpy] == NO)
    {
      NSLog(@"Unable to initialize XIM, using standard keyboard events");
    }
#endif
  return self;
}

- (void) dealloc
{
  DESTROY(server_name);
  [self ximClose];
  [super dealloc];
}

/* ----------------------------------------------------------------------
   XInputFiltering protocol methods
*/
- (BOOL) filterEvent: (XEvent *)event
{
  if (XFilterEvent(event, None)) 
    {
      NSDebugLLog(@"NSKeyEvent", @"Event filtered by XIM\n");
      return YES;
    }
  return NO;
}

- (NSString *) lookupStringForEvent: (XKeyEvent *)event 
			     window: (gswindow_device_t *)windev
			     keysym: (KeySym *)keysymptr
{
  int count = 0;
  NSString *keys = nil;
  KeySym keysym = 0;
  char buf[BUF_LEN];

  /* Process characters */

  /* N.B. The Xutf8LookupString manpage says this macro will be defined 
     in the X headers if that function is available. */
#if defined(X_HAVE_UTF8_STRING)
  if (windev->ic && event->type == KeyPress)
    {
      Status status = 0;
      count = Xutf8LookupString(windev->ic, event, buf, BUF_LEN, 
				&keysym, &status);

      if (status==XBufferOverflow)
	NSDebugLLog(@"NSKeyEvent",@"XmbLookupString buffer overflow\n");
      if (count)
	{
	  keys = [[[NSString alloc] initWithBytes: buf
					   length: count
					 encoding: NSUTF8StringEncoding] autorelease];
	}
      if (status == XLookupKeySym 
	  || status == XLookupBoth)
	{
	  if (keysymptr)
	    *keysymptr = keysym;
	}
    }
 else
#endif
    {
      /* Always returns a Latin-1 string according to the manpage */
      count = XLookupString (event, buf, BUF_LEN, &keysym, NULL);
      if (count)
	{
	  keys = [[[NSString alloc] initWithBytes: buf
					   length: count
					 encoding: NSISOLatin1StringEncoding] autorelease];
	}

      if (keysymptr)
	*keysymptr = keysym;
    }

  return keys;
}

/* ----------------------------------------------------------------------
   NSInputServiceProvider protocol methods
*/
- (void) activeConversationChanged: (id)sender
		 toNewConversation: (long)newConversation
{
  NSWindow *window;
  gswindow_device_t *windev;

  [super activeConversationChanged: sender
	         toNewConversation: newConversation];

  if ([sender respondsToSelector: @selector(window)] == NO)
    {
      [NSException raise: NSInvalidArgumentException
                  format: @"NSTextInput sender does not respond to window"];
    }
  window = [sender window];
  windev = [XGServer _windowWithTag: [window windowNumber]];
  if (windev == NULL)
    {
      [NSException raise: NSInvalidArgumentException
                  format: @"NSTextInput sender has invalid window"];
    }

  [self ximFocusICWindow: windev];
}

- (void) activeConversationWillChange: (id)sender
		  fromOldConversation: (long)oldConversation
{
  [super activeConversationWillChange: sender
	          fromOldConversation: oldConversation];
}

/* ----------------------------------------------------------------------
   XIM private methods
*/
- (BOOL) ximInit: (Display *)dpy
{
  XClassHint class_hints;

  if (!XSetLocaleModifiers (""))
    NSDebugLLog(@"XIM", @"can not set locale modifiers\n");

  /* FIXME: Get these */
  class_hints.res_name = class_hints.res_class = NULL;
  xim = XOpenIM(dpy, NULL, class_hints.res_name, class_hints.res_class);
  if (xim == NULL) 
    {
      NSDebugLLog(@"XIM", @"Can't open XIM.\n");
      return NO;
    }

  if (![self ximStyleInit])
    {
      [self ximClose];
      return NO;
    }

  NSDebugLLog(@"XIM", @"Initialized XIM\n");
  return YES;
}

static XIMStyle
betterStyle(XIMStyle a, XIMStyle b, XIMStyle xim_requested_style)
{
  if (a == xim_requested_style)
    return a;
  if (b == xim_requested_style)
    return b;

  // N.B. We don't support OnTheSpotStyle so it is omitted here

  if (a == OverTheSpotStyle)
    return a;
  if (b == OverTheSpotStyle)
    return b;

  if (a == OffTheSpotStyle)
    return a;
  if (b == OffTheSpotStyle)
    return b;

  if (a == RootWindowStyle)
    return a;
  if (b == RootWindowStyle)
    return b;

  if (a == NoneStyle)
    return a;
  if (b == NoneStyle)
    return b;

  return 0;
}

- (int) ximStyleInit
{
  NSUserDefaults    *uds;
  NSString	    *request;
  XIMStyle	    xim_requested_style;
  XIMStyles	    *styles;
  char		    *failed_arg;
  int		    i;
  
  uds = [NSUserDefaults standardUserDefaults];
  if ((request = [uds stringForKey: @"GSXIMInputMethodStyle"]) == nil)
    {
      xim_requested_style = RootWindowStyle;
    }
  else if ([request isEqual: @"RootWindow"])
    {
      xim_requested_style = RootWindowStyle;
    }
  else if ([request isEqual: @"OffTheSpot"])
    {
      xim_requested_style = OffTheSpotStyle;
    }
  else if ([request isEqual: @"OverTheSpot"])
    {
      xim_requested_style = OverTheSpotStyle;
    }
  else if ([request isEqual: @"OnTheSpot"])
    {
      xim_requested_style = OnTheSpotStyle;
    }
  else
    {
      NSLog(@"XIM: Unknown style '%@'.\n"
	    @"Fallback to RootWindow style.", request);
      xim_requested_style = RootWindowStyle;
    }

  failed_arg = XGetIMValues(xim, XNQueryInputStyle, &styles, NULL);
  if (failed_arg != NULL)
    {
      NSDebugLLog(@"XIM", @"Can't getting the following IM value :%s",
		  failed_arg);
      return 0;
    } 

  for (i = 0; i < styles->count_styles; i++)
    {
      xim_style = betterStyle(xim_style, styles->supported_styles[i],
			      xim_requested_style);
    }
  XFree(styles);

  if (xim_style == 0)
    {
      NSLog(@"XIM: no supported styles found");
      return 0;    
    }
  
  return 1;
}

- (void) ximClose
{
  int i;
  for (i=0;i<num_xics;i++)
    {
      XDestroyIC(xics[i]);
    }
  free(xics);
  num_xics=0;
  xics=NULL;

  NSDebugLLog(@"XIM", @"Closed XIM\n");

  if (xim)
    XCloseIM(xim);
  xim=NULL;
}

- (void) ximFocusICWindow: (gswindow_device_t *)windev
{
  if (xim == NULL)
    return;

  /* Make sure we have an ic for this window */
#ifdef USE_XIM
  if (windev->ic == NULL)
    {
      windev->ic = [self ximCreateIC: windev->ident];
      if (windev->ic == NULL) 
	{
	  [self ximClose];
	}
    }
#endif
  
  /* Now set focus to this window */
  if (windev->ic)
    {
      NSDebugLLog(@"XIM", @"XSetICFocus to window %lu", 
		  windev->ident);
      XSetICFocus(windev->ic);
    }
}

- (XIC) ximCreateIC: (Window)w
{
  XIC xic = NULL;

  if (xim_style == RootWindowStyle || xim_style == NoneStyle)
    {
      xic = XCreateIC(xim,
		      XNInputStyle, xim_style,
		      XNClientWindow, w,
		      NULL);
    }
  else if (xim_style == OffTheSpotStyle || xim_style == OverTheSpotStyle)
    {
      Display	    *dpy = [XGServer xDisplay];
      XFontSet	    font_set;
      char	    **missing_charset;
      int	    num_missing_charset;
      int	    dummy = 0;
      XVaNestedList preedit_args = NULL;
      XVaNestedList status_args = NULL;	
      XRectangle    status_area;
      XRectangle    preedit_area;
      XPoint	    preedit_spot;
      NSString	    *ns_font_size;
      int	    font_size;
      char	    base_font_name[64];

      //
      // Create a FontSet
      //

      // N.B. Because some input methods fail to provide a default font set,
      // we have to do it by ourselves.
      ns_font_size = [self fontSize: &font_size];
      sprintf(base_font_name, "*medium-r-normal--%s*", [ns_font_size cString]);
      font_set = XCreateFontSet(dpy,
				base_font_name,
				&missing_charset,
				&num_missing_charset,
				NULL);
      if (!font_set)
	{
	  goto finish;
	}
      if (missing_charset)
	{
	  int i;
	  NSLog(@"XIM: missing charset: ");
	  for (i = 0; i < num_missing_charset; ++i)
	    NSLog(@"%s", missing_charset[i]);
	  XFreeStringList(missing_charset);
	}

      //
      // Create XIC.
      //
      // At least, XNFontSet and XNPreeditSpotLocation must be specified
      // at initialization time.
      //
      status_area.width = font_size * 2;
      status_area.height = font_size + 2;
      status_area.x = 0;
      status_area.y = 0;

      status_args = XVaCreateNestedList(dummy,
					XNArea, &status_area,
					XNFontSet, font_set,
					NULL);

      preedit_area.width = 120;
      preedit_area.height = status_area.height;
      preedit_area.x = 0;
      preedit_area.y = 0;

      preedit_spot.x = 0;
      preedit_spot.y = 0;

      preedit_args = XVaCreateNestedList(dummy,
					 XNArea, &preedit_area,
					 XNSpotLocation, &preedit_spot,
					 XNFontSet, font_set,
					 NULL);

      xic = XCreateIC(xim,
		      XNInputStyle, xim_style,
		      XNClientWindow, w,
		      XNPreeditAttributes, preedit_args,
		      XNStatusAttributes, status_args,
		      NULL);

      if (preedit_args) { XFree(preedit_args); preedit_args = NULL; }
      if (status_args) { XFree(status_args); status_args = NULL; }
      if (font_set) XFreeFontSet(dpy, font_set);
    }
  else if (xim_style == OnTheSpotStyle)
    {
      NSLog(@"XIM: GNUstep doesn't support 'OnTheSpot'.\n"
	    @"Fallback to RootWindow style.");
      xim_style = RootWindowStyle;
      xic = XCreateIC(xim,
		      XNInputStyle, xim_style,
		      XNClientWindow, w,
		      NULL);
    }

finish:
  if (xic == NULL)
    NSDebugLLog(@"XIM", @"Can't create the input context.\n");

  xics = realloc(xics, sizeof(XIC) * (num_xics + 1));
  xics[num_xics++] = xic;
  return xic;
}

- (unsigned long) ximXicGetMask: (XIC)xic
{
  unsigned long xic_xmask = 0;
  if (XGetICValues(xic,XNFilterEvents,&xic_xmask,NULL)!=NULL)
    NSDebugLLog(@"XIM", @"Can't get the event mask for that input context");

  return xic_xmask;
}

- (void) ximCloseIC: (XIC)xic
{
  int i;
  for (i = 0; i < num_xics; i++)
    {
      if (xics[i] == xic)
        break;
    }
  if (i == num_xics)
    {
      NSLog(@"internal error in ximCloseIC: can't find XIC in list");
      abort();
    }
  for (i++; i < num_xics; i++)
    xics[i - 1] = xics[i];
  num_xics--;

  XDestroyIC(xic);
}

@end

@implementation XIMInputServer (InputMethod)
- (NSString *) inputMethodStyle
{
  if (num_xics > 0)
    {
      if (xim_style == RootWindowStyle)
	return @"RootWindow";
      else if (xim_style == OffTheSpotStyle)
	return @"OffTheSpot";
      else if (xim_style == OverTheSpotStyle)
	return @"OverTheSpot";
      else if (xim_style == OnTheSpotStyle)
	return @"OnTheSpot";
    }
  return nil;
}

- (NSString *) fontSize: (int *)size
{
  NSString *str;

  str = [[NSUserDefaults standardUserDefaults] stringForKey: @"NSFontSize"];
  if (!str)
    str = @"12";
  *size = (int)strtol([str cString], NULL, 10);
  return str;
}

- (BOOL) clientWindowRect: (NSRect *)rect
{
  Window	win;
  Window	dummy;
  Display	*dpy;
  int		abs_x, abs_y;
  int		x, y;
  unsigned int  w, h;
  unsigned int  bw, d;

  if (num_xics <= 0 || !rect) return NO;

  *rect = NSMakeRect(0, 0, 0, 0);

  if (XGetICValues(xics[num_xics - 1], XNClientWindow, &win, NULL))
    return NO;
  dpy = [XGServer xDisplay];
  if (XTranslateCoordinates(dpy, win, DefaultRootWindow(dpy), 0, 0,
			    &abs_x, &abs_y, &dummy) == 0)
    return NO;
  XGetGeometry(dpy, win, &dummy, &x, &y, &w, &h, &bw, &d);

  // X Window Coordinates to GNUstep Coordinates
  x = abs_x;
  y = XDisplayHeight(dpy, 0) - (abs_y + h);

  *rect =  NSMakeRect((float)x, (float)y, (float)w, (float)h);

  return YES;
}

- (BOOL) statusArea: (NSRect *)rect
{
  if (num_xics > 0 && (xim_style & XIMStatusArea))
    {
      XRectangle    area;
      int	    dummy = 0;
      XVaNestedList arglist = NULL;

      if (!(arglist = XVaCreateNestedList(dummy, XNArea, &area, NULL)))
	{
	  return NO;
	}
      XGetICValues(xics[num_xics - 1], XNStatusAttributes, arglist, NULL);
      
      rect->origin.x	= area.x;
      rect->origin.y	= area.y;
      rect->size.width	= area.width;
      rect->size.height	= area.height;

      if (arglist) { XFree(arglist); arglist = NULL; }

      return YES;
    }
  return NO;
}

- (BOOL) preeditArea: (NSRect *)rect
{
  if (num_xics > 0
      && ((xim_style & XIMPreeditArea) || (xim_style & XIMPreeditPosition)))
    {
      XRectangle    area;
      int	    dummy = 0;
      XVaNestedList arglist = NULL;

      if (!(arglist = XVaCreateNestedList(dummy, XNArea, &area, NULL)))
	{
	  return NO;
	}
      XGetICValues(xics[num_xics - 1], XNPreeditAttributes, arglist, NULL);

      rect->origin.x	= area.x;
      rect->origin.y	= area.y;
      rect->size.width	= area.width;
      rect->size.height	= area.height;

      if (arglist) { XFree(arglist); arglist = NULL; }

      return YES;
    }
  return NO;
}

- (BOOL) preeditSpot: (NSPoint *)p
{
  if (num_xics > 0 && (xim_style & XIMPreeditPosition))
    {
      XPoint	    spot;
      int	    dummy = 0;
      XVaNestedList arglist = NULL;

      if (!(arglist = XVaCreateNestedList(dummy, XNSpotLocation, &spot, NULL)))
	{
	  return NO;
	}
      XGetICValues(xics[num_xics - 1], XNPreeditAttributes, arglist, NULL);

      p->x = spot.x;
      p->y = spot.y;

      if (arglist) { XFree(arglist); arglist = NULL; }

      return YES;
    }
  return NO;
}

- (BOOL) setStatusArea: (NSRect *)rect
{
  if (num_xics > 0 && (xim_style & XIMStatusArea))
    {
      XRectangle    area;
      int	    dummy = 0;
      XVaNestedList arglist = NULL;

      area.x	    = rect->origin.x;
      area.y	    = rect->origin.y;
      area.width    = rect->size.width;
      area.height   = rect->size.height;

      if (!(arglist = XVaCreateNestedList(dummy, XNArea, &area, NULL)))
	{
	  return NO;
	}
      XSetICValues(xics[num_xics - 1], XNStatusAttributes, arglist, NULL);

      if (arglist) { XFree(arglist); arglist = NULL; }

      return YES;
    }
  return NO;
}

- (BOOL) setPreeditArea: (NSRect *)rect
{
  if (num_xics > 0
      && ((xim_style & XIMPreeditArea) || (xim_style & XIMPreeditPosition)))
    {
      XRectangle    area;
      int	    dummy = 0;
      XVaNestedList arglist = NULL;

      area.x	    = rect->origin.x;
      area.y	    = rect->origin.y;
      area.width    = rect->size.width;
      area.height   = rect->size.height;

      if (!(arglist = XVaCreateNestedList(dummy, XNArea, &area, NULL)))
	{
	  return NO;
	}
      XSetICValues(xics[num_xics - 1], XNPreeditAttributes, arglist, NULL);

      if (arglist) { XFree(arglist); arglist = NULL; }

      return YES;
    }
  return NO;
}

- (BOOL) setPreeditSpot: (NSPoint *)p
{
  if (num_xics > 0 && (xim_style & XIMPreeditPosition))
    {
      XPoint	    spot;
      int	    dummy = 0;
      XVaNestedList arglist = NULL;

      spot.x = p->x;
      spot.y = p->y;

      if (!(arglist = XVaCreateNestedList(dummy, XNSpotLocation, &spot, NULL)))
	{
	  return NO;
	}
      XSetICValues(xics[num_xics - 1], XNPreeditAttributes, arglist, NULL);

      if (arglist) { XFree(arglist); arglist = NULL; }

      return YES;
    }
  return NO;
}

@end // XIMInputServer (InputMethod)
