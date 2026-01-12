/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

/*	CFICUConverters.c
	Copyright (c) 2004-2014, Apple Inc. All rights reserved.
	Responsibility: Aki Inoue
*/

#include "CFStringEncodingDatabase.h"
#include "CFStringEncodingConverterPriv.h"
#include "CFICUConverters.h"
#include <CoreFoundation/CFStringEncodingExt.h>
#include <CoreFoundation/CFUniChar.h>
#include <unicode/ucnv.h>
#include <unicode/uversion.h>
#include "CFInternal.h"
#include <stdio.h>

// Thread data support
typedef struct {
    uint8_t _numSlots;
    uint8_t _nextSlot;
    UConverter **_converters;
} __CFICUThreadData;

static void __CFICUThreadDataDestructor(void *context) {
    __CFICUThreadData * data = (__CFICUThreadData *)context;
    
    if (NULL != data->_converters) { // scan to make sure deallocation
        UConverter **converter = data->_converters;
        UConverter **limit = converter + data->_numSlots;
        
        while (converter < limit) {
            if (NULL != converter) ucnv_close(*converter);
            ++converter;
        }
        CFAllocatorDeallocate(NULL, data->_converters);
    }
    
    CFAllocatorDeallocate(NULL, data);
}

CF_INLINE __CFICUThreadData *__CFStringEncodingICUGetThreadData() {
    __CFICUThreadData * data;

    data = (__CFICUThreadData *)_CFGetTSD(__CFTSDKeyICUConverter);

    if (NULL == data) {
        data = (__CFICUThreadData *)CFAllocatorAllocate(NULL, sizeof(__CFICUThreadData), 0);
        memset(data, 0, sizeof(__CFICUThreadData));
        _CFSetTSD(__CFTSDKeyICUConverter, (void *)data, __CFICUThreadDataDestructor);
    }

    return data;
}

CF_PRIVATE const char *__CFStringEncodingGetICUName(CFStringEncoding encoding) {
#define STACK_BUFFER_SIZE (60)
    char buffer[STACK_BUFFER_SIZE];
    const char *result = NULL;
    UErrorCode errorCode = U_ZERO_ERROR;
    uint32_t codepage = 0;

    if (kCFStringEncodingUTF7_IMAP == encoding) return "IMAP-mailbox-name";

    if (kCFStringEncodingUnicode != (encoding & 0x0F00)) codepage = __CFStringEncodingGetWindowsCodePage(encoding); // we don't use codepage for UTF to avoid little endian weirdness of Windows

    if ((0 != codepage) && (snprintf(buffer, STACK_BUFFER_SIZE, "windows-%d", codepage) < STACK_BUFFER_SIZE) && (NULL != (result = ucnv_getAlias(buffer, 0, &errorCode)))) return result;

    if (__CFStringEncodingGetCanonicalName(encoding, buffer, STACK_BUFFER_SIZE)) result = ucnv_getAlias(buffer, 0, &errorCode);

    return result;
#undef STACK_BUFFER_SIZE
}

CF_PRIVATE CFStringEncoding __CFStringEncodingGetFromICUName(const char *icuName) {
    uint32_t codepage;
    char *endPtr;
    UErrorCode errorCode = U_ZERO_ERROR;

    if ((0 == strncasecmp_l(icuName, "windows-", strlen("windows-"), NULL)) && (0 != (codepage = strtol(icuName + strlen("windows-"), &endPtr, 10))) && (*endPtr == '\0')) return __CFStringEncodingGetFromWindowsCodePage(codepage);

    if (0 != ucnv_countAliases(icuName, &errorCode)) {
        CFStringEncoding encoding;
        const char *name;

        // Try WINDOWS platform
        name = ucnv_getStandardName(icuName, "WINDOWS", &errorCode);
        
        if (NULL != name) {
            if ((0 == strncasecmp_l(name, "windows-", strlen("windows-"), NULL)) && (0 != (codepage = strtol(name + strlen("windows-"), &endPtr, 10))) && (*endPtr == '\0')) return __CFStringEncodingGetFromWindowsCodePage(codepage);
            
            if (strncasecmp_l(icuName, name, strlen(name), NULL) && (kCFStringEncodingInvalidId != (encoding = __CFStringEncodingGetFromCanonicalName(name)))) return encoding;
        }

        // Try JAVA platform
        name = ucnv_getStandardName(icuName, "JAVA", &errorCode);
        if ((NULL != name) && strncasecmp_l(icuName, name, strlen(name), NULL) && (kCFStringEncodingInvalidId != (encoding = __CFStringEncodingGetFromCanonicalName(name)))) return encoding;

        // Try MIME platform
        name = ucnv_getStandardName(icuName, "MIME", &errorCode);
        if ((NULL != name) && strncasecmp_l(icuName, name, strlen(name), NULL) && (kCFStringEncodingInvalidId != (encoding = __CFStringEncodingGetFromCanonicalName(name)))) return encoding;
    }

    return kCFStringEncodingInvalidId;
}

