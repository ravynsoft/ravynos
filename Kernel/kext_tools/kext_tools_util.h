/*
 *  kext_tools_util.h
 *  kext_tools
 *
 *  Created by Nik Gervae on 4/25/08.
 *  Copyright 2008 Apple Inc. All rights reserved.
 *
 */
#ifndef _KEXT_TOOLS_UTIL_H
#define _KEXT_TOOLS_UTIL_H

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/kext/OSKext.h>
#include <mach/mach_error.h>

#include <getopt.h>
#include <os/log.h>
#include <sysexits.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mount.h>
#include <sys/types.h>

#pragma mark Types
/*******************************************************************************
* Types
*******************************************************************************/
typedef int ExitStatus;

typedef enum {
    kUsageLevelBrief = 0,
    kUsageLevelFull  = 1
} UsageLevel;

typedef struct {
    CFURLRef   saveDirURL;
    Boolean    overwrite;
    Boolean    fatal;
} SaveFileContext;

#pragma mark Constants
/*******************************************************************************
* Constants
*******************************************************************************/
#define _kKextPropertyValuesCacheBasename  "KextPropertyValues_"
#define __kOSKextApplePrefix        CFSTR("com.apple.")

#define kAppleInternalPath      "/AppleInternal"
#define kDefaultDevKernelPath   "/System/Library/Kernels/kernel.development"
#define kDefaultDevKernelSuffix ".development"

#define kImmutableKernelFileName "immutablekernel"

// relative to _kOSKextCachesRootFolder
#define kThirdPartyKextAllowList "kextallow"

// 17 leap days from 1904 to 1970, inclusive
#define UNIX_MAC_TIME_DELTA ((1970-1904)*365*86400 + 17*86400)
#define HFS_TIME_END (((1LL<<32) - 1) - UNIX_MAC_TIME_DELTA)
// date -r $((0xffffffff - 2082844800))
// Mon Feb  6 06:28:15 UTC 2040

#pragma mark Macros
/*********************************************************************
* Macros
*********************************************************************/

#define SAFE_FREE(ptr)  do {          \
    if (ptr) free(ptr);               \
    } while (0)
#define SAFE_FREE_NULL(ptr)  do {     \
    if (ptr) free(ptr);               \
    (ptr) = NULL;                     \
    } while (0)
#define SAFE_RELEASE(ptr)  do {       \
    if (ptr) CFRelease(ptr);          \
    } while (0)
#define SAFE_RELEASE_NULL(ptr)  do {  \
    if (ptr) CFRelease(ptr);          \
    (ptr) = NULL;                     \
    } while (0)

#define RANGE_ALL(a)   CFRangeMake(0, CFArrayGetCount(a))

#define COMPILE_TIME_ASSERT(pred)   switch(0){case 0:case pred:;}

#define N_ELEMS(array) (sizeof(array) / sizeof(array[0]))
/*
 * Macros to support PATHCPY/PATHCAT
 *
 * _ERROR_LABEL() -> finish
 * _ERROR_LABEL(somelabel) -> somelabel
 */
#define _GET_ERROR_LABEL(_0,_label,...) _label
#define _ERROR_LABEL(...) _GET_ERROR_LABEL(_0, ## __VA_ARGS__, finish)

/*
 * Wrap up strlcpy and strlcat for paths.
 * By default, these macros assume you are copying into a buffer of
 * size PATH_MAX, and that a label named "finish" is where you would
 * like to jump on error. Also, we seed errno since strlXXX routines
 * do not set it. This will make downstream error messages more
 * meaningful (since we're often logging the errno value and message).
 * COMPILE_TIME_ASSERT() on PATHCPY breaks schdirparent().
 *
 * If a third argument is passed to this macro, it's used as the name
 * of the error label.
 */
#define PATHCPY(dst,src,...) \
        do { \
            /* COMPILE_TIME_ASSERT(sizeof(dst) == PATH_MAX); */ \
            bool useErrno = (errno == 0); \
            if (useErrno)       errno = ENAMETOOLONG; \
            if (strlcpy(dst, src, PATH_MAX) >= PATH_MAX) { \
                goto _ERROR_LABEL(__VA_ARGS__); \
            } \
            if (useErrno)       errno = 0; \
        } while(0)

