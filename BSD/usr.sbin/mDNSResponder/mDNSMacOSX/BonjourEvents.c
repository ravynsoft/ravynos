/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2010-2015 Apple Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFXPCBridge.h>
#include "dns_sd.h"
#include <UserEventAgentInterface.h>
#include <stdio.h>
#include <stdlib.h>
#include <os/log.h>
#include <xpc/xpc.h>


#pragma mark -
#pragma mark Types
#pragma mark -
static const char*          sPluginIdentifier       = "com.apple.bonjour.events";

// PLIST Keys
static const CFStringRef sServiceNameKey         = CFSTR("ServiceName");
static const CFStringRef sServiceTypeKey         = CFSTR("ServiceType");
static const CFStringRef sServiceDomainKey       = CFSTR("ServiceDomain");

static const CFStringRef sOnServiceAddKey        = CFSTR("OnServiceAdd");
static const CFStringRef sOnServiceRemoveKey     = CFSTR("OnServiceRemove");

static const CFStringRef sLaunchdTokenKey        = CFSTR("LaunchdToken");
static const CFStringRef sLaunchdDictKey         = CFSTR("LaunchdDict");


/************************************************
* Launch Event Dictionary (input from launchd)
* Passed To: ManageEventsCallback
*-----------------------------------------------
* Typing in this dictionary is not enforced
* above us. So this may not be true. Type check
* all input before using it.
*-----------------------------------------------
* sServiceNameKey		- CFString (Optional)
* sServiceTypeKey		- CFString
* sServiceDomainKey	- CFString
*
* One or more of the following.
*-----------------------------------
* sOnServiceAddKey			- CFBoolean
* sOnServiceRemoveKey		- CFBoolean
* sWhileServiceExistsKey	- CFBoolean
************************************************/

/************************************************
* Browser Dictionary
*-----------------------------------------------
* sServiceDomainKey - CFString
* sServiceTypeKey   - CFString
************************************************/

/************************************************
* Event Dictionary
*-----------------------------------------------
* sServiceNameKey	 - CFString (Optional)
* sLaunchdTokenKey	 - CFNumber
************************************************/

typedef struct {
    UserEventAgentInterfaceStruct*      _UserEventAgentInterface;
    CFUUIDRef _factoryID;
    UInt32 _refCount;

    void*                               _pluginContext;

    CFMutableDictionaryRef _tokenToBrowserMap;                  // Maps a token to a browser that can be used to scan the remaining dictionaries.
    CFMutableDictionaryRef _browsers;                           // A Dictionary of Browser Dictionaries where the resposible browser is the key.
    CFMutableDictionaryRef _onAddEvents;                        // A Dictionary of Event Dictionaries that describe events to trigger on a service appearing.
    CFMutableDictionaryRef _onRemoveEvents;                     // A Dictionary of Event Dictionaries that describe events to trigger on a service disappearing.
} BonjourUserEventsPlugin;

typedef struct {
    CFIndex refCount;
    DNSServiceRef browserRef;
} NetBrowserInfo;

#pragma mark -
#pragma mark Prototypes
#pragma mark -
// COM Stuff
static HRESULT  QueryInterface(void *myInstance, REFIID iid, LPVOID *ppv);
static ULONG    AddRef(void* instance);
static ULONG    Release(void* instance);

static BonjourUserEventsPlugin* Alloc(CFUUIDRef factoryID);
static void Dealloc(BonjourUserEventsPlugin* plugin);

void * UserEventAgentFactory(CFAllocatorRef allocator, CFUUIDRef typeID);

// Plugin Management
static void Install(void* instance);
static void ManageEventsCallback(
    UserEventAgentLaunchdAction action,
    CFNumberRef token,
    CFTypeRef eventMatchDict,
    void                      * vContext);


// Plugin Guts
void AddEventToPlugin(BonjourUserEventsPlugin* plugin, CFNumberRef launchdToken, CFDictionaryRef eventParameters);
void RemoveEventFromPlugin(BonjourUserEventsPlugin* plugin, CFNumberRef launchToken);

NetBrowserInfo* CreateBrowser(BonjourUserEventsPlugin* plugin, CFStringRef type, CFStringRef domain);
NetBrowserInfo* BrowserForSDRef(BonjourUserEventsPlugin* plugin, DNSServiceRef sdRef);
void AddEventDictionary(CFDictionaryRef eventDict, CFMutableDictionaryRef allEventsDictionary, NetBrowserInfo* key);
void RemoveEventFromArray(CFMutableArrayRef array, CFNumberRef launchdToken);

// Net Service Browser Stuff
void ServiceBrowserCallback (DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char* serviceName, const char* regtype, const char* replyDomain, void* context);
void HandleTemporaryEventsForService(BonjourUserEventsPlugin* plugin, NetBrowserInfo* browser, CFStringRef serviceName, CFMutableDictionaryRef eventsDictionary);