CF_INLINE UConverter *__CFStringEncodingConverterCreateICUConverter(const char *icuName, uint32_t flags, bool toUnicode) {
    UConverter *converter;
    UErrorCode errorCode = U_ZERO_ERROR;
    uint8_t streamID = CFStringEncodingStreamIDFromMask(flags);

    if (0 != streamID) { // this is a part of streaming previously created
        __CFICUThreadData *data = __CFStringEncodingICUGetThreadData();

        --streamID; // map to array index

        if ((streamID < data->_numSlots) && (NULL != data->_converters[streamID])) return data->_converters[streamID];
    }

    converter = ucnv_open(icuName, &errorCode);

    if (NULL != converter) {
        char lossyByte = CFStringEncodingMaskToLossyByte(flags);

        if ((0 == lossyByte) && (0 != (flags & kCFStringEncodingAllowLossyConversion))) lossyByte = '?';

        if (0 ==lossyByte) {
            if (toUnicode) {
                ucnv_setToUCallBack(converter, &UCNV_TO_U_CALLBACK_STOP, NULL, NULL, NULL, &errorCode);
            } else {
                ucnv_setFromUCallBack(converter, &UCNV_FROM_U_CALLBACK_STOP, NULL, NULL, NULL, &errorCode);
            }
        } else {
            ucnv_setSubstChars(converter, &lossyByte, 1, &errorCode);
        }
    }

    return converter;
}

#define ICU_CONVERTER_SLOT_INCREMENT (10)
#define ICU_CONVERTER_MAX_SLOT (255)

static CFIndex __CFStringEncodingConverterReleaseICUConverter(UConverter *converter, uint32_t flags, CFIndex status) {
    uint8_t streamID = CFStringEncodingStreamIDFromMask(flags);

    if ((kCFStringEncodingInvalidInputStream != status) && ((0 != (flags & kCFStringEncodingPartialInput)) || ((kCFStringEncodingInsufficientOutputBufferLength == status) && (0 != (flags & kCFStringEncodingPartialOutput))))) {
        if (0 == streamID) {
            __CFICUThreadData *data = __CFStringEncodingICUGetThreadData();

            if (NULL == data->_converters) {
                data->_converters = (UConverter **)CFAllocatorAllocate(NULL, sizeof(UConverter *) * ICU_CONVERTER_SLOT_INCREMENT, 0);
                memset(data->_converters, 0, sizeof(UConverter *) * ICU_CONVERTER_SLOT_INCREMENT);
                data->_numSlots = ICU_CONVERTER_SLOT_INCREMENT;
                data->_nextSlot = 0;
            } else if ((data->_nextSlot >= data->_numSlots) || (NULL != data->_converters[data->_nextSlot])) { // Need to find one
                CFIndex index;

                for (index = 0;index < data->_numSlots;index++) {
                    if (NULL == data->_converters[index]) {
                        data->_nextSlot = index;
                        break;
                    }
                }

                if (index >= data->_numSlots) { // we're full
                    UConverter **newConverters;
                    CFIndex newSize = data->_numSlots + ICU_CONVERTER_SLOT_INCREMENT;

                    if (newSize > ICU_CONVERTER_MAX_SLOT) { // something is terribly wrong
                        CFLog(kCFLogLevelError, CFSTR("Per-thread streaming ID for ICU converters exhausted. Ignoring..."));
                        ucnv_close(converter);
                        return 0;
                    }

                    newConverters = (UConverter **)CFAllocatorAllocate(NULL, sizeof(UConverter *) * newSize, 0);
                    memset(newConverters, 0, sizeof(UConverter *) * newSize);
                    memcpy(newConverters, data->_converters, sizeof(UConverter *) * data->_numSlots);
                    CFAllocatorDeallocate(NULL, data->_converters);
                    data->_converters = newConverters;
                    data->_nextSlot = data->_numSlots;
                    data->_numSlots = newSize;
                }
            }

            data->_converters[data->_nextSlot] = converter;
            streamID = data->_nextSlot + 1;

            // now find next slot
            ++data->_nextSlot;

            if ((data->_nextSlot >= data->_numSlots) || (NULL != data->_converters[data->_nextSlot])) {
                data->_nextSlot = 0;

                while ((data->_nextSlot < data->_numSlots) && (NULL != data->_converters[data->_nextSlot])) ++data->_nextSlot;
            }
        }

        return CFStringEncodingStreamIDToMask(streamID);
    }

    if (0 != streamID) {
        __CFICUThreadData *data = __CFStringEncodingICUGetThreadData();

        --streamID; // map to array index

        if ((streamID < data->_numSlots) && (converter == data->_converters[streamID])) {
            data->_converters[streamID] = NULL;
            if (data->_nextSlot > streamID) data->_nextSlot = streamID;
        }
    }

    ucnv_close(converter);

    return 0;
}

