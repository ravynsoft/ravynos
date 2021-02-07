/* Definition of class NSProgress
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   Written by: 	Gregory Casamento <greg.casamento@gmail.com>
   Date: 	July 2019
   
   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#define	GS_NSProgress_IVARS	 \
  NSProgressKind _kind;  \
  NSProgressFileOperationKind _fileOperationKind; \
  NSURL *_fileUrl; \
  BOOL _isFinished; \
  BOOL _old; \
  NSNumber *_estimatedTimeRemaining; \
  NSNumber *_fileCompletedCount; \
  NSNumber *_fileTotalCount; \
  NSNumber *_throughput; \
  int64_t _totalUnitCount; \
  int64_t _completedUnitCount; \
  double _fractionCompleted; \
  NSMutableDictionary *_userInfo; \
  BOOL _cancelled; \
  BOOL _paused; \
  BOOL _cancellable; \
  BOOL _pausable; \
  BOOL _indeterminate; \
  BOOL _finished; \
  GSProgressCancellationHandler _cancellationHandler; \
  GSProgressPausingHandler _pausingHandler; \
  NSProgressPublishingHandler _publishingHandler; \
  NSProgressUnpublishingHandler _unpublishingHandler; \
  GSProgressPendingUnitCountBlock _pendingUnitCountHandler; \
  GSProgressResumingHandler _resumingHandler;              \
  NSString *_localizedDescription; \
  NSString *_localizedAdditionalDescription; \
  NSProgress *_parent;

#define	EXPOSE_NSProgress_IVARS

#import <Foundation/NSObject.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSURL.h>
#import <Foundation/NSString.h>
#import	<Foundation/NSProgress.h>
#import <Foundation/NSKeyValueObserving.h>

#define	GSInternal NSProgressInternal
#include "GSInternal.h"
GS_PRIVATE_INTERNAL(NSProgress)

// NSProgress for current thread....
static NSProgress *__currentProgress = nil;
static NSMutableDictionary *__subscribers = nil; 

@implementation NSProgress

+ (void) initialize
{
  if (self == [NSProgress class])
    {
      __subscribers = [[NSMutableDictionary alloc] initWithCapacity: 10];
    }
}

// Creating progress objects...
- (instancetype) initWithParent: (NSProgress *)parent 
                       userInfo: (NSDictionary *)userInfo
{
  self = [super init];
  if (self != nil)
    {
      GS_CREATE_INTERNAL(NSProgress);
      internal->_kind = nil;
      internal->_fileOperationKind = nil;
      internal->_fileUrl = nil;
      internal->_isFinished = NO;
      internal->_old = NO;
      internal->_estimatedTimeRemaining = nil;
      internal->_fileCompletedCount = nil;
      internal->_fileTotalCount = nil;
      internal->_throughput = nil;
      internal->_totalUnitCount = 0;
      internal->_completedUnitCount = 0;
      internal->_userInfo = RETAIN([userInfo mutableCopy]);
      internal->_cancelled = NO;
      internal->_cancellable = NO;
      internal->_paused = NO;
      internal->_pausable = NO;
      internal->_indeterminate = NO;
      internal->_finished = NO;
      internal->_localizedDescription = nil;
      internal->_localizedAdditionalDescription = nil;
      internal->_parent = parent;  // this is a weak reference and not retained.
    }
  return self;
}

- (void) dealloc
{
  RELEASE(internal->_userInfo);
  RELEASE(internal->_fileOperationKind);
  RELEASE(internal->_kind);
  RELEASE(internal->_estimatedTimeRemaining);
  RELEASE(internal->_fileCompletedCount);
  RELEASE(internal->_fileTotalCount);
  RELEASE(internal->_throughput);
  RELEASE(internal->_userInfo);
  RELEASE(internal->_localizedDescription);
  RELEASE(internal->_localizedAdditionalDescription);
  
  [super dealloc];
}

+ (NSProgress *) discreteProgressWithTotalUnitCount: (int64_t)unitCount
{
  NSProgress *p = [[NSProgress alloc] initWithParent: nil
    userInfo: [NSDictionary dictionary]];
  [p setTotalUnitCount: unitCount];
  return AUTORELEASE(p);
}

+ (NSProgress *) progressWithTotalUnitCount: (int64_t)unitCount
{
  NSProgress *p = [[NSProgress alloc] initWithParent: nil
    userInfo: [NSDictionary dictionary]];
  [p setTotalUnitCount: unitCount];
  return AUTORELEASE(p);
}

+ (NSProgress *)progressWithTotalUnitCount: (int64_t)unitCount 
  parent: (NSProgress *)parent 
  pendingUnitCount: (int64_t)portionOfParentTotalUnitCount
{
  NSProgress *p = [[NSProgress alloc] initWithParent: parent
    userInfo: [NSDictionary dictionary]];
  [p setTotalUnitCount: portionOfParentTotalUnitCount];
  return AUTORELEASE(p);
}

// Private methods
- (void) _setParent: (NSProgress *)p
{
  internal->_parent = p; // Not retained since this is defined in docs as a weak reference
}

- (NSProgress *) _parent
{
  return internal->_parent;
}

// Current progress
+ (NSProgress *) currentProgress
{
  return __currentProgress;
}

- (void) becomeCurrentWithPendingUnitCount: (int64_t)unitCount
{
  [self setTotalUnitCount: unitCount];
  __currentProgress = self;
}

- (void) addChild: (NSProgress *)child
  withPendingUnitCount: (int64_t)inUnitCount
{
  [child _setParent: self];
  [child setTotalUnitCount: inUnitCount];
}

- (void) resignCurrent
{
  int64_t completed = [__currentProgress completedUnitCount];
  [__currentProgress setCompletedUnitCount: completed + [self totalUnitCount]];
}

// Reporting progress
- (int64_t) totalUnitCount
{
  return internal->_totalUnitCount;
}

- (void) setTotalUnitCount: (int64_t)count
{
  internal->_totalUnitCount = count;
}

- (int64_t) completedUnitCount
{
  return internal->_completedUnitCount;
}

- (void) setCompletedUnitCount: (int64_t)count
{
  internal->_completedUnitCount = count;
  [self willChangeValueForKey: @"fractionCompleted"];
  internal->_fractionCompleted = (double)((double)internal->_completedUnitCount
                                         / (double)internal->_totalUnitCount);
  if(internal->_fractionCompleted >= 1)
    {
      [self willChangeValueForKey: @"finished"];
      internal->_finished = YES;
      [self didChangeValueForKey: @"finished"];
    }
  [self didChangeValueForKey: @"fractionCompleted"];

  __currentProgress = nil;
}

- (NSString *) localizedDescription
{
  return internal->_localizedDescription;
}

- (void) setLocalizedDescription: (NSString *)localDescription
{
  ASSIGNCOPY(internal->_localizedDescription, localDescription);
}

- (NSString *) localizedAdditionalDescription
{
  return internal->_localizedAdditionalDescription;
}

- (void) setLocalizedAdditionalDescription: (NSString *)localDescription
{
  ASSIGNCOPY(internal->_localizedAdditionalDescription, localDescription);
}

// Observing progress
- (double) fractionCompleted
{
  return internal->_fractionCompleted;
}

// Controlling progress
- (BOOL) isCancellable
{
  return internal->_cancellable;
}

- (BOOL) isCancelled
{
  return internal->_cancelled;
}

- (BOOL) cancelled
{
  return internal->_cancelled;
}

- (void) cancel
{
  [self willChangeValueForKey: @"cancelled"];
  CALL_BLOCK_NO_ARGS(internal->_cancellationHandler);
  internal->_cancelled = YES;
  [self didChangeValueForKey: @"cancelled"];
}

- (void) setCancellationHandler: (GSProgressCancellationHandler) handler
{
  internal->_cancellationHandler = handler;
}

- (BOOL) isPausable
{
  return internal->_pausable;
}

- (BOOL) isPaused
{
  return internal->_paused;
}

- (void) pause
{
  [self willChangeValueForKey: @"paused"];
  CALL_BLOCK_NO_ARGS(internal->_pausingHandler);
  internal->_paused = YES;
  [self didChangeValueForKey: @"paused"];
}

- (void) setPausingHandler: (GSProgressPausingHandler) handler
{
  internal->_pausingHandler = handler;
}

- (void) resume
{
  CALL_BLOCK_NO_ARGS(internal->_resumingHandler);
}

- (void) setResumingHandler: (GSProgressResumingHandler) handler
{
  internal->_resumingHandler = handler;
}

// Progress Information
- (BOOL) isIndeterminate
{
  return internal->_indeterminate;
}

- (BOOL) indeterminate
{
  return internal->_indeterminate;
}

- (void) setIndeterminate: (BOOL)flag
{
  internal->_indeterminate = flag;
}

- (NSProgressKind) kind
{
  return internal->_kind;
}

- (void) setKind: (NSProgressKind)k
{
  ASSIGN(internal->_kind, k);
}

- (void)setUserInfoObject: (id)obj
                   forKey: (NSProgressUserInfoKey)key
{
  [internal->_userInfo setObject: obj forKey: key];
}

// Instance property accessors...
- (void) setFileOperationKind: (NSProgressFileOperationKind)k;
{
  ASSIGN(internal->_fileOperationKind, k);
}

- (NSProgressFileOperationKind) fileOperationKind
{
  return internal->_fileOperationKind;
}

- (void) setFileUrl: (NSURL *)u
{
  ASSIGN(internal->_fileUrl, u);
}

- (NSURL*) fileUrl
{
  return internal->_fileUrl;
}

- (BOOL) isFinished
{
  return internal->_finished;
}

- (BOOL) finished
{
  return internal->_finished;
}

- (BOOL) isOld
{
  return internal->_old;
}

- (BOOL) old
{
  return internal->_old;
}

- (void) setEstimatedTimeRemaining: (NSNumber *)n
{
  ASSIGNCOPY(internal->_estimatedTimeRemaining, n);
}

- (NSNumber *) estimatedTimeRemaining
{
  return internal->_estimatedTimeRemaining;
}

- (void) setFileCompletedCount: (NSNumber *)n
{
  ASSIGNCOPY(internal->_fileCompletedCount, n);
}

- (NSNumber *) fileCompletedCount
{
  return internal->_fileCompletedCount;
}

- (void) setFileTotalCount: (NSNumber *)n
{
  ASSIGNCOPY(internal->_fileTotalCount, n);
}

- (NSNumber *) fileTotalCount
{
  return internal->_fileTotalCount;
}

- (void) setThroughput: (NSNumber *)n
{
  ASSIGNCOPY(internal->_throughput, n);
}

- (NSNumber *) throughtput
{
  return internal->_throughput;
}

// Instance methods
- (void) publish
{
  CALL_BLOCK(internal->_publishingHandler, self);
}

- (void) unpublish
{
  CALL_BLOCK_NO_ARGS(internal->_unpublishingHandler);
}

- (void) performAsCurrentWithPendingUnitCount: (int64_t)unitCount 
  usingBlock: (GSProgressPendingUnitCountBlock)work
{
  
  int64_t completed = [__currentProgress completedUnitCount];
  // Do pending work...
  CALL_BLOCK_NO_ARGS(work);
  // Update completion count...
  [self setCompletedUnitCount: completed + unitCount];
}

// Type methods
+ (id)addSubscriberForFileURL: (NSURL *)url 
        withPublishingHandler: (NSProgressPublishingHandler)publishingHandler
{
  // [__subscribers addObject: publishingHandler forObject: url];
  return nil;
}

+ (void) removeSubscriber: (id)subscriber
{
  // [__subscribers removeObject: subscriber];
}
  
@end


