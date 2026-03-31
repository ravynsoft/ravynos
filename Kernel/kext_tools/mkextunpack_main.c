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
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/kext/OSKext.h>
#include <IOKit/kext/OSKextPrivate.h>
#include <IOKit/kext/fat_util.h>

#include <System/libkern/mkext.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <libc.h>
#include <mach-o/arch.h>
#include <mach-o/fat.h>

#include <mach/mach.h>
#include <mach/mach_types.h>
#include <mach/kmod.h>
#include "kext_tools_util.h"

// not a utility.[ch] customer yet
static const char * progname = "mkextunpack";
static Boolean gVerbose = false;

u_int32_t local_adler32(u_int8_t *buffer, int32_t length);

Boolean getMkextDataForArch(
    u_int8_t         * fileData,
    size_t             fileSize,
    const NXArchInfo * archInfo,
    void            ** mkextStart,
    void            ** mkextEnd,
    uint32_t         * mkextVersion);

CFArrayRef copyKextsFromMkext2(
    void             * mkextStart,
    void             * mkextEnd,
    const NXArchInfo * archInfo);
Boolean writeMkext2EntriesToDirectory(
    CFArrayRef   kexts,
    char       * outputDirectory);
Boolean writeMkext2ToDirectory(
    OSKextRef aKext,
    const char * outputDirectory,
    CFMutableSetRef kextNames);

CFDictionaryRef extractEntriesFromMkext1(
    void * mkextStart,
    void * mkextEnd);
Boolean uncompressMkext1Entry(
    void * mkext_base_address,
    mkext_file * entry_address,
    CFDataRef * uncompressedEntry);
Boolean writeMkext1EntriesToDirectory(CFDictionaryRef entries,
    char * outputDirectory);
CFStringRef createKextNameFromPlist(
    CFDictionaryRef entries, CFDictionaryRef kextPlist);
int getBundleIDAndVersion(CFDictionaryRef kextPlist, unsigned index,
    char ** bundle_id_out, char ** bundle_version_out);

Boolean writeFileInDirectory(
    const char * basePath,
    char       * subPath,
    const char * fileName,
    const char * fileData,
    size_t       fileLength);

#if 0
/*******************************************************************************
* NOTES!
*******************************************************************************/
How to unpack all arches from an mkext:

Unpack each arch in a fat mkext to a separate array of kexts.
For each arch:
    - Get kext plist
    - Check {kext path, id, vers} against assembled kexts:
        - Already have?
            - Plist identical:
                - "lipo" executable into assembled
                - Add all resources IF NOT PRESENT (check for different?)
            - Plist different:
                - cannot handle, error; or save to separate path
        - Else add kext to assembled list

    - Need to save infoDict, executable, resources
#endif /* 0 */

/*******************************************************************************
*******************************************************************************/
void usage(int num) {
    fprintf(stderr, "usage: %s [-v] [-a arch] [-d output_dir] mkextfile\n", progname);
    fprintf(stderr, "    -d output_dir: where to put kexts (must exist)\n");
    fprintf(stderr, "    -a arch: pick architecture from fat mkext file\n");
    fprintf(stderr, "    -v: verbose output; list kexts in mkextfile\n");
    return;
}

