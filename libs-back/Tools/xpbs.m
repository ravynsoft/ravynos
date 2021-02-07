/*
   xpbs.m

   GNUstep pasteboard server - X extension

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: April 1999

   This file is part of the GNUstep Project

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 3
   of the License, or (at your option) any later version.
    
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public  
   License along with this library; see the file COPYING.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#include "config.h"

#include <Foundation/Foundation.h>
#include <Foundation/NSUserDefaults.h>
#include <AppKit/NSPasteboard.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <x11/xdnd.h>
#if HAVE_XFIXES
#include <X11/extensions/Xfixes.h>
#endif

/*
 *	Non-predefined atoms that are used in the X selection mechanism
 */
static char *atom_names[] = {
  "CHARACTER_POSITION",
  "CLIENT_WINDOW",
  "HOST_NAME",
  "HOSTNAME",
  "LENGTH",
  "LIST_LENGTH",
  "NAME",
  "OWNER_OS",
  "SPAN",
  "TARGETS",
  "TIMESTAMP",
  "USER",
  "TEXT",
  "NULL",
  "FILE_NAME",
  "CLIPBOARD",
  "UTF8_STRING",
  "MULTIPLE",
  "COMPOUND_TEXT",
  "INCR",

  // some MIME types
  "text/plain",
  "text/uri-list",
  "application/postscript",
  "text/tab-separated-values",
  "text/richtext",
  "image/tiff",
  "application/octet-stream",
  "application/x-rootwindow-drop",
  "application/richtext",
  "text/rtf",
  "text/html",
  "application/xhtml+xml",
  "image/png",
  "image/svg",
  "application/rtf",
  "text/richtext"
};
static Atom atoms[sizeof(atom_names)/sizeof(char*)];


/*
 * Macros to access elements in atom_names array.
 */
#define XG_CHAR_POSITION        atoms[0]
#define XG_CLIENT_WINDOW        atoms[1]
#define XG_HOST_NAME            atoms[2]
#define XG_HOSTNAME             atoms[3]
#define XG_LENGTH               atoms[4]
#define XG_LIST_LENGTH          atoms[5]
#define XG_NAME                 atoms[6]
#define XG_OWNER_OS             atoms[7]
#define XG_SPAN                 atoms[8]
#define XG_TARGETS              atoms[9]
#define XG_TIMESTAMP            atoms[10]
#define XG_USER                 atoms[11]
#define XG_TEXT                 atoms[12]
#define XG_NULL                 atoms[13]
#define XG_FILE_NAME		atoms[14]
#define XA_CLIPBOARD		atoms[15]
#define XG_UTF8_STRING		atoms[16]
#define XG_MULTIPLE		atoms[17]
#define XG_COMPOUND_TEXT	atoms[18]
#define XG_INCR         	atoms[19]
#define XG_MIME_PLAIN    	atoms[20]
#define XG_MIME_URI      	atoms[21]
#define XG_MIME_PS      	atoms[22]
#define XG_MIME_TSV      	atoms[23]
#define XG_MIME_RICHTEXT  atoms[24]
#define XG_MIME_TIFF      atoms[25]
#define XG_MIME_OCTET     atoms[26]
#define XG_MIME_ROOTWINDOW atoms[27]
#define XG_MIME_APP_RICHTEXT atoms[28]
#define XG_MIME_RTF       atoms[29]
#define XG_MIME_HTML      atoms[30]
#define XG_MIME_XHTML     atoms[31]
#define XG_MIME_PNG       atoms[32]
#define XG_MIME_SVG       atoms[33]
#define XG_MIME_APP_RTF   atoms[34]
#define XG_MIME_TEXT_RICHTEXT atoms[35]



@interface	XPbOwner : NSObject
{
  NSPasteboard	*_pb;
  NSData	*_obj;
  NSString	*_name;
  Atom		_xPb;
  Time		_waitingForSelection;
  Time		_timeOfLastAppend;
  Time		_timeOfSetSelectionOwner;
  BOOL		_ownedByOpenStep;
}

+ (XPbOwner*) ownerByXPb: (Atom)p;
+ (XPbOwner*) ownerByOsPb: (NSString*)p;
+ (void) receivedEvent: (void*)data
                  type: (RunLoopEventType)type
                 extra: (void*)extra
               forMode: (NSString*)mode;
+ (NSDate*) timedOutEvent: (void*)data
                     type: (RunLoopEventType)type
                  forMode: (NSString*)mode;
+ (void) xEvent: (XEvent *)xEvent;
+ (void) xPropertyNotify: (XPropertyEvent*)xEvent;
+ (void) xSelectionClear: (XSelectionClearEvent*)xEvent;
+ (void) xSelectionNotify: (XSelectionEvent*)xEvent;
+ (void) xSelectionRequest: (XSelectionRequestEvent*)xEvent;

- (NSData*) data;
- (id) initWithXPb: (Atom)x osPb: (NSPasteboard*)o;
- (BOOL) ownedByOpenStep;
- (NSPasteboard*) osPb;
- (void) pasteboardChangedOwner: (NSPasteboard*)sender;
- (void) pasteboard: (NSPasteboard*)pb provideDataForType: (NSString*)type;
- (void) setData: (NSData*)obj;
- (void) setOwnedByOpenStep: (BOOL)flag;
- (void) setTimeOfLastAppend: (Time)when;
- (void) setWaitingForSelection: (Time)when;
- (Time) timeOfLastAppend;
- (Time) waitingForSelection;
- (Atom) xPb;
- (void) xSelectionClear;
- (void) xSelectionNotify: (XSelectionEvent*)xEvent;
- (void) xSelectionRequest: (XSelectionRequestEvent*)xEvent;
#if HAVE_XFIXES
+ (void) xFixesSelectionNotify: (XFixesSelectionNotifyEvent*)xEvent;
#endif
- (BOOL) xProvideSelection: (XSelectionRequestEvent*)xEvent;
- (Time) xTimeByAppending;
- (BOOL) xSendData: (unsigned char*) data format: (int) format 
	     items: (int) numItems type: (Atom) xType
		to: (Window) window property: (Atom) property;
@end



// Special subclass for the drag pasteboard
@interface	XDragPbOwner : XPbOwner
{
}
@end



