#import <CoreData/NSAtomicStore.h>
#import <CoreData/NSManagedObject.h>
#import <CoreData/NSAtomicStoreCacheNode.h>
#import "NSManagedObjectID-Private.h"
#import "NSManagedObject-Private.h"
#import <Foundation/NSMutableSet.h>
#import <Foundation/NSMutableDictionary.h>
#import <Foundation/NSRaise.h>

@implementation NSAtomicStore

-initWithPersistentStoreCoordinator:(NSPersistentStoreCoordinator *)coordinator configurationName:(NSString *)configurationName URL:(NSURL *)url options:(NSDictionary *)options {
   if([super initWithPersistentStoreCoordinator:coordinator configurationName:configurationName URL:url options:options]==nil)
    return nil;

   _metadata=[[NSDictionary alloc] init];
   _cacheNodes=[[NSMutableSet alloc] init];
   _objectIDToCacheNode=[[NSMutableDictionary alloc] init];
   _objectIDTable=[[NSMutableDictionary alloc] init];
   return self;
}

-(void)dealloc {
   [_metadata release];
   [_cacheNodes release];
   [_objectIDToCacheNode release];
   [_objectIDTable release];
   [super dealloc];
}

-(NSSet *)cacheNodes {
   return _cacheNodes;
}

-(NSDictionary *)metadata {
   return _metadata;
}

-(void)setMetadata:(NSDictionary *)value {
   value=[value copy];
   [_metadata release];
   _metadata=value;
}

-(void)addCacheNodes:(NSSet *)values {
   for(NSAtomicStoreCacheNode *node in values){

    [_objectIDToCacheNode setObject:node forKey:[node objectID]];
   }
   [_cacheNodes unionSet:values];
}

-(NSAtomicStoreCacheNode *)cacheNodeForObjectID:(NSManagedObjectID *)objectID {
   NSAtomicStoreCacheNode *result= [_objectIDToCacheNode objectForKey:objectID];

   return result;
}

-(NSAtomicStoreCacheNode *)newCacheNodeForManagedObject:(NSManagedObject *)managedObject {
   return [[NSAtomicStoreCacheNode alloc] initWithObjectID:[managedObject objectID]];
}

-(NSManagedObjectID *)objectIDForEntity:(NSEntityDescription *)entity referenceObject:referenceObject {
   NSMutableDictionary *refTable=[_objectIDTable objectForKey:[entity name]];

   if(refTable==nil){
    refTable=[NSMutableDictionary dictionary];
    [_objectIDTable setObject:refTable forKey:[entity name]];
   }
   
   NSManagedObjectID *result=[refTable objectForKey:referenceObject];
   
   if(result==nil){
    result=[[[NSManagedObjectID alloc] initWithEntity:entity] autorelease];
   
    [result setReferenceObject:referenceObject];
    [result setStoreIdentifier:[self identifier]];
    [result setPersistentStore:self];

    [refTable setObject:result forKey:referenceObject];
   }
   
   return result;
}

-(void)_uniqueObjectID:(NSManagedObjectID *)objectID {
   NSEntityDescription *entity=[objectID entity];
   NSMutableDictionary *refTable=[_objectIDTable objectForKey:[entity name]];
   
   if(refTable==nil){
    refTable=[NSMutableDictionary dictionary];
    [_objectIDTable setObject:refTable forKey:[entity name]];
   }
   
   id referenceObject=[objectID referenceObject];

   [refTable setObject:objectID forKey:referenceObject];
}

-referenceObjectForObjectID:(NSManagedObjectID *)objectID {   
   return [objectID referenceObject];
}

-(void)willRemoveCacheNodes:(NSSet *)cacheNodes {
}

-newReferenceObjectForManagedObject:(NSManagedObject *)managedObject {
   NSInvalidAbstractInvocation();
   return 0;
}

-(void)updateCacheNode:(NSAtomicStoreCacheNode *)node fromManagedObject:(NSManagedObject *)managedObject {
   NSInvalidAbstractInvocation();
}

-(BOOL)load:(NSError **)error {
   NSInvalidAbstractInvocation();
   return 0;
}

-(BOOL)save:(NSError **)error {
   NSInvalidAbstractInvocation();
   return 0;
}

@end