/*******************************************************************************
*******************************************************************************/
int main (int argc, const char * argv[]) {
    int exit_code = 0;

    char               optchar;
    char             * outputDirectory   = NULL;
    const char       * mkextFile         = NULL;
    int                mkextFileFD;
    struct stat        stat_buf;
    uint8_t          * mkextFileContents = NULL;
    void             * mkextStart        = NULL;
    void             * mkextEnd          = NULL;
    CFDictionaryRef    entries           = NULL;
    CFArrayRef         oskexts           = NULL;
    const NXArchInfo * archInfo          = NULL;
    uint32_t           mkextVersion;

    progname = argv[0];

   /* Set the OSKext log callback right away.
    */
    OSKextSetLogOutputFunction(&tool_log);

    while ((optchar = getopt(argc, (char * const *)argv, "a:d:hv")) != -1) {
        switch (optchar) {
          case 'd':
            if (!optarg) {
                fprintf(stderr, "no argument for -d\n");
                usage(0);
                exit_code = 1;
                goto finish;
            }
            outputDirectory = optarg;
            break;
          case 'h':
            usage(1);
            exit_code = 0;
            goto finish;
            break;
          case 'v':
            gVerbose = true;
            break;
          case 'a':
            if (archInfo != NULL) {
                fprintf(stderr, "architecture already specified; replacing\n");
            }
            if (!optarg) {
                fprintf(stderr, "no argument for -a\n");
                usage(0);
                exit_code = 1;
                goto finish;
            }
            archInfo = NXGetArchInfoFromName(optarg);
            if (!archInfo) {
                fprintf(stderr, "architecture '%s' not found\n", optarg);
                exit_code = 1;
                goto finish;
            }
            break;
        }
    }

   /* Update argc, argv based on option processing.
    */
    argc -= optind;
    argv += optind;

    if (argc == 0 || argc > 1) {
        usage(0);
        exit_code = 1;
        goto finish;
    }

    mkextFile = argv[0];

    if (!outputDirectory && !gVerbose) {
        fprintf(stderr, "no work to do; please specify -d or -v\n");
        usage(0);
        exit_code = 1;
        goto finish;
    }

    if (!mkextFile) {
        fprintf(stderr, "no mkext file given\n");
        usage(0);
        exit_code = 1;
        goto finish;
    }

    if (outputDirectory) {
        if (stat(outputDirectory, &stat_buf) < 0) {
            fprintf(stderr, "can't stat directory %s\n",
                outputDirectory);
            exit_code = 1;
            goto finish;
        }

        if ((stat_buf.st_mode & S_IFMT) != S_IFDIR) {
            fprintf(stderr, "%s is not a directory\n",
                outputDirectory);
            exit_code = 1;
            goto finish;
        }

        if (access(outputDirectory, W_OK) == -1) {
            fprintf(stderr, "can't write into directory %s "
                "(permission denied)\n", outputDirectory);
            exit_code = 1;
            goto finish;
        }
    }

    if (stat(mkextFile, &stat_buf) < 0) {
        fprintf(stderr, "can't stat file %s\n", mkextFile);
        exit_code = 1;
        goto finish;
    }

    if (access(mkextFile, R_OK) == -1) {
        fprintf(stderr, "can't read file %s (permission denied)\n",
            mkextFile);
        exit_code = 1;
        goto finish;
    }

    mkextFileFD = open(mkextFile, O_RDONLY, 0);
    if (mkextFileFD < 0) {
        fprintf(stderr, "can't open file %s\n", mkextFile);
        exit_code = 1;
        goto finish;
    }

    if ( !(stat_buf.st_mode & S_IFREG) ) {
        fprintf(stderr, "%s is not a regular file\n",
            mkextFile);
        exit_code = 1;
        goto finish;
    }

    if (!stat_buf.st_size) {
        fprintf(stderr, "%s is an empty file\n",
            mkextFile);
        exit_code = 1;
        goto finish;
    }

    mkextFileContents = mmap(0, (size_t)stat_buf.st_size, PROT_READ,
        MAP_FILE|MAP_PRIVATE, mkextFileFD, 0);
    if (mkextFileContents == (u_int8_t *)-1) {
        fprintf(stderr, "can't map file %s\n", mkextFile);
        exit_code = 1;
        goto finish;
    }

    if (!getMkextDataForArch(mkextFileContents, (size_t)stat_buf.st_size,
        archInfo, &mkextStart, &mkextEnd, &mkextVersion)) {

        exit_code = 1;
        goto finish;
    }

    if (mkextVersion == MKEXT_VERS_2) {
        oskexts = copyKextsFromMkext2(mkextStart, mkextEnd, archInfo);
        if (!oskexts) {
            exit_code = 1;
            goto finish;
        }

        if (outputDirectory &&
            !writeMkext2EntriesToDirectory(oskexts, outputDirectory)) {
            exit_code = 1;
            goto finish;
        }
    } else if (mkextVersion == MKEXT_VERS_1) {
        entries = extractEntriesFromMkext1(mkextStart, mkextEnd);
        if (!entries) {
            fprintf(stderr, "can't unpack file %s\n", mkextFile);
            exit_code = 1;
            goto finish;
        }

        if (outputDirectory &&
            !writeMkext1EntriesToDirectory(entries, outputDirectory)) {

            exit_code = 1;
            goto finish;
        }
    }

finish:
    SAFE_RELEASE(oskexts);
    exit(exit_code);
    return exit_code;
}

