#import <CoreData/NSManagedObjectID.h>
#import <CoreData/NSEntityDescription.h>
#import <CoreData/NSManagedObjectModel.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSString.h>
#import <Foundation/NSURL.h>
#import <CoreFoundation/CFUUID.h>

@implementation NSManagedObjectID

-initWithEntity:(NSEntityDescription *)entity {
   _entity=[entity retain];
   _isTemporaryID=YES;
   _storeIdentifier=nil;
   
   CFUUIDRef uuid=CFUUIDCreate(NULL);
   _referenceObject=(NSString *)CFUUIDCreateString(NULL,uuid);
   CFRelease(uuid);
   
   return self;
}

-(void)dealloc {
   [_entity release];
   [_storeIdentifier release];
   [_referenceObject release];
   [super dealloc];
}

/* NSManagedObjectID is hashed/equal by ptr value, they are uniqued per persistent store and should be the same instance for all
   references to the same underlying NSManagedObject
 */
 
-copyWithZone:(NSZone *)zone {
   return [self retain];
}

-(NSEntityDescription *)entity {
   return _entity;
}

-(NSString *)storeIdentifier {
   return _storeIdentifier;
}

-(BOOL)isTemporaryID {
   return _isTemporaryID;
}

-referenceObject {
   return _referenceObject;
}

-(NSURL *)URIRepresentation {
   NSString *path=[[_entity name] stringByAppendingPathComponent:_referenceObject];
   NSString *host=_isTemporaryID?@"":_storeIdentifier;
   
   return [[[NSURL alloc] initWithScheme:@"x-coredata" host:host path:path] autorelease];
}

-(NSPersistentStore *)persistentStore {
   return _persistentStore;
}

-(void)setStoreIdentifier:(NSString *)value {
   value=[value copy];
   [_storeIdentifier release];
   _storeIdentifier=value;
}

-(void)setReferenceObject:value {
   _isTemporaryID=NO;
   value=[value copy];
   [_referenceObject release];
   _referenceObject=value;
}

-(void)setPersistentStore:(NSPersistentStore *)store {
   _persistentStore=store;
}

-(NSString *)description {
   return [NSString stringWithFormat:@"<%@ %p temp=%d, ref=%@>",isa,self,_isTemporaryID,_referenceObject];
}

@end

