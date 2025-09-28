/* Copyright (c) 2008 Dan Knapp

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreData/NSFetchRequest.h>
#import <CoreData/NSManagedObject.h>
#import <CoreData/NSManagedObjectContext.h>
#import <CoreData/NSEntityDescription.h>
#import <Foundation/NSRaise.h>

@implementation NSFetchRequest

-init {
   _entity=nil;
   _predicate=nil;
   _sortDescriptors=nil;
   _affectedStores=nil;
   _fetchLimit=0;
   return self;
}

-initWithCoder: (NSCoder *) coder {
   NSUnimplementedMethod();
   return nil;
}

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-copyWithZone:(NSZone *)zone {
   NSFetchRequest *result=NSCopyObject(self,0,zone);
   
   result->_entity=[_entity retain];
   result->_predicate=[_predicate copy];
   result->_sortDescriptors=[_sortDescriptors copy];
   result->_affectedStores=[_affectedStores copy];
   result->_propertiesToFetch=[_propertiesToFetch copy];
   result->_relationshipKeyPathsForPrefetching=[_relationshipKeyPathsForPrefetching copy];

   return result;
}

-(void)dealloc {
   [_entity release];
   [_predicate release];
   [_sortDescriptors release];
   [_affectedStores release];
   [super dealloc];
}

-(NSFetchRequestResultType)resultType {
   return _resultType;
}

-(NSEntityDescription *)entity {
   return _entity;
}

-(NSPredicate *)predicate {
   return _predicate;
}

-(NSArray *)sortDescriptors {
   return _sortDescriptors;
}

-(NSArray *)affectedStores {
   return _affectedStores;
}

-(NSUInteger)fetchLimit {
   return _fetchLimit;
}

-(NSUInteger)fetchBatchSize {
   return _fetchBatchSize;
}

-(NSUInteger)fetchOffset {
   return _fetchOffset;
}

-(BOOL)includesPendingChanges {
   return _includesPendingChanges;
}

-(BOOL)includesPropertyValues {
   return _includesPropertyValues;
}

-(BOOL)includesSubentities {
   return _includesSubentities;
}

-(BOOL)returnsDistinctResults {
   return _returnsDistinctResults;
}

-(BOOL)returnsObjectsAsFaults {
   return _returnsObjectsAsFaults;
}

-(NSArray *)propertiesToFetch {
   return _propertiesToFetch;
}

-(NSArray *)relationshipKeyPathsForPrefetching {
   return _relationshipKeyPathsForPrefetching;
}

-(void)setResultType:(NSFetchRequestResultType)type {
   _resultType=type;
}

-(void)setEntity:(NSEntityDescription *)value {
   value=[value retain];
   [_entity release];
   _entity=value;
}

-(void)setPredicate:(NSPredicate *)value {
   value=[value retain];
   [_predicate release];
   _predicate=value;
}

-(void)setSortDescriptors:(NSArray *)value {
   value=[value copy];
   [_sortDescriptors release];
   _sortDescriptors=value;
}

-(void)setAffectedStores:(NSArray *)value {
   value=[value copy];
   [_affectedStores release];
   _affectedStores=value;
}

-(void)setFetchLimit:(NSUInteger)value {
   _fetchLimit=value;
}

-(void)setFetchBatchSize:(NSUInteger)value {
   _fetchBatchSize=value;
}

-(void)setFetchOffset:(NSUInteger)value {
   _fetchOffset=value;
}

-(void)setIncludesPendingChanges:(BOOL)value {
   _includesPendingChanges=value;
}

-(void)setIncludesPropertyValues:(BOOL)value {
   _includesPropertyValues=value;
}

-(void)setIncludesSubentities:(BOOL)value {
   _includesSubentities=value;
}

-(void)setReturnsDistinctResults:(BOOL)value {
   _returnsDistinctResults=value;
}

-(void)setReturnsObjectsAsFaults:(BOOL)value {
   _returnsObjectsAsFaults=value;
}

-(void)setPropertiesToFetch:(NSArray *)value {
   value=[value copy];
   [_propertiesToFetch release];
   _propertiesToFetch=value;
}

-(void)setRelationshipKeyPathsForPrefetching:(NSArray *)value {
   value=[value copy];
   [_relationshipKeyPathsForPrefetching release];
   _relationshipKeyPathsForPrefetching=value;
}

@end
