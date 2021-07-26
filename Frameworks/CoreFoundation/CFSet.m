#import <CoreFoundation/CFSet.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSCFTypeID.h>

const CFSetCallBacks kCFTypeSetCallBacks={
};

const CFSetCallBacks kCFCopyStringSetCallBacks={
};

CFTypeID CFSetGetTypeID(void){
   return kNSCFTypeSet;
}

CFSetRef CFSetCreate(CFAllocatorRef allocator,const void **values,CFIndex count,const CFSetCallBacks *callbacks){
   NSUnimplementedFunction();
   return 0;
}

CFSetRef CFSetCreateCopy(CFAllocatorRef allocator,CFSetRef self){
   NSUnimplementedFunction();
   return 0;
}

CFIndex CFSetGetCount(CFSetRef self){
   NSUnimplementedFunction();
   return 0;
}

const void *CFSetGetValue(CFSetRef self,const void *value){
   NSUnimplementedFunction();
   return 0;
}

void CFSetGetValues(CFSetRef self,const void **values){
   NSUnimplementedFunction();
}

void CFSetApplyFunction(CFSetRef self,CFSetApplierFunction function,void *context){
   NSUnimplementedFunction();
}

Boolean CFSetContainsValue(CFSetRef self,const void *value){
   NSUnimplementedFunction();
   return 0;
}

CFIndex CFSetGetCountOfValue(CFSetRef self,const void *value){
   NSUnimplementedFunction();
   return 0;
}

Boolean CFSetGetValueIfPresent(CFSetRef self,const void *candidate,const void **value){
   NSUnimplementedFunction();
   return 0;
}

// mutable

CFMutableSetRef CFSetCreateMutable(CFAllocatorRef allocator,CFIndex capacity,const CFSetCallBacks *callbacks){
   NSUnimplementedFunction();
   return 0;
}

CFMutableSetRef CFSetCreateMutableCopy(CFAllocatorRef allocator,CFIndex capacity,CFSetRef self){
   NSUnimplementedFunction();
   return 0;
}

void CFSetAddValue(CFMutableSetRef self,const void *value){
   NSUnimplementedFunction();
}

void CFSetRemoveAllValues(CFMutableSetRef self){
   NSUnimplementedFunction();
}

void CFSetRemoveValue(CFMutableSetRef self,const void *value){
   NSUnimplementedFunction();
}

void CFSetReplaceValue(CFMutableSetRef self,const void *value){
   NSUnimplementedFunction();
}

void CFSetSetValue(CFMutableSetRef self,const void *value){
   NSUnimplementedFunction();
}
