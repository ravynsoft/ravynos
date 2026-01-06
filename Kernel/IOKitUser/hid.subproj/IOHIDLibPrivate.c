/*
 * Copyright (c) 1999-2008 Apple Computer, Inc.  All Rights Reserved.
 * 
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include <IOKit/IOKitLib.h>
#include <IOKit/IOReturn.h>
#include <stdarg.h>
#include <asl.h>
#include <os/log.h>
#include <mach/mach_time.h>
#include <sys/kdebug.h>
#include <sys/syscall.h>

#include "IOHIDLibPrivate.h"
#include "IOHIDBase.h"
#include "IOHIDDebugTrace.h"
#include "IOHIDEvent.h"
#include <IOKit/hid/AppleHIDUsageTables.h>
#include <os/assumes.h>

void _IOObjectCFRelease(        CFAllocatorRef          allocator  __unused, 
                                const void *            value)
{
    IOObjectRelease((io_object_t)(uintptr_t) value);
}

const void * _IOObjectCFRetain( CFAllocatorRef          allocator  __unused, 
                                const void *            value)
{
    if (kIOReturnSuccess != IOObjectRetain((io_object_t)(uintptr_t)value))
        return NULL;
    
    return value;
}

void _IOHIDCallbackApplier(const void *callback, 
                           const void *callbackContext, 
                           void *applierContext)
{
    IOHIDCallbackApplierContext *context = (IOHIDCallbackApplierContext*)applierContext;
    if (callback && context)
        ((IOHIDCallback)callback)((void *)callbackContext, context->result, context->sender);
}

//------------------------------------------------------------------------------
// _IOHIDLog
//------------------------------------------------------------------------------
os_log_t _IOHIDLog(void)
{
    static os_log_t log = NULL;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        log = os_log_create("com.apple.iohid", "default");
    });
    return log;
}

//------------------------------------------------------------------------------
// _IOHIDLogCategory
//------------------------------------------------------------------------------
os_log_t _IOHIDLogCategory(IOHIDLogCategory category)
{
    assert(category < kIOHIDLogCategoryCount);
    static os_log_t log[kIOHIDLogCategoryCount];
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{

        log[kIOHIDLogCategoryDefault]           = os_log_create(kIOHIDLogSubsytem, "default");
        log[kIOHIDLogCategoryTrace]             = os_log_create(kIOHIDLogSubsytem, "trace");
        log[kIOHIDLogCategoryProperty]          = os_log_create(kIOHIDLogSubsytem, "property");
        log[kIOHIDLogCategoryActivity]          = os_log_create(kIOHIDLogSubsytem, "activity");
        log[kIOHIDLogCategoryFastPath]          = os_log_create(kIOHIDLogSubsytem, "fastpath");
        log[kIOHIDLogCategoryUserDevice]        = os_log_create(kIOHIDLogSubsytem, "userdevice");
        log[kIOHIDLogCategoryService]           = os_log_create(kIOHIDLogSubsytem, "service");
        log[kIOHIDServiceLogCategoryCarplay]    = os_log_create(kIOHIDLogSubsytem, "service.carplay");
        log[kIOHIDLogCategoryConnection]        = os_log_create(kIOHIDLogSubsytem, "connection");
        log[kIOHIDLogCategoryCursor]            = os_log_create(kIOHIDLogSubsytem, "cursor");	
        log[kIOHIDLogCategorySignpost]          = os_log_create(kIOHIDLogSubsytem, "hidsignpost");

    });
    return log[category];
}

//------------------------------------------------------------------------------
// _IOHIDUnitlCopyTimeString
//------------------------------------------------------------------------------
CFStringRef _IOHIDCreateTimeString(CFAllocatorRef allocator, struct timeval *tv)
{
    struct tm tmd;
    struct tm *local_time;
    char time_str[32] = { 0, };

    local_time = localtime_r(&tv->tv_sec, &tmd);
    if (local_time == NULL) {
        local_time = gmtime_r(&tv->tv_sec, &tmd);
    }

    if (local_time) {
        strftime(time_str, sizeof(time_str), "%F %H:%M:%S", local_time);
    }
  
  
    CFStringRef time = CFStringCreateWithFormat(allocator, NULL, CFSTR("%s.%06d"), time_str, tv->tv_usec);
    return time;
}

//------------------------------------------------------------------------------
// _IOHIDGetMonotonicTime (in ns)
//------------------------------------------------------------------------------
uint64_t  _IOHIDGetMonotonicTime (void) {
    static mach_timebase_info_data_t    timebaseInfo;

    if (timebaseInfo.denom == 0)
        mach_timebase_info(&timebaseInfo);

    return ((mach_absolute_time( ) * timebaseInfo.numer) / timebaseInfo.denom);
}

//------------------------------------------------------------------------------
// _IOHIDGetTimestampDelta
//------------------------------------------------------------------------------
uint64_t _IOHIDGetTimestampDelta(uint64_t timestampA, uint64_t timestampB, uint32_t scaleFactor)
{
    static mach_timebase_info_data_t timebaseInfo;
    uint64_t delta = 0;
    
    if (timebaseInfo.denom == 0)
        mach_timebase_info(&timebaseInfo);
    
    delta = timestampA - timestampB;
    
    delta *= timebaseInfo.numer;
    delta /= timebaseInfo.denom;
    
    return delta / scaleFactor;
}

void _IOHIDDebugTrace(uint32_t code, uint32_t func, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
    
    kdebug_trace(IOHID_DEBUG_CODE(code) | func, arg1, arg2, arg3, arg4);

    if (gIOHIDDebugConfig & kIOHIDDebugTraceWithOsLog) {
        os_log(_IOHIDLogCategory(kIOHIDLogCategoryTrace), "0x%-16llx 0x%-16llx 0x%-16llx 0x%-16llx 0x%-16llx", (unsigned long long) (IOHID_DEBUG_CODE(code) | func), (unsigned long long)arg1, (unsigned long long)arg2, (unsigned long long)arg3, (unsigned long long)arg4);
    }
}

void _IOHIDDebugEventAddPerfData(IOHIDEventRef event, int timepoint, uint64_t timestamp) {
    
    IOHIDEventRef perfEvent = NULL;
    if (!(gIOHIDDebugConfig & kIOHIDDebugPerfEvent)) {
        return ;
    }
    
    if (IOHIDEventConformsTo(event,kIOHIDEventTypeVendorDefined)) {
        CFArrayRef children = IOHIDEventGetChildren(event);
        for (CFIndex i = 0, count = (children) ? CFArrayGetCount(children) : 0;  i < count; i++) {
            IOHIDEventRef child = (IOHIDEventRef)CFArrayGetValueAtIndex(children, i);
            if (IOHIDEventGetType(child) == kIOHIDEventTypeVendorDefined  &&
                IOHIDEventGetIntegerValue(child, kIOHIDEventFieldVendorDefinedUsage) == kHIDUsage_AppleVendor_Perf &&
                IOHIDEventGetIntegerValue(child, kIOHIDEventFieldVendorDefinedUsagePage) == kHIDPage_AppleVendor) {
                perfEvent = child;
                break;
            }
        }
    }
    
    if (!perfEvent) {
        IOHIDEventPerfData data = {0, 0, 0, 0, 0};
        perfEvent = IOHIDEventCreateVendorDefinedEvent(
                                                       CFGetAllocator(event),
                                                       mach_absolute_time(),
                                                       kHIDPage_AppleVendor,
                                                       kHIDUsage_AppleVendor_Perf,
                                                       0,
                                                       (uint8_t *)&data,
                                                       sizeof(data),
                                                       0);
        
        if (perfEvent) {
            IOHIDEventAppendEvent(event, perfEvent, 0);
            CFRelease(perfEvent);
        }
    }
    
    if (!perfEvent) {
        return;
    }
    
    IOHIDEventPerfData *data = NULL;
    CFIndex     eventLength = 0;
    IOHIDEventGetVendorDefinedData(perfEvent, (uint8_t**)&data, &eventLength);
    if (data) {
        switch (timepoint) {
            case kIOHIDEventPerfDataPointEventSystemReceive:
                data->eventSystemReceiveTime = timestamp;
                break;
            case kIOHIDEventPerfDataPointEventSystemFilter:
                data->eventSystemFilterTime = timestamp;
                break;
            case kIOHIDEventPerfDataPointEventSystemDispatch:
                data->eventSystemDispatchTime = timestamp;
                break;
            case kIOHIDEventPerfDataPointEventSystemClientDispatch:
                data->eventSystemClientDispatchTime = timestamp;
                break;
        }
    }
}

typedef struct {
    size_t             count;
    size_t             length;
    size_t             head;
    size_t             tail;
} _IOHIDSimpleQueueHeader;

IOHIDSimpleQueueRef _IOHIDSimpleQueueCreate (CFAllocatorRef allocator, size_t entrySize, size_t count)
{
    size_t bufferLength = entrySize * (count + 1) + sizeof(_IOHIDSimpleQueueHeader);
    
    CFMutableDataRef buffer = CFDataCreateMutable(allocator, bufferLength);
    if (!buffer) {
        return buffer;
    }
    
    CFDataSetLength(buffer, bufferLength);
    
    _IOHIDSimpleQueueHeader * header = (_IOHIDSimpleQueueHeader *) CFDataGetBytePtr(buffer);
    
    header->count  = count + 1;
    header->length = entrySize;
    header->head   = 0;
    header->tail   = 0;
    
    return (IOHIDSimpleQueueRef) buffer;
}

void * _IOHIDSimpleQueuePeek (IOHIDSimpleQueueRef buffer)
{
    _IOHIDSimpleQueueHeader * header = (_IOHIDSimpleQueueHeader *)CFDataGetBytePtr(buffer);
    
    size_t tail = header->tail;
    if (tail == header->head) {
        return NULL;
    }
    return (void *) ((uint8_t*)header + sizeof(header) + header->length * tail);
}

void _IOHIDSimpleQueueApplyBlock (IOHIDSimpleQueueRef buffer, IOHIDSimpleQueueBlock applier, void * ctx)
{
    _IOHIDSimpleQueueHeader * header = (_IOHIDSimpleQueueHeader *) CFDataGetBytePtr(buffer);
    size_t head = header->head;
    do {
        if (header->tail == head) {
            return;
        }
        void *entry =  (void *) ((uint8_t*)header + sizeof (_IOHIDSimpleQueueHeader) + header->length * head);
        
        applier (entry, ctx);
        
        head = (head + 1) % (header->count);
        
    } while (true);
    
}

IOReturn  _IOHIDSimpleQueueEnqueue (IOHIDSimpleQueueRef buffer, void *entry, boolean_t doOverride)
{
    
    IOReturn status = kIOReturnSuccess;
    
    _IOHIDSimpleQueueHeader * header = (_IOHIDSimpleQueueHeader *) CFDataGetBytePtr(buffer);
    
    size_t tail = header->tail;
    size_t newTail = (tail + 1) % (header->count);
    
    if (newTail == header->head) {
        if (doOverride) {
            header->head = (header->head + 1) % (header->count);
        } else {
            status = kIOReturnNoSpace;
            return status;
        }
    }
    
    memcpy ((uint8_t*) header + sizeof (_IOHIDSimpleQueueHeader) + header->length * tail, entry, header->length);
    
    header->tail = newTail;
    
    return status;
}


boolean_t  _IOHIDSimpleQueueDequeue (IOHIDSimpleQueueRef buffer, void * entry)
{
    void * e = _IOHIDSimpleQueuePeek (buffer);
    
    if (!e) {
        return false;
    }
    
    _IOHIDSimpleQueueHeader * header = (_IOHIDSimpleQueueHeader *) CFDataGetBytePtr(buffer);
    
    if (entry) {
        memcpy(entry, e, header->length);
    }
    
    header->head = (header->head + 1) % (header->count);
    
    return true;
}

void _IOHIDDictionaryAddSInt32 (CFMutableDictionaryRef dict, CFStringRef key, SInt32 value)
{
    CFNumberRef num = CFNumberCreate(CFGetAllocator(dict), kCFNumberSInt32Type, &value);
    if (num) {
        CFDictionaryAddValue(dict, key, num);
        CFRelease(num);
    }
}

void _IOHIDDictionaryAddSInt64 (CFMutableDictionaryRef dict, CFStringRef key, SInt64 value)
{
    CFNumberRef num = CFNumberCreate(CFGetAllocator(dict), kCFNumberSInt64Type, &value);
    if (num) {
        CFDictionaryAddValue(dict, key, num);
        CFRelease(num);
    }
}


void _IOHIDDictionaryAddCStr (CFMutableDictionaryRef dict, CFStringRef key, const char * cStr)
{
    CFStringRef str = CFStringCreateWithCString(CFGetAllocator(dict), cStr, kCFStringEncodingMacRoman);
    if (str) {
        CFDictionaryAddValue(dict, key, str);
        CFRelease(str);
    }
}


void _IOHIDArrayAppendSInt64 (CFMutableArrayRef array, SInt64 value)
{
    CFNumberRef num = CFNumberCreate(CFGetAllocator(array), kCFNumberSInt64Type, &value);
    if (num) {
        CFArrayAppendValue(array, num);
        CFRelease(num);
    }
}


const void * _IOHIDObjectInternalRetainCallback (CFAllocatorRef allocator __unused, const void * cf)
{
    return (const void *)_IOHIDObjectInternalRetain ((CFTypeRef)cf);
}

void _IOHIDObjectInternalReleaseCallback (CFAllocatorRef allocator __unused, const  void * cf)
{
    _IOHIDObjectInternalRelease ((CFTypeRef)cf);
}


CFTypeRef _IOHIDObjectInternalRetain (CFTypeRef cf)
{
    const IOHIDObjectClass * class = (const IOHIDObjectClass *)_CFRuntimeGetClassWithTypeID(CFGetTypeID(cf));
    if (class) {
        class->intRetainCount (+1, cf);
    }
    return cf;
}

void _IOHIDObjectInternalRelease (CFTypeRef cf)
{
    const IOHIDObjectClass * class = (const IOHIDObjectClass *)_CFRuntimeGetClassWithTypeID(CFGetTypeID(cf));
    if (class) {
       class->intRetainCount (-1, cf);
    }
}

uint32_t _IOHIDObjectRetainCount (intptr_t op, CFTypeRef cf,  boolean_t isInternal)
{
    uint32_t                  retainCount = 0;
    IOHIDObjectBase           *object =  (IOHIDObjectBase *) cf;
    uint32_t volatile         *cnt =  isInternal ? &object->xref : &object->ref;
    switch (op) {
        case 1:
            retainCount = atomic_fetch_add((_Atomic uint32_t volatile  *)cnt, 1);
            os_assert(retainCount < UINT_MAX);
            retainCount = 0;
            break;
        case 0:
            retainCount = atomic_load((_Atomic uint32_t *)cnt);
            break;
        case -1:
            retainCount = atomic_fetch_sub((_Atomic uint32_t volatile *)cnt, 1);
            os_assert(retainCount);
            if (retainCount == 1) {
                const IOHIDObjectClass * class = (const IOHIDObjectClass *)_CFRuntimeGetClassWithTypeID(CFGetTypeID(cf));
                void (*finalizer)(CFTypeRef cf) = isInternal ? class->intFinalize : class->cfClass.finalize;
                if (finalizer) {
                    finalizer (cf);
                }
                if (isInternal) {
                    CFAllocatorRef allocator = CFGetAllocator(cf);
                    uint8_t * memory = (uint8_t *) cf;
                    if (!_CFAllocatorIsSystemDefault (allocator)) {
                        memory = memory - 16;
                    }
                    CFAllocatorDeallocate(allocator, (void*)memory);
                } else {
                    _IOHIDObjectInternalRelease (cf);
                }
            }
            retainCount = 0;
            break;
        default:
            break;
    }
    return  retainCount;
}

uint32_t _IOHIDObjectExtRetainCount (intptr_t op, CFTypeRef cf)
{
    return _IOHIDObjectRetainCount (op, cf, false);
}

uint32_t _IOHIDObjectIntRetainCount (intptr_t op, CFTypeRef cf)
{
    return _IOHIDObjectRetainCount (op, cf, true);
}

CFTypeRef _IOHIDObjectCreateInstance (CFAllocatorRef allocator, CFTypeID typeID, CFIndex extraBytes, unsigned char * __unused category)
{
    
    IOHIDObjectBase * object;
    object = (IOHIDObjectBase *) _CFRuntimeCreateInstance(allocator, typeID, extraBytes, NULL);
    
    if (!object) {
        return object;
    }
    
    bzero((uint8_t*)object + sizeof(CFRuntimeBase), extraBytes);
    
    object->ref  = 1;
    object->xref = 1;
    
    return object;
}


static void __IOHIDCFSetFunctionApplier (const void *value, void *context)
{
    IOHIDCFSetBlock block = (IOHIDCFSetBlock) context;
    block (value);
}

void _IOHIDCFSetApplyBlock (CFSetRef set, IOHIDCFSetBlock block)
{
    CFSetApplyFunction(set, __IOHIDCFSetFunctionApplier, block);
}

static void __IOHIDCFDictionaryFunctionApplier (const void *key, const void *value, void *context)
{
    IOHIDCFDictionaryBlock block = (IOHIDCFDictionaryBlock) context;
    block (key, value);
}

void _IOHIDCFDictionaryApplyBlock (CFDictionaryRef set, IOHIDCFDictionaryBlock block)
{
    CFDictionaryApplyFunction(set, __IOHIDCFDictionaryFunctionApplier, block);
}

static void __IOHIDCFArrayFunctionApplier(const void *value, void *context)
{
    IOHIDCFArrayBlock block = (IOHIDCFArrayBlock)context;
    block(value);
}

void _IOHIDCFArrayApplyBlock(CFArrayRef array, IOHIDCFArrayBlock block)
{
    CFArrayApplyFunction(array,
                         CFRangeMake(0, CFArrayGetCount(array)),
                         __IOHIDCFArrayFunctionApplier,
                         block);
}
