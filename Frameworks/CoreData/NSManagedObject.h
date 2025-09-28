/* Copyright (c) 2008 Dan Knapp

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSObject.h>

@class NSManagedObjectID, NSEntityDescription, NSManagedObjectContext, NSPropertyDescription, NSMutableDictionary, NSDictionary, NSError, NSArray, NSMutableSet;

@interface NSManagedObject : NSObject {
    NSManagedObjectID *_objectID;
    NSManagedObjectContext *_context;
    BOOL _isInserted, _isUpdated, _isDeleted, _isFault;
    NSDictionary *_committedValues;
    NSMutableDictionary *_changedValues;
    void *_observationInfo;
}

- initWithEntity:(NSEntityDescription *)entity insertIntoManagedObjectContext:(NSManagedObjectContext *)context;

- (NSEntityDescription *)entity;
- (NSManagedObjectID *)objectID;
- self;

- (NSManagedObjectContext *)managedObjectContext;
- (BOOL)isInserted;
- (BOOL)isUpdated;
- (BOOL)isDeleted;
- (BOOL)isFault;
- (BOOL)hasFaultForRelationshipNamed:(NSString *)key;

- (void)awakeFromFetch;
- (void)awakeFromInsert;
- (NSDictionary *)changedValues;
- (NSDictionary *)committedValuesForKeys:(NSArray *)keys;
- (void)dealloc;
- (void)didSave;
- (void)willTurnIntoFault;
- (void)didTurnIntoFault;
- (void)willSave;

- valueForKey:(NSString *)key;
- (void)setValue:value forKey:(NSString *)key;
- (NSMutableSet *)mutableSetValueForKey:(NSString *)key;
- primitiveValueForKey:(NSString *)key;
- (void)setPrimitiveValue:(id)value forKey:(NSString *)key;

- (BOOL)validateValue:(id *)value forKey:(NSString *)key error:(NSError **)error;
- (BOOL)validateForDelete:(NSError **)error;
- (BOOL)validateForInsert:(NSError **)error;
- (BOOL)validateForUpdate:(NSError **)error;

+ (BOOL)automaticallyNotifiesObserversForKey:(NSString *)key;
- (void)didAccessValueForKey:(NSString *)key;
- (void)willAccessValueForKey:(NSString *)key;

@end
