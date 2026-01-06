/*
 * Copyright (c) 1999-2008 Apple Computer, Inc.  All Rights Reserved.
 * 
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include <pthread.h>
#include <System/libkern/OSCrossEndian.h>
#include <CoreFoundation/CFRuntime.h>
#include <CoreFoundation/CFData.h>
#include <IOKit/hid/IOHIDValue.h>
#include <IOKit/hid/IOHIDElement.h>
#include <IOKit/hid/IOHIDLibUserClient.h>
#include <IOKit/hid/IOHIDLibPrivate.h>

#ifndef min
    #define min(a,b) (a<b?a:b)
#endif

static IOHIDValueRef __IOHIDValueCreatePrivate(CFAllocatorRef allocator, CFAllocatorContext * context __unused, size_t extraBytes);
static void __IOHIDValueRelease( CFTypeRef object );
static void __IOHIDValueConvertByteToWord(const UInt8 * src, uint32_t * dst, uint32_t bytesToCopy, Boolean signExtend);
static void __IOHIDValueConvertWordToByte(const uint32_t * src, UInt8 * dst, uint32_t bytesToCopy);
static void __IOHIDValueConvertByteToLongWord(const UInt8 * src, uint64_t * dst, uint64_t bytesToCopy, Boolean signExtend);
static void __IOHIDValueConvertLongWordToByte(const uint64_t * src, UInt8 * dst, uint64_t bytesToCopy);
typedef struct __IOHIDValue
{
    CFRuntimeBase               cfBase;   // base CFType information
    
    IOHIDElementRef             element;
    bool                        weakElementRef;
    uint64_t                    timeStamp;
    uint32_t                    length;
    uint8_t *                   bytePtr;
    uint8_t                     bytes[];
} __IOHIDValue, *__IOHIDValueRef;

static const CFRuntimeClass __IOHIDValueClass = {
    0,                      // version
    "IOHIDValue",           // className
    NULL,                   // init
    NULL,                   // copy
    __IOHIDValueRelease,    // finalize
    NULL,                   // equal
    NULL,                   // hash
    NULL,                   // copyFormattingDesc
    NULL,
    NULL,
    NULL
};

static CFTypeID         __valueTypeID   = _kCFRuntimeNotATypeID;
static pthread_once_t   __valueTypeInit = PTHREAD_ONCE_INIT;

void __IOHIDValueRegister(void)
{
    __valueTypeID = _CFRuntimeRegisterClass(&__IOHIDValueClass);
}

CFTypeID IOHIDValueGetTypeID(void)
{
    /* initialize runtime */
    if ( __valueTypeID == _kCFRuntimeNotATypeID )
        pthread_once(&__valueTypeInit, __IOHIDValueRegister);
        
    return __valueTypeID;
}

IOHIDValueRef __IOHIDValueCreatePrivate(CFAllocatorRef allocator, CFAllocatorContext * context __unused, size_t dataLength)
{
    IOHIDValueRef       event   = NULL;
    void *              offset  = NULL;
    uint32_t            size;
    
    /* allocate session */
    size  = sizeof(__IOHIDValue) - sizeof(CFRuntimeBase) + dataLength;
    event = (IOHIDValueRef)_CFRuntimeCreateInstance(allocator, IOHIDValueGetTypeID(), size, NULL);
    
    if (!event)
        return NULL;

    offset = event;
    bzero(offset + sizeof(CFRuntimeBase), size);
    
    return event;
}

void __IOHIDValueRelease( CFTypeRef object )
{
    IOHIDValueRef event = ( IOHIDValueRef ) object;
    
    if (event->element && !event->weakElementRef) CFRelease(event->element);
}

