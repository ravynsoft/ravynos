/* Generic header info common to X backends for GNUstep

   Copyright (C) 2000 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   Date: Mar 2000
   
   This file is part of the GNUstep project

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

#ifndef	INCLUDED_XGGENERIC_H
#define	INCLUDED_XGGENERIC_H

/*
 * Flags to indicate which protocols the WindowManager follows
 */
typedef	enum {
  XGWM_UNKNOWN = 0,
  XGWM_WINDOWMAKER = 1,
  XGWM_GNOME = 2,
  XGWM_EWMH = 8
} XGWMProtocols;

static char *atom_names[] = {
  "TEXT",
  "UTF8_STRING",
  "WM_PROTOCOLS",
  "WM_TAKE_FOCUS",
  "WM_DELETE_WINDOW",
  "WM_STATE",
  "_NET_WM_PING",
  "_NET_WM_SYNC_REQUEST",
  "_NET_WM_SYNC_REQUEST_COUNTER",
  "_NET_WM_WINDOW_TYPE",
  "_NET_WM_WINDOW_TYPE_DESKTOP",
  "_NET_WM_WINDOW_TYPE_DOCK",
  "_NET_WM_WINDOW_TYPE_TOOLBAR",
  "_NET_WM_WINDOW_TYPE_MENU",
  "_NET_WM_WINDOW_TYPE_DIALOG",
  "_NET_WM_WINDOW_TYPE_NORMAL",
  // New in wmspec 1.2
  "_NET_WM_WINDOW_TYPE_UTILITY",
  "_NET_WM_WINDOW_TYPE_SPLASH",
  // New in wmspec 1.4
  "_NET_WM_WINDOW_TYPE_POPUP_MENU",
  "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU",
  "_NET_WM_WINDOW_TYPE_TOOLTIP",
  "_NET_WM_WINDOW_TYPE_NOTIFICATION",
  "_NET_WM_WINDOW_TYPE_COMBO",
  "_NET_WM_WINDOW_TYPE_DND",
  //KDE extensions
  "_KDE_NET_WM_WINDOW_TYPE_OVERRIDE",
  // Window state
  "_NET_WM_STATE",
  "_NET_WM_STATE_MODAL",
  "_NET_WM_STATE_STICKY",
  "_NET_WM_STATE_MAXIMIZED_VERT",
  "_NET_WM_STATE_MAXIMIZED_HORZ",
  "_NET_WM_STATE_SHADED",
  "_NET_WM_STATE_SKIP_TASKBAR",
  "_NET_WM_STATE_SKIP_PAGER",
  "_NET_WM_STATE_HIDDEN",
  "_NET_WM_STATE_FULLSCREEN",
  "_NET_WM_STATE_ABOVE",
  "_NET_WM_STATE_BELOW",
  "_NET_WM_STATE_DEMANDS_ATTENTION",
  "_NET_WM_NAME",
  "_NET_WM_PID",
  "_NET_WM_ICON",
  "_NET_WM_ICON_NAME",
  "_NET_WM_DESKTOP",
  "_NET_WM_WINDOW_SHADOW",
  "_NET_WM_USER_TIME",
  "_NET_WM_WINDOW_OPACITY",
  "_MOTIF_WM_HINTS",
  "_NET_SUPPORTED",
  "_NET_FRAME_EXTENTS",
  "_NET_REQUEST_FRAME_EXTENTS",
  "_KDE_NET_WM_FRAME_STRUT",
  "_WIN_SUPPORTING_WM_CHECK",
  "_NET_SUPPORTING_WM_CHECK",
  "_NET_DESKTOP_NAMES",
  "_NET_CURRENT_DESKTOP",
  "_NET_NUMBER_OF_DESKTOPS",
  "_NET_CLIENT_LIST_STACKING",
  "_NET_ACTIVE_WINDOW",
  "_WIN_LAYER",
  "_WINDOWMAKER_WM_PROTOCOLS",
  "_WINDOWMAKER_NOTICEBOARD",
  "_WINDOWMAKER_ICON_TILE",
  "_WINDOWMAKER_WM_FUNCTION",
  "_RGBA_IMAGE",
  "_GNUSTEP_WM_MINIATURIZE_WINDOW",
  "_GNUSTEP_WM_HIDE_APP",
  "_GNUSTEP_WM_ATTR",
  "_GNUSTEP_TITLEBAR_STATE",
  "_GNUSTEP_FRAME_OFFSETS",
  "WM_IGNORE_FOCUS_EVENTS"
 };

