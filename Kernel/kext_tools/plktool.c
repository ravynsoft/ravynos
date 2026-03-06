/**
 * plktool.c
 * Author: Zoe Knox      Created: 2026-03-02
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
 *
 * This file implements just enough of OSKext and kext_tools to make a prelinked
 * kernel image from a Linux host, and is used in the ravynOS build toolchain
 */

#define __unused            __attribute__((unused))
#define __kStringUnknown    "(unknown)"
#define KEXT_MIN_ALIGN      6 /* 1 << 6 = 64 */
#define SEG_TEXT_EXEC       "__TEXT_EXEC"
#define SEG_LLVM_COV        "__LLVM_COV"

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <strings.h>
#include <stdio.h>
#include <libgen.h>
#include <byteswap.h>
#include <sys/mman.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>
#include <OSKext.h>
#include <OSKextPrivate.h>
#include <IOKit/IOCFSerialize.h>
#include <libkern/kxld.h>
#include <libkern/kxld_types.h>
#include <libkern/prelink.h>
#include <mach-o/loader.h>
#include <mach-o/swap.h>

typedef enum {
    macho_seek_result_error          = -1,
    macho_seek_result_found          = 0,
    macho_seek_result_found_no_value = 1,
    macho_seek_result_not_found      = 2,
    macho_seek_result_stop           = 3,
} macho_seek_result;

typedef macho_seek_result (*macho_lc_callback)(
    struct load_command * load_command,
    const void          * file_end,
    uint8_t               swap,
    void                * user_data
);

struct _uuid_stuff {
    unsigned int   uuid_size;
    char         * uuid;
};

enum enumSegIdx {
    SEG_IDX_TEXT,
    SEG_IDX_TEXT_EXEC,
    SEG_IDX_DATA,
    SEG_IDX_DATA_CONST,
    SEG_IDX_LLVM_COV,
    SEG_IDX_LINKEDIT,
    SEG_IDX_COUNT,
};

static char * segIdxToName(enum enumSegIdx idx) {
    switch (idx) {
    case SEG_IDX_TEXT:
        return "__TEXT";
    case SEG_IDX_TEXT_EXEC:
        return "__TEXT_EXEC";
    case SEG_IDX_DATA:
        return "__DATA";
    case SEG_IDX_DATA_CONST:
        return "__DATA_CONST";
    case SEG_IDX_LINKEDIT:
        return "__LINKEDIT";
    case SEG_IDX_LLVM_COV:
        return "__LLVM_COV";
    default:
        return NULL;
    }
}

boolean_t __os_warn_unused(const boolean_t x)
{
        return x;
}

/* -----------------------[ from misc_util.c ]----------------------------- */

char * createUTF8CStringForCFString(CFStringRef aString)
{
    char * result = NULL;
    CFIndex bufferLength = 0;

    if (!aString) {
        goto finish;
    }

    bufferLength = sizeof('\0') +
        CFStringGetMaximumSizeForEncoding(CFStringGetLength(aString),
	    kCFStringEncodingUTF8);

    result = (char *)malloc(bufferLength * sizeof(char));
    if (!result) {
        goto finish;
    }
    if (!CFStringGetCString(aString, result, bufferLength,
        kCFStringEncodingUTF8)) {

        if(result)
            CFRelease(result);
        result = NULL;
    }

finish:
    return result;
}

/* ------------------------[ from macho_util.c ]--------------------------- */

boolean_t macho_swap_64(
    u_char    * file)
{
    boolean_t result = FALSE;
    struct mach_header_64 *hdr = (struct mach_header_64 *) file;
    struct load_command *lc = (struct load_command *) &hdr[1];
    struct segment_command_64 *seg = NULL;
    u_long offset = 0;
    u_int cmd = 0;
    u_int cmdsize = 0;
    u_int i = 0;

    if (!hdr || hdr->magic != MH_CIGAM_64) goto finish;

    swap_mach_header_64(hdr, NXHostByteOrder());

    offset = sizeof(*hdr);
    for (i = 0; i < hdr->ncmds; ++i) {
        lc = (struct load_command *) (file + offset);

        cmd = bswap_32(lc->cmd);
        cmdsize = bswap_32(lc->cmdsize);
        offset += cmdsize;

        if (cmd == LC_SEGMENT_64) {
            seg = (struct segment_command_64 *) lc;
            swap_segment_command_64(seg, NXHostByteOrder());
        } else {
            swap_load_command(lc, NXHostByteOrder());
        }
    }

    result = TRUE;
finish:
    return result;
}

boolean_t macho_unswap_64(
    u_char    * file)
{
    boolean_t result = FALSE;
    enum NXByteOrder order = 0;
    struct mach_header_64 *hdr = (struct mach_header_64 *) file;
    struct load_command *lc = (struct load_command *) &hdr[1];
    struct segment_command_64 *seg = NULL;
    u_long offset = 0;
    u_int i = 0;

    if (NXHostByteOrder() == NX_LittleEndian) {
        order = NX_BigEndian;
    } else {
        order = NX_LittleEndian;
    }

    if (!hdr || hdr->magic != MH_MAGIC_64) goto finish;

    offset = sizeof(*hdr);
    for (i = 0; i < hdr->ncmds; ++i) {
        lc = (struct load_command *) (file + offset);
        offset += lc->cmdsize;

        if (lc->cmd == LC_SEGMENT_64) {
            seg = (struct segment_command_64 *) lc;
            swap_segment_command_64(seg, order);
        } else {
            swap_load_command(lc, order);
        }
    }

    swap_mach_header_64(hdr, order);

    result = TRUE;
finish:
    return result;
}

#define CMDSIZE_MULT_32  (4)
#define CMDSIZE_MULT_64  (8)

macho_seek_result macho_scan_load_commands(
    const void        * file_start,
    const void        * file_end,
    macho_lc_callback   lc_callback,
    void              * user_data)
{
    macho_seek_result       result = macho_seek_result_not_found;
    struct mach_header    * mach_header = (struct mach_header *)file_start;

    uint8_t                 swap = 0;
    uint32_t                cmdsize_mult = CMDSIZE_MULT_32;

    uint32_t                num_cmds;
    uint32_t                sizeofcmds;
    char                  * cmds_end;

    uint32_t                cmd_index;
    struct load_command  * load_commands;
    struct load_command  * seek_lc;

    switch (*(uint32_t *)(file_start)) {
      case MH_MAGIC_64:
        cmdsize_mult = CMDSIZE_MULT_64;
        break;
      case MH_CIGAM_64:
        cmdsize_mult = CMDSIZE_MULT_64;
        swap = 1;
        break;
      default:
        result = macho_seek_result_error;
        goto finish;
        break;
    }

    if (cmdsize_mult == CMDSIZE_MULT_64) {
        load_commands = (struct load_command *)
            (file_start + sizeof(struct mach_header_64));
    }

    if (file_start >= file_end || (((void *)load_commands) > file_end)) {
        result = macho_seek_result_error;
        goto finish;
    }

    num_cmds   = swap
        ? bswap_32(mach_header->ncmds)
        : mach_header->ncmds;
    sizeofcmds = swap
        ? bswap_32(mach_header->sizeofcmds)
        : mach_header->sizeofcmds;
    cmds_end = (char *)load_commands + sizeofcmds;

    if (cmds_end > (char *)file_end) {
        result = macho_seek_result_error;
        goto finish;
    }

    seek_lc = load_commands;

    for (cmd_index = 0; cmd_index < num_cmds; cmd_index++) {
        uint32_t cmd_size;
        char * lc_end;

        cmd_size = swap
            ? bswap_32(seek_lc->cmdsize)
            : seek_lc->cmdsize;
        lc_end = (char *)seek_lc + cmd_size;

        if ((cmd_size % cmdsize_mult != 0) || (lc_end > cmds_end)) {
            result = macho_seek_result_error;
            goto finish;
        }

        result = lc_callback(seek_lc, file_end, swap, user_data);

        switch (result) {
          case macho_seek_result_not_found:
            /* Not found, keep scanning. */
            break;

          case macho_seek_result_stop:
            /* Definitely found that it isn't there. */
            result = macho_seek_result_not_found;
            goto finish;
            break;

          case macho_seek_result_found:
            /* Found it! */
            goto finish;
            break;

          default:
            /* Error, fall through default case. */
            result = macho_seek_result_error;
            goto finish;
            break;
        }

        seek_lc = (struct load_command *)((char *)seek_lc + cmd_size);
    }

finish:
    return result;
}


