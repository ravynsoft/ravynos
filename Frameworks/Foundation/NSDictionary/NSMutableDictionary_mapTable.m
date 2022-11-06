/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSMutableDictionary_mapTable.h>

#import <Foundation/NSEnumerator_dictionaryKeys.h>
#import <Foundation/NSAutoreleasePool-private.h>
#import <Foundation/NSString.h>
#import <Foundation/NSException.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSRaiseException.h>

#import <Foundation/CFBaseShim.h>

typedef struct _NSDictNode {
   struct _NSDictNode *next;
   void *key;
   void *value;
} NSDictNode;

typedef struct {
   NSUInteger           _nBuckets;
   struct _NSDictNode **_buckets;
   NSInteger            _i;
   struct _NSDictNode  *_j;
} CFDictionaryEnumerator;

@interface NSEnumerator_CFDictionaryKeys : NSEnumerator {
@public
   CFDictionaryEnumerator _state;
}

-initWithState:(CFDictionaryEnumerator)state;
-nextObject;

@end

@implementation NSEnumerator_CFDictionaryKeys

-initWithState:(CFDictionaryEnumerator)state {
   _state=state;
   return self;
}

BOOL NSNextDictionaryEnumeratorPair(CFDictionaryEnumerator *state,void **key,void **value){

   if(state->_j==NULL)
    return NO;

   *key=state->_j->key;
   *value=state->_j->value;

   if((state->_j=state->_j->next)!=NULL)
    return YES;

   for(state->_i++;state->_i<state->_nBuckets;state->_i++)
    if((state->_j=state->_buckets[state->_i])!=NULL)
     return YES;

   state->_j=NULL;

   return YES;
}

-nextObject {
   void *key,*val;
   return NSNextDictionaryEnumeratorPair(&_state,&key,&val)?key:nil;
}

@end

@implementation NSMutableDictionary_CF

const void *objectRetainCallBack(CFAllocatorRef allocator,const void *value) {
   return CFRetainShim(value);
}

const void *objectCopyCallBack(CFAllocatorRef allocator,const void *value) {
   return [(id <NSCopying>)value copyWithZone:NULL];
}

static void objectReleaseCallBack(CFAllocatorRef allocator,const void *value) {
   CFReleaseShim(value);
}

static CFDictionaryKeyCallBacks objectKeyCallBacks={
 0,objectCopyCallBack,objectReleaseCallBack,CFCopyDescriptionShim,CFEqualShim,CFHashShim,
};

static CFDictionaryValueCallBacks objectValueCallbacks={
 0,objectRetainCallBack,objectReleaseCallBack,CFCopyDescriptionShim,CFEqualShim
};

const void *defaultRetainCallBack(CFAllocatorRef allocator,const void *value) {
   return value;
}

static void defaultReleaseCallBack(CFAllocatorRef allocator,const void *value) {
}


static CFHashCode defaultHashCallBack(const void *value)
{
    return (CFHashCode)value >> 4;
}


static Boolean defaultEqualCallBack(const void *value,const void *other) {
   return (value==other)?TRUE:FALSE;
}

static CFStringRef defaultCopyDescription(const void *value) {
   return (CFStringRef)@"[ UNIMPLEMENTED dictionary value ]";
}

-(NSUInteger)count {
   return _count;
}

-objectForKey:key {
   NSUInteger i=_keyCallBacks.hash(key)%_nBuckets;
   NSDictNode *j;

   for(j=_buckets[i];j!=NULL;j=j->next)
    if(j->key == key || _keyCallBacks.equal(j->key,key))
     return j->value;

   return NULL;
}

static CFDictionaryEnumerator keyEnumeratorState(NSMutableDictionary_CF *self){
  CFDictionaryEnumerator state;

   state._nBuckets=self->_nBuckets;
   state._buckets=self->_buckets;
   for(state._i=0;state._i<state._nBuckets;state._i++)
    if(state._buckets[state._i]!=NULL)
     break;
   state._j=(state._i<state._nBuckets)?state._buckets[state._i]:NULL;
   return state;
}

-(NSEnumerator *)keyEnumerator {
   return [[[NSEnumerator_CFDictionaryKeys allocWithZone:NULL] initWithState:keyEnumeratorState(self)] autorelease];
}

