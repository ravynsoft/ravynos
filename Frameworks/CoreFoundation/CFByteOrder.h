/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreFoundation/CFBase.h>

typedef enum {
    CFByteOrderUnknown,
    CFByteOrderLittleEndian,
    CFByteOrderBigEndian
} CFByteOrder;

typedef struct {
    uint32_t v;
} CFSwappedFloat32;

typedef struct {
    uint64_t v;
} CFSwappedFloat64;

COREFOUNDATION_EXPORT CFByteOrder CFByteOrderGetCurrent(void);
COREFOUNDATION_EXPORT CFSwappedFloat64 CFConvertDoubleHostToSwapped(double value);
COREFOUNDATION_EXPORT double CFConvertDoubleSwappedToHost(CFSwappedFloat64 value);
COREFOUNDATION_EXPORT CFSwappedFloat32 CFConvertFloat32HostToSwapped(Float32 value);
COREFOUNDATION_EXPORT Float32 CFConvertFloat32SwappedToHost(CFSwappedFloat32 value);
COREFOUNDATION_EXPORT CFSwappedFloat64 CFConvertFloat64HostToSwapped(Float64 value);
COREFOUNDATION_EXPORT Float64 CFConvertFloat64SwappedToHost(CFSwappedFloat64 value);
COREFOUNDATION_EXPORT CFSwappedFloat32 CFConvertFloatHostToSwapped(float value);
COREFOUNDATION_EXPORT float CFConvertFloatSwappedToHost(CFSwappedFloat32 value);

COREFOUNDATION_EXPORT uint16_t CFSwapInt16(uint16_t value);
COREFOUNDATION_EXPORT uint16_t CFSwapInt16BigToHost(uint16_t value);
COREFOUNDATION_EXPORT uint16_t CFSwapInt16HostToBig(uint16_t value);
COREFOUNDATION_EXPORT uint16_t CFSwapInt16HostToLittle(uint16_t value);
COREFOUNDATION_EXPORT uint16_t CFSwapInt16LittleToHost(uint16_t value);
COREFOUNDATION_EXPORT uint32_t CFSwapInt32(uint32_t value);
COREFOUNDATION_EXPORT uint32_t CFSwapInt32BigToHost(uint32_t value);
COREFOUNDATION_EXPORT uint32_t CFSwapInt32HostToBig(uint32_t value);
COREFOUNDATION_EXPORT uint32_t CFSwapInt32HostToLittle(uint32_t value);
COREFOUNDATION_EXPORT uint32_t CFSwapInt32LittleToHost(uint32_t value);
COREFOUNDATION_EXPORT uint64_t CFSwapInt64(uint64_t value);
COREFOUNDATION_EXPORT uint64_t CFSwapInt64BigToHost(uint64_t value);
COREFOUNDATION_EXPORT uint64_t CFSwapInt64HostToBig(uint64_t value);
COREFOUNDATION_EXPORT uint64_t CFSwapInt64HostToLittle(uint64_t value);
COREFOUNDATION_EXPORT uint64_t CFSwapInt64LittleToHost(uint64_t value);

COREFOUNDATION_EXPORT uint16_t OSReadBigInt16(const void *ptr, size_t offset);
COREFOUNDATION_EXPORT uint32_t OSReadBigInt32(const void *ptr, size_t offset);
COREFOUNDATION_EXPORT void OSWriteBigInt16(void *ptr, size_t offset, uint16_t value);
COREFOUNDATION_EXPORT void OSWriteBigInt32(void *ptr, size_t offset, uint32_t value);
COREFOUNDATION_EXPORT uint32_t OSSwapInt32(uint32_t value);
COREFOUNDATION_EXPORT uint64_t OSSwapInt64(uint64_t valueX);
COREFOUNDATION_EXPORT uint64_t OSSwapBigToHostInt64(uint64_t value);
COREFOUNDATION_EXPORT uint32_t OSSwapHostToBigInt32(uint32_t value);
COREFOUNDATION_EXPORT uint64_t OSSwapHostToBigInt64(uint64_t value);
