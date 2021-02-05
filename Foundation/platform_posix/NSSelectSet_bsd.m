/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#ifdef PLATFORM_USES_BSD_SOCKETS
#import "NSSelectSet_bsd.h"
#import "NSSocket_bsd.h"
#import <Foundation/NSError.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSString.h>
#import <Foundation/NSRaise.h>

#include <errno.h>
#import <sys/select.h>
#import <sys/types.h>

@implementation NSSelectSet(bsd)

+allocWithZone:(NSZone *)zone {
   return NSAllocateObject([NSSelectSet_bsd class],0,NULL);
}

@end

@implementation NSSelectSet_bsd

typedef struct {
   int     max;
   fd_set *fdset;
} native_set;

native_set *native_set_new(int max){
   native_set *result=NSZoneCalloc(NULL,1,sizeof(native_set));

   result->max=FD_SETSIZE;
   while(result->max<max)
    result->max*=2;
   result->fdset=NSZoneCalloc(NULL,1,sizeof(fd_mask)*(result->max/NFDBITS));

   return result;
}

void native_set_free(native_set *set){
   NSZoneFree(NULL,set->fdset);
   NSZoneFree(NULL,set);
}

void native_set_clear(native_set *set,int descriptor){
#if defined(LINUX)
   __FDS_BITS(set->fdset)[descriptor/NFDBITS]&=~(1<<(descriptor%NFDBITS));
#else
   set->fdset->fds_bits[descriptor/NFDBITS]&=~(1<<(descriptor%NFDBITS));
#endif
}

void native_set_set(native_set *set,int descriptor){
   while(descriptor>set->max){
    int clear=set->max;

    set->max*=2;
    set->fdset=NSZoneRealloc(NULL,set->fdset,sizeof(fd_mask)*(set->max/NFDBITS));

    for(;clear<set->max;clear++)
     native_set_clear(set,clear);
   }

#ifdef LINUX
   __FDS_BITS(set->fdset)[descriptor/NFDBITS]|=(1<<(descriptor%NFDBITS));
#else
   set->fdset->fds_bits[descriptor/NFDBITS]|=(1<<(descriptor%NFDBITS));
#endif
}

BOOL native_set_is_set(native_set *native,int descriptor) {
  if(descriptor>native->max)
   return NO;

#ifdef LINUX
   return (__FDS_BITS(native->fdset)[descriptor/NFDBITS]&(1<<(descriptor%NFDBITS)))?YES:NO;
#else
   return (native->fdset->fds_bits[descriptor/NFDBITS]&(1<<(descriptor%NFDBITS)))?YES:NO;
#endif
}

static int maxDescriptorInSet(NSSet *set){
   int           result=-1;
   NSEnumerator *state=[set objectEnumerator];
   NSSocket_bsd *socket;

   while((socket=[state nextObject])!=nil){
    int check=[socket descriptor];

    if(check>result)
     result=check;
   }

   return result;
}

static int maxDescriptorInThreeSets(NSSet *set1,NSSet *set2,NSSet *set3){
   int check,result=maxDescriptorInSet(set1);

   check=maxDescriptorInSet(set2);
   if(check>result)
    result=check;

   check=maxDescriptorInSet(set3);
   if(check>result)
    result=check;

   return result;
}

void NSSelectSetShutdownForCurrentThread() {
// do nothing
}

static void transferSetToNative(NSSet *set,native_set *native){
   NSEnumerator *state=[set objectEnumerator];
   NSSocket_bsd *socket;

   while((socket=[state nextObject])!=nil)
    native_set_set(native,[socket descriptor]);
}

static void transferNativeToSetWithOriginals(native_set *sset,NSMutableSet *set,NSSet *original,NSSocket_bsd *cheater){
   int i;

   for(i=0;i<sset->max;i++){
    if(native_set_is_set(sset,i)){
     [cheater setDescriptor:i];
     [set addObject:[original member:cheater]];
    }
   }
}


- (NSError *)waitForSelectWithOutputSet:(NSSelectSet **)outputSetX beforeDate:(NSDate *)beforeDate
{
    NSError *result = nil;
    NSSocket_bsd *cheater = [NSSocket_bsd socketWithDescriptor:-1];
    int maxDescriptor = maxDescriptorInThreeSets(_readSet, _writeSet, _exceptionSet);
    native_set *activeRead = native_set_new(maxDescriptor);
    native_set *activeWrite = native_set_new(maxDescriptor);
    native_set *activeExcept = native_set_new(maxDescriptor);
    struct timeval timeval;
    NSTimeInterval interval = 1.0;

    // See NSTask_linux.m
    int numFds = 0;
    while (result == nil && numFds == 0 && interval > 0.0) {
        transferSetToNative(_readSet, activeRead);
        transferSetToNative(_writeSet, activeWrite);
        transferSetToNative(_exceptionSet, activeExcept);

        interval = [beforeDate timeIntervalSinceNow];

        if (interval > 1000000) {
            interval = 1000000;
        }
        if (interval < 0) {
            interval = 0;
        }

        timeval.tv_sec = interval;
        interval -= timeval.tv_sec;
        timeval.tv_usec = (typeof(timeval.tv_usec))(interval * 1000);

        if ((numFds = select(maxDescriptor + 1, activeRead->fdset, activeWrite->fdset, activeExcept->fdset, &timeval)) < 0) {
            if (errno != EINTR) {
                result = [NSError errorWithDomain:NSPOSIXErrorDomain code:errno userInfo:nil];
            }
        }

        if (NSDebugEnabled) {
            interval = [beforeDate timeIntervalSinceNow];
            if (interval > 0.0) {
                NSLog(@"in %@: select returned 0 before timeout ended. Did you wait on a non-blocking socket?", NSStringFromSelector(_cmd));
            }
        }
    }

    if (result == nil) {
        NSSelectSet_bsd *outputSet = (NSSelectSet_bsd *)[[[NSSelectSet alloc] init] autorelease];
        if (numFds) {
            transferNativeToSetWithOriginals(activeRead, outputSet->_readSet, _readSet, cheater);
            transferNativeToSetWithOriginals(activeWrite, outputSet->_writeSet, _writeSet, cheater);
            transferNativeToSetWithOriginals(activeExcept, outputSet->_exceptionSet, _exceptionSet, cheater);
        }
        *outputSetX = outputSet;
    }

    native_set_free(activeRead);
    native_set_free(activeWrite);
    native_set_free(activeExcept);

    return result;
}


@end
#endif
