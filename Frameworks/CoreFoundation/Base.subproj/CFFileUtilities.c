/*	CFFileUtilities.c
	Copyright (c) 1999-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
	Responsibility: Tony Parker
*/

#include "CFInternal.h"
#include <CoreFoundation/CFPriv.h>

#include <assert.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#if TARGET_OS_WIN32
#include <io.h>
#include <fcntl.h>

#define close _close
#define write _write
#define read _read
#define open _NS_open
#define stat _NS_stat
#define fstat _fstat
#define mkdir(a,b) _NS_mkdir(a)
#define rmdir _NS_rmdir
#define unlink _NS_unlink

#define statinfo _stat

#else

#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <pwd.h>
#include <fcntl.h>

#define statinfo stat

#endif

CF_INLINE int openAutoFSNoWait() {
#if TARGET_OS_WIN32
    return -1;
#else
    return (__CFProphylacticAutofsAccess ? open("/dev/autofs_nowait", 0) : -1);
#endif
}

CF_INLINE void closeAutoFSNoWait(int fd) {
    if (-1 != fd) close(fd);
}

CF_PRIVATE CFStringRef _CFCopyExtensionForAbstractType(CFStringRef abstractType) {
    return (abstractType ? (CFStringRef)CFRetain(abstractType) : NULL);
}


CF_PRIVATE Boolean _CFCreateDirectory(const char *path) {
    int no_hang_fd = openAutoFSNoWait();
    int ret = ((mkdir(path, 0777) == 0) ? true : false);
    closeAutoFSNoWait(no_hang_fd);
    return ret;
}

CF_PRIVATE Boolean _CFRemoveDirectory(const char *path) {
    int no_hang_fd = openAutoFSNoWait();
    int ret = ((rmdir(path) == 0) ? true : false);
    closeAutoFSNoWait(no_hang_fd);
    return ret;
}

CF_PRIVATE Boolean _CFDeleteFile(const char *path) {
    int no_hang_fd = openAutoFSNoWait();
    int ret = unlink(path) == 0;
    closeAutoFSNoWait(no_hang_fd);
    return ret;
}

static Boolean _CFReadBytesFromPathAndGetFD(CFAllocatorRef alloc, const char *path, void **bytes, CFIndex *length, CFIndex maxLength, int extraOpenFlags, int *fd) {    // maxLength is the number of bytes desired, or 0 if the whole file is desired regardless of length.
    struct statinfo statBuf;
    
    *bytes = NULL;
    
    
    int no_hang_fd = openAutoFSNoWait();
    *fd = open(path, O_RDONLY|extraOpenFlags|CF_OPENFLGS, 0666);
    
    if (*fd < 0) {
        closeAutoFSNoWait(no_hang_fd);
        return false;
    }
    if (fstat(*fd, &statBuf) < 0) {
        int saveerr = thread_errno();
        close(*fd);
        *fd = -1;
        closeAutoFSNoWait(no_hang_fd);
        thread_set_errno(saveerr);
        return false;
    }
    if ((statBuf.st_mode & S_IFMT) != S_IFREG) {
        close(*fd);
        *fd = -1;
        closeAutoFSNoWait(no_hang_fd);
        thread_set_errno(EACCES);
        return false;
    }
    if (statBuf.st_size == 0) {
        *bytes = CFAllocatorAllocate(alloc, 4, 0); // don't return constant string -- it's freed!
	if (__CFOASafe) __CFSetLastAllocationEventName(*bytes, "CFUtilities (file-bytes)");
        *length = 0;
    } else {
        CFIndex desiredLength;
        if ((maxLength >= statBuf.st_size) || (maxLength == 0)) {
            desiredLength = statBuf.st_size;
        } else {
            desiredLength = maxLength;
        }
        *bytes = CFAllocatorAllocate(alloc, desiredLength, 0);
        if (!bytes) {
            close(*fd);
            *fd = -1;
            closeAutoFSNoWait(no_hang_fd);
            return false;
        }
	if (__CFOASafe) __CFSetLastAllocationEventName(*bytes, "CFUtilities (file-bytes)");
        //	fcntl(fd, F_NOCACHE, 1);
        if (read(*fd, *bytes, desiredLength) < 0) {
            CFAllocatorDeallocate(alloc, *bytes);
            close(*fd);
            *fd = -1;
            closeAutoFSNoWait(no_hang_fd);
            return false;
        }
        *length = desiredLength;
    }
    closeAutoFSNoWait(no_hang_fd);
    return true;
}

static Boolean _CFReadBytesFromPath(CFAllocatorRef alloc, const char *path, void **bytes, CFIndex *length, CFIndex maxLength, int extraOpenFlags) {
    int fd = -1;
    Boolean result = _CFReadBytesFromPathAndGetFD(alloc, path, bytes, length, maxLength, extraOpenFlags, &fd);
    if (fd >= 0) {
        close(fd);
    }
    return result;
}
CF_PRIVATE Boolean _CFReadBytesFromFile(CFAllocatorRef alloc, CFURLRef url, void **bytes, CFIndex *length, CFIndex maxLength, int extraOpenFlags) {
    // maxLength is the number of bytes desired, or 0 if the whole file is desired regardless of length.
    
    char path[CFMaxPathSize];
    if (!CFURLGetFileSystemRepresentation(url, true, (uint8_t *)path, CFMaxPathSize)) {
        return false;
    }
    return _CFReadBytesFromPath(alloc, (const char *)path, bytes, length, maxLength, extraOpenFlags);
}

CF_PRIVATE Boolean _CFWriteBytesToFile(CFURLRef url, const void *bytes, CFIndex length) {
    int fd = -1;
    struct statinfo statBuf;
    char path[CFMaxPathSize];
    if (!CFURLGetFileSystemRepresentation(url, true, (uint8_t *)path, CFMaxPathSize)) {
        return false;
    }

    int no_hang_fd = openAutoFSNoWait();
    int mode = 0666;
    if (0 == stat(path, &statBuf)) {
        mode = statBuf.st_mode;
    } else if (thread_errno() != ENOENT) {
        closeAutoFSNoWait(no_hang_fd);
        return false;
    }
    fd = open(path, O_WRONLY|O_CREAT|O_TRUNC|CF_OPENFLGS, mode);
    if (fd < 0) {
        closeAutoFSNoWait(no_hang_fd);
        return false;
    }
    if (length && write(fd, bytes, length) != length) {
        int saveerr = thread_errno();
        close(fd);
        closeAutoFSNoWait(no_hang_fd);
        thread_set_errno(saveerr);
        return false;
    }
#if TARGET_OS_WIN32
    FlushFileBuffers((HANDLE)_get_osfhandle(fd));
#else
    fsync(fd);
#endif
    close(fd);
    closeAutoFSNoWait(no_hang_fd);
    return true;
}


