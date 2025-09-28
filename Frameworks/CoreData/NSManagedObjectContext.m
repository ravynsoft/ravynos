/* Copyright(c)2008 Dan Knapp

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreData/NSManagedObjectContext.h>
#import <CoreData/NSManagedObjectModel.h>
#import <CoreData/NSFetchRequest.h>
#import <CoreData/NSManagedObject.h>
#import <CoreData/NSPersistentStore.h>
#import "NSManagedObject-Private.h"
#import "NSManagedObjectID-Private.h"
#import "NSEntityDescription-Private.h"
#import <CoreData/NSEntityDescription.h>
#import <CoreData/NSAtomicStore.h>
#import <CoreData/CoreDataErrors.h>
#import "NSPersistentStoreCoordinator-Private.h"
#import <Foundation/NSUndoManager.h>
#import <Foundation/NSRaise.h>

NSString * const NSManagedObjectContextWillSaveNotification=@"NSManagedObjectContextWillSaveNotification";
NSString * const NSManagedObjectContextDidSaveNotification=@"NSManagedObjectContextDidSaveNotification";

NSString * const NSInsertedObjectsKey=@"NSInsertedObjectsKey";
NSString * const NSUpdatedObjectsKey=@"NSUpdatedObjectsKey";
NSString * const NSDeletedObjectsKey=@"NSDeletedObjectsKey";
NSString * const NSRefreshedObjectsKey=@"NSRefreshedObjectsKey";
NSString * const NSInvalidatedObjectsKey=@"NSInvalidatedObjectsKey";
NSString * const NSInvalidatedAllObjectsKey=@"NSInvalidatedAllObjectsKey";

@interface NSAtomicStore(private)
-(void)_uniqueObjectID:(NSManagedObjectID *)objectID;
@end

@implementation NSManagedObjectContext

-init {
   _lock=[[NSLock alloc] init];
   _storeCoordinator=nil;
   _undoManager=[[NSUndoManager alloc] init];
   _registeredObjects=[[NSMutableSet alloc] init];
   _insertedObjects=[[NSMutableSet alloc] init];
   _updatedObjects=[[NSMutableSet alloc] init];
   _deletedObjects=[[NSMutableSet alloc] init];
   
   _objectIdToObject=NSCreateMapTable(NSObjectMapKeyCallBacks,NSObjectMapValueCallBacks,0);
   _requestedProcessPendingChanges = NO;
   return self;
}

-(void)dealloc {
   [_storeCoordinator release];
   [_undoManager release];
   [_registeredObjects release];
   [_insertedObjects release];
   [_objectIdToObject release];
   [super dealloc];
}

-(NSPersistentStoreCoordinator *)persistentStoreCoordinator {
   return _storeCoordinator;
}

-(NSUndoManager *)undoManager {
    return _undoManager;
}

-(BOOL)retainsRegisteredObjects {
    return _retainsRegisteredObjects;
}

-(BOOL)propagatesDeletesAtEndOfEvent {
    return _propagatesDeletesAtEndOfEvent;
}

-(NSTimeInterval)stalenessInterval {
    return _stalenessInterval;
}

-mergePolicy {
    return _mergePolicy;
}

-(void)persistentStoresDidChange:(NSNotification *)note {
   NSArray *stores=[[note userInfo] objectForKey:NSRemovedPersistentStoresKey];
      
   for(NSPersistentStore *store in stores){
    NSArray *allObjects=NSAllMapTableValues(_objectIdToObject);

    for(NSManagedObject *check in allObjects){
     NSManagedObjectID *objectID=[check objectID];
        
     if([objectID persistentStore]==store){
     
     NSEntityDescription *entity=[check entity];
      NSArray             *properties=[[entity propertiesByName] allKeys];

      for(NSString *key in properties)
       [check removeObserver:self forKeyPath:key];

      [_registeredObjects removeObject:check];
      [_insertedObjects removeObject:check];
      [_updatedObjects removeObject:check];
      [_deletedObjects removeObject:check];
      NSMapRemove(_objectIdToObject,objectID);
     }
    }
   }
}


-(void)setPersistentStoreCoordinator:(NSPersistentStoreCoordinator *)value {
   if(_storeCoordinator!=nil)
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSPersistentStoreCoordinatorStoresDidChangeNotification object:_storeCoordinator];
    
   value=[value retain];
   [_storeCoordinator release];
   _storeCoordinator=value;

   if(_storeCoordinator!=nil)
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(persistentStoresDidChange:) name:NSPersistentStoreCoordinatorStoresDidChangeNotification object:_storeCoordinator];
}

-(void)setUndoManager:(NSUndoManager *)value {
   value=[value retain];
   [_undoManager release];
   _undoManager=value;
}

-(void)setRetainsRegisteredObjects:(BOOL)value {
   _retainsRegisteredObjects=value;
// FIXME: actually release/retain if needed
   NSUnimplementedMethod();
}

-(void)setPropagatesDeletesAtEndOfEvent:(BOOL)value {
   _propagatesDeletesAtEndOfEvent=value;
}

-(void)setStalenessInterval:(NSTimeInterval)value {
   _stalenessInterval=value;
}

-(void)setMergePolicy:value {
   value=[value retain];
   [_mergePolicy release];
   _mergePolicy=value;
}

-(NSSet *)registeredObjects {
  return _registeredObjects;
}

-(NSSet *)insertedObjects {
    return _insertedObjects;
}

-(NSSet *)updatedObjects {
   return _updatedObjects;
}

-(NSSet *)deletedObjects {
   return _deletedObjects;
}

-(void)_setHasChanges:(BOOL)value {
   _hasChanges=value;
}

-(BOOL)hasChanges {
    return _hasChanges;
}

-(void)lock {
   [_lock lock];
}

-(void)unlock {
   [_lock unlock];
}

-(BOOL)tryLock {
   return [_lock tryLock];
}

-(void)undo {
    NSUnimplementedMethod();
}


-(void)redo {
    NSUnimplementedMethod();
}


-(void)reset {
    NSUnimplementedMethod();
}


-(void)rollback {
    NSUnimplementedMethod();
}

-(NSAtomicStoreCacheNode *)_cacheNodeForObjectID:(NSManagedObjectID *)objectID {
   NSAtomicStore *store=(NSAtomicStore *)[_storeCoordinator _persistentStoreForObjectID:objectID];

   return [store cacheNodeForObjectID:objectID];
}

-(NSManagedObject *)objectRegisteredForID:(NSManagedObjectID *)objectID {
   return NSMapGet(_objectIdToObject,objectID);
}

-(void)_registerObject:(NSManagedObject *)object {
   [_registeredObjects addObject:object];
   NSMapInsert(_objectIdToObject,[object objectID],object);

   NSEntityDescription *entity=[object entity];
   NSArray             *properties=[[entity propertiesByName] allKeys];

   for(NSString *key in properties){
    [object addObserver:self forKeyPath:key options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld context:nil];
   }
}


-(NSManagedObject *)objectWithID:(NSManagedObjectID *)objectID {
   NSManagedObject *result=NSMapGet(_objectIdToObject,objectID);

   if(result==nil){
    result=[[NSManagedObject alloc] initWithObjectID:objectID managedObjectContext:self];
    NSMapInsert(_objectIdToObject,objectID,result);
    [result release];
    [self _registerObject:result];
   }
   
   return result;
}

-(NSArray *)executeFetchRequest:(NSFetchRequest *)fetchRequest error:(NSError **)error {
   NSArray *affectedStores=[fetchRequest affectedStores];

   if(affectedStores==nil)
    affectedStores=[_storeCoordinator persistentStores];
  
   NSMutableSet *resultSet=[NSMutableSet set];

   for(NSManagedObject *check in _insertedObjects){
    NSEntityDescription *entity=[check entity];
    
    if([entity _isKindOfEntity:[fetchRequest entity]]){       
     if(![_deletedObjects containsObject:check]){
      if([affectedStores containsObject:[[check objectID] persistentStore]]){
       [resultSet addObject:check];
       }
     }
    }
   }
   
   for(NSAtomicStore *store in affectedStores){
    NSSet *nodes=[store cacheNodes];
    
    for(NSAtomicStoreCacheNode *node in nodes){
     NSManagedObjectID   *checkID=[node objectID];
     NSEntityDescription *entity=[checkID entity];
    
     if([entity _isKindOfEntity:[fetchRequest entity]]){
      NSManagedObject *check=[self objectWithID:checkID];
      
      if(![_deletedObjects containsObject:check]){
       [resultSet addObject:check];
       }
	 }
    }
   }
   
   NSMutableArray *result=[NSMutableArray arrayWithArray:[resultSet allObjects]];

   NSPredicate *p=[fetchRequest predicate];
   
   if(p!=nil)
    [result filterUsingPredicate:p];

   [result sortUsingDescriptors:[fetchRequest sortDescriptors]];
 
   return result;
}

-(NSUInteger)countForFetchRequest:(NSFetchRequest *)request error:(NSError **)error {
   return [[self executeFetchRequest:request error:error] count];
}

-(void)insertObject:(NSManagedObject *)object {
   NSPersistentStore *store=[_storeCoordinator _persistentStoreForObject:object];
   
   [[object objectID] setStoreIdentifier:[store identifier]];
   [[object objectID] setPersistentStore:store];

   [_insertedObjects addObject:object];
   [_updatedObjects addObject:object];
   [self _registerObject:object];
}

-(void)deleteObject:(NSManagedObject *)object {
   NSArray *properties=[[object entity] properties];
    
   for(NSPropertyDescription *property in properties)
    [object setValue:nil forKey:[property name]];

   [_deletedObjects addObject:object];
}

-(void)assignObject:object toPersistentStore:(NSPersistentStore *)store {
   [object retain];

   NSMapRemove(_objectIdToObject,[object objectID]);

   [[object objectID] setStoreIdentifier:[store identifier]];

   [[object objectID] setPersistentStore:store];

   NSMapInsert(_objectIdToObject,[object objectID],object);
   [object release];
}

-(void)detectConflictsForObject:(NSManagedObject *)object {
// NOT NEEDED
    NSUnimplementedMethod();
}

-(void)refreshObject:(NSManagedObject *)object mergeChanges:(BOOL)flag {
// NEEDED
    NSUnimplementedMethod();
}

-(void)_requestProcessPendingChanges {
    if(!_requestedProcessPendingChanges){

	NSRunLoop *runLoop = [NSRunLoop mainRunLoop];
	[runLoop performSelector: @selector(_processPendingChangesForRequest)
		 target: self
		 argument: nil
		 order: 0
		 modes: [NSArray arrayWithObject:NSRunLoopCommonModes]];
	_requestedProcessPendingChanges = YES;
   }
}

-(void)_processPendingChanges {
    _requestedProcessPendingChanges = NO;
}

-(void)processPendingChanges {
   if(_requestedProcessPendingChanges){
    [[NSRunLoop mainRunLoop] cancelPerformSelector: @selector(_processPendingChangesForRequest)target: self argument: nil];
   }
   
   [self _processPendingChanges];
}


-(void)_processPendingChangesForRequest {
    [self _processPendingChanges];
}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
   if(NSMapGet(_objectIdToObject,[object objectID])==object)
    [_updatedObjects addObject:object];
}

-(BOOL)obtainPermanentIDsForObjects:(NSArray *)objects error:(NSError **)error {

   for(NSManagedObject *check in objects){
    NSManagedObjectID *checkID=[check objectID];
        
    if([checkID isTemporaryID]){
     NSAtomicStore *store=(NSAtomicStore *)[checkID persistentStore];
     
     NSMapRemove(_objectIdToObject,checkID);

#if 1
     if(store==nil)
      NSLog(@"internal inconsistency , object had no store %@",check);
#else     
     if(store==nil){
      NSString *storeIdentifier=[checkID storeIdentifier];
      
      if(storeIdentifier!=nil)
       store=(NSAtomicStore *)[_storeCoordinator _persistentStoreWithIdentifier:storeIdentifier];
      else {
       store=(NSAtomicStore *)[_storeCoordinator _persistentStoreForObject:check];
       [checkID setStoreIdentifier:[store identifier]];
      }
      
      [checkID setPersistentStore:store];
     }
#endif
               
     id referenceObject=[store newReferenceObjectForManagedObject:check];
     
     [checkID setReferenceObject:referenceObject];
     
     [store _uniqueObjectID:checkID];
     
     NSMapInsert(_objectIdToObject,checkID,check);
    }
   }
   
   return YES;
}

-(BOOL)save:(NSError **)errorp {
   NSMutableArray *errors=[NSMutableArray array];
   NSMutableArray *errorStores=[NSMutableArray array];
   NSError        *idError=nil;
   NSMutableSet   *affectedStores=[NSMutableSet set];
   
   [[NSNotificationCenter defaultCenter] postNotificationName:NSManagedObjectContextWillSaveNotification object:self];
   
// delete cache nodes    
   for(NSManagedObject *deleted in _deletedObjects){
    NSAtomicStore          *store=(NSAtomicStore *)[_storeCoordinator _persistentStoreForObject:deleted];
    NSAtomicStoreCacheNode *node=[store cacheNodeForObjectID:[deleted objectID]];
    
    [store willRemoveCacheNodes:[NSSet setWithObject:node]];

    [affectedStores addObject:store];
    
    // Don't process deleted objects
    NSMapRemove(_objectIdToObject,[deleted objectID]);

    [_insertedObjects removeObject:deleted];
    [_updatedObjects removeObject:deleted];
   }

   if(![self obtainPermanentIDsForObjects:[_insertedObjects allObjects] error:&idError]){
    NSMutableDictionary *userInfo=[NSMutableDictionary dictionary];
   
    [userInfo setObject:@"obtainPermanentIDsForObjects failed" forKey:NSLocalizedDescriptionKey];
   
    if(errorp!=NULL)
     *errorp=[NSError errorWithDomain:NSCocoaErrorDomain code:NSPersistentStoreIncompleteSaveError userInfo:userInfo];
     
    return NO;
   }

   for(NSManagedObject *inserted in _insertedObjects){
    NSAtomicStore          *store=(NSAtomicStore *)[_storeCoordinator _persistentStoreForObject:inserted];
    NSAtomicStoreCacheNode *node=[store newCacheNodeForManagedObject:inserted];
    
    [store addCacheNodes:[NSSet setWithObject:node]];
    
    [affectedStores addObject:store];
   }

   [_insertedObjects removeAllObjects];
   
   for(NSManagedObject *updated in _updatedObjects){
    NSAtomicStore          *store=(NSAtomicStore *)[_storeCoordinator _persistentStoreForObject:updated];
    NSAtomicStoreCacheNode *node=[store cacheNodeForObjectID:[updated objectID]];

    [store updateCacheNode:node fromManagedObject:updated];

    [affectedStores addObject:store];
   }
   
   for(NSPersistentStore *store in affectedStores){
    NSError           *saveError=nil;
    
    if([store isKindOfClass:[NSAtomicStore class]]){
     NSAtomicStore *atomicStore=(NSAtomicStore *)store;
     
     if(![atomicStore save:&saveError]){
      [errorStores addObject:atomicStore];
      [errors addObject:saveError];
     }
    }
   }

   [[NSNotificationCenter defaultCenter] postNotificationName:NSManagedObjectContextDidSaveNotification object:self];
   
   if([errors count]==0)
    return YES;
  
   NSMutableDictionary *userInfo=[NSMutableDictionary dictionary];
   
   [userInfo setObject:@"Unable to save managed object context" forKey:NSLocalizedDescriptionKey];
   [userInfo setObject:errorStores forKey:NSAffectedStoresErrorKey];
   [userInfo setObject:errors forKey:NSDetailedErrorsKey];
   
   if(errorp!=NULL)
    *errorp=[NSError errorWithDomain:NSCocoaErrorDomain code:NSPersistentStoreIncompleteSaveError userInfo:userInfo];
  
   return NO;
}

-(void)mergeChangesFromContextDidSaveNotification:(NSNotification *)notification {
    NSUnimplementedMethod();
}


-(BOOL)commitEditing {
   NSUnimplementedMethod();
   return NO;
}

-(void)commitEditingWithDelegate:(id)delegate didCommitSelector:(SEL)didCommitSelector contextInfo:(void *)contextInfo {
   NSUnimplementedMethod();
}


-(void)discardEditing {
   NSUnimplementedMethod();
}


-(void)objectDidBeginEditing:(id)editor {
   NSUnimplementedMethod();
}


-(void)objectDidEndEditing:(id)editor {
   NSUnimplementedMethod();
}

@end
