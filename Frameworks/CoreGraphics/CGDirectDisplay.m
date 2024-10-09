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

// dictionary of display contexts
static CFMutableDictionaryRef __displayContexts = NULL;
static const CFDictionaryKeyCallBacks __CGDispCtxKeyCallback = {0}; // all NULL - use defaults
static const CFDictionaryValueCallBacks __CGDispCtxValCallback = {0}; // same

// make a RPC to WindowServer with optional reply
kern_return_t _windowServerRPC(void *data, size_t len, void *replyBuf, int *replyLen);

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

        CGContextRef ctx = (__bridge_retained CGContextRef)[[O2Context_builtin alloc]
            initWithBytes:(p+sizeof(int)*6) width:width height:height
            bitsPerComponent:8 bytesPerRow:width*4 colorSpace:CGColorSpaceCreateDeviceRGB()
            bitmapInfo:format releaseCallback:NULL releaseInfo:NULL];

        // cache it so we can release later. we support up to 8 captured displays
        if(__displayContexts == NULL) 
            __displayContexts = CFDictionaryCreateMutable(NULL, 8, &__CGDispCtxKeyCallback, &__CGDispCtxValCallback);
        CFDictionarySetValue(__displayContexts, [NSNumber numberWithInt:display], ctx);
        return ctx;
    }
    return NULL;
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

CGWindowLevel CGWindowLevelForKey(CGWindowLevelKey key) {

}

// Private functions

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