#define MAX_BUFFER_SIZE (1000)

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
#if 0
// we're no longer doing this check. Revive when the status in the bug changed.
#if (U_ICU_VERSION_MAJOR_NUM > 49)
#warning Unknown ICU version. Check binary compatibility issues for rdar://problem/6024743
#endif
#endif
#endif
#define HAS_ICU_BUG_6024743 (1)
#define HAS_ICU_BUG_6025527 (1)

CF_PRIVATE CFIndex __CFStringEncodingICUToBytes(const char *icuName, uint32_t flags, const UniChar *characters, CFIndex numChars, CFIndex *usedCharLen, uint8_t *bytes, CFIndex maxByteLen, CFIndex *usedByteLen) {
    UConverter *converter;
    UErrorCode errorCode = U_ZERO_ERROR;
    const UTF16Char *source = characters;
    const UTF16Char *sourceLimit = source + numChars;
    char *destination = (char *)bytes;
    const char *destinationLimit = destination + maxByteLen;
    bool flush = ((0 == (flags & kCFStringEncodingPartialInput)) ? true : false);
    CFIndex status;

    if (NULL == (converter = __CFStringEncodingConverterCreateICUConverter(icuName, flags, false))) return kCFStringEncodingConverterUnavailable;

    if (0 == maxByteLen) {
        char buffer[MAX_BUFFER_SIZE];
        CFIndex totalLength = 0;

        while ((source < sourceLimit) && (U_ZERO_ERROR == errorCode)) {
            destination = buffer;
            destinationLimit = destination + MAX_BUFFER_SIZE;

            ucnv_fromUnicode(converter, &destination, destinationLimit, (const UChar **)&source, (const UChar *)sourceLimit, NULL, flush, &errorCode);

            totalLength += (destination - buffer);

            if (U_BUFFER_OVERFLOW_ERROR == errorCode) errorCode = U_ZERO_ERROR;
        }

        if (NULL != usedByteLen) *usedByteLen = totalLength;
    } else {
        ucnv_fromUnicode(converter, &destination, destinationLimit, (const UChar **)&source, (const UChar *)sourceLimit, NULL, flush, &errorCode);

#if HAS_ICU_BUG_6024743
/* Another critical ICU design issue. Similar to conversion error, source pointer returned from U_BUFFER_OVERFLOW_ERROR is already beyond the last valid character position. It renders the returned value from source entirely unusable. We have to manually back up until succeeding <rdar://problem/7183045> Intrestingly, this issue doesn't apply to ucnv_toUnicode. The asynmmetric nature makes this more dangerous */
        if (U_BUFFER_OVERFLOW_ERROR == errorCode) {
            const uint8_t *bitmap = CFUniCharGetBitmapPtrForPlane(kCFUniCharNonBaseCharacterSet, 0);
            const uint8_t *nonBase;
            UTF32Char character;

            do {
                // Since the output buffer is filled, we can assume no invalid chars (including stray surrogates)
                do {
                    sourceLimit = (source - 1);
                    character = *sourceLimit;
                    nonBase = bitmap;

                    if (CFUniCharIsSurrogateLowCharacter(character)) {
                        --sourceLimit;
                        character = CFUniCharGetLongCharacterForSurrogatePair(*sourceLimit, character);
                        nonBase = CFUniCharGetBitmapPtrForPlane(kCFUniCharNonBaseCharacterSet, (character >> 16) & 0x000F);
                        character &= 0xFFFF;
                    }
                } while ((sourceLimit > characters) && CFUniCharIsMemberOfBitmap(character, nonBase));

                if (sourceLimit > characters) {
                    source = characters;
                    destination = (char *)bytes;
                    errorCode = U_ZERO_ERROR;

                    ucnv_resetFromUnicode(converter);

                    ucnv_fromUnicode(converter, &destination, destinationLimit, (const UChar **)&source, (const UChar *)sourceLimit, NULL, flush, &errorCode);
                }
            } while (U_BUFFER_OVERFLOW_ERROR == errorCode);

            errorCode = U_BUFFER_OVERFLOW_ERROR;
        }
#endif
        if (NULL != usedByteLen) *usedByteLen = destination - (const char *)bytes;
    }

    status = ((U_ZERO_ERROR == errorCode) ? kCFStringEncodingConversionSuccess : ((U_BUFFER_OVERFLOW_ERROR == errorCode) ? kCFStringEncodingInsufficientOutputBufferLength : kCFStringEncodingInvalidInputStream));

    if (NULL != usedCharLen) {
#if HAS_ICU_BUG_6024743
/* ICU has a serious behavioral inconsistency issue that the source pointer returned from ucnv_fromUnicode() is after illegal input. We have to keep track of any changes in this area in order to prevent future binary compatiibility issues */
	if (kCFStringEncodingInvalidInputStream == status) {
#define MAX_ERROR_BUFFER_LEN (32)
	    UTF16Char errorBuffer[MAX_ERROR_BUFFER_LEN];
	    int8_t errorLength = MAX_ERROR_BUFFER_LEN;
#undef MAX_ERROR_BUFFER_LEN

	    errorCode = U_ZERO_ERROR;

	    ucnv_getInvalidUChars(converter, (UChar *)errorBuffer, &errorLength, &errorCode);

	    if (U_ZERO_ERROR == errorCode) {
		source -= errorLength;
	    } else {
		// Gah, something is terribly wrong. Reset everything
		source = characters; // 0 length
		if (NULL != usedByteLen) *usedByteLen = 0;
	    }
	}
#endif
	*usedCharLen = source - characters;
    }

    status |= __CFStringEncodingConverterReleaseICUConverter(converter, flags, status);

    return status;
}