struct segment_command_64 * macho_get_segment_by_name_64(
    struct mach_header_64      * mach_header,
    const char                 * segname)
{
    struct segment_command_64 *segment = NULL;
    struct load_command *lc = NULL;
    u_char *base = (u_char *) mach_header;
    size_t offset = sizeof(*mach_header);
    u_int i = 0;
    
    if (mach_header->magic != MH_MAGIC_64) goto finish;
    
    for (i = 0; i < mach_header->ncmds; ++i) {
        lc = (struct load_command *) (base + offset);
        
        if (lc->cmd == LC_SEGMENT_64) {
            segment = (struct segment_command_64 *) lc;
            if (!strncmp(segment->segname, segname, sizeof(segment->segname))) {
                break;
            }
            segment = NULL;
        }
        
        offset += lc->cmdsize;
    }
    
finish:    
    return segment;
}

struct section_64 * macho_get_section_by_name_64(
    struct mach_header_64     * mach_header,
    const char                * segname,
    const char                * sectname)
{
    struct segment_command_64 *segment = NULL;
    struct section_64 *section = NULL;
    u_int i = 0;
    
    if (mach_header->magic != MH_MAGIC_64) goto finish;
    
    segment = macho_get_segment_by_name_64(mach_header, segname);
    if (!segment) goto finish;
    
    section = (struct section_64 *) (&segment[1]);
    for (i = 0; i < segment->nsects; ++i, ++section) {
        if (!strncmp(section->sectname, sectname, sizeof(section->sectname))) {
            break;
        }
    }
    
    if (i == segment->nsects) {
        section = NULL;
    }
    
finish:
    return section;
}


/* ------------------------[ from OSKext.c ]------------------------------- */

typedef struct SegInfo {
    uint64_t    vmaddr;     /* vmaddr of the segment */
    uint64_t    vmsize;     /* vmsize of the segment */
    uint64_t    fileoff;    /* file offset of the segment */
} SegInfo;

// fileoff is the offset into the kernelcache file
typedef struct plkSegInfo {
    SegInfo     plkSegInfo;
    const char *plk_seg_name;
    uint64_t    plk_next_kext_vmaddr;   /* vmaddr of next kext */
} plkSegInfo;

typedef struct plkInfo {
    CFDataRef   kernelImage;
    CFMutableDataRef   kernelCacheImage; /* kernelcache file */
    CFMutableSetRef    kaslrOffsets;     /* offsets into kernel cache that need to be slid */
    SegInfo     kernel_TEXT;    /* kernel __TEXT */
    plkSegInfo  plk_TEXT;       /* __PRELINK_TEXT */
    plkSegInfo  plk_TEXT_EXEC;  /* __PLK_TEXT_EXEC */
    plkSegInfo  plk_DATA;       /* __PRELINK_DATA */
    plkSegInfo  plk_DATA_CONST; /* __PLK_DATA_CONST */
    plkSegInfo  plk_LINKEDIT;   /* __PLK_LINKEDIT */
    plkSegInfo  plk_LLVM_COV;   /* __PLK_LLVM_COV */
    SegInfo     plk_INFO;       /* __PRELINK_INFO */
} plkInfo;

typedef struct {
    size_t length;
} __OSKextMmapBufferInfo;

typedef struct __OSKextDiagnostics {
    CFMutableDictionaryRef validationFailures;
    CFMutableDictionaryRef authenticationFailures;
    CFMutableDictionaryRef dependencyFailures; // whether direct or indirect!
    CFMutableDictionaryRef warnings;
    CFMutableDictionaryRef bootLevel;
} __OSKextDiagnostics;

typedef struct __OSKextLoadInfo {
   /* Used whenever a dependency graph is needed (generating an mkext,
    * prelinked kernel, or linking/loading).
    */
    CFMutableArrayRef dependencies;    // may have some missing

   /* These are used when checking the kernel for loaded kexts,
    * or when loading/generating symbols from user space.
    */
    CFDictionaryRef   kernelLoadInfo;   // for lazy eval, cleared when we check
    uint32_t          loadTag;
    splitKextLinkInfo linkInfo;    // used for kcgen and kxld interactions
    uint64_t          sourceAddress;    // For prelinking: where it starts in memory (x86_64)

   /* These only exist while loading from user space.
    */
    CFDataRef         executable;
    CFDataRef         linkedExecutable;
    CFDataRef         prelinkedExecutable;
    kmod_info_t     * kmod_info;
    uint64_t          kmodInfoAddress;
    
    struct {
        unsigned int  hasRawKernelDependency:1;
        unsigned int  hasKernelDependency:1;
        unsigned int  hasKPIDependency:1;
        unsigned int  hasPrivateKPIDependency:1;

        unsigned int  hasAllDependencies:1;
        unsigned int  dependenciesValid:1;
        unsigned int  dependenciesAuthentic:1;

        unsigned int  isLoaded:1;
        unsigned int  isStarted:1;
        unsigned int  otherCFBundleVersionIsLoaded:1;
        unsigned int  otherUUIDIsLoaded:1; // otherVersion is also set if this is
    } flags;
} __OSKextLoadInfo;

typedef struct __OSKext {

   /* base CFType information. */
    CFRuntimeBase         cfBase;

   /* Read/retained at creation time. */
    CFURLRef              bundleURL;
    CFStringRef           bundleID;
    CFURLRef              executableURL;

   /* Read by __OSKextProcessInfoDictionary(). */
    OSKextVersion         version;
    OSKextVersion         compatibleVersion;

   /* May be flushed, may need to reload from disk.
    */
    CFMutableDictionaryRef  infoDictionary;  // read with IOCFUnserialize()

   /* Allocated and maintained as necessary. */
    __OSKextDiagnostics * diagnostics;
    __OSKextLoadInfo    * loadInfo;
    //  __OSKextMkextInfo   * mkextInfo;

    struct {
        unsigned int      isPluginChecked:1;
        unsigned int      isPlugin:1;

        unsigned int      isFromIdentifierCache:1; // must __OSKextRealize on access
        unsigned int      isFromMkext:1;      // i.e. *not* to be updated from bundleURL
    } staticFlags;

    struct {
       /* Set by __OSKextProcessInfoDictionary() */
        unsigned int      isKernelComponent:1;
        unsigned int      isInterface:1;
        unsigned int      declaresKernelExecutable:1;
        unsigned int      declaresUserExecutable:1;
        unsigned int      loggingEnabled:1;
        unsigned int      plistHasEnableLoggingSet:1;
        unsigned int      plistHasIOKitDebugFlags:1;
        unsigned int      isLoadableInSafeBoot:1;

       /* Set as determined or on demand. */
        unsigned int      rootless_trusted:1; // protected by installer:
                                           // don't flush authentication bits

        unsigned int      validated:1;     // all possible checks done
        unsigned int      invalid:1;       // at least 1 failure, or fully validated
        unsigned int      valid:1;         // all possible checks done & passed

        unsigned int      authenticated:1; // all possible checks done
        unsigned int      inauthentic:1;   // at least 1 failure, or all ok
        unsigned int      authentic:1;     // should we ever cache this?

        unsigned int      hasIOKitDebugProperty:1;
        unsigned int      warnForMismatchedKmodInfo:1;
        unsigned int      isSigned:1;
    } flags;

} __OSKext, * __OSKextRef;

static boolean_t SwapHeaders(CFDataRef kernelImage)
{
    u_char *file = (u_char *) CFDataGetBytePtr(kernelImage);
    return macho_swap_64(file);
}

static boolean_t UnswapHeaders(CFDataRef kernelImage)
{
    u_char *file = (u_char *) CFDataGetBytePtr(kernelImage);
    return macho_unswap_64(file);
}

Boolean __OSKextCreateLoadInfo(OSKextRef aKext)
{
    Boolean result = false;

    if (!aKext->loadInfo) {
        aKext->loadInfo = (__OSKextLoadInfo *)
            malloc(sizeof(*(aKext->loadInfo)));
        if (!aKext->loadInfo) {
            perror("malloc");
            goto finish;
        }
        memset(aKext->loadInfo, 0, sizeof(*(aKext->loadInfo)));
    }
    result = true;
finish:
    return result;
}