// Convence Stuff
const char* CStringFromCFString(CFStringRef string);

// NetBrowserInfo "Object"
NetBrowserInfo* NetBrowserInfoCreate(CFStringRef serviceType, CFStringRef domain, void* context);
const void* NetBrowserInfoRetain(CFAllocatorRef allocator, const void* info);
void NetBrowserInfoRelease(CFAllocatorRef allocator, const void* info);
Boolean NetBrowserInfoEqual(const void *value1, const void *value2);
CFHashCode  NetBrowserInfoHash(const void *value);
CFStringRef NetBrowserInfoCopyDescription(const void *value);

static const CFDictionaryKeyCallBacks kNetBrowserInfoDictionaryKeyCallbacks = {
    0,
    NetBrowserInfoRetain,
    NetBrowserInfoRelease,
    NetBrowserInfoCopyDescription,
    NetBrowserInfoEqual,
    NetBrowserInfoHash
};

static const CFDictionaryValueCallBacks kNetBrowserInfoDictionaryValueCallbacks = {
    0,
    NetBrowserInfoRetain,
    NetBrowserInfoRelease,
    NetBrowserInfoCopyDescription,
    NetBrowserInfoEqual
};

// COM type definition goop.
static UserEventAgentInterfaceStruct UserEventAgentInterfaceFtbl = {
    NULL,                   // Required padding for COM
    QueryInterface,         // Query Interface
    AddRef,                 // AddRef()
    Release,                // Release()
    Install                 // Install
};

#pragma mark -
#pragma mark COM Management
#pragma mark -

/*****************************************************************************
*****************************************************************************/
static HRESULT QueryInterface(void *myInstance, REFIID iid, LPVOID *ppv)
{
    CFUUIDRef interfaceID = CFUUIDCreateFromUUIDBytes(NULL, iid);

    // Test the requested ID against the valid interfaces.
    if(CFEqual(interfaceID, kUserEventAgentInterfaceID))
    {
        ((BonjourUserEventsPlugin *) myInstance)->_UserEventAgentInterface->AddRef(myInstance);
        *ppv = myInstance;
        CFRelease(interfaceID);
        return S_OK;
    }
    else if(CFEqual(interfaceID, IUnknownUUID))
    {
        ((BonjourUserEventsPlugin *) myInstance)->_UserEventAgentInterface->AddRef(myInstance);
        *ppv = myInstance;
        CFRelease(interfaceID);
        return S_OK;
    }
    else //  Requested interface unknown, bail with error.
    {
        *ppv = NULL;
        CFRelease(interfaceID);
        return E_NOINTERFACE;
    }
}

/*****************************************************************************
*****************************************************************************/
static ULONG AddRef(void* instance)
{
    BonjourUserEventsPlugin* plugin = (BonjourUserEventsPlugin*)instance;
    return ++plugin->_refCount;
}

/*****************************************************************************
*****************************************************************************/
static ULONG Release(void* instance)
{
    BonjourUserEventsPlugin* plugin = (BonjourUserEventsPlugin*)instance;

    if (plugin->_refCount != 0)
        --plugin->_refCount;

    if (plugin->_refCount == 0)
    {
        Dealloc(instance);
        return 0;
    }

    return plugin->_refCount;
}

/*****************************************************************************
* Alloc
* -
* Functionas as both +[alloc] and -[init] for the plugin. Add any
* initalization of member variables here.
*****************************************************************************/
static BonjourUserEventsPlugin* Alloc(CFUUIDRef factoryID)
{
    BonjourUserEventsPlugin* plugin = malloc(sizeof(BonjourUserEventsPlugin));

    plugin->_UserEventAgentInterface = &UserEventAgentInterfaceFtbl;
    plugin->_pluginContext = NULL;

    if (factoryID)
    {
        plugin->_factoryID = (CFUUIDRef)CFRetain(factoryID);
        CFPlugInAddInstanceForFactory(factoryID);
    }

    plugin->_refCount = 1;
    plugin->_tokenToBrowserMap = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kNetBrowserInfoDictionaryValueCallbacks);
    plugin->_browsers = CFDictionaryCreateMutable(NULL, 0, &kNetBrowserInfoDictionaryKeyCallbacks, &kCFTypeDictionaryValueCallBacks);
    plugin->_onAddEvents = CFDictionaryCreateMutable(NULL, 0, &kNetBrowserInfoDictionaryKeyCallbacks, &kCFTypeDictionaryValueCallBacks);
    plugin->_onRemoveEvents = CFDictionaryCreateMutable(NULL, 0, &kNetBrowserInfoDictionaryKeyCallbacks, &kCFTypeDictionaryValueCallBacks);

    return plugin;
}