/*
 *	The display we are using - everything refers to it.
 */
static Display		*xDisplay;
static Window		xRootWin;
static Window		xAppWin;
static NSMapTable	*ownByX;
static NSMapTable	*ownByO;
static NSString		*xWaitMode = @"XPasteboardWaitMode";
static int              xFixesEventBase;

@implementation	XPbOwner

+ (BOOL) initializePasteboard
{
  XPbOwner *o;
  NSPasteboard *p;
  Atom generalPb, selectionPb;
  NSRunLoop *l = [NSRunLoop currentRunLoop];
  int desc;

  ownByO = NSCreateMapTable(NSObjectMapKeyCallBacks,
                  NSNonOwnedPointerMapValueCallBacks, 0);
  ownByX = NSCreateMapTable(NSIntMapKeyCallBacks,
                  NSNonOwnedPointerMapValueCallBacks, 0);

  xDisplay = XOpenDisplay(NULL);
  if (xDisplay == 0)
    {
      NSLog(@"Unable to open X display - no X interoperation available");
      return NO;
    }

  /*
   * Set up atoms for use in X selection mechanism.
   */
#ifdef HAVE_XINTERNATOMS
   XInternAtoms(xDisplay, atom_names, sizeof(atom_names)/sizeof(char*),
                False, atoms);
#else
   {
     int atomCount;

     for (atomCount = 0; atomCount < sizeof(atom_names)/sizeof(char*); atomCount++)
       atoms[atomCount] = XInternAtom(xDisplay, atom_names[atomCount], False);
   }
#endif

  xRootWin = RootWindow(xDisplay, DefaultScreen(xDisplay));
  xAppWin = XCreateSimpleWindow(xDisplay, xRootWin,
                                0, 0, 100, 100, 1, 1, 0L);
  /*
   * Add the X descriptor to the run loop so we get callbacks when
   * X events arrive.
   */
  desc = XConnectionNumber(xDisplay);

  [l addEvent: (void*)(gsaddr)desc
	 type: ET_RDESC
      watcher: (id<RunLoopEvents>)self
      forMode: NSDefaultRunLoopMode];

  [l addEvent: (void*)(gsaddr)desc
         type: ET_RDESC
      watcher: (id<RunLoopEvents>)self
      forMode: NSConnectionReplyMode];

  [l addEvent: (void*)(gsaddr)desc
         type: ET_RDESC
      watcher: (id<RunLoopEvents>)self
      forMode: xWaitMode];

  XSelectInput(xDisplay, xAppWin, PropertyChangeMask);

#if HAVE_XFIXES
  {
    int error;

    // Subscribe to notifications of when the X clipboard changes,
    // so we can invalidate our cached list of types on it.
    //
    // FIXME: If we don't have Xfixes, we should really set up a polling timer.
    
    if (XFixesQueryExtension(xDisplay, &xFixesEventBase, &error))
      {
       XFixesSelectSelectionInput(xDisplay, xAppWin, XA_CLIPBOARD,
                                  XFixesSetSelectionOwnerNotifyMask |
                                  XFixesSelectionWindowDestroyNotifyMask |
                                  XFixesSelectionClientCloseNotifyMask );
       XFixesSelectSelectionInput(xDisplay, xAppWin, XA_PRIMARY,
                                  XFixesSetSelectionOwnerNotifyMask |
                                  XFixesSelectionWindowDestroyNotifyMask |
                                  XFixesSelectionClientCloseNotifyMask);
       // FIXME: Also handle the dnd pasteboard
       NSDebugLLog(@"Pbs", @"Subscribed to XFixes notifications");
      }
  }
#endif

  XFlush(xDisplay);

  /*
   * According to the new open desktop specification
   * http://www.freedesktop.org/standards/clipboards-spec/clipboards.txt
   * these two pasteboards should be switched around. That is,
   * general should be XA_CLIPBOARD and selection XA_PRIMARY.
   * The problem is that most X programs still use the old way.
   * For these environments we offer a switch to the old mode.
   */
  if ([[NSUserDefaults standardUserDefaults] boolForKey: @"GSOldClipboard"])
    {
      generalPb = XA_PRIMARY;
      selectionPb = XA_CLIPBOARD;
    }
  else
    {
      generalPb = XA_CLIPBOARD;
      selectionPb = XA_PRIMARY;
    }
  /*
   * For the general and the selection pasteboard we establish an initial
   * owner that is the X selection system.  In this way, any X window
   * selection already active will be available to the GNUstep system.
   * These objects are not released!
   */
  p = [NSPasteboard generalPasteboard];
  o = [[XPbOwner alloc] initWithXPb: generalPb osPb: p];
  [o xSelectionClear];

  p = [NSPasteboard pasteboardWithName: @"Selection"];
  o = [[XPbOwner alloc] initWithXPb: selectionPb osPb: p];
  [o xSelectionClear];
      
  p = [NSPasteboard pasteboardWithName: @"Secondary"];
  o = [[XPbOwner alloc] initWithXPb: XA_SECONDARY osPb: p];
  [o xSelectionClear];

  // Call this to get the class initialisation
  [XDragPbOwner class];

  return YES;
}

+ (XPbOwner*) ownerByOsPb: (NSString*)p
{
  return (XPbOwner*)NSMapGet(ownByO, (void*)(gsaddr)p);
}

+ (XPbOwner*) ownerByXPb: (Atom)x
{
  return (XPbOwner*)NSMapGet(ownByX, (void*)(gsaddr)x);
}


/*
 *	This is the event handler called by the runloop when the X descriptor
 *	has data available to read.
 */
+ (void) receivedEvent: (void*)data
                  type: (RunLoopEventType)type
                 extra: (void*)extra
               forMode: (NSString*)mode
{
  int		count;

  NSAssert(type == ET_RDESC, NSInternalInconsistencyException);

  while ((count = XPending(xDisplay)) > 0)
    {
#if 0
      /* Don't attempt to be smart here. We may enter this method recursively
       * when further data is requested while processing this event, which
       * means that the count will no longer be correct when returning to the
       * outer invocation.
       */
      while (count-- > 0)
#endif
        {
          XEvent	xEvent;

          XNextEvent(xDisplay, &xEvent);
	  [self xEvent: &xEvent];
        }
    }
}

