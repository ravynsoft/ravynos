/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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
#ifdef HAVE_CFPLUGIN

#include <IOKit/IOCFPlugIn.h>
#include <IOKit/kext/OSKext.h>

#if 0 // for local logging
#include <asl.h>
void __iocfpluginlog(const char *format, ...);
#endif

// Inited by class IOCFPlugInIniter
CFUUIDRef gIOCFPlugInInterfaceID = NULL;

typedef struct LookUUIDContextStruct {
    const void *	result;
    CFUUIDRef		key;
} LookUUIDContext;

static void
_IOGetWithUUIDKey(const void *key, const void * value, void *ctx)
{
    LookUUIDContext * 	context = (LookUUIDContext *) ctx;
    CFUUIDRef 		uuid;

    uuid = CFUUIDCreateFromString(NULL, (CFStringRef)key);
    if( uuid) {
        if( CFEqual( uuid, context->key))
            context->result = value;
        CFRelease(uuid);
    }
}

static CFURLRef
_CreateIfReachable( CFStringRef thePath )
{
    CFURLRef        pathURL = NULL;  // caller will release
    
    pathURL = CFURLCreateWithFileSystemPath(NULL, thePath,
                                            kCFURLPOSIXPathStyle,
                                            TRUE);
    if (pathURL) {
        if (CFURLResourceIsReachable(pathURL, NULL) == false) {
            CFRelease( pathURL );
            pathURL = NULL;
        }
    }
    return(pathURL);
}

/* Starting in 10.9 Plugins will be looked up in the following order:
 * 1) if IOCFPlugInTypes in the registry entry has a plugin name starting with
 *    a '/' we assume it is a full path and look for the plugin there
 * 2) if #1 fails we will append "/System/Library/Extensions/" to the plugin
 *    name we get from the registry (this was the pre 10.9 behavior)
 * 3) if #2 fails we will append "/Library/Extensions/" to the plugin name we
 *    get from the registry (10.9 is where we started loading kexts from /L/E/
 *    and /S/L/E/ )
 */

static kern_return_t
IOFindPlugIns( io_service_t service,
              CFUUIDRef pluginType,
              CFArrayRef * factories, CFArrayRef * plists )
{
    CFURLRef		pluginURL = NULL;       // must release
    CFPlugInRef		onePlugin = NULL;
    CFBundleRef		bundle;
    CFDictionaryRef	plist;
    CFDictionaryRef	matching;
    CFDictionaryRef	pluginTypes = NULL;     // must release
    CFMutableStringRef  pluginPath = NULL;  // must release
    LookUUIDContext	context;
    CFStringRef		pluginName = NULL;      // do not release
    boolean_t		matches;
    kern_return_t	kr = kIOReturnSuccess;
    
    *factories      = 0;
    *plists         = 0;
    
    do {
        pluginPath = CFStringCreateMutable( kCFAllocatorDefault, 0 );
        if ( pluginPath == NULL ) {
            continue;
        }
        
        pluginTypes = IORegistryEntryCreateCFProperty( service, CFSTR(kIOCFPlugInTypesKey),
                                                      kCFAllocatorDefault, kNilOptions );
        if ( pluginTypes == NULL ) {
            continue;
        }
        
        context.key = pluginType;
        context.result = 0;
        CFDictionaryApplyFunction( pluginTypes, &_IOGetWithUUIDKey, &context);
        pluginName = (CFStringRef) context.result;
        if ( pluginName == NULL ) {
            continue;
        }
        
#if 0 //
        const char *    myPtr;
        myPtr = CFStringGetCStringPtr(pluginName, kCFStringEncodingMacRoman);
        __iocfpluginlog("%s pluginName \"%s\" \n", __func__,
                        myPtr ? myPtr : "no name");
#endif
        
        // see if the plugin name is possibly a full path
        if ( CFStringGetCharacterAtIndex(pluginName, 0) == '/' ) {
            CFStringAppend(pluginPath, pluginName);
            pluginURL = _CreateIfReachable(pluginPath);
            if ( pluginURL ) {
                onePlugin = CFPlugInCreate(NULL, pluginURL);
                CFRelease( pluginURL );
                pluginURL = NULL;
                if ( onePlugin ) {
                    continue;
                }
            }
        }

        CFArrayRef extensionsFolderURLs = OSKextGetSystemExtensionsFolderURLs();
        CFIndex count = CFArrayGetCount(extensionsFolderURLs);
        for (CFIndex i = 0; i < count; i++) {
            CFURLRef directoryURL = CFArrayGetValueAtIndex(extensionsFolderURLs, i);
            pluginURL = CFURLCreateCopyAppendingPathComponent(NULL, directoryURL, pluginName, TRUE);

            // NOTE - on embedded we have cases where the plugin bundle is cached
            // so do NOT use _CreateIfReachable.  In the cached case
            // CFPlugInCreate will actually create the plugin for us.
            if (pluginURL) {
                onePlugin = CFPlugInCreate(NULL, pluginURL);
                CFRelease(pluginURL);
                pluginURL = NULL;
                if (onePlugin) {
                    break;
                }
            }
        }
    } while ( FALSE );
#if 0
    const char *    myPtr;
    myPtr = CFStringGetCStringPtr(pluginPath, kCFStringEncodingMacRoman);
    __iocfpluginlog("%s pluginPath \"%s\" \n", __func__,
                    myPtr ? myPtr : "no name");
#endif
   
    if ( onePlugin
        && (bundle = CFPlugInGetBundle(onePlugin))
        && (plist = CFBundleGetInfoDictionary(bundle))
        && (matching = (CFDictionaryRef)
            CFDictionaryGetValue(plist, CFSTR("Personality")))) {
            kr = IOServiceMatchPropertyTable( service, matching, &matches );
            if ( kr != kIOReturnSuccess )
                matches = FALSE;
        } else {
            matches = TRUE;
        }
    
    if ( matches ) {
        if ( onePlugin ) {
            *factories = CFPlugInFindFactoriesForPlugInTypeInPlugIn(pluginType, onePlugin);
        }
    }
    
    if ( pluginPath )
        CFRelease( pluginPath );
    if ( pluginTypes )
        CFRelease( pluginTypes );
    
#if 0
    __iocfpluginlog("%s kr %d \n", __func__, kr);
#endif
    
    return( kr );
}