/*******************************************************************************
*******************************************************************************/
Boolean getMkextDataForArch(
    u_int8_t         * fileData,
    size_t             fileSize,
    const NXArchInfo * archInfo,
    void            ** mkextStart,
    void            ** mkextEnd,
    uint32_t         * mkextVersion)
{
    Boolean        result = false;
    uint32_t       magic;
    fat_iterator   fatIterator = NULL;  // must fat_iterator_close()
    mkext_header * mkextHeader = NULL;  // do not free
    uint8_t      * crc_address = NULL;  // do not free
    uint32_t       checksum;

    *mkextStart = *mkextEnd = NULL;
    *mkextVersion = 0;

    magic = MAGIC32(fileData);
    if (ISFAT(magic)) {
        if (!archInfo) {
            archInfo = NXGetLocalArchInfo();
        }
        fatIterator = fat_iterator_for_data(fileData,
            fileData + fileSize,
            1 /* mach-o only */);
        if (!fatIterator) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
                "Can't read mkext fat header.");
            goto finish;
        }
        *mkextStart = fat_iterator_find_arch(fatIterator,
            archInfo->cputype, archInfo->cpusubtype, mkextEnd);
        if (!*mkextStart) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
                "Architecture %s not found in mkext.",
                archInfo->name);
            goto finish;
        }
    } else {
        *mkextStart = fileData;
        *mkextEnd = fileData + fileSize;
    }

    mkextHeader = (mkext_header *)*mkextStart;

    if ((MKEXT_GET_MAGIC(mkextHeader) != MKEXT_MAGIC) ||
        (MKEXT_GET_SIGNATURE(mkextHeader) != MKEXT_SIGN)) {

        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
            "Bad mkext magic/signature.");
        goto finish;
    }
    if (MKEXT_GET_LENGTH(mkextHeader) !=
        (*mkextEnd - *mkextStart)) {

        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
            "Mkext length field %d does not match mkext actual size %d.",
            MKEXT_GET_LENGTH(mkextHeader),
            (int)(*mkextEnd - *mkextStart));
        goto finish;
    }
    if (archInfo && MKEXT_GET_CPUTYPE(mkextHeader) != archInfo->cputype) {

        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
            "Mkext cputype %d does not match requested type %d (%s).",
            MKEXT_GET_CPUTYPE(mkextHeader),
            archInfo->cputype,
            archInfo->name);
        goto finish;
    }

    crc_address = (uint8_t *)&mkextHeader->version;
    checksum = local_adler32(crc_address,
        (int32_t)((uintptr_t)mkextHeader +
        MKEXT_GET_LENGTH(mkextHeader) - (uintptr_t)crc_address));

    if (MKEXT_GET_CHECKSUM(mkextHeader) != checksum) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
            "Mkext checksum error.");
        goto finish;
    }

    *mkextVersion = MKEXT_GET_VERSION(mkextHeader);

    result = true;

finish:
    if (fatIterator) {
        fat_iterator_close(fatIterator);
    }
    return result;
}

/*******************************************************************************
*******************************************************************************/
CFArrayRef copyKextsFromMkext2(
    void             * mkextStart,
    void             * mkextEnd,
    const NXArchInfo * archInfo)
{
    CFArrayRef    result          = NULL;  // release on error
    Boolean       ok              = false;
    CFDataRef     mkextDataObject = NULL;  // must release
    char          kextPath[PATH_MAX];
    char        * kextIdentifier  = NULL;  // must free
    char          kextVersion[kOSKextVersionMaxLength];

    if (!archInfo) {
        archInfo = NXGetLocalArchInfo();
    }

    OSKextSetArchitecture(archInfo);
    mkextDataObject = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault,
        (UInt8 *)mkextStart, mkextEnd - mkextStart, NULL);
    if (!mkextDataObject) {
        OSKextLogMemError();
        goto finish;
    }
    result = OSKextCreateKextsFromMkextData(kCFAllocatorDefault, mkextDataObject);
    if (!result) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
            "Can't read mkext2 archive.");
        goto finish;
    }

    if (gVerbose) {
        CFIndex count, i;

        count = CFArrayGetCount(result);
        fprintf(stdout, "Found %d kexts:\n", (int)count);
        for (i = 0; i < count; i++) {
            OSKextRef theKext = (OSKextRef)CFArrayGetValueAtIndex(result, i);

            SAFE_FREE(kextIdentifier);

            if (!CFURLGetFileSystemRepresentation(OSKextGetURL(theKext),
                /* resolveToBase */ false, (UInt8 *)kextPath, sizeof(kextPath))) {

                OSKextLogStringError(theKext);
                goto finish;
            }
            kextIdentifier = createUTF8CStringForCFString(OSKextGetIdentifier(theKext));
            OSKextVersionGetString(OSKextGetVersion(theKext), kextVersion,
                sizeof(kextVersion));
            fprintf(stdout, "%s - %s (%s)\n", kextPath, kextIdentifier,
                kextVersion);
        }
    }

    ok = true;

finish:
    if (!ok) {
        SAFE_RELEASE_NULL(result);
    }
    SAFE_RELEASE(mkextDataObject);
    return result;
}

/*******************************************************************************
*******************************************************************************/
Boolean writeMkext2EntriesToDirectory(
    CFArrayRef   kexts,
    char       * outputDirectory)
{
    Boolean         result          = false;
    CFMutableSetRef kextNames       = NULL;  // must release
    CFIndex         count, i;

    if (!createCFMutableSet(&kextNames, &kCFTypeSetCallBacks)) {
        OSKextLogMemError();
    }

    count = CFArrayGetCount(kexts);
    for (i = 0; i < count; i++) {
        OSKextRef theKext = (OSKextRef)CFArrayGetValueAtIndex(kexts, i);

        if (!writeMkext2ToDirectory(theKext, outputDirectory, kextNames)) {
            goto finish;
        }
    }

finish:
    return result;
}

