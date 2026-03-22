/**
 * plktool.h
 * Author: Zoe Knox      Created: 2026-03-13
 *
 * Copyright (C) 2026 ravynOS Project. All rights reserved.
 * Portions Copyright (c) 2008, 2012 Apple Inc.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code as
 * defined in and that are subject to the Apple Public Source License Version 2.0
 * (the 'License').  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS
 * OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.  Please see the License for the
 * specific language governing rights and limitations under the License.
 */
/*                            !! NOTICE !!
 * The Original Code from Apple has been substantially changed to remove
 * functionality not needed for our specific use case. It has also been modified
 * to compile on Linux.
 */

#ifndef _PLKTOOL_H
#define _PLKTOOL_H

/* We need this for OSKextPrivate.h */
#define __unused __attribute__((unused))

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <strings.h>
#include <stdio.h>
#include <libgen.h>
#include <byteswap.h>
#include <sys/mman.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>
#include <CoreFoundation/CFBundlePriv.h>
#include <OSKext.h>
#include <OSKextPrivate.h>
#include <IOKit/IOCFSerialize.h>
#include <libkern/kxld.h>
#include <libkern/kxld_types.h>
#include <libkern/prelink.h>
#include <mach-o/loader.h>
#include <mach-o/swap.h>

#define KEXT_MIN_ALIGN      6 /* 1 << 6 = 64 */

#define SEG_TEXT_EXEC       "__TEXT_EXEC"
#define SEG_LLVM_COV        "__LLVM_COV"

#define VERS_MAJOR_MULT  (1000000000000)
#define VERS_MINOR_MULT      (100000000)
#define VERS_REVISION_MULT       (10000)
#define VERS_STAGE_MULT           (1000)

#define SAFE_RELEASE(x) if(x) CFRelease(x)

#define __kStringUnknown    "(unknown)"

typedef int64_t OSKextVersion;

struct _uuid_stuff
{
    unsigned int uuid_size;
    char        *uuid;
};

typedef enum
{
    macho_seek_result_error = -1,
    macho_seek_result_found = 0,
    macho_seek_result_found_no_value = 1,
    macho_seek_result_not_found = 2,
    macho_seek_result_stop = 3,
} macho_seek_result;

typedef macho_seek_result (*macho_lc_callback)(
    struct load_command *load_command,
    const void          *file_end,
    uint8_t              swap,
    void                *user_data);

enum enumSegIdx
{
    SEG_IDX_TEXT,
    SEG_IDX_TEXT_EXEC,
    SEG_IDX_DATA,
    SEG_IDX_DATA_CONST,
    SEG_IDX_LLVM_COV,
    SEG_IDX_LINKEDIT,
    SEG_IDX_COUNT,
};

typedef struct prelinked_kernel_header
{
    uint32_t signature;
    uint32_t compressType;
    uint32_t adler32;
    uint32_t uncompressedSize;
    uint32_t compressedSize;
    uint32_t prelinkVersion;
    uint32_t reserved[10];
    char     platformName[64]; // unused
    char     rootPath[256];    // unused
    char     data[0];
} PrelinkedKernelHeader;

typedef struct SegInfo
{
    uint64_t vmaddr;  /* vmaddr of the segment */
    uint64_t vmsize;  /* vmsize of the segment */
    uint64_t fileoff; /* file offset of the segment */
} SegInfo;

// fileoff is the offset into the kernelcache file
typedef struct plkSegInfo
{
    SegInfo     plkSegInfo;
    const char *plk_seg_name;
    uint64_t    plk_next_kext_vmaddr; /* vmaddr of next kext */
} plkSegInfo;