/*
 *	This handler called if an operation times out - never happens 'cos we
 *	don't supply any timeouts - included for protocol conformance.
 */
+ (NSDate*) timedOutEvent: (void*)data
                     type: (RunLoopEventType)type
                  forMode: (NSString*)mode
{
  return nil;
}

#define FULL_LENGTH 8192L	/* Amount to read */

+ (void) xEvent: (XEvent *)xEvent
{
  switch (xEvent->type)
    {
    case PropertyNotify:
      NSDebugLLog(@"Pbs", @"PropertyNotify.");
      [self xPropertyNotify: &xEvent->xproperty];
      break;
                
    case SelectionNotify:
      NSDebugLLog(@"Pbs", @"SelectionNotify.");
      [self xSelectionNotify: &xEvent->xselection];
      break;

    case SelectionClear:
      NSDebugLLog(@"Pbs", @"SelectionClear.");
      [self xSelectionClear: &xEvent->xselectionclear];
      break;

    case SelectionRequest:
      NSDebugLLog(@"Pbs", @"SelectionRequest.");
      [self xSelectionRequest: &xEvent->xselectionrequest];
      break;

    default:
#if HAVE_XFIXES
      if (xEvent->type == xFixesEventBase + XFixesSelectionNotify)
       {
         [self xFixesSelectionNotify: (XFixesSelectionNotifyEvent*)xEvent];
         break;
       }
#endif

      NSDebugLLog(@"Pbs", @"Unexpected X event.");
      break;
    }
}

+ (void) xSelectionClear: (XSelectionClearEvent*)xEvent
{
  XPbOwner	*o;

  o = [self ownerByXPb: xEvent->selection];
  if (o == nil)
    {
      char *name = XGetAtomName(xDisplay, xEvent->selection);

      NSDebugLLog(@"Pbs", @"Selection clear for unknown selection - '%s'.",
                  name);
      XFree(name);
      return;
    }

  if (xEvent->window != (Window)xAppWin)
    {
      NSDebugLLog(@"Pbs", @"Selection clear for wrong (not our) window.");
      return;
    }

  [o xSelectionClear];
}

+ (void) xPropertyNotify: (XPropertyEvent*)xEvent
{
  XPbOwner	*o;

  o = [self ownerByXPb: xEvent->atom];
  if (o == nil)
    {
      char *name = XGetAtomName(xDisplay, xEvent->atom);
      NSDebugLLog(@"Pbs", @"Property notify for unknown property - '%s'.",
                  name);
      XFree(name);
      return;
    }

  if (xEvent->window != (Window)xAppWin)
    {
      NSDebugLLog(@"Pbs", @"Property notify for wrong (not our) window.");
      return;
    }

  if (xEvent->time != 0)
    {
      [o setTimeOfLastAppend: xEvent->time];
    }
}

+ (void) xSelectionNotify: (XSelectionEvent*)xEvent
{
  XPbOwner *o;

  o = [self ownerByXPb: xEvent->selection];
  if (o == nil)
    {
      char *name = XGetAtomName(xDisplay, xEvent->selection);
      NSDebugLLog(@"Pbs", @"Selection notify for unknown selection - '%s'.",
                  name);
      XFree(name);
      return;
    }

  if (xEvent->requestor != (Window)xAppWin)
    {
      NSDebugLLog(@"Pbs", @"Selection notify for wrong (not our) window.");
      return;
    }
  else
    {
      char *sel_name = XGetAtomName(xDisplay, xEvent->selection);
      char *pro_name;
      
      if (xEvent->property == None)
        pro_name = NULL;
      else
        pro_name = XGetAtomName(xDisplay, xEvent->property);

      NSDebugLLog(@"Pbs", @"Selection (%s) notify - '%s'.", sel_name, 
                  pro_name? pro_name : "None");
      XFree(sel_name);
      if (pro_name)
        XFree(pro_name);
    }

  [o xSelectionNotify: xEvent];
}

#if HAVE_XFIXES
+ (void) xFixesSelectionNotify: (XFixesSelectionNotifyEvent*)xEvent
{
  XPbOwner *o = [self ownerByXPb: xEvent->selection];
  
  if (o != nil)
    {
      if (xEvent->owner != (Window)xAppWin)
       {
         NSDebugLLog(@"Pbs", @"Notified that selection %@ changed", [[o osPb] name]);
	 // FIXME: Invalidate the cached types in the pasteboard since they are no longer valid
       }
      else
       {
	 // The notification is telling us that we became the selection owner,
	 // which we already know about since we must have initiated that change.
       }
    }
}
#endif

+ (void) xSelectionRequest: (XSelectionRequestEvent*)xEvent
{
  XPbOwner		*o;

  o = [self ownerByXPb: xEvent->selection];
  if (o == nil)
    {
      char *name = XGetAtomName(xDisplay, xEvent->selection);
      NSDebugLLog(@"Pbs", @"Selection request for unknown selection - '%s'.",
                  name);
      XFree(name);
      return;
    }

  if (xEvent->requestor == (Window)xAppWin)
    {
      NSDebugLLog(@"Pbs", @"Selection request for wrong (our) window.");
      return;
    }

  if (xEvent->property == None)
    {
      NSDebugLLog(@"Pbs", @"Selection request without reply property set.");
      return;
    }

  [o xSelectionRequest: xEvent];
}

- (NSData*) data
{
  return _obj;
}

- (void) dealloc
{
  RELEASE(_pb);
  RELEASE(_obj);
  /*
   * Remove self from map of X pasteboard owners.
   */
  NSMapRemove(ownByX, (void*)(gsaddr)_xPb);
  NSMapRemove(ownByO, (void*)(gsaddr)_name);
  [super dealloc];
}

- (id) initWithXPb: (Atom)x osPb: (NSPasteboard*)o
{
  _pb = RETAIN(o);
  _name = [_pb name];
  _xPb = x;
  /*
   * Add self to map of all X pasteboard owners.
   */
  NSMapInsert(ownByX, (void*)(gsaddr)_xPb, (void*)(gsaddr)self);
  NSMapInsert(ownByO, (void*)(gsaddr)_name, (void*)(gsaddr)self);
  return self;
}