/*******************************************************************************
*******************************************************************************/
Boolean writeMkext2ToDirectory(
    OSKextRef aKext,
    const char * outputDirectory,
    CFMutableSetRef kextNames)
{
    Boolean         result                 = false;
    CFDictionaryRef infoDict               = NULL;  // must release
    CFDataRef       infoDictData           = NULL;  // must release
    CFErrorRef      error                  = NULL;  // must release
    CFDataRef       executable             = NULL;  // must release
    CFURLRef        kextURL                = NULL;  // do not release
    CFStringRef     kextName               = NULL;  // must release
    CFStringRef     executableName         = NULL;  // do not release
    uint32_t        pathLength;
    char            kextPath[PATH_MAX];      // gets trashed during use
    char          * kextNameCString        = NULL;  // do not free
    char          * kextNameSuffix         = NULL;  // do not free
    char          * kextNameCStringAlloced = NULL;  // must free
    char          * executableNameCStringAlloced = NULL;  // must free
    const void    * file_data              = NULL;  // do not free
    char            subPath[PATH_MAX];
    CFIndex         i;

    infoDict = OSKextCopyInfoDictionary(aKext);
    executable = OSKextCopyExecutableForArchitecture(aKext, NULL);
    if (!infoDict) {
        OSKextLogMemError();
        goto finish;
    }

    kextURL = OSKextGetURL(aKext);
    if (!CFURLGetFileSystemRepresentation(kextURL, /* resolveToBase */ true,
        (UInt8 *)kextPath, sizeof(kextPath))) {

        OSKextLogStringError(aKext);
        goto finish;
    }
    pathLength = (uint32_t)strlen(kextPath);
    if (pathLength && kextPath[pathLength-1] == '/') {
        kextPath[pathLength-1] = '\0';
    }
    kextNameCString = rindex(kextPath, '/');
    if (kextNameCString) {
        kextNameCString++;
    }

    SAFE_RELEASE_NULL(kextName);
    kextName = CFStringCreateWithCString(kCFAllocatorDefault,
        kextNameCString, kCFStringEncodingUTF8);
    if (!kextName) {
        OSKextLogMemError();
        goto finish;
    }

   /* Splat off the ".kext" suffix if needed so we can build
    * numbered variants.
    */
    if (CFSetContainsValue(kextNames, kextName)) {
        kextNameSuffix = strstr(kextNameCString, ".kext");
        if (!kextNameSuffix) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
                "Bad mkext data; kext missing suffix.");
            goto finish;
        }
        *kextNameSuffix = '\0';  // truncate at the period
    }

    i = 1;
    while (CFSetContainsValue(kextNames, kextName)) {
        SAFE_RELEASE_NULL(kextName);
        kextName = CFStringCreateWithFormat(kCFAllocatorDefault,
            /* formatOptions */ NULL, CFSTR("%s-%zd.kext"),
            kextNameCString, i);
        i++;
    }
    CFSetAddValue(kextNames, kextName);

    kextNameCStringAlloced = createUTF8CStringForCFString(kextName);

   /*****
    * Write the plist file.
    */
    infoDictData = CFPropertyListCreateData(kCFAllocatorDefault,
        infoDict, kCFPropertyListXMLFormat_v1_0, /* options */ 0, &error);
    if (!infoDictData) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
            "Can't serialize kext info dictionary.");
        goto finish;
    }

    file_data = (u_int8_t *)CFDataGetBytePtr(infoDictData);
    if (!file_data) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
            "Internal error; info dictionary has no data.");
        goto finish;
    }

    if (snprintf(subPath, sizeof(subPath),
            "%s/Contents", kextNameCStringAlloced) >= sizeof(subPath) - 1) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogArchiveFlag | kOSKextLogFileAccessFlag,
            "Output path is too long - %s.", subPath);
        goto finish;
    }
    if (!writeFileInDirectory(outputDirectory, subPath, "Info.plist",
        file_data, CFDataGetLength(infoDictData))) {

        goto finish;
    }

    if (!executable) {
        result = true;
        goto finish;
    }

   /*****
    * Write the executable file.
    */
    file_data = (u_int8_t *)CFDataGetBytePtr(executable);
    if (!file_data) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
            "Internal error; executable has no data.");
        goto finish;
    }

    if (snprintf(subPath, sizeof(subPath),
            "%s/Contents/MacOS", kextNameCStringAlloced) >= sizeof(subPath) - 1) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogArchiveFlag | kOSKextLogFileAccessFlag,
            "Output path is too long - %s.", subPath);
        goto finish;
    }
    executableName = CFDictionaryGetValue(infoDict, CFSTR("CFBundleExecutable"));
    if (!executableName) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
            "Kext %s has an executable but no CFBundleExecutable property.",
            kextNameCStringAlloced);
        goto finish;
    }
    executableNameCStringAlloced = createUTF8CStringForCFString(executableName);
    if (!executableNameCStringAlloced) {
        OSKextLogMemError();
        goto finish;
    }
    if (!writeFileInDirectory(outputDirectory, subPath,
        executableNameCStringAlloced,
        file_data, CFDataGetLength(executable))) {

        goto finish;
    }

    result = true;

