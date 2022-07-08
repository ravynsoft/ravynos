/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSResponder.h>
#import <AppKit/AppKitExport.h>
#import <AppKit/NSGraphics.h>

@class NSWindow, NSImage, NSMenu, NSPasteboard, NSDisplay, NSDockTile, DKConnection, DKMenu;

APPKIT_EXPORT NSString *const NSModalPanelRunLoopMode;
APPKIT_EXPORT NSString *const NSEventTrackingRunLoopMode;

APPKIT_EXPORT NSString *const NSApplicationWillFinishLaunchingNotification;
APPKIT_EXPORT NSString *const NSApplicationDidFinishLaunchingNotification;

APPKIT_EXPORT NSString *const NSApplicationWillBecomeActiveNotification;
APPKIT_EXPORT NSString *const NSApplicationDidBecomeActiveNotification;
APPKIT_EXPORT NSString *const NSApplicationWillResignActiveNotification;
APPKIT_EXPORT NSString *const NSApplicationDidResignActiveNotification;

APPKIT_EXPORT NSString *const NSApplicationWillUpdateNotification;
APPKIT_EXPORT NSString *const NSApplicationDidUpdateNotification;

APPKIT_EXPORT NSString *const NSApplicationWillHideNotification;
APPKIT_EXPORT NSString *const NSApplicationDidHideNotification;
APPKIT_EXPORT NSString *const NSApplicationWillUnhideNotification;
APPKIT_EXPORT NSString *const NSApplicationDidUnhideNotification;

APPKIT_EXPORT NSString *const NSApplicationWillTerminateNotification;

APPKIT_EXPORT NSString *const NSApplicationDidChangeScreenParametersNotification;

APPKIT_EXPORT id NSApp;

typedef id NSModalSession;

enum {
    NSRunStoppedResponse = -1000,
    NSRunAbortedResponse = -1001,
    NSRunContinuesResponse = -1002
};

typedef enum {
    NSTerminateCancel,
    NSTerminateNow,
    NSTerminateLater,
} NSApplicationTerminateReply;

typedef enum {
    NSCriticalRequest,
    NSInformationalRequest,
} NSRequestUserAttentionType;

typedef enum {
    NSApplicationDelegateReplySuccess,
    NSApplicationDelegateReplyCancel,
    NSApplicationDelegateReplyFailure,
} NSApplicationDelegateReply;

typedef enum {
    NSPrintingCancelled,
    NSPrintingSuccess,
    NSPrintingReplyLater,
    NSPrintingFailure,
} NSApplicationPrintReply;

@interface NSApplication : NSResponder {
    NSDisplay *_display;
    id _delegate;
    NSMutableArray *_windows;
    NSWindow *_keyWindow;
    NSWindow *_mainWindow;
    NSMenu *_mainMenu;
    NSMenu *_windowsMenu;
    mach_port_t _wsReplyPort;
    mach_port_t _wsSvcPort;

    NSImage *_applicationIconImage;

    BOOL _isRunning;
    BOOL _isActive;
    BOOL _isHidden;
    BOOL _windowsNeedUpdate;
    NSEvent *_currentEvent;
    NSDockTile *_dockTile;

    NSMutableArray *_modalStack;
    void *_lock;
    NSMutableArray *_orderedWindows; // get rid of
    NSTimer *_attentionTimer;
}

+ (NSApplication *)sharedApplication;

+ (void)detachDrawingThread:(SEL)selector toTarget:target withObject:object;

- init;

- (NSGraphicsContext *)context;

- delegate;
- (NSArray *)windows;
- (NSWindow *)windowWithWindowNumber:(NSInteger)number;

- (NSMenu *)mainMenu;
- (NSMenu *)windowsMenu;
- (NSWindow *)mainWindow;
- (NSWindow *)keyWindow;
- (NSImage *)applicationIconImage;
- (BOOL)isActive;
- (BOOL)isHidden;
- (BOOL)isRunning;

- (NSWindow *)makeWindowsPerform:(SEL)selector inOrder:(BOOL)inOrder;
- (void)miniaturizeAll:sender;

- (NSArray *)orderedDocuments;
- (NSArray *)orderedWindows;
- (void)preventWindowOrdering;

- (void)setDelegate:delegate;
- (void)setMainMenu:(NSMenu *)menu;
- (void)setApplicationIconImage:(NSImage *)image;

