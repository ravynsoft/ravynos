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

/*      CFBundle_Grok.c
        Copyright (c) 1999-2014, Apple Inc.  All rights reserved.
        Responsibility: Tony Parker
*/

#include "CFBundle_Internal.h"

#if defined(BINARY_SUPPORT_DYLD)
// Import the mach-o headers that define the macho magic numbers
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <mach-o/arch.h>
#include <mach-o/dyld.h>
#include <mach-o/getsect.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <crt_externs.h>
#if defined(USE_DYLD_PRIV)
#include <mach-o/dyld_priv.h>
#endif /* USE_DYLD_PRIV */
#endif /* BINARY_SUPPORT_DYLD */

#if defined(BINARY_SUPPORT_DLFCN)
#include <dlfcn.h>
#endif /* BINARY_SUPPORT_DLFCN */

#include <sys/stat.h>
#include <ctype.h>

#if DEPLOYMENT_TARGET_WINDOWS
#define statinfo _stat
#define stat(x,y) _NS_stat(x,y)
#define open _NS_open
#define MAP_FAILED 0

// Windows isspace implementation limits the input chars to < 256 in the ASCII range.  It will
// assert in debug builds.  This is annoying.  We merrily grok chars > 256.
static inline BOOL isspace(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r'|| c == '\v' || c == '\f');
}

#else
#define statinfo stat
#endif

#define UNKNOWN_FILETYPE 0x0
#define PEF_FILETYPE 0x1000
#define PEF_MAGIC 0x4a6f7921
#define PEF_CIGAM 0x21796f4a
#define TEXT_SEGMENT "__TEXT"
#define PLIST_SECTION "__info_plist"
#define OBJC_SEGMENT "__OBJC"
#define IMAGE_INFO_SECTION "__image_info"
#define OBJC_SEGMENT_64 "__DATA"
#define IMAGE_INFO_SECTION_64 "__objc_imageinfo"
#define LIB_X11 "/usr/X11R6/lib/libX"

#define XLS_NAME "Book"
#define XLS_NAME2 "Workbook"
#define DOC_NAME "WordDocument"
#define PPT_NAME "PowerPoint Document"

#define ustrncmp(x, y, z) strncmp((char *)(x), (char *)(y), (z))
#define ustrncasecmp(x, y, z) strncasecmp_l((char *)(x), (char *)(y), (z), NULL)

static const uint32_t __CFBundleMagicNumbersArray[] = {
    0xcafebabe, 0xbebafeca, 0xfeedface, 0xcefaedfe, 0xfeedfacf, 0xcffaedfe, 0x4a6f7921, 0x21796f4a, 
    0x7f454c46, 0xffd8ffe0, 0x4d4d002a, 0x49492a00, 0x47494638, 0x89504e47, 0x69636e73, 0x00000100, 
    0x7b5c7274, 0x25504446, 0x2e7261fd, 0x2e524d46, 0x2e736e64, 0x2e736400, 0x464f524d, 0x52494646, 
    0x38425053, 0x000001b3, 0x000001ba, 0x4d546864, 0x504b0304, 0x53495421, 0x53495432, 0x53495435, 
    0x53495444, 0x53747566, 0x30373037, 0x3c212d2d, 0x25215053, 0xd0cf11e0, 0x62656769, 0x3d796265,
    0x6b6f6c79, 0x3026b275, 0x0000000c, 0xfe370023, 0x09020600, 0x09040600, 0x4f676753, 0x664c6143, 
    0x00010000, 0x74727565, 0x4f54544f, 0x41433130, 0xc809fe02, 0x0809fe02, 0x2356524d, 0x67696d70, 
    0x3c435058, 0x28445746, 0x424f4d53, 0x49544f4c, 0x72746664, 0x63616666, 0x802a5fd7, 0x762f3101
};

// string, with groups of 5 characters being 1 element in the array
static const char * __CFBundleExtensionsArray =
    "mach\0"  "mach\0"  "mach\0"  "mach\0"  "mach\0"  "mach\0"  "pef\0\0" "pef\0\0" 
    "elf\0\0" "jpeg\0"  "tiff\0"  "tiff\0"  "gif\0\0" "png\0\0" "icns\0"  "ico\0\0" 
    "rtf\0\0" "pdf\0\0" "ra\0\0\0""rm\0\0\0""au\0\0\0""au\0\0\0""iff\0\0" "riff\0"  
    "psd\0\0" "mpeg\0"  "mpeg\0"  "mid\0\0" "zip\0\0" "sit\0\0" "sit\0\0" "sit\0\0" 
    "sit\0\0" "sit\0\0" "cpio\0"  "html\0"  "ps\0\0\0""ole\0\0" "uu\0\0\0""ync\0\0"
    "dmg\0\0" "wmv\0\0" "jp2\0\0" "doc\0\0" "xls\0\0" "xls\0\0" "ogg\0\0" "flac\0"
    "ttf\0\0" "ttf\0\0" "otf\0\0" "dwg\0\0" "dgn\0\0" "dgn\0\0" "wrl\0\0" "xcf\0\0"
    "cpx\0\0" "dwf\0\0" "bom\0\0" "lit\0\0" "rtfd\0"  "caf\0\0" "cin\0\0" "exr\0\0";

static const char * __CFBundleOOExtensionsArray = "sxc\0\0" "sxd\0\0" "sxg\0\0" "sxi\0\0" "sxm\0\0" "sxw\0\0";
static const char * __CFBundleODExtensionsArray = "odc\0\0" "odf\0\0" "odg\0\0" "oth\0\0" "odi\0\0" "odm\0\0" "odp\0\0" "ods\0\0" "odt\0\0";

#define EXTENSION_LENGTH                5
#define NUM_EXTENSIONS                  64
#define MAGIC_BYTES_TO_READ             512
#define DMG_BYTES_TO_READ               512
#define ZIP_BYTES_TO_READ               1024
#define OLE_BYTES_TO_READ               512
#define X11_BYTES_TO_READ               4096
#define IMAGE_INFO_BYTES_TO_READ        4096

#if defined(BINARY_SUPPORT_DYLD)

static CFMutableDictionaryRef _CFBundleCreateInfoDictFromData(const char *bytes, uint32_t length) {
    CFMutableDictionaryRef result = NULL;
    CFDataRef infoData = NULL;
    if (bytes && 0 < length) {
        infoData = CFDataCreateWithBytesNoCopy(kCFAllocatorSystemDefault, (uint8_t *)bytes, length, kCFAllocatorNull);
        if (infoData) {
            result = (CFMutableDictionaryRef)CFPropertyListCreateWithData(kCFAllocatorSystemDefault, infoData, kCFPropertyListMutableContainers, NULL, NULL);
            if (result && CFDictionaryGetTypeID() != CFGetTypeID(result)) {
                CFRelease(result);
                result = NULL;
            }
            CFRelease(infoData);
        }
        if (!result) result = CFDictionaryCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    }
    if (result) _CFBundleInfoPlistProcessInfoDictionary((CFMutableDictionaryRef)result);
    return result;
}

static char *_CFBundleGetSectData(const char *segname, const char *sectname, unsigned long *size) {
    char *retval = NULL;
    unsigned long localSize = 0;
    uint32_t i, numImages = _dyld_image_count();
    const void *mhp = (const void *)_NSGetMachExecuteHeader();
    
    for (i = 0; i < numImages; i++) {
        if (mhp == (void *)_dyld_get_image_header(i)) {
#if __LP64__
            const struct section_64 *sp = getsectbynamefromheader_64((const struct mach_header_64 *)mhp, segname, sectname);
            if (sp) {
                retval = (char *)(sp->addr + _dyld_get_image_vmaddr_slide(i));
                localSize = (unsigned long)sp->size;
            }
#else /* __LP64__ */
            const struct section *sp = getsectbynamefromheader((const struct mach_header *)mhp, segname, sectname);
            if (sp) {
                retval = (char *)(sp->addr + _dyld_get_image_vmaddr_slide(i));
                localSize = (unsigned long)sp->size;
            }
#endif /* __LP64__ */
            break;
        }
    }
    if (size) *size = localSize;
    return retval;
}

CF_PRIVATE CFMutableDictionaryRef _CFBundleCreateInfoDictFromMainExecutable() {
    char *bytes = NULL;
    unsigned long length = 0;
    if (getsegbyname(TEXT_SEGMENT)) bytes = _CFBundleGetSectData(TEXT_SEGMENT, PLIST_SECTION, &length);
    return _CFBundleCreateInfoDictFromData(bytes, length);
}

CF_PRIVATE Boolean _CFBundleGrokObjCImageInfoFromMainExecutable(uint32_t *objcVersion, uint32_t *objcFlags) {
    Boolean retval = false;
    uint32_t localVersion = 0, localFlags = 0;
    char *bytes = NULL;
    unsigned long length = 0;
#if __LP64__
    if (getsegbyname(OBJC_SEGMENT_64)) bytes = _CFBundleGetSectData(OBJC_SEGMENT_64, IMAGE_INFO_SECTION_64, &length);
#else /* __LP64__ */
    if (getsegbyname(OBJC_SEGMENT)) bytes = _CFBundleGetSectData(OBJC_SEGMENT, IMAGE_INFO_SECTION, &length);
#endif /* __LP64__ */
    if (bytes && length >= 8) {
        localVersion = *(uint32_t *)bytes;
        localFlags = *(uint32_t *)(bytes + 4);
        retval = true;
    }
    if (objcVersion) *objcVersion = localVersion;
    if (objcFlags) *objcFlags = localFlags;
    return retval;
}

