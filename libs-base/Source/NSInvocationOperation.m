/**Implementation for NSInvocationOperation for GNUStep
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

   <title>NSInvocationOperation class reference</title>
   $Date$ $Revision$
   */

#import "common.h"

#import "Foundation/NSInvocationOperation.h"
#import "Foundation/NSException.h"
#import "Foundation/NSInvocation.h"
#import "Foundation/NSMethodSignature.h"
#import "Foundation/NSValue.h"
#import "GNUstepBase/GSObjCRuntime.h"

@implementation NSInvocationOperation

- (id) initWithInvocation: (NSInvocation *)inv
{
  if (((self = [super init])) != nil)
    {
      [inv retainArguments];
      _invocation = RETAIN(inv);
    }
  return self;
}

- (id) initWithTarget: (id)target selector: (SEL)aSelector object: (id)arg
{
  NSMethodSignature	*methodSignature;
  NSInvocation		*inv;

  methodSignature = [target methodSignatureForSelector: aSelector];
  inv = [NSInvocation invocationWithMethodSignature: methodSignature];
  [inv setTarget: target];
  [inv setSelector: aSelector];
  if ([methodSignature numberOfArguments] > 2)
    {
      [inv setArgument: &arg atIndex: 2];
    }
  return [self initWithInvocation: inv];
}

- (void) main
{
  if (![self isCancelled])
    {
      NS_DURING
	{
	  [_invocation invoke];
	}
      NS_HANDLER
	{
	  ASSIGN(_exception, localException);
	}
      NS_ENDHANDLER
    }
}

- (NSInvocation *) invocation
{
  return AUTORELEASE(RETAIN(_invocation));
}

- (id) result
{
  id result = nil;

  if (![self isFinished])
    {
      return nil;
    }
  if (nil != _exception)
    {
      [_exception raise];
    }
  else if ([self isCancelled])
    {
      [NSException raise: (id)NSInvocationOperationCancelledException
        format: @"*** %s: operation was cancelled", __PRETTY_FUNCTION__];
    }
  else
    {
      const char *returnType = [[_invocation methodSignature] methodReturnType];

      if (0 == strncmp(@encode(void),
        GSSkipTypeQualifierAndLayoutInfo(returnType), 1))
	{
	  [NSException raise: (id)NSInvocationOperationVoidResultException
            format: @"*** %s: void result", __PRETTY_FUNCTION__];
	}
      else if (0 == strncmp(@encode(id),
        GSSkipTypeQualifierAndLayoutInfo(returnType), 1))
	{
	  [_invocation getReturnValue: &result];
	}
      else
	{
	  unsigned char *buffer = malloc([[_invocation methodSignature]
					   methodReturnLength]);

	  [_invocation getReturnValue: buffer];
	  result = [NSValue valueWithBytes: buffer objCType: returnType];
	}
    }
  return result;
}

- (void) dealloc
{
  DESTROY(_invocation);
  DESTROY(_exception);
  [super dealloc];
}

@end

const NSString * NSInvocationOperationVoidResultException
  = @"NSInvocationOperationVoidResultException";
const NSString * NSInvocationOperationCancelledException
  = @"NSInvcationOperationCancelledException";