- (NSPasteboard*) osPb
{
  return _pb;
}

- (BOOL) ownedByOpenStep
{
  return _ownedByOpenStep;
}

- (void) pasteboardChangedOwner: (NSPasteboard*)sender
{
  Window	w;
  /*
   *	If this gets called, a GNUstep object has grabbed the pasteboard
   *	or has changed the types of data available from the pasteboard
   *	so we must tell the X server that we have the current selection.
   *	To conform to ICCCM we need to specify an up-to-date timestamp.
   */

  // FIXME: See note in -xSelectionClear:. This method is called by
  // -[NSPasteboard declareTypes:owner:], but we might not want
  // GNUstep to take ownership. (e.g. suppose selection ownership changes from
  // gnome-terminal to OpenOffice.org. But, we will still need to update the
  // types available on the pasteboard in case a GNUstep app wants to read from
  // the pasteboard.)

  _timeOfSetSelectionOwner = [self xTimeByAppending];
  XSetSelectionOwner(xDisplay, _xPb, xAppWin, _timeOfSetSelectionOwner);

  w = XGetSelectionOwner(xDisplay, _xPb);
  if (w != xAppWin)
    {
      NSLog(@"Failed to set X selection owner to the pasteboard server.");
    }
  else
    {
      [self setOwnedByOpenStep: YES];
    }
}

- (void) requestData: (Atom)xType 
{
  Time whenRequested;

  /*
   * Do a nul append to a property to get a timestamp, if it returns the
   * 'CurrentTime' constant then we haven't been able to get one.
   */
  whenRequested = [self xTimeByAppending];
  if (whenRequested != CurrentTime)
    {
      NSDate *limit;
      
      /*
       * Ok - we got a timestamp, so we can ask the selection system for
       * the pasteboard data that was/is valid for that time.
       * Ask the X system to provide the pasteboard data in the
       * appropriate property of our application root window.
       */
      XConvertSelection(xDisplay, [self xPb], xType,
                        [self xPb], xAppWin, whenRequested);
      XFlush(xDisplay);
      
      /*
       * Run an event loop to read X events until we have aquired the
       * pasteboard data we need.
       * However, before entering the event loop be sure to drain the X event
       * queue, since XFlush may read events from the X connection and thus
       * may have enqueued our expected property change notification in the
       * X event queue already.
       */
      limit = [NSDate dateWithTimeIntervalSinceNow: 20.0];
      [self setWaitingForSelection: whenRequested];
      while (XQLength(xDisplay) > 0 &&
             [self waitingForSelection] == whenRequested)
        {
          XEvent xEvent;

          XNextEvent(xDisplay, &xEvent);
          [[self class] xEvent: &xEvent];
        }
      while ([self waitingForSelection] == whenRequested)
        {
          [[NSRunLoop currentRunLoop] runMode: xWaitMode
                                      beforeDate: limit];
          if ([limit timeIntervalSinceNow] <= 0.0)
            break;	/* Timeout */
        }
      if ([self waitingForSelection] != 0)
        {
          char *name = XGetAtomName(xDisplay, xType);

          [self setWaitingForSelection: 0];
          NSLog(@"Timed out waiting for X selection '%s'", name);
          XFree(name);
        }
    }
}

- (void) pasteboard: (NSPasteboard*)pb provideDataForType: (NSString*)type
{
  [self setData: nil];

  /*
   *	If this gets called, a GNUstep object wants the pasteboard contents
   *	and a plain old X application is providing them, so we must grab
   *	the info.
   */
  if ([type isEqual: NSStringPboardType])
    {
      [self requestData: XG_UTF8_STRING];
      if ([self data] == nil)
        [self requestData: XA_STRING];
      if ([self data] == nil)
        [self requestData: XG_TEXT];
    }
  else if ([type isEqual: NSFilenamesPboardType])
    {

      [self requestData: XG_FILE_NAME];
    }
  else if ([type isEqual: NSRTFPboardType])
    {
      [self requestData: XG_MIME_RTF];
      if ([self data] == nil)
        [self requestData: XG_MIME_APP_RTF];
      if ([self data] == nil)
        [self requestData: XG_MIME_TEXT_RICHTEXT];
    }
  else if ([type isEqual: NSTIFFPboardType])
    {
      [self requestData: XG_MIME_TIFF];
    }
  // FIXME: Support more types
  else
    {
      NSDebugLLog(@"Pbs", @"Request for non-string info from X pasteboard: %@", type);
    }
  [pb setData: [self data] forType: type];
}

- (void) setData: (NSData*)obj
{
  ASSIGN(_obj, obj);
}

- (void) setOwnedByOpenStep: (BOOL)f
{
  _ownedByOpenStep = f;
}

- (void) setTimeOfLastAppend: (Time)when
{
  _timeOfLastAppend = when;
}

- (void) setWaitingForSelection: (Time)when
{
  _waitingForSelection = when;
}

- (Time) timeOfLastAppend
{
  return _timeOfLastAppend;
}

- (Time) waitingForSelection
{
  return _waitingForSelection;
}

- (Atom) xPb
{
  return _xPb;
}

static BOOL appendFailure;
static int
xErrorHandler(Display *d, XErrorEvent *e)
{
  appendFailure = YES;
  return 0;
}

/*
 * Check to see what types of data the selection owner is
 * making available, and declare them all.
 * If this fails, declare string data.
 */