- (void)setWindowsMenu:(NSMenu *)menu;
- (void)addWindowsItem:(NSWindow *)window title:(NSString *)title filename:(BOOL)filename;
- (void)changeWindowsItem:(NSWindow *)window title:(NSString *)title filename:(BOOL)filename;
- (void)removeWindowsItem:(NSWindow *)window;
- (void)updateWindowsItem:(NSWindow *)window;

- (void)finishLaunching;
- (void)run;

- (void)sendEvent:(NSEvent *)event;

- (NSEvent *)nextEventMatchingMask:(unsigned int)mask untilDate:(NSDate *)untilDate inMode:(NSString *)mode dequeue:(BOOL)dequeue;
- (NSEvent *)currentEvent;
- (void)discardEventsMatchingMask:(unsigned)mask beforeEvent:(NSEvent *)event;
- (void)postEvent:(NSEvent *)event atStart:(BOOL)atStart;

- targetForAction:(SEL)action;
- targetForAction:(SEL)action to:target from:sender;
- (BOOL)sendAction:(SEL)action to:target from:sender;
- (BOOL)tryToPerform:(SEL)selector with:object;

- (void)setWindowsNeedUpdate:(BOOL)value;
- (void)updateWindows;

- (void)activateIgnoringOtherApps:(BOOL)flag;
- (void)deactivate;

- (NSWindow *)modalWindow;
- (NSModalSession)beginModalSessionForWindow:(NSWindow *)window;
- (int)runModalSession:(NSModalSession)session;
- (void)endModalSession:(NSModalSession)session;
- (void)stopModalWithCode:(int)code;

- (int)runModalForWindow:(NSWindow *)window;
- (void)stopModal;
- (void)abortModal;

- (void)beginSheet:(NSWindow *)sheet modalForWindow:(NSWindow *)window modalDelegate:modalDelegate didEndSelector:(SEL)didEndSelector contextInfo:(void *)contextInfo;
- (void)endSheet:(NSWindow *)sheet returnCode:(int)returnCode;
- (void)endSheet:(NSWindow *)sheet;

- (void)reportException:(NSException *)exception;

- (int)requestUserAttention:(NSRequestUserAttentionType)attentionType;
- (void)cancelUserAttentionRequest:(int)requestNumber;

- (void)runPageLayout:sender;
- (void)orderFrontColorPanel:sender;
- (void)orderFrontCharacterPalette:sender;

- (void)hide:sender;
- (void)hideOtherApplications:sender;
- (void)unhide:sender;
- (void)unhideAllApplications:sender;
- (void)unhideWithoutActivation;
- (void)stop:sender;
- (void)terminate:sender;

- (void)replyToApplicationShouldTerminate:(BOOL)terminate;
- (void)replyToOpenOrPrint:(NSApplicationDelegateReply)reply;

- (void)arrangeInFront:sender;

- (NSMenu *)servicesMenu;
- (void)setServicesMenu:(NSMenu *)menu;
- servicesProvider;
- (void)setServicesProvider:provider;
- (void)registerServicesMenuSendTypes:(NSArray *)sendTypes returnTypes:(NSArray *)returnTypes;
- validRequestorForSendType:(NSString *)sendType returnType:(NSString *)returnType;

- (void)orderFrontStandardAboutPanel:sender;
- (void)orderFrontStandardAboutPanelWithOptions:(NSDictionary *)options;
- (void)activateContextHelpMode:sender;
- (void)showHelp:sender;

- (NSDockTile *)dockTile;

- (void)addRecentItem:(NSURL *)url;

// private
- (DKMenu *)dbusMenu;
- (DKConnection *)dbusConnection;
- (void)_addWindow:(NSWindow *)window;

- (void)_windowWillBecomeActive:(NSWindow *)window;
- (void)_windowDidBecomeActive:(NSWindow *)window;
- (void)_windowWillBecomeDeactive:(NSWindow *)window;
- (void)_windowDidBecomeDeactive:(NSWindow *)window;

- (mach_port_t)_wsServicePort;

@end

@interface NSObject (NSApplication_serviceRequest)
- (BOOL)writeSelectionToPasteboard:(NSPasteboard *)pasteboard types:(NSArray *)types;
@end

