/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSHashTable.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>

typedef struct NSHashBucket {
   struct NSHashBucket *next;
   void *key;
} NSHashBucket;

struct NSHashTable {
   NSHashTableCallBacks *callBacks;
   NSUInteger       count;
   NSUInteger       nBuckets;
   NSHashBucket **buckets;
};

NSHashTableCallBacks _NSHashTableFixCallbacks(NSHashTableCallBacks callBacks);

NSHashTable *NSCreateHashTable(NSHashTableCallBacks callBacks,
 NSUInteger capacity) {
   return NSCreateHashTableWithZone(callBacks,capacity,NULL);
}

NSHashTable *NSCreateHashTableWithZone(NSHashTableCallBacks callBacks,
 NSUInteger capacity,NSZone *zone) {
   NSHashTable *table;

   if(zone==NULL)
    zone=NSDefaultMallocZone();

   table=NSZoneMalloc(zone,sizeof(NSHashTable));

   table->callBacks=NSZoneMalloc(zone,sizeof(NSHashTableCallBacks));
   *(table->callBacks)=_NSHashTableFixCallbacks(callBacks);

   table->count=0;
   table->nBuckets=(capacity<4)?4:capacity;
   table->buckets=NSZoneCalloc(zone,table->nBuckets,sizeof(NSHashBucket *));

   return table;
}

NSHashTable *NSCopyHashTableWithZone(NSHashTable *table,NSZone *zone) {
   NSHashTable *newTable=NSCreateHashTableWithZone(*(table->callBacks),
      table->count,zone);
   NSHashEnumerator state=NSEnumerateHashTable(table);
   void *entry;

   while((entry=NSNextHashEnumeratorItem(&state))!=NULL)
    NSHashInsert(newTable,entry);

   return newTable;
}

void NSFreeHashTable(NSHashTable *table) {
   NSZone *zone=NSZoneFromPointer(table);
   NSUInteger i;
   NSHashBucket *j,*next;

   for(i=0;i<table->nBuckets;i++){
    for(j=table->buckets[i];j!=NULL;j=next){
     table->callBacks->release(table,j->key);
     next=j->next;
     NSZoneFree(zone,j);
    }
   }
   NSZoneFree(zone,table->buckets);
   NSZoneFree(zone,table->callBacks);
   NSZoneFree(zone,table);
}

void NSResetHashTable(NSHashTable *table) {
   NSZone *zone=NSZoneFromPointer(table);
   NSUInteger i;
   NSHashBucket *j,*next;

   for(i=0;i<table->nBuckets;i++){
    for(j=table->buckets[i];j!=NULL;j=next){
     table->callBacks->release(table,j->key);
     next=j->next;
     NSZoneFree(zone,j);
    }
    table->buckets[i]=NULL;
   }
   table->count=0;
}

BOOL NSCompareHashTables(NSHashTable *table1,NSHashTable *table2) {
   NSUInteger i;
   NSHashBucket *j;

   if(table1->count!=table2->count)
    return NO;

   for(i=0;i<table1->nBuckets;i++)
    for(j=table1->buckets[i];j!=NULL;j=j->next)
     if(NSHashGet(table2,j->key)!=j->key)
      return NO;

   return YES;
}

NSUInteger NSCountHashTable(NSHashTable *table) {
   return table->count;
}

void *NSHashGet(NSHashTable *table,const void *pointer) {
   NSUInteger i=table->callBacks->hash(table,pointer)%table->nBuckets;
   NSHashBucket *j;

   for(j=table->buckets[i];j!=NULL;j=j->next)
    if(table->callBacks->isEqual(table,j->key,pointer))
     return j->key;

   return NULL;
}

NSArray *NSAllHashTableObjects(NSHashTable *table) {
   NSMutableArray *array;
   NSUInteger i;
   NSHashBucket *j;

   array=[[[NSMutableArray allocWithZone:NULL] initWithCapacity:table->count] autorelease];

   for(i=0;i<table->nBuckets;i++)
    for(j=table->buckets[i];j!=NULL;j=j->next)
     [array addObject:j->key];

   return array;
}

