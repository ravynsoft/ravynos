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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "WLWindow.h"
#include "xdg-shell-client-protocol.h"

#import <CoreGraphics/CoreGraphics.h>
#import <Onyx2D/O2Surface.h>
#import <Onyx2D/O2ImageSource_PNG.h>
#import <Onyx2D/O2ImageSource.h>

struct wl_display *display;
WLWindow *win;
int ready = 0;
CGImageRef icon;

static void draw(void *data, struct wl_callback *cb, uint32_t time);
static const struct wl_callback_listener frame_listener = {
    .done = draw,
};

static void draw(void *data, struct wl_callback *cb, uint32_t time) {
    static int fn = 0;
    static struct timespec last;

    if(cb != NULL)
        wl_callback_destroy(cb);
    cb = wl_surface_frame([win wl_surface]);
    wl_callback_add_listener(cb, &frame_listener, NULL);

    CGContextRef ctx = (__bridge CGContextRef)[win cgContext];
    O2Surface *surf = [[win cgContext] surface];
    static float color[3] = {0.3, 0.8, 0.8};
    static float inc = 1, inc2 = 1, inc3 = 1;
    static CGRect imgrect = {
        .origin.x = 100, .origin.y = 100,
        .size.width = 128, .size.height = 128,
    };

    int width = O2ImageGetWidth(surf);
    int height = O2ImageGetHeight(surf);
    CGContextSetRGBFillColor(ctx, color[0], 0.3, color[2], 1);
    CGContextFillRect(ctx, NSMakeRect(0,0,width,height));
    CGContextSetRGBFillColor(ctx, 0.1, color[1], color[1], 1);
    CGContextFillRect(ctx, NSMakeRect(100, 100, width - 200, height - 200));
    CGContextDrawImage(ctx, imgrect, icon);
    imgrect.origin.x += (inc2 * 5);
    imgrect.origin.y += (inc3 * 3);
    if(imgrect.origin.x > (width - 100 - imgrect.size.width) || imgrect.origin.x < 100)
        inc2 = (-1)*inc2;
    if(imgrect.origin.y > (height - 100 - imgrect.size.height) || imgrect.origin.y < 100)
        inc3 = (-1)*inc3;

    color[0] += (inc * 0.01);
    color[1] -= (inc * 0.01);
    color[2] -= (inc * 0.01);
    if(color[0] < 0.3 || color[0] > 0.8)
        inc = (-1)*inc;
    [win flushBuffer];
    
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    unsigned delta = ( (now.tv_sec * 1000) + (now.tv_nsec / 1000000) )
                   - ( (last.tv_sec * 1000) + (last.tv_nsec / 1000000) );
    last.tv_sec = now.tv_sec;
    last.tv_nsec = now.tv_nsec;
    fprintf(stderr, "frame %d %ux%u RGBA (%d fps) ctx %p color %.2f %.2f %.02f       \r",
        fn++, width, height, 1000/delta, ctx, color[0], color[1], color[2]);
}


int main(int argc, char *argv[]) {
        if(getenv("XDG_RUNTIME_DIR") == NULL) {
            char *buf = 0;
            asprintf(&buf, "/tmp/runtime.%u", getuid());
            setenv("XDG_RUNTIME_DIR", buf, 0);
            if(access(buf, R_OK|W_OK|X_OK) != 0) {
                switch(errno) {
                    case ENOENT: mkdir(buf, 0700); break;
                    default: perror("WindowServer"); exit(-1);
                }
            }
            free(buf);
        }

    display = wl_display_connect(NULL);

    win = [[WLWindow alloc] initWithFrame:NSMakeRect(0,0,1280,720)
        styleMask:0 isPanel:NO backingType:0];

    while(!ready)
        wl_display_roundtrip(display);

    CFDataRef data = (__bridge CFDataRef)[NSData dataWithContentsOfFile:@"Icon.png"];
    O2ImageSource_PNG *imgsrc = [O2ImageSource_PNG newImageSourceWithData:data options:nil];
    icon = (__bridge CGImageRef)[imgsrc createImageAtIndex:0 options:nil];
    fprintf(stderr, "\n\n");
    draw(NULL, NULL, 0);

    do {
    } while(wl_display_dispatch(display)); 
}

