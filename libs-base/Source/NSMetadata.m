/**Interface for NSMetadataQuery for GNUStep
   Copyright (C) 2012 Free Software Foundation, Inc.

   Written by: Gregory Casamento
   Date: 2012
   
   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.

   AutogsdocSource: NSMetadata.m
*/ 

#import "common.h"

#define EXPOSE_NSMetadataItem_IVARS 1
#define EXPOSE_NSMetadataQuery_IVARS 1
#define EXPOSE_NSMetadataQueryAttributeValueTuple_IVARS 1
#define EXPOSE_NSMetadataQueryResultGroupInternal_IVARS 1
#define EXPOSE_NSMetadataQueryResultGroup_IVARS 1

#import "Foundation/NSMetadata.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSPredicate.h"
#import "Foundation/NSString.h"
#import "Foundation/NSTimer.h"

@implementation NSMetadataItem

#define	myAttributes	((NSMutableDictionary*)_NSMetadataItemInternal)

- (NSArray *) attributes
{
  return [myAttributes allKeys];
}

- (void) dealloc
{
  [myAttributes release];
  [super dealloc];
}

- (id) init
{
  if (nil != (self = [super init]))
    {
      _NSMetadataItemInternal = (void*)[NSMutableDictionary new];
    }
  return self;
}

- (id) valueForAttribute: (NSString *)key
{
  return [myAttributes objectForKey: key];
}

- (NSDictionary *) valuesForAttributes: (NSArray *)keys
{
  NSMutableDictionary	*results = [NSMutableDictionary dictionary];
  NSEnumerator		*en = [keys objectEnumerator];
  id			key = nil;

  while ((key = [en nextObject]) != nil)
    {
      id value = [self valueForAttribute: key];

      [results setObject: value forKey: key];
    }

  return results;
}

@end

// Metdata Query Constants...
NSString * const NSMetadataQueryUserHomeScope
  = @"NSMetadataQueryUserHomeScope";
NSString * const NSMetadataQueryLocalComputerScope
  = @"NSMetadataQueryLocalComputerScope";
NSString * const NSMetadataQueryNetworkScope
  = @"NSMetadataQueryNetworkScope";
NSString * const NSMetadataQueryUbiquitousDocumentsScope
  = @"NSMetadataQueryUbiquitousDocumentsScope";
NSString * const NSMetadataQueryUbiquitousDataScope
  = @"NSMetadataQueryUbiquitousDataScope";

NSString * const NSMetadataQueryDidFinishGatheringNotification
  = @"NSMetadataQueryDidFinishGatheringNotification";
NSString * const NSMetadataQueryDidStartGatheringNotification
  = @"NSMetadataQueryDidStartGatheringNotification";
NSString * const NSMetadataQueryDidUpdateNotification
  = @"NSMetadataQueryDidUpdateNotification";
NSString * const NSMetadataQueryGatheringProgressNotification
  = @"NSMetadataQueryGatheringProgressNotification";

@interface	NSMetadataQueryInternal : NSObject
{
@public
  BOOL _isStopped;
  BOOL _isGathering;
  BOOL _isStarted;

  NSArray *_searchURLs;
  NSArray *_scopes;
  NSArray *_sortDescriptors;
  NSPredicate *_predicate;
  NSArray *_groupingAttributes;
  NSArray *_valueListAttributes;

  NSTimeInterval _notificationBatchingInterval;
  NSMutableDictionary *_results;

  id<NSMetadataQueryDelegate> _delegate;
}
@end
@implementation	NSMetadataQueryInternal
@end

#ifdef	this
#undef	this
#endif
#define	this	((NSMetadataQueryInternal*)_NSMetadataQueryInternal)

@implementation NSMetadataQuery

- (void) dealloc
{
  [this release];
  [super dealloc];
}

- (id<NSMetadataQueryDelegate>) delegate;
{
  return this->_delegate;
}

- (void) disableUpdates
{
  [self subclassResponsibility: _cmd];
}

- (void) enableUpdates
{
  [self subclassResponsibility: _cmd];
}

- (NSArray *) groupedResults;
{
  return [self subclassResponsibility: _cmd];
}

- (NSArray *) groupingAttributes
{
  return [self subclassResponsibility: _cmd];
}

- (NSUInteger) indexOfResult: (id)result
{
  [self subclassResponsibility: _cmd];
  return NSNotFound;
}

- (id) init
{
  if ((self = [super init]) != nil)
    {
      _NSMetadataQueryInternal = (void*)[NSMetadataQueryInternal new];
      this->_isStopped = YES;
      this->_isGathering = NO;
      this->_isStarted = NO;
      this->_notificationBatchingInterval = (NSTimeInterval)0.0;
    }
  return self;
}

