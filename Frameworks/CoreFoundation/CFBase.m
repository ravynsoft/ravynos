/*
 * CFAllocator classes
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


#import <CoreFoundation/CFBase.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSCFTypeID.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSPlatform.h>
#import <pthread.h>

static struct CFAllocator {
    uintptr_t cfisa;
    CFIndex version;
    void *info;
    void *retain;
    void *release;
    void *copyDescription;
    void *allocate;
    void *reallocate;
    void *deallocate;
    void *preferredSize;
};

@interface __CFAllocator : NSObject {
    CFAllocatorContext _context;
}

-initWithContext:(CFAllocatorContext *)ctx;
-(CFAllocatorContext *)getContext;
@end

@implementation __CFAllocator
-initWithContext:(CFAllocatorContext *)ctx {
    self = [super new];
    memmove(&_context, ctx, sizeof(CFAllocatorContext));
    return self;
}

-(CFAllocatorContext *)getContext {
    return &_context;
}
@end

static void *__CFAllocatorMallocAllocate(CFIndex size, CFOptionFlags hint, void *info) {
    return malloc(size);
}

static CFStringRef __CFAllocatorMallocCopyDescription(const void *info) {
    return (__bridge_retained CFStringRef)[NSString stringWithString:@"CFAllocatorMalloc"];
}

static void __CFAllocatorMallocDeallocate(void *ptr, void *info) {
    free(ptr);
}

static void *__CFAllocatorMallocReallocate(void *ptr, CFIndex size, CFOptionFlags hint, void *info) {
    return realloc(ptr, size);
}

static void *__CFAllocatorNullAllocate(CFIndex size, CFOptionFlags hint, void *info) {
    return NULL;
}

static CFStringRef __CFAllocatorNullCopyDescription(const void *info) {
    return (__bridge_retained CFStringRef)[NSString stringWithString:@"CFAllocatorNull"];
}

static void __CFAllocatorNullDeallocate(void *ptr, void *info) {
    return;
}

static void *__CFAllocatorNullReallocate(void *ptr, CFIndex size, CFOptionFlags hint, void *info) {
    return NULL;
}

static CFIndex __CFAllocatorNullPreferredSize(CFIndex size, CFOptionFlags hint, void *info) {
    return 0;
}

static struct CFAllocator __CFAllocatorMalloc = {
    .cfisa = 0,
    .version = 0,
    .info = NULL,
    .retain = NULL,
    .release = NULL,
    .copyDescription = __CFAllocatorMallocCopyDescription,
    .allocate = __CFAllocatorMallocAllocate,
    .reallocate = __CFAllocatorMallocReallocate,
    .deallocate = __CFAllocatorMallocDeallocate,
    .preferredSize = NULL,
};

static struct CFAllocator __CFAllocatorNull = {
    .cfisa = 0,
    .version = 0,
    .info = NULL,
    .retain = NULL,
    .release = NULL,
    .copyDescription = __CFAllocatorNullCopyDescription,
    .allocate = __CFAllocatorNullAllocate,
    .reallocate = __CFAllocatorNullReallocate,
    .deallocate = __CFAllocatorNullDeallocate,
    .preferredSize = __CFAllocatorNullPreferredSize,
};

const CFAllocatorRef kCFAllocatorDefault = NULL;
const CFAllocatorRef kCFAllocatorSystemDefault = &__CFAllocatorMalloc;
const CFAllocatorRef kCFAllocatorMalloc = &__CFAllocatorMalloc;
const CFAllocatorRef kCFAllocatorMallocZone = &__CFAllocatorMalloc;
const CFAllocatorRef kCFAllocatorNull = &__CFAllocatorNull;
const CFAllocatorRef kCFAllocatorUseContext;

static pthread_key_t __kCFAllocatorDefaultKey;
static Boolean __kCFAllocatorDefaultKeyValid = FALSE;

CFAllocatorRef CFAllocatorGetDefault(void) {
    // First, make sure our predefined allocators are valid. We do it here
    // because this function is called in almost every path.
    __CFAllocatorMalloc.cfisa = (uintptr_t)[__CFAllocator class];
    __CFAllocatorNull.cfisa = (uintptr_t)[__CFAllocator class];

    // check for valid TSD key
    if(!__kCFAllocatorDefaultKeyValid) {
        if(pthread_key_create(&__kCFAllocatorDefaultKey, NULL) == 0) {
            __kCFAllocatorDefaultKeyValid = TRUE;
        }
        // we know there's no value set if we had to create the key
        return kCFAllocatorSystemDefault;
    }

    void *val = pthread_getspecific(__kCFAllocatorDefaultKey);
    if(val != NULL)
        return val;
    return kCFAllocatorSystemDefault;
}

void CFAllocatorSetDefault(CFAllocatorRef self) {
    // check for valid TSD key
    if(!__kCFAllocatorDefaultKeyValid)
        if(pthread_key_create(&__kCFAllocatorDefaultKey, NULL) != 0)
            return;
    __kCFAllocatorDefaultKeyValid = TRUE;
    pthread_setspecific(__kCFAllocatorDefaultKey, self);
}

CFTypeID CFAllocatorGetTypeID(void) {
    return kNSCFTypeAllocator;
}

CFAllocatorRef CFAllocatorCreate(CFAllocatorRef self, CFAllocatorContext *context) {
    CFAllocatorRef newAlloc = NULL;
    __CFAllocator *o = [__CFAllocator alloc];
    size_t size = class_getInstanceSize([o class]);

    if(self == kCFAllocatorUseContext) {
        newAlloc = context->allocate(size, 0, NULL);
    } else {
        CFAllocatorRef alloc = CFAllocatorGetDefault();
        newAlloc = CFAllocatorAllocate(alloc, size, 0);
    }
    memmove(newAlloc, o, size); // fix up the isa pointer
    o = nil;
    [(__CFAllocator *)newAlloc initWithContext:context];
    return newAlloc;
}

void CFAllocatorGetContext(CFAllocatorRef self, CFAllocatorContext *context) {
    if(self == NULL)
        self = CFAllocatorGetDefault();
    memmove(context, [(__CFAllocator *)self getContext], sizeof(CFAllocatorContext));
}

CFIndex CFAllocatorGetPreferredSizeForSize(CFAllocatorRef self, CFIndex size, CFOptionFlags hint) {
    if(size <= 0)
        return size;
    if(self == NULL)
        self = CFAllocatorGetDefault();
    CFAllocatorPreferredSizeCallBack cb = [(__CFAllocator *)self getContext]->preferredSize;
    if(cb == NULL)
        return size;
    return cb(size, 0, NULL);
}

void *CFAllocatorAllocate(CFAllocatorRef self, CFIndex size, CFOptionFlags hint) {
    if(size <= 0)
        return NULL;
    if(self == NULL)
        self = CFAllocatorGetDefault();
    CFAllocatorAllocateCallBack cb = [(__CFAllocator *)self getContext]->allocate;
    if(cb == NULL)
        return NULL;
    return cb(size, 0, NULL);
}

void CFAllocatorDeallocate(CFAllocatorRef self, void *ptr) {
    if(ptr == NULL)
        return;
    if(self == NULL)
        self = CFAllocatorGetDefault();
    CFAllocatorDeallocateCallBack cb = [(__CFAllocator *)self getContext]->deallocate;
    if(cb == NULL)
        return;
    cb(ptr, NULL);
}

void *CFAllocatorReallocate(CFAllocatorRef self, void *ptr, CFIndex size, CFOptionFlags hint) {
    if(self == NULL)
        self = CFAllocatorGetDefault();
    
    if(ptr == NULL) {
        if(size > 0) { // need to allocate
            return CFAllocatorAllocate(self, size, hint);
        }
        return NULL;
    }

    // ptr is not null
    if(size == 0) { // need to deallocate
        CFAllocatorDeallocate(self, ptr);
        return NULL; // guessing here... Apple's doc doesn't specify return code
    }

    CFAllocatorReallocateCallBack cb = [(__CFAllocator *)self getContext]->reallocate;
    if(cb == NULL)
        return NULL;
    return cb(ptr, size, 0, NULL);
}


CFTypeID CFGetTypeID(CFTypeRef self){
   return [(id)self _cfTypeID];
}

CFTypeRef CFRetain(CFTypeRef self){
	return (CFTypeRef)[(id) self retain];
}

void CFRelease(CFTypeRef self){
	return [(id) self release];
}
CFIndex CFGetRetainCount(CFTypeRef self){
   return [(id) self retainCount];
}

CFAllocatorRef CFGetAllocator(CFTypeRef self){
   NSUnimplementedFunction();
   return 0;
}

CFHashCode CFHash(CFTypeRef self){
	return [(id)self hash];
}

Boolean CFEqual(CFTypeRef self,CFTypeRef other){
	return [(id) self isEqual:(id)other];
}

CFStringRef CFCopyTypeIDDescription(CFTypeID typeID){
	NSUnimplementedFunction();
	return 0;
}

CFStringRef CFCopyDescription(CFTypeRef self){
   return (CFStringRef)[[(id) self description] copy];
}

CFTypeRef CFMakeCollectable(CFTypeRef self){
   //does nothing on cocotron
   return 0;
}

#ifndef MACH
uint64_t mach_absolute_time(void) {
   return 0;
}

kern_return_t mach_timebase_info(mach_timebase_info_t timebase) {
   timebase->numer=1;
   timebase->denom=1;
   return KERN_FAILURE;
}
#endif

