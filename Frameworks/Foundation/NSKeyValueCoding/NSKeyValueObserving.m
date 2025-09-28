/* Copyright (c) 2007-2008 Johannes Fortmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSKeyValueObserving.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSLock.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSException.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSKeyValueCoding.h>
#import <Foundation/NSMethodSignature.h>
#import <Foundation/NSIndexSet.h>
#import <Foundation/NSMapTable.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSNull.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSUserDefaults.h>

#import <objc/runtime.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

#import "NSString+KVCAdditions.h"
#import "NSKeyValueObserving-Private.h"
#import "NSKVOInfoPerObject.h"
#import "NSKeyPathObserver.h"
#import "NSKeyObserver.h"

NSString *const NSKeyValueChangeKindKey=@"kind"; // do not change value
NSString *const NSKeyValueChangeNewKey=@"NSKeyValueChangeNewKey";
NSString *const NSKeyValueChangeOldKey=@"NSKeyValueChangeOldKey";
NSString *const NSKeyValueChangeIndexesKey=@"NSKeyValueChangeIndexesKey";
NSString *const NSKeyValueChangeNotificationIsPriorKey=@"NSKeyValueChangeNotificationIsPriorKey";

NSString *const _KVO_DependentKeysTriggeringChangeNotification=@"_KVO_DependentKeysTriggeringChangeNotification";
NSString *const _KVO_KeyPathsForValuesAffectingValueForKey=@"_KVO_KeyPathsForValuesAffectingValueForKey";

static pthread_mutex_t kvoLock=PTHREAD_MUTEX_INITIALIZER;

int NSKeyValueDebugLogLevel = 0;

void NSDetermineKeyValueDebugLoggingLevel()
{
	static BOOL loggingLevelDetermined = NO;
	if (loggingLevelDetermined == NO) {
		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
		NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];

		NSKeyValueDebugLogLevel = [defaults integerForKey: @"NSKeyValueDebugLogLevel"];
		if (NSKeyValueDebugLogLevel > 0) {
			NSLog(@"set NSKeyValueDebugLevel to: '%d'", NSKeyValueDebugLogLevel);
		}
		[pool drain];
		loggingLevelDetermined = YES;
	}
}


@interface NSObject (KVOSettersForwardReferencs)
+(NSDictionary *)_KVO_buildDependencyUnion;
@end

@interface NSObject (KVCPrivateMethod)
-(void)_demangleTypeEncoding:(const char*)type to:(char*)cleanType;
@end

@implementation NSObject (KeyValueObserving)

static pthread_mutex_t masterObservationLock=PTHREAD_MUTEX_INITIALIZER;

static inline NSMapTable *masterObservationInfo(){
   static NSMapTable *observationInfos=NULL;

   if(observationInfos==NULL)
    observationInfos=NSCreateMapTable(NSNonOwnedPointerMapKeyCallBacks,NSNonOwnedPointerMapValueCallBacks,0);

   return observationInfos;
}

-(void *)observationInfo {
   void *result;

   pthread_mutex_lock(&masterObservationLock);

   result=NSMapGet(masterObservationInfo(),self);

   pthread_mutex_unlock(&masterObservationLock);

   return result;
}

-(void)setObservationInfo:(void *)info {
   pthread_mutex_lock(&masterObservationLock);

   if(info==NULL)
    NSMapRemove(masterObservationInfo(),self);
   else
    NSMapInsert(masterObservationInfo(),self,info);

   pthread_mutex_unlock(&masterObservationLock);
}

+(void *)observationInfo {
   void *result;

   pthread_mutex_lock(&masterObservationLock);

   result=NSMapGet(masterObservationInfo(),self);

   pthread_mutex_unlock(&masterObservationLock);

   return result;
}

+(void)setObservationInfo:(void *)info {
   pthread_mutex_lock(&masterObservationLock);

   if(info==NULL)
    NSMapRemove(masterObservationInfo(),self);
   else
    NSMapInsert(masterObservationInfo(),self,info);

   pthread_mutex_unlock(&masterObservationLock);
}

static void addKeyObserver(NSKeyObserver *keyObserver){
	NSKeyValueDebugLog(kNSKeyValueDebugLevel3, @"addKeyObserver: %@",keyObserver);

   id object=[keyObserver object];

   [object _KVO_swizzle];

   NSKVOInfoPerObject *observationInfo=[object observationInfo];


   if(observationInfo==nil){
    observationInfo=[[NSKVOInfoPerObject allocWithZone:NULL] init];
    [object setObservationInfo:observationInfo];
   }

   [observationInfo addKeyObserver:keyObserver];
}

static void removeKeyObserver(NSKeyObserver *keyObserver){
//	NSKeyValueDebugLog(kNSKeyValueDebugLevel3,@"removeKeyObserver:%@",keyObserver);

   if(keyObserver==nil)
    return;

   [keyObserver invalidate];

   id                  object=[keyObserver object];
   NSKVOInfoPerObject *observationInfo=[object observationInfo];

   [observationInfo removeKeyObserver:keyObserver];

   if([observationInfo isEmpty]){
    [object setObservationInfo:NULL];
    [observationInfo release];
   }
}

static NSKeyObserver *keyObserverForObserverAndKeyPath(id object,id observer,NSString *path){
   NSKVOInfoPerObject *observationInfo=[object observationInfo];
   NSString           *restOfPath;
   NSString           *key=_NSKVOSplitKeyPath(path,&restOfPath);

   NSArray *observers=[observationInfo keyObserversForKey:key];

   for(NSKeyObserver *check in observers){
    NSKeyPathObserver *keyPathObserver=[check keyPathObserver];

    if([keyPathObserver observer]==observer && [[keyPathObserver keyPath] isEqualToString:path])
     return check;
   }

   return nil;
}


static NSKeyObserver *addKeyPathObserverToObject(id object,NSString *path,NSKeyPathObserver *keyPathObserver);

static NSArray *addKeyPathObserverToDependantPaths(id object,NSSet *dependentPaths,NSKeyPathObserver *keyPathObserver){
    NSMutableArray *result=[NSMutableArray array];

    for(NSString *path in dependentPaths){
		// Recursively walk the keypath setting up key/keyPath observer pairs for each dependentPath.
		 NSKeyObserver *check=addKeyPathObserverToObject(object,path,keyPathObserver);

		 if(check!=nil)
		  [result addObject:check];
    }

    return result;
}

static void addKeyObserverDependantsAndRestOfPath(NSKeyObserver *keyObserver){
   id        object=[keyObserver object];
   NSString *key=[keyObserver key];
   NSString *restOfPath=[keyObserver restOfPath];
   NSKeyPathObserver *keyPathObserver=[keyObserver keyPathObserver];

   NSArray *dependantObservers=addKeyPathObserverToDependantPaths(object,[[object class] keyPathsForValuesAffectingValueForKey:key],keyPathObserver);

   [keyObserver setDependantKeyObservers:dependantObservers];

   if(restOfPath!=nil) {
	   // Recursively walk the keypath setting up key/keyPath observer pairs for each link in the chain.
	   // [object valueForKey: key] makes sure the observer pair is associated with the correct object.
	   NSKeyObserver *restOfPathObserver=addKeyPathObserverToObject([object valueForKey:key],restOfPath,keyPathObserver);

	   [keyObserver setRestOfPathObserver:restOfPathObserver];
   }
}

static NSKeyObserver *addKeyPathObserverToObject(id object,NSString *path,NSKeyPathObserver *keyPathObserver){
   if(object==nil)
    return nil;

   NSString *restOfPath;
   NSString *key=_NSKVOSplitKeyPath(path,&restOfPath);

   if([key hasPrefix:@"@"]){
    NSUnimplementedFunction();
     // FIXME: operator, ignore?
   }

   NSKeyObserver *keyObserver=[[[NSKeyObserver alloc] initWithObject:object key:key keyPathObserver:keyPathObserver restOfPath:restOfPath] autorelease];

   addKeyObserverDependantsAndRestOfPath(keyObserver);

   addKeyObserver(keyObserver);

/* FIXME: unwind logic if an exception is encountered
 */

   return keyObserver;
}

