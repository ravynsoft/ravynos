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


#import <CoreFoundation/CFMachPort.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSCFTypeID.h>

struct __NSMachPort {
    mach_port_t port;
    CFMachPortContext ctx;
    CFMachPortCallBack callback;
    CFMachPortInvalidationCallBack invalidated;
    Boolean isValid;
};

COREFOUNDATION_EXPORT CFTypeID CFMachPortGetTypeID(void) {
    return kNSCFTypeMachPort;
}

COREFOUNDATION_EXPORT CFMachPortRef CFMachPortCreate(CFAllocatorRef allocator,
        CFMachPortCallBack callback, CFMachPortContext *context, Boolean *callerFreeInfo) {
    if(context == NULL)
        return NULL;

    mach_port_t task = mach_task_self();
    mach_port_t port = 0;
    if(mach_port_allocate(task, MACH_PORT_RIGHT_RECEIVE, &port) != KERN_SUCCESS) 
        return NULL;
    if(mach_port_insert_right(task, port, port, MACH_MSG_TYPE_MAKE_SEND) != KERN_SUCCESS) {
        mach_port_deallocate(task, port);
        return NULL;
    }

    CFMachPortRef self = CFMachPortCreateWithPort(allocator, port, callback, context, callerFreeInfo);
    if(self == NULL)
        mach_port_deallocate(task, port);
    return self;
}

/* FIXME: This should maintain a table of objects by port number. When an object exists for
 * a given port, the function should return it instead of creating a new one
 */
COREFOUNDATION_EXPORT CFMachPortRef CFMachPortCreateWithPort(CFAllocatorRef allocator,
        mach_port_t port, CFMachPortCallBack callback, CFMachPortContext *context,
        Boolean *callerFreeInfo) {
    if(context == NULL)
        return NULL;

    // FIXME: respect allocator selection
    struct __NSMachPort *self = malloc(sizeof(struct __NSMachPort));
    if(self == NULL) {
        if(callerFreeInfo != NULL)
            *callerFreeInfo = TRUE;
        return NULL;
    }

    self->invalidated = NULL;
    self->isValid = TRUE;

    self->ctx.version = context->version;
    self->ctx.info = context->info;
    self->ctx.retain = context->retain;
    self->ctx.release = context->release;
    self->ctx.copyDescription = context->copyDescription;

    if(self->ctx.retain)
        (self->ctx.retain)(self->ctx.info);

    self->callback = callback;
    self->port = port;
    if(callerFreeInfo != NULL)
        *callerFreeInfo = FALSE;
    return self;
}

COREFOUNDATION_EXPORT mach_port_t CFMachPortGetPort(CFMachPortRef self) {
    return self->port;
}

COREFOUNDATION_EXPORT void CFMachPortGetContext(CFMachPortRef self, CFMachPortContext *context) {
    context->version = self->ctx.version;
    context->info = self->ctx.info;
    context->retain = self->ctx.retain;
    context->release = self->ctx.release;
    context->copyDescription = self->ctx.copyDescription;
}

COREFOUNDATION_EXPORT CFMachPortInvalidationCallBack CFMachPortGetInvalidationCallBack(CFMachPortRef self) {
    return self->invalidated;
}

COREFOUNDATION_EXPORT void CFMachPortSetInvalidationCallBack(CFMachPortRef self,
        CFMachPortInvalidationCallBack callback) {
    self->invalidated = callback;
}

COREFOUNDATION_EXPORT CFRunLoopSourceRef
    CFMachPortCreateRunLoopSource(CFAllocatorRef allocator, CFMachPortRef self, CFIndex order) {
        return NULL; // FIXME: implement
}

COREFOUNDATION_EXPORT void CFMachPortInvalidate(CFMachPortRef self) {
    // "prevent the port from ever receiving any more messages"
    if(!self->isValid)
        return;
    self->isValid = FALSE;

    mach_port_mod_refs(mach_task_self(), self->port, MACH_PORT_RIGHT_RECEIVE, 0);
    (self->invalidated)(self, self->ctx.info);
    if(self->ctx.release) {
        (self->ctx.release)(self->ctx.info);
        self->ctx.info = NULL;
    }
}

COREFOUNDATION_EXPORT Boolean CFMachPortIsValid(CFMachPortRef self) {
    return self->isValid;
}