kern_return_t
IOCreatePlugInInterfaceForService(io_service_t service,
                CFUUIDRef pluginType, CFUUIDRef interfaceType,
                IOCFPlugInInterface *** theInterface, SInt32 * theScore)
{
    CFDictionaryRef	plist = 0;
    CFArrayRef		plists;
    CFArrayRef		factories;
    CFMutableArrayRef	candidates;
    CFMutableArrayRef	scores;
    CFIndex		index;
    CFIndex		insert;
    CFUUIDRef		factoryID;
    kern_return_t	kr;
    SInt32		score;
    IOCFPlugInInterface **	interface;
    Boolean		haveOne;

    kr = IOFindPlugIns( service, pluginType,
                        &factories, &plists );
    if( KERN_SUCCESS != kr) {
        if (factories) CFRelease(factories);
        if (plists) CFRelease(plists);
        return( kr );
    }
    if ((KERN_SUCCESS != kr)
        || (factories == NULL)
        || (0 == CFArrayGetCount(factories))) {
//        printf("No factories for type\n");
        if (factories) CFRelease(factories);
        if (plists) CFRelease(plists);
        return( kIOReturnUnsupported );
    }
    candidates = CFArrayCreateMutable(kCFAllocatorDefault, 0, NULL);
    scores = CFArrayCreateMutable(kCFAllocatorDefault, 0, NULL);

    // allocate and Probe all
    if (candidates && scores) {
        CFIndex numfactories = CFArrayGetCount(factories);
        for ( index = 0; index < numfactories; index++ ) {
            IUnknownVTbl **				iunknown;
    
            factoryID = (CFUUIDRef) CFArrayGetValueAtIndex(factories, index);
            iunknown = (IUnknownVTbl **)
                CFPlugInInstanceCreate(NULL, factoryID, pluginType);
            if (!iunknown) {
    //            printf("Failed to create instance (link error?)\n");
                continue;
            }
            (*iunknown)->QueryInterface(iunknown, CFUUIDGetUUIDBytes(interfaceType),
                                (LPVOID *)&interface);
    
            // Now we are done with IUnknown interface
            (*iunknown)->Release(iunknown);
    
            if (!interface) {
    //            printf("Failed to get interface.\n");
                continue;
            }
            if (plists)
                plist = (CFDictionaryRef) CFArrayGetValueAtIndex( plists, index );
            score = 0;   // from property table
            kr = (*interface)->Probe(interface, plist, service, &score);
    
            if (kIOReturnSuccess == kr) {
                CFIndex numscores = CFArrayGetCount(scores);
                for (insert = 0; insert < numscores; insert++) {
                    if (score > (SInt32) ((intptr_t) CFArrayGetValueAtIndex(scores, insert)))
                        break;
                }
                CFArrayInsertValueAtIndex(candidates, insert, (void *) interface);
                CFArrayInsertValueAtIndex(scores, insert, (void *) (intptr_t) score);
            } else
                (*interface)->Release(interface);
        }
    }


    // Start in score order
    CFIndex candidatecount = CFArrayGetCount(candidates);
    for (haveOne = false, index = 0;
         index < candidatecount;
         index++) {

        Boolean freeIt;

        if (plists)
            plist = (CFDictionaryRef) CFArrayGetValueAtIndex(plists, index );
        interface = (IOCFPlugInInterface **)
            CFArrayGetValueAtIndex(candidates, index );
        if (!haveOne) {
            haveOne = (kIOReturnSuccess == (*interface)->Start(interface, plist, service));
            freeIt = !haveOne;
            if (haveOne) {
                *theInterface = interface;
                *theScore = (SInt32) (intptr_t)
		    CFArrayGetValueAtIndex(scores, index );
            }
        } else
            freeIt = true;
        if (freeIt)
            (*interface)->Release(interface);
    }

    if (factories)
        CFRelease(factories);
    if (plists)
        CFRelease(plists);
    if (candidates)
        CFRelease(candidates);
    if (scores)
        CFRelease(scores);
    //    CFRelease(plugin);

    return (haveOne ? kIOReturnSuccess : kIOReturnNoResources);
}

kern_return_t
IODestroyPlugInInterface(IOCFPlugInInterface ** interface)
{
    kern_return_t	err;

    err = (*interface)->Stop(interface);
    (*interface)->Release(interface);

    return( err );
}

kern_return_t
IOCreatePlugInInterfaces(CFUUIDRef pluginType, CFUUIDRef interfaceType);

#if 0 // local logging
void __iocfpluginlog(const char *format, ...)
{
    va_list args;
    
    va_start(args, format);
    asl_vlog(NULL, NULL, ASL_LEVEL_CRIT, format, args);
    va_end(args);
}
#endif

#endif /* !HAVE_CFPLUGIN */
