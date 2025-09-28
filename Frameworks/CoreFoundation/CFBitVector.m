#import <CoreFoundation/CFBitVector.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSCFTypeID.h>

CFTypeID CFBitVectorGetTypeID(void) {
   return kNSCFTypeBitVector;
}

CFBitVectorRef CFBitVectorCreate(CFAllocatorRef allocator,const uint8_t *bytes,CFIndex count) {
   NSUnimplementedFunction();
   return 0;
}

CFBitVectorRef CFBitVectorCreateCopy(CFAllocatorRef allocator,CFBitVectorRef self){
   NSUnimplementedFunction();
   return 0;
}

CFIndex CFBitVectorGetCount(CFBitVectorRef self){
   NSUnimplementedFunction();
   return 0;
}

CFBit CFBitVectorGetBitAtIndex(CFBitVectorRef self,CFIndex index){
   NSUnimplementedFunction();
   return 0;
}

void CFBitVectorGetBits(CFBitVectorRef self,CFRange range,uint8_t *bytes){
   NSUnimplementedFunction();
}

Boolean CFBitVectorContainsBit(CFBitVectorRef self,CFRange range,CFBit value){
   NSUnimplementedFunction();
   return 0;
}

CFIndex CFBitVectorGetCountOfBit(CFBitVectorRef self,CFRange range,CFBit value){
   NSUnimplementedFunction();
   return 0;
}

CFIndex CFBitVectorGetFirstIndexOfBit(CFBitVectorRef self,CFRange range,CFBit value){
   NSUnimplementedFunction();
   return 0;
}

CFIndex CFBitVectorGetLastIndexOfBit(CFBitVectorRef self,CFRange range,CFBit value){
   NSUnimplementedFunction();
   return 0;
}
