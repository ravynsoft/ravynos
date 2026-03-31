/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <mach/mach.h>
#include <mach/mach_error.h>

#include <mach-o/arch.h>

#include <System/libkern/mkext.h>
#include <CoreFoundation/CFBundlePriv.h>
#include <IOKit/kext/OSKextPrivate.h>

#include "kext_tools_util.h"
#include "compression.h"


/*******************************************************************************
*******************************************************************************/
typedef struct {
    CFMutableDataRef   mkext;
    uint32_t           kextIndex;
    uint32_t           compressOffset;
    const NXArchInfo * arch;
    Boolean            fatal;
    Boolean            compress;
} Mkext1Context;

void addToMkext1(
    const void * vKey,
    const void * vValue,
    void       * vContext);

Boolean addDataToMkext(
    CFDataRef       data,
    Mkext1Context * context,
    char          * kextPath,
    Boolean         isInfoDict);

/*******************************************************************************
*******************************************************************************/
CFDataRef createMkext1ForArch(const NXArchInfo * arch, CFArrayRef archiveKexts,
    boolean_t compress)
{
    CFMutableDataRef       result            = NULL;
    CFMutableDictionaryRef kextsByIdentifier = NULL;
    Mkext1Context          context;
    mkext1_header        * mkextHeader       = NULL;  // do not free
    const uint8_t        * adler_point = 0;
    CFIndex count, i;

    result = CFDataCreateMutable(kCFAllocatorDefault, /* capaacity */ 0);
    if (!result || !createCFMutableDictionary(&kextsByIdentifier)) {
        OSKextLogMemError();
        goto finish;
    }

   /* mkext1 can only contain 1 kext for a given bundle identifier, so we
    * have to pick out the most recent versions.
    */
    count = CFArrayGetCount(archiveKexts);
    for (i = 0; i < count; i++) {
        OSKextRef   theKext = (OSKextRef)CFArrayGetValueAtIndex(archiveKexts, i);
        CFStringRef bundleIdentifier = OSKextGetIdentifier(theKext);
        OSKextRef   savedKext = (OSKextRef)CFDictionaryGetValue(kextsByIdentifier,
            bundleIdentifier);
        OSKextVersion thisVersion, savedVersion;


        if (!OSKextSupportsArchitecture(theKext, arch)) {
            continue;
        }

        if (!savedKext) {
            CFDictionarySetValue(kextsByIdentifier, bundleIdentifier, theKext);
            continue;
        }

        thisVersion = OSKextGetVersion(theKext);
        savedVersion = OSKextGetVersion(savedKext);

        if (thisVersion > savedVersion) {
            CFDictionarySetValue(kextsByIdentifier, bundleIdentifier, theKext);
        }
    }

   /* Add room for the mkext header and kext descriptors.
    */
    CFDataSetLength(result, sizeof(mkext1_header) +
        CFDictionaryGetCount(kextsByIdentifier) * sizeof(mkext_kext));

    context.mkext = result;
    context.kextIndex = 0;
    context.compressOffset = (uint32_t)CFDataGetLength(result);
    context.arch = arch;
    context.fatal = false;
    context.compress = compress;
    CFDictionaryApplyFunction(kextsByIdentifier, addToMkext1, &context);
    if (context.fatal) {
        SAFE_RELEASE_NULL(result);
        goto finish;
    }

    mkextHeader = (mkext1_header *)CFDataGetBytePtr(result);
    mkextHeader->magic = OSSwapHostToBigInt32(MKEXT_MAGIC);
    mkextHeader->signature = OSSwapHostToBigInt32(MKEXT_SIGN);
    mkextHeader->version = OSSwapHostToBigInt32(0x01008000);   // 'vers' 1.0.0
    mkextHeader->numkexts =
        OSSwapHostToBigInt32(CFDictionaryGetCount(kextsByIdentifier));
    mkextHeader->cputype = OSSwapHostToBigInt32(arch->cputype);
    mkextHeader->cpusubtype = OSSwapHostToBigInt32(arch->cpusubtype);
    mkextHeader->length = OSSwapHostToBigInt32(CFDataGetLength(result));

    adler_point = (UInt8 *)&mkextHeader->version;
    mkextHeader->adler32 = OSSwapHostToBigInt32(local_adler32(
        (UInt8 *)&mkextHeader->version,
        (int)(CFDataGetLength(result) - (adler_point - (uint8_t *)mkextHeader))));

    OSKextLog(/* kext */ NULL, kOSKextLogProgressLevel | kOSKextLogArchiveFlag,
        "Created mkext for %s containing %lu kexts.",
        arch->name,
        CFDictionaryGetCount(kextsByIdentifier));

finish:
    SAFE_RELEASE(kextsByIdentifier);
    return result;
}

