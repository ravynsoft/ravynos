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
    fbfd = -1;
    stride = -1;
    data = NULL;
    offset = 0;
    size = 0;
    width = 0;
    height = 0;
    depth = 0;
    cs = NULL;
    ctx = NULL;
    return self;
}

- (BOOL)openFramebuffer: (const char *)device
{
    int bytesperline = 0;
    struct fbtype fb;

    fbfd = open(device, O_RDWR);
    if(fbfd < 0)
        fbfd = open(device, O_RDONLY);
    if(fbfd < 0)
        return FALSE;

    if(ioctl(fbfd, FBIOGTYPE, &fb) != 0)
        return FALSE;

    if(ioctl(fbfd, FBIO_GETLINEWIDTH, &bytesperline) != 0)
        return FALSE;

    depth = fb.fb_depth;
    width = fb.fb_width;
    height = fb.fb_height;
    stride = bytesperline;

    size_t pagemask = getpagesize() - 1;
    size = (stride * height + pagemask) & ~pagemask;
    data = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fbfd, 0);
    if(data == MAP_FAILED) 
        return FALSE;

#ifdef DEBUG
    NSLog(@"fb geometry: %dx%d depth %d stride %d size %d", width, height, depth, stride, size);
#endif

    cs = CGColorSpaceCreateDeviceRGB();
    ctx = CGBitmapContextCreate(NULL, width, height, 8, 0, cs, [self format]);
    CGContextSetRGBFillColor(ctx, 255.0, 0, 0, 255.0);
    CGRect rect = CGRectMake(0, 0, width, height);
    CGContextFillRect(ctx, rect);

    CGContextSetRGBFillColor(ctx, 255.0, 255.0, 0, 255.0);
    rect = CGRectMake(width/2 - 300, height/2 - 300, 600, 600);
    CGContextFillRect(ctx, rect);
    
    return TRUE;
}

- (NSRect)geometry
{
    return NSMakeRect(0, 0, width, height);
}

- (void)dealloc
{
    if(fbfd >= 0) {
        munmap(data - offset, size);
        close(fbfd);
    }
    if(cs)
        CGColorSpaceRelease(cs);
    if(ctx)
        CGContextRelease(ctx);
    [super dealloc];
}

- (int)format
{
    switch(depth) {
        case 32:
            return kCGBitmapByteOrder32Little | kCGImageAlphaPremultipliedFirst;
        case 24:
            return 0; // invalid depth 
        default:
            return kCGBitmapByteOrder16Little | kCGImageAlphaPremultipliedFirst;
    }
}

- (void)draw
{
    memcpy(data, [[(O2Context *)ctx surface] pixelBytes], size);
}

@end