/*
 * addObserver:forKeyPath:options:context:
 * 1. Creates a keyPathObserver to track the designated keypath for self
 * 2. Creates a keyObserver for the first key in the path and connects it to the freshly created keyPathObserver
 * 3. Creates keyObserver and keyPathObservers for all dependent keys and the rest of the key path (which for the keyPath recursively calls this set of
 *       operations - but using the current value of the key in the keypath as the root object)
 * 4. Adds the keyObserver to the object - which means
 * 4.1 Swizzles object into a KVO capable class (if it's not already)
 * 4.2 Creates and sets a NSKVOInfoPerObject (if one is not available already) onto the object
 * 4.3 Adds the keyObserver to the info
 */

-(void)addObserver:observer forKeyPath:(NSString *)keyPath options:(NSKeyValueObservingOptions)options context:(void *)context {

	NSKeyValueDebugLog(kNSKeyValueDebugLevel1, @"self: %@ observer: %@, keyPath: %@", self, observer, keyPath);

   NSKeyPathObserver *keyPathObserver=[[[NSKeyPathObserver alloc] initWithObject:self observer:observer keyPath:keyPath options:options context:context] autorelease];

   addKeyPathObserverToObject(self,keyPath,keyPathObserver);

   if(options&NSKeyValueObservingOptionInitial){
    NSUnimplementedMethod();
#if 0
// this is wrong, should generate a clean notification
    [self willChangeValueForKey:key];
    [self didChangeValueForKey:key];
#endif
   }
	NSKeyValueDebugLog(kNSKeyValueDebugLevel2, @"self: %@ added keyPathObserver: %@ for observer: %@ keyPath: %@", self, keyPathObserver, observer, keyPath);
}

static void pruneKeyObserver(NSKeyObserver *keyObserver);

static void pruneRestOfPathAndDependantObservers(NSKeyObserver *keyObserver){
//NSLog(@"pruneRestOfPathAndDependantObservers %@",keyObserver);
   pruneKeyObserver([keyObserver restOfPathObserver]);

   [keyObserver setRestOfPathObserver:nil];

   for(NSKeyObserver *dep in [keyObserver dependantKeyObservers])
    pruneKeyObserver(dep);

   [keyObserver setDependantKeyObservers:nil];
}

static void pruneKeyObserver(NSKeyObserver *keyObserver){
	if(keyObserver==nil) {
		return;
	}

   pruneRestOfPathAndDependantObservers(keyObserver);
   removeKeyObserver(keyObserver);
}

-(void)removeObserver:observer forKeyPath:(NSString *)keyPath {
   NSKeyObserver *keyObserver=keyObserverForObserverAndKeyPath(self,observer,keyPath);

   pruneKeyObserver(keyObserver);
}