/*******************************************************************************
*******************************************************************************/
void addToMkext1(
    const void * vKey __unused,
    const void * vValue,
    void       * vContext)
{
    OSKextRef       aKext            = (OSKextRef)vValue;
    Mkext1Context * context          = (Mkext1Context *)vContext;

    CFBundleRef     kextBundle       = NULL;  // must release
    CFURLRef        infoDictURL      = NULL;  // must release
    CFDataRef       rawInfoDict      = NULL;  // must release
    CFDataRef       executable       = NULL;  // must release
    char            kextPath[PATH_MAX];

    if (context->fatal) {
        goto finish;
    }

    if (!CFURLGetFileSystemRepresentation(OSKextGetURL(aKext),
        /* resolveToBase */ false, (UInt8 *)kextPath, sizeof(kextPath))) {

        strlcpy(kextPath, "(unknown)", sizeof(kextPath));
    }

    OSKextLog(aKext,
        kOSKextLogProgressLevel | kOSKextLogArchiveFlag,
        "Adding %s to mkext.", kextPath);

    OSKextLog(aKext,
        kOSKextLogStepLevel | kOSKextLogArchiveFlag | kOSKextLogFileAccessFlag,
        "Opening CFBundle for %s.", kextPath);
    kextBundle = CFBundleCreate(kCFAllocatorDefault, OSKextGetURL(aKext));
    if (!kextBundle) {
        OSKextLog(aKext,
            kOSKextLogStepLevel | kOSKextLogArchiveFlag | kOSKextLogFileAccessFlag,
            "Can't open bundle for %s.", kextPath);
        context->fatal = true;
        goto finish;
    }

    infoDictURL = _CFBundleCopyInfoPlistURL(kextBundle);
    if (!infoDictURL) {
        OSKextLog(aKext,
            kOSKextLogErrorLevel | kOSKextLogFileAccessFlag,
            "Can't get URL for info dict of %s.", kextPath);
        context->fatal = true;
        goto finish;
    }

    /* create and fill infoDictPath
     */
    char            infoDictPath[PATH_MAX];

    if (!CFURLGetFileSystemRepresentation(infoDictURL,
                                          true,
                                          (uint8_t *)infoDictPath,
                                          sizeof(infoDictPath))) {
        OSKextLogStringError(/* kext */ NULL);
        context->fatal = true;
        goto finish;
    }

    if (!createCFDataFromFile(&rawInfoDict,
                              infoDictPath)) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogErrorLevel | kOSKextLogFileAccessFlag,
                  "%s: Can't read info dictoionary file '%s'",
                  __func__, infoDictPath);
        context->fatal = true;
        goto finish;
    }

    if (!addDataToMkext(rawInfoDict, context, kextPath, /* isInfoDict */ true)) {
        context->fatal = true;
        goto finish;
    }

    executable = OSKextCopyExecutableForArchitecture(aKext, context->arch);
    if (executable) {
        if (!addDataToMkext(executable, context, kextPath, /* isInfoDict */ false)) {
            context->fatal = true;
            goto finish;
        }
    } else if (OSKextDeclaresExecutable(aKext)) {
        OSKextLog(aKext,
            kOSKextLogErrorLevel | kOSKextLogFileAccessFlag,
            "Can't get executable for %s (architecture %s).", kextPath,
            context->arch->name);
        context->fatal = true;
        goto finish;
    }

    context->kextIndex++;

finish:
    if (kextBundle) {
        OSKextLog(aKext,
            kOSKextLogStepLevel | kOSKextLogArchiveFlag | kOSKextLogFileAccessFlag,
            "Releaseing CFBundle for %s.", kextPath);
        SAFE_RELEASE(kextBundle);
    }
    SAFE_RELEASE(infoDictURL);
    SAFE_RELEASE(rawInfoDict);
    SAFE_RELEASE(executable);
    return;
}