/*****************************************************************************
* Dealloc
* -
* Much like Obj-C dealloc this method is responsible for releasing any object
* this plugin is holding. Unlike ObjC, you call directly free() instead of
* [super dalloc].
*****************************************************************************/
static void Dealloc(BonjourUserEventsPlugin* plugin)
{
    CFUUIDRef factoryID = plugin->_factoryID;

    if (factoryID)
    {
        CFPlugInRemoveInstanceForFactory(factoryID);
        CFRelease(factoryID);
    }

    if (plugin->_tokenToBrowserMap)
        CFRelease(plugin->_tokenToBrowserMap);

    if (plugin->_browsers)
        CFRelease(plugin->_browsers);

    if (plugin->_onAddEvents)
        CFRelease(plugin->_onAddEvents);

    if (plugin->_onRemoveEvents)
        CFRelease(plugin->_onRemoveEvents);

    free(plugin);
}

/*******************************************************************************
*******************************************************************************/
void * UserEventAgentFactory(CFAllocatorRef allocator, CFUUIDRef typeID)
{
    (void)allocator;
    BonjourUserEventsPlugin * result = NULL;

    if (typeID && CFEqual(typeID, kUserEventAgentTypeID))
    {
        result = Alloc(kUserEventAgentFactoryID);
    }

    return (void *)result;
}

#pragma mark -
#pragma mark Plugin Management
#pragma mark -
/*****************************************************************************
* Install
* -
* This is invoked once when the plugin is loaded to do initial setup and
* allow us to register with launchd. If UserEventAgent crashes, the plugin
* will need to be reloaded, and hence this will get invoked again.
*****************************************************************************/
static void Install(void *instance)
{
    BonjourUserEventsPlugin* plugin = (BonjourUserEventsPlugin*)instance;

    plugin->_pluginContext = UserEventAgentRegisterForLaunchEvents(sPluginIdentifier, &ManageEventsCallback, plugin);

    if (!plugin->_pluginContext)
    {
        fprintf(stderr, "%s:%s failed to register for launch events.\n", sPluginIdentifier, __FUNCTION__);
        return;
    }

}

/*****************************************************************************
* ManageEventsCallback
* -
* This is invoked when launchd loads a event dictionary and needs to inform
* us what a daemon / agent is looking for.
*****************************************************************************/
static void ManageEventsCallback(UserEventAgentLaunchdAction action, CFNumberRef token, CFTypeRef eventMatchDict, void* vContext)
{
    if (action == kUserEventAgentLaunchdAdd)
    {
        if (!eventMatchDict)
        {
            fprintf(stderr, "%s:%s empty dictionary\n", sPluginIdentifier, __FUNCTION__);
            return;
        }
        if (CFGetTypeID(eventMatchDict) != CFDictionaryGetTypeID())
        {
            fprintf(stderr, "%s:%s given non-dict for event dictionary, action %d\n", sPluginIdentifier, __FUNCTION__, action);
            return;
        }
        // Launchd wants us to add a launch event for this token and matching dictionary.
        os_log_info(OS_LOG_DEFAULT, "%s:%s calling AddEventToPlugin", sPluginIdentifier, __FUNCTION__);
        AddEventToPlugin((BonjourUserEventsPlugin*)vContext, token, (CFDictionaryRef)eventMatchDict);
    }
    else if (action == kUserEventAgentLaunchdRemove)
    {
        // Launchd wants us to remove the event hook we setup for this token / matching dictionary.
        // Note: eventMatchDict can be NULL for Remove.
        os_log_info(OS_LOG_DEFAULT, "%s:%s calling RemoveEventToPlugin", sPluginIdentifier, __FUNCTION__);
        RemoveEventFromPlugin((BonjourUserEventsPlugin*)vContext, token);
    }
    else
    {
        os_log_info(OS_LOG_DEFAULT, "%s:%s unknown callback event\n", sPluginIdentifier, __FUNCTION__);
    }
}


#pragma mark -
#pragma mark Plugin Guts
#pragma mark -

