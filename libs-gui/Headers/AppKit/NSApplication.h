/** 
   NSApplication.h

   The one and only application class

   Copyright (C) 1996-2105 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   Author: Felipe A. Rodriguez <far@ix.netcom.com>
   Date: August 1998
   Author: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: December 1998

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

    AutogsdocSource: NSApplication.m
    AutogsdocSource: GSServicesManager.m

*/ 

#ifndef _GNUstep_H_NSApplication
#define _GNUstep_H_NSApplication
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSResponder.h>
#import <AppKit/NSUserInterfaceValidation.h>

#if defined(__cplusplus)
extern "C" {
#endif

@class NSArray;
@class NSDate;
@class NSError;
@class NSException;
@class NSMutableArray;
@class NSNotification;
@class NSString;
@class NSTimer;

@class NSEvent;
@class NSGraphicsContext;
@class NSImage;
@class NSMenu;
@class NSMenuItem;
@class NSPasteboard;
@class NSWindow;

@class GSInfoPanel;

typedef struct _NSModalSession *NSModalSession;

enum {
  NSRunStoppedResponse = (-1000),
  NSRunAbortedResponse = (-1001),
  NSRunContinuesResponse = (-1002)
};

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
#define NSUpdateWindowsRunLoopOrdering 600000

/**
* Returned by -applicationShouldTerminate: when -terminate: is called.
* Possible types include:
* <example>
* NSTerminateCancel;
* NSTerminateNow;
* NSTerminateLater.
* </example>
*/
enum {
  NSTerminateCancel = NO,
  NSTerminateNow = YES, 
  NSTerminateLater
};
typedef NSUInteger NSApplicationTerminateReply;

/** 
* Type used by -requestUserAttention: when an applications opened.  Possible values are:
* <example>
* NSCriticalRequest;
* NSInformationalRequest;
* </example>
*/
enum {
  NSCriticalRequest = 0,
  NSInformationalRequest = 10
};
typedef NSUInteger NSRequestUserAttentionType;

#define NSAppKitVersionNumber10_0      577
#define NSAppKitVersionNumber10_1      620
#define NSAppKitVersionNumber10_2      663
#define NSAppKitVersionNumber10_2_3    663.6
#define NSAppKitVersionNumber10_3      743
#define NSAppKitVersionNumber10_3_2    743.14
#define NSAppKitVersionNumber10_3_3    743.2
#define NSAppKitVersionNumber10_3_5    743.24
#define NSAppKitVersionNumber10_3_7    743.33
#define NSAppKitVersionNumber10_3_9    743.36
#define NSAppKitVersionNumber10_4      824
#define NSAppKitVersionNumber10_4_1    824.1
#define NSAppKitVersionNumber10_4_3    824.23
#define NSAppKitVersionNumber10_4_4    824.33
#define NSAppKitVersionNumber10_4_7    824.41
#define NSAppKitVersionNumber10_5      949
#define NSAppKitVersionNumber10_5_2    949.27
#define NSAppKitVersionNumber10_5_3    949.33
#define NSAppKitVersionNumber10_6      1038
#define NSAppKitVersionNumber10_7      1138
#define NSAppKitVersionNumber10_7_2    1138.23
#define NSAppKitVersionNumber10_7_3    1138.32
#define NSAppKitVersionNumber10_7_4    1138.47
#define NSAppKitVersionNumber10_8      1187
#define NSAppKitVersionNumber10_9      1265
#define NSAppKitVersionNumber10_10     1343
#define NSAppKitVersionNumber10_10_2   1344
#define NSAppKitVersionNumber10_10_3   1347
#define NSAppKitVersionNumber10_10_4   1348
#define NSAppKitVersionNumber10_10_5   1348
#define NSAppKitVersionNumber10_10_Max 1349
#define NSAppKitVersionNumber10_11     1404
#define NSAppKitVersionNumber10_11_1   1404.13
#define NSAppKitVersionNumber10_11_2   1404.34
#define NSAppKitVersionNumber10_11_3   1404.34
#define NSAppKitVersionNumber10_12     1504
#define NSAppKitVersionNumber10_12_1   1504.60
#define NSAppKitVersionNumber10_12_2   1504.76
#define NSAppKitVersionNumber10_13     1561
#define NSAppKitVersionNumber10_13_1   1561.1
#define NSAppKitVersionNumber10_13_2   1561.2
#define NSAppKitVersionNumber10_13_4   1561.4
#define NSAppKitVersionNumber10_14     1671
#define NSAppKitVersionNumber10_14_1   1671.1
#define NSAppKitVersionNumber10_14_2   1671.2
#define NSAppKitVersionNumber10_14_3   1671.3
#define NSAppKitVersionNumber10_14_4   1671.4
#define NSAppKitVersionNumber10_14_5   1671.5

APPKIT_EXPORT const double NSAppKitVersionNumber;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
enum _NSApplicationDelegateReply
{
  NSApplicationDelegateReplySuccess = 0,
  NSApplicationDelegateReplyCancel  = 1,
  NSApplicationDelegateReplyFailure = 2
};
typedef NSUInteger NSApplicationDelegateReply;

enum _NSApplicationPrintReply
{
  NSPrintingCancelled = NO,
  NSPrintingSuccess = YES,
  NSPrintingFailure,
  NSPrintingReplyLater
};
typedef NSUInteger NSApplicationPrintReply;
#endif


#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
typedef enum _NSApplicationPresentationOptions
{
  NSApplicationPresentationDefault                    = 0,
  NSApplicationPresentationAutoHideDock               = (1 <<  0),
  NSApplicationPresentationHideDock                   = (1 <<  1),
  NSApplicationPresentationAutoHideMenuBar            = (1 <<  2),
  NSApplicationPresentationHideMenuBar                = (1 <<  3),
  NSApplicationPresentationDisableAppleMenu           = (1 <<  4),
  NSApplicationPresentationDisableProcessSwitching    = (1 <<  5),
  NSApplicationPresentationDisableForceQuit           = (1 <<  6),
  NSApplicationPresentationDisableSessionTermination  = (1 <<  7),
  NSApplicationPresentationDisableHideApplication     = (1 <<  8),
  NSApplicationPresentationDisableMenuBarTransparency = (1 <<  9),
  NSApplicationPresentationFullScreen                 = (1 << 10),
  NSApplicationPresentationAutoHideToolbar            = (1 << 11)
} NSApplicationPresentationOptions;
  
#endif
  
APPKIT_EXPORT NSString	*NSModalPanelRunLoopMode;
APPKIT_EXPORT NSString	*NSEventTrackingRunLoopMode;

@interface NSApplication : NSResponder <NSCoding,NSUserInterfaceValidations>
{
  NSGraphicsContext	*_default_context;
  NSEvent		*_current_event;
  NSModalSession	_session;
  NSWindow		*_key_window;
  NSWindow		*_main_window;
  id			_delegate;
  id			_listener;
  NSMenu		*_main_menu;
  NSMenu		*_windows_menu;
  // 6 bits
  BOOL			_app_is_launched;
  BOOL			_app_is_running;
  BOOL			_app_is_active;
  BOOL			_app_is_hidden;
  BOOL			_unhide_on_activation;
  BOOL			_windows_need_update;
  NSImage		*_app_icon;
  NSWindow		*_app_icon_window;
  NSMutableArray	*_hidden;
  NSMutableArray	*_inactive;
  NSWindow		*_hidden_key;
  NSWindow              *_hidden_main;
  GSInfoPanel           *_infoPanel;
  NSApplicationPresentationOptions _presentationOptions;

  /* This autorelease pool should only be created and used by -run, with
     a single exception (which is why it is stored here as an ivar): the
     -terminate: method will destroy this autorelease pool before exiting
     the program.  */
  id             _runLoopPool;
}

/*
 * Class methods
 */

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
+ (void) detachDrawingThread: (SEL)selector 
		    toTarget: (id)target 
		  withObject: (id)argument;
#endif

/*
 * Creating and initializing the NSApplication
 */
+ (NSApplication*) sharedApplication;

/*
 * Instance methods
 */

/*
 * Creating and initializing the NSApplication
 */
- (void) finishLaunching;

/*
 * Changing the active application
 */
- (void) activateIgnoringOtherApps: (BOOL)flag;
- (void) deactivate;
- (BOOL) isActive;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void) hideOtherApplications: (id)sender;
- (void) unhideAllApplications: (id)sender;
#endif

