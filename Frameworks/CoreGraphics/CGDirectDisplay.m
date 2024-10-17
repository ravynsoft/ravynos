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

#import <mach/mach.h>
#import <sys/types.h>
#import <sys/ipc.h>
#import <sys/shm.h>
#import <sys/mman.h>
#import <CoreGraphics/CGDirectDisplay.h>
#import <CoreGraphics/CGError.h>
#import <Onyx2D/O2Context_builtin.h>
#import <CoreFoundation/CFMachPort.h>
#import <WindowServer/message.h>
#import <WindowServer/rpc.h>

const CFStringRef kCGDisplayStreamSourceRect = CFSTR("kCGDisplayStreamSourceRect");
const CFStringRef kCGDisplayStreamDestinationRect = CFSTR("kCGDisplayStreamDestinationRect");
const CFStringRef kCGDisplayStreamPreserveAspectRatio = CFSTR("kCGDisplayStreamPreserveAspectRatio");
const CFStringRef kCGDisplayStreamColorSpace = CFSTR("kCGDisplayStreamColorSpace");
const CFStringRef kCGDisplayStreamMinimumFrameTime = CFSTR("kCGDisplayStreamMinimumFrameTime");
const CFStringRef kCGDisplayStreamShowCursor = CFSTR("kCGDisplayStreamShowCursor");
const CFStringRef kCGDisplayStreamQueueDepth = CFSTR("kCGDisplayStreamQueueDepth");
const CFStringRef kCGDisplayStreamYCbCrMatrix = CFSTR("kCGDisplayStreamYCbCrMatrix");

const CFStringRef kCGDisplayStreamYCbCrMatrix_ITU_R_709_2 = CFSTR("kCGDisplayStreamYCbCrMatrix_ITU_R_709_2");
const CFStringRef kCGDisplayStreamYCbCrMatrix_ITU_R_601_4 = CFSTR("kCGDisplayStreamYCbCrMatrix_ITU_R_601_4");
const CFStringRef kCGDisplayStreamYCbCrMatrix_SMPTE_240M_1995 = CFSTR("kCGDisplayStreamYCbCrMatrix_SMPTE_240M_1995");

struct _CGDisplayConfig {
    struct _CGDisplayConfigInner *inner;
};

static struct CGDisplayStream {
    uintptr_t cfisa;
};

static struct CGDisplayStreamUpdate {
    uintptr_t cfisa;
};

// dictionary of display contexts
static CFMutableDictionaryRef __displayContexts = NULL;
static const CFDictionaryKeyCallBacks __CGDispCtxKeyCallback = {0}; // all NULL - use defaults
static const CFDictionaryValueCallBacks __CGDispCtxValCallback = {0}; // same

// make a RPC to WindowServer with optional reply
kern_return_t _windowServerRPC(void *data, size_t len, void *replyBuf, int *replyLen);
static uint32_t CGDisplayStateFlags(CGDirectDisplayID display);

// Finding displays
CGDirectDisplayID CGMainDisplayID(void) {
    struct wsRPCBase data = { kCGMainDisplayID, 0 };
    struct wsRPCSimple ID;
    int len = sizeof(ID);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &ID, &len);
    if(ret == KERN_SUCCESS)
        return ID.val1;
    return kCGNullDirectDisplay;
}

CGError CGGetDisplayList(uint32_t code, uint32_t maxDisplays, CGDirectDisplayID *displays, uint32_t *displayCount) {
    struct wsRPCBase data = { code, 0 };
    int replyLen = sizeof(data) + sizeof(CGDirectDisplayID)*maxDisplays;
    uint8_t *replyBuf = (uint8_t *)malloc(replyLen);

    if(replyBuf == NULL)
        return kCGErrorFailure;

    _windowServerRPC(&data, sizeof(data), replyBuf, &replyLen);
    if(replyLen < 0) {
        free(replyBuf);
        return kCGErrorFailure;
    }

    memcpy(&data, replyBuf, sizeof(data));
    uint32_t *list = (uint32_t *)(replyBuf + sizeof(data));

    int count = data.len / sizeof(CGDirectDisplayID);
    if(maxDisplays < count)
        count = maxDisplays;

    if(displays != NULL) {
        for(int i = 0; i < count; i++) 
            displays[i] = list[i];
    }
    *displayCount = count;

    free(replyBuf);
    return kCGErrorSuccess;
}

CGError CGGetOnlineDisplayList(uint32_t maxDisplays, CGDirectDisplayID *onlineDisplays, uint32_t *displayCount) {
    return CGGetDisplayList(kCGGetOnlineDisplayList, maxDisplays, onlineDisplays, displayCount);
}

CGError CGGetActiveDisplayList(uint32_t maxDisplays, CGDirectDisplayID *activeDisplays, uint32_t *displayCount) {
    return CGGetDisplayList(kCGGetActiveDisplayList, maxDisplays, activeDisplays, displayCount);
}

CGError CGGetDisplaysWithOpenGLDisplayMask(CGOpenGLDisplayMask mask, uint32_t maxDisplays, CGDirectDisplayID *displays, uint32_t *matchingDisplayCount) {
    struct wsRPCSimple data = { kCGGetDisplaysWithOpenGLDisplayMask, sizeof(CGOpenGLDisplayMask), mask };
    int replyLen = sizeof(struct wsRPCBase) + sizeof(CGDirectDisplayID)*maxDisplays;
    uint8_t *replyBuf = (uint8_t *)malloc(replyLen);

    if(replyBuf == NULL)
        return kCGErrorFailure;

    _windowServerRPC(&data, sizeof(data), replyBuf, &replyLen);
    if(replyLen < 0) {
        free(replyBuf);
        return kCGErrorFailure;
    }

    struct wsRPCBase *base = (struct wsRPCBase *)replyBuf;
    uint32_t *list = (uint32_t *)(replyBuf + sizeof(struct wsRPCBase));

    int count = base->len / sizeof(CGDirectDisplayID);
    if(maxDisplays < count)
        count = maxDisplays;

    if(displays != NULL) {
        for(int i = 0; i < count; i++) 
            displays[i] = list[i];
    }
    *matchingDisplayCount = count;

    free(replyBuf);
    return kCGErrorSuccess;
}

