#import <AppKit/AppKit.h>
#import "MyView.h"

int main(int argc, const char *argv[])
{
    __NSInitializeProcess(argc, argv);

	NSLog(@"Hello from Bar.app");
	NSString *appName = [[NSProcessInfo processInfo] processName];
    NSAutoreleasePool *pool = [NSAutoreleasePool new];

	NSBundle *main = [[NSBundle mainBundle] autorelease];
	NSString *path = [[main pathForResource:@"sample" ofType:@"txt" inDirectory:@"rsc"]
		autorelease];
	NSFileManager *fm = [[NSFileManager defaultManager] autorelease];
	NSData *text = [[fm contentsAtPath:path] autorelease];
	NSLog(@"%s", [[[NSString alloc] initWithData:text encoding:NSASCIIStringEncoding] UTF8String]);

    [NSApplication sharedApplication];

    NSImage *image = [[[NSImage alloc] initWithContentsOfFile:@"/Users/zoe/helium/crazyones.jpg"]
		autorelease];
    NSSize imageSize = [image size];

    NSRect rect = NSMakeRect(0,0,imageSize.width, imageSize.height);
    NSWindow *window = [[[NSWindow alloc] initWithContentRect:rect 
        styleMask:NSTitledWindowMask backing:NSBackingStoreBuffered defer:NO] autorelease];
    [window cascadeTopLeftFromPoint:NSMakePoint(10,10)];
    [window setTitle:[appName stringByAppendingString:@" - crazy ones"]];
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
    [view2 setNeedsDisplay:YES];
	[window2 display];

    [NSApp run];
    return 0;
}