finish:
    SAFE_RELEASE(infoDict);
    SAFE_RELEASE(infoDictData);
    SAFE_RELEASE(error);
    SAFE_RELEASE(executable);
    SAFE_RELEASE(kextName);
    SAFE_FREE(kextNameCStringAlloced);
    SAFE_FREE(executableNameCStringAlloced);
    return result;
}

/*******************************************************************************
*******************************************************************************/
static Boolean CaseInsensitiveEqual(CFTypeRef left, CFTypeRef right)
{
    CFComparisonResult compResult;

    compResult = CFStringCompare(left, right, kCFCompareCaseInsensitive);
    if (compResult == kCFCompareEqualTo) {
        return true;
    }
    return false;
}

/*******************************************************************************
*******************************************************************************/
static CFHashCode CaseInsensitiveHash(const void *key)
{
    return (CFStringGetLength(key));
}

/*******************************************************************************
*******************************************************************************/
CFDictionaryRef extractEntriesFromMkext1(
    void * mkextStart,
    void * mkextEnd)
{
    CFMutableDictionaryRef entries = NULL;  // returned
    Boolean error = false;

    unsigned int    i;

    mkext1_header          * mkextHeader = NULL;  // don't free
    mkext_kext             * onekext_data = 0;     // don't free
    mkext_file             * plist_file = 0;       // don't free
    mkext_file             * module_file = 0;      // don't free
    CFStringRef              entryName = NULL;     // must release
    CFMutableDictionaryRef   entryDict = NULL; // must release
    CFDataRef                kextPlistDataObject = 0; // must release
    CFDictionaryRef          kextPlist = 0;        // must release
    CFStringRef              errorString = NULL;   // must release
    CFDataRef                kextExecutable = 0;   // must release
    CFDictionaryKeyCallBacks keyCallBacks;


    keyCallBacks = kCFTypeDictionaryKeyCallBacks;
    keyCallBacks.equal = &CaseInsensitiveEqual;
    keyCallBacks.hash = &CaseInsensitiveHash;
    entries = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
        &keyCallBacks,
        &kCFTypeDictionaryValueCallBacks);
    if (!entries) {
        goto finish;
    }

    mkextHeader = (mkext1_header *)mkextStart;

    if (gVerbose) {
        fprintf(stdout, "Found %u kexts:\n", MKEXT_GET_COUNT(mkextHeader));
    }

    for (i = 0; i < MKEXT_GET_COUNT(mkextHeader); i++) {
        if (entryName) {
            CFRelease(entryName);
            entryName = NULL;
        }
        if (entryDict) {
            CFRelease(entryDict);
            entryDict = NULL;
        }
        if (kextPlistDataObject) {
            CFRelease(kextPlistDataObject);
            kextPlistDataObject = NULL;
        }
        if (kextPlist) {
            CFRelease(kextPlist);
            kextPlist = NULL;
        }
        if (errorString) {
            CFRelease(errorString);
            errorString = NULL;
        }
        if (kextExecutable) {
            CFRelease(kextExecutable);
            kextExecutable = NULL;
        }

        entryDict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
            &kCFTypeDictionaryKeyCallBacks,
            &kCFTypeDictionaryValueCallBacks);
        if (!entryDict) {
            fprintf(stderr, "internal error.\n");
            error = true;
            goto finish;
        }

        onekext_data = &mkextHeader->kext[i];
        plist_file = &onekext_data->plist;
        module_file = &onekext_data->module;

       /*****
        * Get the plist
        */
        if (!uncompressMkext1Entry(mkextStart,
            plist_file, &kextPlistDataObject) || !kextPlistDataObject) {

            fprintf(stderr, "couldn't uncompress plist at index %d.\n", i);
            continue;
        }

        CFErrorRef  error;
        kextPlist = CFPropertyListCreateWithData(kCFAllocatorDefault,
                                                 kextPlistDataObject,
                                                 kCFPropertyListImmutable,
                                                 NULL,
                                                 &error);
        if (!kextPlist) {
            errorString = CFErrorCopyDescription(error);
            CFRelease(error);
            if (errorString) {
                CFIndex bufsize = CFStringGetMaximumSizeForEncoding(
                CFStringGetLength(errorString), kCFStringEncodingUTF8);
                char * error_string = (char *)malloc((1+bufsize) * sizeof(char));
                if (!error_string) {
                    fprintf(stderr, "memory allocation failure\n");
                    error = true;
                    goto finish;
                }
                if (CFStringGetCString(errorString, error_string,
                     bufsize, kCFStringEncodingUTF8)) {
                    fprintf(stderr, "error reading plist: %s",
                        error_string);
                }
                free(error_string);
            } else {
                fprintf(stderr, "error reading plist");
            }
            goto finish;
        }

        entryName = createKextNameFromPlist(entries, kextPlist);
        if (!entryName) {
            fprintf(stderr, "internal error.\n");
            error = true;
            goto finish;
        }

        CFDictionarySetValue(entryDict, CFSTR("plistData"), kextPlistDataObject);
        CFDictionarySetValue(entryDict, CFSTR("plist"), kextPlist);

        if (gVerbose) {
            char kext_name[PATH_MAX];
            char * bundle_id = NULL;  // don't free
            char * bundle_vers = NULL; // don't free

            if (!CFStringGetCString(entryName, kext_name, sizeof(kext_name) - 1,
                    kCFStringEncodingUTF8)) {
                    fprintf(stderr, "memory or string conversion error\n");
            } else {
                switch (getBundleIDAndVersion(kextPlist, i, &bundle_id,
                    &bundle_vers)) {

                  case 1:
                    fprintf(stdout, "%s - %s (%s)\n", kext_name, bundle_id,
                        bundle_vers);
                    break;
                  case 0:
                    continue;
                  case -1:
                  default:
                    error = true;
                    goto finish;
                    break;
                }
            }
        }

       /*****
        * Get the executable
        */
        if (OSSwapBigToHostInt32(module_file->offset) ||
            OSSwapBigToHostInt32(module_file->compsize) ||
            OSSwapBigToHostInt32(module_file->realsize) ||
            OSSwapBigToHostInt32(module_file->modifiedsecs)) {

            if (!uncompressMkext1Entry(mkextStart,
                module_file, &kextExecutable)) {

                fprintf(stderr, "couldn't uncompress executable at index %d.\n",
                    i);
                continue;
            }

            if (kextExecutable) {
                CFDictionarySetValue(entryDict, CFSTR("executable"),
                    kextExecutable);
            }
        }

        CFDictionarySetValue(entries, entryName, entryDict);
    }