static Boolean __OSKextIsSplitKextMacho64(struct mach_header_64 *kextHeader)
{
    struct segment_command_64 *seg_cmd = NULL;
    if (!kextHeader)
        return false;

    seg_cmd = macho_get_segment_by_name_64(kextHeader, SEG_TEXT_EXEC);
    if (seg_cmd != NULL) {
        return true;
    }
    return false;
}

Boolean __OSKextGetFileSystemPath(
    OSKextRef aKext,
    CFURLRef  anURL,
    Boolean   resolveToBase,
    char    * pathBuffer)
{
    Boolean  result   = false;
    CFURLRef urlToUse = NULL;  // do not release

    if (aKext) {
        if (aKext->bundleURL) {
            urlToUse = aKext->bundleURL;
        }
    } else {
        urlToUse = anURL;
    }
    if (!urlToUse) {
        goto finish;
    }
    result = CFURLGetFileSystemRepresentation(urlToUse,
        resolveToBase, (UInt8 *)pathBuffer, PATH_MAX);

finish:
    if (!result) {
        fprintf(stderr,"failed to get filesystem path of %s\n", urlToUse);
        memcpy(pathBuffer, __kStringUnknown, sizeof(__kStringUnknown));
    }
    return result;
}

void __OSKextDeallocateMmapBuffer(void * pointer, void * vInfo)
{
    __OSKextMmapBufferInfo * info = (__OSKextMmapBufferInfo *)vInfo;
    free(info);
}

static uint32_t __OSKextGetSegMaxAlignment(
                                        struct segment_command_64 * seg_cmd)
{
    struct section_64 * sect = NULL;
    uint32_t            i = 0;
    uint32_t            result = KEXT_MIN_ALIGN;

    if (seg_cmd->cmd != LC_SEGMENT_64)
        goto finish;

    sect = (struct section_64 *)(&seg_cmd[1]);
    for (i = 0; i < seg_cmd->nsects; ++i, ++sect) {
        if (sect->align > result) {
            result = sect->align;
        }
    }

    if (!strncmp(seg_cmd->segname, SEG_TEXT_EXEC, sizeof(seg_cmd->segname))) {
        result = 12;
    }

//    if (g_max_align_to_4k && result < 12)
//        result = 12;

finish:
    return result;
}

static boolean_t __OSKextGetSegmentInfo(
    const UInt8 *   imagePtr,
    const char *    segname,
    uint64_t *      vmaddrOut,
    uint64_t *      vmsizeOut,
    uint64_t *      fileoffOut,
    uint64_t *      filesizeOut,
    uint64_t *      maxAlignOut,
    boolean_t       truncateSegs)
{
    boolean_t result = false;
    uint64_t max_off = 0;

    /* do the normal thing for LINKEDIT/LLVM_COV regardless of truncateSegs flag */
    if (truncateSegs && (!strcmp(segname, SEG_LINKEDIT) || !strcmp(segname, SEG_LLVM_COV)))
        truncateSegs = false;

    struct mach_header_64 *mach_header = (struct mach_header_64 *) imagePtr;
    struct segment_command_64 *seg = macho_get_segment_by_name_64(mach_header, segname);
    if (!seg) {
        goto finish;
    }
    
    if (maxAlignOut) *maxAlignOut = __OSKextGetSegMaxAlignment(seg);
    
    /* special case for __TEXT segment. max_off is at least size of mach header + load commands */
    if (truncateSegs && !strcmp(segname, "__TEXT")) {
        max_off = sizeof(*mach_header) + mach_header->sizeofcmds;
    }
    
    if (vmaddrOut)
        *vmaddrOut = seg->vmaddr;
    if (vmsizeOut) {
        if (truncateSegs) {
            /* walk the sections for this segment, compute vmsize based on highest used offset
             * instead of trusting what the segment header says.
             */
            struct section_64 *sect = (struct section_64 *) &seg[1];
            uint64_t off, end_off;
            
            for (unsigned int i = 0; i < seg->nsects; ++i, ++sect) {
                off = sect->addr - seg->vmaddr;
                end_off = off + sect->size;
                max_off = (end_off > max_off) ? end_off : max_off;
            }
            
            *vmsizeOut = max_off;
        } else {
            *vmsizeOut = seg->vmsize;
        }
    }
    
    if (fileoffOut)
        *fileoffOut = seg->fileoff;
    if (filesizeOut) {
        if (truncateSegs) {
            /* previous truncateSegs code has to have run */
            assert(vmsizeOut);
            *filesizeOut = max_off;
        } else {
            *filesizeOut = seg->filesize;
        }
    }

    result = true;

  finish:
    return result;
}

static uint64_t __OSKextAlignAddress(uint64_t address, uint32_t align)
{
    uint64_t alignment = (1 << align);
    uint64_t low_bits = 0;
    
    if (align == 0) return address;
    
    low_bits = (address) & (alignment - 1);
    if (low_bits) {
        address += (alignment - low_bits);
    }
    
    return address;
}

static uint64_t getKCPlkSegNextVMAddr(plkInfo *plkInfo, enum enumSegIdx idx) {
    assert(plkInfo);

    switch (idx) {
    case SEG_IDX_TEXT:
        return plkInfo->plk_TEXT.plk_next_kext_vmaddr;
    case SEG_IDX_TEXT_EXEC:
        return plkInfo->plk_TEXT_EXEC.plk_next_kext_vmaddr; 
    case SEG_IDX_DATA:
        return plkInfo->plk_DATA.plk_next_kext_vmaddr;
    case SEG_IDX_DATA_CONST:
        return plkInfo->plk_DATA_CONST.plk_next_kext_vmaddr;
    case SEG_IDX_LINKEDIT:
        return plkInfo->plk_LINKEDIT.plk_next_kext_vmaddr;
    case SEG_IDX_LLVM_COV:
        return plkInfo->plk_LLVM_COV.plk_next_kext_vmaddr;
    default:
        /* shouldn't ever be here */
        assert(false);
        return 0;
    }
}

static boolean_t setKCPlkSegNextVMAddr(plkInfo *plkInfo, enum enumSegIdx idx, uint64_t x) {
    if (!plkInfo)
        return false;

    switch (idx) {
    case SEG_IDX_TEXT:
        plkInfo->plk_TEXT.plk_next_kext_vmaddr = x;
        return true;
    case SEG_IDX_TEXT_EXEC:
        plkInfo->plk_TEXT_EXEC.plk_next_kext_vmaddr = x; 
        return true;
    case SEG_IDX_DATA:
        plkInfo->plk_DATA.plk_next_kext_vmaddr = x; 
        return true;
    case SEG_IDX_DATA_CONST:
        plkInfo->plk_DATA_CONST.plk_next_kext_vmaddr = x;
        return true;
    case SEG_IDX_LINKEDIT:
        plkInfo->plk_LINKEDIT.plk_next_kext_vmaddr = x;
        return true;
    case SEG_IDX_LLVM_COV:
        plkInfo->plk_LLVM_COV.plk_next_kext_vmaddr = x;
        return true;
    default:
        /* shouldn't ever be here */
        assert(false);
        return false;
    }
}

static boolean_t setKextVMAddr(OSKextRef aKext, enum enumSegIdx idx, uint64_t vmaddr) {
    if (!aKext)
        return false;

    switch (idx) {
    case SEG_IDX_TEXT:
        aKext->loadInfo->linkInfo.vmaddr_TEXT = vmaddr; 
        return true;
    case SEG_IDX_TEXT_EXEC:
        aKext->loadInfo->linkInfo.vmaddr_TEXT_EXEC = vmaddr;
        return true;
    case SEG_IDX_DATA:
        aKext->loadInfo->linkInfo.vmaddr_DATA = vmaddr; 
        return true;
    case SEG_IDX_DATA_CONST:
        aKext->loadInfo->linkInfo.vmaddr_DATA_CONST = vmaddr;
        return true;
    case SEG_IDX_LINKEDIT:
        aKext->loadInfo->linkInfo.vmaddr_LINKEDIT = vmaddr;
        return true;
    case SEG_IDX_LLVM_COV:
        aKext->loadInfo->linkInfo.vmaddr_LLVM_COV = vmaddr;
        return true;
    default:
        /* shouldn't ever be here */
        assert(false);
        return false;
    }
}

