/*
 This file is part of Darling.

 Copyright (C) 2020 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/
#import "CGSConnectionX11.h"
#import <CoreGraphics/CGSKeyboardLayout.h>
#include "CarbonKeys.h"
#import "X11KeySymToUCS.h"
#import <Foundation/NSDebug.h>
#import <Foundation/NSString.h>
#include <stdlib.h>
#include <stdatomic.h>
#import <X11/Xutil.h>
#import <X11/extensions/Xrandr.h>
#import <X11/XKBlib.h>
#import <X11/extensions/XKBrules.h>

// TODO: Use XInput2:
// https://stackoverflow.com/questions/44095001/getting-double-rawkeypress-events-using-xinput2
// https://github.com/openbsd/xenocara/blob/master/app/xinput/src/test_xi2.c

@implementation CGSConnectionX11

static int errorHandler(Display* display, XErrorEvent* errorEvent)
{
	NSLog(@"************** X11 ERROR!");
	NSLog(@"Request code: %d:%d", errorEvent->request_code, errorEvent->minor_code);
	NSLog(@"Error code: %d", errorEvent->error_code);
	return 0;
}

static void socketCallback(CFSocketRef s, CFSocketCallBackType type, CFDataRef address,
    const void *data, void *info)
{
    CGSConnectionX11* self = (CGSConnectionX11*) info;
    [self processPendingEvents];
}

-(instancetype) initWithConnectionID:(CGSConnectionID)connId
{
	_display = XOpenDisplay(NULL);
	if (!_display)
		_display = XOpenDisplay(":0");
	
	if (!_display)
	{
		NSLog(@"CGSConnectionX11: XOpenDisplay() failed\n");

		[self release];
		return nil;
	}
	self = [super initWithConnectionID: connId];

	if (NSDebugEnabled)
		XSynchronize(_display, True);

	XSetErrorHandler(errorHandler);

	int fd = ConnectionNumber(_display);

	// There's no need to retain/release the display,
    // because the display is guaranteed to outlive
    // the socket.
    CFSocketContext context = {
        .version = 0,
        .info = self,
        .retain = NULL,
        .release = NULL,
        .copyDescription = NULL
    };

    _cfSocket = CFSocketCreateWithNative(kCFAllocatorDefault, fd, kCFSocketReadCallBack, socketCallback, &context);
    _source = CFSocketCreateRunLoopSource(kCFAllocatorDefault, _cfSocket, 0);
    CFRunLoopAddSource(CFRunLoopGetMain(), _source, kCFRunLoopCommonModes);

	int errorBase;
	if (XRRQueryExtension(_display, &_xrrEventBase, &errorBase))
	{
		// Sign up for XRandR events
		// TODO: Do we need all these?
		XRRSelectInput(_display, DefaultRootWindow(_display), RRScreenChangeNotifyMask | RROutputChangeNotifyMask | RRCrtcChangeNotifyMask);
	}

	int major = XkbMajorVersion, minor = XkbMinorVersion;

	if (XkbLibraryVersion(&major, &minor) && XkbQueryExtension(_display, NULL, &_xkbEventBase, &major, &minor, NULL))
	{
		XkbSelectEvents(_display, XkbUseCoreKbd, /*XkbMapNotifyMask |*/ XkbStateNotifyMask, 1);
	}

	return self;
}

-(void) dealloc
{
	if (_display)
		XCloseDisplay(_display);

	CFRunLoopRemoveSource(CFRunLoopGetMain(), _source, kCFRunLoopCommonModes);

	if (_source != NULL)
		CFRelease(_source);
	if (_cfSocket != NULL)
		CFRelease(_cfSocket);

	[_keyboardLayout release];

	[super dealloc];
}

-(CGSKeyboardLayout*) createKeyboardLayout
{
	@synchronized (self)
	{
		if (!_keyboardLayout)
			[self _doCreateKeyboardLayout];
		return [_keyboardLayout retain];
	}
}