CF_PRIVATE CFIndex __CFStringEncodingICUToUnicode(const char *icuName, uint32_t flags, const uint8_t *bytes, CFIndex numBytes, CFIndex *usedByteLen, UniChar *characters, CFIndex maxCharLen, CFIndex *usedCharLen) {
    UConverter *converter;
    UErrorCode errorCode = U_ZERO_ERROR;
    const char *source = (const char *)bytes;
    const char *sourceLimit = source + numBytes;
    UTF16Char *destination = characters;
    const UTF16Char *destinationLimit = destination + maxCharLen;
    bool flush = ((0 == (flags & kCFStringEncodingPartialInput)) ? true : false);
    CFIndex status;

    if (NULL == (converter = __CFStringEncodingConverterCreateICUConverter(icuName, flags, true))) return kCFStringEncodingConverterUnavailable;

    if (0 == maxCharLen) {
        UTF16Char buffer[MAX_BUFFER_SIZE];
        CFIndex totalLength = 0;
        
        while ((source < sourceLimit) && (U_ZERO_ERROR == errorCode)) {
            destination = buffer;
            destinationLimit = destination + MAX_BUFFER_SIZE;
            
            ucnv_toUnicode(converter, (UChar **)&destination, (const UChar *)destinationLimit, &source, sourceLimit, NULL, flush, &errorCode);
            
            totalLength += (destination - buffer);
            
            if (U_BUFFER_OVERFLOW_ERROR == errorCode) errorCode = U_ZERO_ERROR;
        }
        
        if (NULL != usedCharLen) *usedCharLen = totalLength;
    } else {
        ucnv_toUnicode(converter, (UChar **)&destination, (const UChar *)destinationLimit, &source, sourceLimit, NULL, flush, &errorCode);

        if (NULL != usedCharLen) *usedCharLen = destination - characters;
    }

    status = ((U_ZERO_ERROR == errorCode) ? kCFStringEncodingConversionSuccess : ((U_BUFFER_OVERFLOW_ERROR == errorCode) ? kCFStringEncodingInsufficientOutputBufferLength : kCFStringEncodingInvalidInputStream));

    if (NULL != usedByteLen) {
#if HAS_ICU_BUG_6024743
	/* ICU has a serious behavioral inconsistency issue that the source pointer returned from ucnv_toUnicode() is after illegal input. We have to keep track of any changes in this area in order to prevent future binary compatiibility issues */
	if (kCFStringEncodingInvalidInputStream == status) {
#define MAX_ERROR_BUFFER_LEN (32)
	    char errorBuffer[MAX_ERROR_BUFFER_LEN];
	    int8_t errorLength = MAX_ERROR_BUFFER_LEN;
#undef MAX_ERROR_BUFFER_LEN

	    errorCode = U_ZERO_ERROR;
	    
	    ucnv_getInvalidChars(converter, errorBuffer, &errorLength, &errorCode);
	    
	    if (U_ZERO_ERROR == errorCode) {
#if HAS_ICU_BUG_6025527
                // Another ICU oddness here. ucnv_getInvalidUChars() writes the '\0' terminator, and errorLength includes the extra byte.
                if ((errorLength > 0) && ('\0' == errorBuffer[errorLength - 1])) --errorLength;
#endif
		source -= errorLength;
	    } else {
		// Gah, something is terribly wrong. Reset everything
		source = (const char *)bytes; // 0 length
		if (NULL != usedCharLen) *usedCharLen = 0;
	    }
	}
#endif

	*usedByteLen = source - (const char *)bytes;
    }
    
    status |= __CFStringEncodingConverterReleaseICUConverter(converter, flags, status);

    return status;
}