/*
 * Running the event loop
 */
- (void) abortModal;
- (NSModalSession) beginModalSessionForWindow: (NSWindow*)theWindow;
- (void) endModalSession: (NSModalSession)theSession;
- (BOOL) isRunning;
- (void) run;
- (NSInteger) runModalForWindow: (NSWindow*)theWindow;
- (NSInteger) runModalSession: (NSModalSession)theSession;
- (NSWindow *) modalWindow;
- (void) sendEvent: (NSEvent*)theEvent;
- (void) stop: (id)sender;
- (void) stopModal;
- (void) stopModalWithCode: (NSInteger)returnCode;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (NSInteger) runModalForWindow: (NSWindow *)theWindow
               relativeToWindow: (NSWindow *)docWindow;
- (void) beginSheet: (NSWindow *)sheet
     modalForWindow: (NSWindow *)docWindow
      modalDelegate: (id)modalDelegate
     didEndSelector: (SEL)didEndSelector
	contextInfo: (void *)contextInfo;
- (void) endSheet: (NSWindow *)sheet;
- (void) endSheet: (NSWindow *)sheet
       returnCode: (NSInteger)returnCode;
#endif

/*
 * Getting, removing, and posting events
 */
- (NSEvent*) currentEvent;
- (void) discardEventsMatchingMask: (NSUInteger)mask
		       beforeEvent: (NSEvent*)lastEvent;
