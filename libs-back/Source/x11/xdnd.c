/* 
   xdnd.c, xdnd.h - C program library for handling the Xdnd protocol

   Copyright (C) 1998  Paul Sheer

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111 USA.

   http://www.cco.caltech.edu/~jafl/xdnd/

   Further info can also be obtained by emailing the author at,
       psheer@obsidian.co.za

   Released 1998-08-07
*/

#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xatom.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "x11/xdnd.h"


// #define DND_DEBUG 
#define dnd_version_at_least(a,b) ((a) <= (b))

#ifdef DND_DEBUG
#define dnd_debug(a,b...) printf("%s: %d: " a "\n", __FILE__, __LINE__ , ## b)
#else

#ifdef NeXT_RUNTIME
#define dnd_debug //
#else  /* !NeXT_RUNTIME */
#define dnd_debug(a,b...)
#endif /* NeXT_RUNTIME */

#endif



void
xdnd_reset(DndClass * dnd)
{
  dnd->stage = XDND_DROP_STAGE_IDLE;
  dnd->dragging_version = 0;
  dnd->internal_drag = 0;
  dnd->want_position = 0;
  dnd->ready_to_drop = 0;
  dnd->will_accept = 0;
  dnd->rectangle.x = dnd->rectangle.y = 0;
  dnd->rectangle.width = dnd->rectangle.height = 0;
  dnd->dropper_window = 0;
  dnd->dragger_window = 0;
  dnd->dragger_typelist = 0;
  dnd->desired_type = 0;
  dnd->time = 0;
}

void
xdnd_init(DndClass * dnd, Display * display)
{
  memset (dnd, 0, sizeof (*dnd));

  dnd->display = display;
  dnd->root_window = DefaultRootWindow (display);
  dnd->version = XDND_VERSION;
  dnd->XdndAware = XInternAtom (dnd->display, "XdndAware", False);
  dnd->XdndSelection = XInternAtom (dnd->display, "XdndSelection", False);
  dnd->XdndEnter = XInternAtom (dnd->display, "XdndEnter", False);
  dnd->XdndLeave = XInternAtom (dnd->display, "XdndLeave", False);
  dnd->XdndPosition = XInternAtom (dnd->display, "XdndPosition", False);
  dnd->XdndDrop = XInternAtom (dnd->display, "XdndDrop", False);
  dnd->XdndFinished = XInternAtom (dnd->display, "XdndFinished", False);
  dnd->XdndStatus = XInternAtom (dnd->display, "XdndStatus", False);
  dnd->XdndActionCopy = XInternAtom (dnd->display, "XdndActionCopy", False);
  dnd->XdndActionMove = XInternAtom (dnd->display, "XdndActionMove", False);
  dnd->XdndActionLink = XInternAtom (dnd->display, "XdndActionLink", False);
  dnd->XdndActionAsk = XInternAtom (dnd->display, "XdndActionAsk", False);
  dnd->XdndActionPrivate=XInternAtom(dnd->display,"XdndActionPrivate",False);
  dnd->XdndTypeList = XInternAtom (dnd->display, "XdndTypeList", False);
  dnd->XdndActionList = XInternAtom (dnd->display, "XdndActionList", False);
  dnd->XdndActionDescription = XInternAtom(dnd->display,
    "XdndActionDescription", False);
  xdnd_reset(dnd);
}

static int
array_length(Atom * a)
{				// typelist is a null terminated array
  int n = 0;

  while (a[n])
    n++;
  return n;
}

void
xdnd_set_dnd_aware (DndClass * dnd, Window window, Atom * typelist)
{
  XChangeProperty (dnd->display, window, dnd->XdndAware, XA_ATOM, 32, 
    PropModeReplace, (unsigned char *) &dnd->version, 1);
  if (typelist) 
    {
      int n = array_length (typelist);
      if (n)
	XChangeProperty (dnd->display, window, dnd->XdndAware, XA_ATOM, 32, 
	  PropModeAppend, (unsigned char *) typelist, n);
    }
}

