/*
 * Copyright (C) 2024 Zoe Knox <zoe@ravynsoft.com>
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

#import "BSDFramebuffer.h"

@implementation BSDFramebuffer

- (id)init
{
    self = [super init];
    fbfd = -1;
    stride = -1;
    data = NULL;
    size = 0;
    width = 0;
    height = 0;
    depth = 0;
    cs = NULL;
    ctx = NULL;
    ctx2 = NULL;
    activeCtx = ctx;
    _doubleBuffered = NO;
    return self;
}

- (int)openFramebuffer: (const char *)device
{
    struct fbtype fb;

    fbfd = open(device, O_RDWR);
    if(fbfd < 0) {
        perror(device);
        return -1;
    }

    if(ioctl(fbfd, FBIOGTYPE, &fb) < 0) {
        perror("FBIOGTYPE");
        close(fbfd);
        return -1;
    }

    if(ioctl(fbfd, FBIO_GETLINEWIDTH, &stride) < 0) {
        perror("FBIO_GETLINEWIDTH");
        close(fbfd);
        return -1;
    }

    depth = fb.fb_depth;
    width = fb.fb_width;
    height = fb.fb_height;

    size_t pagemask = getpagesize() - 1;
    size = (stride * height + pagemask) & ~pagemask;
    data = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_NOCORE|MAP_NOSYNC, fbfd, 0);

    if(data == MAP_FAILED) {
        perror("mmap");
        return -1;
    }

    NSLog(@"fb geometry: %dx%d depth %d stride %d size %d", width, height, depth, stride, size);

    cs = CGColorSpaceCreateDeviceRGB();
    ctx = [O2BitmapContext createWithBytes:NULL width:width height:height 
                bitsPerComponent:8 bytesPerRow:0 colorSpace:(__bridge O2ColorSpaceRef)cs
                bitmapInfo:[self format] releaseCallback:NULL releaseInfo:NULL];
    ctxPixels = [[ctx surface] pixelBytes];

    ctx2 = [O2BitmapContext createWithBytes:NULL width:width height:height 
                bitsPerComponent:8 bytesPerRow:0 colorSpace:(__bridge O2ColorSpaceRef)cs
                bitmapInfo:[self format] releaseCallback:NULL releaseInfo:NULL];
    ctx2Pixels = [[ctx2 surface] pixelBytes];
    activeCtx = ctx;
    return 0;
}

- (NSRect)geometry
{
    return NSMakeRect(0, 0, width, height);
}

- (void)dealloc
{
    if(fbfd >= 0) {
        munmap(data, size);
        close(fbfd);
    }
    if(cs)
        CGColorSpaceRelease(cs);
    ctx = nil;
    activeCtx = nil;
    ctx2 = nil;
}

- (int)format
{
    switch(depth) {
        case 32:
            return kCGBitmapByteOrderDefault | kCGImageAlphaPremultipliedFirst;
        case 24:
            return kCGBitmapByteOrderDefault | kCGImageAlphaNone;
        default:
            return kCGBitmapByteOrderDefault | kCGImageAlphaNone;
    }
}

- (int)getDepth {
    return depth;
}

// clear screen. does not swap active buffer
-(void)clear {
    O2ContextSetRGBFillColor(activeCtx, 0, 0, 0, 1);
    O2ContextFillRect(activeCtx, (O2Rect)[self geometry]);
    memcpy(data, activeCtx == ctx ? ctxPixels : ctx2Pixels, size);
}

-(BOOL)useDoubleBuffer:(BOOL)val {
    BOOL oldval = _doubleBuffered;
    _doubleBuffered = val;
    return oldval;
}

// draw the back buffer to the front and make it active
- (void)draw
{
    void *pixels = 0;
    if(_doubleBuffered) {
        if(activeCtx == ctx) {
            activeCtx = ctx2;
            pixels = ctx2Pixels;
        } else {
            activeCtx = ctx;
            pixels = ctxPixels;
        }
    } else
	pixels = ctxPixels;
    memcpy(data, pixels, size); // FIXME: this is slooowwww
}

// return the context for drawing, i.e. the back buffer
- (O2BitmapContext *)context
{
    if(_doubleBuffered)
        return (activeCtx == ctx) ? ctx2 : ctx;
    else
        return ctx;
}

- (CGColorSpaceRef)colorSpace
{
    return cs;
}

/* FIXME: this should hash the vendor, model, serial, and other data */
- (uint32_t)getDisplayID {
    return 0xf07f0a10; // arbitrary ID
}

@end