static u_long CopyPrelinkedKexts(
    CFMutableDataRef prelinkImage,
    CFArrayRef       loadList,
    u_long           fileOffsetBase,
    uint64_t         sourceAddrBase)
{
    boolean_t   success     = false;
    u_char    * prelinkData = CFDataGetMutableBytePtr(prelinkImage);
    u_long      size        = 0;
    u_long      totalSize   = 0;
    u_long      fileOffset  = fileOffsetBase;
    uint64_t    sourceAddr  = sourceAddrBase;
    int i = 0;

    /* Set the text segment and section address and offset */
    struct mach_header_64 *mach_header = (struct mach_header_64 *) CFDataGetBytePtr(prelinkImage);
    struct segment_command_64 *seg = macho_get_segment_by_name_64(mach_header,
                                                                  kPrelinkTextSegment);
    if (!seg) {
        goto finish;
    }
    seg->vmaddr = sourceAddrBase;
    seg->fileoff = fileOffsetBase;

    struct section_64 *sect = macho_get_section_by_name_64(mach_header,
                                                           kPrelinkTextSegment,
                                                           kPrelinkTextSection);
    if (!sect) {
        goto finish;
    }
    sect->addr = sourceAddrBase;
    sect->offset = fileOffset;
    
    /* Copy all kext executables */

    for (i = 0; i < CFArrayGetCount(loadList); ++i) {
        OSKextRef aKext = (OSKextRef) CFArrayGetValueAtIndex(loadList, i);

        if (aKext->flags.declaresKernelExecutable == false) {
            continue;
        }

       /* xxx - Is it safe to assume aKext->loadInfo exists here?
        */
        memcpy(prelinkData + fileOffset + size, 
            CFDataGetBytePtr(aKext->loadInfo->prelinkedExecutable),
            CFDataGetLength(aKext->loadInfo->prelinkedExecutable));

        size += (CFDataGetLength(aKext->loadInfo->prelinkedExecutable)
                 + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);
    }

    sourceAddr += size;
    fileOffset += size;
    totalSize += size;

    /* Set the text segment and section size */

    seg->vmsize = size;
    seg->filesize = size;
    sect->size = size;

    return totalSize;
  finish:
    fprintf(stderr, "failed to find seg or section\n");
    return -1;
}

static boolean_t __OSKextValidatePLKInfo(
    OSKextRef              aKext,
    struct mach_header_64 *kextHeader,
    plkInfo               *plkInfo )
{
    boolean_t       result = true;
    boolean_t       isSplitKext = false;
    plkSegInfo     *plkSeg[SEG_IDX_COUNT] = { NULL };
    char            kextPath[PATH_MAX];

    __OSKextGetFileSystemPath(aKext, NULL, true, kextPath);

    isSplitKext = __OSKextIsSplitKextMacho64(kextHeader);

    plkSeg[SEG_IDX_TEXT] = &plkInfo->plk_TEXT;
    if (isSplitKext) {
        plkSeg[SEG_IDX_TEXT_EXEC]  = &plkInfo->plk_TEXT_EXEC;
        plkSeg[SEG_IDX_DATA]       = &plkInfo->plk_DATA;
        plkSeg[SEG_IDX_DATA_CONST] = &plkInfo->plk_DATA_CONST;
        plkSeg[SEG_IDX_LINKEDIT]   = &plkInfo->plk_LINKEDIT;
        plkSeg[SEG_IDX_LLVM_COV]   = &plkInfo->plk_LLVM_COV;
    }

    for (size_t i = 0; i < sizeof(plkSeg)/sizeof(plkSeg[0]); i++) {
        if (!plkSeg[i])
            break;

        SegInfo *segInfo = &plkSeg[i]->plkSegInfo;
        uint64_t seg_vmoff = plkSeg[i]->plk_next_kext_vmaddr - segInfo->vmaddr;
        uint64_t seg_fileoff = segInfo->fileoff + seg_vmoff;
        if (segInfo->vmaddr > plkSeg[i]->plk_next_kext_vmaddr) {
            fprintf(stderr,
                      "%s vmaddr %p larger than plk_next_kext_vmaddr %p for %s",
                      plkSeg[i]->plk_seg_name,
                      (void *)segInfo->vmaddr,
                      (void *)plkSeg[i]->plk_next_kext_vmaddr,
                      kextPath);
            result = false;
        }
        if (segInfo->vmaddr + segInfo->vmsize < plkSeg[i]->plk_next_kext_vmaddr) {
            fprintf(stderr,
                      "%s overflow! plk_next_kext_vmaddr %p past end of segment %p for %s",
                      plkSeg[i]->plk_seg_name,
                      (void *)plkSeg[i]->plk_next_kext_vmaddr,
                      (void *)(segInfo->vmaddr + segInfo->vmsize),
                      kextPath);
            result = false;
        }
        if (segInfo->fileoff > seg_fileoff) {
            fprintf(stderr,
                      "%s fileoff %llu larger than plk_next_kext_fileoff %llu for %s",
                      plkSeg[i]->plk_seg_name,
                      segInfo->fileoff,
                      seg_fileoff,
                      kextPath);
            result = false;
        }
        if (segInfo->fileoff + segInfo->vmsize < seg_fileoff) {
            fprintf(stderr,
                      "%s overflow! plk_next_kext_fileoff %llu past end of segment %llu for %s",
                      plkSeg[i]->plk_seg_name,
                      seg_fileoff,
                      (segInfo->fileoff + segInfo->vmsize),
                      kextPath);
            result = false;
        }
        if (result == false) {
//            __OSKextShowPLKInfo(plkInfo);
            break;
        }
    }

    return result;
}

