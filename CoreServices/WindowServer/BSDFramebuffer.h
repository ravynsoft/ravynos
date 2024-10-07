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


#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <Onyx2D/O2Context.h>
#import <Onyx2D/O2BitmapContext.h>
#import <Onyx2D/O2Context_builtin.h>
#import <Onyx2D/O2Surface.h>

#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include <signal.h>
#include <sys/consio.h>
#include <sys/fbio.h>

#import "WSDisplay.h"

#define DEFAULT_DPI = 100

@interface BSDFramebuffer : WSDisplay {
    int fbfd;
    int stride;
    uint8_t *data;
    int offset;
    int size;
    O2Context *ctx, *ctx2;
    void *ctxPixels;
    void *ctx2Pixels;
    BOOL _doubleBuffered;
}

- (id)init;
- (int)openFramebuffer: (const char *)device;
- (void)dealloc;
- (BOOL)useDoubleBuffer:(BOOL)val;
- (O2Context *)context;
- (CGColorSpaceRef)colorSpace;

@end

