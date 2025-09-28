#import "NSXMLPersistentStore.h"
#import <CoreData/NSPersistentStoreCoordinator.h>
#import <CoreData/NSEntityDescription.h>
#import <CoreData/NSRelationshipDescription.h>
#import <CoreData/NSAttributeDescription.h>
#import <CoreData/NSManagedObject.h>
#import <CoreData/NSManagedObjectModel.h>
#import <CoreData/NSAtomicStoreCacheNode.h>
#import <Foundation/NSXMLDocument.h>
#import <Foundation/NSXMLElement.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSDictionary.h>
#import <CoreFoundation/CFUUID.h>

/* TODO:
   - date formatting
   - binary data??
 */
 
@implementation NSXMLPersistentStore

+(NSDictionary *)metadataForPersistentStoreWithURL:(NSURL *)url error:(NSError **)error {
   NSData   *data=[[NSData alloc] initWithContentsOfURL:url options:0 error:error];
   NSInteger options=NSXMLNodePreserveCharacterReferences|NSXMLNodePreserveWhitespace;

   if([data length]==0)
    return nil;
   
   NSXMLDocument *xml=[[[NSXMLDocument alloc] initWithContentsOfURL:url options:options error:error] autorelease];
   
   if(xml==nil)
    return nil;
   
   NSXMLElement *database=[[xml nodesForXPath:@"database" error:nil] lastObject];
   NSXMLElement *databaseInfo=[[database elementsForName:@"databaseInfo"] lastObject];
   NSXMLElement *uuid=[[databaseInfo elementsForName:@"UUID"] lastObject];

   return [NSDictionary dictionaryWithObjectsAndKeys:[uuid stringValue],NSStoreUUIDKey,NSXMLStoreType,NSStoreTypeKey,nil];
}

-(NSString *)type {
   return NSXMLStoreType;
}

-(NSXMLElement *)databaseElement {
   return [[_document nodesForXPath:@"database" error:nil] lastObject];
}

-(NSXMLElement *)databaseInfoElement {
   return [[[self databaseElement] elementsForName:@"databaseInfo"] lastObject];
}

-(NSXMLElement *)metadataElement {
   return [[[self databaseElement] elementsForName:@"metadata"] lastObject];
}
 
-(NSXMLElement *)identifierElement {
   return [[[self databaseInfoElement] elementsForName:@"UUID"] lastObject];
}

-(NSXMLElement *)nextObjectIDElement {
   return [[[self databaseInfoElement] elementsForName:@"nextObjectID"] lastObject];
}

-initWithPersistentStoreCoordinator:(NSPersistentStoreCoordinator *)coordinator configurationName:(NSString *)configurationName URL:(NSURL *)url options:(NSDictionary *)options {
   if([super initWithPersistentStoreCoordinator:coordinator configurationName:configurationName URL:url options:options]==nil)
    return nil;

   NSData   *data=[[NSData alloc] initWithContentsOfURL:[self URL] options:0 error:NULL];
   NSInteger xmlOptions=NSXMLNodePreserveCharacterReferences | NSXMLNodePreserveWhitespace;

// A valid load can be from a non-existent file or a zero file, zero length checks for both

   if([data length]!=0){   
    if((_document=[[NSXMLDocument alloc] initWithData:data options:xmlOptions error:NULL])==nil){
     [self dealloc];
     return nil;
    }
   }
   else {
    _document=[[NSXMLDocument alloc] initWithKind:NSXMLDocumentXMLKind options:xmlOptions];
    
    NSXMLElement *database=[[NSXMLElement alloc] initWithName:@"database"];
    NSXMLElement *databaseInfo=[[NSXMLElement alloc] initWithName:@"databaseInfo"];
    NSXMLElement *versionElement=[[NSXMLElement alloc] initWithName:@"version" stringValue:@"1"];
    NSXMLElement *uuidElement=[[NSXMLElement alloc] initWithName:@"UUID" stringValue:[self identifier]];
    NSXMLElement *nextObjectID=[[NSXMLElement alloc] initWithName:@"nextObjectID" stringValue:@"1"];
    NSXMLElement *metadata=[[NSXMLElement alloc] initWithName:@"metadata"];
     
    [_document addChild:database];
    [database addChild:databaseInfo];
    [databaseInfo addChild:versionElement];
    [databaseInfo addChild:uuidElement];
    [databaseInfo addChild:nextObjectID];
    [databaseInfo addChild:metadata];
    
    [metadata release];
    [nextObjectID release];
    [uuidElement release];
    [versionElement release];
    [databaseInfo release];
    [database release];
   }
   
   [super setIdentifier:[[self identifierElement] stringValue]];
   [self setMetadata:[NSDictionary dictionaryWithObjectsAndKeys:[self identifier],NSStoreUUIDKey,[self type],NSStoreTypeKey,nil]];

   _referenceToCacheNode=[[NSMutableDictionary alloc] init];
   _referenceToElement=[[NSMutableDictionary alloc] init];
   _usedReferences=[[NSMutableSet alloc] init];
      
   return self;
}