- (NSEvent*) nextEventMatchingMask: (NSUInteger)mask
			 untilDate: (NSDate*)expiration
			    inMode: (NSString*)mode
			   dequeue: (BOOL)flag;
- (void) postEvent: (NSEvent*)event atStart: (BOOL)flag;

/*
 * Sending action messages
 */
- (BOOL) sendAction: (SEL)aSelector
		 to: (id)aTarget
	       from: (id)sender;
- (id) targetForAction: (SEL)aSelector;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (id)targetForAction: (SEL)theAction 
                   to: (id)theTarget 
                 from: (id)sender;
#endif
- (BOOL) tryToPerform: (SEL)aSelector
		 with: (id)anObject;

/*
 * Setting the application's icon
 */
- (void) setApplicationIconImage: (NSImage*)anImage;
- (NSImage*) applicationIconImage;

/*
 * Hiding all windows
 */
- (void) hide: (id)sender;
- (BOOL) isHidden;
- (void) unhide: (id)sender;
- (void) unhideWithoutActivation;

/*
 * Managing windows
 */
- (NSWindow*) keyWindow;
- (NSWindow*) mainWindow;
- (NSWindow*) makeWindowsPerform: (SEL)aSelector
			 inOrder: (BOOL)flag;
- (void) miniaturizeAll: (id)sender;
- (void) preventWindowOrdering;
- (void) setWindowsNeedUpdate: (BOOL)flag;
- (void) updateWindows;
- (NSArray*) windows;
- (NSWindow*) windowWithWindowNumber: (NSInteger)windowNum;

/*
 * Showing Standard Panels
 */
#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)
/* GNUstep extensions displaying an infoPanel, title is 'Info' */
/* For a list of the useful values in the dictionary, see GSInfoPanel.h. 
   The entries are mostly compatible with macosx. */
- (void) orderFrontStandardInfoPanel: (id)sender;
- (void) orderFrontStandardInfoPanelWithOptions: (NSDictionary *)dictionary;
#endif 
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
/* macosx extensions displaying an aboutPanel, title is 'About'. 
   NB: These two methods do exactly the same as the two methods above, 
   only the title is different. */
- (void) orderFrontStandardAboutPanel: (id)sender;
- (void) orderFrontStandardAboutPanelWithOptions: (NSDictionary *)dictionary;
#endif 

/*
 * Getting the main menu
 */
- (NSMenu*) mainMenu;
- (void) setMainMenu: (NSMenu*)aMenu;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void) setAppleMenu: (NSMenu*)aMenu;
#endif

/*
 * Managing the Windows menu
 */