static void NSDictInsert(NSMutableDictionary_CF *self,const void *key,const void *value){
   NSZone    *zone;
   NSUInteger   hash=self->_keyCallBacks.hash(key);
   NSUInteger        i=hash%self->_nBuckets;
   NSDictNode *j;

   for(j=self->_buckets[i];j!=NULL;j=j->next)
    if(self->_keyCallBacks.equal(j->key,key)){
     void *oldKey=j->key;
     void *oldValue=j->value;

     key=self->_keyCallBacks.retain(NULL,key);
     value=self->_valueCallBacks.retain(NULL,value);
     j->key=(void *)key;
     j->value=(void *)value;
     self->_keyCallBacks.release(NULL,oldKey);
     self->_valueCallBacks.release(NULL,oldValue);

     return;
    }

   zone=NSZoneFromPointer(self);

   if(self->_count>=self->_nBuckets){
    NSUInteger         nBuckets=self->_nBuckets;
    NSDictNode **buckets=self->_buckets,*next;

    self->_nBuckets=nBuckets*2;
    self->_buckets=NSZoneCalloc(zone,self->_nBuckets,sizeof(NSDictNode *));

    for(i=0;i<nBuckets;i++)
     for(j=buckets[i];j!=NULL;j=next){
      NSUInteger newi=self->_keyCallBacks.hash(j->key)%self->_nBuckets;

      next=j->next;
      j->next=self->_buckets[newi];
      self->_buckets[newi]=j;
     }
    NSZoneFree(zone,buckets);
    i=hash%self->_nBuckets;
   }

   key=self->_keyCallBacks.retain(NULL,key);
   value=self->_valueCallBacks.retain(NULL,value);
   j=NSZoneMalloc(zone,sizeof(NSDictNode));
   j->key=(void *)key;
   j->value=(void *)value;
   j->next=self->_buckets[i];
   self->_buckets[i]=j;
   self->_count++;
}

static inline void setValueForKey(NSMutableDictionary_CF *self,const void *value,const void *key){
   NSDictInsert(self,key,value);
}

static inline void setObjectForKey(NSMutableDictionary_CF *self,id object,id key){
   if (key==nil){
    NSRaiseException(NSInvalidArgumentException,self,@selector(setObject:forKey:),@"Attempt to insert object with nil key");
    return;
   }
   else if(object==nil){
    NSRaiseException(NSInvalidArgumentException,self,@selector(setObject:forKey:),@"Attempt to insert nil object for key %@", key);
    return;
   }

   setValueForKey(self,object,key);
}

-(void)setObject:object forKey:key {
   setObjectForKey(self,object,key);
}

static void NSDictRemove(NSMutableDictionary_CF *self,const void *key){
   NSUInteger i=self->_keyCallBacks.hash(key)%self->_nBuckets;
   NSDictNode *j=self->_buckets[i],*prev=j;

   for(;j!=NULL;j=j->next){
    if(self->_keyCallBacks.equal(j->key,key)){
     if(prev==j)
      self->_buckets[i]=j->next;
     else
      prev->next=j->next;
     self->_keyCallBacks.release(NULL,j->key);
     self->_valueCallBacks.release(NULL,j->value);
     NSZoneFree(NSZoneFromPointer(j),j);
     self->_count--;
     return;
    }
    prev=j;
   }
}

-(void)removeObjectForKey:key {
   NSDictRemove(self,key);
}

-init {
   return [self initWithObjects:NULL forKeys:NULL count:0];
}