/*****************************************************************************
* AddEventToPlugin
* -
* This method is invoked when launchd wishes the plugin to setup a launch
* event matching the parameters in the dictionary.
*****************************************************************************/
void AddEventToPlugin(BonjourUserEventsPlugin* plugin, CFNumberRef launchdToken, CFDictionaryRef eventParameters)
{
    CFStringRef domain = CFDictionaryGetValue(eventParameters, sServiceDomainKey);
    CFStringRef type = CFDictionaryGetValue(eventParameters, sServiceTypeKey);
    CFStringRef name = CFDictionaryGetValue(eventParameters, sServiceNameKey);
    CFBooleanRef cfOnAdd = CFDictionaryGetValue(eventParameters, sOnServiceAddKey);
    CFBooleanRef cfOnRemove = CFDictionaryGetValue(eventParameters, sOnServiceRemoveKey);

    Boolean onAdd = false;
    Boolean onRemove = false;

    if (cfOnAdd && CFGetTypeID(cfOnAdd) == CFBooleanGetTypeID() && CFBooleanGetValue(cfOnAdd))
        onAdd = true;

    if (cfOnRemove && CFGetTypeID(cfOnRemove) == CFBooleanGetTypeID() && CFBooleanGetValue(cfOnRemove))
        onRemove = true;

    // A type is required. If none is specified, BAIL
    if (!type || CFGetTypeID(type) != CFStringGetTypeID())
    {
        fprintf(stderr, "%s:%s: a LaunchEvent is missing a service type.\n", sPluginIdentifier, __FUNCTION__);
        return;
    }

    // If we aren't suppose to launch on services appearing or disappearing, this service does nothing. Ignore.
    if (!onAdd && !onRemove)
    {
        fprintf(stderr, "%s:%s a LaunchEvent is missing both onAdd and onRemove events\n", sPluginIdentifier, __FUNCTION__);
        return;
    }

    // If no domain is specified, assume local.
    if (!domain)
    {
        domain = CFSTR("local");
    }
    else if (CFGetTypeID(domain) != CFStringGetTypeID() ) // If the domain is not a string, fail
    {
        fprintf(stderr, "%s:%s a LaunchEvent has a domain that is not a string.\n", sPluginIdentifier, __FUNCTION__);
        return;
    }

    // If we have a name filter, but it's not a string. This event is broken, bail.
    if (name && CFGetTypeID(name) != CFStringGetTypeID())
    {
        fprintf(stderr, "%s:%s a LaunchEvent has a domain that is not a string.\n", sPluginIdentifier, __FUNCTION__);
        return;
    }

    // Get us a browser
    NetBrowserInfo* browser = CreateBrowser(plugin, type, domain);

    if (!browser)
    {
        fprintf(stderr, "%s:%s cannot create browser\n", sPluginIdentifier, __FUNCTION__);
        return;
    }

    // Create Event Dictionary
    CFMutableDictionaryRef eventDictionary = CFDictionaryCreateMutable(NULL, 4, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    // We store both the Token and the Dictionary. UserEventAgentSetLaunchEventState needs
    // the token and UserEventAgentSetFireEvent needs both the token and the dictionary
    CFDictionarySetValue(eventDictionary, sLaunchdTokenKey, launchdToken);
    CFDictionarySetValue(eventDictionary, sLaunchdDictKey, eventParameters);

    if (name)
        CFDictionarySetValue(eventDictionary, sServiceNameKey, name);

    // Add to the correct dictionary.
    if (onAdd)
    {
        os_log_info(OS_LOG_DEFAULT, "%s:%s: Adding browser to AddEvents", sPluginIdentifier, __FUNCTION__);
        AddEventDictionary(eventDictionary, plugin->_onAddEvents, browser);
    }

    if (onRemove)
    {
        os_log_info(OS_LOG_DEFAULT, "%s:%s: Adding browser to RemoveEvents", sPluginIdentifier, __FUNCTION__);
        AddEventDictionary(eventDictionary, plugin->_onRemoveEvents, browser);
    }

    // Add Token Mapping
    CFDictionarySetValue(plugin->_tokenToBrowserMap, launchdToken, browser);

    // Release Memory
    CFRelease(eventDictionary);
}

/*****************************************************************************
* RemoveEventFromPlugin
* -
* This method is invoked when launchd wishes the plugin to setup a launch
* event matching the parameters in the dictionary.
*****************************************************************************/
void RemoveEventFromPlugin(BonjourUserEventsPlugin* plugin, CFNumberRef launchdToken)
{
    NetBrowserInfo* browser = (NetBrowserInfo*)CFDictionaryGetValue(plugin->_tokenToBrowserMap, launchdToken);
    Boolean othersUsingBrowser = false;

    if (!browser)
    {
        long long value = 0;
        CFNumberGetValue(launchdToken, kCFNumberLongLongType, &value);
        fprintf(stderr, "%s:%s Launchd asked us to remove a token we did not register! ==Token:%lld== \n", sPluginIdentifier, __FUNCTION__, value);
        return;
    }

    CFMutableArrayRef onAddEvents = (CFMutableArrayRef)CFDictionaryGetValue(plugin->_onAddEvents, browser);
    CFMutableArrayRef onRemoveEvents = (CFMutableArrayRef)CFDictionaryGetValue(plugin->_onRemoveEvents, browser);

    if (onAddEvents)
    {
        os_log_info(OS_LOG_DEFAULT, "%s:%s: Calling RemoveEventFromArray for OnAddEvents", sPluginIdentifier, __FUNCTION__);
        RemoveEventFromArray(onAddEvents, launchdToken);

        // Is the array now empty, clean up
        if (CFArrayGetCount(onAddEvents) == 0)
        {
            os_log_info(OS_LOG_DEFAULT, "%s:%s: Removing the browser from AddEvents", sPluginIdentifier, __FUNCTION__);
            CFDictionaryRemoveValue(plugin->_onAddEvents, browser);
        }
    }

    if (onRemoveEvents)
    {
        os_log_info(OS_LOG_DEFAULT, "%s:%s: Calling RemoveEventFromArray for OnRemoveEvents", sPluginIdentifier, __FUNCTION__);
        RemoveEventFromArray(onRemoveEvents, launchdToken);

        // Is the array now empty, clean up
        if (CFArrayGetCount(onRemoveEvents) == 0)
        {
            os_log_info(OS_LOG_DEFAULT, "%s:%s: Removing the browser from RemoveEvents", sPluginIdentifier, __FUNCTION__);
            CFDictionaryRemoveValue(plugin->_onRemoveEvents, browser);
        }
    }

    // Remove ourselves from the token dictionary.
    CFDictionaryRemoveValue(plugin->_tokenToBrowserMap, launchdToken);

    // Check to see if anyone else is using this browser.
    CFIndex i;
    CFIndex count = CFDictionaryGetCount(plugin->_tokenToBrowserMap);
    NetBrowserInfo** browsers = malloc(count * sizeof(NetBrowserInfo*));

    // Fetch the values of the token dictionary
    CFDictionaryGetKeysAndValues(plugin->_tokenToBrowserMap, NULL, (const void**)browsers);

    for (i = 0; i < count; ++i)
    {
        if (NetBrowserInfoEqual(browsers[i], browser))
        {
            othersUsingBrowser = true;
            break;
        }
    }

    // If no one else is useing our browser, clean up!
    if (!othersUsingBrowser)
    {
        os_log_info(OS_LOG_DEFAULT, "%s:%s: Removing browser %p from _browsers", sPluginIdentifier, __FUNCTION__, browser);
        CFDictionaryRemoveValue(plugin->_browsers, browser); // This triggers release and dealloc of the browser
    }
    else
    {
        os_log_info(OS_LOG_DEFAULT, "%s:%s: Decrementing browsers %p count", sPluginIdentifier, __FUNCTION__, browser);
        // Decrement my reference count (it was incremented when it was added to _browsers in CreateBrowser)
        NetBrowserInfoRelease(NULL, browser);
    }

    free(browsers);
}


/*****************************************************************************
* CreateBrowser
* -
* This method returns a NetBrowserInfo that is looking for a type of
* service in a domain. If no browser exists, it will create one and return it.
*****************************************************************************/
NetBrowserInfo* CreateBrowser(BonjourUserEventsPlugin* plugin, CFStringRef type, CFStringRef domain)
{
    CFIndex i;
    CFIndex count = CFDictionaryGetCount(plugin->_browsers);
    NetBrowserInfo* browser = NULL;
    CFDictionaryRef* dicts = malloc(count * sizeof(CFDictionaryRef));
    NetBrowserInfo** browsers = malloc(count * sizeof(NetBrowserInfo*));

    // Fetch the values of the browser dictionary
    CFDictionaryGetKeysAndValues(plugin->_browsers, (const void**)browsers, (const void**)dicts);


    // Loop thru the browsers list and see if we can find a matching one.
    for (i = 0; i < count; ++i)
    {
        CFDictionaryRef browserDict = dicts[i];

        CFStringRef browserType = CFDictionaryGetValue(browserDict, sServiceTypeKey);
        CFStringRef browserDomain = CFDictionaryGetValue(browserDict, sServiceDomainKey);

        // If we have a matching browser, break
        if ((CFStringCompare(browserType, type, kCFCompareCaseInsensitive) == kCFCompareEqualTo) &&
            (CFStringCompare(browserDomain, domain, kCFCompareCaseInsensitive) == kCFCompareEqualTo))
        {
            os_log_info(OS_LOG_DEFAULT, "%s:%s: found a duplicate browser\n", sPluginIdentifier, __FUNCTION__);
            browser = browsers[i];
            NetBrowserInfoRetain(NULL, browser);
            break;
        }
    }

    // No match found, lets create one!
    if (!browser)
    {

        browser = NetBrowserInfoCreate(type, domain, plugin);

        if (!browser)
        {
            fprintf(stderr, "%s:%s failed to search for %s.%s", sPluginIdentifier, __FUNCTION__, CStringFromCFString(type), CStringFromCFString(domain));
            free(dicts);
            free(browsers);
            return NULL;
        }

        // Service browser created, lets add this to ourselves to the dictionary.
        CFMutableDictionaryRef browserDict = CFDictionaryCreateMutable(NULL, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

        CFDictionarySetValue(browserDict, sServiceTypeKey, type);
        CFDictionarySetValue(browserDict, sServiceDomainKey, domain);

        // Add the dictionary to the browsers dictionary.
        CFDictionarySetValue(plugin->_browsers, browser, browserDict);

        // Release Memory
        CFRelease(browserDict);
    }

    free(dicts);
    free(browsers);

    return browser;
}

/*****************************************************************************
* BrowserForSDRef
* -
* This method returns a NetBrowserInfo that matches the calling SDRef passed
* in via the callback.
*****************************************************************************/
NetBrowserInfo* BrowserForSDRef(BonjourUserEventsPlugin* plugin, DNSServiceRef sdRef)
{
    CFIndex i;
    CFIndex count = CFDictionaryGetCount(plugin->_browsers);
    NetBrowserInfo* browser = NULL;
    NetBrowserInfo** browsers = malloc(count * sizeof(NetBrowserInfo*));

    // Fetch the values of the browser dictionary
    CFDictionaryGetKeysAndValues(plugin->_browsers, (const void**)browsers, NULL);

    // Loop thru the browsers list and see if we can find a matching one.
    for (i = 0; i < count; ++i)
    {
        NetBrowserInfo* currentBrowser = browsers[i];

        if (currentBrowser->browserRef == sdRef)
        {
            browser = currentBrowser;
            break;
        }
    }


    free(browsers);

    return browser;
}

/*****************************************************************************
* AddEventDictionary
* -
* Adds a event to a browser's event dictionary
*****************************************************************************/

void AddEventDictionary(CFDictionaryRef eventDict, CFMutableDictionaryRef allEventsDictionary, NetBrowserInfo* key)
{
    CFMutableArrayRef eventsForBrowser = (CFMutableArrayRef)CFDictionaryGetValue(allEventsDictionary, key);

    if (!eventsForBrowser) // We have no events for this browser yet, lets add him.
    {
        eventsForBrowser = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
        CFDictionarySetValue(allEventsDictionary, key, eventsForBrowser);
        os_log_info(OS_LOG_DEFAULT, "%s:%s creating a new array", sPluginIdentifier, __FUNCTION__);
    }
    else
    {
        os_log_info(OS_LOG_DEFAULT, "%s:%s Incrementing refcount", sPluginIdentifier, __FUNCTION__);
        CFRetain(eventsForBrowser);
    }

    CFArrayAppendValue(eventsForBrowser, eventDict);
    CFRelease(eventsForBrowser);
}

/*****************************************************************************
* RemoveEventFromArray
* -
* Searches a Array of Event Dictionaries to find one with a matching launchd
* token and remove it.
*****************************************************************************/

void RemoveEventFromArray(CFMutableArrayRef array, CFNumberRef launchdToken)
{
    CFIndex i;
    CFIndex count = CFArrayGetCount(array);

    // Loop thru looking for us.
    for (i = 0; i < count; )
    {
        CFDictionaryRef eventDict = CFArrayGetValueAtIndex(array, i);
        CFNumberRef token = CFDictionaryGetValue(eventDict, sLaunchdTokenKey);

        if (CFEqual(token, launchdToken)) // This is the same event?
        {
            os_log_info(OS_LOG_DEFAULT, "%s:%s found token", sPluginIdentifier, __FUNCTION__);
            CFArrayRemoveValueAtIndex(array, i);    // Remove the event,
            break; // The token should only exist once, so it makes no sense to continue.
        }
        else
        {
            ++i; // If it's not us, advance.
        }
    }
    if (i == count) os_log_info(OS_LOG_DEFAULT, "%s:%s did not find token", sPluginIdentifier, __FUNCTION__);
}

#pragma mark -
#pragma mark Net Service Browser Stuff
#pragma mark -

/*****************************************************************************
* ServiceBrowserCallback
* -
* This method is the heart of the plugin. It's the runloop callback annoucing
* the appearence and disappearance of network services.
*****************************************************************************/

void ServiceBrowserCallback (DNSServiceRef sdRef,
                             DNSServiceFlags flags,
                             uint32_t interfaceIndex,
                             DNSServiceErrorType errorCode,
                             const char*                serviceName,
                             const char*                regtype,
                             const char*                replyDomain,
                             void*                      context )
{
    (void)interfaceIndex;
    (void)regtype;
    (void)replyDomain;
    BonjourUserEventsPlugin* plugin = (BonjourUserEventsPlugin*)context;
    NetBrowserInfo* browser = BrowserForSDRef(plugin, sdRef);

    if (!browser) // Missing browser?
    {
        fprintf(stderr, "%s:%s ServiceBrowserCallback: missing browser\n", sPluginIdentifier, __FUNCTION__);
        return;
    }

    if (errorCode != kDNSServiceErr_NoError)
    {
        fprintf(stderr, "%s:%s ServiceBrowserCallback: errcode set %d\n", sPluginIdentifier, __FUNCTION__, errorCode);
        return;
    }

    CFStringRef cfServiceName = CFStringCreateWithCString(NULL, serviceName, kCFStringEncodingUTF8);
    if (cfServiceName == NULL)
    {
        static int msgCount = 0;
        if (msgCount < 1000)
        {
            os_log_info(OS_LOG_DEFAULT, "%s:%s Can not create CFString for serviceName %s", sPluginIdentifier, __FUNCTION__, serviceName);
            msgCount++;
        }
        return;
    }

    if (flags & kDNSServiceFlagsAdd)
    {
        os_log_info(OS_LOG_DEFAULT, "%s:%s calling HandleTemporaryEventsForService Add\n", sPluginIdentifier, __FUNCTION__);
        HandleTemporaryEventsForService(plugin, browser, cfServiceName, plugin->_onAddEvents);
    }
    else
    {
        os_log_info(OS_LOG_DEFAULT, "%s:%s calling HandleTemporaryEventsForService Remove\n", sPluginIdentifier, __FUNCTION__);
        HandleTemporaryEventsForService(plugin, browser, cfServiceName, plugin->_onRemoveEvents);
    }

    CFRelease(cfServiceName);
}

/*****************************************************************************
* HandleTemporaryEventsForService
* -
* This method handles the firing of one shot events. Aka. Events that are
* signaled when a service appears / disappears. They have a temporarly
* signaled state.
*****************************************************************************/
void HandleTemporaryEventsForService(BonjourUserEventsPlugin* plugin, NetBrowserInfo* browser, CFStringRef serviceName, CFMutableDictionaryRef eventsDictionary)
{
    CFArrayRef events = (CFArrayRef)CFDictionaryGetValue(eventsDictionary, browser); // Get events for the browser we passed in.
    CFIndex i;
    CFIndex count;

    if (!events)  // Somehow we have a orphan browser...
        return;

    count = CFArrayGetCount(events);

    // Go thru the events and run filters, notifity if they pass.
    for (i = 0; i < count; ++i)
    {
        CFDictionaryRef eventDict = (CFDictionaryRef)CFArrayGetValueAtIndex(events, i);
        CFStringRef eventServiceName = (CFStringRef)CFDictionaryGetValue(eventDict, sServiceNameKey);
        CFNumberRef token = (CFNumberRef) CFDictionaryGetValue(eventDict, sLaunchdTokenKey);
        CFDictionaryRef dict = (CFDictionaryRef) CFDictionaryGetValue(eventDict, sLaunchdDictKey);

        // Currently we only filter on service name, that makes this as simple as...
        if (!eventServiceName || CFEqual(serviceName, eventServiceName))
        {
            uint64_t tokenUint64;
            // Signal Event: This is edge trigger. When the action has been taken, it will not
            // be remembered anymore.

            os_log_info(OS_LOG_DEFAULT, "%s:%s HandleTemporaryEventsForService signal\n", sPluginIdentifier, __FUNCTION__);
            CFNumberGetValue(token, kCFNumberLongLongType, &tokenUint64);

            xpc_object_t jobRequest = _CFXPCCreateXPCObjectFromCFObject(dict);

            UserEventAgentFireEvent(plugin->_pluginContext, tokenUint64, jobRequest);
            xpc_release(jobRequest);
        }
    }
}

#pragma mark -
#pragma mark Convenience
#pragma mark -

/*****************************************************************************
* CStringFromCFString
* -
* Silly convenence function for dealing with non-critical CFSTR -> cStr
* conversions.
*****************************************************************************/

const char* CStringFromCFString(CFStringRef string)
{
    const char* defaultString = "??????";
    const char* cstring;

    if (!string)
        return defaultString;

    cstring = CFStringGetCStringPtr(string, kCFStringEncodingUTF8);

    return (cstring) ? cstring : defaultString;

}

#pragma mark -
#pragma mark NetBrowserInfo "Object"
#pragma mark -
/*****************************************************************************
* NetBrowserInfoCreate
* -
* The method creates a NetBrowserInfo Object and initalizes it.
*****************************************************************************/
NetBrowserInfo* NetBrowserInfoCreate(CFStringRef serviceType, CFStringRef domain, void* context)
{
    NetBrowserInfo* outObj = NULL;
    DNSServiceRef browserRef = NULL;
    char* cServiceType = NULL;
    char* cDomain = NULL;
    Boolean success = true;

    CFIndex serviceSize = CFStringGetMaximumSizeForEncoding(CFStringGetLength(serviceType), kCFStringEncodingUTF8);
    cServiceType = calloc(serviceSize, 1);
    success = CFStringGetCString(serviceType, cServiceType, serviceSize, kCFStringEncodingUTF8);


    if (domain)
    {
        CFIndex domainSize = CFStringGetMaximumSizeForEncoding(CFStringGetLength(domain), kCFStringEncodingUTF8);
        if (domainSize)
        {
            cDomain = calloc(domainSize, 1);
            success = success && CFStringGetCString(domain, cDomain, domainSize, kCFStringEncodingUTF8);
        }
    }

    if (!success)
    {
        fprintf(stderr, "%s:%s LaunchEvent has badly encoded service type or domain.\n", sPluginIdentifier, __FUNCTION__);
        free(cServiceType);

        if (cDomain)
            free(cDomain);

        return NULL;
    }

    DNSServiceErrorType err = DNSServiceBrowse(&browserRef, 0, 0, cServiceType, cDomain, ServiceBrowserCallback, context);

    if (err != kDNSServiceErr_NoError)
    {
        fprintf(stderr, "%s:%s Failed to create browser for %s, %s\n", sPluginIdentifier, __FUNCTION__, cServiceType, cDomain);
        free(cServiceType);

        if (cDomain)
            free(cDomain);

        return NULL;
    }

    DNSServiceSetDispatchQueue(browserRef, dispatch_get_main_queue());


    outObj = malloc(sizeof(NetBrowserInfo));

    outObj->refCount = 1;
    outObj->browserRef = browserRef;

    os_log_info(OS_LOG_DEFAULT, "%s:%s: created new object %p", sPluginIdentifier, __FUNCTION__, outObj);

    free(cServiceType);

    if (cDomain)
        free(cDomain);

    return outObj;
}

/*****************************************************************************
* NetBrowserInfoRetain
* -
* The method retains a NetBrowserInfo object.
*****************************************************************************/
const void* NetBrowserInfoRetain(CFAllocatorRef allocator, const void* info)
{
    (void)allocator;
    NetBrowserInfo* obj = (NetBrowserInfo*)info;

    if (!obj)
        return NULL;

    ++obj->refCount;
    os_log_info(OS_LOG_DEFAULT, "%s:%s: Incremented ref count on %p, count %d", sPluginIdentifier, __FUNCTION__, obj->browserRef, (int)obj->refCount);

    return obj;
}

/*****************************************************************************
* NetBrowserInfoRelease
* -
* The method releases a NetBrowserInfo object.
*****************************************************************************/
void NetBrowserInfoRelease(CFAllocatorRef allocator, const void* info)
{
    (void)allocator;
    NetBrowserInfo* obj = (NetBrowserInfo*)info;

    if (!obj)
        return;

    if (obj->refCount == 1)
    {
        os_log_info(OS_LOG_DEFAULT, "%s:%s: DNSServiceRefDeallocate %p", sPluginIdentifier, __FUNCTION__, obj->browserRef);
        DNSServiceRefDeallocate(obj->browserRef);
        free(obj);
    }
    else
    {
        --obj->refCount;
        os_log_info(OS_LOG_DEFAULT, "%s:%s: Decremented ref count on %p, count %d", sPluginIdentifier, __FUNCTION__, obj->browserRef, (int)obj->refCount);
    }

}

/*****************************************************************************
* NetBrowserInfoEqual
* -
* The method is used to compare two NetBrowserInfo objects for equality.
*****************************************************************************/
Boolean NetBrowserInfoEqual(const void *value1, const void *value2)
{
    NetBrowserInfo* obj1 = (NetBrowserInfo*)value1;
    NetBrowserInfo* obj2 = (NetBrowserInfo*)value2;

    if (obj1->browserRef == obj2->browserRef)
        return true;

    return false;
}

/*****************************************************************************
* NetBrowserInfoHash
* -
* The method is used to make a hash for the object. We can cheat and use the
* browser pointer.
*****************************************************************************/
CFHashCode  NetBrowserInfoHash(const void *value)
{
    return (CFHashCode)((NetBrowserInfo*)value)->browserRef;
}


/*****************************************************************************
* NetBrowserInfoCopyDescription
* -
* Make CF happy.
*****************************************************************************/
CFStringRef NetBrowserInfoCopyDescription(const void *value)
{
    (void)value;
    return CFStringCreateWithCString(NULL, "NetBrowserInfo: No useful description", kCFStringEncodingUTF8);
}

