#import <CoreFoundation/CFBag.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSCFTypeID.h>

const CFBagCallBacks kCFTypeBagCallBacks={
};

const CFBagCallBacks kCFCopyStringBagCallBacks={
};

CFTypeID CFBagGetTypeID(void){
   return kNSCFTypeBag;
}

void CFBagApplyFunction(CFBagRef self,CFBagApplierFunction function,void *context){
   NSUnimplementedFunction();
}

Boolean CFBagContainsValue(CFBagRef self,const void *value){
   NSUnimplementedFunction();
   return 0;
}

CFBagRef CFBagCreate(CFAllocatorRef allocator,const void **values,CFIndex count,const CFBagCallBacks *callbacks){
   NSUnimplementedFunction();
   return 0;
}

CFBagRef CFBagCreateCopy(CFAllocatorRef allocator,CFBagRef self){
   NSUnimplementedFunction();
   return 0;
}

CFIndex CFBagGetCount(CFBagRef self){
   NSUnimplementedFunction();
   return 0;
}

CFIndex CFBagGetCountOfValue(CFBagRef self,const void *value){
   NSUnimplementedFunction();
   return 0;
}

const void *CFBagGetValue(CFBagRef self,const void *value){
   NSUnimplementedFunction();
   return 0;
}

Boolean CFBagGetValueIfPresent(CFBagRef self,const void *candidate,const void **value){
   NSUnimplementedFunction();
   return 0;
}

void CFBagGetValues(CFBagRef self,const void **values){
   NSUnimplementedFunction();
}
// mutable

void CFBagAddValue(CFMutableBagRef self,const void *value){
   NSUnimplementedFunction();
}

CFMutableBagRef CFBagCreateMutable(CFAllocatorRef allocator,CFIndex capacity,const CFBagCallBacks *callbacks){
   NSUnimplementedFunction();
   return 0;
}

CFMutableBagRef CFBagCreateMutableCopy(CFAllocatorRef allocator,CFIndex capacity,CFBagRef self){
   NSUnimplementedFunction();
   return 0;
}

void CFBagRemoveAllValues(CFMutableBagRef self){
   NSUnimplementedFunction();
}

void CFBagRemoveValue(CFMutableBagRef self,const void *value){
   NSUnimplementedFunction();
}

void CFBagReplaceValue(CFMutableBagRef self,const void *value){
   NSUnimplementedFunction();
}

void CFBagSetValue(CFMutableBagRef self,const void *value){
   NSUnimplementedFunction();
}