static void willChangeValueForKey(id object,NSString *key,NSDictionary *changeInfo) {

	NSKeyValueDebugLog(kNSKeyValueDebugLevel3, @"object: %@, key: %@", object, key);

   NSKVOInfoPerObject *observationInfo=[object observationInfo];

	if(observationInfo==nil) {
		NSKeyValueDebugLog(kNSKeyValueDebugLevel3, @"observationInfo is nil - so bailing");
		return;
	}

   NSArray *keyObserversArray=[NSArray arrayWithArray:[observationInfo keyObserversForKey:key]];
   NSInteger count=[keyObserversArray count];
	if (count > 0) {
		NSKeyValueDebugLog(kNSKeyValueDebugLevel2, @"notifying %d observers of change to keyPath: %@ which are: %@", count, key, keyObserversArray);
	}
// Cocoa does notifications in this order, last to first
   while(--count>=0){
    NSKeyObserver *keyObserver=[keyObserversArray objectAtIndex:count];

	   if(![keyObserver isValid]) {
		   NSKeyValueDebugLog(kNSKeyValueDebugLevel3, @"skipping invalid keyObserver: %@", keyObserver);
		   continue;
	   }

    NSKeyPathObserver *keyPathObserver=[keyObserver keyPathObserver];
    NSKeyValueObservingOptions observingOptions=[keyPathObserver options];

	   if([keyPathObserver willChangeAlreadyChanging]) {
		   NSKeyValueDebugLog(kNSKeyValueDebugLevel3, @"skipping keyObserver: %@ as already changing", keyObserver);
		   continue;
	   }

    id                   rootObject=[keyPathObserver object];
    id                   rootObserver=[keyPathObserver observer];
    NSString            *rootKeyPath=[keyPathObserver keyPath];
    //unused
    //void                *rootContext=[keyPathObserver context];
    NSMutableDictionary *changeDictionary=[keyPathObserver changeDictionaryWithInfo:changeInfo];

    if(observingOptions&NSKeyValueObservingOptionOld && ![changeDictionary objectForKey:NSKeyValueChangeOldKey]){
     NSIndexSet *idxs=[changeInfo objectForKey:NSKeyValueChangeIndexesKey];

     if(idxs==nil)
      [changeDictionary setValue:[rootObject valueForKeyPath:rootKeyPath] forKey:NSKeyValueChangeOldKey];
     else {
// FIXME: this is wrong, the type of change will depend on the position in the key path
      int type=[[changeDictionary objectForKey:NSKeyValueChangeKindKey] intValue];

      // for to-many relationships, oldvalue is only sensible for replace and remove
      if(type == NSKeyValueChangeReplacement || type == NSKeyValueChangeRemoval)
       [changeDictionary setValue:[[object mutableArrayValueForKeyPath:rootKeyPath] objectsAtIndexes:idxs] forKey:NSKeyValueChangeOldKey];
     }
    }

    if(observingOptions&NSKeyValueObservingOptionPrior) {
     [changeDictionary setObject:[NSNumber numberWithBool:YES] forKey:NSKeyValueChangeNotificationIsPriorKey];

	 NSKeyValueDebugLog(kNSKeyValueDebugLevel2, @"informing observer: %@ prior to change: %@ inKeyPath: %@", rootObserver, changeDictionary, rootKeyPath);
     [rootObserver observeValueForKeyPath:rootKeyPath ofObject:rootObject change:changeDictionary context:[keyPathObserver context]];

     [changeDictionary removeObjectForKey:NSKeyValueChangeNotificationIsPriorKey];
    }

    pruneRestOfPathAndDependantObservers(keyObserver);
   }
}

-(BOOL)_hasObserverForKey:(NSString*)key
{
	NSKVOInfoPerObject *observationInfo=[self observationInfo];
	if(observationInfo==nil) {
		return NO;
	}
	NSArray *keyObserversArray=[observationInfo keyObserversForKey:key];
	NSInteger count=[keyObserversArray count];
	return count > 0;
}

-(void)willChangeValueForKey:(NSString *)key {
	NSMutableDictionary *changeInfo=[[NSMutableDictionary allocWithZone:NULL] init];
	[changeInfo setObject:[NSNumber numberWithInt:NSKeyValueChangeSetting] forKey:NSKeyValueChangeKindKey];
    willChangeValueForKey(self,key,changeInfo);
	[changeInfo release];
}

-(void)willChange:(NSKeyValueChange)change valuesAtIndexes:(NSIndexSet *)indexes forKey:(NSString *)key {
	NSMutableDictionary *changeInfo=[[NSMutableDictionary allocWithZone:NULL] init];

	[changeInfo setObject:[NSNumber numberWithUnsignedInteger:change] forKey:NSKeyValueChangeKindKey];
	[changeInfo setObject:indexes forKey:NSKeyValueChangeIndexesKey];

    willChangeValueForKey(self,key,changeInfo);

	[changeInfo release];
}

-(void)willChangeValueForKey:(NSString *)key withSetMutation:(NSKeyValueSetMutationKind)mutation usingObjects:(NSSet*)objects {
    NSMutableSet* changeSet;
    NSMutableDictionary* changeInfo=[[NSMutableDictionary allocWithZone:NULL] init];
    
    switch (mutation) {
        case NSKeyValueUnionSetMutation:
            changeSet = [objects mutableCopy];
            [changeSet minusSet:[self valueForKey:key]];
            [changeInfo setValue:changeSet forKey:NSKeyValueChangeNewKey];
            [changeInfo setValue:[NSSet set] forKey:NSKeyValueChangeOldKey];
            [changeSet release];
            break;
        case NSKeyValueMinusSetMutation:
            changeSet = [objects mutableCopy];
            [changeSet intersectSet:[self valueForKey:key]];
            [changeInfo setValue:changeSet forKey:NSKeyValueChangeOldKey];
            [changeInfo setValue:[NSSet set] forKey:NSKeyValueChangeNewKey];
            [changeSet release];
            break;
        case NSKeyValueIntersectSetMutation:
            changeSet = [[self valueForKey:key] mutableCopy];
            [changeSet minusSet:objects];
            [changeInfo setValue:changeSet forKey:NSKeyValueChangeOldKey];
            [changeInfo setValue:[NSSet set] forKey:NSKeyValueChangeNewKey];
            [changeSet release];
            break;
        case NSKeyValueSetSetMutation:
            [changeInfo setValue:[self valueForKey:key] forKey:NSKeyValueChangeOldKey];
            [changeInfo setValue:objects forKey:NSKeyValueChangeNewKey];
            break;
    }
    
    willChangeValueForKey(self,key,changeInfo);
    
    [changeInfo release];
}