#define PATHCAT(dst,src,...) \
        do { \
            COMPILE_TIME_ASSERT(sizeof(dst) == PATH_MAX); \
            bool useErrno = (errno == 0); \
            if (useErrno)       errno = ENAMETOOLONG; \
            if (strlcat(dst, src, PATH_MAX) >= PATH_MAX) { \
                goto _ERROR_LABEL(__VA_ARGS__); \
            } \
            if (useErrno)       errno = 0; \
        } while(0)


/*********************************************************************
*********************************************************************/

/* Library default is not to log Basic, but kextd & kextcache spawned
 * by kextd want to do that.
 */
#define kDefaultServiceLogFilter ((OSKextLogSpec) kOSKextLogBasicLevel | \
                                             kOSKextLogVerboseFlagsMask)

#pragma mark Shared Command-line Option Definitions
/*******************************************************************************
* Command-line options. This data is used by getopt_long_only().
*******************************************************************************/
#define kOptNameArch                    "arch"
#define kOptNameBundleIdentifier        "bundle-id"
#define kOptNameRepository              "repository"

#define kOptNameSafeBoot                "safe-boot"
#define kOptNameSystemExtensions        "system-extensions"
#define kOptNameKernel                  "kernel"
#define kOptNameNoAuthentication        "no-authentication"

#define kOptNameHelp                    "help"
#define kOptNameQuiet                   "quiet"
#define kOptNameVerbose                 "verbose"
#define kOptNameLayoutMap               "layout"
#define kOptNameTargetOverride          "target-override"

#define kOptNameLongindexHack           "________"

// Can't use -a globally for -arch because of kextutil...?
#define kOptBundleIdentifier 'b'
// Can't use -r globally for -repository because of kextcache...?

// Can't use -x globally for -safe-boot because of kextcache's -s...?
// Can't use -e globally for -system-extensions because of kextutil...?
// Can't use -k globally for -kernel because of kextcache...?
#define kOptNoAuthentication 'z'

#define kOptHelp             'h'
#define kOptQuiet            'q'
#define kOptVerbose          'v'
#define kOptLayoutMap        'l'
#define kOptTargetOverride   'T'

// Long opts always defined in each program to avoid # collisions

#pragma mark Function Protos
/*********************************************************************
* Function Protos
*********************************************************************/

Boolean createCFMutableArray(CFMutableArrayRef * arrayOut,
    const CFArrayCallBacks * callbacks);
Boolean createCFMutableDictionary(CFMutableDictionaryRef * dictionaryOut);
Boolean createCFMutableSet(CFMutableSetRef * setOut,
    const CFSetCallBacks * callbacks);
Boolean createCFDataFromFile(CFDataRef  *dataRefOut,
                             const char *filePath);
Boolean createCFDataFromFD(int fd, CFDataRef *dataRefOut);

char *get_bootarg(char *bootArg);
bool get_bootarg_int(char *bootarg, uint32_t *value);

void addToArrayIfAbsent(CFMutableArrayRef array, const void * value);

ExitStatus checkPath(
    const char * path,
    const char * suffix,  // w/o the dot
    Boolean      directoryRequired,
    Boolean      writableRequired);

ExitStatus setLogFilterForOpt(
    int            argc,
    char * const * argv,
    OSKextLogSpec  forceOnFlags);

void beQuiet(void);

extern FILE *  g_log_stream;
// tool_openlog(), tool_log() copied to bootroot.h for libBootRoot clients
void tool_initlog(void);
void tool_openlog(const char * name);
os_log_t get_signpost_log(void);
void tool_log(
    OSKextRef aKext,
    OSKextLogSpec logSpec,
    const char * format,
    ...);
void log_CFError(
    OSKextRef     aKext __unused,
    OSKextLogSpec msgLogSpec,
    CFErrorRef    error);

const char * safe_mach_error_string(mach_error_t error_code);

#define REPLY_ERROR (-1)
#define REPLY_NO     (0)
#define REPLY_YES    (1)
#define REPLY_ALL    (2)

int user_approve(Boolean ask_all, int default_answer, const char * format, ...);

const char * user_input(Boolean * eof, const char * format, ...);
void saveFile(const void * vKey, const void * vValue, void * vContext);

CFStringRef copyKextPath(OSKextRef aKext);
Boolean readSystemKextPropertyValues(
    CFStringRef        propertyKey,
    const NXArchInfo * arch,
    Boolean            forceUpdateFlag,
    CFArrayRef       * valuesOut);