NSHashEnumerator NSEnumerateHashTable(NSHashTable *table) {
   NSHashEnumerator state;

   state.table=table;
   for(state.i=0;state.i<table->nBuckets;state.i++)
    if(table->buckets[state.i]!=NULL)
     break;
   state.j=(state.i<table->nBuckets)?table->buckets[state.i]:NULL;

   return state;
}

void *NSNextHashEnumeratorItem(NSHashEnumerator *state) {
   void *key;

   if(state->j==NULL)
    return NULL;

   key=state->j->key;

   if((state->j=state->j->next)!=NULL)
    return key;

   for(state->i++;state->i<state->table->nBuckets;state->i++)
    if((state->j=state->table->buckets[state->i])!=NULL)
     return key;

   state->j=NULL;

   return key;
}

void NSHashInsert(NSHashTable *table,const void *pointer) {
   NSZone *zone;
   NSUInteger hash=table->callBacks->hash(table,pointer);
   NSUInteger i=hash%table->nBuckets;
   NSHashBucket *j;

   for(j=table->buckets[i];j!=NULL;j=j->next)
    if(table->callBacks->isEqual(table,j->key,pointer)){
     void *old=j->key;
     table->callBacks->retain(table,pointer);
     j->key=(void *)pointer;
     table->callBacks->release(table,old);
     return;
    }

   zone=NSZoneFromPointer(table);

   if(table->count>=table->nBuckets){
    NSUInteger nBuckets=table->nBuckets;
    NSHashBucket **buckets=table->buckets,*next;

    table->nBuckets=nBuckets*2;
    table->buckets=NSZoneCalloc(zone,table->nBuckets,sizeof(NSHashBucket *));
    for(i=0;i<nBuckets;i++)
     for(j=buckets[i];j!=NULL;j=next){
      NSUInteger newi=table->callBacks->hash(table,j->key)%table->nBuckets;
      next=j->next;
      j->next=table->buckets[newi];
      table->buckets[newi]=j;
     }
    NSZoneFree(zone,buckets);
    i=hash%table->nBuckets;
   }

   table->callBacks->retain(table,pointer);
   j=NSZoneMalloc(zone,sizeof(NSHashBucket));
   j->key=(void *)pointer;
   j->next=table->buckets[i];
   table->buckets[i]=j;
   table->count++;
}

void NSHashInsertKnownAbsent(NSHashTable *table,const void *pointer) {
   if(NSHashGet(table,pointer)!=NULL){
    // FIX
    // [NSException raise:NSInvalidArgumentException format:@"NSHashGet
    //   returned non-nil in NSHashInsertKnownAbsent"];
   }
   NSHashInsert(table,pointer);
}

void *NSHashInsertIfAbsent(NSHashTable *table,const void *pointer) {
   void *old=NSHashGet(table,pointer);

   if(old!=NULL)
    return old;
   NSHashInsert(table,pointer);
   return NULL;
}

void NSHashRemove(NSHashTable *table,const void *pointer) {
   NSUInteger i=table->callBacks->hash(table,pointer)%table->nBuckets;
   NSHashBucket *j=table->buckets[i],*prev=j;

   for(;j!=NULL;j=j->next){
    if(table->callBacks->isEqual(table,j->key,pointer)){
     if(prev==j)
      table->buckets[i]=j->next;
     else
      prev->next=j->next;
     table->callBacks->release(table,j->key);
     NSZoneFree(NSZoneFromPointer(j),j);
     table->count--;
     return;
    }
    prev=j;
   }
}

NSString *NSStringFromHashTable(NSHashTable *table) {
   NSMutableString *string=[NSMutableString string];
   NSString *fmt=@"%p";
   NSUInteger i;
   NSHashBucket *j;

   for(i=0;i<table->nBuckets;i++){
    for(j=table->buckets[i];j!=NULL;j=j->next){
     NSString *desc;

     if((desc=table->callBacks->describe(table,j->key))!=nil)
      [string appendString:desc];
     else
      [string appendFormat:fmt,j->key];
    }
   }

   return string;
}

