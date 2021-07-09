/*
 * Airyx LaunchServices
 *
 * Copyright (C) 2021 Zoe Knox <zoe@pixin.net>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#import <Foundation/Foundation.h>
#import <Foundation/NSRaiseException.h>
#import <LaunchServices/LaunchServices.h>
#import <CoreFoundation/CoreFoundation.h>

// #include <KIO/ApplicationLauncherJob>
// #include <KIO/OpenUrlJob>
// #include <KService>


// FIXME: these should talk to a privileged service (maybe over DBus) but for now we'll
// just manipulate some files. The service is /System/Library/CoreServices/launchservicesd
// on macOS.
// See https://developer.gnome.org/DBusApplicationLaunching/ and 
// https://techbase.kde.org/Development/Tutorials/D-Bus/Autostart_Services

// FIXME: which error code to return for each case is just a guess

// FIXME: stuff to track per application:
// - URL schemes accepted
// - DBus activatable boolean


OSStatus LSOpenCFURLRef(CFURLRef inURL, CFURLRef _Nullable *outLaunchedURL)
{
    LSLaunchURLSpec spec;
    spec.appURL = inURL;
    spec.asyncRefCon = 0;
    spec.itemURLs = CFArrayCreate(NULL, (const void **)&inURL, 1, NULL);
    spec.launchFlags = kLSLaunchDefaults;
    OSStatus rc = LSOpenFromURLSpec(&spec, outLaunchedURL);
    CFRelease(spec.itemURLs);
    return rc;
}

static OSStatus _LSOpenAllWithSpecifiedApp(const LSLaunchURLSpec *inLaunchSpec, CFURLRef _Nullable *outLaunchedURL)
{
    const NSURL *appURL = (NSURL *)inLaunchSpec->appURL;
    if([appURL isFileURL] == NO)
        return kLSDataErr;

    // Launch app and pass all itemURLs to it
    // FIXME: use GURL or odoc events instead of passing on cmd line
    NSFileManager *fm = [NSFileManager defaultManager];
    NSString *appPath = [appURL path];
    BOOL isDir;
    BOOL exists = [fm fileExistsAtPath:appPath isDirectory:&isDir];

    if(exists == NO)
        return kLSNoExecutableErr;

    if(isDir == YES) {
        NSBundle *app = [NSBundle bundleWithPath:appPath];
        if([fm isExecutableFileAtPath:[app executablePath]] == NO)
            return kLSNoExecutableErr;

        if(outLaunchedURL != NULL)
            *outLaunchedURL = (CFURLRef)[NSURL fileURLWithPath:[app executablePath]];
        
        NSMutableArray *args = [[app infoDictionary] objectForKey:@"ProgramArguments"];
        [args addObjectsFromArray:(NSArray *)inLaunchSpec->itemURLs];
        [NSTask launchedTaskWithLaunchPath:[app executablePath] arguments:args];
    } else {
        // it's not a bundle so just try to exec the file
        if([fm isExecutableFileAtPath:appPath] == NO)
            return kLSNoExecutableErr;

        if(outLaunchedURL != NULL)
            *outLaunchedURL = (CFURLRef)[NSURL fileURLWithPath:appPath];

        [NSTask launchedTaskWithLaunchPath:appPath arguments:(NSArray*)inLaunchSpec->itemURLs];
    }
    return 0;
}

OSStatus LSOpenFromURLSpec(const LSLaunchURLSpec *inLaunchSpec, CFURLRef _Nullable *outLaunchedURL)
{
    if(inLaunchSpec->appURL) {
        // We are launching this specific application which must be a file URL
        return _LSOpenAllWithSpecifiedApp(inLaunchSpec, outLaunchedURL);
    }

    // We are opening one or more files or URLs with their preferred apps
    // If any of the items refer to application bundles, they will be launched
    // NOTE: Plain executable files are NOT considered "applications" here
    // because file permissions are not reliable. e.g. Samba shares often have
    // execute permission on non-executable files.

    NSEnumerator *items = [(inLaunchSpec->itemURLs) objectEnumerator];
    NSURL *item;

    while(item = [items nextObject]) {
        NSCLog("item = %s", [[item description] UTF8String]);
        NSArray *args = [[NSArray arrayWithObjects:[item path], nil] retain];
        [NSTask launchedTaskWithLaunchPath:@"/usr/local/bin/xdg-open" arguments:args];
        [args release];
    }

    return 0;
}

OSStatus LSRegisterURL(CFURLRef inURL, Boolean inUpdate)
{

}