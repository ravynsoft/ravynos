/**
 * kext.c
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
 * to compile on Linux. Most of this code originates from kext.subproj/OSKext.c.
 */

#include "plktool.h"

static const boolean_t g_max_align_to_4k = false;

const CFRuntimeClass __OSKextClass = {
    0,                       // version
    "OSKext",                // className
    NULL,                    // init
    NULL,                    // copy
    __OSKextReleaseContents, // finalize
    NULL,                    // equal: pointer equality, baby.
    NULL,                    // hash
    NULL,                    // copyFormattingDesc
    NULL,                    // copyDebugDesc
#if CF_RECLAIM_AVAILABLE
    NULL, // xxx - need to set reclaim field for garbage collection
#endif
#if CF_REFCOUNT_AVAILABLE
    NULL
#endif
};

CFTypeID __kOSKextTypeID = _kCFRuntimeNotATypeID;

typedef struct __OSKextKXLDCallbackContext
{
    OSKextRef kext;
    uint64_t  kernelLoadAddress;
} __OSKextKXLDCallbackContext;

CFTypeID
OSKextGetTypeID(void)
{
    return __kOSKextTypeID;
}

boolean_t
__os_warn_unused(const boolean_t x)
{
    return x;
}

static char *
segIdxToName(enum enumSegIdx idx)
{
    switch (idx)
    {
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

static void
__OSKextReleaseContents(CFTypeRef cfObject)
{
    OSKextRef aKext = (OSKextRef) cfObject;
    if (aKext->bundleURL)
        CFRelease(aKext->bundleURL);
    if (aKext->bundleID)
        CFRelease(aKext->bundleID);
    if (aKext->executableURL)
        CFRelease(aKext->executableURL);
    if (aKext->infoDictionary)
        CFRelease(aKext->infoDictionary);
    return;
}


void
__OSKextLoggingCallback(KXLDLogSubsystem subsystem,
                        KXLDLogLevel     level,
                        const char      *format,
                        va_list          argList,
                        void            *user_data)
{
    vfprintf(stdout, format, argList);
    putc('\n', stdout);
}

static void
__OSKextCheckLoaded(OSKextRef aKext)
{
    if (!aKext->loadInfo) return;

    /* It's guaranteed to not be loaded on a Linux host :) */
    aKext->loadInfo->flags.isLoaded = 0;
    aKext->loadInfo->flags.otherUUIDIsLoaded = 0;
    aKext->loadInfo->flags.isStarted = 0;
    aKext->loadInfo->loadTag = 0;

    aKext->loadInfo->linkInfo.vmaddr_TEXT = 0; // FIXME: calculate from top of __text?
    aKext->loadInfo->linkInfo.linkedKextSize = 0; // FIXME: what goes here? loadSize;
}

uint64_t
OSKextGetLoadAddress(OSKextRef aKext)
{
    uint64_t result = 0x0;

    if (!aKext->loadInfo)
    {
        goto finish;
    }
    if (aKext->loadInfo->kernelLoadInfo)
    {
        __OSKextCheckLoaded(aKext);
    }
    result = aKext->loadInfo->linkInfo.vmaddr_TEXT;

finish:
    return result;
}

kxld_addr_t
__OSKextLinkAddressCallback(u_long                   size,
                            KXLDAllocateFlags       *flags __unused,
                            void                    *user_data)
{
    kxld_addr_t                  result = 0;
    kxld_addr_t                  kextAddress = 0;
    static kxld_addr_t           loadAddressOffset = 0;
    __OSKextKXLDCallbackContext *context =
        (__OSKextKXLDCallbackContext *) user_data;

    context->kext->loadInfo->linkInfo.linkedKextSize = size;
    kextAddress = (kxld_addr_t) OSKextGetLoadAddress(context->kext);
    if (kextAddress)
    {
        result = kextAddress;
    }
    else
    {
        result = context->kernelLoadAddress + loadAddressOffset;
        loadAddressOffset += size;
    }

    return result;
}

macho_seek_result
__OSKextUUIDCallback(struct load_command *load_command,
                     const void          *file_end,
                     uint8_t              swap __unused,
                     void                *user_data)
{
    struct _uuid_stuff *uuid_stuff = (struct _uuid_stuff *) user_data;
    if (load_command->cmd == LC_UUID)
    {
        struct uuid_command *uuid_command = (struct uuid_command *) load_command;
        if (((void *) load_command + load_command->cmdsize) > file_end)
        {
            return macho_seek_result_error;
        }
        uuid_stuff->uuid_size = sizeof(uuid_command->uuid);
        uuid_stuff->uuid = (char *) uuid_command->uuid;
        return macho_seek_result_found;
    }
    return macho_seek_result_not_found;
}

OSKextRef __OSKextAlloc(
    CFAllocatorRef       allocator,
    CFAllocatorContext * context __unused)
{
    OSKextRef   result  = NULL;
    char      * offset  = NULL;
    UInt32      size;

    size  = sizeof(__OSKext) - sizeof(CFRuntimeBase);
    result = (OSKextRef)_CFRuntimeCreateInstance(allocator,
        __kOSKextTypeID, size, NULL);
    if (!result) {
        fprintf(stderr, "Out of memory\n");
        goto finish;
    }
    offset = (char *)result;
    bzero(offset + sizeof(CFRuntimeBase), size);

finish:
    return result;
}


Boolean
__OSKextCreateLoadInfo(OSKextRef aKext)
{
    Boolean result = false;

    if (!aKext->loadInfo)
    {
        aKext->loadInfo = (__OSKextLoadInfo *)
            malloc(sizeof(*(aKext->loadInfo)));
        if (!aKext->loadInfo)
        {
            perror("malloc");
            goto finish;
        }
        memset(aKext->loadInfo, 0, sizeof(*(aKext->loadInfo)));
    }
    result = true;
finish:
    return result;
}

static Boolean
__OSKextIsSplitKextMacho64(struct mach_header_64 *kextHeader)
{
    struct segment_command_64 *seg_cmd = NULL;
    if (!kextHeader)
        return false;

    seg_cmd = macho_get_segment_by_name_64(kextHeader, SEG_TEXT_EXEC);
    if (seg_cmd != NULL)
    {
        return true;
    }
    return false;
}

Boolean
__OSKextGetFileSystemPath(OSKextRef aKext,
                          CFURLRef  anURL,
                          Boolean   resolveToBase,
                          char     *pathBuffer)
{
    Boolean  result = false;
    CFURLRef urlToUse = NULL; // do not release

    if (aKext)
    {
        if (aKext->bundleURL)
        {
            urlToUse = aKext->bundleURL;
        }
    }
    else
    {
        urlToUse = anURL;
    }
    if (!urlToUse)
    {
        goto finish;
    }
    result = CFURLGetFileSystemRepresentation(urlToUse,
                                              resolveToBase,
                                              (UInt8 *) pathBuffer,
                                              PATH_MAX);

finish:
    if (!result)
    {
        fprintf(stderr, "failed to get filesystem path of %s\n", urlToUse);
        memcpy(pathBuffer, __kStringUnknown, sizeof(__kStringUnknown));
    }
    return result;
}

void
__OSKextDeallocateMmapBuffer(void *pointer, void *vInfo)
{
    __OSKextMmapBufferInfo *info = (__OSKextMmapBufferInfo *) vInfo;
    free(info);
}

static uint32_t
__OSKextGetSegMaxAlignment(struct segment_command_64 *seg_cmd)
{
    struct section_64 *sect = NULL;
    uint32_t           i = 0;
    uint32_t           result = KEXT_MIN_ALIGN;

    if (seg_cmd->cmd != LC_SEGMENT_64)
        goto finish;

    sect = (struct section_64 *) (&seg_cmd[1]);
    for (i = 0; i < seg_cmd->nsects; ++i, ++sect)
    {
        if (sect->align > result)
        {
            result = sect->align;
        }
    }

    if (!strncmp(seg_cmd->segname, SEG_TEXT_EXEC, sizeof(seg_cmd->segname)))
    {
        result = 12;
    }

    if (g_max_align_to_4k && result < 12)
        result = 12;

finish:
    return result;
}

static boolean_t
__OSKextGetSegmentInfo(const UInt8 *imagePtr,
                       const char  *segname,
                       uint64_t    *vmaddrOut,
                       uint64_t    *vmsizeOut,
                       uint64_t    *fileoffOut,
                       uint64_t    *filesizeOut,
                       uint64_t    *maxAlignOut,
                       boolean_t    truncateSegs)
{
    boolean_t result = false;
    uint64_t  max_off = 0;

    /* do the normal thing for LINKEDIT/LLVM_COV regardless of truncateSegs flag */
    if (truncateSegs && (!strcmp(segname, SEG_LINKEDIT) || !strcmp(segname, SEG_LLVM_COV)))
        truncateSegs = false;

    struct mach_header_64     *mach_header = (struct mach_header_64 *) imagePtr;
    struct segment_command_64 *seg = macho_get_segment_by_name_64(mach_header, segname);
    if (!seg)
    {
        goto finish;
    }

    if (maxAlignOut) *maxAlignOut = __OSKextGetSegMaxAlignment(seg);

    /* special case for __TEXT segment. max_off is at least size of mach header + load commands */
    if (truncateSegs && !strcmp(segname, "__TEXT"))
    {
        max_off = sizeof(*mach_header) + mach_header->sizeofcmds;
    }

    if (vmaddrOut)
        *vmaddrOut = seg->vmaddr;
    if (vmsizeOut)
    {
        if (truncateSegs)
        {
            /* walk the sections for this segment, compute vmsize based on highest used offset
             * instead of trusting what the segment header says.
             */
            struct section_64 *sect = (struct section_64 *) &seg[1];
            uint64_t           off, end_off;

            for (unsigned int i = 0; i < seg->nsects; ++i, ++sect)
            {
                off = sect->addr - seg->vmaddr;
                end_off = off + sect->size;
                max_off = (end_off > max_off) ? end_off : max_off;
            }

            *vmsizeOut = max_off;
        }
        else
        {
            *vmsizeOut = seg->vmsize;
        }
    }

    if (fileoffOut)
        *fileoffOut = seg->fileoff;
    
    if (filesizeOut)
    {
        if (truncateSegs)
        {
            /* previous truncateSegs code has to have run */
            assert(vmsizeOut);
            *filesizeOut = max_off;
        }
        else
        {
            *filesizeOut = seg->filesize;
        }
    }

    result = true;

finish:
    return result;
}

static uint64_t
__OSKextAlignAddress(uint64_t address, uint32_t align)
{
    uint64_t alignment = (1 << align);
    uint64_t low_bits = 0;

    if (align == 0) return address;

    low_bits = (address) & (alignment - 1);
    if (low_bits)
    {
        address += (alignment - low_bits);
    }

    return address;
}

boolean_t
__OSKextGetSegmentAddressAndOffsetDataRef(
    CFDataRef   imageRef,
    const char *segname,
    uint32_t   *fileOffsetOut,
    uint64_t   *loadAddrOut)
{
    boolean_t    result;
    const UInt8 *imagePtr = CFDataGetBytePtr(imageRef);

    result = __OSKextGetSegmentAddressAndOffset(imagePtr,
                                                segname,
                                                fileOffsetOut,
                                                loadAddrOut);

    return result;
}


boolean_t
__OSKextGetSegmentFileAndVMSize(
    const UInt8 *imagePtr,
    const char  *segname,
    uint64_t    *fileSizeOut,
    uint64_t    *VMSizeOut)
{
    boolean_t result = false;
    uint64_t  filesize = 0;
    uint64_t  vmsize = 0;

    struct mach_header_64     *mach_header = (struct mach_header_64 *) imagePtr;
    struct segment_command_64 *seg = macho_get_segment_by_name_64(mach_header,
                                                                  segname);
    if (!seg)
    {
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

boolean_t
__OSKextGetSegmentFileAndVMSizeDataRef(
    CFDataRef   imageRef,
    const char *segname,
    uint64_t   *fileSizeOut,
    uint64_t   *VMSizeOut)
{
    boolean_t    result;
    const UInt8 *imagePtr = CFDataGetBytePtr(imageRef);

    result = __OSKextGetSegmentFileAndVMSize(imagePtr,
                                             segname,
                                             fileSizeOut,
                                             VMSizeOut);

    return result;
}

static boolean_t
setKextVMAddr(OSKextRef aKext, enum enumSegIdx idx, uint64_t vmaddr)
{
    if (!aKext)
        return false;

    switch (idx)
    {
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

Boolean
__OSKextReadExecutable(OSKextRef aKext)
{
    Boolean                 result = false;
    char                    kextPath[PATH_MAX];
    char                    executablePath[PATH_MAX];
    char                   *executableBuffer = NULL;
    struct stat             statbuf;
    CFAllocatorContext      mmapAllocatorContext;
    CFAllocatorRef          mmapAllocator;
    __OSKextMmapBufferInfo *mmapAllocatorInfo;
    int                     fd = -1;
    int                     length = 0;

    if (!aKext || aKext->flags.declaresKernelExecutable == false)
        goto finish;

    if (aKext->loadInfo && aKext->loadInfo->executable)
    {
        return true;
    }
    else
    {
        if (!__OSKextCreateLoadInfo(aKext))
        {
            goto finish;
        }

        __OSKextGetFileSystemPath(aKext, NULL, true, kextPath);
        int pos = strlen(kextPath);
        memcpy(executablePath, kextPath, pos);
        memcpy(executablePath + pos, "/Contents/MacOS/", 16);
        pos += 16;
        char *base = basename(kextPath);
        int   i = strlen(base);
        while (i > 0 && *(base + i) != '.')
            --i;
        *(base + i) = '\0';
        memcpy(executablePath + pos, base, strlen(base));
        pos += strlen(base);
        *(executablePath + pos) = '\0';

        if (stat(executablePath, &statbuf) < 0)
        {
            pos = strlen(kextPath);
            memcpy(executablePath, kextPath, pos);
            memcpy(executablePath + pos, ".kext/", 6);
            pos += 6;
            memcpy(executablePath + pos, base, strlen(base));
            pos += strlen(base);
            *(executablePath + pos) = '\0';
        }

        if (stat(executablePath, &statbuf) < 0)
        {
            perror("stat");
            goto finish;
        }

        length = statbuf.st_size;
        fd = open(executablePath, O_RDONLY);
        executableBuffer = mmap(NULL,
                                length,
                                PROT_READ | PROT_WRITE,
                                MAP_FILE | MAP_PRIVATE,
                                fd,
                                0);

        CFAllocatorGetContext(kCFAllocatorDefault, &mmapAllocatorContext);
        mmapAllocatorInfo =
            (__OSKextMmapBufferInfo *) malloc(sizeof(__OSKextMmapBufferInfo));
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
    if (mmapAllocator)
        CFRelease(mmapAllocator);
    mmapAllocator = NULL;
    if (fd != -1)
        close(fd);

    if (!result)
    {
        if (executableBuffer != 0)
        {
            fprintf(stderr,
                    "Error encountered, unmapping executable file %s (%lu bytes).\n",
                    executablePath,
                    (unsigned long) length);
            munmap(executableBuffer, length);
        }
    }

    return result;
}

static Boolean
__OSKextPerformLink(OSKextRef    aKext,
                    CFDataRef    kernelImage,
                    uint64_t     kernelLoadAddress,
                    Boolean      stripSymbolsFlag,
                    KXLDContext *kxldContext)
{
    /* clang-format off */
    Boolean           result = false;
    char             *bundleIDCString = NULL;      // must free
    CFArrayRef        dependencies = NULL;         // must release
    CFMutableArrayRef indirectDependencies = NULL; // must release
    CFDataRef         kextExecutable = NULL;       // must release

    kern_return_t     kxldResult = KERN_FAILURE;
    KXLDDependency   *kxldDependencies = NULL; // must free
    CFIndex           numKxldDependencies = 0;
    kxld_addr_t       kmodInfoKern = 0;

    CFIndex           numDirectDependencies = 0;
    CFIndex           numIndirectDependencies = 0;

    __OSKextKXLDCallbackContext linkAddressContext;

    u_char           *relocBytes = NULL;    // do not free
    u_char          **relocBytesPtr = NULL; // do not free

    CFDataRef         relocData = NULL; // must release
    char              kextPath[PATH_MAX];
    CFIndex           i;
    /* clang-format on */

    /* Skip with success unless this kext has executable kernel code */
    if (aKext->flags.declaresKernelExecutable == false)
    {
        result = true;
        goto finish;
    }

    __OSKextGetFileSystemPath(aKext, NULL, true, kextPath);

    kxldDependencies = malloc(sizeof(KXLDDependency)*8);
    
    if (!__OSKextReadExecutable(aKext))
    {
        fprintf(stderr, "failed to read executable for %s\n", kextPath);
        goto finish;
    }
    kextExecutable = aKext->loadInfo->executable;

    /* FIXME: We probably need to handle dependencies properly here ...
              This is an ugly hack */
    kxldDependencies[0].kext = CFDataGetBytePtr(kernelImage);
    kxldDependencies[0].kext_size = CFDataGetLength(kernelImage);
    kxldDependencies[0].kext_name = "com.apple.kernel";
        kxldDependencies[0].is_direct_dependency = 1;
    ++numKxldDependencies;
    
    if (strstr(kextPath, "System.kext") != NULL)
    {
        aKext->flags.isInterface = true;
        kxldDependencies[0].interface = CFDataGetBytePtr(kextExecutable);
        kxldDependencies[0].interface_size = CFDataGetLength(kextExecutable);
        kxldDependencies[0].interface_name = CFStringGetCStringPtr(aKext->bundleID, kCFStringEncodingUTF8);
        kxldDependencies[0].is_direct_dependency = 1;
    }
    else
    {
        kxldDependencies[1].kext = CFDataGetBytePtr(kextExecutable);
        kxldDependencies[1].kext_size = CFDataGetLength(kextExecutable);
        kxldDependencies[1].kext_name = CFStringGetCStringPtr(aKext->bundleID, kCFStringEncodingUTF8);
        kxldDependencies[1].is_direct_dependency = 1;
        ++numKxldDependencies;
    }

    if (aKext->flags.isInterface == true)
    {
        aKext->loadInfo->linkedExecutable = CFRetain(kextExecutable);
        aKext->loadInfo->prelinkedExecutable = CFRetain(kextExecutable);
        aKext->loadInfo->linkInfo.linkedKextSize = CFDataGetLength(kextExecutable);
        result = true;
        goto finish;
    }

    bundleIDCString = createUTF8CStringForCFString(aKext->bundleID);

    relocBytesPtr = &relocBytes;

    linkAddressContext.kernelLoadAddress = kernelLoadAddress;
    linkAddressContext.kext = aKext;

    kxldResult = kxld_link_file(kxldContext,
                                (void *) CFDataGetBytePtr(kextExecutable),
                                CFDataGetLength(kextExecutable),
                                bundleIDCString,
                                /* callbackData */ (void *)&linkAddressContext,
                                kxldDependencies,
                                numKxldDependencies,
                                relocBytesPtr,
                                &kmodInfoKern);


    if (kxldResult != KERN_SUCCESS)
    {
        fprintf(stderr, "Link failed (error code %d).\n", kxldResult);
        goto finish;
    }

    if (relocBytes && aKext->loadInfo->linkInfo.linkedKextSize)
    {
        relocData = CFDataCreateWithBytesNoCopy(CFGetAllocator(aKext),
                                                relocBytes,
                                                aKext->loadInfo->linkInfo.linkedKextSize,
                                                kCFAllocatorDefault);
        if (!relocData)
        {
            goto finish;
        }
        aKext->loadInfo->linkedExecutable = CFRetain(relocData);
        aKext->loadInfo->kmodInfoAddress = kmodInfoKern;

        if (!aKext->loadInfo->prelinkedExecutable)
        {
            aKext->loadInfo->prelinkedExecutable = CFRetain(relocData);
        }
    } // relocBytes...

    result = true;

finish:
    if (kextExecutable)
        CFRelease(kextExecutable);
    if (relocData)
        CFRelease(relocData);
    if (bundleIDCString)
        CFRelease(bundleIDCString);
    return result;
}

CFMutableArrayRef
OSKextCopyLoadListForKexts(CFArrayRef kexts, Boolean needAll)
{
    CFMutableArrayRef result = NULL;
    CFMutableArrayRef globalLoadList = NULL;
    CFMutableSetRef   resolvedKexts = NULL;
    CFArrayRef        loadList = NULL;
    CFIndex           kextCount, loadListCount, i, j;

    /* Create a set to track the kexts whose dependencies have been resolved */

    resolvedKexts = CFSetCreateMutable(kCFAllocatorDefault, 0, &kCFTypeSetCallBacks);
    if (!resolvedKexts)
    {
        fprintf(stderr, "Out of memory\n");
        goto finish;
    }

    /* Create the global load list */

    globalLoadList = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    if (!globalLoadList)
    {
        fprintf(stderr, "Out of memory\n");
        goto finish;
    }

    /* Generate the global load list */

    kextCount = CFArrayGetCount(kexts);
    for (i = 0; i < kextCount; ++i)
    {
        Boolean valid = false;

        if (loadList)
            CFRelease(loadList);
        loadList = NULL;

        OSKextRef theKext = (OSKextRef) CFArrayGetValueAtIndex(kexts, i);

        /* If we've already determined this kext's load order, skip it.
        */
        if (CFSetGetValue(resolvedKexts, theKext)) continue;

        /* We are deliberately not handling dependencies here. You must
         * provide a full set of kexts to link
         * FIXME: Do this right ...
         */
        CFArrayAppendValue(globalLoadList, theKext);
        CFSetSetValue(resolvedKexts, theKext);
    }

    result = globalLoadList;
    globalLoadList = NULL;

finish:
    if (resolvedKexts)
        CFRelease(resolvedKexts);
    if (globalLoadList)
        CFRelease(globalLoadList);
    if (loadList)
        CFRelease(loadList);

    return result;
}

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
                     Boolean      stripSymbolsFlag)
{
    CFArrayRef        result = NULL;
    boolean_t         success = false;
    CFMutableArrayRef loadList = NULL;
    uint64_t          loadAddr = loadAddrBase;
    uint64_t          sourceAddr = sourceAddrBase;
    u_long            loadSize = 0;
    char             *kextIdentifierCString = NULL; // must free
    CFIndex           i;

    loadList = OSKextCopyLoadListForKexts(kextArray, false);
    if (!loadList)
    {
        fprintf(stderr, "failed to build loadlist\n");
        goto finish;
    }

    /* Link each kext in the load list */
    for (i = 0; i < CFArrayGetCount(loadList); ++i)
    {
        OSKextRef aKext = (OSKextRef) CFArrayGetValueAtIndex(loadList, i);

        if (kextIdentifierCString)
            free(kextIdentifierCString);
        kextIdentifierCString = NULL;

        if (aKext->flags.declaresKernelExecutable == false)
            continue;

        kextIdentifierCString =
            createUTF8CStringForCFString(aKext->bundleID);

        /* Set the load address of the kext.
        */

        loadAddr = (loadAddrBase + loadSize + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        sourceAddr = sourceAddrBase + loadSize;

        __OSKextCreateLoadInfo(aKext);
        aKext->loadInfo->linkInfo.vmaddr_TEXT = loadAddr;
        aKext->loadInfo->sourceAddress = sourceAddr;

        success = __OSKextPerformLink(aKext, kernelImage, 0, false, kxldContext);

        if (!success)
        {
            fprintf(stderr, "Prelink failed at %s\n", kextIdentifierCString);
            goto finish;
        }

        loadSize += aKext->loadInfo->linkInfo.linkedKextSize;
        loadSize += (PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    } // kext loadList for loop

    result = CFRetain(loadList);

    *loadSizeOut = loadSize;

finish:
    if (loadList)
        CFRelease(loadList);
    if (kextIdentifierCString)
        free(kextIdentifierCString);
    return result;
}

static boolean_t
__OSKextGetSegmentAddressAndOffset(const UInt8 *imagePtr,
                                   const char  *segname,
                                   uint32_t    *fileOffsetOut,
                                   uint64_t    *loadAddrOut)
{
    boolean_t result = false;
    uint32_t  fileOffset = 0;
    uint64_t  loadAddr = 0;

    struct mach_header_64     *mach_header = (struct mach_header_64 *) imagePtr;
    struct segment_command_64 *seg = macho_get_segment_by_name_64(mach_header, segname);
    if (!seg)
    {
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

#include "../IOKitUser/IOCFUnserialize.tab.c"

Boolean
_GetStringProperty(OSKextRef    aKext,
                   CFStringRef  propKey,
                   CFStringRef *valueOut)
{
    if (CFStringHasPrefix(propKey, CFSTR("OS")) || CFStringHasPrefix(propKey, CFSTR("IO")))
    {
        const NXArchInfo *lookupArchInfo = NXGetArchInfoFromCpuType(CPU_TYPE_X86_64,
                                                                    CPU_SUBTYPE_MULTIPLE);

        if (lookupArchInfo)
        {
            CFTypeRef   result = NULL;
            CFStringRef compositeKey = CFStringCreateWithFormat(CFGetAllocator(aKext),
                                                                NULL,
                                                                CFSTR("%s_%s"),
                                                                propKey,
                                                                lookupArchInfo->name);
            if (!compositeKey)
            {
                perror("calloc");
                return false;
            }

            result = CFDictionaryGetValue(aKext->infoDictionary, compositeKey);
            if (!result)
            {
                result = CFDictionaryGetValue(aKext->infoDictionary, propKey);
            }

            CFRelease(compositeKey);

            if (valueOut)
            {
                *valueOut = result;
            }

            return true;
        }
        else
        {
            fprintf(stderr, "failed to get arch info\n");
            return false;
        }
    }
    else
    {
        CFStringRef value = CFDictionaryGetValue(aKext->infoDictionary, propKey);
        if (valueOut)
        {
            *valueOut = value;
        }

        return (value != NULL);
    }

    fprintf(stderr, "you shouldn't be here, friend\n");
    return false;
}

Boolean
_GetBooleanProperty(OSKextRef     aKext,
                    CFStringRef   propKey,
                    CFBooleanRef *valueOut)
{
    CFBooleanRef value = CFDictionaryGetValue(aKext->infoDictionary, propKey);
    if (!value)
    {
        return false;
    }
    if (valueOut)
    {
        *valueOut = value;
    }

    return true;
}

CFDataRef
__OSKextCreatePrelinkInfoDictionary(plkInfo   *plkInfo,
                                    CFArrayRef loadList,
                                    CFURLRef   volumeRootURL,
                                    Boolean    includeAllPersonalities,
                                    Boolean    isSplitKexts,
                                    CFDataRef  kernelUUID)
{
    CFDataRef              result = NULL; // do not release

    char                   kextPath[PATH_MAX] = "";
    char                   volumePath[PATH_MAX] = "";
    CFArrayRef             allKextsByBundleID = NULL;   // must release
    CFArrayRef             kextPersonalities = NULL;    // must release
    CFMutableArrayRef      kextInfoDictArray = NULL;    // must release
    CFDataRef              prelinkInfoData = NULL;      // must release
    CFDataRef              uuid = NULL;                 // must release
    CFMutableDictionaryRef kextInfoDict = NULL;         // must release
    CFMutableDictionaryRef prelinkInfoDict = NULL;      // must release
    CFNumberRef            cfnum = NULL;                // must release
    CFStringRef            bundleVolPath = NULL;        // must release
    CFStringRef            executableRelPath = NULL;    // must release
    CFStringRef            archPersonalitiesKey = NULL; // must release
    CFSetRef               loadListIDs = NULL;          // must release
    char                  *kextVolPath = NULL;          // do not free
    int                    i = 0;
    int                    count = 0;
    EVP_MD_CTX            *ctx = NULL;
    unsigned char          kernelCacheHash[SHA256_DIGEST_LENGTH];
    CFDataRef              kcID = NULL; // must release

    /* Create a dictionary for all prelinked kernel metadata */

    prelinkInfoDict = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                                0,
                                                &kCFTypeDictionaryKeyCallBacks,
                                                &kCFTypeDictionaryValueCallBacks);
    if (!prelinkInfoDict)
    {
        perror("prelinkInfoDict");
        goto finish;
    }

    /* Create an array to hold all of the info dictionaries */

    kextInfoDictArray = CFArrayCreateMutable(kCFAllocatorDefault,
                                             CFArrayGetCount(loadList),
                                             &kCFTypeArrayCallBacks);
    if (!kextInfoDictArray)
    {
        perror("kextInfoDictArray");
        goto finish;
    }

    CFDictionarySetValue(prelinkInfoDict, CFSTR(kPrelinkInfoDictionaryKey), kextInfoDictArray);

    ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);

    if (kernelUUID)
    {
        EVP_DigestUpdate(ctx, (unsigned char *) CFDataGetBytePtr(kernelUUID), CFDataGetLength(kernelUUID));
    }

    /* Create an info dictionary for each kext in the load list */

    count = CFArrayGetCount(loadList);
    for (i = 0; i < count; ++i)
    {
        Boolean   gotPath = FALSE;
        OSKextRef aKext = (OSKextRef) CFArrayGetValueAtIndex(loadList, i);

        if (kextInfoDict)
            CFRelease(kextInfoDict);
        kextInfoDict = NULL;
        if (bundleVolPath)
            CFRelease(bundleVolPath);
        bundleVolPath = NULL;

        __OSKextGetFileSystemPath(aKext, /* otherURL */ NULL,
                                  /* resolveToBase */ true,
                                  kextPath);

        /* Get the existing info dictionary from the kext */
        kextInfoDict = aKext->infoDictionary;

        CFMutableDictionaryRef result = NULL;
        result = CFDictionaryCreateMutableCopy(CFGetAllocator(aKext), 0, kextInfoDict);

        if (!kextInfoDict)
        {
            perror("kextInfoDict");
            goto finish;
        }

        /* Add the load address, source address, and kmod info address information.
         */
        if (aKext->flags.declaresKernelExecutable == true)
        {
            cfnum = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &aKext->loadInfo->linkInfo.vmaddr_TEXT);
            if (!cfnum)
            {
                perror("malloc");
                goto finish;
            }
            CFDictionarySetValue(kextInfoDict, CFSTR(kPrelinkExecutableLoadKey), cfnum);
            if (cfnum)
                CFRelease(cfnum);
            cfnum = NULL;

            cfnum = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &aKext->loadInfo->sourceAddress);

            if (!cfnum)
            {
                perror("malloc");
                goto finish;
            }
            CFDictionarySetValue(kextInfoDict, CFSTR(kPrelinkExecutableSourceKey), cfnum);
            if (cfnum)
                CFRelease(cfnum);
            cfnum = NULL;

            u_long num;
            num = CFDataGetLength(aKext->loadInfo->prelinkedExecutable);
            cfnum = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &num);
            if (!cfnum)
            {
                perror("malloc");
                goto finish;
            }
            CFDictionarySetValue(kextInfoDict, CFSTR(kPrelinkExecutableSizeKey), cfnum);
            if (cfnum)
                CFRelease(cfnum);
            cfnum = NULL;

            // Update the KC ID based on the start address of the KEXT
            EVP_DigestUpdate(ctx, &aKext->loadInfo->linkInfo.vmaddr_TEXT, sizeof(aKext->loadInfo->linkInfo.vmaddr_TEXT));

            cfnum = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &aKext->loadInfo->kmodInfoAddress);
            if (!cfnum)
            {
                perror("malloc");
                goto finish;
            }
            CFDictionarySetValue(kextInfoDict, CFSTR(kPrelinkKmodInfoKey), cfnum);
            if (cfnum)
                CFRelease(cfnum);
            cfnum = NULL;
        }

        /* Add this info dictionary to the info dict array */
        CFArrayAppendValue(kextInfoDictArray, kextInfoDict);
    }

    int lenp = 0;
    EVP_DigestFinal_ex(ctx, (unsigned char *) &kernelCacheHash, &lenp);
    kcID = CFDataCreate(kCFAllocatorDefault, kernelCacheHash, sizeof(uuid_t));
    if (kcID)
    {
        /* Add the kernelcache ID */
        CFDictionarySetValue(prelinkInfoDict, CFSTR(kPrelinkInfoKCIDKey), kcID);

        fprintf(stdout, "KernelCache ID: ");
        for (unsigned long i = 0; i < sizeof(kernelCacheHash) / 2; i++)
        {
            fprintf(stdout, "%02X", kernelCacheHash[i]);
        }
        fprintf(stdout, "\n");
    }
    else
    {
        fprintf(stderr, "Failed to allocate kernelcache ID\n");
    }

    /* Serialize the info dictionary */

    prelinkInfoData = IOCFSerialize(prelinkInfoDict, kNilOptions);
    if (!prelinkInfoData)
    {
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

CFMutableDictionaryRef
OSKextCopyInfoDictionary(OSKextRef aKext)
{
    CFMutableDictionaryRef result = NULL;

    if (aKext->infoDictionary)
    {
        result = CFDictionaryCreateMutableCopy(CFGetAllocator(aKext), 0, aKext->infoDictionary);
    }

finish:
    return result;
}


Boolean
__OSKextReadInfoDictionary(OSKextRef   aKext,
                           CFBundleRef kextBundle)
{
    Boolean     result = false;
    struct stat statbuf;
    char       *infoDictXML = NULL; // must free
    int         fd = -1;            // must close
    ssize_t     totalBytesRead;
    CFStringRef errorString = NULL; // must release
    char        kextPath[PATH_MAX];
    char        infoDictPath[PATH_MAX];

    __OSKextGetFileSystemPath(aKext, /* otherURL */ NULL,
                              /* resolveToBase */ false,
                              kextPath);

    if (aKext->infoDictionary)
    {
        result = true;
        goto finish;
    }

    if (strstr(kextPath, "PlugIns"))
    {
        sprintf(infoDictPath, "%s/Info.plist", kextPath);
        aKext->staticFlags.isPlugin = 1;
    }
    else
    {
        sprintf(infoDictPath, "%s/Contents/Info.plist", kextPath);
        if (stat(infoDictPath, &statbuf) < 0)
        {
            sprintf(infoDictPath, "%s/Info.plist", kextPath);
        }
    }

    if (stat(infoDictPath, &statbuf) < 0)
    {
        fprintf(stderr, "%s has no Info.plist file.\n", kextPath);
        goto finish;
    }

    fd = open(infoDictPath, O_RDONLY);
    if (fd < 0)
    {
        perror("infoDict: open");
        goto finish;
    }

    infoDictXML = (char *) malloc((1 + statbuf.st_size) * sizeof(char));
    if (!infoDictXML)
    {
        /* XXX - Basically hosed if this happens. */
        perror("infoDict: malloc");
        goto finish;
    }

    for (totalBytesRead = 0; totalBytesRead < statbuf.st_size; /* nothing */)
    {
        ssize_t bytesRead = read(fd, infoDictXML + totalBytesRead, statbuf.st_size - totalBytesRead);
        if (bytesRead < 0)
        {
            perror("infoDict: read");
            goto finish;
        }
        totalBytesRead += bytesRead;
    }

    infoDictXML[totalBytesRead] = '\0';

    CFDictionaryRef dict = (CFDictionaryRef) IOCFUnserialize(
        (const char *) infoDictXML, CFGetAllocator(aKext), 0, &errorString);

    if (!dict || CFDictionaryGetTypeID() != CFGetTypeID(dict))
    {
        /* This is a full abort! Issue the abort codes right away and log this. */
        fprintf(stderr, "Can't read info dictionary for %s: %s.\n", kextPath, CFStringGetCStringPtr(errorString, kCFStringEncodingUTF8));
        goto finish;
    }
    aKext->infoDictionary = CFRetain(dict);

    result = true;

finish:
    if (infoDictXML)
        free(infoDictXML);
    if (errorString)
        CFRelease(errorString);

    if (fd >= 0)
    {
        close(fd);
    }

    if (!result)
    {
        aKext->flags.invalid = 1;
        aKext->flags.valid = 0;
    }
    return result;
}

OSKextVersion
OSKextParseVersionString(const char *versionString)
{
    OSKextVersion result = -1;
    int           vers_digit = -1;
    int           num_digits_scanned = 0;
    OSKextVersion vers_major = 0;
    OSKextVersion vers_minor = 0;
    OSKextVersion vers_revision = 0;
    OSKextVersion vers_stage = 0;
    OSKextVersion vers_stage_level = 0;
    char         *current_char_p;
    const char   *start_of_segment;

    if (!versionString || *versionString == '\0')
    {
        return -1;
    }

    start_of_segment = (const char *) &versionString[0];
    current_char_p = (const char *) &versionString[0];

    /* find first period in string */
    while (*current_char_p && *current_char_p != '.')
        ++current_char_p;
    *current_char_p = '\0';
    sscanf(start_of_segment, "%d", &vers_major);

    start_of_segment = current_char_p + 1; /* skip '.' */

    while (*current_char_p && *current_char_p != '.')
        ++current_char_p;
    *current_char_p = '\0';
    sscanf(start_of_segment, "%d", &vers_minor);

    start_of_segment = current_char_p + 1; /* skip '.' */

    while (*current_char_p && *current_char_p >= '0' && *current_char_p <= '9')
        ++current_char_p;
    *current_char_p = '\0';
    sscanf(start_of_segment, "%d", &vers_revision);

    vers_stage = 1; // development stage. 3=alpha, 5=beta, RC=7, release=9
    vers_stage_level = 1;

finish:

    result = (vers_major * VERS_MAJOR_MULT) +
        (vers_minor * VERS_MINOR_MULT) +
        (vers_revision * VERS_REVISION_MULT) +
        (vers_stage * VERS_STAGE_MULT) +
        vers_stage_level;

    return result;
}

Boolean
__OSKextProcessInfoDictionary(OSKextRef   aKext,
                              CFBundleRef kextBundle)
{
    CFBooleanRef boolValue = NULL;   // do not release
    CFStringRef  stringValue = NULL; // do not release
    Boolean       isInterfaceSetFalse = false;
    OSKextVersion bundleVersion = -1;
    OSKextVersion compatibleVersion = -1;

    if (!__OSKextReadInfoDictionary(aKext, kextBundle))
    {
        return false;
    }

    /* The real version of this does a lot of checking. We skip all that.
       Just retrieve the keys from the kext infoDict and go. */

    _GetStringProperty(aKext, CFSTR("CFBundleIdentifier"), &stringValue);
    aKext->bundleID = CFRetain(stringValue);

    _GetStringProperty(aKext, CFSTR("CFBundleVersion"), &stringValue);

    /* clang-format off */
    aKext->version = OSKextParseVersionString(
                         CFStringGetCStringPtr(stringValue, kCFStringEncodingUTF8));

    _GetStringProperty(aKext, CFSTR("OSBundleCompatibleVersion"), &stringValue);
    aKext->compatibleVersion = OSKextParseVersionString(
                               CFStringGetCStringPtr(stringValue, kCFStringEncodingUTF8));
    /* clang-format on */

    /* FIXME: need this to be real */
    aKext->flags.isKernelComponent = 0;
    aKext->flags.isInterface = 0;
    aKext->flags.declaresKernelExecutable = 0;

    _GetStringProperty(aKext, CFSTR("CFBundleExecutable"), &stringValue);
    if (stringValue)
    {
        char path[PATH_MAX];
        CFStringGetCString(stringValue, path, PATH_MAX - 1, kCFStringEncodingUTF8);
        if (path[0])
            aKext->flags.declaresKernelExecutable = 1;
    }

    /* No logging for you! */
    aKext->flags.loggingEnabled = 0;
    aKext->flags.plistHasEnableLoggingSet = 0;

    aKext->flags.isLoadableInSafeBoot = 0; /* we ignore this anyway */
    aKext->flags.invalid = 0;
    aKext->flags.valid = 1;

    return true;
}

Boolean
__OSKextInitWithPath(OSKextRef   aKext,
                     const char *kextPath)
{
    Boolean     result = false;
    CFBundleRef kextBundle = NULL; // must release

    CFURLRef urlPath = CFURLCreateFromFileSystemRepresentation(CFGetAllocator(aKext),
                                                               kextPath,
                                                               strlen(kextPath),
                                                               false /* isDirectory */);
    if (!urlPath)
    {
        fprintf(stderr, "Failed to create CFURL for %s\n", kextPath);
        goto finish;
    }
    kextBundle = CFBundleCreate(CFGetAllocator(aKext), urlPath);
    if (!kextBundle)
    {
        fprintf(stderr, "Can't open CFBundle for %s.\n", kextPath);
        goto finish;
    }

    /* Save the URL only after we've confirmed we can open a bundle there.
    * See __OSKextRemoveKext().
    */
    aKext->bundleURL = CFRetain(urlPath);

    /* If we can't get the info dictionary at all, we don't even
    * have an examinable broken kext.
    */
    if (!__OSKextReadInfoDictionary(aKext, kextBundle))
    {
        goto finish;
    }

    /* Don't worry about the return value of this; we want to be
    * able to open bad kexts to do further diagnostics. It's up
    * to the client to close out unusable kexts.
    */
    __OSKextProcessInfoDictionary(aKext, kextBundle);
    result = true;

finish:
    if (kextBundle)
    {
        CFRelease(kextBundle);
    }
    return result;
}