IOHIDValueRef _IOHIDValueCreateWithElementValuePtr(CFAllocatorRef allocator, IOHIDElementRef element, IOHIDElementValue * pElementValue)
{
    IOHIDValueRef   event   = NULL;
    uint32_t        length  = 0;

    if ( !element || !pElementValue )
        return (_Nonnull IOHIDValueRef)NULL;
        
    length  =  min(_IOHIDElementGetLength(element), (CFIndex)(pElementValue->totalSize - sizeof(*pElementValue) + sizeof(pElementValue->value)));
    event   = __IOHIDValueCreatePrivate(allocator, NULL, length);

    if (!event)
        return (_Nonnull IOHIDValueRef)NULL;
        
    event->element      = (IOHIDElementRef)CFRetain(element);
    event->timeStamp    = *((uint64_t *)&(pElementValue->timestamp));
    event->length       = length;
  
    __IOHIDValueConvertWordToByte((const uint32_t *)&(pElementValue->value[0]), event->bytes, length);
    
    return event;
}

IOHIDValueRef _IOHIDValueCreateWithStruct(CFAllocatorRef allocator, IOHIDElementRef element, IOHIDEventStruct * pEventStruct)
{
    IOHIDValueRef   event       = NULL;
    Boolean         isLongValue = FALSE;
    uint32_t        length      = 0;

    if ( !element || !pEventStruct )
        return (_Nonnull IOHIDValueRef)NULL;
        
    isLongValue = (pEventStruct->longValue && pEventStruct->longValueSize);
    length  = _IOHIDElementGetLength(element);
    event   = __IOHIDValueCreatePrivate(allocator, NULL, isLongValue ? 0 : length);

    if (!event)
        return (_Nonnull IOHIDValueRef)NULL;
        
    event->element      = (IOHIDElementRef)CFRetain(element);
    event->timeStamp    = *((uint64_t *)&(pEventStruct->timestamp));
    event->length       = length;

    if ( isLongValue )
    {
        event->bytePtr  = pEventStruct->longValue;
    }
    else
        __IOHIDValueConvertWordToByte((const uint32_t *)&(pEventStruct->value), event->bytes, min(sizeof(uint32_t), event->length));

    return event;
}

IOHIDValueRef IOHIDValueCreateWithIntegerValue(CFAllocatorRef allocator, IOHIDElementRef element, uint64_t timeStamp, CFIndex value)
{
    IOHIDValueRef   event   = NULL;
    uint32_t        length  = 0;
    uint64_t        tempValue;

    if ( !element )
        return (_Nonnull IOHIDValueRef)NULL;

    length  = _IOHIDElementGetLength(element);
    event   = __IOHIDValueCreatePrivate(allocator, NULL, length);

    if (!event)
        return (_Nonnull IOHIDValueRef)NULL;
        
    event->element      = (IOHIDElementRef)CFRetain(element);
    event->timeStamp    = timeStamp;
    event->length       = length;
        
    tempValue = value;
    
    __IOHIDValueConvertLongWordToByte((const uint64_t *)&tempValue, event->bytes, min(length, sizeof(uint64_t)));
    
    return event;
}

IOHIDValueRef IOHIDValueCreateWithBytes(CFAllocatorRef allocator, IOHIDElementRef element, uint64_t timeStamp, const uint8_t * bytes, CFIndex byteLength)
{
    IOHIDValueRef   event   = NULL;
    CFIndex         length  = 0;

    if ( !element || !bytes || !byteLength )
        return NULL;

    length  = _IOHIDElementGetLength(element);
    event   = __IOHIDValueCreatePrivate(allocator, NULL, length);

    if (!event)
        return NULL;
        
    event->element      = (IOHIDElementRef)CFRetain(element);
    event->timeStamp    = timeStamp;
    event->length       = length;
    
    bcopy(bytes, event->bytes, min(length, byteLength));

    return event;
}

