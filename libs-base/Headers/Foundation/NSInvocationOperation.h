/**Interface for NSInvocationOperation for GNUStep
   Copyright (C) 2013 Free Software Foundation, Inc.

   Written by:  Graham Lee <iamleeg@gmail.com>
   Date: 2013

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

#ifndef __NSInvocationOperation_h_GNUSTEP_BASE_INCLUDE
#define __NSInvocationOperation_h_GNUSTEP_BASE_INCLUDE

#import <Foundation/NSOperation.h>
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@class NSInvocation;
@class NSException;

GS_EXPORT_CLASS
@interface NSInvocationOperation : NSOperation
{
  @private
    NSInvocation *_invocation;
    NSException *_exception;
    void        *_reserved;
}

- (id) initWithInvocation: (NSInvocation *)inv;
- (id) initWithTarget: (id)target selector: (SEL)aSelector object: (id)arg;

- (NSInvocation *) invocation;
- (id) result;

@end

extern const NSString * NSInvocationOperationVoidResultException;
extern const NSString * NSInvocationOperationCancelledException;

#if	defined(__cplusplus)
}
#endif

#endif /* OS_API_VERSION */
#endif /* __NSInvocationOperation_h_GNUSTEP_BASE_INCLUDE */
