/* Declarations for NSTimer for GNUStep
   Copyright (C) 1995, 1996, 1999 Free Software Foundation, Inc.

   Written by:  Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>
   Date: 1995
   
   This file is part of the GNUstep Base Library.

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

#ifndef __NSTimer_h_GNUSTEP_BASE_INCLUDE
#define __NSTimer_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import	<Foundation/NSDate.h>

@class NSTimer;
DEFINE_BLOCK_TYPE(GSTimerBlock, void, NSTimer*);

#if	defined(__cplusplus)
extern "C" {
#endif

/*
 *	NB. NSRunLoop is optimised using a hack that knows about the
 *	class layout for the fire date and invialidation flag in NSTimer.
 *	These MUST remain the first two items in the class.
 *	Other classes must not attempt to use instance variables as
 *	they are subject to change.
 */
GS_EXPORT_CLASS
@interface NSTimer : NSObject
{
#if	GS_EXPOSE(NSTimer)
@public
  NSDate 	 *_date;	/* Must be 1st - for NSRunLoop optimisation */
  BOOL		 _invalidated;	/* Must be 2nd - for NSRunLoop optimisation */
  BOOL		 _repeats;
  NSTimeInterval _interval;
  id		 _target;
  SEL		 _selector;
  id		 _info;
  GSTimerBlock   _block;
#endif
#if     GS_NONFRAGILE
#else
  /* Pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available.
   * Use this mechanism rather than changing the instance variable
   * layout (see Source/GSInternal.h for details).
   */
  @private id _internal GS_UNUSED_IVAR;
#endif
}

/* Creating timer objects. */

+ (NSTimer*) scheduledTimerWithTimeInterval: (NSTimeInterval)ti
				 invocation: (NSInvocation*)invocation
				    repeats: (BOOL)f;
  
+ (NSTimer*) scheduledTimerWithTimeInterval: (NSTimeInterval)ti
				     target: (id)object
				   selector: (SEL)selector
				   userInfo: (id)info
				    repeats: (BOOL)f;
  
+ (NSTimer *) scheduledTimerWithTimeInterval: (NSTimeInterval)ti
                                     repeats: (BOOL)f
                                       block: (GSTimerBlock)block;
  
+ (NSTimer*) timerWithTimeInterval: (NSTimeInterval)ti
		        invocation: (NSInvocation*)invocation
			   repeats: (BOOL)f;

+ (NSTimer*) timerWithTimeInterval: (NSTimeInterval)ti
			    target: (id)object
			  selector: (SEL)selector
			  userInfo: (id)info
			   repeats: (BOOL)f;

- (void) fire;
- (NSDate*) fireDate;
- (void) invalidate;
- (id) userInfo;

#if	OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (BOOL) isValid;
- (NSTimeInterval) timeInterval;
#endif

#if	OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
- (id) initWithFireDate: (NSDate*)fd
	       interval: (NSTimeInterval)ti
		 target: (id)object
	       selector: (SEL)selector
	       userInfo: (id)info
		repeats: (BOOL)f;
- (void) setFireDate: (NSDate*)fireDate;
#endif

#if	OS_API_VERSION(MAC_OS_X_VERSION_10_12, GS_API_LATEST)  
- (instancetype) initWithFireDate: (NSDate *)date 
                         interval: (NSTimeInterval)interval 
                          repeats: (BOOL)repeats 
                            block: (GSTimerBlock)block;
#endif
  
@end

#if	defined(__cplusplus)
}
#endif

#endif	/* __NSTimer_h_GNUSTEP_BASE_INCLUDE */
