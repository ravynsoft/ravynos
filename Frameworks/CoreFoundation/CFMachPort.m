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
#import <Foundation/NSCFTypeID.h>

struct __NSMachPort {
    mach_port_t port;
    CFMachPortContext ctx;
};


COREFOUNDATION_EXPORT CFTypeID CFMachPortGetTypeID(void) {
    return kNSCFTypeMachPort;
}

COREFOUNDATION_EXPORT CFMachPortRef CFMachPortCreate(CFAllocatorRef allocator,
        CFMachPortCallBack callback, CFMachPortContext *context, Boolean *callerFreeInfo) {
    if(context == NULL)
        goto err_out;

    // FIXME: respect allocator selection
    struct __NSMachPort *self = malloc(sizeof(struct __NSMachPort));
    if(self == NULL)
        goto err_out;

    self->ctx.version = context->version;
    self->ctx.info = context->info;
    self->ctx.retain = context->retain;
    self->ctx.release = context->release;
    self->ctx.copyDescription = context->copyDescription;

    mach_port_t task = mach_task_self();
    if(mach_port_allocate(task, MACH_PORT_RIGHT_RECEIVE, &self->port) != KERN_SUCCESS) 
        goto err_out;
    if(mach_port_insert_right(task, self->port, self->port, 
                MACH_MSG_TYPE_MAKE_SEND) != KERN_SUCCESS) {
        mach_port_deallocate(self->port);
        goto err_out;
    }
    return self;

err_out:
    if(self != NULL)
        free(self);
    if(callerFreeInfo != NULL)
        *callerFreeInfo = true;
    return NULL;
}

#if 0
COREFOUNDATION_EXPORT CFMachPortRef CFMachPortCreateWithPort(CFAllocatorRef allocator, mach_port_t port, CFMachPortCallBack callback, CFMachPortContext *context, Boolean *callerFreeInfo);

COREFOUNDATION_EXPORT mach_port_t CFMachPortGetPort(CFMachPortRef self);
COREFOUNDATION_EXPORT void CFMachPortGetContext(CFMachPortRef self, CFMachPortContext *context);
COREFOUNDATION_EXPORT CFMachPortInvalidationCallBack CFMachPortGetInvalidationCallBack(CFMachPortRef self);
COREFOUNDATION_EXPORT void CFMachPortSetInvalidationCallBack(CFMachPortRef self, CFMachPortInvalidationCallBack callback);

COREFOUNDATION_EXPORT CFRunLoopSourceRef CFMachPortCreateRunLoopSource(CFAllocatorRef allocator, CFMachPortRef self, CFIndex order);
COREFOUNDATION_EXPORT void CFMachPortInvalidate(CFMachPortRef self);
COREFOUNDATION_EXPORT Boolean CFMachPortIsValid(CFMachPortRef self);
#endif