static void didChangeValueForKey(id object,NSString *key)  {
	NSKeyValueDebugLog(kNSKeyValueDebugLevel3, @"object: %@, key: %@", object, key);

	NSKVOInfoPerObject *observationInfo=[object observationInfo];

	if(observationInfo==nil) {
		NSKeyValueDebugLog(kNSKeyValueDebugLevel3, @"observationInfo is nil - so bailing");
		return;
	}

	NSArray *keyObserversArray=[NSArray arrayWithArray:[observationInfo keyObserversForKey:key]];
	NSInteger count=[keyObserversArray count];
	if (count > 0) {
		NSKeyValueDebugLog(kNSKeyValueDebugLevel2, @"notifying %d observers of change to keyPath: %@ which are: %@", count, key, keyObserversArray);
	}

// Cocoa does notifications in this order, last to first
   while(--count>=0){
    NSKeyObserver *keyObserver=[keyObserversArray objectAtIndex:count];

	   if(![keyObserver isValid]) {
		   NSKeyValueDebugLog(kNSKeyValueDebugLevel3, @"skipping invalid keyObserver: %@", keyObserver);
		   continue;
	   }

    NSKeyPathObserver   *keyPathObserver=[keyObserver keyPathObserver];
    NSKeyValueObservingOptions observerOptions=[keyPathObserver options];

	   if([keyPathObserver didChangeAlreadyChanging]) {
		NSKeyValueDebugLog(kNSKeyValueDebugLevel3, @"skipping keyObserver: %@ as already changing", keyObserver);
	   continue;
   }

    id                   rootObject=[keyPathObserver object];
    id                   rootObserver=[keyPathObserver observer];
    NSString            *rootKeyPath=[keyPathObserver keyPath];
    //unused
    //void                *rootContext=[keyPathObserver context];
    NSMutableDictionary *changeDictionary=[keyPathObserver changeDictionary];

    if(observerOptions&NSKeyValueObservingOptionNew && ![changeDictionary objectForKey:NSKeyValueChangeNewKey]){
     NSIndexSet *idxs=[changeDictionary objectForKey:NSKeyValueChangeIndexesKey];

     if(idxs==nil)
      [changeDictionary setValue:[rootObject valueForKeyPath:rootKeyPath] forKey:NSKeyValueChangeNewKey];
     else {
      int type=[[changeDictionary objectForKey:NSKeyValueChangeKindKey] intValue];
				// for to-many relationships, newvalue is only sensible for replace and insert

      if(type==NSKeyValueChangeReplacement || type==NSKeyValueChangeInsertion)
       [changeDictionary setValue:[[rootObject mutableArrayValueForKeyPath:rootKeyPath] objectsAtIndexes:idxs] forKey:NSKeyValueChangeNewKey];
      }
    }

    addKeyObserverDependantsAndRestOfPath(keyObserver);

	   NSKeyValueDebugLog(kNSKeyValueDebugLevel2, @"informing observer: %@ after change: %@ inKeyPath: %@", rootObserver, changeDictionary, rootKeyPath);
    [rootObserver observeValueForKeyPath:rootKeyPath ofObject:rootObject change:changeDictionary context:[keyPathObserver context]];
    [keyPathObserver clearChangeDictionary];
   }
}

-(void)didChangeValueForKey:(NSString *)key {
   didChangeValueForKey(self,key);
}

-(void)didChange:(NSKeyValueChange)change valuesAtIndexes:(NSIndexSet *)indexes forKey:(NSString *)key {
   didChangeValueForKey(self,key);
}

-(void)didChangeValueForKey:(NSString *)key withSetMutation:(NSKeyValueSetMutationKind)mutation usingObjects:(NSSet*)objects {
    didChangeValueForKey(self,key);
}

+(void)setKeys:(NSArray *)keys triggerChangeNotificationsForDependentKey:(NSString *)dependentKey {
   NSKVOInfoPerObject* observationInfo=[self observationInfo];

   if(!observationInfo) {
    observationInfo=[[NSKVOInfoPerObject allocWithZone:NULL] init];
    [self setObservationInfo:observationInfo];
   }

   NSMutableDictionary *dependencies=[observationInfo objectForKey:_KVO_DependentKeysTriggeringChangeNotification];
   if(!dependencies){
    dependencies=[NSMutableDictionary dictionary];
    [observationInfo setObject:dependencies forKey:_KVO_DependentKeysTriggeringChangeNotification];
   }

   for(id key in keys){
    NSMutableSet* allDependencies=[dependencies objectForKey:key];

    if(!allDependencies){
     allDependencies=[NSMutableSet new];
     [dependencies setObject:allDependencies forKey:key];
     [allDependencies release];
    }
    [allDependencies addObject:dependentKey];
   }
}

static SEL selectorForKeyPathsForValuesAffecting(NSString *key){
   const char *prefix="keyPathsForValuesAffecting";
   char keyCString[[key length]+1];
   char buffer[strlen(prefix)+strlen(keyCString)+1];

   [key getCString:keyCString];
   keyCString[0]=toupper(keyCString[0]);
   strcpy(buffer,prefix);
   strcat(buffer,keyCString);

   SEL       result=sel_getUid(buffer);

   return result;
}

+(NSSet *)keyPathsForValuesAffectingValueForKey:(NSString *)key {
   SEL       sel=selectorForKeyPathsForValuesAffecting(key);
   NSSet    *result=nil;

   if([self respondsToSelector:sel])
    result=[self performSelector:sel];
   else {
    NSKVOInfoPerObject *observationInfo=[self observationInfo];
    NSDictionary *keyPathsByKey=[observationInfo objectForKey:_KVO_KeyPathsForValuesAffectingValueForKey];

    if(keyPathsByKey==nil){
     keyPathsByKey=[self _KVO_buildDependencyUnion];
    }

    result=[keyPathsByKey objectForKey:key];
   }

   return result;
}
@end


/* The following functions define suitable setters and getters which
 call willChangeValueForKey: and didChangeValueForKey: on their superclass
 _KVO_swizzle changes the class of its object to a subclass which overrides
 each setter with a suitable KVO-Notifying one.
*/

// selector for change type
#define CHANGE_SELECTOR(type) KVO_notifying_change_ ## type :

// definition for change type
#define CHANGE_DEFINE(type) -( void ) KVO_notifying_change_ ## type : ( type ) value

// original selector called by swizzled selector
#define ORIGINAL_SELECTOR(name) NSSelectorFromString([NSString stringWithFormat:@"_original_%@", name])

