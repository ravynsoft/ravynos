/*
 * Copyright (C) 2022 Zoe Knox <zoe@pixin.net>
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

#import <AppKit/NSApplication.h>
#import <AppKit/NSFont.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSView.h>
#import <AppKit/NSTextField.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>

NSImage *icon;
NSWindow *win;

@interface GLApplication: NSApplication
@end

void *draw(void *data) {
    static int fn = 0;
    static struct timespec last;

    //NSWindow *win = (__bridge void *)data;
    NSView *view = [win contentView];

    //while(1) {
        static float inc2 = 1, inc3 = 1;
        static NSRect imgrect = {
            .origin.x = 100, .origin.y = 100,
            .size.width = 128, .size.height = 128,
        };
        NSRect rect = [view bounds];

        [[win backgroundColor] setFill];
        NSRectFill(rect);
        [view lockFocus];
        [icon drawInRect:imgrect fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];
        [view unlockFocus];

        imgrect.origin.x += (inc2 * 5);
        imgrect.origin.y += (inc3 * 3);
        if(imgrect.origin.x > (rect.size.width - 100 - imgrect.size.width)
            || imgrect.origin.x < (rect.origin.x + 100))
            inc2 = (-1)*inc2;
        if(imgrect.origin.y > (rect.size.height - 100 - imgrect.size.height)
            || imgrect.origin.y < (rect.origin.y + 100))
            inc3 = (-1)*inc3;
        
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        unsigned delta = ( (now.tv_sec * 1000) + (now.tv_nsec / 1000000) )
                       - ( (last.tv_sec * 1000) + (last.tv_nsec / 1000000) );
        unsigned fps = (delta ? (1000/delta) : 0);
        if(!delta)
            usleep(500);
        last.tv_sec = now.tv_sec;
        last.tv_nsec = now.tv_nsec;
        fprintf(stderr, "frame %d RGBA (%d fps)      \r", fn++, fps);
    //}
}

@implementation GLApplication
-(void)timerFired:(NSTimer *)timer {
    draw(win);
    [win flushWindow];
}

-(void)run {
  static BOOL didlaunch = NO;
  NSAutoreleasePool *pool;

    win = [[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,1280,720)
        styleMask:NSTitledWindowMask backing:NSBackingStoreBuffered defer:NO];

    icon = [[NSImage alloc] initWithContentsOfFile:@"MarmosetLogo.tiff"];

    [win setTitle:@"An AppKit Window"];
    //NSTextField *tf = [[NSTextField alloc] initWithFrame:NSMakeRect(20, 100, 600, 140)];
    //[tf setEditable:YES];
    //[tf setFont:[NSFont systemFontOfSize:16]];
    //[win setContentView:tf];
    [win makeKeyAndOrderFront:nil];


  _isRunning=YES;

  if (!didlaunch) {
    didlaunch = YES;
    [self finishLaunching];
  }

   NSTimer *timer = [NSTimer scheduledTimerWithTimeInterval:0.01 target:self 
        selector:@selector(timerFired:) userInfo:nil repeats:YES];
   do {
    NSEvent           *event;

    event=[self nextEventMatchingMask:NSAnyEventMask untilDate:[NSDate distantFuture] inMode:NSDefaultRunLoopMode dequeue:YES];

    NS_DURING
     [self sendEvent:event];

    NS_HANDLER
     [self reportException:localException];
    NS_ENDHANDLER

    [self _checkForReleasedWindows];
    [self _checkForTerminate];

   }while(_isRunning);
   [timer invalidate];
}
@end

int main(int argc, const char *argv[]) {
    __NSInitializeProcess(argc, argv);

    GLApplication *app = [GLApplication new];
    NSApp = app;
    [app run];
    return 0;
}

