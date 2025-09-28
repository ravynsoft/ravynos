/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSInlineSetTable.h>
#import <Foundation/NSString.h>

NSUInteger NSSetTableRoundCount(NSUInteger count) {
   return (count<4)?4:count;
}

void NSSetTableInit(NSSetTable *table,NSUInteger capacity,NSZone *zone) {
   table->count=0;
   table->numBuckets=NSSetTableRoundCount(capacity);
   table->buckets=NSZoneCalloc(zone,table->numBuckets,sizeof(NSSetBucket *));
}

void NSSetTableFreeObjects(NSSetTable *table){
   int i;

   for(i=0;i<table->numBuckets;i++){
    NSSetBucket *current=table->buckets[i],*next;

    for(;current!=NULL;current=next){
     next=current->next;
     [current->key release];
     NSZoneFree(NSZoneFromPointer(current),current);
    }
   }
}

void NSSetTableFreeBuckets(NSSetTable *table){
   NSZoneFree(NSZoneFromPointer(table->buckets),table->buckets);
}

NSSetBucket *NSSetBucketAddObject(NSSetBucket *bucket,id object){
   NSSetBucket *current=bucket;

   for(;current!=NULL;current=current->next)
    if([current->key isEqual:object])
     return NULL;

   current=NSZoneMalloc(NULL,sizeof(NSSetBucket));
   current->next=bucket;
   current->key=[object retain];

   return current;
}

void NSSetTableAddObjectNoGrow(NSSetTable *table,id object){
   NSUInteger     index=[object hash]%table->numBuckets;
   NSSetBucket *bucket;

   if((bucket=NSSetBucketAddObject(table->buckets[index],object))!=NULL){
    table->buckets[index]=bucket;
    table->count++;
   }
}

// add growing
void NSSetTableAddObject(NSSetTable *table,id object){
   NSUInteger     index=[object hash]%table->numBuckets;
   NSSetBucket *bucket;

   if((bucket=NSSetBucketAddObject(table->buckets[index],object))!=NULL){
    table->buckets[index]=bucket;
    table->count++;
   }
}


id NSSetTableMember(NSSetTable *table,id object) {
   NSUInteger     index=[object hash]%table->numBuckets;
   NSSetBucket *current,*bucket=table->buckets[index];

   for(current=bucket;current!=NULL;current=current->next)
    if([current->key isEqual:object])
     return current->key;

   return nil;
}

// add shrinking
void NSSetTableRemoveObject(NSSetTable *table,id object){
   NSUInteger     index=[object hash]%table->numBuckets;
   NSSetBucket *current,*last,*bucket=table->buckets[index];

	// Make sure the object lives through the operation
	[object retain];
   for(current=last=bucket;current!=NULL;last=current,current=current->next)
    if([current->key isEqual:object]){
     if(last==current)
      table->buckets[index]=current->next;
     else
      last->next=current->next;

     table->count--;
     [current->key release];
     NSZoneFree(NSZoneFromPointer(current),current);
     break;
    }
	[object release];
}

NSUInteger NSSetTableObjectCount(NSSetTable *table,id object) {
   NSUInteger     index=[object hash]%table->numBuckets;
   NSCountBucket *current,*bucket=(NSCountBucket *)table->buckets[index];

   for(current=bucket;current!=NULL;current=current->next)
    if([current->key isEqual:object])
     return current->count;

   return 0;
}

NSCountBucket *NSSetBucketAddObjectCount(NSCountBucket *bucket,id object){
   NSCountBucket *current=bucket;

   for(;current!=NULL;current=current->next)
    if([current->key isEqual:object]){
     current->count++;
     return NULL;
    }

   current=NSZoneMalloc(NULL,sizeof(NSCountBucket));
   current->next=bucket;
   current->key=[object retain];
   current->count=1;

   return current;
}

void NSSetTableAddObjectCount(NSSetTable *table,id object) {
   NSUInteger       index=[object hash]%table->numBuckets;
   NSCountBucket *bucket=(NSCountBucket *)table->buckets[index];

   if((bucket=NSSetBucketAddObjectCount(bucket,object))!=NULL){
    table->buckets[index]=(NSSetBucket *)bucket;
    table->count++;
   }

}

void NSSetTableRemoveObjectCount(NSSetTable *table,id object){
   NSUInteger     index=[object hash]%table->numBuckets;
   NSCountBucket *current,*last,*bucket=(NSCountBucket *)table->buckets[index];

   for(current=last=bucket;current!=NULL;last=current,current=current->next)
    if([current->key isEqual:object]){
     current->count--;
     if(current->count==0){
      if(last==current)
       table->buckets[index]=(NSSetBucket *)current->next;
      else
       last->next=current->next;

      table->count--;
      [current->key release];
      NSZoneFree(NSZoneFromPointer(current),current);
     }
     break;
    }
}
