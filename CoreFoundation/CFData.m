#import <CoreFoundation/CFData.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSData.h>
#import <Foundation/NSCFTypeID.h>

struct __CFMutableData {
};

#define ToCFData(x) ((CFDataRef)(x))
#define ToCFMutableData(x) ((CFMutableDataRef)(x))
#define ToNSData(x) ((NSData *)(x))
#define ToNSMutableData(x) ((NSMutableData *)(x))

static inline NSRange NSRangeFromCFRange(CFRange range){
   NSRange result={range.location,range.length};
   return result;
}

CFTypeID CFDataGetTypeID(void){
   return kNSCFTypeData;
}

CFDataRef CFDataCreate(CFAllocatorRef allocator,const uint8_t *bytes,CFIndex length){
   return ToCFData([[NSData allocWithZone:NULL] initWithBytes:bytes length:length]);
}

CFDataRef CFDataCreateWithBytesNoCopy(CFAllocatorRef allocator,const uint8_t *bytes,CFIndex length,CFAllocatorRef bytesDeallocator){
    NSUnimplementedFunction();
    return 0;
}

CFDataRef CFDataCreateCopy(CFAllocatorRef allocator,CFDataRef self){
   return ToCFData([ToNSData(self) copy]);
}

CFIndex CFDataGetLength(CFDataRef self){
   return [ToNSData(self) length];
}

const uint8_t *CFDataGetBytePtr(CFDataRef self){
   return [ToNSData(self) bytes];
}

void CFDataGetBytes(CFDataRef self,CFRange range,uint8_t *bytes){
   [ToNSData(self) getBytes:bytes range:NSRangeFromCFRange(range)];
}

// mutable

CFMutableDataRef CFDataCreateMutable(CFAllocatorRef allocator,CFIndex capacity){
   return ToCFMutableData([[NSMutableData allocWithZone:NULL] initWithCapacity:capacity]);
}

CFMutableDataRef CFDataCreateMutableCopy(CFAllocatorRef allocator,CFIndex capacity,CFDataRef other){
   CFMutableDataRef self=CFDataCreateMutable(allocator,capacity);
   
   [ToNSMutableData(self) setData:ToNSData(other)];

   return self;
}

uint8_t *CFDataGetMutableBytePtr(CFMutableDataRef self){
   return [ToNSMutableData(self) mutableBytes];
}

void CFDataSetLength(CFMutableDataRef self,CFIndex length){
   [ToNSMutableData(self) setLength:length];
}

void CFDataAppendBytes(CFMutableDataRef self,const uint8_t *bytes,CFIndex length){
   [ToNSMutableData(self) appendBytes:bytes length:length];
}

void CFDataDeleteBytes(CFMutableDataRef self,CFRange range){
   [ToNSMutableData(self) replaceBytesInRange:NSRangeFromCFRange(range) withBytes:NULL length:0];
}

void CFDataIncreaseLength(CFMutableDataRef self,CFIndex delta){
   [ToNSMutableData(self) increaseLengthBy:delta];
}

void CFDataReplaceBytes(CFMutableDataRef self,CFRange range,const uint8_t *bytes,CFIndex length){
   [ToNSMutableData(self) replaceBytesInRange:NSRangeFromCFRange(range) withBytes:bytes length:length];
}
