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
#import <CoreGraphics/CGDirectDisplay.h>
#import <CoreGraphics/CGError.h>
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

CGError CGGetOnlineDisplayList(uint32_t maxDisplays, CGDirectDisplayID *onlineDisplays, uint32_t *displayCount) {
    struct wsRPCBase data = { kCGGetOnlineDisplayList, 0 };
    
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
    printf("results bytes: %d count: %d\n", data.len, count);
    if(maxDisplays < count)
        count = maxDisplays;
    printf("returning %d displays\n", count);
    for(int i = 0; i < count; i++) {
        printf("ID: %x\n", list[i]);
        onlineDisplays[i] = list[i];
    }
    *displayCount = count;
    free(replyBuf);
    return kCGErrorSuccess;
}

CGError CGGetActiveDisplayList(uint32_t maxDisplays, CGDirectDisplayID *activeDisplays, uint32_t *displayCount) {

}

CGError CGGetDisplaysWithOpenGLDisplayMask(CGOpenGLDisplayMask mask, uint32_t maxDisplays, CGDirectDisplayID *displays, uint32_t *matchingDisplayCount) {

}

CGError CGGetDisplaysWithPoint(CGPoint point, uint32_t maxDisplays, CGDirectDisplayID *displays, uint32_t *matchingDisplayCount) {

}

CGError CGGetDisplaysWithRect(CGRect rect, uint32_t maxDisplays, CGDirectDisplayID *displays, uint32_t *matchingDisplayCount) {

}

CGDirectDisplayID CGOpenGLDisplayMaskToDisplayID(CGOpenGLDisplayMask mask) {

}

CGOpenGLDisplayMask CGDisplayIDToOpenGLDisplayMask(CGDirectDisplayID display) {

}


CGError CGReleaseAllDisplays(void) {
   return 0;
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

