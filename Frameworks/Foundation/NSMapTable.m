/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSMapTable.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>
#import <Foundation/NSZone.h>
#import <Foundation/NSEnumerator_dictionaryKeys.h>

@implementation NSMapTable

typedef struct _NSMapNode {
   struct _NSMapNode *next;
   void *key;
   void *value;
} NSMapNode;

const void *NSNotAnIntMapKey=(const void *)0x80000000;
const void *NSNotAPointerMapKey=(const void *)0xffffffff;

static NSUInteger _NSMapPointerHash(NSMapTable *table,const void *object){
   return (NSUInteger)object>>5;
}

static NSUInteger _NSMapObjectHash(NSMapTable *table,const void *object){
   return [(id)object hash];
}

static BOOL _NSMapPointerIsEqual(NSMapTable *table,const void *object1,
  const void *object2){
   return (object1==object2)?YES:NO;
}

static BOOL _NSMapObjectIsEqual(NSMapTable *table,const void *object1,
  const void *object2){
   BOOL result=[(id)object1 isEqual:(id)object2];

   return result;
}

static void _NSMapEmptyRetain(NSMapTable *table,const void *object){
}

static void _NSMapObjectRetain(NSMapTable *table,const void *object){
   [(id)object retain];
}

static void _NSMapEmptyRelease(NSMapTable *table,void *object){
}

static void _NSMapObjectRelease(NSMapTable *table,void *object){
   [(id)object release];
}

static void _NSMapPointerRelease(NSMapTable *table,void *object){
   NSZoneFree(NSZoneFromPointer(object),object);
}

static NSString *_NSMapEmptyDescribe(NSMapTable *table,const void *object){
   return nil;
}

static NSString *_NSMapObjectDescribe(NSMapTable *table,const void *object){
   return [(id)object description];
}

const NSMapTableKeyCallBacks NSIntMapKeyCallBacks={
 NULL, NULL, NULL, NULL, NULL
};

const NSMapTableValueCallBacks NSIntMapValueCallBacks={
 NULL, NULL, NULL
};

const NSMapTableKeyCallBacks NSIntegerMapKeyCallBacks={
 NULL, NULL, NULL, NULL, NULL
};

const NSMapTableValueCallBacks NSIntegerMapValueCallBacks={
 NULL, NULL, NULL
};

const NSMapTableKeyCallBacks NSNonOwnedPointerMapKeyCallBacks={
 NULL, NULL, NULL, NULL, NULL
};
 
const NSMapTableValueCallBacks NSNonOwnedPointerMapValueCallBacks={
 NULL, NULL, NULL
};

const NSMapTableKeyCallBacks NSNonOwnedPointerOrNullMapKeyCallBacks={
 NULL, NULL, NULL, NULL, NULL
};
 
const NSMapTableKeyCallBacks NSNonRetainedObjectMapKeyCallBacks={
 _NSMapObjectHash, _NSMapObjectIsEqual, NULL, NULL, _NSMapObjectDescribe
};
 
const NSMapTableValueCallBacks NSNonRetainedObjectMapValueCallBacks={
 NULL, NULL, _NSMapObjectDescribe
};

const NSMapTableKeyCallBacks NSObjectMapKeyCallBacks={
 _NSMapObjectHash, _NSMapObjectIsEqual, _NSMapObjectRetain, _NSMapObjectRelease, _NSMapObjectDescribe
};
 
const NSMapTableValueCallBacks NSObjectMapValueCallBacks={
 _NSMapObjectRetain, _NSMapObjectRelease, _NSMapObjectDescribe
};
 
const NSMapTableKeyCallBacks NSOwnedPointerMapKeyCallBacks={
 NULL, NULL, NULL, _NSMapPointerRelease, NULL
};
 
const NSMapTableValueCallBacks NSOwnedPointerMapValueCallBacks={
 NULL, _NSMapPointerRelease, NULL
};

NSMapTable *NSCreateMapTable(NSMapTableKeyCallBacks keyCallBacks,
   NSMapTableValueCallBacks valueCallBacks,NSUInteger capacity) {
   return NSCreateMapTableWithZone(keyCallBacks,valueCallBacks,capacity,NULL);
}

