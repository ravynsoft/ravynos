/* Implementation of extension methods to base additions

   Copyright (C) 2010 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02111 USA.

*/


#import "common.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSSet.h"
#import "GNUstepBase/NSProcessInfo+GNUstepBase.h"

#include <stdio.h>

@implementation NSProcessInfo(GNUstepBase)

static NSMutableSet	*_debug_set = nil;
static BOOL     debugTemporarilyDisabled = NO;


BOOL GSDebugSet(NSString *level)
{
  static id (*debugImp)(id,SEL,id) = 0;
  static SEL debugSel;

  if (debugTemporarilyDisabled == YES)
    {
      return NO;
    }
  if (debugImp == 0)
    {
      debugSel = @selector(member:);
      if (_debug_set == nil)
	{
	  [[NSProcessInfo processInfo] debugSet];
	}
      debugImp = (id (*)(id,SEL,id))[_debug_set methodForSelector: debugSel];
      if (debugImp == 0)
	{
	  fprintf(stderr, "Unable to set up with [NSProcessInfo-debugSet]\n");
	  return NO;
	}
    }
  if ((*debugImp)(_debug_set, debugSel, level) == nil)
    {
      return NO;
    }
  return YES;
}

- (BOOL) debugLoggingEnabled
{
  if (debugTemporarilyDisabled == YES)
    {
      return NO;
    }
  else
    {
      return YES;
    }
}

- (NSMutableSet *) debugSet
{
  if (_debug_set == nil)
    {
      int		argc = [[self arguments] count];
      NSMutableSet	*mySet;
      int		i;

      mySet = [NSMutableSet new];
      for (i = 0; i < argc; i++)
	{
	  NSString	*str = [[self arguments] objectAtIndex: i];

	  if ([str hasPrefix: @"--GNU-Debug="])
	    {
	      [mySet addObject: [str substringFromIndex: 12]];
	    }
	}
      _debug_set = mySet;
    }
  return _debug_set;
}

- (void) setDebugLoggingEnabled: (BOOL)flag
{
  if (flag == NO)
    {
      debugTemporarilyDisabled = YES;
    }
  else
    {
      debugTemporarilyDisabled = NO;
    }
}

@end