// declaration of change function:
// extracts key from selector called, calls original function
#define CHANGE_DECLARATION(type) CHANGE_DEFINE(type) \
{ \
	const char* origName = sel_getName(_cmd); \
	size_t selLen=strlen(origName); \
	char *sel=__builtin_alloca(selLen+1); \
	strcpy(sel, origName); \
	sel[selLen-1]='\0'; \
	if(sel[0]=='_') \
		sel+=4; \
	else \
		sel+=3; \
	sel[0]=tolower(sel[0]); \
	NSString *key=[[NSString allocWithZone:NULL] initWithCString:sel]; \
	BOOL shouldNotify = [self _hasObserverForKey:key];\
	if (shouldNotify) [self willChangeValueForKey:key]; \
	typedef id (*sender)(id obj, SEL selector, type value); \
	sender implementation=(sender)[[self superclass] instanceMethodForSelector:_cmd]; \
	(void)*implementation(self, _cmd, value); \
	if (shouldNotify) [self didChangeValueForKey:key]; \
	[key release]; \
}


// FIX: add more types
@interface NSObject (KVOSetters)
CHANGE_DEFINE(float);
CHANGE_DEFINE(double);
CHANGE_DEFINE(id);
CHANGE_DEFINE(int);
CHANGE_DEFINE(NSSize);
CHANGE_DEFINE(NSPoint);
CHANGE_DEFINE(NSRect);
CHANGE_DEFINE(NSRange);
CHANGE_DEFINE(char);
CHANGE_DEFINE(long);
CHANGE_DEFINE(SEL);
@end

@implementation NSObject (KVOSetters)
CHANGE_DECLARATION(float)
CHANGE_DECLARATION(double)
CHANGE_DECLARATION(id)
CHANGE_DECLARATION(int)
CHANGE_DECLARATION(NSSize)
CHANGE_DECLARATION(NSPoint)
CHANGE_DECLARATION(NSRect)
CHANGE_DECLARATION(NSRange)
CHANGE_DECLARATION(char)
CHANGE_DECLARATION(long)
CHANGE_DECLARATION(SEL)

-(void)KVO_notifying_change_setObject:(id)object forKey:(NSString*)key {
	BOOL shouldNotify = [self _hasObserverForKey:key];
	if (shouldNotify) {
		[self willChangeValueForKey:key];
	}
	typedef id (*sender)(id obj, SEL selector, id object, id key);
	sender implementation=(sender)[[self superclass] instanceMethodForSelector:_cmd];
	implementation(self, _cmd, object, key);
	if (shouldNotify) {
		[self didChangeValueForKey:key];
	}
}

-(void)KVO_notifying_change_removeObjectForKey:(NSString*)key {
	BOOL shouldNotify = [self _hasObserverForKey:key];
	if (shouldNotify) {
		[self willChangeValueForKey:key];
	}
	typedef id (*sender)(id obj, SEL selector, id key);
	sender implementation=(sender)[[self superclass] instanceMethodForSelector:_cmd];
	implementation(self, _cmd, key);
	if (shouldNotify) {
		[self didChangeValueForKey:key];
	}
}


-(void)KVO_notifying_change_insertObject:(id)object inKeyAtIndex:(NSInteger)index {
	const char* origName = sel_getName(_cmd);

	size_t selLen=strlen(origName);
	char *sel=__builtin_alloca(selLen+1);
	strcpy(sel, origName);
	sel[selLen-1]='\0';
	sel+=strlen("insertObject:in");
	sel[strlen(sel)-strlen("AtIndex:")+1]='\0';

	sel[0]=tolower(sel[0]);
	NSString *key=[[NSString allocWithZone:NULL] initWithCString:sel];

	[self willChange:NSKeyValueChangeInsertion valuesAtIndexes:[NSIndexSet indexSetWithIndex:index] forKey:key];
	typedef id (*sender)(id obj, SEL selector, id value, NSInteger index);
	sender implementation=(sender)[[self superclass] instanceMethodForSelector:_cmd];
	(void)*implementation(self, _cmd, object, index);
	[self didChange:NSKeyValueChangeInsertion valuesAtIndexes:[NSIndexSet indexSetWithIndex:index] forKey:key];
	[key release];
}

-(void)KVO_notifying_change_insertKey:(NSArray*)objects atIndexes:(NSIndexSet*)indexes {
	const char* origName = sel_getName(_cmd);
    
	size_t selLen=strlen(origName);
	char *sel=__builtin_alloca(selLen+1);
	strcpy(sel, origName);
	sel[selLen-1]='\0';
	sel+=strlen("insert");
	sel[strlen(sel)-strlen(":atIndexes:")+1]='\0';
    
	sel[0]=tolower(sel[0]);
	NSString *key=[[NSString allocWithZone:NULL] initWithCString:sel];
    
	[self willChange:NSKeyValueChangeInsertion valuesAtIndexes:indexes forKey:key];
	typedef id (*sender)(id obj, SEL selector, NSArray* value, NSIndexSet* indexes);
	sender implementation=(sender)[[self superclass] instanceMethodForSelector:_cmd];
	(void)*implementation(self, _cmd, objects, indexes);
	[self didChange:NSKeyValueChangeInsertion valuesAtIndexes:indexes forKey:key];
	[key release];
}

-(void)KVO_notifying_change_addKeyObject:(id)object {
    const char* origName = sel_getName(_cmd);
    
	size_t selLen=strlen(origName);
	char *sel=__builtin_alloca(selLen+1);
	strcpy(sel, origName);
	sel[selLen-1]='\0';
	sel+=strlen("add");
	sel[strlen(sel)-strlen("Object:")+1]='\0';
    
    char *countSelName=__builtin_alloca(strlen(sel)+strlen("countOf")+1);
    strcpy(countSelName, "countOf");
    strcat(countSelName, sel);
    
    NSUInteger idx=(NSUInteger)[self performSelector:sel_getUid(countSelName)];
    
    sel[0]=tolower(sel[0]);
    
    NSString *key=[[NSString allocWithZone:NULL] initWithCString:sel];
    [self willChange:NSKeyValueChangeInsertion valuesAtIndexes:[NSIndexSet indexSetWithIndex:idx] forKey:key];
    typedef id (*sender)(id obj, SEL selector, id value);
    sender implementation=(sender)[[self superclass] instanceMethodForSelector:_cmd];
    (void)*implementation(self, _cmd, object);
    [self didChange:NSKeyValueChangeInsertion valuesAtIndexes:[NSIndexSet indexSetWithIndex:idx] forKey:key];
    [key release];
}

