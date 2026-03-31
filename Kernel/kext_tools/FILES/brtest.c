/*
 * FILE: brtest.c
 * AUTH: Soren Spies (sspies)
 * DATE: 10 March 2011 (Copyright Apple Inc.)
 * DESC: test libBootRoot
 *
 */

// CFLAGS: -lBootRoot

#include "bootroot.h"

#include <bootfiles.h>
#include <err.h>
#include <paths.h>
#include <stdio.h>
#include <strings.h>
#include <sys/param.h>      // MIN()
#include <sys/stat.h>
#include <sysexits.h>

#include <CoreFoundation/CoreFoundation.h>


// #define VERBOSE

void usage(int exval) __attribute__((noreturn));
void usage(int exval)
{
    fprintf(stderr,
            "Usage: brtest update <vol> [-f]\n"
            "       brtest listboots <vol>\n"
            "       brtest erasefiles <srcVol> <bootDev> [-f]\n"
            "       brtest copyfiles <src> [options] <bootDev> [<BlessStyle>]\n"
            "           Options:\n"
            "               -anyboot - update <src>'s bootstamps (no UUID)\n"
            "               -pickerLabel <label> - specific text for opt-boot\n"
            "               -stagingDir - use the staging directory\n"
            "       brtest copyfiles <src> <root> /<dmg> <tgt>[/<dir>] [<BS>]\n"
            "              (/<dmg> is relative to <root>)\n"

        //  hopefully disable will be implicit when "stealing" an Apple_Boot
        //  "       brtest disableHelperUpdates <[src]Vol> [<tgtVol>]\n"
            );

    exit(exval);
}

int
update(CFURLRef volURL, int argc, char *argv[])
{
    Boolean force = false;

    if (argc == 2) {
        if (argv[1][0] == '-' && argv[1][1] == 'f') {
            force = true;
        } else {
            return EINVAL;
        }
    }

    return BRUpdateBootFiles(volURL, force);
}

#define DEVMAXPATHSIZE  128
int
listboots(char *volpath, CFURLRef volURL)
{
    int result;
    CFArrayRef boots;
    CFIndex i, bcount = 0;

    boots = BRCopyActiveBootPartitions(volURL);

    if (!boots) {
        printf("%s: no boot partitions\n", volpath);
        result = 0;
        goto finish;
    }

    printf("boot support for %s:\n", volpath);
    bcount = CFArrayGetCount(boots);
    for (i = 0; i < bcount; i++) {
        CFShow(CFArrayGetValueAtIndex(boots, i));  // sufficient?
    }

    result = 0;

finish:
    if (boots)      CFRelease(boots);

    return result;
}