finish:
    if (error) {
        if (entries) {
            CFRelease(entries);
            entries = NULL;
        }
    }
    if (entryName)       CFRelease(entryName);
    if (entryDict)       CFRelease(entryDict);
    if (kextPlistDataObject) CFRelease(kextPlistDataObject);
    if (kextPlist)       CFRelease(kextPlist);
    if (errorString)     CFRelease(errorString);
    if (kextExecutable)  CFRelease(kextExecutable);

    return entries;
}

/*******************************************************************************
*******************************************************************************/
Boolean uncompressMkext1Entry(
    void * mkext_base_address,
    mkext_file * entry_address,
    CFDataRef * uncompressedEntry)
{
    Boolean result = true;

    u_int8_t * uncompressed_data = NULL;    // must free
    CFDataRef uncompressedData = NULL;      // returned
    size_t uncompressed_size = 0;

    size_t offset = OSSwapBigToHostInt32(entry_address->offset);
    size_t compsize = OSSwapBigToHostInt32(entry_address->compsize);
    size_t realsize = OSSwapBigToHostInt32(entry_address->realsize);
    time_t modifiedsecs = OSSwapBigToHostInt32(entry_address->modifiedsecs);

    *uncompressedEntry = NULL;

   /* If realsize is 0 there is nothing to uncompress or if any of the other
    * three fields are 0, but that isn't an error.
    */
    if (realsize == 0 ||
        (offset == 0 && compsize == 0 && modifiedsecs == 0)) {
        goto finish;
    }

    uncompressed_data = malloc(realsize);
    if (!uncompressed_data) {
        fprintf(stderr, "malloc failure\n");
        result = false;
        goto finish;
    }

    if (compsize != 0) {
        uncompressed_size = decompress_lzss(uncompressed_data,
            (u_int32_t)realsize,
            mkext_base_address + offset,
            (u_int32_t)compsize);
        if (uncompressed_size != realsize) {
            fprintf(stderr, "uncompressed file is not the length "
                  "recorded.\n");
            result = false;
            goto finish;
        }
    } else {
        bcopy(mkext_base_address + offset, uncompressed_data,
            realsize);
    }

    uncompressedData = CFDataCreate(kCFAllocatorDefault,
        (const UInt8 *)uncompressed_data, realsize);
    if (!uncompressedData) {
        fprintf(stderr, "malloc failure\n");
        result = false;
        goto finish;
    }
    *uncompressedEntry = uncompressedData;

finish:
    if (uncompressed_data) free(uncompressed_data);

    return result;
}