- (BOOL) isGathering
{
  return this->_isGathering;
}

- (BOOL) isStarted
{
  return this->_isStarted;
}

- (BOOL) isStopped
{
  return this->_isStopped;
}

- (NSTimeInterval) notificationBatchingInterval
{
  [self subclassResponsibility: _cmd];
  return (NSTimeInterval)0;
}

- (NSPredicate *) predicate
{
  return this->_predicate;
}

- (id) resultAtIndex: (NSUInteger)index
{
  return [self subclassResponsibility: _cmd];
}

- (NSUInteger) resultCount
{
  [self subclassResponsibility: _cmd];
  return 0;
}

- (NSArray *) results
{
  return [self subclassResponsibility: _cmd];
}

- (NSArray *) searchItemURLs
{
  return this->_searchURLs;
}

- (NSArray *) searchScopes
{
  return this->_scopes;
}

- (void) setDelegate: (id<NSMetadataQueryDelegate>)delegate;
{
  this->_delegate = delegate;
}

- (void) setGroupingAttributes: (NSArray *)attrs
{
  [self subclassResponsibility: _cmd];
}

- (void) setNotificationBatchingInterval: (NSTimeInterval)interval
{
  [self subclassResponsibility: _cmd];
}

- (void) setPredicate: (NSPredicate *)predicate
{
  ASSIGNCOPY(this->_predicate, predicate);
}

- (void) setSearchItemURLs: (NSArray *)urls
{
  ASSIGNCOPY(this->_searchURLs, urls); 
}

- (void) setSearchScopes: (NSArray *)scopes
{
  ASSIGNCOPY(this->_scopes, scopes);
}

- (void) setSortDescriptors: (NSArray *)descriptors
{
  ASSIGNCOPY(this->_sortDescriptors, descriptors);
}

- (void) setValueListAttributes: (NSArray *)attrs
{
  [self subclassResponsibility: _cmd];
}

- (NSArray *) sortDescriptors
{
  return this->_sortDescriptors;
}

- (BOOL) startQuery
{
  [self subclassResponsibility: _cmd];
  return NO;
}

- (void) stopQuery
{
  [self subclassResponsibility: _cmd];
}

- (id) valueOfAttribute: (id)attr forResultAtIndex: (NSUInteger)index
{
  return [self subclassResponsibility: _cmd];
}

- (NSDictionary *) valueLists
{
  return [self subclassResponsibility: _cmd];
}

- (NSArray *) valueListAttributes
{
  return [self subclassResponsibility: _cmd];
}

@end

@interface NSMetadataQueryAttributeValueTupleInternal : NSObject
{
  @public
  id         _attribute;
  id         _value;
  NSUInteger _count;
}
@end
@implementation NSMetadataQueryAttributeValueTupleInternal
@end

#ifdef	this
#undef	this
#endif
#define	this	((NSMetadataQueryAttributeValueTupleInternal*)\
_NSMetadataQueryAttributeValueTupleInternal)

@implementation NSMetadataQueryAttributeValueTuple

- (NSString *) attribute
{
  return this->_attribute;
}

- (NSUInteger) count
{
  return this->_count;
}

- (void) dealloc
{
  [this release];
  [super dealloc];
}

- (id) init
{
  if (nil != (self = [super init]))
    {
      _NSMetadataQueryAttributeValueTupleInternal =
	(void*)[NSMetadataQueryAttributeValueTupleInternal new];
    }
  return self;
}

- (id) value
{
  return this->_value;
}

@end


@interface NSMetadataQueryResultGroupInternal : NSObject
{
  @public
  id         	_attribute;
  id         	_value;
  NSMutableArray *_subgroups;
}
@end
@implementation NSMetadataQueryResultGroupInternal
@end

#ifdef	this
#undef	this
#endif
#define	this	((NSMetadataQueryResultGroupInternal*)\
_NSMetadataQueryResultGroupInternal)

@implementation NSMetadataQueryResultGroup : NSObject

- (NSString *) attribute
{
  return this->_attribute;
}

- (void) dealloc
{
  [this release];
  [super dealloc];
}

- (id) init
{
  if (nil != (self = [super init]))
    {
      _NSMetadataQueryResultGroupInternal =
	(void*)[NSMetadataQueryResultGroupInternal new];
    }
  return self;
}

- (id) resultAtIndex: (NSUInteger)index
{
  return [this->_subgroups objectAtIndex:index];
}

- (NSUInteger) resultCount
{
  return [this->_subgroups count];
}

- (NSArray *) results
{
  return [self subgroups];
}

- (NSArray *) subgroups
{
  return this->_subgroups;
}

- (id) value
{
  return this->_value;
}

@end
