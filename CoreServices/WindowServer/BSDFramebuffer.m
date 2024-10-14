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
#import <sys/types.h>
#import <sys/ipc.h>
#import <sys/shm.h>
#import "rpc.h" // for mode definitions

@implementation BSDFramebuffer

- (id)init
{
    self = [super init];
    fbfd = -1;
    stride = -1;
    data = NULL;
    size = 0;
    ctx = NULL;
    ctx2 = NULL;
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

    _currentMode->width = width;
    _currentMode->height = height;
    _currentMode->refresh = 0; // FIXME: can we get this?
    _currentMode->flags = 0;

    size_t pagemask = getpagesize() - 1;
    size = (stride * height + pagemask) & ~pagemask;
    data = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_NOCORE|MAP_NOSYNC, fbfd, 0);

    if(data == MAP_FAILED) {
        perror("mmap");
        return -1;
    }

    NSLog(@"fb geometry: %dx%d depth %d stride %d size %d", width, height, depth, stride, size);

    cs = CGColorSpaceCreateDeviceRGB();
    ctx = [O2Context createWithBytes:NULL width:width height:height 
                bitsPerComponent:8 bytesPerRow:0 colorSpace:(__bridge O2ColorSpaceRef)cs
                bitmapInfo:[self format] releaseCallback:NULL releaseInfo:NULL];
    ctxPixels = [[ctx surface] pixelBytes];
    ctx2 = [O2Context createWithBytes:NULL width:width height:height 
            bitsPerComponent:8 bytesPerRow:0 colorSpace:(__bridge O2ColorSpaceRef)cs
            bitmapInfo:[self format] releaseCallback:NULL releaseInfo:NULL];
    ctx2Pixels = [[ctx2 surface] pixelBytes];
    activeCtx = ctx;
    return 0;
}

- (void)dealloc
{
    [self releaseCapture];
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

// clear screen. does not swap active buffer
-(void)clear {
    void *pixels = NULL;
    if(_captured)
        pixels = [[captureCtx surface] pixelBytes];
    else 
        pixels = ctxPixels;

    O2ContextSetRGBFillColor(activeCtx, 0, 0, 0, 1);
    O2ContextFillRect(activeCtx, (O2Rect)NSMakeRect(0, 0, width, height));
    memcpy(data, pixels, size);
}

- (void)draw
{
    void *pixels = 0;
    if(_captured)
        pixels = ctx2Pixels;
    else
        pixels = ctxPixels;
    memcpy(data, pixels, size); // FIXME: this is slooowwww
}

- (void)drawWithCursor:(O2Image *)cursor inRect:(O2Rect)rect {
    O2ContextDrawImage(ctx2, NSMakeRect(0,0,width,height), [captureCtx surface]);
    O2ContextSetBlendMode(ctx2, kCGBlendModeNormal);
    O2ContextDrawImage(ctx2, rect, cursor);
    memcpy(data, ctx2Pixels, size); // FIXME: this is slooowwww
}

// return the context for drawing, i.e. the back buffer
- (O2Context *)context
{
    if(_captured)
        return captureCtx;
    else
        return ctx;
}

-(BOOL)capture:(pid_t)pid withOptions:(uint32_t)options {
    if(_captured != 0)
        return NO;

    pthread_mutex_lock(&renderLock);
    _captured = pid;

    int reserved = 6*sizeof(int); // save space for dimensions info 
    shmSize = size + reserved;
    shmid = shmget([self getDisplayID] ^ random(), shmSize, IPC_CREAT|0666);
    if(shmid == 0)
        return NO;

    uint8_t *p = shmat(shmid, NULL, 0);
    if(!p) {
        shmctl(shmid, IPC_RMID, NULL);
        shmid = 0;
        return NO;
    }

    uint8_t *bufaddr = (p + reserved);
    captureCtx = [[O2Context_builtin alloc] initWithBytes:(void *)bufaddr
                width:width height:height bitsPerComponent:8 bytesPerRow:width*4
                colorSpace:(__bridge O2ColorSpaceRef)cs
                bitmapInfo:[self format] releaseCallback:NULL releaseInfo:NULL];
    activeCtx = captureCtx;
    pthread_mutex_unlock(&renderLock);
    intptr_t *q = (intptr_t *)p;
    q[0] = width;
    q[1] = height;
    q[2] = [self format];
    
    // we ignore the deprecated options and always fill with black
    [self clear];
    return YES;
}

-(void)releaseCapture {
    pthread_mutex_lock(&renderLock);
    _captured = 0;
    activeCtx = ctx;
    if(captureCtx != nil) {
        shmctl(shmid, IPC_RMID, NULL);
        void *buffer = [[captureCtx surface] pixelBytes];
        buffer -= 6*sizeof(int);
        shmdt(buffer);
        shmid = 0;
        shmSize = 0;
    }
    captureCtx = nil;
    pthread_mutex_unlock(&renderLock);
}

/* FIXME: this should hash the vendor, model, serial, and other data */
- (uint32_t)getDisplayID {
    return 0xf07f0a10; // arbitrary ID
}

@end
