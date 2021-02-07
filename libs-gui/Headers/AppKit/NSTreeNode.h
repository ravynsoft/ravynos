/*
   NSTreeNode.h

   The tree node class

   Copyright (C) 2013 Free Software Foundation, Inc.

   Author:  Dr. H. Nikolaus Schaller
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
#ifndef _GNUstep_H_NSTreeNode
#define _GNUstep_H_NSTreeNode

#import <GNUstepBase/GSVersionMacros.h>
#import <Foundation/NSObject.h>

@class NSArray;
@class NSIndexPath;
@class NSMutableArray;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
@interface NSTreeNode : NSObject
{
  id _representedObject;
  NSMutableArray *_childNodes;
  NSTreeNode *_parentNode;
}

+ (id) treeNodeWithRepresentedObject: (id)modelObject;

- (NSArray*) childNodes;
- (NSTreeNode*) descendantNodeAtIndexPath: (NSIndexPath*)path;
- (NSIndexPath*) indexPath;
- (id) initWithRepresentedObject: (id)repObj;
- (BOOL) isLeaf;
- (NSMutableArray*) mutableChildNodes;
- (NSTreeNode*) parentNode;
- (id) representedObject;
- (void) sortWithSortDescriptors: (NSArray*)sortDescs recursively: (BOOL)flag;

@end
#endif
#endif // _GNUstep_H_NSTreeNode
