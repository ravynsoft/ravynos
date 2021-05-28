#import <AppKit/AppKit.h>
#import "MyView.h"

int main(int argc, const char *argv[])
{
    __NSInitializeProcess(argc, argv);

	NSString *appName = [[NSProcessInfo processInfo] processName];
    NSAutoreleasePool *pool = [NSAutoreleasePool new];

	NSBundle *main = [[NSBundle mainBundle] autorelease];
	NSString *path = [[main pathForResource:@"sample" ofType:@"txt" inDirectory:@"rsc"]
		autorelease];
	NSFileManager *fm = [[NSFileManager defaultManager] autorelease];
	NSString *text = [[[NSString alloc] initWithData:[fm contentsAtPath:path]
		encoding:NSASCIIStringEncoding] autorelease];

    [NSApplication sharedApplication];

	NSMenu *menubar = [[NSMenu new] autorelease];
	NSMenuItem *appMenuItem = [[NSMenuItem new] autorelease];
	NSMenuItem *fileMenuItem = [[NSMenuItem new] autorelease];
	[menubar addItem:appMenuItem];
	[menubar addItem:fileMenuItem];
	[appMenuItem setTitle:appName];
    [fileMenuItem setTitle:@"File"];

	NSMenu *appMenu = [[NSMenu new] autorelease];
	NSMenuItem *quitMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Quit"
		action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
	[appMenu addItem:quitMenuItem];
	[appMenuItem setSubmenu:appMenu];

    NSMenu *fileMenu = [[NSMenu new] autorelease];
    NSMenuItem *openMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Open"
        action:@selector(terminate:) keyEquivalent:@"o"] autorelease];
    [fileMenu addItem:openMenuItem];
    [fileMenuItem setSubmenu:fileMenu];

	[NSApp setMainMenu:menubar];

    NSImage *image = [[[NSImage alloc] initWithContentsOfFile:
    	[main pathForResource:@"helium" ofType:@"jpg"]] autorelease];
    NSSize imageSize = [image size];

    NSRect rect = NSMakeRect(0,0,imageSize.width, imageSize.height);
    NSWindow *window = [[[NSWindow alloc] initWithContentRect:rect 
        styleMask:NSTitledWindowMask backing:NSBackingStoreBuffered defer:NO] autorelease];
    [window cascadeTopLeftFromPoint:NSMakePoint(10,10)];
    [window setTitle:appName];
    [window makeKeyAndOrderFront:nil];

	NSImageView *view = [[[NSImageView alloc] init] autorelease];
	[view setImage:image];
	[window setContentView:view];
	[view setNeedsDisplay:YES];
    [window display];

    rect = NSMakeRect(0,0,200,200);
    NSWindow *window2 = [[[NSWindow alloc] initWithContentRect:rect 
        styleMask:NSTitledWindowMask backing:NSBackingStoreBuffered defer:NO] autorelease];
    [window2 cascadeTopLeftFromPoint:NSMakePoint(imageSize.width+20,10)];
    [window2 setTitle:[appName stringByAppendingString:@" - custom view"]];
    [window2 makeKeyAndOrderFront:nil];

    MyView *view2 = [[[MyView alloc] initWithFrame:[window2 frame]] autorelease];
    [window2 setContentView:view2];
	[view2 setFont: [NSFont fontWithName:@"NimbusSans-Regular" size:16.0]];
	[view2 setText:text];
    [view2 setNeedsDisplay:YES];
	[window2 display];

    [NSApp run];
    return 0;
}