static NSUInteger _NSHashPointerHash(NSHashTable *table,const void *object){
   return (NSUInteger)object>>5;
}

static NSUInteger _NSHashObjectHash(NSHashTable *table,const void *object){
   return [(id)object hash];
}

static BOOL _NSHashPointerIsEqual(NSHashTable *table,const void *object1,
  const void *object2){
   return (object1==object2)?YES:NO;
}

static BOOL _NSHashObjectIsEqual(NSHashTable *table,const void *object1,
  const void *object2){
   return [(id)object1 isEqual:(id)object2];
}

static void _NSHashEmptyRetain(NSHashTable *table,const void *object){
}

static void _NSHashObjectRetain(NSHashTable *table,const void *object){
   [(id)object retain];
}

static void _NSHashEmptyRelease(NSHashTable *table,void *object){
}

static void _NSHashObjectRelease(NSHashTable *table,void *object){
   [(id)object release];
}

static void _NSHashPointerRelease(NSHashTable *table,void *object){
   NSZoneFree(NSZoneFromPointer(object),object);
}

static NSString *_NSHashEmptyDescribe(NSHashTable *table,const void *object){
   return nil;
}

static NSString *_NSHashObjectDescribe(NSHashTable *table,const void *object){
   return [(id)object description];
}

static NSUInteger _NSHashPointerToStructHash(NSHashTable *table,const void *object){
   const struct { NSInteger i; } *ptr=object;
   return (NSUInteger)ptr->i;
}

static BOOL _NSHashPointerToStructIsEqual(NSHashTable *table,const void *object1,
  const void *object2){
   const struct { NSInteger i; } *ptr1=object1,*ptr2=object2;
   return (ptr1->i==ptr2->i)?YES:NO;
}

const NSHashTableCallBacks NSIntHashCallBacks={
 NULL, NULL, NULL, NULL, NULL
};

const NSHashTableCallBacks NSNonOwnedPointerHashCallBacks={
 NULL, NULL, NULL, NULL, NULL
};

const NSHashTableCallBacks NSNonRetainedObjectHashCallBacks={
 _NSHashObjectHash, _NSHashObjectIsEqual, NULL, NULL, _NSHashObjectDescribe
};

const NSHashTableCallBacks NSObjectHashCallBacks={
 _NSHashObjectHash, _NSHashObjectIsEqual, _NSHashObjectRetain, _NSHashObjectRelease, _NSHashObjectDescribe
};

const NSHashTableCallBacks NSOwnedObjectIdentityHashCallBacks={
 _NSHashPointerHash, _NSHashPointerIsEqual, _NSHashObjectRetain, _NSHashObjectRelease, _NSHashObjectDescribe
};

const NSHashTableCallBacks NSOwnedPointerHashCallBacks={
 NULL, NULL, NULL, _NSHashPointerRelease, NULL
};

const NSHashTableCallBacks NSPointerToStructHashCallBacks={
 _NSHashPointerToStructHash, _NSHashPointerToStructIsEqual, NULL, _NSHashPointerRelease, NULL
};

NSHashTableCallBacks _NSHashTableFixCallbacks(NSHashTableCallBacks callBacks){
   if(callBacks.hash==NULL)
    callBacks.hash=_NSHashPointerHash;
   if(callBacks.isEqual==NULL)
    callBacks.isEqual=_NSHashPointerIsEqual;
   if(callBacks.retain==NULL)
    callBacks.retain=_NSHashEmptyRetain;
   if(callBacks.release==NULL)
    callBacks.release=_NSHashEmptyRelease;
   if(callBacks.describe==NULL)
    callBacks.describe=_NSHashEmptyDescribe;

   return callBacks;
}