static Boolean _CFBundleGrokX11FromFile(int fd, const void *bytes, CFIndex length, uint32_t offset, Boolean swapped, Boolean sixtyFour) {
    static const char libX11name[] = LIB_X11;
    char *buffer = NULL;
    const char *loc = NULL;
    unsigned i;
    Boolean result = false;
    
    if (fd >= 0 && lseek(fd, offset, SEEK_SET) == (off_t)offset) {
        buffer = malloc(X11_BYTES_TO_READ);
        if (buffer && read(fd, buffer, X11_BYTES_TO_READ) >= X11_BYTES_TO_READ) loc = buffer;
    } else if (bytes && length >= offset + X11_BYTES_TO_READ) {
        loc = bytes + offset;
    }
    if (loc) {
        if (sixtyFour) {
            uint32_t ncmds = _CFBundleSwapInt32Conditional(((struct mach_header_64 *)loc)->ncmds, swapped);
            uint32_t sizeofcmds = _CFBundleSwapInt32Conditional(((struct mach_header_64 *)loc)->sizeofcmds, swapped);
            const char *startofcmds = loc + sizeof(struct mach_header_64);
            const char *endofcmds = startofcmds + sizeofcmds;
            struct dylib_command *dlp = (struct dylib_command *)startofcmds;
            if (endofcmds > loc + X11_BYTES_TO_READ) endofcmds = loc + X11_BYTES_TO_READ;
            for (i = 0; !result && i < ncmds && startofcmds <= (char *)dlp && (char *)dlp < endofcmds; i++) {
                if (LC_LOAD_DYLIB == _CFBundleSwapInt32Conditional(dlp->cmd, swapped)) {
                    uint32_t nameoffset = _CFBundleSwapInt32Conditional(dlp->dylib.name.offset, swapped);
                    const char *name = (const char *)dlp + nameoffset;
                    if (startofcmds <= name && name + sizeof(libX11name) <= endofcmds && 0 == strncmp(name, libX11name, sizeof(libX11name) - 1)) result = true;
                }
                dlp = (struct dylib_command *)((char *)dlp + _CFBundleSwapInt32Conditional(dlp->cmdsize, swapped));
            }
        } else {
            uint32_t ncmds = _CFBundleSwapInt32Conditional(((struct mach_header *)loc)->ncmds, swapped);
            uint32_t sizeofcmds = _CFBundleSwapInt32Conditional(((struct mach_header *)loc)->sizeofcmds, swapped);
            const char *startofcmds = loc + sizeof(struct mach_header);
            const char *endofcmds = startofcmds + sizeofcmds;
            struct dylib_command *dlp = (struct dylib_command *)startofcmds;
            if (endofcmds > loc + X11_BYTES_TO_READ) endofcmds = loc + X11_BYTES_TO_READ;
            for (i = 0; !result && i < ncmds && startofcmds <= (char *)dlp && (char *)dlp < endofcmds; i++) {
                if (LC_LOAD_DYLIB == _CFBundleSwapInt32Conditional(dlp->cmd, swapped)) {
                    uint32_t nameoffset = _CFBundleSwapInt32Conditional(dlp->dylib.name.offset, swapped);
                    const char *name = (const char *)dlp + nameoffset;
                    if (startofcmds <= name && name + sizeof(libX11name) <= endofcmds && 0 == strncmp(name, libX11name, sizeof(libX11name) - 1)) result = true;
                }
                dlp = (struct dylib_command *)((char *)dlp + _CFBundleSwapInt32Conditional(dlp->cmdsize, swapped));
            }
        }
    }
    if (buffer) free(buffer);
    return result;
}
    
