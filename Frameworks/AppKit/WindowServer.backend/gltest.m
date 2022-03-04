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

#import <AppKit/AppKit.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <pthread.h>

#import <CoreGraphics/CoreGraphics.h>
#import <Onyx2D/O2Surface.h>
#import <Onyx2D/O2ImageSource_PNG.h>
#import <Onyx2D/O2ImageSource.h>

CGImageRef icon;

void *draw(void *data) {
    static int fn = 0;
    static struct timespec last;

    NSWindow *win = (__bridge void *)data;

    while ([[win platformWindow] isReady] == NO) {
        NSLog(@"waiting on window");
        sleep(1);
    }

    CGContextRef ctx = (__bridge CGContextRef)[win cgContext];
    static float color[3] = {0.3, 0.8, 1};
    static float inc = 1, inc2 = 1, inc3 = 1;
    static CGRect imgrect = {
        .origin.x = 100, .origin.y = 100,
        .size.width = 128, .size.height = 128,
    };

    while(1) {
        CGRect rect = [[win platformWindow] frame]; // this is the content frame inside decorations
        CGContextSetGrayFillColor(ctx, 0.666, 1);
        CGContextFillRect(ctx, NSMakeRect(rect.origin.x,rect.origin.y,
            rect.size.width,rect.size.height));
        CGContextSetGrayStrokeColor(ctx, 0, 1);
        CGContextStrokeRect(ctx, NSMakeRect(rect.origin.x + 100, rect.origin.y + 100,
            rect.size.width - 200, rect.size.height - 200));
        CGContextDrawImage(ctx, imgrect, icon);
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
        if(1000/delta > 24)
            usleep(5000);
        last.tv_sec = now.tv_sec;
        last.tv_nsec = now.tv_nsec;
        fprintf(stderr, "frame %d %.0fx%.0f RGBA (%d fps) ctx %p     \r",
            fn++, rect.size.width, rect.size.height, 1000/delta, ctx);
    }
}

int main(int argc, char *argv[]) {
    __NSInitializeProcess(argc, argv);

    [NSApplication sharedApplication];
    NSWindow *win = [[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,1280,720)
        styleMask:NSTitledWindowMask backing:NSBackingStoreBuffered defer:NO];

    CFDataRef data = (__bridge CFDataRef)[NSData dataWithContentsOfFile:@"Icon.png"];
    O2ImageSource_PNG *imgsrc = [O2ImageSource_PNG newImageSourceWithData:data options:nil];
    icon = (__bridge CGImageRef)[imgsrc createImageAtIndex:0 options:nil];

    pthread_t thread;

    [win setTitle:@"An AppKit Window"];
    NSTextField *tf = [[NSTextField alloc] initWithFrame:NSMakeRect(20, 100, 600, 140)];
    [tf setEditable:YES];
    [tf setFont:[NSFont systemFontOfSize:16]];
    [win setContentView:tf];
    [win makeKeyAndOrderFront:nil];

    //pthread_create(&thread, NULL, draw, win);

    [NSApp run];
    return 0;
}