- (void) addWindowsItem: (NSWindow*)aWindow
		  title: (NSString*)aString
	       filename: (BOOL)isFilename;
- (void) arrangeInFront: (id)sender;
- (void) changeWindowsItem: (NSWindow*)aWindow
		     title: (NSString*)aString
		  filename: (BOOL)isFilename;
- (void) removeWindowsItem: (NSWindow*)aWindow;
- (void) setWindowsMenu: (NSMenu*)aMenu;
- (void) updateWindowsItem: (NSWindow*)aWindow;
- (NSMenu*) windowsMenu;

/*
 * Managing the Service menu
 */
- (void) registerServicesMenuSendTypes: (NSArray*)sendTypes
			   returnTypes: (NSArray*)returnTypes;
- (NSMenu*) servicesMenu;
- (id) servicesProvider;
- (void) setServicesMenu: (NSMenu*)aMenu;
- (void) setServicesProvider: (id)anObject;
- (id) validRequestorForSendType: (NSString*)sendType
		      returnType: (NSString*)returnType;

/*
 * Getting the display context
 */
- (NSGraphicsContext*) context;

/*
 * Reporting an exception
 */
- (void) reportException: (NSException*)anException;

/*
 * Terminating the application
 */
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void) replyToApplicationShouldTerminate: (BOOL)shouldTerminate;
#endif 
#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (void) replyToOpenOrPrint: (NSApplicationDelegateReply)reply;
#endif 
- (void) terminate: (id)sender;

/*
 * Assigning a delegate
 */
- (id) delegate;
- (void) setDelegate: (id)anObject;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
/*
 * Methods for scripting
 */
- (NSArray *) orderedDocuments;
- (NSArray *) orderedWindows;
/*
 * Methods for user attention requests
 */
- (void) cancelUserAttentionRequest: (NSInteger)request;
- (NSInteger) requestUserAttention: (NSRequestUserAttentionType)requestType;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
- (NSApplicationPresentationOptions) currentPresentationOptions;
- (NSApplicationPresentationOptions) presentationOptions;
- (void)setPresentationOptions: (NSApplicationPresentationOptions)options;
#endif

@end

/**
 * Informal protocol declaring methods for sending to and receiving from
 * remote services providers.
 */
@interface NSObject (NSServicesRequests)

/*
 * Pasteboard Read/Write
 */
/**
 * Request to transfer data from given pasteboard to selection (called when
 * a called remote service has provided data to this pasteboard).
 */
- (BOOL) readSelectionFromPasteboard: (NSPasteboard*)pboard;

/**
 * Request to write selection data to given pasteboard (called when a called
 * remote service is to be invoked).
 */
- (BOOL) writeSelectionToPasteboard: (NSPasteboard*)pboard
                              types: (NSArray*)types;

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)
- (NSWindow*) iconWindow;
#endif
@end

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
@interface NSApplication (CharacterPanel)
- (void) orderFrontCharacterPalette: (id)sender;
@end
#endif

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)

@interface NSApplication (GSGUIInternal)
- (void) _windowWillDealloc: (NSWindow *)window;
@end

/**
 * This is now a formal optional protocol.
 * Your delegate does not need to implement the full formal protocol.  Your
 * delegate should just implement the methods it needs to, which will allow
 * <code>NSApp</code> to use default implementations in other cases.
 */
@protocol NSApplicationDelegate <NSObject>
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST) && GS_PROTOCOLS_HAVE_OPTIONAL
@optional
#else
@end
@interface NSObject (NSApplicationDelegate)
#endif

/**
 * Sender app (not necessarily this application) requests application to open
 * file without bringing up its normal UI, for programmatic manipulation.
 * YES should be returned on success, NO on failure.
 */
- (BOOL) application: (NSApplication*)app
   openFileWithoutUI: (NSString*)filename;

/**
 * Sender requests application to open filename.
 * YES should be returned on success, NO on failure.
 */
- (BOOL) application: (NSApplication*)app
	    openFile: (NSString*)filename;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
/**
 * Sender requests application to open filenames.
 */
- (void) application: (NSApplication*)app openFiles: (NSArray*)filenames;
#endif

/**
 * Sender requests application to open a temporary file.  Responsibility
 * for eventual deletion lies with this application.
 * YES should be returned on success, NO on failure.
 */