NSMapTable *NSCreateMapTableWithZone(NSMapTableKeyCallBacks keyCallBacks,
   NSMapTableValueCallBacks valueCallBacks,NSUInteger capacity,NSZone *zone) {
   NSMapTable *table;

   table=[NSMapTable allocWithZone:zone];

   table->keyCallBacks=NSZoneMalloc(zone,sizeof(NSMapTableKeyCallBacks));
   table->keyCallBacks->hash=(keyCallBacks.hash!=NULL)?keyCallBacks.hash:_NSMapPointerHash;
   table->keyCallBacks->isEqual=(keyCallBacks.isEqual!=NULL)?keyCallBacks.isEqual:_NSMapPointerIsEqual;
   table->keyCallBacks->retain=(keyCallBacks.retain!=NULL)?keyCallBacks.retain:_NSMapEmptyRetain;
   table->keyCallBacks->release=(keyCallBacks.release!=NULL)?keyCallBacks.release:_NSMapEmptyRelease;
   table->keyCallBacks->describe=(keyCallBacks.describe!=NULL)?keyCallBacks.describe:_NSMapEmptyDescribe;

   table->valueCallBacks=NSZoneMalloc(zone,sizeof(NSMapTableValueCallBacks));
   table->valueCallBacks->retain=(valueCallBacks.retain!=NULL)?valueCallBacks.retain:_NSMapEmptyRetain;
   table->valueCallBacks->release=(valueCallBacks.release!=NULL)?valueCallBacks.release:_NSMapEmptyRelease;
   table->valueCallBacks->describe=(valueCallBacks.describe!=NULL)?valueCallBacks.describe:_NSMapEmptyDescribe;

   table->count=0;
   table->nBuckets=(capacity<4)?4:capacity;
   table->buckets=NSZoneCalloc(zone,table->nBuckets,sizeof(NSMapNode *));

   return table;
}

NSMapTable *NSCopyMapTableWithZone(NSMapTable *table,NSZone *zone){
   NSMapTable *newTable=NSCreateMapTableWithZone(*(table->keyCallBacks),
     *(table->valueCallBacks),table->count,zone);
   NSMapEnumerator state=NSEnumerateMapTable(table);
   void *key,*val;

   while(NSNextMapEnumeratorPair(&state,&key,&val))
    NSMapInsert(newTable,key,val);

   return newTable;
}

void NSFreeMapTable(NSMapTable *table){
   NSZone *zone=NSZoneFromPointer(table);
   NSUInteger i;
   NSMapNode *j,*next;

   for(i=0;i<table->nBuckets;i++){
    for(j=table->buckets[i];j!=NULL;j=next){
     table->keyCallBacks->release(table,j->key);
     table->valueCallBacks->release(table,j->value);
     next=j->next;
     NSZoneFree(zone,j);
    }
   }
   NSZoneFree(zone,table->buckets);
   NSZoneFree(zone,table->keyCallBacks);
   NSZoneFree(zone,table->valueCallBacks);
   NSDeallocateObject(table);
}

void NSResetMapTable(NSMapTable *table){
   NSZone *zone=NSZoneFromPointer(table);
   NSUInteger i;
   NSMapNode *j,*next;

   for(i=0;i<table->nBuckets;i++){
    for(j=table->buckets[i];j!=NULL;j=next){
     table->keyCallBacks->release(table,j->key);
     table->valueCallBacks->release(table,j->value);
     next=j->next;
     NSZoneFree(zone,j);
    }
    table->buckets[i]=NULL;
   }
   table->count=0;
}

BOOL NSCompareMapTables(NSMapTable *table1,NSMapTable *table2){
   NSUInteger i;
   NSMapNode *j;

   if(table1->count!=table2->count)
    return NO;

   for(i=0;i<table1->nBuckets;i++)
    for(j=table1->buckets[i];j!=NULL;j=j->next)
     if(NSMapGet(table2,j->key)!=j->key)
      return NO;

   return YES;
}

