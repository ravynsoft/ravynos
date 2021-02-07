/* Implementation of class NSTextFinder
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: 02-08-2020

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
#import <Foundation/NSArchiver.h>

#import "AppKit/NSTextFinder.h"
#import "AppKit/NSTextView.h"
#import "AppKit/NSUserInterfaceValidation.h"

#import "GSTextFinder.h"

@implementation NSTextFinder

- (instancetype) init
{
  self = [super init];
  if (self != nil)
    {
      // initialize finder...
      _finder = [[GSTextFinder alloc] init];
    }
  return self;
}

// Validating and performing
- (void) performFindPanelAction: (id)sender
{
  [self performAction: [sender tag]];
}

- (void) performTextFinderAction: (id)sender
{
  [self performFindPanelAction: sender];
}

- (NSInteger) tag
{
  return _tag;
}

- (void) _mapOpToTag: (NSTextFinderAction)op
{
    switch (op)
    {
    case NSTextFinderActionShowFindInterface:
      _tag = NSFindPanelActionShowFindPanel;
      break;
    case NSTextFinderActionNextMatch:
      _tag = NSFindPanelActionNext;
      break;
    case NSTextFinderActionPreviousMatch:
      _tag = NSFindPanelActionPrevious;
      break;
    case NSTextFinderActionReplaceAll:
      _tag = NSFindPanelActionReplaceAll;
      break;
    case NSTextFinderActionReplace:
      _tag = NSFindPanelActionReplace;
      break;
    case NSTextFinderActionReplaceAndFind:
      _tag = NSFindPanelActionReplaceAndFind;
      break;
    case NSTextFinderActionSetSearchString:
      _tag = NSFindPanelActionSetFindString;
      break;
    case NSTextFinderActionReplaceAllInSelection:
      _tag = NSFindPanelActionReplaceAllInSelection;
      break;
    case NSTextFinderActionSelectAll:
      _tag = NSFindPanelActionSelectAll;
      break;
    case NSTextFinderActionSelectAllInSelection:
      _tag = NSFindPanelActionSelectAllInSelection;
      break;
    case NSTextFinderActionHideFindInterface:
      // unsupported;
      break;
    case NSTextFinderActionShowReplaceInterface:
      // unsupported;
      break;
    case NSTextFinderActionHideReplaceInterface:
      // unsupported;
      break;
    default:
      NSLog(@"Unknown operation: %ld", op);
      break;
    }
}

- (void) performAction: (NSTextFinderAction)op
{
  [self _mapOpToTag: op];
  [_finder performFindPanelAction: self];
}

- (BOOL) validateUserInterfaceAction: (id<NSValidatedUserInterfaceItem>)item
{
  SEL action = [item action];
  if (sel_isEqual(action, @selector(performTextFinderAction:)) ||
      sel_isEqual(action, @selector(performFindPanelAction:)))
    {
      return [self validateAction: [item tag]];
    }

  return YES;
}

- (BOOL) validateAction: (NSTextFinderAction)op
{
  [self _mapOpToTag: op];
  return [_finder validateFindPanelAction: self
                             withTextView: nil];
}

- (void)cancelFindIndicator;
{
}

// Properties
- (id<NSTextFinderClient>) client
{
  return _client;
}

- (void) setClient: (id<NSTextFinderClient>) client
{
  _client = client;
}

- (id<NSTextFinderBarContainer>) findBarContainer
{
  return _findBarContainer;
}

- (void) setFindBarContainer: (id<NSTextFinderBarContainer>) findBarContainer
{
  _findBarContainer = findBarContainer;
}

- (BOOL) findIndicatorNeedsUpdate
{
  return _findIndicatorNeedsUpdate;
}

- (void) setFindIndicatorNeedsUpdate: (BOOL)flag
{
  _findIndicatorNeedsUpdate = flag;
}

- (BOOL) isIncrementalSearchingEnabled
{
  return _incrementalSearchingEnabled;
}

- (void) setIncrementalSearchingEnabled: (BOOL)flag
{
  _incrementalSearchingEnabled = flag;
}

- (BOOL) incrementalSearchingShouldDimContentView
{
  return _incrementalSearchingShouldDimContentView;
}

- (void) setIncrementalSearchingShouldDimContentView: (BOOL)flag
{
  _incrementalSearchingShouldDimContentView = flag;
}

- (NSArray *) incrementalMatchRanges
{
  return _incrementalMatchRanges;
}

+ (void) drawIncrementalMatchHighlightInRect: (NSRect)rect
{
}

- (void) noteClientStringWillChange
{
  // nothing...
}

// NSCoding...
- (instancetype) initWithCoder: (NSCoder *)coder
{
  self = [super init];
  if (self != nil)
    {
      if ([coder allowsKeyedCoding])
        {
          if ([coder containsValueForKey: @"NSFindIndicatorNeedsUpdate"])
            {
              _findIndicatorNeedsUpdate = [coder decodeBoolForKey: @"NSFindIndicatorNeedsUpdate"];
            }
          if ([coder containsValueForKey: @"NSIncrementalSearchingEnabled"])
            {
              _incrementalSearchingEnabled = [coder decodeBoolForKey: @"NSIncrementalSearchingEnabled"];
            }
          if ([coder containsValueForKey: @"NSIncrementalSearchingShouldDimContentView"])
            {
              _incrementalSearchingShouldDimContentView = [coder decodeBoolForKey: @"NSIncrementalSearchingShouldDimContentView"];
            }
          if ([coder containsValueForKey: @"NSIncrementalMatchRanges"])
            {
              ASSIGN(_incrementalMatchRanges, [coder decodeObjectForKey: @"NSIncrementalMatchRanges"]);
            }

          // initialize finder...
          _finder = [GSTextFinder sharedTextFinder];
        }
      else
        {
          [coder decodeValueOfObjCType: @encode(BOOL)
                                    at: &_findIndicatorNeedsUpdate];
          [coder decodeValueOfObjCType: @encode(BOOL)
                                    at: &_incrementalSearchingEnabled];
          [coder decodeValueOfObjCType: @encode(BOOL)
                                    at: &_incrementalSearchingShouldDimContentView];
          ASSIGN(_incrementalMatchRanges, [coder decodeObject]);
        }
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  if ([coder allowsKeyedCoding])
    {
      [coder encodeBool: _findIndicatorNeedsUpdate
                 forKey: @"NSFindIndicatorNeedsUpdate"];
      [coder encodeBool: _incrementalSearchingEnabled
                 forKey: @"NSIncrementalSearchingEnabled"];
      [coder encodeBool: _incrementalSearchingShouldDimContentView
                 forKey: @"NSIncrementalSearchingShouldDimContentView"];
      [coder encodeObject: _incrementalMatchRanges
                   forKey: @"NSIncrementalMatchRanges"];
    }
  else
    {
      [coder encodeValueOfObjCType: @encode(BOOL)
                                at: &_findIndicatorNeedsUpdate];
      [coder encodeValueOfObjCType: @encode(BOOL)
                                at: &_incrementalSearchingEnabled];
      [coder encodeValueOfObjCType: @encode(BOOL)
                                at: &_incrementalSearchingShouldDimContentView];
      [coder encodeObject: _incrementalMatchRanges];
    }
}

@end