-(void)KVO_notifying_change_addKey:(NSSet*)objects {
    const char* origName = sel_getName(_cmd);
    
	size_t selLen=strlen(origName);
	char *sel=__builtin_alloca(selLen+1);
	strcpy(sel, origName);
	sel[selLen-1]='\0';
	sel+=strlen("add");
	sel[strlen(sel)-strlen(":")+1]='\0';
    
	sel[0]=tolower(sel[0]);
	NSString *key=[[NSString allocWithZone:NULL] initWithCString:sel];
    
    [self willChangeValueForKey:key withSetMutation:NSKeyValueUnionSetMutation usingObjects:objects];
	typedef id (*sender)(id obj, SEL selector, NSSet* objects);
	sender implementation=(sender)[[self superclass] instanceMethodForSelector:_cmd];
	(void)*implementation(self, _cmd, objects);
    [self didChangeValueForKey:key withSetMutation:NSKeyValueUnionSetMutation usingObjects:objects];
	[key release];
}

-(void)KVO_notifying_change_removeKeyObject:(id)object {
    const char* origName = sel_getName(_cmd);
    
	size_t selLen=strlen(origName);
	char *sel=__builtin_alloca(selLen+1);
	strcpy(sel, origName);
	sel[selLen-1]='\0';
	sel+=strlen("remove");
	sel[strlen(sel)-strlen("Object:")+1]='\0';
    
    char *countSelName=__builtin_alloca(strlen(sel)+strlen("countOf")+1);
    strcpy(countSelName, "countOf");
    strcat(countSelName, sel);
    
    NSUInteger idx=(NSUInteger)[self performSelector:sel_getUid(countSelName)];
    
    sel[0]=tolower(sel[0]);
    
    NSString *key=[[NSString allocWithZone:NULL] initWithCString:sel];
    [self willChange:NSKeyValueChangeRemoval valuesAtIndexes:[NSIndexSet indexSetWithIndex:idx] forKey:key];
    typedef id (*sender)(id obj, SEL selector, id value);
    sender implementation=(sender)[[self superclass] instanceMethodForSelector:_cmd];
    (void)*implementation(self, _cmd, object);
    [self didChange:NSKeyValueChangeRemoval valuesAtIndexes:[NSIndexSet indexSetWithIndex:idx] forKey:key];
    [key release];
}

-(void)KVO_notifying_change_removeKey:(NSSet*)objects {
    const char* origName = sel_getName(_cmd);
    
	size_t selLen=strlen(origName);
	char *sel=__builtin_alloca(selLen+1);
	strcpy(sel, origName);
	sel[selLen-1]='\0';
	sel+=strlen("remove");
	sel[strlen(sel)-strlen(":")+1]='\0';
    
	sel[0]=tolower(sel[0]);
	NSString *key=[[NSString allocWithZone:NULL] initWithCString:sel];
    
    [self willChangeValueForKey:key withSetMutation:NSKeyValueMinusSetMutation usingObjects:objects];
	typedef id (*sender)(id obj, SEL selector, NSSet* objects);
	sender implementation=(sender)[[self superclass] instanceMethodForSelector:_cmd];
	(void)*implementation(self, _cmd, objects);
    [self didChangeValueForKey:key withSetMutation:NSKeyValueMinusSetMutation usingObjects:objects];
	[key release];
}

-(void)KVO_notifying_change_removeObjectFromKeyAtIndex:(int)index {
	const char* origName = sel_getName(_cmd);
	size_t selLen=strlen(origName);
	char *sel=__builtin_alloca(selLen+1);
	strcpy(sel, origName);
	sel[selLen-1]='\0';
	sel+=strlen("removeObjectFrom");
	sel[strlen(sel)-strlen("AtIndex:")+1]='\0';

	sel[0]=tolower(sel[0]);
	NSString *key=[[NSString allocWithZone:NULL] initWithCString:sel];
	[self willChange:NSKeyValueChangeRemoval valuesAtIndexes:[NSIndexSet indexSetWithIndex:index] forKey:key];
	typedef id (*sender)(id obj, SEL selector, int index);
	sender implementation=(sender)[[self superclass] instanceMethodForSelector:_cmd];
	(void)*implementation(self, _cmd, index);
	[self didChange:NSKeyValueChangeRemoval valuesAtIndexes:[NSIndexSet indexSetWithIndex:index] forKey:key];
	[key release];
}

-(void)KVO_notifying_change_removeKeyAtIndexes:(NSIndexSet*)indexes {
	const char* origName = sel_getName(_cmd);
	size_t selLen=strlen(origName);
	char *sel=__builtin_alloca(selLen+1);
	strcpy(sel, origName);
	sel[selLen-1]='\0';
	sel+=strlen("remove");
	sel[strlen(sel)-strlen("AtIndexes:")+1]='\0';
    
	sel[0]=tolower(sel[0]);
	NSString *key=[[NSString allocWithZone:NULL] initWithCString:sel];
	[self willChange:NSKeyValueChangeRemoval valuesAtIndexes:indexes forKey:key];
	typedef id (*sender)(id obj, SEL selector, NSIndexSet* indexes);
	sender implementation=(sender)[[self superclass] instanceMethodForSelector:_cmd];
	(void)*implementation(self, _cmd, indexes);
	[self didChange:NSKeyValueChangeRemoval valuesAtIndexes:indexes forKey:key];
	[key release];
}