-(void)dealloc {
   [_document release];
   [_referenceToCacheNode release];
   [_referenceToElement release];
   [_usedReferences release];
   [super dealloc];
}


-(void)setIdentifier:(NSString *)identifier {
   [super setIdentifier:identifier];
   [[self identifierElement] setStringValue:identifier];
}

-(NSAtomicStoreCacheNode *)cacheNodeForEntity:(NSEntityDescription *)entity referenceObject:reference {
   NSAtomicStoreCacheNode *result=[_referenceToCacheNode objectForKey:reference];
    
   if(result==nil){
    NSManagedObjectID *objectID=[self objectIDForEntity:entity referenceObject:reference];
    
    result=[[NSAtomicStoreCacheNode alloc] initWithObjectID:objectID];

    [_referenceToCacheNode setObject:result forKey:reference];
    
    [result release];
   }
    
   return result;
}
  
-(NSAtomicStoreCacheNode *)loadEntityElement:(NSXMLElement *)entityElement model:(NSManagedObjectModel *)model {
   NSString  *entityName=[[entityElement attributeForName:@"type"] stringValue];
   NSString  *entityReference=[[entityElement attributeForName:@"id"] stringValue];
   NSArray   *attributeElements=[entityElement elementsForName:@"attribute"];
   NSArray   *relationshipElements=[entityElement elementsForName:@"relationship"];

   NSEntityDescription *entity=[[model entitiesByName] objectForKey:entityName];

   if(entity==nil){
    NSLog(@"Unable to find entity %@ in model",entityName);
    return nil;
   }
   
   [_referenceToElement setObject:entityElement forKey:entityReference];

   NSAtomicStoreCacheNode *cacheNode=[self cacheNodeForEntity:entity referenceObject:entityReference];

   NSDictionary *attributesByName=[entity attributesByName];
   
   for(NSXMLElement *attribute in attributeElements){
    NSString               *name=[[attribute attributeForName:@"name"] stringValue];
    NSAttributeDescription *description=[attributesByName objectForKey:name]; 

    if(description==nil){
     NSLog(@"Unable to find attribute named %@ for entity named %@",name,entityName);
     continue;
    }

    NSString *type=[[attribute attributeForName:@"type"] stringValue];
    NSString *stringValue=[attribute stringValue];
    id        objectValue=nil;

    switch([description attributeType]){
    
     case NSUndefinedAttributeType:
      NSLog(@"Unhandled attribute type NSUndefinedAttributeType");
      break;
      
     case NSInteger16AttributeType:
      objectValue=[NSNumber numberWithInteger:[stringValue integerValue]];
      break;
      
     case NSInteger32AttributeType:
      objectValue=[NSNumber numberWithInteger:[stringValue integerValue]];
      break;
      
     case NSInteger64AttributeType:
      objectValue=[NSNumber numberWithInteger:[stringValue integerValue]];
      break;
      
     case NSDecimalAttributeType:
     // decimal types not supported right now, use double
      objectValue=[NSNumber numberWithDouble:[stringValue doubleValue]];
//      objectValue=[NSDecimalNumber decimalNumberWithString:stringValue];
      break;
      
     case NSDoubleAttributeType:
      objectValue=[NSNumber numberWithDouble:[stringValue doubleValue]];
      break;
      
     case NSFloatAttributeType:
      objectValue=[NSNumber numberWithFloat:[stringValue floatValue]];
      break;
      
     case NSStringAttributeType:
      objectValue=stringValue;
      break;
      
     case NSBooleanAttributeType:
      objectValue=[NSNumber numberWithBool:[stringValue intValue]];
      break;
      
     case NSDateAttributeType:
      objectValue=nil;
      // we don't want to use NSCalendarDate
   //   objectValue=[NSCalendarDate dateWithNaturalLanguageString:stringValue];
      break;
      
     case NSBinaryDataAttributeType:
      NSLog(@"Unhandled attribute type NSBinaryDataAttributeType");
      break;
      
     case NSTransformableAttributeType:
      NSLog(@"Unhandled attribute type NSTransformableAttributeType");
      break;
      
    }

    if(objectValue!=nil)
     [cacheNode setValue:objectValue forKey:name];
   }
   
   NSDictionary *relationshipsByName=[entity relationshipsByName];

   for(NSXMLElement *relationship in relationshipElements){
    NSString                  *name=[[relationship attributeForName:@"name"] stringValue];
    NSRelationshipDescription *description=[relationshipsByName objectForKey:name];
    
    if(description==nil){
     NSLog(@"No description for relationship name %@ in %@",name,entityName);
     continue;
    }
    
    NSString            *destinationEntityName=[[relationship attributeForName:@"destination"] stringValue];
    NSEntityDescription *destinationEntity=[[model entitiesByName] objectForKey:destinationEntityName];
    NSString            *type=[[relationship attributeForName:@"type"] stringValue];
    NSString            *idrefsString=[[relationship attributeForName:@"idrefs"] stringValue];
    NSArray             *idrefs=[idrefsString length]?[idrefsString componentsSeparatedByString:@" "]:nil;
    id                   objectValue=[NSMutableSet set];
        
    for(NSString *ref in idrefs){     
     NSAtomicStoreCacheNode *cacheNode=[self cacheNodeForEntity:destinationEntity referenceObject:ref];

     [objectValue addObject:cacheNode];
    }
    
    if(![description isToMany]){
    
     if([objectValue count]>1){
      NSLog(@"relationship description is not to many, but destination is %d",[objectValue count]);
     }
     
     objectValue=[objectValue anyObject];
    }

    [cacheNode setValue:objectValue forKey:name];
   }
    
   return cacheNode;
}

