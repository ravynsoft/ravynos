/* Copyright (c) 2008 Dan Knapp

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreData/NSManagedObject.h>
#import "NSManagedObjectID-Private.h"
#import "NSManagedObjectContext-Private.h"
#import "NSEntityDescription-Private.h"
#import <CoreData/NSAttributeDescription.h>
#import <CoreData/NSRelationshipDescription.h>
#import <CoreData/NSAtomicStoreCacheNode.h>
#import <Foundation/NSRaise.h>
#import "NSManagedObjectSet.h"
#import "NSManagedObjectSetEnumerator.h"
#import "NSManagedObjectMutableSet.h"

@implementation NSManagedObject

-init {
   NSLog(@"Error - can't initialize an NSManagedObject with -init");
   return nil;
}

-initWithObjectID:(NSManagedObjectID *)objectID managedObjectContext:(NSManagedObjectContext *)context {
   NSEntityDescription *entity=[objectID entity];
   NSString            *className=[entity managedObjectClassName];
   Class                class=NSClassFromString(className);
 
   if(class==Nil){
    NSLog(@"Unable to find class %@ specified by entity %@ in the runtime, using NSManagedObject,objectID=%@",className,[entity name],objectID);
    
    class=[NSManagedObject class];
   }
   
   [super dealloc];
   self=[class allocWithZone:NULL];
   
   _objectID=[objectID copy];
   _context=context;
   _committedValues = nil;
   _changedValues = [[NSMutableDictionary alloc] init];
   _isFault=YES;
   return self;
}

-initWithEntity:(NSEntityDescription *)entity insertIntoManagedObjectContext:(NSManagedObjectContext *)context {
   NSManagedObjectID *objectID=[[[NSManagedObjectID alloc] initWithEntity:entity] autorelease];
   
   if((self=[self initWithObjectID:objectID managedObjectContext:context])==nil)
    return nil;

   NSDictionary *attributes=[entity attributesByName];
   NSEnumerator *state=[attributes keyEnumerator];
   NSString     *key;
   
   while((key=[state nextObject])!=nil){
    id object=[attributes objectForKey:key];
    id value=[object defaultValue];
    
    if(value!=nil)
     [self setPrimitiveValue:value forKey:key]; 
   }

   [context insertObject:self];
   
   return self;
}

-(void)dealloc {
   [self didTurnIntoFault];
   
   [_objectID release];
   [_changedValues release];
   [super dealloc];
}

-(NSUInteger)hash {
   return [_objectID hash];
}

-(BOOL)isEqual:otherX {
   if(![otherX isKindOfClass:[NSManagedObject class]])
    return NO;
   
   NSManagedObject *other=otherX;
   
   return [_objectID isEqual:other->_objectID];
}

-(NSEntityDescription *)entity {
   return [_objectID entity];
}

-(NSManagedObjectID *)objectID {
   return _objectID;
}

-self {
   return self;
}

-(NSManagedObjectContext *)managedObjectContext {
   return _context;
}

-(BOOL)isInserted {
   return _isInserted;
}

-(BOOL)isUpdated {
   return _isUpdated;
}

-(BOOL)isDeleted {
   return _isDeleted;
}

-(BOOL)isFault {
   return _isFault;
}

- (BOOL) hasFaultForRelationshipNamed:(NSString *) key {
    NSUnimplementedMethod();
    return NO;
}

- (void) awakeFromFetch {
    NSUnimplementedMethod();
}

- (void) awakeFromInsert {
    NSUnimplementedMethod();
}

-(NSDictionary *)changedValues {
   return _changedValues;
}

-(NSDictionary *)_committedValues {

   if([[self objectID] isTemporaryID])
    return nil;
    
   if(_committedValues==nil){
    NSAtomicStoreCacheNode *node=[_context _cacheNodeForObjectID:[self objectID]];
    NSDictionary           *propertyCache=[node propertyCache];
    NSMutableDictionary    *storedValues=[[NSMutableDictionary alloc] init];
    
    NSArray *properties=[[self entity] properties];
    
    for(NSPropertyDescription *property in properties){
     NSString *name=[property name];
     
     if([property isKindOfClass:[NSAttributeDescription class]]){
      id value=[propertyCache objectForKey:name];
      
      if(value!=nil){
       [storedValues setObject:value forKey:name];
      }
     }
     else if([property isKindOfClass:[NSRelationshipDescription class]]){
      NSRelationshipDescription *relationship=(NSRelationshipDescription *)property;
      id                         indirectValue=[propertyCache objectForKey:name];
 
      if(indirectValue!=nil){
       id value;
             
       if(![relationship isToMany])
        value=[indirectValue objectID];
       else {
        value=[NSMutableSet set];
        
        for(NSAtomicStoreCacheNode *relNode in indirectValue){
         [value addObject:[relNode objectID]];
        }
       }
       
       [storedValues setObject:value forKey:name];
      }
     }
    }
    
    [_committedValues release];
    _committedValues=storedValues;
   }
   return _committedValues;
}

-(NSDictionary *)committedValuesForKeys:(NSArray *)keys {   
   if(keys==nil)
    return [self _committedValues];
   else {
    NSMutableDictionary *result=[NSMutableDictionary dictionary];
    
    for(NSString *key in keys){
     id object=[[self _committedValues] objectForKey:key];
     
     if(object!=nil){
      [result setObject:object forKey:key];
     }
    }
    
    return result;
   }
}

- (void) didSave {
}

- (void) willTurnIntoFault {
  // do nothing per doc.s
}

- (void) didTurnIntoFault {
  // do nothing also?
}

- (void) willSave {

}

-valueForKey:(NSString *)key {
   if(!key)
    return [self valueForUndefinedKey:nil];

   NSPropertyDescription *property=[[self entity] _propertyForSelector:NSSelectorFromString(key)];
   NSString *propertyName=[property name];
   
   if([property isKindOfClass:[NSAttributeDescription class]]){
    [self willAccessValueForKey:propertyName];
    
    id result=[self primitiveValueForKey:propertyName];
    
    [self didAccessValueForKey:propertyName];
    
    return result;
   }
   else if([property isKindOfClass:[NSRelationshipDescription class]]){
    NSRelationshipDescription *relationship=(NSRelationshipDescription *)property;
    
    [self willAccessValueForKey:propertyName];

    id result=[self primitiveValueForKey:propertyName];
    
    if(result!=nil){
     if([relationship isToMany])
      result=[[[NSManagedObjectSet alloc] initWithManagedObjectContext:_context set:result] autorelease];
     else
      result=[_context objectWithID:result];
    }
    
    [self didAccessValueForKey:propertyName];
    
    return result;
   }
  
   return [super valueForKey:key];
}


-(void)setValue:value forKey:(NSString *) key {
   NSPropertyDescription *property= [[self entity] _propertyForSelector:NSSelectorFromString(key)];
   NSString              *propertyName=[property name];
   
   if([property isKindOfClass:[NSAttributeDescription class]]){
    [self willChangeValueForKey:propertyName];
    [self setPrimitiveValue:value forKey:propertyName];
    [self didChangeValueForKey:propertyName];
    return;
   }
   else if([property isKindOfClass:[NSRelationshipDescription class]]){
    NSRelationshipDescription *relationship=(NSRelationshipDescription *)property;
    NSRelationshipDescription *inverse=[relationship inverseRelationship];
    NSString                  *inverseName=[inverse name];
    id                         valueByID;
        
    if([relationship isToMany]){
     NSMutableSet *set=[NSMutableSet set];
     
     for(NSManagedObject *object in value)
      [set addObject:[object objectID]];
      
     valueByID=set;
    }
    else {
     valueByID=[value objectID];
    }

    if(inverse!=nil){
     id primitivePrevious=[self primitiveValueForKey:key];
     
     if(primitivePrevious!=nil){
      NSSet *allPrevious=[relationship isToMany]?primitivePrevious:[NSSet setWithObject:primitivePrevious];
      
      for(NSManagedObjectID *previousID in allPrevious){
       NSManagedObject *previous=[_context objectWithID:previousID];

     // FIXME: should be using set mutation notifications for to many
       [previous willChangeValueForKey:inverseName];

       if([inverse isToMany])
        [[previous primitiveValueForKey:inverseName] removeObject:[self objectID]];
       else
        [previous setPrimitiveValue:nil forKey:inverseName];
      
       [previous didChangeValueForKey:inverseName];
      }
     }
    }

    [self willChangeValueForKey:propertyName];
    [self setPrimitiveValue:valueByID forKey:propertyName];
    [self didChangeValueForKey:propertyName];

    if(inverse!=nil){     
     NSSet *allValues=[relationship isToMany]?valueByID:[NSSet setWithObject:valueByID];

     for(NSManagedObjectID *valueID in allValues){
      NSManagedObject *value=[_context objectWithID:valueID];

     // FIXME: should be using set mutation notifications for to many
      [value willChangeValueForKey:inverseName];
     
      if([inverse isToMany]){
       NSMutableSet *set=[value primitiveValueForKey:inverseName];
    
       if(set==nil){
        set=[NSMutableSet set];
        [value setPrimitiveValue:set forKey:inverseName];
       }

       [set addObject:[self objectID]];
      }
      else{
       [value setPrimitiveValue:[self objectID] forKey:inverseName];
      }
     
      [value didChangeValueForKey:inverseName];
     }
    }
    return;
   }
   
   [super setValue:value forKey:key];
}

-(NSMutableSet *) mutableSetValueForKey:(NSString *) key {
   return [[[NSManagedObjectMutableSet alloc] initWithManagedObject:self key:key] autorelease];
}

-primitiveValueForKey:(NSString *) key {
   id result=[_changedValues objectForKey:key];
   
   if(result==nil)
    result=[[self _committedValues] objectForKey:key];
   
   return result;
}

-(void)setPrimitiveValue:value forKey:(NSString *)key {
   if(value==nil)
    [_changedValues removeObjectForKey:key];
   else {
    [_changedValues setObject:value forKey:key];
   }
}

- (BOOL) validateValue:(id *) value forKey:(NSString *) key error:(NSError **) error {
    NSUnimplementedMethod();
    return YES;
}


- (BOOL) validateForDelete:(NSError **) error {
    NSUnimplementedMethod();
    return YES;
}


- (BOOL) validateForInsert:(NSError **) error {
    NSUnimplementedMethod();
    return YES;
}


- (BOOL) validateForUpdate:(NSError **) error {
    NSUnimplementedMethod();
    return YES;
}


+(BOOL)automaticallyNotifiesObserversForKey:(NSString *)key {
  // FIXME: Doc.s for NSManagedObject state this returns YES for unmodeled properties.

   return NO;
}

-(void)didAccessValueForKey:(NSString *) key {
}

-(void *)observationInfo {
   return _observationInfo;
}


-(void)setObservationInfo:(void *) value {
   _observationInfo=value;
}

-(void)willAccessValueForKey:(NSString *)key {
}

-(NSString *)description {
   NSMutableDictionary *values=[NSMutableDictionary dictionaryWithDictionary:[self _committedValues]];
   
   [values addEntriesFromDictionary:_changedValues];
   
   return [NSString stringWithFormat:@"<%@ %x:objectID=%@ entity name=%@, values=%@>",isa,self,_objectID,[self entity],values];
}

@end