static boolean_t __OSKextSetLinkInfo(
    OSKextRef       aKext,
    plkInfo *       plkInfo,
    CFDataRef       kextExecutable)
{
    boolean_t                   result = false;
    boolean_t                   swapped = false;
    boolean_t                   isSplitKext = false;
    struct mach_header_64 *     kextHeader;
    uint32_t                    maxAlign;
    uint64_t                    next_vmaddr;

    if (!__OSKextCreateLoadInfo(aKext)) {
        goto finish;
    }

    {
        char *kextID = NULL;
        kextID = createUTF8CStringForCFString(aKext->bundleID);
        fprintf(stderr, "processing %s", kextID);
        if(kextID)
            free(kextID);
    }

    kextHeader = (struct mach_header_64 *)CFDataGetBytePtr(kextExecutable);
    isSplitKext = __OSKextIsSplitKextMacho64(kextHeader);

    u_char *file = (u_char *) CFDataGetBytePtr(kextExecutable);
    swapped = macho_swap_64(file);
    
    /* TODO: refactor special case for interace kext */
    if (aKext->flags.isInterface == true) {
        // NOTE - interface kexts only have __LINKEDIT segment. There is
        // only a macho header, load commands and the __LINKEDIT data so we
        // hang this off the __PRELINK_TEXT segment.
        // __LINKEDIT doesn't have alignment info so take a best guess.
        maxAlign = KEXT_MIN_ALIGN;

        aKext->loadInfo->linkedExecutable = CFRetain(kextExecutable);
        aKext->loadInfo->prelinkedExecutable = CFRetain(kextExecutable);
        aKext->loadInfo->linkInfo.linkedKext = (u_char *) CFDataGetBytePtr(kextExecutable);
        aKext->loadInfo->linkInfo.linkedKextSize = CFDataGetLength(kextExecutable);
    
        next_vmaddr = __OSKextAlignAddress(getKCPlkSegNextVMAddr(plkInfo, SEG_IDX_TEXT), maxAlign);
        aKext->loadInfo->linkInfo.vmaddr_TEXT = next_vmaddr;

        setKCPlkSegNextVMAddr(
            plkInfo,
            SEG_IDX_TEXT,
            next_vmaddr + sizeof(kextHeader) + kextHeader->sizeofcmds);
        result = true;
        goto finish;
    } // interface kext

    aKext->loadInfo->linkInfo.kextExecutable = (u_char *) CFDataGetBytePtr(kextExecutable);
    aKext->loadInfo->linkInfo.kextSize = CFDataGetLength(kextExecutable);

    /* set target VM addr and save original VM addr for all segments in this kext */
    for(enum enumSegIdx segIndex = SEG_IDX_TEXT; segIndex < SEG_IDX_COUNT; segIndex++) {
        uint64_t my_vmaddr, my_vmsize, my_fileoff, my_filesize, my_maxalign, next_vmaddr;
        char *segName = segIdxToName(segIndex);

        result = __OSKextGetSegmentInfo(aKext->loadInfo->linkInfo.kextExecutable,
                                        segName,
                                        &my_vmaddr,
                                        &my_vmsize,
                                        &my_fileoff,
                                        &my_filesize,
                                        &my_maxalign,
                                        true);

        if (result == false)
            continue;

        fprintf(stderr,
                "segName %s vmaddr %llx vmsize %llx fileoff %llx filesize %llx maxalign %llx",
                segName, my_vmaddr, my_vmsize, my_fileoff, my_filesize, my_maxalign);

        {
            struct segment_command_64 *seg = macho_get_segment_by_name_64(kextHeader, segName);
            assert(seg);

            seg->vmsize = my_vmsize;
            seg->filesize = my_filesize;
        }

        next_vmaddr = __OSKextAlignAddress(getKCPlkSegNextVMAddr(plkInfo, segIndex), my_maxalign);
        fprintf(stderr, "segName %s new vmaddr %llx", segName, next_vmaddr);

        setKextVMAddr(aKext, segIndex, next_vmaddr);

        setKCPlkSegNextVMAddr(plkInfo, segIndex, next_vmaddr + my_vmsize);
    }

    if (__OSKextValidatePLKInfo(aKext, kextHeader, plkInfo) == false) {
        abort();
        goto finish;
    }

    result = true;

finish:
    if (swapped) {
        enum NXByteOrder order = 0;
        struct mach_header_64 *hdr = (struct mach_header_64 *) file;
        struct load_command *lc = (struct load_command *) &hdr[1];
        struct segment_command_64 *seg = NULL;
        u_long offset = 0;
        u_int i = 0;

        if (NXHostByteOrder() == NX_LittleEndian) {
            order = NX_BigEndian;
        } else {
            order = NX_LittleEndian;
        }
        
        if (!hdr || hdr->magic != MH_MAGIC_64) goto finish;
        
        offset = sizeof(*hdr);
        for (i = 0; i < hdr->ncmds; ++i) {
            lc = (struct load_command *) (file + offset);
            offset += lc->cmdsize;
            
            if (lc->cmd == LC_SEGMENT_64) {
                seg = (struct segment_command_64 *) lc;
                swap_segment_command_64(seg, order);
            } else {
                swap_load_command(lc, order);
            }
        }

        swap_mach_header_64(hdr, order);
    }
    
    return result;
}

/* ------------------------------------------------------------------------ */

Boolean __OSKextReadExecutable(OSKextRef aKext)
{
    Boolean result = false;
    char kextPath[PATH_MAX];
    char executablePath[PATH_MAX];
    char *executableBuffer = NULL;
    struct stat statbuf;
    CFAllocatorContext mmapAllocatorContext;
    CFAllocatorRef mmapAllocator;
    __OSKextMmapBufferInfo *mmapAllocatorInfo;
    int fd = -1;
    int length = 0;
    
    if (!aKext || aKext->flags.declaresKernelExecutable == false)
        goto finish;

    if (aKext->loadInfo && aKext->loadInfo->executable) {
        return true;
    } else {
        if (!__OSKextCreateLoadInfo(aKext)) {
            goto finish;
        }
        
        __OSKextGetFileSystemPath(aKext, NULL, true, kextPath);
        int pos = strlen(kextPath);
        memcpy(executablePath, kextPath, pos);
        memcpy(executablePath + pos, "/Contents/MacOS/", 16);
        pos += 16;
        char *base = basename(kextPath);
        int i = strlen(base);
        while(i > 0 && *(base+i) != '.')
            --i;
        *(base+i) = '\0';
        memcpy(executablePath + pos, base, strlen(base));
        pos += strlen(base);
        *(executablePath + pos) = '\0';
        
        stat(executablePath, &statbuf);
        length = statbuf.st_size;
        fd = open(executablePath, O_RDONLY);
        executableBuffer = mmap(NULL,
                                length,
                                PROT_READ|PROT_WRITE,
                                MAP_FILE|MAP_PRIVATE,
                                fd,
                                0);
        fprintf(stderr, "mapped executable file %s, %lu bytes\n",
                executablePath, length);
        
        CFAllocatorGetContext(kCFAllocatorDefault, &mmapAllocatorContext);
        mmapAllocatorInfo =
            (__OSKextMmapBufferInfo *)malloc(sizeof(__OSKextMmapBufferInfo));
        mmapAllocatorInfo->length = length;
        mmapAllocatorContext.info = mmapAllocatorInfo;
        mmapAllocatorContext.deallocate = &__OSKextDeallocateMmapBuffer;
        mmapAllocator = CFAllocatorCreate(kCFAllocatorDefault,
                                          &mmapAllocatorContext);
        aKext->loadInfo->executable = CFDataCreateWithBytesNoCopy(
            CFGetAllocator(aKext), executableBuffer, length,
            /* bytesDeallocator */ mmapAllocator);
    }

    if (!aKext->loadInfo->executable)
        return false;
    result = true;

  finish:
    if(mmapAllocator)
        CFRelease(mmapAllocator);
    if (fd != -1)
        close(fd);
    
    if (!result) {
        if(mmapAllocatorInfo)
            CFRelease(mmapAllocatorInfo);

        if (executableBuffer) {
            fprintf(stderr,
                    "Error encountered, unmapping executable file %s (%lu bytes).",
                    executablePath, (unsigned long)length);
            munmap(executableBuffer, length);
        }
    }
    
    return result;
}

static Boolean __OSKextPerformLink(
    OSKextRef        aKext,
    CFDataRef        kernelImage,
    uint64_t         kernelLoadAddress,
    Boolean          stripSymbolsFlag,
    KXLDContext *    kxldContext)
{
    Boolean                    result              = false;
    char                     * bundleIDCString     = NULL;      // must free
    CFArrayRef                 dependencies        = NULL;      // must release
    CFMutableArrayRef          indirectDependencies = NULL;     // must release
    CFDataRef                  kextExecutable      = NULL;      // must release
    
    kern_return_t              kxldResult          = KERN_FAILURE;
    KXLDDependency           * kxldDependencies    = NULL;      // must free
    CFIndex                    numKxldDependencies = 0;
    kxld_addr_t                kmodInfoKern        = 0;
    
    CFIndex                    numDirectDependencies    = 0;
    CFIndex                    numIndirectDependencies  = 0;
    
//    __OSKextKXLDCallbackContext linkAddressContext;
    
    u_char                   * relocBytes          = NULL;    // do not free
    u_char                  ** relocBytesPtr       = NULL;    // do not free
    
    CFDataRef                  relocData           = NULL;    // must release
    char                       kextPath[PATH_MAX];
    CFIndex                    i;
    
    if (aKext->flags.declaresKernelExecutable == false) {
        result = true;
        goto finish;
    }

     __OSKextGetFileSystemPath(aKext, NULL, true, kextPath);

     if(!__OSKextReadExecutable(aKext)) {
         fprintf(stderr, "failed to read executable for %s\n", kextPath);
         goto finish;
     }
     kextExecutable = aKext->loadInfo->executable;
    
     if (aKext->flags.isInterface == true) {
         aKext->loadInfo->linkedExecutable = CFRetain(kextExecutable);
         aKext->loadInfo->prelinkedExecutable = CFRetain(kextExecutable);
         aKext->loadInfo->linkInfo.linkedKextSize = CFDataGetLength(kextExecutable);
         result = true;
         goto finish;
     }
        
     fprintf(stderr, "Linking %s.", kextPath);
     bundleIDCString = createUTF8CStringForCFString(aKext->bundleID);

     relocBytesPtr = &relocBytes;
     
//     linkAddressContext.kernelLoadAddress = kernelLoadAddress;
//     linkAddressContext.kext = aKext;
    
     kxldResult = kxld_link_file(kxldContext,
                                 (void *)CFDataGetBytePtr(kextExecutable),
                                 CFDataGetLength(kextExecutable),
                                 bundleIDCString,
                                 /* callbackData */ (void *)0, //&linkAddressContext,
                                 NULL, 0,
                                 relocBytesPtr,
                                 &kmodInfoKern);
    
    
     if (kxldResult != KERN_SUCCESS) {
         fprintf(stderr, "Link failed (error code %d).", kxldResult);
         goto finish;
     }
    
     if (relocBytes && aKext->loadInfo->linkInfo.linkedKextSize) {
         relocData = CFDataCreateWithBytesNoCopy(CFGetAllocator(aKext),
                                                 relocBytes,
                                                 aKext->loadInfo->linkInfo.linkedKextSize,
                                                 kCFAllocatorDefault);
         if (!relocData) {
             goto finish;
         }
         aKext->loadInfo->linkedExecutable = CFRetain(relocData);
         aKext->loadInfo->kmodInfoAddress = kmodInfoKern;
            
         if (!aKext->loadInfo->prelinkedExecutable) {
             aKext->loadInfo->prelinkedExecutable = CFRetain(relocData);
         }
     } // relocBytes...
        
     result = true;
        
  finish:
     if(kextExecutable)
         CFRelease(kextExecutable);
     if(relocData)
         CFRelease(relocData);
     if(bundleIDCString)
         CFRelease(bundleIDCString);
     return result;
}

