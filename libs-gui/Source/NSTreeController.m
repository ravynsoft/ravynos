/* 
   NSTreeController.h

   The tree controller class.

   Copyright (C) 2012 Free Software Foundation, Inc.

   Author:  Gregory Casamento <greg.casamento@gmail.com>
   Date: 2012
   
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
/* 
   NSTreeController.h

   The tree controller class.

   Copyright (C) 2012 Free Software Foundation, Inc.

   Author:  Gregory Casamento <greg.casamento@gmail.com>
   Date: 2012
   
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
#import <Foundation/NSIndexPath.h>
#import <Foundation/NSString.h>
#import <Foundation/NSSortDescriptor.h>

#import <AppKit/NSTreeController.h>
#import <AppKit/NSTreeNode.h>

@implementation NSTreeController

- (id) initWithContent: (id)content
{
  if ((self = [super initWithContent: content]) != nil)
    {
    }
  
  return self;
}

- (void) dealloc
{
  RELEASE(_childrenKeyPath);
  RELEASE(_countKeyPath);
  RELEASE(_leafKeyPath);
  RELEASE(_sortDescriptors);
  [super dealloc];
}

- (BOOL) addSelectionIndexPaths: (NSArray*)indexPaths
{
  // FIXME
  return NO;
}

- (BOOL) alwaysUsesMultipleValuesMarker
{
  return _alwaysUsesMultipleValuesMarker;
}

- (BOOL) avoidsEmptySelection
{
  return _avoidsEmptySelection;
}

- (BOOL) canAddChid
{
  // FIXME
  return NO;
}

- (BOOL) canInsert
{
  // FIXME
  return NO;
}

- (BOOL) canInsertChild
{
  // FIXME
  return NO;
}

- (BOOL) preservesSelection
{
  return _preservesSelection;
}

- (BOOL) selectsInsertedObjects
{
  return _selectsInsertedObjects;
}

- (BOOL) setSelectionIndexPath: (NSIndexPath*)indexPath
{
  // FIXME
  return NO;
}

- (BOOL) setSelectionIndexPaths: (NSArray*)indexPaths
{
  // FIXME
  return NO;
}

- (id) arrangedObjects
{
  // FIXME
  return nil;
}

- (id) content
{
  // FIXME
  return [super content];
}

- (NSArray*) selectedObjects
{
  // FIXME
  return [super selectedObjects];
}

- (NSIndexPath*) selectionIndexPath
{
  // FIXME
  return nil;
}

- (NSArray*) selectionIndexPaths
{
  // FIXME
  return nil;
}

- (NSArray*) sortDescriptors
{
  return _sortDescriptors;
}

- (NSString*) childrenKeyPath
{
  return _childrenKeyPath;
}

- (NSString*) countKeyPath
{
  return _countKeyPath;;
}

- (NSString*) leafKeyPath
{
  return _leafKeyPath;
}

- (void) addChild: (id)sender
{
  // FIXME
}

- (void) add: (id)sender
{
  // FIXME
  [super add: sender];
}

- (void) insertChild: (id)sender
{
  // FIXME
}

- (void) insertObject: (id)object atArrangedObjectIndexPath: (NSIndexPath*)indexPath
{
  // FIXME
}

- (void) insertObjects: (NSArray*)objects atArrangedObjectIndexPaths: (NSArray*)indexPaths
{
  // FIXME
}

- (void) insert: (id)sender
{
  // FIXME
}

- (void) rearrangeObjects
{
  // FIXME
}

- (void) removeObjectAtArrangedObjectIndexPath: (NSIndexPath*)indexPath
{
  // FIXME
}

- (void) removeObjectsAtArrangedObjectIndexPaths: (NSArray*)indexPaths
{
  // FIXME
}

- (void) removeSelectionIndexPaths: (NSArray*)indexPaths
{
  // FIXME
}

- (void) remove: (id)sender
{
  // FIXME
  [super remove: sender];
}

- (void) setAlwaysUsesMultipleValuesMarker: (BOOL)flag
{
  _alwaysUsesMultipleValuesMarker = flag;
}

- (void) setAvoidsEmptySelection: (BOOL)flag
{
  _avoidsEmptySelection = flag;
}

- (void) setChildrenKeyPath: (NSString*)path
{
  ASSIGN(_childrenKeyPath, path);
}

- (void) setContent: (id)content
{
  // FIXME
  [super setContent: content];
}

- (void) setCountKeyPath: (NSString*)path
{
  ASSIGN(_countKeyPath, path);
}

- (void) setLeafPathKey: (NSString*)key
{
  ASSIGN(_leafKeyPath, key);
}

- (void) setPreservesSelection: (BOOL)flag
{
  _preservesSelection = flag;
}

- (void) setSelectsInsertedObjects: (BOOL)flag
{
  _selectsInsertedObjects = flag;
}

- (void) setSortDescriptors: (NSArray*)descriptors
{
  ASSIGN(_sortDescriptors, descriptors);
}

- (NSString*) childrenKeyPathForNode: (NSTreeNode*)node
{
  // FIXME
  return nil;
}

- (NSString*) countKeyPathForNode: (NSTreeNode*)node
{
  // FIXME
  return nil;
}

- (NSString*) leafKeyPathForNode: (NSTreeNode*)node
{
  // FIXME
  return nil;
}

- (void) moveNode: (NSTreeNode*)node toIndexPath: (NSIndexPath*)indexPath
{
  // FIXME
}

- (void) moveNodes: (NSArray*)nodes toIndexPath: (NSIndexPath*)startingIndexPath
{
  // FIXME
}

- (NSArray*) selectedNodes
{
  // FIXME
  return nil;
}

- (id) initWithCoder: (NSCoder*)coder
{
  return self;
}

- (void) encodeWithCoder: (NSCoder*)coder
{
  // Do nothing...
}

- (id) copyWithZone: (NSZone*)zone
{
  return [self retain];
}

@end
