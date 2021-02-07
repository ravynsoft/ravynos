/* Implementation of class NSPathCell
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Wed Apr 22 18:19:07 EDT 2020

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

#import "AppKit/NSPathCell.h"
#import "AppKit/NSWorkspace.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSPathComponentCell.h"
#import "GSGuiPrivate.h"

static Class pathComponentCellClass;

@interface NSPathCell (Private)
+ (NSArray *) _generateCellsForURL: (NSURL *)url;
@end

@interface NSPathComponentCell (Private)
- (void) _setLastComponent: (BOOL)f;
@end

@implementation NSPathCell

+ (void) initialize
{
  if (self == [NSPathCell class])
    {
      [self setVersion: 1.0];
      [self setPathComponentCellClass: [NSPathComponentCell class]];
    }
}

- (void) dealloc
{
  RELEASE(_backgroundColor);
  RELEASE(_placeholderAttributedString);
  RELEASE(_allowedTypes);
  RELEASE(_url);
  RELEASE(_pathComponentCells);
  [super dealloc];
}

- (void)mouseEntered:(NSEvent *)event 
           withFrame:(NSRect)frame 
              inView:(NSView *)view
{
  // This is the method where the expansion of the cell happens
  // if the path component cells are shortened.
  // This is currently not implemented.
}

- (void)mouseExited:(NSEvent *)event 
          withFrame:(NSRect)frame 
             inView:(NSView *)view
{
  // This is the method where the contraction of the cell happens
  // if the path component cells are shortened.
  // This is currently not implemented.
}

- (void) setAllowedTypes: (NSArray *)types
{
  ASSIGNCOPY(_allowedTypes, types);
}

- (NSArray *) allowedTypes
{
  return _allowedTypes;
}

- (NSPathStyle) pathStyle
{
  return _pathStyle;
}

- (void) setPathStyle: (NSPathStyle)pathStyle
{
  _pathStyle = pathStyle;
}

- (NSAttributedString *) placeholderAttributedString
{
  return _placeholderAttributedString; 
}

- (void) setPlaceholderAttributedString: (NSAttributedString *)string
{
  ASSIGNCOPY(_placeholderAttributedString, string);
}

- (NSString *) placeholderString
{
  return [_placeholderAttributedString string];
}

- (void) setPlaceholderString: (NSString *)string
{
  NSAttributedString *as = [[NSAttributedString alloc] initWithString: string];
  [self setPlaceholderAttributedString: as];
  RELEASE(as);
}

- (NSColor *) backgroundColor
{
  return _backgroundColor;
}

- (void) setBackgroundColor: (NSColor *)color
{
  ASSIGNCOPY(_backgroundColor, color);
}

+ (Class) pathComponentCellClass
{
  return pathComponentCellClass;
}

+ (void) setPathComponentCellClass: (Class)clz
{
  pathComponentCellClass = clz;
}

- (NSRect)rectOfPathComponentCell:(NSPathComponentCell *)cell 
                        withFrame:(NSRect)frame 
                           inView:(NSView *)view
{
  NSUInteger index = [_pathComponentCells indexOfObject: cell];
  CGFloat cellWidth = (frame.size.width / (CGFloat)[_pathComponentCells count]);
  return NSMakeRect(frame.origin.x + (cellWidth * (CGFloat)index),
                    frame.origin.y,
                    cellWidth,
                    frame.size.height);
}

- (NSPathComponentCell *)pathComponentCellAtPoint:(NSPoint)point 
                                        withFrame:(NSRect)frame 
                                           inView:(NSView *)view
{
  NSUInteger c = [_pathComponentCells count];
  NSUInteger woc = frame.size.width / c;
  NSUInteger item = (NSUInteger)point.x / woc;

  return [_pathComponentCells objectAtIndex: item];
}

- (NSPathComponentCell *) clickedPathComponentCell
{
  return _clickedPathComponentCell;
}

- (NSArray *) pathComponentCells
{
  return _pathComponentCells;
}

- (void) setPathComponentCells: (NSArray *)cells
{
  ASSIGNCOPY(_pathComponentCells, cells);
}

- (SEL) doubleAction
{
  return _doubleAction;
}

- (void) setDoubleAction: (SEL)action
{
  _doubleAction = action;
}

- (id) objectValue
{
  return _objectValue;
}

- (void) setObjectValue: (id)obj
{
  ASSIGN(_objectValue, obj);
  [self setPathComponentCells:
          [NSPathCell _generateCellsForURL: (NSURL *)_objectValue]];
}

- (NSURL *) URL
{
  return [self objectValue];
}

- (void) setURL: (NSURL *)url
{
  [self setObjectValue: url];
}

- (id<NSPathCellDelegate>) delegate
{
  return _delegate;
}

- (void) setDelegate: (id<NSPathCellDelegate>)delegate
{
  _delegate = delegate;
}

- (void) drawInteriorWithFrame: (NSRect)frame inView: (NSView *)controlView
{
  NSEnumerator *en = [_pathComponentCells objectEnumerator];
  NSPathComponentCell *cell = nil;

  [super drawInteriorWithFrame: frame
                        inView: controlView];
  
  while ((cell = (NSPathComponentCell *)[en nextObject]) != nil)
    {
      NSRect f = [self rectOfPathComponentCell: cell
                                     withFrame: frame
                                        inView: controlView]; 
      [cell drawInteriorWithFrame: f
                           inView: controlView];
    }
}

- (id) initWithCoder: (NSCoder *)coder
{
  self = [super initWithCoder: coder];
  if ([coder allowsKeyedCoding])
    {
      [self setPathStyle: NSPathStyleStandard];
      
      if ([coder containsValueForKey: @"NSPathStyle"]) 
        {
          [self setPathStyle: [coder decodeIntegerForKey: @"NSPathStyle"]];
        }
    }
  else
    {
      decode_NSUInteger(coder, &_pathStyle);
      [self setPathComponentCells: [coder decodeObject]];
    }

  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  [super encodeWithCoder: coder];
  if ([coder allowsKeyedCoding])
    {
      [coder encodeInteger: [self pathStyle]
                    forKey: @"NSPathStyle"];
      [coder encodeObject: [self pathComponentCells]
                   forKey: @"NSPathComponentCells"];
    }
  else
    {
      encode_NSUInteger(coder, &_pathStyle);
      [coder encodeObject: [self pathComponentCells]];
    }
}

@end

@implementation NSPathCell (Private)

// Private...
+ (NSArray *) _generateCellsForURL: (NSURL *)url
{
  NSMutableArray *array = [NSMutableArray arrayWithCapacity: 10];
  
  // Create cells
  if (url != nil)
    {
      BOOL isDir = NO;
      BOOL at_root = NO;
      NSFileManager *fm = [NSFileManager defaultManager];
      NSURL *u = url;
      
      // Decompose string...
      while (at_root == NO)
        {
          NSPathComponentCell *cell = [[NSPathComponentCell alloc] init];
          NSImage *image = nil;
          NSString *string = [u path];
          
          [cell setURL: u];
          [fm fileExistsAtPath: string
               isDirectory: &isDir];

          if ([string isEqualToString: @"/"])
            {
              at_root = YES;
            }

          if (isDir && at_root == NO)
            {
              image = [NSImage imageNamed: @"NSFolder"];
            }
          else if (isDir == YES && at_root == YES)
            {
              image = [NSImage imageNamed: @"NSComputer"];
            }
          else
            {
              image = [[NSWorkspace sharedWorkspace] iconForFile: [[url path] lastPathComponent]];
            }

          [cell setImage: image];

          if ([array count] == 0) // the element we are adding is the last component that will show
            {
              [cell _setLastComponent: YES];
            }
          else
            {
              [cell _setLastComponent: NO];
            }
          
          [array insertObject: cell
                      atIndex: 0];
          RELEASE(cell);
          string = [string stringByDeletingLastPathComponent];
          u = [NSURL URLWithString: string
                     relativeToURL: nil];
          if (u == nil && at_root == NO)
            {
              // Because when we remove the last path component
              // all that is left is a blank... so we add the "/" so that
              // it is shown.
              u = [NSURL URLWithString: @"/"];
            }
        }
    }
  
  return [array copy];
}

@end

@implementation NSPathComponentCell (Private)

- (void) _setLastComponent: (BOOL)f
{
  _lastComponent = f;
}

@end