CGError CGGetDisplaysWithPoint(CGPoint point, uint32_t maxDisplays, CGDirectDisplayID *displays, uint32_t *matchingDisplayCount) {
    struct wsRPCSimple data = { kCGGetDisplaysWithPoint, 8 };
    data.val1 = (uint32_t)point.x;
    data.val2 = (uint32_t)point.y;

    int replyLen = sizeof(struct wsRPCBase) + sizeof(CGDirectDisplayID)*maxDisplays;
    uint8_t *replyBuf = (uint8_t *)malloc(replyLen);

    if(replyBuf == NULL)
        return kCGErrorFailure;

    _windowServerRPC(&data, sizeof(data), replyBuf, &replyLen);
    if(replyLen < 0) {
        free(replyBuf);
        return kCGErrorFailure;
    }

    struct wsRPCBase *base = (struct wsRPCBase *)replyBuf;
    uint32_t *list = (uint32_t *)(replyBuf + sizeof(struct wsRPCBase));

    int count = base->len / sizeof(CGDirectDisplayID);
    if(maxDisplays == 0)
        displays = NULL;
    else if(maxDisplays < count && displays != NULL)
        count = maxDisplays;

    if(displays != NULL) {
        for(int i = 0; i < count; i++) 
            displays[i] = list[i];
    }
    *matchingDisplayCount = count;

    free(replyBuf);
    return kCGErrorSuccess;
}

CGError CGGetDisplaysWithRect(CGRect rect, uint32_t maxDisplays, CGDirectDisplayID *displays, uint32_t *matchingDisplayCount) {
    struct wsRPCSimple data = { kCGGetDisplaysWithPoint, 16 };
    data.val1 = (uint32_t)rect.origin.x;
    data.val2 = (uint32_t)rect.origin.y;
    data.val3 = (uint32_t)rect.size.width;
    data.val4 = (uint32_t)rect.size.height;

    int replyLen = sizeof(struct wsRPCBase) + sizeof(CGDirectDisplayID)*maxDisplays;
    uint8_t *replyBuf = (uint8_t *)malloc(replyLen);

    if(replyBuf == NULL)
        return kCGErrorFailure;

    _windowServerRPC(&data, sizeof(data), replyBuf, &replyLen);
    if(replyLen < 0) {
        free(replyBuf);
        return kCGErrorFailure;
    }

    struct wsRPCBase *base = (struct wsRPCBase *)replyBuf;
    uint32_t *list = (uint32_t *)(replyBuf + sizeof(struct wsRPCBase));

    int count = base->len / sizeof(CGDirectDisplayID);
    if(maxDisplays == 0)
        displays = NULL;
    else if(maxDisplays < count && displays != NULL)
        count = maxDisplays;

    if(displays != NULL) {
        for(int i = 0; i < count; i++) 
            displays[i] = list[i];
    }
    *matchingDisplayCount = count;

    free(replyBuf);
    return kCGErrorSuccess;
}

CGDirectDisplayID CGOpenGLDisplayMaskToDisplayID(CGOpenGLDisplayMask mask) {
    uint32_t display;
    int32_t len = sizeof(display);

    struct wsRPCSimple data = { kCGOpenGLDisplayMaskToDisplayID, 4 };
    data.val1 = mask;

    _windowServerRPC(&data, sizeof(data), &display, &len);
    if(len < 0)
        return kCGNullDirectDisplay;
    return display;
}

CGOpenGLDisplayMask CGDisplayIDToOpenGLDisplayMask(CGDirectDisplayID display) {
    uint32_t mask;
    int32_t len = sizeof(mask);

    struct wsRPCSimple data = { kCGDisplayIDToOpenGLDisplayMask, 4 };
    data.val1 = display;

    _windowServerRPC(&data, sizeof(data), &mask, &len);
    if(len < 0)
        return 0;
    return mask;
}

// Capturing and Releasing Displays
CGError CGDisplayCapture(CGDirectDisplayID display) {
    return CGDisplayCaptureWithOptions(display, kCGCaptureNoOptions);
}

CGError CGDisplayCaptureWithOptions(CGDirectDisplayID display, CGCaptureOptions options) {
    struct wsRPCSimple data = { kCGDisplayCaptureWithOptions, 8 };
    data.val1 = display;
    data.val2 = options;
    int len = sizeof(data);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &data, &len);
    if(ret == KERN_SUCCESS)
        return data.val1;
    return kCGErrorFailure;
}

CGError CGDisplayRelease(CGDirectDisplayID display) {
    struct wsRPCSimple data = { kCGDisplayRelease, 4 };
    data.val1 = display;
    int len = sizeof(data);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &data, &len);
    if(ret == KERN_SUCCESS) {
        if(__displayContexts != NULL) {
            CGContextRef ctx = CFDictionaryGetValue(__displayContexts, [NSNumber numberWithInt:display]);
            if(ctx) {
                void *buffer = [[ctx surface] pixelBytes];
                buffer -= 6*sizeof(int);
                shmdt(buffer);
                CGContextRelease(ctx);
                CFDictionaryRemoveValue(__displayContexts, [NSNumber numberWithInt:display]);
            }
        }
        return data.val1;
    }
    return kCGErrorFailure;
}

CGError CGCaptureAllDisplays(void) {
    return CGCaptureAllDisplaysWithOptions(kCGCaptureNoOptions);
}

CGError CGCaptureAllDisplaysWithOptions(CGCaptureOptions options) {
    struct wsRPCSimple data = { kCGCaptureAllDisplaysWithOptions, 0 };
    data.val1 = options;
    int len = sizeof(data);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &data, &len);
    if(ret == KERN_SUCCESS)
        return data.val1;
    return kCGErrorFailure;
}