-(void)KVO_notifying_change_replaceObjectInKeyAtIndex:(int)index withObject:(id)object {
	const char* origName = sel_getName(_cmd);
	size_t selLen=strlen(origName);
	char *sel=__builtin_alloca(selLen+1);
	strcpy(sel, origName);
	sel[selLen-1]='\0';
	sel+=strlen("replaceObjectIn");
	sel[strlen(sel)-strlen("AtIndex:WithObject:")+1]='\0';
	sel[0]=tolower(sel[0]);

	NSString *key=[[NSString allocWithZone:NULL] initWithCString:sel];
	[self willChange:NSKeyValueChangeReplacement valuesAtIndexes:[NSIndexSet indexSetWithIndex:index] forKey:key];
	typedef id (*sender)(id obj, SEL selector, int index, id object);
	sender implementation=(sender)[[self superclass] instanceMethodForSelector:_cmd];
	(void)*implementation(self, _cmd, index, object);
	[self didChange:NSKeyValueChangeReplacement valuesAtIndexes:[NSIndexSet indexSetWithIndex:index] forKey:key];
	[key release];
}


-(id)_KVO_className {
	return [NSString stringWithCString:class_getName(isa)+strlen("KVONotifying_")];
}

-(Class)_KVO_class {
    return class_getSuperclass(isa);
}

-(Class)_KVO_classForCoder {
    return class_getSuperclass(isa);
}


// This method gathers dependent keys from all superclasses and merges them together
+(NSDictionary *)_KVO_buildDependencyUnion {
   NSKVOInfoPerObject *observationInfo=[self observationInfo];

   if(!observationInfo) {
    observationInfo=[[NSKVOInfoPerObject allocWithZone:NULL] init];
    [self setObservationInfo:observationInfo];
   }

	NSMutableDictionary *keyPathsByKey=[[NSMutableDictionary alloc] init];

	id class=self;
	while(class != [NSObject class]){
		NSDictionary* classDependents=[(NSDictionary*)[class observationInfo] objectForKey:_KVO_DependentKeysTriggeringChangeNotification];

		for(id key in [classDependents allKeys]) {
			for(id value in [classDependents objectForKey:key]) {
				NSMutableSet *pathSet=[keyPathsByKey objectForKey:value];
				if(!pathSet) {
					pathSet=[NSMutableSet set];
					[keyPathsByKey setObject:pathSet forKey:value];
				}
				[pathSet addObject:key];
			}
		}

		class=[class superclass];
	}
	[observationInfo setObject:keyPathsByKey forKey:_KVO_KeyPathsForValuesAffectingValueForKey];
    [keyPathsByKey release];

    return keyPathsByKey;
}



-(void)_KVO_swizzle
{
	NSString* className=[self className];
	if([className hasPrefix:@"KVONotifying_"])
		return; // this class is already swizzled
    pthread_mutex_lock(&kvoLock);
	isa=[self _KVO_swizzledClass];
    pthread_mutex_unlock(&kvoLock);
}

static BOOL methodIsAutoNotifyingSetter(Class class,const char *methodCString){
   size_t cStringLength=strlen(methodCString),keyCStringLength=0;
   char   keyCString[cStringLength+1];
   enum {
    STATE_START,
    STATE_UNDERSCORE,
    STATE_S,
    STATE_E,
    STATE_T,
    STATE_UNTILCOLON,
   } state=STATE_START;

   for(;*methodCString!='\0';methodCString++){

    switch(state){

     case STATE_START:
      if(*methodCString=='s')
       state=STATE_S;
      else if(*methodCString=='_')
       state=STATE_UNDERSCORE;
      else
       return NO;
      break;

     case STATE_UNDERSCORE:
      if(*methodCString=='s')
       state=STATE_S;
      else
       return NO;
      break;

     case STATE_S:
      if(*methodCString=='e')
       state=STATE_E;
      else
       return NO;
      break;

     case STATE_E:
      if(*methodCString=='t')
       state=STATE_T;
      else
       return NO;
      break;

     case STATE_T:
      if(*methodCString==':')
       return NO;
      keyCString[keyCStringLength++]=tolower(*methodCString);
      state=STATE_UNTILCOLON;
      break;

     case STATE_UNTILCOLON:
      if(*methodCString!=':')
       keyCString[keyCStringLength++]=*methodCString;
      break;
    }
   }
   if(keyCStringLength==0)
    return NO;

   NSString *keyName=[[NSString alloc] initWithCString:keyCString length:keyCStringLength];

   BOOL result=[class automaticallyNotifiesObserversForKey:keyName];
   [keyName release];

   return result;
}


