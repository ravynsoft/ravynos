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

   Further info can also be obtained by emailing the author at,
       psheer@obsidian.co.za
*/

#ifndef _X_DND_H
#define _X_DND_H

#define XDND_VERSION 2

/* XdndEnter */
#define XDND_THREE 3
#define XDND_ENTER_SOURCE_WIN(e)		((e)->xclient.data.l[0])
#define XDND_ENTER_THREE_TYPES(e)		(((e)->xclient.data.l[1] & 0x1UL) == 0)
#define XDND_ENTER_THREE_TYPES_SET(e,b)	(e)->xclient.data.l[1] = ((e)->xclient.data.l[1] & ~0x1UL) | (((b) == 0) ? 0 : 0x1UL)
#define XDND_ENTER_VERSION(e)			((e)->xclient.data.l[1] >> 24)
#define XDND_ENTER_VERSION_SET(e,v)		(e)->xclient.data.l[1] = ((e)->xclient.data.l[1] & ~(0xFF << 24)) | ((v) << 24)
#define XDND_ENTER_TYPE(e,i)			((e)->xclient.data.l[2 + i])	/* i => (0, 1, 2) */

/* XdndPosition */
#define XDND_POSITION_SOURCE_WIN(e)		((e)->xclient.data.l[0])
#define XDND_POSITION_ROOT_X(e)			((e)->xclient.data.l[2] >> 16)
#define XDND_POSITION_ROOT_Y(e)			((e)->xclient.data.l[2] & 0xFFFFUL)
#define XDND_POSITION_ROOT_SET(e,x,y)	(e)->xclient.data.l[2]  = ((x) << 16) | ((y) & 0xFFFFUL)
#define XDND_POSITION_TIME(e)			((e)->xclient.data.l[3])
#define XDND_POSITION_ACTION(e)			((e)->xclient.data.l[4])

/* XdndStatus */
#define XDND_STATUS_TARGET_WIN(e)			((e)->xclient.data.l[0])
#define XDND_STATUS_WILL_ACCEPT(e)			((e)->xclient.data.l[1] & 0x1L)
#define XDND_STATUS_WILL_ACCEPT_SET(e,b)	(e)->xclient.data.l[1] = ((e)->xclient.data.l[1] & ~0x1UL) | (((b) == 0) ? 0 : 0x1UL)
#define XDND_STATUS_WANT_POSITION(e)		((e)->xclient.data.l[1] & 0x2UL)
#define XDND_STATUS_WANT_POSITION_SET(e,b) (e)->xclient.data.l[1] = ((e)->xclient.data.l[1] & ~0x2UL) | (((b) == 0) ? 0 : 0x2UL)
#define XDND_STATUS_RECT_X(e)				((e)->xclient.data.l[2] >> 16)
#define XDND_STATUS_RECT_Y(e)				((e)->xclient.data.l[2] & 0xFFFFL)
#define XDND_STATUS_RECT_WIDTH(e)			((e)->xclient.data.l[3] >> 16)
#define XDND_STATUS_RECT_HEIGHT(e)			((e)->xclient.data.l[3] & 0xFFFFL)
#define XDND_STATUS_RECT_SET(e,x,y,w,h)		{(e)->xclient.data.l[2] = ((x) << 16) | ((y) & 0xFFFFUL); (e)->xclient.data.l[3] = ((w) << 16) | ((h) & 0xFFFFUL); }
#define XDND_STATUS_ACTION(e)		((e)->xclient.data.l[4])

/* XdndLeave */
#define XDND_LEAVE_SOURCE_WIN(e)	((e)->xclient.data.l[0])

/* XdndDrop */
#define XDND_DROP_SOURCE_WIN(e)		((e)->xclient.data.l[0])
#define XDND_DROP_TIME(e)			((e)->xclient.data.l[2])

/* XdndFinished */
#define XDND_FINISHED_TARGET_WIN(e)	((e)->xclient.data.l[0])

typedef struct _DndClass DndClass;

struct _DndClass {

    Display *display;

    Atom XdndAware;
    Atom XdndSelection;
    Atom XdndEnter;
    Atom XdndLeave;
    Atom XdndPosition;
    Atom XdndDrop;
    Atom XdndFinished;
    Atom XdndStatus;
    Atom XdndActionCopy;
    Atom XdndActionMove;
    Atom XdndActionLink;
    Atom XdndActionAsk;
    Atom XdndActionPrivate;
    Atom XdndTypeList;
    Atom XdndActionList;
    Atom XdndActionDescription;
    Atom Xdnd_NON_PROTOCOL_ATOM;
    Atom version;
    Window root_window;

#define XDND_DROP_STAGE_IDLE		0
#define XDND_DRAG_STAGE_DRAGGING	1
#define XDND_DRAG_STAGE_ENTERED		2
#define XDND_DROP_STAGE_CONVERTING	3
#define XDND_DROP_STAGE_ENTERED		4
    int stage;
    int dragging_version;
    int internal_drag;
    int want_position;
    int ready_to_drop;
    int will_accept;
    XRectangle rectangle;
    Window dropper_window, dragger_window;
    Atom *dragger_typelist;
    Atom desired_type;
    Atom supported_action;
    Time time;
/* drop position from last XdndPosition */
    int x, y;

/* block for only this many seconds on not receiving a XdndFinished from target, default : 10 */
    int time_out;
};

void xdnd_init (DndClass * dnd, Display * display);
void xdnd_set_dnd_aware (DndClass * dnd, Window window, Atom * typelist);
int xdnd_is_dnd_aware (DndClass * dnd, Window window, int *version, Atom * typelist);
void xdnd_set_type_list (DndClass * dnd, Window window, Atom * typelist);
void xdnd_get_type_list (DndClass * dnd, Window window, Atom ** typelist);
void xdnd_send_enter (DndClass * dnd, Window window, Window from, Atom * typelist);
void xdnd_send_position (DndClass * dnd, Window window, Window from, Atom action, int x, int y, unsigned long etime);
void xdnd_send_status (DndClass * dnd, Window window, Window from, int will_accept,
	     int want_position, int x, int y, int w, int h, Atom action);
void xdnd_send_leave (DndClass * dnd, Window window, Window from);
void xdnd_send_drop (DndClass * dnd, Window window, Window from, unsigned long etime);
void xdnd_send_finished (DndClass * dnd, Window window, Window from, int error);
int xdnd_convert_selection (DndClass * dnd, Window window, Window requester, Atom type);
int xdnd_set_selection_owner (DndClass * dnd, Window window, Atom type);
void xdnd_selection_send (DndClass * dnd, XSelectionRequestEvent * request, unsigned char *data, int length);

#endif 	/* !_X_DND_H */
