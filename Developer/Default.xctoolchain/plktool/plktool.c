/**
 * plktool.c
 * Author: Zoe Knox      Created: 2026-03-02
 *
 * Copyright (C) 2026 ravynOS Project. All rights reserved.
 * Portions Copyright (c) 2008, 2012 Apple Inc.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code as
 * defined in and that are subject to the Apple Public Source License Version
 * 2.0 (the 'License').  You may not use this file except in compliance with
 * the License.  Please obtain a copy of the License at
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

extern CFMutableArrayRef __sOSAllKexts;

const uint64_t __kOSKextMaxDisplacement = 2 * 1024 * 1024 * 1024ULL;

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
    for (i = 0; i < CFArrayGetCount(loadList); ++i)
    {
        OSKextRef aKext = (OSKextRef) CFArrayGetValueAtIndex(loadList, i);

        if (aKext->flags.declaresKernelExecutable == false)
        {
            continue;
        }

        memcpy(prelinkData + fileOffset + size, 
               CFDataGetBytePtr(aKext->loadInfo->prelinkedExecutable),
               CFDataGetLength(aKext->loadInfo->prelinkedExecutable));
        
        size += (CFDataGetLength(aKext->loadInfo->prelinkedExecutable) +
                        PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    }

    sourceAddr += size;
    fileOffset += size;
    totalSize += size;

    /* Set the text segment and section size */

    seg->vmsize = totalSize;
    seg->filesize = totalSize;
    sect->size = totalSize;

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
    uint32_t                     baseFileOffset = 0;
    uint64_t                     baseLoadAddr = 0;
    uint64_t                     baseSrcAddr = 0;
    const struct mach_header_64 *mach_header;
    const unsigned char         *file_end;
    KXLDContext                 *kxldContext;
    u_long                       size = 0;
    uint64_t                     textLoadAddr;
    uint64_t                     textVMSize;
    CFDataRef                    result = NULL;

    baseFileOffset = CFDataGetLength(kernelImage);

    kxld_create_context(&kxldContext,
                        __OSKextLinkAddressCallback,
                        __OSKextLoggingCallback,
                        kKxldFlagDefault|kKXLDFlagIncludeRelocs,
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


    baseLoadAddr = (textLoadAddr + textVMSize + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    if (baseLoadAddr < __kOSKextMaxDisplacement)
    {
        fprintf(stderr, "Kext base load address underflow\n");
        goto failed;
    }
    baseLoadAddr -= __kOSKextMaxDisplacement; /* Kext VMA base */
    
    loadList = __OSKextPrelinkKexts(kextArray,
                                    kernelImage,
                                    baseLoadAddr, // where we load kexts: 2GB from top of __TEXT
                                    baseSrcAddr,
                                    kxldContext,
                                    &size,
                                    true,   /* (flags & kOSKextKernelcacheNeedAllFlag), */
                                    true,   /* (flags & kOSKextKernelcacheSkipAuthenticationFlag), */
                                    true,   /* (flags & kOSKextKernelcachePrintDiagnosticsFlag), */
                                    false); /* (flags & kOSKextKernelcacheStripSymbolsFlag)); */

    if (!loadList)
        goto failed;

    fprintf(stdout, "Linked %d bytes\n", size);
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
    prelinkImage = CFDataCreateMutable(kCFAllocatorDefault, 0);
    CFDataSetLength(prelinkImage, baseFileOffset);

    CFRange  range = CFRangeMake(0, baseFileOffset);
    uint8_t *bytePtr = CFDataGetBytePtr(kernelImage);
    CFDataReplaceBytes(prelinkImage, range, bytePtr, baseFileOffset);

    uintptr_t fileOffset = baseFileOffset;
    uintptr_t srcAddr = baseSrcAddr;

    size = CopyPrelinkedKexts(prelinkImage,
                              loadList,
                              fileOffset,
                              srcAddr);

    fileOffset += size;
    srcAddr += size;
    CFDataSetLength(prelinkImage, fileOffset);

    u_long pdsize = CFDataGetLength(prelinkInfoData);
    CFDataAppendBytes(prelinkImage, CFDataGetBytePtr(prelinkInfoData), pdsize);
    pdsize = (pdsize + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    CFDataSetLength(prelinkImage, fileOffset + pdsize);

    /* Set the info dictionary segment headers */
    mach_header = (struct mach_header_64 *) CFDataGetBytePtr(prelinkImage);
    struct segment_command_64 *seg = macho_get_segment_by_name_64(mach_header, kPrelinkInfoSegment);
    if (!seg)
    {
        fprintf(stderr, "no seg\n");
        goto failed;
    }

    seg->vmaddr = srcAddr;
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

    sect->addr = srcAddr;
    sect->offset = fileOffset;
    sect->size = pdsize;

    fileOffset += pdsize;
    srcAddr = (srcAddr + pdsize + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    
    CFDataSetLength(prelinkImage, fileOffset);

    if (gPEKext)
    {
        /* now create the __BUILTIN segment with our PlatformExpert */
        int segmentSize = 0;

        seg = macho_get_segment_by_name_64(mach_header, "__BUILTIN");
        if (!seg) {
            fprintf(stderr, "no __BUILTIN seg\n");
            goto failed;
        }

        seg->vmaddr = srcAddr;
        seg->fileoff = fileOffset;

        CFMutableArrayRef dicts = CFArrayCreateMutable(kCFAllocatorDefault,
                                                       0,
                                                       &kCFTypeArrayCallBacks);
        CFArrayAppendValue(dicts, gPEKext->infoDictionary);
        
        CFDataRef infodict = IOCFSerialize(dicts, kNilOptions);
        int size = CFDataGetLength(infodict);
        segmentSize += size;
            
        CFDataAppendBytes(prelinkImage,
                          CFDataGetBytePtr(infodict),
                          size);
            
        sect = macho_get_section_by_name_64(mach_header,
                                            "__BUILTIN",
                                            "__info");
        if (!sect)
        {
            fprintf(stderr, "no sect\n");
            goto failed;
        }

        sect->size = size;
        sect->addr = srcAddr;
        sect->offset = fileOffset;

        fileOffset += size;
        srcAddr = (srcAddr + size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

        seg->vmsize = segmentSize;
        seg->filesize = segmentSize;
        printf("Added Platform Expert kext to __BUILTIN: %p %d %d %d\n",
               seg->vmaddr, seg->vmsize, seg->fileoff, seg->filesize);
    } /* gBuiltin */
    
    result = CFRetain(prelinkImage);

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

    if(!initializeAllKexts())
        goto finish;
    
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

	if(!CFArrayContainsValue(__sOSAllKexts, RANGE_ALL(__sOSAllKexts), theKext))
            CFArrayAppendValue(kextArray, CFRetain(theKext));
    }

    fprintf(stdout, "Linking ...\n");
    CFDataRef prelinkImage = CreatePrelinkedKernel(kernelImage, __sOSAllKexts);
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
        0, 0, 0, 0, 0,        /* reserved[10] */
        0, 0, 0, 0, 0,
        "GenericX86_64",      /* platformName */
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
