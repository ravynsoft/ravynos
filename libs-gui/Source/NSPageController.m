/* Implementation of class NSPageController
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: 27-07-2020

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#import <Foundation/NSArray.h>
#import <Foundation/NSKeyValueObserving.h>
#import <Foundation/NSArchiver.h>

#import "AppKit/NSPageController.h"
#import "AppKit/NSView.h"

@implementation NSPageController

- (instancetype) init
{
  self = [super init];
  if (self != nil)
    {
      _transitionStyle = NSPageControllerTransitionStyleStackHistory;
      _delegate = nil;
      _arrangedObjects = [[NSMutableArray alloc] initWithCapacity: 10];
      _selectedIndex = 0;
      _selectedViewController = nil;
    }
  return self;
}

- (void) dealloc
{
  RELEASE(_arrangedObjects);
  [super dealloc];
}

- (instancetype) initWithCoder: (NSCoder *)coder
{
  self = [super initWithCoder: coder];
  if (self != nil)
  {
    if ([coder allowsKeyedCoding])
      {
        if ([coder containsValueForKey: @"NSTransitionStyle"])
          {
            _transitionStyle = [coder decodeIntForKey: @"NSTransitionStyle"];
          }
      }
    else
      {
        [coder decodeValueOfObjCType: @encode(NSInteger)
                                  at: &_transitionStyle];
      }
  }
  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      [coder encodeInt: _transitionStyle
                forKey: @"NSTransitionStyle"];
    }
  else
    {
      [coder encodeValueOfObjCType: @encode(NSInteger)
                                at: &_transitionStyle];
    }
}

// Set/Get properties
- (NSPageControllerTransitionStyle) transitionStyle
{
  return _transitionStyle;
}

- (void) setTransitionStyle: (NSPageControllerTransitionStyle)style
{
  _transitionStyle = style;
}

- (id) delegate
{
  return _delegate;
}

- (void) setDelegate: (id)delegate
{
  _delegate = delegate;
}

- (NSArray *) arrangedObjects
{
  return _arrangedObjects;
}

- (void) setArrangedObjects: (NSArray *)array
{
  [_arrangedObjects removeAllObjects];
  [_arrangedObjects addObjectsFromArray: array];
}

- (NSInteger) selectedIndex
{
  return _selectedIndex;
}

- (void) setSelectedIndex: (NSInteger)index
{
  if ([_delegate respondsToSelector: @selector(pageControllerWillStartLiveTransition:)])
    {
      [_delegate pageControllerWillStartLiveTransition: self];
    }
  [self willChangeValueForKey: @"selectedIndex"];
  _selectedIndex = index;
  _selectedViewController = [_arrangedObjects objectAtIndex: _selectedIndex];
  [self didChangeValueForKey: @"selectedIndex"];

  // Complete...
  [self completeTransition];

  // End transition...
  if ([_delegate respondsToSelector: @selector(pageControllerDidEndLiveTransition:)])
    {
      [_delegate pageControllerDidEndLiveTransition: self];
    }

  // Notify delegate that transition is finished.
  if ([_delegate respondsToSelector: @selector(pageController:didTransitionToObject:)])
    {
      [_delegate pageController: self didTransitionToObject: _selectedViewController];
    }

  // Resize based on frame...
  if ([_delegate respondsToSelector: @selector(pageController:frameForObject:)])
    {
      NSRect rect = [_delegate pageController: self frameForObject: _selectedViewController];
      [[_selectedViewController view] setFrame: rect];
    }
}

- (NSViewController *) selectedViewController
{
  return _selectedViewController;
}

// Handle page transitions
- (void) navigateForwardToObject: (id)object
{
  NSInteger index = [_arrangedObjects indexOfObject: object];
  [self setSelectedIndex: index];
}

- (void) completeTransition
{
  [self setView: [_selectedViewController view]];
}

- (IBAction) navigateBack: (id)sender
{
  NSInteger idx = [self selectedIndex] - 1;
  [self setSelectedIndex: (idx >= 0) ? idx : 0];
}

- (IBAction) navigateForward: (id)sender
{
  NSInteger idx = [self selectedIndex] + 1;
  NSInteger lastPage = ([_arrangedObjects count] - 1);
  [self setSelectedIndex: (idx > lastPage) ? lastPage : idx];
}

- (IBAction) takeSelectedIndexFrom: (id)sender // uses integerValue from sender
{
  if ([sender respondsToSelector: @selector(integerValue)])
    {
      NSInteger i = [sender integerValue];
      [self setSelectedIndex: i];
    }
}
@end

