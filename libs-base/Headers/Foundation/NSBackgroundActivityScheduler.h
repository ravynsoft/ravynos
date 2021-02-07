/* Definition of class NSBackgroundActivityScheduler
   Copyright (C) 2019 Free Software Foundation, Inc.
   
   By: Gregory John Casamento <greg.casamento@gmail.com>
   Date: Fri Oct 25 00:52:54 EDT 2019

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

#ifndef _NSBackgroundActivityScheduler_h_GNUSTEP_BASE_INCLUDE
#define _NSBackgroundActivityScheduler_h_GNUSTEP_BASE_INCLUDE

#include <Foundation/NSObject.h>
#include <Foundation/NSDate.h>
#include <Foundation/NSProcessInfo.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

// is the activity finished?
enum {
  NSBackgroundActivityResultFinished = 1,
  NSBackgroundActivityResultDeferred = 2,
};
typedef NSInteger NSBackgroundActivityResult;

// How the activity will be treated... this is declared in NSObjCRuntime on macOS.
enum {
  NSQualityOfServiceUserInteractive = 0x21,
  NSQualityOfServiceUserInitiated = 0x19,
  NSQualityOfServiceUtility = 0x11,
  NSQualityOfServiceBackground = 0x09,
  NSQualityOfServiceDefault = -1
};
typedef NSInteger NSQualityOfService;
  
@class NSString, NSTimer;

# ifndef __has_feature
# define __has_feature(x) 0
# endif

//# if __has_feature(blocks)
//typedef void(^NSBackgroundActivityCompletionHandler)(NSBackgroundActivityResult result) GSScheduledBlock;  
//# else
  DEFINE_BLOCK_TYPE(NSBackgroundActivityCompletionHandler, void, NSBackgroundActivityResult);
  DEFINE_BLOCK_TYPE(GSScheduledBlock, void, NSBackgroundActivityCompletionHandler);  
//# endif

GS_EXPORT_CLASS
@interface NSBackgroundActivityScheduler : NSObject
{
  NSString *_identifier;
  NSQualityOfService _qualityOfService;
  NSTimeInterval _interval;
  NSTimeInterval _tolerance;
  BOOL _repeats;
  BOOL _shouldDefer;
  NSTimer *_timer;
  NSActivityOptions _opts;
  id _token;
  NSString *_reason;
  BLOCK_SCOPE GSScheduledBlock _block;
}
  
- (instancetype) initWithIdentifier: (NSString *)identifier;

- (NSString *) identifier;
- (void) setIdentifier: (NSString *)identifier;

- (NSQualityOfService) qualityOfService;
- (void) setQualityOfService: (NSQualityOfService)qualityOfService;

- (BOOL) repeats;
- (void) setRepeats: (BOOL)flag;

- (NSTimeInterval) interval;
- (void) setInterval: (NSTimeInterval)interval;

- (NSTimeInterval) tolerance;
- (void) setTolerance: (NSTimeInterval)tolerance;

- (BOOL) shouldDefer;
- (void) setShouldDefer: (BOOL)flag;
  
- (void) scheduleWithBlock: (GSScheduledBlock)block;
  
- (void) invalidate;
  
@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSBackgroundActivityScheduler_h_GNUSTEP_BASE_INCLUDE */