NSUInteger NSCountMapTable(NSMapTable *table){
   return table->count;
}

BOOL NSMapMember(NSMapTable *table,const void *key,void **originalKey,
   void **value){
   NSUInteger i=table->keyCallBacks->hash(table,key)%table->nBuckets;
   NSMapNode *j;

   for(j=table->buckets[i];j!=NULL;j=j->next)
    if(table->keyCallBacks->isEqual(table,j->key,key)){
     *originalKey=j->key;
     *value=j->value;
     return YES;
    }

   return NO;
}

void *NSMapGet(NSMapTable *table,const void *key){
   NSUInteger i=table->keyCallBacks->hash(table,key)%table->nBuckets;
   NSMapNode *j;

   for(j=table->buckets[i];j!=NULL;j=j->next)
    if(j->key == key || table->keyCallBacks->isEqual(table,j->key,key))
     return j->value;

   return NULL;
}

NSMapEnumerator NSEnumerateMapTable(NSMapTable *table){
   NSMapEnumerator state;

   state.table=table;
   for(state.i=0;state.i<table->nBuckets;state.i++)
    if(table->buckets[state.i]!=NULL)
     break;
   state.j=(state.i<table->nBuckets)?table->buckets[state.i]:NULL;

   return state;
}

BOOL NSNextMapEnumeratorPair(NSMapEnumerator *state,void **key,
   void **value){

   if(state->j==NULL)
    return NO;

   *key=state->j->key;
   *value=state->j->value;

   if((state->j=state->j->next)!=NULL)
    return YES;

   for(state->i++;state->i<state->table->nBuckets;state->i++)
    if((state->j=state->table->buckets[state->i])!=NULL)
     return YES;

   state->j=NULL;

   return YES;
}

NSArray *NSAllMapTableKeys(NSMapTable *table){
   NSMutableArray *array;
   NSUInteger i;
   NSMapNode *j;

   array=[[[NSMutableArray allocWithZone:NULL] initWithCapacity:table->count] autorelease];

   for(i=0;i<table->nBuckets;i++)
    for(j=table->buckets[i];j!=NULL;j=j->next)
     [array addObject:j->key];

   return array;
}

NSArray *NSAllMapTableValues(NSMapTable *table){
   NSMutableArray *array;
   NSUInteger i;
   NSMapNode *j;

   array=[[[NSMutableArray allocWithZone:NULL] initWithCapacity:table->count] autorelease];

   for(i=0;i<table->nBuckets;i++)
    for(j=table->buckets[i];j!=NULL;j=j->next)
     [array addObject:j->value];

   return array;
}

void NSMapInsert(NSMapTable *table,const void *key,const void *value){
   NSZone    *zone;
   NSUInteger   hash=table->keyCallBacks->hash(table,key);
   NSUInteger        i=hash%table->nBuckets;
   NSMapNode *j;

   for(j=table->buckets[i];j!=NULL;j=j->next)
    if(table->keyCallBacks->isEqual(table,j->key,key)){
     void *oldKey=j->key;
     void *oldValue=j->value;

     table->keyCallBacks->retain(table,key);
     table->valueCallBacks->retain(table,value);
     j->key=(void *)key;
     j->value=(void *)value;
     table->keyCallBacks->release(table,oldKey);
     table->valueCallBacks->release(table,oldValue);

     return;
    }
   zone=NSZoneFromPointer(table);

   if(table->count>=table->nBuckets){
    NSUInteger         nBuckets=table->nBuckets;
    NSMapNode **buckets=table->buckets,*next;

    table->nBuckets=nBuckets*2;
    table->buckets=NSZoneCalloc(zone,table->nBuckets,sizeof(NSMapNode *));

    for(i=0;i<nBuckets;i++)
     for(j=buckets[i];j!=NULL;j=next){
      NSUInteger newi=table->keyCallBacks->hash(table,j->key)%table->nBuckets;

      next=j->next;
      j->next=table->buckets[newi];
      table->buckets[newi]=j;
     }
    NSZoneFree(zone,buckets);
    i=hash%table->nBuckets;
   }

   table->keyCallBacks->retain(table,key);
   table->valueCallBacks->retain(table,value);
   j=NSZoneMalloc(zone,sizeof(NSMapNode));
   j->key=(void *)key;
   j->value=(void *)value;
   j->next=table->buckets[i];
   table->buckets[i]=j;
   table->count++;
}