- (BOOL)load:(NSError **)errorp {

   
   NSManagedObjectModel *model=[[self persistentStoreCoordinator] managedObjectModel];

   NSXMLElement *database=[self databaseElement];
   NSArray      *objects=[database elementsForName:@"object"];
   int           i,count=[objects count];
   NSMutableSet *newNodes=[NSMutableSet set];
   
   for(i=0;i<count;i++){
    NSXMLElement *element=[objects objectAtIndex:i];
    NSAtomicStoreCacheNode *node=[self loadEntityElement:element model:model];
    
    if(node!=nil)
     [newNodes addObject:node];
   }
   
   [self addCacheNodes:newNodes];
   
   return YES;
}
 
 
- (BOOL)save:(NSError **)error {
    
    NSData *data=[_document XMLData];

    return [data writeToURL:[self URL] atomically:YES];
}
 
-(NSXMLElement *)entityElementForObjectID:(NSManagedObjectID *)objectID {
   id reference=[self referenceObjectForObjectID:objectID];
   
   return [_referenceToElement objectForKey:reference];
}

-(void)updateCacheNode:(NSAtomicStoreCacheNode *)node fromManagedObject:(NSManagedObject *)managedObject {
   NSXMLElement   *entityElement=[self entityElementForObjectID:[managedObject objectID]];
   NSDictionary   *attributesByName=[[managedObject entity] attributesByName];
   NSArray        *attributeKeys=[attributesByName allKeys];
   NSMutableArray *children=[NSMutableArray array];

   for(NSString *attributeName in attributeKeys){
    NSAttributeDescription *attributeDescription=[attributesByName objectForKey:attributeName];
    NSXMLElement           *attributeElement=[NSXMLNode elementWithName:@"attribute"];
    id                      value=[managedObject primitiveValueForKey:attributeName];
    NSString               *type=nil;
    NSString               *stringValue=nil;
        
    switch([attributeDescription attributeType]){
     case NSUndefinedAttributeType:
      NSLog(@"Unhandled attribute type NSUndefinedAttributeType");
      break;
      
     case NSInteger16AttributeType:
      type=@"int16";
      stringValue=[value description];
      break;
      
     case NSInteger32AttributeType:
      type=@"int32";
      stringValue=[value description];
      break;
      
     case NSInteger64AttributeType:
      type=@"int64";
      stringValue=[value description];
      break;
      
     case NSDecimalAttributeType:
      type=@"decimal";
      stringValue=[value description];
      break;
      
     case NSDoubleAttributeType:
      type=@"double";
      stringValue=[value description];
      break;
      
     case NSFloatAttributeType:
      type=@"float";
      stringValue=[value description];
      break;
      
     case NSStringAttributeType:
      type=@"string";
      stringValue=[value description];
      break;
      
     case NSBooleanAttributeType:
      type=@"bool";
      stringValue=[value description];
      break;
      
     case NSDateAttributeType:
      type=@"date";
      stringValue=[value description];
      break;
      
     case NSBinaryDataAttributeType:
      type=@"bin";
      NSLog(@"Unhandled attribute type NSBinaryDataAttributeType");
      break;
      
     case NSTransformableAttributeType:
      NSLog(@"Unhandled attribute type NSTransformableAttributeType");
      break;
      
    }
    
    [attributeElement setStringValue:stringValue];
    [attributeElement addAttribute:[NSXMLNode attributeWithName:@"name" stringValue:attributeName]];
    [attributeElement addAttribute:[NSXMLNode attributeWithName:@"type" stringValue:type]];
    
    [children addObject:attributeElement];
    
    [node setValue:value forKey:attributeName];
   }

   NSDictionary *relationshipsByName=[[managedObject entity] relationshipsByName];
   NSArray      *relationshipKeys=[relationshipsByName allKeys];

   for(NSString *relationshipName in relationshipKeys){
    NSRelationshipDescription *relationshipDescription=[relationshipsByName objectForKey:relationshipName];
    NSXMLElement              *relationshipElement=[NSXMLNode elementWithName:@"relationship"];
    NSString                  *relationshipType=[NSString stringWithFormat:@"%d/%d",[relationshipDescription minCount],[relationshipDescription maxCount]];
    NSEntityDescription       *destinationEntity=[relationshipDescription destinationEntity];
    id                         value=[managedObject primitiveValueForKey:relationshipName];
    NSSet                     *valueSet;
    NSMutableSet              *cacheNodeSet=[NSMutableSet set];
    
    if([relationshipDescription isToMany]){
     if(value!=nil && ![value isKindOfClass:[NSSet class]]){
      NSLog(@"relationship isToMany, value is not a set");
      continue;
     }
     valueSet=value;
    }
    else {
     valueSet=[NSSet setWithObject:value];
    }
    
    [relationshipElement addAttribute:[NSXMLNode attributeWithName:@"name" stringValue:relationshipName]];
    [relationshipElement addAttribute:[NSXMLNode attributeWithName:@"type" stringValue:relationshipType]];
    [relationshipElement addAttribute:[NSXMLNode attributeWithName:@"destination" stringValue:[destinationEntity name]]];
    
    NSMutableArray *idrefArray=[NSMutableArray array];

  
    for(NSManagedObjectID *objectID in valueSet){
     id referenceObject=[self referenceObjectForObjectID:objectID];
     
     [idrefArray addObject:referenceObject];
     
     NSAtomicStoreCacheNode *relNode=[self cacheNodeForEntity:destinationEntity referenceObject:referenceObject];

     [cacheNodeSet addObject:relNode];
    }

    [relationshipElement addAttribute:[NSXMLNode attributeWithName:@"idrefs" stringValue:[idrefArray componentsJoinedByString:@" "]]];

    [children addObject:relationshipElement];

    if([relationshipDescription isToMany]){
     [node setValue:cacheNodeSet forKey:relationshipName];
    }
    else {
     if([cacheNodeSet count]>1){
      NSLog(@"relationship is one to one, yet cacheNodeSet count is %d",[cacheNodeSet count]);
      continue;
     }
     
     if([cacheNodeSet count]==0)
      [node setValue:nil forKey:relationshipName];
     else
      [node setValue:[cacheNodeSet anyObject] forKey:relationshipName];
    }
   }
   
   [entityElement setChildren:children];

}
 
