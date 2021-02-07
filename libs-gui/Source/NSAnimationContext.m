/*
   NSAnimationContext.h
 
   Created by Gregory John Casamento on Wed Jun 10 2015.
   Copyright (c) 2015 Free Software Foundation, Inc.
 
   Author: Gregory Casamento <greg.casamento@gmail.com>
 
   This file is part of the GNUstep GUI Library.
 
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
 */ 

#import <AppKit/NSAnimationContext.h>

static NSAnimationContext *_currentContext = nil;

@implementation NSAnimationContext : NSObject

// Begin and end grouping
+ (void) beginGrouping
{
}

+ (void) endGrouping
{
}

// Retrieve current context
+ (NSAnimationContext *)currentContext
{
  return _currentContext;
}

// run
+ (void)runAnimationGroup: (GSAnimationContextChanges)changes
        completionHandler: (GSAnimationContextCompletionHandler)completionHandler
{
}

// Properties...
- (void) setDuration: (NSTimeInterval)duration
{
  _duration = duration;
}

- (NSTimeInterval) duration
{
  return _duration;
}

- (GSAnimationContextCompletionHandler) completionHandler
{
  return _completionHandler;
}

- (void) setCompletionHandler: (GSAnimationContextCompletionHandler) completionHandler
{
  _completionHandler = completionHandler;
}

- (void *) timingFunction
{
  return NULL;
}

- (void) setTimingFunction: (void *)timingFunction
{
}

@end