void *NSMapInsertIfAbsent(NSMapTable *table,const void *key,const void *value){
   void *old=NSMapGet(table,key);

   if(old!=NULL)
    return old;
   NSMapInsert(table,key,value);
   return NULL;
}

void NSMapInsertKnownAbsent(NSMapTable *table,const void *key,
   const void *value){
   if(NSMapGet(table,key)!=NULL){
    // FIX
    // [NSException raise:NSInvalidArgumentException format:@"NSMapGet
    //   returned non-nil in NSMapInsertKnownAbsent"];
   }
   NSMapInsert(table,key,value);
}

void NSMapRemove(NSMapTable *table,const void *key){
   NSUInteger i=table->keyCallBacks->hash(table,key)%table->nBuckets;
   NSMapNode *j=table->buckets[i],*prev=j;

   for(;j!=NULL;j=j->next){
    if(table->keyCallBacks->isEqual(table,j->key,key)){
     if(prev==j)
      table->buckets[i]=j->next;
     else
      prev->next=j->next;
     table->keyCallBacks->release(table,j->key);
     table->valueCallBacks->release(table,j->value);
     NSZoneFree(NSZoneFromPointer(j),j);
     table->count--;
     return;
    }
    prev=j;
   }
}

NSString *NSStringFromMapTable(NSMapTable *table){
   NSMutableString *string=[NSMutableString string];
   NSString *fmt=@"%p",*eq=@" = ",*nl=@";\n";
   NSUInteger i;
   NSMapNode *j;

   for(i=0;i<table->nBuckets;i++){
    for(j=table->buckets[i];j!=NULL;j=j->next){
     NSString *desc;

     if((desc=table->keyCallBacks->describe(table,j->key))!=nil)
      [string appendString:desc];
     else
      [string appendFormat:fmt,j->key];
     [string appendString:eq];
     if((desc=table->valueCallBacks->describe(table,j->value))!=nil)
      [string appendString:desc];
     else
      [string appendFormat:fmt,j->value];
     [string appendString:nl];
    }
   }

   return string;
}

+mapTableWithStrongToStrongObjects {
    return [NSCreateMapTable(NSObjectMapKeyCallBacks,NSObjectMapValueCallBacks,0) autorelease];
}

+mapTableWithStrongToWeakObjects {
    return [NSCreateMapTable(NSObjectMapKeyCallBacks,NSNonRetainedObjectMapValueCallBacks,0) autorelease];
}

+mapTableWithWeakToStrongObjects {
    return [NSCreateMapTable(NSNonRetainedObjectMapKeyCallBacks,NSObjectMapValueCallBacks,0) autorelease];
}

+mapTableWithWeakToWeakObjects {
    return [NSCreateMapTable(NSNonRetainedObjectMapKeyCallBacks,NSNonRetainedObjectMapValueCallBacks,0) autorelease];
}

+strongToStrongObjectsMapTable {
    return [self mapTableWithStrongToStrongObjects];
}

+strongToWeakObjectsMapTable {
    return [self mapTableWithStrongToWeakObjects];
}

+weakToStrongObjectsMapTable {
    return [self mapTableWithWeakToStrongObjects];
}

+weakToWeakObjectsMapTable {
    return [self mapTableWithWeakToWeakObjects];
}

-(void)dealloc {
   NSFreeMapTable(self);
   return;
   [super dealloc];
}

-objectForKey:key {
   return NSMapGet(self,key);
}

-(void)removeObjectForKey:key {
   NSMapRemove(self,key);
}

-(void)setObject:object forKey:key {
   NSMapInsert(self,key,object);
}

-(void)removeAllObjects {
   NSResetMapTable(self);
}

-(NSEnumerator *)keyEnumerator {
   return [NSEnumerator_dictionaryKeysNew(self) autorelease];
}

@end