IOHIDValueRef IOHIDValueCreateWithBytesNoCopy(CFAllocatorRef allocator, IOHIDElementRef element, uint64_t timeStamp, const uint8_t * bytes, CFIndex length)
{
    IOHIDValueRef   event   = NULL;

    if ( !element || !bytes || !length )
        return NULL;

    event = __IOHIDValueCreatePrivate(allocator, NULL, 0);

    if (!event)
        return NULL;
        
    event->element      = (IOHIDElementRef)CFRetain(element);
    event->timeStamp    = timeStamp;
    event->length       = min(length, _IOHIDElementGetLength(element));
    event->bytePtr      = (uint8_t *)bytes;
    
    return event;
}

// creates a IOHIDValueRef that doesn't cause a retain cycle between element <-> value
// the assumption here is that the value will only exist during the lifetime
// of the element.
IOHIDValueRef _IOHIDValueCreateWithValue(CFAllocatorRef allocator, IOHIDValueRef value, IOHIDElementRef element)
{
    IOHIDValueRef result = NULL;
    
    result = __IOHIDValueCreatePrivate(allocator, NULL, value->length);
    
    if (!result) {
        return NULL;
    }
    
    result->element         = element; // NO RETAIN!
    result->weakElementRef  = true; // So we don't release later
    result->timeStamp       = value->timeStamp;
    result->length          = value->length;
    bcopy(value->bytes, result->bytes, value->length);
    
    return result;
}


IOHIDElementRef IOHIDValueGetElement(IOHIDValueRef event)
{
    return event->element;
}

uint64_t IOHIDValueGetTimeStamp(IOHIDValueRef event)
{
    return event->timeStamp;
}

CFIndex IOHIDValueGetIntegerValue(IOHIDValueRef event)
{    
    uint64_t value = 0;
    IOHIDElementRef element = event->element;
    __IOHIDValueConvertByteToLongWord(IOHIDValueGetBytePtr(event), &value, MIN(event->length, sizeof(value)), IOHIDElementGetLogicalMin(element) < 0 || IOHIDElementGetLogicalMax(element) < 0);
    return (CFIndex)value;
}

double_t IOHIDValueGetScaledValue(IOHIDValueRef event, IOHIDValueScaleType type)
{
    IOHIDElementRef element         = event->element;
    CFIndex         logicalValue    = IOHIDValueGetIntegerValue(event);
    CFIndex         logicalMin      = IOHIDElementGetLogicalMin(element);
    CFIndex         logicalMax      = IOHIDElementGetLogicalMax(element);
    CFIndex         logicalRange    = 0;
    CFIndex         scaledMin       = IOHIDElementGetPhysicalMin(element);
    CFIndex         scaledMax       = IOHIDElementGetPhysicalMax(element);
    CFIndex         scaledRange     = 0;
    double_t        granularity     = 0.0;
    double_t        returnValue     = 0.0;
    double_t        exponent        = 0.0;

    if ( type == kIOHIDValueScaleTypeCalibrated ){
        IOHIDCalibrationInfo * calibrationInfo;

        calibrationInfo = _IOHIDElementGetCalibrationInfo(element);

        if ( calibrationInfo ) {
            if ( calibrationInfo->min != calibrationInfo->max ) {
                scaledMin = calibrationInfo->min;
                scaledMax = calibrationInfo->max;
            } else {
                scaledMin = -1;
                scaledMax = 1;
            }

            // check saturation first
            if ( calibrationInfo->satMin != calibrationInfo->satMax ) {
                if ( logicalValue <= calibrationInfo->satMin )
                    return scaledMin;
                if ( logicalValue >= calibrationInfo->satMax )
                    return scaledMax;

                logicalMin      = calibrationInfo->satMin;
                logicalMax      = calibrationInfo->satMax;
            }

           // now check the dead zone
           if (calibrationInfo->dzMin != calibrationInfo->dzMax) {
                double_t scaledMid = scaledMin + ((scaledMax - scaledMin) / 2.0);
                if (logicalValue < calibrationInfo->dzMin) {
                    logicalMax = calibrationInfo->dzMin;
                    scaledMax = scaledMid;
                } else if ( logicalValue > calibrationInfo->dzMax) {
                    logicalMin = calibrationInfo->dzMax;
                    scaledMin = scaledMid;
                } else {
                    return scaledMid;
                }
            }

            granularity = calibrationInfo->gran;
        }
    } else if (type == kIOHIDValueScaleTypeExponent) {
        uint8_t unitExp = IOHIDElementGetUnitExponent(element) & 0x0F;
        
        // Per HID spec:
        // Exponent value of 0x1 - 0x7 is a positive exponent. Exponent value of
        // 0x8 - 0xF is a negative exponent, where 0x8 is 10^-8 and 0xF is 10^-1
        exponent = unitExp < 8 ? pow(10, unitExp) : 1.0 / pow(10, 0x10 - unitExp);
    }
    
    logicalRange    = logicalMax - logicalMin;
    scaledRange     = scaledMax - scaledMin;
    returnValue     = ((double_t)(logicalValue - logicalMin) * (double_t)scaledRange / (double_t)logicalRange) + scaledMin;
    
    if (exponent) {
        returnValue *= exponent;
    }

    if ( granularity )
        returnValue = granularity * llround(returnValue / granularity);

    return returnValue;
}