CGError CGReleaseAllDisplays(void) {
    struct wsRPCSimple data = { kCGReleaseAllDisplays, 0 };
    int len = sizeof(data);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &data, &len);
    if(ret == KERN_SUCCESS) {
        if(__displayContexts != NULL) {
            int count = CFDictionaryGetCount(__displayContexts);
            NSNumber *keys[count];
            CGContextRef vals[count];
            CFDictionaryGetKeysAndValues(__displayContexts, &keys, &vals);
            for(int i = 0; i < count; ++i) {
                CGContextRef ctx = vals[i];
                void *buffer = [[ctx surface] pixelBytes];
                buffer -= 6*sizeof(int);
                shmdt(buffer);
                CGContextRelease(ctx);
            }
            CFDictionaryRemoveAllValues(__displayContexts);
        }
        return data.val1;
    }
    return kCGErrorFailure;
}

CGWindowID CGShieldingWindowID(CGDirectDisplayID display) {
    // Not implemented
    return 0;
}

CGWindowLevel CGShieldingWindowLevel(void) {
    // Not implemented
    return 0;
}

CGContextRef CGDisplayGetDrawingContext(CGDirectDisplayID display) {
    struct wsRPCSimple data = { kCGDisplayGetDrawingContext, 4 };
    data.val1 = display;
    int len = sizeof(data);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &data, &len);
    if(ret == KERN_SUCCESS) {
        if(data.val1 == 0)
            return NULL; // display is not captured
        void *p = shmat(data.val1, NULL, 0);
        if((void *)-1 == p)
            return NULL;
        intptr_t *q = (intptr_t *)p;
        int width = q[0];
        int height = q[1];
        int format = q[2];

        CGContextRef ctx = (CGContextRef)[[[O2Context_builtin alloc]
            initWithBytes:(p+sizeof(int)*6) width:width height:height
            bitsPerComponent:8 bytesPerRow:width*4 colorSpace:CGColorSpaceCreateDeviceRGB()
            bitmapInfo:format releaseCallback:NULL releaseInfo:NULL] retain];

        // cache it so we can release later. we support up to 8 captured displays
        if(__displayContexts == NULL) 
            __displayContexts = CFDictionaryCreateMutable(NULL, 8, &__CGDispCtxKeyCallback, &__CGDispCtxValCallback);
        CFDictionarySetValue(__displayContexts, [NSNumber numberWithInt:display], ctx);
        return ctx;
    }
    return NULL;
}

// Creating Images from the Display
CGImageRef CGDisplayCreateImage(CGDirectDisplayID display) {
    CGRect rect = CGDisplayBounds(display);
    return CGDisplayCreateImageForRect(display, rect);
}

CGImageRef CGDisplayCreateImageForRect(CGDirectDisplayID display, CGRect rect) {
    struct wsRPCSimple data = { {kCGDisplayCreateImageForRect, 12} };
    data.val1 = display;
    // convert floats to int
    int x = rect.origin.x;
    int y = rect.origin.y;
    int w = rect.size.width;
    int h = rect.size.height;
    // and pack them
    data.val2 = (x << 16) | y;
    data.val3 = (w << 16) | h;
    int len = sizeof(data);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &data, &len);
    if(ret == KERN_SUCCESS) {
        if(data.val1 == 0)
            return NULL;
        uint8_t *p = shmat(data.val1, NULL, 0);
        CGDataProviderRef d = CGDataProviderCreateWithData(NULL, p, h*w*4, NULL);
        CGImageRef img = CGImageCreate(w, h, 8, 32, w*4, CGColorSpaceCreateDeviceRGB(),
            kCGBitmapByteOrderDefault|kCGImageAlphaPremultipliedFirst, d, NULL, 0, kCGRenderingIntentDefault);
        CFRelease(d);
        shmctl(data.val1, IPC_RMID, 0);
        shmdt(p);
        return img;
    }
    return NULL;
}

// Configuring Displays
CGError CGBeginDisplayConfiguration(CGDisplayConfigRef *config) {
    if(config == NULL)
        return kCGErrorIllegalArgument;

    *config = calloc(sizeof(struct _CGDisplayConfig), 1);
    if(*config == NULL)
        return kCGErrorFailure;

    (*config)->inner = calloc(sizeof(struct _CGDisplayConfigInner), 1);
    if((*config)->inner == NULL) {
        free(*config);
        return kCGErrorFailure;
    }

    (*config)->inner->length = sizeof(struct _CGDisplayConfigInner);
    return kCGErrorSuccess;
}

CGError CGCancelDisplayConfiguration(CGDisplayConfigRef config) {
    if(config == NULL)
        return kCGErrorIllegalArgument;
    free(config->inner);
    free(config);
    return kCGErrorSuccess;
}

CGError CGCompleteDisplayConfiguration(CGDisplayConfigRef config, CGConfigureOption option) {
    if(config == NULL || config->inner == NULL)
        return kCGErrorIllegalArgument;

    config->inner->rpc.code = kCGCompleteDisplayConfiguration;
    config->inner->rpc.len = config->inner->length - sizeof(config->inner->rpc); // trailing data length
    config->inner->option = option;

    struct wsRPCSimple reply = {0};
    int len = sizeof(reply);

    kern_return_t ret = _windowServerRPC(config->inner, config->inner->length, &reply, &len);
    free(config->inner);
    free(config);

    if(ret == KERN_SUCCESS)
        return reply.val1;
    return kCGErrorFailure;
}

CGError CGConfigureDisplayMirrorOfDisplay(CGDisplayConfigRef config, CGDirectDisplayID display, CGDirectDisplayID primary) {
    if(config == NULL || config->inner == NULL || display == kCGNullDirectDisplay)
        return kCGErrorIllegalArgument;

    void *_inner = realloc(config->inner, config->inner->length + sizeof(struct _CGDispCfgMirror));
    if(_inner == NULL)
        return kCGErrorFailure;
    config->inner = _inner;
    struct _CGDispCfgMirror *op = ((char *)config->inner + config->inner->length); 
    op->opcode = CGDISPCFG_MIRROR;
    op->display = display;
    op->primary = primary;
    config->inner->length += sizeof(struct _CGDispCfgMirror);
    return kCGErrorSuccess;
}

