/* Copyright (c) 2008 Dan Knapp

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSKeyedUnarchiver.h>
#import <Foundation/NSKeyValueObserving.h>

@class NSIndexSet, NSFetchRequest, NSTableColumn;

@interface _NSManagedProxy_observerInfo : NSObject {
    NSObject *observer;
    NSIndexSet *indexSet;
    NSString *keyPath;
    NSKeyValueObservingOptions options;
    void *context;
}
@property(assign) NSObject *observer;
@property(retain) NSIndexSet *indexSet;
@property(retain) NSString *keyPath;
@property(assign) NSKeyValueObservingOptions options;
@property(assign) void *context;
@end

@class NSPredicate;
@class NSEntityDescription;
@class NSManagedObjectContext;
@class NSManagedObject;
@interface _NSManagedProxy : NSObject {
    NSManagedObject *_object;
    NSEntityDescription *_entity;
    NSString *_entityName;
    NSManagedObjectContext *_context;
    NSFetchRequest *_fetchRequest;
    NSMutableArray *_observers;
}

- (id)initWithCoder:(NSCoder *)coder;
- (id)initWithParent:(_NSManagedProxy *)parent object:(NSManagedObject *)object;
- (id)representedObject;
- (NSString *)entityName;
- (NSEntityDescription *)entity;
- (NSManagedObjectContext *)managedObjectContext;
- (void)setManagedObjectContext:(NSManagedObjectContext *)context;
- (id)valueForKey:(NSString *)key;
- (id)objectAtIndex:(NSUInteger)index;
- (NSUInteger)count;
- (void)addObserver:(NSObject *)observer
    toObjectsAtIndexes:(NSIndexSet *)indexes
            forKeyPath:(NSString *)keyPath
               options:(NSKeyValueObservingOptions)options
               context:(void *)context;
- (void)removeObserver:(NSObject *)observer
    fromObjectsAtIndexes:(NSIndexSet *)indexes
              forKeyPath:(NSString *)keyPath;
- (void)notifyObserver:(_NSManagedProxy_observerInfo *)observerInfo;
- (void)_refresh;
- (id)objectValueForTableColumn:(NSTableColumn *)column;

@end