int
copyfiles(CFURLRef srcVol, int argc, char *argv[])
{
    int result = ELAST + 1;
    char *targetSpec, *tdir, *blessarg = NULL;
    char *hostpath, *dmgpath;
    char helperName[DEVMAXPATHSIZE], helperDev[DEVMAXPATHSIZE] = _PATH_DEV;
    char path[PATH_MAX];
    struct stat sb;

    BRCopyFilesOpts opts = kBROptsNone;
    CFStringRef pickerLabel = NULL;
    CFArrayRef helpers = NULL;
    CFURLRef hostVol = NULL;
    CFStringRef bootDev = NULL;
    CFURLRef rootDMG = NULL;
    CFStringRef rootDMGURLStr;     // no need to release?
    CFStringRef bootArgs = NULL;
    CFMutableDictionaryRef plistOverrides = NULL;
    CFURLRef targetDir = NULL;
    BRBlessStyle blessSpec = kBRBlessFSDefault;

    if (argc > 3 && strcmp(argv[3], "-anyboot") == 0) {
        opts |= kBRAnyBootStamps;
        argv++; argc--;
    }
    if (argc > 4 && strcmp(argv[3], "-pickerLabel") == 0) {
        pickerLabel = CFStringCreateWithFileSystemRepresentation(nil, argv[4]);
        argv += 2; argc -= 2;
    }
    if (argc > 3 && strcmp(argv[3], "-stagingDir") == 0) {
        opts |= kBRUseStagingDir;
        argv++; argc--;
    }

    // argv[1-2] processed by main()
    switch (argc) {
        char path[PATH_MAX];
    case 4:
    case 5:
        // brtest copyfiles <src> <bootDev>[/<dir>] [<BlessStyle>]
        hostVol = CFRetain(srcVol);
        (void)CFURLGetFileSystemRepresentation(hostVol, true,
                                               (UInt8*)path, PATH_MAX);
        hostpath = path;
        targetSpec = argv[3];
        blessarg = argv[4];
        break;

    case 6:
    case 7:
        // brtest copyfiles <src> <root> /<dmg> <tgt>[/<dir>] [<BS>]
        hostpath = argv[3];
        dmgpath = argv[4];
        targetSpec = argv[5];
        blessarg = argv[6];

        // make sure the dmg actually exists (at the right path)
        (void)strlcpy(path, hostpath, PATH_MAX);
        (void)strlcat(path, argv[4], PATH_MAX);
        if (stat(path, &sb)) {
            err(EX_NOINPUT, "%s", path);
        }

        hostVol = CFURLCreateFromFileSystemRepresentation(nil,(UInt8*)hostpath,
                                                      strlen(hostpath), true);
        if (!hostVol)           goto finish;

    /* !! from CFURL.h !!
     * Note that, strictly speaking any leading '/' is not considered part
     * of the URL's path, although its presence or absence determines
     * whether the path is absolute. */
        // an absolute URL appears a critical to get root-dmg=file:///foo.dmg
        if (dmgpath[0] != '/')      usage(EX_USAGE);
        rootDMG = CFURLCreateFromFileSystemRepresentation(nil, (UInt8*)dmgpath,
                                                  strlen(dmgpath), false);
        if (!rootDMG)           goto finish;

        // Soren does not understand CFRURL.  An absolute URL isn't enough,
        // it also has to have CFURLGetString() called on it.   Awesome(?)ly,
        // this value could be ignored as it apparently changes the
        // description of rootDMG(!?).  To keep the code clear, we use
        // rootDMGURLStr when creating bootArgs.
        rootDMGURLStr = CFURLGetString(rootDMG);
        if (!rootDMGURLStr)    goto finish;

        // pass arguments to the kernel so it roots off the specified DMG
        // root-dmg must be a URL, not a path, with or w/o /localhost/
        bootArgs = CFStringCreateWithFormat(nil, nil, CFSTR("root-dmg=%@"),
                                            rootDMGURLStr);
        // fputc('\t', stderr); CFShow(bootArgs);
        // container-dmg=%@ allows another layer of disk image :P
        if (!bootArgs)          goto finish;
        plistOverrides = CFDictionaryCreateMutable(nil, 1,
                                         &kCFTypeDictionaryKeyCallBacks,
                                         &kCFTypeDictionaryValueCallBacks);
        if (!plistOverrides)    goto finish;
        CFDictionarySetValue(plistOverrides, CFSTR(kKernelFlagsKey), bootArgs);
        break;

    default:
        usage(EX_USAGE);
    }

    // extract any target directory from argument (e.g. disk0s3/mydir)
    if ((tdir = strchr(targetSpec, '/'))) {
        size_t tlen = tdir-targetSpec;
        if (*(tdir + 1) == '\0')    usage(EX_USAGE);
        (void)strlcpy(helperName, targetSpec, MIN(tlen + 1, DEVMAXPATHSIZE));
        bootDev = CFStringCreateWithBytes(nil, (UInt8*)targetSpec,
                  tlen, kCFStringEncodingUTF8, false);
        targetDir = CFURLCreateFromFileSystemRepresentation(nil, (UInt8*)tdir,
                    strlen(tdir), true);
    } else {
        bootDev = CFStringCreateWithFileSystemRepresentation(nil, targetSpec);
        (void)strlcpy(helperName, targetSpec, DEVMAXPATHSIZE);
    }

    // make sure the target /dev/ node exists
    (void)strlcat(helperDev, helperName, DEVMAXPATHSIZE);
    if (stat(helperDev, &sb)) {
        err(EX_NOINPUT, "%s", helperDev);
    }

    // warn if hostVol requires Boot!=Root but bootDev isn't one of its helpers
    if (hostVol && (helpers = BRCopyActiveBootPartitions(hostVol))
        && CFArrayGetCount(helpers) > 0) {
        CFRange searchRange = { 0, CFArrayGetCount(helpers) };
        if (!CFArrayContainsValue(helpers, searchRange, bootDev)) {
            fprintf(stderr,"%s doesn't 'belong to' %s; CSFDE might not work\n",
                    helperName, hostpath);
        }
    }
    if (helpers) CFRelease(helpers);

    // evaluate any bless style argument
    if (blessarg) {
        if (strcasestr(blessarg, "none")) {
            blessSpec = kBRBlessNone;
        } else if (strcasecmp(blessarg, "default") == 0) {
            // blessSpec = kBRBlessFSDefault;
        } else if (strcasecmp(blessarg, "full") == 0) {
            blessSpec = kBRBlessFull;
        } else if (strcasecmp(blessarg, "once") == 0) {
            blessSpec = kBRBlessOnce;
        } else if (strcasecmp(blessarg, "fsonce") == 0) {
            blessSpec |= kBRBlessOnce;
        } else if (strcasecmp(blessarg, "recovery") == 0) {
            blessSpec = kBRBlessRecovery;
        } else {
            usage(EX_USAGE);
        }
    }

    // use the fancier function depending on how custom we are
    if (opts || pickerLabel || targetDir || blessarg) {
        if (targetDir && !pickerLabel) {
            pickerLabel = CFURLCopyLastPathComponent(targetDir);
        }
#if LOG_ARGS
CFShow(CFSTR("ToDir() args ..."));
CFShow(srcVol);
CFShow(hostVol);
CFShow(plistOverrides);
CFShow(bootDev);
CFShow(targetDir);
fprintf(stderr, "blessSpec: %d\n", blessSpec);
CFShow(CFURLCopyLastPathComponent(targetDir));
#endif
        result = BRCopyBootFilesToDir(srcVol, hostVol, plistOverrides,
                              bootDev, targetDir, blessSpec, pickerLabel, opts);
    } else {
        result = BRCopyBootFiles(srcVol, hostVol, bootDev, plistOverrides);
    }

finish:
    if (pickerLabel)    CFRelease(pickerLabel);
    if (targetDir)      CFRelease(targetDir);
    if (plistOverrides) CFRelease(plistOverrides);
    if (bootArgs)       CFRelease(bootArgs);
    if (rootDMG)        CFRelease(rootDMG);
    if (bootDev)        CFRelease(bootDev);
    if (hostVol)        CFRelease(hostVol);

    return result;
}

