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

/*	CFUniChar.c
	Copyright (c) 2001-2014, Apple Inc. All rights reserved.
	Responsibility: Aki Inoue
*/

#include <CoreFoundation/CFByteOrder.h>
#include "CFInternal.h"
#include "CFUniChar.h" 
#include "CFStringEncodingConverterExt.h"
#include "CFUnicodeDecomposition.h"
#include "CFUniCharPriv.h"
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX || DEPLOYMENT_TARGET_FREEBSD
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#endif
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
#include <mach/mach.h>
#endif

#if DEPLOYMENT_TARGET_WINDOWS
extern void _CFGetFrameworkPath(wchar_t *path, int maxLength);
#endif

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
#define __kCFCharacterSetDir "/System/Library/CoreServices"
#elif DEPLOYMENT_TARGET_LINUX || DEPLOYMENT_TARGET_FREEBSD || DEPLOYMENT_TARGET_EMBEDDED_MINI
#define __kCFCharacterSetDir "/usr/local/share/CoreFoundation"
#elif DEPLOYMENT_TARGET_WINDOWS
#define __kCFCharacterSetDir "\\Windows\\CoreFoundation"
#endif

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
#define USE_MACHO_SEGMENT 1
#endif

enum {
    kCFUniCharLastExternalSet = kCFUniCharNewlineCharacterSet,
    kCFUniCharFirstInternalSet = kCFUniCharCompatibilityDecomposableCharacterSet,
    kCFUniCharLastInternalSet = kCFUniCharGraphemeExtendCharacterSet,
    kCFUniCharFirstBitmapSet = kCFUniCharDecimalDigitCharacterSet
};

CF_INLINE uint32_t __CFUniCharMapExternalSetToInternalIndex(uint32_t cset) { return ((kCFUniCharFirstInternalSet <= cset) ? ((cset - kCFUniCharFirstInternalSet) + kCFUniCharLastExternalSet) : cset) - kCFUniCharFirstBitmapSet; }
CF_INLINE uint32_t __CFUniCharMapCompatibilitySetID(uint32_t cset) { return ((cset == kCFUniCharControlCharacterSet) ? kCFUniCharControlAndFormatterCharacterSet : (((cset > kCFUniCharLastExternalSet) && (cset < kCFUniCharFirstInternalSet)) ? ((cset - kCFUniCharLastExternalSet) + kCFUniCharFirstInternalSet) : cset)); }

#if (DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED) && USE_MACHO_SEGMENT
#include <mach-o/getsect.h>
#include <mach-o/dyld.h>
#include <mach-o/ldsyms.h>

extern const void* unicode_csbitmaps_section_start      __asm("section$start$__UNICODE$__csbitmaps");
extern const void* unicode_csbitmaps_section_end        __asm("section$end$__UNICODE$__csbitmaps");
extern const void* unicode_properties_section_start     __asm("section$start$__UNICODE$__properties");
extern const void* unicode_properties_section_end       __asm("section$end$__UNICODE$__properties");
extern const void* unicode_data_section_start           __asm("section$start$__UNICODE$__data");
extern const void* unicode_data_section_end             __asm("section$end$__UNICODE$__data");

static const void *__CFGetSectDataPtr(const char *segname, const char *sectname, uint64_t *sizep) {
    // special case three common sections to have fast access
    if ( strcmp(segname, "__UNICODE") == 0 ) {
        if ( strcmp(sectname, "__csbitmaps") == 0)  {
            if (sizep) *sizep = &unicode_csbitmaps_section_end - &unicode_csbitmaps_section_start;
            return &unicode_csbitmaps_section_start;
        }
        else if ( strcmp(sectname, "__properties") == 0 ) {
            if (sizep) *sizep = &unicode_properties_section_end - &unicode_properties_section_start;
            return &unicode_properties_section_start;
        }
        else if ( strcmp(sectname, "__data") == 0 ) {
            if (sizep) *sizep = &unicode_data_section_end - &unicode_data_section_start;
            return &unicode_data_section_start;
        }
    }
    
    uint32_t idx, cnt = _dyld_image_count();
    for (idx = 0; idx < cnt; idx++) {
       void *mh = (void *)_dyld_get_image_header(idx);
       if (mh != &_mh_dylib_header) continue;
#if __LP64__
       const struct section_64 *sect = getsectbynamefromheader_64((struct mach_header_64 *)mh, segname, sectname);
#else
       const struct section *sect = getsectbynamefromheader((struct mach_header *)mh, segname, sectname);
#endif
       if (!sect) break;
       if (sizep) *sizep = (uint64_t)sect->size;
       return (char *)sect->addr + _dyld_get_image_vmaddr_slide(idx);
    }
    if (sizep) *sizep = 0ULL;
    return NULL;
}
#endif

#if !USE_MACHO_SEGMENT