- (NSArray*) availableTypes
{
  NSMutableArray *types;
  NSData *data;
  unsigned int count;
  unsigned int i;
  Atom *targets;

  NSDebugLLog(@"Pbs", @"%@ availableTypes called", [[self osPb] name]);

  [self setData: nil];
  [self requestData: XG_TARGETS];
  data = [self data];
  if (data == nil)
    return [NSArray arrayWithObject: NSStringPboardType];

  count = [data length] / sizeof(Atom);
  targets = (Atom*)[data bytes];
  types = [NSMutableArray arrayWithCapacity: count];

  NSDebugLLog(@"Pbs", @"%@ availableTypes: %d types available", [[self osPb] name], count);

  for (i = 0; i < count; i++)
    {
      Atom type;

      type = targets[i];

      if ((type == XG_UTF8_STRING) 
          || (type == XA_STRING)
          || (type == XG_TEXT)
          || (type == XG_MIME_PLAIN))
        {
          [types addObject: NSStringPboardType];
        }
      else if (type == XG_FILE_NAME)
        {
          [types addObject: NSFilenamesPboardType];
        }
      else if ((type == XG_MIME_RTF)
               || (type == XG_MIME_APP_RTF)
	       || (type == XG_MIME_TEXT_RICHTEXT))
        {
          [types addObject: NSRTFPboardType];
        }
      else if (type == XG_MIME_TIFF)
        {
          [types addObject: NSTIFFPboardType];
        }
      else if ((type == XG_TARGETS)
               || (type == XG_TIMESTAMP)
               || (type == XG_OWNER_OS)
               || (type == XG_USER)
               || (type == XG_HOST_NAME)
               || (type == XG_HOSTNAME)
               || (type == XG_MULTIPLE))
        {
            // Standard types
        }
      // FIXME: Support more types
      else
	{
          char *name = XGetAtomName(xDisplay, type);

          // FIXME: We should rather add this type to the
          // pasteboard as a string.
          NSDebugLLog(@"Pbs", @"Unsupported selection type '%s' available", 
                      name);
          XFree(name);
	}
    }

  return types;
}

/*
 * Should be called when ever the clipboard contents changes.
 * Currently it gets called when GNUstep looses the ownership 
 * of the clipboard.
 */
- (void) xSelectionClear
{
  // FIXME: This will cause -pasteboardChangedOwner: to be called, which will
  // take ownership of the X selection. That is probably wrong...
  [_pb declareTypes: [self availableTypes] owner: self];
  [self setOwnedByOpenStep: NO];
}

- (NSMutableData*) getSelectionData: (XSelectionEvent*)xEvent
                               type: (Atom*)type
{
  int		status;
  unsigned char	*data;
  long long_offset = 0L;
  long long_length = FULL_LENGTH;
  Atom req_type = AnyPropertyType;
  Atom actual_type;
  int		actual_format;
  unsigned long	bytes_remaining;
  unsigned long	number_items;
  NSMutableData	*md = nil;

  /*
   * Read data from property identified in SelectionNotify event.
   */
  do
    {
      status = XGetWindowProperty(xDisplay,
                                  xEvent->requestor,
                                  xEvent->property,
                                  long_offset,         // offset
                                  long_length,
                                  False,               // Aug 2011 - changed to False (don't delete property)
                                  req_type,
                                  &actual_type,
                                  &actual_format,
                                  &number_items,
                                  &bytes_remaining,
                                  &data);
      
      if ((status == Success) && (number_items > 0))
        {
          long count;
	  if (actual_type == XA_ATOM)
	    {
	      // xlib will report an actual_format of 32, even if
	      // data contains an array of 64-bit Atoms
	      count = number_items * sizeof(Atom);
	    }
	  else
	    {
	      count = number_items * actual_format / 8;
	    }
            
          if (md == nil)
            {
              md = [[NSMutableData alloc] initWithBytes: (void *)data
                                          length: count];  
              req_type = actual_type;
            }
          else
            {
              if (req_type != actual_type)
                {
                  char *req_name = XGetAtomName(xDisplay, req_type);
                  char *act_name = XGetAtomName(xDisplay, actual_type);
                  
                  NSLog(@"Selection changed type from %s to %s.", 
                        req_name, act_name);
                  XFree(req_name);
                  XFree(act_name);
                  RELEASE(md);
                  return nil;
                }
              [md appendBytes: (void *)data length: count];
            }
          
          long_offset += count / 4;
          if (data)
            {
              XFree(data);
            }
        }
    }
  while ((status == Success) && (bytes_remaining > 0));

  if (status == Success)
    {
      *type = actual_type;
      return AUTORELEASE(md);
    }
  else
    {
      RELEASE(md);
      return nil;
    }
}

- (void) xSelectionNotify: (XSelectionEvent*)xEvent
{
  Atom actual_type;
  NSMutableData	*md = nil;

  if (xEvent->property == (Atom)None)
    {
      NSDebugLLog(@"Pbs", @"Owning program failed to convert data.");
      [self setWaitingForSelection: 0];
      return;
    }

  if ([self waitingForSelection] > xEvent->time)
    {
      NSDebugLLog(@"Pbs", @"Unexpected selection notify - time %lu.", xEvent->time);
      return;
    }
  [self setWaitingForSelection: 0];

  md = [self getSelectionData: xEvent type: &actual_type];

  if (md != nil)
    {
      if (actual_type == XG_INCR)
        {
          XEvent event;
          NSMutableData	*imd = nil;
          BOOL wait = YES;
          
          md = nil;
          while (wait)
            {
              XNextEvent(xDisplay, &event);
              
              if (event.type == PropertyNotify)
                {
                  if (event.xproperty.state != PropertyNewValue) continue;
                  
                  imd = [self getSelectionData: xEvent type: &actual_type];
                  if (imd != nil)
                    {
                      if (md == nil)
                        {
                          md = imd;
                        }
                      else
                        {
                          [md appendData: imd];
                        }
                    }
                  else
                    {
                      wait = NO;
                    }
                }
            }
        }
    }
  
  if (md != nil)
    {
      // Convert data to text string.
      if (actual_type == XG_UTF8_STRING)
        {
          NSString	*s;
          NSData	*d;
          
          s = [[NSString alloc] initWithData: md
                                encoding: NSUTF8StringEncoding];
          if (s != nil)
            {
              d = [NSSerializer serializePropertyList: s];
              RELEASE(s);
              [self setData: d];
            }
        }
      else if ((actual_type == XA_STRING)
               || (actual_type == XG_TEXT)
               || (actual_type == XG_MIME_PLAIN))
        {
          NSString	*s;
          NSData	*d;
          
          s = [[NSString alloc] initWithData: md
                                encoding: NSISOLatin1StringEncoding];
          if (s != nil)
            {
              d = [NSSerializer serializePropertyList: s];
              RELEASE(s);
              [self setData: d];
            }
        }
      else if (actual_type == XG_FILE_NAME)
        {
          NSArray *names;
          NSData *d;
          NSString *s;
          NSURL *url;

          s = [[NSString alloc] initWithData: md
                                encoding: NSUTF8StringEncoding];
          url = [[NSURL alloc] initWithString: s];
          RELEASE(s);
          if ([url isFileURL])
            {
              s = [url path];
              names = [NSArray arrayWithObject: s];
              d = [NSSerializer serializePropertyList: names];
              [self setData: d];
            }
          RELEASE(url);
        }
      else if ((actual_type == XG_MIME_RTF)
               || (actual_type == XG_MIME_APP_RTF)
	       || (actual_type == XG_MIME_TEXT_RICHTEXT))
        {
          [self setData: md];
        }
      else if (actual_type == XG_MIME_TIFF)
        {
          [self setData: md];
        }
      else if (actual_type == XA_ATOM)
        {
          // Used when requesting TARGETS to get available types
          [self setData: md];
        }
      else
        {
          char *name = XGetAtomName(xDisplay, actual_type);
          
          NSDebugLLog(@"Pbs", @"Unsupported data type '%s' from X selection.", 
                      name);
          XFree(name);
        }
    }
}

