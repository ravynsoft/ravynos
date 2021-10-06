#import <AppKit/AppKit.h>
#import "MyView.h"

int main(int argc, const char *argv[])
{
    __NSInitializeProcess(argc, argv);

	NSString *appName = [[NSProcessInfo processInfo] processName];
        [NSApplication sharedApplication];

        NSAutoreleasePool *pool = [NSAutoreleasePool new];

	NSBundle *main = [[NSBundle mainBundle] retain];
	NSString *path = [main pathForResource:@"sample" ofType:@"txt" inDirectory:@"rsc"];
	NSFileManager *fm = [NSFileManager defaultManager];
	NSString *text = [[NSString alloc] initWithData:[fm contentsAtPath:path]
		encoding:NSASCIIStringEncoding];


	NSMenu *menubar = [[NSMenu new] autorelease];
	NSMenuItem *fileMenuItem = [[NSMenuItem new] autorelease];
        NSMenuItem *windowsMenuItem = [[NSMenuItem new] autorelease];
	[menubar addItem:fileMenuItem];
        [menubar addItem:windowsMenuItem];
        [fileMenuItem setTitle:@"File"];
        [windowsMenuItem setTitle:@"Window"];


    NSMenu *fileMenu = [[NSMenu new] autorelease];
    NSMenuItem *openMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Open"
        action:@selector(openFile:) keyEquivalent:@"o"] autorelease];
    [fileMenu addItem:openMenuItem];
    [fileMenuItem setSubmenu:fileMenu];

    NSMenu *windowsMenu = [[NSMenu new] autorelease];
    [windowsMenuItem setSubmenu:windowsMenu];
    [NSApp setWindowsMenu:windowsMenu];

    NSImage *image = [[NSImage alloc] initWithContentsOfFile:
    	[main pathForResource:@"Cloudy_Mountains" ofType:@"jpg"]];
    NSSize imageSize = [image size];

    NSRect rect = NSMakeRect(0,0,imageSize.width, imageSize.height);
    NSWindow *window = [[NSWindow alloc] initWithContentRect:rect 
        styleMask:NSTitledWindowMask backing:NSBackingStoreBuffered defer:NO];
    [window cascadeTopLeftFromPoint:NSMakePoint(10,10)];
    [window setTitle:appName];
    [window makeKeyAndOrderFront:nil];

	NSImageView *view = [[NSImageView alloc] init];
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
	[view2 setFont: [NSFont fontWithName:@"URW Gothic" size:20.0]];
	[view2 setText:text];
    [view2 setNeedsDisplay:YES];
	[window2 display];
	[NSApp setMainMenu:menubar];

    [pool release];
    [NSApp run];
    return 0;
}

@implementation NSApplication(CocoaDemo)
-(void)openFile:sender {
//    NSRunAlertPanel(@"Open File", @"File selection panel is not implemented yet",
//        @"OK", nil, nil);
    [NSApp performSelectorOnMainThread:@selector(openWin3) withObject:nil waitUntilDone:NO modes:[NSArray arrayWithObjects:NSDefaultRunLoopMode,NSModalPanelRunLoopMode,nil]];
}

-(void)openWin3 {
    // deliberately leak the object
    NSWindow *win3 = [[[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,200,200)
        styleMask:NSTitledWindowMask backing:NSBackingStoreBuffered defer:NO] retain];
    [win3 setTitle:@"Popup 3"];
    [win3 cascadeTopLeftFromPoint:NSMakePoint(50,50)];
    [win3 makeKeyAndOrderFront:nil];
    [win3 display];
    [NSApp addWindowsItem:win3 title:@"POPUP" filename:NO];
}

@end