int
xdnd_is_dnd_aware(DndClass *dnd, Window window, int *version, Atom *typelist)
{
  Atom actual;
  int format;
  unsigned long count, remaining;
  unsigned char *data = 0;
  Atom *types, *t;
  int result = 1;

  *version = 0;
  XGetWindowProperty (dnd->display, window, dnd->XdndAware,
    0, 0x8000000L, False, XA_ATOM, &actual, &format,
    &count, &remaining, &data);

  if (actual != XA_ATOM || format != 32 || count == 0 || !data) 
    {
      dnd_debug("XGetWindowProperty failed in xdnd_is_dnd_aware - XdndAware = %ld", dnd->XdndAware);
      if (data)
	XFree(data);
      return 0;
    }

  types = (Atom *) data;
  *version = dnd->version < types[0] ? dnd->version : types[0];	// minimum
  dnd_debug ("Using XDND version %d", *version);
  if (count > 1) 
    {
      result = 0;
      for (t = typelist; *t; t++) 
	{
	  unsigned long j;
	  for (j = 1; j < count; j++) 
	    {
	      if (types[j] == *t) 
		{
		  result = 1;
		  break;
		}
	    }
	  if (result)
	    break;
	}
    }
  XFree(data);
  return result;
}

void
xdnd_send_enter(DndClass *dnd, Window window, Window from, Atom *typelist)
{
  XEvent xevent;
  int n, i;

  n = array_length (typelist);

  memset(&xevent, 0, sizeof (xevent));

  xevent.xany.type = ClientMessage;
  xevent.xany.display = dnd->display;
  xevent.xclient.window = window;
  xevent.xclient.message_type = dnd->XdndEnter;
  xevent.xclient.format = 32;

  XDND_ENTER_SOURCE_WIN (&xevent) = from;
  XDND_ENTER_THREE_TYPES_SET (&xevent, n > XDND_THREE);
  XDND_ENTER_VERSION_SET (&xevent, dnd->version);
  for (i = 0; i < n && i < XDND_THREE; i++)
    {
      XDND_ENTER_TYPE (&xevent, i) = typelist[i];
    }

  XSendEvent (dnd->display, window, 0, 0, &xevent);
}

void
xdnd_send_position(DndClass *dnd, Window window, Window from, Atom action, 
   int x, int y, unsigned long time)
{
  XEvent xevent;

  memset (&xevent, 0, sizeof (xevent));

  xevent.xany.type = ClientMessage;
  xevent.xany.display = dnd->display;
  xevent.xclient.window = window;
  xevent.xclient.message_type = dnd->XdndPosition;
  xevent.xclient.format = 32;

  XDND_POSITION_SOURCE_WIN (&xevent) = from;
  XDND_POSITION_ROOT_SET (&xevent, x, y);
  if (dnd_version_at_least (dnd->dragging_version, 1))
    XDND_POSITION_TIME (&xevent) = time;
  if (dnd_version_at_least (dnd->dragging_version, 2))
    XDND_POSITION_ACTION (&xevent) = action;

  XSendEvent (dnd->display, window, 0, 0, &xevent);
}

void
xdnd_send_status(DndClass *dnd, Window window, Window from, int will_accept, 
  int want_position, int x, int y, int w, int h, Atom action)
{
  XEvent xevent;

  memset (&xevent, 0, sizeof (xevent));

  xevent.xany.type = ClientMessage;
  xevent.xany.display = dnd->display;
  xevent.xclient.window = window;
  xevent.xclient.message_type = dnd->XdndStatus;
  xevent.xclient.format = 32;

  XDND_STATUS_TARGET_WIN (&xevent) = from;
  XDND_STATUS_WILL_ACCEPT_SET (&xevent, will_accept);
  if (will_accept)
    XDND_STATUS_WANT_POSITION_SET (&xevent, want_position);
  if (want_position)
    XDND_STATUS_RECT_SET (&xevent, x, y, w, h);
  if (dnd_version_at_least (dnd->dragging_version, 2))
    if (will_accept)
      XDND_STATUS_ACTION (&xevent) = action;

  XSendEvent (dnd->display, window, 0, 0, &xevent);
}

void
xdnd_send_leave(DndClass *dnd, Window window, Window from)
{
  XEvent xevent;

  memset(&xevent, 0, sizeof (xevent));

  xevent.xany.type = ClientMessage;
  xevent.xany.display = dnd->display;
  xevent.xclient.window = window;
  xevent.xclient.message_type = dnd->XdndLeave;
  xevent.xclient.format = 32;

  XDND_LEAVE_SOURCE_WIN (&xevent) = from;

  XSendEvent (dnd->display, window, 0, 0, &xevent);
}