CFMutableArrayRef OSKextCopyLoadListForKexts(CFArrayRef kexts, Boolean needAll)
{
    CFMutableArrayRef result         = NULL;
    CFMutableArrayRef globalLoadList = NULL;
    CFMutableSetRef   resolvedKexts  = NULL;
    CFArrayRef        loadList       = NULL;
    CFIndex           kextCount, loadListCount, i, j;
    
    /* Create a set to track the kexts whose dependencies have been resolved */
    
    resolvedKexts = CFSetCreateMutable(kCFAllocatorDefault, 0,
        &kCFTypeSetCallBacks);
    if (!resolvedKexts) {
        fprintf(stderr, "Out of memory\n");
        goto finish;
    }
    
    /* Create the global load list */
    
    globalLoadList = CFArrayCreateMutable(kCFAllocatorDefault, 0,
        &kCFTypeArrayCallBacks);
    if (!globalLoadList) {
        fprintf(stderr, "Out of memory\n");
        goto finish;
    }
    
    /* Generate the global load list */
    
    kextCount = CFArrayGetCount(kexts);
    for (i = 0; i < kextCount; ++i) {
        Boolean valid     = false;

        if(loadList)
            CFRelease(loadList);
        loadList = NULL;
        
        OSKextRef theKext = (OSKextRef) CFArrayGetValueAtIndex(kexts, i);
        
       /* If we've already determined this kext's load order, skip it.
        */
        if (CFSetGetValue(resolvedKexts, theKext)) continue;

        /* We are deliberately not handling dependencies here. You must
         * provide a full set of kexts to link
         */
        CFArrayAppendValue(globalLoadList, theKext);
        CFSetSetValue(resolvedKexts, theKext);
    }
    
    result = globalLoadList;
    globalLoadList = NULL;
    
finish:
    if(resolvedKexts)
        CFRelease(resolvedKexts);
    if(globalLoadList)
        CFRelease(globalLoadList);
    if(loadList)
        CFRelease(loadList);
    
    return result;
}