- (BOOL) application: (NSApplication*)app
	openTempFile: (NSString*)filename;

/**
 * Sender requests application to print filename.  This should generally be
 * done without presenting a GUI to the user, unless default options are
 * likely to be changed.
 * YES should be returned on success, NO on failure.
 */
- (BOOL) application: (NSApplication *)theApplication 
           printFile:(NSString *)filename;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
/**
 * <em>Not sent yet on GNUstep.</em>
 */
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (NSApplicationPrintReply) application: (NSApplication*)app
                             printFiles: (NSArray*)files 
                           withSettings: (NSDictionary*)settings 
                        showPrintPanels: (BOOL)flag;

#else
// Deprecated in 10.4
- (void) application: (NSApplication*)app printFiles: (NSArray*)filenames;
#endif
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
/**
 * Ask delegate for an error replacement.
 */
- (NSError*) application: (NSApplication*)app willPresentError: (NSError*)error;
#endif

/**
 * Sender requests application to open a fresh document.
 * YES should be returned on success, NO on failure.
 */
- (BOOL) applicationOpenUntitledFile: (NSApplication*)app;

/**
 * Sender will request application to open a fresh document, unless NO
 * is returned here.
 */
- (BOOL) applicationShouldOpenUntitledFile:(NSApplication *)sender;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
/**
 * Sent from within the [NSApplication-terminate:].  If
 * <code>NSTerminateNow</code> is returned, termination will proceed.  If
 * <code>NSTerminateCancel</code> is returned, termination will NOT proceed.
 * If <code>NSTerminateLater</code> is returned, termination will be halted,
 * but the application should call
 * [NSApplication-replyToApplicationShouldTerminate:] with a YES or NO.  (Used
 * if confirmation windows, etc. need to be put up.)
 */
- (NSApplicationTerminateReply) applicationShouldTerminate: (NSApplication *)sender;
#else
/**
 * Sent from within the [NSApplication-terminate:].  If NO is returned
 * termination will not proceed. 
 */
- (BOOL) applicationShouldTerminate: (id)sender;
#endif

/**
 * Invoked when the last window is closed in an application.  If YES is
 * returned, -applicationShouldTerminate: is invoked.
 */
- (BOOL) applicationShouldTerminateAfterLastWindowClosed: (id)sender;

/**
 * Invoked on notification that application has become active.
 */
- (void) applicationDidBecomeActive: (NSNotification*)aNotification;

/**
 * Invoked on notification that application has finished launching
 * ([NSApplication-finishLaunching] has completed, but no event dispatching
 * has begun. 
 */
- (void) applicationDidFinishLaunching: (NSNotification*)aNotification;

/**
 * Invoked on notification that application has just been hidden.
 */
- (void) applicationDidHide: (NSNotification*)aNotification;

/**
 * Invoked on notification that application has just been deactivated.
 */
- (void) applicationDidResignActive: (NSNotification*)aNotification;

/**
 * Invoked on notification that application has just been unhidden.
 */
- (void) applicationDidUnhide: (NSNotification*)aNotification;

/**
 * Invoked on notification that application has updated its windows.
 */
- (void) applicationDidUpdate: (NSNotification*)aNotification;

/**
 * Invoked on notification that application will become active.
 */
- (void) applicationWillBecomeActive: (NSNotification*)aNotification;

/**
 * Invoked on notification that application will become active.
 */
- (void) applicationWillFinishLaunching: (NSNotification*)aNotification;

/**
 * Invoked on notification that application will be hidden.
 */
- (void) applicationWillHide: (NSNotification*)aNotification;

/**
 * Invoked on notification just before application resigns active status.
 */
- (void) applicationWillResignActive: (NSNotification*)aNotification;

/**
 * Invoked on notification just before application terminates.  (There is
 * no opportunity to avert it now.)
 */
- (void) applicationWillTerminate:(NSNotification*)aNotification;

/**
 * Invoked on notification that application will be unhidden.
 */
- (void) applicationWillUnhide: (NSNotification*)aNotification;

/**
 * Invoked on notification that application will now update its windows.
 * (See [NSApplication-updateWindows].
 */