/*
 * Macros to access elements in atom_names array.
 */
#define TEXT_ATOM                              atoms[0]
#define UTF8_STRING_ATOM                       atoms[1]
#define WM_PROTOCOLS_ATOM                      atoms[2]
#define WM_TAKE_FOCUS_ATOM                     atoms[3]
#define WM_DELETE_WINDOW_ATOM                  atoms[4]
#define WM_STATE_ATOM                          atoms[5]
#define _NET_WM_PING_ATOM                      atoms[6]
#define _NET_WM_SYNC_REQUEST_ATOM              atoms[7]
#define _NET_WM_SYNC_REQUEST_COUNTER_ATOM      atoms[8]
#define _NET_WM_WINDOW_TYPE_ATOM               atoms[9]
#define _NET_WM_WINDOW_TYPE_DESKTOP_ATOM       atoms[10]
#define _NET_WM_WINDOW_TYPE_DOCK_ATOM          atoms[11]
#define _NET_WM_WINDOW_TYPE_TOOLBAR_ATOM       atoms[12]
#define _NET_WM_WINDOW_TYPE_MENU_ATOM          atoms[13]
#define _NET_WM_WINDOW_TYPE_DIALOG_ATOM        atoms[14]
#define _NET_WM_WINDOW_TYPE_NORMAL_ATOM        atoms[15]
#define _NET_WM_WINDOW_TYPE_UTILITY_ATOM       atoms[16]
#define _NET_WM_WINDOW_TYPE_SPLASH_ATOM        atoms[17]
#define _NET_WM_WINDOW_TYPE_POPUP_MENU_ATOM    atoms[18]
#define _NET_WM_WINDOW_TYPE_DROPDOWN_MENU_ATOM atoms[19]
#define _NET_WM_WINDOW_TYPE_TOOLTIP_ATOM       atoms[20]
#define _NET_WM_WINDOW_TYPE_NOTIFICATION_ATOM  atoms[21]
#define _NET_WM_WINDOW_TYPE_COMBO_ATOM         atoms[22]
#define _NET_WM_WINDOW_TYPE_DND_ATOM           atoms[23]
#define _KDE_NET_WM_WINDOW_TYPE_OVERRIDE_ATOM  atoms[24]
#define _NET_WM_STATE_ATOM                     atoms[25]
#define _NET_WM_STATE_MODAL_ATOM               atoms[26]
#define _NET_WM_STATE_STICKY_ATOM              atoms[27]
#define _NET_WM_STATE_MAXIMIZED_VERT_ATOM      atoms[28]
#define _NET_WM_STATE_MAXIMIZED_HORZ_ATOM      atoms[29]
#define _NET_WM_STATE_SHADED_ATOM              atoms[30]
#define _NET_WM_STATE_SKIP_TASKBAR_ATOM        atoms[31]
#define _NET_WM_STATE_SKIP_PAGER_ATOM          atoms[32]
#define _NET_WM_STATE_HIDDEN_ATOM              atoms[33]
#define _NET_WM_STATE_FULLSCREEN_ATOM          atoms[34]
#define _NET_WM_STATE_ABOVE_ATOM               atoms[35]
#define _NET_WM_STATE_BELOW_ATOM               atoms[36]
#define _NET_WM_STATE_DEMANDS_ATTENTION_ATOM   atoms[37]
#define _NET_WM_NAME_ATOM                      atoms[38]
#define _NET_WM_PID_ATOM                       atoms[39]
#define _NET_WM_ICON_ATOM                      atoms[40]
#define _NET_WM_ICON_NAME_ATOM                 atoms[41]
#define _NET_WM_DESKTOP_ATOM                   atoms[42]
#define _NET_WM_WINDOW_SHADOW_ATOM             atoms[43]
#define _NET_WM_USER_TIME_ATOM                 atoms[44]
#define _NET_WM_WINDOW_OPACITY_ATOM            atoms[45]
#define _MOTIF_WM_HINTS_ATOM                   atoms[46]
#define _NET_SUPPORTED_ATOM                    atoms[47]
#define _NET_FRAME_EXTENTS_ATOM                atoms[48]
#define _NET_REQUEST_FRAME_EXTENTS_ATOM        atoms[49]
#define _KDE_NET_WM_FRAME_STRUT_ATOM           atoms[50]
#define _WIN_SUPPORTING_WM_CHECK_ATOM          atoms[51]
#define _NET_SUPPORTING_WM_CHECK_ATOM          atoms[52]
#define _NET_DESKTOP_NAMES_ATOM                atoms[53]
#define _NET_CURRENT_DESKTOP_ATOM              atoms[54]
#define _NET_NUMBER_OF_DESKTOPS_ATOM           atoms[55]
#define _NET_CLIENT_LIST_STACKING_ATOM         atoms[56]
#define _NET_ACTIVE_WINDOW_ATOM                atoms[57]
#define _WIN_LAYER_ATOM                        atoms[58]
#define _WINDOWMAKER_WM_PROTOCOLS_ATOM         atoms[59]
#define _WINDOWMAKER_NOTICEBOARD_ATOM          atoms[60]
#define _WINDOWMAKER_ICON_TILE_ATOM            atoms[61]
#define _WINDOWMAKER_WM_FUNCTION_ATOM          atoms[62]
#define _RGBA_IMAGE_ATOM                       atoms[63]
#define _GNUSTEP_WM_MINIATURIZE_WINDOW_ATOM    atoms[64]
#define _GNUSTEP_WM_HIDE_APP_ATOM              atoms[65]
#define _GNUSTEP_WM_ATTR_ATOM                  atoms[66]
#define _GNUSTEP_TITLEBAR_STATE_ATOM           atoms[67]
#define _GNUSTEP_FRAME_OFFSETS_ATOM            atoms[68]
#define WM_IGNORE_FOCUS_EVENTS_ATOM            atoms[69]