@interface NSObject (NSApplication_notifications)
- (void)applicationWillFinishLaunching:(NSNotification *)note;
- (void)applicationDidFinishLaunching:(NSNotification *)note;

- (void)applicationWillBecomeActive:(NSNotification *)note;
- (void)applicationDidBecomeActive:(NSNotification *)note;
- (void)applicationWillResignActive:(NSNotification *)note;
- (void)applicationDidResignActive:(NSNotification *)note;

- (void)applicationWillUpdate:(NSNotification *)note;
- (void)applicationDidUpdate:(NSNotification *)note;

- (void)applicationWillHide:(NSNotification *)note;
- (void)applicationDidHide:(NSNotification *)note;
- (void)applicationWillUnhide:(NSNotification *)note;
- (void)applicationDidUnhide:(NSNotification *)note;

- (void)applicationWillTerminate:(NSNotification *)note;

- (void)applicationDidChangeScreenParameters:(NSNotification *)note;
@end

@interface NSObject (NSApplication_delegate)
- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)application;
- (BOOL)applicationOpenUntitledFile:(NSApplication *)application;
- (BOOL)application:(NSApplication *)application openFile:(NSString *)path;
- (void)application:(NSApplication *)application openFiles:(NSArray *)pathArray;
- (BOOL)application:(NSApplication *)application openFileWithoutUI:(NSString *)path;
- (BOOL)application:(NSApplication *)applicationsender openTempFile:(NSString *)path;
- (BOOL)applicationShouldHandleReopen:(NSApplication *)application hasVisibleWindows:(BOOL)visible;

- (BOOL)application:(NSApplication *)application printFile:(NSString *)path;
- (NSApplicationPrintReply)application:(NSApplication *)application printFiles:(NSArray *)pathArray withSettings:(NSDictionary *)settings showPrintPanels:(BOOL)showPanel;

- (NSMenu *)applicationDockMenu:(NSApplication *)application;
- (BOOL)application:(NSApplication *)application delegateHandlesKey:(NSString *)key;

- (NSError *)application:(NSApplication *)application willPresentError:(NSError *)error;

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)application;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)application;

@end

@protocol NSApplicationDelegate

// FIXME: @optional is broken in compiler
//@optional

- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename;
- (void)application:(NSApplication *)sender openFiles:(NSArray *)filenames;
- (BOOL)application:(id)sender openFileWithoutUI:(NSString *)filename;
- (BOOL)application:(NSApplication *)theApplication openTempFile:(NSString *)filename;
- (BOOL)application:(NSApplication *)theApplication printFile:(NSString *)filename;
- (NSApplicationPrintReply)application:(NSApplication *)application printFiles:(NSArray *)fileNames withSettings:(NSDictionary *)printSettings showPrintPanels:(BOOL)showPrintPanels;
- (NSError *)application:(NSApplication *)application willPresentError:(NSError *)error;
- (void)applicationDidBecomeActive:(NSNotification *)aNotification;
- (void)applicationDidChangeScreenParameters:(NSNotification *)aNotification;
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification;
- (void)applicationDidHide:(NSNotification *)aNotification;
- (void)applicationDidResignActive:(NSNotification *)aNotification;
- (void)applicationDidUnhide:(NSNotification *)aNotification;
- (void)applicationDidUpdate:(NSNotification *)aNotification;
- (NSMenu *)applicationDockMenu:(NSApplication *)sender;
- (BOOL)applicationOpenUntitledFile:(NSApplication *)theApplication;
- (BOOL)applicationShouldHandleReopen:(NSApplication *)theApplication hasVisibleWindows:(BOOL)flag;
- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender;
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication;
- (void)applicationWillBecomeActive:(NSNotification *)aNotification;
- (void)applicationWillFinishLaunching:(NSNotification *)aNotification;
- (void)applicationWillHide:(NSNotification *)aNotification;
- (void)applicationWillResignActive:(NSNotification *)aNotification;
- (void)applicationWillTerminate:(NSNotification *)aNotification;
- (void)applicationWillUnhide:(NSNotification *)aNotification;
- (void)applicationWillUpdate:(NSNotification *)aNotification;

@end

APPKIT_EXPORT int NSApplicationMain(int argc, const char *argv[]);

APPKIT_EXPORT void NSUpdateDynamicServices(void);
APPKIT_EXPORT BOOL NSPerformService(NSString *itemName, NSPasteboard *pasteboard);