- (void) xSelectionRequest: (XSelectionRequestEvent*)xEvent
{
  XSelectionEvent	notify_event;
  BOOL			status;

  status = [self xProvideSelection: xEvent];
  if (!status)
    NSDebugLLog(@"Pbs", @"Could not provide selection upon request.");

  /*
   * Set up the selection notify information from the event information
   * so we comply with the ICCCM.
   */
  notify_event.display    = xEvent->display;
  notify_event.type       = SelectionNotify;
  notify_event.requestor  = xEvent->requestor;
  notify_event.selection  = xEvent->selection;
  notify_event.target     = xEvent->target;
  notify_event.time       = xEvent->time;
  notify_event.send_event = True;
  if (xEvent->property == None)
    {
      notify_event.property = xEvent->target;
    }
  else 
    {
      notify_event.property = xEvent->property;
    }

  /*
   * If for any reason we cannot provide the data to the requestor, we must
   * send a selection notify with a property of 'None' so that the requestor
   * knows the request failed.
   */
  if (status == NO)
    notify_event.property = None;

  XSendEvent(xEvent->display, xEvent->requestor, False, 0L,
    (XEvent*)&notify_event);
}

- (BOOL) xProvideSelection: (XSelectionRequestEvent*)xEvent
{
  NSArray	*types = [_pb types];
  Atom		xType = XG_NULL;
  unsigned char	*data = 0;
  int		format = 0;
  int		numItems = 0;

  if (xEvent->target == XG_TARGETS)
    {
      unsigned	numTypes = 0;
      // ATTENTION: Increase this array when adding more types
      Atom	xTypes[16];
      
      /*
       * The requestor wants a list of the types we can supply it with.
       * We can supply one or more types of data to the requestor so
       * we will give it a list of the types supported.
       */
      xTypes[numTypes++] = XG_TARGETS;
      xTypes[numTypes++] = XG_TIMESTAMP;
      xTypes[numTypes++] = XG_MULTIPLE;
      xTypes[numTypes++] = XG_USER;
      xTypes[numTypes++] = XG_HOST_NAME;
      xTypes[numTypes++] = XG_OWNER_OS;
      // FIXME: ICCCM requires even more types from us.
      
      if ([types containsObject: NSStringPboardType])
        {
          xTypes[numTypes++] = XG_UTF8_STRING;
          xTypes[numTypes++] = XG_COMPOUND_TEXT;
          xTypes[numTypes++] = XA_STRING;
          xTypes[numTypes++] = XG_TEXT;
        }
      
      if ([types containsObject: NSFilenamesPboardType])
        {
          xTypes[numTypes++] = XG_FILE_NAME;
        }
      
      if ([types containsObject: NSRTFPboardType])
        {
          xTypes[numTypes++] = XG_MIME_RTF;
          xTypes[numTypes++] = XG_MIME_APP_RTF;
	  xTypes[numTypes++] = XG_MIME_TEXT_RICHTEXT;
        }

      if ([types containsObject: NSTIFFPboardType])
        {
          xTypes[numTypes++] = XG_MIME_TIFF;
        }

      xType = XA_ATOM;
      format = 32;
      numItems = numTypes;
      data = malloc(numTypes*sizeof(Atom));
      memcpy(data, xTypes, numTypes*sizeof(Atom));
    }
  else if (xEvent->target == XG_TIMESTAMP)
    {
      xType = XA_INTEGER;
      format = 32;
      numItems = 1;
      data = malloc(sizeof(int));
      memcpy(data, &_timeOfSetSelectionOwner, sizeof(int));      
    }
  else if (xEvent->target == XG_USER)
    {
      NSString *s = NSUserName();
      NSData *d = nil;
      
      xType = XG_TEXT;
      format = 8;
      d = [s dataUsingEncoding: NSISOLatin1StringEncoding];
      if (d != nil)
        {
          numItems = [d length];
          data = malloc(numItems + 1);
          if (data)
            [d getBytes: data];
        }
    }
  else if (xEvent->target == XG_OWNER_OS)
    {
      NSString *s = [[NSProcessInfo processInfo] operatingSystemName];
      NSData *d = nil;
      
      xType = XG_TEXT;
      format = 8;
      d = [s dataUsingEncoding: NSISOLatin1StringEncoding];
      if (d != nil)
        {
          numItems = [d length];
          data = malloc(numItems + 1);
          if (data)
            [d getBytes: data];
        }
    }
  else if ((xEvent->target == XG_HOST_NAME) 
           || (xEvent->target == XG_HOSTNAME))
    {
      NSString *s = [[NSProcessInfo processInfo] hostName];
      NSData *d = nil;
      
      xType = XG_TEXT;
      format = 8;
      d = [s dataUsingEncoding: NSISOLatin1StringEncoding];
      if (d != nil)
        {
          numItems = [d length];
          data = malloc(numItems + 1);
          if (data)
            [d getBytes: data];
        }
    }
  else if (xEvent->target == AnyPropertyType)
    {
      /*
       * The requestor will accept any type of data - so we use the first
       * OpenStep type that corresponds to a known X type.
       */
      if ([types containsObject: NSStringPboardType])
        {
          xEvent->target = XG_UTF8_STRING;
          [self xProvideSelection: xEvent];
        }
      else if ([types containsObject: NSFilenamesPboardType])
        {
          xEvent->target = XG_FILE_NAME;
          [self xProvideSelection: xEvent];
        }
      else if ([types containsObject: NSRTFPboardType])
        {
          xEvent->target = XG_MIME_RTF;
          [self xProvideSelection: xEvent];
        }
      else if ([types containsObject: NSTIFFPboardType])
        {
          xEvent->target = XG_MIME_TIFF;
          [self xProvideSelection: xEvent];
        }
    }
  else if (xEvent->target == XG_MULTIPLE)
    {
      if (xEvent->property != None)
      {
        Atom *multipleAtoms= NULL;
        int actual_format;
        Atom actual_type;
        unsigned long number_items, bytes_remaining;
        int status;

        status = XGetWindowProperty(xDisplay,
                                    xEvent->requestor,
                                    xEvent->property,
                                    0, 
                                    100, 
                                    False,
                                    AnyPropertyType,
                                    &actual_type, 
                                    &actual_format,
                                    &number_items,
                                    &bytes_remaining,
                                    (unsigned char **)&multipleAtoms);
        if ((status == Success) && (bytes_remaining == 0) && 
            (actual_format == 32) && (actual_type == XA_ATOM))
          {
            int i;
            XSelectionRequestEvent requestEvent;
            
            memcpy(&requestEvent, xEvent, sizeof(XSelectionRequestEvent));
            for (i = 0; i < number_items; i += 2)
              {
                requestEvent.target= multipleAtoms[i];
                requestEvent.property= multipleAtoms[i+1];
                if (requestEvent.target != None)
                  {
                    // Recursive call to this method for each pair.
                    if (![self xProvideSelection: &requestEvent])
                      {
                        multipleAtoms[i+1]= None;
                      }
                  }
              }
            // FIXME: Should we call XChangeProperty to set the invalid types?
          }
      }
    }
  else if ((xEvent->target == XG_COMPOUND_TEXT) &&
	   [types containsObject: NSStringPboardType])
    {
      NSString *s = [_pb stringForType: NSStringPboardType];
      const char *d;
      int status;

      xType = XG_COMPOUND_TEXT;
      format = 8;
      d = [s cString];
      if (d)
        {
          char *list[]= {(char *)d, NULL};
          XTextProperty textProperty;
          
          status = XmbTextListToTextProperty(xEvent->display, list, 1,
                                             XCompoundTextStyle, &textProperty);
          if (status == Success)
            {
              NSAssert(textProperty.format == 8, @"textProperty.format == 8");
              numItems = textProperty.nitems;
              data = malloc(numItems + 1);
              memcpy(data, textProperty.value, numItems + 1);
              XFree((void *)textProperty.value);
            }
        }
    }
  else if (((xEvent->target == XG_UTF8_STRING) || 
            (xEvent->target == XA_STRING) || 
            (xEvent->target == XG_TEXT)) &&
           [types containsObject: NSStringPboardType])
    {
      NSString *s = [_pb stringForType: NSStringPboardType];
      NSData *d = nil;

      xType = xEvent->target;
      format = 8;

      /*
       * Now we know what type of data is required - so get it from the
       * pasteboard and convert to a format X can understand.
       */
      if (xType == XG_UTF8_STRING)
        {
          d = [s dataUsingEncoding: NSUTF8StringEncoding];
        }
      else if ((xType == XA_STRING) || (xType == XG_TEXT))
        {
          d = [s dataUsingEncoding: NSISOLatin1StringEncoding];
        }

      if (d != nil)
        {
          numItems = [d length];
          data = malloc(numItems + 1);
          if (data)
            [d getBytes: data];
        }
    }
  else if ((xEvent->target == XG_FILE_NAME) &&
	   [types containsObject: NSFilenamesPboardType])
    {
      NSArray	*names = [_pb propertyListForType: NSFilenamesPboardType];
      NSString *file = [[names lastObject] stringByStandardizingPath];
      NSURL *url = [[NSURL alloc] initWithScheme: NSURLFileScheme
                                  host: @"localhost"
                                  path: file];
      NSString *s = [url absoluteString];
      NSData *d;

      RELEASE(url);
      d = [s dataUsingEncoding: NSISOLatin1StringEncoding];
      xType = xEvent->target;
      format = 8;
      if (d != nil)
        {
          numItems = [d length];
          data = malloc(numItems + 1);
          if (data)
            [d getBytes: data];
        }
    }
  else if (((xEvent->target == XG_MIME_RTF) 
            || (xEvent->target == XG_MIME_APP_RTF)
	    || (xEvent->target == XG_MIME_TEXT_RICHTEXT))
           && [types containsObject: NSRTFPboardType])
    {
      NSData *d = [_pb dataForType: NSRTFPboardType];

      xType = xEvent->target;
      format = 8;
      if (d != nil)
        {
          numItems = [d length];
          data = malloc(numItems + 1);
          if (data)
            [d getBytes: data];
        }
    }
  else if ((xEvent->target == XG_MIME_TIFF) &&
	   [types containsObject: NSTIFFPboardType])
    {
      NSData *d = [_pb dataForType: NSTIFFPboardType];

      xType = xEvent->target;
      format = 8;
      if (d != nil)
        {
          numItems = [d length];
          data = malloc(numItems + 1);
          if (data)
            [d getBytes: data];
        }
    }
  // FIXME: Support more types
  else
    {
      char *name = XGetAtomName(xDisplay, xEvent->target);

      NSLog(@"Request for unsupported data type '%s'.", name);
      XFree(name);
      return NO;
    }

  return [self xSendData: data format: format items: numItems type: xType
               to: xEvent->requestor property: xEvent->property];
}