ExitStatus writeToFile(
    int           fileDescriptor,
    const UInt8 * data,
    CFIndex       length);

ExitStatus statURL(CFURLRef anURL, struct stat * statBuffer);
ExitStatus statPath(const char *path, struct stat *statBuffer);
ExitStatus statParentPath(const char *thePath, struct stat *statBuffer);
ExitStatus getLatestTimesFromCFURLArray(
                                        CFArrayRef          fileURLArray,
                                        struct timeval      fileTimes[2]);
ExitStatus getLatestTimesFromDirURL(
                                    CFURLRef       dirURL,
                                    struct timeval dirTimeVals[2]);

ExitStatus getLatestTimesFromDirPath(
                                     const char *   dirPath,
                                     struct timeval dirTimeVals[2]);

ExitStatus getFilePathTimes(
                            const char        * filePath,
                            struct timeval      cacheFileTimes[2]);

ExitStatus getFileDescriptorTimes(
                                   int the_fd,
                                   struct timeval   cacheFileTimes[2]);

ExitStatus getParentPathTimes(
                              const char        * thePath,
                              struct timeval      cacheFileTimes[2] );

void postNoteAboutKexts(
                        CFStringRef theNotificationCenterName,
                        CFMutableDictionaryRef theDict );

void postNoteAboutKextLoadsMT(
                            CFStringRef theNotificationCenterName,
                            CFMutableArrayRef theKextPathArray );

void addKextToAlertDict(
                        CFMutableDictionaryRef *theDictPtr,
                        OSKextRef theKext );

char * getPathExtension(const char * pathPtr);

int getFileDevAndInoAndSizeWith_fd(int the_fd, dev_t * the_dev_t, ino_t * the_ino_t, size_t * the_size);
int getFileDevAndInoWith_fd(int the_fd, dev_t * the_dev_t, ino_t * the_ino_t);
int getFileDevAndIno(const char * thePath, dev_t * the_dev_t, ino_t * the_ino_t);
Boolean isSameFileDevAndIno(int the_fd,
                            const char * thePath,
                            bool followSymlinks,
                            dev_t the_dev_t,
                            ino_t the_ino_t);
Boolean isSameFileDevAndInoWith_fd(int      the_fd,
                                   dev_t    the_dev_t,
                                   ino_t    the_ino_t);

Boolean createRawBytesFromHexString(char *bytePtr,
                                    size_t byteLen,
                                    const char *hexPtr,
                                    size_t hexLen);

Boolean createHexStringFromRawBytes(char *hexPtr,
                                    size_t hexLen,
                                    const char *bytePtr,
                                    size_t byteLen);

// bootcaches.plist helpers
CFDictionaryRef copyBootCachesDictForURL(CFURLRef theVolRootURL);
Boolean getKernelPathForURL(
                            CFURLRef theVolRootURL,
                            char * theBuffer,
                            int theBufferSize );

bool readKextHashAllowList(bool mustMatchCurrentBoot,
                           CFStringRef *bootUUIDStr,
                           CFArrayRef *allowedHashesRef,
                           CFArrayRef *allowedBundleIDsRef,
                           CFArrayRef *exceptionListBundlesRef);

ExitStatus writeKextAllowList(const char *bootuuid,
                              CFDataRef cdhashData,
                              int to_dir_fd, const char *to_fname);

// Development kernel support
Boolean useDevelopmentKernel(const char * theKernelPath);
Boolean isDebugSetInBootargs(void);

// path translation from prelinkedkernel to immutablekernel
bool translatePrelinkedToImmutablePath(const char *prelinked_path,
                                       char *imk_path, size_t imk_len);

/*********************************************************************
* From IOKitUser/kext.subproj/OSKext.c.
*********************************************************************/

extern char * createUTF8CStringForCFString(CFStringRef aString);

void setVariantSuffix(void);

ExitStatus copyLoadedKextInfo(CFArrayRef *loadedKexts, bool waitToQuiesce);

int findmnt(dev_t devid, char mntpt[MNAMELEN], bool getDevicePath);

#if defined(ROSP_HACKS) && __has_include(<APFS/APFS.h>)
int isUserDataVolume(const char *systemVolumeDevicePath, const char *candidateMountPath);
#endif /* !ROSP_HACKS && __has_include(<APFS/APFS.h>) */
#endif /* _KEXT_TOOLS_UTIL_H */