- (void) _doCreateKeyboardLayout
{
	int major = XkbMajorVersion, minor = XkbMinorVersion;
	XkbDescPtr pKBDesc;
	unsigned char group = 0;
	XkbStateRec state;

	struct MyLayout
	{
		UCKeyboardLayout layout;
		UCKeyModifiersToTableNum modifierVariants;
		UInt8 secondTableNum, thirdTableNum;
		UCKeyToCharTableIndex tableIndex;
		UInt32 secontTableOffset;
		UCKeyOutput table1[128];
		UCKeyOutput table2[128];
	};

	if (!XkbLibraryVersion(&major, &minor))
		return;
	if (!XkbQueryExtension(_display, NULL, NULL, &major, &minor, NULL))
		return;

	pKBDesc = XkbGetMap(_display, XkbAllClientInfoMask, XkbUseCoreKbd);
	if (!pKBDesc)
		return;

	XkbDescPtr desc = XkbGetKeyboard(_display, XkbAllComponentsMask, XkbUseCoreKbd);
	if (!desc)
		return;

	if (XkbGetState(_display, XkbUseCoreKbd, &state) == Success)
		group = state.group;

	struct MyLayout* layout = (struct MyLayout*) malloc(sizeof(struct MyLayout));

	layout->layout.keyLayoutHeaderFormat = kUCKeyLayoutHeaderFormat;
	layout->layout.keyLayoutDataVersion = 0;
	layout->layout.keyLayoutFeatureInfoOffset = 0;
	layout->layout.keyboardTypeCount = 1;

	memset(layout->layout.keyboardTypeList, 0, sizeof(UCKeyboardTypeHeader));

	layout->layout.keyboardTypeList[0].keyModifiersToTableNumOffset = offsetof(struct MyLayout, modifierVariants);
	layout->layout.keyboardTypeList[0].keyToCharTableIndexOffset = offsetof(struct MyLayout, tableIndex);

	layout->modifierVariants.keyModifiersToTableNumFormat = kUCKeyModifiersToTableNumFormat;
	layout->modifierVariants.defaultTableNum = 0;
	layout->modifierVariants.modifiersCount = 3;
	layout->modifierVariants.tableNum[0] = 0;
	layout->modifierVariants.tableNum[1] = 0; // cmd key bit
	layout->modifierVariants.tableNum[2] = 1; // shift key bit

	layout->tableIndex.keyToCharTableIndexFormat = kUCKeyToCharTableIndexFormat;
	layout->tableIndex.keyToCharTableSize = 128;
	layout->tableIndex.keyToCharTableCount = 2;
	layout->tableIndex.keyToCharTableOffsets[0] = offsetof(struct MyLayout, table1);
	layout->tableIndex.keyToCharTableOffsets[1] = offsetof(struct MyLayout, table2);

	for (int shift = 0; shift <= 1; shift++)
	{
		UCKeyOutput* outTable = (shift == 0) ? layout->table1 : layout->table2;
		for (int i = 0; i < 128; i++)
		{
			// Reverse the operation we do in -postXEvent:
			const int x11KeyCode = carbonToX11[i];

			if (!x11KeyCode)
			{
				outTable[i] = 0;
				continue;
			}

			// NOTE: Not using the group here. It just works with 0 instead...
			KeySym sym = XkbKeycodeToKeysym(_display, x11KeyCode, 0, shift);

			if (sym != NoSymbol)
				outTable[i] = X11KeySymToUCS(sym);
			else
				outTable[i] = 0;
		}
	}

	XkbFreeClientMap(pKBDesc, XkbAllClientInfoMask, true);
	NSString *name = @"?", *fullName = @"?";

	if (fullName != NULL)
	{
		char *group = XGetAtomName(_display, desc->names->groups[state.group]);

		if (group != NULL)
		{
			fullName = [NSString stringWithUTF8String: group];
			XFree(group);
		}
	}

	XkbRF_VarDefsRec vd;
	if (name != NULL && XkbRF_GetNamesProp(_display, NULL, &vd))
	{
		char* saveptr;
		char *tok = strtok_r(vd.layout, ",", &saveptr);

		for (int i = 0; i < state.group; i++)
		{
			tok = strtok_r(NULL, ",", &saveptr);
			if (tok == NULL)
				break;
		}

		if (tok != NULL)
		{
			name = [NSString stringWithUTF8String: tok];
		}
	}

	CGSKeyboardLayout* cgsLayout = [CGSKeyboardLayout new];

	[cgsLayout setLayout: &layout->layout length: sizeof(struct MyLayout)];
	cgsLayout.name = name;
	cgsLayout.fullName = fullName;

	_keyboardLayout = cgsLayout;
	_keyboardLayoutGroup = state.group;
}

-(CGPoint) mouseLocation
{
	Window child, root = DefaultRootWindow(_display);

    int root_x, root_y;
    int win_x, win_y;
    unsigned int mask;

    XQueryPointer(_display, root, &root, &child, &root_x, &root_y, &win_x, &win_y, &mask);
	return CGPointMake(root_x, root_y);
}

-(void) processXEvent:(XEvent*) event
{
	switch (event->type)
	{
		case KeyPress:
		case KeyRelease:
			break;
		case ButtonPress:
			break;
		case ButtonRelease:
			break;
		case MotionNotify:
			break;
		case EnterNotify:
			break;
		case FocusIn:
			break;
		case FocusOut:
			break;
		case KeymapNotify:
			break;
		case Expose:
			break;
		case ConfigureNotify:
			break;
		case ClientMessage:
		{
			static Atom wmDeleteWindowAtom;
			if (!wmDeleteWindowAtom)
				wmDeleteWindowAtom = XInternAtom(_display, "WM_DELETE_WINDOW", False);

			if (event->xclient.format == 32 && event->xclient.data.l[0] == wmDeleteWindowAtom)
				; // platformWindowWillClose
			break;
		}
		default:

		if (/*event->type == _xkbEventBase + XkbMapNotify ||*/ event->type == _xkbEventBase + XkbStateNotify)
		{
			XkbEvent* xkbevt = (XkbEvent*) event;
			// Invalidate cached keyboard layout information
			if (_keyboardLayout && _keyboardLayoutGroup != xkbevt->state.group)
			{
				// TODO: Emit kTISNotifySelectedKeyboardInputSourceChanged via NSDistributedNotificationCenter

				@synchronized (self)
				{
					[_keyboardLayout release];
					_keyboardLayout = nil;
				}
				_keyboardLayoutGroup = xkbevt->state.group;
			}

			break;
		}
		else if (event->type == _xrrEventBase + RRScreenChangeNotify || event->type == _xrrEventBase + RRNotify)
		{
			// Invalidate cached information
			if (_screens)
			{
				@synchronized (self)
				{
					[_screens release];
					_screens = nil;
				}
			}
		}
	}
}

-(void) processPendingEvents
{
	while (XPending(_display) > 0)
	{
		XEvent event;

		if (XNextEvent(_display, &event) == Success)
			[self processXEvent: &event];
	}
}

-(void) _doGetScreenInformation
{

}

-(NSArray<CGSScreen*>*) createScreens
{
	@synchronized (self)
	{
		if (!_screens)
			[self _doGetScreenInformation];
		return [_screens retain];
	}
}

-(CGSWindow*) newWindow:(CGSRegionRef)region
{

}

+(BOOL) isAvailable
{
	if (getenv("DISPLAY") != NULL)
		return YES;
	// TODO: we could be a little smarter here
	return NO;
}
@end
