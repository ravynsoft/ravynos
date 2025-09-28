/* Copyright (c) 2008 Dan Knapp

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSObject.h>

@class NSEntityDescription, NSPredicate, NSArray;

enum {
    NSManagedObjectResultType = 0x00,
    NSManagedObjectIDResultType = 0x01,
    NSDictionaryResultType = 0x02
};
typedef NSUInteger NSFetchRequestResultType;

@interface NSFetchRequest : NSObject <NSCoding, NSCopying> {
    NSFetchRequestResultType _resultType;
    NSEntityDescription *_entity;
    NSPredicate *_predicate;
    NSArray *_sortDescriptors;
    NSArray *_affectedStores;
    NSUInteger _fetchLimit;
    NSUInteger _fetchBatchSize;
    NSUInteger _fetchOffset;
    BOOL _includesPendingChanges;
    BOOL _includesPropertyValues;
    BOOL _includesSubentities;
    BOOL _returnsDistinctResults;
    BOOL _returnsObjectsAsFaults;
    NSArray *_propertiesToFetch;
    NSArray *_relationshipKeyPathsForPrefetching;
}

- (NSFetchRequestResultType)resultType;

- (NSEntityDescription *)entity;
- (NSPredicate *)predicate;
- (NSArray *)sortDescriptors;
- (NSArray *)affectedStores;

- (NSUInteger)fetchLimit;
- (NSUInteger)fetchBatchSize;
- (NSUInteger)fetchOffset;

- (BOOL)includesPendingChanges;
- (BOOL)includesPropertyValues;
- (BOOL)includesSubentities;

- (BOOL)returnsDistinctResults;
- (BOOL)returnsObjectsAsFaults;

- (NSArray *)propertiesToFetch;

- (NSArray *)relationshipKeyPathsForPrefetching;

- (void)setResultType:(NSFetchRequestResultType)type;
- (void)setEntity:(NSEntityDescription *)value;
- (void)setPredicate:(NSPredicate *)value;
- (void)setSortDescriptors:(NSArray *)value;
- (void)setAffectedStores:(NSArray *)value;

- (void)setFetchLimit:(NSUInteger)value;
- (void)setFetchBatchSize:(NSUInteger)value;
- (void)setFetchOffset:(NSUInteger)value;

- (void)setIncludesPendingChanges:(BOOL)value;
- (void)setIncludesPropertyValues:(BOOL)value;
- (void)setIncludesSubentities:(BOOL)value;

- (void)setReturnsDistinctResults:(BOOL)value;
- (void)setReturnsObjectsAsFaults:(BOOL)value;

- (void)setPropertiesToFetch:(NSArray *)value;

- (void)setRelationshipKeyPathsForPrefetching:(NSArray *)value;

@end