int
erasefiles(char *volpath, CFURLRef srcVol, char *devname, char *forceArg)
{
    int result;
    CFStringRef bsdName = NULL;
    CFArrayRef helpers = NULL;
    Boolean force = false;

    if (forceArg) {
        if (forceArg[0] == '-' && forceArg[1] == 'f') {
            force = true;
        } else {
            result = EINVAL; goto finish;
        }
    }


    // build args
    if (!strstr(devname, "disk")) {
        usage(EX_USAGE);
    }
    bsdName = CFStringCreateWithFileSystemRepresentation(nil, devname);

    // prevent user from erasing srcVol's Apple_Boot(s) (-f overrides)
    // X: doesn't prevent user from whacking another volume's Apple_Boot
    if (!force) {
        helpers = BRCopyActiveBootPartitions(srcVol);
        if (helpers) {
            CFRange searchRange = { 0, CFArrayGetCount(helpers) };
            if (CFArrayContainsValue(helpers, searchRange, bsdName)) {
                fprintf(stderr, "%s currently required to boot '%s'! (-f?)\n",
                    devname, volpath);
                result = /* kPOSIXErrorBase +*/ EBUSY;
                goto finish;
            }
        }
    }

    result = BREraseBootFiles(srcVol, bsdName);

finish:
    if (helpers)        CFRelease(helpers);
    if (bsdName)        CFRelease(bsdName);

    return result;
}

int
main(int argc, char *argv[])
{
    int result, exval;
    char *verb, *volpath;
    struct stat sb;
    CFURLRef volURL = NULL;

    // check for -h or not enough args
    if (argc >= 2 && argv[1][0] == '-' && argv[1][1] == 'h')
        usage(EX_OK);
    if (argc < 3)
        usage(EX_USAGE);

#ifdef USE_ASL  // example code for daemon clients
    // OSKextSetLogOutputFunction(&tool_log);
    // tool_openlog(getprogname());
    // tool_openlog("brtest/libBootRoot");
#endif // USE_ASL
#ifdef VERBOSE
    OSKextSetLogFilter(kOSKextLogDetailLevel |
                       kOSKextLogVerboseFlagsMask |
                       kOSKextLogKextOrGlobalMask,
                       false);
#endif

    verb = argv[1];
    volpath = argv[2];
    if (stat(volpath, &sb) != 0) {
        if (volpath[0] == '-') {
            usage(EX_USAGE);
        } else {
            err(EX_NOINPUT, "%s", volpath);
        }
    }
    volURL = CFURLCreateFromFileSystemRepresentation(nil, (UInt8*)volpath,
                                                     strlen(volpath), true);
    if (!volURL) {
        usage(EX_OSERR);
    }

    if (strcasecmp(verb, "update") == 0) {
        if (argc < 3 || argc > 4)
            usage(EX_USAGE);
        result = update(volURL, argc-2, argv+2);
    } else if (strcasecmp(verb, "listboots") == 0) {
        if (argc != 3)
            usage(EX_USAGE);
        result = listboots(volpath, volURL);
    } else if (strcasecmp(verb, "copyfiles") == 0) {
        result = copyfiles(volURL, argc, argv);
    } else if (strcasecmp(verb, "erasefiles") == 0) {
        if (argc != 4 && argc != 5)
            usage(EX_USAGE);
        result = erasefiles(argv[2], volURL, argv[3], argv[4]);
    /* ... other verbs ... */
    } else {
        fprintf(stderr, "no recognized verb!\n");
        usage(EX_USAGE);
    }

    if (result < 0) {
        printf("brtest function result = %#x\n", result);
    } else {
        printf("brtest function result = %d", result);
        if (result == -1) {
            printf(": errno %d -> %s", errno, strerror(errno));
        } else if (result && result <= ELAST) {
            printf(": %s", strerror(result));
        }
        printf("\n");
    }

    if (result == -1) {
        exval = EX_OSERR;
    } else if (result == EINVAL) {
        exval = EX_USAGE;
    } else if (result) {
        exval = EX_SOFTWARE;
    } else {
        exval = result;
    }

// fprintf(stderr, "check for leaks now\n");
// pause()
    if (volURL)     CFRelease(volURL);

    return exval;
}