CGError CGConfigureDisplayOrigin(CGDisplayConfigRef config, CGDirectDisplayID display, int32_t x, int32_t y) {
    if(config == NULL || config->inner == NULL || display == kCGNullDirectDisplay)
        return kCGErrorIllegalArgument;

    void *_inner = realloc(config->inner, config->inner->length + sizeof(struct _CGDispCfgOrigin));
    if(_inner == NULL)
        return kCGErrorFailure;
    config->inner = _inner;
    struct _CGDispCfgOrigin *op = ((char *)config->inner + config->inner->length);
    op->opcode = CGDISPCFG_ORIGIN;
    op->display = display;
    op->x = x;
    op->y = y;
    config->inner->length += sizeof(struct _CGDispCfgOrigin);
    return kCGErrorSuccess;
}

void CGRestorePermanentDisplayConfiguration(void) {
    struct wsRPCBase data = { kCGRestorePermanentDisplayConfiguration, 0 };
    _windowServerRPC(&data, sizeof(data), NULL, NULL);
}

CGError CGConfigureDisplayStereoOperation(CGDisplayConfigRef config, CGDirectDisplayID display, boolean_t stereo, boolean_t forceBlueLine) {
    return kCGErrorFailure; // not implemented
}

CGError CGDisplaySetStereoOperation(CGDirectDisplayID display, boolean_t stereo, boolean_t forceBlueLine, CGConfigureOption option) {
    return kCGErrorFailure; // not implemented
}

CGError CGConfigureDisplayWithDisplayMode(CGDisplayConfigRef config, CGDirectDisplayID display, CGDisplayModeRef mode, CFDictionaryRef options) {
    if(config == NULL || config->inner == NULL || display == kCGNullDirectDisplay)
        return kCGErrorIllegalArgument;

    struct _CGDispCfgMode op = {
        .opcode = CGDISPCFG_MODE,
        .display = display,
    };
    memcpy(&op.mode, mode, sizeof(struct CGDisplayMode));

    void *_inner = realloc(config->inner, config->inner->length + sizeof(op));
    if(_inner == NULL)
        return kCGErrorFailure;
    config->inner = _inner;
    memcpy((char *)config->inner + config->inner->length, &op, sizeof(op));
    config->inner->length += sizeof(op);
    return kCGErrorSuccess;
}

// Getting the Display Configuration
CGColorSpaceRef CGDislayCopyColorSpace(CGDirectDisplayID display) {
    // FIXME: implement this properly, then use it in drawing ctx/image for rect
    return CGColorSpaceRetain(CGColorSpaceCreateDeviceRGB());
}

boolean_t CGDisplayIsActive(CGDirectDisplayID display) {
    return (CGDisplayStateFlags(display) & kWSDisplayActive);
}

boolean_t CGDisplayIsAlwaysInMirrorSet(CGDirectDisplayID display) {
    return FALSE;
}

boolean_t CGDisplayIsAsleep(CGDirectDisplayID display) {
    return (CGDisplayStateFlags(display) & kWSDisplaySleeping);
}

boolean_t CGDisplayIsBuiltin(CGDirectDisplayID display) {
    return (CGDisplayStateFlags(display) & kWSDisplayBuiltin);
}

boolean_t CGDisplayIsInHWMirrorSet(CGDirectDisplayID display) {
    return (CGDisplayStateFlags(display) & kWSDisplayHWMirror);
}

boolean_t CGDisplayIsInMirrorSet(CGDirectDisplayID display) {
    return (CGDisplayStateFlags(display) & kWSDisplayMirrored);
}

boolean_t CGDisplayIsMain(CGDirectDisplayID display) {
    return (CGDisplayStateFlags(display) & kWSDisplayMain);
}

boolean_t CGDisplayIsOnline(CGDirectDisplayID display) {
    return (CGDisplayStateFlags(display) & kWSDisplayOnline);
}

boolean_t CGDisplayIsStereo(CGDirectDisplayID display) {
    return FALSE; // not implemented
}

CGDirectDisplayID CGDisplayMirrorsDisplay(CGDirectDisplayID display) {
    struct wsRPCSimple data = { kCGDisplayMirrorsDisplay, 4 };
    data.val1 = display;
    int len = sizeof(data);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &data, &len);
    if(ret == KERN_SUCCESS)
        return (data.val1 != 0) ? data.val1 : kCGNullDirectDisplay;
    return kCGNullDirectDisplay;
}

uint32_t CGDisplayModelNumber(CGDirectDisplayID display) {
    struct wsRPCSimple data = { kCGDisplayModelNumber, 4 };
    data.val1 = display;
    int len = sizeof(data);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &data, &len);
    if(ret == KERN_SUCCESS)
        return data.val1;
    return 0xFFFFFFFF;
}

uint32_t CGDisplayPrimaryDisplay(CGDirectDisplayID display) {
    struct wsRPCSimple data = { kCGDisplayPrimaryDisplay, 4 };
    data.val1 = display;
    int len = sizeof(data);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &data, &len);
    if(ret == KERN_SUCCESS)
        return data.val1;
    return display;
}

double CGDisplayRotation(CGDirectDisplayID display) {
    struct wsRPCSimple data = { kCGDisplayRotation, 4 };
    data.val1 = display;
    int len = sizeof(data);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &data, &len);
    if(ret == KERN_SUCCESS)
        return (double)(data.val1);
    return 0;
}

CGSize CGDisplayScreenSize(CGDirectDisplayID display) {
    struct wsRPCSimple data = { kCGDisplayScreenSize, 4 };
    data.val1 = display;
    int len = sizeof(data);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &data, &len);
    if(ret == KERN_SUCCESS)
        return NSMakeSize(data.val1, data.val2);
    return NSZeroSize;
}

uint32_t CGDisplaySerialNumber(CGDirectDisplayID display) {
    struct wsRPCSimple data = { kCGDisplaySerialNumber, 4 };
    data.val1 = display;
    int len = sizeof(data);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &data, &len);
    if(ret == KERN_SUCCESS)
        return data.val1;
    return 0xFFFFFFFF;
}

uint32_t CGDisplayUnitNumber(CGDirectDisplayID display) {
    struct wsRPCSimple data = { kCGDisplayUnitNumber, 4 };
    data.val1 = display;
    int len = sizeof(data);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &data, &len);
    if(ret == KERN_SUCCESS)
        return data.val1;
    return 0xFFFFFFFF;
}

