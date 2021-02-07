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

#ifndef _NSProgress_h_GNUSTEP_BASE_INCLUDE
#define _NSProgress_h_GNUSTEP_BASE_INCLUDE

#import	<GNUstepBase/GSVersionMacros.h>
#import	<Foundation/NSObject.h>
#import <GNUstepBase/GSBlocks.h>

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSString, NSDictionary, NSArray, NSNumber, NSURL, NSProgress;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_9, GS_API_LATEST)

typedef NSString* NSProgressKind;
typedef NSString* NSProgressUserInfoKey;
typedef NSString* NSProgressFileOperationKind;  

DEFINE_BLOCK_TYPE_NO_ARGS(GSProgressCancellationHandler, void);
DEFINE_BLOCK_TYPE_NO_ARGS(GSProgressPausingHandler, void);
DEFINE_BLOCK_TYPE(NSProgressPublishingHandler, void, NSProgress*);
DEFINE_BLOCK_TYPE_NO_ARGS(NSProgressUnpublishingHandler, void); 
DEFINE_BLOCK_TYPE_NO_ARGS(GSProgressPendingUnitCountBlock, void); 
DEFINE_BLOCK_TYPE_NO_ARGS(GSProgressResumingHandler, void); 
  
GS_EXPORT_CLASS
@interface NSProgress : NSObject
{
#if	GS_EXPOSE(NSProgress)
#endif
#if     GS_NONFRAGILE
#  if	defined(GS_NSProgress_IVARS)
@public
GS_NSProgress_IVARS;
#  endif
#else
  /* Pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available.
   * Use this mechanism rather than changing the instance variable
   * layout (see Source/GSInternal.h for details).
   */
  @private id _internal GS_UNUSED_IVAR;
#endif
}
  
// Creating progress objects...
- (instancetype) initWithParent: (NSProgress *)parent 
                       userInfo: (NSDictionary *)userInfo;
+ (NSProgress *) discreteProgressWithTotalUnitCount: (int64_t)unitCount;
+ (NSProgress *) progressWithTotalUnitCount: (int64_t)unitCount;
+ (NSProgress *) progressWithTotalUnitCount: (int64_t)unitCount 
  parent: (NSProgress *)parent 
  pendingUnitCount: (int64_t)portionOfParentTotalUnitCount;

// Current progress
+ (NSProgress *) currentProgress;
- (void) becomeCurrentWithPendingUnitCount: (int64_t)unitCount;
- (void) addChild: (NSProgress *)child
  withPendingUnitCount: (int64_t)inUnitCount;
- (void) resignCurrent;

// Reporting progress
- (int64_t) totalUnitCount;
- (void) setTotalUnitCount: (int64_t)unitCount;

- (int64_t) completedUnitCount;
- (void) setCompletedUnitCount: (int64_t)unitCount;

- (NSString *) localizedDescription;
- (NSString *) localizedAdditionalDescription;

// Observing progress
- (double) fractionCompleted;

// Controlling progress
- (BOOL) isCancellable;
- (BOOL) isCancelled;
- (void) cancel;
- (void) setCancellationHandler: (GSProgressCancellationHandler) handler;

- (BOOL) isPausable;
- (BOOL) isPaused;
- (void) pause;
- (void) setPausingHandler: (GSProgressPausingHandler) handler;

- (void) resume;
- (void) setResumingHandler: (GSProgressResumingHandler) handler;

// Progress Information
- (BOOL) isIndeterminate;
- (void) setIndeterminate: (BOOL)flag;
- (NSProgressKind) kind;
- (void) setKind: (NSProgressKind)k;
- (void) setUserInfoObject: (id)obj
                    forKey: (NSProgressUserInfoKey)key;

// Instance property accessors...
- (void) setFileOperationKind: (NSProgressFileOperationKind)k;
- (NSProgressFileOperationKind) fileOperationKind;
- (void) setFileUrl: (NSURL *)u;
- (NSURL *) fileUrl;
- (BOOL) isFinished;
- (BOOL) isOld;
- (void) setEstimatedTimeRemaining: (NSNumber *)n;
- (NSNumber *) estimatedTimeRemaining;
- (void) setFileCompletedCount: (NSNumber *)n;
- (NSNumber *) fileCompletedCount;
- (void) setFileTotalCount: (NSNumber *)n;
- (NSNumber *) fileTotalCount;
- (void) setThroughput: (NSNumber *)n;
- (NSNumber *) throughtput;

// Instance methods
- (void) publish;
- (void) unpublish;
- (void) performAsCurrentWithPendingUnitCount: (int64_t)unitCount 
  usingBlock: (GSProgressPendingUnitCountBlock)work;

// Type methods
+ (id) addSubscriberForFileURL: (NSURL *)url 
         withPublishingHandler: (NSProgressPublishingHandler)publishingHandler;
+ (void) removeSubscriber: (id)subscriber;
  
@end


@protocol NSProgressReporting

- (NSProgress *) progress;

@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSProgress_h_GNUSTEP_BASE_INCLUDE */

