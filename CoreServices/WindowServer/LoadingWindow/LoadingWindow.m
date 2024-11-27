/*
 * Copyright (C) 2024 Zoe Knox <zoe@pixin.net>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#import "LoadingWindow.h"
#import <unistd.h>

NSTimer *timer;

@implementation LoadingWindow
- init {
    NSScreen *screen = [NSScreen mainScreen];
    self = [super initWithContentRect:[screen frame] styleMask:NSBorderlessWindowMask
        backing:NSBackingStoreBuffered defer:NO];
    [self setLevel:kCGMaximumWindowLevelKey];
    return self;
}

-(void)applicationWillFinishLaunching:(NSNotification *)note {
    backdrop = [[NSImageView alloc] initWithFrame:[self frame]];
    NSString *path = [[NSBundle mainBundle] pathForResource:@"splash-1" ofType:@"png"];
    NSImage *splash = [[NSImage alloc] initWithContentsOfFile:path];
    [splash setScalesWhenResized:YES];
    [splash setSize:NSMakeSize([self frame].size.width, [self frame].size.height)];
    [backdrop setImage:splash];

    float width = [self frame].size.width;
    float height = [self frame].size.height;

    spinner = [[NSProgressIndicator alloc]
        initWithFrame:NSMakeRect(width / 2 - 32, height / 4, 64, 64)];
    [spinner setControlSize:NSRegularControlSize];
    [spinner setIndeterminate:YES];
    [spinner setAnimationDelay:0.025];
    [spinner setUsesThreadedAnimation:NO];
    [spinner setStyle:NSProgressIndicatorSpinningStyle];

    [backdrop addSubview:spinner];
    [[self contentView] addSubview:backdrop];
    [spinner startAnimation:nil];

    [self becomeMainWindow];
    [self makeKeyAndOrderFront:nil];

    timer = [NSTimer scheduledTimerWithTimeInterval:0.05 target:nil selector:NULL
                                   userInfo:nil repeats:YES];
    [NSThread detachNewThreadSelector:@selector(watchForFile:) toTarget:self withObject:nil];
}

-(void)watchForFile:(id)object {
    while(access("/var/run/windowserver", F_OK) != 0) {
        usleep(50000);
    }

    [spinner stopAnimation:nil];
    [NSApp terminate:self];
}

@end

int main(int argc, const char *argv[]) {
    __NSInitializeProcess(argc, argv);

    @autoreleasepool {
        NSApplication *app = [NSApplication sharedApplication];
        LoadingWindow *del = [LoadingWindow new];
        [NSApp setDelegate:del];
        [NSApp run];
    }
    exit(0);
}