- (Class)_KVO_swizzledClass
{
    // find swizzled class
    const char *swizzledName = [[NSString stringWithFormat:@"KVONotifying_%@", [self className]] UTF8String];
    Class swizzledClass = objc_lookUpClass(swizzledName);

    if (swizzledClass) {
        return swizzledClass;
    }

    // swizzled class doesn't exist; create
    swizzledClass = objc_allocateClassPair(isa, swizzledName, 0);
    if(!swizzledClass) {
        [NSException raise:@"NSClassCreationException" format:@"couldn't swizzle class %@ for KVO", [self className]];
    }

    // add KVO-Observing methods
    // override className so it returns the original class name
    Method className = class_getInstanceMethod([self class], @selector(_KVO_className));
    Method class = class_getInstanceMethod([self class], @selector(_KVO_class));
    Method classForCoder = class_getInstanceMethod([self class], @selector(_KVO_classForCoder));
    IMP classNameImp = method_getImplementation(className);
    IMP classImp = method_getImplementation(class);
    IMP classForCoderImp = method_getImplementation(classForCoder);
    const char *classNameTypes = method_getTypeEncoding(className);
    const char *classTypes = method_getTypeEncoding(class);
    const char *classForCoderTypes = method_getTypeEncoding(classForCoder);

    class_addMethod(swizzledClass, @selector(className), classNameImp, classNameTypes);
    class_addMethod(swizzledClass, @selector(class), classImp, classTypes);
    class_addMethod(swizzledClass, @selector(classForCoder), classForCoderImp, classForCoderTypes);

    Class currentClass = isa;

    for (; currentClass && class_getSuperclass(currentClass) != currentClass; currentClass = class_getSuperclass(currentClass)) {
        unsigned int count;
        Method *methods = class_copyMethodList(currentClass, &count);
        NSAutoreleasePool *pool = [NSAutoreleasePool new];
        for (int i = 0; i < count; ++i) {
            Method method = methods[i];
            SEL sel = method_getName(method);
            const char *methodCString = sel_getName(sel);
            NSUInteger numberOfArguments = method_getNumberOfArguments(method);
            SEL kvoSelector = 0;

            // current method is a setter?
            if (numberOfArguments == 3 && methodIsAutoNotifyingSetter([self class], methodCString)) {
                NSMethodSignature *signature = [self methodSignatureForSelector:sel];
                const char *firstParameterType = [signature getArgumentTypeAtIndex:2];
                const char *returnType = [signature methodReturnType];

                char *cleanFirstParameterType = __builtin_alloca(strlen(firstParameterType) + 1);
                [self _demangleTypeEncoding:firstParameterType to:cleanFirstParameterType];

                /* check for correct type: either perfect match
                or primitive signed type matching unsigned type
                (i.e. tolower(@encode(unsigned long)[0])==@encode(long)[0])
                */
                #define CHECK_AND_ASSIGN(a) \
                        if (!strcmp(cleanFirstParameterType, @encode(a)) || \
                                (strlen(@encode(a)) == 1 && \
                                strlen(cleanFirstParameterType) == 1 && \
                                tolower(cleanFirstParameterType[0]) == @encode(a)[0])) { \
                            kvoSelector = @selector(CHANGE_SELECTOR(a)); \
                        }
                // FIX: add more types
                CHECK_AND_ASSIGN(id);
                CHECK_AND_ASSIGN(float);
                CHECK_AND_ASSIGN(double);
                CHECK_AND_ASSIGN(int);
                CHECK_AND_ASSIGN(NSSize);
                CHECK_AND_ASSIGN(NSPoint);
                CHECK_AND_ASSIGN(NSRect);
                CHECK_AND_ASSIGN(NSRange);
                CHECK_AND_ASSIGN(char);
                CHECK_AND_ASSIGN(long);
                CHECK_AND_ASSIGN(SEL);
                #undef CHECK_AND_ASSIGN

                if (kvoSelector == 0 && NSDebugEnabled) {
                    NSLog(@"NSDebugEnabled type %s not defined in %s:%i (selector %s on class %@)", cleanFirstParameterType, __FILE__, __LINE__, methodCString, [self className]);
                }
                if (returnType[0] != _C_VOID) {
                    kvoSelector = 0;
                }
            }

            // long selectors
            if (kvoSelector == 0) {
                NSString *methodName = NSStringFromSelector(sel);
                if (numberOfArguments == 4 && [methodName _KVC_isSetterForSelectorNameStartingWith:@"insertObject:in" endingWith:@"AtIndex:"]) {
                    kvoSelector = @selector(KVO_notifying_change_insertObject:inKeyAtIndex:);
                } else if (numberOfArguments == 3 && [methodName _KVC_isSetterForSelectorNameStartingWith:@"removeObjectFrom" endingWith:@"AtIndex:"]) {
                    kvoSelector = @selector(KVO_notifying_change_removeObjectFromKeyAtIndex:);
                } else if (numberOfArguments == 4 && [methodName _KVC_isSetterForSelectorNameStartingWith:@"replaceObjectIn" endingWith:@"AtIndex:withObject:"]) {
                    kvoSelector = @selector(KVO_notifying_change_replaceObjectInKeyAtIndex:withObject:);
                } else if (numberOfArguments == 4 && [methodName _KVC_isSetterForSelectorNameStartingWith:@"insert" endingWith:@":atIndexes:"]) {
                    kvoSelector = @selector(KVO_notifying_change_insertKey:atIndexes:);
                } else if (numberOfArguments == 3 && [methodName _KVC_isSetterForSelectorNameStartingWith:@"remove" endingWith:@"AtIndexes:"]) {
                    kvoSelector = @selector(KVO_notifying_change_removeKeyAtIndexes:);
                } else if (numberOfArguments == 3 && [methodName _KVC_isSetterForSelectorNameStartingWith:@"remove" endingWith:@"Object:"]) {
                    kvoSelector = @selector(KVO_notifying_change_removeKeyObject:);
                } else if (numberOfArguments == 3 && [methodName _KVC_isSetterForSelectorNameStartingWith:@"add" endingWith:@"Object:"]) {
                    kvoSelector = @selector(KVO_notifying_change_addKeyObject:);
#if 0
// Disabled - this is wrong - this is expecting any addXXX: removeXXX: methods to play with NSSet
                } else if (numberOfArguments == 3 && [methodName _KVC_isSetterForSelectorNameStartingWith:@"remove" endingWith:@":"]) {
                    kvoSelector = @selector(KVO_notifying_change_removeKey:);
                } else if (numberOfArguments == 3 && [methodName _KVC_isSetterForSelectorNameStartingWith:@"add" endingWith:@":"]) {
                    kvoSelector = @selector(KVO_notifying_change_addKey:);
#endif
                }
            }

            // these are swizzled so e.g. subclasses of NSMutableDictionary get change notifications in setObject:forKey:
            if (strcmp(methodCString,"setObject:forKey:") == 0) {
                kvoSelector = @selector(KVO_notifying_change_setObject:forKey:);
            } else if (strcmp(methodCString,"removeObjectForKey:") == 0) {
                kvoSelector = @selector(KVO_notifying_change_removeObjectForKey:);
            }

            // there's a suitable selector for us
            if (kvoSelector != 0) {
                const char *types = method_getTypeEncoding(method);

                class_addMethod(swizzledClass, sel, [self methodForSelector:kvoSelector], types);
                //NSLog(@"replaced method %s by %@ in class %@", methodNameCString, NSStringFromSelector(newMethod->method_name), [self className]);
            }
        }
        [pool release];
        if (methods != NULL) {
            free(methods);
        }
    }

    objc_registerClassPair(swizzledClass);

    // done
    return swizzledClass;
}


+ (BOOL)automaticallyNotifiesObserversForKey:(NSString *)key; {
   if([key isEqualToString:@"observationInfo"]) {
    return NO;
   }

   return YES;
}
@end