/* On Mac OS 8/9, one of dirSpec and dirURL must be non-NULL.  On all other platforms, one of path and dirURL must be non-NULL
If both are present, they are assumed to be in-synch; that is, they both refer to the same directory.  */
/* Lately, dirSpec appears to be (rightfully) unused. */
CF_PRIVATE CFMutableArrayRef _CFCreateContentsOfDirectory(CFAllocatorRef alloc, char *dirPath, void *dirSpec, CFURLRef dirURL, CFStringRef matchingAbstractType) {
    CFMutableArrayRef files = NULL;
    Boolean releaseBase = false;
    CFIndex pathLength = dirPath ? strlen(dirPath) : 0;
    // MF:!!! Need to use four-letter type codes where appropriate.
    CFStringRef extension = (matchingAbstractType ? _CFCopyExtensionForAbstractType(matchingAbstractType) : NULL);
    CFIndex targetExtLen = (extension ? CFStringGetLength(extension) : 0);

#if TARGET_OS_WIN32
    // This is a replacement for 'dirent' below, and also uses wchar_t to support unicode paths
    wchar_t extBuff[CFMaxPathSize];
    int extBuffInteriorDotCount = 0; //people insist on using extensions like ".trace.plist", so we need to know how many dots back to look :(
    
    if (targetExtLen > 0) {
        CFIndex usedBytes = 0;
        CFStringGetBytes(extension, CFRangeMake(0, targetExtLen), kCFStringEncodingUTF16, 0, false, (uint8_t *)extBuff, CFMaxPathLength, &usedBytes);
        targetExtLen = usedBytes / sizeof(wchar_t);
        extBuff[targetExtLen] = '\0';
        wchar_t *extBuffStr = (wchar_t *)extBuff;
        if (extBuffStr[0] == '.')
            extBuffStr++; //skip the first dot, it's legitimate to have ".plist" for example
        
        wchar_t *extBuffDotPtr = extBuffStr;
        while ((extBuffDotPtr = wcschr(extBuffStr, '.'))) { //find the next . in the extension...
            extBuffInteriorDotCount++;
            extBuffStr = extBuffDotPtr + 1;
        }
    }
    
    wchar_t pathBuf[CFMaxPathSize];
    
    if (!dirPath) {
        if (!_CFURLGetWideFileSystemRepresentation(dirURL, true, pathBuf, CFMaxPathLength)) {
            if (extension) CFRelease(extension);
            return NULL;
        }
        
        pathLength = wcslen(pathBuf);

    } else {
        // Convert dirPath to a wide representation and put it into our pathBuf
        // Get the real length of the string in UTF16 characters
        CFStringRef dirPathStr = CFStringCreateWithCString(kCFAllocatorSystemDefault, dirPath, kCFStringEncodingUTF8);
        CFIndex strLen = CFStringGetLength(dirPathStr);
        
        // Copy the string into the buffer and terminate
        CFStringGetCharacters(dirPathStr, CFRangeMake(0, strLen), (UniChar *)pathBuf);
        pathBuf[strLen] = 0;
        
        CFRelease(dirPathStr);
    }
    
    WIN32_FIND_DATAW  file;
    HANDLE handle;
    
    if (pathLength + 2 >= CFMaxPathLength) {
        if (extension) {
            CFRelease(extension);
        }
        return NULL;
    }

    pathBuf[pathLength] = '\\';
    pathBuf[pathLength + 1] = '*';
    pathBuf[pathLength + 2] = '\0';
    handle = FindFirstFileW(pathBuf, (LPWIN32_FIND_DATAW)&file);
    if (INVALID_HANDLE_VALUE == handle) {
        pathBuf[pathLength] = '\0';
        if (extension) {
            CFRelease(extension);
        }
        return NULL;
    }

    files = CFArrayCreateMutable(alloc, 0, &kCFTypeArrayCallBacks);

    do {
        CFURLRef fileURL;
        CFIndex namelen = wcslen(file.cFileName);
        if (file.cFileName[0] == '.' && (namelen == 1 || (namelen == 2  && file.cFileName[1] == '.'))) {
            continue;
        }

        if (targetExtLen > namelen) continue;    // if the extension is the same length or longer than the name, it can't possibly match.

        if (targetExtLen > 0) {
	    if (file.cFileName[namelen - 1] == '.') continue; //filename ends with a dot, no extension
	    
	    wchar_t *fileExt = NULL;
            
            if (extBuffInteriorDotCount == 0) {
                fileExt = wcsrchr(file.cFileName, '.');
            } else { //find the Nth occurrence of . from the end of the string, to handle ".foo.bar"
                wchar_t *save = file.cFileName;
                while ((save = wcschr(save, '.')) && !fileExt) {
                    wchar_t *temp = save;
                    int moreDots = 0;
                    while ((temp = wcschr(temp, '.'))) {
                        if (++moreDots == extBuffInteriorDotCount) break;
                    }
                    if (moreDots == extBuffInteriorDotCount) {
                        fileExt = save;
                    }
                }
            }
	    
	    if (!fileExt) continue; //no extension
	    
	    if (((const wchar_t *)extBuff)[0] != '.')
		fileExt++; //omit the dot if the target file extension omits the dot
	    
	    CFIndex fileExtLen = wcslen(fileExt);
	    
	    //if the extensions are different lengths, they can't possibly match
	    if (fileExtLen != targetExtLen) continue;
	    
            // Check to see if it matches the extension we're looking for.
            if (_wcsicmp(fileExt, (const wchar_t *)extBuff) != 0) {
                continue;
            }
        }
	if (dirURL == NULL) {
	    CFStringRef dirURLStr = CFStringCreateWithBytes(alloc, (const uint8_t *)pathBuf, pathLength * sizeof(wchar_t), kCFStringEncodingUTF16, NO);
	    dirURL = CFURLCreateWithFileSystemPath(alloc, dirURLStr, kCFURLWindowsPathStyle, true);
	    CFRelease(dirURLStr);
            releaseBase = true;
        }
        // MF:!!! What about the trailing slash?
        CFStringRef fileURLStr = CFStringCreateWithBytes(alloc, (const uint8_t *)file.cFileName, namelen * sizeof(wchar_t), kCFStringEncodingUTF16, NO);
        fileURL = CFURLCreateWithFileSystemPathRelativeToBase(alloc, fileURLStr, kCFURLWindowsPathStyle, (file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false, dirURL);
        CFArrayAppendValue(files, fileURL);
        CFRelease(fileURL);
        CFRelease(fileURLStr);
    } while (FindNextFileW(handle, &file));
    FindClose(handle);
    pathBuf[pathLength] = '\0';

#elif TARGET_OS_MAC || TARGET_OS_LINUX || TARGET_OS_BSD
    uint8_t extBuff[CFMaxPathSize];
    int extBuffInteriorDotCount = 0; //people insist on using extensions like ".trace.plist", so we need to know how many dots back to look :(
    
    if (targetExtLen > 0) {
        CFStringGetBytes(extension, CFRangeMake(0, targetExtLen), CFStringFileSystemEncoding(), 0, false, extBuff, CFMaxPathLength, &targetExtLen);
        extBuff[targetExtLen] = '\0';
        char *extBuffStr = (char *)extBuff;
        if (extBuffStr[0] == '.')
            extBuffStr++; //skip the first dot, it's legitimate to have ".plist" for example
        
        char *extBuffDotPtr = extBuffStr;
        while ((extBuffDotPtr = strchr(extBuffStr, '.'))) { //find the next . in the extension...
            extBuffInteriorDotCount++;
            extBuffStr = extBuffDotPtr + 1;
        }
    }
    
    uint8_t pathBuf[CFMaxPathSize];
    
    if (!dirPath) {
        if (!CFURLGetFileSystemRepresentation(dirURL, true, pathBuf, CFMaxPathLength)) {
            if (extension) CFRelease(extension);
            return NULL;
        } else {
            dirPath = (char *)pathBuf;
            pathLength = strlen(dirPath);
        }
    }
    
#if TARGET_OS_MAC
    struct dirent buffer;
#endif
    struct dirent *dp;
    int err;
   
    int no_hang_fd = __CFProphylacticAutofsAccess ? open("/dev/autofs_nowait", 0) : -1;
 
    DIR *dirp = opendir(dirPath);
    if (!dirp) {
        if (extension) {
            CFRelease(extension);
        }
	if (-1 != no_hang_fd) close(no_hang_fd);
        return NULL;
        // raiseErrno("opendir", path);
    }
    files = CFArrayCreateMutable(alloc, 0, & kCFTypeArrayCallBacks);

#if TARGET_OS_MAC
    while((0 == readdir_r(dirp, &buffer, &dp)) && dp) {
#else
    while((dp = readdir(dirp))) {
#endif
        CFURLRef fileURL;
	unsigned namelen = strlen(dp->d_name);

        // skip . & ..; they cause descenders to go berserk
	if (dp->d_name[0] == '.' && (namelen == 1 || (namelen == 2 && dp->d_name[1] == '.'))) {
            continue;
        }
        
        if (targetExtLen > namelen) continue;    // if the extension is the same length or longer than the name, it can't possibly match.
                
        if (targetExtLen > 0) {
	    if (dp->d_name[namelen - 1] == '.') continue; //filename ends with a dot, no extension
	      
            char *fileExt = NULL;
            if (extBuffInteriorDotCount == 0) {
                fileExt = strrchr(dp->d_name, '.'); 
            } else { //find the Nth occurrence of . from the end of the string, to handle ".foo.bar"
                char *save = dp->d_name;
                while ((save = strchr(save, '.')) && !fileExt) {
                    char *temp = save;
                    int moreDots = 0;
                    while ((temp = strchr(temp, '.'))) {
                        if (++moreDots == extBuffInteriorDotCount) break;
                    }
                    if (moreDots == extBuffInteriorDotCount) {
                        fileExt = save;
                    }
                }
            }
	    
	    if (!fileExt) continue; //no extension
	    
	    if (((char *)extBuff)[0] != '.')
		fileExt++; //omit the dot if the target extension omits the dot; safe, because we checked to make sure it isn't the last character just before
	    
	    size_t fileExtLen = strlen(fileExt);
	    
	    //if the extensions are different lengths, they can't possibly match
	    if (fileExtLen != targetExtLen) continue;
	    
	    // Check to see if it matches the extension we're looking for.
            if (strncmp(fileExt, (char *)extBuff, fileExtLen) != 0) {
                continue;
            }
        }
        if (dirURL == NULL) {
            dirURL = CFURLCreateFromFileSystemRepresentation(alloc, (uint8_t *)dirPath, pathLength, true);
            releaseBase = true;
        }
#if !defined(__OpenBSD__)
        if (dp->d_type == DT_DIR || dp->d_type == DT_UNKNOWN || dp->d_type == DT_LNK || dp->d_type == DT_WHT) {
#else
        if (dp->d_type == DT_DIR || dp->d_type == DT_UNKNOWN || dp->d_type == DT_LNK) {
#endif
            Boolean isDir = (dp->d_type == DT_DIR);
            if (!isDir) {
                // Ugh; must stat.
                char subdirPath[CFMaxPathLength];
                struct statinfo statBuf;
                strlcpy(subdirPath, dirPath, sizeof(subdirPath));
                strlcat(subdirPath, "/", sizeof(subdirPath));
                strlcat(subdirPath, dp->d_name, sizeof(subdirPath));
                if (stat(subdirPath, &statBuf) == 0) {
                    isDir = ((statBuf.st_mode & S_IFMT) == S_IFDIR);
                }
            }
#if TARGET_OS_LINUX
            fileURL = CFURLCreateFromFileSystemRepresentationRelativeToBase(alloc, (uint8_t *)dp->d_name, namelen, isDir, dirURL);
#else
            fileURL = CFURLCreateFromFileSystemRepresentationRelativeToBase(alloc, (uint8_t *)dp->d_name, dp->d_namlen, isDir, dirURL);
#endif
        } else {
#if TARGET_OS_LINUX
            fileURL = CFURLCreateFromFileSystemRepresentationRelativeToBase (alloc, (uint8_t *)dp->d_name, namelen, false, dirURL);
#else
            fileURL = CFURLCreateFromFileSystemRepresentationRelativeToBase (alloc, (uint8_t *)dp->d_name, dp->d_namlen, false, dirURL);
#endif
        }
        CFArrayAppendValue(files, fileURL);
        CFRelease(fileURL);
    }
    err = closedir(dirp);
    if (-1 != no_hang_fd) close(no_hang_fd);
    if (err != 0) {
        CFRelease(files);
        if (releaseBase) {
            CFRelease(dirURL);
        }
        if (extension) {
            CFRelease(extension);
        }
        return NULL;
    }
    
#else
    
#error _CFCreateContentsOfDirectory() unknown architecture, not implemented
    
#endif

    if (extension) {
        CFRelease(extension);
    }
    if (releaseBase) {
        CFRelease(dirURL);
    }
    return files;
}

CF_PRIVATE SInt32 _CFGetPathProperties(CFAllocatorRef alloc, char *path, Boolean *exists, SInt32 *posixMode, int64_t *size, CFDateRef *modTime, SInt32 *ownerID, CFArrayRef *dirContents) {
    Boolean fileExists;
    Boolean isDirectory = false;
    
    if ((exists == NULL) && (posixMode == NULL) && (size == NULL) && (modTime == NULL) && (ownerID == NULL) && (dirContents == NULL)) {
        // Nothing to do.
        return 0;
    }
    
    struct statinfo statBuf;
    
    if (stat(path, &statBuf) != 0) {
        // stat failed, but why?
        if (thread_errno() == ENOENT) {
            fileExists = false;
        } else {
            return thread_errno();
        }
    } else {
        fileExists = true;
        isDirectory = ((statBuf.st_mode & S_IFMT) == S_IFDIR);
    }
    
    
    if (exists != NULL) {
        *exists = fileExists;
    }
    
    if (posixMode != NULL) {
        if (fileExists) {
            
            *posixMode = statBuf.st_mode;
            
        } else {
            *posixMode = 0;
        }
    }
    
    if (size != NULL) {
        if (fileExists) {
            
            *size = statBuf.st_size;
            
        } else {
            *size = 0;
        }
    }
    
    if (modTime != NULL) {
        if (fileExists) {
#if TARGET_OS_WIN32 || TARGET_OS_LINUX
            struct timespec ts = {statBuf.st_mtime, 0};
#else
            struct timespec ts = statBuf.st_mtimespec;
#endif
            *modTime = CFDateCreate(alloc, _CFAbsoluteTimeFromFileTimeSpec(ts));
        } else {
            *modTime = NULL;
        }
    }
    
    if (ownerID != NULL) {
        if (fileExists) {
            
            *ownerID = statBuf.st_uid;
            
        } else {
            *ownerID = -1;
        }
    }
    
    if (dirContents != NULL) {
        if (fileExists && isDirectory) {
            
            CFMutableArrayRef contents = _CFCreateContentsOfDirectory(alloc, (char *)path, NULL, NULL, NULL);
            
            if (contents) {
                *dirContents = contents;
            } else {
                *dirContents = NULL;
            }
        } else {
            *dirContents = NULL;
        }
    }
    return 0;
}

CF_PRIVATE SInt32 _CFGetFileProperties(CFAllocatorRef alloc, CFURLRef pathURL, Boolean *exists, SInt32 *posixMode, int64_t *size, CFDateRef *modTime, SInt32 *ownerID, CFArrayRef *dirContents) {
    
    char path[CFMaxPathSize];

    if (!CFURLGetFileSystemRepresentation(pathURL, true, (uint8_t *)path, CFMaxPathLength)) {
        return -1;
    }

    return _CFGetPathProperties(alloc, path, exists, posixMode, size, modTime, ownerID, dirContents);
}

CF_PRIVATE bool _CFURLExists(CFURLRef url) {
    Boolean exists = false;
    return url && (0 == _CFGetFileProperties(kCFAllocatorSystemDefault, url, &exists, NULL, NULL, NULL, NULL, NULL)) && exists;
}

#if TARGET_OS_WIN32
#define WINDOWS_PATH_SEMANTICS
#else
#define UNIX_PATH_SEMANTICS
#endif

#if defined(WINDOWS_PATH_SEMANTICS)
    #define CFPreferredSlash	((UniChar)'\\')
    #define CFPreferredSlashStr CFSTR("\\")
#elif defined(UNIX_PATH_SEMANTICS)
    #define CFPreferredSlash	((UniChar)'/')
    #define CFPreferredSlashStr CFSTR("/")
#else
    #error Cannot define NSPreferredSlash on this platform
#endif

static Boolean _hasDrive(CFStringRef path) {
    if (CFStringGetLength(path) >= 2) {
        UniChar firstCharacters[2];
        firstCharacters[0] = CFStringGetCharacterAtIndex(path, 0);
        firstCharacters[1] = CFStringGetCharacterAtIndex(path, 1);
        if (firstCharacters[1] == ':' &&
            (('A' <= (firstCharacters)[0] && (firstCharacters)[0] <= 'Z') ||
             ('a' <= (firstCharacters)[0] && (firstCharacters)[0] <= 'z'))
            ) {
            return true;
        }
    }
    return false;
}

static Boolean _hasNet(CFStringRef path) {
    if (CFStringGetLength(path) >= 2) {
        UniChar firstCharacters[2];
        firstCharacters[0] = CFStringGetCharacterAtIndex(path, 0);
        firstCharacters[1] = CFStringGetCharacterAtIndex(path, 1);
        if (firstCharacters[0] == '\\' && firstCharacters[1] == '\\') return true;
    }
    return false;
}

#define HAS_DRIVE(S) ((S)[1] == ':' && (('A' <= (S)[0] && (S)[0] <= 'Z') || ('a' <= (S)[0] && (S)[0] <= 'z')))
#define HAS_NET(S) ((S)[0] == '\\' && (S)[1] == '\\')

#if defined(WINDOWS_PATH_SEMANTICS)
    #define IS_SLASH(C)	((C) == '\\' || (C) == '/')
#elif defined(UNIX_PATH_SEMANTICS)
    #define IS_SLASH(C)	((C) == '/')
#endif

CF_PRIVATE UniChar _CFGetSlash() {
    return CFPreferredSlash;
}

CF_PRIVATE CFStringRef _CFGetSlashStr() {
    return CFPreferredSlashStr;
}

CF_PRIVATE Boolean _CFIsAbsolutePath(UniChar *unichars, CFIndex length) {
    if (length < 1) {
        return false;
    }
#if defined(WINDOWS_PATH_SEMANTICS)
    if (unichars[0] == '~') {
        return true;
    }
    if (length < 2) {
        return false;
    }
    if (HAS_NET(unichars)) {
        return true;
    }
    if (length < 3) {
        return false;
    }
    if (IS_SLASH(unichars[2]) && HAS_DRIVE(unichars)) {
        return true;
    }
#else
    if (unichars[0] == '~') {
        return true;
    }
    if (IS_SLASH(unichars[0])) {
        return true;
    }
#endif
    return false;
}

CF_PRIVATE Boolean _CFStripTrailingPathSlashes(UniChar *unichars, CFIndex *length) {
    Boolean destHasDrive = (1 < *length) && HAS_DRIVE(unichars);
    CFIndex oldLength = *length;
    while (((destHasDrive && 3 < *length) || (!destHasDrive && 1 < *length)) && IS_SLASH(unichars[*length - 1])) {
        (*length)--;
    }
    return (oldLength != *length);
}

static Boolean _CFAppendTrailingPathSlash(UniChar *unichars, CFIndex *length, CFIndex maxLength) {
    if (maxLength < *length + 1) {
        return false;
    }
    switch (*length) {
        case 0:
            break;
        case 1:
            if (!IS_SLASH(unichars[0])) {
                unichars[(*length)++] = CFPreferredSlash;
            }
            break;
        case 2:
            if (!HAS_DRIVE(unichars) && !HAS_NET(unichars)) {
                unichars[(*length)++] = CFPreferredSlash;
            }
            break;
        default:
            unichars[(*length)++] = CFPreferredSlash;
            break;
    }
    return true;
}

CF_PRIVATE void _CFAppendTrailingPathSlash2(CFMutableStringRef path) {
    static const UniChar slash[1] = { CFPreferredSlash };
    CFIndex len = CFStringGetLength(path);
    if (len == 0) {
        // Do nothing for this case
    } else if (len == 1) {
        UniChar character = CFStringGetCharacterAtIndex((CFStringRef)path, 0);
        if (!IS_SLASH(character)) {
            CFStringAppendCharacters(path, slash, 1);
        }
    } else if (len == 2) {
        if (!_hasDrive(path) && !_hasNet(path)) {
            CFStringAppendCharacters(path, slash, 1);
        }
    } else {
        CFStringAppendCharacters(path, slash, 1);
    }
}

CF_PRIVATE void _CFAppendConditionalTrailingPathSlash2(CFMutableStringRef path) {
    static const UniChar slash[1] = { CFPreferredSlash };
    UniChar character = CFStringGetCharacterAtIndex((CFStringRef)path, CFStringGetLength(path) - 1);
    if (!IS_SLASH(character)) {
        CFStringAppendCharacters(path, slash, 1);
    }
}

CF_PRIVATE void _CFAppendPathComponent2(CFMutableStringRef path, CFStringRef component) {
    _CFAppendTrailingPathSlash2(path);
    CFStringAppend(path, component);
}

CF_PRIVATE Boolean _CFAppendPathComponent(UniChar *unichars, CFIndex *length, CFIndex maxLength, UniChar *component, CFIndex componentLength) {
    if (0 == componentLength) {
        return true;
    }
    if (maxLength < *length + 1 + componentLength) {
        return false;
    }
    _CFAppendTrailingPathSlash(unichars, length, maxLength);
    memmove(unichars + *length, component, componentLength * sizeof(UniChar));
    *length += componentLength;
    return true;
}

CF_PRIVATE Boolean _CFAppendPathExtension2(CFMutableStringRef path, CFStringRef extension) {
    if (!path) {
        return false;
    }
    
    if (0 < CFStringGetLength(extension) && IS_SLASH(CFStringGetCharacterAtIndex(extension, 0))) {
        return false;
    }
    if (1 < CFStringGetLength(extension)) {
        if (_hasDrive(extension)) return false;
    }
    
    Boolean destHasDrive = (1 < CFStringGetLength(path)) && _hasDrive(path);
    while (((destHasDrive && 3 < CFStringGetLength(path)) || (!destHasDrive && 1 < CFStringGetLength(path))) && IS_SLASH(CFStringGetCharacterAtIndex(path, CFStringGetLength(path) - 1))) {
        CFStringDelete(path, CFRangeMake(CFStringGetLength(path) - 1, 1));
    }

    if (CFStringGetLength(path) == 0) {
        return false;
    }
    
    UniChar firstChar = CFStringGetCharacterAtIndex(path, 0);
    CFIndex newLength = CFStringGetLength(path);
    switch (newLength) {
        case 0:
            return false;
        case 1:
            if (IS_SLASH(firstChar) || firstChar == '~') {
                return false;
            }
            break;
        case 2:
            if (_hasDrive(path) || _hasNet(path)) {
                return false;
            }
            break;
        case 3:
            if (IS_SLASH(CFStringGetCharacterAtIndex(path, 2)) && _hasDrive(path)) {
                return false;
            }
            break;
    }
    if (0 < newLength && firstChar == '~') {
        // Make sure we have a slash in the string
        if (!CFStringFindWithOptions(path, CFPreferredSlashStr, CFRangeMake(1, newLength - 1), 0, NULL)) {
            return false;
        }
    }
    static const UniChar dotChar = '.';
    CFStringAppendCharacters(path, &dotChar, 1);
    CFStringAppend(path, extension);
    return true;
}

CF_PRIVATE Boolean _CFAppendPathExtension(UniChar *unichars, CFIndex *length, CFIndex maxLength, UniChar *extension, CFIndex extensionLength) {
    if (maxLength < *length + 1 + extensionLength) {
        return false;
    }
    if ((0 < extensionLength && IS_SLASH(extension[0])) || (1 < extensionLength && HAS_DRIVE(extension))) {
        return false;
    }
    _CFStripTrailingPathSlashes(unichars, length);
    switch (*length) {
    case 0:
        return false;
    case 1:
        if (IS_SLASH(unichars[0]) || unichars[0] == '~') {
            return false;
        }
        break;
    case 2:
        if (HAS_DRIVE(unichars) || HAS_NET(unichars)) {
            return false;
        }
        break;
    case 3:
        if (IS_SLASH(unichars[2]) && HAS_DRIVE(unichars)) {
            return false;
        }
        break;
    }
    if (0 < *length && unichars[0] == '~') {
        CFIndex idx;
        Boolean hasSlash = false;
        for (idx = 1; idx < *length; idx++) {
            if (IS_SLASH(unichars[idx])) {
                hasSlash = true;
                break;
            }
        }
        if (!hasSlash) {
            return false;
        }
    }
    unichars[(*length)++] = '.';
    memmove(unichars + *length, extension, extensionLength * sizeof(UniChar));
    *length += extensionLength;
    return true;
}

CF_PRIVATE Boolean _CFTransmutePathSlashes(UniChar *unichars, CFIndex *length, UniChar replSlash) {
    CFIndex didx, sidx, scnt = *length;
    sidx = (1 < *length && HAS_NET(unichars)) ? 2 : 0;
    didx = sidx;
    while (sidx < scnt) {
        if (IS_SLASH(unichars[sidx])) {
            unichars[didx++] = replSlash;
            for (sidx++; sidx < scnt && IS_SLASH(unichars[sidx]); sidx++);
        } else {
            unichars[didx++] = unichars[sidx++];
        }
    }
    *length = didx;
    return (scnt != didx);
}

CF_PRIVATE CFStringRef _CFCreateLastPathComponent(CFAllocatorRef alloc, CFStringRef path, CFIndex *slashIndex) {
    CFIndex len = CFStringGetLength(path);
    if (len < 2) {
        // Can't be any path components in a string this short
        if (slashIndex) *slashIndex = -1;
        return (CFStringRef)CFRetain(path);
    }
    
    // Find the last slash
    for (CFIndex i = len - 1; i >= 0; i--) {
        if (IS_SLASH(CFStringGetCharacterAtIndex(path, i))) {
            if (slashIndex) *slashIndex = i;
            return CFStringCreateWithSubstring(alloc, path, CFRangeMake(i + 1, len - i - 1));
        }
    }
    
    // Strip any drive if we have one
    if (len > 2 && _hasDrive(path)) {
        if (slashIndex) *slashIndex = -1;
        return CFStringCreateWithSubstring(alloc, path, CFRangeMake(2, len - 2));
    }
    
    // No slash, so just return the same string
    if (slashIndex) *slashIndex = -1;
    return (CFStringRef)CFRetain(path);
}

CF_PRIVATE CFIndex _CFStartOfLastPathComponent(UniChar *unichars, CFIndex length) {
    CFIndex idx;
    if (length < 2) {
        return 0;
    }
    for (idx = length - 1; idx; idx--) {
        if (IS_SLASH(unichars[idx - 1])) {
            return idx;
        }
    }
    if ((2 < length) && HAS_DRIVE(unichars)) {
        return 2;
    }
    return 0;
}

CF_PRIVATE CFIndex _CFStartOfLastPathComponent2(CFStringRef path) {
    CFIndex length = CFStringGetLength(path);
    if (length < 2) {
        return 0;
    }
    for (CFIndex idx = length - 1; idx; idx--) {
        if (IS_SLASH(CFStringGetCharacterAtIndex(path, idx - 1))) {
            return idx;
        }
    }
    if ((2 < length && _hasDrive(path))) {
        return 2;
    }
    return 0;
}

CF_PRIVATE CFIndex _CFLengthAfterDeletingLastPathComponent(UniChar *unichars, CFIndex length) {
    CFIndex idx;
    if (length < 2) {
        return 0;
    }
    for (idx = length - 1; idx; idx--) {
        if (IS_SLASH(unichars[idx - 1])) {
            if ((idx != 1) && (!HAS_DRIVE(unichars) || idx != 3)) {
                return idx - 1;
            }
            return idx;
        }
    }
    if ((2 < length) && HAS_DRIVE(unichars)) {
        return 2;
    }
    return 0;
}

CF_PRIVATE CFIndex _CFStartOfPathExtension2(CFStringRef path) {
    if (CFStringGetLength(path) < 2) {
        return 0;
    }
    Boolean hasDrive = _hasDrive(path);
    for (CFIndex idx = CFStringGetLength(path) - 1; idx; idx--) {
        UniChar thisCharacter = CFStringGetCharacterAtIndex(path, idx);
        if (IS_SLASH(thisCharacter)) {
            return 0;
        }
        if (thisCharacter != '.') {
            continue;
        }
        if (idx == 2 && hasDrive) {
            return 0;
        }
        return idx;
    }
    return 0;
}

CF_PRIVATE CFIndex _CFStartOfPathExtension(UniChar *unichars, CFIndex length) {
    CFIndex idx;
    if (length < 2) {
        return 0;
    }
    for (idx = length - 1; idx; idx--) {
        if (IS_SLASH(unichars[idx - 1])) {
            return 0;
        }
        if (unichars[idx] != '.') {
            continue;
        }
        if (idx == 2 && HAS_DRIVE(unichars)) {
            return 0;
        }
        return idx;
    }
    return 0;
}

CF_PRIVATE CFIndex _CFLengthAfterDeletingPathExtension2(CFStringRef path) {
    CFIndex start = _CFStartOfPathExtension2(path);
    return ((0 < start) ? start : CFStringGetLength(path));
}

CF_PRIVATE CFIndex _CFLengthAfterDeletingPathExtension(UniChar *unichars, CFIndex length) {
    CFIndex start = _CFStartOfPathExtension(unichars, length);
    return ((0 < start) ? start : length);
}

#if TARGET_OS_WIN32
#define	DT_DIR		 4
#define	DT_REG		 8
#define	DT_LNK		10
#define	DT_UNKNOWN	 0
#endif

// NOTE: on Windows the filename is UTF16-encoded, the fileNameLen is result of wcslen. This function automatically skips '.' and '..', and '._' files
CF_PRIVATE void _CFIterateDirectory(CFStringRef directoryPath, Boolean appendSlashForDirectories, CFArrayRef stuffToPrefix, Boolean (^fileHandler)(CFStringRef fileName, CFStringRef fileNameWithPrefix, uint8_t fileType)) {
    char directoryPathBuf[CFMaxPathSize];
    if (!CFStringGetFileSystemRepresentation(directoryPath, directoryPathBuf, CFMaxPathSize)) return;
    
#if TARGET_OS_WIN32
    // Make sure there is room for the additional space we need in the win32 api
    if (strlen(directoryPathBuf) > CFMaxPathSize - 2) return;

    strlcat(directoryPathBuf, "\\*", CFMaxPathSize);

    UniChar wideBuf[CFMaxPathSize];

    // Convert UTF8 buffer to windows appropriate UTF-16LE
    // Get the real length of the string in UTF16 characters
    CFStringRef cfStr = CFStringCreateWithCString(kCFAllocatorSystemDefault, directoryPathBuf, kCFStringEncodingUTF8);

    // Copy the string into the buffer and terminate
    CFStringGetCharacters(cfStr, CFRangeMake(0, CFStringGetLength(cfStr)), wideBuf);
    wideBuf[CFStringGetLength(cfStr)] = L'\0';

    CFRelease(cfStr);

    WIN32_FIND_DATAW file;
    HANDLE handle = FindFirstFileW(wideBuf, &file);
    if (handle != INVALID_HANDLE_VALUE) {
        do {
            CFIndex nameLen = wcslen(file.cFileName);
            if (file.cFileName[0] == '.' && (nameLen == 1 || (nameLen == 2  && file.cFileName[1] == '.'))) {
                continue;
            }

            CFStringRef fileName = CFStringCreateWithCharacters(kCFAllocatorSystemDefault, file.cFileName, nameLen);
            if (!fileName) {
                continue;
            }

            const UniChar kSlash = CFPreferredSlash;

            CFStringAppendBuffer buffer;
            CFStringInitAppendBuffer(kCFAllocatorSystemDefault, &buffer);

            if (stuffToPrefix) {
              for (CFIndex i = 0, e = CFArrayGetCount(stuffToPrefix); i < e; i++) {
                CFStringRef entry = CFArrayGetValueAtIndex(stuffToPrefix, i);
                CFStringAppendStringToAppendBuffer(&buffer, entry);
                if (CFStringGetCharacterAtIndex(entry, CFStringGetLength(entry) - 1) != _CFGetSlash()) {
                  CFStringAppendCharactersToAppendBuffer(&buffer, &kSlash, 1);
                }
              }
            }

            CFStringAppendStringToAppendBuffer(&buffer, fileName);
            Boolean isDirectory = file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
            if (appendSlashForDirectories && isDirectory) {
              CFStringAppendCharactersToAppendBuffer(&buffer, &kSlash, 1);
            }

            CFMutableStringRef filePath = CFStringCreateMutableWithAppendBuffer(&buffer);
            Boolean result = fileHandler(fileName, filePath, isDirectory ? DT_DIR : DT_REG);
            CFRelease(fileName);
            CFRelease(filePath);
            if (!result) break;
        } while (FindNextFileW(handle, &file));

        FindClose(handle);
    }
#else
    DIR *dirp;
    struct dirent *dent;
    if ((dirp = opendir(directoryPathBuf))) {
        while ((dent = readdir(dirp))) {
#if TARGET_OS_LINUX
            CFIndex nameLen = strlen(dent->d_name);
            if (dent->d_type == DT_UNKNOWN) {
                // on some old file systems readdir may always fill d_type as DT_UNKNOWN (0), double check with stat
                struct stat statBuf;
                char pathToStat[sizeof(dent->d_name)];
                strncpy(pathToStat, directoryPathBuf, sizeof(pathToStat));
                strlcat(pathToStat, "/", sizeof(pathToStat));
                strlcat(pathToStat, dent->d_name, sizeof(pathToStat));
                if (stat(pathToStat, &statBuf) == 0) {
                    if (S_ISDIR(statBuf.st_mode)) {
                        dent->d_type = DT_DIR;
                    } else if (S_ISREG(statBuf.st_mode)) {
                        dent->d_type = DT_REG;
                    } else if (S_ISLNK(statBuf.st_mode)) {
                        dent->d_type = DT_LNK;
                    }
                }
            }
#else
            CFIndex nameLen = dent->d_namlen;
#endif
            if (0 == nameLen || 0 == dent->d_fileno || ('.' == dent->d_name[0] && (1 == nameLen || (2 == nameLen && '.' == dent->d_name[1]) || '_' == dent->d_name[1]))) {
                continue;
            }
            
            // This part is easy
            CFStringRef fileName = CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, dent->d_name);
            
            // This buffer has to be 1 bigger than the size of the one in the dirent so we can hold the extra '/' if it's required
            // Be sure to initialize the first character to null, so that strlcat below works correctly
            char fullPathToFile[sizeof(dent->d_name) + 1];
            fullPathToFile[0] = 0;
            CFIndex startAt = 0;

            // Prefix anything as requested
            if (stuffToPrefix) {
                for (CFIndex i = 0; i < CFArrayGetCount(stuffToPrefix); i++) {
                    CFStringRef onePrefix = CFArrayGetValueAtIndex(stuffToPrefix, i);
                    // Note: CFStringGetBytes does not null-terminate - we will do that below
                    CFIndex usedBufLen = 0;
                    startAt += CFStringGetBytes(onePrefix, CFRangeMake(0, CFStringGetLength(onePrefix)), CFStringFileSystemEncoding(), 0, false, (UInt8 *)fullPathToFile + startAt, sizeof(fullPathToFile) - startAt, &usedBufLen);
                    if (startAt > 0) { // Add a / if the string did not have one
                        // In some cases, startAt and usedBufLen differ (e.g. the num of bytes returned is less than the number written to the buffer).
                        if (startAt < usedBufLen) {
                            if (fullPathToFile[usedBufLen - 1] != (char)_CFGetSlash()) {
                                fullPathToFile[usedBufLen] = (char)_CFGetSlash();
                                startAt += (usedBufLen - startAt) + 1;
                            }
                        } else {
                            if (fullPathToFile[startAt - 1] != (char)_CFGetSlash()) {
                                fullPathToFile[startAt++] = (char)_CFGetSlash();
                            }
                        }
                    }
                }
                fullPathToFile[startAt] = 0;
            }
            
            // Suffix the slash if the file is a directory and it is requested
            Boolean isDirectory = false;
            if (appendSlashForDirectories) {
                // If the file is a directory we need to append a /
                // Regardless if it is a directory or a file, we need to append that result to pathPrefix, if pathPrefix is non-null
                
                // Do some checks to see if this is a directory, and if so make sure that we honor the appendSlash argument
                if (dent->d_type == DT_DIR) {
                    isDirectory = true;
                }
#if TARGET_OS_MAC || TARGET_OS_LINUX || TARGET_OS_BSD
                else if (dent->d_type == DT_UNKNOWN) {
                    // We need to do an additional stat on this to see if it's really a directory or not.
                    // This path should be uncommon.
                    struct stat statBuf;
                    char pathToStat[sizeof(dent->d_name)];
                    strncpy(pathToStat, directoryPathBuf, sizeof(pathToStat));
                    strlcat(pathToStat, "/", sizeof(pathToStat));
                    strlcat(pathToStat, dent->d_name, sizeof(pathToStat));
                    if (stat(pathToStat, &statBuf) == 0) {
                        isDirectory = S_ISDIR(statBuf.st_mode);
                    }
                }
#endif
            }
            
            if (isDirectory) {
                // Append the file name and the trailing /
                strlcat(fullPathToFile, dent->d_name, sizeof(fullPathToFile));
                strlcat(fullPathToFile, "/", sizeof(fullPathToFile));
            } else if (stuffToPrefix) {
                // Append just the file name to our previously-used buffer
                strlcat(fullPathToFile, dent->d_name, sizeof(fullPathToFile));
            }
            
            
            CFStringRef fileNameWithPrefix = NULL;
            if (stuffToPrefix || isDirectory) {
                // We used the buffer - create a new string
                fileNameWithPrefix = CFStringCreateWithFileSystemRepresentation(kCFAllocatorSystemDefault, fullPathToFile);
            }
            
            if (!fileName) {
                // Don't call the block with a NULL file name
                if (fileNameWithPrefix) CFRelease(fileNameWithPrefix);
                continue;
            }
            if (!fileNameWithPrefix) {
                // Don't call the block with a NULL fileNameWithPrefix either - but here we can fallback to the fileName
                fileNameWithPrefix = CFRetain(fileName);
            }
            
            Boolean result = fileHandler(fileName, fileNameWithPrefix, dent->d_type);
            
            CFRelease(fileName);
            CFRelease(fileNameWithPrefix);
            if (!result) break;
        }
        (void)closedir(dirp);
    }
#endif
}

#if !DEPLOYMENT_RUNTIME_OBJC

// https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html
// Version 0.8

// This may not be safe to assume
#define _kCFXDGStringEncoding kCFStringEncodingUTF8

// All paths set in these environment variables must be absolute. If an implementation encounters a relative path in any of these variables it should consider the path invalid and ignore it.
static CFStringRef _CFXDGCreateHome(void) {
    const char *home = __CFgetenv("HOME");
    if (home && strnlen(home, CFMaxPathSize) > 0) {
        return CFStringCreateWithCString(kCFAllocatorSystemDefault, home, _kCFXDGStringEncoding);
    } else {
        return CFRetain(CFSTR(""));
    }
}

/// a single base directory relative to which user-specific data files should be written. This directory is defined by the environment variable $XDG_DATA_HOME.
CF_CROSS_PLATFORM_EXPORT
CFStringRef _CFXDGCreateDataHomePath(void) {
    // $XDG_DATA_HOME defines the base directory relative to which user specific data files should be stored. If $XDG_DATA_HOME is either not set or empty, a default equal to $HOME/.local/share should be used.
    const char *dataHome = __CFgetenv("XDG_DATA_HOME");
    if (dataHome && strnlen(dataHome, CFMaxPathSize) > 1 && dataHome[0] == '/') {
        return CFStringCreateWithCString(kCFAllocatorSystemDefault, dataHome, _kCFXDGStringEncoding);
    } else {
        CFStringRef home = _CFXDGCreateHome();
        CFStringRef result = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%@/.local/share"), home);
        CFRelease(home);
        return result;
    }
}

/// a single base directory relative to which user-specific configuration files should be written. This directory is defined by the environment variable $XDG_CONFIG_HOME.
CF_CROSS_PLATFORM_EXPORT
CFStringRef _CFXDGCreateConfigHomePath(void) {
    // $XDG_CONFIG_HOME defines the base directory relative to which user specific configuration files should be stored. If $XDG_CONFIG_HOME is either not set or empty, a default equal to $HOME/.config should be used.
    const char *configHome = __CFgetenv("XDG_CONFIG_HOME");
    if (configHome && strnlen(configHome, CFMaxPathSize) > 1 && configHome[0] == '/') {
        return CFStringCreateWithCString(kCFAllocatorSystemDefault, configHome, _kCFXDGStringEncoding);
    } else {
        CFStringRef home = _CFXDGCreateHome();
        CFStringRef result = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%@/.config"), home);
        CFRelease(home);
        return result;
    }
}

/// a set of preference ordered base directories relative to which data files should be searched. This set of directories is defined by the environment variable $XDG_DATA_DIRS.
CF_CROSS_PLATFORM_EXPORT
CFArrayRef _CFXDGCreateDataDirectoriesPaths(void) {
    // $XDG_DATA_DIRS defines the preference-ordered set of base directories to search for data files in addition to the $XDG_DATA_HOME base directory. The directories in $XDG_DATA_DIRS should be separated with a colon ':'.
    // If $XDG_DATA_DIRS is either not set or empty, a value equal to /usr/local/share/:/usr/share/ should be used.
    const char *dataDirectoriesPaths = __CFgetenv("XDG_DATA_DIRS");
    CFStringRef defaultPath[2];
    defaultPath[0] = CFSTR("/usr/local/share");
    defaultPath[1] = CFSTR("/usr/share");
    if ((dataDirectoriesPaths == NULL) || (dataDirectoriesPaths[0] == '\0')) {
        // Environmental variable not set. Return default value.
        return CFArrayCreate(kCFAllocatorSystemDefault, (const void **)defaultPath, 2, &kCFTypeArrayCallBacks);
    }
    CFArrayRef dataDirPathsArray = _CFCreateCFArrayByTokenizingString(dataDirectoriesPaths, ':');
    if(CFArrayGetCount(dataDirPathsArray) == 0) {
        CFStringRef logMessage = CFSTR("Value set in XDG_DATA_DIRS variable not honoured. Returning the default.");
        CFLog(kCFLogLevelWarning, CFSTR("%@"), logMessage);
        CFRelease(dataDirPathsArray);
        return CFArrayCreate(kCFAllocatorSystemDefault, (const void **)defaultPath, 2, &kCFTypeArrayCallBacks);
    }
    return dataDirPathsArray;
}


/// a set of preference ordered base directories relative to which configuration files should be searched. This set of directories is defined by the environment variable $XDG_CONFIG_DIRS.
CF_CROSS_PLATFORM_EXPORT
CFArrayRef _CFXDGCreateConfigDirectoriesPaths(void) {
    // $XDG_CONFIG_DIRS defines the preference-ordered set of base directories to search for configuration files in addition to the $XDG_CONFIG_HOME base directory. The directories in $XDG_CONFIG_DIRS should be separated with a colon ':'.
    // If $XDG_CONFIG_DIRS is either not set or empty, a value equal to /etc/xdg should be used.
    const char *configDirectoriesPaths = __CFgetenv("XDG_CONFIG_DIRS");
    CFStringRef defaultPath[1];
    defaultPath[0] = CFSTR("/etc/xdg");
    if ((configDirectoriesPaths == NULL) || (configDirectoriesPaths[0] == '\0')) {
        //Environmental variable not set. Return default value.
        return CFArrayCreate(kCFAllocatorSystemDefault, (const void **)defaultPath, 1, &kCFTypeArrayCallBacks);
    }
    CFArrayRef configDirPathsArray = _CFCreateCFArrayByTokenizingString(configDirectoriesPaths, ':');
    if(CFArrayGetCount(configDirPathsArray) == 0) {
        CFStringRef logMessage = CFSTR("Value set in XDG_CONFIG_DIRS variable not honoured. Returning the default.");
        CFLog(kCFLogLevelWarning, CFSTR("%@"), logMessage);
        CFRelease(configDirPathsArray);
        return CFArrayCreate(kCFAllocatorSystemDefault, (const void **)defaultPath, 1, &kCFTypeArrayCallBacks);
    }
    return configDirPathsArray;
}

/// a single base directory relative to which user-specific non-essential (cached) data should be written. This directory is defined by the environment variable $XDG_CACHE_HOME.
CF_CROSS_PLATFORM_EXPORT
CFStringRef _CFXDGCreateCacheDirectoryPath(void) {
    //$XDG_CACHE_HOME defines the base directory relative to which user specific non-essential data files should be stored. If $XDG_CACHE_HOME is either not set or empty, a default equal to $HOME/.cache should be used.
    const char *cacheHome = __CFgetenv("XDG_CACHE_HOME");
    const char *path = __CFgetenv("PATH");
    if (cacheHome && strnlen(cacheHome, CFMaxPathSize) > 1 && cacheHome[0] == '/') {
        return CFStringCreateWithCString(kCFAllocatorSystemDefault, cacheHome, _kCFXDGStringEncoding);
    } else {
        CFStringRef home = _CFXDGCreateHome();
        CFStringRef result = CFStringCreateWithFormat(kCFAllocatorSystemDefault, NULL, CFSTR("%@/.cache"), home);
        CFRelease(home);
        return result;
    }
}

/// a single base directory relative to which user-specific runtime files and other file objects should be placed. This directory is defined by the environment variable $XDG_RUNTIME_DIR.
CF_CROSS_PLATFORM_EXPORT
CFStringRef _CFXDGCreateRuntimeDirectoryPath(void) {
    const char *runtimeDir = __CFgetenv("XDG_RUNTIME_DIR");
    if (runtimeDir && strnlen(runtimeDir, CFMaxPathSize) > 1 && runtimeDir[0] == '/') {
        return CFStringCreateWithCString(kCFAllocatorSystemDefault, runtimeDir, _kCFXDGStringEncoding);
    } else {
        // If $XDG_RUNTIME_DIR is not set applications should fall back to a replacement directory with similar capabilities and print a warning message.
        return CFStringCreateWithCString(kCFAllocatorSystemDefault, "", _kCFXDGStringEncoding);
    }
}


CF_PRIVATE CFArrayRef _CFCreateCFArrayByTokenizingString(const char *values, char delimiter) {
    size_t pathCount = 0;
    char* tmpDirectoriesPaths = (char*)values;
    char* last_colon = 0;
    // Count how many paths will be extracted.
    while (*tmpDirectoriesPaths)
    {
        if (*tmpDirectoriesPaths == delimiter)
        {
            pathCount++;
            last_colon = tmpDirectoriesPaths;
        }
        tmpDirectoriesPaths++;
    }
    // Add count for trailing path unless ending with colon.
    pathCount += last_colon < (values + strlen(values) - 1);
    if (pathCount > 64) {
        return CFArrayCreate(kCFAllocatorSystemDefault, NULL, 0, &kCFTypeArrayCallBacks);
    }
    if (pathCount > 0)
    {
        size_t validPathCount  = 0;
        CFStringRef pathList[pathCount];
        char* copyDirPath = strdup(values);
        char delimiterStr[2];
        delimiterStr[0] = delimiter;
        delimiterStr[1] = '\0';
        char* path = strtok(copyDirPath, delimiterStr);
        while (path)
        {
            assert(validPathCount < pathCount);
            char* pathString  = strdup(path);
            CFStringRef dirPath = CFStringCreateWithCString(kCFAllocatorSystemDefault, pathString, _kCFXDGStringEncoding);
            CFStringRef slash = CFSTR("/");
            CFStringRef tilde = CFSTR("~");
            // Check for absolutePath, if not ignore.
            if (CFStringHasPrefix(dirPath, slash) || CFStringHasPrefix(dirPath, tilde) ) {
                pathList[validPathCount++] = dirPath;
            } else {
                CFRelease(dirPath);
            }
            path = strtok(NULL, delimiterStr);
            free(pathString);
        }
        free(copyDirPath);
        CFArrayRef pathArray = CFArrayCreate(kCFAllocatorSystemDefault, (const void **)pathList  , validPathCount, &kCFTypeArrayCallBacks);
        for(int i = 0; i < validPathCount; i ++) {
            CFRelease(pathList[i]);
        }
        return pathArray;
    }
    return CFArrayCreate(kCFAllocatorSystemDefault, NULL, 0, &kCFTypeArrayCallBacks);
}

#endif // !DEPLOYMENT_RUNTIME_OBJC