/*******************************************************************************
*******************************************************************************/
Boolean addDataToMkext(
    CFDataRef       data,
    Mkext1Context * context,
    char          * kextPath,
    Boolean         isInfoDict)
{
    Boolean         result             = false;
    CFIndex         newMkextLength;
    uint32_t        origCompressOffset;
    const UInt8   * mkextStart         = NULL;  // must calc after changing length!
    UInt8         * addedDataStart     = NULL;  // must calc after changing length!
    UInt8         * addedDataEnd       = NULL;  // do not free
    uint32_t        addedDataFullLength;
    uint32_t        compressedLength;

    uint8_t       * checkBuffer        = NULL;  // must free

    mkext1_header * mkextHeader        = NULL;  // do not free
    mkext_kext    * mkextKextEntry     = NULL;  // do not free
    mkext_file    * mkextFileEntry     = NULL;  // do not free

   /* Add enough to the mkext buffer to append the whole uncompressed file.
    * If the file can't be compressed, we'll just copy it in; if it can
    * be compressed, we'll set the mkext buffer length to fit exactly.
    */
    addedDataFullLength = (uint32_t)CFDataGetLength(data);
    newMkextLength = CFDataGetLength(context->mkext) + addedDataFullLength;
    CFDataSetLength(context->mkext, newMkextLength);
    if (CFDataGetLength(context->mkext) != newMkextLength) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
            "Can't resize mkext buffer.");
        goto finish;
    }
    mkextStart = CFDataGetBytePtr(context->mkext);

    origCompressOffset = context->compressOffset;
    addedDataStart = (UInt8 *)mkextStart + origCompressOffset;

    if (context->compress) {
        addedDataEnd = compress_lzss((uint8_t *)addedDataStart, addedDataFullLength,
            (uint8_t *)CFDataGetBytePtr(data), addedDataFullLength);
        if (!addedDataEnd) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogWarningLevel | kOSKextLogArchiveFlag,
                "%s did not compress; copying file (%d bytes).",
                isInfoDict ? "info dictionary" : "executable",
                addedDataFullLength);
        }
    }

    if (!addedDataEnd) {
        memcpy(addedDataStart, (const void *)CFDataGetBytePtr(data),
            addedDataFullLength);
        addedDataEnd = addedDataStart + addedDataFullLength;
        compressedLength = 0;
        context->compressOffset += addedDataFullLength;
    } else {
        size_t checkLength;

        compressedLength = (uint32_t)(addedDataEnd - addedDataStart);
        context->compressOffset += compressedLength;

        checkBuffer = (uint8_t *)malloc(addedDataFullLength);
        if (!checkBuffer) {
            OSKextLogMemError();
            goto finish;
        }

        checkLength = decompress_lzss(checkBuffer, addedDataFullLength,
            addedDataStart, compressedLength);
        if (checkLength != addedDataFullLength) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
                "%s - %s decompressed size %d differs from original size %d",
                kextPath,
                isInfoDict ? "info dictionary" : "executable",
                (int)checkLength, (int)addedDataFullLength);
            goto finish;
        }
        if (0 != memcmp(checkBuffer, CFDataGetBytePtr(data), checkLength)) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
                "%s - %s decompressed data differs from input",
                kextPath,
                isInfoDict ? "info dictionary" : "executable");
            goto finish;
        }

        OSKextLog(/* kext */ NULL,
            kOSKextLogDetailLevel | kOSKextLogArchiveFlag,
            "Compressed %s from %u to %u bytes (%.2f%%).",
            isInfoDict ? "info dict" : "executable",
            addedDataFullLength, compressedLength,
            (100.0 * (float)compressedLength/(float)addedDataFullLength));
    }

   /* Truncate the mkext to exactly fit the new total size.
    */
    CFDataSetLength(context->mkext, addedDataEnd - mkextStart);

    mkextHeader = (mkext1_header *)CFDataGetBytePtr(context->mkext);
    mkextKextEntry = &(mkextHeader->kext[context->kextIndex]);
    if (isInfoDict) {
        mkextFileEntry = &(mkextKextEntry->plist);
    } else {
        mkextFileEntry = &(mkextKextEntry->module);
    }
    mkextFileEntry->offset = OSSwapHostToBigInt32(origCompressOffset);
    mkextFileEntry->realsize = OSSwapHostToBigInt32(addedDataFullLength);
    mkextFileEntry->compsize = OSSwapHostToBigInt32(compressedLength);
    mkextFileEntry->modifiedsecs = 0;  // we never use this anyway

    result = true;

finish:
    SAFE_FREE(checkBuffer);
    return result;
}
