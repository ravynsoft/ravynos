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
 */

#include "plktool.h"

char *
createUTF8CStringForCFString(CFStringRef aString)
{
    char   *result = NULL;
    CFIndex bufferLength = 0;

    if (!aString)
    {
        goto finish;
    }

    bufferLength = sizeof('\0') +
        CFStringGetMaximumSizeForEncoding(CFStringGetLength(aString),
                                          kCFStringEncodingUTF8);

    result = (char *) malloc(bufferLength * sizeof(char));
    if (!result)
    {
        goto finish;
    }
    if (!CFStringGetCString(aString, result, bufferLength, kCFStringEncodingUTF8))
    {
        if (result)
            CFRelease(result);
        result = NULL;
    }

finish:
    return result;
}

static boolean_t
SwapHeaders(CFDataRef kernelImage)
{
    u_char *file = (u_char *) CFDataGetBytePtr(kernelImage);
    return macho_swap_64(file);
}

static boolean_t
UnswapHeaders(CFDataRef kernelImage)
{
    u_char *file = (u_char *) CFDataGetBytePtr(kernelImage);
    return macho_unswap_64(file);
}


/* FIXME: remove? not used */

static uint64_t
getKCPlkSegNextVMAddr(plkInfo *plkInfo, enum enumSegIdx idx)
{
    assert(plkInfo);

    switch (idx)
    {
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

static boolean_t
setKCPlkSegNextVMAddr(plkInfo *plkInfo, enum enumSegIdx idx, uint64_t x)
{
    if (!plkInfo)
        return false;

    switch (idx)
    {
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

static u_long
CopyPrelinkedKexts(CFMutableDataRef prelinkImage,
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
    struct mach_header_64     *mach_header = (struct mach_header_64 *) CFDataGetBytePtr(prelinkImage);
    struct segment_command_64 *seg = macho_get_segment_by_name_64(mach_header,
                                                                  kPrelinkTextSegment);
    if (!seg)
    {
        goto finish;
    }
    seg->vmaddr = sourceAddrBase;
    seg->fileoff = fileOffsetBase;

    struct section_64 *sect = macho_get_section_by_name_64(mach_header,
                                                           kPrelinkTextSegment,
                                                           kPrelinkTextSection);
    if (!sect)
    {
        goto finish;
    }
    sect->addr = sourceAddrBase;
    sect->offset = fileOffset;

    /* Copy all kext executables */

    int rounded = 0;
    for (i = 0; i < CFArrayGetCount(loadList); ++i)
    {
        OSKextRef aKext = (OSKextRef) CFArrayGetValueAtIndex(loadList, i);

        if (aKext->flags.declaresKernelExecutable == false)
        {
            continue;
        }

        /* xxx - Is it safe to assume aKext->loadInfo exists here?
        */
        size = CFDataGetLength(aKext->loadInfo->prelinkedExecutable);
        rounded = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        
        CFDataAppendBytes(prelinkImage,
                          CFDataGetBytePtr(aKext->loadInfo->prelinkedExecutable),
                          size);
        CFDataIncreaseLength(prelinkImage, rounded - size);
        
        sourceAddr += rounded;
        fileOffset += rounded;
        totalSize += rounded;
    }

    /* Set the text segment and section size */

    seg->vmsize = rounded;
    seg->filesize = rounded;
    sect->size = rounded;

    return totalSize;

finish:
    fprintf(stderr, "Failed to find segment or section\n");
    return -1;
}


static boolean_t
GetLastKernelLoadAddr(CFDataRef kernelImage,
                      uint64_t *lastLoadAddrOut)
{
    boolean_t            result         = false;
    const UInt8        * kernelImagePtr = CFDataGetBytePtr(kernelImage);
    uint64_t             lastLoadAddr   = 0;
    uint64_t             i;

    struct mach_header_64 *kernel_header =
        (struct mach_header_64 *) kernelImagePtr;
    struct segment_command_64 *seg_cmd =
        (struct segment_command_64 *) ((uintptr_t) kernel_header + sizeof(*kernel_header));

    for (i = 0; i < kernel_header->ncmds; i++)
    {
        if (seg_cmd->cmd == LC_SEGMENT_64)
        {
            if (seg_cmd->vmaddr + seg_cmd->vmsize > lastLoadAddr)
            {
                lastLoadAddr = seg_cmd->vmaddr + seg_cmd->vmsize;
            }
        }
        seg_cmd = (struct segment_command_64 *) ((uintptr_t) seg_cmd + seg_cmd->cmdsize);
    }

    if (lastLoadAddrOut)
        *lastLoadAddrOut = lastLoadAddr;
    result = true;

    return result;
}


CFDataRef
CreatePrelinkedKernel(CFDataRef  kernelImage,
                      CFArrayRef kextArray)
{
    plkInfo                      plkInfo;
    CFMutableDataRef             prelinkImage = NULL;
    CFArrayRef                   loadList = NULL;
    CFDataRef                    kernelUUID = NULL;
    uintptr_t                    baseFileOffset = 0;
    uintptr_t                    baseLoadAddr = 0;
    uintptr_t                    baseSrcAddr = 0;
    const struct mach_header_64 *mach_header;
    const unsigned char         *file_end;
    KXLDContext                 *kxldContext;
    u_long                       size = 0;
    uintptr_t                    textLoadAddr;
    uintptr_t                    textVMSize;


    baseFileOffset = CFDataGetLength(kernelImage);
    memset(&plkInfo, 0, sizeof(plkInfo));

    kxld_create_context(&kxldContext,
                        __OSKextLinkAddressCallback,
                        __OSKextLoggingCallback,
                        kKxldFlagDefault | kKXLDFlagIncludeRelocs,
                        CPU_TYPE_X86_64,
                        CPU_SUBTYPE_X86_64_ALL,
                        0);

    boolean_t swapped = SwapHeaders(kernelImage);
    if (!GetLastKernelLoadAddr(kernelImage, &baseSrcAddr))
        goto failed;
    baseSrcAddr = ((baseSrcAddr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));

    boolean_t success = __OSKextGetSegmentAddressAndOffsetDataRef(kernelImage,
                                                                  SEG_TEXT,
                                                                  NULL,
                                                                  &textLoadAddr);
    if (!success)
    {
        fprintf(stderr, "Could not get kernel text address.\n");
        goto failed;
    }

    success = __OSKextGetSegmentFileAndVMSizeDataRef(kernelImage,
                                                     SEG_TEXT,
                                                     NULL,
                                                     &textVMSize);
    if (!success)
    {
        fprintf(stderr, "Could not get kernel text vmsize.\n");
        goto failed;
    }


    baseLoadAddr = baseSrcAddr;

    loadList = __OSKextPrelinkKexts(kextArray,
                                    kernelImage,
                                    baseLoadAddr, // where we load kext __TEXT segments
                                    baseSrcAddr,
                                    kxldContext,
                                    &size,
                                    true,   /* (flags & kOSKextKernelcacheNeedAllFlag), */
                                    true,   /* (flags & kOSKextKernelcacheSkipAuthenticationFlag), */
                                    true,   /* (flags & kOSKextKernelcachePrintDiagnosticsFlag), */
                                    false); /* (flags & kOSKextKernelcacheStripSymbolsFlag)); */ /* if this is true, do we have to provide the callback? */

    if (!loadList)
        goto failed;

    mach_header = (const struct mach_header_64 *) CFDataGetBytePtr(kernelImage);
    file_end = (((const char *) mach_header) + CFDataGetLength(kernelImage));
    struct _uuid_stuff seek_uuid;
    macho_seek_result  seek_result = macho_scan_load_commands(mach_header,
                                                              file_end,
                                                              __OSKextUUIDCallback,
                                                              (const void **) &seek_uuid);
    if (seek_result == macho_seek_result_found)
    {
        kernelUUID = CFDataCreate(kCFAllocatorDefault,
                                  (u_char *) seek_uuid.uuid,
                                  swapped
                                    ? bswap_32(seek_uuid.uuid_size)
                                    : seek_uuid.uuid_size);
    }

    CFDataRef prelinkInfoData =
        __OSKextCreatePrelinkInfoDictionary(&plkInfo,
                                            loadList,
                                            NULL, /* volumeRootURL */
                                            1,    /* flags: gotta link em all */
                                            0,    /* isARM64 */
                                            kernelUUID);

    /* create the prelink image in a CFData */
    u_long kernlen = CFDataGetLength(kernelImage);
    prelinkImage = CFDataCreateMutable(kCFAllocatorDefault, 0);
    CFDataSetLength(prelinkImage, kernlen);

    CFRange  range = CFRangeMake(0, kernlen);
    uint8_t *bytePtr = CFDataGetBytePtr(kernelImage);
    CFDataReplaceBytes(prelinkImage, range, bytePtr, kernlen);

    uintptr_t fileOffset = kernlen;
    uintptr_t srcAddr = baseSrcAddr;

    size = CopyPrelinkedKexts(prelinkImage,
                              loadList,
                              fileOffset,
                              srcAddr);
    fileOffset += (size + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);
    srcAddr += (size + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);
    uintptr_t loadAddr = (baseLoadAddr + size + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);

    CFDataSetLength(prelinkImage, fileOffset); // round to page

    u_long pdsize = 0;
    pdsize = CFDataGetLength(prelinkInfoData);
    CFDataAppendBytes(prelinkImage, CFDataGetBytePtr(prelinkInfoData), pdsize);

    /* Set the info dictionary segment headers */
    mach_header = (struct mach_header_64 *) CFDataGetBytePtr(prelinkImage);
    struct segment_command_64 *seg = macho_get_segment_by_name_64(mach_header, kPrelinkInfoSegment);
    if (!seg)
    {
        fprintf(stderr, "no seg\n");
        goto failed;
    }

    seg->vmaddr = loadAddr;
    seg->vmsize = pdsize;
    seg->fileoff = fileOffset;
    seg->filesize = pdsize;

    struct section_64 *sect = macho_get_section_by_name_64(mach_header,
                                                           kPrelinkInfoSegment,
                                                           kPrelinkInfoSection);
    if (!sect)
    {
        fprintf(stderr, "no sect\n");
        goto failed;
    }

    sect->addr = loadAddr;
    sect->offset = fileOffset;
    sect->size = pdsize;

    CFDataRef result = CFRetain(prelinkImage);

failed:
    kxld_destroy_context(kxldContext);
    return result;
}


int
main(int argc, char **argv)
{
    struct stat st;
    int         kernfd = -1;
    char       *kernelbuf = NULL;
    int         kernlen = 0;
    CFDataRef   kernelImage = NULL;
    CFArrayRef  kextArray = NULL;
    int         result = 1; /* assume something will fail */

    if (argc < 4)
    {
        fprintf(stdout, "plktool kernelcache kernel kext [kext ...]\n");
        return 1;
    }

    fprintf(stdout, "Examining kernel image\n");
    if (stat(argv[2], &st) < 0)
    {
        perror("stat");
        goto finish;
    }
    kernlen = st.st_size;

    kernfd = open(argv[2], O_RDWR);
    if (kernfd < 0)
    {
        perror("open");
        goto finish;
    }

    kernelbuf = mmap(0, kernlen, PROT_READ | PROT_WRITE, MAP_PRIVATE, kernfd, 0);
    if (!kernelbuf)
    {
        perror("mmap");
        goto finish;
    }

    kernelImage = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault,
                                              kernelbuf,
                                              kernlen,
                                              NULL);
    if (!kernelImage)
    {
        fprintf(stderr, "Failed to create CFData for kernel image\n");
        goto finish;
    }
    CFRetain(kernelImage);

    kextArray = CFArrayCreateMutable(kCFAllocatorDefault,
                                     argc,
                                     &kCFTypeArrayCallBacks);
    if (!kextArray)
    {
        fprintf(stderr, "Failed to create CFArray for kext images\n");
        goto finish;
    }

    __kOSKextTypeID = _CFRuntimeRegisterClass(&__OSKextClass);

    /* now load the kexts from the arg list */
    for (int i = 3; i < argc; ++i)
    {
        if (stat(argv[i], &st) < 0)
        {
            fprintf(stderr, "Skipping %s - stat error %s\n", argv[i], strerror(errno));
            continue;
        }

        __OSKextRef theKext = __OSKextAlloc(kCFAllocatorDefault, /* context */ NULL);
        if (!theKext)
        {
            fprintf(stderr, "Failed to create OSKext for %s\n", argv[i]);
            goto finish;
        }
        if (!__OSKextInitWithPath(theKext, argv[i]))
        {
            CFRelease(theKext);
            theKext = NULL;
            goto finish;
        }

        CFArrayAppendValue(kextArray, CFRetain(theKext));
    }

    fprintf(stdout, "Linking ...\n");
    CFDataRef prelinkImage = CreatePrelinkedKernel(kernelImage, kextArray);
    if (!prelinkImage)
    {
        fprintf(stderr, "Failed to create prelinked kernel.\n");
        goto finish;
    }

    int fd_out = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    if (fd_out < 0)
    {
        perror("open");
        goto finish;
    }

    size_t prelinkLength = CFDataGetLength(prelinkImage);

    PrelinkedKernelHeader pkh = {
        htonl(0x636f6d70),    /* signature 'comp' */
        htonl(0x6e756c6c),    /* compressType 'null' */
        0,                    /* adler32 */
        htonl(prelinkLength), /* uncompressedSize */
        htonl(prelinkLength), /* compressedSize */
        htonl(1),             /* prelinkVersion */
        0,                    /* reserved */
        0,                    /* platformName */
        0,                    /* rootPath */
        0                     /* data */
    };

    size_t bytes = write(fd_out, &pkh, sizeof(pkh));
    bytes += write(fd_out, CFDataGetBytePtr(prelinkImage), CFDataGetLength(prelinkImage));
    close(fd_out);

    fprintf(stdout, "Wrote %d bytes to %s\nFinished!\n", bytes, argv[1]);

finish:
    if (kernelbuf)
        munmap(kernelbuf, kernlen);

    if (kernfd >= 0)
        close(kernfd);

    return result;
}
