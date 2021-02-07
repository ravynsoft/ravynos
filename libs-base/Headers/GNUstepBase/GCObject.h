/** Interface for simple garbage collected classes

   Copyright (C) 2002 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   Inspired by gc classes of  Ovidiu Predescu and Mircea Oancea

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

   AutogsdocSource: Additions/GCObject.m
   AutogsdocSource: Additions/GCArray.m
   AutogsdocSource: Additions/GCDictionary.m

*/

#ifndef __INCLUDED_GCOBJECT_H
#define __INCLUDED_GCOBJECT_H

#ifndef NeXT_Foundation_LIBRARY
#import <Foundation/NSObject.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSMapTable.h>
#else
#import <Foundation/Foundation.h>
#endif

#if	defined(__cplusplus)
extern "C" {
#endif


@class	GCObject;

typedef struct {
  GCObject	*next;
  GCObject	*previous;
  struct {
    unsigned visited:1;
    unsigned refCount:31;
  } flags;
} gcInfo;

GS_EXPORT_CLASS
@interface GCObject : NSObject
{
  gcInfo	gc;
}
  
+ (void) gcCollectGarbage;
+ (BOOL) gcIsCollecting;
+ (void) gcObjectWillBeDeallocated: (GCObject*)anObject;

- (void) gcDecrementRefCount;
- (void) gcDecrementRefCountOfContainedObjects;
- (void) gcIncrementRefCount;
- (BOOL) gcIncrementRefCountOfContainedObjects;

@end

@interface GCObject (Extra)
- (GCObject*) gcNextObject;
- (GCObject*) gcPreviousObject;
- (GCObject*) gcSetNextObject: (GCObject*)anObject;
- (GCObject*) gcSetPreviousObject: (GCObject*)anObject;
- (BOOL) gcAlreadyVisited;
- (void) gcSetVisited: (BOOL)flag;
@end

GS_EXPORT_CLASS
@interface GCArray : NSArray
{
  gcInfo	gc;
  id		*_contents;	// C array of content objects
  BOOL		*_isGCObject;	// Is content object collectable?
  unsigned int	_count;		// Number of content objects.
}
@end

GS_EXPORT_CLASS
@interface GCMutableArray : NSMutableArray
{
  gcInfo	gc;
  id		*_contents;
  BOOL		*_isGCObject;
  unsigned	_count;
  unsigned	_maxCount;	// Maximum number of content objects.
}
@end

GS_EXPORT_CLASS
@interface GCDictionary : NSDictionary
{
  gcInfo	gc;
  NSMapTable	*_map;
}
@end

GS_EXPORT_CLASS
@interface GCMutableDictionary : NSMutableDictionary
{
  gcInfo	gc;
  NSMapTable	*_map;
}
@end

#if	defined(__cplusplus)
}
#endif

#endif /* __INCLUDED_GCOBJECT_H */