// Memory map the file

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
CF_INLINE void __CFUniCharCharacterSetPath(char *cpath) {
#elif DEPLOYMENT_TARGET_WINDOWS
CF_INLINE void __CFUniCharCharacterSetPath(wchar_t *wpath) {
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
    strlcpy(cpath, __kCFCharacterSetDir, MAXPATHLEN);
#elif DEPLOYMENT_TARGET_LINUX
    strlcpy(cpath, __kCFCharacterSetDir, MAXPATHLEN);
#elif DEPLOYMENT_TARGET_WINDOWS
    wchar_t frameworkPath[MAXPATHLEN];
    _CFGetFrameworkPath(frameworkPath, MAXPATHLEN);
    wcsncpy(wpath, frameworkPath, MAXPATHLEN);
    wcsncat(wpath, L"\\CoreFoundation.resources\\", MAXPATHLEN - wcslen(wpath));
#else
    strlcpy(cpath, __kCFCharacterSetDir, MAXPATHLEN);
    strlcat(cpath, "/CharacterSets/", MAXPATHLEN);
#endif
}

#if DEPLOYMENT_TARGET_WINDOWS
#define MAX_BITMAP_STATE 512
//
//  If a string is placed into this array, then it has been previously 
//  determined that the bitmap-file cannot be found.  Thus, we make
//  the assumption it won't be there in future calls and we avoid
//  hitting the disk un-necessarily.  This assumption isn't 100%
//  correct, as bitmap-files can be added.  We would have to re-start
//  the application in order to pick-up the new bitmap info.  
//
//  We should probably re-visit this.
//
static wchar_t *mappedBitmapState[MAX_BITMAP_STATE];
static int __nNumStateEntries = -1;
CRITICAL_SECTION __bitmapStateLock = {0};

bool __GetBitmapStateForName(const wchar_t *bitmapName) {
    if (NULL == __bitmapStateLock.DebugInfo)
        InitializeCriticalSection(&__bitmapStateLock);
    EnterCriticalSection(&__bitmapStateLock);
    if (__nNumStateEntries >= 0) {
        for (int i = 0; i < __nNumStateEntries; i++) {
            if (wcscmp(mappedBitmapState[i], bitmapName) == 0) {
                LeaveCriticalSection(&__bitmapStateLock);
                return true;
            }
        }
    }
    LeaveCriticalSection(&__bitmapStateLock);
    return false;
}
void __AddBitmapStateForName(const wchar_t *bitmapName) {
    if (NULL == __bitmapStateLock.DebugInfo)
        InitializeCriticalSection(&__bitmapStateLock);
    EnterCriticalSection(&__bitmapStateLock);
    __nNumStateEntries++;
    mappedBitmapState[__nNumStateEntries] = (wchar_t *)malloc((lstrlenW(bitmapName)+1) * sizeof(wchar_t));
    lstrcpyW(mappedBitmapState[__nNumStateEntries], bitmapName);
    LeaveCriticalSection(&__bitmapStateLock);
}
#endif

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
static bool __CFUniCharLoadBytesFromFile(const char *fileName, const void **bytes, int64_t *fileSize) {
#elif DEPLOYMENT_TARGET_WINDOWS
static bool __CFUniCharLoadBytesFromFile(const wchar_t *fileName, const void **bytes, int64_t *fileSize) {
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif
#if DEPLOYMENT_TARGET_WINDOWS
    HANDLE bitmapFileHandle = NULL;
    HANDLE mappingHandle = NULL;
    
    if (__GetBitmapStateForName(fileName)) {
        // The fileName has been tried in the past, so just return false
        // and move on.
        *bytes = NULL;
        return false;
    }
    mappingHandle = OpenFileMappingW(FILE_MAP_READ, TRUE, fileName);
    if (NULL == mappingHandle) {
        if ((bitmapFileHandle = CreateFileW(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE) {
            // We tried to get the bitmap file for mapping, but it's not there.  Add to list of non-existant bitmap-files so
            // we don't have to try this again in the future.
            __AddBitmapStateForName(fileName);
            return false;
        }
        mappingHandle = CreateFileMapping(bitmapFileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
        CloseHandle(bitmapFileHandle);
        if (!mappingHandle) return false;
    }

    *bytes = MapViewOfFileEx(mappingHandle, FILE_MAP_READ, 0, 0, 0, 0);

    if (NULL != fileSize) {
	MEMORY_BASIC_INFORMATION memoryInfo;

	if (0 == VirtualQueryEx(mappingHandle, *bytes, &memoryInfo, sizeof(memoryInfo))) {
	    *fileSize = 0; // This indicates no checking. Is it right ?
	} else {
	    *fileSize = memoryInfo.RegionSize;
	}
    }

    CloseHandle(mappingHandle);

    return (*bytes ? true : false);
#else
    struct stat statBuf;
    int fd = -1;

    if ((fd = open(fileName, O_RDONLY, 0)) < 0) {
	return false;
    }
    if (fstat(fd, &statBuf) < 0 || (*bytes = mmap(0, statBuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == (void *)-1) {
        close(fd);
        return false;
    }
    close(fd);

    if (NULL != fileSize) *fileSize = statBuf.st_size;

    return true;
#endif
}

#endif // USE_MACHO_SEGMENT

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
static bool __CFUniCharLoadFile(const char *bitmapName, const void **bytes, int64_t *fileSize) {
#elif DEPLOYMENT_TARGET_WINDOWS
static bool __CFUniCharLoadFile(const wchar_t *bitmapName, const void **bytes, int64_t *fileSize) {
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif
#if USE_MACHO_SEGMENT
	*bytes = __CFGetSectDataPtr("__UNICODE", bitmapName, NULL);

    if (NULL != fileSize) *fileSize = 0;

    return *bytes ? true : false;
#else
#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
    char cpath[MAXPATHLEN];
    __CFUniCharCharacterSetPath(cpath);
    strlcat(cpath, bitmapName, MAXPATHLEN);
    Boolean needToFree = false;
    const char *possiblyFrameworkRootedCPath = CFPathRelativeToAppleFrameworksRoot(cpath, &needToFree);
    bool result = __CFUniCharLoadBytesFromFile(possiblyFrameworkRootedCPath, bytes, fileSize);
    if (needToFree) free((void *)possiblyFrameworkRootedCPath);
    return result;
#elif DEPLOYMENT_TARGET_WINDOWS
    wchar_t wpath[MAXPATHLEN];
    __CFUniCharCharacterSetPath(wpath);
    wcsncat(wpath, bitmapName, MAXPATHLEN);
    return __CFUniCharLoadBytesFromFile(wpath, bytes, fileSize);
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif
#endif
}

// Bitmap functions
/*
 Currently unused but left in for symmetry/informative purposes
 CF_INLINE bool isControl(UTF32Char theChar, uint16_t charset, const void *data) { // ISO Control
    return (((theChar <= 0x001F) || (theChar >= 0x007F && theChar <= 0x009F)) ? true : false);
}*/

CF_INLINE bool isWhitespace(UTF32Char theChar, uint16_t charset, const void *data) { // Space
    return (((theChar == 0x0020) || (theChar == 0x0009) || (theChar == 0x00A0) || (theChar == 0x1680) || (theChar >= 0x2000 && theChar <= 0x200B) || (theChar == 0x202F) || (theChar == 0x205F) || (theChar == 0x3000)) ? true : false);
}

CF_INLINE bool isNewline(UTF32Char theChar, uint16_t charset, const void *data) { // White space
    return (((theChar >= 0x000A && theChar <= 0x000D) || (theChar == 0x0085) || (theChar == 0x2028) || (theChar == 0x2029)) ? true : false);
}

CF_INLINE bool isWhitespaceAndNewline(UTF32Char theChar, uint16_t charset, const void *data) { // White space
    return ((isWhitespace(theChar, charset, data) || isNewline(theChar, charset, data)) ? true : false);
}

#if USE_MACHO_SEGMENT
CF_INLINE bool __CFSimpleFileSizeVerification(const void *bytes, int64_t fileSize) { return true; }
#elif 1
// <rdar://problem/8961744> __CFSimpleFileSizeVerification is broken
static bool __CFSimpleFileSizeVerification(const void *bytes, int64_t fileSize) { return true; }
#else
static bool __CFSimpleFileSizeVerification(const void *bytes, int64_t fileSize) {
    bool result = true;

    if (fileSize > 0) {
	if ((sizeof(uint32_t) * 2) > fileSize) {
	    result = false;
	} else {
	    uint32_t headerSize = CFSwapInt32BigToHost(*((uint32_t *)((char *)bytes + 4)));

	    if ((headerSize < (sizeof(uint32_t) * 4)) || (headerSize > fileSize)) {
		result = false;
	    } else {
		const uint32_t *lastElement = (uint32_t *)(((uint8_t *)bytes) + headerSize) - 2;

		if ((headerSize + CFSwapInt32BigToHost(lastElement[0]) + CFSwapInt32BigToHost(lastElement[1])) > headerSize) result = false;
	    }
	}
    }

    if (!result) CFLog(kCFLogLevelCritical, CFSTR("File size verification for Unicode database file failed."));

    return result;
}
#endif // USE_MACHO_SEGMENT

typedef struct {
    uint32_t _numPlanes;
    const uint8_t **_planes;
} __CFUniCharBitmapData;

static char __CFUniCharUnicodeVersionString[8] = {0, 0, 0, 0, 0, 0, 0, 0};

static uint32_t __CFUniCharNumberOfBitmaps = 0;
static __CFUniCharBitmapData *__CFUniCharBitmapDataArray = NULL;

static CFLock_t __CFUniCharBitmapLock = CFLockInit;

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
#if !defined(CF_UNICHAR_BITMAP_FILE)
#if USE_MACHO_SEGMENT
#define CF_UNICHAR_BITMAP_FILE "__csbitmaps"
#else
#define CF_UNICHAR_BITMAP_FILE "/CFCharacterSetBitmaps.bitmap"
#endif
#endif
#elif DEPLOYMENT_TARGET_WINDOWS
#if !defined(CF_UNICHAR_BITMAP_FILE)
#define CF_UNICHAR_BITMAP_FILE L"CFCharacterSetBitmaps.bitmap"
#endif
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif

static bool __CFUniCharLoadBitmapData(void) {
    __CFUniCharBitmapData *array;
    uint32_t headerSize;
    uint32_t bitmapSize;
    int numPlanes;
    uint8_t currentPlane;
    const void *bytes;
    const void *bitmapBase;
    const void *bitmap;
    int idx, bitmapIndex;
    int64_t fileSize;

    __CFLock(&__CFUniCharBitmapLock);

    if (__CFUniCharBitmapDataArray || !__CFUniCharLoadFile(CF_UNICHAR_BITMAP_FILE, &bytes, &fileSize) || !__CFSimpleFileSizeVerification(bytes, fileSize)) {
        __CFUnlock(&__CFUniCharBitmapLock);
        return false;
    }

    for (idx = 0;idx < 4 && ((const uint8_t *)bytes)[idx];idx++) {
        __CFUniCharUnicodeVersionString[idx * 2] = ((const uint8_t *)bytes)[idx];
        __CFUniCharUnicodeVersionString[idx * 2 + 1] = '.';
    }
    __CFUniCharUnicodeVersionString[(idx < 4 ? idx * 2 - 1 : 7)] = '\0';

    headerSize = CFSwapInt32BigToHost(*((uint32_t *)((char *)bytes + 4)));

    bitmapBase = (uint8_t *)bytes + headerSize;
    bytes = (uint8_t *)bytes + (sizeof(uint32_t) * 2);
    headerSize -= (sizeof(uint32_t) * 2);

    __CFUniCharNumberOfBitmaps = headerSize / (sizeof(uint32_t) * 2);

    array = (__CFUniCharBitmapData *)CFAllocatorAllocate(kCFAllocatorSystemDefault, sizeof(__CFUniCharBitmapData) * __CFUniCharNumberOfBitmaps, 0);

    for (idx = 0;idx < (int)__CFUniCharNumberOfBitmaps;idx++) {
        bitmap = (uint8_t *)bitmapBase + CFSwapInt32BigToHost(*((uint32_t *)bytes)); bytes = (uint8_t *)bytes + sizeof(uint32_t);
        bitmapSize = CFSwapInt32BigToHost(*((uint32_t *)bytes)); bytes = (uint8_t *)bytes + sizeof(uint32_t);

        numPlanes = bitmapSize / (8 * 1024);
        numPlanes = *(const uint8_t *)((char *)bitmap + (((numPlanes - 1) * ((8 * 1024) + 1)) - 1)) + 1;
        array[idx]._planes = (const uint8_t **)CFAllocatorAllocate(kCFAllocatorSystemDefault, sizeof(const void *) * numPlanes, 0);
        array[idx]._numPlanes = numPlanes;

        currentPlane = 0;
        for (bitmapIndex = 0;bitmapIndex < numPlanes;bitmapIndex++) {
            if (bitmapIndex == currentPlane) {
                array[idx]._planes[bitmapIndex] = (const uint8_t *)bitmap;
                bitmap = (uint8_t *)bitmap + (8 * 1024);
#if defined (__cplusplus)				
				currentPlane = *(((const uint8_t*&)bitmap)++);
#else
				currentPlane = *((const uint8_t *)bitmap++);
#endif

            } else {
                array[idx]._planes[bitmapIndex] = NULL;
            }
        }
    }

    __CFUniCharBitmapDataArray = array;

    __CFUnlock(&__CFUniCharBitmapLock);

    return true;
}

CF_PRIVATE const char *__CFUniCharGetUnicodeVersionString(void) {
    if (NULL == __CFUniCharBitmapDataArray) __CFUniCharLoadBitmapData();
    return __CFUniCharUnicodeVersionString;
}

bool CFUniCharIsMemberOf(UTF32Char theChar, uint32_t charset) {
    charset = __CFUniCharMapCompatibilitySetID(charset);

    switch (charset) {
        case kCFUniCharWhitespaceCharacterSet:
            return isWhitespace(theChar, charset, NULL);

        case kCFUniCharWhitespaceAndNewlineCharacterSet:
            return isWhitespaceAndNewline(theChar, charset, NULL);

        case kCFUniCharNewlineCharacterSet:
            return isNewline(theChar, charset, NULL);

        default: {
            uint32_t tableIndex = __CFUniCharMapExternalSetToInternalIndex(charset);

            if (NULL == __CFUniCharBitmapDataArray) __CFUniCharLoadBitmapData();

            if (tableIndex < __CFUniCharNumberOfBitmaps) {
                __CFUniCharBitmapData *data = __CFUniCharBitmapDataArray + tableIndex;
                uint8_t planeNo = (theChar >> 16) & 0xFF;

                // The bitmap data for kCFUniCharIllegalCharacterSet is actually LEGAL set less Plane 14 ~ 16
                if (charset == kCFUniCharIllegalCharacterSet) {
                    if (planeNo == 0x0E) { // Plane 14
                        theChar &= 0xFF;
                        return (((theChar == 0x01) || ((theChar > 0x1F) && (theChar < 0x80))) ? false : true);
                    } else if (planeNo == 0x0F || planeNo == 0x10) { // Plane 15 & 16
                        return ((theChar & 0xFF) > 0xFFFD ? true : false);
                    } else {
                        return (planeNo < data->_numPlanes && data->_planes[planeNo] ? !CFUniCharIsMemberOfBitmap(theChar, data->_planes[planeNo]) : true);
                    }
                } else if (charset == kCFUniCharControlAndFormatterCharacterSet) {
                    if (planeNo == 0x0E) { // Plane 14
                        theChar &= 0xFF;
                        return (((theChar == 0x01) || ((theChar > 0x1F) && (theChar < 0x80))) ? true : false);
                    } else {
                        return (planeNo < data->_numPlanes && data->_planes[planeNo] ? CFUniCharIsMemberOfBitmap(theChar, data->_planes[planeNo]) : false);
                    }
                } else {
                    return (planeNo < data->_numPlanes && data->_planes[planeNo] ? CFUniCharIsMemberOfBitmap(theChar, data->_planes[planeNo]) : false);
                }
            }
            return false;
        }
    }
}

const uint8_t *CFUniCharGetBitmapPtrForPlane(uint32_t charset, uint32_t plane) {
    if (NULL == __CFUniCharBitmapDataArray) __CFUniCharLoadBitmapData();

    charset = __CFUniCharMapCompatibilitySetID(charset);

    if ((charset > kCFUniCharWhitespaceAndNewlineCharacterSet) && (charset != kCFUniCharIllegalCharacterSet) && (charset != kCFUniCharNewlineCharacterSet)) {
        uint32_t tableIndex = __CFUniCharMapExternalSetToInternalIndex(charset);

        if (tableIndex < __CFUniCharNumberOfBitmaps) {
            __CFUniCharBitmapData *data = __CFUniCharBitmapDataArray + tableIndex;

            return (plane < data->_numPlanes ? data->_planes[plane] : NULL);
        }
    }
    return NULL;
}

CF_PRIVATE uint8_t CFUniCharGetBitmapForPlane(uint32_t charset, uint32_t plane, void *bitmap, bool isInverted) {
    const uint8_t *src = CFUniCharGetBitmapPtrForPlane(charset, plane);
    int numBytes = (8 * 1024);

    if (src) {
        if (isInverted) {
#if defined (__cplusplus)
			while (numBytes-- > 0) *(((uint8_t *&)bitmap)++) = ~(*(src++));
#else
			while (numBytes-- > 0) *((uint8_t *)bitmap++) = ~(*(src++));
#endif
        } else {
#if defined (__cplusplus)
            while (numBytes-- > 0) *(((uint8_t *&)bitmap)++) = *(src++);
#else
			while (numBytes-- > 0) *((uint8_t *)bitmap++) = *(src++);
#endif
        }
        return kCFUniCharBitmapFilled;
    } else if (charset == kCFUniCharIllegalCharacterSet) {
        __CFUniCharBitmapData *data = __CFUniCharBitmapDataArray + __CFUniCharMapExternalSetToInternalIndex(__CFUniCharMapCompatibilitySetID(charset));

        if (plane < data->_numPlanes && (src = data->_planes[plane])) {
            if (isInverted) {
#if defined (__cplusplus)                
				while (numBytes-- > 0) *(((uint8_t *&)bitmap)++) = *(src++);
#else
				while (numBytes-- > 0) *((uint8_t *)bitmap++) = *(src++);
#endif
            } else {
#if defined (__cplusplus)                
                while (numBytes-- > 0) *(((uint8_t *&)bitmap)++) = ~(*(src++));
#else
				while (numBytes-- > 0) *((uint8_t *)bitmap++) = ~(*(src++));
#endif
            }
            return kCFUniCharBitmapFilled;
        } else if (plane == 0x0E) { // Plane 14
            int idx;
            uint8_t asciiRange = (isInverted ? (uint8_t)0xFF : (uint8_t)0);
            uint8_t otherRange = (isInverted ? (uint8_t)0 : (uint8_t)0xFF);

#if defined (__cplusplus)
			*(((uint8_t *&)bitmap)++) = 0x02; // UE0001 LANGUAGE TAG
#else
			*((uint8_t *)bitmap++) = 0x02; // UE0001 LANGUAGE TAG
#endif
            for (idx = 1;idx < numBytes;idx++) {
#if defined (__cplusplus)                
				*(((uint8_t *&)bitmap)++) = ((idx >= (0x20 / 8) && (idx < (0x80 / 8))) ? asciiRange : otherRange);
#else
				*((uint8_t *)bitmap++) = ((idx >= (0x20 / 8) && (idx < (0x80 / 8))) ? asciiRange : otherRange);
#endif
            }
            return kCFUniCharBitmapFilled;
        } else if (plane == 0x0F || plane == 0x10) { // Plane 15 & 16
            uint32_t value = (isInverted ? ~0 : 0);
            numBytes /= 4; // for 32bit

            while (numBytes-- > 0) {
                *((uint32_t *)bitmap) = value;
#if defined (__cplusplus)                
				bitmap = (uint8_t *)bitmap + sizeof(uint32_t);				
#else
				bitmap += sizeof(uint32_t);
#endif
            }
            *(((uint8_t *)bitmap) - 5) = (isInverted ? 0x3F : 0xC0); // 0xFFFE & 0xFFFF
            return kCFUniCharBitmapFilled;
        }
        return (isInverted ? kCFUniCharBitmapEmpty : kCFUniCharBitmapAll);
    } else if ((charset < kCFUniCharDecimalDigitCharacterSet) || (charset == kCFUniCharNewlineCharacterSet)) {
        if (plane) return (isInverted ? kCFUniCharBitmapAll : kCFUniCharBitmapEmpty);

        uint8_t *bitmapBase = (uint8_t *)bitmap;
        CFIndex idx;
        uint8_t nonFillValue = (isInverted ? (uint8_t)0xFF : (uint8_t)0);

#if defined (__cplusplus)
                    while (numBytes-- > 0) *(((uint8_t *&)bitmap)++) = nonFillValue;
#else
                    while (numBytes-- > 0) *((uint8_t *)bitmap++) = nonFillValue;
#endif

        if ((charset == kCFUniCharWhitespaceAndNewlineCharacterSet) || (charset == kCFUniCharNewlineCharacterSet)) {
            const UniChar newlines[] = {0x000A, 0x000B, 0x000C, 0x000D, 0x0085, 0x2028, 0x2029};

            for (idx = 0;idx < (int)(sizeof(newlines) / sizeof(*newlines)); idx++) {
                if (isInverted) {
                    CFUniCharRemoveCharacterFromBitmap(newlines[idx], bitmapBase);
                } else {
                    CFUniCharAddCharacterToBitmap(newlines[idx], bitmapBase);
                }
            }

            if (charset == kCFUniCharNewlineCharacterSet) return kCFUniCharBitmapFilled;
        }

        if (isInverted) {
            CFUniCharRemoveCharacterFromBitmap(0x0009, bitmapBase);
            CFUniCharRemoveCharacterFromBitmap(0x0020, bitmapBase);
            CFUniCharRemoveCharacterFromBitmap(0x00A0, bitmapBase);
            CFUniCharRemoveCharacterFromBitmap(0x1680, bitmapBase);
            CFUniCharRemoveCharacterFromBitmap(0x202F, bitmapBase);
            CFUniCharRemoveCharacterFromBitmap(0x205F, bitmapBase);
            CFUniCharRemoveCharacterFromBitmap(0x3000, bitmapBase);
        } else {
            CFUniCharAddCharacterToBitmap(0x0009, bitmapBase);
            CFUniCharAddCharacterToBitmap(0x0020, bitmapBase);
            CFUniCharAddCharacterToBitmap(0x00A0, bitmapBase);
            CFUniCharAddCharacterToBitmap(0x1680, bitmapBase);
            CFUniCharAddCharacterToBitmap(0x202F, bitmapBase);
            CFUniCharAddCharacterToBitmap(0x205F, bitmapBase);
            CFUniCharAddCharacterToBitmap(0x3000, bitmapBase);
        }

        for (idx = 0x2000;idx <= 0x200B;idx++) {
            if (isInverted) {
                CFUniCharRemoveCharacterFromBitmap(idx, bitmapBase);
            } else {
                CFUniCharAddCharacterToBitmap(idx, bitmapBase);
            }
        }
        return kCFUniCharBitmapFilled;
    }
    return (isInverted ? kCFUniCharBitmapAll : kCFUniCharBitmapEmpty);
}

CF_PRIVATE uint32_t CFUniCharGetNumberOfPlanes(uint32_t charset) {
    if ((charset == kCFUniCharControlCharacterSet) || (charset == kCFUniCharControlAndFormatterCharacterSet)) {
        return 15; // 0 to 14
    } else if (charset < kCFUniCharDecimalDigitCharacterSet) {
        return 1;
    } else if (charset == kCFUniCharIllegalCharacterSet) {
        return 17;
    } else {
        uint32_t numPlanes;

        if (NULL == __CFUniCharBitmapDataArray) __CFUniCharLoadBitmapData();

        numPlanes = __CFUniCharBitmapDataArray[__CFUniCharMapExternalSetToInternalIndex(__CFUniCharMapCompatibilitySetID(charset))]._numPlanes;

        return numPlanes;
    }
}

// Mapping data loading
static const void **__CFUniCharMappingTables = NULL;

static CFLock_t __CFUniCharMappingTableLock = CFLockInit;

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
#if __CF_BIG_ENDIAN__
#if USE_MACHO_SEGMENT
#define MAPPING_TABLE_FILE "__data"
#else
#define MAPPING_TABLE_FILE "/CFUnicodeData-B.mapping"
#endif
#else
#if USE_MACHO_SEGMENT
#define MAPPING_TABLE_FILE "__data"
#else
#define MAPPING_TABLE_FILE "/CFUnicodeData-L.mapping"
#endif
#endif
#elif DEPLOYMENT_TARGET_WINDOWS
#if __CF_BIG_ENDIAN__
#if USE_MACHO_SEGMENT
#define MAPPING_TABLE_FILE "__data"
#else
#define MAPPING_TABLE_FILE L"CFUnicodeData-B.mapping"
#endif
#else
#if USE_MACHO_SEGMENT
#define MAPPING_TABLE_FILE "__data"
#else
#define MAPPING_TABLE_FILE L"CFUnicodeData-L.mapping"
#endif
#endif
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif

CF_PRIVATE const void *CFUniCharGetMappingData(uint32_t type) {

    __CFLock(&__CFUniCharMappingTableLock);

    if (NULL == __CFUniCharMappingTables) {
        const void *bytes;
        const void *bodyBase;
        int headerSize;
        int idx, count;
	int64_t fileSize;

        if (!__CFUniCharLoadFile(MAPPING_TABLE_FILE, &bytes, &fileSize) || !__CFSimpleFileSizeVerification(bytes, fileSize)) {
            __CFUnlock(&__CFUniCharMappingTableLock);
            return NULL;
        }

#if defined (__cplusplus)
		bytes = (uint8_t *)bytes + 4; // Skip Unicode version
		headerSize = *((uint8_t *)bytes); bytes = (uint8_t *)bytes + sizeof(uint32_t);
#else
		bytes += 4; // Skip Unicode version
		headerSize = *((uint32_t *)bytes); bytes += sizeof(uint32_t);
#endif    
        headerSize -= (sizeof(uint32_t) * 2);
        bodyBase = (char *)bytes + headerSize;

        count = headerSize / sizeof(uint32_t);

        __CFUniCharMappingTables = (const void **)CFAllocatorAllocate(kCFAllocatorSystemDefault, sizeof(const void *) * count, 0);

        for (idx = 0;idx < count;idx++) {
#if defined (__cplusplus)            
			__CFUniCharMappingTables[idx] = (char *)bodyBase + *((uint32_t *)bytes); bytes = (uint8_t *)bytes + sizeof(uint32_t);
#else
			__CFUniCharMappingTables[idx] = (char *)bodyBase + *((uint32_t *)bytes); bytes += sizeof(uint32_t);
#endif
        }
    }

    __CFUnlock(&__CFUniCharMappingTableLock);

    return __CFUniCharMappingTables[type];
}

// Case mapping functions
#define DO_SPECIAL_CASE_MAPPING 1

static uint32_t *__CFUniCharCaseMappingTableCounts = NULL;
static uint32_t **__CFUniCharCaseMappingTable = NULL;
static const uint32_t **__CFUniCharCaseMappingExtraTable = NULL;

typedef struct {
    uint32_t _key;
    uint32_t _value;
} __CFUniCharCaseMappings;

/* Binary searches CFStringEncodingUnicodeTo8BitCharMap */
static uint32_t __CFUniCharGetMappedCase(const __CFUniCharCaseMappings *theTable, uint32_t numElem, UTF32Char character) {
    const __CFUniCharCaseMappings *p, *q, *divider;

    if ((character < theTable[0]._key) || (character > theTable[numElem-1]._key)) {
        return 0;
    }
    p = theTable;
    q = p + (numElem-1);
    while (p <= q) {
        divider = p + ((q - p) >> 1);	/* divide by 2 */
        if (character < divider->_key) { q = divider - 1; }
        else if (character > divider->_key) { p = divider + 1; }
        else { return divider->_value; }
    }
    return 0;
}

#define NUM_CASE_MAP_DATA (kCFUniCharCaseFold + 1)

static bool __CFUniCharLoadCaseMappingTable(void) {
    uint32_t *countArray;
    int idx;

    if (NULL == __CFUniCharMappingTables) (void)CFUniCharGetMappingData(kCFUniCharToLowercase);
    if (NULL == __CFUniCharMappingTables) return false;

    __CFLock(&__CFUniCharMappingTableLock);

    if (__CFUniCharCaseMappingTableCounts) {
        __CFUnlock(&__CFUniCharMappingTableLock);
        return true;
    }

    countArray = (uint32_t *)CFAllocatorAllocate(kCFAllocatorSystemDefault, sizeof(uint32_t) * NUM_CASE_MAP_DATA + sizeof(uint32_t *) * NUM_CASE_MAP_DATA * 2, 0);
    __CFUniCharCaseMappingTable = (uint32_t **)((char *)countArray + sizeof(uint32_t) * NUM_CASE_MAP_DATA);
    __CFUniCharCaseMappingExtraTable = (const uint32_t **)__CFUniCharCaseMappingTable + NUM_CASE_MAP_DATA;

    for (idx = 0;idx < NUM_CASE_MAP_DATA;idx++) {
        countArray[idx] = *((uint32_t *)__CFUniCharMappingTables[idx]) / (sizeof(uint32_t) * 2);
        __CFUniCharCaseMappingTable[idx] = ((uint32_t *)__CFUniCharMappingTables[idx]) + 1;
        __CFUniCharCaseMappingExtraTable[idx] = (const uint32_t *)((char *)__CFUniCharCaseMappingTable[idx] + *((uint32_t *)__CFUniCharMappingTables[idx]));
    }

    __CFUniCharCaseMappingTableCounts = countArray;

    __CFUnlock(&__CFUniCharMappingTableLock);
    return true;
}

#if __CF_BIG_ENDIAN__
#define TURKISH_LANG_CODE	(0x7472) // tr
#define LITHUANIAN_LANG_CODE	(0x6C74) // lt
#define AZERI_LANG_CODE		(0x617A) // az
#define DUTCH_LANG_CODE		(0x6E6C) // nl
#define GREEK_LANG_CODE		(0x656C) // el
#else
#define TURKISH_LANG_CODE	(0x7274) // tr
#define LITHUANIAN_LANG_CODE	(0x746C) // lt
#define AZERI_LANG_CODE		(0x7A61) // az
#define DUTCH_LANG_CODE		(0x6C6E) // nl
#define GREEK_LANG_CODE		(0x6C65) // el
#endif

CFIndex CFUniCharMapCaseTo(UTF32Char theChar, UTF16Char *convertedChar, CFIndex maxLength, uint32_t ctype, uint32_t flags, const uint8_t *langCode) {
    __CFUniCharBitmapData *data;
    uint8_t planeNo = (theChar >> 16) & 0xFF;

caseFoldRetry:

#if DO_SPECIAL_CASE_MAPPING
    if (flags & kCFUniCharCaseMapFinalSigma) {
        if (theChar == 0x03A3) { // Final sigma
            *convertedChar = (ctype == kCFUniCharToLowercase ? 0x03C2 : 0x03A3);
            return 1;
        }
    }

    if (langCode) {
        if (flags & kCFUniCharCaseMapGreekTonos) { // localized Greek uppercasing
            if (theChar == 0x0301) { // GREEK TONOS
                return 0;
            } else if (theChar == 0x0344) {// COMBINING GREEK DIALYTIKA TONOS
                *convertedChar = 0x0308; // COMBINING GREEK DIALYTIKA
                return 1;
            } else if (CFUniCharIsMemberOf(theChar, kCFUniCharDecomposableCharacterSet)) {
                UTF32Char buffer[MAX_DECOMPOSED_LENGTH];
                CFIndex length = CFUniCharDecomposeCharacter(theChar, buffer, MAX_DECOMPOSED_LENGTH);

                if (length > 1) {
                    UTF32Char *characters = buffer + 1;
                    UTF32Char *tail = buffer + length;

                    while (characters < tail) {
                        if (*characters == 0x0301) break;
                        ++characters;
                    }

                    if (characters < tail) { // found a tonos
                        CFIndex convertedLength = CFUniCharMapCaseTo(*buffer, convertedChar, maxLength, ctype, 0, langCode);

                        if (convertedLength == 0) {
                            *convertedChar = (UTF16Char)*buffer;
                            convertedLength = 1;
                        }

                        characters = buffer + 1;

                        while (characters < tail) {
                            if (*characters != 0x0301) { // not tonos
                                if (*characters < 0x10000) { // BMP
                                    convertedChar[convertedLength] = (UTF16Char)*characters;
                                    ++convertedLength;
                                } else {
                                    UTF32Char character = *characters - 0x10000;
                                    convertedChar[convertedLength++] = (UTF16Char)((character >> 10) + 0xD800UL);
                                    convertedChar[convertedLength++] = (UTF16Char)((character & 0x3FF) + 0xDC00UL);
                                }
                            }
                            ++characters;
                        }

                        return convertedLength;
                    }
                }
            }
        }
        switch (*(uint16_t *)langCode) {
            case LITHUANIAN_LANG_CODE:
                if (theChar == 0x0307 && (flags & kCFUniCharCaseMapAfter_i)) {
                    return 0;
                } else if (ctype == kCFUniCharToLowercase) {
                    if (flags & kCFUniCharCaseMapMoreAbove) {
                        switch (theChar) {
                            case 0x0049: // LATIN CAPITAL LETTER I
                                *(convertedChar++) = 0x0069;
                                *(convertedChar++) = 0x0307;
                                return 2;

                            case 0x004A: // LATIN CAPITAL LETTER J
                                *(convertedChar++) = 0x006A;
                                *(convertedChar++) = 0x0307;
                                return 2;

                            case 0x012E: // LATIN CAPITAL LETTER I WITH OGONEK
                                *(convertedChar++) = 0x012F;
                                *(convertedChar++) = 0x0307;
                                return 2;

                            default: break;
                        }
                    }
                    switch (theChar) {
                        case 0x00CC: // LATIN CAPITAL LETTER I WITH GRAVE
                            *(convertedChar++) = 0x0069;
                            *(convertedChar++) = 0x0307;
                            *(convertedChar++) = 0x0300;
                            return 3;

                        case 0x00CD: // LATIN CAPITAL LETTER I WITH ACUTE
                            *(convertedChar++) = 0x0069;
                            *(convertedChar++) = 0x0307;
                            *(convertedChar++) = 0x0301;
                            return 3;

                        case 0x0128: // LATIN CAPITAL LETTER I WITH TILDE
                            *(convertedChar++) = 0x0069;
                            *(convertedChar++) = 0x0307;
                            *(convertedChar++) = 0x0303;
                            return 3;

                        default: break;
                    }
                }
            break;

            case TURKISH_LANG_CODE:
            case AZERI_LANG_CODE:
                if ((theChar == 0x0049) || (theChar == 0x0131)) { // LATIN CAPITAL LETTER I & LATIN SMALL LETTER DOTLESS I
                    *convertedChar = (((ctype == kCFUniCharToLowercase) || (ctype == kCFUniCharCaseFold))  ? ((kCFUniCharCaseMapMoreAbove & flags) ? 0x0069 : 0x0131) : 0x0049);
                    return 1;
                } else if ((theChar == 0x0069) || (theChar == 0x0130)) { // LATIN SMALL LETTER I & LATIN CAPITAL LETTER I WITH DOT ABOVE
                    *convertedChar = (((ctype == kCFUniCharToLowercase) || (ctype == kCFUniCharCaseFold)) ? 0x0069 : 0x0130);
                    return 1;
                } else if (theChar == 0x0307 && (kCFUniCharCaseMapAfter_i & flags)) { // COMBINING DOT ABOVE AFTER_i
                    if (ctype == kCFUniCharToLowercase) {
                        return 0;
                    } else {
                        *convertedChar = 0x0307;
                        return 1;
                    }
                }
                break;

	    case DUTCH_LANG_CODE:
		if ((theChar == 0x004A) || (theChar == 0x006A)) {
                    *convertedChar = (((ctype == kCFUniCharToUppercase) || (ctype == kCFUniCharToTitlecase) || (kCFUniCharCaseMapDutchDigraph & flags)) ? 0x004A  : 0x006A);
                    return 1;
		}
		break;

            default: break;
        }
    }
#endif // DO_SPECIAL_CASE_MAPPING

    if (NULL == __CFUniCharBitmapDataArray) __CFUniCharLoadBitmapData();

    data = __CFUniCharBitmapDataArray + __CFUniCharMapExternalSetToInternalIndex(__CFUniCharMapCompatibilitySetID(ctype + kCFUniCharHasNonSelfLowercaseCharacterSet));

    if (planeNo < data->_numPlanes && data->_planes[planeNo] && CFUniCharIsMemberOfBitmap(theChar, data->_planes[planeNo]) && (__CFUniCharCaseMappingTableCounts || __CFUniCharLoadCaseMappingTable())) {
        uint32_t value = __CFUniCharGetMappedCase((const __CFUniCharCaseMappings *)__CFUniCharCaseMappingTable[ctype], __CFUniCharCaseMappingTableCounts[ctype], theChar);

        if (!value && ctype == kCFUniCharToTitlecase) {
            value = __CFUniCharGetMappedCase((const __CFUniCharCaseMappings *)__CFUniCharCaseMappingTable[kCFUniCharToUppercase], __CFUniCharCaseMappingTableCounts[kCFUniCharToUppercase], theChar);
            if (value) ctype = kCFUniCharToUppercase;
        }

        if (value) {
            CFIndex count = CFUniCharConvertFlagToCount(value);

            if (count == 1) {
                if (value & kCFUniCharNonBmpFlag) {
                    if (maxLength > 1) {
                        value = (value & 0xFFFFFF) - 0x10000;
                        *(convertedChar++) = (UTF16Char)(value >> 10) + 0xD800UL;
                        *(convertedChar++) = (UTF16Char)(value & 0x3FF) + 0xDC00UL;
                        return 2;
                    }
                } else {
                    *convertedChar = (UTF16Char)value;
                    return 1;
                }
            } else if (count < maxLength) {
                const uint32_t *extraMapping = __CFUniCharCaseMappingExtraTable[ctype] + (value & 0xFFFFFF);

                if (value & kCFUniCharNonBmpFlag) {
                    CFIndex copiedLen = 0;

                    while (count-- > 0) {
                        value = *(extraMapping++);
                        if (value > 0xFFFF) {
                            if (copiedLen + 2 >= maxLength) break;
                            value = (value & 0xFFFFFF) - 0x10000;
                            convertedChar[copiedLen++] = (UTF16Char)(value >> 10) + 0xD800UL;
                            convertedChar[copiedLen++] = (UTF16Char)(value & 0x3FF) + 0xDC00UL;
                        } else {
                            if (copiedLen + 1 >= maxLength) break;
                            convertedChar[copiedLen++] = value;
                        }
                    }
                    if (!count) return copiedLen;
                } else {
                    CFIndex idx;

                    for (idx = 0;idx < count;idx++) *(convertedChar++) = (UTF16Char)*(extraMapping++);
                    return count;
                }
            }
        }
    } else if (ctype == kCFUniCharCaseFold) {
        ctype = kCFUniCharToLowercase;
        goto caseFoldRetry;
    }

    if (theChar > 0xFFFF) { // non-BMP
        theChar = (theChar & 0xFFFFFF) - 0x10000;
        *(convertedChar++) = (UTF16Char)(theChar >> 10) + 0xD800UL;
        *(convertedChar++) = (UTF16Char)(theChar & 0x3FF) + 0xDC00UL;
        return 2;
    } else {
        *convertedChar = theChar;
        return 1;
    }
}

CFIndex CFUniCharMapTo(UniChar theChar, UniChar *convertedChar, CFIndex maxLength, uint16_t ctype, uint32_t flags) {
    if (ctype == kCFUniCharCaseFold + 1) { // kCFUniCharDecompose
        if (CFUniCharIsDecomposableCharacter(theChar, false)) {
            UTF32Char buffer[MAX_DECOMPOSED_LENGTH];
            CFIndex usedLength = CFUniCharDecomposeCharacter(theChar, buffer, MAX_DECOMPOSED_LENGTH);
            CFIndex idx;

            for (idx = 0;idx < usedLength;idx++) *(convertedChar++) = buffer[idx];
            return usedLength;
        } else {
            *convertedChar = theChar;
            return 1;
        }
    } else {
        return CFUniCharMapCaseTo(theChar, convertedChar, maxLength, ctype, flags, NULL);
    }
}

CF_INLINE bool __CFUniCharIsMoreAbove(UTF16Char *buffer, CFIndex length) {
    UTF32Char currentChar;
    uint32_t property;

    while (length-- > 0) {
        currentChar = *(buffer)++;
        if (CFUniCharIsSurrogateHighCharacter(currentChar) && (length > 0) && CFUniCharIsSurrogateLowCharacter(*(buffer + 1))) {
            currentChar = CFUniCharGetLongCharacterForSurrogatePair(currentChar, *(buffer++));
            --length;
        }
        if (!CFUniCharIsMemberOf(currentChar, kCFUniCharNonBaseCharacterSet)) break;

        property = CFUniCharGetCombiningPropertyForCharacter(currentChar, (const uint8_t *)CFUniCharGetUnicodePropertyDataForPlane(kCFUniCharCombiningProperty, (currentChar >> 16) & 0xFF));

        if (property == 230) return true; // Above priority
    }
    return false;
}

CF_INLINE bool __CFUniCharIsAfter_i(UTF16Char *buffer, CFIndex length) {
    UTF32Char currentChar = 0;
    uint32_t property;
    UTF32Char decomposed[MAX_DECOMPOSED_LENGTH];
    CFIndex decompLength;
    CFIndex idx;

    if (length < 1) return 0;

    buffer += length;
    while (length-- > 1) {
        currentChar = *(--buffer);
        if (CFUniCharIsSurrogateLowCharacter(currentChar)) {
            if ((length > 1) && CFUniCharIsSurrogateHighCharacter(*(buffer - 1))) {
                currentChar = CFUniCharGetLongCharacterForSurrogatePair(*(--buffer), currentChar);
                --length;
            } else {
                break;
            }
        }
        if (!CFUniCharIsMemberOf(currentChar, kCFUniCharNonBaseCharacterSet)) break;

        property = CFUniCharGetCombiningPropertyForCharacter(currentChar, (const uint8_t *)CFUniCharGetUnicodePropertyDataForPlane(kCFUniCharCombiningProperty, (currentChar >> 16) & 0xFF));

        if (property == 230) return false; // Above priority
    }
    if (length == 0) {
        currentChar = *(--buffer);
    } else if (CFUniCharIsSurrogateLowCharacter(currentChar) && CFUniCharIsSurrogateHighCharacter(*(--buffer))) {
        currentChar = CFUniCharGetLongCharacterForSurrogatePair(*buffer, currentChar);
    }

    decompLength = CFUniCharDecomposeCharacter(currentChar, decomposed, MAX_DECOMPOSED_LENGTH);
    currentChar = *decomposed;


    for (idx = 1;idx < decompLength;idx++) {
        currentChar = decomposed[idx];
        property = CFUniCharGetCombiningPropertyForCharacter(currentChar, (const uint8_t *)CFUniCharGetUnicodePropertyDataForPlane(kCFUniCharCombiningProperty, (currentChar >> 16) & 0xFF));

        if (property == 230) return false; // Above priority
    }
    return true;
}

CF_PRIVATE uint32_t CFUniCharGetConditionalCaseMappingFlags(UTF32Char theChar, UTF16Char *buffer, CFIndex currentIndex, CFIndex length, uint32_t type, const uint8_t *langCode, uint32_t lastFlags) {
    if (theChar == 0x03A3) { // GREEK CAPITAL LETTER SIGMA
        if ((type == kCFUniCharToLowercase) && (currentIndex > 0)) {
            UTF16Char *start = buffer;
            UTF16Char *end = buffer + length;
            UTF32Char otherChar;

            // First check if we're after a cased character
            buffer += (currentIndex - 1);
            while (start <= buffer) {
                otherChar = *(buffer--);
                if (CFUniCharIsSurrogateLowCharacter(otherChar) && (start <= buffer) && CFUniCharIsSurrogateHighCharacter(*buffer)) {
                    otherChar = CFUniCharGetLongCharacterForSurrogatePair(*(buffer--), otherChar);
                }
                if (!CFUniCharIsMemberOf(otherChar, kCFUniCharCaseIgnorableCharacterSet)) {
                    if (!CFUniCharIsMemberOf(otherChar, kCFUniCharUppercaseLetterCharacterSet) && !CFUniCharIsMemberOf(otherChar, kCFUniCharLowercaseLetterCharacterSet)) return 0; // Uppercase set contains titlecase
                    break;
                }
            }

            // Next check if we're before a cased character
            buffer = start + currentIndex + 1;
            while (buffer < end) {
                otherChar = *(buffer++);
                if (CFUniCharIsSurrogateHighCharacter(otherChar) && (buffer < end) && CFUniCharIsSurrogateLowCharacter(*buffer)) {
                    otherChar = CFUniCharGetLongCharacterForSurrogatePair(otherChar, *(buffer++));
                }
                if (!CFUniCharIsMemberOf(otherChar, kCFUniCharCaseIgnorableCharacterSet)) {
                    if (CFUniCharIsMemberOf(otherChar, kCFUniCharUppercaseLetterCharacterSet) || CFUniCharIsMemberOf(otherChar, kCFUniCharLowercaseLetterCharacterSet)) return 0; // Uppercase set contains titlecase
                    break;
                }
            }
            return kCFUniCharCaseMapFinalSigma;
        }
    } else if (langCode) {
        if (*((const uint16_t *)langCode) == LITHUANIAN_LANG_CODE) {
            if ((theChar == 0x0307) && ((kCFUniCharCaseMapAfter_i|kCFUniCharCaseMapMoreAbove) & lastFlags) == (kCFUniCharCaseMapAfter_i|kCFUniCharCaseMapMoreAbove)) {
                return (__CFUniCharIsAfter_i(buffer, currentIndex) ? kCFUniCharCaseMapAfter_i : 0);
            } else if (type == kCFUniCharToLowercase) {
                if ((theChar == 0x0049) || (theChar == 0x004A) || (theChar == 0x012E)) {
                    ++currentIndex;
                    return (__CFUniCharIsMoreAbove(buffer + currentIndex, length - currentIndex) ? kCFUniCharCaseMapMoreAbove : 0);
                }
            } else if ((theChar == 'i') || (theChar == 'j')) {
                ++currentIndex;
                return (__CFUniCharIsMoreAbove(buffer + currentIndex, length - currentIndex) ? (kCFUniCharCaseMapAfter_i|kCFUniCharCaseMapMoreAbove) : 0);
            }
        } else if ((*((const uint16_t *)langCode) == TURKISH_LANG_CODE) || (*((const uint16_t *)langCode) == AZERI_LANG_CODE)) {
            if (type == kCFUniCharToLowercase) {
                if (theChar == 0x0307) {
                    return (kCFUniCharCaseMapMoreAbove & lastFlags ? kCFUniCharCaseMapAfter_i : 0);
                } else if (theChar == 0x0049) {
                    return (((++currentIndex < length) && (buffer[currentIndex] == 0x0307)) ? kCFUniCharCaseMapMoreAbove : 0);
                }
            }
        } else if (*((const uint16_t *)langCode) == DUTCH_LANG_CODE) {
	    if (kCFUniCharCaseMapDutchDigraph & lastFlags) {
		return (((theChar == 0x006A) || (theChar == 0x004A)) ? kCFUniCharCaseMapDutchDigraph : 0);
	    } else {
		if ((type == kCFUniCharToTitlecase) && ((theChar == 0x0069) || (theChar == 0x0049))) {
		    return (((++currentIndex < length) && ((buffer[currentIndex] == 0x006A) || (buffer[currentIndex] == 0x004A))) ? kCFUniCharCaseMapDutchDigraph : 0);
		}
	    }
	}

        if (kCFUniCharCaseMapGreekTonos & lastFlags) { // still searching for tonos
            if (CFUniCharIsMemberOf(theChar, kCFUniCharNonBaseCharacterSet)) {
                return kCFUniCharCaseMapGreekTonos;
            }
        }
        if (((theChar >= 0x0370) && (theChar < 0x0400)) || ((theChar >= 0x1F00) && (theChar < 0x2000))) { // Greek/Coptic & Greek extended ranges
            if ((type == kCFUniCharToUppercase) && (CFUniCharIsMemberOf(theChar, kCFUniCharLetterCharacterSet))) return kCFUniCharCaseMapGreekTonos;
        }
    }
    return 0;
}

// Unicode property database
static __CFUniCharBitmapData *__CFUniCharUnicodePropertyTable = NULL;
static int __CFUniCharUnicodePropertyTableCount = 0;

static CFLock_t __CFUniCharPropTableLock = CFLockInit;

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED || DEPLOYMENT_TARGET_EMBEDDED_MINI || DEPLOYMENT_TARGET_LINUX
#if USE_MACHO_SEGMENT
#define PROP_DB_FILE "__properties"
#else
#define PROP_DB_FILE "/CFUniCharPropertyDatabase.data"
#endif
#elif DEPLOYMENT_TARGET_WINDOWS
#if USE_MACHO_SEGMENT
#define PROP_DB_FILE "__properties"
#else
#define PROP_DB_FILE L"CFUniCharPropertyDatabase.data"
#endif
#else
#error Unknown or unspecified DEPLOYMENT_TARGET
#endif

const void *CFUniCharGetUnicodePropertyDataForPlane(uint32_t propertyType, uint32_t plane) {

    __CFLock(&__CFUniCharPropTableLock);

    if (NULL == __CFUniCharUnicodePropertyTable) {
        __CFUniCharBitmapData *table;
        const void *bytes;
        const void *bodyBase;
        const void *planeBase;
        int headerSize;
        int idx, count;
        int planeIndex, planeCount;
        int planeSize;
	int64_t fileSize;

        if (!__CFUniCharLoadFile(PROP_DB_FILE, &bytes, &fileSize) || !__CFSimpleFileSizeVerification(bytes, fileSize)) {
            __CFUnlock(&__CFUniCharPropTableLock);
            return NULL;
        }

#if defined (__cplusplus)
		bytes = (uint8_t*)bytes + 4; // Skip Unicode version
		headerSize = CFSwapInt32BigToHost(*((uint32_t *)bytes)); bytes = (uint8_t *)bytes + sizeof(uint32_t);
#else
		bytes += 4; // Skip Unicode version
		headerSize = CFSwapInt32BigToHost(*((uint32_t *)bytes)); bytes += sizeof(uint32_t);
#endif
		
        headerSize -= (sizeof(uint32_t) * 2);
        bodyBase = (char *)bytes + headerSize;

        count = headerSize / sizeof(uint32_t);
        __CFUniCharUnicodePropertyTableCount = count;

        table = (__CFUniCharBitmapData *)CFAllocatorAllocate(kCFAllocatorSystemDefault, sizeof(__CFUniCharBitmapData) * count, 0);

        for (idx = 0;idx < count;idx++) {
            planeCount = *((const uint8_t *)bodyBase);
            planeBase = (char *)bodyBase + planeCount + (planeCount % 4 ? 4 - (planeCount % 4) : 0);
            table[idx]._planes = (const uint8_t **)CFAllocatorAllocate(kCFAllocatorSystemDefault, sizeof(const void *) * planeCount, 0);

            for (planeIndex = 0;planeIndex < planeCount;planeIndex++) {
                if ((planeSize = ((const uint8_t *)bodyBase)[planeIndex + 1])) {
                    table[idx]._planes[planeIndex] = (const uint8_t *)planeBase;
#if defined (__cplusplus)   
					planeBase = (char*)planeBase + (planeSize * 256);
#else
					planeBase += (planeSize * 256);
#endif
                } else {
                    table[idx]._planes[planeIndex] = NULL;
                }
            }

            table[idx]._numPlanes = planeCount;
#if defined (__cplusplus)   
			bodyBase = (const uint8_t *)bodyBase + (CFSwapInt32BigToHost(*(uint32_t *)bytes));
			((uint32_t *&)bytes) ++;
#else
			bodyBase += (CFSwapInt32BigToHost(*((uint32_t *)bytes++)));
#endif
        }

        __CFUniCharUnicodePropertyTable = table;
    }

    __CFUnlock(&__CFUniCharPropTableLock);

    return (plane < __CFUniCharUnicodePropertyTable[propertyType]._numPlanes ? __CFUniCharUnicodePropertyTable[propertyType]._planes[plane] : NULL);
}

CF_PRIVATE uint32_t CFUniCharGetNumberOfPlanesForUnicodePropertyData(uint32_t propertyType) {
    (void)CFUniCharGetUnicodePropertyDataForPlane(propertyType, 0);
    return __CFUniCharUnicodePropertyTable[propertyType]._numPlanes;
}

CF_PRIVATE uint32_t CFUniCharGetUnicodeProperty(UTF32Char character, uint32_t propertyType) {
    if (propertyType == kCFUniCharCombiningProperty) {
        return CFUniCharGetCombiningPropertyForCharacter(character, (const uint8_t *)CFUniCharGetUnicodePropertyDataForPlane(propertyType, (character >> 16) & 0xFF));
    } else if (propertyType == kCFUniCharBidiProperty) {
        return CFUniCharGetBidiPropertyForCharacter(character, (const uint8_t *)CFUniCharGetUnicodePropertyDataForPlane(propertyType, (character >> 16) & 0xFF));
    } else {
        return 0;
    }
}



/*
    The UTF8 conversion in the following function is derived from ConvertUTF.c
*/
/*
 * Copyright 2001 Unicode, Inc.
 * 
 * Disclaimer
 * 
 * This source code is provided as is by Unicode, Inc. No claims are
 * made as to fitness for any particular purpose. No warranties of any
 * kind are expressed or implied. The recipient agrees to determine
 * applicability of information provided. If this file has been
 * purchased on magnetic or optical media from Unicode, Inc., the
 * sole remedy for any claim will be exchange of defective media
 * within 90 days of receipt.
 * 
 * Limitations on Rights to Redistribute This Code
 * 
 * Unicode, Inc. hereby grants the right to freely use the information
 * supplied in this file in the creation of products supporting the
 * Unicode Standard, and to make copies of this file in any form
 * for internal or external distribution as long as this notice
 * remains attached.
 */
#define UNI_REPLACEMENT_CHAR (0x0000FFFDUL)

bool CFUniCharFillDestinationBuffer(const UTF32Char *src, CFIndex srcLength, void **dst, CFIndex dstLength, CFIndex *filledLength, uint32_t dstFormat) {
    UTF32Char currentChar;
    CFIndex usedLength = *filledLength;

    if (dstFormat == kCFUniCharUTF16Format) {
        UTF16Char *dstBuffer = (UTF16Char *)*dst;

        while (srcLength-- > 0) {
            currentChar = *(src++);

            if (currentChar > 0xFFFF) { // Non-BMP
                usedLength += 2;
                if (dstLength) {
                    if (usedLength > dstLength) return false;
                    currentChar -= 0x10000;
                    *(dstBuffer++) = (UTF16Char)((currentChar >> 10) + 0xD800UL);
                    *(dstBuffer++) = (UTF16Char)((currentChar & 0x3FF) + 0xDC00UL);
                }
            } else {
                ++usedLength;
                if (dstLength) {
                    if (usedLength > dstLength) return false;
                    *(dstBuffer++) = (UTF16Char)currentChar;
                }
            }
        }

        *dst = dstBuffer;
    } else if (dstFormat == kCFUniCharUTF8Format) {
        uint8_t *dstBuffer = (uint8_t *)*dst;
        uint16_t bytesToWrite = 0;
        const UTF32Char byteMask = 0xBF;
        const UTF32Char byteMark = 0x80; 
        static const uint8_t firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

        while (srcLength-- > 0) {
            currentChar = *(src++);

            /* Figure out how many bytes the result will require */
            if (currentChar < (UTF32Char)0x80) {
                bytesToWrite = 1;
            } else if (currentChar < (UTF32Char)0x800) {
                bytesToWrite = 2;
            } else if (currentChar < (UTF32Char)0x10000) {
                bytesToWrite = 3;
            } else if (currentChar < (UTF32Char)0x200000) {
                bytesToWrite = 4;
            } else {
                bytesToWrite = 2;
                currentChar = UNI_REPLACEMENT_CHAR;
            }

            usedLength += bytesToWrite;

            if (dstLength) {
                if (usedLength > dstLength) return false;

                dstBuffer += bytesToWrite;
                switch (bytesToWrite) {	/* note: everything falls through. */
                    case 4:	*--dstBuffer = (currentChar | byteMark) & byteMask; currentChar >>= 6;
                    case 3:	*--dstBuffer = (currentChar | byteMark) & byteMask; currentChar >>= 6;
                    case 2:	*--dstBuffer = (currentChar | byteMark) & byteMask; currentChar >>= 6;
                    case 1:	*--dstBuffer =  currentChar | firstByteMark[bytesToWrite];
                }
                dstBuffer += bytesToWrite;
            }
        }

        *dst = dstBuffer;
    } else {
        UTF32Char *dstBuffer = (UTF32Char *)*dst;

        while (srcLength-- > 0) {
            currentChar = *(src++);

            ++usedLength;
            if (dstLength) {
                if (usedLength > dstLength) return false;
                *(dstBuffer++) = currentChar;
            }
        }

        *dst = dstBuffer;
    }

    *filledLength = usedLength;

    return true;
}

#if DEPLOYMENT_TARGET_WINDOWS
void __CFUniCharCleanup(void)
{
    int	idx;
    
    // cleanup memory allocated by __CFUniCharLoadBitmapData()
    __CFLock(&__CFUniCharBitmapLock);
    
    if (__CFUniCharBitmapDataArray != NULL) {
        for (idx = 0; idx < (int)__CFUniCharNumberOfBitmaps; idx++) {
            CFAllocatorDeallocate(kCFAllocatorSystemDefault, __CFUniCharBitmapDataArray[idx]._planes);
            __CFUniCharBitmapDataArray[idx]._planes = NULL;
        }
        
        CFAllocatorDeallocate(kCFAllocatorSystemDefault, __CFUniCharBitmapDataArray);
        __CFUniCharBitmapDataArray = NULL;
        __CFUniCharNumberOfBitmaps = 0;
    }
    
    __CFUnlock(&__CFUniCharBitmapLock);
    
    // cleanup memory allocated by CFUniCharGetMappingData()
    __CFLock(&__CFUniCharMappingTableLock);
    
    if (__CFUniCharMappingTables != NULL) {
        CFAllocatorDeallocate(kCFAllocatorSystemDefault, __CFUniCharMappingTables);
        __CFUniCharMappingTables = NULL;
    }
    
    // cleanup memory allocated by __CFUniCharLoadCaseMappingTable()
    if (__CFUniCharCaseMappingTableCounts != NULL) {
        CFAllocatorDeallocate(kCFAllocatorSystemDefault, __CFUniCharCaseMappingTableCounts);
        __CFUniCharCaseMappingTableCounts = NULL;
        
        __CFUniCharCaseMappingTable = NULL;
        __CFUniCharCaseMappingExtraTable = NULL;
    }
    
    __CFUnlock(&__CFUniCharMappingTableLock);
    
    // cleanup memory allocated by CFUniCharGetUnicodePropertyDataForPlane()
    __CFLock(&__CFUniCharPropTableLock);
    
    if (__CFUniCharUnicodePropertyTable != NULL) {
        for (idx = 0; idx < __CFUniCharUnicodePropertyTableCount; idx++) {
            CFAllocatorDeallocate(kCFAllocatorSystemDefault, __CFUniCharUnicodePropertyTable[idx]._planes);
            __CFUniCharUnicodePropertyTable[idx]._planes = NULL;
        }
        
        CFAllocatorDeallocate(kCFAllocatorSystemDefault, __CFUniCharUnicodePropertyTable);
        __CFUniCharUnicodePropertyTable = NULL;
        __CFUniCharUnicodePropertyTableCount = 0;
    }
    
    __CFUnlock(&__CFUniCharPropTableLock);
}
#endif

#undef USE_MACHO_SEGMENT