boolean_t CGDisplayUsesOpenGLAcceleration(CGDirectDisplayID display) {
    return FALSE; // FIXME: implement with Quartz Extreme
}

uint32_t CGDisplayVendorNumber(CGDirectDisplayID display) {
    struct wsRPCSimple data = { kCGDisplayVendorNumber, 4 };
    data.val1 = display;
    int len = sizeof(data);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &data, &len);
    if(ret == KERN_SUCCESS)
        return data.val1;
    return 0xFFFFFFFF;
}

// Registering for Notification of Config Changes
CGError CGDisplayRegisterReconfigurationCallback(CGDisplayReconfigurationCallBack callback, void *userInfo) {
    return kCGErrorFailure; // FIXME: not yet implemented
}

CGError CGDisplayRemoveReconfigurationCallback(CGDisplayReconfigurationCallBack callback, void *userInfo) {
    return kCGErrorFailure; // FIXME: not yet implemented
}

// Retrieving Display Parameters
// FIXME: these are returning info on the current mode, not the display itself
CGRect CGDisplayBounds(CGDirectDisplayID display) {
    struct wsRPCSimple data = { {kCGDisplayBounds, 4} };
    data.val1 = display;
    int len = sizeof(data);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &data, &len);
    if(ret == KERN_SUCCESS) 
        return NSMakeRect(data.val1, data.val2, data.val3, data.val4);
    return NSZeroRect;
}

size_t CGDisplayPixelsHigh(CGDirectDisplayID display) {
    CGRect rect = CGDisplayBounds(display);
    return rect.size.height;
}

size_t CGDisplayPixelsWide(CGDirectDisplayID display) {
    CGRect rect = CGDisplayBounds(display);
    return rect.size.width;
}

// Creating and Managing Display Modes
CGDisplayModeRef CGDisplayCopyDisplayMode(CGDirectDisplayID display) {
    struct wsRPCSimple data = { {kCGDisplayCopyDisplayMode, 4} };
    data.val1 = display;
    struct {
        struct wsRPCBase base;
        struct CGDisplayMode mode;
    } reply;
    int len = sizeof(reply);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &reply, &len);
    if(ret == KERN_SUCCESS) {
        if(reply.base.len > 0) {
            struct CGDisplayMode *ret = calloc(sizeof(struct CGDisplayMode), 1);
            if(ret) {
                memcpy(ret, &reply.mode, sizeof(struct CGDisplayMode));
                // we don't retain ret because it already has a refcount from WS
            }
            return ret;
        }
    }
    return NULL;
}

CFArrayRef CGDisplayCopyAllDisplayModes(CGDirectDisplayID display, CFDictionaryRef options) {
    struct wsRPCSimple data = { {kCGDisplayCopyAllDisplayModes, 4} };
    data.val1 = display;
    struct {
        struct wsRPCBase base;
        struct CGDisplayMode mode[32];
    } reply;
    int len = sizeof(reply);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &reply, &len);
    if(ret == KERN_SUCCESS) {
        if(reply.base.len > 0) {
            CFMutableArrayRef entries = CFArrayCreateMutable(NULL, 
                    reply.base.len / sizeof(struct CGDisplayMode), NULL);
            for(int i = 0; i < reply.base.len / sizeof(struct CGDisplayMode); ++i) {
                struct CGDisplayMode *entry = calloc(sizeof(struct CGDisplayMode), 1);
                if(entry) {
                    memcpy(entry, &reply.mode[i], sizeof(struct CGDisplayMode));
                    // we don't retain entry because it already has a refcount from WS
                    CFArrayAppendValue(entries, entry);
                }
            }
            CFRetain(entries);
            return entries;
        }
    }
    return NULL;

}

CGError CGDisplaySetDisplayMode(CGDirectDisplayID display, CGDisplayModeRef mode, CFDictionaryRef options) {
    if(mode == NULL)
        return kCGErrorIllegalArgument;

    struct {
        struct wsRPCBase base;
        uint32_t display;
        struct CGDisplayMode mode;
    } data = {
        .base = { kCGDisplaySetDisplayMode, 4 + sizeof(struct CGDisplayMode) }
    };

    data.display = display;
    memcpy(&data.mode, mode, sizeof(data.mode));

    struct wsRPCSimple reply = {0};
    int len = sizeof(reply);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &reply, &len);

    if(ret == KERN_SUCCESS) {
        return reply.val1;
    }
    return kCGErrorFailure;
}

CGDisplayModeRef CGDisplayModeRetain(CGDisplayModeRef mode) {
    if(mode != NULL) {
        int one = 1;
        __sync_fetch_and_add(&(mode->refcount), one);
    }
    return mode;
}

void CGDisplayModeRelease(CGDisplayModeRef mode) {
    if(mode == NULL)
        return;

    int one = 1;
    int count = __sync_fetch_and_sub(&(mode->refcount), one);

    if(count == 1) // we just released the last ref
        free(mode);
}

// Getting Information About a Display Mode
size_t CGDisplayModeGetWidth(CGDisplayModeRef mode) {
    if(mode == NULL)
        return 0;
    return mode->width;
}

size_t CGDisplayModeGetHeight(CGDisplayModeRef mode) {
    if(mode == NULL)
        return 0;
    return mode->height;
}

double CGDisplayModeGetRefreshRate(CGDisplayModeRef mode) {
    if(mode == NULL)
        return 0;
    return mode->refresh;
}

uint32_t CGDisplayModeGetIOFlags(CGDisplayModeRef mode) {
    if(mode == NULL)
        return 0;
    return mode->flags;
}

int32_t CGDisplayModeGetIODisplayModeID(CGDisplayModeRef mode) {
    return 0;
}

bool CGDisplayModeIsUsableForDesktopGUI(CGDisplayModeRef mode) {
    if(mode == NULL)
        return false;
    if(mode->width >= 1024 && mode->height >= 768 && mode->depth >= 24)
        return true;
    return false;
}