/*
 * Frame offsets for window inside parent decoration window.
 */
typedef struct {
  short	l;	// offset from left
  short	r;	// offset from right
  short	t;	// offset from top
  short	b;	// offset from bottom
  BOOL	known;	// obtained from Reparent event or just guessed?
} Offsets;

/*
 * Structure containing ivars that are common to all X backend contexts.
 */
struct XGGeneric {
  int   		wm;
  struct {
    unsigned	useWindowMakerIcons:1;
    unsigned    appOwnsMiniwindow:1;
    unsigned    doubleParentWindow:1;
  } flags;
  // Time of last X event
  Time			lastTime;
  // Approximate local time for last X event, used to decide 
  // if the last X event time is still valid.
  NSTimeInterval lastTimeStamp;
  // last reference time on X server, used to prevent time drift between
  // local machine and X server.
  Time baseXServerTime;
  Time			lastClick;
  Window		lastClickWindow;
  int			lastClickX;
  int			lastClickY;
  Time			lastMotion;
  // Name for application root window.
  char			*rootName;
  long			currentFocusWindow;
  long			desiredFocusWindow;
  unsigned long		focusRequestNumber;
  unsigned char		lMouse;
  unsigned char		mMouse;
  unsigned char		rMouse;
  unsigned char		upMouse;
  unsigned char		downMouse;
  unsigned char		scrollLeftMouse;
  unsigned char		scrollRightMouse;
  int			lMouseMask;
  int			mMouseMask;
  int			rMouseMask;
  Window		appRootWindow;
  void			*cachedWindow;	// last gswindow_device_t used.
  Offsets		offsets[16];
  Atom atoms[sizeof(atom_names)/sizeof(char*)];
};

/* GNOME Window layers */
#define WIN_LAYER_DESKTOP                0
#define WIN_LAYER_BELOW                  2
#define WIN_LAYER_NORMAL                 4
#define WIN_LAYER_ONTOP                  6
#define WIN_LAYER_DOCK                   8
#define WIN_LAYER_ABOVE_DOCK             10
#define WIN_LAYER_MENU                   12

/* NET WM State */
#define _NET_WM_STATE_REMOVE        0    /* remove/unset property */
#define _NET_WM_STATE_ADD           1    /* add/set property */
#define _NET_WM_STATE_TOGGLE        2    /* toggle property  */

#endif

