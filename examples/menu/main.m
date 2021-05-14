#import <AppKit/AppKit.h>

int main(int argc, const char *argv[])
{
    __NSInitializeProcess(argc, argv);

	NSString *appName = [[NSProcessInfo processInfo] processName];
    NSAutoreleasePool *pool = [NSAutoreleasePool new];

	NSBundle *main = [[NSBundle mainBundle] autorelease];
    [NSApplication sharedApplication];

	NSMenu *menubar = [[NSMenu new] autorelease];
	NSMenuItem *appMenuItem = [[NSMenuItem new] autorelease];
	[menubar addItem:appMenuItem];
	[appMenuItem setTitle:appName];

	NSMenu *appMenu = [[NSMenu new] autorelease];
	NSMenuItem *quitMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Quit"
		action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
	[appMenu addItem:quitMenuItem];

	[appMenuItem setSubmenu:appMenu];
	[NSApp setMainMenu:menubar];

    NSRect rect = NSMakeRect(0,0,400,240);
    NSWindow *window = [[[NSWindow alloc] initWithContentRect:rect 
        styleMask:NSTitledWindowMask backing:NSBackingStoreBuffered defer:NO] autorelease];
    [window cascadeTopLeftFromPoint:NSMakePoint(10,10)];
    [window setTitle:appName];
    [window makeKeyAndOrderFront:nil];
    [window display];

    [NSApp run];
    return 0;
}