CFIndex IOHIDValueGetLength(IOHIDValueRef event)
{    
    return event->length;
}

const uint8_t * IOHIDValueGetBytePtr(IOHIDValueRef event)
{    
    return event->bytePtr ? event->bytePtr : event->bytes;
}

void _IOHIDValueCopyToElementValuePtr(IOHIDValueRef value, IOHIDElementValue * pElementValue)
{
    IOHIDElementRef element = IOHIDValueGetElement(value);
    
    __IOHIDValueConvertByteToWord(IOHIDValueGetBytePtr(value), (uint32_t *)(pElementValue->value), value->length, IOHIDElementGetLogicalMin(element) < 0 || IOHIDElementGetLogicalMax(element) < 0);
}

#if defined (__LITTLE_ENDIAN__) 
    #define ON_INTEL 1
#else
    #define ON_INTEL 0
#endif

#define BIT_MASK(bits)  (((uint64_t)1 << (bits)) - 1)

void __IOHIDValueConvertByteToWord(const UInt8 * src, uint32_t * dst, uint32_t length, Boolean signExtend)
{
    bcopy(src, dst, length);
    
    if ( signExtend && length )
    {
        uint32_t lastOffset = length / sizeof(uint32_t);
        uint32_t wordBitsProcessed = length % sizeof(uint32_t);

        lastOffset += (wordBitsProcessed) ? 0:-1;
        wordBitsProcessed = wordBitsProcessed << 3;
    
        if (wordBitsProcessed && (dst[lastOffset] & (1<<(wordBitsProcessed-1))))
            dst[lastOffset] |= ~(BIT_MASK(wordBitsProcessed));
    }
}

void __IOHIDValueConvertWordToByte(const uint32_t * src, UInt8 * dst, uint32_t bytesToCopy)
{
    bcopy(src, dst, bytesToCopy);
}

void __IOHIDValueConvertByteToLongWord(const UInt8 * src, uint64_t * dst, uint64_t length, Boolean signExtend)
{    
    bcopy(src, dst, length);
    
    if ( signExtend && length )
    {
        uint32_t lastOffset = length / sizeof(uint64_t);
        uint32_t longWordBitsProcessed = length % sizeof(uint64_t);

        lastOffset += (longWordBitsProcessed) ? 0:-1;
        longWordBitsProcessed = longWordBitsProcessed << 3;
    
        if (longWordBitsProcessed && (dst[lastOffset] & (1<<(longWordBitsProcessed-1))))
            dst[lastOffset] |= ~(BIT_MASK(longWordBitsProcessed));
    }
}

void __IOHIDValueConvertLongWordToByte(const uint64_t * src, UInt8 * dst, uint64_t bytesToCopy)
{
    bcopy(src, dst, bytesToCopy);
}
