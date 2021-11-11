#import <AppKit/AppKit.h>
#import "MyView.h"

int main(int argc, const char *argv[])
{
    __NSInitializeProcess(argc, argv);

    [NSApplication sharedApplication];
    NSAutoreleasePool *pool = [NSAutoreleasePool new];

    NSMenu *menubar = [[NSMenu new] autorelease];
    NSMenuItem *windowsMenuItem = [[NSMenuItem new] autorelease];
    [menubar addItem:windowsMenuItem];
    [windowsMenuItem setTitle:@"Window"];

    NSMenu *windowsMenu = [[NSMenu new] autorelease];
    [windowsMenuItem setSubmenu:windowsMenu];
    [NSApp setWindowsMenu:windowsMenu];

    NSRect rect = NSMakeRect(0,0,700,400);
    NSWindow *window2 = [[[[NSWindow alloc] initWithContentRect:rect
        styleMask:NSTitledWindowMask backing:NSBackingStoreBuffered defer:NO] retain] autorelease];
    [window2 cascadeTopLeftFromPoint:NSMakePoint(20,10)];
    [window2 setTitle:@"Swatches"];
    [window2 makeKeyAndOrderFront:nil];

    MyView *view2 = [[[[MyView alloc] initWithFrame:[window2 frame]] retain] autorelease];
    [window2 setContentView:view2];
    [view2 setNeedsDisplay:YES];
    [window2 display];
    [NSApp setMainMenu:menubar];

    [pool release];
    [NSApp run];
    return 0;
}
