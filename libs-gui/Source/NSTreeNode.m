/*
   NSTreeNode.m

   The tree node class

   Copyright (C) 2013 Free Software Foundation, Inc.

   Author:  Fred Kiefer <fredkiefer@gmx.de>
   Date: 2013

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

#import <AppKit/NSTreeNode.h>

@interface NSTreeNode (Private)
- (NSMutableArray*) _childNodes;
- (void) _setParentNode: (NSTreeNode*)parentNode;
@end

@implementation NSTreeNode (Private)
- (NSMutableArray*) _childNodes
{
  return _childNodes;
}

- (void) _setParentNode: (NSTreeNode*)parentNode
{
  _parentNode = parentNode;
}

@end


@interface GSTreeNodeArray : NSMutableArray
{
  NSMutableArray *array;
  NSTreeNode *parent;
}

@end

@implementation GSTreeNodeArray

- (id) initForTreeNode: (NSTreeNode*)node
{
  ASSIGN(parent, node);
  array = [parent _childNodes];
  return self;
}

- (void) dealloc
{
  RELEASE(parent);
  [super dealloc];
}

- (NSUInteger) count
{
  return [array count];
}

- (id) objectAtIndex: (NSUInteger)index
{
  return [array objectAtIndex: index];
}

- (void) addObject: (id)anObject
{
  [array addObject: anObject];
  [(NSTreeNode*)anObject _setParentNode: parent];
}

- (void) replaceObjectAtIndex: (NSUInteger)index withObject: (id)anObject
{
  id old = [array objectAtIndex: index];

  [(NSTreeNode*)old _setParentNode: nil];
  [array replaceObjectAtIndex: index withObject: anObject];
  [(NSTreeNode*)anObject _setParentNode: parent];
}

- (void) insertObject: anObject atIndex: (NSUInteger)index
{
  [array insertObject: anObject atIndex: index];
  [(NSTreeNode*)anObject _setParentNode: parent];
}

- (void) removeObjectAtIndex: (NSUInteger)index
{
  id old = [array objectAtIndex: index];

  [(NSTreeNode*)old _setParentNode: nil];
  [array removeObjectAtIndex: index];
}

@end


@implementation NSTreeNode

+ (id) treeNodeWithRepresentedObject: (id)modelObject
{
  NSTreeNode *node = [[NSTreeNode alloc] initWithRepresentedObject: modelObject];

  return AUTORELEASE(node);
}

- (NSArray*) childNodes
{
  return [NSArray arrayWithArray: _childNodes];
}

- (NSTreeNode*) descendantNodeAtIndexPath: (NSIndexPath*)path
{
  NSUInteger len = [path length];
  NSUInteger i;
  NSTreeNode *node = self;

  for (i = 0; i < len; i++)
    {
      NSUInteger index = [path indexAtPosition: i];

      node = [node->_childNodes objectAtIndex: index];
      if (node == nil)
        {
          return nil;
        }
    }

  return node;
}

- (NSIndexPath*) indexPath
{
  if (_parentNode != nil)
    {
      NSIndexPath *path;
      NSUInteger index;

      index = [_parentNode->_childNodes indexOfObject: self];
      path = [_parentNode indexPath];
      if (path != nil)
        {
          return [path indexPathByAddingIndex: index];
        }
      else
        {
          return [NSIndexPath indexPathWithIndex: index];
        }
    }
  else
    {
      return nil;
    }
}

- (id) initWithRepresentedObject: (id)repObj
{
  ASSIGN(_representedObject, repObj);
  _childNodes = [[NSMutableArray alloc] init];
  return self;
}

- (void) dealloc
{
  RELEASE(_representedObject);
  RELEASE(_childNodes);
  [super dealloc];
}

- (BOOL) isLeaf
{
  return [_childNodes count] == 0;
}

- (NSMutableArray*) mutableChildNodes
{
  GSTreeNodeArray *nodeArray = [[GSTreeNodeArray alloc] initForTreeNode: self];

  return AUTORELEASE(nodeArray);
}

- (NSTreeNode*) parentNode
{
  return _parentNode;
}

- (id) representedObject
{
  return _representedObject;
}

- (void) sortWithSortDescriptors: (NSArray*)sortDescs recursively: (BOOL)flag
{
  // Sort children nodes
  NSUInteger i;
  NSUInteger len = [sortDescs count];
  NSMutableArray *newSortDescs = [[NSMutableArray alloc] init];

  for (i = 0; i < len; i++)
    {
      NSSortDescriptor *oldDesc = [sortDescs objectAtIndex: i];
      NSString * newKey = [@"representedObject." stringByAppendingString: [oldDesc key]];
      NSSortDescriptor *newDesc = [[NSSortDescriptor alloc]
                                    initWithKey: newKey
                                   ascending: [oldDesc ascending]
                                    selector: [oldDesc selector]];
      
      [newSortDescs addObject: newDesc];
      RELEASE(newDesc);
    }
  
  [_childNodes sortUsingDescriptors: newSortDescs];
  RELEASE(newSortDescs);

  if (flag)
    {
      // sort recursive
      NSUInteger count = [_childNodes count];

      for (i = 0; i < count; i++)
        {
          [[_childNodes objectAtIndex: i] sortWithSortDescriptors: sortDescs
                                                      recursively: YES];
        }
    }
}

@end