static CFArrayRef __OSKextPrelinkKexts(
                                       CFArrayRef        kextArray,
                                       CFDataRef         kernelImage,
                                       uint64_t          loadAddrBase,
                                       uint64_t          sourceAddrBase,
                                       KXLDContext     * kxldContext,
                                       u_long          * loadSizeOut,
                                       Boolean           needAllFlag,
                                       Boolean           skipAuthenticationFlag,
                                       Boolean           printDiagnosticsFlag,
                                       Boolean           stripSymbolsFlag)
{
    CFArrayRef        result   = NULL;
    boolean_t         success  = false;
    CFMutableArrayRef loadList = NULL;
    uint64_t          loadAddr = loadAddrBase;
    uint64_t          sourceAddr = sourceAddrBase;
    u_long            loadSize = 0;
    char            * kextIdentifierCString = NULL;  // must free
    CFIndex           i;

    loadList = OSKextCopyLoadListForKexts(kextArray, false);
    if (!loadList) {
        fprintf(stderr, "failed to build loadlist\n");
        goto finish;
    }

    /* Link each kext in the load list */
    for (i = 0; i < CFArrayGetCount(loadList); ++i) {
        OSKextRef aKext = (OSKextRef) CFArrayGetValueAtIndex(loadList, i);

        if(kextIdentifierCString)
            CFRelease(kextIdentifierCString);
        
        if (aKext->flags.declaresKernelExecutable == false)
            continue;

        kextIdentifierCString =
            createUTF8CStringForCFString(aKext->bundleID);

       /* Set the load address of the kext.
        */

        loadAddr = loadAddrBase + loadSize;
        sourceAddr = sourceAddrBase + loadSize;
        
        __OSKextCreateLoadInfo(aKext);
        aKext->loadInfo->linkInfo.vmaddr_TEXT = loadAddr;
        aKext->loadInfo->sourceAddress = sourceAddr;

        success = __OSKextPerformLink(aKext, kernelImage, 0, false, kxldContext);

        if (!success) {
            fprintf(stderr, "prelink failed at %s\n", kextIdentifierCString);
            goto finish;
        }

        loadSize += aKext->loadInfo->linkInfo.linkedKextSize;
        loadSize += (PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    } // kext loadList for loop

    result = CFRetain(loadList);
    
    *loadSizeOut = loadSize;

finish:
    if(loadList)
        CFRelease(loadList);
    if(kextIdentifierCString)
        CFRelease(kextIdentifierCString);
    return result;
}

void __OSKextLoggingCallback(
    KXLDLogSubsystem    subsystem, 
    KXLDLogLevel        level, 
    const char        * format, 
    va_list             argList,
    void              * user_data)
{
    vfprintf(stdout, format, argList);
}

static boolean_t GetLastKernelLoadAddr(
    CFDataRef  kernelImage, 
    uint64_t * lastLoadAddrOut)
{
    boolean_t            result         = false;
    const UInt8        * kernelImagePtr = CFDataGetBytePtr(kernelImage);
    uint64_t             lastLoadAddr   = 0;
    uint64_t             i;

    struct mach_header_64 * kernel_header =
        (struct mach_header_64 *)kernelImagePtr;
    struct segment_command_64 * seg_cmd =
        (struct segment_command_64 *)
        ((uintptr_t)kernel_header + sizeof(*kernel_header));
    
    for (i = 0; i < kernel_header->ncmds; i++){
        if (seg_cmd->cmd == LC_SEGMENT_64) {
            if (seg_cmd->vmaddr + seg_cmd->vmsize > lastLoadAddr) {
                lastLoadAddr = seg_cmd->vmaddr + seg_cmd->vmsize;
            }
        }
        seg_cmd = (struct segment_command_64 *)
            ((uintptr_t)seg_cmd + seg_cmd->cmdsize);
    }

    if (lastLoadAddrOut)
        *lastLoadAddrOut = lastLoadAddr;
    result = true;

    return result;
}

macho_seek_result __OSKextUUIDCallback(
    struct load_command * load_command,
    const void * file_end,
    uint8_t swap __unused,
    void * user_data)
{
    struct _uuid_stuff * uuid_stuff = (struct _uuid_stuff *)user_data;
    if (load_command->cmd == LC_UUID) {
        struct uuid_command * uuid_command = (struct uuid_command *)load_command;
        if (((void *)load_command + load_command->cmdsize) > file_end) {
            return macho_seek_result_error;
        }
        uuid_stuff->uuid_size = sizeof(uuid_command->uuid);
        uuid_stuff->uuid = (char *)uuid_command->uuid;
        return macho_seek_result_found;
    }
    return macho_seek_result_not_found;
}

static boolean_t __OSKextGetSegmentAddressAndOffset(
    const UInt8 *   imagePtr,
    const char *    segname,
    uint32_t *      fileOffsetOut,
    uint64_t *      loadAddrOut)
{
    boolean_t result = false;
    uint32_t fileOffset = 0;
    uint64_t loadAddr = 0;
    
    struct mach_header_64 *mach_header = (struct mach_header_64 *) imagePtr;
    struct segment_command_64 *seg = macho_get_segment_by_name_64(mach_header, segname);
    if (!seg) {
        goto finish;
    }
    
    fileOffset = seg->fileoff;
    loadAddr = seg->vmaddr;

    if (fileOffsetOut) *fileOffsetOut = fileOffset;
    if (loadAddrOut) *loadAddrOut = loadAddr;
    result = true;

finish:
    return result;
}

static boolean_t __OSKextGetSegmentAddressAndOffsetDataRef(
    CFDataRef       imageRef,
    const char *    segname,
    uint32_t *      fileOffsetOut,
    uint64_t *      loadAddrOut)
{
    boolean_t       result;
    const UInt8 *   imagePtr = CFDataGetBytePtr(imageRef);
    
    result = __OSKextGetSegmentAddressAndOffset(imagePtr, segname, fileOffsetOut, loadAddrOut);
    
    return result;
}


static boolean_t __OSKextGetSegmentFileAndVMSize(
    const UInt8 *   imagePtr,
    const char *    segname,
    uint64_t *      fileSizeOut,
    uint64_t *      VMSizeOut)
{
    boolean_t result = false;
    uint64_t filesize = 0;
    uint64_t vmsize = 0;
    
    struct mach_header_64 *mach_header = (struct mach_header_64 *) imagePtr;
    struct segment_command_64 *seg = macho_get_segment_by_name_64(mach_header, segname);
    if (!seg) {
        goto finish;
    }
    
    filesize = seg->filesize;
    vmsize = seg->vmsize;
    
    if (fileSizeOut) *fileSizeOut = filesize;
    if (VMSizeOut) *VMSizeOut = vmsize;
    result = true;
    
finish:
    return result;
}

static boolean_t __OSKextGetSegmentFileAndVMSizeDataRef(
    CFDataRef       imageRef,
    const char *    segname,
    uint64_t *      fileSizeOut,
    uint64_t *      VMSizeOut)
{
    boolean_t       result;
    const UInt8 *   imagePtr = CFDataGetBytePtr(imageRef);
    
    result = __OSKextGetSegmentFileAndVMSize(imagePtr, segname, fileSizeOut, VMSizeOut);

    return result;
}

CF_RETURNS_RETAINED
CFTypeRef
IOCFUnserialize(const char *buffer,
                CFAllocatorRef allocator,
                CFOptionFlags options,
                CFStringRef *errorString)
{
    /* stub to satisfy linker */
    return (CFTypeRef)0;
}

#define SAFE_RELEASE(x) if(x) CFRelease(x)
static CFDataRef __OSKextCreatePrelinkInfoDictionary(
    plkInfo   *plkInfo,
    CFArrayRef loadList,
    CFURLRef   volumeRootURL,
    Boolean    includeAllPersonalities,
    Boolean    isSplitKexts,
    CFDataRef  kernelUUID)
{
    CFDataRef                   result                  = NULL; // do not release

    char                        kextPath[PATH_MAX]      = "";
    char                        volumePath[PATH_MAX]    = "";
    CFArrayRef                  allKextsByBundleID      = NULL; // must release
    CFArrayRef                  kextPersonalities       = NULL; // must release
    CFMutableArrayRef           kextInfoDictArray       = NULL; // must release
    CFDataRef                   prelinkInfoData         = NULL; // must release
    CFDataRef                   uuid                    = NULL; // must release
    CFMutableDictionaryRef      kextInfoDict            = NULL; // must release
    CFMutableDictionaryRef      prelinkInfoDict         = NULL; // must release
    CFNumberRef                 cfnum                   = NULL; // must release
    CFStringRef                 bundleVolPath           = NULL; // must release
    CFStringRef                 executableRelPath       = NULL; // must release
    CFStringRef                 archPersonalitiesKey    = NULL; // must release
    CFSetRef                    loadListIDs             = NULL; // must release
    char                      * kextVolPath             = NULL; // do not free
    int                         i                       = 0;
    int                         count                   = 0;
    EVP_MD_CTX                * ctx;
    unsigned char               kernelCacheHash[SHA256_DIGEST_LENGTH];
    CFDataRef                   kcID                    = NULL; // must release

    /* Create a dictionary for all prelinked kernel metadata */

    prelinkInfoDict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (!prelinkInfoDict) {
        perror("prelinkInfoDict");
        goto finish;
    }

    /* Create an array to hold all of the info dictionaries */

    kextInfoDictArray = CFArrayCreateMutable(kCFAllocatorDefault,
        CFArrayGetCount(loadList), &kCFTypeArrayCallBacks);
    if (!kextInfoDictArray) {
        perror("kextInfoDictArray");
        goto finish;
    }

    CFDictionarySetValue(prelinkInfoDict, CFSTR(kPrelinkInfoDictionaryKey),
        kextInfoDictArray);

    ctx = EVP_MD_CTX_new();
    EVP_DigestInit(&ctx, EVP_sha256);

    if (kernelUUID) {
        EVP_DigestUpdate(&ctx, (unsigned char*) CFDataGetBytePtr(kernelUUID), CFDataGetLength(kernelUUID));
    }

    /* Create an info dictionary for each kext in the load list */

    count = CFArrayGetCount(loadList);
    for (i = 0; i < count; ++i) {
        Boolean   gotPath = FALSE;
        OSKextRef aKext   = (OSKextRef)CFArrayGetValueAtIndex(loadList, i);

        if(kextInfoDict)
            CFRelease(kextInfoDict);
        kextInfoDict = NULL;
        if(bundleVolPath)
            CFRelease(bundleVolPath);
        bundleVolPath = NULL;

        __OSKextGetFileSystemPath(aKext, /* otherURL */ NULL,
            /* resolveToBase */ true, kextPath);

        fprintf(stdout, "Adding %s to prelinked kernel.", kextPath);

        /* Get the existing info dictionary from the kext */
        kextInfoDict =     aKext->infoDictionary;

        CFMutableDictionaryRef result = NULL;
        result = CFDictionaryCreateMutableCopy(CFGetAllocator(aKext), 0,
                                               kextInfoDict);

        if (!kextInfoDict) {
            perror("kextInfoDict");
            goto finish;
        }

        /* Add the load address, source address, and kmod info address information.
         */
        if (aKext->flags.declaresKernelExecutable == true) {
            cfnum = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type,
                                   &aKext->loadInfo->linkInfo.vmaddr_TEXT);
            if (!cfnum) {
                perror("malloc");
                goto finish;
            }
            CFDictionarySetValue(kextInfoDict, CFSTR(kPrelinkExecutableLoadKey), cfnum);
            if(cfnum)
                CFRelease(cfnum);
            cfnum = NULL;
                        
            cfnum = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type,
                                   &aKext->loadInfo->sourceAddress);
            
            if (!cfnum) {
                perror("malloc");
                goto finish;
            }
            CFDictionarySetValue(kextInfoDict, CFSTR(kPrelinkExecutableSourceKey), cfnum);
            if(cfnum)
                CFRelease(cfnum);
            cfnum = NULL;
            
            u_long num;
            num = CFDataGetLength(aKext->loadInfo->prelinkedExecutable);
            cfnum = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &num);
            if (!cfnum) {
                perror("malloc");
                goto finish;
            }
            CFDictionarySetValue(kextInfoDict, CFSTR(kPrelinkExecutableSizeKey), cfnum);
            if(cfnum)
                CFRelease(cfnum);
            cfnum = NULL;
            
            // Update the KC ID based on the start address of the KEXT
            EVP_DigestUpdate(ctx, &aKext->loadInfo->linkInfo.vmaddr_TEXT,
                             sizeof(aKext->loadInfo->linkInfo.vmaddr_TEXT));
                
            cfnum = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type,
                               &aKext->loadInfo->kmodInfoAddress);
            if (!cfnum) {
                perror("malloc");
                goto finish;
            }
            CFDictionarySetValue(kextInfoDict, CFSTR(kPrelinkKmodInfoKey), cfnum);
            if(cfnum)
                CFRelease(cfnum);
            cfnum = NULL;
        }
            
        /* Add this info dictionary to the info dict array */
        CFArrayAppendValue(kextInfoDictArray, kextInfoDict);
    }

    int lenp = 0;
    EVP_DigestFinal_ex(ctx, (unsigned char*)&kernelCacheHash, &lenp);
    kcID = CFDataCreate(kCFAllocatorDefault, kernelCacheHash, sizeof(uuid_t));
    if (kcID) {
        /* Add the kernelcache ID */
        CFDictionarySetValue(prelinkInfoDict, CFSTR(kPrelinkInfoKCIDKey), kcID);

        fprintf(stdout, "KernelCache ID: ");
        for (unsigned long i = 0; i < sizeof(kernelCacheHash)/2; i++)  {
            fprintf(stdout, "%02X", kernelCacheHash[i]);
        }
        fprintf(stdout, "\n");
    } else {
        fprintf(stderr, "Failed to allocate kernelcache ID\n");
    }

    /* Serialize the info dictionary */

    prelinkInfoData = IOCFSerialize(prelinkInfoDict, kNilOptions);
    if (!prelinkInfoData) {
        perror("malloc");
        goto finish;
    }

    result = CFRetain(prelinkInfoData);

