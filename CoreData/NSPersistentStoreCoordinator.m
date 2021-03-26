/* Copyright (c)2008 Dan Knapp

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreData/NSPersistentStoreCoordinator.h>
#import "NSXMLPersistentStore.h"
#import "NSManagedObjectModel.h"
#import "NSManagedObject.h"
#import "NSInMemoryPersistentStore.h"
#import "NSManagedObjectID-Private.h"
#import <Foundation/NSRaise.h>

NSString * const NSStoreTypeKey=@"NSStoreTypeKey";
NSString * const NSStoreUUIDKey=@"NSStoreUUIDKey";

NSString * const NSXMLStoreType=@"NSXMLStoreType";
NSString * const NSInMemoryStoreType=@"NSInMemoryStoreType";
NSString * const NSMigratePersistentStoresAutomaticallyOption=@"NSMigratePersistentStoresAutomaticallyOption";

NSString * const NSPersistentStoreCoordinatorStoresDidChangeNotification=@"NSPersistentStoreCoordinatorStoresDidChangeNotification";
NSString * const NSAddedPersistentStoresKey=@"NSAddedPersistentStoresKey";
NSString * const NSRemovedPersistentStoresKey=@"NSRemovedPersistentStoresKey";
NSString * const NSUUIDChangedPersistentStoresKey=@"NSUUIDChangedPersistentStoresKey";

@implementation NSPersistentStoreCoordinator

static NSMutableDictionary *_storeTypes=nil;

+(void)initialize {
   if(self==[NSPersistentStoreCoordinator class]){
    _storeTypes=[NSMutableDictionary new];
    [_storeTypes setObject:[NSInMemoryPersistentStore class] forKey:NSInMemoryStoreType];
    [_storeTypes setObject:[NSXMLPersistentStore class] forKey:NSXMLStoreType];
   }
}

+(NSDictionary *)registeredStoreTypes {
    return _storeTypes;
}

+(void)registerStoreClass:(Class)storeClass forStoreType:(NSString *)storeType {
   [_storeTypes setObject:storeClass forKey:storeType];
}

-initWithManagedObjectModel:(NSManagedObjectModel *)model {
   _lock=[[NSLock alloc] init];
   _model=[model retain];
   _stores=[[NSMutableArray alloc] init];
   return self;
}

-(void)dealloc {
   [_lock release];
   [_model release];
   [_stores release];
   [super dealloc];
}

-(NSManagedObjectModel *)managedObjectModel {
   return _model;
}

-(NSPersistentStore *)addPersistentStoreWithType:(NSString *)storeType configuration:(NSString *)configuration URL:(NSURL *)storeURL options:(NSDictionary *)options error:(NSError **)error {
/* It appears that this method will determine the store type if it is nil */
   if(storeType==nil){
   
    for(Class class in [_storeTypes allValues]){
     NSDictionary *metadata=[class metadataForPersistentStoreWithURL:storeURL error:nil];
     if((storeType=[metadata objectForKey:NSStoreTypeKey])!=nil)
      break;
    }
   }
   
   Class          class=[[isa registeredStoreTypes] objectForKey:storeType];
   NSAtomicStore *store=[[[class alloc] initWithPersistentStoreCoordinator:self configurationName:configuration URL:storeURL options:options] autorelease];

   if(![store load:error])
    return nil;

   [_stores addObject:store];

   return store;
}

-(BOOL)setURL:(NSURL *)url forPersistentStore:(NSPersistentStore *)store {
   [store setURL:url];
   return YES;
}

- (BOOL)removePersistentStore:(NSPersistentStore *)store error:(NSError **)error {
   NSArray      *remove=[NSArray arrayWithObject:store];
   NSDictionary *userInfo=[NSDictionary dictionaryWithObject:remove forKey:NSRemovedPersistentStoresKey];

   [store willRemoveFromPersistentStoreCoordinator:self];

   [[NSNotificationCenter defaultCenter] postNotificationName:NSPersistentStoreCoordinatorStoresDidChangeNotification object:self userInfo:userInfo];
   
   [_stores removeObjectIdenticalTo:store];
   
   return YES;
}

