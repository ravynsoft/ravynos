/** <title>NSCollectionViewItem</title>
 
   Copyright (C) 2013 Free Software Foundation, Inc.
 
   Author: Doug Simons (doug.simons@testplant.com)
           Frank LeGrand (frank.legrand@testplant.com)
   Date: February 2013
 
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

#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSKeyedArchiver.h>

#import "AppKit/NSCollectionView.h"
#import "AppKit/NSCollectionViewItem.h"
#import "AppKit/NSImageView.h"
#import "AppKit/NSKeyValueBinding.h"
#import "AppKit/NSTextField.h"

@implementation NSCollectionViewItem

- (void) awakeFromNib
{
}

- (BOOL) isSelected
{
  return _isSelected;
}

- (void) dealloc
{
  DESTROY(textField);
  DESTROY(imageView);
  [super dealloc];
}

- (NSCollectionView *) collectionView
{
  return (NSCollectionView *)[[self view] superview];
}

- (NSArray *) draggingImageComponents
{
  // FIXME: We don't have NSDraggingImageComponent
  return [NSArray array];
}

- (void) setSelected: (BOOL)flag
{
  if (_isSelected != flag)
    {
      _isSelected = flag;
    }
}

- (id) representedObject
{
  return [super representedObject];
}

- (void) setRepresentedObject: (id)anObject
{
  [super setRepresentedObject:anObject];
  //[textField setStringValue:[self representedObject]];
}

- (NSTextField *) textField
{
  return textField;
}

- (void) setTextField: (NSTextField *)aTextField
{
  ASSIGN(textField, aTextField);
}

- (NSImageView *) imageView
{
  return imageView;
}

- (void) setImageView: (NSImageView *)anImageView
{
  ASSIGN(imageView, anImageView);
}

- (id) initWithCoder: (NSCoder *)aCoder
{
  self = [super initWithCoder: aCoder];
  if (nil != self)
    {
      if (YES == [aCoder allowsKeyedCoding])
	{
          if ([aCoder containsValueForKey: @"textField"])
            {
              [self setTextField: [aCoder decodeObjectForKey: @"textField"]];
            }
          else
            {
              textField = [[NSTextField alloc] initWithFrame: NSMakeRect(0.0, 0.0, 100.0, 20.0)];
            }
          if ([aCoder containsValueForKey: @"imageView"])
            {
              [self setImageView: [aCoder decodeObjectForKey: @"imageView"]];
            }
          else
            {
              imageView = [[NSImageView alloc] initWithFrame: NSMakeRect(0.0, 0.0, 100.0, 100.0)];
            }
	}
      else
	{
	  [self setTextField: [aCoder decodeObject]];
	  [self setImageView: [aCoder decodeObject]];
	}
    }
  return self;
}

- (void)encodeWithCoder:(NSCoder *)aCoder
{
  [super encodeWithCoder: aCoder];
  if (YES == [aCoder allowsKeyedCoding])
    {
      [aCoder encodeObject: textField forKey: @"textField"];
      [aCoder encodeObject: imageView forKey: @"imageView"];
    }
  else
    {
      [aCoder encodeObject: textField];
      [aCoder encodeObject: imageView];
    }
}

- (void) copyBindingsTo: (NSCollectionViewItem*)newItem
                   from: (NSView*)view
                   onto: (NSView*)newView
{
  NSArray *exposedBindings = [view exposedBindings];
  NSEnumerator *e = [exposedBindings objectEnumerator];
  NSString *binding = nil;
  while ((binding = [e nextObject]) != nil)
    {
      NSDictionary *info = [view infoForBinding: binding];
      if (info != nil)
        {
          NSObject *target = [info objectForKey: NSObservedObjectKey];
          if (target == self)
            {
              [newView bind: binding
                   toObject: newItem
                withKeyPath: [info objectForKey: NSObservedKeyPathKey]
                    options: [info objectForKey: NSOptionsKey]];
            }
        }
    }

  NSView *sub1 = nil;
  NSEnumerator *e1 = [[view subviews] objectEnumerator];
  NSView *sub2 = nil;
  NSEnumerator *e2 = [[newView subviews] objectEnumerator];
  while ((sub1 = [e1 nextObject]) != nil)
    {
      sub2 = [e2 nextObject];
      [self copyBindingsTo: newItem from: sub1 onto: sub2];
    }
 }

- (id) copyWithZone: (NSZone *)zone 
{
  // FIXME: Cache this data, as we need a lot of copies
  NSData *itemAsData = [NSKeyedArchiver archivedDataWithRootObject: self];
  NSCollectionViewItem *newItem = 
    [NSKeyedUnarchiver unarchiveObjectWithData: itemAsData];

  // Try to copy bindings too
  [self copyBindingsTo: newItem from: [self view] onto: [newItem view]];
 
  return RETAIN(newItem);
}

@end