finish:
    SAFE_RELEASE(allKextsByBundleID);
    SAFE_RELEASE(kextPersonalities);
    SAFE_RELEASE(kextInfoDictArray);
    SAFE_RELEASE(prelinkInfoData);
    SAFE_RELEASE(uuid);
    SAFE_RELEASE(kcID);
    SAFE_RELEASE(kextInfoDict);
    SAFE_RELEASE(prelinkInfoDict);
    SAFE_RELEASE(cfnum);
    SAFE_RELEASE(bundleVolPath);
    SAFE_RELEASE(executableRelPath);
    SAFE_RELEASE(archPersonalitiesKey);
    SAFE_RELEASE(loadListIDs);
    return result;
}

CFMutableDictionaryRef OSKextCopyInfoDictionary(OSKextRef aKext)
{
    CFMutableDictionaryRef result = NULL;

    if (aKext->infoDictionary) {

        result = CFDictionaryCreateMutableCopy(CFGetAllocator(aKext), 0,
                                               aKext->infoDictionary);
    }

finish:
    return result;
}



CFDataRef CreatePrelinkedKernel(
    CFDataRef kernelImage,
    CFArrayRef kextArray)
{
    plkInfo plkInfo;
    CFMutableDataRef prelinkImage = NULL;
    CFArrayRef loadList = NULL;
    CFDataRef kernelUUID = NULL;
    uintptr_t baseFileOffset = 0;
    uintptr_t baseLoadAddr = 0;
    uintptr_t baseSrcAddr = 0;
    const struct mach_header_64 *mach_header;
    const unsigned char *file_end;
    KXLDContext * kxldContext;
    u_long size = 0;
    uintptr_t textLoadAddr;
    uintptr_t textVMSize;


    baseFileOffset = CFDataGetLength(kernelImage);
    memset(&plkInfo, 0, sizeof(plkInfo));
    
    kxld_create_context(&kxldContext,
                        NULL, // __OSKextLinkAddressCallback,
                        __OSKextLoggingCallback,
                        kKxldFlagDefault | kKXLDFlagIncludeRelocs,
                        CPU_TYPE_X86_64,
                        CPU_SUBTYPE_X86_64_ALL,
                        0);

    Boolean swapped = SwapHeaders(kernelImage);
    if(!GetLastKernelLoadAddr(kernelImage, &baseSrcAddr))
        goto failed;

    /* round up to page size */
    baseFileOffset = (baseFileOffset + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);
    baseSrcAddr = (baseSrcAddr + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);

    Boolean success = __OSKextGetSegmentAddressAndOffsetDataRef(kernelImage,
                                                                SEG_TEXT,
                                                                NULL,
                                                                &textLoadAddr);
    if (!success) {
        fprintf(stderr, "Could not get kernel text address.\n");
        goto failed;
    }

    success = __OSKextGetSegmentFileAndVMSizeDataRef(kernelImage,
                                                     SEG_TEXT,
                                                     NULL,
                                                     &textVMSize);
    if (!success) {
        fprintf(stderr, "Could not get kernel text vmsize.\n");
        goto failed;
    }

    baseLoadAddr = (textLoadAddr + textVMSize
                    + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);

    loadList = __OSKextPrelinkKexts(kextArray,
                                    kernelImage,
                                    baseLoadAddr, // where we load kext __TEXT segments
                                    baseSrcAddr,
                                    kxldContext,
                                    &size,
                                    true, /* (flags & kOSKextKernelcacheNeedAllFlag), */
                                    true, /* (flags & kOSKextKernelcacheSkipAuthenticationFlag), */
                                    true, /* (flags & kOSKextKernelcachePrintDiagnosticsFlag), */
                                    false); /* (flags & kOSKextKernelcacheStripSymbolsFlag)); */

    if(!loadList)
        goto failed;

    mach_header = (const struct mach_header *)CFDataGetBytePtr(kernelImage);
    file_end = (((const char *)mach_header) + CFDataGetLength(kernelImage));
    struct _uuid_stuff seek_uuid;
    macho_seek_result seek_result = macho_scan_load_commands(mach_header,
                                                             file_end,
                                                             __OSKextUUIDCallback,
                                                             (const void **)&seek_uuid);
    if (seek_result == macho_seek_result_found) {
        kernelUUID = CFDataCreate(kCFAllocatorDefault, (u_char *)seek_uuid.uuid,
                                  swapped
                                  ? bswap_32(seek_uuid.uuid_size)
                                  : seek_uuid.uuid_size);
    }
    
    CFDataRef prelinkInfoData =
        __OSKextCreatePrelinkInfoDictionary(&plkInfo,
                                            loadList,
                                            NULL, /* volumeRootURL */
                                            0, /* flags */
                                            0, /* isARM64 */
                                            kernelUUID);

    /* grow the prelink image */
    size = CFDataGetLength(prelinkImage) + CFDataGetLength(prelinkInfoData);
    size += (PAGE_SIZE - 1) & ~(PAGE_SIZE - 1); /* round to page size */     
    prelinkImage = CFDataCreateMutable(kCFAllocatorDefault, size);
    CFDataSetLength(prelinkImage, size);
    
    if(swapped)
        UnswapHeaders(kernelImage);
    swapped = 0;
    
    CFDataReplaceBytes(prelinkImage, CFRangeMake(0, CFDataGetLength(kernelImage)),
                       CFDataGetBytePtr(kernelImage), CFDataGetLength(kernelImage));
    
    uintptr_t fileOffset = baseFileOffset;
    uintptr_t srcAddr = baseSrcAddr;
    size = CopyPrelinkedKexts(prelinkImage,
                              loadList,
                              fileOffset,
                              srcAddr);
    srcAddr += size;

    u_char    * prelinkData = CFDataGetMutableBytePtr(prelinkImage);
    u_long      pdsize      = 0;
    
    pdsize = CFDataGetLength(prelinkInfoData);
    memcpy(prelinkData + fileOffset, CFDataGetBytePtr(prelinkInfoData), size);
    
    /* Set the info dictionary segment headers */
    mach_header = (struct mach_header_64 *) CFDataGetBytePtr(prelinkImage);
    struct segment_command_64 *seg = macho_get_segment_by_name_64(mach_header, kPrelinkInfoSegment);
    if (!seg) {
        goto failed;
    }
    
    seg->vmaddr = srcAddr;
    seg->vmsize = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    seg->fileoff = fileOffset;
    seg->filesize = pdsize;

    struct section_64 *sect = macho_get_section_by_name_64(mach_header,
                                                           kPrelinkInfoSegment,
                                                           kPrelinkInfoSection);
    if (!sect) {
        goto failed;
    }
        
    sect->addr = srcAddr;
    sect->offset = fileOffset;
    sect->size = pdsize;

    pdsize += (PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    CFDataSetLength(prelinkImage, fileOffset);
    CFDataRef result = CFRetain(prelinkImage);
    
  failed:
    CFRelease(loadList);
    CFRelease(prelinkInfoData);
    CFRelease(prelinkImage);
    CFRelease(kernelUUID);
    kxld_destroy_context(&kxldContext);
    return result;
}


int main(int argc, char **argv)
{
    printf("plktool -k kernel [kexts ...]\n");
    return 0;
}