/*******************************************************************************
*******************************************************************************/
Boolean writeMkext1EntriesToDirectory(CFDictionaryRef entryDict,
    char * outputDirectory)
{
    Boolean           result          = false;
    CFStringRef     * kextNames       = NULL;  // must free
    CFDictionaryRef * entries         = NULL;  // must free
    char            * kext_name       = NULL;  // must free
    char            * executable_name = NULL;  // must free
    char              subPath[PATH_MAX];
    unsigned int      count, i;

    count = (unsigned int)CFDictionaryGetCount(entryDict);

    kextNames = (CFStringRef *)malloc(count * sizeof(CFStringRef));
    entries = (CFDictionaryRef *)malloc(count * sizeof(CFDictionaryRef));
    if (!kextNames || !entries) {
        fprintf(stderr, "malloc failure\n");
        result = false;
        goto finish;
    }

    CFDictionaryGetKeysAndValues(entryDict, (const void **)kextNames,
        (const void **)entries);

    for (i = 0; i < count; i++) {
        CFStringRef     kextName       = kextNames[i];
        CFDictionaryRef kextEntry      = entries[i];
        CFStringRef     executableName = NULL;  // do not release
        CFDataRef       fileData       = NULL;  // do not release
        CFDictionaryRef plist;

        const void    * file_data = NULL;

        SAFE_FREE_NULL(kext_name);
        SAFE_FREE_NULL(executable_name);

        kext_name = createUTF8CStringForCFString(kextName);
        if (!kext_name) {
            OSKextLogMemError();
            goto finish;
        }

       /*****
        * Write the plist file.
        */
        fileData = CFDictionaryGetValue(kextEntry, CFSTR("plistData"));
        if (!fileData) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
                "Kext entry %d has no plist.", i);
            continue;
        }
        file_data = (u_int8_t *)CFDataGetBytePtr(fileData);
        if (!file_data) {
            OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
            "Kext %s has no plist.", kext_name);
            continue;
        }

        if (snprintf(subPath, sizeof(subPath),
                "%s.kext/Contents", kext_name) >= sizeof(subPath) - 1) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogFileAccessFlag,
                "Output path is too long - %s.", subPath);
            goto finish;
        }
        if (!writeFileInDirectory(outputDirectory, subPath, "Info.plist",
            file_data, CFDataGetLength(fileData))) {

            goto finish;
        }

       /*****
        * Write the executable file.
        */
        fileData = CFDictionaryGetValue(kextEntry, CFSTR("executable"));
        if (!fileData) {
            continue;
        }
        file_data = (u_int8_t *)CFDataGetBytePtr(fileData);
        if (!file_data) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
                "Executable missing from kext %s.",
                kext_name);
            continue;
        }

        if (snprintf(subPath, sizeof(subPath),
                "%s.kext/Contents/MacOS", kext_name) >= sizeof(subPath) - 1) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogFileAccessFlag,
                "Output path is too long - %s.", subPath);
            goto finish;
        }
        plist = CFDictionaryGetValue(kextEntry, CFSTR("plist"));
        executableName = CFDictionaryGetValue(plist, CFSTR("CFBundleExecutable"));
        if (!executableName) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogArchiveFlag,
                "Kext %s has an executable but no CFBundleExecutable property.",
                kext_name);
            continue;
        }
        executable_name = createUTF8CStringForCFString(executableName);
        if (!executable_name) {
            OSKextLogMemError();
            goto finish;
        }
        if (!writeFileInDirectory(outputDirectory, subPath, executable_name,
            file_data, CFDataGetLength(fileData))) {

            goto finish;
        }
    }

    result = true;

finish:
    if (kextNames) free(kextNames);
    if (entries)   free(entries);
    return result;
}

/*******************************************************************************
*******************************************************************************/
CFStringRef createKextNameFromPlist(
    CFDictionaryRef entries, CFDictionaryRef kextPlist)
{
    CFStringRef result = NULL; // returned
    CFStringRef bundleName = NULL; // don't release
    CFStringRef bundleID = NULL; // don't release
    static unsigned int nameUnknownIndex = 1;
    unsigned int dupIndex = 1;
    CFArrayRef idParts = NULL; // must release

   /* First see if there's a CFBundleExecutable.
    */
    bundleName = CFDictionaryGetValue(kextPlist, CFSTR("CFBundleExecutable"));
    if (!bundleName) {

       /* No? Try for CFBundleIdentifier and get the last component.
        */
        bundleID = CFDictionaryGetValue(kextPlist, CFSTR("CFBundleIdentifier"));
        if (bundleID) {
            CFIndex length;

            idParts = CFStringCreateArrayBySeparatingStrings(
            kCFAllocatorDefault, bundleID, CFSTR("."));
            length = CFArrayGetCount(idParts);
            bundleName = (CFStringRef)CFArrayGetValueAtIndex(idParts, length - 1);

           /* Identifier ends with a period? We got no name, then.
            */
            if (!CFStringGetLength(bundleName)) {
                bundleName = NULL;
            }
        }

       /* If we didn't find a name to use, conjure one up.
        */
        if (!bundleName) {
            result = CFStringCreateWithFormat(kCFAllocatorDefault,
                NULL, CFSTR("NameUnknown-%d"), nameUnknownIndex);
            nameUnknownIndex++;
            goto finish;
        }
    }

   /* See if we already have the name we found based on executable/bundle ID
    * (as opposed to making up with NameUnknown); if so, add numbers until we get a unique.
    */
    if (bundleName) {
    if (CFDictionaryGetValue(entries, bundleName)) {
        for ( ; ; dupIndex++) {
            result = CFStringCreateWithFormat(kCFAllocatorDefault,
                NULL, CFSTR("%@-%d"), bundleName, dupIndex);
            if (!CFDictionaryGetValue(entries, result)) {
                goto finish;
            }
            CFRelease(result);
            result = NULL;
        }
    } else {
        result = CFRetain(bundleName);
        goto finish;
    }
    }
finish:
    if (idParts) CFRelease(idParts);
    return result;
}