typedef struct plkInfo
{
    CFDataRef        kernelImage;
    CFMutableDataRef kernelCacheImage; /* kernelcache file */
    CFMutableSetRef  kaslrOffsets;     /* offsets into kernel cache that need to be slid */
    SegInfo          kernel_TEXT;      /* kernel __TEXT */
    plkSegInfo       plk_TEXT;         /* __PRELINK_TEXT */
    plkSegInfo       plk_TEXT_EXEC;    /* __PLK_TEXT_EXEC */
    plkSegInfo       plk_DATA;         /* __PRELINK_DATA */
    plkSegInfo       plk_DATA_CONST;   /* __PLK_DATA_CONST */
    plkSegInfo       plk_LINKEDIT;     /* __PLK_LINKEDIT */
    plkSegInfo       plk_LLVM_COV;     /* __PLK_LLVM_COV */
    SegInfo          plk_INFO;         /* __PRELINK_INFO */
} plkInfo;

typedef struct
{
    size_t length;
} __OSKextMmapBufferInfo;

typedef struct __OSKextDiagnostics
{
    CFMutableDictionaryRef validationFailures;
    CFMutableDictionaryRef authenticationFailures;
    CFMutableDictionaryRef dependencyFailures; // whether direct or indirect!
    CFMutableDictionaryRef warnings;
    CFMutableDictionaryRef bootLevel;
} __OSKextDiagnostics;

typedef struct __OSKextLoadInfo
{
    /* Used whenever a dependency graph is needed (generating an mkext,
    * prelinked kernel, or linking/loading).
    */
    CFMutableArrayRef dependencies; // may have some missing

    /* These are used when checking the kernel for loaded kexts,
    * or when loading/generating symbols from user space.
    */
    CFDictionaryRef   kernelLoadInfo; // for lazy eval, cleared when we check
    uint32_t          loadTag;
    splitKextLinkInfo linkInfo;      // used for kcgen and kxld interactions
    uint64_t          sourceAddress; // For prelinking: where it starts in memory (x86_64)

    /* These only exist while loading from user space.
    */
    CFDataRef    executable;
    CFDataRef    linkedExecutable;
    CFDataRef    prelinkedExecutable;
    kmod_info_t *kmod_info;
    uint64_t     kmodInfoAddress;

    struct
    {
        unsigned int hasRawKernelDependency : 1;
        unsigned int hasKernelDependency : 1;
        unsigned int hasKPIDependency : 1;
        unsigned int hasPrivateKPIDependency : 1;

        unsigned int hasAllDependencies : 1;
        unsigned int dependenciesValid : 1;
        unsigned int dependenciesAuthentic : 1;

        unsigned int isLoaded : 1;
        unsigned int isStarted : 1;
        unsigned int otherCFBundleVersionIsLoaded : 1;
        unsigned int otherUUIDIsLoaded : 1; // otherVersion is also set if this is
    } flags;
} __OSKextLoadInfo;

typedef struct __OSKext
{
    /* base CFType information. */
    CFRuntimeBase cfBase;

    /* Read/retained at creation time. */
    CFURLRef    bundleURL;
    CFStringRef bundleID;
    CFURLRef    executableURL;

    /* Read by __OSKextProcessInfoDictionary(). */
    OSKextVersion version;
    OSKextVersion compatibleVersion;

    /* May be flushed, may need to reload from disk.
    */
    CFMutableDictionaryRef infoDictionary; // read with IOCFUnserialize()

    /* Allocated and maintained as necessary. */
    __OSKextDiagnostics *diagnostics;
    __OSKextLoadInfo    *loadInfo;
    //  __OSKextMkextInfo   * mkextInfo;

    struct
    {
        unsigned int isPluginChecked : 1;
        unsigned int isPlugin : 1;

        unsigned int isFromIdentifierCache : 1; // must __OSKextRealize on access
        unsigned int isFromMkext : 1;           // i.e. *not* to be updated from bundleURL
    } staticFlags;

    struct
    {
        /* Set by __OSKextProcessInfoDictionary() */
        unsigned int isKernelComponent : 1;
        unsigned int isInterface : 1;
        unsigned int declaresKernelExecutable : 1;
        unsigned int declaresUserExecutable : 1;
        unsigned int loggingEnabled : 1;
        unsigned int plistHasEnableLoggingSet : 1;
        unsigned int plistHasIOKitDebugFlags : 1;
        unsigned int isLoadableInSafeBoot : 1;

        /* Set as determined or on demand. */
        unsigned int rootless_trusted : 1; // protected by installer:
                                           // don't flush authentication bits

        unsigned int validated : 1; // all possible checks done
        unsigned int invalid : 1;   // at least 1 failure, or fully validated
        unsigned int valid : 1;     // all possible checks done & passed

        unsigned int authenticated : 1; // all possible checks done
        unsigned int inauthentic : 1;   // at least 1 failure, or all ok
        unsigned int authentic : 1;     // should we ever cache this?

        unsigned int hasIOKitDebugProperty : 1;
        unsigned int warnForMismatchedKmodInfo : 1;
        unsigned int isSigned : 1;
    } flags;

} __OSKext, *__OSKextRef;