CF_PRIVATE CFIndex __CFStringEncodingICUCharLength(const char *icuName, uint32_t flags, const uint8_t *bytes, CFIndex numBytes) {
    CFIndex usedCharLen;
    return (__CFStringEncodingICUToUnicode(icuName, flags, bytes, numBytes, NULL, NULL, 0, &usedCharLen) == kCFStringEncodingConversionSuccess ? usedCharLen : 0);
}

CF_PRIVATE CFIndex __CFStringEncodingICUByteLength(const char *icuName, uint32_t flags, const UniChar *characters, CFIndex numChars) {
    CFIndex usedByteLen;
    return (__CFStringEncodingICUToBytes(icuName, flags, characters, numChars, NULL, NULL, 0, &usedByteLen) == kCFStringEncodingConversionSuccess ? usedByteLen : 0);
}

CF_PRIVATE CFStringEncoding *__CFStringEncodingCreateICUEncodings(CFAllocatorRef allocator, CFIndex *numberOfIndex) {
    CFIndex count = ucnv_countAvailable();
    CFIndex numEncodings = 0;
    CFStringEncoding *encodings;
    CFStringEncoding encoding;
    CFIndex index;

    if (0 == count) return NULL;

    encodings = (CFStringEncoding *)CFAllocatorAllocate(NULL, sizeof(CFStringEncoding) * count, 0);

    for (index = 0;index < count;index++) {
        encoding = __CFStringEncodingGetFromICUName(ucnv_getAvailableName(index));

        if (kCFStringEncodingInvalidId != encoding) encodings[numEncodings++] = encoding;
    }

    if (0 == numEncodings) {
        CFAllocatorDeallocate(allocator, encodings);
        encodings = NULL;
    }

    *numberOfIndex = numEncodings;

    return encodings;
}