/*******************************************************************************
*******************************************************************************/
int getBundleIDAndVersion(CFDictionaryRef kextPlist, unsigned index,
    char ** bundle_id_out, char ** bundle_version_out)
{
    int result = 1;

    CFStringRef bundleID = NULL; // don't release
    CFStringRef bundleVersion = NULL; // don't release
    static char bundle_id[KMOD_MAX_NAME];
    static char bundle_vers[KMOD_MAX_NAME];

    bundleID = CFDictionaryGetValue(kextPlist, CFSTR("CFBundleIdentifier"));
    if (!bundleID) {
        fprintf(stderr, "kext entry %d has no CFBundleIdentifier\n", index);
        result = 0;
        goto finish;
    } else if (CFGetTypeID(bundleID) != CFStringGetTypeID()) {
        fprintf(stderr, "kext entry %d, CFBundleIdentifier is not a string\n", index);
        result = 0;
        goto finish;
    } else {
        if (!CFStringGetCString(bundleID, bundle_id, sizeof(bundle_id) - 1,
            kCFStringEncodingUTF8)) {
            fprintf(stderr, "memory or string conversion error\n");
            result = -1;
            goto finish;
        }
    }

    bundleVersion = CFDictionaryGetValue(kextPlist,
        CFSTR("CFBundleVersion"));
    if (!bundleVersion) {
        fprintf(stderr, "kext entry %d has no CFBundleVersion\n", index);
        result = 0;
        goto finish;
    } else if (CFGetTypeID(bundleVersion) != CFStringGetTypeID()) {
        fprintf(stderr, "kext entry %d, CFBundleVersion is not a string\n", index);
        result = 0;
        goto finish;
    } else {
        if (!CFStringGetCString(bundleVersion, bundle_vers,
            sizeof(bundle_vers) - 1, kCFStringEncodingUTF8)) {
            fprintf(stderr, "memory or string conversion error\n");
            result = -1;
            goto finish;
        }
    }

    if (bundle_id_out) {
        *bundle_id_out = bundle_id;
    }

    if (bundle_version_out) {
        *bundle_version_out = bundle_vers;
    }


finish:

    return result;
}

/*******************************************************************************
*******************************************************************************/
Boolean writeFileInDirectory(
    const char * basePath,
    char       * subPath,
    const char * fileName,
    const char * fileData,
    size_t       fileLength)
{
    Boolean  result = false;
    char     path[PATH_MAX];
    char   * pathComponent = NULL;     // do not free
    char   * pathComponentEnd = NULL;  // do not free
    int      fd = -1;
    uint32_t bytesWritten;

    if (strlcpy(path, basePath, sizeof(path)) >= sizeof(path)) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogFileAccessFlag,
            "Output path is too long - %s.", basePath);
        goto finish;
    }

    pathComponent = subPath;
    while (pathComponent) {
        pathComponentEnd = index(pathComponent, '/');
        if (pathComponentEnd) {
            *pathComponentEnd = '\0';
        }
        if (strlcat(path, "/", sizeof(path)) >= sizeof(path) ||
            strlcat(path, pathComponent, sizeof(path)) >= sizeof(path)) {

            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogFileAccessFlag,
                "Output path is too long - %s.", path);
            goto finish;
        }

        if ((mkdir(path, 0777) < 0) && (errno != EEXIST)) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogFileAccessFlag,
                "Can't create directory %s - %s", path, strerror(errno));
            goto finish;
        }
        if (pathComponentEnd) {
            *pathComponentEnd = '/';
            pathComponent = pathComponentEnd + 1;
            pathComponentEnd = NULL;
        } else {
            break;
        }
    }

    if (strlcat(path, "/", sizeof(path)) >= sizeof(path) ||
        strlcat(path, fileName, sizeof(path)) >= sizeof(path)) {

        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogFileAccessFlag,
            "Output path is too long - %s.", path);
        goto finish;
    }

    fd = open(path, O_WRONLY | O_CREAT, 0777);
    if (fd < 0) {
        OSKextLog(/* kext */ NULL,
            kOSKextLogErrorLevel | kOSKextLogFileAccessFlag,
            "Can't open %s for writing - %s.", path, strerror(errno));
        goto finish;
    }

    bytesWritten = 0;
    while (bytesWritten < fileLength) {
        int writeResult;
        writeResult = (int)write(fd, fileData + bytesWritten,
            fileLength - bytesWritten);
        if (writeResult < 0) {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogFileAccessFlag,
                "Write failed for %s - %s.", path, strerror(errno));
            goto finish;
        }
        bytesWritten += writeResult;
    }

    result = true;

finish:
    if (fd != -1) {
        close(fd);
    }
    return result;

}