/* CF Type stuff */
extern CFTypeID             __kOSKextTypeID;
extern const CFRuntimeClass __OSKextClass;

/* kext.c */
CFTypeID
OSKextGetTypeID(void);

static char *
segIdxToName(enum enumSegIdx idx);

boolean_t
initializeAllKexts(void);

static void
__OSKextReleaseContents(CFTypeRef cfObject);

boolean_t
__os_warn_unused(const boolean_t x);

void
__OSKextLoggingCallback(KXLDLogSubsystem subsystem,
                        KXLDLogLevel     level,
                        const char      *format,
                        va_list          argList,
                        void            *user_data);
kxld_addr_t
__OSKextLinkAddressCallback(u_long             size,
                            KXLDAllocateFlags *flags __unused,
                            void              *user_data);

macho_seek_result
__OSKextUUIDCallback(struct load_command *load_command,
                     const void          *file_end,
                     uint8_t swap         __unused,
                     void                *user_data);

OSKextRef
__OSKextAlloc(CFAllocatorRef      allocator,
              CFAllocatorContext *context __unused);

Boolean
__OSKextCreateLoadInfo(OSKextRef aKext);

static Boolean
__OSKextIsSplitKextMacho64(struct mach_header_64 *kextHeader);

Boolean
__OSKextGetFileSystemPath(OSKextRef aKext,
                          CFURLRef  anURL,
                          Boolean   resolveToBase,
                          char     *pathBuffer);

void
__OSKextDeallocateMmapBuffer(void *pointer, void *vInfo);

static uint32_t
__OSKextGetSegMaxAlignment(struct segment_command_64 *seg_cmd);

static boolean_t
__OSKextGetSegmentInfo(const UInt8 *imagePtr,
                       const char  *segname,
                       uint64_t    *vmaddrOut,
                       uint64_t    *vmsizeOut,
                       uint64_t    *fileoffOut,
                       uint64_t    *filesizeOut,
                       uint64_t    *maxAlignOut,
                       boolean_t    truncateSegs);

static uint64_t
__OSKextAlignAddress(uint64_t address, uint32_t align);

boolean_t
__OSKextGetSegmentAddressAndOffsetDataRef(CFDataRef   imageRef,
                                          const char *segname,
                                          uint32_t   *fileOffsetOut,
                                          uint64_t   *loadAddrOut);

boolean_t
__OSKextGetSegmentFileAndVMSize(const UInt8 *imagePtr,
                                const char  *segname,
                                uint64_t    *fileSizeOut,
                                uint64_t    *VMSizeOut);

boolean_t
__OSKextGetSegmentFileAndVMSizeDataRef(CFDataRef   imageRef,
                                       const char *segname,
                                       uint64_t   *fileSizeOut,
                                       uint64_t   *VMSizeOut);

static boolean_t
setKextVMAddr(OSKextRef aKext,
              enum enumSegIdx idx,
              uint64_t vmaddr);

static boolean_t
__OSKextValidatePLKInfo(OSKextRef              aKext,
                        struct mach_header_64 *kextHeader,
                        plkInfo               *plkInfo);

Boolean
__OSKextReadExecutable(OSKextRef aKext);