static CFDictionaryRef _CFBundleCreateInfoDictFromFile(int fd, const void *bytes, CFIndex length, uint32_t offset, Boolean swapped, Boolean sixtyFour) {
    struct statinfo statBuf;
    off_t fileLength = 0;
    char *maploc = NULL;
    const char *loc;
    unsigned i, j;
    CFDictionaryRef result = NULL;
    Boolean foundit = false;
    if (fd >= 0 && fstat(fd, &statBuf) == 0 && (maploc = mmap(0, statBuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) != (void *)-1) {
        loc = maploc;
        fileLength = statBuf.st_size;
    } else {
        loc = bytes;
        fileLength = length;
    }
    if (fileLength > offset + sizeof(struct mach_header_64)) {
        if (sixtyFour) {
            uint32_t ncmds = _CFBundleSwapInt32Conditional(((struct mach_header_64 *)(loc + offset))->ncmds, swapped);
            uint32_t sizeofcmds = _CFBundleSwapInt32Conditional(((struct mach_header_64 *)(loc + offset))->sizeofcmds, swapped);
            const char *startofcmds = loc + offset + sizeof(struct mach_header_64);
            const char *endofcmds = startofcmds + sizeofcmds;
            struct segment_command_64 *sgp = (struct segment_command_64 *)startofcmds;
            if (endofcmds > loc + fileLength) endofcmds = loc + fileLength;
            for (i = 0; !foundit && i < ncmds && startofcmds <= (char *)sgp && (char *)sgp < endofcmds; i++) {
                if (LC_SEGMENT_64 == _CFBundleSwapInt32Conditional(sgp->cmd, swapped)) {
                    struct section_64 *sp = (struct section_64 *)((char *)sgp + sizeof(struct segment_command_64));
                    uint32_t nsects = _CFBundleSwapInt32Conditional(sgp->nsects, swapped);
                    for (j = 0; !foundit && j < nsects && startofcmds <= (char *)sp && (char *)sp < endofcmds; j++) {
                        if (0 == strncmp(sp->sectname, PLIST_SECTION, sizeof(sp->sectname)) && 0 == strncmp(sp->segname, TEXT_SEGMENT, sizeof(sp->segname))) {
                            uint64_t sectlength64 = _CFBundleSwapInt64Conditional(sp->size, swapped);
                            uint32_t sectlength = (uint32_t)(sectlength64 & 0xffffffff);
                            uint32_t sectoffset = _CFBundleSwapInt32Conditional(sp->offset, swapped);
                            const char *sectbytes = loc + offset + sectoffset;
                            // we don't support huge-sized plists
                            if (sectlength64 <= 0xffffffff && loc <= sectbytes && sectbytes + sectlength <= loc + fileLength) result = (CFDictionaryRef)_CFBundleCreateInfoDictFromData(sectbytes, sectlength);
                            foundit = true;
                        }
                        sp = (struct section_64 *)((char *)sp + sizeof(struct section_64));
                    }
                }
                sgp = (struct segment_command_64 *)((char *)sgp + _CFBundleSwapInt32Conditional(sgp->cmdsize, swapped));
            }
        } else {
            uint32_t ncmds = _CFBundleSwapInt32Conditional(((struct mach_header *)(loc + offset))->ncmds, swapped);
            uint32_t sizeofcmds = _CFBundleSwapInt32Conditional(((struct mach_header *)(loc + offset))->sizeofcmds, swapped);
            const char *startofcmds = loc + offset + sizeof(struct mach_header);
            const char *endofcmds = startofcmds + sizeofcmds;
            struct segment_command *sgp = (struct segment_command *)startofcmds;
            if (endofcmds > loc + fileLength) endofcmds = loc + fileLength;
            for (i = 0; !foundit && i < ncmds && startofcmds <= (char *)sgp && (char *)sgp < endofcmds; i++) {
                if (LC_SEGMENT == _CFBundleSwapInt32Conditional(sgp->cmd, swapped)) {
                    struct section *sp = (struct section *)((char *)sgp + sizeof(struct segment_command));
                    uint32_t nsects = _CFBundleSwapInt32Conditional(sgp->nsects, swapped);
                    for (j = 0; !foundit && j < nsects && startofcmds <= (char *)sp && (char *)sp < endofcmds; j++) {
                        if (0 == strncmp(sp->sectname, PLIST_SECTION, sizeof(sp->sectname)) && 0 == strncmp(sp->segname, TEXT_SEGMENT, sizeof(sp->segname))) {
                            uint32_t sectlength = _CFBundleSwapInt32Conditional(sp->size, swapped);
                            uint32_t sectoffset = _CFBundleSwapInt32Conditional(sp->offset, swapped);
                            const char *sectbytes = loc + offset + sectoffset;
                            if (loc <= sectbytes && sectbytes + sectlength <= loc + fileLength) result = (CFDictionaryRef)_CFBundleCreateInfoDictFromData(sectbytes, sectlength);
                            foundit = true;
                        }
                        sp = (struct section *)((char *)sp + sizeof(struct section));
                    }
                }
                sgp = (struct segment_command *)((char *)sgp + _CFBundleSwapInt32Conditional(sgp->cmdsize, swapped));
            }
        }
    }
    if (maploc) munmap(maploc, statBuf.st_size);
    return result;
}

static void _CFBundleGrokObjcImageInfoFromFile(int fd, const void *bytes, CFIndex length, uint32_t offset, Boolean swapped, Boolean sixtyFour, Boolean *hasObjc, uint32_t *objcVersion, uint32_t *objcFlags) {
    uint32_t sectlength = 0, sectoffset = 0, localVersion = 0, localFlags = 0;
    char *buffer = NULL;
    char sectbuffer[8];
    const char *loc = NULL;
    unsigned i, j;
    Boolean foundit = false, localHasObjc = false;
    
    if (fd >= 0 && lseek(fd, offset, SEEK_SET) == (off_t)offset) {
        buffer = malloc(IMAGE_INFO_BYTES_TO_READ);
        if (buffer && read(fd, buffer, IMAGE_INFO_BYTES_TO_READ) >= IMAGE_INFO_BYTES_TO_READ) loc = buffer;
    } else if (bytes && length >= offset + IMAGE_INFO_BYTES_TO_READ) {
        loc = bytes + offset;
    }
    if (loc) {
        if (sixtyFour) {
            uint32_t ncmds = _CFBundleSwapInt32Conditional(((struct mach_header_64 *)loc)->ncmds, swapped);
            uint32_t sizeofcmds = _CFBundleSwapInt32Conditional(((struct mach_header_64 *)loc)->sizeofcmds, swapped);
            const char *startofcmds = loc + sizeof(struct mach_header_64);
            const char *endofcmds = startofcmds + sizeofcmds;
            struct segment_command_64 *sgp = (struct segment_command_64 *)startofcmds;
            if (endofcmds > loc + IMAGE_INFO_BYTES_TO_READ) endofcmds = loc + IMAGE_INFO_BYTES_TO_READ;
            for (i = 0; !foundit && i < ncmds && startofcmds <= (char *)sgp && (char *)sgp < endofcmds; i++) {
                if (LC_SEGMENT_64 == _CFBundleSwapInt32Conditional(sgp->cmd, swapped)) {
                    struct section_64 *sp = (struct section_64 *)((char *)sgp + sizeof(struct segment_command_64));
                    uint32_t nsects = _CFBundleSwapInt32Conditional(sgp->nsects, swapped);
                    for (j = 0; !foundit && j < nsects && startofcmds <= (char *)sp && (char *)sp < endofcmds; j++) {
                        if (0 == strncmp(sp->segname, OBJC_SEGMENT_64, sizeof(sp->segname))) localHasObjc = true;
                        if (0 == strncmp(sp->sectname, IMAGE_INFO_SECTION_64, sizeof(sp->sectname)) && 0 == strncmp(sp->segname, OBJC_SEGMENT_64, sizeof(sp->segname))) {
                            uint64_t sectlength64 = _CFBundleSwapInt64Conditional(sp->size, swapped);
                            sectlength = (uint32_t)(sectlength64 & 0xffffffff);
                            sectoffset = _CFBundleSwapInt32Conditional(sp->offset, swapped);
                            foundit = true;
                        }
                        sp = (struct section_64 *)((char *)sp + sizeof(struct section_64));
                    }
                }
                sgp = (struct segment_command_64 *)((char *)sgp + _CFBundleSwapInt32Conditional(sgp->cmdsize, swapped));
            }
        } else {
            uint32_t ncmds = _CFBundleSwapInt32Conditional(((struct mach_header *)loc)->ncmds, swapped);
            uint32_t sizeofcmds = _CFBundleSwapInt32Conditional(((struct mach_header *)loc)->sizeofcmds, swapped);
            const char *startofcmds = loc + sizeof(struct mach_header);
            const char *endofcmds = startofcmds + sizeofcmds;
            struct segment_command *sgp = (struct segment_command *)startofcmds;
            if (endofcmds > loc + IMAGE_INFO_BYTES_TO_READ) endofcmds = loc + IMAGE_INFO_BYTES_TO_READ;
            for (i = 0; !foundit && i < ncmds && startofcmds <= (char *)sgp && (char *)sgp < endofcmds; i++) {
                if (LC_SEGMENT == _CFBundleSwapInt32Conditional(sgp->cmd, swapped)) {
                    struct section *sp = (struct section *)((char *)sgp + sizeof(struct segment_command));
                    uint32_t nsects = _CFBundleSwapInt32Conditional(sgp->nsects, swapped);
                    for (j = 0; !foundit && j < nsects && startofcmds <= (char *)sp && (char *)sp < endofcmds; j++) {
                        if (0 == strncmp(sp->segname, OBJC_SEGMENT, sizeof(sp->segname))) localHasObjc = true;
                        if (0 == strncmp(sp->sectname, IMAGE_INFO_SECTION, sizeof(sp->sectname)) && 0 == strncmp(sp->segname, OBJC_SEGMENT, sizeof(sp->segname))) {
                            sectlength = _CFBundleSwapInt32Conditional(sp->size, swapped);
                            sectoffset = _CFBundleSwapInt32Conditional(sp->offset, swapped);
                            foundit = true;
                        }
                        sp = (struct section *)((char *)sp + sizeof(struct section));
                    }
                }
                sgp = (struct segment_command *)((char *)sgp + _CFBundleSwapInt32Conditional(sgp->cmdsize, swapped));
            }
        }
        if (sectlength >= 8) {
            if (fd >= 0 && lseek(fd, offset + sectoffset, SEEK_SET) == (off_t)(offset + sectoffset) && read(fd, sectbuffer, 8) >= 8) {
                localVersion = _CFBundleSwapInt32Conditional(*(uint32_t *)sectbuffer, swapped);
                localFlags = _CFBundleSwapInt32Conditional(*(uint32_t *)(sectbuffer + 4), swapped);
            } else if (bytes && length >= offset + sectoffset + 8) {
                localVersion = _CFBundleSwapInt32Conditional(*(uint32_t *)(bytes + offset + sectoffset), swapped);
                localFlags = _CFBundleSwapInt32Conditional(*(uint32_t *)(bytes + offset + sectoffset + 4), swapped);
            }
        }
    }
    if (buffer) free(buffer);
    if (hasObjc) *hasObjc = localHasObjc;
    if (objcVersion) *objcVersion = localVersion;
    if (objcFlags) *objcFlags = localFlags;
}

static UInt32 _CFBundleGrokMachTypeForFatFile(int fd, const void *bytes, CFIndex length, Boolean swap, Boolean *isX11, CFArrayRef *architectures, CFDictionaryRef *infodict, Boolean *hasObjc, uint32_t *objcVersion, uint32_t *objcFlags) {
    CFIndex headerLength = length;
    unsigned char headerBuffer[MAGIC_BYTES_TO_READ];
    UInt32 machtype = UNKNOWN_FILETYPE, magic, numFatHeaders, maxFatHeaders, i;
    unsigned char buffer[sizeof(struct mach_header_64)];
    const unsigned char *moreBytes = NULL;
    const NXArchInfo *archInfo = NXGetLocalArchInfo();
    SInt32 curArch = _CFBundleCurrentArchitecture();
    
    struct fat_arch *fat = NULL;

    if (isX11) *isX11 = false;
    if (architectures) *architectures = NULL;
    if (infodict) *infodict = NULL;
    if (hasObjc) *hasObjc = false;
    if (objcVersion) *objcVersion = 0;
    if (objcFlags) *objcFlags = 0;

    if (headerLength > MAGIC_BYTES_TO_READ) headerLength = MAGIC_BYTES_TO_READ;
    (void)memmove(headerBuffer, bytes, headerLength);
    if (swap) {
        for (i = 0; i < headerLength; i += 4) *(UInt32 *)(headerBuffer + i) = CFSwapInt32(*(UInt32 *)(headerBuffer + i));
    }
    numFatHeaders = ((struct fat_header *)headerBuffer)->nfat_arch;
    maxFatHeaders = (headerLength - sizeof(struct fat_header)) / sizeof(struct fat_arch);
    if (numFatHeaders > maxFatHeaders) numFatHeaders = maxFatHeaders;
    if (numFatHeaders > 0) {
        if (archInfo) fat = NXFindBestFatArch(archInfo->cputype, archInfo->cpusubtype, (struct fat_arch *)(headerBuffer + sizeof(struct fat_header)), numFatHeaders);
        if (!fat && curArch != 0) fat = NXFindBestFatArch((cpu_type_t)curArch, (cpu_subtype_t)0, (struct fat_arch *)(headerBuffer + sizeof(struct fat_header)), numFatHeaders);
        if (!fat) fat = (struct fat_arch *)(headerBuffer + sizeof(struct fat_header));
        if (architectures) {
            CFMutableArrayRef mutableArchitectures = CFArrayCreateMutable(kCFAllocatorSystemDefault, 0, &kCFTypeArrayCallBacks);
            for (i = 0; i < numFatHeaders; i++) {
                CFNumberRef architecture = CFNumberCreate(kCFAllocatorSystemDefault, kCFNumberSInt32Type, headerBuffer + sizeof(struct fat_header) + i * sizeof(struct fat_arch));
                if (CFArrayGetFirstIndexOfValue(mutableArchitectures, CFRangeMake(0, CFArrayGetCount(mutableArchitectures)), architecture) < 0) CFArrayAppendValue(mutableArchitectures, architecture);
                CFRelease(architecture);
            }
            *architectures = (CFArrayRef)mutableArchitectures;
        }
    } 
    if (fat) {
        if (fd >= 0 && lseek(fd, fat->offset, SEEK_SET) == (off_t)fat->offset && read(fd, buffer, sizeof(struct mach_header_64)) >= (int)sizeof(struct mach_header_64)) {
            moreBytes = buffer;
        } else if (bytes && (uint32_t)length >= fat->offset + sizeof(struct mach_header_64)) {
            moreBytes = bytes + fat->offset;
        }
        if (moreBytes) {
            magic = *((UInt32 *)moreBytes);
            if (MH_MAGIC == magic) {
                machtype = ((struct mach_header *)moreBytes)->filetype;
                if (isX11 && MH_EXECUTE == machtype) *isX11 = _CFBundleGrokX11FromFile(fd, bytes, length, fat->offset, false, false);
                if (infodict) *infodict = _CFBundleCreateInfoDictFromFile(fd, bytes, length, fat->offset, false, false);
                if (hasObjc || objcVersion || objcFlags) _CFBundleGrokObjcImageInfoFromFile(fd, bytes, length, fat->offset, false, false, hasObjc, objcVersion, objcFlags);
            } else if (MH_CIGAM == magic) {
                machtype = CFSwapInt32(((struct mach_header *)moreBytes)->filetype);
                if (isX11 && MH_EXECUTE == machtype) *isX11 = _CFBundleGrokX11FromFile(fd, bytes, length, fat->offset, true, false);
                if (infodict) *infodict = _CFBundleCreateInfoDictFromFile(fd, bytes, length, fat->offset, true, false);
                if (hasObjc || objcVersion || objcFlags) _CFBundleGrokObjcImageInfoFromFile(fd, bytes, length, fat->offset, true, false, hasObjc, objcVersion, objcFlags);
            } else if (MH_MAGIC_64 == magic) {
                machtype = ((struct mach_header_64 *)moreBytes)->filetype;
                if (isX11 && MH_EXECUTE == machtype) *isX11 = _CFBundleGrokX11FromFile(fd, bytes, length, fat->offset, false, true);
                if (infodict) *infodict = _CFBundleCreateInfoDictFromFile(fd, bytes, length, fat->offset, false, true);
                if (hasObjc || objcVersion || objcFlags) _CFBundleGrokObjcImageInfoFromFile(fd, bytes, length, fat->offset, false, true, hasObjc, objcVersion, objcFlags);
            } else if (MH_CIGAM_64 == magic) {
                machtype = CFSwapInt32(((struct mach_header_64 *)moreBytes)->filetype);
                if (isX11 && MH_EXECUTE == machtype) *isX11 = _CFBundleGrokX11FromFile(fd, bytes, length, fat->offset, true, true);
                if (infodict) *infodict = _CFBundleCreateInfoDictFromFile(fd, bytes, length, fat->offset, true, true);
                if (hasObjc || objcVersion || objcFlags) _CFBundleGrokObjcImageInfoFromFile(fd, bytes, length, fat->offset, true, true, hasObjc, objcVersion, objcFlags);
            }
        }
    }
    return machtype;
}

static UInt32 _CFBundleGrokMachType(int fd, const void *bytes, CFIndex length, Boolean *isX11, CFArrayRef *architectures, CFDictionaryRef *infodict, Boolean *hasObjc, uint32_t *objcVersion, uint32_t *objcFlags) {
    unsigned int magic = *((UInt32 *)bytes), machtype = UNKNOWN_FILETYPE, cputype;
    CFNumberRef architecture = NULL;

    if (isX11) *isX11 = false;
    if (architectures) *architectures = NULL;
    if (infodict) *infodict = NULL;
    if (hasObjc) *hasObjc = false;
    if (objcVersion) *objcVersion = 0;
    if (objcFlags) *objcFlags = 0;
    if (MH_MAGIC == magic) {
        machtype = ((struct mach_header *)bytes)->filetype;
        cputype = ((struct mach_header *)bytes)->cputype;
        if (architectures) architecture = CFNumberCreate(kCFAllocatorSystemDefault, kCFNumberSInt32Type, &cputype);
        if (isX11 && MH_EXECUTE == machtype) *isX11 = _CFBundleGrokX11FromFile(fd, bytes, length, 0, false, false);
        if (infodict) *infodict = _CFBundleCreateInfoDictFromFile(fd, bytes, length, 0, false, false);
        if (hasObjc || objcVersion || objcFlags) _CFBundleGrokObjcImageInfoFromFile(fd, bytes, length, 0, false, false, hasObjc, objcVersion, objcFlags);
    } else if (MH_CIGAM == magic) {
        machtype = CFSwapInt32(((struct mach_header *)bytes)->filetype);
        cputype = CFSwapInt32(((struct mach_header *)bytes)->cputype);
        if (architectures) architecture = CFNumberCreate(kCFAllocatorSystemDefault, kCFNumberSInt32Type, &cputype);
        if (isX11 && MH_EXECUTE == machtype) *isX11 = _CFBundleGrokX11FromFile(fd, bytes, length, 0, true, false);
        if (infodict) *infodict = _CFBundleCreateInfoDictFromFile(fd, bytes, length, 0, true, false);
        if (hasObjc || objcVersion || objcFlags) _CFBundleGrokObjcImageInfoFromFile(fd, bytes, length, 0, true, false, hasObjc, objcVersion, objcFlags);
    } else if (MH_MAGIC_64 == magic) {
        machtype = ((struct mach_header_64 *)bytes)->filetype;
        cputype = ((struct mach_header_64 *)bytes)->cputype;
        if (architectures) architecture = CFNumberCreate(kCFAllocatorSystemDefault, kCFNumberSInt32Type, &cputype);
        if (isX11 && MH_EXECUTE == machtype) *isX11 = _CFBundleGrokX11FromFile(fd, bytes, length, 0, false, true);
        if (infodict) *infodict = _CFBundleCreateInfoDictFromFile(fd, bytes, length, 0, false, true);
        if (hasObjc || objcVersion || objcFlags) _CFBundleGrokObjcImageInfoFromFile(fd, bytes, length, 0, false, true, hasObjc, objcVersion, objcFlags);
    } else if (MH_CIGAM_64 == magic) {
        machtype = CFSwapInt32(((struct mach_header_64 *)bytes)->filetype);
        cputype = CFSwapInt32(((struct mach_header_64 *)bytes)->cputype);
        if (architectures) architecture = CFNumberCreate(kCFAllocatorSystemDefault, kCFNumberSInt32Type, &cputype);
        if (isX11 && MH_EXECUTE == machtype) *isX11 = _CFBundleGrokX11FromFile(fd, bytes, length, 0, true, true);
        if (infodict) *infodict = _CFBundleCreateInfoDictFromFile(fd, bytes, length, 0, true, true);
        if (hasObjc || objcVersion || objcFlags) _CFBundleGrokObjcImageInfoFromFile(fd, bytes, length, 0, true, true, hasObjc, objcVersion, objcFlags);
    } else if (FAT_MAGIC == magic) {
        machtype = _CFBundleGrokMachTypeForFatFile(fd, bytes, length, false, isX11, architectures, infodict, hasObjc, objcVersion, objcFlags);
    } else if (FAT_CIGAM == magic) {
        machtype = _CFBundleGrokMachTypeForFatFile(fd, bytes, length, true, isX11, architectures, infodict, hasObjc, objcVersion, objcFlags);
    } else if (PEF_MAGIC == magic || PEF_CIGAM == magic) {
        machtype = PEF_FILETYPE;
    }
    if (architectures && architecture) *architectures = CFArrayCreate(kCFAllocatorSystemDefault, (const void **)&architecture, 1, &kCFTypeArrayCallBacks);
    if (architecture) CFRelease(architecture);
    return machtype;
}

#endif /* BINARY_SUPPORT_DYLD */

static Boolean _CFBundleGrokFileTypeForZipMimeType(const unsigned char *bytes, CFIndex length, const char **ext) {
    unsigned namelength = CFSwapInt16HostToLittle(*((UInt16 *)(bytes + 26))), extralength = CFSwapInt16HostToLittle(*((UInt16 *)(bytes + 28)));
    const unsigned char *data = bytes + 30 + namelength + extralength;
    int i = -1;
    if (bytes < data && data + 56 <= bytes + length && 0 == CFSwapInt16HostToLittle(*((UInt16 *)(bytes + 8))) && (0 == ustrncasecmp(data, "application/vnd.", 16) || 0 == ustrncasecmp(data, "application/x-vnd.", 18))) {
        data += ('.' == *(data + 15)) ? 16 : 18;
        if (0 == ustrncasecmp(data, "sun.xml.", 8)) {
            data += 8;
            if (0 == ustrncasecmp(data, "calc", 4)) i = 0;
            else if (0 == ustrncasecmp(data, "draw", 4)) i = 1;
            else if (0 == ustrncasecmp(data, "writer.global", 13)) i = 2;
            else if (0 == ustrncasecmp(data, "impress", 7)) i = 3;
            else if (0 == ustrncasecmp(data, "math", 4)) i = 4;
            else if (0 == ustrncasecmp(data, "writer", 6)) i = 5;
            if (i >= 0 && ext) *ext = __CFBundleOOExtensionsArray + i * EXTENSION_LENGTH;
        } else if (0 == ustrncasecmp(data, "oasis.opendocument.", 19)) {
            data += 19;
            if (0 == ustrncasecmp(data, "chart", 5)) i = 0;
            else if (0 == ustrncasecmp(data, "formula", 7)) i = 1;
            else if (0 == ustrncasecmp(data, "graphics", 8)) i = 2;
            else if (0 == ustrncasecmp(data, "text-web", 8)) i = 3;
            else if (0 == ustrncasecmp(data, "image", 5)) i = 4;
            else if (0 == ustrncasecmp(data, "text-master", 11)) i = 5;
            else if (0 == ustrncasecmp(data, "presentation", 12)) i = 6;
            else if (0 == ustrncasecmp(data, "spreadsheet", 11)) i = 7;
            else if (0 == ustrncasecmp(data, "text", 4)) i = 8;
            if (i >= 0 && ext) *ext = __CFBundleODExtensionsArray + i * EXTENSION_LENGTH;
        }
    } else if (bytes < data && data + 41 <= bytes + length && 8 == CFSwapInt16HostToLittle(*((UInt16 *)(bytes + 8))) && 0x4b2c28c8 == CFSwapInt32HostToBig(*((UInt32 *)data)) && 0xc94c4e2c == CFSwapInt32HostToBig(*((UInt32 *)(data + 4)))) {
        // AbiWord compressed mimetype odt
        if (ext) *ext = "odt";
        // almost certainly this should set i to 0 but I don't want to upset the apple cart now
    } else if (bytes < data && data + 29 <= bytes + length && (0 == ustrncasecmp(data, "application/oebps-package+xml", 29))) {
        // epub, official epub 3 mime type
        if (ext) *ext = "epub";
        i = 0;
    } else if (bytes < data && data + 20 <= bytes + length && (0 == ustrncasecmp(data, "application/epub+zip", 20))) {
        // epub, unofficial epub 2 mime type
        if (ext) *ext = "epub";
        i = 0;
    }
    return (i >= 0);
}

static const char *_CFBundleGrokFileTypeForZipFile(int fd, const unsigned char *bytes, CFIndex length, off_t fileLength) {
    const char *ext = "zip";
    const unsigned char *moreBytes = NULL;
    unsigned char *buffer = NULL;
    CFIndex i;
    Boolean foundMimetype = false, hasMetaInf = false, hasContentXML = false, hasManifestMF = false, hasManifestXML = false, hasRels = false, hasContentTypes = false, hasWordDocument = false, hasExcelDocument = false, hasPowerPointDocument = false, hasOPF = false, hasSMIL = false;

    if (bytes) {
        for (i = 0; !foundMimetype && i + 30 < length; i++) {
            if (0x50 == bytes[i] && 0x4b == bytes[i + 1]) {
                unsigned namelength = 0, offset = 0;
                if (0x01 == bytes[i + 2] && 0x02 == bytes[i + 3]) {
                    namelength = (unsigned)CFSwapInt16HostToLittle(*((UInt16 *)(bytes + i + 28)));
                    offset = 46;
                } else if (0x03 == bytes[i + 2] && 0x04 == bytes[i + 3]) {
                    namelength = (unsigned)CFSwapInt16HostToLittle(*((UInt16 *)(bytes + i + 26)));
                    offset = 30;
                }
                if (offset > 0 && (CFIndex)(i + offset + namelength) <= length) {
                    //printf("%.*s\n", namelength, bytes + i + offset);
                    if (8 == namelength && 30 == offset && 0 == ustrncasecmp(bytes + i + offset, "mimetype", 8)) foundMimetype = _CFBundleGrokFileTypeForZipMimeType(bytes + i, length - i, &ext);
                    else if (9 == namelength && 0 == ustrncasecmp(bytes + i + offset, "META-INF/", 9)) hasMetaInf = true;
                    else if (11 == namelength && 0 == ustrncasecmp(bytes + i + offset, "content.xml", 11)) hasContentXML = true;
                    else if (11 == namelength && 0 == ustrncasecmp(bytes + i + offset, "_rels/.rels", 11)) hasRels = true;
                    else if (19 == namelength && 0 == ustrncasecmp(bytes + i + offset, "[Content_Types].xml", 19)) hasContentTypes = true;
                    else if (20 == namelength && 0 == ustrncasecmp(bytes + i + offset, "META-INF/MANIFEST.MF", 20)) hasManifestMF = true;
                    else if (21 == namelength && 0 == ustrncasecmp(bytes + i + offset, "META-INF/manifest.xml", 21)) hasManifestXML = true;
                    else if (4 < namelength && 0 == ustrncasecmp(bytes + i + offset + namelength - 4, ".opf", 4)) hasOPF = true;
                    else if (4 < namelength && 0 == ustrncasecmp(bytes + i + offset + namelength - 4, ".sml", 4)) hasSMIL = true;
                    else if (5 < namelength && 0 == ustrncasecmp(bytes + i + offset + namelength - 5, ".smil", 5)) hasSMIL = true;
                    else if (7 < namelength && 0 == ustrncasecmp(bytes + i + offset, "xl/", 3) && 0 == ustrncasecmp(bytes + i + offset + namelength - 4, ".xml", 4)) hasExcelDocument = true;
                    else if (8 < namelength && 0 == ustrncasecmp(bytes + i + offset, "ppt/", 4) && 0 == ustrncasecmp(bytes + i + offset + namelength - 4, ".xml", 4)) hasPowerPointDocument = true;
                    else if (9 < namelength && 0 == ustrncasecmp(bytes + i + offset, "word/", 5) && 0 == ustrncasecmp(bytes + i + offset + namelength - 4, ".xml", 4)) hasWordDocument = true;
                    else if (10 < namelength && 0 == ustrncasecmp(bytes + i + offset, "excel/", 6) && 0 == ustrncasecmp(bytes + i + offset + namelength - 4, ".xml", 4)) hasExcelDocument = true;
                    else if (15 < namelength && 0 == ustrncasecmp(bytes + i + offset, "powerpoint/", 11) && 0 == ustrncasecmp(bytes + i + offset + namelength - 4, ".xml", 4)) hasPowerPointDocument = true;
                    i += offset + namelength - 1;
                }
            }
        }
    }
    if (!foundMimetype) {
        if (fileLength >= ZIP_BYTES_TO_READ) {
            if (fd >= 0 && lseek(fd, fileLength - ZIP_BYTES_TO_READ, SEEK_SET) == fileLength - ZIP_BYTES_TO_READ) {
                buffer = (unsigned char *)malloc(ZIP_BYTES_TO_READ);
                if (buffer && read(fd, buffer, ZIP_BYTES_TO_READ) >= ZIP_BYTES_TO_READ) moreBytes = buffer;
            } else if (bytes && length >= ZIP_BYTES_TO_READ) {
                moreBytes = bytes + length - ZIP_BYTES_TO_READ;
            }
        }
        if (moreBytes) {
            for (i = 0; i + 30 < ZIP_BYTES_TO_READ; i++) {
                if (0x50 == moreBytes[i] && 0x4b == moreBytes[i + 1]) {
                    unsigned namelength = 0, offset = 0;
                    if (0x01 == moreBytes[i + 2] && 0x02 == moreBytes[i + 3]) {
                        namelength = CFSwapInt16HostToLittle(*((UInt16 *)(moreBytes + i + 28)));
                        offset = 46;
                    } else if (0x03 == moreBytes[i + 2] && 0x04 == moreBytes[i + 3]) {
                        namelength = CFSwapInt16HostToLittle(*((UInt16 *)(moreBytes + i + 26)));
                        offset = 30;
                    }
                    if (offset > 0 && i + offset + namelength <= ZIP_BYTES_TO_READ) {
                        //printf("%.*s\n", namelength, moreBytes + i + offset);
                        if (9 == namelength && 0 == ustrncasecmp(moreBytes + i + offset, "META-INF/", 9)) hasMetaInf = true;
                        else if (11 == namelength && 0 == ustrncasecmp(moreBytes + i + offset, "content.xml", 11)) hasContentXML = true;
                        else if (11 == namelength && 0 == ustrncasecmp(moreBytes + i + offset, "_rels/.rels", 11)) hasRels = true;
                        else if (19 == namelength && 0 == ustrncasecmp(moreBytes + i + offset, "[Content_Types].xml", 19)) hasContentTypes = true;
                        else if (20 == namelength && 0 == ustrncasecmp(moreBytes + i + offset, "META-INF/MANIFEST.MF", 20)) hasManifestMF = true;
                        else if (21 == namelength && 0 == ustrncasecmp(moreBytes + i + offset, "META-INF/manifest.xml", 21)) hasManifestXML = true;
                        else if (4 < namelength && 0 == ustrncasecmp(moreBytes + i + offset + namelength - 4, ".opf", 4)) hasOPF = true;
                        else if (4 < namelength && 0 == ustrncasecmp(moreBytes + i + offset + namelength - 4, ".sml", 4)) hasSMIL = true;
                        else if (5 < namelength && 0 == ustrncasecmp(moreBytes + i + offset + namelength - 5, ".smil", 5)) hasSMIL = true;
                        else if (7 < namelength && 0 == ustrncasecmp(moreBytes + i + offset, "xl/", 3) && 0 == ustrncasecmp(moreBytes + i + offset + namelength - 4, ".xml", 4)) hasExcelDocument = true;
                        else if (8 < namelength && 0 == ustrncasecmp(moreBytes + i + offset, "ppt/", 4) && 0 == ustrncasecmp(moreBytes + i + offset + namelength - 4, ".xml", 4)) hasPowerPointDocument = true;
                        else if (9 < namelength && 0 == ustrncasecmp(moreBytes + i + offset, "word/", 5) && 0 == ustrncasecmp(moreBytes + i + offset + namelength - 4, ".xml", 4)) hasWordDocument = true;
                        else if (10 < namelength && 0 == ustrncasecmp(moreBytes + i + offset, "excel/", 6) && 0 == ustrncasecmp(moreBytes + i + offset + namelength - 4, ".xml", 4)) hasExcelDocument = true;
                        else if (15 < namelength && 0 == ustrncasecmp(moreBytes + i + offset, "powerpoint/", 11) && 0 == ustrncasecmp(moreBytes + i + offset + namelength - 4, ".xml", 4)) hasPowerPointDocument = true;
                        i += offset + namelength - 1;
                    }
                }
            }
        }
        //printf("hasManifestMF %d hasManifestXML %d hasContentXML %d hasRels %d hasContentTypes %d hasWordDocument %d hasExcelDocument %d hasPowerPointDocument %d hasMetaInf %d hasOPF %d hasSMIL %d\n", hasManifestMF, hasManifestXML, hasContentXML, hasRels, hasContentTypes, hasWordDocument, hasExcelDocument, hasPowerPointDocument, hasMetaInf, hasOPF, hasSMIL);
        if (hasManifestMF) ext = "jar";
        else if ((hasRels || hasContentTypes) && hasWordDocument) ext = "docx";
        else if ((hasRels || hasContentTypes) && hasExcelDocument) ext = "xlsx";
        else if ((hasRels || hasContentTypes) && hasPowerPointDocument) ext = "pptx";
        else if (hasManifestXML || hasContentXML) ext = "odt";
        else if (hasMetaInf) ext = "jar";
        else if (hasOPF && hasSMIL) ext = "dtb";
        else if (hasOPF) ext = "oeb";

        if (buffer) free(buffer);
    }
    return ext;
}

static Boolean _CFBundleCheckOLEName(const char *name, const char *bytes, unsigned length) {
    Boolean retval = true;
    unsigned j;
    for (j = 0; retval && j < length; j++) if (bytes[2 * j] != name[j]) retval = false;
    return retval;
}

static const char *_CFBundleGrokFileTypeForOLEFile(int fd, const void *bytes, CFIndex length, off_t offset) {
    const char *ext = "ole", *moreBytes = NULL;
    char *buffer = NULL;
    
    if (fd >= 0 && lseek(fd, offset, SEEK_SET) == (off_t)offset) {
        buffer = (char *)malloc(OLE_BYTES_TO_READ);
        if (buffer && read(fd, buffer, OLE_BYTES_TO_READ) >= OLE_BYTES_TO_READ) moreBytes = buffer;
    } else if (bytes && length >= offset + OLE_BYTES_TO_READ) {
        moreBytes = (char *)bytes + offset;
    }
    if (moreBytes) {
        Boolean foundit = false;
        unsigned i;
        for (i = 0; !foundit && i < 4; i++) {
            char namelength = moreBytes[128 * i + 64] / 2;
            foundit = true;
            if (sizeof(XLS_NAME) == namelength && _CFBundleCheckOLEName(XLS_NAME, moreBytes + 128 * i, namelength - 1)) ext = "xls";
            else if (sizeof(XLS_NAME2) == namelength && _CFBundleCheckOLEName(XLS_NAME2, moreBytes + 128 * i, namelength - 1)) ext = "xls";
            else if (sizeof(DOC_NAME) == namelength && _CFBundleCheckOLEName(DOC_NAME, moreBytes + 128 * i, namelength - 1)) ext = "doc";
            else if (sizeof(PPT_NAME) == namelength && _CFBundleCheckOLEName(PPT_NAME, moreBytes + 128 * i, namelength - 1)) ext = "ppt";
            else foundit = false;
        }
    }
    if (buffer) free(buffer);
    return ext;
}

static Boolean _CFBundleGrokFileType(CFURLRef url, CFDataRef data, CFStringRef *extension, UInt32 *machtype, CFArrayRef *architectures, CFDictionaryRef *infodict, Boolean *hasObjc, uint32_t *objcVersion, uint32_t *objcFlags) {
    int fd = -1;
    const unsigned char *bytes = NULL;
    unsigned char buffer[MAGIC_BYTES_TO_READ];
    CFIndex i, length = 0;
    off_t fileLength = 0;
    const char *ext = NULL;
    UInt32 mt = UNKNOWN_FILETYPE;
#if defined(BINARY_SUPPORT_DYLD)
    Boolean isX11 = false;
#endif /* BINARY_SUPPORT_DYLD */
    Boolean isFile = false, isPlain = true, isZero = true, isSpace = true, hasBOM = false;
    // extensions returned:  o, tool, x11app, pef, core, dylib, bundle, elf, jpeg, jp2, tiff, gif, png, pict, icns, ico, rtf, rtfd, pdf, ra, rm, au, aiff, aifc, caf, wav, avi, wmv, ogg, flac, psd, mpeg, mid, zip, jar, sit, cpio, html, ps, mov, qtif, ttf, otf, sfont, bmp, hqx, bin, class, tar, txt, gz, Z, uu, ync, bz, bz2, sh, pl, py, rb, dvi, sgi, tga, mp3, xml, plist, xls, doc, ppt, mp4, m4a, m4b, m4p, m4v, 3gp, 3g2, dmg, cwk, webarchive, dwg, dgn, pfa, pfb, afm, tfm, xcf, cpx, dwf, swf, swc, abw, bom, lit, svg, rdf, x3d, oeb, dtb, docx, xlsx, pptx, sxc, sxd, sxg, sxi, sxm, sxw, odc, odf, odg, oth, odi, odm, odp, ods, cin, exr
    // ??? we do not distinguish between different wm types, returning wmv for any of wmv, wma, or asf
    // ??? we do not distinguish between ordinary documents and template versions (often there is no difference in file contents)
    // ??? the distinctions between docx, xlsx, and pptx may not be entirely reliable
    if (architectures) *architectures = NULL;
    if (infodict) *infodict = NULL;
    if (hasObjc) *hasObjc = false;
    if (objcVersion) *objcVersion = 0;
    if (objcFlags) *objcFlags = 0;
    if (url) {
        Boolean gotPath = FALSE;
        char path[CFMaxPathSize];
        gotPath = CFURLGetFileSystemRepresentation(url, true, (uint8_t *)path, CFMaxPathSize);
        struct statinfo statBuf;
        if (gotPath && stat(path, &statBuf) == 0 && (statBuf.st_mode & S_IFMT) == S_IFREG && (fd = open(path, O_RDONLY | CF_OPENFLGS, 0777)) >= 0) {
            length = read(fd, buffer, MAGIC_BYTES_TO_READ);
            fileLength = statBuf.st_size;
            bytes = buffer;
            isFile = true;
        }
    }
    if (!isFile && data) {
        length = CFDataGetLength(data);
        fileLength = (off_t)length;
        bytes = CFDataGetBytePtr(data);
        if (length == 0) ext = "txt";
    }
    if (bytes) {
        if (length >= 4) {
            UInt32 magic = CFSwapInt32HostToBig(*((UInt32 *)bytes));
            for (i = 0; !ext && i < NUM_EXTENSIONS; i++) {
                if (__CFBundleMagicNumbersArray[i] == magic) ext = __CFBundleExtensionsArray + i * EXTENSION_LENGTH;
            }
            if (ext) {
                if (0xcafebabe == magic && 8 <= length && 0 != *((UInt16 *)(bytes + 4))) ext = "class";
#if defined(BINARY_SUPPORT_DYLD)
                else if ((int)sizeof(struct mach_header_64) <= length) mt = _CFBundleGrokMachType(fd, bytes, length, extension ? &isX11 : NULL, architectures, infodict, hasObjc, objcVersion, objcFlags);
                
                if (MH_OBJECT == mt) ext = "o";
                else if (MH_EXECUTE == mt) ext = isX11 ? "x11app" : "tool";
                else if (PEF_FILETYPE == mt) ext = "pef";
                else if (MH_CORE == mt) ext = "core";
                else if (MH_DYLIB == mt) ext = "dylib";
                else if (MH_BUNDLE == mt) ext = "bundle";
#endif /* BINARY_SUPPORT_DYLD */
                else if (0x7b5c7274 == magic && (6 > length || 'f' != bytes[4])) ext = NULL;
                else if (0x25504446 == magic && (6 > length || '-' != bytes[4])) ext = NULL;
                else if (0x00010000 == magic && (6 > length || 0 != bytes[4])) ext = NULL;
                else if (0x47494638 == magic && (6 > length || (0x3761 != CFSwapInt16HostToBig(*((UInt16 *)(bytes + 4))) && 0x3961 != CFSwapInt16HostToBig(*((UInt16 *)(bytes + 4))))))  ext = NULL;
                else if (0x0000000c == magic && (6 > length || 0x6a50 != CFSwapInt16HostToBig(*((UInt16 *)(bytes + 4))))) ext = NULL;
                else if (0x2356524d == magic && (6 > length || 0x4c20 != CFSwapInt16HostToBig(*((UInt16 *)(bytes + 4))))) ext = NULL;
                else if (0x28445746 == magic && (6 > length || 0x2056 != CFSwapInt16HostToBig(*((UInt16 *)(bytes + 4))))) ext = NULL;
                else if (0x30373037 == magic && (6 > length || 0x30 != bytes[4] || !isdigit(bytes[5]))) ext = NULL;
                else if (0x41433130 == magic && (6 > length || 0x31 != bytes[4] || !isdigit(bytes[5]))) ext = NULL;
                else if (0x89504e47 == magic && (8 > length || 0x0d0a1a0a != CFSwapInt32HostToBig(*((UInt32 *)(bytes + 4))))) ext = NULL;
                else if (0x53747566 == magic && (8 > length || 0x66497420 != CFSwapInt32HostToBig(*((UInt32 *)(bytes + 4))))) ext = NULL;
                else if (0x3026b275 == magic && (8 > length || 0x8e66cf11 != CFSwapInt32HostToBig(*((UInt32 *)(bytes + 4))))) ext = NULL;
                else if (0x67696d70 == magic && (8 > length || 0x20786366 != CFSwapInt32HostToBig(*((UInt32 *)(bytes + 4))))) ext = NULL;
                else if (0x424f4d53 == magic && (8 > length || 0x746f7265 != CFSwapInt32HostToBig(*((UInt32 *)(bytes + 4))))) ext = NULL;
                else if (0x49544f4c == magic && (8 > length || 0x49544c53 != CFSwapInt32HostToBig(*((UInt32 *)(bytes + 4))))) ext = NULL;
                else if (0x72746664 == magic && (8 > length || 0x00000000 != CFSwapInt32HostToBig(*((UInt32 *)(bytes + 4))))) ext = NULL;
                else if (0x3d796265 == magic && (12 > length || 0x67696e20 != CFSwapInt32HostToBig(*((UInt32 *)(bytes + 4))) || (0x6c696e65 != CFSwapInt32HostToBig(*((UInt32 *)(bytes + 8))) && 0x70617274 != CFSwapInt32HostToBig(*((UInt32 *)(bytes + 8)))))) ext = NULL;
                else if (0x63616666 == magic && (12 > length || 0 != bytes[4] || 0x64657363 != CFSwapInt32HostToBig(*((UInt32 *)(bytes + 8))))) ext = NULL;
                else if (0x504b0304 == magic) ext = _CFBundleGrokFileTypeForZipFile(fd, bytes, length, fileLength);
                else if (0x25215053 == magic) {
                    if (11 <= length && 0 == ustrncmp(bytes + 4, "-Adobe-", 7)) ext = "ps";
                    else if (14 <= length && 0 == ustrncmp(bytes + 4, "-AdobeFont", 10)) ext = "pfa";
                    else ext = NULL;
                } else if (0x464f524d == magic) {
                    // IFF
                    ext = NULL;
                    if (12 <= length) {
                        UInt32 iffMagic = CFSwapInt32HostToBig(*((UInt32 *)(bytes + 8)));
                        if (0x41494646 == iffMagic) ext = "aiff";
                        else if (0x414946 == iffMagic) ext = "aifc";
                    }
                } else if (0x52494646 == magic) {
                    // RIFF
                    ext = NULL;
                    if (12 <= length) {
                        UInt32 riffMagic = CFSwapInt32HostToBig(*((UInt32 *)(bytes + 8)));
                        if (0x57415645 == riffMagic) ext = "wav";
                        else if (0x41564920 == riffMagic) ext = "avi";
                    }
                } else if (0xd0cf11e0 == magic) {
                    // OLE
                    if (52 <= length) ext = _CFBundleGrokFileTypeForOLEFile(fd, bytes, length, 512 * (1 + CFSwapInt32HostToLittle(*((UInt32 *)(bytes + 48)))));
                } else if (0x62656769 == magic) {
                    // uu
                    ext = NULL;
                    if (76 <= length && 'n' == bytes[4] && ' ' == bytes[5] && isdigit(bytes[6]) && isdigit(bytes[7]) && isdigit(bytes[8]) && ' ' == bytes[9]) {
                        CFIndex endOfLine = 0;
                        for (i = 10; 0 == endOfLine && i < length; i++) if ('\n' == bytes[i]) endOfLine = i;
                        if (10 <= endOfLine && endOfLine + 62 < length && 'M' == bytes[endOfLine + 1] && '\n' == bytes[endOfLine + 62]) {
                            ext = "uu";
                            for (i = endOfLine + 1; ext && i < endOfLine + 62; i++) if (!isprint(bytes[i])) ext = NULL;
                        }
                    }
                }
            }
            if (extension && !ext) {
                UInt16 shortMagic = CFSwapInt16HostToBig(*((UInt16 *)bytes));
                if (5 <= length && 0 == bytes[3] && 0 == bytes[4] && ((1 == bytes[1] && 1 == (0xf7 & bytes[2])) || (0 == bytes[1] && (2 == (0xf7 & bytes[2]) || (3 == (0xf7 & bytes[2])))))) ext = "tga";
                else if (8 <= length && (0x6d6f6f76 == CFSwapInt32HostToBig(*((UInt32 *)(bytes + 4))) || 0x6d646174 == CFSwapInt32HostToBig(*((UInt32 *)(bytes + 4))) || 0x77696465 == CFSwapInt32HostToBig(*((UInt32 *)(bytes + 4))))) ext = "mov";
                else if (8 <= length && (0x69647363 == CFSwapInt32HostToBig(*((UInt32 *)(bytes + 4))) || 0x69646174 == CFSwapInt32HostToBig(*((UInt32 *)(bytes + 4))))) ext = "qtif";
                else if (8 <= length && 0x424f424f == CFSwapInt32HostToBig(*((UInt32 *)(bytes + 4)))) ext = "cwk";
                else if (8 <= length && 0x62706c69 == magic && 0x7374 == CFSwapInt16HostToBig(*((UInt16 *)(bytes + 4))) && isdigit(bytes[6]) && isdigit(bytes[7])) {
                    for (i = 8; !ext && i < 128 && i + 16 <= length; i++) {
                        if (0 == ustrncmp(bytes + i, "WebMainResource", 15)) ext = "webarchive";
                    }
                    if (!ext) ext = "plist";
                } else if (0 == shortMagic && 12 <= length && 0x66747970 == CFSwapInt32HostToBig(*((UInt32 *)(bytes + 4)))) {
                    // ??? may want more ftyp values
                    UInt32 ftyp = CFSwapInt32HostToBig(*((UInt32 *)(bytes + 8)));
                    if (0x6d703431 == ftyp || 0x6d703432 == ftyp || 0x69736f6d == ftyp || 0x69736f32 == ftyp) ext = "mp4";
                    else if (0x4d344120 == ftyp) ext = "m4a";
                    else if (0x4d344220 == ftyp) ext = "m4b";
                    else if (0x4d345020 == ftyp) ext = "m4p";
                    else if (0x4d345620 == ftyp || 0x4d345648 == ftyp || 0x4d345650 == ftyp) ext = "m4v";
                    else if (0x3367 == (ftyp >> 16)) {
                        UInt16 remainder = (ftyp & 0xffff);
                        if (0x6536 == remainder || 0x6537 == remainder || 0x6736 == remainder || 0x7034 == remainder || 0x7035 == remainder || 0x7036 == remainder || 0x7236 == remainder || 0x7336 == remainder || 0x7337 == remainder) ext = "3gp";
                        else if (0x3261 == remainder) ext = "3g2";
                    }
                } else if (0x424d == shortMagic && 18 <= length) {
                    UInt32 btyp = CFSwapInt32HostToLittle(*((UInt32 *)(bytes + 14)));
                    if (40 == btyp || btyp == 12 || btyp == 64 || btyp == 108 || btyp == 124) ext = "bmp";
                } else if (20 <= length && 0 == ustrncmp(bytes + 6, "%!PS-AdobeFont", 14)) ext = "pfb";
                else if (40 <= length && 0x42696e48 == CFSwapInt32HostToBig(*((UInt32 *)(bytes + 34))) && 0x6578 == CFSwapInt16HostToBig(*((UInt16 *)(bytes + 38)))) ext = "hqx";
                else if (128 <= length && 0x6d42494e == CFSwapInt32HostToBig(*((UInt32 *)(bytes + 102)))) ext = "bin";
                else if (128 <= length && 0 == bytes[0] && 0 < bytes[1] && bytes[1] < 64 && 0 == bytes[74] && 0 == bytes[82] && 0 == (fileLength % 128)) {
                    UInt32 df = CFSwapInt32HostToBig(*((UInt32 *)(bytes + 83))), rf = CFSwapInt32HostToBig(*((UInt32 *)(bytes + 87))), blocks = 1 + (df + 127) / 128 + (rf + 127) / 128;
                    if (df < 0x00800000 && rf < 0x00800000 && 1 < blocks && (off_t)(128 * blocks) == fileLength) ext = "bin";
                } else if (265 <= length && 0x75737461 == CFSwapInt32HostToBig(*((UInt32 *)(bytes + 257))) && (0x72202000 == CFSwapInt32HostToBig(*((UInt32 *)(bytes + 261))) || 0x7200 == CFSwapInt16HostToBig(*((UInt16 *)(bytes + 261))))) ext = "tar";
                else if (0xfeff == shortMagic || 0xfffe == shortMagic) {
                    ext = "txt";
                    if (12 <= length && ((0x3cfeff == *((UInt32 *)bytes) && 0x740068 == *((UInt32 *)(bytes + 4)) && 0x6c006d == *((UInt32 *)(bytes + 8))) || (0xfffe3c00 == *((UInt32 *)bytes) && 0x68007400 == *((UInt32 *)(bytes + 4)) && 0x6d006c00 == *((UInt32 *)(bytes + 8))))) ext = "html";
                } else if (0x1f9d == shortMagic) ext = "Z";
                else if (0x1f8b == shortMagic) ext = "gz";
                else if (0x71c7 == shortMagic || 0xc771 == shortMagic) ext = "cpio";
                else if (0xf702 == shortMagic) ext = "dvi";
                else if (0x01da == shortMagic && (0 == bytes[2] || 1 == bytes[2]) && (0 < bytes[3] && 16 > bytes[3])) ext = "sgi";
                else if (0x2321 == shortMagic) {
                    CFIndex endOfLine = 0, lastSlash = 0;
                    for (i = 2; 0 == endOfLine && i < length; i++) if ('\n' == bytes[i]) endOfLine = i;
                    if (endOfLine > 3) {
                        for (i = endOfLine - 1; 0 == lastSlash && i > 1; i--) if ('/' == bytes[i]) lastSlash = i;
                        if (lastSlash > 0) {
                            if (0 == ustrncmp(bytes + lastSlash + 1, "perl", 4)) ext = "pl";
                            else if (0 == ustrncmp(bytes + lastSlash + 1, "python", 6)) ext = "py";
                            else if (0 == ustrncmp(bytes + lastSlash + 1, "ruby", 4)) ext = "rb";
                            else ext = "sh";
                        }
                    } 
                } else if (0xffd8 == shortMagic && 0xff == bytes[2]) ext = "jpeg";
                else if (0x4657 == shortMagic && 0x53 == bytes[2]) ext = "swf";
                else if (0x4357 == shortMagic && 0x53 == bytes[2]) ext = "swc";
                else if (0x4944 == shortMagic && '3' == bytes[2] && 0x20 > bytes[3]) ext = "mp3";
                else if (0x425a == shortMagic && isdigit(bytes[2]) && isdigit(bytes[3])) ext = "bz";
                else if (0x425a == shortMagic && 'h' == bytes[2] && isdigit(bytes[3]) && 8 <= length && (0x31415926 == CFSwapInt32HostToBig(*((UInt32 *)(bytes + 4))) || 0x17724538 == CFSwapInt32HostToBig(*((UInt32 *)(bytes + 4))))) ext = "bz2";
                else if (0x0011 == CFSwapInt16HostToBig(*((UInt16 *)(bytes + 2))) || 0x0012 == CFSwapInt16HostToBig(*((UInt16 *)(bytes + 2)))) ext = "tfm";
            }
        }
        if (extension && !ext) {
            //??? what about MacOSRoman?
            if (0xef == bytes[0] && 0xbb == bytes[1] && 0xbf == bytes[2]) {   // UTF-8 BOM
                hasBOM = true;
                isZero = false;
            }
            for (i = (hasBOM ? 3 : 0); (isPlain || isZero) && !ext && i < length && i < 512; i++) {
                char c = bytes[i];
                if (isPlain && '<' == c && i + 14 <= length && 0 == ustrncasecmp(bytes + i + 1, "!doctype html", 13)) ext = "html";
                if (isSpace && '<' == c && i + 14 <= length) {
                    if (0 == ustrncasecmp(bytes + i + 1, "!doctype html", 13) || 0 == ustrncasecmp(bytes + i + 1, "head", 4) || 0 == ustrncasecmp(bytes + i + 1, "title", 5) || 0 == ustrncasecmp(bytes + i + 1, "script", 6) || 0 == ustrncasecmp(bytes + i + 1, "html", 4)) {
                        ext = "html";
                    } else if (0 == ustrncasecmp(bytes + i + 1, "?xml", 4)) {
                        for (i += 4; !ext && i < 128 && i + 20 <= length; i++) {
                            if ('<' == bytes[i]) {
                                if (0 == ustrncasecmp(bytes + i + 1, "abiword", 7)) ext = "abw";
                                else if (0 == ustrncasecmp(bytes + i + 1, "!doctype svg", 12)) ext = "svg";
                                else if (0 == ustrncasecmp(bytes + i + 1, "!doctype rdf", 12)) ext = "rdf";
                                else if (0 == ustrncasecmp(bytes + i + 1, "!doctype x3d", 12)) ext = "x3d";
                                else if (0 == ustrncasecmp(bytes + i + 1, "!doctype html", 13)) ext = "html";
                                else if (0 == ustrncasecmp(bytes + i + 1, "!doctype posingfont", 19)) ext = "sfont";
                                else if (0 == ustrncasecmp(bytes + i + 1, "!doctype plist", 14)) {
                                    for (i += 14; !ext && i < 256 && i + 16 <= length; i++) {
                                        if (0 == ustrncmp(bytes + i, "WebMainResource", 15)) ext = "webarchive";
                                    }
                                    if (!ext) ext = "plist";
                                }
                            }
                        }
                        if (!ext) ext = "xml";
                    }
                }
                if (0 != c) isZero = false;
                if (isZero || 0x7f <= c || (0x20 > c && !isspace(c))) isPlain = false;
                if (isZero || !isspace(c)) isSpace = false;
            }
            if (!ext) {
                if (isPlain) {
                    if (16 <= length && 0 == ustrncmp(bytes, "StartFontMetrics", 16)) ext = "afm";
                    else ext = "txt";
                } else if (isZero && length >= MAGIC_BYTES_TO_READ && fileLength >= 526) {
                    if (isFile) {
                        if (lseek(fd, 512, SEEK_SET) == 512 && read(fd, buffer, MAGIC_BYTES_TO_READ) >= 14) {
                            if (0x001102ff == CFSwapInt32HostToBig(*((UInt32 *)(buffer + 10)))) ext = "pict";
                        }
                    } else {
                        if (526 <= length && 0x001102ff == CFSwapInt32HostToBig(*((UInt32 *)(bytes + 522)))) ext = "pict";
                    }
                }
            }
        }
        if (extension && (!ext || 0 == strcmp(ext, "bz2")) && length >= MAGIC_BYTES_TO_READ && fileLength >= DMG_BYTES_TO_READ) {
            if (isFile) {
                if (lseek(fd, fileLength - DMG_BYTES_TO_READ, SEEK_SET) == fileLength - DMG_BYTES_TO_READ && read(fd, buffer, DMG_BYTES_TO_READ) >= DMG_BYTES_TO_READ) {
                    if (0x6b6f6c79 == CFSwapInt32HostToBig(*((UInt32 *)buffer)) || (0x63647361 == CFSwapInt32HostToBig(*((UInt32 *)(buffer + DMG_BYTES_TO_READ - 8))) && 0x656e6372 == CFSwapInt32HostToBig(*((UInt32 *)(buffer + DMG_BYTES_TO_READ - 4))))) ext = "dmg";
                }
            } else {
                if (DMG_BYTES_TO_READ <= length && (0x6b6f6c79 == CFSwapInt32HostToBig(*((UInt32 *)(bytes + length - DMG_BYTES_TO_READ))) || (0x63647361 == CFSwapInt32HostToBig(*((UInt32 *)(bytes + length - 8))) && 0x656e6372 == CFSwapInt32HostToBig(*((UInt32 *)(bytes + length - 4)))))) ext = "dmg";
            }
        }
    }
    if (extension) *extension = ext ? CFStringCreateWithCStringNoCopy(kCFAllocatorSystemDefault, ext, kCFStringEncodingUTF8, kCFAllocatorNull) : NULL;
    if (machtype) *machtype = mt;
    if (fd >= 0) close(fd);
    return (ext ? true : false);
}

CFStringRef _CFBundleCopyFileTypeForFileURL(CFURLRef url) {
    CFStringRef extension = NULL;
    (void)_CFBundleGrokFileType(url, NULL, &extension, NULL, NULL, NULL, NULL, NULL, NULL);
    return extension;
}

CFStringRef _CFBundleCopyFileTypeForFileData(CFDataRef data) {
    CFStringRef extension = NULL;
    (void)_CFBundleGrokFileType(NULL, data, &extension, NULL, NULL, NULL, NULL, NULL, NULL);
    return extension;
}

CF_PRIVATE CFDictionaryRef _CFBundleCopyInfoDictionaryInExecutable(CFURLRef url) {
    CFDictionaryRef result = NULL;
    (void)_CFBundleGrokFileType(url, NULL, NULL, NULL, NULL, &result, NULL, NULL, NULL);
    return result;
}

CF_PRIVATE CFArrayRef _CFBundleCopyArchitecturesForExecutable(CFURLRef url) {
    CFArrayRef result = NULL;
    (void)_CFBundleGrokFileType(url, NULL, NULL, NULL, &result, NULL, NULL, NULL, NULL);
    return result;
}

#if DEPLOYMENT_TARGET_MACOSX || DEPLOYMENT_TARGET_EMBEDDED
static Boolean _CFBundleGetObjCImageInfoForExecutable(CFURLRef url, uint32_t *objcVersion, uint32_t *objcFlags) {
    Boolean retval = false;
    (void)_CFBundleGrokFileType(url, NULL, NULL, NULL, NULL, NULL, &retval, objcVersion, objcFlags);
    return retval;
}

CF_PRIVATE Boolean _CFBundleGetObjCImageInfo(CFBundleRef bundle, uint32_t *objcVersion, uint32_t *objcFlags) {
    Boolean retval = false;
    uint32_t localVersion = 0, localFlags = 0;
    CFURLRef executableURL = CFBundleCopyExecutableURL(bundle);
    if (executableURL) {
        retval = _CFBundleGetObjCImageInfoForExecutable(executableURL, &localVersion, &localFlags);
        CFRelease(executableURL);
    }
    if (objcVersion) *objcVersion = localVersion;
    if (objcFlags) *objcFlags = localFlags;
    return retval;
}
#endif

#if defined(BINARY_SUPPORT_DYLD)

CF_PRIVATE __CFPBinaryType _CFBundleGrokBinaryType(CFURLRef executableURL) {
    // Attempt to grok the type of the binary by looking for DYLD magic numbers.  If one of the DYLD magic numbers is found, find out what type of Mach-o file it is.  Otherwise, look for the PEF magic numbers to see if it is CFM.
    __CFPBinaryType result = executableURL ? __CFBundleUnreadableBinary : __CFBundleNoBinary;
    UInt32 machtype = UNKNOWN_FILETYPE;
    if (_CFBundleGrokFileType(executableURL, NULL, NULL, &machtype, NULL, NULL, NULL, NULL, NULL)) {
        switch (machtype) {
            case MH_EXECUTE:
                result = __CFBundleDYLDExecutableBinary;
                break;
            case MH_BUNDLE:
                result = __CFBundleDYLDBundleBinary;
                break;
            case MH_DYLIB:
                result = __CFBundleDYLDFrameworkBinary;
                break;
            case PEF_FILETYPE:
                result = __CFBundleCFMBinary;
                break;
        }
    }
    return result;
}

#endif /* BINARY_SUPPORT_DYLD */

