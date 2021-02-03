#import <CoreFoundation/CFByteOrder.h>

CFSwappedFloat32 CFConvertFloat32HostToSwapped(Float32 value) {
    union {
        CFSwappedFloat32 w;
        Float32  f;
    } swap;
    swap.f=value;
    
#ifdef __LITTLE_ENDIAN__
    swap.w.v=CFSwapInt32(swap.w.v);
#endif
    
    return swap.w;
}

Float32 CFConvertFloat32SwappedToHost(CFSwappedFloat32 value) {
   union {
    CFSwappedFloat32 w;
    Float32  f;
   } swap;
   swap.w=value;

#ifdef __LITTLE_ENDIAN__
   swap.w.v=CFSwapInt32(swap.w.v);
#endif

   return swap.f;
}

uint16_t CFSwapInt16(uint16_t value) {
	uint16_t result;
	
	result=value<<8;
	result|=value>>8;
	
	return result;
}

uint16_t CFSwapInt16BigToHost(uint16_t value) {
#ifdef __LITTLE_ENDIAN__
	return CFSwapInt16(value);
#endif
#ifdef __BIG_ENDIAN__
	return value;
#endif
}

uint16_t CFSwapInt16LittleToHost(uint16_t value) {
#ifdef __BIG_ENDIAN__
	return CFSwapInt16(value);
#endif
#ifdef __LITTLE_ENDIAN__
	return value;
#endif
}

uint16_t CFSwapInt16HostToBig(uint16_t value) {
#ifdef __LITTLE_ENDIAN__
	return CFSwapInt16(value);
#endif
#ifdef __BIG_ENDIAN__
	return value;
#endif
}

uint16_t CFSwapInt16HostToLittle(uint16_t value) {
#ifdef __BIG_ENDIAN__
	return CFSwapInt16(value);
#endif
#ifdef __LITTLE_ENDIAN__
	return value;
#endif
}

uint32_t CFSwapInt32(uint32_t value) {
   uint32_t result;

   result=value<<24;
   result|=(value<<8)&0x00FF0000;
   result|=(value>>8)&0x0000FF00;
   result|=value>>24;

   return result;
}

uint32_t CFSwapInt32BigToHost(uint32_t value) {
#ifdef __LITTLE_ENDIAN__
	return CFSwapInt32(value);
#endif
#ifdef __BIG_ENDIAN__
	return value;
#endif
}

uint32_t CFSwapInt32LittleToHost(uint32_t value) {
#ifdef __BIG_ENDIAN__
	return CFSwapInt32(value);
#endif
#ifdef __LITTLE_ENDIAN__
	return value;
#endif
}

uint32_t CFSwapInt32HostToBig(uint32_t value) {
#ifdef __LITTLE_ENDIAN__
	return CFSwapInt32(value);
#endif
#ifdef __BIG_ENDIAN__
	return value;
#endif
}

uint32_t CFSwapInt32HostToLittle(uint32_t value) {
#ifdef __BIG_ENDIAN__
	return CFSwapInt32(value);
#endif
#ifdef __LITTLE_ENDIAN__
	return value;
#endif
}


uint16_t OSReadBigInt16(const void *ptr,size_t offset){
   const uint8_t *bytes=ptr+offset;
   uint16_t result;
   
   result=bytes[0];
   result<<=8;
   result|=bytes[1];
   
   return result;
}

uint32_t OSReadBigInt32(const void *ptr,size_t offset){
   const uint8_t *bytes=ptr+offset;
   uint32_t result;
   
   result=bytes[0];
   result<<=8;
   result|=bytes[1];
   result<<=8;
   result|=bytes[2];
   result<<=8;
   result|=bytes[3];
   
   return result;
}

void OSWriteBigInt16(void *ptr,size_t offset,uint16_t value){
   uint8_t *bytes=ptr+offset;
   
   bytes[0]=(value>>8)&0xFF;
   bytes[1]=value&0xFF;
}

void OSWriteBigInt32(void *ptr,size_t offset,uint32_t value){
   uint8_t *bytes=ptr+offset;
   
   bytes[0]=(value>>24)&0xFF;
   bytes[1]=(value>>16)&0xFF;
   bytes[2]=(value>>8)&0xFF;
   bytes[3]=value&0xFF;
}

uint32_t OSSwapInt32(uint32_t value){
   uint32_t result;

   result=value<<24;
   result|=(value<<8)&0x00FF0000;
   result|=(value>>8)&0x0000FF00;
   result|=value>>24;

   return result;
}

uint64_t OSSwapInt64(uint64_t valueX){
   union {
    uint64_t word;
    uint8_t  bytes[8];
   } value,result;

   value.word=valueX;

   result.bytes[0]=value.bytes[7];
   result.bytes[1]=value.bytes[6];
   result.bytes[2]=value.bytes[5];
   result.bytes[3]=value.bytes[4];
   result.bytes[4]=value.bytes[3];
   result.bytes[5]=value.bytes[2];
   result.bytes[6]=value.bytes[1];
   result.bytes[7]=value.bytes[0];

   return result.word;
}

uint64_t OSSwapBigToHostInt64(uint64_t value){
#ifdef __LITTLE_ENDIAN__
   return OSSwapInt64(value);
#else
   return value;
#endif
}

uint32_t OSSwapHostToBigInt32(uint32_t value){
#ifdef __LITTLE_ENDIAN__
   return OSSwapInt32(value);
#else
   return value;
#endif
}

uint64_t OSSwapHostToBigInt64(uint64_t value){
#ifdef __LITTLE_ENDIAN__
   return OSSwapInt64(value);
#else
   return value;
#endif
}