- (BOOL) xSendData: (unsigned char*) data format: (int) format 
             items: (int) numItems type: (Atom) xType
                to: (Window) window property: (Atom) property
{
  BOOL status = NO;
  
  /*
   * If we have managed to convert data of the appropritate type, we must now
   * append the data to the property on the requesting window.
   * We do this in small chunks, checking for errors, in case the window
   * manager puts a limit on the data size we can use.
   * This is not thread-safe - but I think that's a general problem with X.
   */
  if (data != 0 && numItems != 0 && format != 0)
    {
      int	(*oldHandler)(Display*, XErrorEvent*);
      int	mode = PropModeReplace;
      int	pos = 0;
      int	maxItems = 4096 * 8 / format;

      appendFailure = NO;
      oldHandler = XSetErrorHandler(xErrorHandler);

      while (appendFailure == NO && pos < numItems)
        {
          if (pos + maxItems > numItems)
            {
              maxItems = numItems - pos;
            }
          XChangeProperty(xDisplay, window, property,
                          xType, format, mode, &data[pos*format/8], maxItems);
          mode = PropModeAppend;
          pos += maxItems;
          XSync(xDisplay, False);
        }
      free(data);
      XSetErrorHandler(oldHandler);
      if (appendFailure == NO)
        {
          status = YES;
        }
    }
  return status;
}