void
xdnd_send_drop(DndClass *dnd, Window window, Window from, unsigned long time)
{
  XEvent xevent;

  memset (&xevent, 0, sizeof (xevent));

  xevent.xany.type = ClientMessage;
  xevent.xany.display = dnd->display;
  xevent.xclient.window = window;
  xevent.xclient.message_type = dnd->XdndDrop;
  xevent.xclient.format = 32;

  XDND_DROP_SOURCE_WIN (&xevent) = from;
  if (dnd_version_at_least (dnd->dragging_version, 1))
    XDND_DROP_TIME (&xevent) = time;

  XSendEvent (dnd->display, window, 0, 0, &xevent);
}

void
xdnd_send_finished(DndClass * dnd, Window window, Window from, int error)
{
  XEvent xevent;
  memset (&xevent, 0, sizeof (xevent));
  xevent.xany.type = ClientMessage;
  xevent.xany.display = dnd->display;
  xevent.xclient.window = window;				
  xevent.xclient.message_type = dnd->XdndFinished;
  xevent.xclient.format = 32;

  XDND_FINISHED_TARGET_WIN (&xevent) = from;

  XSendEvent (dnd->display, window, 0, 0, &xevent);
}

int
xdnd_convert_selection(DndClass *dnd, Window window, Window requester, Atom type)
{
  if (window != XGetSelectionOwner (dnd->display, dnd->XdndSelection)) 
    {
      dnd_debug ("xdnd_convert_selection(): XGetSelectionOwner failed");
      return 1;
    }

  XConvertSelection (dnd->display, dnd->XdndSelection, type,
    dnd->Xdnd_NON_PROTOCOL_ATOM, requester, CurrentTime);
  return 0;
}

int
xdnd_set_selection_owner(DndClass * dnd, Window window, Atom type)
{
  if (!XSetSelectionOwner(dnd->display,dnd->XdndSelection,window,CurrentTime)) 	
    {
      dnd_debug ("xdnd_set_selection_owner(): XSetSelectionOwner failed");
      return 1;	
    }	

  return 0;
}

void
xdnd_selection_send(DndClass * dnd, XSelectionRequestEvent * request, 
  unsigned char *data, int length)
{
  XEvent xevent;

  dnd_debug (" requestor = %ld", request->requestor);
  dnd_debug (" property = %ld", request->property);
  dnd_debug (" length = %d", length);

  XChangeProperty (dnd->display, request->requestor, request->property,
    request->target, 8, PropModeReplace, data, length);

  xevent.xselection.type = SelectionNotify;
  xevent.xselection.property = request->property;
  xevent.xselection.display = request->display;
  xevent.xselection.requestor = request->requestor;
  xevent.xselection.selection = request->selection;
  xevent.xselection.target = request->target;
  xevent.xselection.time = request->time;

  XSendEvent (dnd->display, request->requestor, 0, 0, &xevent);
}

//
// Unused
//

void
xdnd_set_type_list(DndClass * dnd, Window window, Atom * typelist)
{
  int n = array_length (typelist);

  XChangeProperty (dnd->display, window, dnd->XdndTypeList, XA_ATOM, 32,
    PropModeReplace, (unsigned char *) typelist, n);
}

/* result must be free'd */
void
xdnd_get_type_list(DndClass * dnd, Window window, Atom ** typelist)
{
  Atom type, *a;
  int format;
  unsigned long i, count, remaining;
  unsigned char *data = NULL;

  *typelist = 0;

  XGetWindowProperty (dnd->display, window, dnd->XdndTypeList,
    0, 0x8000000L, False, XA_ATOM, &type, &format, &count, &remaining, &data);

  if (type != XA_ATOM || format != 32 || count == 0 || !data)
    {
      if (data)
	XFree (data);
      dnd_debug ("XGetWindowProperty failed in xdnd_get_type_list - dnd->XdndTypeList = %ld", dnd->XdndTypeList);
      return;
    }
  *typelist = malloc ((count + 1) * sizeof (Atom));
  a = (Atom *) data;
  for (i = 0; i < count; i++)
    (*typelist)[i] = a[i];
  (*typelist)[count] = 0;

  XFree (data);
}