CFTypeID CGDisplayModeGetTypeID(void) {
    return 0; // FIXME
}

// Adjusting the Display Gamma
CGError CGSetDisplayTransferByFormula(CGDirectDisplayID display,
        CGGammaValue redMin, CGGammaValue redMax, CGGammaValue redGamma,
        CGGammaValue greenMin, CGGammaValue greenMax, CGGammaValue greenGamma,
        CGGammaValue blueMin, CGGammaValue blueMax, CGGammaValue blueGamma) {
    if(redMin > redMax || redMin < 0 ||
        greenMin > greenMax || greenMin < 0 ||
        blueMin > blueMax || blueMin < 0 ||
        redMax < redMin || redMax > 1 ||
        greenMax < greenMin || greenMax > 1 ||
        blueMax < blueMin || blueMax > 1)
            return kCGErrorIllegalArgument;

    struct {
        struct wsRPCBase base;
        uint32_t display;
        CGGammaValue vals[9];
    } data;
    data.base.code = kCGSetDisplayTransferByFormula;
    data.base.len = sizeof(data) - sizeof(struct wsRPCBase);
    data.display = display;
    data.vals[0] = redMin;
    data.vals[1] = redMax;
    data.vals[2] = redGamma;
    data.vals[3] = greenMin;
    data.vals[4] = greenMax;
    data.vals[5] = greenGamma;
    data.vals[6] = blueMin;
    data.vals[7] = blueMax;
    data.vals[8] = blueGamma;

    struct wsRPCSimple reply = {0};
    int len = sizeof(reply);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &reply, &len);

    if(ret == KERN_SUCCESS) {
        return reply.val1;
    }
    return kCGErrorFailure;
}

CGError CGGetDisplayTransferByFormula(CGDirectDisplayID display,
        CGGammaValue *redMin, CGGammaValue *redMax, CGGammaValue *redGamma,
        CGGammaValue *greenMin, CGGammaValue *greenMax, CGGammaValue *greenGamma,
        CGGammaValue *blueMin, CGGammaValue *blueMax, CGGammaValue *blueGamma) {
    if(redMin == NULL || redMax == NULL || redGamma == NULL ||
        greenMin == NULL || greenMax == NULL || greenGamma == NULL ||
        blueMin == NULL || blueMax == NULL || blueGamma == NULL)
            return kCGErrorIllegalArgument;

    struct {
        struct wsRPCBase base;
        uint32_t display;
        CGGammaValue vals[9];
    } data;
    data.base.code = kCGGetDisplayTransferByFormula;
    data.base.len = sizeof(data) - sizeof(struct wsRPCBase);
    data.display = display;

    int len = sizeof(data);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &data, &len);

    if(ret == KERN_SUCCESS) {
        *redMin = data.vals[0];
        *redMax = data.vals[1];
        *redGamma = data.vals[2];
        *greenMin = data.vals[3];
        *greenMax = data.vals[4];
        *greenGamma = data.vals[5];
        *blueMin = data.vals[6];
        *blueMax = data.vals[7];
        *blueGamma = data.vals[8];
        return data.display; // actually our return value
    }
    return kCGErrorFailure;
}

CGError CGSetDisplayTransferByTable(CGDirectDisplayID display, uint32_t tableSize,
        const CGGammaValue *redTable, const CGGammaValue *greenTable, const CGGammaValue *blueTable) {
    if(redTable == NULL || greenTable == NULL || blueTable == NULL || tableSize < 0)
        return kCGErrorIllegalArgument;
    if(tableSize == 0)
        return kCGErrorSuccess; // nothing to do

    size_t dataSize = 3 * tableSize * sizeof(CGGammaValue) + sizeof(struct wsRPCSimple);
    if(dataSize > 128*1024)
        return kCGErrorFailure;

    char *data = calloc(dataSize, 1);
    if(data == NULL)
        return kCGErrorFailure;

    struct wsRPCSimple *base = data;
    base->base.code = kCGSetDisplayTransferByTable;
    base->base.len = dataSize - sizeof(struct wsRPCBase) + 4;
    base->val1 = display;

    CGGammaValue *p = (CGGammaValue *)(data + sizeof(struct wsRPCBase) + 4);

    for(int i = 0; i < tableSize; ++i) {
        *p++ = redTable[i];
        *p++ = greenTable[i];
        *p++ = blueTable[i];
    }

    int len = dataSize;
    kern_return_t ret = _windowServerRPC(&data, dataSize, &data, &len);
    free(data);

    if(ret == KERN_SUCCESS) {
        return base->val1;
    }
    return kCGErrorFailure;
}


CGError CGGetDisplayTransferByTable(CGDirectDisplayID display, uint32_t capacity,
        CGGammaValue *redTable, CGGammaValue *greenTable, CGGammaValue *blueTable, uint32_t *sampleCount) {
    if(redTable == NULL || greenTable == NULL || blueTable == NULL || capacity <= 0)
        return kCGErrorIllegalArgument;

    struct wsRPCSimple data = { {kCGGetDisplayTransferByTable, 4}, 0, 0, 0, 0 };
    data.val1 = display;
    int len = sizeof(data);

    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &data, &len);
    if(ret != KERN_SUCCESS)
        return kCGErrorFailure;

    int samples = 0;
    struct { CGGammaValue r; CGGammaValue g; CGGammaValue b; } *vals = &(data.val2);
    for(int i = 0; i < capacity; ++i) {
        if(i * 3 * sizeof(CGGammaValue) > data.base.len)
            break;
        *(redTable + i) = vals->r;
        *(greenTable + i) = vals->g;
        *(blueTable + i) = vals->b;
        vals += sizeof(CGGammaValue) * 3;
        samples++;
    }
    *sampleCount = samples;
    return kCGErrorSuccess;
}

