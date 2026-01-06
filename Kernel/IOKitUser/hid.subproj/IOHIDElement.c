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
#include <CoreFoundation/CFRuntime.h>
#include <CoreFoundation/CFArray.h>
#include "IOHIDElementPrivate.h"
#include <IOKit/hid/IOHIDLibUserClient.h>
#include <IOKit/hid/IOHIDPrivateKeys.h>
#include <IOKit/hid/IOHIDDevicePlugIn.h>
#include <IOKit/hid/IOHIDLibPrivate.h>
#include "IOHIDManagerPersistentProperties.h"
#include "HIDElementIvar.h"

typedef struct  __IOHIDElement {
    struct objc_object base;
    struct {
    HIDElementIvar
    };
} __IOHIDElement;

static IOHIDElementStruct * __IOHIDElementGetElementStruct(
                                    IOHIDElementRef         element);
static void                 _IOHIDElementAttach(
                                    IOHIDElementRef         element, 
                                    IOHIDElementRef         toAttach,
                                    Boolean                 propagate);
static void                 _IOHIDElementDetach(
                                    IOHIDElementRef         element, 
                                    IOHIDElementRef         toAttach,
                                    Boolean                 propagate);
static void                 __IOHIDElementApplyCalibration(
                                    IOHIDElementRef element);