-initWithKeys:(const void **)keys values:(const void **)values count:(NSUInteger)count keyCallBacks:(const CFDictionaryKeyCallBacks *)keyCallBacks valueCallBacks:(const CFDictionaryValueCallBacks *)valueCallBacks {

   _keyCallBacks.hash=(keyCallBacks->hash!=NULL)?keyCallBacks->hash:defaultHashCallBack;
   _keyCallBacks.equal=(keyCallBacks->equal!=NULL)?keyCallBacks->equal:defaultEqualCallBack;
   _keyCallBacks.retain=(keyCallBacks->retain!=NULL)?keyCallBacks->retain:defaultRetainCallBack;
   _keyCallBacks.release=(keyCallBacks->release!=NULL)?keyCallBacks->release:defaultReleaseCallBack;
   _keyCallBacks.copyDescription=(keyCallBacks->copyDescription!=NULL)?keyCallBacks->copyDescription:defaultCopyDescription;

   _valueCallBacks.retain=(valueCallBacks->retain!=NULL)?valueCallBacks->retain:defaultRetainCallBack;
   _valueCallBacks.release=(valueCallBacks->release!=NULL)?valueCallBacks->release:defaultReleaseCallBack;
   _valueCallBacks.copyDescription=(valueCallBacks->copyDescription!=NULL)?valueCallBacks->copyDescription:defaultCopyDescription;

   _count=0;
    _nBuckets=4;
    _buckets=NSZoneCalloc(NULL,_nBuckets,sizeof(NSDictNode *));
    
    NSInteger i;
    
    for(i=0;i<count;i++) {
        if (keys[i]==nil){
            [self autorelease];
            NSRaiseException(NSInvalidArgumentException,self,_cmd,@"Attempt to insert object with nil key");
        }
        else if(values[i]==nil){
            [self autorelease];
            NSRaiseException(NSInvalidArgumentException,self,_cmd,@"Attempt to insert nil object for key %@", keys[i]);
        }
        setValueForKey(self,values[i],keys[i]);
    }

   return self;
}

-initWithObjects:(id *)objects forKeys:(id *)keys count:(NSUInteger)count {
   return [self initWithKeys:(const void **)keys values:(const void **)objects count:count keyCallBacks:&objectKeyCallBacks valueCallBacks:&objectValueCallbacks];
}

-initWithCapacity:(NSUInteger)capacity {
   return [self initWithKeys:NULL values:NULL count:0 keyCallBacks:&objectKeyCallBacks valueCallBacks:&objectValueCallbacks];
}

-(void)dealloc {
   NSZone *zone=NSZoneFromPointer(self);
   NSUInteger i;
   NSDictNode *j,*next;

   for(i=0;i<_nBuckets;i++){
    for(j=self->_buckets[i];j!=NULL;j=next){
     _keyCallBacks.release(NULL,j->key);
     _valueCallBacks.release(NULL,j->value);
     next=j->next;
     NSZoneFree(zone,j);
    }
   }
   NSZoneFree(zone,self->_buckets);
   NSDeallocateObject(self);
   return;
   [super dealloc];
}

-(void)addEntriesFromDictionary:(NSDictionary *)dictionary {
   NSUInteger i,otherCount=[dictionary count];
   id keys[otherCount],objects[otherCount];

   [dictionary getObjects:objects andKeys:keys];

   for(i=0;i<otherCount;i++)
    setObjectForKey(self,objects[i],keys[i]);
}

-(void)getObjects:(id *)objects andKeys:(id *)keys {
   NSInteger i;

   CFDictionaryEnumerator state=keyEnumeratorState(self);

   for(i=0;i<self->_count;i++)
    NSNextDictionaryEnumeratorPair(&state,(void **)&(keys[i]),(void **)&(objects[i]));
}

static NSDictionary *copyWithClassAndZone(NSMutableDictionary_CF *self,Class cls,NSZone *zone){
   void **keys=__builtin_alloca(sizeof(void *)*self->_count);
   void **values=__builtin_alloca(sizeof(void *)*self->_count);

   [self getObjects:(id *)values andKeys:(id *)keys];

   return [[cls alloc] initWithKeys:(const void **)keys values:(const void **)values count:self->_count keyCallBacks:&(self->_keyCallBacks) valueCallBacks:&(self->_valueCallBacks)];
}

-copy {
   return copyWithClassAndZone(self,[NSDictionary_CF class],NULL);
}

-copyWithZone:(NSZone *)zone {
   return copyWithClassAndZone(self,[NSDictionary_CF class],NULL);
}

-mutableCopy {
   return copyWithClassAndZone(self,[NSMutableDictionary_CF class],NULL);
}

-mutableCopyWithZone:(NSZone *)zone {
   return copyWithClassAndZone(self,[NSMutableDictionary_CF class],NULL);
}

@end

@implementation NSDictionary_CF

-(void)setObject:object forKey:key {
   [self doesNotRecognizeSelector:_cmd];
}

-(void)addEntriesFromDictionary:(NSDictionary *)dictionary {
   [self doesNotRecognizeSelector:_cmd];
}

-(void)removeObjectForKey:key {
   [self doesNotRecognizeSelector:_cmd];
}

@end