-(NSPersistentStore *)migratePersistentStore:(NSPersistentStore *)store toURL:(NSURL *)URL options:(NSDictionary *)options withType:(NSString *)storeType error:(NSError **)error {
    NSUnimplementedMethod();
    return nil;
}

-(NSArray *)persistentStores {
   return _stores;
}

-(NSPersistentStore *)persistentStoreForURL:(NSURL *)URL {
/* This only returns a store if it exists in the coordinator */

   for(NSPersistentStore *check in _stores){
    if([[check URL] isEqual:URL])
     return check;
   }
   
   return nil;
}

-(NSURL *)URLForPersistentStore:(NSPersistentStore *)store {
   return [store URL];
}

-(void)lock {
   [_lock lock];
}


-(BOOL)tryLock {
   return [_lock tryLock];
}

-(void)unlock {
   [_lock unlock];
}

-(NSDictionary *)metadataForPersistentStore:(NSPersistentStore *)store {
   return [store metadata];
}

- (void)setMetadata:(NSDictionary *)value forPersistentStore:(NSPersistentStore *)store {
   [store setMetadata:value];
}

+(BOOL)setMetadata:(NSDictionary *)metadata forPersistentStoreOfType:(NSString *)storeType URL:(NSURL *)url error:(NSError **)error {
   Class check=[[self registeredStoreTypes] objectForKey:storeType];
   
   return [check setMetadata:metadata forPersistentStoreWithURL:url error:error];
}

+(NSDictionary *)metadataForPersistentStoreOfType:(NSString *)storeType URL:(NSURL *)url error:(NSError **)error {
   Class check=[[self registeredStoreTypes] objectForKey:storeType];
   
   return [check metadataForPersistentStoreWithURL:url error:error];
}

-(NSPersistentStore *)_persistentStoreWithIdentifier:(NSString *)identifier {
   for(NSPersistentStore *check in _stores)
    if([[check identifier] isEqualToString:identifier])
     return check;
   
   return nil;
}

-(NSPersistentStore *)_persistentStoreForObjectID:(NSManagedObjectID *)objectID {
   NSEntityDescription  *entity=[objectID entity];
   NSString             *storeIdentifier=[objectID storeIdentifier];
   NSPersistentStore    *check=[self _persistentStoreWithIdentifier:storeIdentifier];
   
   if(check!=nil)
    return check;
    
   NSManagedObjectModel *model=[self managedObjectModel];
   
   if([_stores count]==0){
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] no persistent stores",isa,_cmd];
    return nil;
   }
   
   for(check in _stores){
    NSString *configurationName=[check configurationName];
    
    if(configurationName==nil){
     NSArray  *entities=[model entitiesForConfiguration:configurationName];
        
     if([entities containsObject:entity])
      return check;
    }
   }

   return [_stores objectAtIndex:0];
}

-(NSPersistentStore *)_persistentStoreForObject:(NSManagedObject *)object {
   return [self _persistentStoreForObjectID:[object objectID]];
}

-(NSManagedObjectID *)managedObjectIDForURIRepresentation:(NSURL *)URL {
   NSString             *scheme=[URL scheme];
   NSString             *host=[URL host];
   NSString             *path=[URL path];
   NSString             *referenceObject=[path lastPathComponent];
   NSString             *entityName=[[path stringByDeletingLastPathComponent] lastPathComponent];
   NSManagedObjectModel *model=[self managedObjectModel];
   NSEntityDescription  *entity=[[model entitiesByName] objectForKey:entityName];
   
   return [(NSAtomicStore *)[self _persistentStoreWithIdentifier:host] objectIDForEntity:entity referenceObject:referenceObject];
}

@end