- (void) applicationWillUpdate: (NSNotification*)aNotification;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
/**
 * Method called by scripting framework on OS X.  <em>Not implemented (sent)
 * yet on GNUstep.</em>
 */
- (BOOL) application: (NSApplication*)sender 
  delegateHandlesKey: (NSString*)key;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_1, GS_API_LATEST)
/**
 * Method used on OS X to allow an application to override the standard menu
 * obtained by right-clicking on the application's dock icon.  <em>Called
 * when the application uses Macintosh or Windows95 style menus.</em>
 */
- (NSMenu *) applicationDockMenu: (NSApplication*)sender;
#endif

/**
 * Method used on OS X to allow delegate to handle event when user clicks on
 * dock icon of an already-open app.  If YES is returned, a default
 * implementation executes (for example, to create a new untitled document);
 * if NO is returned nothing is done (and you can handle it here in this
 * method).  <em>Not sent yet under GNUstep.</em>
 */
- (BOOL) applicationShouldHandleReopen: (NSApplication*)theApplication 
		     hasVisibleWindows: (BOOL)flag;

/**
 * Called on OS X when the resolution or other characteristics of the display
 * have changed (through control panel operation, connecting a new monitor,
 * etc.).  <em>Not implemented/sent yet under GNUstep.</em>
 */
- (void) applicationDidChangeScreenParameters: (NSNotification*)aNotification;
#endif 
@end
#endif

/*
 * Notifications
 */
APPKIT_EXPORT NSString	*NSApplicationDidBecomeActiveNotification;
APPKIT_EXPORT NSString	*NSApplicationDidChangeScreenParametersNotification;
APPKIT_EXPORT NSString	*NSApplicationDidFinishLaunchingNotification;
APPKIT_EXPORT NSString	*NSApplicationDidHideNotification;
APPKIT_EXPORT NSString	*NSApplicationDidResignActiveNotification;
APPKIT_EXPORT NSString	*NSApplicationDidUnhideNotification;
APPKIT_EXPORT NSString	*NSApplicationDidUpdateNotification;
APPKIT_EXPORT NSString	*NSApplicationWillBecomeActiveNotification;
APPKIT_EXPORT NSString	*NSApplicationWillFinishLaunchingNotification;
APPKIT_EXPORT NSString	*NSApplicationWillHideNotification;
APPKIT_EXPORT NSString	*NSApplicationWillResignActiveNotification;
APPKIT_EXPORT NSString	*NSApplicationWillTerminateNotification;
APPKIT_EXPORT NSString	*NSApplicationWillUnhideNotification;
APPKIT_EXPORT NSString	*NSApplicationWillUpdateNotification;

/*
 * Determine Whether an Item Is Included in Services Menus
 */
APPKIT_EXPORT int
NSSetShowsServicesMenuItem(NSString *name, BOOL enabled);

APPKIT_EXPORT BOOL
NSShowsServicesMenuItem(NSString *name);

/*
 * Programmatically Invoke a Service
 */
APPKIT_EXPORT BOOL
NSPerformService(NSString *serviceItem, NSPasteboard *pboard);

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)
APPKIT_EXPORT id
GSContactApplication(NSString *appName, NSString *port, NSDate *expire);
#endif

/*
 * Force Services Menu to Update Based on New Services
 */
APPKIT_EXPORT void
NSUpdateDynamicServices(void);

/*
 * Register object to handle services requests.
 */
APPKIT_EXPORT void
NSRegisterServicesProvider(id provider, NSString *name);

APPKIT_EXPORT void 
NSUnRegisterServicesProvider(NSString *name);

APPKIT_EXPORT int
NSApplicationMain(int argc, const char **argv);

APPKIT_EXPORT void 
NSShowSystemInfoPanel(NSDictionary *options);

/*
 * The NSApp global variable.
 */
APPKIT_EXPORT NSApplication	*NSApp;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
enum
{
  NSModalResponseStop = -1000,
  NSModalResponseAbort = -1001,
  NSModalResponseContinue = -1002
};
typedef NSInteger NSModalResponse;
#endif

#if defined(__cplusplus)
}
#endif

#endif // _GNUstep_H_NSApplication