- (Time) xTimeByAppending
{
  NSDate	*limit;
  Time		whenRequested;
  Atom		actualType = 0;
  int		actualFormat = 0;
  unsigned long	ni;
  unsigned long	ba;
  unsigned char	*pr;

  /*
   * Do a nul append to a property to get a timestamp,
   * - but first we must determine the property-type and format.
   */
  XGetWindowProperty(xDisplay, xAppWin, [self xPb], 0, 0, False,
    AnyPropertyType, &actualType, &actualFormat, &ni, &ba, &pr);
  if (pr != 0)
    XFree(pr);
    
  if (actualType == None)
    {
      /*
       * The property doesn't exist - so we will be creating a new (empty)
       * property.
       */
      actualType = XA_ATOM;
      actualFormat = 32;
    }

  XChangeProperty(xDisplay, xAppWin, [self xPb], actualType, actualFormat,
    PropModeReplace, 0, 0);
  XFlush(xDisplay);
  limit = [NSDate dateWithTimeIntervalSinceNow: 3.0];
  [self setTimeOfLastAppend: 0];
  /*
   * Run an event loop until we get a notification for our nul append.
   * this will give us an up-to-date timestamp as required by ICCCM.
   * However, before entering the event loop be sure to drain the X event
   * queue, since XFlush may read events from the X connection and thus may
   * have enqueued our expected property change notification in the X event
   * queue already.
   */
  while (XQLength(xDisplay) && [self timeOfLastAppend] == 0)
    {
      XEvent xEvent;

      XNextEvent(xDisplay, &xEvent);
      [[self class] xEvent: &xEvent];
    }
  while ([self timeOfLastAppend] == 0)
    {
      [[NSRunLoop currentRunLoop] runMode: xWaitMode
			       beforeDate: limit];
      if ([limit timeIntervalSinceNow] <= 0.0)
        break;	/* Timeout */
    }
  if ((whenRequested = [self timeOfLastAppend]) == 0)
    {
        NSLog(@"Timed out waiting for X append for %@", _name);
      whenRequested = CurrentTime;
    }
  return whenRequested;
}

@end



// This are copies of functions from XGContextEvent.m. 
// We should create a separate file for them.
static inline
Atom *
mimeTypeForPasteboardType(Display *xDisplay, NSZone *zone, NSArray *types)
{
  Atom *typelist;
  int count = [types count];
  int i;

  typelist = NSZoneMalloc(zone, (count+1) * sizeof(Atom));
  for (i = 0; i < count; i++)
    {
      NSString *mime = [NSPasteboard mimeTypeForPasteboardType: 
		       [types objectAtIndex: i]];
      typelist[i] = XInternAtom(xDisplay, [mime cString], False);
    }
  typelist[count] = 0;

  return typelist;
}

static inline
NSArray *
pasteboardTypeForMimeType(Display *xDisplay, NSZone *zone, Atom *typelist)
{
  Atom *type = typelist;
  NSMutableArray *newTypes = [[NSMutableArray allocWithZone: zone] init];

  while (*type != None)
    {
      char *s = XGetAtomName(xDisplay, *type);
      
      if (s)
	{
	  [newTypes addObject: [NSPasteboard pasteboardTypeForMimeType: 
	    [NSString stringWithCString: s]]];
	  XFree(s);
	}
    }
  
  return AUTORELEASE(newTypes);
}

static DndClass dnd;

@implementation	XDragPbOwner

+ (void) initialize
{
  if (self == [XDragPbOwner class])
    {
      NSPasteboard	*p;

      xdnd_init(&dnd, xDisplay);
      p = [NSPasteboard pasteboardWithName: NSDragPboard];
      [[XDragPbOwner alloc] initWithXPb: dnd.XdndSelection osPb: p];
    }
}

- (void) pasteboardChangedOwner: (NSPasteboard*)sender
{
  NSArray *types;
  Atom *typelist;

  [super pasteboardChangedOwner: sender];

  // We also have to set the supported types for our window
  types = [_pb types];
  typelist = mimeTypeForPasteboardType(xDisplay, [self zone], types);
  xdnd_set_type_list(&dnd, xAppWin, typelist);
  NSZoneFree([self zone], typelist);
}

- (NSArray*) availableTypes
{
  Window window;
  Atom *types;
  NSArray *newTypes;
	
  window = XGetSelectionOwner(xDisplay, dnd.XdndSelection);
  if (window == None)
    return nil;
  xdnd_get_type_list(&dnd, window, &types);
  newTypes = pasteboardTypeForMimeType(xDisplay, [self zone], types);
  free(types);
  return newTypes;
}

- (void) pasteboard: (NSPasteboard*)pb provideDataForType: (NSString*)type
{
  NSString *mime = [NSPasteboard mimeTypeForPasteboardType: type];
  Atom mType = XInternAtom(xDisplay, [mime cString], False);

  [self setData: nil];
	[self requestData: mType];
  [pb setData: [self data] forType: type];
}

- (void) xSelectionClear
{
  // Do nothing as we don't know, which new types will be supplied
  [self setOwnedByOpenStep: NO];
}

@end
