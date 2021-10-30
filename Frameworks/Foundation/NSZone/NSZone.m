/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSZone.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSHashTable.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSZombieObject.h>
#import <Foundation/NSDebug.h>
#import <objc/objc-arc.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// NSZone functions implemented in platform subproject

void NSIncrementExtraRefCount(id object) {
    objc_retain_fast_np(object);
}

BOOL NSDecrementExtraRefCountWasZero(id object) {
    return objc_release_fast_no_destroy_np(object);
}

NSUInteger NSExtraRefCount(id object) {
    return object_getRetainCount_np(object) - 1;
}

BOOL NSShouldRetainWithZone(id object,NSZone *zone) {
   return (zone==NULL || zone==NSDefaultMallocZone() || zone==[object zone])?YES:NO;
}

static void (*__NSAllocateObjectHook)(id object) = 0;

void NSSetAllocateObjectHook(void (*hook)(id object))
{
    __NSAllocateObjectHook = hook;
}


id NSAllocateObject(Class class, NSUInteger extraBytes, NSZone *zone)
{
    id result;

    // FIXME: make this support Zones
    result = class_createInstance(class, extraBytes);
    if(result != nil) {
        if (__NSAllocateObjectHook) {
            __NSAllocateObjectHook(result);
        }
    }

    return result;
}


void NSDeallocateObject(id object)
{
#if !defined(APPLE_RUNTIME_4)
    //delete associations
    objc_removeAssociatedObjects(object);
#endif
    
    if (NSZombieEnabled) {
        NSRegisterZombie(object);
    } else {

#if !defined(GCC_RUNTIME_3) && !defined(APPLE_RUNTIME_4)
        object->isa = 0;
#endif

//         object = (id)((uintptr_t)object-sizeof(uintptr_t));
        object_dispose(object);
    }
}


id NSCopyObject(id object, NSUInteger extraBytes, NSZone *zone)
{
    if (object == nil) {
        return nil;
    }

    id result = NSAllocateObject(object_getClass(object), extraBytes, zone);

    if (result) {
        memcpy(result, object, class_getInstanceSize(object_getClass(object)) + extraBytes);
    }
    
    return result;
}
