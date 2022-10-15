/*	CFKnownLocations.c
	Copyright (c) 1999-2017, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2017, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#include "CFKnownLocations.h"

#include <CoreFoundation/CFString.h>
#include "CFPriv.h"
#include "CFInternal.h"
#include "CFUtilities.h"

#include <assert.h>

CFURLRef _Nullable _CFKnownLocationCreatePreferencesURLForUser(CFKnownLocationUser user, CFStringRef _Nullable username) {
    CFURLRef location = NULL;
    
#if TARGET_OS_MAC
    
/*
 Building for a Darwin OS. (We use these paths on Swift builds as well, so that we can interoperate a little with Darwin's defaults(1) command and the other system facilities; but you want to use the system version of CF if possible on those platforms, which will talk to cfprefsd(8) and has stronger interprocess consistency guarantees.)
 
 User:
 - Any: /Library/Preferences
 - Current: $HOME/Library/Preferences
 */
    
    switch (user) {
        case _kCFKnownLocationUserAny:
            location = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, CFSTR("/Library/Preferences"), kCFURLPOSIXPathStyle, true);
            break;
            
        case _kCFKnownLocationUserCurrent:
            username = NULL;
            // passthrough to:
        case _kCFKnownLocationUserByName: {
            CFURLRef home = CFCopyHomeDirectoryURLForUser(username);
            location = CFURLCreateWithFileSystemPathRelativeToBase(kCFAllocatorSystemDefault, CFSTR("/Library/Preferences"), kCFURLPOSIXPathStyle, true, home);
            CFRelease(home);
            
            break;
        }
            
    }
#elif !DEPLOYMENT_RUNTIME_OBJC && !TARGET_OS_WIN32 && !TARGET_OS_ANDROID
    
/*
 Building for an OS that uses the FHS, BSD's hier(7), and/or the XDG specification for paths:
 
 User:
 - Any: /usr/local/etc/
 - Current: $XDG_CONFIG_PATH (usually: $HOME/.config/).
 */
    
    switch (user) {
        case _kCFKnownLocationUserAny:
            location = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, CFSTR("/usr/local/etc"), kCFURLPOSIXPathStyle, true);
            break;
            
        case _kCFKnownLocationUserByName:
            assert(username == NULL);
            // passthrough to:
        case _kCFKnownLocationUserCurrent: {
            CFStringRef path = _CFXDGCreateConfigHomePath();
            location = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, path, kCFURLPOSIXPathStyle, true);
            CFRelease(path);
            
            break;
        }
    }
    
#elif TARGET_OS_WIN32

    switch (user) {
        case _kCFKnownLocationUserAny:
            location = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault, CFSTR("\\Users\\All Users\\AppData\\Local"), kCFURLWindowsPathStyle, true);
            break;
        case _kCFKnownLocationUserCurrent:
            username = CFGetUserName();
            // fallthrough
        case _kCFKnownLocationUserByName:
            const char *user = CFStringGetCStringPtr(username, kCFStringEncodingUTF8);
            CFURLRef userdir = CFURLCreateFromFileSystemRepresentation(kCFAllocatorSystemDefault, (const unsigned char *)user, strlen(user), true);
            CFURLRef homedir = CFURLCreateWithFileSystemPathRelativeToBase(kCFAllocatorSystemDefault, CFSTR("\\Users"), kCFURLWindowsPathStyle, true, userdir);
            location = CFURLCreateWithFileSystemPathRelativeToBase(kCFAllocatorSystemDefault, CFSTR("\\AppData\\Local"),  kCFURLWindowsPathStyle, true, homedir);
            CFRelease(homedir);
            CFRelease(userdir);
            break;
    }

#elif TARGET_OS_ANDROID

    // Android doesn't support users, and apps cannot write outside their
    // sandbox. All the preferences will be local to the application, mapping
    // "any user" and other users by name to different directories inside the
    // sandbox.
    CFURLRef userdir = CFCopyHomeDirectoryURL();
    switch (user) {
        case _kCFKnownLocationUserAny:
            location = CFURLCreateWithFileSystemPathRelativeToBase(kCFAllocatorSystemDefault, CFSTR("/Apple/Library/Preferences/AnyUser"), kCFURLPOSIXPathStyle, true, userdir);
            break;
        case _kCFKnownLocationUserByName: {
            CFURLRef tmp = CFURLCreateWithFileSystemPathRelativeToBase(kCFAllocatorSystemDefault, CFSTR("/Apple/Library/Preferences/ByUser"), kCFURLPOSIXPathStyle, true, userdir);
            location = CFURLCreateWithFileSystemPathRelativeToBase(kCFAllocatorSystemDefault, username, kCFURLPOSIXPathStyle, true, tmp);
            CFRelease(tmp);
            break;
        }
        case _kCFKnownLocationUserCurrent:
            location = CFURLCreateWithFileSystemPathRelativeToBase(kCFAllocatorSystemDefault, CFSTR("/Apple/Library/Preferences"), kCFURLPOSIXPathStyle, true, userdir);
            break;
    }
    CFRelease(userdir);

#else
    
    #error For this platform, you need to define a preferences path for both 'any user' (i.e. installation-wide preferences) or the current user.
    
#endif
    
    return location;
}
