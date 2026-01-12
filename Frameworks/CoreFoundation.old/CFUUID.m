#import <CoreFoundation/CFUUID.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSCFTypeID.h>
#ifdef WINDOWS
#include <windows.h>
#include <rpc.h>
#endif

#define CastToCFUUID(x) ((CFUUID *)(x))

@interface CFUUID : NSObject {
   CFUUIDBytes _bytes;
}
@end

@implementation CFUUID

CFTypeID CFUUIDGetTypeID(void){
   return kNSCFTypeUUID;
}

CFUUIDRef CFUUIDCreate(CFAllocatorRef alloc){
   CFUUID *result=[CFUUID alloc];
   
#ifdef WINDOWS
   UUID guid;
   
   UuidCreate(&guid);
   
   result->_bytes.byte0=(guid.Data1>>24)&0xFF;
   result->_bytes.byte1=(guid.Data1>>16)&0xFF;
   result->_bytes.byte2=(guid.Data1>>8)&0xFF;
   result->_bytes.byte3=(guid.Data1>>0)&0xFF;
   result->_bytes.byte4=(guid.Data2>>8)&0xFF;
   result->_bytes.byte5=(guid.Data2>>0)&0xFF;
   result->_bytes.byte6=(guid.Data3>>8)&0xFF;
   result->_bytes.byte7=(guid.Data3>>0)&0xFF;
   result->_bytes.byte8=guid.Data4[0];
   result->_bytes.byte9=guid.Data4[1];
   result->_bytes.byte10=guid.Data4[2];
   result->_bytes.byte11=guid.Data4[3];
   result->_bytes.byte12=guid.Data4[4];
   result->_bytes.byte13=guid.Data4[5];
   result->_bytes.byte14=guid.Data4[6];
   result->_bytes.byte15=guid.Data4[7];
   
#else
#endif

   return (CFUUIDRef)result;
}

CFUUIDRef CFUUIDCreateFromString(CFAllocatorRef allocator,CFStringRef string){
   NSUnimplementedFunction();
   return 0;
}

CFUUIDRef CFUUIDCreateFromUUIDBytes(CFAllocatorRef allocator,CFUUIDBytes bytes){
   CFUUID *result=[CFUUID alloc];
   
   result->_bytes=bytes;
   
   return (CFUUIDRef)result;
}

CFUUIDRef CFUUIDCreateWithBytes(CFAllocatorRef allocator,uint8_t byte0,uint8_t byte1,uint8_t byte2,uint8_t byte3,uint8_t byte4,uint8_t byte5,uint8_t byte6,uint8_t byte7,uint8_t byte8,uint8_t byte9,uint8_t byte10,uint8_t byte11,uint8_t byte12,uint8_t byte13,uint8_t byte14,uint8_t byte15){
   CFUUID *result=[CFUUID alloc];
      
   result->_bytes.byte0=byte0;
   result->_bytes.byte1=byte1;
   result->_bytes.byte2=byte2;
   result->_bytes.byte3=byte3;
   result->_bytes.byte4=byte4;
   result->_bytes.byte5=byte5;
   result->_bytes.byte6=byte6;
   result->_bytes.byte7=byte7;
   result->_bytes.byte8=byte8;
   result->_bytes.byte9=byte9;
   result->_bytes.byte10=byte10;
   result->_bytes.byte11=byte11;
   result->_bytes.byte12=byte12;
   result->_bytes.byte13=byte13;
   result->_bytes.byte14=byte14;
   result->_bytes.byte15=byte15;
   
   return (CFUUIDRef)result;
}

CFUUIDRef CFUUIDGetConstantUUIDWithBytes(CFAllocatorRef allocator,uint8_t byte0,uint8_t byte1,uint8_t byte2,uint8_t byte3,uint8_t byte4,uint8_t byte5,uint8_t byte6,uint8_t byte7,uint8_t byte8,uint8_t byte9,uint8_t byte10,uint8_t byte11,uint8_t byte12,uint8_t byte13,uint8_t byte14,uint8_t byte15){
   CFUUID *result=[CFUUID alloc];
      
   result->_bytes.byte0=byte0;
   result->_bytes.byte1=byte1;
   result->_bytes.byte2=byte2;
   result->_bytes.byte3=byte3;
   result->_bytes.byte4=byte4;
   result->_bytes.byte5=byte5;
   result->_bytes.byte6=byte6;
   result->_bytes.byte7=byte7;
   result->_bytes.byte8=byte8;
   result->_bytes.byte9=byte9;
   result->_bytes.byte10=byte10;
   result->_bytes.byte11=byte11;
   result->_bytes.byte12=byte12;
   result->_bytes.byte13=byte13;
   result->_bytes.byte14=byte14;
   result->_bytes.byte15=byte15;
   
   return (CFUUIDRef)result;
}

CFUUIDBytes CFUUIDGetUUIDBytes(CFUUIDRef self){
    return ((CFUUID *)self)->_bytes;
}

CFStringRef CFUUIDCreateString(CFAllocatorRef allocator,CFUUIDRef self){
   return (CFStringRef)[[NSString alloc] initWithFormat:@"%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X%02X%02X",
    ((CFUUID *)self)->_bytes.byte0,
    ((CFUUID *)self)->_bytes.byte1,
    ((CFUUID *)self)->_bytes.byte2,
    ((CFUUID *)self)->_bytes.byte3,
    ((CFUUID *)self)->_bytes.byte4,
    ((CFUUID *)self)->_bytes.byte5,
    ((CFUUID *)self)->_bytes.byte6,
    ((CFUUID *)self)->_bytes.byte7,
    ((CFUUID *)self)->_bytes.byte8,
    ((CFUUID *)self)->_bytes.byte9,
    ((CFUUID *)self)->_bytes.byte10,
    ((CFUUID *)self)->_bytes.byte11,
    ((CFUUID *)self)->_bytes.byte12,
    ((CFUUID *)self)->_bytes.byte13,
    ((CFUUID *)self)->_bytes.byte14,
    ((CFUUID *)self)->_bytes.byte15];
}

@end