-(NSAtomicStoreCacheNode *)newCacheNodeForManagedObject:(NSManagedObject *)managedObject {
   NSEntityDescription    *entity=[managedObject entity];
   NSManagedObjectID      *objectID=[managedObject objectID];
   id                      reference=[self referenceObjectForObjectID:objectID];
   NSAtomicStoreCacheNode *cacheNode=[[NSAtomicStoreCacheNode alloc] initWithObjectID:objectID];
   
   NSXMLElement           *entityElement=[[NSXMLElement alloc] initWithName:@"object"];
   NSXMLNode              *nameAttribute=[NSXMLNode attributeWithName:@"type" stringValue:[entity name]];
   NSXMLNode              *idAttribute=[NSXMLNode attributeWithName:@"id" stringValue:reference];
   
   [entityElement addAttribute:nameAttribute];
   [entityElement addAttribute:idAttribute];
   
   [_referenceToElement setObject:entityElement forKey:reference];

   [[self databaseElement] addChild:entityElement];
   
   [self updateCacheNode:cacheNode fromManagedObject:managedObject];

   return cacheNode;
}

-newReferenceObjectForManagedObject:(NSManagedObject *)managedObject {
   NSXMLElement *nextObjectIDElement=[self nextObjectIDElement];
   NSInteger     nextInteger=[[nextObjectIDElement stringValue] integerValue];
   NSNumber     *check=nil;

   do{
    [check release];
    
    check=[[NSNumber alloc] initWithInteger:nextInteger];
    
    if(![_usedReferences containsObject:check])
     break;
     
    nextInteger++;
     
   }while(YES);
   
   [_usedReferences addObject:check];
   
   [check release];

   [nextObjectIDElement setStringValue:[NSString stringWithFormat:@"%d",nextInteger+1]];
   
   return [[NSString alloc] initWithFormat:@"r%d",nextInteger];
}

-(void)willRemoveCacheNodes:(NSSet *)cacheNodes {
   NSXMLElement *database=[self databaseElement];

   for (NSAtomicStoreCacheNode *node in cacheNodes) {
    NSManagedObjectID   *objectID=[node objectID];
    NSEntityDescription *entity=[objectID entity];
    NSXMLElement        *entityElement=[self entityElementForObjectID:objectID];
    id                   entityReference=[self referenceObjectForObjectID:objectID];
    NSInteger            index=[[database children] indexOfObjectIdenticalTo:entityElement];
    
    if(index==NSNotFound)
     NSLog(@"unable to remove object %@ from database - not found",objectID);
    else
     [[self databaseElement] removeChildAtIndex:index];
    
    [_referenceToElement removeObjectForKey:entityReference];

// Should really be maintaining our own set for this     
    [_cacheNodes removeObject:node];
   }
}
 
 
-(void)willRemoveFromPersistentStoreCoordinator:(NSPersistentStoreCoordinator *)coordinator {
   [_document release];
   _document = nil;
   [super willRemoveFromPersistentStoreCoordinator:coordinator];
}
 
@end