CGError CGSetDisplayTransferByByteTable(CGDirectDisplayID display, uint32_t tableSize,
        const uint8_t *redTable, const uint8_t *greenTable, const uint8_t *blueTable) {
    if(redTable == NULL || greenTable == NULL || blueTable == NULL || tableSize < 0)
        return kCGErrorIllegalArgument;
    if(tableSize == 0)
        return kCGErrorSuccess; // nothing to do

    size_t dataSize = 3 * tableSize + sizeof(struct wsRPCSimple);
    if(dataSize > 128*1024)
        return kCGErrorFailure;

    char *data = calloc(dataSize, 1);
    if(data == NULL)
        return kCGErrorFailure;

    struct wsRPCSimple *base = data;
    base->base.code = kCGSetDisplayTransferByByteTable;
    base->base.len = dataSize - sizeof(struct wsRPCBase) + 4;
    base->val1 = display;

    uint8_t *p = data + sizeof(struct wsRPCBase) + 4;

    for(int i = 0; i < tableSize; ++i) {
        *p++ = redTable[i];
        *p++ = greenTable[i];
        *p++ = blueTable[i];
    }

    int len = dataSize;
    kern_return_t ret = _windowServerRPC(&data, dataSize, &data, &len);
    free(data);

    if(ret == KERN_SUCCESS) {
        return base->val1;
    }
    return kCGErrorFailure;
}

void CGDisplayRestoreColorSyncSettings(void) {
    struct wsRPCBase data = {kCGDisplayRestoreColorSyncSettings, 0};
    _windowServerRPC(&data, sizeof(data), NULL, NULL);
}

uint32_t CGDisplayGammaTableCapacity(CGDirectDisplayID display) {
    struct wsRPCSimple data = { {kCGDisplayGammaTableCapacity, 4}, 0, 0, 0, 0};
    data.val1 = display;
    int len = sizeof(data);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &data, &len);

    if(ret == KERN_SUCCESS) {
        return data.val1;
    }
    return 0;
}

// Display Fade Effects -- all stubs for now!
CGError CGConfigureDisplayFadeEffect(CGDisplayConfigRef config, CGDisplayFadeInterval fadeOutSeconds, CGDisplayFadeInterval fadeInSeconds, float fadeRed, float fadeGreen, float fadeBlue) {
    return kCGErrorFailure;
}

CGError CGAcquireDisplayFadeReservation(CGDisplayReservationInterval seconds, CGDisplayFadeReservationToken *token) {
    return kCGErrorFailure;
}

CGError CGDisplayFade(CGDisplayFadeReservationToken token, CGDisplayFadeInterval duration, CGDisplayBlendFraction startBlend, CGDisplayBlendFraction endBlend, float redBlend, float greenBlend, float blueBlend, boolean_t synchronous) {
    return kCGErrorFailure;
}

CGError CGReleaseDisplayFadeReservation(CGDisplayFadeReservationToken token) {
    return kCGErrorFailure;
}

// Controlling the Mouse Cursor
CGError CGDisplayHideCursor(CGDirectDisplayID display) {
    struct wsRPCBase data = {kCGDisplayHideCursor, 0};
    _windowServerRPC(&data, sizeof(data), NULL, NULL);
}

CGError CGDisplayShowCursor(CGDirectDisplayID display) {
    struct wsRPCBase data = {kCGDisplayShowCursor, 0};
    _windowServerRPC(&data, sizeof(data), NULL, NULL);
}

CGError CGDisplayMoveCursorToPoint(CGDirectDisplayID display, CGPoint point) {
    struct wsRPCSimple data = { {kCGDisplayMoveCursorToPoint, 12}, 0, 0, 0, 0};
    data.val1 = display;
    data.val2 = (int32_t)point.x;
    data.val3 = (int32_t)point.y;
    int len = sizeof(data);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &data, &len);

    if(ret == KERN_SUCCESS) {
        return data.val1;
    }
    return 0;
}

CGError CGAssociateMouseAndMouseCursorPosition(boolean_t connected) {
    struct wsRPCSimple data = { {kCGAssociateMouseAndMouseCursorPosition, 4}, 0, 0, 0, 0};
    data.val1 = connected;
    int len = sizeof(data);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &data, &len);

    if(ret == KERN_SUCCESS) {
        return data.val1;
    }
    return 0;
}

CGError CGWarpMouseCursorPosition(CGPoint newCursorPosition) {
    struct wsRPCSimple data = { {kCGWarpMouseCursorPosition, 8}, 0, 0, 0, 0};
    data.val1 = newCursorPosition.x;
    data.val2 = newCursorPosition.y;
    int len = sizeof(data);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &data, &len);

    if(ret == KERN_SUCCESS) {
        return data.val1;
    }
    return 0;
}

void CGGetLastMouseDelta(int32_t *deltaX, int32_t *deltaY) {
    struct wsRPCSimple data = { {kCGGetLastMouseDelta, 0}, 0, 0, 0, 0};
    int len = sizeof(data);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &data, &len);

    if(ret == KERN_SUCCESS) {
        *deltaX = data.val1;
        *deltaY = data.val2;
    }
}


// WindowServer info
CFDictionaryRef CGSessionCopyCurrentDictionary(void) {

}

// This function resolves the bootstrap service and returns a port with send rights
// for calling WindowServer. The port must not be deallocated or modified!
CFMachPortRef CGWindowServerCFMachPort(void) {
    static CFMachPortRef wsPort = NULL;
    if(wsPort == NULL) {
        mach_port_t p;
        if(bootstrap_look_up(bootstrap_port, WINDOWSERVER_SVC_NAME, &p) == KERN_SUCCESS) {
            Boolean shouldFree;
            CFMachPortContext context = {0};
            wsPort = CFMachPortCreateWithPort(NULL, p, NULL, &context, &shouldFree);
        }
    }
    return wsPort;
}

// The values below were obtained by testing macOS Sonoma
CGWindowLevel CGWindowLevelForKey(CGWindowLevelKey key) {
    if(key < 0 || key > kCGNumberOfWindowLevelKeys)
        return -INT_MAX;
    const int32_t levels[kCGNumberOfWindowLevelKeys] = {
        -INT_MAX, -INT_MAX, -INT_MAX, -20, 0,
        3, 3, 20, 24, 25, 8, 101, 500, 1000,
        INT_MAX, 102, 200, 19, -INT_MAX, INT_MAX, 1500
    };
    return levels[key];
}

