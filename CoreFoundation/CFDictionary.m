#import <CoreFoundation/CFDictionary.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSMutableDictionary_mapTable.h>
#import <Foundation/NSCFTypeID.h>

struct __CFDictionary {
};

#define ToNSDictionary(object) ((NSDictionary *)object)
#define ToNSMutableDictionary(object) ((NSMutableDictionary *)object)
#define ToCFDictionary(object) ((CFDictionaryRef)object)
#define ToCFMutableDictionary(object) ((CFMutableDictionaryRef)object)

const void *cfRetainCallBack(CFAllocatorRef allocator,const void *value) {
   return CFRetain(value);
}

void cfReleaseCallBack(CFAllocatorRef allocator,const void *value) {
   CFRelease(value);
}

const CFDictionaryKeyCallBacks kCFCopyStringDictionaryKeyCallBacks={
	0,(CFDictionaryRetainCallBack)CFStringCreateCopy,cfReleaseCallBack,CFCopyDescription,CFEqual,CFHash,
};

const CFDictionaryKeyCallBacks kCFTypeDictionaryKeyCallBacks={
 0,cfRetainCallBack,cfReleaseCallBack,CFCopyDescription,CFEqual,CFHash,
};

const CFDictionaryValueCallBacks kCFTypeDictionaryValueCallBacks={
 0,cfRetainCallBack,cfReleaseCallBack,CFCopyDescription,CFEqual,
};

CFTypeID CFDictionaryGetTypeID(void){
   return kNSCFTypeDictionary;
}

CFDictionaryRef CFDictionaryCreate(CFAllocatorRef allocator,const void **keys,const void **values,CFIndex count,const CFDictionaryKeyCallBacks *keyCallBacks,const CFDictionaryValueCallBacks *valueCallBacks){
   return ToCFDictionary([[NSMutableDictionary_CF allocWithZone:NULL] initWithKeys:keys values:values count:count keyCallBacks:keyCallBacks valueCallBacks:valueCallBacks]);
}

CFDictionaryRef CFDictionaryCreateCopy(CFAllocatorRef allocator,CFDictionaryRef self){
   NSUnimplementedFunction();
   return 0;
}

void CFDictionaryApplyFunction(CFDictionaryRef self,CFDictionaryApplierFunction function,void *context){
   NSUnimplementedFunction();
}

Boolean CFDictionaryContainsKey(CFDictionaryRef self,const void *key){
   NSUnimplementedFunction();
   return 0;
}

Boolean CFDictionaryContainsValue(CFDictionaryRef self,const void *value){
   NSUnimplementedFunction();
   return 0;
}

CFIndex CFDictionaryGetCount(CFDictionaryRef self){
   return [ToNSDictionary(self) count];
}

CFIndex CFDictionaryGetCountOfKey(CFDictionaryRef self,const void *key){
   NSUnimplementedFunction();
   return 0;
}

CFIndex CFDictionaryGetCountOfValue(CFDictionaryRef self,const void *value){
   NSUnimplementedFunction();
   return 0;
}

void CFDictionaryGetKeysAndValues(CFDictionaryRef self,const void **keys,const void **values){
   NSUnimplementedFunction();
}

const void *CFDictionaryGetValue(CFDictionaryRef self,const void *key){
   return [ToNSDictionary(self) objectForKey:(id)key];
}

Boolean CFDictionaryGetValueIfPresent(CFDictionaryRef self,const void *key,const void **value){
   NSUnimplementedFunction();
   return 0;
}

CFMutableDictionaryRef CFDictionaryCreateMutable(CFAllocatorRef allocator,CFIndex capacity,const CFDictionaryKeyCallBacks *keyCallBacks,const CFDictionaryValueCallBacks *valueCallBacks){
   return ToCFMutableDictionary([[NSMutableDictionary_CF allocWithZone:NULL] initWithKeys:NULL values:NULL count:0 keyCallBacks:keyCallBacks valueCallBacks:valueCallBacks]);
}

CFMutableDictionaryRef CFDictionaryCreateMutableCopy(CFAllocatorRef allocator,CFIndex capacity,CFDictionaryRef self){
   NSUnimplementedFunction();
   return 0;
}

void CFDictionaryAddValue(CFMutableDictionaryRef self,const void *key,const void *value){
   if(CFDictionaryGetValue(self,key)==NULL)
    CFDictionarySetValue(self,key,value);
}

void CFDictionaryRemoveAllValues(CFMutableDictionaryRef self){
   [ToNSMutableDictionary(self) removeAllObjects];
}

void CFDictionaryRemoveValue(CFMutableDictionaryRef self,const void *key){
   [ToNSMutableDictionary(self) removeObjectForKey:(id)key];
}

void CFDictionaryReplaceValue(CFMutableDictionaryRef self,const void *key,const void *value){
   if(CFDictionaryGetValue(self,key)!=NULL)
    CFDictionarySetValue(self,key,value);
}

void CFDictionarySetValue(CFMutableDictionaryRef self,const void *key,const void *value){
   [ToNSMutableDictionary(self) setObject:(id)value forKey:(id)key];
}