static CFStringRef      __KIOHIDElementSpecialKeys[]    = {
    CFSTR(kIOHIDElementCalibrationMinKey),
    CFSTR(kIOHIDElementCalibrationMaxKey),
    CFSTR(kIOHIDElementCalibrationSaturationMinKey),
    CFSTR(kIOHIDElementCalibrationSaturationMaxKey),
    CFSTR(kIOHIDElementCalibrationMaxKey),
    CFSTR(kIOHIDElementCalibrationMaxKey),
    CFSTR(kIOHIDElementCalibrationMaxKey),
    NULL
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// _IOHIDElementReleasePrivate
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void _IOHIDElementReleasePrivate(IOHIDElementRef element)
{
    CFRELEASE_IF_NOT_NULL(element->attachedElements);
    CFRELEASE_IF_NOT_NULL(element->childElements);
    CFRELEASE_IF_NOT_NULL(element->parentElement);
    CFRELEASE_IF_NOT_NULL(element->data);
    CFRELEASE_IF_NOT_NULL(element->originalElement);
    CFRELEASE_IF_NOT_NULL(element->properties);
    CFRELEASE_IF_NOT_NULL(element->rootKey);
    CFRELEASE_IF_NOT_NULL(element->value);

    if (element->calibrationPtr)    free(element->calibrationPtr);
    element->calibrationPtr = NULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// __IOHIDElementGetElementStruct
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IOHIDElementStruct * __IOHIDElementGetElementStruct(IOHIDElementRef element)
{
    return element->elementStructPtr;
}

//------------------------------------------------------------------------------
// _IOHIDElementCreateWithParentAndData
//------------------------------------------------------------------------------
IOHIDElementRef _IOHIDElementCreateWithParentAndData(
                                        CFAllocatorRef          allocator, 
                                        IOHIDElementRef         parent, 
                                        CFDataRef               data, 
                                        IOHIDElementStruct *    elementStruct, 
                                        uint32_t                index)
{
    IOHIDElementRef element = NULL;
    
    if (!elementStruct)
        return (_Nonnull IOHIDElementRef)NULL;

    element = _IOHIDElementCreatePrivate(allocator);

    if (!element)
        return (_Nonnull IOHIDElementRef)NULL;
                    
    element->data               = (CFDataRef)CFRetain(data);
    element->elementStructPtr   = elementStruct;
    element->index              = index;
    element->parentElement      = (parent) ? (IOHIDElementRef)CFRetain(parent) : 0;

    return element;
}

//------------------------------------------------------------------------------
// _IOHIDElementCreateWithElement
//------------------------------------------------------------------------------
IOHIDElementRef _IOHIDElementCreateWithElement(
                                        CFAllocatorRef          allocator, 
                                        IOHIDElementRef         original, 
                                        uint32_t                usagePage, 
                                        uint32_t                usage)
{
    IOHIDElementRef element = NULL;
    
    if ( !original )
        return NULL;

    element = _IOHIDElementCreatePrivate(allocator);

    if (!element)
        return NULL;
                    
    element->index              = 0;
    element->originalElement     = (IOHIDElementRef)CFRetain(original);
    
    // Unlike normal IOHIDElements, this element does not reference IOHIDDeviceClass memory
    element->data               = CFDataCreateMutable(allocator, sizeof(IOHIDElementStruct));
    element->elementStructPtr   = (IOHIDElementStruct *)CFDataGetMutableBytePtr((CFMutableDataRef)element->data);
    
    bcopy(__IOHIDElementGetElementStruct(original), element->elementStructPtr, sizeof(IOHIDElementStruct));
    
   // To denote a mapped virual element, a simple approach would be to use
    // the negative cookie value of the original
    intptr_t cookie = (intptr_t)IOHIDElementGetCookie(original);
    element->elementStructPtr->cookieMin            = -cookie;
    element->elementStructPtr->cookieMax            = -cookie;
    
    element->elementStructPtr->usagePage            = usagePage;
    element->elementStructPtr->usageMin             = usage;
    element->elementStructPtr->usageMax             = usage;
    element->elementStructPtr->duplicateIndex       = 0;
    element->elementStructPtr->duplicateValueSize   = 0;
    
    return element;
}

//------------------------------------------------------------------------------
// _IOHIDElementSetDevice
//------------------------------------------------------------------------------
void _IOHIDElementSetDevice(IOHIDElementRef element, IOHIDDeviceRef device)
{
    element->device = device;
}

//------------------------------------------------------------------------------
// _IOHIDElementSetDeviceInterface
//------------------------------------------------------------------------------
void _IOHIDElementSetDeviceInterface(IOHIDElementRef element, IOHIDDeviceDeviceInterface ** interface)
{
    element->deviceInterface = interface;
}

//------------------------------------------------------------------------------
// _IOHIDElementGetLength
//------------------------------------------------------------------------------
CFIndex _IOHIDElementGetLength(IOHIDElementRef element)
{
    CFIndex bits = element->elementStructPtr->size;
    
    if ( element->elementStructPtr->duplicateValueSize && (element->index != 0))
        bits = element->elementStructPtr->reportSize;
        
    return (bits + 7) / 8;
}

//------------------------------------------------------------------------------
// IOHIDElementGetCookie
//------------------------------------------------------------------------------
IOHIDElementCookie IOHIDElementGetCookie(IOHIDElementRef element)
{
    return (IOHIDElementCookie)
                        (element->elementStructPtr->cookieMin + element->index);
}

//------------------------------------------------------------------------------
// IOHIDElementGetType
//------------------------------------------------------------------------------
IOHIDElementType IOHIDElementGetType(IOHIDElementRef element)
{
    return element->elementStructPtr->type;
}

//------------------------------------------------------------------------------
// IOHIDElementCreateWithDictionary
//------------------------------------------------------------------------------
IOHIDElementRef IOHIDElementCreateWithDictionary(
                                        CFAllocatorRef          allocator, 
                                        CFDictionaryRef         dictionary)
{
    IOHIDElementRef element = NULL;
    
    if ( !dictionary )
        return (_Nonnull IOHIDElementRef)NULL;

    element = _IOHIDElementCreatePrivate(allocator);

    if (!element)
        return (_Nonnull IOHIDElementRef)NULL;
                    
    element->index              = 0;
    
    // Unlike normal IOHIDElements, this element 
    // does not reference IOHIDDeviceClass memory
    element->data = CFDataCreateMutable( allocator, sizeof(IOHIDElementStruct));
    
    if ( !element->data ) {
        CFRelease(element);
        return (_Nonnull IOHIDElementRef)NULL;
    }
    
    element->elementStructPtr = (IOHIDElementStruct *)CFDataGetMutableBytePtr(
                                            (CFMutableDataRef)element->data);
    
    // fill in element->elementStructPtr

    return element;
}


//------------------------------------------------------------------------------
// IOHIDElementGetCollectionType
//------------------------------------------------------------------------------
IOHIDElementCollectionType IOHIDElementGetCollectionType(IOHIDElementRef element)
{
    return element->elementStructPtr->collectionType;
}

//------------------------------------------------------------------------------
// IOHIDElementGetUsagePage
//------------------------------------------------------------------------------
uint32_t IOHIDElementGetUsagePage(IOHIDElementRef element)
{
    return element->elementStructPtr->usagePage;
}

//------------------------------------------------------------------------------
// IOHIDElementGetUsagePage
//------------------------------------------------------------------------------
uint32_t IOHIDElementGetUsage(IOHIDElementRef element)
{
    return element->elementStructPtr->usageMin + ((element->elementStructPtr->usageMin != element->elementStructPtr->usageMax) ? element->index : 0);
}

//------------------------------------------------------------------------------
// _IOHIDElementGetFlags
//------------------------------------------------------------------------------
uint32_t _IOHIDElementGetFlags(IOHIDElementRef element)
{
    return element->elementStructPtr->flags;
}

Boolean IOHIDElementIsVirtual(IOHIDElementRef element)
{
    return ( element->originalElement != NULL );
}

//------------------------------------------------------------------------------
// IOHIDElementIsRelative
//------------------------------------------------------------------------------
Boolean IOHIDElementIsRelative(IOHIDElementRef element)
{
    return ((element->elementStructPtr->flags & kIOHIDElementFlagsRelativeMask) != 0);
}

//------------------------------------------------------------------------------
// IOHIDElementIsWrapping
//------------------------------------------------------------------------------
Boolean IOHIDElementIsWrapping(IOHIDElementRef element)
{
    return ((element->elementStructPtr->flags & kIOHIDElementFlagsWrapMask) != 0);
}

//------------------------------------------------------------------------------
// IOHIDElementIsArray
//------------------------------------------------------------------------------
Boolean IOHIDElementIsArray(IOHIDElementRef element)
{
    return ((element->elementStructPtr->flags & kIOHIDElementFlagsVariableMask) == 0);
}

//------------------------------------------------------------------------------
// IOHIDElementIsNonLinear
//------------------------------------------------------------------------------
Boolean IOHIDElementIsNonLinear(IOHIDElementRef element)
{
    return ((element->elementStructPtr->flags & kIOHIDElementFlagsNonLinearMask) != 0);
}

//------------------------------------------------------------------------------
// IOHIDElementHasPreferredState
//------------------------------------------------------------------------------
Boolean IOHIDElementHasPreferredState(IOHIDElementRef element)
{
    return ((element->elementStructPtr->flags & kIOHIDElementFlagsNoPreferredMask) == 0);
}

//------------------------------------------------------------------------------
// IOHIDElementHasNullState
//------------------------------------------------------------------------------
Boolean IOHIDElementHasNullState(IOHIDElementRef element)
{
    return (element->elementStructPtr->flags & kIOHIDElementFlagsNullStateMask);
}

//------------------------------------------------------------------------------
// IOHIDElementGetName
//------------------------------------------------------------------------------
CFStringRef IOHIDElementGetName(IOHIDElementRef element)
{
    CFTypeRef type = IOHIDElementGetProperty(   element, 
                                                CFSTR(kIOHIDElementNameKey));
        
    return (type && (CFGetTypeID(type) == CFStringGetTypeID())) ? 
                                                (CFStringRef)type : NULL;
}

//------------------------------------------------------------------------------
// IOHIDElementGetReportID
//------------------------------------------------------------------------------
uint32_t IOHIDElementGetReportID(IOHIDElementRef element)
{
    return element->elementStructPtr->reportID;
}

//------------------------------------------------------------------------------
// IOHIDElementGetReportSize
//------------------------------------------------------------------------------
uint32_t IOHIDElementGetReportSize(IOHIDElementRef element)
{
    return element->elementStructPtr->reportSize;
}

//------------------------------------------------------------------------------
// IOHIDElementGetReportCount
//------------------------------------------------------------------------------
uint32_t IOHIDElementGetReportCount(IOHIDElementRef element)
{
    uint32_t reportCount = element->elementStructPtr->reportCount;

    if ( element->elementStructPtr->duplicateValueSize && (element->index != 0))
        reportCount = 1;
    
    return reportCount;
}

//------------------------------------------------------------------------------
// IOHIDElementGetUnit
//------------------------------------------------------------------------------
uint32_t IOHIDElementGetUnit(IOHIDElementRef element)
{
    return element->elementStructPtr->unit;
}

//------------------------------------------------------------------------------
// IOHIDElementGetUnitExponent
//------------------------------------------------------------------------------
uint32_t IOHIDElementGetUnitExponent(IOHIDElementRef element)
{
    return element->elementStructPtr->unitExponent;
}

//------------------------------------------------------------------------------
// IOHIDElementGetLogicalMin
//------------------------------------------------------------------------------
CFIndex IOHIDElementGetLogicalMin(IOHIDElementRef element)
{
    return element->elementStructPtr->min;
}

//------------------------------------------------------------------------------
// IOHIDElementGetLogicalMax
//------------------------------------------------------------------------------
CFIndex IOHIDElementGetLogicalMax(IOHIDElementRef element)
{
    return element->elementStructPtr->max;
}

//------------------------------------------------------------------------------
// IOHIDElementGetPhysicalMin
//------------------------------------------------------------------------------
CFIndex IOHIDElementGetPhysicalMin(IOHIDElementRef element)
{
    return element->elementStructPtr->scaledMin;
}

//------------------------------------------------------------------------------
// IOHIDElementGetPhysicalMax
//------------------------------------------------------------------------------
CFIndex IOHIDElementGetPhysicalMax(IOHIDElementRef element)
{
    return element->elementStructPtr->scaledMax;
}

//------------------------------------------------------------------------------
// IOHIDElementGetDuplicateIndex
//------------------------------------------------------------------------------
uint32_t IOHIDElementGetDuplicateIndex(IOHIDElementRef element)
{
    uint32_t dupIndex = 0;

    if ( element->elementStructPtr->duplicateValueSize && (element->index != 0))
        dupIndex = element->index - 1;
    
    return dupIndex;
}

//------------------------------------------------------------------------------
// IOHIDElementGetDevice
//------------------------------------------------------------------------------
IOHIDDeviceRef IOHIDElementGetDevice(IOHIDElementRef element)
{
    return element->device;
}

//------------------------------------------------------------------------------
// IOHIDElementGetParent
//------------------------------------------------------------------------------
IOHIDElementRef IOHIDElementGetParent(IOHIDElementRef element)
{
    if (!element->parentElement && element->deviceInterface) {
        CFMutableDictionaryRef  matchingDict;
        CFArrayRef              elementArray;
        
        matchingDict = CFDictionaryCreateMutable(
                                            CFGetAllocator(element),
                                            1, 
                                            &kCFTypeDictionaryKeyCallBacks, 
                                            &kCFTypeDictionaryValueCallBacks);
        
        if ( matchingDict ) {                       
            uint32_t cookie = (uint32_t)element->elementStructPtr->parentCookie;
            CFNumberRef cookieNumber = CFNumberCreate(
                                            CFGetAllocator(element),
                                            kCFNumberIntType, 
                                            &cookie);
                                            
            CFDictionarySetValue(           matchingDict, 
                                            CFSTR(kIOHIDElementCookieKey), 
                                            cookieNumber);
            CFRelease(cookieNumber);
            
            (*(element->deviceInterface))->copyMatchingElements(
                                            element->deviceInterface, 
                                            matchingDict, 
                                            &elementArray, 
                                            0);
            
            if (elementArray) {
                element->parentElement = (IOHIDElementRef)CFRetain(
                                    CFArrayGetValueAtIndex(elementArray, 0));
                CFRelease(elementArray);
            }
            
            CFRelease(matchingDict);
        }
    }
    
    return element->parentElement;
}

//------------------------------------------------------------------------------
// IOHIDElementGetChildren
//------------------------------------------------------------------------------
CFArrayRef IOHIDElementGetChildren(IOHIDElementRef element)
{
    CFArrayRef childrenArray = NULL;
    
    if (!element->childElements && element->deviceInterface) {
        CFMutableDictionaryRef matchingDict;
        
        matchingDict = CFDictionaryCreateMutable(
                                            CFGetAllocator(element),
                                            1, 
                                            &kCFTypeDictionaryKeyCallBacks, 
                                            &kCFTypeDictionaryValueCallBacks);

        if ( matchingDict ) {                       
            uint32_t cookie = (uint32_t)IOHIDElementGetCookie(element);
            
            CFNumberRef cookieNumber = CFNumberCreate(
                                            CFGetAllocator(element),
                                            kCFNumberIntType, &cookie);
                                            
            CFDictionarySetValue(   matchingDict, 
                                    CFSTR(kIOHIDElementCollectionCookieKey), 
                                    cookieNumber);
                                    
            CFRelease(cookieNumber);
            
            (*(element->deviceInterface))->copyMatchingElements(
                                            element->deviceInterface, 
                                            matchingDict, &childrenArray, 0);
            
            if (childrenArray)
                element->childElements = childrenArray;
            
            CFRelease(matchingDict);
        }
    } else
        childrenArray = element->childElements;
    
    return childrenArray;
}

//------------------------------------------------------------------------------
// IOHIDElementAttach
//------------------------------------------------------------------------------
void IOHIDElementAttach(IOHIDElementRef element, IOHIDElementRef toAttach)
{
    _IOHIDElementAttach(element, toAttach, TRUE);
}

//------------------------------------------------------------------------------
// IOHIDElementDetach
//------------------------------------------------------------------------------
void IOHIDElementDetach(IOHIDElementRef element, IOHIDElementRef toDetach)
{
    _IOHIDElementDetach(element, toDetach, TRUE);
}

//------------------------------------------------------------------------------
// _IOHIDElementAttach
//------------------------------------------------------------------------------
void _IOHIDElementAttach(IOHIDElementRef element, IOHIDElementRef toAttach, Boolean propagate)
{
    if ( !element->attachedElements )
        element->attachedElements = CFArrayCreateMutable(   
                                        CFGetAllocator(element),
                                        0, 
                                        &kCFTypeArrayCallBacks);
             
    if ( !element->attachedElements )
        return;
        
    CFIndex index = CFArrayGetFirstIndexOfValue(    
                        element->attachedElements, 
                        CFRangeMake(0, CFArrayGetCount(element->attachedElements)),
                        toAttach);
                                            
    if ( index != kCFNotFound )
        return;
        
    CFArrayAppendValue(element->attachedElements, toAttach);
    
    if ( propagate )
        _IOHIDElementAttach(toAttach, element, FALSE);
}

//------------------------------------------------------------------------------
// _IOHIDElementDetach
//------------------------------------------------------------------------------
void _IOHIDElementDetach(IOHIDElementRef element, IOHIDElementRef toDetach, Boolean propagate)
{
    if ( !element->attachedElements )
        return;
        
    CFIndex index = CFArrayGetFirstIndexOfValue(    
                        element->attachedElements, 
                        CFRangeMake(0, CFArrayGetCount(element->attachedElements)),
                        toDetach);
                                            
    if ( index == kCFNotFound )
        return;
        
    CFArrayRemoveValueAtIndex(element->attachedElements, index);
    
    if ( propagate )
        _IOHIDElementDetach(toDetach, element, FALSE);
}

//------------------------------------------------------------------------------
// IOHIDElementCopyAttached
//------------------------------------------------------------------------------
CFArrayRef IOHIDElementCopyAttached(IOHIDElementRef element)
{
    return element->attachedElements ? 
            CFArrayCreateCopy(CFGetAllocator(element), element->attachedElements) :
            NULL;
}


//------------------------------------------------------------------------------
// _IOHIDElementGetValue
//------------------------------------------------------------------------------
IOHIDValueRef _IOHIDElementGetValue(IOHIDElementRef element)
{
    return element->value;
}

//------------------------------------------------------------------------------
// _IOHIDElementSetValue
//------------------------------------------------------------------------------
void _IOHIDElementSetValue(IOHIDElementRef element, IOHIDValueRef value)
{
    if (element->value) {
        CFRelease(element->value);
        element->value = NULL;
    }
    
    if (value) {
        IOHIDValueRef new = _IOHIDValueCreateWithValue(kCFAllocatorDefault,
                                                       value,
                                                       element);
        element->value = new;
    }
}

//------------------------------------------------------------------------------
// _IOHIDElementGetCalibrationInfo
//------------------------------------------------------------------------------
IOHIDCalibrationInfo * _IOHIDElementGetCalibrationInfo(IOHIDElementRef element)
{
    return element->calibrationPtr;
}

//------------------------------------------------------------------------------
// IOHIDElementGetProperty
//------------------------------------------------------------------------------
CFTypeRef IOHIDElementGetProperty(IOHIDElementRef element, CFStringRef key)
{
    if ( !element->properties )
        return NULL;
        
    return CFDictionaryGetValue(element->properties, key);
}

//------------------------------------------------------------------------------
// IOHIDElementSetProperty
//------------------------------------------------------------------------------
CF_EXPORT 
Boolean IOHIDElementSetProperty(            IOHIDElementRef         element, 
                                            CFStringRef             key, 
                                            CFTypeRef               property)
{
    if ( !element->properties ) {
        element->properties = CFDictionaryCreateMutable(CFGetAllocator(element), 
                                                        0, 
                                                        &kCFTypeDictionaryKeyCallBacks, 
                                                        &kCFTypeDictionaryValueCallBacks);
        
        if ( !element->properties )
            return FALSE;
    }
    
    boolean_t   isCalMin = FALSE;
    boolean_t   isCalMax = FALSE;
    boolean_t   isSatMin = FALSE;
    boolean_t   isSatMax = FALSE;
    boolean_t   isDZMin  = FALSE;
    boolean_t   isDZMax  = FALSE;
    boolean_t   isGran   = FALSE;
     
    element->isDirty = TRUE;

    if ((CFGetTypeID(property) == CFNumberGetTypeID()) && (
        (isCalMin   = CFEqual(key, CFSTR(kIOHIDElementCalibrationMinKey))) || 
        (isCalMax   = CFEqual(key, CFSTR(kIOHIDElementCalibrationMaxKey))) || 
        (isSatMin   = CFEqual(key, CFSTR(kIOHIDElementCalibrationSaturationMinKey))) || 
        (isSatMax   = CFEqual(key, CFSTR(kIOHIDElementCalibrationSaturationMaxKey))) || 
        (isDZMin    = CFEqual(key, CFSTR(kIOHIDElementCalibrationDeadZoneMinKey))) || 
        (isDZMax    = CFEqual(key, CFSTR(kIOHIDElementCalibrationDeadZoneMaxKey))) ||
        (isGran     = CFEqual(key, CFSTR(kIOHIDElementCalibrationGranularityKey))))) {
            
        if ( !element->calibrationPtr ) {
            element->calibrationPtr = 
            (IOHIDCalibrationInfo *)malloc(sizeof(IOHIDCalibrationInfo));
            
            bzero(element->calibrationPtr, sizeof(IOHIDCalibrationInfo));
        }
        
        if ( element->calibrationPtr ) {
            
            CFIndex value = 0;
            CFNumberGetValue(property, kCFNumberCFIndexType, &value);
            
            if ( isCalMin )
                element->calibrationPtr->min = value;
            else if ( isCalMax )
                element->calibrationPtr->max = value;
            else if ( isSatMin )
                element->calibrationPtr->satMin = value;
            else if ( isSatMax )
                element->calibrationPtr->satMax = value;
            else if ( isDZMin )
                element->calibrationPtr->dzMin  = value;
            else if ( isDZMax )
                element->calibrationPtr->dzMax  = value;
            else if ( isGran )
                CFNumberGetValue(property, kCFNumberFloat64Type, &element->calibrationPtr->gran);
        }
            
    }
        
    CFDictionarySetValue(element->properties, key, property);
   
    return TRUE;
}

//------------------------------------------------------------------------------
CFStringRef __IOHIDElementGetRootKey(IOHIDElementRef element)
{
    if (!element->rootKey) {
        // Device Root Key
        // All *required* matching information
        CFStringRef device = __IOHIDDeviceGetUUIDKey(element->device);
        long int usagePage = (long int)IOHIDElementGetUsagePage(element);
        long int usage = (long int)IOHIDElementGetUsage(element);
        long int cookie = (long int)IOHIDElementGetCookie(element);
        long int type = (long int)IOHIDElementGetType(element);
        
        element->rootKey = CFStringCreateWithFormat(NULL, NULL, 
                                                    CFSTR("%@#%04lx#%04lx#%016lx#%ld"), 
                                                    device,
                                                    usagePage,
                                                    usage,
                                                    cookie,
                                                    type);
    }
    
    return element->rootKey;
}

//------------------------------------------------------------------------------
void __IOHIDElementSaveProperties(IOHIDElementRef element, __IOHIDPropertyContext *context)
{
    if (element->isDirty && element->properties) {
        __IOHIDPropertySaveToKeyWithSpecialKeys(element->properties, __IOHIDElementGetRootKey(element), __KIOHIDElementSpecialKeys, context);
        element->isDirty = FALSE;
    }
}

//------------------------------------------------------------------------------
void __IOHIDElementLoadProperties(IOHIDElementRef element)
{
    CFMutableDictionaryRef properties = __IOHIDPropertyLoadFromKeyWithSpecialKeys(__IOHIDElementGetRootKey(element), __KIOHIDElementSpecialKeys);
    
    if (properties) {
        CFRELEASE_IF_NOT_NULL(element->properties);
        element->properties = properties;
        __IOHIDElementApplyCalibration(element);
        element->isDirty = FALSE;
    }
}

//------------------------------------------------------------------------------
void __IOHIDElementApplyCalibration(IOHIDElementRef element)
{
    if (element->properties) {
        CFNumberRef property;
        
        property = CFDictionaryGetValue(element->properties, CFSTR(kIOHIDElementCalibrationMinKey));
        if (property && (CFGetTypeID(property) == CFNumberGetTypeID())) {
            CFNumberGetValue(property, kCFNumberCFIndexType, &element->calibrationPtr->min);
        }
        
        property = CFDictionaryGetValue(element->properties, CFSTR(kIOHIDElementCalibrationMaxKey));
        if (property && (CFGetTypeID(property) == CFNumberGetTypeID())) {
            CFNumberGetValue(property, kCFNumberCFIndexType, &element->calibrationPtr->max);
        }
        
        property = CFDictionaryGetValue(element->properties, CFSTR(kIOHIDElementCalibrationSaturationMinKey));
        if (property && (CFGetTypeID(property) == CFNumberGetTypeID())) {
            CFNumberGetValue(property, kCFNumberCFIndexType, &element->calibrationPtr->satMin);
        }
        
        property = CFDictionaryGetValue(element->properties, CFSTR(kIOHIDElementCalibrationSaturationMaxKey));
        if (property && (CFGetTypeID(property) == CFNumberGetTypeID())) {
            CFNumberGetValue(property, kCFNumberCFIndexType, &element->calibrationPtr->satMax);
        }
        
        property = CFDictionaryGetValue(element->properties, CFSTR(kIOHIDElementCalibrationDeadZoneMinKey));
        if (property && (CFGetTypeID(property) == CFNumberGetTypeID())) {
            CFNumberGetValue(property, kCFNumberCFIndexType, &element->calibrationPtr->dzMin);
        }
        
        property = CFDictionaryGetValue(element->properties, CFSTR(kIOHIDElementCalibrationDeadZoneMaxKey));
        if (property && (CFGetTypeID(property) == CFNumberGetTypeID())) {
            CFNumberGetValue(property, kCFNumberCFIndexType, &element->calibrationPtr->dzMax);
        }
        
        property = CFDictionaryGetValue(element->properties, CFSTR(kIOHIDElementCalibrationGranularityKey));
        if (property && (CFGetTypeID(property) == CFNumberGetTypeID())) {
            CFNumberGetValue(property, kCFNumberFloat64Type, &element->calibrationPtr->gran);
        }
    }
}

//------------------------------------------------------------------------------
void __IOHIDSaveElementSet(const void *value, void *context) {
    IOHIDElementRef element = (IOHIDElementRef)value;
    if (element)
        __IOHIDElementSaveProperties(element, (__IOHIDPropertyContext*)context);
}

//------------------------------------------------------------------------------
void __IOHIDLoadElementSet(const void *value, void *context __unused) {
    IOHIDElementRef element = (IOHIDElementRef)value;
    if (element)
        __IOHIDElementLoadProperties(element);
}

//------------------------------------------------------------------------------