// Streaming the Display Contents -- all stubs for now! (deprecated after 14.4)

CGDisplayStreamRef CGDisplayStreamCreate(CGDirectDisplayID display, size_t outputWidth, size_t outputHeight, int32_t pixelFormat, CFDictionaryRef properties, CGDisplayStreamFrameAvailableHandler handler) {
    return NULL;
}

CGDisplayStreamRef CGDisplayStreamCreateWithDispatchQueue(CGDirectDisplayID display, size_t outputWidth, size_t outputHeight, int32_t pixelFormat, CFDictionaryRef properties, dispatch_queue_t queue, CGDisplayStreamFrameAvailableHandler handler) {
    return NULL;
}

CGError CGDisplayStreamStart(CGDisplayStreamRef displayStream) {
    return kCGErrorFailure;
}

CGError CGDisplayStreamStop(CGDisplayStreamRef displayStream) {
    return kCGErrorSuccess;
}

CFRunLoopSourceRef CGDisplayStreamGetRunLoopSource(CGDisplayStreamRef displayStream) {
    return NULL;
}

const CGRect * CGDisplayStreamUpdateGetRects(CGDisplayStreamUpdateRef updateRef, CGDisplayStreamUpdateRectType rectType, size_t *rectCount) {
    return NULL;
}

CGDisplayStreamUpdateRef CGDisplayStreamUpdateCreateMergedUpdate(CGDisplayStreamUpdateRef firstUpdate, CGDisplayStreamUpdateRef secondUpdate) {
    return NULL;
}

void CGDisplayStreamUpdateGetMovedRectsDelta(CGDisplayStreamUpdateRef updateRef, CGFloat *dx, CGFloat *dy) {
}

size_t CGDisplayStreamUpdateGetDropCount(CGDisplayStreamUpdateRef updateRef) {
    return 0;
}

CFTypeID CGDisplayStreamGetTypeID(void) {
    return 0;
}

CFTypeID CGDisplayStreamUpdateGetTypeID(void) {
    return 0;
}

// Private functions

static uint32_t CGDisplayStateFlags(CGDirectDisplayID display) {
    struct wsRPCSimple data = { {kCGDisplayStateFlags, 4} };
    data.val1 = display;
    int len = sizeof(data);
    kern_return_t ret = _windowServerRPC(&data, sizeof(data), &data, &len);
    if(ret == KERN_SUCCESS) 
        return data.val1;
    return 0;
}


/*
 * Make a synchronous RPC call to WS.
 *
 * On entry, data must contain necessary function params and len must contain the length of
 * the data block. These do not need to persist beyond the call. If a reply is needed,
 * replyBuf must point to a suitable block of memory and replyLen must contain the size of
 * the buffer.
 *
 * A send request times out after 2500ms. A receive blocks until WS responds. This is to maintain
 * atomicity of requests.
 *
 * Returns KERN_SUCCESS on success or a Mach error code on failure. If a reply is requested and
 * the buffer is too small for the received data, replyLen is set to -1 and replyBuf is unchanged.
 * Otherwise, the reply data is copied into replyBuf and the replyLen is set to the length of the
 * data block.
 */
kern_return_t _windowServerRPC(void *data, size_t len, void *replyBuf, int *replyLen) {
    static mach_port_t replyPort = MACH_PORT_NULL;
    static mach_port_t wsPort = MACH_PORT_NULL;

    // Get WindowServer's service port
    if(wsPort == MACH_PORT_NULL) {
        CFMachPortRef wsCF = CGWindowServerCFMachPort();
        if(wsCF)
            wsPort = CFMachPortGetPort(wsCF);
        if(wsPort == MACH_PORT_NULL)
            return KERN_FAILURE;
    }

    // Create a port with send/receive rights for us to use
    if(replyPort == MACH_PORT_NULL) {
        kern_return_t ret = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &replyPort);
        if(ret == KERN_SUCCESS)
            ret = mach_port_insert_right(mach_task_self(), replyPort, replyPort, MACH_MSG_TYPE_MAKE_SEND);
        if(ret != KERN_SUCCESS)
            return ret;
    }

    // OK, we seem to have ports set up... do the RPC. This is a blocking call until WS responds.
    PortMessage msg = {0};
    msg.header.msgh_remote_port = wsPort;
    msg.header.msgh_bits = MACH_MSGH_BITS_SET(MACH_MSG_TYPE_COPY_SEND, 0, 0, MACH_MSGH_BITS_COMPLEX);
    msg.header.msgh_id = MSG_ID_RPC;
    msg.header.msgh_size = sizeof(msg);
    msg.msgh_descriptor_count = (replyBuf == NULL) ? 0 : 1;
    if(msg.msgh_descriptor_count) {
        msg.descriptor.type = MACH_MSG_PORT_DESCRIPTOR;
        msg.descriptor.name = replyPort;
        msg.descriptor.disposition = MACH_MSG_TYPE_MAKE_SEND;
    }
    msg.pid = getpid();
    CFStringRef bundleID = (__bridge CFStringRef)[[NSBundle mainBundle] bundleIdentifier];
    if(bundleID)
        strncpy(msg.bundleID, [(__bridge NSString *)bundleID cString], sizeof(msg.bundleID));

    if(len > sizeof(msg.data))
        return MACH_SEND_TOO_LARGE;
    memmove(msg.data, data, len);
    msg.len = len;

    int flags = 0;
    if(replyBuf != NULL && replyLen != NULL)
        flags = MACH_RCV_MSG;

    kern_return_t ret = KERN_SUCCESS;
    ret = mach_msg((mach_msg_header_t *)&msg, MACH_SEND_MSG|MACH_SEND_TIMEOUT|flags, sizeof(msg),
            sizeof(msg), replyPort, 2000, MACH_PORT_NULL);
    if(ret == KERN_SUCCESS && replyBuf != NULL) {
        Message *rmsg = (Message *)&msg;
        if(*replyLen >= rmsg->len) {
            *replyLen = rmsg->len;
            memmove(replyBuf, rmsg->data, *replyLen);
        } else {
            *replyLen = -1;
        }
    }
    return ret;
}