static Boolean
__OSKextPerformLink(OSKextRef    aKext,
                    CFDataRef    kernelImage,
                    uint64_t     kernelLoadAddress,
                    Boolean      stripSymbolsFlag,
                    KXLDContext *kxldContext);

CFMutableArrayRef
OSKextCopyLoadListForKexts(CFArrayRef kexts, Boolean needAll);

CFArrayRef
__OSKextPrelinkKexts(CFArrayRef   kextArray,
                     CFDataRef    kernelImage,
                     uint64_t     loadAddrBase,
                     uint64_t     sourceAddrBase,
                     KXLDContext *kxldContext,
                     u_long      *loadSizeOut,
                     Boolean      needAllFlag,
                     Boolean      skipAuthenticationFlag,
                     Boolean      printDiagnosticsFlag,
                     Boolean      stripSymbolsFlag);

static boolean_t
__OSKextGetSegmentAddressAndOffset(const UInt8 *imagePtr,
                                   const char  *segname,
                                   uint32_t    *fileOffsetOut,
                                   uint64_t    *loadAddrOut);

Boolean
_GetStringProperty(OSKextRef    aKext,
                   CFStringRef  propKey,
                   CFStringRef *valueOut);

Boolean
_GetBooleanProperty(OSKextRef     aKext,
                    CFStringRef   propKey,
                    CFBooleanRef *valueOut);

CFDataRef
__OSKextCreatePrelinkInfoDictionary(plkInfo   *plkInfo,
                                    CFArrayRef loadList,
                                    CFURLRef   volumeRootURL,
                                    Boolean    includeAllPersonalities,
                                    Boolean    isSplitKexts,
                                    CFDataRef  kernelUUID);

CFMutableDictionaryRef
OSKextCopyInfoDictionary(OSKextRef aKext);

Boolean
__OSKextReadInfoDictionary(OSKextRef   aKext,
                           CFBundleRef kextBundle);

OSKextVersion
OSKextParseVersionString(const char *versionString);

Boolean
__OSKextProcessInfoDictionary(OSKextRef   aKext,
                              CFBundleRef kextBundle);

Boolean
__OSKextInitWithPath(OSKextRef   aKext,
                     const char *kextPath);


/* macho.c */
boolean_t
macho_swap_64(u_char *file);

boolean_t
macho_unswap_64(u_char *file);

macho_seek_result
macho_scan_load_commands(const void       *file_start,
                         const void       *file_end,
                         macho_lc_callback lc_callback,
                         void             *user_data);

struct segment_command_64 *
macho_get_segment_by_name_64(struct mach_header_64 *mach_header,
                             const char            *segname);

struct section_64 *
macho_get_section_by_name_64(struct mach_header_64 *mach_header,
                             const char            *segname,
                             const char            *sectname);

void *
macho_find_section_numbered(const void *file_start,
                            const void *file_end,
                            uint8_t     sect_num);

macho_seek_result
macho_find_symbol(const void  *file_start,
                  const void  *file_end,
                  const char  *name,
                  uint8_t     *nlist_type,
                  const void **symbol_address);

/* plktool.c */
char *
createUTF8CStringForCFString(CFStringRef aString);

static boolean_t
SwapHeaders(CFDataRef kernelImage);

static boolean_t
UnswapHeaders(CFDataRef kernelImage);

static uint64_t
getKCPlkSegNextVMAddr(plkInfo *plkInfo, enum enumSegIdx idx);

static boolean_t
setKCPlkSegNextVMAddr(plkInfo *plkInfo, enum enumSegIdx idx, uint64_t x);

static u_long
CopyPrelinkedKexts(CFMutableDataRef prelinkImage,
                   CFArrayRef       loadList,
                   u_long           fileOffsetBase,
                   uint64_t         sourceAddrBase);

static boolean_t
GetLastKernelLoadAddr(CFDataRef kernelImage,
                      uint64_t *lastLoadAddrOut);

CFDataRef
CreatePrelinkedKernel(CFDataRef  kernelImage,
                      CFArrayRef kextArray);


#endif /* _PLKTOOL_H */
