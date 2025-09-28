/* Copyright (c) 2006-2007 Christopher J. W. Lloyd
   Copyright (C) 2024 Zoe Knox <zoe@ravynsoft.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

#ifdef PLATFORM_IS_POSIX
#import <Foundation/NSPlatform.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSZombieObject.h>
#import <Foundation/NSString.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSError.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

// some notes:
// - this uses POSIX thread local storage functions
// - there is no zone support (deprecated in 64 bit macOS)
// also corrected behavior of NSAllocateMemoryPages to be page-aligned


void *NSAllocateMemoryPages(NSUInteger byteCount) {
    size_t bytes = NSRoundUpToMultipleOfPageSize(byteCount);
    void *buffer = calloc(bytes, 1);
    if (buffer == NULL) {
        //fprintf(stderr, "NSAllocateMemoryPages(%u) failed. Error: %s\n", byteCount, strerror(errno));
        [NSException raise:NSInvalidArgumentException format:@"size: %u error: %s", bytes, strerror(errno)];
    }
    return buffer;
}

void NSDeallocateMemoryPages(void *pointer,NSUInteger byteCount) {
   free(pointer);
}

void NSCopyMemoryPages(const void *src,void *dst,NSUInteger byteCount) {
   const uint8_t *srcb=src;
   uint8_t       *dstb=dst;
   NSUInteger     i;

   for(i=0;i<byteCount;i++)
    dstb[i]=srcb[i];
}

NSZone *NSCreateZone(NSUInteger startSize,NSUInteger granularity,BOOL canFree){
   return NULL;
}

NSZone *NSDefaultMallocZone(void){
   return NULL;
}

void NSRecycleZone(NSZone *zone) {
}

void NSSetZoneName(NSZone *zone,NSString *name){
}

NSString *NSZoneName(NSZone *zone) {
   return @"zone";
}

NSZone *NSZoneFromPointer(void *pointer){
   return NULL;
}

void *NSZoneCalloc(NSZone *zone,NSUInteger numElems,NSUInteger numBytes){
    void *buffer = calloc(numElems,numBytes);
    if (buffer == NULL) {
        fprintf(stderr, "NSZoneCalloc(zone, %u, %u) failed. Error: %s\n", numElems, numBytes, strerror(errno));
    }
    return buffer;
}

void NSZoneFree(NSZone *zone,void *pointer){
   free(pointer);
}

void *NSZoneMalloc(NSZone *zone,NSUInteger size){
   void *buffer = malloc(size);
    if (buffer == NULL) {
        fprintf(stderr, "NSZoneMalloc(zone, %u) failed. Error: %s\n", size, strerror(errno));
    }
    return buffer;
}

void *NSZoneRealloc(NSZone *zone,void *pointer,NSUInteger size){
    void *buffer = realloc(pointer, size);
    if (buffer == NULL && size > 0) {
        fprintf(stderr, "NSZoneRealloc(zone, %p, %u) failed. Error: %s\n", pointer, size, strerror(errno));
    }
    return buffer;
}

static pthread_key_t _NSThreadInstanceKey() {
	static pthread_key_t key = -1;	
	if (key == -1) 
	{
		if (pthread_key_create(&key, NULL) != 0)
			[NSException raise:NSInternalInconsistencyException format:@"pthread_key_create failed"];
	}

	return key;
}

void NSPlatformSetCurrentThread(NSThread *thread) {
	pthread_setspecific(_NSThreadInstanceKey(), thread);
}

NSThread *NSPlatformCurrentThread() {
	NSThread *thread=pthread_getspecific(_NSThreadInstanceKey());
	
	if(!thread)
	{
		// maybe NSThread is not +initialize'd
		[NSThread class];
		thread=pthread_getspecific(_NSThreadInstanceKey());
        if(!thread) {
            thread = [NSThread alloc];
            if(thread) {
                NSPlatformSetCurrentThread(thread);
                {
                    NSAutoreleasePool *pool = [NSAutoreleasePool new];
                    [thread init];
                    [pool release];
                }
            }
        }        
		if(!thread)
		{
			[NSException raise:NSInternalInconsistencyException format:@"No current thread"];
		}
	}
	
	return thread;
}

NSUInteger NSPlatformDetachThread(void *(*func)(void *arg), void *arg, NSError **errorp) {
	pthread_t thread;
    int err;
	if ((err = pthread_create(&thread, NULL, func, arg)) != 0) {
        if (errorp) *errorp = [NSError errorWithDomain:NSPOSIXErrorDomain code:err userInfo:nil];
        return 0;
    }
	return (NSUInteger)thread;
}
#endif
