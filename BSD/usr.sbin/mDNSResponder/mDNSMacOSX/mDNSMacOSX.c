/* -*- Mode: C; tab-width: 4; c-file-style: "bsd"; c-basic-offset: 4; fill-column: 108; indent-tabs-mode: nil; -*-
 *
 * Copyright (c) 2002-2019 Apple Inc. All rights reserved.
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

// ***************************************************************************
// mDNSMacOSX.c:
// Supporting routines to run mDNS on a CFRunLoop platform
// ***************************************************************************

// For debugging, set LIST_ALL_INTERFACES to 1 to display all found interfaces,
// including ones that mDNSResponder chooses not to use.
#define LIST_ALL_INTERFACES 0

#include "mDNSEmbeddedAPI.h"        // Defines the interface provided to the client layer above
#include "DNSCommon.h"
#include "uDNS.h"
#include "mDNSMacOSX.h"             // Defines the specific types needed to run mDNS on this platform
#include "dns_sd.h"                 // For mDNSInterface_LocalOnly etc.
#include "dns_sd_internal.h"
#include "PlatformCommon.h"
#include "uds_daemon.h"
#include "CryptoSupport.h"

#include <stdio.h>
#include <stdarg.h>                 // For va_list support
#include <stdlib.h>                 // For arc4random
#include <net/if.h>
#include <net/if_types.h>           // For IFT_ETHER
#include <net/if_dl.h>
#include <net/bpf.h>                // For BIOCSETIF etc.
#include <sys/uio.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/event.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>                   // platform support for UTC time
#include <arpa/inet.h>              // for inet_aton
#include <pthread.h>
#include <netdb.h>                  // for getaddrinfo
#include <sys/sockio.h>             // for SIOCGIFEFLAGS
#include <notify.h>
#include <netinet/in.h>             // For IP_RECVTTL
#ifndef IP_RECVTTL
#define IP_RECVTTL 24               // bool; receive reception TTL w/dgram
#endif

#include <netinet/in_systm.h>       // For n_long, required by <netinet/ip.h> below
#include <netinet/ip.h>             // For IPTOS_LOWDELAY etc.
#include <netinet6/in6_var.h>       // For IN6_IFF_TENTATIVE etc.

#include <netinet/tcp.h>

#include <DebugServices.h>
#include "dnsinfo.h"

#include <ifaddrs.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/IOMessage.h>

#include <IOKit/ps/IOPowerSources.h>
#include <IOKit/ps/IOPowerSourcesPrivate.h>
#include <IOKit/ps/IOPSKeys.h>

#include <mach/mach_error.h>
#include <mach/mach_port.h>
#include <mach/mach_time.h>
#include "helper.h"
#include "P2PPacketFilter.h"

#include <SystemConfiguration/SCPrivate.h>

#if TARGET_OS_IPHONE
#include <MobileWiFi/WiFiManagerClient.h> // For WiFiManagerClientRef etc, declarations.
#include <dlfcn.h>
#endif // TARGET_OS_IPHONE

#if MDNSRESPONDER_SUPPORTS(APPLE, IGNORE_HOSTS_FILE)
#include "system_utilities.h"               // For os_variant_has_internal_diagnostics().
#endif

// Include definition of opaque_presence_indication for KEV_DL_NODE_PRESENCE handling logic.
#include <Kernel/IOKit/apple80211/apple80211_var.h>

#if APPLE_OSX_mDNSResponder
#include <ne_session.h> // for ne_session_set_socket_attributes()
#endif

#if APPLE_OSX_mDNSResponder && TARGET_OS_OSX
#include <IOKit/platform/IOPlatformSupportPrivate.h>
#endif

#ifdef UNIT_TEST
#include "unittest.h"
#endif

#define kInterfaceSpecificOption "interface="

#define mDNS_IOREG_KEY               "mDNS_KEY"
#define mDNS_IOREG_VALUE             "2009-07-30"
#define mDNS_IOREG_KA_KEY            "mDNS_Keepalive"
#define mDNS_USER_CLIENT_CREATE_TYPE 'mDNS'

#define DARK_WAKE_TIME 16 // Time we hold an idle sleep assertion for maintenance after a wake notification

// cache the InterfaceID of the AWDL interface 
mDNSInterfaceID AWDLInterfaceID;

// ***************************************************************************
// Globals

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark - Globals
#endif

// By default we don't offer sleep proxy service
// If OfferSleepProxyService is set non-zero (typically via command-line switch),
// then we'll offer sleep proxy service on desktop Macs that are set to never sleep.
// We currently do not offer sleep proxy service on laptops, or on machines that are set to go to sleep.
mDNSexport int OfferSleepProxyService = 0;
mDNSexport int DisableSleepProxyClient = 0;
mDNSexport int UseInternalSleepProxy = 1;       // Set to non-zero to use internal (in-NIC) Sleep Proxy

mDNSexport int OSXVers, iOSVers;
mDNSexport int KQueueFD;

#ifndef NO_SECURITYFRAMEWORK
static CFArrayRef ServerCerts;
OSStatus SSLSetAllowAnonymousCiphers(SSLContextRef context, Boolean enable);
#endif /* NO_SECURITYFRAMEWORK */

static CFStringRef NetworkChangedKey_IPv4;
static CFStringRef NetworkChangedKey_IPv6;
static CFStringRef NetworkChangedKey_Hostnames;
static CFStringRef NetworkChangedKey_Computername;
static CFStringRef NetworkChangedKey_DNS;
static CFStringRef NetworkChangedKey_StateInterfacePrefix;
static CFStringRef NetworkChangedKey_DynamicDNS       = CFSTR("Setup:/Network/DynamicDNS");
static CFStringRef NetworkChangedKey_PowerSettings    = CFSTR("State:/IOKit/PowerManagement/CurrentSettings");

static char HINFO_HWstring_buffer[32];
static char *HINFO_HWstring = "Device";
static int HINFO_HWstring_prefixlen = 6;

mDNSexport int WatchDogReportingThreshold = 250;

dispatch_queue_t SSLqueue;

#if APPLE_OSX_mDNSResponder
static mDNSu8 SPMetricPortability   = 99;
static mDNSu8 SPMetricMarginalPower = 99;
static mDNSu8 SPMetricTotalPower    = 99;
static mDNSu8 SPMetricFeatures      = 1; /* The current version supports TCP Keep Alive Feature */
#endif // APPLE_OSX_mDNSResponder

#if MDNSRESPONDER_SUPPORTS(APPLE, UNICAST_DOTLOCAL)
domainname ActiveDirectoryPrimaryDomain;
static int ActiveDirectoryPrimaryDomainLabelCount;
static mDNSAddr ActiveDirectoryPrimaryDomainServer;
#endif

// Don't send triggers too often. We arbitrarily limit it to three minutes.
#define DNS_TRIGGER_INTERVAL (180 * mDNSPlatformOneSecond)

const char dnsprefix[] = "dns:";

// String Array used to write list of private domains to Dynamic Store
static CFArrayRef privateDnsArray = NULL;

// ***************************************************************************
// Functions

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Utility Functions
#endif

// We only attempt to send and receive multicast packets on interfaces that are
// (a) flagged as multicast-capable
// (b) *not* flagged as point-to-point (e.g. modem)
// Typically point-to-point interfaces are modems (including mobile-phone pseudo-modems), and we don't want
// to run up the user's bill sending multicast traffic over a link where there's only a single device at the
// other end, and that device (e.g. a modem bank) is probably not answering Multicast DNS queries anyway.

#if MDNSRESPONDER_SUPPORTS(APPLE, BONJOUR_ON_DEMAND)
#define MulticastInterface(i) ((i)->m->BonjourEnabled && ((i)->ifa_flags & IFF_MULTICAST) && !((i)->ifa_flags & IFF_POINTOPOINT))
#else
#define MulticastInterface(i) (((i)->ifa_flags & IFF_MULTICAST) && !((i)->ifa_flags & IFF_POINTOPOINT))
#endif
#define SPSInterface(i)       ((i)->ifinfo.McastTxRx && !((i)->ifa_flags & IFF_LOOPBACK) && !(i)->D2DInterface)

mDNSlocal void SetNetworkChanged(mDNSs32 delay);

mDNSexport void NotifyOfElusiveBug(const char *title, const char *msg)  // Both strings are UTF-8 text
{
    // Unless ForceAlerts is defined, we only show these bug report alerts on machines that have a 17.x.x.x address
    #if !ForceAlerts
    {
        // Determine if we're at Apple (17.*.*.*)
        NetworkInterfaceInfoOSX *i;
        for (i = mDNSStorage.p->InterfaceList; i; i = i->next)
            if (i->ifinfo.ip.type == mDNSAddrType_IPv4 && i->ifinfo.ip.ip.v4.b[0] == 17)
                break;
        if (!i) 
            return; // If not at Apple, don't show the alert
    }
    #endif

    LogMsg("NotifyOfElusiveBug: %s", title);
    LogMsg("NotifyOfElusiveBug: %s", msg);

    // If we display our alert early in the boot process, then it vanishes once the desktop appears.
    // To avoid this, we don't try to display alerts in the first three minutes after boot.
    if ((mDNSu32)(mDNSPlatformRawTime()) < (mDNSu32)(mDNSPlatformOneSecond * 180))
    { 
        LogMsg("Suppressing notification early in boot: %d", mDNSPlatformRawTime()); 
        return; 
    }

#ifndef NO_CFUSERNOTIFICATION
    static int notifyCount = 0; // To guard against excessive display of warning notifications
    if (notifyCount < 5) 
    { 
        notifyCount++; 
        mDNSNotify(title, msg); 
    }
#endif /* NO_CFUSERNOTIFICATION */

}

// Write a syslog message and display an alert, then if ForceAlerts is set, generate a stack trace
#if MDNS_MALLOC_DEBUGGING >= 1
mDNSexport void LogMemCorruption(const char *format, ...)
{
    char buffer[512];
    va_list ptr;
    va_start(ptr,format);
    buffer[mDNS_vsnprintf((char *)buffer, sizeof(buffer), format, ptr)] = 0;
    va_end(ptr);
    LogMsg("!!!! %s !!!!", buffer);
    NotifyOfElusiveBug("Memory Corruption", buffer);
#if ForceAlerts
    *(volatile long*)0 = 0;  // Trick to crash and get a stack trace right here, if that's what we want
#endif
}
#endif

// Like LogMemCorruption above, but only display the alert if ForceAlerts is set and we're going to generate a stack trace
#if APPLE_OSX_mDNSResponder
mDNSexport void LogFatalError(const char *format, ...)
{
    char buffer[512];
    va_list ptr;
    va_start(ptr,format);
    buffer[mDNS_vsnprintf((char *)buffer, sizeof(buffer), format, ptr)] = 0;
    va_end(ptr);
    LogMsg("!!!! %s !!!!", buffer);
#if ForceAlerts
    NotifyOfElusiveBug("Fatal Error. See /Library/Logs/DiagnosticReports", buffer);
    *(volatile long*)0 = 0;  // Trick to crash and get a stack trace right here, if that's what we want
#endif
}
#endif

// Returns true if it is an AppleTV based hardware running iOS, false otherwise
mDNSlocal mDNSBool IsAppleTV(void)
{
#if TARGET_OS_TV
    return mDNStrue;
#else 
    return mDNSfalse;
#endif
}

mDNSlocal struct ifaddrs *myGetIfAddrs(int refresh)
{
    static struct ifaddrs *ifa = NULL;

    if (refresh && ifa)
    {
        freeifaddrs(ifa);
        ifa = NULL;
    }

    if (ifa == NULL) 
        getifaddrs(&ifa);
    return ifa;
}

mDNSlocal void DynamicStoreWrite(int key, const char* subkey, uintptr_t value, signed long valueCnt)
{
    CFStringRef sckey       = NULL;
    Boolean release_sckey   = FALSE;
    CFDataRef bytes         = NULL;
    CFPropertyListRef plist = NULL;

    switch ((enum mDNSDynamicStoreSetConfigKey)key)
    {
        case kmDNSMulticastConfig:
            sckey = CFSTR("State:/Network/" kDNSServiceCompMulticastDNS);
            break;
        case kmDNSDynamicConfig:
            sckey = CFSTR("State:/Network/DynamicDNS");
            break;
        case kmDNSPrivateConfig:
            sckey = CFSTR("State:/Network/" kDNSServiceCompPrivateDNS);
            break;
        case kmDNSBackToMyMacConfig:
            sckey = CFSTR("State:/Network/BackToMyMac");
            break;
        case kmDNSSleepProxyServersState:
        {
            CFMutableStringRef tmp = CFStringCreateMutable(kCFAllocatorDefault, 0);
            CFStringAppend(tmp, CFSTR("State:/Network/Interface/"));
            CFStringAppendCString(tmp, subkey, kCFStringEncodingUTF8);
            CFStringAppend(tmp, CFSTR("/SleepProxyServers"));
            sckey = CFStringCreateCopy(kCFAllocatorDefault, tmp);
            release_sckey = TRUE;
            CFRelease(tmp);
            break;
        }
        case kmDNSDebugState:
            sckey = CFSTR("State:/Network/mDNSResponder/DebugState");
            break;
        default:
            LogMsg("unrecognized key %d", key);
            goto fin;
    }
    if (NULL == (bytes = CFDataCreateWithBytesNoCopy(NULL, (void *)value,
                                                     valueCnt, kCFAllocatorNull)))
    {
        LogMsg("CFDataCreateWithBytesNoCopy of value failed");
        goto fin;
    }
    if (NULL == (plist = CFPropertyListCreateWithData(NULL, bytes, kCFPropertyListImmutable, NULL, NULL)))
    {
        LogMsg("CFPropertyListCreateWithData of bytes failed");
        goto fin;
    }
    CFRelease(bytes);
    bytes = NULL;
    SCDynamicStoreSetValue(NULL, sckey, plist);

fin:
    if (NULL != bytes)
        CFRelease(bytes);
    if (NULL != plist)
        CFRelease(plist);
    if (release_sckey && sckey)
        CFRelease(sckey);
}

mDNSexport void mDNSDynamicStoreSetConfig(int key, const char *subkey, CFPropertyListRef value)
{
    CFPropertyListRef valueCopy;
    char *subkeyCopy  = NULL;
    if (!value)
        return;

    // We need to copy the key and value before we dispatch off the block below as the
    // caller will free the memory once we return from this function.
    valueCopy = CFPropertyListCreateDeepCopy(NULL, value, kCFPropertyListImmutable);
    if (!valueCopy)
    {   
        LogMsg("mDNSDynamicStoreSetConfig: ERROR valueCopy NULL");
        return;
    }
    if (subkey)
    {
        int len    = strlen(subkey);
        subkeyCopy = mDNSPlatformMemAllocate(len + 1);
        if (!subkeyCopy)
        {
            LogMsg("mDNSDynamicStoreSetConfig: ERROR subkeyCopy NULL");
            CFRelease(valueCopy);
            return;
        }
        mDNSPlatformMemCopy(subkeyCopy, subkey, len);
        subkeyCopy[len] = 0;
    }

    dispatch_async(dispatch_get_main_queue(), ^{
        CFWriteStreamRef stream = NULL;
        CFDataRef bytes = NULL;
        CFIndex ret;
        KQueueLock();

        if (NULL == (stream = CFWriteStreamCreateWithAllocatedBuffers(NULL, NULL)))
        {
            LogMsg("mDNSDynamicStoreSetConfig : CFWriteStreamCreateWithAllocatedBuffers failed (Object creation failed)");
            goto END;
        }
        CFWriteStreamOpen(stream);
        ret = CFPropertyListWrite(valueCopy, stream, kCFPropertyListBinaryFormat_v1_0, 0, NULL);
        if (ret == 0)
        {
            LogMsg("mDNSDynamicStoreSetConfig : CFPropertyListWriteToStream failed (Could not write property list to stream)");
            goto END;
        }
        if (NULL == (bytes = CFWriteStreamCopyProperty(stream, kCFStreamPropertyDataWritten)))
        {
            LogMsg("mDNSDynamicStoreSetConfig : CFWriteStreamCopyProperty failed (Object creation failed) ");
            goto END;
        }
        CFWriteStreamClose(stream);
        CFRelease(stream);
        stream = NULL;
        DynamicStoreWrite(key, subkeyCopy ? subkeyCopy : "", (uintptr_t)CFDataGetBytePtr(bytes), CFDataGetLength(bytes));

    END:
        CFRelease(valueCopy);
        if (NULL != stream)
        {
            CFWriteStreamClose(stream);
            CFRelease(stream);
        }
        if (NULL != bytes)
            CFRelease(bytes); 
        if (subkeyCopy)
            mDNSPlatformMemFree(subkeyCopy);

        KQueueUnlock("mDNSDynamicStoreSetConfig");
    });
}

// To match *either* a v4 or v6 instance of this interface name, pass AF_UNSPEC for type
mDNSlocal NetworkInterfaceInfoOSX *SearchForInterfaceByName(const char *ifname, int type)
{
    NetworkInterfaceInfoOSX *i;
    for (i = mDNSStorage.p->InterfaceList; i; i = i->next)
        if (i->Exists && !strcmp(i->ifinfo.ifname, ifname) &&
            ((type == AF_UNSPEC                                         ) ||
             (type == AF_INET  && i->ifinfo.ip.type == mDNSAddrType_IPv4) ||
             (type == AF_INET6 && i->ifinfo.ip.type == mDNSAddrType_IPv6))) return(i);
    return(NULL);
}

mDNSlocal int myIfIndexToName(u_short ifindex, char *name)
{
    struct ifaddrs *ifa;
    for (ifa = myGetIfAddrs(0); ifa; ifa = ifa->ifa_next)
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_LINK)
            if (((struct sockaddr_dl*)ifa->ifa_addr)->sdl_index == ifindex)
            { strlcpy(name, ifa->ifa_name, IF_NAMESIZE); return 0; }
    return -1;
}

mDNSexport NetworkInterfaceInfoOSX *IfindexToInterfaceInfoOSX(mDNSInterfaceID ifindex)
{
    mDNS *const m = &mDNSStorage;
    mDNSu32 scope_id = (mDNSu32)(uintptr_t)ifindex;
    NetworkInterfaceInfoOSX *i;

    // Don't get tricked by inactive interfaces
    for (i = m->p->InterfaceList; i; i = i->next)
        if (i->Registered && i->scope_id == scope_id) return(i);

    return mDNSNULL;
}

mDNSexport mdns_interface_monitor_t GetInterfaceMonitorForIndex(uint32_t ifIndex)
{
    mDNS *const m = &mDNSStorage;

    // We assume that interface should always be real interface, and should never be 0.
    if (ifIndex == 0) return NULL;

    if (!m->p->InterfaceMonitors)
    {
        m->p->InterfaceMonitors = CFArrayCreateMutable(kCFAllocatorDefault, 0, &mdns_cfarray_callbacks);
        if (!m->p->InterfaceMonitors)
        {
            LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_ERROR, "Failed to create InterfaceMonitors array");
            return NULL;
        }
    }

    // Search for interface monitor given the interface index.
    mdns_interface_monitor_t monitor;
    for (CFIndex i = 0, n = CFArrayGetCount(m->p->InterfaceMonitors); i < n; i++)
    {
        monitor = (mdns_interface_monitor_t) CFArrayGetValueAtIndex(m->p->InterfaceMonitors, i);
        if (mdns_interface_monitor_get_interface_index(monitor) == ifIndex) return monitor;
    }

    // If we come here, it means the interface is a new interface that needs to be monitored.
    monitor = mdns_interface_monitor_create(ifIndex);
    if (!monitor)
    {
        LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_ERROR, "Failed to create an interface monitor for index %u", ifIndex);
        return NULL;
    }
    CFArrayAppendValue(m->p->InterfaceMonitors, monitor);

    // Put the monitor into serial queue.
    mdns_interface_monitor_set_queue(monitor, dispatch_get_main_queue());

    // When the interface configuration is changed, this block will be called.
    mdns_interface_monitor_set_update_handler(monitor,
    ^(mdns_interface_flags_t changeFlags)
    {
        const mdns_interface_flags_t relevantFlags =
            mdns_interface_flag_expensive   |
            mdns_interface_flag_constrained |
            mdns_interface_flag_clat46;
        if ((changeFlags & relevantFlags) == 0) return;

        KQueueLock();
        const CFRange range = CFRangeMake(0, CFArrayGetCount(m->p->InterfaceMonitors));
        if (CFArrayContainsValue(m->p->InterfaceMonitors, range, monitor))
        {
            m->p->if_interface_changed = mDNStrue;

#if MDNSRESPONDER_SUPPORTS(APPLE, OS_LOG)
            LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_INFO, "Monitored interface changed: %@", monitor);
#endif
            // Let mDNSResponder update its network configuration.
            mDNS_Lock(m);
            SetNetworkChanged((mDNSPlatformOneSecond + 39) / 40);   // 25 ms delay
            mDNS_Unlock(m);
        }
        KQueueUnlock("interface monitor update handler");
    });

    mdns_interface_monitor_set_event_handler(monitor,
    ^(mdns_event_t event, OSStatus error)
    {
        switch (event)
        {
            case mdns_event_invalidated:
                mdns_release(monitor);
                break;

            case mdns_event_error:
                LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_ERROR, "Interface monitor for index %u error: %ld",
                    mdns_interface_monitor_get_interface_index(monitor), (long) error);
                KQueueLock();
                if (m->p->InterfaceMonitors)
                {
                    const CFRange range = CFRangeMake(0, CFArrayGetCount(m->p->InterfaceMonitors));
                    const CFIndex i = CFArrayGetFirstIndexOfValue(m->p->InterfaceMonitors, range, monitor);
                    if (i >= 0) CFArrayRemoveValueAtIndex(m->p->InterfaceMonitors, i);
                }
                KQueueUnlock("interface monitor event handler");
                mdns_interface_monitor_invalidate(monitor);
                break;

            default:
                break;
        }
    });
    mdns_interface_monitor_activate(monitor);

    return monitor;
}

mDNSexport mDNSInterfaceID mDNSPlatformInterfaceIDfromInterfaceIndex(mDNS *const m, mDNSu32 ifindex)
{
    (void) m;
    if (ifindex == kDNSServiceInterfaceIndexLocalOnly) return(mDNSInterface_LocalOnly);
    if (ifindex == kDNSServiceInterfaceIndexP2P      ) return(mDNSInterface_P2P);
    if (ifindex == kDNSServiceInterfaceIndexBLE      ) return(mDNSInterface_BLE);
    if (ifindex == kDNSServiceInterfaceIndexAny      ) return(mDNSNULL);

    NetworkInterfaceInfoOSX* ifi = IfindexToInterfaceInfoOSX((mDNSInterfaceID)(uintptr_t)ifindex);
    if (!ifi)
    {
        // Not found. Make sure our interface list is up to date, then try again.
        LogInfo("mDNSPlatformInterfaceIDfromInterfaceIndex: InterfaceID for interface index %d not found; Updating interface list", ifindex);
        mDNSMacOSXNetworkChanged();
        ifi = IfindexToInterfaceInfoOSX((mDNSInterfaceID)(uintptr_t)ifindex);
    }

    if (!ifi) return(mDNSNULL);

    return(ifi->ifinfo.InterfaceID);
}


mDNSexport mDNSu32 mDNSPlatformInterfaceIndexfromInterfaceID(mDNS *const m, mDNSInterfaceID id, mDNSBool suppressNetworkChange)
{
    NetworkInterfaceInfoOSX *i;
    if (id == mDNSInterface_Any      ) return(0);
    if (id == mDNSInterface_LocalOnly) return(kDNSServiceInterfaceIndexLocalOnly);
    if (id == mDNSInterface_P2P      ) return(kDNSServiceInterfaceIndexP2P);
    if (id == mDNSInterface_BLE      ) return(kDNSServiceInterfaceIndexBLE);

    mDNSu32 scope_id = (mDNSu32)(uintptr_t)id;

    // Don't use i->Registered here, because we DO want to find inactive interfaces, which have no Registered set
    for (i = m->p->InterfaceList; i; i = i->next)
        if (i->scope_id == scope_id) return(i->scope_id);

    // If we are supposed to suppress network change, return "id" back
    if (suppressNetworkChange) return scope_id;

    // Not found. Make sure our interface list is up to date, then try again.
    LogInfo("Interface index for InterfaceID %p not found; Updating interface list", id);
    mDNSMacOSXNetworkChanged();
    for (i = m->p->InterfaceList; i; i = i->next)
        if (i->scope_id == scope_id) return(i->scope_id);

    return(0);
}

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - UDP & TCP send & receive
#endif

mDNSlocal mDNSBool AddrRequiresPPPConnection(const struct sockaddr *addr)
{
    mDNSBool result = mDNSfalse;
    SCNetworkConnectionFlags flags;
    CFDataRef remote_addr;
    CFMutableDictionaryRef options;
    SCNetworkReachabilityRef ReachRef = NULL;

    options = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    remote_addr = CFDataCreate(NULL, (const UInt8 *)addr, addr->sa_len);
    CFDictionarySetValue(options, kSCNetworkReachabilityOptionRemoteAddress, remote_addr);
    CFDictionarySetValue(options, kSCNetworkReachabilityOptionServerBypass, kCFBooleanTrue);
    ReachRef = SCNetworkReachabilityCreateWithOptions(kCFAllocatorDefault, options);
    CFRelease(options);
    CFRelease(remote_addr);

    if (!ReachRef) 
    { 
        LogMsg("ERROR: RequiresConnection - SCNetworkReachabilityCreateWithOptions"); 
        goto end; 
    }
    if (!SCNetworkReachabilityGetFlags(ReachRef, &flags)) 
    { 
        LogMsg("ERROR: AddrRequiresPPPConnection - SCNetworkReachabilityGetFlags"); 
        goto end; 
    }
    result = flags & kSCNetworkFlagsConnectionRequired;

end:
    if (ReachRef) 
        CFRelease(ReachRef);
    return result;
}

// Set traffic class for socket
mDNSlocal void setTrafficClass(int socketfd, mDNSBool useBackgroundTrafficClass)
{
    int traffic_class;

    if (useBackgroundTrafficClass)
        traffic_class = SO_TC_BK_SYS;
    else
        traffic_class = SO_TC_CTL;

    (void) setsockopt(socketfd, SOL_SOCKET, SO_TRAFFIC_CLASS, (void *)&traffic_class, sizeof(traffic_class));
}

#ifdef UNIT_TEST
UNITTEST_SETSOCKOPT
#else
mDNSlocal int mDNSPlatformGetSocktFd(void *sockCxt, mDNSTransport_Type transType, mDNSAddr_Type addrType)
{
    if (transType == mDNSTransport_UDP)
    {
        UDPSocket* sock = (UDPSocket*) sockCxt;
        return (addrType == mDNSAddrType_IPv4) ? sock->ss.sktv4 : sock->ss.sktv6;
    }
    else if (transType == mDNSTransport_TCP)
    {
        TCPSocket* sock = (TCPSocket*) sockCxt;
        return sock->fd;
    }
    else
    {
        LogInfo("mDNSPlatformGetSocktFd: invalid transport %d", transType);
        return kInvalidSocketRef;
    }
}

mDNSexport void mDNSPlatformSetSocktOpt(void *sockCxt, mDNSTransport_Type transType, mDNSAddr_Type addrType, const DNSQuestion *q)
{
    int sockfd;
    char unenc_name[MAX_ESCAPED_DOMAIN_NAME];

    // verify passed-in arguments exist and that sockfd is valid
    if (q == mDNSNULL || sockCxt == mDNSNULL || (sockfd = mDNSPlatformGetSocktFd(sockCxt, transType, addrType)) < 0)
        return;

    if (q->pid)
    {
        if (setsockopt(sockfd, SOL_SOCKET, SO_DELEGATED, &q->pid, sizeof(q->pid)) == -1)
            LogMsg("mDNSPlatformSetSocktOpt: Delegate PID failed %s for PID %d", strerror(errno), q->pid);
    }
    else
    {
        if (setsockopt(sockfd, SOL_SOCKET, SO_DELEGATED_UUID, &q->uuid, sizeof(q->uuid)) == -1)
            LogMsg("mDNSPlatformSetSocktOpt: Delegate UUID failed %s", strerror(errno));
    }

    // set the domain on the socket
    ConvertDomainNameToCString(&q->qname, unenc_name);
    if (!(ne_session_set_socket_attributes(sockfd, unenc_name, NULL)))
        LogInfo("mDNSPlatformSetSocktOpt: ne_session_set_socket_attributes()-> setting domain failed for %s", unenc_name);

    int nowake = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_NOWAKEFROMSLEEP, &nowake, sizeof(nowake)) == -1)
        LogInfo("mDNSPlatformSetSocktOpt: SO_NOWAKEFROMSLEEP failed %s", strerror(errno));
}
#endif // UNIT_TEST

// Note: If InterfaceID is NULL, it means, "send this packet through our anonymous unicast socket"
// Note: If InterfaceID is non-NULL it means, "send this packet through our port 5353 socket on the specified interface"
// OR send via our primary v4 unicast socket
// UPDATE: The UDPSocket *src parameter now allows the caller to specify the source socket
mDNSexport mStatus mDNSPlatformSendUDP(const mDNS *const m, const void *const msg, const mDNSu8 *const end,
                                       mDNSInterfaceID InterfaceID, UDPSocket *src, const mDNSAddr *dst, 
                                       mDNSIPPort dstPort, mDNSBool useBackgroundTrafficClass)
{
    NetworkInterfaceInfoOSX *info = mDNSNULL;
    struct sockaddr_storage to;
    int s = -1, err;
    mStatus result = mStatus_NoError;
    int sendto_errno;
    const DNSMessage *const dns_msg = msg;

    if (InterfaceID)
    {
        info = IfindexToInterfaceInfoOSX(InterfaceID);
        if (info == NULL)
        {
            // We may not have registered interfaces with the "core" as we may not have
            // seen any interface notifications yet. This typically happens during wakeup
            // where we might try to send DNS requests (non-SuppressUnusable questions internal
            // to mDNSResponder) before we receive network notifications.
            LogInfo("mDNSPlatformSendUDP: Invalid interface index %p", InterfaceID);
            return mStatus_BadParamErr;
        }
    }

    char *ifa_name = InterfaceID ? info->ifinfo.ifname : "unicast";

    if (dst->type == mDNSAddrType_IPv4)
    {
        struct sockaddr_in *sin_to = (struct sockaddr_in*)&to;
        sin_to->sin_len            = sizeof(*sin_to);
        sin_to->sin_family         = AF_INET;
        sin_to->sin_port           = dstPort.NotAnInteger;
        sin_to->sin_addr.s_addr    = dst->ip.v4.NotAnInteger;
        s = (src ? src->ss : m->p->permanentsockets).sktv4;

        if (!mDNSAddrIsDNSMulticast(dst))
        {
        #ifdef IP_BOUND_IF
            const mDNSu32 ifindex = info ? info->scope_id : IFSCOPE_NONE;
            setsockopt(s, IPPROTO_IP, IP_BOUND_IF, &ifindex, sizeof(ifindex));
        #else
            static int displayed = 0;
            if (displayed < 1000)
            {
                displayed++;
                LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_INFO,
                          "[Q%u] IP_BOUND_IF socket option not defined -- cannot specify interface for unicast packets",
                          mDNSVal16(dns_msg->h.id));
            }
        #endif
        }
        else if (info)
        {
        #ifdef IP_MULTICAST_IFINDEX
            err = setsockopt(s, IPPROTO_IP, IP_MULTICAST_IFINDEX, &info->scope_id, sizeof(info->scope_id));
            // We get an error when we compile on a machine that supports this option and run the binary on
            // a different machine that does not support it
            if (err < 0)
            {
                if (errno != ENOPROTOOPT)
                {
                    LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_ERROR,
                              "[Q%u] mDNSPlatformSendUDP: setsockopt: IP_MUTLTICAST_IFINDEX returned %d",
                              mDNSVal16(dns_msg->h.id), errno);
                }
                err = setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, &info->ifa_v4addr, sizeof(info->ifa_v4addr));
                if (err < 0 && !m->NetworkChanged)
                {
                    LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_ERROR,
                              "[Q%u] setsockopt - IP_MULTICAST_IF error " PRI_IPv4_ADDR " %d errno %d (" PUB_S ")",
                              mDNSVal16(dns_msg->h.id), &info->ifa_v4addr, err, errno, strerror(errno));
                }
            }
        #else
            err = setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, &info->ifa_v4addr, sizeof(info->ifa_v4addr));
            if (err < 0 && !m->NetworkChanged)
            {
                LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_ERROR,
                          "[Q%u] setsockopt - IP_MULTICAST_IF error " PRI_IPv4_ADDR " %d errno %d (" PUB_S ")",
                          mDNSVal16(dns_msg->h.id), &info->ifa_v4addr, err, errno, strerror(errno));
            }
        #endif
        }
    }
    else if (dst->type == mDNSAddrType_IPv6)
    {
        struct sockaddr_in6 *sin6_to = (struct sockaddr_in6*)&to;
        sin6_to->sin6_len            = sizeof(*sin6_to);
        sin6_to->sin6_family         = AF_INET6;
        sin6_to->sin6_port           = dstPort.NotAnInteger;
        sin6_to->sin6_flowinfo       = 0;
        sin6_to->sin6_addr           = *(struct in6_addr*)&dst->ip.v6;
        sin6_to->sin6_scope_id       = info ? info->scope_id : 0;
        s = (src ? src->ss : m->p->permanentsockets).sktv6;
        if (info && mDNSAddrIsDNSMulticast(dst))    // Specify outgoing interface
        {
            err = setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_IF, &info->scope_id, sizeof(info->scope_id));
            if (err < 0)
            {
                const int setsockopt_errno = errno;
                char name[IFNAMSIZ];
                if (if_indextoname(info->scope_id, name) != NULL)
                {
                    LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_ERROR,
                              "[Q%u] setsockopt - IPV6_MULTICAST_IF error %d errno %d (" PUB_S ")",
                              mDNSVal16(dns_msg->h.id), err, setsockopt_errno, strerror(setsockopt_errno));
                }
                else
                {
                    LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_ERROR,
                              "[Q%u] setsockopt - IPV6_MUTLICAST_IF scopeid %d, not a valid interface",
                              mDNSVal16(dns_msg->h.id), info->scope_id);
                }
            }
        }
#ifdef IPV6_BOUND_IF
        if (info)   // Specify outgoing interface for non-multicast destination
        {
            if (!mDNSAddrIsDNSMulticast(dst))
            {
                if (info->scope_id == 0)
                {
                    LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_INFO,
                              "[Q%u] IPV6_BOUND_IF socket option not set -- info %p (" PUB_S ") scope_id is zero",
                              mDNSVal16(dns_msg->h.id), info, ifa_name);
                }
                else
                {
                    setsockopt(s, IPPROTO_IPV6, IPV6_BOUND_IF, &info->scope_id, sizeof(info->scope_id));
                }
            }
        }
#endif
    }
    else
    {
        LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_FAULT,
                  "[Q%u] mDNSPlatformSendUDP: dst is not an IPv4 or IPv6 address!", mDNSVal16(dns_msg->h.id));
        return mStatus_BadParamErr;
    }

    if (s >= 0)
    {
        verbosedebugf("mDNSPlatformSendUDP: sending on InterfaceID %p %5s/%ld to %#a:%d skt %d",
                      InterfaceID, ifa_name, dst->type, dst, mDNSVal16(dstPort), s);
    }
    else
    {
        verbosedebugf("mDNSPlatformSendUDP: NOT sending on InterfaceID %p %5s/%ld (socket of this type not available)",
                      InterfaceID, ifa_name, dst->type, dst, mDNSVal16(dstPort));
    }

    // Note: When sending, mDNSCore may often ask us to send both a v4 multicast packet and then a v6 multicast packet
    // If we don't have the corresponding type of socket available, then return mStatus_Invalid
    if (s < 0) return(mStatus_Invalid);

    // switch to background traffic class for this message if requested
    if (useBackgroundTrafficClass)
        setTrafficClass(s, useBackgroundTrafficClass);

    err = sendto(s, msg, (UInt8*)end - (UInt8*)msg, 0, (struct sockaddr *)&to, to.ss_len);
    sendto_errno = (err < 0) ? errno : 0;

    // set traffic class back to default value
    if (useBackgroundTrafficClass)
        setTrafficClass(s, mDNSfalse);

    if (err < 0)
    {
        static int MessageCount = 0;
        LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_ERROR,
                  "[Q%u] mDNSPlatformSendUDP -> sendto(%d) failed to send packet on InterfaceID %p " PUB_S "/%d to " PRI_IP_ADDR ":%d skt %d error %d errno %d (" PUB_S ") %u",
                  mDNSVal16(dns_msg->h.id), s, InterfaceID, ifa_name, dst->type, dst, mDNSVal16(dstPort), s, err, sendto_errno, strerror(sendto_errno), (mDNSu32)(m->timenow));
        if (!mDNSAddressIsAllDNSLinkGroup(dst))
        {
            if ((sendto_errno == EHOSTUNREACH) || (sendto_errno == ENETUNREACH)) return(mStatus_HostUnreachErr);
            if ((sendto_errno == EHOSTDOWN)    || (sendto_errno == ENETDOWN))    return(mStatus_TransientErr);
        }
        // Don't report EHOSTUNREACH in the first three minutes after boot
        // This is because mDNSResponder intentionally starts up early in the boot process (See <rdar://problem/3409090>)
        // but this means that sometimes it starts before configd has finished setting up the multicast routing entries.
        if (sendto_errno == EHOSTUNREACH && (mDNSu32)(mDNSPlatformRawTime()) < (mDNSu32)(mDNSPlatformOneSecond * 180)) return(mStatus_TransientErr);
        // Don't report EADDRNOTAVAIL ("Can't assign requested address") if we're in the middle of a network configuration change
        if (sendto_errno == EADDRNOTAVAIL && m->NetworkChanged) return(mStatus_TransientErr);
        if (sendto_errno == EHOSTUNREACH || sendto_errno == EADDRNOTAVAIL || sendto_errno == ENETDOWN)
        {
            LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_ERROR,
                      "[Q%u] mDNSPlatformSendUDP sendto(%d) failed to send packet on InterfaceID %p " PUB_S "/%d to " PRI_IP_ADDR ":%d skt %d error %d errno %d (" PUB_S ") %u",
                      mDNSVal16(dns_msg->h.id), s, InterfaceID, ifa_name, dst->type, dst, mDNSVal16(dstPort), s, err, sendto_errno, strerror(sendto_errno), (mDNSu32)(m->timenow));
        }
        else
        {
            MessageCount++;
            if (MessageCount < 50) // Cap and ensure NO spamming of LogMsgs
            {
                LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_ERROR,
                          "[Q%u] mDNSPlatformSendUDP: sendto(%d) failed to send packet on InterfaceID %p " PUB_S "/%d to " PRI_IP_ADDR ":%d skt %d error %d errno %d (" PUB_S ") %u MessageCount is %d",
                          mDNSVal16(dns_msg->h.id), s, InterfaceID, ifa_name, dst->type, dst, mDNSVal16(dstPort), s, err, sendto_errno, strerror(sendto_errno), (mDNSu32)(m->timenow), MessageCount);
            }
            else // If logging is enabled, remove the cap and log aggressively
            {
                LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_INFO,
                          "[Q%u] mDNSPlatformSendUDP: sendto(%d) failed to send packet on InterfaceID %p " PUB_S "/%d to " PRI_IP_ADDR ":%d skt %d error %d errno %d (" PUB_S ") %u MessageCount is %d",
                          mDNSVal16(dns_msg->h.id), s, InterfaceID, ifa_name, dst->type, dst, mDNSVal16(dstPort), s, err, sendto_errno, strerror(sendto_errno), (mDNSu32)(m->timenow), MessageCount);
            }
        }

        result = mStatus_UnknownErr;
    }

    return(result);
}

mDNSexport ssize_t myrecvfrom(const int s, void *const buffer, const size_t max,
                             struct sockaddr *const from, size_t *const fromlen, mDNSAddr *dstaddr, char ifname[IF_NAMESIZE], mDNSu8 *ttl)
{
    static unsigned int numLogMessages = 0;
    struct iovec databuffers = { (char *)buffer, max };
    struct msghdr msg;
    ssize_t n;
    struct cmsghdr *cmPtr;
    char ancillary[1024];

    *ttl = 255;  // If kernel fails to provide TTL data (e.g. Jaguar doesn't) then assume the TTL was 255 as it should be

    // Set up the message
    msg.msg_name       = (caddr_t)from;
    msg.msg_namelen    = *fromlen;
    msg.msg_iov        = &databuffers;
    msg.msg_iovlen     = 1;
    msg.msg_control    = (caddr_t)&ancillary;
    msg.msg_controllen = sizeof(ancillary);
    msg.msg_flags      = 0;

    // Receive the data
    n = recvmsg(s, &msg, 0);
    if (n<0)
    {
        if (errno != EWOULDBLOCK && numLogMessages++ < 100) LogMsg("mDNSMacOSX.c: recvmsg(%d) returned error %d errno %d", s, n, errno);
        return(-1);
    }
    if (msg.msg_controllen < (int)sizeof(struct cmsghdr))
    {
        if (numLogMessages++ < 100) LogMsg("mDNSMacOSX.c: recvmsg(%d) returned %d msg.msg_controllen %d < sizeof(struct cmsghdr) %lu, errno %d",
                                           s, n, msg.msg_controllen, sizeof(struct cmsghdr), errno);
        return(-1);
    }
    if (msg.msg_flags & MSG_CTRUNC)
    {
        if (numLogMessages++ < 100) LogMsg("mDNSMacOSX.c: recvmsg(%d) msg.msg_flags & MSG_CTRUNC", s);
        return(-1);
    }

    *fromlen = msg.msg_namelen;

    // Parse each option out of the ancillary data.
    for (cmPtr = CMSG_FIRSTHDR(&msg); cmPtr; cmPtr = CMSG_NXTHDR(&msg, cmPtr))
    {
        // debugf("myrecvfrom cmsg_level %d cmsg_type %d", cmPtr->cmsg_level, cmPtr->cmsg_type);
        if (cmPtr->cmsg_level == IPPROTO_IP && cmPtr->cmsg_type == IP_RECVDSTADDR)
        {
            dstaddr->type = mDNSAddrType_IPv4;
            dstaddr->ip.v4 = *(mDNSv4Addr*)CMSG_DATA(cmPtr);
            //LogMsg("mDNSMacOSX.c: recvmsg IP_RECVDSTADDR %.4a", &dstaddr->ip.v4);
        }
        if (cmPtr->cmsg_level == IPPROTO_IP && cmPtr->cmsg_type == IP_RECVIF)
        {
            struct sockaddr_dl *sdl = (struct sockaddr_dl *)CMSG_DATA(cmPtr);
            if (sdl->sdl_nlen < IF_NAMESIZE)
            {
                mDNSPlatformMemCopy(ifname, sdl->sdl_data, sdl->sdl_nlen);
                ifname[sdl->sdl_nlen] = 0;
                // debugf("IP_RECVIF sdl_index %d, sdl_data %s len %d", sdl->sdl_index, ifname, sdl->sdl_nlen);
            }
        }
        if (cmPtr->cmsg_level == IPPROTO_IP && cmPtr->cmsg_type == IP_RECVTTL)
            *ttl = *(u_char*)CMSG_DATA(cmPtr);
        if (cmPtr->cmsg_level == IPPROTO_IPV6 && cmPtr->cmsg_type == IPV6_PKTINFO)
        {
            struct in6_pktinfo *ip6_info = (struct in6_pktinfo*)CMSG_DATA(cmPtr);
            dstaddr->type = mDNSAddrType_IPv6;
            dstaddr->ip.v6 = *(mDNSv6Addr*)&ip6_info->ipi6_addr;
            myIfIndexToName(ip6_info->ipi6_ifindex, ifname);
        }
        if (cmPtr->cmsg_level == IPPROTO_IPV6 && cmPtr->cmsg_type == IPV6_HOPLIMIT)
            *ttl = *(int*)CMSG_DATA(cmPtr);
    }

    return(n);
}

// What is this for, and why does it use xor instead of a simple equality check? -- SC
mDNSlocal mDNSInterfaceID FindMyInterface(const mDNSAddr *addr)
{
    NetworkInterfaceInfo *intf;

    if (addr->type == mDNSAddrType_IPv4)
    {
        for (intf = mDNSStorage.HostInterfaces; intf; intf = intf->next)
        {
            if (intf->ip.type == addr->type && intf->McastTxRx)
            {
                if ((intf->ip.ip.v4.NotAnInteger ^ addr->ip.v4.NotAnInteger) == 0)
                {
                    return(intf->InterfaceID);
                }
            }
        }
    }

    if (addr->type == mDNSAddrType_IPv6)
    {
        for (intf = mDNSStorage.HostInterfaces; intf; intf = intf->next)
        {
            if (intf->ip.type == addr->type && intf->McastTxRx)
            {
                if (((intf->ip.ip.v6.l[0] ^ addr->ip.v6.l[0]) == 0) &&
                    ((intf->ip.ip.v6.l[1] ^ addr->ip.v6.l[1]) == 0) &&
                    ((intf->ip.ip.v6.l[2] ^ addr->ip.v6.l[2]) == 0) &&
                    (((intf->ip.ip.v6.l[3] ^ addr->ip.v6.l[3]) == 0)))
                    {
                        return(intf->InterfaceID);
                    }
            }
        }
    }
    return(mDNSInterface_Any);
}

mDNSexport void myKQSocketCallBack(int s1, short filter, void *context, mDNSBool encounteredEOF)
{
    KQSocketSet *const ss = (KQSocketSet *)context;
    mDNS *const m = ss->m;
    int err = 0, count = 0, closed = 0, recvfrom_errno = 0;

    if (filter != EVFILT_READ)
        LogMsg("myKQSocketCallBack: Why is filter %d not EVFILT_READ (%d)?", filter, EVFILT_READ);

    if (s1 != ss->sktv4 && s1 != ss->sktv6)
    {
        LogMsg("myKQSocketCallBack: native socket %d", s1);
        LogMsg("myKQSocketCallBack: sktv4 %d sktv6 %d", ss->sktv4, ss->sktv6);
    }

    if (encounteredEOF)
    {
        LogMsg("myKQSocketCallBack: socket %d is no longer readable (EOF)", s1);
        if (s1 == ss->sktv4)
        {
            ss->sktv4EOF = mDNStrue;
            KQueueSet(ss->sktv4, EV_DELETE, EVFILT_READ, &ss->kqsv4);
        }
        else if (s1 == ss->sktv6)
        {
            ss->sktv6EOF = mDNStrue;
            KQueueSet(ss->sktv6, EV_DELETE, EVFILT_READ, &ss->kqsv6);
        }
        return;
    }

    while (!closed)
    {
        mDNSAddr senderAddr, destAddr = zeroAddr;
        mDNSIPPort senderPort;
        struct sockaddr_storage from;
        size_t fromlen = sizeof(from);
        char packetifname[IF_NAMESIZE] = "";
        mDNSu8 ttl;
        err = myrecvfrom(s1, &m->imsg, sizeof(m->imsg), (struct sockaddr *)&from, &fromlen, &destAddr, packetifname, &ttl);
        if (err < 0)
        {
            recvfrom_errno = errno;
            break;
        }

        if ((destAddr.type == mDNSAddrType_IPv4 && (destAddr.ip.v4.b[0] & 0xF0) == 0xE0) ||
            (destAddr.type == mDNSAddrType_IPv6 && (destAddr.ip.v6.b[0]         == 0xFF))) m->p->num_mcasts++;

        count++;
        if (from.ss_family == AF_INET)
        {
            struct sockaddr_in *s = (struct sockaddr_in*)&from;
            senderAddr.type = mDNSAddrType_IPv4;
            senderAddr.ip.v4.NotAnInteger = s->sin_addr.s_addr;
            senderPort.NotAnInteger = s->sin_port;
            //LogInfo("myKQSocketCallBack received IPv4 packet from %#-15a to %#-15a on skt %d %s", &senderAddr, &destAddr, s1, packetifname);
        }
        else if (from.ss_family == AF_INET6)
        {
            struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)&from;
            senderAddr.type = mDNSAddrType_IPv6;
            senderAddr.ip.v6 = *(mDNSv6Addr*)&sin6->sin6_addr;
            senderPort.NotAnInteger = sin6->sin6_port;
            //LogInfo("myKQSocketCallBack received IPv6 packet from %#-15a to %#-15a on skt %d %s", &senderAddr, &destAddr, s1, packetifname);
        }
        else
        {
            LogMsg("myKQSocketCallBack from is unknown address family %d", from.ss_family);
            return;
        }

        // Note: When handling multiple packets in a batch, MUST reset InterfaceID before handling each packet
        mDNSInterfaceID InterfaceID = mDNSNULL;
        NetworkInterfaceInfoOSX *intf = m->p->InterfaceList;
        while (intf) 
        {
            if (intf->Exists && !strcmp(intf->ifinfo.ifname, packetifname))
                break;
            intf = intf->next;
        }

        // When going to sleep we deregister all our interfaces, but if the machine
        // takes a few seconds to sleep we may continue to receive multicasts
        // during that time, which would confuse mDNSCoreReceive, because as far
        // as it's concerned, we should have no active interfaces any more.
        // Hence we ignore multicasts for which we can find no matching InterfaceID.
        if (intf)
            InterfaceID = intf->ifinfo.InterfaceID;
        else if (mDNSAddrIsDNSMulticast(&destAddr))
            continue;

        if (!InterfaceID)
        {
            InterfaceID = FindMyInterface(&destAddr);
        }

//      LogMsg("myKQSocketCallBack got packet from %#a to %#a on interface %#a/%s",
//          &senderAddr, &destAddr, &ss->info->ifinfo.ip, ss->info->ifinfo.ifname);

        // mDNSCoreReceive may close the socket we're reading from.  We must break out of our
        // loop when that happens, or we may try to read from an invalid FD.  We do this by
        // setting the closeFlag pointer in the socketset, so CloseSocketSet can inform us
        // if it closes the socketset.
        ss->closeFlag = &closed;

        if (ss->proxy)
        {
            m->p->UDPProxyCallback(&m->p->UDPProxy, &m->imsg.m, (unsigned char*)&m->imsg + err, &senderAddr,
                senderPort, &destAddr, ss->port, InterfaceID, NULL);
        }
        else
        {
            mDNSCoreReceive(m, &m->imsg.m, (unsigned char*)&m->imsg + err, &senderAddr, senderPort, &destAddr, ss->port, InterfaceID);
        }

        // if we didn't close, we can safely dereference the socketset, and should to
        // reset the closeFlag, since it points to something on the stack
        if (!closed) ss->closeFlag = mDNSNULL;
    }

    // If a client application's sockets are marked as defunct
    // sockets we have delegated to it with SO_DELEGATED will also go defunct.
    // We get an ENOTCONN error for defunct sockets and should just close the socket in that case.
    if (err < 0 && recvfrom_errno == ENOTCONN)
    {
        LogInfo("myKQSocketCallBack: ENOTCONN, closing socket");
        close(s1);
        return;
    }

    if (err < 0 && (recvfrom_errno != EWOULDBLOCK || count == 0))
    {
        // Something is busted here.
        // kqueue says there is a packet, but myrecvfrom says there is not.
        // Try calling select() to get another opinion.
        // Find out about other socket parameter that can help understand why select() says the socket is ready for read
        // All of this is racy, as data may have arrived after the call to select()
        static unsigned int numLogMessages = 0;
        int so_error = -1;
        int so_nread = -1;
        int fionread = -1;
        socklen_t solen;
        fd_set readfds;
        struct timeval timeout;
        int selectresult;
        FD_ZERO(&readfds);
        FD_SET(s1, &readfds);
        timeout.tv_sec  = 0;
        timeout.tv_usec = 0;
        selectresult = select(s1+1, &readfds, NULL, NULL, &timeout);
        solen = (socklen_t)sizeof(so_error);
        if (getsockopt(s1, SOL_SOCKET, SO_ERROR, &so_error, &solen) == -1)
            LogMsg("myKQSocketCallBack getsockopt(SO_ERROR) error %d", errno);
        solen = (socklen_t)sizeof(so_nread);
        if (getsockopt(s1, SOL_SOCKET, SO_NREAD, &so_nread, &solen) == -1)
            LogMsg("myKQSocketCallBack getsockopt(SO_NREAD) error %d", errno);
        if (ioctl(s1, FIONREAD, &fionread) == -1)
            LogMsg("myKQSocketCallBack ioctl(FIONREAD) error %d", errno);
        if (numLogMessages++ < 100)
            LogMsg("myKQSocketCallBack recvfrom skt %d error %d errno %d (%s) select %d (%spackets waiting) so_error %d so_nread %d fionread %d count %d",
                   s1, err, recvfrom_errno, strerror(recvfrom_errno), selectresult, FD_ISSET(s1, &readfds) ? "" : "*NO* ", so_error, so_nread, fionread, count);
        if (numLogMessages > 5)
            NotifyOfElusiveBug("Flaw in Kernel (select/recvfrom mismatch)",
                               "Congratulations, you've reproduced an elusive bug.\r"
                               "Please send email to radar-3387020@group.apple.com.)\r"
                               "If possible, please leave your machine undisturbed so that someone can come to investigate the problem.");

        sleep(1);       // After logging this error, rate limit so we don't flood syslog
    }
}

mDNSlocal void doTcpSocketCallback(TCPSocket *sock)
{
    mDNSBool c = !sock->connected;
    if (!sock->connected && sock->err == mStatus_NoError)
    {
        sock->connected = mDNStrue;
    }
    sock->callback(sock, sock->context, c, sock->err);
    // Note: the callback may call CloseConnection here, which frees the context structure!
}

#ifndef NO_SECURITYFRAMEWORK

mDNSlocal OSStatus tlsWriteSock(SSLConnectionRef connection, const void *data, size_t *dataLength)
{
    int ret = send(((TCPSocket *)connection)->fd, data, *dataLength, 0);
    if (ret >= 0 && (size_t)ret < *dataLength) { *dataLength = ret; return(errSSLWouldBlock); }
    if (ret >= 0)                              { *dataLength = ret; return(noErr); }
    *dataLength = 0;
    if (errno == EAGAIN                      ) return(errSSLWouldBlock);
    if (errno == ENOENT                      ) return(errSSLClosedGraceful);
    if (errno == EPIPE || errno == ECONNRESET) return(errSSLClosedAbort);
    LogMsg("ERROR: tlsWriteSock: %d error %d (%s)\n", ((TCPSocket *)connection)->fd, errno, strerror(errno));
    return(errSSLClosedAbort);
}

mDNSlocal OSStatus tlsReadSock(SSLConnectionRef connection, void *data, size_t *dataLength)
{
    int ret = recv(((TCPSocket *)connection)->fd, data, *dataLength, 0);
    if (ret > 0 && (size_t)ret < *dataLength) { *dataLength = ret; return(errSSLWouldBlock); }
    if (ret > 0)                              { *dataLength = ret; return(noErr); }
    *dataLength = 0;
    if (ret == 0 || errno == ENOENT    ) return(errSSLClosedGraceful);
    if (            errno == EAGAIN    ) return(errSSLWouldBlock);
    if (            errno == ECONNRESET) return(errSSLClosedAbort);
    LogMsg("ERROR: tlsSockRead: error %d (%s)\n", errno, strerror(errno));
    return(errSSLClosedAbort);
}

mDNSlocal OSStatus tlsSetupSock(TCPSocket *sock, SSLProtocolSide pside, SSLConnectionType ctype)
{
    char domname_cstr[MAX_ESCAPED_DOMAIN_NAME];

    sock->tlsContext = SSLCreateContext(kCFAllocatorDefault, pside, ctype);
    if (!sock->tlsContext) 
    { 
        LogMsg("ERROR: tlsSetupSock: SSLCreateContext failed"); 
        return(mStatus_UnknownErr); 
    }

    mStatus err = SSLSetIOFuncs(sock->tlsContext, tlsReadSock, tlsWriteSock);
    if (err) 
    { 
        LogMsg("ERROR: tlsSetupSock: SSLSetIOFuncs failed with error code: %d", err); 
        goto fail; 
    }

    err = SSLSetConnection(sock->tlsContext, (SSLConnectionRef) sock);
    if (err) 
    { 
        LogMsg("ERROR: tlsSetupSock: SSLSetConnection failed with error code: %d", err); 
        goto fail; 
    }

    // Set the default ciphersuite configuration
    err = SSLSetSessionConfig(sock->tlsContext, CFSTR("default"));
    if (err) 
    {
        LogMsg("ERROR: tlsSetupSock: SSLSetSessionConfig failed with error code: %d", err);
        goto fail;
    }

    // We already checked for NULL in hostname and this should never happen. Hence, returning -1
    // (error not in OSStatus space) is okay.
    if (!sock->hostname || !sock->hostname->c[0]) 
    {
        LogMsg("ERROR: tlsSetupSock: hostname NULL"); 
        err = -1;
        goto fail;
    }

    ConvertDomainNameToCString(sock->hostname, domname_cstr);
    err = SSLSetPeerDomainName(sock->tlsContext, domname_cstr, strlen(domname_cstr));
    if (err) 
    { 
        LogMsg("ERROR: tlsSetupSock: SSLSetPeerDomainname: %s failed with error code: %d", domname_cstr, err); 
        goto fail; 
    }

    return(err);

fail:
    if (sock->tlsContext)
        CFRelease(sock->tlsContext);
    return(err);
}

#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
mDNSlocal void doSSLHandshake(TCPSocket *sock)
{
    mStatus err = SSLHandshake(sock->tlsContext);

    //Can't have multiple threads in mDNS core. When MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM is
    //defined, KQueueLock is a noop. Hence we need to serialize here
    //
    //NOTE: We just can't serialize doTcpSocketCallback alone on the main queue.
    //We need the rest of the logic also. Otherwise, we can enable the READ
    //events below, dispatch a doTcpSocketCallback on the main queue. Assume it is
    //ConnFailed which means we are going to free the tcpInfo. While it
    //is waiting to be dispatched, another read event can come into tcpKQSocketCallback
    //and potentially call doTCPCallback with error which can close the fd and free the
    //tcpInfo. Later when the thread gets dispatched it will crash because the tcpInfo
    //is already freed.

    dispatch_async(dispatch_get_main_queue(), ^{

                       LogInfo("doSSLHandshake %p: got lock", sock); // Log *after* we get the lock

                       if (sock->handshake == handshake_to_be_closed)
                       {
                           LogInfo("SSLHandshake completed after close");
                           mDNSPlatformTCPCloseConnection(sock);
                       }
                       else
                       {
                           if (sock->fd != -1) KQueueSet(sock->fd, EV_ADD, EVFILT_READ, sock->kqEntry);
                           else LogMsg("doSSLHandshake: sock->fd is -1");

                           if (err == errSSLWouldBlock)
                               sock->handshake = handshake_required;
                           else
                           {
                               if (err)
                               {
                                   LogMsg("SSLHandshake failed: %d%s", err, err == errSSLPeerInternalError ? " (server busy)" : "");
                                   CFRelease(sock->tlsContext);
                                   sock->tlsContext = NULL;
                               }

                               sock->err = err ? mStatus_ConnFailed : 0;
                               sock->handshake = handshake_completed;

                               LogInfo("doSSLHandshake: %p calling doTcpSocketCallback fd %d", sock, sock->fd);
                               doTcpSocketCallback(sock);
                           }
                       }

                       LogInfo("SSLHandshake %p: dropping lock for fd %d", sock, sock->fd);
                       return;
                   });
}
#else // MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
mDNSlocal void *doSSLHandshake(TCPSocket *sock)
{
    // Warning: Touching sock without the kqueue lock!
    // We're protected because sock->handshake == handshake_in_progress
    mStatus err = SSLHandshake(sock->tlsContext);

    KQueueLock();
    debugf("doSSLHandshake %p: got lock", sock); // Log *after* we get the lock

    if (sock->handshake == handshake_to_be_closed)
    {
        LogInfo("SSLHandshake completed after close");
        mDNSPlatformTCPCloseConnection(sock);
    }
    else
    {
        if (sock->fd != -1) KQueueSet(sock->fd, EV_ADD, EVFILT_READ, &sock->kqEntry);
        else LogMsg("doSSLHandshake: sock->fd is -1");

        if (err == errSSLWouldBlock)
            sock->handshake = handshake_required;
        else
        {
            if (err)
            {
                LogMsg("SSLHandshake failed: %d%s", err, err == errSSLPeerInternalError ? " (server busy)" : "");
                CFRelease(sock->tlsContext);
                sock->tlsContext = NULL;
            }

            sock->err = err ? mStatus_ConnFailed : 0;
            sock->handshake = handshake_completed;

            debugf("doSSLHandshake: %p calling doTcpSocketCallback fd %d", sock, sock->fd);
            doTcpSocketCallback(sock);
        }
    }

    debugf("SSLHandshake %p: dropping lock for fd %d", sock, sock->fd);
    KQueueUnlock("doSSLHandshake");
    return NULL;
}
#endif // MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM

mDNSlocal void spawnSSLHandshake(TCPSocket* sock)
{
    debugf("spawnSSLHandshake %p: entry", sock);

    if (sock->handshake != handshake_required) LogMsg("spawnSSLHandshake: handshake status not required: %d", sock->handshake);
    sock->handshake = handshake_in_progress;
    KQueueSet(sock->fd, EV_DELETE, EVFILT_READ, &sock->kqEntry);

    // Dispatch it on a separate queue to help avoid blocking other threads/queues, and
    // to limit the number of threads used for SSLHandshake
    dispatch_async(SSLqueue, ^{doSSLHandshake(sock);});

    debugf("spawnSSLHandshake %p: done for %d", sock, sock->fd);
}

#endif /* NO_SECURITYFRAMEWORK */

mDNSlocal void tcpKQSocketCallback(__unused int fd, short filter, void *context, __unused mDNSBool encounteredEOF)
{
    TCPSocket *sock = context;
    sock->err = mStatus_NoError;

    //if (filter == EVFILT_READ ) LogMsg("myKQSocketCallBack: tcpKQSocketCallback %d is EVFILT_READ", filter);
    //if (filter == EVFILT_WRITE) LogMsg("myKQSocketCallBack: tcpKQSocketCallback %d is EVFILT_WRITE", filter);
    // EV_ONESHOT doesn't seem to work, so we add the filter with EV_ADD, and explicitly delete it here with EV_DELETE
    if (filter == EVFILT_WRITE) 
    {
        // sock->connected gets set by doTcpSocketCallback(), which may be called from here, or may be called
        // from the TLS connect code.   If we asked for a writability test, we are connecting
        // (sock->connected == mDNSFalse).
        if (sock->connected)
        {
            LogInfo("ERROR: TCPConnectCallback called with write event when socket is connected.");
        }
        else
        {
            int result = 0;
            socklen_t len = (socklen_t)sizeof(result);
            if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &result, &len) < 0)
            {
                LogInfo("ERROR: TCPConnectCallback - unable to get connect error: socket %d: Error %d (%s)",
                        sock->fd, errno, strerror(errno));
                sock->err = mStatus_ConnFailed;
            }
            else
            {
                if (result != 0)
                {
                    sock->err = mStatus_ConnFailed;
                    if (result == EHOSTUNREACH || result == EADDRNOTAVAIL || result == ENETDOWN)
                    {
                        LogInfo("ERROR: TCPConnectCallback - connect failed: socket %d: Error %d (%s)",
                                sock->fd, result, strerror(result));
                    }
                    else
                    {
                        LogMsg("ERROR: TCPConnectCallback - connect failed: socket %d: Error %d (%s)",
                               sock->fd, result, strerror(result));
                    }
                }
            }
        }
        KQueueSet(sock->fd, EV_DELETE, EVFILT_WRITE, &sock->kqEntry);

        // If we set the EVFILT_READ event in mDNSPlatformTCPConnect, it's possible to get a read event
        // before the write event--apparently the socket is both readable and writable once that happens,
        // even if the connect fails.   If we set it here, after we've gotten a successful connection, then
        // we shouldn't run into that problem.
        if (sock->err == mStatus_NoError &&
            KQueueSet(sock->fd, EV_ADD, EVFILT_READ, &sock->kqEntry))
        {
            // And of course if that fails, we can't use the connection even though we have it.
            LogMsg("ERROR: tcpKQSocketCallback - KQueueSet failed");
            sock->err = mStatus_TransientErr;
        }
    }
    
    if (sock->flags & kTCPSocketFlags_UseTLS)
    {
#ifndef NO_SECURITYFRAMEWORK
        // Don't try to set up TLS if the connect failed.
        if (sock->err == mStatus_NoError) {
            if (!sock->setup) 
            {
                sock->setup = mDNStrue; 
                sock->err = tlsSetupSock(sock, kSSLClientSide, kSSLStreamType);
                if (sock->err)
                {
                    LogMsg("ERROR: tcpKQSocketCallback: tlsSetupSock failed with error code: %d", sock->err);
                    return;
                }
            }
            if (sock->handshake == handshake_required) 
            { 
                spawnSSLHandshake(sock); 
                return;
            }
            else if (sock->handshake == handshake_in_progress || sock->handshake == handshake_to_be_closed)
            {
                return;
            }
            else if (sock->handshake != handshake_completed)
            {
                if (!sock->err) 
                    sock->err = mStatus_UnknownErr;
                LogMsg("tcpKQSocketCallback called with unexpected SSLHandshake status: %d", sock->handshake);
            }
        }
#else  /* NO_SECURITYFRAMEWORK */ 
        sock->err = mStatus_UnsupportedErr;
#endif /* NO_SECURITYFRAMEWORK */
    }

    doTcpSocketCallback(sock);
}

#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
mDNSexport int KQueueSet(int fd, u_short flags, short filter, KQueueEntry *const entryRef)
{
    dispatch_queue_t queue = dispatch_get_main_queue();
    dispatch_source_t source;
    if (flags == EV_DELETE)
    {
        if (filter == EVFILT_READ)
        {
            dispatch_source_cancel(entryRef->readSource);
            dispatch_release(entryRef->readSource);
            entryRef->readSource = mDNSNULL;
            debugf("KQueueSet: source cancel for read %p, %p", entryRef->readSource, entryRef->writeSource);
        }
        else if (filter == EVFILT_WRITE)
        {
            dispatch_source_cancel(entryRef->writeSource);
            dispatch_release(entryRef->writeSource);
            entryRef->writeSource = mDNSNULL;
            debugf("KQueueSet: source cancel for write %p, %p", entryRef->readSource, entryRef->writeSource);
        }
        else
            LogMsg("KQueueSet: ERROR: Wrong filter value %d for EV_DELETE", filter);
        return 0;
    }
    if (flags != EV_ADD) LogMsg("KQueueSet: Invalid flags %d", flags);

    if (filter == EVFILT_READ)
    {
        source = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, fd, 0, queue);
    }
    else if (filter == EVFILT_WRITE)
    {
        source = dispatch_source_create(DISPATCH_SOURCE_TYPE_WRITE, fd, 0, queue);
    }
    else
    {
        LogMsg("KQueueSet: ERROR: Wrong filter value %d for EV_ADD", filter);
        return -1;
    }
    if (!source) return -1;
    dispatch_source_set_event_handler(source, ^{

                                          mDNSs32 stime = mDNSPlatformRawTime();
                                          entryRef->KQcallback(fd, filter, entryRef->KQcontext);
                                          mDNSs32 etime = mDNSPlatformRawTime();
                                          if (etime - stime >= WatchDogReportingThreshold)
                                              LogInfo("KQEntryCallback Block: WARNING: took %dms to complete", etime - stime);

                                          // Trigger the event delivery to the application. Even though we trigger the
                                          // event completion after handling every event source, these all will hopefully
                                          // get merged
                                          TriggerEventCompletion();

                                      });
    dispatch_source_set_cancel_handler(source, ^{
                                           if (entryRef->fdClosed)
                                           {
                                               //LogMsg("CancelHandler: closing fd %d", fd);
                                               close(fd);
                                           }
                                       });
    dispatch_resume(source);
    if (filter == EVFILT_READ)
        entryRef->readSource = source;
    else
        entryRef->writeSource = source;

    return 0;
}

mDNSexport void KQueueLock()
{
}
mDNSexport void KQueueUnlock(const char const *task)
{
    (void)task; //unused
}
#else

mDNSexport int KQueueSet(int fd, u_short flags, short filter, const KQueueEntry *const entryRef)
{
    struct kevent new_event;
    EV_SET(&new_event, fd, filter, flags, 0, 0, (void*)entryRef);
    return (kevent(KQueueFD, &new_event, 1, NULL, 0, NULL) < 0) ? errno : 0;
}

mDNSexport void KQueueLock()
{
    mDNS *const m = &mDNSStorage;
    pthread_mutex_lock(&m->p->BigMutex);
    m->p->BigMutexStartTime = mDNSPlatformRawTime();
}

mDNSexport void KQueueUnlock(const char* task)
{
    mDNS *const m = &mDNSStorage;
    mDNSs32 end = mDNSPlatformRawTime();
    (void)task;
    if (end - m->p->BigMutexStartTime >= WatchDogReportingThreshold)
    {
        LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_WARNING,
            "WARNING: " PUB_S " took %d ms to complete", task, end - m->p->BigMutexStartTime);
    }

    pthread_mutex_unlock(&m->p->BigMutex);

    char wake = 1;
    if (send(m->p->WakeKQueueLoopFD, &wake, sizeof(wake), 0) == -1)
        LogMsg("ERROR: KQueueWake: send failed with error code: %d (%s)", errno, strerror(errno));
}
#endif

mDNSexport void mDNSPlatformCloseFD(KQueueEntry *kq, int fd)
{
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
        (void) fd; //unused
    if (kq->readSource)
    {
        dispatch_source_cancel(kq->readSource);
        kq->readSource = mDNSNULL;
    }
    if (kq->writeSource)
    {
        dispatch_source_cancel(kq->writeSource);
        kq->writeSource = mDNSNULL;
    }
    // Close happens in the cancellation handler
    debugf("mDNSPlatformCloseFD: resetting sources for %d", fd);
    kq->fdClosed = mDNStrue;
#else
    (void)kq; //unused
    close(fd);
#endif
}

mDNSlocal mStatus SetupTCPSocket(TCPSocket *sock, mDNSAddr_Type addrtype, mDNSIPPort *port, mDNSBool useBackgroundTrafficClass)
{
    int skt;

    skt = -1;
    if (!mDNSPosixTCPSocketSetup(&skt, addrtype, port, &sock->port))
    {
        if (skt != -1) close(skt);
        return mStatus_UnknownErr;
    }
            
    // for TCP sockets, the traffic class is set once and not changed
    setTrafficClass(skt, useBackgroundTrafficClass);

    sock->fd = skt;
    sock->kqEntry.KQcallback = tcpKQSocketCallback;
    sock->kqEntry.KQcontext  = sock;
    sock->kqEntry.KQtask     = "mDNSPlatformTCPSocket";
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
    sock->kqEntry.readSource = mDNSNULL;
    sock->kqEntry.writeSource = mDNSNULL;
    sock->kqEntry.fdClosed = mDNSfalse;
#endif
    return mStatus_NoError;
}

mDNSexport TCPSocket *mDNSPlatformTCPSocket(TCPSocketFlags flags, mDNSAddr_Type addrtype, mDNSIPPort *port, domainname *hostname, mDNSBool useBackgroundTrafficClass)
{
    mStatus err;
    mDNSu32 lowWater = 16384;
    size_t len = sizeof (TCPSocket);
    if (hostname) {
        len += sizeof (domainname);
    }

    TCPSocket *sock = (TCPSocket *) callocL("TCPSocket/mDNSPlatformTCPSocket", len);
    if (!sock) { LogMsg("mDNSPlatformTCPSocket: memory allocation failure"); return(mDNSNULL); }

    if (hostname)
    {
        sock->hostname = (domainname *)(sock + 1); // Allocated together so can be freed together
        debugf("mDNSPlatformTCPSocket: hostname %##s", hostname->c);
        AssignDomainName(sock->hostname, hostname);
    }

    err = SetupTCPSocket(sock, addrtype, port, useBackgroundTrafficClass);

    if (err)
    {
        LogMsg("mDNSPlatformTCPSocket: socket error %d errno %d (%s)", sock->fd, errno, strerror(errno));
        freeL("TCPSocket/mDNSPlatformTCPSocket", sock);
        return(mDNSNULL);
    }

    if (setsockopt(sock->fd, IPPROTO_TCP, TCP_NOTSENT_LOWAT, &lowWater, sizeof lowWater) < 0)
    {
        LogMsg("mDNSPlatformTCPSocket: TCP_NOTSENT_LOWAT returned %d", errno);
               mDNSPlatformTCPCloseConnection(sock);
        return mDNSNULL;
    }
    
    sock->callback          = mDNSNULL;
    sock->flags             = flags;
    sock->context           = mDNSNULL;
    sock->setup             = mDNSfalse;
    sock->connected         = mDNSfalse;
    sock->handshake         = handshake_required;
    sock->m                 =  &mDNSStorage;
    sock->err               = mStatus_NoError;

    return sock;
}

mDNSexport mStatus mDNSPlatformTCPConnect(TCPSocket *sock, const mDNSAddr *dst, mDNSOpaque16 dstport, mDNSInterfaceID InterfaceID, TCPConnectionCallback callback, void *context)
{
    mStatus err = mStatus_NoError;
    struct sockaddr_storage ss;

    sock->callback          = callback;
    sock->context           = context;
    sock->setup             = mDNSfalse;
    sock->connected         = mDNSfalse;
    sock->handshake         = handshake_required;
    sock->err               = mStatus_NoError;

    if (dst->type == mDNSAddrType_IPv4)
    {
        struct sockaddr_in *saddr = (struct sockaddr_in *)&ss;
        mDNSPlatformMemZero(saddr, sizeof(*saddr));
        saddr->sin_family      = AF_INET;
        saddr->sin_port        = dstport.NotAnInteger;
        saddr->sin_len         = sizeof(*saddr);
        saddr->sin_addr.s_addr = dst->ip.v4.NotAnInteger;
    }
    else
    {
        struct sockaddr_in6 *saddr6 = (struct sockaddr_in6 *)&ss;
        mDNSPlatformMemZero(saddr6, sizeof(*saddr6));
        saddr6->sin6_family      = AF_INET6;
        saddr6->sin6_port        = dstport.NotAnInteger;
        saddr6->sin6_len         = sizeof(*saddr6);
        saddr6->sin6_addr        = *(struct in6_addr *)&dst->ip.v6;
    }

    // Watch for connect complete (write is ready)
    // EV_ONESHOT doesn't seem to work, so we add the filter with EV_ADD, and explicitly delete it in tcpKQSocketCallback using EV_DELETE
    if (KQueueSet(sock->fd, EV_ADD /* | EV_ONESHOT */, EVFILT_WRITE, &sock->kqEntry))
    {
        LogMsg("ERROR: mDNSPlatformTCPConnect - KQueueSet failed");
        return errno;
    }

    if (fcntl(sock->fd, F_SETFL, fcntl(sock->fd, F_GETFL, 0) | O_NONBLOCK) < 0) // set non-blocking
    {
        LogMsg("ERROR: setsockopt O_NONBLOCK - %s", strerror(errno));
        return mStatus_UnknownErr;
    }

    // We bind to the interface and all subsequent packets including the SYN will be sent out
    // on this interface
    //
    // Note: If we are in Active Directory domain, we may try TCP (if the response can't fit in
    // UDP).
    if (InterfaceID)
    {
        NetworkInterfaceInfoOSX *info = IfindexToInterfaceInfoOSX(InterfaceID);
        if (dst->type == mDNSAddrType_IPv4)
        {
        #ifdef IP_BOUND_IF
            if (info) setsockopt(sock->fd, IPPROTO_IP, IP_BOUND_IF, &info->scope_id, sizeof(info->scope_id));
            else { LogMsg("mDNSPlatformTCPConnect: Invalid interface index %p", InterfaceID); return mStatus_BadParamErr; }
        #else
            (void)InterfaceID; // Unused
            (void)info; // Unused
        #endif
        }
        else
        {
        #ifdef IPV6_BOUND_IF
            if (info) setsockopt(sock->fd, IPPROTO_IPV6, IPV6_BOUND_IF, &info->scope_id, sizeof(info->scope_id));
            else { LogMsg("mDNSPlatformTCPConnect: Invalid interface index %p", InterfaceID); return mStatus_BadParamErr; }
        #else
            (void)InterfaceID; // Unused
            (void)info; // Unused
        #endif
        }
    }

    // mDNSPlatformReadTCP/WriteTCP (unlike the UDP counterpart) does not provide the destination address
    // from which we can infer the destination address family. Hence we need to remember that here.
    // Instead of remembering the address family, we remember the right fd.
    sock->fd = sock->fd;
    // initiate connection wth peer
    if (connect(sock->fd, (struct sockaddr *)&ss, ss.ss_len) < 0)
    {
        if (errno == EINPROGRESS) return mStatus_ConnPending;
        if (errno == EHOSTUNREACH || errno == EADDRNOTAVAIL || errno == ENETDOWN)
            LogInfo("ERROR: mDNSPlatformTCPConnect - connect failed: socket %d: Error %d (%s)", sock->fd, errno, strerror(errno));
        else
            LogMsg("ERROR: mDNSPlatformTCPConnect - connect failed: socket %d: Error %d (%s) length %d", sock->fd, errno, strerror(errno), ss.ss_len);
        return mStatus_ConnFailed;
    }

    LogMsg("NOTE: mDNSPlatformTCPConnect completed synchronously");
    // kQueue should notify us, but this LogMsg is to help track down if it doesn't
    // Experimentation shows that even a connection to a local listener returns EINPROGRESS, so this
    // will likely never happen.

    return err;
}

// Replace the existing socket callback with a new one, or establish a callback where none was present.
mDNSexport mStatus mDNSPlatformTCPSocketSetCallback(TCPSocket *sock, TCPConnectionCallback callback, void *context)
{
    sock->callback = callback;
    sock->context = context;

    // dnsextd currently reaches into the TCPSocket structure layer to do its own thing; this won't work for
    // any code (e.g., the Discovery Proxy or Discovery Relay) that actually uses the mDNSPlatform layer as
    // an opaque layer.   So for that code, we have this.   dnsextd should probably be platformized if it's
    // still relevant.
    if (!sock->callback) {
        // Watch for incoming data
        if (KQueueSet(sock->fd, EV_ADD, EVFILT_READ, &sock->kqEntry))
        {
            LogMsg("ERROR: mDNSPlatformTCPConnect - KQueueSet failed");
            return mStatus_UnknownErr;
        }
    }
    
    sock->kqEntry.KQcallback = tcpKQSocketCallback;
    sock->kqEntry.KQcontext  = sock;
    sock->kqEntry.KQtask     = "mDNSPlatformTCPSocket";
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
    sock->kqEntry.readSource = mDNSNULL;
    sock->kqEntry.writeSource = mDNSNULL;
    sock->kqEntry.fdClosed = mDNSfalse;
#endif
    return mStatus_NoError;
}

// Why doesn't mDNSPlatformTCPAccept actually call accept() ?
// mDNSPlatformTCPAccept is only called by dnsextd.c.   It's called _after_ accept has returned
// a connected socket.   The purpose appears to be to allocate and initialize the TCPSocket structure
// and set up TLS, if required for this connection.   dnsextd appears to be the only thing in mDNSResponder
// that accepts incoming TLS connections.
mDNSexport TCPSocket *mDNSPlatformTCPAccept(TCPSocketFlags flags, int fd)
{
    mStatus err = mStatus_NoError;

    TCPSocket *sock = (TCPSocket *) callocL("TCPSocket/mDNSPlatformTCPAccept", sizeof(*sock));
    if (!sock) return(mDNSNULL);

    sock->fd = fd;
    sock->flags = flags;

    if (flags & kTCPSocketFlags_UseTLS)
    {
#ifndef NO_SECURITYFRAMEWORK
        if (!ServerCerts) { LogMsg("ERROR: mDNSPlatformTCPAccept: unable to find TLS certificates"); err = mStatus_UnknownErr; goto exit; }

        err = tlsSetupSock(sock, kSSLServerSide, kSSLStreamType);
        if (err) { LogMsg("ERROR: mDNSPlatformTCPAccept: tlsSetupSock failed with error code: %d", err); goto exit; }

        err = SSLSetCertificate(sock->tlsContext, ServerCerts);
        if (err) { LogMsg("ERROR: mDNSPlatformTCPAccept: SSLSetCertificate failed with error code: %d", err); goto exit; }
#else
        err = mStatus_UnsupportedErr;
#endif /* NO_SECURITYFRAMEWORK */
    }
#ifndef NO_SECURITYFRAMEWORK
exit:
#endif

    if (err) { freeL("TCPSocket/mDNSPlatformTCPAccept", sock); return(mDNSNULL); }
    return(sock);
}

mDNSlocal void tcpListenCallback(int fd, __unused short filter, void *context, __unused mDNSBool encounteredEOF)
{
    TCPListener *listener = context;
    TCPSocket *sock;
    
    sock = mDNSPosixDoTCPListenCallback(fd, listener->addressType, listener->socketFlags,
                                 listener->callback, listener->context);
    
    if (sock != mDNSNULL)
    {
        KQueueSet(sock->fd, EV_ADD, EVFILT_READ, &sock->kqEntry);
 
        sock->kqEntry.KQcallback = tcpKQSocketCallback;
        sock->kqEntry.KQcontext  = sock;
        sock->kqEntry.KQtask     = "mDNSPlatformTCPListen";
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
        sock->kqEntry.readSource = mDNSNULL;
        sock->kqEntry.writeSource = mDNSNULL;
        sock->kqEntry.fdClosed = mDNSfalse;
#endif
    }
}

mDNSexport TCPListener *mDNSPlatformTCPListen(mDNSAddr_Type addrtype, mDNSIPPort *port, mDNSAddr *addr,
                                              TCPSocketFlags socketFlags, mDNSBool reuseAddr, int queueLength,
                                              TCPAcceptedCallback callback, void *context)
{
    TCPListener *ret;
    int fd = -1;

    if (!mDNSPosixTCPListen(&fd, addrtype, port, addr, reuseAddr, queueLength)) {
        if (fd != -1) {
            close(fd);
        }
        return mDNSNULL;
    }
    
    // Allocate a listener structure
    ret = (TCPListener *) mDNSPlatformMemAllocateClear(sizeof *ret);
    if (ret == mDNSNULL)
    {
        LogMsg("mDNSPlatformTCPListen: no memory for TCPListener struct.");
        close(fd);
        return mDNSNULL;
    }
    ret->fd = fd;
    ret->callback = callback;
    ret->context = context;
    ret->socketFlags = socketFlags;

    // Watch for incoming data
    KQueueSet(ret->fd, EV_ADD, EVFILT_READ, &ret->kqEntry);
    ret->kqEntry.KQcallback = tcpListenCallback;
    ret->kqEntry.KQcontext  = ret;
    ret->kqEntry.KQtask     = "mDNSPlatformTCPListen";
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
    ret->kqEntry.readSource = mDNSNULL;
    ret->kqEntry.writeSource = mDNSNULL;
    ret->kqEntry.fdClosed = mDNSfalse;
#endif
    return ret;
}

mDNSexport mDNSu16 mDNSPlatformGetUDPPort(UDPSocket *sock)
{
    mDNSu16 port;

    port = -1;
    if (sock)
    {
        port = sock->ss.port.NotAnInteger;
    }
    return port;
}

mDNSlocal void CloseSocketSet(KQSocketSet *ss)
{
    if (ss->sktv4 != -1)
    {
        mDNSPlatformCloseFD(&ss->kqsv4,  ss->sktv4);
        ss->sktv4 = -1;
    }
    if (ss->sktv6 != -1)
    {
        mDNSPlatformCloseFD(&ss->kqsv6,  ss->sktv6);
        ss->sktv6 = -1;
    }
    if (ss->closeFlag) *ss->closeFlag = 1;
}

mDNSexport void mDNSPlatformTCPCloseConnection(TCPSocket *sock)
{
    if (sock)
    {
#ifndef NO_SECURITYFRAMEWORK
        if (sock->tlsContext)
        {
            if (sock->handshake == handshake_in_progress) // SSLHandshake thread using this sock (esp. tlsContext)
            {
                LogInfo("mDNSPlatformTCPCloseConnection: called while handshake in progress");
                // When we come back from SSLHandshake, we will notice that a close was here and
                // call this function again which will do the cleanup then.
                sock->handshake = handshake_to_be_closed;
                return;
            }

            SSLClose(sock->tlsContext);
            CFRelease(sock->tlsContext);
            sock->tlsContext = NULL;
        }
#endif /* NO_SECURITYFRAMEWORK */
        if (sock->fd != -1) {
            shutdown(sock->fd, 2);
            mDNSPlatformCloseFD(&sock->kqEntry, sock->fd);
            sock->fd = -1;
        }
        
        freeL("TCPSocket/mDNSPlatformTCPCloseConnection", sock);
    }
}

mDNSexport long mDNSPlatformReadTCP(TCPSocket *sock, void *buf, unsigned long buflen, mDNSBool *closed)
{
    ssize_t nread = 0;
    *closed = mDNSfalse;

    // We can get here if the caller set up a TCP connection but didn't check the status when it got the
    // callback.
    if (!sock->connected) {
        return mStatus_DefunctConnection;
    }
    
    if (sock->flags & kTCPSocketFlags_UseTLS)
    {
#ifndef NO_SECURITYFRAMEWORK
        if (sock->handshake == handshake_required) { LogMsg("mDNSPlatformReadTCP called while handshake required"); return 0; }
        else if (sock->handshake == handshake_in_progress) return 0;
        else if (sock->handshake != handshake_completed) LogMsg("mDNSPlatformReadTCP called with unexpected SSLHandshake status: %d", sock->handshake);

        //LogMsg("Starting SSLRead %d %X", sock->fd, fcntl(sock->fd, F_GETFL, 0));
        mStatus err = SSLRead(sock->tlsContext, buf, buflen, (size_t *)&nread);
        //LogMsg("SSLRead returned %d (%d) nread %d buflen %d", err, errSSLWouldBlock, nread, buflen);
        if (err == errSSLClosedGraceful) { nread = 0; *closed = mDNStrue; }
        else if (err && err != errSSLWouldBlock)
        { LogMsg("ERROR: mDNSPlatformReadTCP - SSLRead: %d", err); nread = -1; *closed = mDNStrue; }
#else
        nread = -1;
        *closed = mDNStrue;
#endif /* NO_SECURITYFRAMEWORK */
    }
    else
    {
        nread = mDNSPosixReadTCP(sock->fd, buf, buflen, closed);
    }

    return nread;
}

mDNSexport long mDNSPlatformWriteTCP(TCPSocket *sock, const char *msg, unsigned long len)
{
    int nsent;

    if (!sock->connected) {
        return mStatus_DefunctConnection;
    }

    if (sock->flags & kTCPSocketFlags_UseTLS)
    {
#ifndef NO_SECURITYFRAMEWORK
        size_t processed;
        if (sock->handshake == handshake_required) { LogMsg("mDNSPlatformWriteTCP called while handshake required"); return 0; }
        if (sock->handshake == handshake_in_progress) return 0;
        else if (sock->handshake != handshake_completed) LogMsg("mDNSPlatformWriteTCP called with unexpected SSLHandshake status: %d", sock->handshake);

        mStatus err = SSLWrite(sock->tlsContext, msg, len, &processed);

        if (!err) nsent = (int) processed;
        else if (err == errSSLWouldBlock) nsent = 0;
        else { LogMsg("ERROR: mDNSPlatformWriteTCP - SSLWrite returned %d", err); nsent = -1; }
#else
        nsent = -1;
#endif /* NO_SECURITYFRAMEWORK */
    }
    else
    {
        nsent = mDNSPosixWriteTCP(sock->fd, msg, len);
    }
    return nsent;
}

mDNSexport int mDNSPlatformTCPGetFD(TCPSocket *sock)
{
    return sock->fd;
}

// This function checks to see if the socket is writable.   It will be writable if the kernel TCP output
// buffer is less full than TCP_NOTSENT_LOWAT.   This should be half or less of the actual kernel buffer
// size.   This check is done in cases where data should be written if there's space, for example in the
// Discovery Relay code, where we may be receiving mDNS messages at arbitrary times, and generally there
// should be buffer space to relay them, but in exceptional cases there might not be.   In this case it's

mDNSexport mDNSBool mDNSPlatformTCPWritable(TCPSocket *sock)
{
    int kfd = kqueue();
    struct kevent kin, kout;
    int count;
    struct timespec ts;
    
    if (kfd < 0)
    {
        LogMsg("ERROR: kqueue failed: %m");
        return mDNSfalse;
    }
    ts.tv_sec = 0;
    ts.tv_nsec = 0;
    EV_SET(&kin, sock->fd, EVFILT_WRITE, EV_ADD, 0, 0, 0);
    count = kevent(kfd, &kin, 1, &kout, 1, &ts);
    close(kfd);
    if (count == 1 && (int)kout.ident == sock->fd && kout.filter == EVFILT_WRITE)
    {
        return mDNStrue;
    }
    return mDNSfalse;
}

// If mDNSIPPort port is non-zero, then it's a multicast socket on the specified interface
// If mDNSIPPort port is zero, then it's a randomly assigned port number, used for sending unicast queries
mDNSlocal mStatus SetupSocket(KQSocketSet *cp, const mDNSIPPort port, u_short sa_family, mDNSIPPort *const outport)
{
    int         *s        = (sa_family == AF_INET) ? &cp->sktv4 : &cp->sktv6;
    KQueueEntry *k        = (sa_family == AF_INET) ? &cp->kqsv4 : &cp->kqsv6;
    const int on = 1;
    const int twofivefive = 255;
    mStatus err = mStatus_NoError;
    char *errstr = mDNSNULL;
    const int mtu = 0;
    int saved_errno;

    cp->closeFlag = mDNSNULL;

    int skt = socket(sa_family, SOCK_DGRAM, IPPROTO_UDP);
    if (skt < 3) { if (errno != EAFNOSUPPORT) LogMsg("SetupSocket: socket error %d errno %d (%s)", skt, errno, strerror(errno));return(skt); }

    // set default traffic class
    setTrafficClass(skt, mDNSfalse);

#ifdef SO_RECV_ANYIF
    // Enable inbound packets on IFEF_AWDL interface.
    // Only done for multicast sockets, since we don't expect unicast socket operations
    // on the IFEF_AWDL interface. Operation is a no-op for other interface types.
    if (mDNSSameIPPort(port, MulticastDNSPort)) 
    {
        err = setsockopt(skt, SOL_SOCKET, SO_RECV_ANYIF, &on, sizeof(on));
        if (err < 0) { errstr = "setsockopt - SO_RECV_ANYIF"; goto fail; }
    }
#endif // SO_RECV_ANYIF

    // ... with a shared UDP port, if it's for multicast receiving
    if (mDNSSameIPPort(port, MulticastDNSPort) || mDNSSameIPPort(port, NATPMPAnnouncementPort))
    {
        err = setsockopt(skt, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
        if (err < 0) { errstr = "setsockopt - SO_REUSEPORT"; goto fail; }
    }

    // Don't want to wake from sleep for inbound packets on the mDNS sockets
    if (mDNSSameIPPort(port, MulticastDNSPort)) 
    {
        int nowake = 1;
        if (setsockopt(skt, SOL_SOCKET, SO_NOWAKEFROMSLEEP, &nowake, sizeof(nowake)) == -1)
            LogInfo("SetupSocket: SO_NOWAKEFROMSLEEP failed %s", strerror(errno));
    }

    if (sa_family == AF_INET)
    {
        // We want to receive destination addresses
        err = setsockopt(skt, IPPROTO_IP, IP_RECVDSTADDR, &on, sizeof(on));
        if (err < 0) { errstr = "setsockopt - IP_RECVDSTADDR"; goto fail; }

        // We want to receive interface identifiers
        err = setsockopt(skt, IPPROTO_IP, IP_RECVIF, &on, sizeof(on));
        if (err < 0) { errstr = "setsockopt - IP_RECVIF"; goto fail; }

        // We want to receive packet TTL value so we can check it
        err = setsockopt(skt, IPPROTO_IP, IP_RECVTTL, &on, sizeof(on));
        if (err < 0) { errstr = "setsockopt - IP_RECVTTL"; goto fail; }

        // Send unicast packets with TTL 255
        err = setsockopt(skt, IPPROTO_IP, IP_TTL, &twofivefive, sizeof(twofivefive));
        if (err < 0) { errstr = "setsockopt - IP_TTL"; goto fail; }

        // And multicast packets with TTL 255 too
        err = setsockopt(skt, IPPROTO_IP, IP_MULTICAST_TTL, &twofivefive, sizeof(twofivefive));
        if (err < 0) { errstr = "setsockopt - IP_MULTICAST_TTL"; goto fail; }

        // And start listening for packets
        struct sockaddr_in listening_sockaddr;
        listening_sockaddr.sin_family      = AF_INET;
        listening_sockaddr.sin_port        = port.NotAnInteger;     // Pass in opaque ID without any byte swapping
        listening_sockaddr.sin_addr.s_addr = mDNSSameIPPort(port, NATPMPAnnouncementPort) ? AllHosts_v4.NotAnInteger : 0;
        err = bind(skt, (struct sockaddr *) &listening_sockaddr, sizeof(listening_sockaddr));
        if (err) { errstr = "bind"; goto fail; }
        if (outport) outport->NotAnInteger = listening_sockaddr.sin_port;
    }
    else if (sa_family == AF_INET6)
    {
        // NAT-PMP Announcements make no sense on IPv6, and we don't support IPv6 for PCP, so bail early w/o error
        if (mDNSSameIPPort(port, NATPMPAnnouncementPort)) { if (outport) *outport = zeroIPPort; close(skt); return mStatus_NoError; }

        // We want to receive destination addresses and receive interface identifiers
        err = setsockopt(skt, IPPROTO_IPV6, IPV6_RECVPKTINFO, &on, sizeof(on));
        if (err < 0) { errstr = "setsockopt - IPV6_RECVPKTINFO"; goto fail; }

        // We want to receive packet hop count value so we can check it
        err = setsockopt(skt, IPPROTO_IPV6, IPV6_RECVHOPLIMIT, &on, sizeof(on));
        if (err < 0) { errstr = "setsockopt - IPV6_RECVHOPLIMIT"; goto fail; }

        // We want to receive only IPv6 packets. Without this option we get IPv4 packets too,
        // with mapped addresses of the form 0:0:0:0:0:FFFF:xxxx:xxxx, where xxxx:xxxx is the IPv4 address
        err = setsockopt(skt, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on));
        if (err < 0) { errstr = "setsockopt - IPV6_V6ONLY"; goto fail; }

        // Send unicast packets with TTL 255
        err = setsockopt(skt, IPPROTO_IPV6, IPV6_UNICAST_HOPS, &twofivefive, sizeof(twofivefive));
        if (err < 0) { errstr = "setsockopt - IPV6_UNICAST_HOPS"; goto fail; }

        // And multicast packets with TTL 255 too
        err = setsockopt(skt, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &twofivefive, sizeof(twofivefive));
        if (err < 0) { errstr = "setsockopt - IPV6_MULTICAST_HOPS"; goto fail; }

        // Want to receive our own packets
        err = setsockopt(skt, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &on, sizeof(on));
        if (err < 0) { errstr = "setsockopt - IPV6_MULTICAST_LOOP"; goto fail; }

        // Disable default option to send mDNSv6 packets at min IPv6 MTU: RFC 3542, Sec 11
        err = setsockopt(skt, IPPROTO_IPV6, IPV6_USE_MIN_MTU, &mtu, sizeof(mtu));
        if (err < 0) // Since it is an optimization if we fail just log the err, no need to close the skt
            LogMsg("SetupSocket: setsockopt - IPV6_USE_MIN_MTU: IP6PO_MINMTU_DISABLE socket %d err %d errno %d (%s)", 
                    skt, err, errno, strerror(errno));
        
        // And start listening for packets
        struct sockaddr_in6 listening_sockaddr6;
        mDNSPlatformMemZero(&listening_sockaddr6, sizeof(listening_sockaddr6));
        listening_sockaddr6.sin6_len         = sizeof(listening_sockaddr6);
        listening_sockaddr6.sin6_family      = AF_INET6;
        listening_sockaddr6.sin6_port        = port.NotAnInteger;       // Pass in opaque ID without any byte swapping
        listening_sockaddr6.sin6_flowinfo    = 0;
        listening_sockaddr6.sin6_addr        = in6addr_any; // Want to receive multicasts AND unicasts on this socket
        listening_sockaddr6.sin6_scope_id    = 0;
        err = bind(skt, (struct sockaddr *) &listening_sockaddr6, sizeof(listening_sockaddr6));
        if (err) { errstr = "bind"; goto fail; }
        if (outport) outport->NotAnInteger = listening_sockaddr6.sin6_port;
    }

    fcntl(skt, F_SETFL, fcntl(skt, F_GETFL, 0) | O_NONBLOCK); // set non-blocking
    fcntl(skt, F_SETFD, 1); // set close-on-exec
    *s = skt;
    k->KQcallback = myKQSocketCallBack;
    k->KQcontext  = cp;
    k->KQtask     = "UDP packet reception";
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
    k->readSource = mDNSNULL;
    k->writeSource = mDNSNULL;
    k->fdClosed = mDNSfalse;
#endif
    KQueueSet(*s, EV_ADD, EVFILT_READ, k);

    return(mStatus_NoError);

fail:
    saved_errno = errno;
    // For "bind" failures, only write log messages for our shared mDNS port, or for binding to zero
    if (strcmp(errstr, "bind") || mDNSSameIPPort(port, MulticastDNSPort) || mDNSIPPortIsZero(port))
        LogMsg("%s skt %d port %d error %d errno %d (%s)", errstr, skt, mDNSVal16(port), err, saved_errno, strerror(saved_errno));

    // If we got a "bind" failure of EADDRINUSE, inform the caller as it might need to try another random port
    if (!strcmp(errstr, "bind") && saved_errno == EADDRINUSE)
    {
        err = EADDRINUSE;
        if (mDNSSameIPPort(port, MulticastDNSPort))
            NotifyOfElusiveBug("Setsockopt SO_REUSEPORT failed",
                               "Congratulations, you've reproduced an elusive bug.\r"
                               "Please contact the current assignee of <rdar://problem/3814904>.\r"
                               "Alternatively, you can send email to radar-3387020@group.apple.com. (Note number is different.)\r"
                               "If possible, please leave your machine undisturbed so that someone can come to investigate the problem.");
    }

    mDNSPlatformCloseFD(k, skt);
    return(err);
}

mDNSexport UDPSocket *mDNSPlatformUDPSocket(const mDNSIPPort requestedport)
{
    mStatus err;
    mDNSIPPort port = requestedport;
    mDNSBool randomizePort = mDNSIPPortIsZero(requestedport);
    int i = 10000; // Try at most 10000 times to get a unique random port
    UDPSocket *p = (UDPSocket *) callocL("UDPSocket", sizeof(*p));
    if (!p) { LogMsg("mDNSPlatformUDPSocket: memory exhausted"); return(mDNSNULL); }
    p->ss.port  = zeroIPPort;
    p->ss.m     = &mDNSStorage;
    p->ss.sktv4 = -1;
    p->ss.sktv6 = -1;
    p->ss.proxy = mDNSfalse;

    do
    {
        // The kernel doesn't do cryptographically strong random port allocation, so we do it ourselves here
        if (randomizePort) port = mDNSOpaque16fromIntVal(0xC000 + mDNSRandom(0x3FFF));
        err = SetupSocket(&p->ss, port, AF_INET, &p->ss.port);
        if (!err)
        {
            err = SetupSocket(&p->ss, port, AF_INET6, &p->ss.port);
            if (err) { mDNSPlatformCloseFD(&p->ss.kqsv4, p->ss.sktv4); p->ss.sktv4 = -1; }
        }
        i--;
    } while (err == EADDRINUSE && randomizePort && i);

    if (err)
    {
        // In customer builds we don't want to log failures with port 5351, because this is a known issue
        // of failing to bind to this port when Internet Sharing has already bound to it
        // We also don't want to log about port 5350, due to a known bug when some other
        // process is bound to it.
        if (mDNSSameIPPort(requestedport, NATPMPPort) || mDNSSameIPPort(requestedport, NATPMPAnnouncementPort))
            LogInfo("mDNSPlatformUDPSocket: SetupSocket %d failed error %d errno %d (%s)", mDNSVal16(requestedport), err, errno, strerror(errno));
        else LogMsg("mDNSPlatformUDPSocket: SetupSocket %d failed error %d errno %d (%s)", mDNSVal16(requestedport), err, errno, strerror(errno));
        freeL("UDPSocket", p);
        return(mDNSNULL);
    }
    return(p);
}

#ifdef UNIT_TEST
UNITTEST_UDPCLOSE
#else
mDNSexport void mDNSPlatformUDPClose(UDPSocket *sock)
{
    CloseSocketSet(&sock->ss);
    freeL("UDPSocket", sock);
}
#endif

mDNSexport mDNSBool mDNSPlatformUDPSocketEncounteredEOF(const UDPSocket *sock)
{
    return (sock->ss.sktv4EOF || sock->ss.sktv6EOF);
}

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - BPF Raw packet sending/receiving
#endif

#if APPLE_OSX_mDNSResponder

mDNSexport void mDNSPlatformSendRawPacket(const void *const msg, const mDNSu8 *const end, mDNSInterfaceID InterfaceID)
{
    if (!InterfaceID) { LogMsg("mDNSPlatformSendRawPacket: No InterfaceID specified"); return; }
    NetworkInterfaceInfoOSX *info;

    info = IfindexToInterfaceInfoOSX(InterfaceID);
    if (info == NULL)
    {
        LogMsg("mDNSPlatformSendRawPacket: Invalid interface index %p", InterfaceID);
        return;
    }
    if (info->BPF_fd < 0)
        LogMsg("mDNSPlatformSendRawPacket: %s BPF_fd %d not ready", info->ifinfo.ifname, info->BPF_fd);
    else
    {
        //LogMsg("mDNSPlatformSendRawPacket %d bytes on %s", end - (mDNSu8 *)msg, info->ifinfo.ifname);
        if (write(info->BPF_fd, msg, end - (mDNSu8 *)msg) < 0)
            LogMsg("mDNSPlatformSendRawPacket: BPF write(%d) failed %d (%s)", info->BPF_fd, errno, strerror(errno));
    }
}

mDNSexport void mDNSPlatformSetLocalAddressCacheEntry(const mDNSAddr *const tpa, const mDNSEthAddr *const tha, mDNSInterfaceID InterfaceID)
{
    if (!InterfaceID) { LogMsg("mDNSPlatformSetLocalAddressCacheEntry: No InterfaceID specified"); return; }
    NetworkInterfaceInfoOSX *info;
    info = IfindexToInterfaceInfoOSX(InterfaceID);
    if (info == NULL) { LogMsg("mDNSPlatformSetLocalAddressCacheEntry: Invalid interface index %p", InterfaceID); return; }
    // Manually inject an entry into our local ARP cache.
    // (We can't do this by sending an ARP broadcast, because the kernel only pays attention to incoming ARP packets, not outgoing.)
    if (!mDNS_AddressIsLocalSubnet(&mDNSStorage, InterfaceID, tpa))
        LogSPS("Don't need address cache entry for %s %#a %.6a",            info->ifinfo.ifname, tpa, tha);
    else
    {
        int result = mDNSSetLocalAddressCacheEntry(info->scope_id, tpa->type, tpa->ip.v6.b, tha->b);
        if (result) LogMsg("Set local address cache entry for %s %#a %.6a failed: %d", info->ifinfo.ifname, tpa, tha, result);
        else LogSPS("Set local address cache entry for %s %#a %.6a",            info->ifinfo.ifname, tpa, tha);
    }
}

mDNSlocal void CloseBPF(NetworkInterfaceInfoOSX *const i)
{
    LogSPS("%s closing BPF fd %d", i->ifinfo.ifname, i->BPF_fd);
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
    // close will happen in the cancel handler
    dispatch_source_cancel(i->BPF_source);
#else

    // Note: MUST NOT close() the underlying native BSD sockets.
    // CFSocketInvalidate() will do that for us, in its own good time, which may not necessarily be immediately, because
    // it first has to unhook the sockets from its select() call on its other thread, before it can safely close them.
    CFRunLoopRemoveSource(CFRunLoopGetMain(), i->BPF_rls, kCFRunLoopDefaultMode);
    CFRelease(i->BPF_rls);
    CFSocketInvalidate(i->BPF_cfs);
    CFRelease(i->BPF_cfs);
#endif
    i->BPF_fd = -1;
    if (i->BPF_mcfd >= 0) { close(i->BPF_mcfd); i->BPF_mcfd = -1; }
}

mDNSlocal void bpf_callback_common(NetworkInterfaceInfoOSX *info)
{
    KQueueLock();

    // Now we've got the lock, make sure the kqueue thread didn't close the fd out from under us (will not be a problem once the OS X
    // kernel has a mechanism for dispatching all events to a single thread, but for now we have to guard against this race condition).
    if (info->BPF_fd < 0) goto exit;

    ssize_t n = read(info->BPF_fd, &info->m->imsg, info->BPF_len);
    const mDNSu8 *ptr = (const mDNSu8 *)&info->m->imsg;
    const mDNSu8 *end = (const mDNSu8 *)&info->m->imsg + n;
    debugf("%3d: bpf_callback got %d bytes on %s", info->BPF_fd, n, info->ifinfo.ifname);

    if (n<0)
    {
        /* <rdar://problem/10287386>
         * sometimes there can be a race condition btw when the bpf socket
         * gets data and the callback get scheduled and when we call BIOCSETF (which
         * clears the socket).  this can cause the read to hang for a really long time
         * and effectively prevent us from responding to requests for long periods of time.
         * to prevent this make the socket non blocking and just bail if we dont get anything
         */
        if (errno == EAGAIN)
        {
            LogMsg("bpf_callback got EAGAIN bailing");
            goto exit;
        }
        LogMsg("Closing %s BPF fd %d due to error %d (%s)", info->ifinfo.ifname, info->BPF_fd, errno, strerror(errno));
        CloseBPF(info);
        goto exit;
    }

    while (ptr < end)
    {
        const struct bpf_hdr *const bh = (const struct bpf_hdr *)ptr;
        debugf("%3d: bpf_callback ptr %p bh_hdrlen %d data %p bh_caplen %4d bh_datalen %4d next %p remaining %4d",
               info->BPF_fd, ptr, bh->bh_hdrlen, ptr + bh->bh_hdrlen, bh->bh_caplen, bh->bh_datalen,
               ptr + BPF_WORDALIGN(bh->bh_hdrlen + bh->bh_caplen), end - (ptr + BPF_WORDALIGN(bh->bh_hdrlen + bh->bh_caplen)));
        // Note that BPF guarantees that the NETWORK LAYER header will be word aligned, not the link-layer header.
        // Given that An Ethernet header is 14 bytes, this means that if the network layer header (e.g. IP header,
        // ARP message, etc.) is 4-byte aligned, then necessarily the Ethernet header will be NOT be 4-byte aligned.
        mDNSCoreReceiveRawPacket(info->m, ptr + bh->bh_hdrlen, ptr + bh->bh_hdrlen + bh->bh_caplen, info->ifinfo.InterfaceID);
        ptr += BPF_WORDALIGN(bh->bh_hdrlen + bh->bh_caplen);
    }
exit:
    KQueueUnlock("bpf_callback");
}
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
mDNSlocal void bpf_callback_dispatch(NetworkInterfaceInfoOSX *const info)
{
    bpf_callback_common(info);
}
#else
mDNSlocal void bpf_callback(const CFSocketRef cfs, const CFSocketCallBackType CallBackType, const CFDataRef address, const void *const data, void *const context)
{
    (void)cfs;
    (void)CallBackType;
    (void)address;
    (void)data;
    bpf_callback_common((NetworkInterfaceInfoOSX *)context);
}
#endif

mDNSexport void mDNSPlatformSendKeepalive(mDNSAddr *sadd, mDNSAddr *dadd, mDNSIPPort *lport, mDNSIPPort *rport, mDNSu32 seq, mDNSu32 ack, mDNSu16 win)
{
    LogMsg("mDNSPlatformSendKeepalive called\n");
    mDNSSendKeepalive(sadd->ip.v6.b, dadd->ip.v6.b, lport->NotAnInteger, rport->NotAnInteger, seq, ack, win);
}

mDNSexport mStatus mDNSPlatformClearSPSData(void)
{
    CFStringRef  spsAddressKey  = NULL;
    CFStringRef  ownerOPTRecKey = NULL;
    SCDynamicStoreRef addrStore = SCDynamicStoreCreate(NULL, CFSTR("mDNSResponder:SPSAddresses"), NULL, NULL);
    SCDynamicStoreRef optStore  = SCDynamicStoreCreate(NULL, CFSTR("mDNSResponder:SPSOPTRecord"), NULL, NULL);

    spsAddressKey = SCDynamicStoreKeyCreateNetworkInterfaceEntity (kCFAllocatorDefault, kSCDynamicStoreDomainState, kSCCompAnyRegex, CFSTR("BonjourSleepProxyAddress"));
    if (spsAddressKey != NULL)
    {
        CFArrayRef keyList = SCDynamicStoreCopyKeyList(addrStore, spsAddressKey);
        if (keyList != NULL)
        {
            if (SCDynamicStoreSetMultiple(addrStore, NULL, keyList, NULL) == false)
                LogSPS("mDNSPlatformClearSPSData: Unable to remove %s : error %s", CFStringGetCStringPtr( spsAddressKey, kCFStringEncodingASCII), SCErrorString(SCError()));
        }
        if (keyList) CFRelease(keyList);
    }
    ownerOPTRecKey= SCDynamicStoreKeyCreateNetworkInterfaceEntity (kCFAllocatorDefault, kSCDynamicStoreDomainState, kSCCompAnyRegex, CFSTR("BonjourSleepProxyOPTRecord"));
    if(ownerOPTRecKey != NULL)
    {
        CFArrayRef keyList = SCDynamicStoreCopyKeyList(addrStore, ownerOPTRecKey);
        if (keyList != NULL)
        {
            if (SCDynamicStoreSetMultiple(optStore, NULL, keyList, NULL) == false)
                LogSPS("mDNSPlatformClearSPSData: Unable to remove %s : error %s", CFStringGetCStringPtr(ownerOPTRecKey, kCFStringEncodingASCII), SCErrorString(SCError()));
        }
        if (keyList) CFRelease(keyList);
    }

    if (addrStore)   CFRelease(addrStore);
    if (optStore)    CFRelease(optStore);
    if (spsAddressKey)  CFRelease(spsAddressKey);
    if (ownerOPTRecKey) CFRelease(ownerOPTRecKey);
    return KERN_SUCCESS;
}

mDNSlocal int getMACAddress(int family, v6addr_t raddr, v6addr_t gaddr, int *gfamily, ethaddr_t eth)
{
    struct
    {
        struct rt_msghdr m_rtm;
        char   m_space[512];
    } m_rtmsg;
    
    struct rt_msghdr *rtm = &(m_rtmsg.m_rtm);
    char  *cp  = m_rtmsg.m_space;
    int    seq = 6367, sock, rlen, i;
    struct sockaddr_in      *sin  = NULL;
    struct sockaddr_in6     *sin6 = NULL;
    struct sockaddr_dl      *sdl  = NULL;
    struct sockaddr_storage  sins;
    struct sockaddr_dl       sdl_m;
    
#define NEXTADDR(w, s, len)         \
if (rtm->rtm_addrs & (w))       \
{                               \
bcopy((char *)s, cp, len);  \
cp += len;                  \
}
    
    bzero(&sins,  sizeof(struct sockaddr_storage));
    bzero(&sdl_m, sizeof(struct sockaddr_dl));
    bzero((char *)&m_rtmsg, sizeof(m_rtmsg));
    
    sock = socket(PF_ROUTE, SOCK_RAW, 0);
    if (sock < 0)
    {
        const int socket_errno = errno;
        LogMsg("getMACAddress: Can not open the socket - %s", strerror(socket_errno));
        return socket_errno;
    }
    
    rtm->rtm_addrs   |= RTA_DST | RTA_GATEWAY;
    rtm->rtm_type     = RTM_GET;
    rtm->rtm_flags    = 0;
    rtm->rtm_version  = RTM_VERSION;
    rtm->rtm_seq      = ++seq;
    
    sdl_m.sdl_len     = sizeof(sdl_m);
    sdl_m.sdl_family  = AF_LINK;
    if (family == AF_INET)
    {
        sin = (struct sockaddr_in*)&sins;
        sin->sin_family = AF_INET;
        sin->sin_len    = sizeof(struct sockaddr_in);
        memcpy(&sin->sin_addr, raddr, sizeof(struct in_addr));
        NEXTADDR(RTA_DST, sin, sin->sin_len);
    }
    else if (family == AF_INET6)
    {
        sin6 = (struct sockaddr_in6 *)&sins;
        sin6->sin6_len    = sizeof(struct sockaddr_in6);
        sin6->sin6_family = AF_INET6;
        memcpy(&sin6->sin6_addr, raddr, sizeof(struct in6_addr));
        NEXTADDR(RTA_DST, sin6, sin6->sin6_len);
    }
    NEXTADDR(RTA_GATEWAY, &sdl_m, sdl_m.sdl_len);
    rtm->rtm_msglen = rlen = cp - (char *)&m_rtmsg;
    
    if (write(sock, (char *)&m_rtmsg, rlen) < 0)
    {
        const int write_errno = errno;
        LogMsg("getMACAddress: writing to routing socket: %s", strerror(write_errno));
        close(sock);
        return write_errno;
    }
    
    do
    {
        rlen = read(sock, (char *)&m_rtmsg, sizeof(m_rtmsg));
    }
    while (rlen > 0 && (rtm->rtm_seq != seq || rtm->rtm_pid != getpid()));
    
    if (rlen < 0)
        LogMsg("getMACAddress: Read from routing socket failed");
    
    if (family == AF_INET)
    {
        sin = (struct sockaddr_in *) (rtm + 1);
        sdl = (struct sockaddr_dl *) (sin->sin_len + (char *) sin);
    }
    else if (family == AF_INET6)
    {
        sin6 = (struct sockaddr_in6 *) (rtm +1);
        sdl  = (struct sockaddr_dl  *) (sin6->sin6_len + (char *) sin6);
    }
    
    if (!sdl)
    {
        LogMsg("getMACAddress: sdl is NULL for family %d", family);
        close(sock);
        return -1;
    }
    
    // If the address is not on the local net, we get the IP address of the gateway.
    // We would have to repeat the process to get the MAC address of the gateway
    *gfamily = sdl->sdl_family;
    if (sdl->sdl_family == AF_INET)
    {
        if (sin)
        {
            struct sockaddr_in *new_sin = (struct sockaddr_in *)(sin->sin_len +(char*) sin);
            memcpy(gaddr, &new_sin->sin_addr, sizeof(struct in_addr));
        }
        else
        {
            LogMsg("getMACAddress: sin is NULL");
        }
        close(sock);
        return -1;
    }
    else if (sdl->sdl_family == AF_INET6)
    {
        if (sin6)
        {
            struct sockaddr_in6 *new_sin6 = (struct sockaddr_in6 *)(sin6->sin6_len +(char*) sin6);
            memcpy(gaddr, &new_sin6->sin6_addr, sizeof(struct in6_addr));
        }
        else
        {
            LogMsg("getMACAddress: sin6 is NULL");
        }
        close(sock);
        return -1;
    }
    
    unsigned char *ptr = (unsigned char *)LLADDR(sdl);
    for (i = 0; i < ETHER_ADDR_LEN; i++)
        (eth)[i] = *(ptr +i);
    
    close(sock);
    
    return KERN_SUCCESS;
}

mDNSlocal int GetRemoteMacinternal(int family, v6addr_t raddr, ethaddr_t eth)
{
    int      ret = 0;
    v6addr_t gateway;
    int      gfamily = 0;
    int      count = 0;

    do
    {
        ret = getMACAddress(family, raddr, gateway, &gfamily, eth);
        if (ret == -1)
        {
            memcpy(raddr, gateway, (gfamily == AF_INET) ? 4 : 16);
            family = gfamily;
            count++;
        }
    }
    while ((ret == -1) && (count < 5));
    return ret;
}

mDNSlocal int StoreSPSMACAddressinternal(int family, v6addr_t spsaddr, const char *ifname)
{
    ethaddr_t              eth;
    char                   spsip[INET6_ADDRSTRLEN];
    int                    ret        = 0;
    CFStringRef            sckey      = NULL;
    SCDynamicStoreRef      store      = SCDynamicStoreCreate(NULL, CFSTR("mDNSResponder:StoreSPSMACAddress"), NULL, NULL);
    SCDynamicStoreRef      ipstore    = SCDynamicStoreCreate(NULL, CFSTR("mDNSResponder:GetIPv6Addresses"), NULL, NULL);
    CFMutableDictionaryRef dict       = NULL;
    CFStringRef            entityname = NULL;
    CFDictionaryRef        ipdict     = NULL;
    CFArrayRef             addrs      = NULL;
    
    if ((store == NULL) || (ipstore == NULL))
    {
        LogMsg("StoreSPSMACAddressinternal: Unable to accesss SC Dynamic Store");
        ret = -1;
        goto fin;
    }
    
    // Get the MAC address of the Sleep Proxy Server
    memset(eth, 0, sizeof(eth));
    ret = GetRemoteMacinternal(family, spsaddr, eth);
    if (ret !=  0)
    {
        LogMsg("StoreSPSMACAddressinternal: Failed to determine the MAC address");
        goto fin;
    }
    
    // Create/Update the dynamic store entry for the specified interface
    sckey = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%s%s%s"), "State:/Network/Interface/", ifname, "/BonjourSleepProxyAddress");
    dict  = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (!dict)
    {
        LogMsg("StoreSPSMACAddressinternal: SPSCreateDict() Could not create CFDictionary dict");
        ret = -1;
        goto fin;
    }
    
    CFStringRef macaddr = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%02x:%02x:%02x:%02x:%02x:%02x"), eth[0], eth[1], eth[2], eth[3], eth[4], eth[5]);
    CFDictionarySetValue(dict, CFSTR("MACAddress"), macaddr);
    if (NULL != macaddr)
        CFRelease(macaddr);
    
    if( NULL == inet_ntop(family, (void *)spsaddr, spsip, sizeof(spsip)))
    {
        LogMsg("StoreSPSMACAddressinternal: inet_ntop failed: %s", strerror(errno));
        ret = -1;
        goto fin;
    }
    
    CFStringRef ipaddr = CFStringCreateWithCString(NULL, spsip, kCFStringEncodingUTF8);
    CFDictionarySetValue(dict, CFSTR("IPAddress"), ipaddr);
    if (NULL != ipaddr)
        CFRelease(ipaddr);
    
    // Get the current IPv6 addresses on this interface and store them so NAs can be sent on wakeup
    if ((entityname = CFStringCreateWithFormat(NULL, NULL, CFSTR("State:/Network/Interface/%s/IPv6"), ifname)) != NULL)
    {
        if ((ipdict = SCDynamicStoreCopyValue(ipstore, entityname)) != NULL)
        {
            if((addrs = CFDictionaryGetValue(ipdict, CFSTR("Addresses"))) != NULL)
            {
                addrs = CFRetain(addrs);
                CFDictionarySetValue(dict, CFSTR("RegisteredAddresses"), addrs);
            }
        }
    }
    SCDynamicStoreSetValue(store, sckey, dict);
    
fin:
    if (store)      CFRelease(store);
    if (ipstore)    CFRelease(ipstore);
    if (sckey)      CFRelease(sckey);
    if (dict)       CFRelease(dict);
    if (ipdict)     CFRelease(ipdict);
    if (entityname) CFRelease(entityname);
    if (addrs)      CFRelease(addrs);
    
    return ret;
}

mDNSlocal void mDNSStoreSPSMACAddress(int family, v6addr_t spsaddr, char *ifname)
{
    struct
    {
        v6addr_t saddr;
    } addr;
    int err = 0;
    
    mDNSPlatformMemCopy(addr.saddr, spsaddr, sizeof(v6addr_t));
    
    err = StoreSPSMACAddressinternal(family, (uint8_t *)addr.saddr, ifname);
    if (err != 0)
        LogMsg("mDNSStoreSPSMACAddress : failed");
}

mDNSexport mStatus mDNSPlatformStoreSPSMACAddr(mDNSAddr *spsaddr, char *ifname)
{
    int family = (spsaddr->type == mDNSAddrType_IPv4) ? AF_INET : AF_INET6;
    
    LogInfo("mDNSPlatformStoreSPSMACAddr : Storing %#a on interface %s", spsaddr, ifname);
    mDNSStoreSPSMACAddress(family, spsaddr->ip.v6.b, ifname);
    
    return KERN_SUCCESS;
}


mDNSexport mStatus mDNSPlatformStoreOwnerOptRecord(char *ifname, DNSMessage* msg, int length)
{
    int                    ret   = 0;
    CFStringRef            sckey = NULL;
    SCDynamicStoreRef      store = SCDynamicStoreCreate(NULL, CFSTR("mDNSResponder:StoreOwnerOPTRecord"), NULL, NULL);
    CFMutableDictionaryRef dict  = NULL;

    if (store == NULL)
    {
        LogMsg("mDNSPlatformStoreOwnerOptRecord: Unable to accesss SC Dynamic Store");
        ret = -1;
        goto fin;
    }

    // Create/Update the dynamic store entry for the specified interface
    sckey = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%s%s%s"), "State:/Network/Interface/", ifname, "/BonjourSleepProxyOPTRecord");
    dict  = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (!dict)
    {
        LogMsg("mDNSPlatformStoreOwnerOptRecord: Could not create CFDictionary dictionary to store OPT Record");       
        ret =-1;
        goto fin;
    }

    CFDataRef optRec = NULL;
    optRec = CFDataCreate(NULL, (const uint8_t *)msg, (CFIndex)length);
    CFDictionarySetValue(dict, CFSTR("OwnerOPTRecord"), optRec);
    if (NULL != optRec) CFRelease(optRec);

    SCDynamicStoreSetValue(store, sckey, dict);

fin:
    if (NULL != store)  CFRelease(store);
    if (NULL != sckey)  CFRelease(sckey);
    if (NULL != dict)   CFRelease(dict);
    return ret;
}

mDNSlocal void mDNSGet_RemoteMAC(int family, v6addr_t raddr)
{
    ethaddr_t            eth;
    IPAddressMACMapping *addrMapping;
    int kr = KERN_FAILURE;
    struct
    {
        v6addr_t addr;
    } dst;

    bzero(eth, sizeof(ethaddr_t));
    mDNSPlatformMemCopy(dst.addr, raddr, sizeof(v6addr_t));

    kr = GetRemoteMacinternal(family, (uint8_t *)dst.addr, eth);

    // If the call to get the remote MAC address succeeds, allocate and copy
    // the values and schedule a task to update the MAC address in the TCP Keepalive record.
    if (kr == 0)
    {
        addrMapping = (IPAddressMACMapping *) mDNSPlatformMemAllocateClear(sizeof(*addrMapping));
        // This memory allocation is not checked for failure
        // It also shoudn’t need to be a memory allocation at all -- why not just use a stack variable? -- SC
        snprintf(addrMapping->ethaddr, sizeof(addrMapping->ethaddr), "%02x:%02x:%02x:%02x:%02x:%02x",
                     eth[0], eth[1], eth[2], eth[3], eth[4], eth[5]);
        // Why is the address represented using text? The UpdateRMAC routine just parses it back into a six-byte MAC address. -- SC
        if (family == AF_INET)
        {
            addrMapping->ipaddr.type = mDNSAddrType_IPv4;
            mDNSPlatformMemCopy(addrMapping->ipaddr.ip.v4.b, raddr, sizeof(v6addr_t));
            // This is the wrong size. It’s using sizeof(v6addr_t) for an IPv4 address -- SC
        }
        else
        {
            addrMapping->ipaddr.type = mDNSAddrType_IPv6;
            mDNSPlatformMemCopy(addrMapping->ipaddr.ip.v6.b, raddr, sizeof(v6addr_t));
        }
        UpdateRMAC(&mDNSStorage, addrMapping);
    }
}

mDNSexport mStatus mDNSPlatformGetRemoteMacAddr(mDNSAddr *raddr)
{
    int family = (raddr->type == mDNSAddrType_IPv4) ? AF_INET : AF_INET6;
    
    LogInfo("mDNSPlatformGetRemoteMacAddr calling mDNSGet_RemoteMAC");
    mDNSGet_RemoteMAC(family, raddr->ip.v6.b);
    
    return KERN_SUCCESS;
}

mDNSexport mStatus mDNSPlatformRetrieveTCPInfo(mDNSAddr *laddr, mDNSIPPort *lport, mDNSAddr *raddr, mDNSIPPort *rport, mDNSTCPInfo *mti)
{
    mDNSs32 intfid;
    mDNSs32 error  = 0;
    int     family = (laddr->type == mDNSAddrType_IPv4) ? AF_INET : AF_INET6;

    error = mDNSRetrieveTCPInfo(family, laddr->ip.v6.b, lport->NotAnInteger, raddr->ip.v6.b, rport->NotAnInteger, (uint32_t *)&(mti->seq), (uint32_t *)&(mti->ack), (uint16_t *)&(mti->window), (int32_t*)&intfid);
    if (error != KERN_SUCCESS)
    {
        LogMsg("%s: mDNSRetrieveTCPInfo returned : %d", __func__, error);
        return error;
    }
    mti->IntfId = mDNSPlatformInterfaceIDfromInterfaceIndex(&mDNSStorage, intfid);
    return error;
}

#define BPF_SetOffset(from, cond, to) (from)->cond = (to) - 1 - (from)

mDNSlocal int CountProxyTargets(NetworkInterfaceInfoOSX *x, int *p4, int *p6)
{
    int numv4 = 0, numv6 = 0;
    AuthRecord *rr;

    for (rr = mDNSStorage.ResourceRecords; rr; rr=rr->next)
        if (rr->resrec.InterfaceID == x->ifinfo.InterfaceID && rr->AddressProxy.type == mDNSAddrType_IPv4)
        {
            if (p4) LogSPS("CountProxyTargets: fd %d %-7s IP%2d %.4a", x->BPF_fd, x->ifinfo.ifname, numv4, &rr->AddressProxy.ip.v4);
            numv4++;
        }

    for (rr = mDNSStorage.ResourceRecords; rr; rr=rr->next)
        if (rr->resrec.InterfaceID == x->ifinfo.InterfaceID && rr->AddressProxy.type == mDNSAddrType_IPv6)
        {
            if (p6) LogSPS("CountProxyTargets: fd %d %-7s IP%2d %.16a", x->BPF_fd, x->ifinfo.ifname, numv6, &rr->AddressProxy.ip.v6);
            numv6++;
        }

    if (p4) *p4 = numv4;
    if (p6) *p6 = numv6;
    return(numv4 + numv6);
}

mDNSexport void mDNSPlatformUpdateProxyList(const mDNSInterfaceID InterfaceID)
{
    mDNS *const m = &mDNSStorage;
    NetworkInterfaceInfoOSX *x;

    // Note: We can't use IfIndexToInterfaceInfoOSX because that looks for Registered also.
    for (x = m->p->InterfaceList; x; x = x->next) if ((x->ifinfo.InterfaceID == InterfaceID) && (x->BPF_fd >= 0)) break;

    if (!x) { LogMsg("mDNSPlatformUpdateProxyList: ERROR InterfaceID %p not found", InterfaceID); return; }

    #define MAX_BPF_ADDRS 250
    int numv4 = 0, numv6 = 0;

    if (CountProxyTargets(x, &numv4, &numv6) > MAX_BPF_ADDRS)
    {
        LogMsg("mDNSPlatformUpdateProxyList: ERROR Too many address proxy records v4 %d v6 %d", numv4, numv6);
        if (numv4 > MAX_BPF_ADDRS) numv4 = MAX_BPF_ADDRS;
        numv6 = MAX_BPF_ADDRS - numv4;
    }

    LogSPS("mDNSPlatformUpdateProxyList: fd %d %-7s MAC  %.6a %d v4 %d v6", x->BPF_fd, x->ifinfo.ifname, &x->ifinfo.MAC, numv4, numv6);

    // Caution: This is a static structure, so we need to be careful that any modifications we make to it
    // are done in such a way that they work correctly when mDNSPlatformUpdateProxyList is called multiple times
    static struct bpf_insn filter[17 + MAX_BPF_ADDRS] =
    {
        BPF_STMT(BPF_LD  + BPF_H   + BPF_ABS, 12),              // 0 Read Ethertype (bytes 12,13)

        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, 0x0806, 0, 1),      // 1 If Ethertype == ARP goto next, else 3
        BPF_STMT(BPF_RET + BPF_K,             42),              // 2 Return 42-byte ARP

        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, 0x0800, 4, 0),      // 3 If Ethertype == IPv4 goto 8 (IPv4 address list check) else next

        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, 0x86DD, 0, 9),      // 4 If Ethertype == IPv6 goto next, else exit
        BPF_STMT(BPF_LD  + BPF_H   + BPF_ABS, 20),              // 5 Read Protocol and Hop Limit (bytes 20,21)
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, 0x3AFF, 0, 9),      // 6 If (Prot,TTL) == (3A,FF) goto next, else IPv6 address list check
        BPF_STMT(BPF_RET + BPF_K,             86),              // 7 Return 86-byte ND

        // Is IPv4 packet; check if it's addressed to any IPv4 address we're proxying for
        BPF_STMT(BPF_LD  + BPF_W   + BPF_ABS, 30),              // 8 Read IPv4 Dst (bytes 30,31,32,33)
    };

    // Special filter program to use when there are no address proxy records
    static struct bpf_insn nullfilter[] =
    {
        BPF_STMT(BPF_RET | BPF_K, 0)                            // 0 Match no packets and return size 0
    };

    struct bpf_program prog;
    if (!numv4 && !numv6)
    {
        LogSPS("mDNSPlatformUpdateProxyList: No need for filter");
        if (m->timenow == 0) LogMsg("mDNSPlatformUpdateProxyList: m->timenow == 0");

        // Cancel any previous ND group memberships we had
        if (x->BPF_mcfd >= 0)
        {
            close(x->BPF_mcfd);
            x->BPF_mcfd = -1;
        }

        // Schedule check to see if we can close this BPF_fd now
        if (!m->NetworkChanged) m->NetworkChanged = NonZeroTime(m->timenow + mDNSPlatformOneSecond * 2);
        if (x->BPF_fd < 0) return;      // If we've already closed our BPF_fd, no need to generate an error message below
        prog.bf_len   = 1;
        prog.bf_insns = nullfilter;
    }
    else
    {
        struct bpf_insn *pc   = &filter[9];
        struct bpf_insn *chk6 = pc   + numv4 + 1;   // numv4 address checks, plus a "return 0"
        struct bpf_insn *fail = chk6 + 1 + numv6;   // Get v6 Dst LSW, plus numv6 address checks
        struct bpf_insn *ret4 = fail + 1;
        struct bpf_insn *ret6 = ret4 + 4;

        static const struct bpf_insn rf  = BPF_STMT(BPF_RET + BPF_K, 0);                // No match: Return nothing

        static const struct bpf_insn g6  = BPF_STMT(BPF_LD  + BPF_W   + BPF_ABS, 50);   // Read IPv6 Dst LSW (bytes 50,51,52,53)

        static const struct bpf_insn r4a = BPF_STMT(BPF_LDX + BPF_B   + BPF_MSH, 14);   // Get IP Header length (normally 20)
        static const struct bpf_insn r4b = BPF_STMT(BPF_LD  + BPF_IMM,           54);   // A = 54 (14-byte Ethernet plus 20-byte TCP + 20 bytes spare)
        static const struct bpf_insn r4c = BPF_STMT(BPF_ALU + BPF_ADD + BPF_X,    0);   // A += IP Header length
        static const struct bpf_insn r4d = BPF_STMT(BPF_RET + BPF_A, 0);                // Success: Return Ethernet + IP + TCP + 20 bytes spare (normally 74)

        static const struct bpf_insn r6a = BPF_STMT(BPF_RET + BPF_K, 94);               // Success: Return Eth + IPv6 + TCP + 20 bytes spare

        BPF_SetOffset(&filter[4], jf, fail);    // If Ethertype not ARP, IPv4, or IPv6, fail
        BPF_SetOffset(&filter[6], jf, chk6);    // If IPv6 but not ICMPv6, go to IPv6 address list check

        // BPF Byte-Order Note
        // The BPF API designers apparently thought that programmers would not be smart enough to use htons
        // and htonl correctly to convert numeric values to network byte order on little-endian machines,
        // so instead they chose to make the API implicitly byte-swap *ALL* values, even literal byte strings
        // that shouldn't be byte-swapped, like ASCII text, Ethernet addresses, IP addresses, etc.
        // As a result, if we put Ethernet addresses and IP addresses in the right byte order, the BPF API
        // will byte-swap and make them backwards, and then our filter won't work. So, we have to arrange
        // that on little-endian machines we deliberately put addresses in memory with the bytes backwards,
        // so that when the BPF API goes through and swaps them all, they end up back as they should be.
        // In summary, if we byte-swap all the non-numeric fields that shouldn't be swapped, and we *don't*
        // swap any of the numeric values that *should* be byte-swapped, then the filter will work correctly.

        // IPSEC capture size notes:
        //  8 bytes UDP header
        //  4 bytes Non-ESP Marker
        // 28 bytes IKE Header
        // --
        // 40 Total. Capturing TCP Header + 20 gets us enough bytes to receive the IKE Header in a UDP-encapsulated IKE packet.

        AuthRecord *rr;
        for (rr = m->ResourceRecords; rr; rr=rr->next)
            if (rr->resrec.InterfaceID == InterfaceID && rr->AddressProxy.type == mDNSAddrType_IPv4)
            {
                mDNSv4Addr a = rr->AddressProxy.ip.v4;
                pc->code = BPF_JMP + BPF_JEQ + BPF_K;
                BPF_SetOffset(pc, jt, ret4);
                pc->jf   = 0;
                pc->k    = (bpf_u_int32)a.b[0] << 24 | (bpf_u_int32)a.b[1] << 16 | (bpf_u_int32)a.b[2] << 8 | (bpf_u_int32)a.b[3];
                pc++;
            }
        *pc++ = rf;

        if (pc != chk6) LogMsg("mDNSPlatformUpdateProxyList: pc %p != chk6 %p", pc, chk6);
        *pc++ = g6; // chk6 points here

        // First cancel any previous ND group memberships we had, then create a fresh socket
        if (x->BPF_mcfd >= 0) close(x->BPF_mcfd);
        x->BPF_mcfd = socket(AF_INET6, SOCK_DGRAM, 0);

        for (rr = m->ResourceRecords; rr; rr=rr->next)
            if (rr->resrec.InterfaceID == InterfaceID && rr->AddressProxy.type == mDNSAddrType_IPv6)
            {
                const mDNSv6Addr *const a = &rr->AddressProxy.ip.v6;
                pc->code = BPF_JMP + BPF_JEQ + BPF_K;
                BPF_SetOffset(pc, jt, ret6);
                pc->jf   = 0;
                pc->k    = (bpf_u_int32)a->b[0x0C] << 24 | (bpf_u_int32)a->b[0x0D] << 16 | (bpf_u_int32)a->b[0x0E] << 8 | (bpf_u_int32)a->b[0x0F];
                pc++;

                struct ipv6_mreq i6mr;
                i6mr.ipv6mr_interface = x->scope_id;
                i6mr.ipv6mr_multiaddr = *(const struct in6_addr*)&NDP_prefix;
                i6mr.ipv6mr_multiaddr.s6_addr[0xD] = a->b[0xD];
                i6mr.ipv6mr_multiaddr.s6_addr[0xE] = a->b[0xE];
                i6mr.ipv6mr_multiaddr.s6_addr[0xF] = a->b[0xF];

                // Do precautionary IPV6_LEAVE_GROUP first, necessary to clear stale kernel state
                mStatus err = setsockopt(x->BPF_mcfd, IPPROTO_IPV6, IPV6_LEAVE_GROUP, &i6mr, sizeof(i6mr));
                if (err < 0 && (errno != EADDRNOTAVAIL))
                    LogMsg("mDNSPlatformUpdateProxyList: IPV6_LEAVE_GROUP error %d errno %d (%s) group %.16a on %u", err, errno, strerror(errno), &i6mr.ipv6mr_multiaddr, i6mr.ipv6mr_interface);

                err = setsockopt(x->BPF_mcfd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &i6mr, sizeof(i6mr));
                if (err < 0 && (errno != EADDRINUSE))   // Joining same group twice can give "Address already in use" error -- no need to report that
                    LogMsg("mDNSPlatformUpdateProxyList: IPV6_JOIN_GROUP error %d errno %d (%s) group %.16a on %u", err, errno, strerror(errno), &i6mr.ipv6mr_multiaddr, i6mr.ipv6mr_interface);

                LogSPS("Joined IPv6 ND multicast group %.16a for %.16a", &i6mr.ipv6mr_multiaddr, a);
            }

        if (pc != fail) LogMsg("mDNSPlatformUpdateProxyList: pc %p != fail %p", pc, fail);
        *pc++ = rf;     // fail points here

        if (pc != ret4) LogMsg("mDNSPlatformUpdateProxyList: pc %p != ret4 %p", pc, ret4);
        *pc++ = r4a;    // ret4 points here
        *pc++ = r4b;
        *pc++ = r4c;
        *pc++ = r4d;

        if (pc != ret6) LogMsg("mDNSPlatformUpdateProxyList: pc %p != ret6 %p", pc, ret6);
        *pc++ = r6a;    // ret6 points here
#if 0
        // For debugging BPF filter program
        unsigned int q;
        for (q=0; q<prog.bf_len; q++)
            LogSPS("mDNSPlatformUpdateProxyList: %2d { 0x%02x, %d, %d, 0x%08x },", q, prog.bf_insns[q].code, prog.bf_insns[q].jt, prog.bf_insns[q].jf, prog.bf_insns[q].k);
#endif
        prog.bf_len   = (u_int)(pc - filter);
        prog.bf_insns = filter;
    }

    if (ioctl(x->BPF_fd, BIOCSETFNR, &prog) < 0) LogMsg("mDNSPlatformUpdateProxyList: BIOCSETFNR(%d) failed %d (%s)", prog.bf_len, errno, strerror(errno));
    else LogSPS("mDNSPlatformUpdateProxyList: BIOCSETFNR(%d) successful", prog.bf_len);
}

mDNSexport void mDNSPlatformReceiveBPF_fd(int fd)
{
    mDNS *const m = &mDNSStorage;
    mDNS_Lock(m);

    NetworkInterfaceInfoOSX *i;
    for (i = m->p->InterfaceList; i; i = i->next) if (i->BPF_fd == -2) break;
    if (!i) { LogSPS("mDNSPlatformReceiveBPF_fd: No Interfaces awaiting BPF fd %d; closing", fd); close(fd); }
    else
    {
        LogSPS("%s using   BPF fd %d", i->ifinfo.ifname, fd);

        struct bpf_version v;
        if (ioctl(fd, BIOCVERSION, &v) < 0)
            LogMsg("mDNSPlatformReceiveBPF_fd: %d %s BIOCVERSION failed %d (%s)", fd, i->ifinfo.ifname, errno, strerror(errno));
        else if (BPF_MAJOR_VERSION != v.bv_major || BPF_MINOR_VERSION != v.bv_minor)
            LogMsg("mDNSPlatformReceiveBPF_fd: %d %s BIOCVERSION header %d.%d kernel %d.%d",
                   fd, i->ifinfo.ifname, BPF_MAJOR_VERSION, BPF_MINOR_VERSION, v.bv_major, v.bv_minor);

        if (ioctl(fd, BIOCGBLEN, &i->BPF_len) < 0)
            LogMsg("mDNSPlatformReceiveBPF_fd: %d %s BIOCGBLEN failed %d (%s)", fd, i->ifinfo.ifname, errno, strerror(errno));

        if (i->BPF_len > sizeof(m->imsg))
        {
            i->BPF_len = sizeof(m->imsg);
            if (ioctl(fd, BIOCSBLEN, &i->BPF_len) < 0)
                LogMsg("mDNSPlatformReceiveBPF_fd: %d %s BIOCSBLEN failed %d (%s)", fd, i->ifinfo.ifname, errno, strerror(errno));
            else
                LogSPS("mDNSPlatformReceiveBPF_fd: %d %s BIOCSBLEN %d", fd, i->ifinfo.ifname, i->BPF_len);
        }

        static const u_int opt_one = 1;
        if (ioctl(fd, BIOCIMMEDIATE, &opt_one) < 0)
            LogMsg("mDNSPlatformReceiveBPF_fd: %d %s BIOCIMMEDIATE failed %d (%s)", fd, i->ifinfo.ifname, errno, strerror(errno));

        //if (ioctl(fd, BIOCPROMISC, &opt_one) < 0)
        //  LogMsg("mDNSPlatformReceiveBPF_fd: %d %s BIOCPROMISC failed %d (%s)", fd, i->ifinfo.ifname, errno, strerror(errno));

        //if (ioctl(fd, BIOCSHDRCMPLT, &opt_one) < 0)
        //  LogMsg("mDNSPlatformReceiveBPF_fd: %d %s BIOCSHDRCMPLT failed %d (%s)", fd, i->ifinfo.ifname, errno, strerror(errno));

        /*  <rdar://problem/10287386>
         *  make socket non blocking see comments in bpf_callback_common for more info
         */
        if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) < 0) // set non-blocking
        {
            LogMsg("mDNSPlatformReceiveBPF_fd: %d %s O_NONBLOCK failed %d (%s)", fd, i->ifinfo.ifname, errno, strerror(errno));
        }

        struct ifreq ifr;
        mDNSPlatformMemZero(&ifr, sizeof(ifr));
        strlcpy(ifr.ifr_name, i->ifinfo.ifname, sizeof(ifr.ifr_name));
        if (ioctl(fd, BIOCSETIF, &ifr) < 0)
        { LogMsg("mDNSPlatformReceiveBPF_fd: %d %s BIOCSETIF failed %d (%s)", fd, i->ifinfo.ifname, errno, strerror(errno)); i->BPF_fd = -3; }
        else
        {
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
            i->BPF_fd  = fd;
            i->BPF_source = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, fd, 0, dispatch_get_main_queue());
            if (!i->BPF_source) {LogMsg("mDNSPlatformReceiveBPF_fd: dispatch source create failed"); return;}
            dispatch_source_set_event_handler(i->BPF_source, ^{bpf_callback_dispatch(i);});
            dispatch_source_set_cancel_handler(i->BPF_source, ^{close(fd);});
            dispatch_resume(i->BPF_source);
#else
            CFSocketContext myCFSocketContext = { 0, i, NULL, NULL, NULL };
            i->BPF_fd  = fd;
            i->BPF_cfs = CFSocketCreateWithNative(kCFAllocatorDefault, fd, kCFSocketReadCallBack, bpf_callback, &myCFSocketContext);
            i->BPF_rls = CFSocketCreateRunLoopSource(kCFAllocatorDefault, i->BPF_cfs, 0);
            CFRunLoopAddSource(CFRunLoopGetMain(), i->BPF_rls, kCFRunLoopDefaultMode);
#endif
            mDNSPlatformUpdateProxyList(i->ifinfo.InterfaceID);
        }
    }

    mDNS_Unlock(m);
}

#endif // APPLE_OSX_mDNSResponder

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Key Management
#endif

#ifndef NO_SECURITYFRAMEWORK
mDNSlocal CFArrayRef CopyCertChain(SecIdentityRef identity)
{
    CFMutableArrayRef certChain = NULL;
    if (!identity) { LogMsg("CopyCertChain: identity is NULL"); return(NULL); }
    SecCertificateRef cert;
    OSStatus err = SecIdentityCopyCertificate(identity, &cert);
    if (err || !cert) LogMsg("CopyCertChain: SecIdentityCopyCertificate() returned %d", (int) err);
    else
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        SecPolicySearchRef searchRef;
        err = SecPolicySearchCreate(CSSM_CERT_X_509v3, &CSSMOID_APPLE_X509_BASIC, NULL, &searchRef);
        if (err || !searchRef) LogMsg("CopyCertChain: SecPolicySearchCreate() returned %d", (int) err);
        else
        {
            SecPolicyRef policy;
            err = SecPolicySearchCopyNext(searchRef, &policy);
            if (err || !policy) LogMsg("CopyCertChain: SecPolicySearchCopyNext() returned %d", (int) err);
            else
            {
                CFArrayRef wrappedCert = CFArrayCreate(NULL, (const void**) &cert, 1, &kCFTypeArrayCallBacks);
                if (!wrappedCert) LogMsg("CopyCertChain: wrappedCert is NULL");
                else
                {
                    SecTrustRef trust;
                    err = SecTrustCreateWithCertificates(wrappedCert, policy, &trust);
                    if (err || !trust) LogMsg("CopyCertChain: SecTrustCreateWithCertificates() returned %d", (int) err);
                    else
                    {
                        err = SecTrustEvaluate(trust, NULL);
                        if (err) LogMsg("CopyCertChain: SecTrustEvaluate() returned %d", (int) err);
                        else
                        {
                            CFArrayRef rawCertChain;
                            CSSM_TP_APPLE_EVIDENCE_INFO *statusChain = NULL;
                            err = SecTrustGetResult(trust, NULL, &rawCertChain, &statusChain);
                            if (err || !rawCertChain || !statusChain) LogMsg("CopyCertChain: SecTrustGetResult() returned %d", (int) err);
                            else
                            {
                                certChain = CFArrayCreateMutableCopy(NULL, 0, rawCertChain);
                                if (!certChain) LogMsg("CopyCertChain: certChain is NULL");
                                else
                                {
                                    // Replace the SecCertificateRef at certChain[0] with a SecIdentityRef per documentation for SSLSetCertificate:
                                    // <http://devworld.apple.com/documentation/Security/Reference/secureTransportRef/index.html>
                                    CFArraySetValueAtIndex(certChain, 0, identity);
                                    // Remove root from cert chain, but keep any and all intermediate certificates that have been signed by the root certificate
                                    if (CFArrayGetCount(certChain) > 1) CFArrayRemoveValueAtIndex(certChain, CFArrayGetCount(certChain) - 1);
                                }
                                CFRelease(rawCertChain);
                                // Do not free statusChain:
                                // <http://developer.apple.com/documentation/Security/Reference/certifkeytrustservices/Reference/reference.html> says:
                                // certChain: Call the CFRelease function to release this object when you are finished with it.
                                // statusChain: Do not attempt to free this pointer; it remains valid until the trust management object is released...
                            }
                        }
                        CFRelease(trust);
                    }
                    CFRelease(wrappedCert);
                }
                CFRelease(policy);
            }
            CFRelease(searchRef);
        }
#pragma clang diagnostic pop
        CFRelease(cert);
    }
    return certChain;
}
#endif /* NO_SECURITYFRAMEWORK */

mDNSexport mStatus mDNSPlatformTLSSetupCerts(void)
{
#ifdef NO_SECURITYFRAMEWORK
    return mStatus_UnsupportedErr;
#else
    SecIdentityRef identity = nil;
    SecIdentitySearchRef srchRef = nil;
    OSStatus err;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    // search for "any" identity matching specified key use
    // In this app, we expect there to be exactly one
    err = SecIdentitySearchCreate(NULL, CSSM_KEYUSE_DECRYPT, &srchRef);
    if (err) { LogMsg("ERROR: mDNSPlatformTLSSetupCerts: SecIdentitySearchCreate returned %d", (int) err); return err; }

    err = SecIdentitySearchCopyNext(srchRef, &identity);
    if (err) { LogMsg("ERROR: mDNSPlatformTLSSetupCerts: SecIdentitySearchCopyNext returned %d", (int) err); return err; }
#pragma clang diagnostic pop

    if (CFGetTypeID(identity) != SecIdentityGetTypeID())
    { LogMsg("ERROR: mDNSPlatformTLSSetupCerts: SecIdentitySearchCopyNext CFTypeID failure"); return mStatus_UnknownErr; }

    // Found one. Call CopyCertChain to create the correct certificate chain.
    ServerCerts = CopyCertChain(identity);
    if (ServerCerts == nil) { LogMsg("ERROR: mDNSPlatformTLSSetupCerts: CopyCertChain error"); return mStatus_UnknownErr; }

    return mStatus_NoError;
#endif /* NO_SECURITYFRAMEWORK */
}

mDNSexport void  mDNSPlatformTLSTearDownCerts(void)
{
#ifndef NO_SECURITYFRAMEWORK
    if (ServerCerts) { CFRelease(ServerCerts); ServerCerts = NULL; }
#endif /* NO_SECURITYFRAMEWORK */
}


mDNSlocal void mDNSDomainLabelFromCFString(CFStringRef cfs, domainlabel *const namelabel);

// This gets the text of the field currently labelled "Computer Name" in the Sharing Prefs Control Panel
mDNSlocal void GetUserSpecifiedFriendlyComputerName(domainlabel *const namelabel)
{
    CFStringEncoding encoding = kCFStringEncodingUTF8;
    CFStringRef cfs = SCDynamicStoreCopyComputerName(NULL, &encoding);

    if (cfs == mDNSNULL) {
        return;
    }

    mDNSDomainLabelFromCFString(cfs, namelabel);

    CFRelease(cfs);
}

mDNSlocal void GetUserSpecifiedLocalHostName(domainlabel *const namelabel)
{
    CFStringRef cfs = SCDynamicStoreCopyLocalHostName(NULL);

    if (cfs == mDNSNULL) {
        return;
    }

    mDNSDomainLabelFromCFString(cfs, namelabel);

    CFRelease(cfs);
}

mDNSlocal void mDNSDomainLabelFromCFString(CFStringRef cfs, domainlabel *const namelabel)
{
    CFIndex num_of_bytes_write = 0;
    CFStringGetBytes(cfs, CFRangeMake(0, CFStringGetLength(cfs)), kCFStringEncodingUTF8, 0, FALSE, namelabel->c + 1, sizeof(*namelabel) - 1, &num_of_bytes_write);
    namelabel->c[0] = num_of_bytes_write;
}

mDNSexport mDNSBool DictionaryIsEnabled(CFDictionaryRef dict)
{
    mDNSs32 val;
    CFNumberRef state = (CFNumberRef)CFDictionaryGetValue(dict, CFSTR("Enabled"));
    if (state == NULL) return mDNSfalse;
    if (!CFNumberGetValue(state, kCFNumberSInt32Type, &val))
    { LogMsg("ERROR: DictionaryIsEnabled - CFNumberGetValue"); return mDNSfalse; }
    return val ? mDNStrue : mDNSfalse;
}

mDNSlocal mStatus SetupAddr(mDNSAddr *ip, const struct sockaddr *const sa)
{
    if (!sa) { LogMsg("SetupAddr ERROR: NULL sockaddr"); return(mStatus_Invalid); }

    if (sa->sa_family == AF_INET)
    {
        struct sockaddr_in *ifa_addr = (struct sockaddr_in *)sa;
        ip->type = mDNSAddrType_IPv4;
        ip->ip.v4.NotAnInteger = ifa_addr->sin_addr.s_addr;
        return(mStatus_NoError);
    }

    if (sa->sa_family == AF_INET6)
    {
        struct sockaddr_in6 *ifa_addr = (struct sockaddr_in6 *)sa;
        // Inside the BSD kernel they use a hack where they stuff the sin6->sin6_scope_id
        // value into the second word of the IPv6 link-local address, so they can just
        // pass around IPv6 address structures instead of full sockaddr_in6 structures.
        // Those hacked IPv6 addresses aren't supposed to escape the kernel in that form, but they do.
        // To work around this we always whack the second word of any IPv6 link-local address back to zero.
        if (IN6_IS_ADDR_LINKLOCAL(&ifa_addr->sin6_addr)) ifa_addr->sin6_addr.__u6_addr.__u6_addr16[1] = 0;
        ip->type = mDNSAddrType_IPv6;
        ip->ip.v6 = *(mDNSv6Addr*)&ifa_addr->sin6_addr;
        return(mStatus_NoError);
    }

    LogMsg("SetupAddr invalid sa_family %d", sa->sa_family);
    return(mStatus_Invalid);
}

mDNSlocal mDNSEthAddr GetBSSID(char *ifa_name)
{
    mDNSEthAddr eth = zeroEthAddr;

    CFStringRef entityname = CFStringCreateWithFormat(NULL, NULL, CFSTR("State:/Network/Interface/%s/AirPort"), ifa_name);
    if (entityname)
    {
        CFDictionaryRef dict = SCDynamicStoreCopyValue(NULL, entityname);
        if (dict)
        {
            CFRange range = { 0, 6 };       // Offset, length
            CFDataRef data = CFDictionaryGetValue(dict, CFSTR("BSSID"));
            if (data && CFDataGetLength(data) == 6)
                CFDataGetBytes(data, range, eth.b);
            CFRelease(dict);
        }
        CFRelease(entityname);
    }

    return(eth);
}

mDNSlocal int GetMAC(mDNSEthAddr *eth, u_short ifindex)
{
    struct ifaddrs *ifa;
    for (ifa = myGetIfAddrs(0); ifa; ifa = ifa->ifa_next)
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_LINK)
        {
            const struct sockaddr_dl *const sdl = (const struct sockaddr_dl *)ifa->ifa_addr;
            if (sdl->sdl_index == ifindex)
            { mDNSPlatformMemCopy(eth->b, sdl->sdl_data + sdl->sdl_nlen, 6); return 0; }
        }
    *eth = zeroEthAddr;
    return -1;
}

#ifndef SIOCGIFWAKEFLAGS
#define SIOCGIFWAKEFLAGS _IOWR('i', 136, struct ifreq) /* get interface wake property flags */
#endif

#ifndef IF_WAKE_ON_MAGIC_PACKET
#define IF_WAKE_ON_MAGIC_PACKET 0x01
#endif

#ifndef ifr_wake_flags
#define ifr_wake_flags ifr_ifru.ifru_intval
#endif

mDNSlocal
kern_return_t
RegistryEntrySearchCFPropertyAndIOObject( io_registry_entry_t     entry,
                                          const io_name_t         plane,
                                          CFStringRef             keystr,
                                          CFTypeRef *             outProperty,
                                          io_registry_entry_t *   outEntry)
{
    kern_return_t       kr;

    IOObjectRetain(entry);
    while (entry)
    {
        CFTypeRef ref = IORegistryEntryCreateCFProperty(entry, keystr, kCFAllocatorDefault, mDNSNULL);
        if (ref)
        {
            if (outProperty) *outProperty = ref;
            else             CFRelease(ref);
            break;
        }
        io_registry_entry_t parent;
        kr = IORegistryEntryGetParentEntry(entry, plane, &parent);
        if (kr != KERN_SUCCESS) parent = mDNSNULL;
        IOObjectRelease(entry);
        entry = parent;
    }
    if (!entry)          kr = kIOReturnNoDevice;
    else
    {
        if (outEntry)   *outEntry = entry;
        else            IOObjectRelease(entry);
        kr = KERN_SUCCESS;
    }
    return(kr);
}

mDNSlocal mDNSBool  CheckInterfaceSupport(NetworkInterfaceInfo *const intf, const char *key)
{
    io_service_t service = IOServiceGetMatchingService(kIOMasterPortDefault, IOBSDNameMatching(kIOMasterPortDefault, 0, intf->ifname));
    if (!service)
    {
        LogSPS("CheckInterfaceSupport: No service for interface %s", intf->ifname);
        return mDNSfalse;
    }

    mDNSBool    ret    = mDNSfalse;

    CFStringRef keystr =  CFStringCreateWithCString(NULL, key, kCFStringEncodingUTF8);
    kern_return_t kr = RegistryEntrySearchCFPropertyAndIOObject(service, kIOServicePlane, keystr, mDNSNULL, mDNSNULL);
    CFRelease(keystr);
    if (kr == KERN_SUCCESS) ret = mDNStrue;
    else
    {
        io_name_t n1;
        IOObjectGetClass(service, n1);
        LogRedact(MDNS_LOG_CATEGORY_SPS, MDNS_LOG_INFO,
            "CheckInterfaceSupport: No " PUB_S " for interface " PUB_S "/" PUB_S " kr 0x%X", key, intf->ifname, n1, kr);
        ret = mDNSfalse;
    }

    IOObjectRelease(service);
    return ret;
}


#if !TARGET_OS_WATCH
mDNSlocal  mDNSBool InterfaceSupportsKeepAlive(NetworkInterfaceInfo *const intf)
{
    return CheckInterfaceSupport(intf, mDNS_IOREG_KA_KEY);
}
#endif

mDNSlocal mDNSBool NetWakeInterface(NetworkInterfaceInfoOSX *i)
{
#if TARGET_OS_WATCH
    (void) i;   // unused
    return(mDNSfalse);
#else
    // We only use Sleep Proxy Service on multicast-capable interfaces, except loopback and D2D.
    if (!MulticastInterface(i) || (i->ifa_flags & IFF_LOOPBACK) || i->D2DInterface)
    {
        LogRedact(MDNS_LOG_CATEGORY_SPS, MDNS_LOG_DEBUG,
            "NetWakeInterface: returning false for " PUB_S, i->ifinfo.ifname);
        return(mDNSfalse);
    }

    // If the interface supports TCPKeepalive, it is capable of waking up for a magic packet
    // This check is needed since the SIOCGIFWAKEFLAGS ioctl returns wrong values for WOMP capability
    // when the power source is not AC Power.
    if (InterfaceSupportsKeepAlive(&i->ifinfo))
    {
        LogSPS("NetWakeInterface: %s supports TCP Keepalive returning true", i->ifinfo.ifname);
        return mDNStrue;
    }

    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) { LogMsg("NetWakeInterface socket failed %s error %d errno %d (%s)", i->ifinfo.ifname, s, errno, strerror(errno)); return(mDNSfalse); }

    struct ifreq ifr;
    strlcpy(ifr.ifr_name, i->ifinfo.ifname, sizeof(ifr.ifr_name));
    if (ioctl(s, SIOCGIFWAKEFLAGS, &ifr) < 0)
    {
        const int ioctl_errno = errno;
        // For some strange reason, in /usr/include/sys/errno.h, EOPNOTSUPP is defined to be
        // 102 when compiling kernel code, and 45 when compiling user-level code. Since this
        // error code is being returned from the kernel, we need to use the kernel version.
        #define KERNEL_EOPNOTSUPP 102
        if (ioctl_errno != KERNEL_EOPNOTSUPP) // "Operation not supported on socket", the expected result on Leopard and earlier
            LogMsg("NetWakeInterface SIOCGIFWAKEFLAGS %s errno %d (%s)", i->ifinfo.ifname, ioctl_errno, strerror(ioctl_errno));
        // If on Leopard or earlier, we get EOPNOTSUPP, so in that case
        // we enable WOL if this interface is not AirPort and "Wake for Network access" is turned on.
        ifr.ifr_wake_flags = (ioctl_errno == KERNEL_EOPNOTSUPP && !(i)->BSSID.l[0] && i->m->SystemWakeOnLANEnabled) ? IF_WAKE_ON_MAGIC_PACKET : 0;
    }

    close(s);

    // ifr.ifr_wake_flags = IF_WAKE_ON_MAGIC_PACKET;    // For testing with MacBook Air, using a USB dongle that doesn't actually support Wake-On-LAN

    LogRedact(MDNS_LOG_CATEGORY_SPS, MDNS_LOG_INFO,
        "NetWakeInterface: " PUB_S " " PRI_IP_ADDR " " PUB_S " WOMP",
        i->ifinfo.ifname, &i->ifinfo.ip, (ifr.ifr_wake_flags & IF_WAKE_ON_MAGIC_PACKET) ? "supports" : "no");

    return((ifr.ifr_wake_flags & IF_WAKE_ON_MAGIC_PACKET) != 0);
#endif  // TARGET_OS_WATCH
}

mDNSlocal u_int64_t getExtendedFlags(const char *ifa_name)
{
    int sockFD;
    struct ifreq ifr;

    sockFD = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockFD < 0)
    {
        LogMsg("getExtendedFlags: socket() call failed, errno = %d (%s)", errno, strerror(errno));
        return 0;
    }

    ifr.ifr_addr.sa_family = AF_INET;
    strlcpy(ifr.ifr_name, ifa_name, sizeof(ifr.ifr_name));

    if (ioctl(sockFD, SIOCGIFEFLAGS, (caddr_t)&ifr) == -1)
    {
        LogMsg("getExtendedFlags: SIOCGIFEFLAGS failed for %s, errno = %d (%s)", ifa_name, errno, strerror(errno));
        ifr.ifr_eflags = 0;
    }

    close(sockFD);
    return ifr.ifr_eflags;
}

mDNSlocal mDNSBool isExcludedInterface(int sockFD, char * ifa_name)
{
    struct ifreq ifr;

    // llw0 interface is excluded from Bonjour discover.
    // There currently is no interface attributed based way to identify this interface
    // until rdar://problem/47933782 is addressed.
    if (strncmp(ifa_name, "llw", 3) == 0)
    {
        LogMsg("isExcludedInterface: excluding %s", ifa_name);
        return mDNStrue;
    }

    // Coprocessor interfaces are also excluded.
    if (sockFD < 0)
    {
        LogMsg("isExcludedInterface: invalid socket FD passed: %d", sockFD);
        return mDNSfalse;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    strlcpy(ifr.ifr_name, ifa_name, sizeof(ifr.ifr_name));

    if (ioctl(sockFD, SIOCGIFFUNCTIONALTYPE, (caddr_t)&ifr) == -1)
    {
        LogMsg("isExcludedInterface: SIOCGIFFUNCTIONALTYPE failed, errno = %d (%s)", errno, strerror(errno));
        return mDNSfalse;
    }

    if (ifr.ifr_functional_type == IFRTYPE_FUNCTIONAL_INTCOPROC)
    {
        LogMsg("isExcludedInterface: excluding coprocessor interface %s", ifa_name);
        return mDNStrue;
    }
    else
        return mDNSfalse;
}

#if TARGET_OS_IPHONE

// Function pointers for the routines we use in the MobileWiFi framework.
static WiFiManagerClientRef (*WiFiManagerClientCreate_p)(CFAllocatorRef allocator, WiFiClientType type) = mDNSNULL;
static CFArrayRef (*WiFiManagerClientCopyDevices_p)(WiFiManagerClientRef manager) = mDNSNULL;
static WiFiNetworkRef (*WiFiDeviceClientCopyCurrentNetwork_p)(WiFiDeviceClientRef device) = mDNSNULL;
static bool (*WiFiNetworkIsCarPlay_p)(WiFiNetworkRef network) = mDNSNULL;

mDNSlocal mDNSBool MobileWiFiLibLoad(void)
{
    static mDNSBool isInitialized = mDNSfalse;
    static void *MobileWiFiLib_p = mDNSNULL;
    static const char path[] = "/System/Library/PrivateFrameworks/MobileWiFi.framework/MobileWiFi";

    if (!isInitialized)
    {
        if (!MobileWiFiLib_p)
        {
            MobileWiFiLib_p = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
            if (!MobileWiFiLib_p)
            {
                LogInfo("MobileWiFiLibLoad: dlopen() failed.");
                goto exit;
            }
        }

        if (!WiFiManagerClientCreate_p)
        {
            WiFiManagerClientCreate_p = dlsym(MobileWiFiLib_p, "WiFiManagerClientCreate");
            if (!WiFiManagerClientCreate_p)
            {
                LogInfo("MobileWiFiLibLoad: load of WiFiManagerClientCreate symbol failed.");
                goto exit;
            }
        }

        if (!WiFiManagerClientCopyDevices_p)
        {
            WiFiManagerClientCopyDevices_p = dlsym(MobileWiFiLib_p, "WiFiManagerClientCopyDevices");
            if (!WiFiManagerClientCopyDevices_p)
            {
                LogInfo("MobileWiFiLibLoad: load of WiFiManagerClientCopyDevices symbol failed.");
                goto exit;
            }
        }

        if (!WiFiDeviceClientCopyCurrentNetwork_p)
        {
            WiFiDeviceClientCopyCurrentNetwork_p = dlsym(MobileWiFiLib_p, "WiFiDeviceClientCopyCurrentNetwork");
            if (!WiFiDeviceClientCopyCurrentNetwork_p)
            {
                LogInfo("MobileWiFiLibLoad: load of WiFiDeviceClientCopyCurrentNetwork symbol failed.");
                goto exit;
            }
        }

        if (!WiFiNetworkIsCarPlay_p)
        {
            WiFiNetworkIsCarPlay_p = dlsym(MobileWiFiLib_p, "WiFiNetworkIsCarPlay");
            if (!WiFiNetworkIsCarPlay_p)
            {
                LogInfo("MobileWiFiLibLoad: load of WiFiNetworkIsCarPlay symbol failed.");
                goto exit;
            }
        }

        isInitialized = mDNStrue;
    }

exit:
    return isInitialized;
}

#define CARPLAY_DEBUG 0

// Return true if the interface is associate to a CarPlay hosted SSID.
// If we have associated with a CarPlay hosted SSID, then use the same 
// optimizations that are used when an interface has the IFEF_DIRECTLINK flag set.
mDNSlocal mDNSBool IsCarPlaySSID(char *ifa_name)
{
    static WiFiManagerClientRef manager = NULL;
    CFArrayRef              devices;
    WiFiDeviceClientRef     device;
    WiFiNetworkRef          network;
    mDNSBool                rvalue = mDNSfalse;

    if (!MobileWiFiLibLoad())
    {
        LogInfo("IsCarPlaySSID: MobileWiFiLibLoad() failed!");
        return mDNSfalse;
    }

    // Cache the WiFiManagerClientRef.
    if (manager == NULL)
        manager = WiFiManagerClientCreate_p(NULL, kWiFiClientTypeNormal);

    if (manager == NULL)
    {
        LogInfo("IsCarPlaySSID: WiFiManagerClientCreate() failed!");
        return mDNSfalse;
    }

    devices = WiFiManagerClientCopyDevices_p(manager);

    // If the first call fails, update the cached WiFiManagerClientRef pointer and try again.
    if (devices == NULL)
    {
        LogInfo("IsCarPlaySSID: First call to WiFiManagerClientCopyDevices() returned NULL for %s", ifa_name);

        // Release the previously cached WiFiManagerClientRef which is apparently now stale.
        CFRelease(manager);
        manager = WiFiManagerClientCreate_p(NULL, kWiFiClientTypeNormal);
        if (manager == NULL)
        {
            LogInfo("IsCarPlaySSID: WiFiManagerClientCreate() failed!");
            return mDNSfalse;
        }
        devices = WiFiManagerClientCopyDevices_p(manager);
        if (devices == NULL)
        {
            LogInfo("IsCarPlaySSID: Second call to WiFiManagerClientCopyDevices() returned NULL for %s", ifa_name);
            return mDNSfalse;
        }
    }

    device = (WiFiDeviceClientRef)CFArrayGetValueAtIndex(devices, 0);
    network = WiFiDeviceClientCopyCurrentNetwork_p(device);
    if (network != NULL)
    {
        if (WiFiNetworkIsCarPlay_p(network))
        {
            LogInfo("IsCarPlaySSID: %s is CarPlay hosted", ifa_name);
            rvalue = mDNStrue;
        }
#if CARPLAY_DEBUG
        else
            LogInfo("IsCarPlaySSID: %s is NOT CarPlay hosted", ifa_name);
#endif // CARPLAY_DEBUG

        CFRelease(network);
    }
    else
        LogInfo("IsCarPlaySSID: WiFiDeviceClientCopyCurrentNetwork() returned NULL for %s", ifa_name);

    CFRelease(devices);

    return rvalue;
}

#else   // TARGET_OS_IPHONE

mDNSlocal mDNSBool IsCarPlaySSID(char *ifa_name)
{
    (void)ifa_name;  // unused

    // OSX WifiManager currently does not implement WiFiNetworkIsCarPlay()
    return mDNSfalse;;
}

#endif  // TARGET_OS_IPHONE

// Returns pointer to newly created NetworkInterfaceInfoOSX object, or
// pointer to already-existing NetworkInterfaceInfoOSX object found in list, or
// may return NULL if out of memory (unlikely) or parameters are invalid for some reason
// (e.g. sa_family not AF_INET or AF_INET6)

mDNSlocal NetworkInterfaceInfoOSX *AddInterfaceToList(struct ifaddrs *ifa, mDNSs32 utc)
{
    mDNS *const m = &mDNSStorage;
    mDNSu32 scope_id  = if_nametoindex(ifa->ifa_name);
    mDNSEthAddr bssid = GetBSSID(ifa->ifa_name);
    u_int64_t   eflags = getExtendedFlags(ifa->ifa_name);

    mDNSAddr ip, mask;
    if (SetupAddr(&ip,   ifa->ifa_addr   ) != mStatus_NoError) return(NULL);
    if (SetupAddr(&mask, ifa->ifa_netmask) != mStatus_NoError) return(NULL);

    NetworkInterfaceInfoOSX **p;
    for (p = &m->p->InterfaceList; *p; p = &(*p)->next)
        if (scope_id == (*p)->scope_id &&
            mDNSSameAddress(&ip, &(*p)->ifinfo.ip) &&
            mDNSSameEthAddress(&bssid, &(*p)->BSSID))
        {
            debugf("AddInterfaceToList: Found existing interface %lu %.6a with address %#a at %p, ifname before %s, after %s", scope_id, &bssid, &ip, *p, (*p)->ifinfo.ifname, ifa->ifa_name);
            // The name should be updated to the new name so that we don't report a wrong name in our SIGINFO output.
            // When interfaces are created with same MAC address, kernel resurrects the old interface.
            // Even though the interface index is the same (which should be sufficient), when we receive a UDP packet
            // we get the corresponding name for the interface index on which the packet was received and check against
            // the InterfaceList for a matching name. So, keep the name in sync
            strlcpy((*p)->ifinfo.ifname, ifa->ifa_name, sizeof((*p)->ifinfo.ifname));

            // Determine if multicast state has changed.
            const mDNSBool txrx = MulticastInterface(*p);
            if ((*p)->ifinfo.McastTxRx != txrx)
            {
                (*p)->ifinfo.McastTxRx = txrx;
                (*p)->Exists = MulticastStateChanged; // State change; need to deregister and reregister this interface
            }
            else
                 (*p)->Exists = mDNStrue;

            // If interface was not in getifaddrs list last time we looked, but it is now, update 'AppearanceTime' for this record
            if ((*p)->LastSeen != utc) (*p)->AppearanceTime = utc;

            // If Wake-on-LAN capability of this interface has changed (e.g. because power cable on laptop has been disconnected)
            // we may need to start or stop or sleep proxy browse operation
            const mDNSBool NetWake = NetWakeInterface(*p);
            if ((*p)->ifinfo.NetWake != NetWake)
            {
                (*p)->ifinfo.NetWake = NetWake;
                // If this interface is already registered with mDNSCore, then we need to start or stop its NetWake browse on-the-fly.
                // If this interface is not already registered (i.e. it's a dormant interface we had in our list
                // from when we previously saw it) then we mustn't do that, because mDNSCore doesn't know about it yet.
                // In this case, the mDNS_RegisterInterface() call will take care of starting the NetWake browse if necessary.
                if ((*p)->Registered)
                {
                    mDNS_Lock(m);
                    if (NetWake) mDNS_ActivateNetWake_internal  (m, &(*p)->ifinfo);
                    else         mDNS_DeactivateNetWake_internal(m, &(*p)->ifinfo);
                    mDNS_Unlock(m);
                }
            }
            // Reset the flag if it has changed this time.
            (*p)->ifinfo.IgnoreIPv4LL = ((eflags & IFEF_ARPLL) != 0) ? mDNSfalse : mDNStrue;

            return(*p);
        }

    NetworkInterfaceInfoOSX *i = (NetworkInterfaceInfoOSX *) callocL("NetworkInterfaceInfoOSX", sizeof(*i));
    debugf("AddInterfaceToList: Making   new   interface %lu %.6a with address %#a at %p", scope_id, &bssid, &ip, i);
    if (!i) return(mDNSNULL);
    i->ifinfo.InterfaceID = (mDNSInterfaceID)(uintptr_t)scope_id;
    i->ifinfo.ip          = ip;
    i->ifinfo.mask        = mask;
    strlcpy(i->ifinfo.ifname, ifa->ifa_name, sizeof(i->ifinfo.ifname));
    i->ifinfo.ifname[sizeof(i->ifinfo.ifname)-1] = 0;
    // We can be configured to disable multicast advertisement, but we want to to support
    // local-only services, which need a loopback address record.
    i->ifinfo.Advertise   = m->DivertMulticastAdvertisements ? ((ifa->ifa_flags & IFF_LOOPBACK) ? mDNStrue : mDNSfalse) : m->AdvertiseLocalAddresses;
    i->ifinfo.Loopback    = ((ifa->ifa_flags & IFF_LOOPBACK) != 0) ? mDNStrue : mDNSfalse;
    i->ifinfo.IgnoreIPv4LL = ((eflags & IFEF_ARPLL) != 0) ? mDNSfalse : mDNStrue;

    // Setting DirectLink indicates we can do the optimization of skipping the probe phase 
    // for the interface address records since they should be unique.
    // Unfortunately, the legacy p2p* interfaces do not set the IFEF_LOCALNET_PRIVATE 
    // or IFEF_DIRECTLINK flags, so we have to match against the name.
    if ((eflags & (IFEF_DIRECTLINK | IFEF_AWDL)) || (strncmp(i->ifinfo.ifname, "p2p", 3) == 0))
        i->ifinfo.DirectLink  = mDNStrue;
    else
        i->ifinfo.DirectLink  = IsCarPlaySSID(ifa->ifa_name);

    if (i->ifinfo.DirectLink)
        LogInfo("AddInterfaceToList: DirectLink set for %s", ifa->ifa_name);

    i->next            = mDNSNULL;
    i->m               = m;
    i->Exists          = mDNStrue;
    i->Flashing        = mDNSfalse;
    i->Occulting       = mDNSfalse;

    i->D2DInterface    = ((eflags & IFEF_LOCALNET_PRIVATE) || (strncmp(i->ifinfo.ifname, "p2p", 3) == 0)) ? mDNStrue: mDNSfalse;
    if (i->D2DInterface)
        LogInfo("AddInterfaceToList: D2DInterface set for %s", ifa->ifa_name);
    i->isAWDL          = (eflags & IFEF_AWDL)      ? mDNStrue: mDNSfalse;

    if (eflags & IFEF_AWDL)
    {
        // Set SupportsUnicastMDNSResponse false for the AWDL interface since unicast reserves
        // limited AWDL resources so we don't set the kDNSQClass_UnicastResponse bit in
        // Bonjour requests over the AWDL interface.
        i->ifinfo.SupportsUnicastMDNSResponse = mDNSfalse;
        AWDLInterfaceID = i->ifinfo.InterfaceID;
        LogInfo("AddInterfaceToList: AWDLInterfaceID = %d", (int) AWDLInterfaceID);
    }
    else
    {
        i->ifinfo.SupportsUnicastMDNSResponse = mDNStrue;
    }
    i->AppearanceTime  = utc;       // Brand new interface; AppearanceTime is now
    i->LastSeen        = utc;
    i->ifa_flags       = ifa->ifa_flags;
    i->scope_id        = scope_id;
    i->BSSID           = bssid;
    i->sa_family       = ifa->ifa_addr->sa_family;
    i->BPF_fd          = -1;
    i->BPF_mcfd        = -1;
    i->BPF_len         = 0;
    i->Registered      = mDNSNULL;

    // MulticastInterface() depends on the "m" and "ifa_flags" values being initialized above.
    i->ifinfo.McastTxRx   = MulticastInterface(i);
    // Do this AFTER i->BSSID has been set up
    i->ifinfo.NetWake  = (eflags & IFEF_EXPENSIVE)? mDNSfalse :  NetWakeInterface(i);
    GetMAC(&i->ifinfo.MAC, scope_id);
    if (i->ifinfo.NetWake && !i->ifinfo.MAC.l[0])
        LogMsg("AddInterfaceToList: Bad MAC address %.6a for %d %s %#a", &i->ifinfo.MAC, scope_id, i->ifinfo.ifname, &ip);

    *p = i;
    return(i);
}

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Power State & Configuration Change Management
#endif

mDNSlocal mStatus ReorderInterfaceList()
{
    // Disable Reorder lists till <rdar://problem/30071012> is fixed to prevent spurious name conflicts
#ifdef PR_30071012_FIXED
    mDNS *const m = &mDNSStorage;
    nwi_state_t state = nwi_state_copy();

    if (state == mDNSNULL)
    {
        LogMsg("NWI State is NULL!");
        return (mStatus_Invalid);
    }

    // Get the count of interfaces
    mDNSu32 count =  nwi_state_get_interface_names(state, mDNSNULL, 0);
    if (count == 0)
    {
        LogMsg("Unable to get the ordered list of interface names");
        nwi_state_release(state);
        return (mStatus_Invalid);
    }

    // Get the ordered interface list
    int i;
    const char *names[count];
    count = nwi_state_get_interface_names(state, names, count);

    NetworkInterfaceInfo *newList = mDNSNULL;
    for (i = count-1; i >= 0; i--)
    {   // Build a new ordered interface list
        NetworkInterfaceInfo **ptr = &m->HostInterfaces;
        while (*ptr != mDNSNULL )
        {
            if (strcmp((*ptr)->ifname, names[i]) == 0)
            {
                NetworkInterfaceInfo *node = *ptr;
                *ptr = (*ptr)->next;
                node->next = newList;
                newList = node;
            }
            else
                ptr = &((*ptr)->next);
        }
    }

    // Get to the end of the list
    NetworkInterfaceInfo *newListEnd = newList;
    while (newListEnd != mDNSNULL && newListEnd->next != mDNSNULL)
        newListEnd = newListEnd->next;

    // Add any remaing interfaces to the end of the sorted list
    if (newListEnd != mDNSNULL)
        newListEnd->next  = m->HostInterfaces;

    // If we have a valid new list, point to that now
    if (newList != mDNSNULL)
        m->HostInterfaces = newList;

    nwi_state_release(state);
#endif // PR_30071012_FIXED
    return (mStatus_NoError);
}

mDNSlocal mStatus UpdateInterfaceList(mDNSs32 utc)
{
    mDNS *const m = &mDNSStorage;
    struct ifaddrs *ifa = myGetIfAddrs(0);
    char defaultname[64];
    int InfoSocket = socket(AF_INET6, SOCK_DGRAM, 0);
    if (InfoSocket < 3 && errno != EAFNOSUPPORT) 
        LogMsg("UpdateInterfaceList: InfoSocket error %d errno %d (%s)", InfoSocket, errno, strerror(errno));

    if (m->SleepState == SleepState_Sleeping) ifa = NULL;

    while (ifa)
    {
#if LIST_ALL_INTERFACES
        if (ifa->ifa_addr)
        {
            if (ifa->ifa_addr->sa_family == AF_APPLETALK)
                LogMsg("UpdateInterfaceList: %5s(%d) Flags %04X Family %2d is AF_APPLETALK",
                       ifa->ifa_name, if_nametoindex(ifa->ifa_name), ifa->ifa_flags, ifa->ifa_addr->sa_family);
            else if (ifa->ifa_addr->sa_family == AF_LINK)
                LogMsg("UpdateInterfaceList: %5s(%d) Flags %04X Family %2d is AF_LINK",
                       ifa->ifa_name, if_nametoindex(ifa->ifa_name), ifa->ifa_flags, ifa->ifa_addr->sa_family);
            else if (ifa->ifa_addr->sa_family != AF_INET && ifa->ifa_addr->sa_family != AF_INET6)
                LogMsg("UpdateInterfaceList: %5s(%d) Flags %04X Family %2d not AF_INET (2) or AF_INET6 (30)",
                       ifa->ifa_name, if_nametoindex(ifa->ifa_name), ifa->ifa_flags, ifa->ifa_addr->sa_family);
        }
        else
            LogMsg("UpdateInterfaceList: %5s(%d) Flags %04X ifa_addr is NOT set",
                   ifa->ifa_name, if_nametoindex(ifa->ifa_name), ifa->ifa_flags);
        
        if (!(ifa->ifa_flags & IFF_UP))
            LogMsg("UpdateInterfaceList: %5s(%d) Flags %04X Family %2d Interface not IFF_UP",
                   ifa->ifa_name, if_nametoindex(ifa->ifa_name), ifa->ifa_flags,
                   ifa->ifa_addr ? ifa->ifa_addr->sa_family : 0);
        if (!(ifa->ifa_flags & IFF_MULTICAST))
            LogMsg("UpdateInterfaceList: %5s(%d) Flags %04X Family %2d Interface not IFF_MULTICAST",
                   ifa->ifa_name, if_nametoindex(ifa->ifa_name), ifa->ifa_flags,
                   ifa->ifa_addr ? ifa->ifa_addr->sa_family : 0);
        if (ifa->ifa_flags & IFF_POINTOPOINT)
            LogMsg("UpdateInterfaceList: %5s(%d) Flags %04X Family %2d Interface IFF_POINTOPOINT",
                   ifa->ifa_name, if_nametoindex(ifa->ifa_name), ifa->ifa_flags,
                   ifa->ifa_addr ? ifa->ifa_addr->sa_family : 0);
        if (ifa->ifa_flags & IFF_LOOPBACK)
            LogMsg("UpdateInterfaceList: %5s(%d) Flags %04X Family %2d Interface IFF_LOOPBACK",
                   ifa->ifa_name, if_nametoindex(ifa->ifa_name), ifa->ifa_flags,
                   ifa->ifa_addr ? ifa->ifa_addr->sa_family : 0);
#endif

        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_LINK)
        {
            struct sockaddr_dl *sdl = (struct sockaddr_dl *)ifa->ifa_addr;
            if (sdl->sdl_type == IFT_ETHER && sdl->sdl_alen == sizeof(m->PrimaryMAC) && mDNSSameEthAddress(&m->PrimaryMAC, &zeroEthAddr))
                mDNSPlatformMemCopy(m->PrimaryMAC.b, sdl->sdl_data + sdl->sdl_nlen, 6);
        }

        if (ifa->ifa_flags & IFF_UP && ifa->ifa_addr && !isExcludedInterface(InfoSocket, ifa->ifa_name))
            if (ifa->ifa_addr->sa_family == AF_INET || ifa->ifa_addr->sa_family == AF_INET6)
            {
                if (!ifa->ifa_netmask)
                {
                    mDNSAddr ip;
                    SetupAddr(&ip, ifa->ifa_addr);
                    LogMsg("UpdateInterfaceList: ifa_netmask is NULL for %5s(%d) Flags %04X Family %2d %#a",
                           ifa->ifa_name, if_nametoindex(ifa->ifa_name), ifa->ifa_flags, ifa->ifa_addr->sa_family, &ip);
                }
                // Apparently it's normal for the sa_family of an ifa_netmask to sometimes be zero, so we don't complain about that
                // <rdar://problem/5492035> getifaddrs is returning invalid netmask family for fw0 and vmnet
                else if (ifa->ifa_netmask->sa_family != ifa->ifa_addr->sa_family && ifa->ifa_netmask->sa_family != 0)
                {
                    mDNSAddr ip;
                    SetupAddr(&ip, ifa->ifa_addr);
                    LogMsg("UpdateInterfaceList: ifa_netmask for %5s(%d) Flags %04X Family %2d %#a has different family: %d",
                           ifa->ifa_name, if_nametoindex(ifa->ifa_name), ifa->ifa_flags, ifa->ifa_addr->sa_family, &ip, ifa->ifa_netmask->sa_family);
                }
                // Currently we use a few internal ones like mDNSInterfaceID_LocalOnly etc. that are negative values (0, -1, -2).
                else if ((int)if_nametoindex(ifa->ifa_name) <= 0)
                {
                    LogMsg("UpdateInterfaceList: if_nametoindex returned zero/negative value for %5s(%d)", ifa->ifa_name, if_nametoindex(ifa->ifa_name));
                }
                else
                {
                    // Make sure ifa_netmask->sa_family is set correctly
                    // <rdar://problem/5492035> getifaddrs is returning invalid netmask family for fw0 and vmnet
                    ifa->ifa_netmask->sa_family = ifa->ifa_addr->sa_family;
                    int ifru_flags6 = 0;

                    struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)ifa->ifa_addr;
                    if (ifa->ifa_addr->sa_family == AF_INET6 && InfoSocket >= 0)
                    {
                        struct in6_ifreq ifr6;
                        mDNSPlatformMemZero((char *)&ifr6, sizeof(ifr6));
                        strlcpy(ifr6.ifr_name, ifa->ifa_name, sizeof(ifr6.ifr_name));
                        ifr6.ifr_addr = *sin6;
                        if (ioctl(InfoSocket, SIOCGIFAFLAG_IN6, &ifr6) != -1)
                            ifru_flags6 = ifr6.ifr_ifru.ifru_flags6;
                        verbosedebugf("%s %.16a %04X %04X", ifa->ifa_name, &sin6->sin6_addr, ifa->ifa_flags, ifru_flags6);
                    }

                    if (!(ifru_flags6 & (IN6_IFF_TENTATIVE | IN6_IFF_DETACHED | IN6_IFF_DEPRECATED | IN6_IFF_TEMPORARY)))
                    {
                        AddInterfaceToList(ifa, utc);
                    }
                }
            }
        ifa = ifa->ifa_next;
    }

    if (InfoSocket >= 0) 
        close(InfoSocket);

    mDNS_snprintf(defaultname, sizeof(defaultname), "%.*s-%02X%02X%02X%02X%02X%02X", HINFO_HWstring_prefixlen, HINFO_HWstring,
                  m->PrimaryMAC.b[0], m->PrimaryMAC.b[1], m->PrimaryMAC.b[2], m->PrimaryMAC.b[3], m->PrimaryMAC.b[4], m->PrimaryMAC.b[5]);

    // Set up the nice label
    domainlabel nicelabel;
    nicelabel.c[0] = 0;
    GetUserSpecifiedFriendlyComputerName(&nicelabel);
    if (nicelabel.c[0] == 0)
    {
        debugf("Couldn’t read user-specified Computer Name; using default “%s” instead", defaultname);
        MakeDomainLabelFromLiteralString(&nicelabel, defaultname);
    }

    // Set up the RFC 1034-compliant label
    domainlabel hostlabel;
    hostlabel.c[0] = 0;
    GetUserSpecifiedLocalHostName(&hostlabel);
    if (hostlabel.c[0] == 0)
    {
        debugf("Couldn’t read user-specified Local Hostname; using default “%s.local” instead", defaultname);
        MakeDomainLabelFromLiteralString(&hostlabel, defaultname);
    }

    // We use a case-sensitive comparison here because even though changing the capitalization
    // of the name alone is not significant to DNS, it's still a change from the user's point of view
    if (SameDomainLabelCS(m->p->usernicelabel.c, nicelabel.c))
        debugf("Usernicelabel (%#s) unchanged since last time; not changing m->nicelabel (%#s)", m->p->usernicelabel.c, m->nicelabel.c);
    else
    {
        if (m->p->usernicelabel.c[0])   // Don't show message first time through, when we first read name from prefs on boot
            LogMsg("User updated Computer Name from “%#s” to “%#s”", m->p->usernicelabel.c, nicelabel.c);
        m->p->usernicelabel = m->nicelabel = nicelabel;
    }

    if (SameDomainLabelCS(m->p->userhostlabel.c, hostlabel.c))
        debugf("Userhostlabel (%#s) unchanged since last time; not changing m->hostlabel (%#s)", m->p->userhostlabel.c, m->hostlabel.c);
    else
    {
        if (m->p->userhostlabel.c[0])   // Don't show message first time through, when we first read name from prefs on boot
            LogMsg("User updated Local Hostname from “%#s” to “%#s”", m->p->userhostlabel.c, hostlabel.c);
        m->p->userhostlabel = m->hostlabel = hostlabel;
        mDNS_SetFQDN(m);
    }

    return(mStatus_NoError);
}

// Returns number of leading one-bits in mask: 0-32 for IPv4, 0-128 for IPv6
// Returns -1 if all the one-bits are not contiguous
mDNSlocal int CountMaskBits(mDNSAddr *mask)
{
    int i = 0, bits = 0;
    int bytes = mask->type == mDNSAddrType_IPv4 ? 4 : mask->type == mDNSAddrType_IPv6 ? 16 : 0;
    while (i < bytes)
    {
        mDNSu8 b = mask->ip.v6.b[i++];
        while (b & 0x80) { bits++; b <<= 1; }
        if (b) return(-1);
    }
    while (i < bytes) if (mask->ip.v6.b[i++]) return(-1);
    return(bits);
}

// Returns count of non-link local V4 addresses registered (why? -- SC)
mDNSlocal int SetupActiveInterfaces(mDNSs32 utc)
{
    mDNS *const m = &mDNSStorage;
    NetworkInterfaceInfoOSX *i;
    int count = 0;

    // Recalculate SuppressProbes time based on the current set of active interfaces.
    m->SuppressProbes = 0;
    for (i = m->p->InterfaceList; i; i = i->next)
        if (i->Exists)
        {
            NetworkInterfaceInfo *const n = &i->ifinfo;
            NetworkInterfaceInfoOSX *primary = SearchForInterfaceByName(i->ifinfo.ifname, AF_UNSPEC);

            if (i->Registered && i->Registered != primary)  // Sanity check
            {
                LogMsg("SetupActiveInterfaces ERROR! n->Registered %p != primary %p", i->Registered, primary);
                i->Registered = mDNSNULL;
            }

            if (!i->Registered)
            {
                InterfaceActivationSpeed activationSpeed;

                // Note: If i->Registered is set, that means we've called mDNS_RegisterInterface() for this interface,
                // so we need to make sure we call mDNS_DeregisterInterface() before disposing it.
                // If i->Registered is NOT set, then we haven't registered it and we should not try to deregister it.
                i->Registered = primary;

                // If i->LastSeen == utc, then this is a brand-new interface, just created, or an interface that never went away.
                // If i->LastSeen != utc, then this is an old interface, previously seen, that went away for (utc - i->LastSeen) seconds.
                // If the interface is an old one that went away and came back in less than a minute, then we're in a flapping scenario.
                i->Occulting = !(i->ifa_flags & IFF_LOOPBACK) && (utc - i->LastSeen > 0 && utc - i->LastSeen < 60);

                // The "p2p*" interfaces used for legacy AirDrop reuse the scope-id, MAC address and the IP address
                // every time a new interface is created. We think it is a duplicate and hence consider it
                // as flashing and occulting, that is, flapping. If an interface is marked as flapping,
                // mDNS_RegisterInterface() changes the probe delay from 1/2 second to 5 seconds and
                // logs a warning message to system.log noting frequent interface transitions.
                // The same logic applies when the IFEF_DIRECTLINK flag is set on the interface.
                if ((strncmp(i->ifinfo.ifname, "p2p", 3) == 0) || i->ifinfo.DirectLink)
                {
                    activationSpeed = FastActivation;
                    LogInfo("SetupActiveInterfaces: %s DirectLink interface registering", i->ifinfo.ifname);
                }
#if MDNSRESPONDER_SUPPORTS(APPLE, SLOW_ACTIVATION)
                else if (i->Flashing && i->Occulting)
                {
                    activationSpeed = SlowActivation;
                }
#endif
                else
                {
                    activationSpeed = NormalActivation;
                }

                mDNS_RegisterInterface(m, n, activationSpeed);

                if (!mDNSAddressIsLinkLocal(&n->ip)) count++;
                LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_INFO,
                        "SetupActiveInterfaces: Registered " PUB_S " (%u) BSSID " PRI_MAC_ADDR " Struct addr %p, primary %p,"
                        " " PRI_IP_ADDR "/%d" PUB_S PUB_S PUB_S,
                        i->ifinfo.ifname, i->scope_id, &i->BSSID, i, primary, &n->ip, CountMaskBits(&n->mask),
                        i->Flashing        ? " (Flashing)"  : "",
                        i->Occulting       ? " (Occulting)" : "",
                        n->InterfaceActive ? " (Primary)"   : "");

                if (!n->McastTxRx)
                {
                    debugf("SetupActiveInterfaces:   No Tx/Rx on   %5s(%lu) %.6a InterfaceID %p %#a", i->ifinfo.ifname, i->scope_id, &i->BSSID, i->ifinfo.InterfaceID, &n->ip);
#if TARGET_OS_IPHONE
                    // We join the Bonjour multicast group on Apple embedded platforms ONLY when a client request is active,
                    // so we leave the multicast group here to clear any residual group membership.
                    if (i->sa_family == AF_INET)
                    {
                        struct ip_mreq imr;
                        primary->ifa_v4addr.s_addr = n->ip.ip.v4.NotAnInteger;
                        imr.imr_multiaddr.s_addr = AllDNSLinkGroup_v4.ip.v4.NotAnInteger;
                        imr.imr_interface        = primary->ifa_v4addr;

                        if (SearchForInterfaceByName(i->ifinfo.ifname, AF_INET) == i)
                        {
                            LogInfo("SetupActiveInterfaces: %5s(%lu) Doing IP_DROP_MEMBERSHIP for %.4a on %.4a", i->ifinfo.ifname, i->scope_id, &imr.imr_multiaddr, &imr.imr_interface);
                            mStatus err = setsockopt(m->p->permanentsockets.sktv4, IPPROTO_IP, IP_DROP_MEMBERSHIP, &imr, sizeof(imr));
                            if (err < 0 && (errno != EADDRNOTAVAIL))
                                LogMsg("setsockopt - IP_DROP_MEMBERSHIP error %d errno %d (%s)", err, errno, strerror(errno));
                        }
                    }
                    if (i->sa_family == AF_INET6)
                    {
                        struct ipv6_mreq i6mr;
                        i6mr.ipv6mr_interface = primary->scope_id;
                        i6mr.ipv6mr_multiaddr = *(struct in6_addr*)&AllDNSLinkGroup_v6.ip.v6;

                        if (SearchForInterfaceByName(i->ifinfo.ifname, AF_INET6) == i)
                        {
                            LogInfo("SetupActiveInterfaces: %5s(%lu) Doing IPV6_LEAVE_GROUP for %.16a on %u", i->ifinfo.ifname, i->scope_id, &i6mr.ipv6mr_multiaddr, i6mr.ipv6mr_interface);
                            mStatus err = setsockopt(m->p->permanentsockets.sktv6, IPPROTO_IPV6, IPV6_LEAVE_GROUP, &i6mr, sizeof(i6mr));
                            if (err < 0 && (errno != EADDRNOTAVAIL))
                                LogMsg("setsockopt - IPV6_LEAVE_GROUP error %d errno %d (%s) group %.16a on %u", err, errno, strerror(errno), &i6mr.ipv6mr_multiaddr, i6mr.ipv6mr_interface);
                        }
                    }
#endif // TARGET_OS_IPHONE
                }
                else
                {
                    if (i->sa_family == AF_INET)
                    {
                        struct ip_mreq imr;
                        primary->ifa_v4addr.s_addr = n->ip.ip.v4.NotAnInteger;
                        imr.imr_multiaddr.s_addr = AllDNSLinkGroup_v4.ip.v4.NotAnInteger;
                        imr.imr_interface        = primary->ifa_v4addr;

                        // If this is our *first* IPv4 instance for this interface name, we need to do a IP_DROP_MEMBERSHIP first,
                        // before trying to join the group, to clear out stale kernel state which may be lingering.
                        // In particular, this happens with removable network interfaces like USB Ethernet adapters -- the kernel has stale state
                        // from the last time the USB Ethernet adapter was connected, and part of the kernel thinks we've already joined the group
                        // on that interface (so we get EADDRINUSE when we try to join again) but a different part of the kernel thinks we haven't
                        // joined the group (so we receive no multicasts). Doing an IP_DROP_MEMBERSHIP before joining seems to flush the stale state.
                        // Also, trying to make the code leave the group when the adapter is removed doesn't work either,
                        // because by the time we get the configuration change notification, the interface is already gone,
                        // so attempts to unsubscribe fail with EADDRNOTAVAIL (errno 49 "Can't assign requested address").
                        // <rdar://problem/5585972> IP_ADD_MEMBERSHIP fails for previously-connected removable interfaces
                        if (SearchForInterfaceByName(i->ifinfo.ifname, AF_INET) == i)
                        {
                            LogInfo("SetupActiveInterfaces: %5s(%lu) Doing precautionary IP_DROP_MEMBERSHIP for %.4a on %.4a", i->ifinfo.ifname, i->scope_id, &imr.imr_multiaddr, &imr.imr_interface);
                            mStatus err = setsockopt(m->p->permanentsockets.sktv4, IPPROTO_IP, IP_DROP_MEMBERSHIP, &imr, sizeof(imr));
                            if (err < 0 && (errno != EADDRNOTAVAIL))
                                LogMsg("setsockopt - IP_DROP_MEMBERSHIP error %d errno %d (%s)", err, errno, strerror(errno));
                        }

                        LogInfo("SetupActiveInterfaces: %5s(%lu) joining IPv4 mcast group %.4a on %.4a", i->ifinfo.ifname, i->scope_id, &imr.imr_multiaddr, &imr.imr_interface);
                        mStatus err = setsockopt(m->p->permanentsockets.sktv4, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imr, sizeof(imr));
                        // Joining same group twice can give "Address already in use" error -- no need to report that
                        if (err < 0 && (errno != EADDRINUSE))
                            LogMsg("setsockopt - IP_ADD_MEMBERSHIP error %d errno %d (%s) group %.4a on %.4a", err, errno, strerror(errno), &imr.imr_multiaddr, &imr.imr_interface);
                    }
                    if (i->sa_family == AF_INET6)
                    {
                        struct ipv6_mreq i6mr;
                        i6mr.ipv6mr_interface = primary->scope_id;
                        i6mr.ipv6mr_multiaddr = *(struct in6_addr*)&AllDNSLinkGroup_v6.ip.v6;

                        if (SearchForInterfaceByName(i->ifinfo.ifname, AF_INET6) == i)
                        {
                            LogInfo("SetupActiveInterfaces: %5s(%lu) Doing precautionary IPV6_LEAVE_GROUP for %.16a on %u", i->ifinfo.ifname, i->scope_id, &i6mr.ipv6mr_multiaddr, i6mr.ipv6mr_interface);
                            mStatus err = setsockopt(m->p->permanentsockets.sktv6, IPPROTO_IPV6, IPV6_LEAVE_GROUP, &i6mr, sizeof(i6mr));
                            if (err < 0 && (errno != EADDRNOTAVAIL))
                                LogMsg("setsockopt - IPV6_LEAVE_GROUP error %d errno %d (%s) group %.16a on %u", err, errno, strerror(errno), &i6mr.ipv6mr_multiaddr, i6mr.ipv6mr_interface);
                        }

                        LogInfo("SetupActiveInterfaces: %5s(%lu) joining IPv6 mcast group %.16a on %u", i->ifinfo.ifname, i->scope_id, &i6mr.ipv6mr_multiaddr, i6mr.ipv6mr_interface);
                        mStatus err = setsockopt(m->p->permanentsockets.sktv6, IPPROTO_IPV6, IPV6_JOIN_GROUP, &i6mr, sizeof(i6mr));
                        // Joining same group twice can give "Address already in use" error -- no need to report that
                        if (err < 0 && (errno != EADDRINUSE))
                            LogMsg("setsockopt - IPV6_JOIN_GROUP error %d errno %d (%s) group %.16a on %u", err, errno, strerror(errno), &i6mr.ipv6mr_multiaddr, i6mr.ipv6mr_interface);
                    }
                }
            }
        }

    return count;
}

mDNSlocal void MarkAllInterfacesInactive(mDNSs32 utc)
{
    NetworkInterfaceInfoOSX *i;
    for (i = mDNSStorage.p->InterfaceList; i; i = i->next)
    {
        if (i->Exists) i->LastSeen = utc;
        i->Exists = mDNSfalse;
    }
}

// Returns count of non-link local V4 addresses deregistered (why? -- SC)
mDNSlocal int ClearInactiveInterfaces(mDNSs32 utc)
{
    mDNS *const m = &mDNSStorage;
    // First pass:
    // If an interface is going away, then deregister this from the mDNSCore.
    // We also have to deregister it if the primary interface that it's using for its InterfaceID is going away.
    // We have to do this because mDNSCore will use that InterfaceID when sending packets, and if the memory
    // it refers to has gone away we'll crash.
    NetworkInterfaceInfoOSX *i;
    int count = 0;
    for (i = m->p->InterfaceList; i; i = i->next)
    {
        // If this interface is no longer active, or its InterfaceID is changing, deregister it
        NetworkInterfaceInfoOSX *primary = SearchForInterfaceByName(i->ifinfo.ifname, AF_UNSPEC);
        if (i->Registered)
            if (i->Exists == 0 || i->Exists == MulticastStateChanged || i->Registered != primary)
            {
                InterfaceActivationSpeed activationSpeed;

                i->Flashing = !(i->ifa_flags & IFF_LOOPBACK) && (utc - i->AppearanceTime < 60);
                LogInfo("ClearInactiveInterfaces: Deregistering %5s(%lu) %.6a InterfaceID %p(%p), primary %p, %#a/%d%s%s%s",
                        i->ifinfo.ifname, i->scope_id, &i->BSSID, i->ifinfo.InterfaceID, i, primary,
                        &i->ifinfo.ip, CountMaskBits(&i->ifinfo.mask),
                        i->Flashing               ? " (Flashing)"  : "",
                        i->Occulting              ? " (Occulting)" : "",
                        i->ifinfo.InterfaceActive ? " (Primary)"   : "");

                // "p2p*" interfaces used for legacy AirDrop reuse the scope-id, MAC address and the IP address
                // every time it creates a new interface. We think it is a duplicate and hence consider it
                // as flashing and occulting. The "core" does not flush the cache for this case. This leads to
                // stale data returned to the application even after the interface is removed. The application
                // then starts to send data but the new interface is not yet created.
                // The same logic applies when the IFEF_DIRECTLINK flag is set on the interface.
                if ((strncmp(i->ifinfo.ifname, "p2p", 3) == 0) || i->ifinfo.DirectLink)
                {
                    activationSpeed = FastActivation;
                    LogInfo("ClearInactiveInterfaces: %s DirectLink interface deregistering", i->ifinfo.ifname);
                }
#if MDNSRESPONDER_SUPPORTS(APPLE, SLOW_ACTIVATION)
                else if (i->Flashing && i->Occulting)
                {
                    activationSpeed = SlowActivation;
                }
#endif
                else
                {
                    activationSpeed = NormalActivation;
                }
                mDNS_DeregisterInterface(m, &i->ifinfo, activationSpeed);

                if (!mDNSAddressIsLinkLocal(&i->ifinfo.ip)) count++;
                i->Registered = mDNSNULL;
                // Note: If i->Registered is set, that means we've called mDNS_RegisterInterface() for this interface,
                // so we need to make sure we call mDNS_DeregisterInterface() before disposing it.
                // If i->Registered is NOT set, then it's not registered and we should not call mDNS_DeregisterInterface() on it.

                // Caution: If we ever decide to add code here to leave the multicast group, we need to make sure that this
                // is the LAST representative of this physical interface, or we'll unsubscribe from the group prematurely.
            }
    }

    // Second pass:
    // Now that everything that's going to deregister has done so, we can clean up and free the memory
    NetworkInterfaceInfoOSX **p = &m->p->InterfaceList;
    while (*p)
    {
        i = *p;
        // If no longer active, delete interface from list and free memory
        if (!i->Exists)
        {
            if (i->LastSeen == utc) i->LastSeen = utc - 1;
            const mDNSBool delete = ((utc - i->LastSeen) >= 60) ? mDNStrue : mDNSfalse;
            LogInfo("ClearInactiveInterfaces: %-13s %5s(%lu) %.6a InterfaceID %p(%p) %#a/%d Age %d%s", delete ? "Deleting" : "Holding",
                    i->ifinfo.ifname, i->scope_id, &i->BSSID, i->ifinfo.InterfaceID, i,
                    &i->ifinfo.ip, CountMaskBits(&i->ifinfo.mask), utc - i->LastSeen,
                    i->ifinfo.InterfaceActive ? " (Primary)" : "");
#if APPLE_OSX_mDNSResponder
            if (i->BPF_fd >= 0) CloseBPF(i);
#endif // APPLE_OSX_mDNSResponder
            if (delete)
            {
                *p = i->next;
                freeL("NetworkInterfaceInfoOSX", i);
                continue;   // After deleting this object, don't want to do the "p = &i->next;" thing at the end of the loop
            }
        }
        p = &i->next;
    }
    return count;
}

mDNSlocal void AppendDNameListElem(DNameListElem ***List, mDNSu32 uid, domainname *name)
{
    DNameListElem *dnle = (DNameListElem*) callocL("DNameListElem/AppendDNameListElem", sizeof(*dnle));
    if (!dnle) LogMsg("ERROR: AppendDNameListElem: memory exhausted");
    else
    {
        dnle->next = mDNSNULL;
        dnle->uid  = uid;
        AssignDomainName(&dnle->name, name);
        **List = dnle;
        *List = &dnle->next;
    }
}

mDNSlocal int compare_dns_configs(const void *aa, const void *bb)
{
    dns_resolver_t *a = *(dns_resolver_t**)aa;
    dns_resolver_t *b = *(dns_resolver_t**)bb;

    return (a->search_order < b->search_order) ? -1 : (a->search_order == b->search_order) ? 0 : 1;
}

mDNSlocal void UpdateSearchDomainHash(MD5_CTX *sdc, char *domain, mDNSInterfaceID InterfaceID)
{
    mDNS *const m = &mDNSStorage;
    char *buf = ".";
    mDNSu32 scopeid = 0;
    char ifid_buf[16];

    if (domain)
        buf = domain;
    //
    // Hash the search domain name followed by the InterfaceID.
    // As we have scoped search domains, we also included InterfaceID. If either of them change,
    // we will detect it. Even if the order of them change, we will detect it.
    //
    // Note: We have to handle a few of these tricky cases.
    //
    // 1) Current: com, apple.com Changing to: comapple.com
    // 2) Current: a.com,b.com Changing to a.comb.com
    // 3) Current: a.com,b.com (ifid 8), Changing to a.com8b.com (ifid 8)
    // 4) Current: a.com (ifid 12), Changing to a.com1 (ifid: 2)
    //
    // There are more variants of the above. The key thing is if we include the null in each case
    // at the end of name and the InterfaceID, it will prevent a new name (which can't include
    // NULL as part of the name) to be mistakenly thought of as a old name.

    scopeid = mDNSPlatformInterfaceIndexfromInterfaceID(m, InterfaceID, mDNStrue);
    // mDNS_snprintf always null terminates
    if (mDNS_snprintf(ifid_buf, sizeof(ifid_buf), "%u", scopeid) >= sizeof(ifid_buf))
        LogMsg("UpdateSearchDomainHash: mDNS_snprintf failed for scopeid %u", scopeid);

    LogInfo("UpdateSearchDomainHash: buf %s, ifid_buf %s", buf, ifid_buf);
    MD5_Update(sdc, buf, strlen(buf) + 1);
    MD5_Update(sdc, ifid_buf, strlen(ifid_buf) + 1);
}

mDNSlocal void FinalizeSearchDomainHash(MD5_CTX *sdc)
{
    mDNS *const m = &mDNSStorage;
    mDNSu8 md5_hash[MD5_LEN];

    MD5_Final(md5_hash, sdc);

    if (memcmp(md5_hash, m->SearchDomainsHash, MD5_LEN))
    {
        // If the hash is different, either the search domains have changed or
        // the ordering between them has changed. Restart the questions that
        // would be affected by this.
        LogInfo("FinalizeSearchDomains: The hash is different");
        memcpy(m->SearchDomainsHash, md5_hash, MD5_LEN);
        RetrySearchDomainQuestions(m);
    }
    else { LogInfo("FinalizeSearchDomains: The hash is same"); }
}

mDNSlocal void ConfigSearchDomains(dns_resolver_t *resolver, mDNSInterfaceID interfaceId, mDNSu32 scope,  MD5_CTX *sdc, uint64_t generation)
{
    const char *scopeString = DNSScopeToString(scope);
    int j;
    domainname d;

    if (scope == kScopeNone)
        interfaceId = mDNSInterface_Any;

    if (scope == kScopeNone || scope == kScopeInterfaceID)
    {
        for (j = 0; j < resolver->n_search; j++)
        {
            if (MakeDomainNameFromDNSNameString(&d, resolver->search[j]) != NULL)
            {
                char interface_buf[32];
                mDNS_snprintf(interface_buf, sizeof(interface_buf), "for interface %s", InterfaceNameForID(&mDNSStorage,  interfaceId));
                LogInfo("ConfigSearchDomains: (%s) configuring search domain %s %s (generation= %llu)", scopeString,
                        resolver->search[j], (interfaceId == mDNSInterface_Any) ? "" : interface_buf, generation);
                UpdateSearchDomainHash(sdc, resolver->search[j], interfaceId);
                mDNS_AddSearchDomain_CString(resolver->search[j], interfaceId);
            }
            else
            {
                LogInfo("ConfigSearchDomains: An invalid search domain was detected for %s domain %s n_nameserver %d, (generation= %llu)",
                        DNSScopeToString(scope), resolver->domain, resolver->n_nameserver, generation);
            }
        }
    }
    else
    {
        LogInfo("ConfigSearchDomains: (%s) Ignoring search domain for interface %s", scopeString, InterfaceNameForID(&mDNSStorage, interfaceId));
    }
}

mDNSlocal mDNSInterfaceID ConfigParseInterfaceID(mDNSu32 ifindex)
{
    NetworkInterfaceInfoOSX *ni;
    mDNSInterfaceID interface;

    for (ni = mDNSStorage.p->InterfaceList; ni; ni = ni->next)
    {
        if (ni->ifinfo.InterfaceID && ni->scope_id == ifindex) 
            break;
    }
    if (ni != NULL) 
    {
        interface = ni->ifinfo.InterfaceID;
    }
    else
    {
        // In rare circumstances, we could potentially hit this case where we cannot parse the InterfaceID
        // (see <rdar://problem/13214785>). At this point, we still accept the DNS Config from configd 
        // Note: We currently ack the whole dns configuration and not individual resolvers or DNS servers. 
        // As the caller is going to ack the configuration always, we have to add all the DNS servers 
        // in the configuration. Otherwise, we won't have any DNS servers up until the network change.

        LogMsg("ConfigParseInterfaceID: interface specific index %d not found (interface may not be UP)",ifindex);

        // Set the correct interface from configd before passing this to mDNS_AddDNSServer() below
        interface = (mDNSInterfaceID)(unsigned long)ifindex;
    }
    return interface;
}

mDNSlocal void ConfigNonUnicastResolver(dns_resolver_t *r)
{
    char *opt = r->options;
    domainname d; 

    if (opt && !strncmp(opt, "mdns", strlen(opt)))
    {
        if (!MakeDomainNameFromDNSNameString(&d, r->domain))
        { 
            LogMsg("ConfigNonUnicastResolver: config->resolver bad domain %s", r->domain); 
            return;
        }
        mDNS_AddMcastResolver(&mDNSStorage, &d, mDNSInterface_Any, r->timeout);
    }
}

mDNSlocal void ConfigDNSServers(dns_resolver_t *r, mDNSInterfaceID interfaceID, mDNSu32 scope, mDNSu32 resGroupID)
{
    domainname domain;
    if (!r->domain || (*r->domain == '\0'))
    {
        domain.c[0] = 0;
    }
    else if (!MakeDomainNameFromDNSNameString(&domain, r->domain))
    { 
        LogMsg("ConfigDNSServers: bad domain %s", r->domain); 
        return;
    }
    // Parse the resolver specific attributes that affects all the DNS servers.
    const int32_t serviceID = (scope == kScopeServiceID) ? r->service_identifier : 0;

    const mdns_interface_monitor_t monitor = GetInterfaceMonitorForIndex((uint32_t)((uintptr_t)interfaceID));
    const mDNSBool isExpensive   = (monitor && mdns_interface_monitor_is_expensive(monitor))   ? mDNStrue : mDNSfalse;
    const mDNSBool isConstrained = (monitor && mdns_interface_monitor_is_constrained(monitor)) ? mDNStrue : mDNSfalse;
    const mDNSBool isCLAT46      = (monitor && mdns_interface_monitor_is_clat46(monitor))      ? mDNStrue : mDNSfalse;
    const mDNSBool usableA       = (r->flags & DNS_RESOLVER_FLAGS_REQUEST_A_RECORDS)           ? mDNStrue : mDNSfalse;
    const mDNSBool usableAAAA    = (r->flags & DNS_RESOLVER_FLAGS_REQUEST_AAAA_RECORDS)        ? mDNStrue : mDNSfalse;
#if TARGET_OS_IPHONE
    const mDNSBool isCell        = (r->reach_flags & kSCNetworkReachabilityFlagsIsWWAN)        ? mDNStrue : mDNSfalse;
#else
    const mDNSBool isCell        = mDNSfalse;
#endif

    const mDNSIPPort port = (r->port != 0) ? mDNSOpaque16fromIntVal(r->port) : UnicastDNSPort;
    for (int32_t i = 0; i < r->n_nameserver; i++)
    {
        const int family = r->nameserver[i]->sa_family;
        if ((family != AF_INET) && (family != AF_INET6)) continue;

        mDNSAddr saddr;
        if (SetupAddr(&saddr, r->nameserver[i]))
        {
            LogMsg("ConfigDNSServers: Bad address");
            continue;
        }

        // The timeout value is for all the DNS servers in a given resolver, hence we pass
        // the timeout value only for the first DNSServer. If we don't have a value in the
        // resolver, then use the core's default value
        //
        // Note: this assumes that when the core picks a list of DNSServers for a question,
        // it takes the sum of all the timeout values for all DNS servers. By doing this, it
        // tries all the DNS servers in a specified timeout
        DNSServer *s = mDNS_AddDNSServer(&mDNSStorage, &domain, interfaceID, serviceID, &saddr, port, scope,
            (i == 0) ? (r->timeout ? r->timeout : DEFAULT_UDNS_TIMEOUT) : 0, isCell, isExpensive, isConstrained, isCLAT46,
            resGroupID, usableA, usableAAAA, mDNStrue);
        if (s)
        {
            LogInfo("ConfigDNSServers(%s): DNS server %#a:%d for domain %##s",
                DNSScopeToString(scope), &s->addr, mDNSVal16(s->port), domain.c);
        }
    }
}

// ConfigResolvers is called for different types of resolvers: Unscoped resolver, Interface scope resolver and
// Service scope resolvers. This is indicated by the scope argument.
//
// "resolver" has entries that should only be used for unscoped questions.
//
// "scoped_resolver" has entries that should only be used for Interface scoped question i.e., questions that specify an
// interface index (q->InterfaceID)
//
// "service_specific_resolver" has entries that should be used for Service scoped question i.e., questions that specify
// a service identifier (q->ServiceID)
//
mDNSlocal void ConfigResolvers(dns_config_t *config, mDNSu32 scope, mDNSBool setsearch, mDNSBool setservers, MD5_CTX *sdc)
{
    int i;
    dns_resolver_t **resolver;
    int nresolvers;
    const char *scopeString = DNSScopeToString(scope);
    mDNSInterfaceID interface;

    switch (scope)
    {
        case kScopeNone:
            resolver = config->resolver;
            nresolvers = config->n_resolver;
            break;
        case kScopeInterfaceID:
            resolver = config->scoped_resolver;
            nresolvers = config->n_scoped_resolver;
            break;
        case kScopeServiceID:
            resolver = config->service_specific_resolver;
            nresolvers = config->n_service_specific_resolver;
            break;
        default:
            return;
    }
    qsort(resolver, nresolvers, sizeof(dns_resolver_t*), compare_dns_configs);

    for (i = 0; i < nresolvers; i++)
    {
        dns_resolver_t *r = resolver[i];

        LogInfo("ConfigResolvers: %s resolver[%d] domain %s n_nameserver %d", scopeString, i, r->domain, r->n_nameserver);

        interface = mDNSInterface_Any;

        // Parse the interface index 
        if (r->if_index != 0)
        {
            interface = ConfigParseInterfaceID(r->if_index);
        }

        if (setsearch)
        {
            ConfigSearchDomains(resolver[i], interface, scope, sdc, config->generation);
            
            // Parse other scoped resolvers for search lists
            if (!setservers) 
                continue;
        }

        if (r->port == 5353 || r->n_nameserver == 0)
        {
            ConfigNonUnicastResolver(r);
        }
        else
        {
            ConfigDNSServers(r, interface, scope, mDNS_GetNextResolverGroupID());
        }
    }
}

#if MDNSRESPONDER_SUPPORTS(APPLE, REACHABILITY_TRIGGER)
mDNSlocal mDNSBool QuestionValidForDNSTrigger(const DNSQuestion *q)
{
    if (q->Suppressed)
    {
        debugf("QuestionValidForDNSTrigger: Suppressed: %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
        return mDNSfalse;
    }
    if (mDNSOpaque16IsZero(q->TargetQID))
    {
        debugf("QuestionValidForDNSTrigger: Multicast: %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
        return mDNSfalse;
    }
    // If we answered using LocalOnly records e.g., /etc/hosts, don't consider that a valid response
    // for trigger.
    if (q->LOAddressAnswers)
    {
        debugf("QuestionValidForDNSTrigger: LocalOnly answers: %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
        return mDNSfalse;
    }
    return mDNStrue;
}

// This function is called if we are not delivering unicast answers to "A" or "AAAA" questions.
// We set our state appropriately so that if we start receiving answers, trigger the
// upper layer to retry DNS questions.
mDNSexport void mDNSPlatformUpdateDNSStatus(const DNSQuestion *q)
{
    mDNS *const m = &mDNSStorage;
    if (!QuestionValidForDNSTrigger(q))
        return;

    // Ignore applications that start and stop queries for no reason before we ever talk
    // to any DNS server.
    if (!q->triedAllServersOnce)
    {
        LogInfo("QuestionValidForDNSTrigger: question %##s (%s) stopped too soon", q->qname.c, DNSTypeName(q->qtype));
        return;
    }
    if (q->qtype == kDNSType_A)
        m->p->v4answers = 0;
    if (q->qtype == kDNSType_AAAA)
        m->p->v6answers = 0;
    if (!m->p->v4answers || !m->p->v6answers)
    {
        LogInfo("mDNSPlatformUpdateDNSStatus: Trigger needed v4 %d, v6 %d, question %##s (%s)", m->p->v4answers, m->p->v6answers, q->qname.c,
            DNSTypeName(q->qtype));
    }
}
#endif  // MDNSRESPONDER_SUPPORTS(APPLE, REACHABILITY_TRIGGER)

mDNSlocal void AckConfigd(dns_config_t *config)
{
    mDNS_CheckLock(&mDNSStorage);

    // Acking the configuration triggers configd to reissue the reachability queries
    mDNSStorage.p->DNSTrigger = NonZeroTime(mDNSStorage.timenow);
    _dns_configuration_ack(config, "com.apple.mDNSResponder");
}

#if MDNSRESPONDER_SUPPORTS(APPLE, REACHABILITY_TRIGGER)
// If v4q is non-NULL, it means we have received some answers for "A" type questions
// If v6q is non-NULL, it means we have received some answers for "AAAA" type questions
mDNSexport void mDNSPlatformTriggerDNSRetry(const DNSQuestion *v4q, const DNSQuestion *v6q)
{
    mDNS *const m = &mDNSStorage;
    mDNSBool trigger = mDNSfalse;
    mDNSs32 timenow;

    // Don't send triggers too often.
    // If we have started delivering answers to questions, we should send a trigger
    // if the time permits. If we are delivering answers, we should set the state
    // of v4answers/v6answers to 1 and avoid sending a trigger.  But, we don't know
    // whether the answers that are being delivered currently is for configd or some
    // other application. If we set the v4answers/v6answers to 1 and not deliver a trigger,
    // then we won't deliver the trigger later when it is okay to send one as the
    // "answers" are already set to 1. Hence, don't affect the state of v4answers and
    // v6answers if we are not delivering triggers.
    mDNS_Lock(m);
    timenow = m->timenow;
    if (m->p->DNSTrigger && (timenow - m->p->DNSTrigger) < DNS_TRIGGER_INTERVAL)
    {
        if (!m->p->v4answers || !m->p->v6answers)
        {
            debugf("mDNSPlatformTriggerDNSRetry: not triggering, time since last trigger %d ms, v4ans %d, v6ans %d",
                (timenow - m->p->DNSTrigger), m->p->v4answers, m->p->v6answers);
        }
        mDNS_Unlock(m);
        return;
    }
    mDNS_Unlock(m);
    if (v4q != NULL && QuestionValidForDNSTrigger(v4q))
    {
        int old = m->p->v4answers;

        m->p->v4answers = 1;

        // If there are IPv4 answers now and previously we did not have
        // any answers, trigger a DNS change so that reachability
        // can retry the queries again.
        if (!old)
        {
            LogInfo("mDNSPlatformTriggerDNSRetry: Triggering because of IPv4, last trigger %d ms, %##s (%s)", (timenow - m->p->DNSTrigger),
                v4q->qname.c, DNSTypeName(v4q->qtype));
            trigger = mDNStrue;
        }
    }
    if (v6q != NULL && QuestionValidForDNSTrigger(v6q))
    {
        int old = m->p->v6answers;

        m->p->v6answers = 1;
        // If there are IPv6 answers now and previously we did not have
        // any answers, trigger a DNS change so that reachability
        // can retry the queries again.
        if (!old)
        {
            LogInfo("mDNSPlatformTriggerDNSRetry: Triggering because of IPv6, last trigger %d ms, %##s (%s)", (timenow - m->p->DNSTrigger),
                v6q->qname.c, DNSTypeName(v6q->qtype));
            trigger = mDNStrue;
        }
    }
    if (trigger)
    {
        dns_config_t *config = dns_configuration_copy();
        if (config)
        {
            mDNS_Lock(m);
            AckConfigd(config);
            mDNS_Unlock(m);
            dns_configuration_free(config);
        }
        else
        {
            LogMsg("mDNSPlatformTriggerDNSRetry: ERROR!! configd did not return config");
        }
    }
}
#endif  // MDNSRESPONDER_SUPPORTS(APPLE, REACHABILITY_TRIGGER)

#if MDNSRESPONDER_SUPPORTS(APPLE, UNICAST_DOTLOCAL)
mDNSlocal void SetupActiveDirectoryDomain(dns_config_t *config)
{
    // Record the so-called "primary" domain, which we use as a hint to tell if the user is on a network set up
    // by someone using Microsoft Active Directory using "local" as a private internal top-level domain
    if (config->n_resolver && config->resolver[0]->domain && config->resolver[0]->n_nameserver &&
        config->resolver[0]->nameserver[0])
    {
        MakeDomainNameFromDNSNameString(&ActiveDirectoryPrimaryDomain, config->resolver[0]->domain);
    }
    else
    {
         ActiveDirectoryPrimaryDomain.c[0] = 0;
    }

    //MakeDomainNameFromDNSNameString(&ActiveDirectoryPrimaryDomain, "test.local");
    ActiveDirectoryPrimaryDomainLabelCount = CountLabels(&ActiveDirectoryPrimaryDomain);
    if (config->n_resolver && config->resolver[0]->n_nameserver &&
        SameDomainName(SkipLeadingLabels(&ActiveDirectoryPrimaryDomain, ActiveDirectoryPrimaryDomainLabelCount - 1), &localdomain))
    {
        SetupAddr(&ActiveDirectoryPrimaryDomainServer, config->resolver[0]->nameserver[0]);
    }
    else
    {
        AssignConstStringDomainName(&ActiveDirectoryPrimaryDomain, "");
        ActiveDirectoryPrimaryDomainLabelCount = 0;
        ActiveDirectoryPrimaryDomainServer = zeroAddr;
    }
}
#endif

mDNSlocal void SetupDDNSDomains(domainname *const fqdn, DNameListElem **RegDomains, DNameListElem **BrowseDomains)
{
    int i;
    char buf[MAX_ESCAPED_DOMAIN_NAME];  // Max legal C-string name, including terminating NULL
    domainname d;

    CFDictionaryRef ddnsdict = SCDynamicStoreCopyValue(NULL, NetworkChangedKey_DynamicDNS);
    if (ddnsdict)
    {
        if (fqdn)
        {
            CFArrayRef fqdnArray = CFDictionaryGetValue(ddnsdict, CFSTR("HostNames"));
            if (fqdnArray && CFArrayGetCount(fqdnArray) > 0)
            {
                // for now, we only look at the first array element.  if we ever support multiple configurations, we will walk the list
                CFDictionaryRef fqdnDict = CFArrayGetValueAtIndex(fqdnArray, 0);
                if (fqdnDict && DictionaryIsEnabled(fqdnDict))
                {
                    CFStringRef name = CFDictionaryGetValue(fqdnDict, CFSTR("Domain"));
                    if (name)
                    {
                        if (!CFStringGetCString(name, buf, sizeof(buf), kCFStringEncodingUTF8) ||
                            !MakeDomainNameFromDNSNameString(fqdn, buf) || !fqdn->c[0])
                            LogMsg("GetUserSpecifiedDDNSConfig SCDynamicStore bad DDNS host name: %s", buf[0] ? buf : "(unknown)");
                        else 
                            debugf("GetUserSpecifiedDDNSConfig SCDynamicStore DDNS host name: %s", buf);
                    }
                }
            }
        }
        if (RegDomains)
        {
            CFArrayRef regArray = CFDictionaryGetValue(ddnsdict, CFSTR("RegistrationDomains"));
            if (regArray && CFArrayGetCount(regArray) > 0)
            {
                CFDictionaryRef regDict = CFArrayGetValueAtIndex(regArray, 0);
                if (regDict && DictionaryIsEnabled(regDict))
                {
                    CFStringRef name = CFDictionaryGetValue(regDict, CFSTR("Domain"));
                    if (name)
                    {
                        if (!CFStringGetCString(name, buf, sizeof(buf), kCFStringEncodingUTF8) ||
                            !MakeDomainNameFromDNSNameString(&d, buf) || !d.c[0])
                            LogMsg("GetUserSpecifiedDDNSConfig SCDynamicStore bad DDNS registration domain: %s", buf[0] ? buf : "(unknown)");
                        else
                        {
                            debugf("GetUserSpecifiedDDNSConfig SCDynamicStore DDNS registration domain: %s", buf);
                            AppendDNameListElem(&RegDomains, 0, &d);
                        }
                    }
                }
            }
        }
        if (BrowseDomains)
        {
            CFArrayRef browseArray = CFDictionaryGetValue(ddnsdict, CFSTR("BrowseDomains"));
            if (browseArray)
            {
                for (i = 0; i < CFArrayGetCount(browseArray); i++)
                {
                    CFDictionaryRef browseDict = CFArrayGetValueAtIndex(browseArray, i);
                    if (browseDict && DictionaryIsEnabled(browseDict))
                    {
                        CFStringRef name = CFDictionaryGetValue(browseDict, CFSTR("Domain"));
                        if (name)
                        {
                            if (!CFStringGetCString(name, buf, sizeof(buf), kCFStringEncodingUTF8) ||
                                !MakeDomainNameFromDNSNameString(&d, buf) || !d.c[0])
                                LogMsg("GetUserSpecifiedDDNSConfig SCDynamicStore bad DDNS browsing domain: %s", buf[0] ? buf : "(unknown)");
                            else
                            {
                                debugf("GetUserSpecifiedDDNSConfig SCDynamicStore DDNS browsing domain: %s", buf);
                                AppendDNameListElem(&BrowseDomains, 0, &d);
                            }
                        }
                    }
                }
            }
        }
        CFRelease(ddnsdict);
    }
}

// Returns mDNSfalse, if it does not set the configuration i.e., if the DNS configuration did not change
mDNSexport mDNSBool mDNSPlatformSetDNSConfig(mDNSBool setservers, mDNSBool setsearch, domainname *const fqdn,
                                             DNameListElem **RegDomains, DNameListElem **BrowseDomains, mDNSBool ackConfig)
{
    mDNS *const m = &mDNSStorage;
    MD5_CTX sdc;    // search domain context

    // Need to set these here because we need to do this even if SCDynamicStoreCreate() or SCDynamicStoreCopyValue() below don't succeed
    if (fqdn         ) fqdn->c[0]      = 0;
    if (RegDomains   ) *RegDomains     = NULL;
    if (BrowseDomains) *BrowseDomains  = NULL;

    LogInfo("mDNSPlatformSetDNSConfig:%s%s%s%s%s",
            setservers    ? " setservers"    : "",
            setsearch     ? " setsearch"     : "",
            fqdn          ? " fqdn"          : "",
            RegDomains    ? " RegDomains"    : "",
            BrowseDomains ? " BrowseDomains" : "");

    if (setsearch) MD5_Init(&sdc);

    // Add the inferred address-based configuration discovery domains
    // (should really be in core code I think, not platform-specific)
    if (setsearch)
    {
        struct ifaddrs *ifa = mDNSNULL;
        struct sockaddr_in saddr;
        mDNSPlatformMemZero(&saddr, sizeof(saddr));
        saddr.sin_len = sizeof(saddr);
        saddr.sin_family = AF_INET;
        saddr.sin_port = 0;
        saddr.sin_addr.s_addr = *(in_addr_t *)&m->Router.ip.v4;

        // Don't add any reverse-IP search domains if doing the WAB bootstrap queries would cause dial-on-demand connection initiation
        if (!AddrRequiresPPPConnection((struct sockaddr *)&saddr)) ifa =  myGetIfAddrs(1);

        while (ifa)
        {
            mDNSAddr a, n;
            char buf[64];

            if (ifa->ifa_addr->sa_family == AF_INET &&
                ifa->ifa_netmask                    &&
                !(ifa->ifa_flags & IFF_LOOPBACK)    &&
                !SetupAddr(&a, ifa->ifa_addr)       &&
                !mDNSv4AddressIsLinkLocal(&a.ip.v4)  )
            {
                // Apparently it's normal for the sa_family of an ifa_netmask to sometimes be incorrect, so we explicitly fix it here before calling SetupAddr
                // <rdar://problem/5492035> getifaddrs is returning invalid netmask family for fw0 and vmnet
                ifa->ifa_netmask->sa_family = ifa->ifa_addr->sa_family;     // Make sure ifa_netmask->sa_family is set correctly
                SetupAddr(&n, ifa->ifa_netmask);
                // Note: This is reverse order compared to a normal dotted-decimal IP address, so we can't use our customary "%.4a" format code
                mDNS_snprintf(buf, sizeof(buf), "%d.%d.%d.%d.in-addr.arpa.", a.ip.v4.b[3] & n.ip.v4.b[3],
                              a.ip.v4.b[2] & n.ip.v4.b[2],
                              a.ip.v4.b[1] & n.ip.v4.b[1],
                              a.ip.v4.b[0] & n.ip.v4.b[0]);
                UpdateSearchDomainHash(&sdc, buf, NULL);
                mDNS_AddSearchDomain_CString(buf, mDNSNULL);
            }
            ifa = ifa->ifa_next;
        }
    }

#ifndef MDNS_NO_DNSINFO
    if (setservers || setsearch)
    {
        dns_config_t *config = dns_configuration_copy();
        if (!config)
        {
            // On 10.4, calls to dns_configuration_copy() early in the boot process often fail.
            // Apparently this is expected behaviour -- "not a bug".
            // Accordingly, we suppress syslog messages for the first three minutes after boot.
            // If we are still getting failures after three minutes, then we log them.
            if ((mDNSu32)mDNSPlatformRawTime() > (mDNSu32)(mDNSPlatformOneSecond * 180))
                LogMsg("mDNSPlatformSetDNSConfig: Error: dns_configuration_copy returned NULL");
        }
        else
        {
            LogInfo("mDNSPlatformSetDNSConfig: config->n_resolver = %d, generation %llu, last %llu", config->n_resolver, config->generation, m->p->LastConfigGeneration);

            // For every network change, mDNSPlatformSetDNSConfig is called twice. First,
            // to update the search domain list (in which case, the setsearch bool is set);
            // and second, to update the DNS server list (in which case, the setservers bool
            // is set). The code assumes only one of these flags, setsearch or setserver,
            // will be set when mDNSPlatformSetDNSConfig is called to handle a network change.
            // The mDNSPlatformSetDNSConfig function also assumes that ackCfg will be set
            // when setservers is set.

            // The search domains update occurs on every network change to avoid sync issues
            // that may occur if a network change happens during the processing
            // of a network change.  The dns servers update occurs when the DNS config
            // changes. The dns servers stay in sync by saving the config's generation number
            // on every update; and only updating when the generation number changes.

            // If this is a DNS server update and the configuration hasn't changed, then skip update
            if (setservers && !m->p->if_interface_changed && m->p->LastConfigGeneration == config->generation)
            {
                LogInfo("mDNSPlatformSetDNSConfig(setservers): generation number %llu same, not processing", config->generation);
                dns_configuration_free(config);
                SetupDDNSDomains(fqdn, RegDomains, BrowseDomains);
                return mDNSfalse;
            }
            if (setservers) {
                // Must check if setservers is true, because mDNSPlatformSetDNSConfig can be called for multiple times
                // with setservers equals to false. If setservers is false, we will end up with clearing if_interface_changed
                // without really updating the server.
                m->p->if_interface_changed = mDNSfalse;
            }

#if MDNSRESPONDER_SUPPORTS(APPLE, UNICAST_DOTLOCAL)
            SetupActiveDirectoryDomain(config);
#endif
            ConfigResolvers(config, kScopeNone, setsearch, setservers, &sdc);
            ConfigResolvers(config, kScopeInterfaceID, setsearch, setservers, &sdc);
            ConfigResolvers(config, kScopeServiceID, setsearch, setservers, &sdc);

            const CFIndex n = m->p->InterfaceMonitors ? CFArrayGetCount(m->p->InterfaceMonitors) : 0;
            for (CFIndex i = n - 1; i >= 0; i--)
            {
                mdns_interface_monitor_t monitor;
                monitor = (mdns_interface_monitor_t) CFArrayGetValueAtIndex(m->p->InterfaceMonitors, i);
                const uint32_t ifIndex = mdns_interface_monitor_get_interface_index(monitor);
                DNSServer *server;
                for (server = m->DNSServers; server; server = server->next)
                {
                    if ((((uintptr_t)server->interface) == ifIndex) && !(server->flags & DNSServerFlag_Delete))
                    {
                        break;
                    }
                }
                if (!server)
                {
                    mdns_retain(monitor);
                    CFArrayRemoveValueAtIndex(m->p->InterfaceMonitors, i);
                    mdns_interface_monitor_invalidate(monitor);
                    mdns_release(monitor);
                }
            }

            // Acking provides a hint to other processes that the current DNS configuration has completed
            // its update.  When configd receives the ack, it publishes a notification.
            // Applications monitoring the notification then know when to re-issue their DNS queries
            // after a network change occurs.
            if (ackConfig)
            {
                // Note: We have to set the generation number here when we are acking.
                // For every DNS configuration change, we do the following:
                //
                // 1) Copy dns configuration, handle search domains change
                // 2) Copy dns configuration, handle dns server change
                //
                // If we update the generation number at step (1), we won't process the
                // DNS servers the second time because generation number would be the same.
                // As we ack only when we process dns servers, we set the generation number
                // during acking.
                m->p->LastConfigGeneration = config->generation;
                LogInfo("mDNSPlatformSetDNSConfig: Acking configuration setservers %d, setsearch %d", setservers, setsearch);
                AckConfigd(config);
            }
            dns_configuration_free(config);
            if (setsearch) FinalizeSearchDomainHash(&sdc);
        }
    }
#endif // MDNS_NO_DNSINFO
    SetupDDNSDomains(fqdn, RegDomains, BrowseDomains);
    return mDNStrue;
}


mDNSexport mStatus mDNSPlatformGetPrimaryInterface(mDNSAddr *v4, mDNSAddr *v6, mDNSAddr *r)
{
    char buf[256];

    CFDictionaryRef dict = SCDynamicStoreCopyValue(NULL, NetworkChangedKey_IPv4);
    if (dict)
    {
        r->type  = mDNSAddrType_IPv4;
        r->ip.v4 = zerov4Addr;
        CFStringRef string = CFDictionaryGetValue(dict, kSCPropNetIPv4Router);
        if (string)
        {
            if (!CFStringGetCString(string, buf, 256, kCFStringEncodingUTF8))
                LogMsg("Could not convert router to CString");
            else
            {
                struct sockaddr_in saddr;
                saddr.sin_len = sizeof(saddr);
                saddr.sin_family = AF_INET;
                saddr.sin_port = 0;
                inet_aton(buf, &saddr.sin_addr);
                *(in_addr_t *)&r->ip.v4 = saddr.sin_addr.s_addr;
            }
        }
        string = CFDictionaryGetValue(dict, kSCDynamicStorePropNetPrimaryInterface);
        if (string)
        {
            mDNSBool HavePrimaryGlobalv6 = mDNSfalse;  // does the primary interface have a global v6 address?
            struct ifaddrs *ifa = myGetIfAddrs(1);
            *v4 = *v6 = zeroAddr;

            if (!CFStringGetCString(string, buf, 256, kCFStringEncodingUTF8)) 
            { 
                LogMsg("Could not convert router to CString"); 
                goto exit; 
            }
            // find primary interface in list
            while (ifa && (mDNSIPv4AddressIsZero(v4->ip.v4) || mDNSv4AddressIsLinkLocal(&v4->ip.v4) || !HavePrimaryGlobalv6))
            {
                if (!ifa->ifa_addr)
                {
                    LogMsg("Skip interface, %s, since ifa_addr is not set.", (ifa->ifa_name) ? ifa->ifa_name: "name not found");
                    ifa = ifa->ifa_next;
                    continue;
                }
                mDNSAddr tmp6 = zeroAddr;
                if (!strcmp(buf, ifa->ifa_name))
                {
                    if (ifa->ifa_addr->sa_family == AF_INET)
                    {
                        if (mDNSIPv4AddressIsZero(v4->ip.v4) || mDNSv4AddressIsLinkLocal(&v4->ip.v4)) 
                            SetupAddr(v4, ifa->ifa_addr);
                    }
                    else if (ifa->ifa_addr->sa_family == AF_INET6)
                    {
                        SetupAddr(&tmp6, ifa->ifa_addr);
                        if (tmp6.ip.v6.b[0] >> 5 == 1)   // global prefix: 001
                        { 
                            HavePrimaryGlobalv6 = mDNStrue; 
                            *v6 = tmp6; 
                        }
                    }
                }
                else
                {
                    // We'll take a V6 address from the non-primary interface if the primary interface doesn't have a global V6 address
                    if (!HavePrimaryGlobalv6 && ifa->ifa_addr->sa_family == AF_INET6 && !v6->ip.v6.b[0])
                    {
                        SetupAddr(&tmp6, ifa->ifa_addr);
                        if (tmp6.ip.v6.b[0] >> 5 == 1) 
                            *v6 = tmp6;
                    }
                }
                ifa = ifa->ifa_next;
            }
            // Note that while we advertise v6, we still require v4 (possibly NAT'd, but not link-local) because we must use
            // V4 to communicate w/ our DNS server
        }

exit:
        CFRelease(dict);
    }
    return mStatus_NoError;
}

mDNSexport void mDNSPlatformDynDNSHostNameStatusChanged(const domainname *const dname, const mStatus status)
{
    LogInfo("mDNSPlatformDynDNSHostNameStatusChanged %d %##s", status, dname->c);
    char uname[MAX_ESCAPED_DOMAIN_NAME];    // Max legal C-string name, including terminating NUL
    ConvertDomainNameToCString(dname, uname);

    char *p = uname;
    while (*p)
    {
        *p = tolower(*p);
        if (!(*(p+1)) && *p == '.') *p = 0; // if last character, strip trailing dot
        p++;
    }

    // We need to make a CFDictionary called "State:/Network/DynamicDNS" containing (at present) a single entity.
    // That single entity is a CFDictionary with name "HostNames".
    // The "HostNames" CFDictionary contains a set of name/value pairs, where the each name is the FQDN
    // in question, and the corresponding value is a CFDictionary giving the state for that FQDN.
    // (At present we only support a single FQDN, so this dictionary holds just a single name/value pair.)
    // The CFDictionary for each FQDN holds (at present) a single name/value pair,
    // where the name is "Status" and the value is a CFNumber giving an errror code (with zero meaning success).

    const CFStringRef StateKeys [1] = { CFSTR("HostNames") };
    const CFStringRef HostKeys  [1] = { CFStringCreateWithCString(NULL, uname, kCFStringEncodingUTF8) };
    const CFStringRef StatusKeys[1] = { CFSTR("Status") };
    if (!HostKeys[0]) LogMsg("SetDDNSNameStatus: CFStringCreateWithCString(%s) failed", uname);
    else
    {
        const CFNumberRef StatusVals[1] = { CFNumberCreate(NULL, kCFNumberSInt32Type, &status) };
        if (StatusVals[0] == NULL) LogMsg("SetDDNSNameStatus: CFNumberCreate(%d) failed", status);
        else
        {
            const CFDictionaryRef HostVals[1] = { CFDictionaryCreate(NULL, (void*)StatusKeys, (void*)StatusVals, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks) };
            if (HostVals[0])
            {
                const CFDictionaryRef StateVals[1] = { CFDictionaryCreate(NULL, (void*)HostKeys, (void*)HostVals, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks) };
                if (StateVals[0])
                {
                    CFDictionaryRef StateDict = CFDictionaryCreate(NULL, (void*)StateKeys, (void*)StateVals, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
                    if (StateDict)
                    {
                        mDNSDynamicStoreSetConfig(kmDNSDynamicConfig, mDNSNULL, StateDict);
                        CFRelease(StateDict);
                    }
                    CFRelease(StateVals[0]);
                }
                CFRelease(HostVals[0]);
            }
            CFRelease(StatusVals[0]);
        }
        CFRelease(HostKeys[0]);
    }
}

// MUST be called holding the lock
mDNSlocal void SetDomainSecrets_internal(mDNS *m)
{
#ifdef NO_SECURITYFRAMEWORK
        (void) m;
    LogMsg("Note: SetDomainSecrets: no keychain support");
#else

    LogInfo("SetDomainSecrets");

    // Rather than immediately deleting all keys now, we mark them for deletion in ten seconds.
    // In the case where the user simultaneously removes their DDNS host name and the key
    // for it, this gives mDNSResponder ten seconds to gracefully delete the name from the
    // server before it loses access to the necessary key. Otherwise, we'd leave orphaned
    // address records behind that we no longer have permission to delete.
    DomainAuthInfo *ptr;
    for (ptr = m->AuthInfoList; ptr; ptr = ptr->next)
        ptr->deltime = NonZeroTime(m->timenow + mDNSPlatformOneSecond*10);

    // String Array used to write list of private domains to Dynamic Store
    CFMutableArrayRef sa = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    if (!sa) { LogMsg("SetDomainSecrets: CFArrayCreateMutable failed"); return; }
    CFIndex i;
    CFDataRef data = NULL;
    const int itemsPerEntry = 4; // domain name, key name, key value, Name value
    CFArrayRef secrets = NULL;
    int err = mDNSKeychainGetSecrets(&secrets);
    if (err || !secrets)
        LogMsg("SetDomainSecrets: mDNSKeychainGetSecrets failed error %d CFArrayRef %p", err, secrets);
    else
    {
        CFIndex ArrayCount = CFArrayGetCount(secrets);
        // Iterate through the secrets
        for (i = 0; i < ArrayCount; ++i)
        {
            int j, offset;
            CFArrayRef entry = CFArrayGetValueAtIndex(secrets, i);
            if (CFArrayGetTypeID() != CFGetTypeID(entry) || itemsPerEntry != CFArrayGetCount(entry))
            { LogMsg("SetDomainSecrets: malformed entry %d, itemsPerEntry %d", i, itemsPerEntry); continue; }
            for (j = 0; j < CFArrayGetCount(entry); ++j)
                if (CFDataGetTypeID() != CFGetTypeID(CFArrayGetValueAtIndex(entry, j)))
                { LogMsg("SetDomainSecrets: malformed entry item %d", j); continue; }

            // The names have already been vetted by the helper, but checking them again here helps humans and automated tools verify correctness

            // Max legal domainname as C-string, including space for dnsprefix and terminating NUL
            // Get DNS domain this key is for (kmDNSKcWhere)
            char stringbuf[MAX_ESCAPED_DOMAIN_NAME + sizeof(dnsprefix)];
            data = CFArrayGetValueAtIndex(entry, kmDNSKcWhere);
            if (CFDataGetLength(data) >= (int)sizeof(stringbuf))
            { LogMsg("SetDomainSecrets: Bad kSecServiceItemAttr length %d", CFDataGetLength(data)); continue; }
            CFDataGetBytes(data, CFRangeMake(0, CFDataGetLength(data)), (UInt8 *)stringbuf);
            stringbuf[CFDataGetLength(data)] = '\0';

            offset = 0;
            if (!strncmp(stringbuf, dnsprefix, strlen(dnsprefix)))
                offset = strlen(dnsprefix);

            domainname domain;
            if (!MakeDomainNameFromDNSNameString(&domain, stringbuf + offset)) { LogMsg("SetDomainSecrets: bad key domain %s", stringbuf); continue; }

            // Get key name (kmDNSKcAccount)
            data = CFArrayGetValueAtIndex(entry, kmDNSKcAccount);
            if (CFDataGetLength(data) >= (int)sizeof(stringbuf))
            { LogMsg("SetDomainSecrets: Bad kSecAccountItemAttr length %d", CFDataGetLength(data)); continue; }
            CFDataGetBytes(data, CFRangeMake(0,CFDataGetLength(data)), (UInt8 *)stringbuf);
            stringbuf[CFDataGetLength(data)] = '\0';

            domainname keyname;
            if (!MakeDomainNameFromDNSNameString(&keyname, stringbuf)) { LogMsg("SetDomainSecrets: bad key name %s", stringbuf); continue; }

            // Get key data (kmDNSKcKey)
            data = CFArrayGetValueAtIndex(entry, kmDNSKcKey);
            if (CFDataGetLength(data) >= (int)sizeof(stringbuf))
            { 
                LogMsg("SetDomainSecrets: Shared secret too long: %d", CFDataGetLength(data));
                continue;
            }
            CFDataGetBytes(data, CFRangeMake(0, CFDataGetLength(data)), (UInt8 *)stringbuf);
            stringbuf[CFDataGetLength(data)] = '\0';    // mDNS_SetSecretForDomain requires NULL-terminated C string for key

            // Get the Name of the keychain entry (kmDNSKcName) host or host:port
            // The hostname also has the port number and ":". It should take a maximum of 6 bytes.
            char hostbuf[MAX_ESCAPED_DOMAIN_NAME + 6];  // Max legal domainname as C-string, including terminating NUL
            data = CFArrayGetValueAtIndex(entry, kmDNSKcName);
            if (CFDataGetLength(data) >= (int)sizeof(hostbuf))
            { 
                LogMsg("SetDomainSecrets: host:port data too long: %d", CFDataGetLength(data));
                continue;
            }
            CFDataGetBytes(data, CFRangeMake(0,CFDataGetLength(data)), (UInt8 *)hostbuf);
            hostbuf[CFDataGetLength(data)] = '\0';

            domainname hostname;
            mDNSIPPort port;
            char *hptr;
            hptr = strchr(hostbuf, ':');

            port.NotAnInteger = 0;
            if (hptr)
            {
                mDNSu8 *p;
                mDNSu16 val = 0;

                *hptr++ = '\0';
                while(hptr && *hptr != 0)
                {
                    if (*hptr < '0' || *hptr > '9')
                    { LogMsg("SetDomainSecrets: Malformed Port number %d, val %d", *hptr, val); val = 0; break;}
                    val = val * 10 + *hptr - '0';
                    hptr++;
                }
                if (!val) continue;
                p = (mDNSu8 *)&val;
                port.NotAnInteger = p[0] << 8 | p[1];
            }
            // The hostbuf is of the format dsid@hostname:port. We don't care about the dsid.
            hptr = strchr(hostbuf, '@');
            if (hptr)
                hptr++;
            else
                hptr = hostbuf;
            if (!MakeDomainNameFromDNSNameString(&hostname, hptr)) { LogMsg("SetDomainSecrets: bad host name %s", hptr); continue; }

            DomainAuthInfo *FoundInList;
            for (FoundInList = m->AuthInfoList; FoundInList; FoundInList = FoundInList->next)
                if (SameDomainName(&FoundInList->domain, &domain)) break;

            // Uncomment the line below to view the keys as they're read out of the system keychain
            // DO NOT SHIP CODE THIS WAY OR YOU'LL LEAK SECRET DATA INTO A PUBLICLY READABLE FILE!
            //LogInfo("SetDomainSecrets: domain %##s keyname %##s key %s hostname %##s port %d", &domain.c, &keyname.c, stringbuf, hostname.c, (port.b[0] << 8 | port.b[1]));
            LogInfo("SetDomainSecrets: domain %##s keyname %##s hostname %##s port %d", &domain.c, &keyname.c, hostname.c, (port.b[0] << 8 | port.b[1]));

            // If didn't find desired domain in the list, make a new entry
            ptr = FoundInList;
            if (!FoundInList)
            {
                ptr = (DomainAuthInfo*) callocL("DomainAuthInfo", sizeof(*ptr));
                if (!ptr) { LogMsg("SetDomainSecrets: No memory"); continue; }
            }

            if (mDNS_SetSecretForDomain(m, ptr, &domain, &keyname, stringbuf, &hostname, &port) == mStatus_BadParamErr)
            {
                if (!FoundInList) mDNSPlatformMemFree(ptr);     // If we made a new DomainAuthInfo here, and it turned out bad, dispose it immediately
                continue;
            }

            ConvertDomainNameToCString(&domain, stringbuf);
            CFStringRef cfs = CFStringCreateWithCString(NULL, stringbuf, kCFStringEncodingUTF8);
            if (cfs) { CFArrayAppendValue(sa, cfs); CFRelease(cfs); }
        }
        CFRelease(secrets);
    }

    if (!privateDnsArray || !CFEqual(privateDnsArray, sa))
    {
        if (privateDnsArray)
            CFRelease(privateDnsArray);
        
        privateDnsArray = sa;
        CFRetain(privateDnsArray);
        mDNSDynamicStoreSetConfig(kmDNSPrivateConfig, mDNSNULL, privateDnsArray);
    }
    CFRelease(sa);

    CheckSuppressUnusableQuestions(m);

#endif /* NO_SECURITYFRAMEWORK */
}

mDNSexport void SetDomainSecrets(mDNS *m)
{
#if DEBUG
    // Don't get secrets for BTMM if running in debug mode
    if (!IsDebugSocketInUse())
#endif
    SetDomainSecrets_internal(m);
}

mDNSlocal void SetLocalDomains(void)
{
    CFMutableArrayRef sa = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    if (!sa) { LogMsg("SetLocalDomains: CFArrayCreateMutable failed"); return; }

    CFArrayAppendValue(sa, CFSTR("local"));
    CFArrayAppendValue(sa, CFSTR("254.169.in-addr.arpa"));
    CFArrayAppendValue(sa, CFSTR("8.e.f.ip6.arpa"));
    CFArrayAppendValue(sa, CFSTR("9.e.f.ip6.arpa"));
    CFArrayAppendValue(sa, CFSTR("a.e.f.ip6.arpa"));
    CFArrayAppendValue(sa, CFSTR("b.e.f.ip6.arpa"));

    mDNSDynamicStoreSetConfig(kmDNSMulticastConfig, mDNSNULL, sa);
    CFRelease(sa);
}

#if !MDNSRESPONDER_SUPPORTS(APPLE, NO_WAKE_FOR_NET_ACCESS)
mDNSlocal void GetCurrentPMSetting(const CFStringRef name, mDNSs32 *val)
{
    CFDictionaryRef dict = SCDynamicStoreCopyValue(NULL, NetworkChangedKey_PowerSettings);
    if (!dict)
    {
        LogSPS("GetCurrentPMSetting: Could not get IOPM CurrentSettings dict");
    }
    else
    {
        CFNumberRef number = CFDictionaryGetValue(dict, name);
        if ((number == NULL) || CFGetTypeID(number) != CFNumberGetTypeID() || !CFNumberGetValue(number, kCFNumberSInt32Type, val))
            *val = 0;
        CFRelease(dict);
    }
}
#endif

#if APPLE_OSX_mDNSResponder

static CFMutableDictionaryRef spsStatusDict = NULL;
static const CFStringRef kMetricRef = CFSTR("Metric");

mDNSlocal void SPSStatusPutNumber(CFMutableDictionaryRef dict, const mDNSu8* const ptr, CFStringRef key)
{
    mDNSu8 tmp = (ptr[0] - '0') * 10 + ptr[1] - '0';
    CFNumberRef num = CFNumberCreate(NULL, kCFNumberSInt8Type, &tmp);
    if (num == NULL)
        LogMsg("SPSStatusPutNumber: Could not create CFNumber");
    else
    {
        CFDictionarySetValue(dict, key, num);
        CFRelease(num);
    }
}

mDNSlocal CFMutableDictionaryRef SPSCreateDict(const mDNSu8* const ptr)
{
    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (!dict) { LogMsg("SPSCreateDict: Could not create CFDictionary dict"); return dict; }

    char buffer[1024];
    buffer[mDNS_snprintf(buffer, sizeof(buffer), "%##s", ptr) - 1] = 0;
    CFStringRef spsname = CFStringCreateWithCString(NULL, buffer, kCFStringEncodingUTF8);
    if (!spsname) { LogMsg("SPSCreateDict: Could not create CFString spsname full"); CFRelease(dict); return NULL; }
    CFDictionarySetValue(dict, CFSTR("FullName"), spsname);
    CFRelease(spsname);

    if (ptr[0] >=  2) SPSStatusPutNumber(dict, ptr + 1, CFSTR("Type"));
    if (ptr[0] >=  5) SPSStatusPutNumber(dict, ptr + 4, CFSTR("Portability"));
    if (ptr[0] >=  8) SPSStatusPutNumber(dict, ptr + 7, CFSTR("MarginalPower"));
    if (ptr[0] >= 11) SPSStatusPutNumber(dict, ptr +10, CFSTR("TotalPower"));

    mDNSu32 tmp = SPSMetric(ptr);
    CFNumberRef num = CFNumberCreate(NULL, kCFNumberSInt32Type, &tmp);
    if (num == NULL)
        LogMsg("SPSCreateDict: Could not create CFNumber");
    else
    {
        CFDictionarySetValue(dict, kMetricRef, num);
        CFRelease(num);
    }

    if (ptr[0] >= 12)
    {
        memcpy(buffer, ptr + 13, ptr[0] - 12);
        buffer[ptr[0] - 12] = 0;
        spsname = CFStringCreateWithCString(NULL, buffer, kCFStringEncodingUTF8);
        if (!spsname) { LogMsg("SPSCreateDict: Could not create CFString spsname"); CFRelease(dict); return NULL; }
        else
        {
            CFDictionarySetValue(dict, CFSTR("PrettyName"), spsname);
            CFRelease(spsname);
        }
    }

    return dict;
}

mDNSlocal CFComparisonResult CompareSPSEntries(const void *val1, const void *val2, void *context)
{
    (void)context;
    return CFNumberCompare((CFNumberRef)CFDictionaryGetValue((CFDictionaryRef)val1, kMetricRef),
                           (CFNumberRef)CFDictionaryGetValue((CFDictionaryRef)val2, kMetricRef),
                           NULL);
}

mDNSlocal void UpdateSPSStatus(mDNS *const m, DNSQuestion *question, const ResourceRecord *const answer, QC_result AddRecord)
{
    NetworkInterfaceInfo* info = (NetworkInterfaceInfo*)question->QuestionContext;
    debugf("UpdateSPSStatus: %s %##s %s %s", info->ifname, question->qname.c, AddRecord ? "Add" : "Rmv", answer ? RRDisplayString(m, answer) : "<null>");

    mDNS_Lock(m);
    mDNS_UpdateAllowSleep(m);
    mDNS_Unlock(m);

    if (answer && SPSMetric(answer->rdata->u.name.c) > 999999) return;  // Ignore instances with invalid names

    if (!spsStatusDict)
    {
        spsStatusDict = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        if (!spsStatusDict) { LogMsg("UpdateSPSStatus: Could not create CFDictionary spsStatusDict"); return; }
    }

    CFStringRef ifname = CFStringCreateWithCString(NULL, info->ifname, kCFStringEncodingUTF8);
    if (!ifname) { LogMsg("UpdateSPSStatus: Could not create CFString ifname"); return; }

    CFMutableArrayRef array = NULL;

    if (!CFDictionaryGetValueIfPresent(spsStatusDict, ifname, (const void**) &array))
    {
        array = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
        if (!array) { LogMsg("UpdateSPSStatus: Could not create CFMutableArray"); CFRelease(ifname); return; }
        CFDictionarySetValue(spsStatusDict, ifname, array);
        CFRelease(array); // let go of our reference, now that the dict has one
    }
    else
    if (!array) { LogMsg("UpdateSPSStatus: Could not get CFMutableArray for %s", info->ifname); CFRelease(ifname); return; }

    if (!answer) // special call that means the question has been stopped (because the interface is going away)
        CFArrayRemoveAllValues(array);
    else
    {
        CFMutableDictionaryRef dict = SPSCreateDict(answer->rdata->u.name.c);
        if (!dict) { CFRelease(ifname); return; }

        if (AddRecord)
        {
            if (!CFArrayContainsValue(array, CFRangeMake(0, CFArrayGetCount(array)), dict))
            {
                int i=0;
                for (i=0; i<CFArrayGetCount(array); i++)
                    if (CompareSPSEntries(CFArrayGetValueAtIndex(array, i), dict, NULL) != kCFCompareLessThan)
                        break;
                CFArrayInsertValueAtIndex(array, i, dict);
            }
            else LogMsg("UpdateSPSStatus: %s array already contains %##s", info->ifname, answer->rdata->u.name.c);
        }
        else
        {
            CFIndex i = CFArrayGetFirstIndexOfValue(array, CFRangeMake(0, CFArrayGetCount(array)), dict);
            if (i != -1) CFArrayRemoveValueAtIndex(array, i);
            else LogMsg("UpdateSPSStatus: %s array does not contain %##s", info->ifname, answer->rdata->u.name.c);
        }

        CFRelease(dict);
    }

    if (!m->ShutdownTime) mDNSDynamicStoreSetConfig(kmDNSSleepProxyServersState, info->ifname, array);

    CFRelease(ifname);
}

mDNSlocal mDNSs32 GetSystemSleepTimerSetting(void)
{
    mDNSs32 val = -1;
    SCDynamicStoreRef store = SCDynamicStoreCreate(NULL, CFSTR("mDNSResponder:GetSystemSleepTimerSetting"), NULL, NULL);
    if (!store)
        LogMsg("GetSystemSleepTimerSetting: SCDynamicStoreCreate failed: %s", SCErrorString(SCError()));
    else
    {
        CFDictionaryRef dict = SCDynamicStoreCopyValue(store, NetworkChangedKey_PowerSettings);
        if (dict)
        {
            CFNumberRef number = CFDictionaryGetValue(dict, CFSTR("System Sleep Timer"));
            if (number != NULL) CFNumberGetValue(number, kCFNumberSInt32Type, &val);
            CFRelease(dict);
        }
        CFRelease(store);
    }
    return val;
}

mDNSlocal void SetSPS(mDNS *const m)
{
    
    // If we ever want to know InternetSharing status in the future, use DNSXEnableProxy()
    mDNSu8 sps = (OfferSleepProxyService && GetSystemSleepTimerSetting() == 0) ? mDNSSleepProxyMetric_IncidentalSoftware : 0;

    // For devices that are not running NAT, but are set to never sleep, we may choose to act
    // as a Sleep Proxy, but only for non-portable Macs (Portability > 35 means nominal weight < 3kg)
    //if (sps > mDNSSleepProxyMetric_PrimarySoftware && SPMetricPortability > 35) sps = 0;

    // If we decide to let laptops act as Sleep Proxy, we should do it only when running on AC power, not on battery

    // For devices that are unable to sleep at all to save power, or save 1W or less by sleeping,
    // it makes sense for them to offer low-priority Sleep Proxy service on the network.
    // We rate such a device as metric 70 ("Incidentally Available Hardware")
    if (SPMetricMarginalPower <= 60 && !sps) sps = mDNSSleepProxyMetric_IncidentalHardware;

    // If the launchd plist specifies an explicit value for the Intent Metric, then use that instead of the
    // computed value (currently 40 "Primary Network Infrastructure Software" or 80 "Incidentally Available Software")
    if (sps && OfferSleepProxyService && OfferSleepProxyService < 100) sps = OfferSleepProxyService;

#ifdef NO_APPLETV_SLEEP_PROXY_ON_WIFI
    // AppleTVs are not reliable sleep proxy servers on WiFi. Do not offer to be a BSP if the WiFi interface is active.
    if (IsAppleTV())
    {
        NetworkInterfaceInfo *intf  = mDNSNULL;
        mDNSEthAddr           bssid = zeroEthAddr;
        for (intf = GetFirstActiveInterface(m->HostInterfaces); intf; intf = GetFirstActiveInterface(intf->next))
        {
            if (intf->InterfaceID == AWDLInterfaceID) continue;
            bssid = GetBSSID(intf->ifname);
            if (!mDNSSameEthAddress(&bssid, &zeroEthAddr))
            {
                LogMsg("SetSPS: AppleTV on WiFi - not advertising BSP services");
                sps = 0;
                break;
            }
        }
    }
#endif  //  NO_APPLETV_SLEEP_PROXY_ON_WIFI

    mDNSCoreBeSleepProxyServer(m, sps, SPMetricPortability, SPMetricMarginalPower, SPMetricTotalPower, SPMetricFeatures);
}

// The definitions below should eventually come from some externally-supplied header file.
// However, since these definitions can't really be changed without breaking binary compatibility,
// they should never change, so in practice it should not be a big problem to have them defined here.

enum
{                               // commands from the daemon to the driver
    cmd_mDNSOffloadRR = 21,     // give the mdns update buffer to the driver
};

typedef union { void *ptr; mDNSOpaque64 sixtyfourbits; } FatPtr;

typedef struct
{                                       // cmd_mDNSOffloadRR structure
    uint32_t command;                 // set to OffloadRR
    uint32_t rrBufferSize;            // number of bytes of RR records
    uint32_t numUDPPorts;             // number of SRV UDP ports
    uint32_t numTCPPorts;             // number of SRV TCP ports
    uint32_t numRRRecords;            // number of RR records
    uint32_t compression;             // rrRecords - compression is base for compressed strings
    FatPtr rrRecords;                 // address of array of pointers to the rr records
    FatPtr udpPorts;                  // address of udp port list (SRV)
    FatPtr tcpPorts;                  // address of tcp port list (SRV)
} mDNSOffloadCmd;

#include <IOKit/IOKitLib.h>
#include <dns_util.h>

mDNSlocal mDNSu32 GetPortArray(int trans, mDNSIPPort *portarray)
{
    mDNS *const m = &mDNSStorage;
    const domainlabel *const tp = (trans == mDNSTransport_UDP) ? (const domainlabel *)"\x4_udp" : (const domainlabel *)"\x4_tcp";
    mDNSu32 count = 0;

    AuthRecord *rr;
    for (rr = m->ResourceRecords; rr; rr=rr->next)
    {
        if (rr->resrec.rrtype == kDNSType_SRV && SameDomainLabel(ThirdLabel(rr->resrec.name)->c, tp->c))
        {
            if (!portarray)
                count++;
            else
            {
                mDNSu32 i;
                for (i = 0; i < count; i++)
                    if (mDNSSameIPPort(portarray[i], rr->resrec.rdata->u.srv.port))
                        break;

                // Add it into the port list only if it not already present in the list
                if (i >= count)
                    portarray[count++] = rr->resrec.rdata->u.srv.port;
            }
        }
    }
    return(count);
}

#if APPLE_OSX_mDNSResponder && TARGET_OS_OSX
mDNSlocal mDNSBool SupportsTCPKeepAlive()
{
    IOReturn  ret      = kIOReturnSuccess;
    CFTypeRef obj      = NULL;
    mDNSBool  supports = mDNSfalse;

    ret = IOPlatformCopyFeatureActive(CFSTR("TCPKeepAliveDuringSleep"), &obj);
    if ((kIOReturnSuccess == ret) && (obj != NULL))
    {
        supports = (obj ==  kCFBooleanTrue)? mDNStrue : mDNSfalse;
        CFRelease(obj);
    }
    LogSPS("%s: The hardware %s TCP Keep Alive", __func__, (supports ? "supports" : "does not support"));
    return supports;
}

mDNSlocal mDNSBool OnBattery(void)
{
    CFTypeRef powerInfo = IOPSCopyPowerSourcesInfo();
    CFTypeRef powerSrc  = IOPSGetProvidingPowerSourceType(powerInfo);
    mDNSBool  result    = mDNSfalse;

    if (powerInfo != NULL)
    {
        result = CFEqual(CFSTR(kIOPSBatteryPowerValue), powerSrc);
        CFRelease(powerInfo);
    }
    LogSPS("%s: The system is on %s", __func__, (result)? "Battery" : "AC Power");
    return result;
}
#endif

#define TfrRecordToNIC(RR) \
    ((!(RR)->resrec.InterfaceID && ((RR)->ForceMCast || IsLocalDomain((RR)->resrec.name))))

mDNSlocal mDNSu32 CountProxyRecords(uint32_t *const numbytes, mDNSBool TCPKAOnly, mDNSBool supportsTCPKA)
{
    mDNS *const m = &mDNSStorage;
    *numbytes = 0;
    uint32_t count = 0;
    mDNSBool isKeepAliveRecord = mDNSfalse;

    AuthRecord *rr;

    for (rr = m->ResourceRecords; rr; rr=rr->next)
    {
        if (!(rr->AuthFlags & AuthFlagsWakeOnly) && rr->resrec.RecordType > kDNSRecordTypeDeregistering)
        {
#if APPLE_OSX_mDNSResponder && TARGET_OS_OSX
            isKeepAliveRecord = mDNS_KeepaliveRecord(&rr->resrec);
            // Skip over all other records if we are registering TCP KeepAlive records only
            // Skip over TCP KeepAlive records if the policy prohibits it or if the interface does not support TCP Keepalive.
            if ((TCPKAOnly && !isKeepAliveRecord) || (isKeepAliveRecord && !supportsTCPKA))
                continue;
#else
            (void) TCPKAOnly;     // unused
            (void) supportsTCPKA; // unused
#endif
            if (TfrRecordToNIC(rr))
            {
                // For KeepAlive records, use an estimated length of 256, which is the maximum size.
                const uint32_t rdataLen   = isKeepAliveRecord ? ((uint32_t)sizeof(UTF8str255)) : rr->resrec.rdestimate;
                const uint32_t recordSize = DomainNameLength(rr->resrec.name) + 10 + rdataLen;
                *numbytes += recordSize;
                LogSPS("CountProxyRecords: %3u size %5u total %5u %s", count, recordSize, *numbytes, ARDisplayString(m,rr));
                count++;
            }
        }
    }
    return(count);
}

mDNSlocal void GetProxyRecords(DNSMessage *const msg, uint32_t *const numbytes, FatPtr *const records,
    uint32_t *outRecordCount, NetworkInterfaceInfo *const intf, mDNSBool TCPKAOnly, mDNSBool supportsTCPKA)
{
    mDNS *const m = &mDNSStorage;
    mDNSu8 *p = msg->data;
    const mDNSu8 *const limit = p + *numbytes;
    InitializeDNSMessage(&msg->h, zeroID, zeroID);

    uint32_t count = 0;
    AuthRecord *rr;

    for (rr = m->ResourceRecords; rr; rr=rr->next)
    {
        if (!(rr->AuthFlags & AuthFlagsWakeOnly) && rr->resrec.RecordType > kDNSRecordTypeDeregistering)
        {
#if APPLE_OSX_mDNSResponder && TARGET_OS_OSX
            const mDNSBool isKeepAliveRecord = mDNS_KeepaliveRecord(&rr->resrec);

            // Skip over all other records if we are registering TCP KeepAlive records only
            // Skip over TCP KeepAlive records if the policy prohibits it or if the interface does not support TCP Keepalive
            // supportsTCPKA is set to true if both policy and interface allow TCP Keepalive
            if ((TCPKAOnly && !isKeepAliveRecord) || (isKeepAliveRecord && !supportsTCPKA))
                continue;

            // Update the record before calculating the number of bytes required
            // We offload the TCP Keepalive record even if the update fails. When the driver gets the record, it will
            // attempt to update the record again.
            if (isKeepAliveRecord)
            {
                if (UpdateKeepaliveRData(m, rr, intf, mDNSfalse, mDNSNULL) != mStatus_NoError)
                {
                    LogSPS("GetProxyRecords: Failed to update keepalive record - %s", ARDisplayString(m, rr));
                    continue;
                }
                // Offload only Valid Keepalive records
                if (!mDNSValidKeepAliveRecord(rr))
                {
                    continue;
                }
            }
#else
            (void) intf;          // unused
            (void) TCPKAOnly;     // unused
            (void) supportsTCPKA; // unused
#endif
            if (TfrRecordToNIC(rr))
            {
                records[count].sixtyfourbits = zeroOpaque64;
                records[count].ptr = p;
                if (rr->resrec.RecordType & kDNSRecordTypeUniqueMask)
                    rr->resrec.rrclass |= kDNSClass_UniqueRRSet;    // Temporarily set the 'unique' bit so PutResourceRecord will set it
                p = PutResourceRecordTTLWithLimit(msg, p, &msg->h.mDNS_numUpdates, &rr->resrec, rr->resrec.rroriginalttl, limit);
                rr->resrec.rrclass &= ~kDNSClass_UniqueRRSet;       // Make sure to clear 'unique' bit back to normal state
                LogSPS("GetProxyRecords: %3d start %p end %p size %5d total %5d %s",
                       count, records[count].ptr, p, p - (mDNSu8 *)records[count].ptr, p - msg->data, ARDisplayString(m,rr));
                count++;
            }
        }
    }
    *numbytes = p - msg->data;
    if (outRecordCount) *outRecordCount = count;
}

mDNSexport mDNSBool SupportsInNICProxy(NetworkInterfaceInfo *const intf)
{
    if(!UseInternalSleepProxy)
    {
        LogMsg("SupportsInNICProxy: Internal Sleep Proxy is disabled");
        return mDNSfalse;
    }
    return CheckInterfaceSupport(intf, mDNS_IOREG_KEY);
}

// Called with the lock held
mDNSexport mStatus ActivateLocalProxy(NetworkInterfaceInfo *const intf, mDNSBool offloadKeepAlivesOnly, mDNSBool *keepaliveOnly)
{
    mStatus      result        = mStatus_UnknownErr;
    mDNSBool     TCPKAOnly     = mDNSfalse;
    mDNSBool     supportsTCPKA = mDNSfalse;
    io_service_t service       = IOServiceGetMatchingService(kIOMasterPortDefault, IOBSDNameMatching(kIOMasterPortDefault, 0, intf->ifname));

#if APPLE_OSX_mDNSResponder && TARGET_OS_OSX
    // Check if the interface supports TCP Keepalives and the system policy says it is ok to offload TCP Keepalive records
    supportsTCPKA = (InterfaceSupportsKeepAlive(intf) && SupportsTCPKeepAlive()) ? mDNStrue : mDNSfalse;
    if (!offloadKeepAlivesOnly)
    {
        // Only TCP Keepalive records are to be offloaded if
        // - The system is on battery
        // - OR wake for network access is not set but powernap is enabled
        TCPKAOnly = supportsTCPKA && ((mDNSStorage.SystemWakeOnLANEnabled == mDNS_WakeOnBattery) || OnBattery());
    }
    else
    {
        TCPKAOnly = mDNStrue;
    }
#else
    (void)offloadKeepAlivesOnly; // Unused.
#endif
    if (!service) { LogMsg("ActivateLocalProxy: No service for interface %s", intf->ifname); return(mStatus_UnknownErr); }

    io_name_t       n1, n2;
    IOObjectGetClass(service, n1);
    
    CFTypeRef       ref;
    io_object_t     parent;
    kern_return_t   kr = RegistryEntrySearchCFPropertyAndIOObject(service, kIOServicePlane, CFSTR(mDNS_IOREG_KEY), &ref, &parent);
    IOObjectRelease(service);
    if (kr != KERN_SUCCESS) LogSPS("ActivateLocalProxy: No mDNS_IOREG_KEY for interface %s/%s kr %d", intf->ifname, n1, kr);
    else
    {
        IOObjectGetClass(parent, n2);
        LogSPS("ActivateLocalProxy: Interface %s service %s parent %s", intf->ifname, n1, n2);

        if (CFGetTypeID(ref) != CFStringGetTypeID() || !CFEqual(ref, CFSTR(mDNS_IOREG_VALUE)))
            LogMsg("ActivateLocalProxy: mDNS_IOREG_KEY for interface %s/%s/%s value %s != %s",
                   intf->ifname, n1, n2, CFStringGetCStringPtr(ref, mDNSNULL), mDNS_IOREG_VALUE);
        else if (!UseInternalSleepProxy)
            LogSPS("ActivateLocalProxy: Not using internal (NIC) sleep proxy for interface %s", intf->ifname);
        else
        {
            io_connect_t conObj;
            kr = IOServiceOpen(parent, mach_task_self(), mDNS_USER_CLIENT_CREATE_TYPE, &conObj);
            if (kr != KERN_SUCCESS) LogMsg("ActivateLocalProxy: IOServiceOpen for %s/%s/%s failed %d", intf->ifname, n1, n2, kr);
            else
            {
                mDNSOffloadCmd cmd;
                mDNSPlatformMemZero(&cmd, sizeof(cmd)); // When compiling 32-bit, make sure top 32 bits of 64-bit pointers get initialized to zero
                cmd.command       = cmd_mDNSOffloadRR;
                cmd.numUDPPorts   = TCPKAOnly ? 0 : GetPortArray(mDNSTransport_UDP, mDNSNULL);
                cmd.numTCPPorts   = TCPKAOnly ? 0 : GetPortArray(mDNSTransport_TCP, mDNSNULL);
                cmd.numRRRecords  = CountProxyRecords(&cmd.rrBufferSize, TCPKAOnly, supportsTCPKA);
                cmd.compression   = sizeof(DNSMessageHeader);

                DNSMessage *msg   = (DNSMessage *) callocL("mDNSOffloadCmd msg", sizeof(DNSMessageHeader) + cmd.rrBufferSize);
                cmd.rrRecords.ptr = cmd.numRRRecords ? callocL("mDNSOffloadCmd rrRecords", cmd.numRRRecords * sizeof(FatPtr))     : NULL;
                cmd.udpPorts.ptr  = cmd.numUDPPorts  ? callocL("mDNSOffloadCmd udpPorts" , cmd.numUDPPorts  * sizeof(mDNSIPPort)) : NULL;
                cmd.tcpPorts.ptr  = cmd.numTCPPorts  ? callocL("mDNSOffloadCmd tcpPorts" , cmd.numTCPPorts  * sizeof(mDNSIPPort)) : NULL;

                LogSPS("ActivateLocalProxy: msg %p %u RR %p %u, UDP %p %u, TCP %p %u",
                       msg, cmd.rrBufferSize,
                       cmd.rrRecords.ptr, cmd.numRRRecords,
                       cmd.udpPorts.ptr, cmd.numUDPPorts,
                       cmd.tcpPorts.ptr, cmd.numTCPPorts);

                if (msg && cmd.rrRecords.ptr)
                {
                    GetProxyRecords(msg, &cmd.rrBufferSize, cmd.rrRecords.ptr, &cmd.numRRRecords, intf, TCPKAOnly, supportsTCPKA);
                }
                if (cmd.udpPorts.ptr) cmd.numUDPPorts = TCPKAOnly ? 0 : GetPortArray(mDNSTransport_UDP, cmd.udpPorts.ptr);
                if (cmd.tcpPorts.ptr) cmd.numTCPPorts = TCPKAOnly ? 0 : GetPortArray(mDNSTransport_TCP, cmd.tcpPorts.ptr);

                char outputData[2];
                size_t outputDataSize = sizeof(outputData);
                kr = IOConnectCallStructMethod(conObj, 0, &cmd, sizeof(cmd), outputData, &outputDataSize);
                LogSPS("ActivateLocalProxy: IOConnectCallStructMethod for %s/%s/%s %d", intf->ifname, n1, n2, kr);
                if (kr == KERN_SUCCESS) result = mStatus_NoError;

                if (cmd.tcpPorts.ptr) freeL("mDNSOffloadCmd udpPorts",  cmd.tcpPorts.ptr);
                if (cmd.udpPorts.ptr) freeL("mDNSOffloadCmd tcpPorts",  cmd.udpPorts.ptr);
                if (cmd.rrRecords.ptr) freeL("mDNSOffloadCmd rrRecords", cmd.rrRecords.ptr);
                if (msg) freeL("mDNSOffloadCmd msg",       msg);
                IOServiceClose(conObj);
            }
        }
        CFRelease(ref);
        IOObjectRelease(parent);
    }
    *keepaliveOnly = (TCPKAOnly && supportsTCPKA) ? mDNStrue : mDNSfalse;
    return result;
}

#endif // APPLE_OSX_mDNSResponder

mDNSlocal mDNSu8 SystemWakeForNetworkAccess(void)
{
#if MDNSRESPONDER_SUPPORTS(APPLE, NO_WAKE_FOR_NET_ACCESS)
    LogRedact(MDNS_LOG_CATEGORY_SPS, MDNS_LOG_DEBUG, "SystemWakeForNetworkAccess: compile-time disabled");
    return ((mDNSu8)mDNS_NoWake);
#else
    mDNSs32 val = 0;
    mDNSu8  ret = (mDNSu8)mDNS_NoWake;

    if (DisableSleepProxyClient)
    {
       LogSPS("SystemWakeForNetworkAccess: Sleep Proxy Client disabled by command-line option");
       return ret;
    }

    GetCurrentPMSetting(CFSTR("Wake On LAN"), &val);

    ret = (mDNSu8)(val != 0) ? mDNS_WakeOnAC : mDNS_NoWake;

#if APPLE_OSX_mDNSResponder && TARGET_OS_OSX
    // If we have TCP Keepalive support, system is capable of registering for TCP Keepalives.
    // Further policy decisions on whether to offload the records is handled during sleep processing.
    if ((ret == mDNS_NoWake) && SupportsTCPKeepAlive())
        ret = (mDNSu8)mDNS_WakeOnBattery;
#endif // APPLE_OSX_mDNSResponder

    LogSPS("SystemWakeForNetworkAccess: Wake On LAN: %d", ret);
    return ret;
#endif
}

mDNSlocal mDNSBool SystemSleepOnlyIfWakeOnLAN(void)
{
    mDNSs32 val = 0;
    // PrioritizeNetworkReachabilityOverSleep has been deprecated.
    // GetCurrentPMSetting(CFSTR("PrioritizeNetworkReachabilityOverSleep"), &val);
    // Statically set the PrioritizeNetworkReachabilityOverSleep value to 1 for AppleTV
    if (IsAppleTV())
        val = 1;
    return val != 0 ? mDNStrue : mDNSfalse;
}

mDNSlocal mDNSBool IsAppleNetwork(mDNS *const m)
{
    DNSServer *s;
    // Determine if we're on AppleNW based on DNSServer having 17.x.y.z IPv4 addr
    for (s = m->DNSServers; s; s = s->next)
    {
        if (s->addr.ip.v4.b[0] == 17)
        {     
            LogInfo("IsAppleNetwork: Found 17.x.y.z DNSServer concluding that we are on AppleNW: %##s %#a", s->domain.c, &s->addr);
            return mDNStrue;
        }     
    }
    return mDNSfalse;
}

// Called with KQueueLock & mDNS lock
// SetNetworkChanged is allowed to shorten (but not extend) the pause while we wait for configuration changes to settle
mDNSlocal void SetNetworkChanged(mDNSs32 delay)
{
    mDNS *const m = &mDNSStorage;
    mDNS_CheckLock(m);
    if (!m->NetworkChanged || m->NetworkChanged - NonZeroTime(m->timenow + delay) > 0)
    {
        m->NetworkChanged = NonZeroTime(m->timenow + delay);
        LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_INFO, "SetNetworkChanged: Scheduling in %d ticks", delay);
    }
    else
   	{
        LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_INFO,
            "SetNetworkChanged: *NOT* increasing delay from %d to %d", m->NetworkChanged - m->timenow, delay);
    }
}

// Called with KQueueLock & mDNS lock
mDNSlocal void SetKeyChainTimer(mDNSs32 delay)
{
    mDNS *const m = &mDNSStorage;
    // If it's not set or it needs to happen sooner than when it's currently set
    if (!m->p->KeyChainTimer || m->p->KeyChainTimer - NonZeroTime(m->timenow + delay) > 0)
    {
        m->p->KeyChainTimer = NonZeroTime(m->timenow + delay);
        LogInfo("SetKeyChainTimer: %d", delay);
    }
}

mDNSexport void mDNSMacOSXNetworkChanged(void)
{
    mDNS *const m = &mDNSStorage;
    LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_INFO,
        "*** Network Configuration Change ***  %d ticks late" PUB_S,
        m->NetworkChanged ? mDNS_TimeNow(m) - m->NetworkChanged : 0,
        m->NetworkChanged ? "" : " (no scheduled configuration change)");
    m->NetworkChanged = 0;       // If we received a network change event and deferred processing, we're now dealing with it

    // If we have *any* TENTATIVE IPv6 addresses, wait until they've finished configuring
    int InfoSocket = socket(AF_INET6, SOCK_DGRAM, 0);
    if (InfoSocket > 0)
    {
        mDNSBool tentative = mDNSfalse;
        struct ifaddrs *ifa = myGetIfAddrs(1);
        while (ifa)
        {
            if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET6)
            {
                struct in6_ifreq ifr6;
                mDNSPlatformMemZero((char *)&ifr6, sizeof(ifr6));
                strlcpy(ifr6.ifr_name, ifa->ifa_name, sizeof(ifr6.ifr_name));
                ifr6.ifr_addr = *(struct sockaddr_in6 *)ifa->ifa_addr;
                // We need to check for IN6_IFF_TENTATIVE here, not IN6_IFF_NOTREADY, because
                // IN6_IFF_NOTREADY includes both IN6_IFF_TENTATIVE and IN6_IFF_DUPLICATED addresses.
                // We can expect that an IN6_IFF_TENTATIVE address will shortly become ready,
                // but an IN6_IFF_DUPLICATED address may not.
                if (ioctl(InfoSocket, SIOCGIFAFLAG_IN6, &ifr6) != -1)
                {
                    if (ifr6.ifr_ifru.ifru_flags6 & IN6_IFF_TENTATIVE)
                    {
                        LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_INFO,
                            "*** Network Configuration Change ***  IPv6 address " PRI_IPv6_ADDR " TENTATIVE, will retry",
                            &ifr6.ifr_addr.sin6_addr);
                        tentative = mDNStrue;
                        // no need to check other interfaces if we already found out that one interface is TENTATIVE
                        break;
                    }
                }
            }
            ifa = ifa->ifa_next;
        }
        close(InfoSocket);
        if (tentative)
        {
            mDNS_Lock(m);
            SetNetworkChanged(mDNSPlatformOneSecond / 2);
            mDNS_Unlock(m);
            return;
        }
        LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_INFO,
            "*** Network Configuration Change ***  No IPv6 address TENTATIVE, will continue");
    }

    mDNSs32 utc = mDNSPlatformUTC();
    m->SystemWakeOnLANEnabled = SystemWakeForNetworkAccess();
    m->SystemSleepOnlyIfWakeOnLAN = SystemSleepOnlyIfWakeOnLAN();
    MarkAllInterfacesInactive(utc);
    UpdateInterfaceList(utc);
    ClearInactiveInterfaces(utc);
    SetupActiveInterfaces(utc);
    ReorderInterfaceList();

#if APPLE_OSX_mDNSResponder
    SetSPS(m);

    NetworkInterfaceInfoOSX *i;
    for (i = m->p->InterfaceList; i; i = i->next)
    {
        if (!m->SPSSocket) // Not being Sleep Proxy Server; close any open BPF fds
        {
            if (i->BPF_fd >= 0 && CountProxyTargets(i, mDNSNULL, mDNSNULL) == 0)
                CloseBPF(i);
        }
        else // else, we're Sleep Proxy Server; open BPF fds
        {
            if (i->Exists && (i->Registered == i) && SPSInterface(i) && i->BPF_fd == -1)
            {
                LogMsg("%s mDNSMacOSXNetworkChanged: requesting BPF", i->ifinfo.ifname);
                i->BPF_fd = -2;
                mDNSRequestBPF();
            }
        }
    }

#endif // APPLE_OSX_mDNSResponder

    uDNS_SetupDNSConfig(m);
    mDNS_ConfigChanged(m);

    if (IsAppleNetwork(m) != mDNS_McastTracingEnabled)
    {
        mDNS_McastTracingEnabled = mDNS_McastTracingEnabled ? mDNSfalse : mDNStrue; 
        LogInfo("mDNSMacOSXNetworkChanged: Multicast Tracing %s", mDNS_McastTracingEnabled ? "Enabled" : "Disabled");
        UpdateDebugState();
    }

}

// Copy the fourth slash-delimited element from either:
//   State:/Network/Interface/<bsdname>/IPv4
// or
//   Setup:/Network/Service/<servicename>/Interface
mDNSlocal CFStringRef CopyNameFromKey(CFStringRef key)
{
    CFArrayRef a;
    CFStringRef name = NULL;

    a = CFStringCreateArrayBySeparatingStrings(NULL, key, CFSTR("/"));
    if (a && CFArrayGetCount(a) == 5) name = CFRetain(CFArrayGetValueAtIndex(a, 3));
    if (a != NULL) CFRelease(a);

    return name;
}

// Whether a key from a network change notification corresponds to
// an IP service that is explicitly configured for IPv4 Link Local
mDNSlocal int ChangedKeysHaveIPv4LL(CFArrayRef inkeys)
{
    CFDictionaryRef dict = NULL;
    CFMutableArrayRef a;
    const void **keys = NULL, **vals = NULL;
    CFStringRef pattern = NULL;
    int i, ic, j, jc;
    int found = 0;

    jc = CFArrayGetCount(inkeys);
    if (!jc) goto done;

    a = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    if (a == NULL) goto done;

    // Setup:/Network/Service/[^/]+/Interface
    pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL, kSCDynamicStoreDomainSetup, kSCCompAnyRegex, kSCEntNetInterface);
    if (pattern == NULL) goto done;
    CFArrayAppendValue(a, pattern);
    CFRelease(pattern);

    // Setup:/Network/Service/[^/]+/IPv4
    pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL, kSCDynamicStoreDomainSetup, kSCCompAnyRegex, kSCEntNetIPv4);
    if (pattern == NULL) goto done;
    CFArrayAppendValue(a, pattern);
    CFRelease(pattern);

    dict = SCDynamicStoreCopyMultiple(NULL, NULL, a);
    CFRelease(a);

    if (!dict)
    {
        LogMsg("ChangedKeysHaveIPv4LL: Empty dictionary");
        goto done;
    }

    ic = CFDictionaryGetCount(dict);
    vals = (const void **) mDNSPlatformMemAllocate(sizeof(void *) * ic);
    keys = (const void **) mDNSPlatformMemAllocate(sizeof(void *) * ic);
    CFDictionaryGetKeysAndValues(dict, keys, vals);

    // For each key we were given...
    for (j = 0; j < jc; j++)
    {
        CFStringRef key = CFArrayGetValueAtIndex(inkeys, j);
        CFStringRef ifname = NULL;

        char buf[256];

        // It would be nice to use a regex here
        if (!CFStringHasPrefix(key, CFSTR("State:/Network/Interface/")) || !CFStringHasSuffix(key, kSCEntNetIPv4)) continue;

        if ((ifname = CopyNameFromKey(key)) == NULL) continue;
        if (mDNS_LoggingEnabled)
        {
            if (!CFStringGetCString(ifname, buf, sizeof(buf), kCFStringEncodingUTF8)) buf[0] = 0;
            LogInfo("ChangedKeysHaveIPv4LL: potential ifname %s", buf);
        }

        // Loop over the interfaces to find matching the ifname, and see if that one has kSCValNetIPv4ConfigMethodLinkLocal
        for (i = 0; i < ic; i++)
        {
            CFDictionaryRef ipv4dict;
            CFStringRef name;
            CFStringRef serviceid;
            CFStringRef configmethod;

            if (!CFStringHasSuffix(keys[i], kSCEntNetInterface)) continue;

            if (CFDictionaryGetTypeID() != CFGetTypeID(vals[i])) continue;

            if ((name = CFDictionaryGetValue(vals[i], kSCPropNetInterfaceDeviceName)) == NULL) continue;

            if (!CFEqual(ifname, name)) continue;

            if ((serviceid = CopyNameFromKey(keys[i])) == NULL) continue;
            if (mDNS_LoggingEnabled)
            {
                if (!CFStringGetCString(serviceid, buf, sizeof(buf), kCFStringEncodingUTF8)) buf[0] = 0;
                LogInfo("ChangedKeysHaveIPv4LL: found serviceid %s", buf);
            }

            pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL, kSCDynamicStoreDomainSetup, serviceid, kSCEntNetIPv4);
            CFRelease(serviceid);
            if (pattern == NULL) continue;

            ipv4dict = CFDictionaryGetValue(dict, pattern);
            CFRelease(pattern);
            if (!ipv4dict || CFDictionaryGetTypeID() != CFGetTypeID(ipv4dict)) continue;

            configmethod = CFDictionaryGetValue(ipv4dict, kSCPropNetIPv4ConfigMethod);
            if (!configmethod) continue;

            if (mDNS_LoggingEnabled)
            {
                if (!CFStringGetCString(configmethod, buf, sizeof(buf), kCFStringEncodingUTF8)) buf[0] = 0;
                LogInfo("ChangedKeysHaveIPv4LL: configmethod %s", buf);
            }

            if (CFEqual(configmethod, kSCValNetIPv4ConfigMethodLinkLocal)) { found++; break; }
        }

        CFRelease(ifname);
    }

done:
    if (vals != NULL) mDNSPlatformMemFree(vals);
    if (keys != NULL) mDNSPlatformMemFree(keys);
    if (dict != NULL) CFRelease(dict);

    return found;
}

mDNSlocal void NetworkChanged(SCDynamicStoreRef store, CFArrayRef changedKeys, void *context)
{
    (void)store;        // Parameter not used
    mDNS *const m = (mDNS *const)context;
    KQueueLock();
    mDNS_Lock(m);

    //mDNSs32 delay = mDNSPlatformOneSecond * 2;                // Start off assuming a two-second delay
    const mDNSs32 delay = (mDNSPlatformOneSecond + 39) / 40;    // 25 ms delay

    const int c = CFArrayGetCount(changedKeys);                   // Count changes
    CFRange range = { 0, c };
    const int c_host = (CFArrayContainsValue(changedKeys, range, NetworkChangedKey_Hostnames   ) != 0);
    const int c_comp = (CFArrayContainsValue(changedKeys, range, NetworkChangedKey_Computername) != 0);
    const int c_udns = (CFArrayContainsValue(changedKeys, range, NetworkChangedKey_DNS         ) != 0);
    const int c_ddns = (CFArrayContainsValue(changedKeys, range, NetworkChangedKey_DynamicDNS  ) != 0);
    const int c_v4ll = ChangedKeysHaveIPv4LL(changedKeys);
    int c_fast = 0;
    
    // Do immediate network changed processing for "p2p*" interfaces and
    // for interfaces with the IFEF_DIRECTLINK or IFEF_AWDL flag set or association with a CarPlay
    // hosted SSID.
    {
        CFArrayRef  labels;
        CFIndex     n;
        for (int i = 0; i < c; i++)
        {
            CFStringRef key = CFArrayGetValueAtIndex(changedKeys, i);

            // Only look at keys with prefix "State:/Network/Interface/"
            if (!CFStringHasPrefix(key, NetworkChangedKey_StateInterfacePrefix))
                continue;

            // And suffix "IPv6" or "IPv4".
            if (!CFStringHasSuffix(key, kSCEntNetIPv6) && !CFStringHasSuffix(key, kSCEntNetIPv4))
                continue;

            labels = CFStringCreateArrayBySeparatingStrings(NULL, key, CFSTR("/"));
            if (labels == NULL)
                break;
            n = CFArrayGetCount(labels);

            // Interface changes will have keys of the form: 
            //     State:/Network/Interface/<interfaceName>/IPv6
            // Thus five '/' seperated fields, the 4th one being the <interfaceName> string.
            if (n == 5)
            {
                char buf[256];

                // The 4th label (index = 3) should be the interface name.
                if (CFStringGetCString(CFArrayGetValueAtIndex(labels, 3), buf, sizeof(buf), kCFStringEncodingUTF8)
                    && (strstr(buf, "p2p") || (getExtendedFlags(buf) & (IFEF_DIRECTLINK | IFEF_AWDL)) || IsCarPlaySSID(buf)))
                {
                    LogInfo("NetworkChanged: interface %s qualifies for reduced change handling delay", buf);
                    c_fast++;
                    CFRelease(labels);
                    break;
                }
            }
            CFRelease(labels);
        }
    }

    //if (c && c - c_host - c_comp - c_udns - c_ddns - c_v4ll - c_fast == 0)
    //    delay = mDNSPlatformOneSecond/10;  // If these were the only changes, shorten delay

    if (mDNS_LoggingEnabled)
    {
        int i;
        for (i=0; i<c; i++)
        {
            char buf[256];
            if (!CFStringGetCString(CFArrayGetValueAtIndex(changedKeys, i), buf, sizeof(buf), kCFStringEncodingUTF8)) buf[0] = 0;
            LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_INFO, "*** Network Configuration Change *** SC key: " PUB_S, buf);
        }
        LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_INFO,
            "*** Network Configuration Change *** %d change" PUB_S " " PUB_S PUB_S PUB_S PUB_S PUB_S PUB_S "delay %d" PUB_S,
            c, c>1 ? "s" : "",
            c_host ? "(Local Hostname) " : "",
            c_comp ? "(Computer Name) "  : "",
            c_udns ? "(DNS) "            : "",
            c_ddns ? "(DynamicDNS) "     : "",
            c_v4ll ? "(kSCValNetIPv4ConfigMethodLinkLocal) " : "",
            c_fast ? "(P2P/IFEF_DIRECTLINK/IFEF_AWDL/IsCarPlaySSID) "  : "",
            delay,
            c_ddns ? " + SetKeyChainTimer" : "");
    }

    SetNetworkChanged(delay);

    // Other software might pick up these changes to register or browse in WAB or BTMM domains,
    // so in order for secure updates to be made to the server, make sure to read the keychain and
    // setup the DomainAuthInfo before handing the network change.
    // If we don't, then we will first try to register services in the clear, then later setup the
    // DomainAuthInfo, which is incorrect.
    if (c_ddns)
        SetKeyChainTimer(delay);

    // Don't try to call mDNSMacOSXNetworkChanged() here -- we're running on the wrong thread

    mDNS_Unlock(m);
    KQueueUnlock("NetworkChanged");
}

#if APPLE_OSX_mDNSResponder
mDNSlocal void RefreshSPSStatus(const void *key, const void *value, void *context)
{
    (void)context;
    char buf[IFNAMSIZ];

    CFStringRef ifnameStr = (CFStringRef)key;
    CFArrayRef array = (CFArrayRef)value;
    if (!CFStringGetCString(ifnameStr, buf, sizeof(buf), kCFStringEncodingUTF8)) 
        buf[0] = 0;

    LogInfo("RefreshSPSStatus: Updating SPS state for key %s, array count %d", buf, CFArrayGetCount(array));
    mDNSDynamicStoreSetConfig(kmDNSSleepProxyServersState, buf, value);
}
#endif

mDNSlocal void DynamicStoreReconnected(SCDynamicStoreRef store, void *info)
{
    mDNS *const m = (mDNS *const)info;
    (void)store;

    KQueueLock();   // serialize with KQueueLoop()

    LogInfo("DynamicStoreReconnected: Reconnected");

    // State:/Network/MulticastDNS
    SetLocalDomains();

    // State:/Network/DynamicDNS
    if (m->FQDN.c[0])
        mDNSPlatformDynDNSHostNameStatusChanged(&m->FQDN, 1);

    // Note: PrivateDNS and BackToMyMac are automatically populated when configd is restarted
    // as we receive network change notifications and thus not necessary. But we leave it here
    // so that if things are done differently in the future, this code still works.

    // State:/Network/PrivateDNS
    if (privateDnsArray)
        mDNSDynamicStoreSetConfig(kmDNSPrivateConfig, mDNSNULL, privateDnsArray);

#if APPLE_OSX_mDNSResponder
    // State:/Network/Interface/en0/SleepProxyServers
    if (spsStatusDict) 
        CFDictionaryApplyFunction(spsStatusDict, RefreshSPSStatus, NULL);
#endif
    KQueueUnlock("DynamicStoreReconnected");
}

mDNSlocal mStatus WatchForNetworkChanges(mDNS *const m)
{
    mStatus err = -1;
    SCDynamicStoreContext context = { 0, m, NULL, NULL, NULL };
    SCDynamicStoreRef store    = SCDynamicStoreCreate(NULL, CFSTR("mDNSResponder:WatchForNetworkChanges"), NetworkChanged, &context);
    CFMutableArrayRef keys     = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
    CFStringRef pattern1 = SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL, kSCDynamicStoreDomainState, kSCCompAnyRegex, kSCEntNetIPv4);
    CFStringRef pattern2 = SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL, kSCDynamicStoreDomainState, kSCCompAnyRegex, kSCEntNetIPv6);
    CFMutableArrayRef patterns = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

    if (!store) { LogMsg("SCDynamicStoreCreate failed: %s", SCErrorString(SCError())); goto error; }
    if (!keys || !pattern1 || !pattern2 || !patterns) goto error;

    CFArrayAppendValue(keys, NetworkChangedKey_IPv4);
    CFArrayAppendValue(keys, NetworkChangedKey_IPv6);
    CFArrayAppendValue(keys, NetworkChangedKey_Hostnames);
    CFArrayAppendValue(keys, NetworkChangedKey_Computername);
    CFArrayAppendValue(keys, NetworkChangedKey_DNS);
    CFArrayAppendValue(keys, NetworkChangedKey_DynamicDNS);
    CFArrayAppendValue(keys, NetworkChangedKey_PowerSettings);
    CFArrayAppendValue(patterns, pattern1);
    CFArrayAppendValue(patterns, pattern2);
    CFArrayAppendValue(patterns, CFSTR("State:/Network/Interface/[^/]+/AirPort"));
    if (!SCDynamicStoreSetNotificationKeys(store, keys, patterns))
    { LogMsg("SCDynamicStoreSetNotificationKeys failed: %s", SCErrorString(SCError())); goto error; }

#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
    if (!SCDynamicStoreSetDispatchQueue(store, dispatch_get_main_queue()))
    { LogMsg("SCDynamicStoreCreateRunLoopSource failed: %s", SCErrorString(SCError())); goto error; }
#else
    m->p->StoreRLS = SCDynamicStoreCreateRunLoopSource(NULL, store, 0);
    if (!m->p->StoreRLS) { LogMsg("SCDynamicStoreCreateRunLoopSource failed: %s", SCErrorString(SCError())); goto error; }
    CFRunLoopAddSource(CFRunLoopGetMain(), m->p->StoreRLS, kCFRunLoopDefaultMode);
#endif
    SCDynamicStoreSetDisconnectCallBack(store, DynamicStoreReconnected);
    m->p->Store = store;
    err = 0;
    goto exit;

error:
    if (store) CFRelease(store);

exit:
    if (patterns) CFRelease(patterns);
    if (pattern2) CFRelease(pattern2);
    if (pattern1) CFRelease(pattern1);
    if (keys) CFRelease(keys);

    return(err);
}

#if TARGET_OS_OSX
mDNSlocal void mDNSSetPacketFilterRules(char * ifname, const ResourceRecord *const excludeRecord)
{
    mDNS *const m = &mDNSStorage;
    AuthRecord  *rr;
    pfArray_t portArray;
    pfArray_t protocolArray;
    uint32_t count = 0;

    for (rr = m->ResourceRecords; rr; rr=rr->next)
    {
        if ((rr->resrec.rrtype == kDNSServiceType_SRV) 
            && ((rr->ARType == AuthRecordAnyIncludeP2P) || (rr->ARType == AuthRecordAnyIncludeAWDLandP2P)))
        {
            const mDNSu8    *p;

            if (count >= PFPortArraySize)
            {
                LogMsg("mDNSSetPacketFilterRules: %d service limit, skipping %s", PFPortArraySize, ARDisplayString(m, rr));
                continue;
            }

            if (excludeRecord && IdenticalResourceRecord(&rr->resrec, excludeRecord))
            {
                LogInfo("mDNSSetPacketFilterRules: record being removed, skipping %s", ARDisplayString(m, rr));
                continue;
            }

            LogMsg("mDNSSetPacketFilterRules: found %s", ARDisplayString(m, rr));

            portArray[count] = rr->resrec.rdata->u.srv.port.NotAnInteger;

            // Assume <Service Instance>.<App Protocol>.<Transport Protocol>.<Name>
            p = rr->resrec.name->c;

            // Skip to App Protocol
            if (p[0])
                p += 1 + p[0];

            // Skip to Transport Protocol
            if (p[0])
                p += 1 + p[0];

            if      (SameDomainLabel(p, (mDNSu8 *)"\x4" "_tcp"))
            {
                protocolArray[count] = IPPROTO_TCP;
            }
            else if (SameDomainLabel(p, (mDNSu8 *)"\x4" "_udp"))
            {
                protocolArray[count] = IPPROTO_UDP;
            }
            else
            {
                LogMsg("mDNSSetPacketFilterRules: could not determine transport protocol of service");
                LogMsg("mDNSSetPacketFilterRules: %s", ARDisplayString(m, rr));
                return;
            }
            count++;
        }
    }
    mDNSPacketFilterControl(PF_SET_RULES, ifname, count, portArray, protocolArray);
}

// If the p2p interface already exists, update the Bonjour packet filter rules for it.
mDNSexport void mDNSUpdatePacketFilter(const ResourceRecord *const excludeRecord)
{
    mDNS *const m = &mDNSStorage;

    NetworkInterfaceInfo *intf = GetFirstActiveInterface(m->HostInterfaces);
    while (intf)
    {
        if (strncmp(intf->ifname, "p2p", 3) == 0)
        {
            LogInfo("mDNSInitPacketFilter: Setting rules for ifname %s", intf->ifname);
            mDNSSetPacketFilterRules(intf->ifname, excludeRecord);
            break;
        }
        intf = GetFirstActiveInterface(intf->next);
    }
}
#else // !TARGET_OS_OSX
// Currently no packet filter setup required on embedded platforms.
mDNSexport void mDNSUpdatePacketFilter(const ResourceRecord *const excludeRecord)
{
    (void) excludeRecord; // unused
}
#endif

// AWDL should no longer generate KEV_DL_MASTER_ELECTED events, so just log a message if we receive one.
mDNSlocal void newMasterElected(struct net_event_data * ptr)
{
    char        ifname[IFNAMSIZ];
    mDNSu32     interfaceIndex;

    snprintf(ifname, IFNAMSIZ, "%s%d", ptr->if_name, ptr->if_unit);
    interfaceIndex  = if_nametoindex(ifname);

    if (!interfaceIndex)
    {
        LogMsg("newMasterElected: if_nametoindex(%s) failed", ifname);
        return;
    }

    LogInfo("newMasterElected: KEV_DL_MASTER_ELECTED received on ifname = %s, interfaceIndex = %d", ifname, interfaceIndex);
}

// An ssth array of all zeroes indicates the peer has no services registered.
mDNSlocal mDNSBool allZeroSSTH(struct opaque_presence_indication *op)
{
    int i;
    int *intp = (int *) op->ssth;

    // MAX_SSTH_SIZE should always be a multiple of sizeof(int), if
    // it's not, print an error message and return false so that
    // corresponding peer records are not flushed when KEV_DL_NODE_PRESENCE event
    // is received.
    if (MAX_SSTH_SIZE % sizeof(int))
    {
        LogInfo("allZeroSSTH: MAX_SSTH_SIZE = %d not a multiple of sizeof(int)", MAX_SSTH_SIZE);
        return mDNSfalse;
    }

    for (i = 0; i < (int)(MAX_SSTH_SIZE / sizeof(int)); i++, intp++)
    {
        if (*intp)
            return mDNSfalse;
    }
    return mDNStrue;
}

// Mark records from this peer for deletion from the cache.
mDNSlocal void removeCachedPeerRecords(mDNSu32 ifindex, mDNSAddr *ap, bool purgeNow)
{
    mDNS *const m = &mDNSStorage;
    mDNSu32     slot;
    CacheGroup  *cg;
    CacheRecord *cr;
    NetworkInterfaceInfoOSX *infoOSX;
    mDNSInterfaceID InterfaceID;

    // Using mDNSPlatformInterfaceIDfromInterfaceIndex() would lead to recursive
    // locking issues, see: <rdar://problem/21332983>
    infoOSX = IfindexToInterfaceInfoOSX((mDNSInterfaceID)(uintptr_t)ifindex);
    if (!infoOSX)
    {
        LogInfo("removeCachedPeerRecords: interface %d not yet active", ifindex);
        return;
    }
    InterfaceID = infoOSX->ifinfo.InterfaceID;

    FORALL_CACHERECORDS(slot, cg, cr)
    {
        if ((InterfaceID == cr->resrec.InterfaceID) && mDNSSameAddress(ap, & cr->sourceAddress))
        {
            LogInfo("removeCachedPeerRecords: %s %##s marking for deletion",
                 DNSTypeName(cr->resrec.rrtype), cr->resrec.name->c);

            if (purgeNow)
                mDNS_PurgeCacheResourceRecord(m, cr);
            else
                mDNS_Reconfirm_internal(m, cr, 0);  // use default minimum reconfirm time 
        }
    }
}

// Handle KEV_DL_NODE_PRESENCE event.
mDNSlocal void nodePresence(struct kev_dl_node_presence * p)
{
    struct opaque_presence_indication *op = (struct opaque_presence_indication *) p->node_service_info;

    LogInfo("nodePresence: IPv6 address: %.16a, SUI %d", p->sin6_node_address.sin6_addr.s6_addr, op->SUI);
 
    // AWDL will generate a KEV_DL_NODE_PRESENCE event with SSTH field of
    // all zeroes when a node is present and has no services registered.
    if (allZeroSSTH(op))
    {
        mDNSAddr    peerAddr;

        peerAddr.type = mDNSAddrType_IPv6;
        peerAddr.ip.v6 = *(mDNSv6Addr*)&p->sin6_node_address.sin6_addr;

        LogInfo("nodePresence: ssth is all zeroes, reconfirm cached records for this peer");
        removeCachedPeerRecords(p->sdl_node_address.sdl_index, & peerAddr, false);
    }
}

// Handle KEV_DL_NODE_ABSENCE event.
mDNSlocal void nodeAbsence(struct kev_dl_node_absence * p)
{
    mDNSAddr    peerAddr;

    peerAddr.type = mDNSAddrType_IPv6;
    peerAddr.ip.v6 = *(mDNSv6Addr*)&p->sin6_node_address.sin6_addr;

    LogInfo("nodeAbsence: immediately purge cached records from %.16a", p->sin6_node_address.sin6_addr.s6_addr);
    removeCachedPeerRecords(p->sdl_node_address.sdl_index, & peerAddr, true);
}

mDNSlocal void SysEventCallBack(int s1, short __unused filter, void *context, __unused mDNSBool encounteredEOF)
{
    mDNS *const m = (mDNS *const)context;

    mDNS_Lock(m);

    struct { struct kern_event_msg k; char extra[256]; } msg;
    int bytes = recv(s1, &msg, sizeof(msg), 0);
    if (bytes < 0)
        LogMsg("SysEventCallBack: recv error %d errno %d (%s)", bytes, errno, strerror(errno));
    else
    {
        LogInfo("SysEventCallBack got %d bytes size %d %X %s %X %s %X %s id %d code %d %s",
                bytes, msg.k.total_size,
                msg.k.vendor_code, msg.k.vendor_code  == KEV_VENDOR_APPLE  ? "KEV_VENDOR_APPLE"  : "?",
                msg.k.kev_class, msg.k.kev_class    == KEV_NETWORK_CLASS ? "KEV_NETWORK_CLASS" : "?",
                msg.k.kev_subclass, msg.k.kev_subclass == KEV_DL_SUBCLASS   ? "KEV_DL_SUBCLASS"   : "?",
                msg.k.id, msg.k.event_code,
                msg.k.event_code == KEV_DL_SIFFLAGS             ? "KEV_DL_SIFFLAGS"             :
                msg.k.event_code == KEV_DL_SIFMETRICS           ? "KEV_DL_SIFMETRICS"           :
                msg.k.event_code == KEV_DL_SIFMTU               ? "KEV_DL_SIFMTU"               :
                msg.k.event_code == KEV_DL_SIFPHYS              ? "KEV_DL_SIFPHYS"              :
                msg.k.event_code == KEV_DL_SIFMEDIA             ? "KEV_DL_SIFMEDIA"             :
                msg.k.event_code == KEV_DL_SIFGENERIC           ? "KEV_DL_SIFGENERIC"           :
                msg.k.event_code == KEV_DL_ADDMULTI             ? "KEV_DL_ADDMULTI"             :
                msg.k.event_code == KEV_DL_DELMULTI             ? "KEV_DL_DELMULTI"             :
                msg.k.event_code == KEV_DL_IF_ATTACHED          ? "KEV_DL_IF_ATTACHED"          :
                msg.k.event_code == KEV_DL_IF_DETACHING         ? "KEV_DL_IF_DETACHING"         :
                msg.k.event_code == KEV_DL_IF_DETACHED          ? "KEV_DL_IF_DETACHED"          :
                msg.k.event_code == KEV_DL_LINK_OFF             ? "KEV_DL_LINK_OFF"             :
                msg.k.event_code == KEV_DL_LINK_ON              ? "KEV_DL_LINK_ON"              :
                msg.k.event_code == KEV_DL_PROTO_ATTACHED       ? "KEV_DL_PROTO_ATTACHED"       :
                msg.k.event_code == KEV_DL_PROTO_DETACHED       ? "KEV_DL_PROTO_DETACHED"       :
                msg.k.event_code == KEV_DL_LINK_ADDRESS_CHANGED ? "KEV_DL_LINK_ADDRESS_CHANGED" :
                msg.k.event_code == KEV_DL_WAKEFLAGS_CHANGED    ? "KEV_DL_WAKEFLAGS_CHANGED"    :
                msg.k.event_code == KEV_DL_IF_IDLE_ROUTE_REFCNT ? "KEV_DL_IF_IDLE_ROUTE_REFCNT" :
                msg.k.event_code == KEV_DL_IFCAP_CHANGED        ? "KEV_DL_IFCAP_CHANGED"        :
                msg.k.event_code == KEV_DL_LINK_QUALITY_METRIC_CHANGED    ? "KEV_DL_LINK_QUALITY_METRIC_CHANGED"    :
                msg.k.event_code == KEV_DL_NODE_PRESENCE        ? "KEV_DL_NODE_PRESENCE"        :
                msg.k.event_code == KEV_DL_NODE_ABSENCE         ? "KEV_DL_NODE_ABSENCE"         :
                msg.k.event_code == KEV_DL_MASTER_ELECTED       ? "KEV_DL_MASTER_ELECTED"       :
                 "?");

        if (msg.k.event_code == KEV_DL_NODE_PRESENCE)
            nodePresence((struct kev_dl_node_presence *) &msg.k.event_data);

        if (msg.k.event_code == KEV_DL_NODE_ABSENCE)
            nodeAbsence((struct kev_dl_node_absence *) &msg.k.event_data);

        if (msg.k.event_code == KEV_DL_MASTER_ELECTED)
            newMasterElected((struct net_event_data *) &msg.k.event_data);

        // We receive network change notifications both through configd and through SYSPROTO_EVENT socket.
        // Configd may not generate network change events for manually configured interfaces (i.e., non-DHCP)
        // always during sleep/wakeup due to some race conditions (See radar:8666757). At the same time, if
        // "Wake on Network Access" is not turned on, the notification will not have KEV_DL_WAKEFLAGS_CHANGED.
        // Hence, during wake up, if we see a KEV_DL_LINK_ON (i.e., link is UP), we trigger a network change.

        if (msg.k.event_code == KEV_DL_WAKEFLAGS_CHANGED || msg.k.event_code == KEV_DL_LINK_ON)
            SetNetworkChanged(mDNSPlatformOneSecond * 2);

#if TARGET_OS_OSX
        // For p2p interfaces, need to open the advertised service port in the firewall.
        if (msg.k.event_code == KEV_DL_IF_ATTACHED)
        {
            struct net_event_data   * p;
            p = (struct net_event_data *) &msg.k.event_data;

            if (strncmp(p->if_name, "p2p", 3) == 0)
            {
                char ifname[IFNAMSIZ];
                snprintf(ifname, IFNAMSIZ, "%s%d", p->if_name, p->if_unit);

                LogInfo("SysEventCallBack: KEV_DL_IF_ATTACHED if_family = %d, if_unit = %d, if_name = %s", p->if_family, p->if_unit, p->if_name);

                mDNSSetPacketFilterRules(ifname, NULL);
            }
        }

        // For p2p interfaces, need to clear the firewall rules on interface detach
        if (msg.k.event_code == KEV_DL_IF_DETACHED)
        {
            struct net_event_data   * p;
            p = (struct net_event_data *) &msg.k.event_data;

            if (strncmp(p->if_name, "p2p", 3) == 0)
            {
                pfArray_t portArray, protocolArray; // not initialized since count is 0 for PF_CLEAR_RULES
                char ifname[IFNAMSIZ];
                snprintf(ifname, IFNAMSIZ, "%s%d", p->if_name, p->if_unit);

                LogInfo("SysEventCallBack: KEV_DL_IF_DETACHED if_family = %d, if_unit = %d, if_name = %s", p->if_family, p->if_unit, p->if_name);

                mDNSPacketFilterControl(PF_CLEAR_RULES, ifname, 0, portArray, protocolArray);
            }
        }
#endif
    }

    mDNS_Unlock(m);
}

mDNSlocal mStatus WatchForSysEvents(mDNS *const m)
{
    m->p->SysEventNotifier = socket(PF_SYSTEM, SOCK_RAW, SYSPROTO_EVENT);
    if (m->p->SysEventNotifier < 0)
    { LogMsg("WatchForSysEvents: socket failed error %d errno %d (%s)", m->p->SysEventNotifier, errno, strerror(errno)); return(mStatus_NoMemoryErr); }

    struct kev_request kev_req = { KEV_VENDOR_APPLE, KEV_NETWORK_CLASS, KEV_DL_SUBCLASS };
    int err = ioctl(m->p->SysEventNotifier, SIOCSKEVFILT, &kev_req);
    if (err < 0)
    {
        LogMsg("WatchForSysEvents: SIOCSKEVFILT failed error %d errno %d (%s)", err, errno, strerror(errno));
        close(m->p->SysEventNotifier);
        m->p->SysEventNotifier = -1;
        return(mStatus_UnknownErr);
    }

    m->p->SysEventKQueue.KQcallback = SysEventCallBack;
    m->p->SysEventKQueue.KQcontext  = m;
    m->p->SysEventKQueue.KQtask     = "System Event Notifier";
    KQueueSet(m->p->SysEventNotifier, EV_ADD, EVFILT_READ, &m->p->SysEventKQueue);

    return(mStatus_NoError);
}

#ifndef NO_SECURITYFRAMEWORK
mDNSlocal OSStatus KeychainChanged(SecKeychainEvent keychainEvent, SecKeychainCallbackInfo *info, void *context)
{
    LogInfo("***   Keychain Changed   ***");
    mDNS *const m = (mDNS *const)context;
    SecKeychainRef skc;
    OSStatus err = SecKeychainCopyDefault(&skc);
    if (!err)
    {
        if (info->keychain == skc)
        {
            // For delete events, attempt to verify what item was deleted fail because the item is already gone, so we just assume they may be relevant
            mDNSBool relevant = (keychainEvent == kSecDeleteEvent);
            if (!relevant)
            {
                UInt32 tags[3] = { kSecTypeItemAttr, kSecServiceItemAttr, kSecAccountItemAttr };
                SecKeychainAttributeInfo attrInfo = { 3, tags, NULL };  // Count, array of tags, array of formats
                SecKeychainAttributeList *a = NULL;
                err = SecKeychainItemCopyAttributesAndData(info->item, &attrInfo, NULL, &a, NULL, NULL);
                if (!err)
                {
                    relevant = ((a->attr[0].length == 4 && (!strncasecmp(a->attr[0].data, "ddns", 4) || !strncasecmp(a->attr[0].data, "sndd", 4))) ||
                                (a->attr[1].length >= mDNSPlatformStrLen(dnsprefix) && (!strncasecmp(a->attr[1].data, dnsprefix, mDNSPlatformStrLen(dnsprefix)))));
                    SecKeychainItemFreeAttributesAndData(a, NULL);
                }
            }
            if (relevant)
            {
                LogInfo("***   Keychain Changed   *** KeychainEvent=%d %s",
                        keychainEvent,
                        keychainEvent == kSecAddEvent    ? "kSecAddEvent"    :
                        keychainEvent == kSecDeleteEvent ? "kSecDeleteEvent" :
                        keychainEvent == kSecUpdateEvent ? "kSecUpdateEvent" : "<Unknown>");
                // We're running on the CFRunLoop (Mach port) thread, not the kqueue thread, so we need to grab the KQueueLock before proceeding
                KQueueLock();
                mDNS_Lock(m);

                // To not read the keychain twice: when BTMM is enabled, changes happen to the keychain
                // then the BTMM DynStore dictionary, so delay reading the keychain for a second.
                // NetworkChanged() will reset the keychain timer to fire immediately when the DynStore changes.
                //
                // In the "fixup" case where the BTMM DNS servers aren't accepting the key mDNSResponder has,
                // the DynStore dictionary won't change (because the BTMM zone won't change).  In that case,
                // a one second delay is ok, as we'll still converge to correctness, and there's no race
                // condition between the RegistrationDomain and the DomainAuthInfo.
                //
                // Lastly, non-BTMM WAB cases can use the keychain but not the DynStore, so we need to set
                // the timer here, as it will not get set by NetworkChanged().
                SetKeyChainTimer(mDNSPlatformOneSecond);

                mDNS_Unlock(m);
                KQueueUnlock("KeychainChanged");
            }
        }
        CFRelease(skc);
    }

    return 0;
}
#endif

mDNSlocal void PowerOn(mDNS *const m)
{
    mDNSCoreMachineSleep(m, false);     // Will set m->SleepState = SleepState_Awake;

    if (m->p->WakeAtUTC)
    {
        long utc = mDNSPlatformUTC();
        mDNSPowerRequest(-1,-1);        // Need to explicitly clear any previous power requests -- they're not cleared automatically on wake
        if (m->p->WakeAtUTC - utc > 30)
        {
            LogSPS("PowerChanged PowerOn %d seconds early, assuming not maintenance wake", m->p->WakeAtUTC - utc);
        }
        else if (utc - m->p->WakeAtUTC > 30)
        {
            LogSPS("PowerChanged PowerOn %d seconds late, assuming not maintenance wake", utc - m->p->WakeAtUTC);
        }
        else if (IsAppleTV())
        {
            LogSPS("PowerChanged PowerOn %d seconds late, device is an AppleTV running iOS so not re-sleeping", utc - m->p->WakeAtUTC);
        }
        else
        {
            LogSPS("PowerChanged: Waking for network maintenance operations %d seconds early; re-sleeping in 20 seconds", m->p->WakeAtUTC - utc);
            m->p->RequestReSleep = mDNS_TimeNow(m) + 20 * mDNSPlatformOneSecond;
        }
    }

    // Hold on to a sleep assertion to allow mDNSResponder to perform its maintenance activities.
    // This allows for the network link to come up, DHCP to get an address, mDNS to issue queries etc.
    // We will clear this assertion as soon as we think the mainenance activities are done.
    mDNSPlatformPreventSleep(DARK_WAKE_TIME, "mDNSResponder:maintenance");

}

mDNSlocal void PowerChanged(void *refcon, io_service_t service, natural_t messageType, void *messageArgument)
{
    mDNS *const m = (mDNS *const)refcon;
    KQueueLock();
    (void)service;    // Parameter not used
    debugf("PowerChanged %X %lX", messageType, messageArgument);

    // Make sure our m->SystemWakeOnLANEnabled value correctly reflects the current system setting
    m->SystemWakeOnLANEnabled = SystemWakeForNetworkAccess();

    switch(messageType)
    {
    case kIOMessageCanSystemPowerOff:       LogSPS("PowerChanged kIOMessageCanSystemPowerOff     (no action)"); break;          // E0000240
    case kIOMessageSystemWillPowerOff:      LogSPS("PowerChanged kIOMessageSystemWillPowerOff");                                // E0000250
        mDNSCoreMachineSleep(m, true);
        if (m->SleepState == SleepState_Sleeping) mDNSMacOSXNetworkChanged();
        break;
    case kIOMessageSystemWillNotPowerOff:   LogSPS("PowerChanged kIOMessageSystemWillNotPowerOff (no action)"); break;          // E0000260
    case kIOMessageCanSystemSleep:          LogSPS("PowerChanged kIOMessageCanSystemSleep");                    break;          // E0000270
    case kIOMessageSystemWillSleep:         LogSPS("PowerChanged kIOMessageSystemWillSleep");                                   // E0000280
        mDNSCoreMachineSleep(m, true);
        break;
    case kIOMessageSystemWillNotSleep:      LogSPS("PowerChanged kIOMessageSystemWillNotSleep    (no action)"); break;          // E0000290
    case kIOMessageSystemHasPoweredOn:      LogSPS("PowerChanged kIOMessageSystemHasPoweredOn");                                // E0000300
        // If still sleeping (didn't get 'WillPowerOn' message for some reason?) wake now
        if (m->SleepState)
        {
            LogMsg("PowerChanged kIOMessageSystemHasPoweredOn: ERROR m->SleepState %d", m->SleepState);
            PowerOn(m);
        }
        // Just to be safe, schedule a mDNSMacOSXNetworkChanged(), in case we never received
        // the System Configuration Framework "network changed" event that we expect
        // to receive some time shortly after the kIOMessageSystemWillPowerOn message
        mDNS_Lock(m);
        SetNetworkChanged(mDNSPlatformOneSecond * 2);
        mDNS_Unlock(m);

        break;
    case kIOMessageSystemWillRestart:       LogSPS("PowerChanged kIOMessageSystemWillRestart     (no action)"); break;          // E0000310
    case kIOMessageSystemWillPowerOn:       LogSPS("PowerChanged kIOMessageSystemWillPowerOn");                                 // E0000320

        // Make sure our interface list is cleared to the empty state, then tell mDNSCore to wake
        if (m->SleepState != SleepState_Sleeping)
        {
            LogMsg("kIOMessageSystemWillPowerOn: ERROR m->SleepState %d", m->SleepState);
            m->SleepState = SleepState_Sleeping;
            mDNSMacOSXNetworkChanged();
        }
        PowerOn(m);
        break;
    default:                                LogSPS("PowerChanged unknown message %X", messageType); break;
    }

    if (messageType == kIOMessageSystemWillSleep)
        m->p->SleepCookie = (long)messageArgument;
    else if (messageType == kIOMessageCanSystemSleep)
        IOAllowPowerChange(m->p->PowerConnection, (long)messageArgument);

    KQueueUnlock("PowerChanged Sleep/Wake");
}

// iPhone OS doesn't currently have SnowLeopard's IO Power Management
// but it does define kIOPMAcknowledgmentOptionSystemCapabilityRequirements
#if defined(kIOPMAcknowledgmentOptionSystemCapabilityRequirements) && TARGET_OS_OSX
mDNSlocal void SnowLeopardPowerChanged(void *refcon, IOPMConnection connection, IOPMConnectionMessageToken token, IOPMSystemPowerStateCapabilities eventDescriptor)
{
    mDNS *const m = (mDNS *const)refcon;
    KQueueLock();
    LogSPS("SnowLeopardPowerChanged %X %X %X%s%s%s%s%s",
           connection, token, eventDescriptor,
           eventDescriptor & kIOPMSystemPowerStateCapabilityCPU     ? " CPU"     : "",
           eventDescriptor & kIOPMSystemPowerStateCapabilityVideo   ? " Video"   : "",
           eventDescriptor & kIOPMSystemPowerStateCapabilityAudio   ? " Audio"   : "",
           eventDescriptor & kIOPMSystemPowerStateCapabilityNetwork ? " Network" : "",
           eventDescriptor & kIOPMSystemPowerStateCapabilityDisk    ? " Disk"    : "");

    // Make sure our m->SystemWakeOnLANEnabled value correctly reflects the current system setting
    m->SystemWakeOnLANEnabled = SystemWakeForNetworkAccess();

    if (eventDescriptor & kIOPMSystemPowerStateCapabilityCPU)
    {
        // We might be in Sleeping or Transferring state. When we go from "wakeup" to "sleep" state, we don't
        // go directly to sleep state, but transfer in to the sleep state during which SleepState is set to
        // SleepState_Transferring. During that time, we might get another wakeup before we transition to Sleeping
        // state. In that case, we need to acknowledge the previous "sleep" before we acknowledge the wakeup.
        if (m->SleepLimit)
        {
            LogSPS("SnowLeopardPowerChanged: Waking up, Acking old Sleep, SleepLimit %d SleepState %d", m->SleepLimit, m->SleepState);
            IOPMConnectionAcknowledgeEvent(connection, m->p->SleepCookie);
            m->SleepLimit = 0;
        }
        LogSPS("SnowLeopardPowerChanged: Waking up, Acking Wakeup, SleepLimit %d SleepState %d", m->SleepLimit, m->SleepState);
        // CPU Waking. Note: Can get this message repeatedly, as other subsystems power up or down.
        if (m->SleepState != SleepState_Awake)
        {
            PowerOn(m);
            // If the network notifications have already come before we got the wakeup, we ignored them and
            // in case we get no more, we need to trigger one.
            mDNS_Lock(m);
            SetNetworkChanged(mDNSPlatformOneSecond * 2);
            mDNS_Unlock(m);
        }
        IOPMConnectionAcknowledgeEvent(connection, token);
    }
    else
    {
        // CPU sleeping. Should not get this repeatedly -- once we're told that the CPU is halting
        // we should hear nothing more until we're told that the CPU has started executing again.
        if (m->SleepState) LogMsg("SnowLeopardPowerChanged: Sleep Error %X m->SleepState %d", eventDescriptor, m->SleepState);
        //sleep(5);
        //mDNSMacOSXNetworkChanged(m);
        mDNSCoreMachineSleep(m, true);
        //if (m->SleepState == SleepState_Sleeping) mDNSMacOSXNetworkChanged(m);
        m->p->SleepCookie = token;
    }

    KQueueUnlock("SnowLeopardPowerChanged Sleep/Wake");
}
#endif

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - /etc/hosts support
#endif

// Implementation Notes
//
// As /etc/hosts file can be huge (1000s of entries - when this comment was written, the test file had about
// 23000 entries with about 4000 duplicates), we can't use a linked list to store these entries. So, we parse
// them into a hash table. The implementation need to be able to do the following things efficiently
//
// 1. Detect duplicates e.g., two entries with "1.2.3.4 foo"
// 2. Detect whether /etc/hosts has changed and what has changed since the last read from the disk
// 3. Ability to support multiple addresses per name e.g., "1.2.3.4 foo, 2.3.4.5 foo". To support this, we
//    need to be able set the RRSet of a resource record to the first one in the list and also update when
//    one of them go away. This is needed so that the core thinks that they are all part of the same RRSet and
//    not a duplicate
// 4. Don't maintain any local state about any records registered with the core to detect changes to /etc/hosts
//
// CFDictionary is not a suitable candidate because it does not support duplicates and even if we use a custom
// "hash" function to solve this, the others are hard to solve. Hence, we share the hash (AuthHash) implementation
// of the core layer which does all of the above very efficiently

#define ETCHOSTS_BUFSIZE    1024    // Buffer size to parse a single line in /etc/hosts

mDNSexport void FreeEtcHosts(mDNS *const m, AuthRecord *const rr, mStatus result)
{
    (void)m;  // unused
    (void)rr;
    (void)result;
    if (result == mStatus_MemFree)
    {
        LogInfo("FreeEtcHosts: %s", ARDisplayString(m, rr));
        freeL("etchosts", rr);
    }
}

// Returns true on success and false on failure
mDNSlocal mDNSBool mDNSMacOSXCreateEtcHostsEntry(const domainname *domain, const struct sockaddr *sa, const domainname *cname, char *ifname, AuthHash *auth)
{
    AuthRecord *rr;
    mDNSu32 namehash;
    AuthGroup *ag;
    mDNSInterfaceID InterfaceID = mDNSInterface_LocalOnly;
    mDNSu16 rrtype;

    if (!domain)
    {
        LogMsg("mDNSMacOSXCreateEtcHostsEntry: ERROR!! name NULL");
        return mDNSfalse;
    }
    if (!sa && !cname)
    {
        LogMsg("mDNSMacOSXCreateEtcHostsEntry: ERROR!! sa and cname both NULL");
        return mDNSfalse;
    }

    if (sa && sa->sa_family != AF_INET && sa->sa_family != AF_INET6)
    {
        LogMsg("mDNSMacOSXCreateEtcHostsEntry: ERROR!! sa with bad family %d", sa->sa_family);
        return mDNSfalse;
    }


    if (ifname)
    {
        mDNSu32 ifindex = if_nametoindex(ifname);
        if (!ifindex)
        {
            LogMsg("mDNSMacOSXCreateEtcHostsEntry: hosts entry %##s with invalid ifname %s", domain->c, ifname);
            return mDNSfalse;
        }
        InterfaceID = (mDNSInterfaceID)(uintptr_t)ifindex;
    }

    if (sa)
        rrtype = (sa->sa_family == AF_INET ? kDNSType_A : kDNSType_AAAA);
    else
        rrtype = kDNSType_CNAME;

    // Check for duplicates. See whether we parsed an entry before like this ?
    namehash = DomainNameHashValue(domain);
    ag = AuthGroupForName(auth, namehash, domain);
    if (ag)
    {
        rr = ag->members;
        while (rr)
        {
            if (rr->resrec.rrtype == rrtype)
            {
                if (rrtype == kDNSType_A)
                {
                    mDNSv4Addr ip;
                    ip.NotAnInteger = ((struct sockaddr_in*)sa)->sin_addr.s_addr;
                    if (mDNSSameIPv4Address(rr->resrec.rdata->u.ipv4, ip) && InterfaceID == rr->resrec.InterfaceID)
                    {
                        LogInfo("mDNSMacOSXCreateEtcHostsEntry: Same IPv4 address and InterfaceID for name %##s ID %d", domain->c, IIDPrintable(InterfaceID));
                        return mDNSfalse;
                    }
                }
                else if (rrtype == kDNSType_AAAA)
                {
                    mDNSv6Addr ip6;
                    ip6.l[0] = ((struct sockaddr_in6*)sa)->sin6_addr.__u6_addr.__u6_addr32[0];
                    ip6.l[1] = ((struct sockaddr_in6*)sa)->sin6_addr.__u6_addr.__u6_addr32[1];
                    ip6.l[2] = ((struct sockaddr_in6*)sa)->sin6_addr.__u6_addr.__u6_addr32[2];
                    ip6.l[3] = ((struct sockaddr_in6*)sa)->sin6_addr.__u6_addr.__u6_addr32[3];
                    if (mDNSSameIPv6Address(rr->resrec.rdata->u.ipv6, ip6) && InterfaceID == rr->resrec.InterfaceID)
                    {
                        LogInfo("mDNSMacOSXCreateEtcHostsEntry: Same IPv6 address and InterfaceID for name %##s ID %d", domain->c, IIDPrintable(InterfaceID));
                        return mDNSfalse;
                    }
                }
                else if (rrtype == kDNSType_CNAME)
                {
                    if (SameDomainName(&rr->resrec.rdata->u.name, cname))
                    {
                        LogInfo("mDNSMacOSXCreateEtcHostsEntry: Same cname %##s for name %##s", cname->c, domain->c);
                        return mDNSfalse;
                    }
                }
            }
            rr = rr->next;
        }
    }
    rr = (AuthRecord *) callocL("etchosts", sizeof(*rr));
    if (rr == NULL) return mDNSfalse;
    mDNS_SetupResourceRecord(rr, NULL, InterfaceID, rrtype, 1, kDNSRecordTypeKnownUnique, AuthRecordLocalOnly, FreeEtcHosts, NULL);
    AssignDomainName(&rr->namestorage, domain);

    if (sa)
    {
        rr->resrec.rdlength = sa->sa_family == AF_INET ? sizeof(mDNSv4Addr) : sizeof(mDNSv6Addr);
        if (sa->sa_family == AF_INET)
            rr->resrec.rdata->u.ipv4.NotAnInteger = ((struct sockaddr_in*)sa)->sin_addr.s_addr;
        else
        {
            rr->resrec.rdata->u.ipv6.l[0] = ((struct sockaddr_in6*)sa)->sin6_addr.__u6_addr.__u6_addr32[0];
            rr->resrec.rdata->u.ipv6.l[1] = ((struct sockaddr_in6*)sa)->sin6_addr.__u6_addr.__u6_addr32[1];
            rr->resrec.rdata->u.ipv6.l[2] = ((struct sockaddr_in6*)sa)->sin6_addr.__u6_addr.__u6_addr32[2];
            rr->resrec.rdata->u.ipv6.l[3] = ((struct sockaddr_in6*)sa)->sin6_addr.__u6_addr.__u6_addr32[3];
        }
    }
    else
    {
        rr->resrec.rdlength = DomainNameLength(cname);
        rr->resrec.rdata->u.name.c[0] = 0;
        AssignDomainName(&rr->resrec.rdata->u.name, cname);
    }
    rr->resrec.namehash = DomainNameHashValue(rr->resrec.name);
    SetNewRData(&rr->resrec, mDNSNULL, 0);  // Sets rr->rdatahash for us
    LogInfo("mDNSMacOSXCreateEtcHostsEntry: Adding resource record %s ID %d", ARDisplayString(&mDNSStorage, rr), IIDPrintable(rr->resrec.InterfaceID));
    InsertAuthRecord(&mDNSStorage, auth, rr);
    return mDNStrue;
}

mDNSlocal int EtcHostsParseOneName(int start, int length, char *buffer, char **name)
{
    int i;

    *name = NULL;
    for (i = start; i < length; i++)
    {
        if (buffer[i] == '#')
            return -1;
        if (buffer[i] != ' ' && buffer[i] != ',' && buffer[i] != '\t')
        {
            *name = &buffer[i];

            // Found the start of a name, find the end and null terminate
            for (i++; i < length; i++)
            {
                if (buffer[i] == ' ' || buffer[i] == ',' || buffer[i] == '\t')
                {
                    buffer[i] = 0;
                    break;
                }
            }
            return i;
        }
    }
    return -1;
}

mDNSlocal void mDNSMacOSXParseEtcHostsLine(char *buffer, ssize_t length, AuthHash *auth)
{
    int i;
    int ifStart = 0;
    char *ifname = NULL;
    domainname name1d;
    domainname name2d;
    char *name1;
    char *name2;
    int aliasIndex;

    //Ignore leading whitespaces and tabs
    while (*buffer == ' ' || *buffer == '\t')
    {
        buffer++;
        length--;
    }

    // Find the end of the address string
    for (i = 0; i < length; i++)
    {
        if (buffer[i] == ' ' || buffer[i] == ',' || buffer[i] == '\t' || buffer[i] == '%')
        {
            if (buffer[i] == '%')
                ifStart = i + 1;
            buffer[i] = 0;
            break;
        }
    }

    // Convert the address string to an address
    struct addrinfo hints;
    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_NUMERICHOST;
    struct addrinfo *gairesults = NULL;
    if (getaddrinfo(buffer, NULL, &hints, &gairesults) != 0)
    {
        LogInfo("mDNSMacOSXParseEtcHostsLine: getaddrinfo returning null");
        return;
    }

    if (ifStart)
    {
        // Parse the interface
        ifname = &buffer[ifStart];
        for (i = ifStart + 1; i < length; i++)
        {
            if (buffer[i] == ' ' || buffer[i] == ',' || buffer[i] == '\t')
            {
                buffer[i] = 0;
                break;
            }
        }
    }

    i = EtcHostsParseOneName(i + 1, length, buffer, &name1);
    if (i == length)
    {
        // Common case (no aliases) : The entry is of the form "1.2.3.4 somehost" with no trailing white spaces/tabs etc.
        if (!MakeDomainNameFromDNSNameString(&name1d, name1))
        {
            LogMsg("mDNSMacOSXParseEtcHostsLine: ERROR!! cannot convert to domain name %s", name1);
            freeaddrinfo(gairesults);
            return;
        }
        mDNSMacOSXCreateEtcHostsEntry(&name1d, gairesults->ai_addr, mDNSNULL, ifname, auth);
    }
    else if (i != -1)
    {
        domainname first;
        // We might have some extra white spaces at the end for the common case of "1.2.3.4 somehost".
        // When we parse again below, EtchHostsParseOneName would return -1 and we will end up
        // doing the right thing.
        
        if (!MakeDomainNameFromDNSNameString(&first, name1))
        {
            LogMsg("mDNSMacOSXParseEtcHostsLine: ERROR!! cannot convert to domain name %s", name1);
            freeaddrinfo(gairesults);
            return;
        }
        mDNSMacOSXCreateEtcHostsEntry(&first, gairesults->ai_addr, mDNSNULL, ifname, auth);
        
        // /etc/hosts alias discussion:
        //
        // If the /etc/hosts has an entry like this
        //
        //  ip_address cname [aliases...]
        //  1.2.3.4    sun    star    bright
        //
        // star and bright are aliases (gethostbyname h_alias should point to these) and sun is the canonical
        // name (getaddrinfo ai_cannonname and gethostbyname h_name points to "sun")
        //
        // To achieve this, we need to add the entry like this:
        //
        // sun A 1.2.3.4
        // star CNAME sun
        // bright CNAME sun
        //
        // We store the first name we parsed in "first" and add the address (A/AAAA) record.
        // Then we parse additional names adding CNAME records till we reach the end.
        
        aliasIndex = 0;
        while (i < length)
        {
            // Continue to parse additional aliases until we reach end of the line and
            // for each "alias" parsed, add a CNAME record where "alias" points to the first "name".
            // See also /etc/hosts alias discussion above
            
            i = EtcHostsParseOneName(i + 1, length, buffer, &name2);
            
            if (name2)
            {
                if ((aliasIndex) && (*buffer == *name2))
                    break; // break out of the loop if we wrap around
                
                if (!MakeDomainNameFromDNSNameString(&name2d, name2))
                {
                    LogMsg("mDNSMacOSXParseEtcHostsLine: ERROR!! cannot convert to domain name %s", name2);
                    freeaddrinfo(gairesults);
                    return;
                }
                // Ignore if it points to itself
                if (!SameDomainName(&first, &name2d))
                {
                    if (!mDNSMacOSXCreateEtcHostsEntry(&name2d, mDNSNULL, &first, ifname, auth))
                    {
                        freeaddrinfo(gairesults);
                        return;
                    }
                }
                else
                {
                    LogInfo("mDNSMacOSXParseEtcHostsLine: Ignoring entry with same names first %##s, name2 %##s", first.c, name2d.c);
                }
                aliasIndex++;
            }
            else if (!aliasIndex)
            {
                // We have never parsed any aliases. This case happens if there
                // is just one name and some extra white spaces at the end.
                LogInfo("mDNSMacOSXParseEtcHostsLine: White space at the end of %##s", first.c);
                break;
            }
        }
    }
    freeaddrinfo(gairesults);
}

mDNSlocal void mDNSMacOSXParseEtcHosts(int fd, AuthHash *auth)
{
    mDNSBool good;
    char buf[ETCHOSTS_BUFSIZE];
    ssize_t len;
    FILE *fp;

    if (fd == -1) { LogInfo("mDNSMacOSXParseEtcHosts: fd is -1"); return; }

    fp = fopen("/etc/hosts", "r");
    if (!fp) { LogInfo("mDNSMacOSXParseEtcHosts: fp is NULL"); return; }

    while (1)
    {
        good = (fgets(buf, ETCHOSTS_BUFSIZE, fp) != NULL);
        if (!good) break;

        // skip comment and empty lines
        if (buf[0] == '#' || buf[0] == '\r' || buf[0] == '\n')
            continue;

        len = strlen(buf);
        if (!len) break;    // sanity check
        //Check for end of line code(mostly only \n but pre-OS X Macs could have only \r)
        if (buf[len - 1] == '\r' || buf[len - 1] == '\n')
        {
            buf[len - 1] = '\0';
            len = len - 1;
        }
        // fgets always null terminates and hence even if we have no
        // newline at the end, it is null terminated. The callee
        // (mDNSMacOSXParseEtcHostsLine) expects the length to be such that
        // buf[length] is zero and hence we decrement len to reflect that.
        if (len)
        {
            //Additional check when end of line code is 2 chars ie\r\n(DOS, other old OSes)
            //here we need to check for just \r but taking extra caution.
            if (buf[len - 1] == '\r' || buf[len - 1] == '\n')
            {
                buf[len - 1] = '\0';
                len = len - 1;
            }
        }
        if (!len) //Sanity Check: len should never be zero
        {
            LogMsg("mDNSMacOSXParseEtcHosts: Length is zero!");
            continue;
        }
        mDNSMacOSXParseEtcHostsLine(buf, len, auth);
    }
    fclose(fp);
}

mDNSlocal void mDNSMacOSXUpdateEtcHosts(mDNS *const m);

mDNSlocal int mDNSMacOSXGetEtcHostsFD(void)
{
    mDNS *const m = &mDNSStorage;
#ifdef __DISPATCH_GROUP__
    // Can't do this stuff to be notified of changes in /etc/hosts if we don't have libdispatch
    static dispatch_queue_t etcq     = 0;
    static dispatch_source_t etcsrc   = 0;
    static dispatch_source_t hostssrc = 0;

    // First time through? just schedule ourselves on the main queue and we'll do the work later
    if (!etcq)
    {
        etcq = dispatch_get_main_queue();
        if (etcq)
        {
            // Do this work on the queue, not here - solves potential synchronization issues
            dispatch_async(etcq, ^{mDNSMacOSXUpdateEtcHosts(m);});
        }
        return -1;
    }

    if (hostssrc) return dispatch_source_get_handle(hostssrc);
#endif

    int fd = open("/etc/hosts", O_RDONLY);

#ifdef __DISPATCH_GROUP__
    // Can't do this stuff to be notified of changes in /etc/hosts if we don't have libdispatch
    if (fd == -1)
    {
        // If the open failed and we're already watching /etc, we're done
        if (etcsrc) { LogInfo("mDNSMacOSXGetEtcHostsFD: Returning etcfd because no etchosts"); return fd; }

        // we aren't watching /etc, we should be
        fd = open("/etc", O_RDONLY);
        if (fd == -1) { LogInfo("mDNSMacOSXGetEtcHostsFD: etc does not exist"); return -1; }
        etcsrc = dispatch_source_create(DISPATCH_SOURCE_TYPE_VNODE, fd, DISPATCH_VNODE_DELETE | DISPATCH_VNODE_WRITE | DISPATCH_VNODE_RENAME, etcq);
        if (etcsrc == NULL)
        {
            close(fd);
            return -1;
        }
        dispatch_source_set_event_handler(etcsrc,
                                          ^{
                                              u_int32_t flags = dispatch_source_get_data(etcsrc);
                                              LogMsg("mDNSMacOSXGetEtcHostsFD: /etc changed 0x%x", flags);
                                              if ((flags & (DISPATCH_VNODE_DELETE | DISPATCH_VNODE_RENAME)) != 0)
                                              {
                                                  dispatch_source_cancel(etcsrc);
                                                  dispatch_release(etcsrc);
                                                  etcsrc = NULL;
                                                  dispatch_async(etcq, ^{mDNSMacOSXUpdateEtcHosts(m);});
                                                  return;
                                              }
                                              if ((flags & DISPATCH_VNODE_WRITE) != 0 && hostssrc == NULL)
                                              {
                                                  mDNSMacOSXUpdateEtcHosts(m);
                                              }
                                          });
        dispatch_source_set_cancel_handler(etcsrc, ^{close(fd);});
        dispatch_resume(etcsrc);

        // Try and open /etc/hosts once more now that we're watching /etc, in case we missed the creation
        fd = open("/etc/hosts", O_RDONLY | O_EVTONLY);
        if (fd == -1) { LogMsg("mDNSMacOSXGetEtcHostsFD etc hosts does not exist, watching etc"); return -1; }
    }

    // create a dispatch source to watch for changes to hosts file
    hostssrc = dispatch_source_create(DISPATCH_SOURCE_TYPE_VNODE, fd,
                                      (DISPATCH_VNODE_DELETE | DISPATCH_VNODE_WRITE | DISPATCH_VNODE_RENAME |
                                       DISPATCH_VNODE_ATTRIB | DISPATCH_VNODE_EXTEND | DISPATCH_VNODE_LINK | DISPATCH_VNODE_REVOKE), etcq);
    if (hostssrc == NULL)
    {
        close(fd);
        return -1;
    }
    dispatch_source_set_event_handler(hostssrc,
                                      ^{
                                          u_int32_t flags = dispatch_source_get_data(hostssrc);
                                          LogInfo("mDNSMacOSXGetEtcHostsFD: /etc/hosts changed 0x%x", flags);
                                          if ((flags & (DISPATCH_VNODE_DELETE | DISPATCH_VNODE_RENAME)) != 0)
                                          {
                                              dispatch_source_cancel(hostssrc);
                                              dispatch_release(hostssrc);
                                              hostssrc = NULL;
                                              // Bug in LibDispatch: wait a second before scheduling the block. If we schedule
                                              // the block immediately, we try to open the file and the file may not exist and may
                                              // fail to get a notification in the future. When the file does not exist and
                                              // we start to monitor the directory, on "dispatch_resume" of that source, there
                                              // is no guarantee that the file creation will be notified always because when
                                              // the dispatch_resume returns, the kevent manager may not have registered the
                                              // kevent yet but the file may have been created
                                              usleep(1000000);
                                              dispatch_async(etcq, ^{mDNSMacOSXUpdateEtcHosts(m);});
                                              return;
                                          }
                                          if ((flags & DISPATCH_VNODE_WRITE) != 0)
                                          {
                                              mDNSMacOSXUpdateEtcHosts(m);
                                          }
                                      });
    dispatch_source_set_cancel_handler(hostssrc, ^{LogInfo("mDNSMacOSXGetEtcHostsFD: Closing etchosts fd %d", fd); close(fd);});
    dispatch_resume(hostssrc);

    // Cleanup /etc source, no need to watch it if we already have /etc/hosts
    if (etcsrc)
    {
        dispatch_source_cancel(etcsrc);
        dispatch_release(etcsrc);
        etcsrc = NULL;
    }

    LogInfo("mDNSMacOSXGetEtcHostsFD: /etc/hosts being monitored, and not etc");
    return hostssrc ? (int)dispatch_source_get_handle(hostssrc) : -1;
#else
    (void)m;
    return fd;
#endif
}

// When /etc/hosts is modified, flush all the cache records as there may be local
// authoritative answers now
mDNSlocal void FlushAllCacheRecords(mDNS *const m)
{
    CacheRecord *cr;
    mDNSu32 slot;
    CacheGroup *cg;

    FORALL_CACHERECORDS(slot, cg, cr)
    {
        // Skip multicast.
        if (cr->resrec.InterfaceID) continue;

        // If a resource record can answer A or AAAA, they need to be flushed so that we will
        // never used to deliver an ADD or RMV
        if (RRTypeAnswersQuestionType(&cr->resrec, kDNSType_A) ||
            RRTypeAnswersQuestionType(&cr->resrec, kDNSType_AAAA))
        {
            LogInfo("FlushAllCacheRecords: Purging Resourcerecord %s", CRDisplayString(m, cr));
            mDNS_PurgeCacheResourceRecord(m, cr);
        }
    }
}

// Add new entries to the core. If justCheck is set, this function does not add, just returns true
mDNSlocal mDNSBool EtcHostsAddNewEntries(AuthHash *newhosts, mDNSBool justCheck)
{
    mDNS *const m = &mDNSStorage;
    AuthGroup *ag;
    mDNSu32 slot;
    AuthRecord *rr, *primary, *rrnext;
    for (slot = 0; slot < AUTH_HASH_SLOTS; slot++)
        for (ag = newhosts->rrauth_hash[slot]; ag; ag = ag->next)
        {
            primary = NULL;
            for (rr = ag->members; rr; rr = rrnext)
            {
                rrnext = rr->next;
                AuthGroup *ag1;
                AuthRecord *rr1;
                mDNSBool found = mDNSfalse;
                ag1 = AuthGroupForRecord(&m->rrauth, &rr->resrec);
                if (ag1 && ag1->members)
                {
                    if (!primary) primary = ag1->members;
                    rr1 = ag1->members;
                    while (rr1)
                    {
                        // We are not using InterfaceID in checking for duplicates. This means,
                        // if there are two addresses for a given name e.g., fe80::1%en0 and
                        // fe80::1%en1, we only add the first one. It is not clear whether
                        // this is a common case. To fix this, we also need to modify
                        // mDNS_Register_internal in how it handles duplicates. If it becomes a
                        // common case, we will fix it then.
                        if (IdenticalResourceRecord(&rr1->resrec, &rr->resrec) && rr1->resrec.InterfaceID == rr->resrec.InterfaceID)
                        {
                            LogInfo("EtcHostsAddNewEntries: Skipping, not adding %s", ARDisplayString(m, rr1));
                            found = mDNStrue;
                            break;
                        }
                        rr1 = rr1->next;
                    }
                }
                if (!found)
                {
                    if (justCheck)
                    {
                        LogInfo("EtcHostsAddNewEntries: Entry %s not registered with core yet", ARDisplayString(m, rr));
                        return mDNStrue;
                    }
                    RemoveAuthRecord(m, newhosts, rr);
                    // if there is no primary, point to self
                    rr->RRSet = (primary ? primary : rr);
                    rr->next = NULL;
                    LogInfo("EtcHostsAddNewEntries: Adding %s ID %d", ARDisplayString(m, rr), IIDPrintable(rr->resrec.InterfaceID));
                    if (mDNS_Register_internal(m, rr) != mStatus_NoError)
                        LogMsg("EtcHostsAddNewEntries: mDNS_Register failed for %s", ARDisplayString(m, rr));
                }
            }
        }
    return mDNSfalse;
}

// Delete entries from the core that are no longer needed. If justCheck is set, this function
// does not delete, just returns true
mDNSlocal mDNSBool EtcHostsDeleteOldEntries(AuthHash *newhosts, mDNSBool justCheck)
{
    mDNS *const m = &mDNSStorage;
    AuthGroup *ag;
    mDNSu32 slot;
    AuthRecord *rr, *rrnext;
    for (slot = 0; slot < AUTH_HASH_SLOTS; slot++)
        for (ag = m->rrauth.rrauth_hash[slot]; ag; ag = ag->next)
            for (rr = ag->members; rr; rr = rrnext)
            {
                mDNSBool found = mDNSfalse;
                AuthGroup *ag1;
                AuthRecord *rr1;
                rrnext = rr->next;
                if (rr->RecordCallback != FreeEtcHosts) continue;
                ag1 = AuthGroupForRecord(newhosts, &rr->resrec);
                if (ag1)
                {
                    rr1 = ag1->members;
                    while (rr1)
                    {
                        if (IdenticalResourceRecord(&rr1->resrec, &rr->resrec))
                        {
                            LogInfo("EtcHostsDeleteOldEntries: Old record %s found in new, skipping", ARDisplayString(m, rr));
                            found = mDNStrue;
                            break;
                        }
                        rr1 = rr1->next;
                    }
                }
                // there is no corresponding record in newhosts for the same name. This means
                // we should delete this from the core.
                if (!found)
                {
                    if (justCheck)
                    {
                        LogInfo("EtcHostsDeleteOldEntries: Record %s not found in new, deleting", ARDisplayString(m, rr));
                        return mDNStrue;
                    }
                    // if primary is going away, make sure that the rest of the records
                    // point to the new primary
                    if (rr == ag->members)
                    {
                        AuthRecord *new_primary = rr->next;
                        AuthRecord *r = new_primary;
                        while (r)
                        {
                            if (r->RRSet == rr)
                            {
                                LogInfo("EtcHostsDeleteOldEntries: Updating Resource Record %s to primary", ARDisplayString(m, r));
                                r->RRSet = new_primary;
                            }
                            else LogMsg("EtcHostsDeleteOldEntries: ERROR!! Resource Record %s not pointing to primary %##s", ARDisplayString(m, r), r->resrec.name);
                            r = r->next;
                        }
                    }
                    LogInfo("EtcHostsDeleteOldEntries: Deleting %s", ARDisplayString(m, rr));
                    mDNS_Deregister_internal(m, rr, mDNS_Dereg_normal);
                }
            }
    return mDNSfalse;
}

mDNSlocal void UpdateEtcHosts(mDNS *const m, void *context)
{
    AuthHash *newhosts = (AuthHash *)context;

    mDNS_CheckLock(m);

    //Delete old entries from the core if they are not present in the newhosts
    EtcHostsDeleteOldEntries(newhosts, mDNSfalse);
    // Add the new entries to the core if not already present in the core
    EtcHostsAddNewEntries(newhosts, mDNSfalse);
}

mDNSlocal void FreeNewHosts(AuthHash *newhosts)
{
    mDNSu32 slot;
    AuthGroup *ag, *agnext;
    AuthRecord *rr, *rrnext;

    for (slot = 0; slot < AUTH_HASH_SLOTS; slot++)
        for (ag = newhosts->rrauth_hash[slot]; ag; ag = agnext)
        {
            agnext = ag->next;
            for (rr = ag->members; rr; rr = rrnext)
            {
                rrnext = rr->next;
                freeL("etchosts", rr);
            }
            freeL("AuthGroups", ag);
        }
}

mDNSlocal void mDNSMacOSXUpdateEtcHosts(mDNS *const m)
{
    AuthHash newhosts;

    // As we will be modifying the core, we can only have one thread running at
    // any point in time.
    KQueueLock();

    mDNSPlatformMemZero(&newhosts, sizeof(AuthHash));

    // Get the file desecriptor (will trigger us to start watching for changes)
    int fd = mDNSMacOSXGetEtcHostsFD();
    if (fd != -1)
    {
        LogInfo("mDNSMacOSXUpdateEtcHosts: Parsing /etc/hosts fd %d", fd);
        mDNSMacOSXParseEtcHosts(fd, &newhosts);
    }
    else LogInfo("mDNSMacOSXUpdateEtcHosts: /etc/hosts is not present");

    // Optimization: Detect whether /etc/hosts changed or not.
    //
    // 1. Check to see if there are any new entries. We do this by seeing whether any entries in
    //    newhosts is already registered with core.  If we find at least one entry that is not
    //    registered with core, then it means we have work to do.
    //
    // 2. Next, we check to see if any of the entries that are registered with core is not present
    //   in newhosts. If we find at least one entry that is not present, it means we have work to
    //   do.
    //
    // Note: We may not have to hold the lock right here as KQueueLock is held which prevents any
    // other thread from running. But mDNS_Lock is needed here as we will be traversing the core
    // data structure in EtcHostsDeleteOldEntries/NewEntries which might expect the lock to be held
    // in the future and this code does not have to change.
    mDNS_Lock(m);
    // Add the new entries to the core if not already present in the core
    if (!EtcHostsAddNewEntries(&newhosts, mDNStrue))
    {
        // No new entries to add, check to see if we need to delete any old entries from the
        // core if they are not present in the newhosts
        if (!EtcHostsDeleteOldEntries(&newhosts, mDNStrue))
        {
            LogInfo("mDNSMacOSXUpdateEtcHosts: No work");
            FreeNewHosts(&newhosts);
            mDNS_Unlock(m);
            KQueueUnlock("/etc/hosts changed");
            return;
        }
    }

    // This will flush the cache, stop and start the query so that the queries
    // can look at the /etc/hosts again
    //
    // Notes:
    //
    // We can't delete and free the records here. We wait for the mDNSCoreRestartAddressQueries to
    // deliver RMV events. It has to be done in a deferred way because we can't deliver RMV
    // events for local records *before* the RMV events for cache records. mDNSCoreRestartAddressQueries
    // delivers these events in the right order and then calls us back to delete them.
    //
    // Similarly, we do a deferred Registration of the record because mDNSCoreRestartAddressQueries
    // is a common function that looks at all local auth records and delivers a RMV including
    // the records that we might add here. If we deliver a ADD here, it will get a RMV and then when
    // the query is restarted, it will get another ADD. To avoid this (ADD-RMV-ADD), we defer registering
    // the record until the RMVs are delivered in mDNSCoreRestartAddressQueries after which UpdateEtcHosts
    // is called back where we do the Registration of the record. This results in RMV followed by ADD which
    // looks normal.
    mDNSCoreRestartAddressQueries(m, mDNSfalse, FlushAllCacheRecords, UpdateEtcHosts, &newhosts);
    FreeNewHosts(&newhosts);
    mDNS_Unlock(m);
    KQueueUnlock("/etc/hosts changed");
}

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Initialization & Teardown
#endif

CF_EXPORT CFDictionaryRef _CFCopySystemVersionDictionary(void);
CF_EXPORT const CFStringRef _kCFSystemVersionProductNameKey;
CF_EXPORT const CFStringRef _kCFSystemVersionProductVersionKey;
CF_EXPORT const CFStringRef _kCFSystemVersionBuildVersionKey;

// Major version 13 is 10.9.x 
mDNSexport void mDNSMacOSXSystemBuildNumber(char *HINFO_SWstring)
{
    int major = 0, minor = 0;
    char letter = 0, prodname[256]="<Unknown>", prodvers[256]="<Unknown>", buildver[256]="<Unknown>";
    CFDictionaryRef vers = _CFCopySystemVersionDictionary();
    if (vers)
    {
        CFStringRef cfprodname = CFDictionaryGetValue(vers, _kCFSystemVersionProductNameKey);
        CFStringRef cfprodvers = CFDictionaryGetValue(vers, _kCFSystemVersionProductVersionKey);
        CFStringRef cfbuildver = CFDictionaryGetValue(vers, _kCFSystemVersionBuildVersionKey);
        if (cfprodname) 
            CFStringGetCString(cfprodname, prodname, sizeof(prodname), kCFStringEncodingUTF8);
        if (cfprodvers) 
            CFStringGetCString(cfprodvers, prodvers, sizeof(prodvers), kCFStringEncodingUTF8);
        if (cfbuildver && CFStringGetCString(cfbuildver, buildver, sizeof(buildver), kCFStringEncodingUTF8))
            sscanf(buildver, "%d%c%d", &major, &letter, &minor);
        CFRelease(vers);
    }
    if (!major) 
    { 
        major = 13; 
        LogMsg("Note: No Major Build Version number found; assuming 13"); 
    }
    if (HINFO_SWstring) 
        mDNS_snprintf(HINFO_SWstring, 256, "%s %s (%s), %s", prodname, prodvers, buildver, STRINGIFY(mDNSResponderVersion));
    //LogMsg("%s %s (%s), %d %c %d", prodname, prodvers, buildver, major, letter, minor);

    // If product name is "Mac OS X" (or similar) we set OSXVers, else we set iOSVers;
    if ((prodname[0] & 0xDF) == 'M') 
        OSXVers = major;
    else 
        iOSVers = major;
}

// Test to see if we're the first client running on UDP port 5353, by trying to bind to 5353 without using SO_REUSEPORT.
// If we fail, someone else got here first. That's not a big problem; we can share the port for multicast responses --
// we just need to be aware that we shouldn't expect to successfully receive unicast UDP responses.
mDNSlocal mDNSBool mDNSPlatformInit_CanReceiveUnicast(void)
{
    int err = -1;
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 3)
        LogMsg("mDNSPlatformInit_CanReceiveUnicast: socket error %d errno %d (%s)", s, errno, strerror(errno));
    else
    {
        struct sockaddr_in s5353;
        s5353.sin_family      = AF_INET;
        s5353.sin_port        = MulticastDNSPort.NotAnInteger;
        s5353.sin_addr.s_addr = 0;
        err = bind(s, (struct sockaddr *)&s5353, sizeof(s5353));
        close(s);
    }

    if (err) LogMsg("No unicast UDP responses");
    else debugf("Unicast UDP responses okay");
    return(err == 0);
}

mDNSlocal void CreatePTRRecord(const domainname *domain)
{
    AuthRecord *rr;
    const domainname *pname = (domainname *)"\x9" "localhost";

    rr = (AuthRecord *) callocL("localhosts", sizeof(*rr));
    if (rr == NULL) return;

    mDNS_SetupResourceRecord(rr, mDNSNULL, mDNSInterface_LocalOnly, kDNSType_PTR, kHostNameTTL, kDNSRecordTypeKnownUnique, AuthRecordLocalOnly, mDNSNULL, mDNSNULL);
    AssignDomainName(&rr->namestorage, domain);

    rr->resrec.rdlength = DomainNameLength(pname);
    rr->resrec.rdata->u.name.c[0] = 0;
    AssignDomainName(&rr->resrec.rdata->u.name, pname);

    rr->resrec.namehash = DomainNameHashValue(rr->resrec.name);
    SetNewRData(&rr->resrec, mDNSNULL, 0);  // Sets rr->rdatahash for us
    mDNS_Register(&mDNSStorage, rr);
}

// Setup PTR records for 127.0.0.1 and ::1. This helps answering them locally rather than relying
// on the external DNS server to answer this. Sometimes, the DNS servers don't respond in a timely
// fashion and applications depending on this e.g., telnetd, times out after 30 seconds creating
// a bad user experience. For now, we specifically create only localhosts to handle radar://9354225
//
// Note: We could have set this up while parsing the entries in /etc/hosts. But this is kept separate
// intentionally to avoid adding to the complexity of code handling /etc/hosts.
mDNSlocal void SetupLocalHostRecords(void)
{
    domainname name;

    MakeDomainNameFromDNSNameString(&name, "1.0.0.127.in-addr.arpa.");
    CreatePTRRecord(&name);

    MakeDomainNameFromDNSNameString(&name, "1.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.ip6.arpa.");
    CreatePTRRecord(&name);
}

#if APPLE_OSX_mDNSResponder // Don't compile for dnsextd target
mDNSlocal void setSameDomainLabelPointer(void);
#endif

// Construction of Default Browse domain list (i.e. when clients pass NULL) is as follows:
// 1) query for b._dns-sd._udp.local on LocalOnly interface
//    (.local manually generated via explicit callback)
// 2) for each search domain (from prefs pane), query for b._dns-sd._udp.<searchdomain>.
// 3) for each result from (2), register LocalOnly PTR record b._dns-sd._udp.local. -> <result>
// 4) result above should generate a callback from question in (1).  result added to global list
// 5) global list delivered to client via GetSearchDomainList()
// 6) client calls to enumerate domains now go over LocalOnly interface
//    (!!!KRS may add outgoing interface in addition)

#if MDNSRESPONDER_SUPPORTS(APPLE, IGNORE_HOSTS_FILE)
mDNSlocal mStatus RegisterLocalOnlyAddressRecord(const domainname *const name, mDNSu16 type, const void *rdata, mDNSu16 rdlength)
{
    switch(type)
    {
    case kDNSType_A:
        if (rdlength != 4) return (mStatus_BadParamErr);
        break;

    case kDNSType_AAAA:
        if (rdlength != 16) return (mStatus_BadParamErr);
        break;

    default:
        return (mStatus_BadParamErr);
    }

    AuthRecord *rr = (AuthRecord *) callocL("etchosts", sizeof(*rr));
    if (!rr) return (mStatus_NoMemoryErr);

    mDNS_SetupResourceRecord(rr, NULL, mDNSInterface_LocalOnly, type, 1, kDNSRecordTypeKnownUnique, AuthRecordLocalOnly, FreeEtcHosts, NULL);
    AssignDomainName(&rr->namestorage, name);
    mDNSPlatformMemCopy(rr->resrec.rdata->u.data, rdata, rdlength);

    const mStatus err = mDNS_Register_internal(&mDNSStorage, rr);
    if (err)
    {
        LogMsg("RegisterLocalOnlyAddressRecord: mDNS_Register error %d registering %s", err, ARDisplayString(&mDNSStorage, rr));
        freeL("etchosts", rr);
    }
    return (err);
}

mDNSlocal void RegisterLocalOnlyARecord(const domainname *const name, const mDNSv4Addr *const addr)
{
    RegisterLocalOnlyAddressRecord(name, kDNSType_A, addr->b, (mDNSu16)sizeof(mDNSv4Addr));
}

mDNSlocal void RegisterLocalOnlyAAAARecord(const domainname *const name, const mDNSv6Addr *const addr)
{
    RegisterLocalOnlyAddressRecord(name, kDNSType_AAAA, addr->b, (mDNSu16)sizeof(mDNSv6Addr));
}
#endif  // MDNSRESPONDER_SUPPORTS(APPLE, IGNORE_HOSTS_FILE)

mDNSlocal mStatus mDNSPlatformInit_setup(mDNS *const m)
{
    mStatus err;

    char HINFO_SWstring[256] = "";
    mDNSMacOSXSystemBuildNumber(HINFO_SWstring);

#if APPLE_OSX_mDNSResponder
    setSameDomainLabelPointer();
#endif

    err = mDNSHelperInit();
    if (err)
        return err;
    
    // Store mDNSResponder Platform 
    if (OSXVers)
    {
        m->mDNS_plat = platform_OSX;
    }
    else if (iOSVers)
    {
        if (IsAppleTV())
            m->mDNS_plat = platform_Atv;
        else
            m->mDNS_plat = platform_iOS;
    }
    else
    {
        m->mDNS_plat = platform_NonApple; 
    }   
        
    // In 10.4, mDNSResponder is launched very early in the boot process, while other subsystems are still in the process of starting up.
    // If we can't read the user's preferences, then we sleep a bit and try again, for up to five seconds before we give up.
    int i;
    for (i=0; i<100; i++)
    {
        domainlabel testlabel;
        testlabel.c[0] = 0;
        GetUserSpecifiedLocalHostName(&testlabel);
        if (testlabel.c[0]) break;
        usleep(50000);
    }

    m->hostlabel.c[0]        = 0;

#if MDNSRESPONDER_SUPPORTS(APPLE, RANDOM_AWDL_HOSTNAME)
    GetRandomUUIDLocalHostname(&m->RandomizedHostname);
#endif
    int get_model[2] = { CTL_HW, HW_MODEL };
    size_t len_model = sizeof(HINFO_HWstring_buffer);

    // Normal Apple model names are of the form "iPhone2,1", and
    // internal code names are strings containing no commas, e.g. "N88AP".
    // We used to ignore internal code names, but Apple now uses these internal code names
    // even in released shipping products, so we no longer ignore strings containing no commas.
//  if (sysctl(get_model, 2, HINFO_HWstring_buffer, &len_model, NULL, 0) == 0 && strchr(HINFO_HWstring_buffer, ','))
    if (sysctl(get_model, 2, HINFO_HWstring_buffer, &len_model, NULL, 0) == 0)
        HINFO_HWstring = HINFO_HWstring_buffer;

    // For names of the form "iPhone2,1" we use "iPhone" as the prefix for automatic name generation.
    // For names of the form "N88AP" containg no comma, we use the entire string.
    HINFO_HWstring_prefixlen = strchr(HINFO_HWstring_buffer, ',') ? strcspn(HINFO_HWstring, "0123456789") : strlen(HINFO_HWstring);

    if (mDNSPlatformInit_CanReceiveUnicast()) 
        m->CanReceiveUnicastOn5353 = mDNStrue;

    mDNSu32 hlen = mDNSPlatformStrLen(HINFO_HWstring);
    mDNSu32 slen = mDNSPlatformStrLen(HINFO_SWstring);
    if (hlen + slen < 254)
    {
        m->HIHardware.c[0] = hlen;
        m->HISoftware.c[0] = slen;
        mDNSPlatformMemCopy(&m->HIHardware.c[1], HINFO_HWstring, hlen);
        mDNSPlatformMemCopy(&m->HISoftware.c[1], HINFO_SWstring, slen);
    }

    m->p->permanentsockets.port  = MulticastDNSPort;
    m->p->permanentsockets.m     = m;
    m->p->permanentsockets.sktv4 = -1;
    m->p->permanentsockets.kqsv4.KQcallback = myKQSocketCallBack;
    m->p->permanentsockets.kqsv4.KQcontext  = &m->p->permanentsockets;
    m->p->permanentsockets.kqsv4.KQtask     = "IPv4 UDP packet reception";
    m->p->permanentsockets.sktv6 = -1;
    m->p->permanentsockets.kqsv6.KQcallback = myKQSocketCallBack;
    m->p->permanentsockets.kqsv6.KQcontext  = &m->p->permanentsockets;
    m->p->permanentsockets.kqsv6.KQtask     = "IPv6 UDP packet reception";

    err = SetupSocket(&m->p->permanentsockets, MulticastDNSPort, AF_INET, mDNSNULL);
    if (err) LogMsg("mDNSPlatformInit_setup: SetupSocket(AF_INET) failed error %d errno %d (%s)", err, errno, strerror(errno));
    err = SetupSocket(&m->p->permanentsockets, MulticastDNSPort, AF_INET6, mDNSNULL);
    if (err) LogMsg("mDNSPlatformInit_setup: SetupSocket(AF_INET6) failed error %d errno %d (%s)", err, errno, strerror(errno));

    struct sockaddr_in s4;
    socklen_t n4 = sizeof(s4);
    if (getsockname(m->p->permanentsockets.sktv4, (struct sockaddr *)&s4, &n4) < 0) 
        LogMsg("getsockname v4 error %d (%s)", errno, strerror(errno));
    else 
        m->UnicastPort4.NotAnInteger = s4.sin_port;

    if (m->p->permanentsockets.sktv6 >= 0)
    {
        struct sockaddr_in6 s6;
        socklen_t n6 = sizeof(s6);
        if (getsockname(m->p->permanentsockets.sktv6, (struct sockaddr *)&s6, &n6) < 0) LogMsg("getsockname v6 error %d (%s)", errno, strerror(errno));
        else m->UnicastPort6.NotAnInteger = s6.sin6_port;
    }

    m->p->InterfaceList         = mDNSNULL;
    m->p->InterfaceMonitors     = NULL;
    m->p->userhostlabel.c[0]    = 0;
    m->p->usernicelabel.c[0]    = 0;
    m->p->prevoldnicelabel.c[0] = 0;
    m->p->prevnewnicelabel.c[0] = 0;
    m->p->prevoldhostlabel.c[0] = 0;
    m->p->prevnewhostlabel.c[0] = 0;
    m->p->NotifyUser         = 0;
    m->p->KeyChainTimer      = 0;
    m->p->WakeAtUTC          = 0;
    m->p->RequestReSleep     = 0;
    // Assume that everything is good to begin with. If something is not working,
    // we will detect that when we start sending questions.
    m->p->v4answers          = 1;
    m->p->v6answers          = 1;
    m->p->DNSTrigger         = 0;
    m->p->LastConfigGeneration = 0;
    m->p->if_interface_changed = mDNSfalse;

    NetworkChangedKey_IPv4         = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL, kSCDynamicStoreDomainState, kSCEntNetIPv4);
    NetworkChangedKey_IPv6         = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL, kSCDynamicStoreDomainState, kSCEntNetIPv6);
    NetworkChangedKey_Hostnames    = SCDynamicStoreKeyCreateHostNames(NULL);
    NetworkChangedKey_Computername = SCDynamicStoreKeyCreateComputerName(NULL);
    NetworkChangedKey_DNS          = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL, kSCDynamicStoreDomainState, kSCEntNetDNS);
    NetworkChangedKey_StateInterfacePrefix = SCDynamicStoreKeyCreateNetworkInterfaceEntity(NULL, kSCDynamicStoreDomainState, CFSTR(""), NULL);
    if (!NetworkChangedKey_IPv4 || !NetworkChangedKey_IPv6 || !NetworkChangedKey_Hostnames || !NetworkChangedKey_Computername || !NetworkChangedKey_DNS || !NetworkChangedKey_StateInterfacePrefix)
    { LogMsg("SCDynamicStore string setup failed"); return(mStatus_NoMemoryErr); }

    err = WatchForNetworkChanges(m);
    if (err) { LogMsg("mDNSPlatformInit_setup: WatchForNetworkChanges failed %d", err); return(err); }

    err = WatchForSysEvents(m);
    if (err) { LogMsg("mDNSPlatformInit_setup: WatchForSysEvents failed %d", err); return(err); }

    mDNSs32 utc = mDNSPlatformUTC();
    m->SystemWakeOnLANEnabled = SystemWakeForNetworkAccess();
    myGetIfAddrs(1);
    UpdateInterfaceList(utc);
    SetupActiveInterfaces(utc);
    ReorderInterfaceList();

    // Explicitly ensure that our Keychain operations utilize the system domain.
#ifndef NO_SECURITYFRAMEWORK
    SecKeychainSetPreferenceDomain(kSecPreferencesDomainSystem);
#endif

    mDNS_Lock(m);
    SetDomainSecrets(m);
    SetLocalDomains();
    mDNS_Unlock(m);

#ifndef NO_SECURITYFRAMEWORK
    err = SecKeychainAddCallback(KeychainChanged, kSecAddEventMask|kSecDeleteEventMask|kSecUpdateEventMask, m);
    if (err) { LogMsg("mDNSPlatformInit_setup: SecKeychainAddCallback failed %d", err); return(err); }
#endif

#if !defined(kIOPMAcknowledgmentOptionSystemCapabilityRequirements) || TARGET_OS_IPHONE
    LogMsg("Note: Compiled without SnowLeopard Fine-Grained Power Management support");
#else
    IOPMConnection c;
    IOReturn iopmerr = IOPMConnectionCreate(CFSTR("mDNSResponder"), kIOPMSystemPowerStateCapabilityCPU, &c);
    if (iopmerr) LogMsg("IOPMConnectionCreate failed %d", iopmerr);
    else
    {
        iopmerr = IOPMConnectionSetNotification(c, m, SnowLeopardPowerChanged);
        if (iopmerr) LogMsg("IOPMConnectionSetNotification failed %d", iopmerr);
        else
        {
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
            IOPMConnectionSetDispatchQueue(c, dispatch_get_main_queue());
            LogInfo("IOPMConnectionSetDispatchQueue is now running");
#else
            iopmerr = IOPMConnectionScheduleWithRunLoop(c, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
            if (iopmerr) LogMsg("IOPMConnectionScheduleWithRunLoop failed %d", iopmerr);
            LogInfo("IOPMConnectionScheduleWithRunLoop is now running");
#endif /* MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM */
        }
    }
    m->p->IOPMConnection = iopmerr ? mDNSNULL : c;
    if (iopmerr) // If IOPMConnectionCreate unavailable or failed, proceed with old-style power notification code below
#endif // kIOPMAcknowledgmentOptionSystemCapabilityRequirements
    {
        m->p->PowerConnection = IORegisterForSystemPower(m, &m->p->PowerPortRef, PowerChanged, &m->p->PowerNotifier);
        if (!m->p->PowerConnection) { LogMsg("mDNSPlatformInit_setup: IORegisterForSystemPower failed"); return(-1); }
        else
        {
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
            IONotificationPortSetDispatchQueue(m->p->PowerPortRef, dispatch_get_main_queue());
#else
            CFRunLoopAddSource(CFRunLoopGetMain(), IONotificationPortGetRunLoopSource(m->p->PowerPortRef), kCFRunLoopDefaultMode);
#endif /* MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM */
        }
    }

#if APPLE_OSX_mDNSResponder
    // Note: We use SPMetricPortability > 35 to indicate a laptop of some kind
    // SPMetricPortability <= 35 means nominally a non-portable machine (i.e. Mac mini or better)
    // Apple TVs, AirPort base stations, and Time Capsules do not actually weigh 3kg, but we assign them
    // higher 'nominal' masses to indicate they should be treated as being relatively less portable than a laptop
    if      (!strncasecmp(HINFO_HWstring, "Xserve",       6)) { SPMetricPortability = 25 /* 30kg */; SPMetricMarginalPower = 84 /* 250W */; SPMetricTotalPower = 85 /* 300W */; }
    else if (!strncasecmp(HINFO_HWstring, "RackMac",      7)) { SPMetricPortability = 25 /* 30kg */; SPMetricMarginalPower = 84 /* 250W */; SPMetricTotalPower = 85 /* 300W */; }
    else if (!strncasecmp(HINFO_HWstring, "MacPro",       6)) { SPMetricPortability = 27 /* 20kg */; SPMetricMarginalPower = 84 /* 250W */; SPMetricTotalPower = 85 /* 300W */; }
    else if (!strncasecmp(HINFO_HWstring, "PowerMac",     8)) { SPMetricPortability = 27 /* 20kg */; SPMetricMarginalPower = 82 /* 160W */; SPMetricTotalPower = 83 /* 200W */; }
    else if (!strncasecmp(HINFO_HWstring, "iMac",         4)) { SPMetricPortability = 30 /* 10kg */; SPMetricMarginalPower = 77 /*  50W */; SPMetricTotalPower = 78 /*  60W */; }
    else if (!strncasecmp(HINFO_HWstring, "Macmini",      7)) { SPMetricPortability = 33 /*  5kg */; SPMetricMarginalPower = 73 /*  20W */; SPMetricTotalPower = 74 /*  25W */; }
    else if (!strncasecmp(HINFO_HWstring, "TimeCapsule", 11)) { SPMetricPortability = 34 /*  4kg */; SPMetricMarginalPower = 10 /*  ~0W */; SPMetricTotalPower = 70 /*  13W */; }
    else if (!strncasecmp(HINFO_HWstring, "AirPort",      7)) { SPMetricPortability = 35 /*  3kg */; SPMetricMarginalPower = 10 /*  ~0W */; SPMetricTotalPower = 70 /*  12W */; }
    else if (  IsAppleTV()  )                                 { SPMetricPortability = 35 /*  3kg */; SPMetricMarginalPower = 60 /*   1W */; SPMetricTotalPower = 63 /*   2W */; }
    else if (!strncasecmp(HINFO_HWstring, "MacBook",      7)) { SPMetricPortability = 37 /*  2kg */; SPMetricMarginalPower = 71 /*  13W */; SPMetricTotalPower = 72 /*  15W */; }
    else if (!strncasecmp(HINFO_HWstring, "PowerBook",    9)) { SPMetricPortability = 37 /*  2kg */; SPMetricMarginalPower = 71 /*  13W */; SPMetricTotalPower = 72 /*  15W */; }
    LogSPS("HW_MODEL: %.*s (%s) Portability %d Marginal Power %d Total Power %d Features %d",
           HINFO_HWstring_prefixlen, HINFO_HWstring, HINFO_HWstring, SPMetricPortability, SPMetricMarginalPower, SPMetricTotalPower, SPMetricFeatures);
#endif // APPLE_OSX_mDNSResponder

    // Currently this is not defined. SSL code will eventually fix this. If it becomes
    // critical, we will define this to workaround the bug in SSL.
#ifdef __SSL_NEEDS_SERIALIZATION__
    SSLqueue = dispatch_queue_create("com.apple.mDNSResponder.SSLQueue", NULL);
#else
    SSLqueue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
#endif
    if (SSLqueue == mDNSNULL) LogMsg("dispatch_queue_create: SSL queue NULL");

#if MDNSRESPONDER_SUPPORTS(APPLE, IGNORE_HOSTS_FILE)
    // On device OSes (iOS, tvOS, watchOS, etc.), ignore /etc/hosts unless the OS is an internal build. When the /etc/hosts
    // file is ignored, LocalOnly auth records will be registered for localhost and broadcasthost addresses contained in the
    // standard /etc/hosts file:
    //
    //  127.0.0.1       localhost
    //  255.255.255.255 broadcasthost
    //  ::1             localhost

    if (!IsAppleInternalBuild())
    {
        const domainname *const localHostName     = (const domainname *) "\x9" "localhost";
        const domainname *const broadcastHostName = (const domainname *) "\xd" "broadcasthost";
        const mDNSv4Addr        localHostV4       = { { 127, 0, 0, 1 } };
        mDNSv6Addr              localHostV6;

        // Register localhost 127.0.0.1 A record.

        RegisterLocalOnlyARecord(localHostName, &localHostV4);

        // Register broadcasthost 255.255.255.255 A record.

        RegisterLocalOnlyARecord(broadcastHostName, &onesIPv4Addr);

        // Register localhost ::1 AAAA record.

        mDNSPlatformMemZero(&localHostV6, sizeof(localHostV6));
        localHostV6.b[15] = 1;
        RegisterLocalOnlyAAAARecord(localHostName, &localHostV6);
    }
    else
#endif
    {
        mDNSMacOSXUpdateEtcHosts(m);
    }
    SetupLocalHostRecords();
    
#if MDNSRESPONDER_SUPPORTS(COMMON, DNS_PUSH)
    dso_transport_init();
#endif

    return(mStatus_NoError);
}

mDNSexport mStatus mDNSPlatformInit(mDNS *const m)
{
#if MDNS_NO_DNSINFO
    LogMsg("Note: Compiled without Apple-specific Split-DNS support");
#endif

    // Adding interfaces will use this flag, so set it now.
    m->DivertMulticastAdvertisements = !m->AdvertiseLocalAddresses;

#if APPLE_OSX_mDNSResponder
    m->SPSBrowseCallback = UpdateSPSStatus;
#endif // APPLE_OSX_mDNSResponder

    mStatus result = mDNSPlatformInit_setup(m);

    // We don't do asynchronous initialization on OS X, so by the time we get here the setup will already
    // have succeeded or failed -- so if it succeeded, we should just call mDNSCoreInitComplete() immediately
    if (result == mStatus_NoError)
    {
        mDNSCoreInitComplete(m, mStatus_NoError);
        initializeD2DPlugins(m);
    }
    result = DNSSECCryptoInit(m);
    return(result);
}

mDNSexport void mDNSPlatformClose(mDNS *const m)
{
    if (m->p->PowerConnection)
    {
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
        IONotificationPortSetDispatchQueue(m->p->PowerPortRef, NULL);
#else
        CFRunLoopRemoveSource(CFRunLoopGetMain(), IONotificationPortGetRunLoopSource(m->p->PowerPortRef), kCFRunLoopDefaultMode);
#endif
        // According to <http://developer.apple.com/qa/qa2004/qa1340.html>, a single call
        // to IORegisterForSystemPower creates *three* objects that need to be disposed individually:
        IODeregisterForSystemPower(&m->p->PowerNotifier);
        IOServiceClose            ( m->p->PowerConnection);
        IONotificationPortDestroy ( m->p->PowerPortRef);
        m->p->PowerConnection = 0;
    }

    if (m->p->Store)
    {
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
        if (!SCDynamicStoreSetDispatchQueue(m->p->Store, NULL))
            LogMsg("mDNSPlatformClose: SCDynamicStoreSetDispatchQueue failed");
#else
        CFRunLoopRemoveSource(CFRunLoopGetMain(), m->p->StoreRLS, kCFRunLoopDefaultMode);
        CFRunLoopSourceInvalidate(m->p->StoreRLS);
        CFRelease(m->p->StoreRLS);
        m->p->StoreRLS = NULL;
#endif
        CFRelease(m->p->Store);
        m->p->Store    = NULL;
    }

    if (m->p->PMRLS)
    {
        CFRunLoopRemoveSource(CFRunLoopGetMain(), m->p->PMRLS, kCFRunLoopDefaultMode);
        CFRunLoopSourceInvalidate(m->p->PMRLS);
        CFRelease(m->p->PMRLS);
        m->p->PMRLS = NULL;
    }

    if (m->p->SysEventNotifier >= 0) { close(m->p->SysEventNotifier); m->p->SysEventNotifier = -1; }
    terminateD2DPlugins();

    mDNSs32 utc = mDNSPlatformUTC();
    MarkAllInterfacesInactive(utc);
    ClearInactiveInterfaces(utc);
    CloseSocketSet(&m->p->permanentsockets);

    if (m->p->InterfaceMonitors)
    {
        CFArrayRef monitors = m->p->InterfaceMonitors;
        m->p->InterfaceMonitors = NULL;
        const CFIndex n = CFArrayGetCount(monitors);
        for (CFIndex i = 0; i < n; i++)
        {
            mdns_interface_monitor_invalidate((mdns_interface_monitor_t) CFArrayGetValueAtIndex(monitors, i));
        }
        CFRelease(monitors);
    }
}

#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - General Platform Support Layer functions
#endif

mDNSexport mDNSu32 mDNSPlatformRandomNumber(void)
{
    return(arc4random());
}

mDNSexport mDNSs32 mDNSPlatformOneSecond = 1000;
mDNSexport mDNSu32 mDNSPlatformClockDivisor = 0;

mDNSexport mStatus mDNSPlatformTimeInit(void)
{
    // Notes: Typical values for mach_timebase_info:
    // tbi.numer = 1000 million
    // tbi.denom =   33 million
    // These are set such that (mach_absolute_time() * numer/denom) gives us nanoseconds;
    //          numer  / denom = nanoseconds per hardware clock tick (e.g. 30);
    //          denom  / numer = hardware clock ticks per nanosecond (e.g. 0.033)
    // (denom*1000000) / numer = hardware clock ticks per millisecond (e.g. 33333)
    // So: mach_absolute_time() / ((denom*1000000)/numer) = milliseconds
    //
    // Arithmetic notes:
    // tbi.denom is at least 1, and not more than 2^32-1.
    // Therefore (tbi.denom * 1000000) is at least one million, but cannot overflow a uint64_t.
    // tbi.denom is at least 1, and not more than 2^32-1.
    // Therefore clockdivisor should end up being a number roughly in the range 10^3 - 10^9.
    // If clockdivisor is less than 10^3 then that means that the native clock frequency is less than 1MHz,
    // which is unlikely on any current or future Macintosh.
    // If clockdivisor is greater than 10^9 then that means the native clock frequency is greater than 1000GHz.
    // When we ship Macs with clock frequencies above 1000GHz, we may have to update this code.
    struct mach_timebase_info tbi;
    kern_return_t result = mach_timebase_info(&tbi);
    if (result == KERN_SUCCESS) mDNSPlatformClockDivisor = ((uint64_t)tbi.denom * 1000000) / tbi.numer;
    return(result);
}

mDNSexport mDNSs32 mDNSPlatformRawTime(void)
{
    if (mDNSPlatformClockDivisor == 0) { LogMsg("mDNSPlatformRawTime called before mDNSPlatformTimeInit"); return(0); }

    static uint64_t last_mach_absolute_time = 0;
    //static uint64_t last_mach_absolute_time = 0x8000000000000000LL;   // Use this value for testing the alert display
    uint64_t this_mach_absolute_time = mach_absolute_time();
    if ((int64_t)this_mach_absolute_time - (int64_t)last_mach_absolute_time < 0)
    {
        LogMsg("mDNSPlatformRawTime: last_mach_absolute_time %08X%08X", last_mach_absolute_time);
        LogMsg("mDNSPlatformRawTime: this_mach_absolute_time %08X%08X", this_mach_absolute_time);
        // Update last_mach_absolute_time *before* calling NotifyOfElusiveBug()
        last_mach_absolute_time = this_mach_absolute_time;
        // Note: This bug happens all the time on 10.3
        NotifyOfElusiveBug("mach_absolute_time went backwards!",
                           "This error occurs from time to time, often on newly released hardware, "
                           "and usually the exact cause is different in each instance.\r\r"
                           "Please file a new Radar bug report with the title “mach_absolute_time went backwards” "
                           "and assign it to Radar Component “Kernel” Version “X”.");
    }
    last_mach_absolute_time = this_mach_absolute_time;

    return((mDNSs32)(this_mach_absolute_time / mDNSPlatformClockDivisor));
}

mDNSexport mDNSs32 mDNSPlatformUTC(void)
{
    return time(NULL);
}

// Locking is a no-op here, because we're single-threaded with a CFRunLoop, so we can never interrupt ourselves
mDNSexport void     mDNSPlatformLock   (const mDNS *const m) { (void)m; }
mDNSexport void     mDNSPlatformUnlock (const mDNS *const m) { (void)m; }
mDNSexport mDNSu32  mDNSPlatformStrLCopy(     void *dst, const void *src, mDNSu32 dstlen) { return (strlcpy((char *)dst, (const char *)src, dstlen)); }
mDNSexport mDNSu32  mDNSPlatformStrLen (                 const void *src)              { return(strlen((const char*)src)); }
mDNSexport void     mDNSPlatformMemCopy(      void *dst, const void *src, mDNSu32 len) { memcpy(dst, src, len); }
mDNSexport mDNSBool mDNSPlatformMemSame(const void *dst, const void *src, mDNSu32 len) { return(memcmp(dst, src, len) == 0); }
mDNSexport int      mDNSPlatformMemCmp(const void *dst, const void *src, mDNSu32 len) { return(memcmp(dst, src, len)); }
mDNSexport void     mDNSPlatformMemZero(      void *dst,                  mDNSu32 len) { memset(dst, 0, len); }
mDNSexport void     mDNSPlatformQsort  (      void *base, int nel, int width, int (*compar)(const void *, const void *))
{
    return (qsort(base, nel, width, compar));
}
#if !MDNS_MALLOC_DEBUGGING
mDNSexport void *mDNSPlatformMemAllocate(mDNSu32 len)      { return(mallocL("mDNSPlatformMemAllocate", len)); }
mDNSexport void *mDNSPlatformMemAllocateClear(mDNSu32 len) { return(callocL("mDNSPlatformMemAllocateClear", len)); }
mDNSexport void  mDNSPlatformMemFree    (void *mem)                 { freeL("mDNSPlatformMemFree", mem); }
#endif

mDNSexport void mDNSPlatformSetAllowSleep(mDNSBool allowSleep, const char *reason)
{
    mDNS *const m = &mDNSStorage;
    if (allowSleep && m->p->IOPMAssertion)
    {
        LogInfo("%s Destroying NoIdleSleep power assertion", __FUNCTION__);
        IOPMAssertionRelease(m->p->IOPMAssertion);
        m->p->IOPMAssertion = 0;
    }
    else if (!allowSleep)
    {
#ifdef kIOPMAssertionTypeNoIdleSleep
        if (m->p->IOPMAssertion)
        {
            IOPMAssertionRelease(m->p->IOPMAssertion);
            m->p->IOPMAssertion = 0;
        }

        CFStringRef assertionName = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%s.%d %s"), getprogname(), getpid(), reason ? reason : "");
        IOPMAssertionCreateWithName(kIOPMAssertionTypeNoIdleSleep, kIOPMAssertionLevelOn, assertionName ? assertionName : CFSTR("mDNSResponder"), &m->p->IOPMAssertion);
        if (assertionName) CFRelease(assertionName);
        LogInfo("%s Creating NoIdleSleep power assertion", __FUNCTION__);
#endif
    }
}

mDNSexport void mDNSPlatformPreventSleep(mDNSu32 timeout, const char *reason)
{
    mDNS *const m = &mDNSStorage;
    if (m->p->IOPMAssertion)
    {
        LogSPS("Sleep Assertion is already being held. Will not attempt to get it again for %d seconds for %s", timeout, reason);
        return;
    }
#ifdef kIOPMAssertionTypeNoIdleSleep

#if TARGET_OS_IPHONE
    if (!IsAppleTV())
        return; // No need for maintenance wakes on non-AppleTV embedded devices.
#endif

    double timeoutVal = (double)timeout;
    CFStringRef str = CFStringCreateWithCString(NULL, reason, kCFStringEncodingUTF8);
    CFNumberRef Timeout_num = CFNumberCreate(NULL, kCFNumberDoubleType, &timeoutVal);
    CFMutableDictionaryRef assertionProperties = CFDictionaryCreateMutable(NULL, 0,
                                                                           &kCFTypeDictionaryKeyCallBacks,
                                                                           &kCFTypeDictionaryValueCallBacks);
    if (IsAppleTV())
        CFDictionarySetValue(assertionProperties, kIOPMAssertionTypeKey, kIOPMAssertPreventUserIdleSystemSleep);
    else
        CFDictionarySetValue(assertionProperties, kIOPMAssertionTypeKey, kIOPMAssertMaintenanceActivity);

    CFDictionarySetValue(assertionProperties, kIOPMAssertionTimeoutKey, Timeout_num);
    CFDictionarySetValue(assertionProperties, kIOPMAssertionNameKey,    str);

    IOPMAssertionCreateWithProperties(assertionProperties, (IOPMAssertionID *)&m->p->IOPMAssertion);
    CFRelease(str);
    CFRelease(Timeout_num);
    CFRelease(assertionProperties);
    LogSPS("Got an idle sleep assertion for %d seconds for %s", timeout, reason);
#endif
}

mDNSexport void mDNSPlatformSendWakeupPacket(mDNSInterfaceID InterfaceID, char *EthAddr, char *IPAddr, int iteration)
{
    mDNSu32 ifindex;

    // Sanity check
    ifindex = mDNSPlatformInterfaceIndexfromInterfaceID(&mDNSStorage, InterfaceID, mDNStrue);
    if (ifindex <= 0)
    {
        LogMsg("mDNSPlatformSendWakeupPacket: ERROR!! Invalid InterfaceID %u", ifindex);
        return;
    }
    mDNSSendWakeupPacket(ifindex, EthAddr, IPAddr, iteration);
}

mDNSexport mDNSBool mDNSPlatformInterfaceIsD2D(mDNSInterfaceID InterfaceID)
{
    NetworkInterfaceInfoOSX *info;

    if (InterfaceID == mDNSInterface_P2P)
        return mDNStrue;

    // mDNSInterface_BLE not considered a D2D interface for the purpose of this
    // routine, since it's not implemented via a D2D plugin.
    if (InterfaceID == mDNSInterface_BLE)
        return mDNSfalse;

    if (   (InterfaceID == mDNSInterface_Any) 
        || (InterfaceID == mDNSInterfaceMark)
        || (InterfaceID == mDNSInterface_LocalOnly))
        return mDNSfalse;

    // Compare to cached AWDL interface ID.
    if (AWDLInterfaceID && (InterfaceID == AWDLInterfaceID))
        return mDNStrue;

    info = IfindexToInterfaceInfoOSX(InterfaceID);
    if (info == NULL)
    {
        // this log message can print when operations are stopped on an interface that has gone away
        LogInfo("mDNSPlatformInterfaceIsD2D: Invalid interface index %d", InterfaceID);
        return mDNSfalse;
    }

    return (mDNSBool) info->D2DInterface;
}

#if MDNSRESPONDER_SUPPORTS(APPLE, RANDOM_AWDL_HOSTNAME)
mDNSexport mDNSBool mDNSPlatformInterfaceIsAWDL(const mDNSInterfaceID interfaceID)
{
    return ((AWDLInterfaceID && (interfaceID == AWDLInterfaceID)) ? mDNStrue : mDNSfalse);
}
#endif

// Filter records send over P2P (D2D) type interfaces
// Note that the terms P2P and D2D are used synonymously in the current code and comments.
mDNSexport mDNSBool mDNSPlatformValidRecordForInterface(const AuthRecord *rr, mDNSInterfaceID InterfaceID)
{
    // For an explicit match to a valid interface ID, return true. 
    if (rr->resrec.InterfaceID == InterfaceID)
        return mDNStrue;

    // Only filtering records for D2D type interfaces, return true for all other interface types.
    if (!mDNSPlatformInterfaceIsD2D(InterfaceID))
        return mDNStrue;
    
    // If it's an AWDL interface the record must be explicitly marked to include AWDL.
    if (InterfaceID == AWDLInterfaceID)
    {
        if (rr->ARType == AuthRecordAnyIncludeAWDL || rr->ARType == AuthRecordAnyIncludeAWDLandP2P)
            return mDNStrue;
        else
            return mDNSfalse;
    }
    
    // Send record if it is explicitly marked to include all other P2P type interfaces.
    if (rr->ARType == AuthRecordAnyIncludeP2P || rr->ARType == AuthRecordAnyIncludeAWDLandP2P)
        return mDNStrue;

    // Don't send the record over this interface.
    return mDNSfalse;
}

// Filter questions send over P2P (D2D) type interfaces.
mDNSexport mDNSBool mDNSPlatformValidQuestionForInterface(DNSQuestion *q, const NetworkInterfaceInfo *intf)
{
    // For an explicit match to a valid interface ID, return true. 
    if (q->InterfaceID == intf->InterfaceID)
        return mDNStrue;

    // Only filtering questions for D2D type interfaces
    if (!mDNSPlatformInterfaceIsD2D(intf->InterfaceID))
        return mDNStrue;

    // If it's an AWDL interface the question must be explicitly marked to include AWDL.
    if (intf->InterfaceID == AWDLInterfaceID)
    {
        if (q->flags & kDNSServiceFlagsIncludeAWDL)
            return mDNStrue;
        else
            return mDNSfalse;
    }
    
    // Sent question if it is explicitly marked to include all other P2P type interfaces.
    if (q->flags & kDNSServiceFlagsIncludeP2P)
        return mDNStrue;

    // Don't send the question over this interface.
    return mDNSfalse;
}

// Returns true unless record was received over the AWDL interface and
// the question was not specific to the AWDL interface or did not specify kDNSServiceInterfaceIndexAny
// with the kDNSServiceFlagsIncludeAWDL flag set.
mDNSexport mDNSBool   mDNSPlatformValidRecordForQuestion(const ResourceRecord *const rr, const DNSQuestion *const q)
{
    if (!rr->InterfaceID || (rr->InterfaceID == q->InterfaceID))
        return mDNStrue;

    if ((rr->InterfaceID == AWDLInterfaceID) && !(q->flags & kDNSServiceFlagsIncludeAWDL))
        return mDNSfalse;

    return mDNStrue;
}

// formating time to RFC 4034 format
mDNSexport void mDNSPlatformFormatTime(unsigned long te, mDNSu8 *buf, int bufsize)
{
    struct tm tmTime;
    time_t t = (time_t)te;
    // Time since epoch : strftime takes "tm". Convert seconds to "tm" using
    // gmtime_r first and then use strftime
    gmtime_r(&t, &tmTime);
    strftime((char *)buf, bufsize, "%Y%m%d%H%M%S", &tmTime);
}

mDNSexport mDNSs32 mDNSPlatformGetPID()
{
    return getpid();
}

// Schedule a function asynchronously on the main queue
mDNSexport void mDNSPlatformDispatchAsync(mDNS *const m, void *context, AsyncDispatchFunc func)
{
    // KQueueLock/Unlock is used for two purposes
    //
    // 1. We can't be running along with the KQueue thread and hence acquiring the lock
    //    serializes the access to the "core"
    //
    // 2. KQueueUnlock also sends a message wake up the KQueue thread which in turn wakes
    //    up and calls udsserver_idle which schedules the messages across the uds socket.
    //    If "func" delivers something to the uds socket from the dispatch thread, it will
    //    not be delivered immediately if not for the Unlock.
    dispatch_async(dispatch_get_main_queue(), ^{
        KQueueLock();
        func(m, context);
        KQueueUnlock("mDNSPlatformDispatchAsync");
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
        // KQueueUnlock is a noop. Hence, we need to run kick off the idle loop
        // to handle any message that "func" might deliver.
        TriggerEventCompletion();
#endif
    });
}

// definitions for device-info record construction
#define DEVINFO_MODEL       "model="
#define DEVINFO_MODEL_LEN   sizeof_string(DEVINFO_MODEL)

#define OSX_VER         "osxvers="
#define OSX_VER_LEN     sizeof_string(OSX_VER) 
#define VER_NUM_LEN     2  // 2 digits of version number added to base string

#define MODEL_COLOR           "ecolor="
#define MODEL_COLOR_LEN       sizeof_string(MODEL_COLOR)
#define MODEL_RGB_VALUE_LEN   sizeof_string("255,255,255") // 'r,g,b'

// Bytes available in TXT record for model name after subtracting space for other
// fixed size strings and their length bytes.
#define MAX_MODEL_NAME_LEN   (256 - (DEVINFO_MODEL_LEN + 1) - (OSX_VER_LEN + VER_NUM_LEN + 1) - (MODEL_COLOR_LEN + MODEL_RGB_VALUE_LEN + 1))

mDNSlocal mDNSu8 getModelIconColors(char *color)
{
    mDNSPlatformMemZero(color, MODEL_RGB_VALUE_LEN + 1);

#if TARGET_OS_OSX && defined(kIOPlatformDeviceEnclosureColorKey)
    mDNSu8   red      = 0;
    mDNSu8   green    = 0;
    mDNSu8   blue     = 0;

    IOReturn rGetDeviceColor = IOPlatformGetDeviceColor(kIOPlatformDeviceEnclosureColorKey,
                                                        &red, &green, &blue);
    if (kIOReturnSuccess == rGetDeviceColor)
    {
        // IOKit was able to get enclosure color for the current device.
        return snprintf(color, MODEL_RGB_VALUE_LEN + 1, "%d,%d,%d", red, green, blue);
    }
#endif

    return 0;
}


// Initialize device-info TXT record contents and return total length of record data.
mDNSexport mDNSu32 initializeDeviceInfoTXT(mDNS *m, mDNSu8 *ptr)
{
    mDNSu8 *bufferStart = ptr;
    mDNSu8 len = m->HIHardware.c[0] < MAX_MODEL_NAME_LEN ? m->HIHardware.c[0] : MAX_MODEL_NAME_LEN;

    *ptr = DEVINFO_MODEL_LEN + len; // total length of DEVINFO_MODEL string plus the hardware name string
    ptr++;
    mDNSPlatformMemCopy(ptr, DEVINFO_MODEL, DEVINFO_MODEL_LEN);
    ptr += DEVINFO_MODEL_LEN;
    mDNSPlatformMemCopy(ptr, m->HIHardware.c + 1, len);
    ptr += len;

    // only include this string for OSX
    if (OSXVers)
    {
        char    ver_num[VER_NUM_LEN + 1]; // version digits + null written by snprintf
        *ptr = OSX_VER_LEN + VER_NUM_LEN; // length byte
        ptr++;
        mDNSPlatformMemCopy(ptr, OSX_VER, OSX_VER_LEN);
        ptr += OSX_VER_LEN;
        // convert version number to ASCII, add 1 for terminating null byte written by snprintf()
        // WARNING: This code assumes that OSXVers is always exactly two digits
        snprintf(ver_num, VER_NUM_LEN + 1, "%d", OSXVers);
        mDNSPlatformMemCopy(ptr, ver_num, VER_NUM_LEN);
        ptr += VER_NUM_LEN;

        char rgb[MODEL_RGB_VALUE_LEN + 1]; // RGB value + null written by snprintf
        len = getModelIconColors(rgb);
        if (len)
        {
            *ptr = MODEL_COLOR_LEN + len; // length byte
            ptr++;

            mDNSPlatformMemCopy(ptr, MODEL_COLOR, MODEL_COLOR_LEN);
            ptr += MODEL_COLOR_LEN;

            mDNSPlatformMemCopy(ptr, rgb, len);
            ptr += len;
        }
    }

    return (ptr - bufferStart);
}

#if APPLE_OSX_mDNSResponder // Don't compile for dnsextd target

// Use the scalar version of SameDomainLabel() by default
mDNSlocal mDNSBool scalarSameDomainLabel(const mDNSu8 *a, const mDNSu8 *b);
mDNSlocal mDNSBool vectorSameDomainLabel(const mDNSu8 *a, const mDNSu8 *b);
mDNSlocal mDNSBool (*SameDomainLabelPointer)(const mDNSu8 *a, const mDNSu8 *b) = scalarSameDomainLabel;

#include <System/machine/cpu_capabilities.h>
#define _cpu_capabilities   ((uint32_t*) _COMM_PAGE_CPU_CAPABILITIES)[0]

#if TARGET_OS_IPHONE
#include <arm_neon.h>

// Cache line aligned table that returns 32 for the upper case letters.
// This will take up 4 cache lines.
static const __attribute__ ((aligned(64))) uint8_t upper_to_lower_case_table[256] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Neon version
mDNSlocal mDNSBool vectorSameDomainLabel(const mDNSu8 *a, const mDNSu8 *b)
{
    const int len = *a++;
    
    if (len > MAX_DOMAIN_LABEL)
    {
        fprintf(stderr, "v: Malformed label (too long)\n");
        return(mDNSfalse);
    }
    
    if (len != *b++)
    {
        return(mDNSfalse);
    }
    
    uint32_t len_count = len;
    
    uint8x16_t vA, vB, vARotated, vBRotated, vMaskA, vMaskB;
    
    uint8x16_t v32 = vdupq_n_u8(32);
    uint8x16_t v37 = vdupq_n_u8(37);
    uint8x16_t v101 = vdupq_n_u8(101);
#if !defined __arm64__
    uint32x4_t vtemp32;
    uint32x2_t vtemp32d;
    uint32_t sum;
#endif
    
    while(len_count > 15)
    {
        vA = vld1q_u8(a);
        vB = vld1q_u8(b);
        a += 16;
        b += 16;
        
        //Make vA to lowercase if there is any uppercase.
        vARotated = vaddq_u8(vA, v37);            //Map 'A' ~ 'Z' from '65' ~ '90' to '102' ~ '127'.
        vMaskA    = vcgtq_s8(vARotated, v101);    //Check if anything is greater than '101' which means we have uppercase letters.
        vMaskA    = vandq_u8(vMaskA, v32);        //Prepare 32 for the elements with uppercase letters.
        vA        = vaddq_u8(vA, vMaskA);         //Add 32 only to the uppercase letters to make them lowercase letters.
        
        //Make vB to lowercase if there is any uppercase.
        vBRotated = vaddq_u8(vB, v37);            //Map 'A' ~ 'Z' from '65' ~ '90' to '102' ~ '127'.
        vMaskB    = vcgtq_s8(vBRotated, v101);    //Check if anything is greater than '101' which means we have uppercase letters.
        vMaskB    = vandq_u8(vMaskB, v32);        //Prepare 32 for the elements with uppercase letters.
        vB        = vaddq_u8(vB, vMaskB);         //Add 32 only to the uppercase letters to make them lowercase letters.
        
        //Compare vA & vB
        vA = vceqq_u8(vA, vB);
        
#if defined __arm64__
        //View 8-bit element as 32-bit => a3 a2 a1 a0
        //If min of 4 32-bit values in vA is 0xffffffff, then it means we have 0xff for all 16.
        if(vminvq_u32(vA) != 0xffffffffU)
        {
            return(mDNSfalse);
            
        }
#else
        //See if any element was not same.
        //View 8-bit element as 16-bit => a7 a6 a5 a4  a3 a2 a1 a0
        //(a7+a6) (a5+a4) (a3+a2) (a1+a0) => Each will be 0xffff + 0xffff = 0x0001fffe when all same.
        vtemp32  = vpaddlq_u16(vA);
        vtemp32d = vpadd_u32(vget_low_u32(vtemp32), vget_high_u32(vtemp32));
        vtemp32d = vpadd_u32(vtemp32d, vtemp32d);
        sum      = vget_lane_u32(vtemp32d, 0);
        
        //0x0001fffe + 0x0001fffe + 0x0001fffe + 0x0001fffe = 0x0007fff8U when all same.
        if(sum != 0x0007fff8U)
        {
            return(mDNSfalse);
        }
#endif
        
        len_count -= 16;
    }
    
    uint8x8_t vAd, vBd, vARotatedd, vBRotatedd, vMaskAd, vMaskBd;
    
    uint8x8_t v32d = vdup_n_u8(32);
    uint8x8_t v37d = vdup_n_u8(37);
    uint8x8_t v101d = vdup_n_u8(101);
    
    while(len_count > 7)
    {
        vAd = vld1_u8(a);
        vBd = vld1_u8(b);
        a += 8;
        b += 8;
        
        //Make vA to lowercase if there is any uppercase.
        vARotatedd = vadd_u8(vAd, v37d);            //Map 'A' ~ 'Z' from '65' ~ '90' to '102' ~ '127'.
        vMaskAd    = vcgt_s8(vARotatedd, v101d);    //Check if anything is greater than '101' which means we have uppercase letters.
        vMaskAd    = vand_u8(vMaskAd, v32d);        //Prepare 32 for the elements with uppercase letters.
        vAd        = vadd_u8(vAd, vMaskAd);         //Add 32 only to the uppercase letters to make them lowercase letters.
        
        //Make vB to lowercase if there is any uppercase.
        vBRotatedd = vadd_u8(vBd, v37d);            //Map 'A' ~ 'Z' from '65' ~ '90' to '102' ~ '127'.
        vMaskBd    = vcgt_s8(vBRotatedd, v101d);    //Check if anything is greater than '101' which means we have uppercase letters.
        vMaskBd    = vand_u8(vMaskBd, v32d);        //Prepare 32 for the elements with uppercase letters.
        vBd        = vadd_u8(vBd, vMaskBd);         //Add 32 only to the uppercase letters to make them lowercase letters.
        
        //Compare vA & vB
        vAd = vceq_u8(vAd, vBd);
        
#if defined __arm64__
        //View 8-bit element as 32-bit => a1 a0
        //If min of 2 32-bit values in vAd is 0xffffffff, then it means we have 0xff for all 16.
        if(vminv_u32(vAd) != 0xffffffffU)
        {
            return(mDNSfalse);
            
        }
#else
        //See if any element was not same.
        //View 8-bit element as 16-bit => a3 a2 a1 a0
        //(a3+a2) (a1+a0) => Each will be 0xffff + 0xffff = 0x0001fffe when all same.
        vtemp32d = vpaddl_u16(vAd);
        vtemp32d = vpadd_u32(vtemp32d, vtemp32d);
        sum      = vget_lane_u32(vtemp32d, 0);
        
        //0x0001fffe + 0x0001fffe = 0x0003fffc when all same.
        if(sum != 0x0003fffcU)
        {
            return(mDNSfalse);
        }
#endif
        
        len_count -= 8;
    }
    
    while(len_count > 0)
    {
        mDNSu8 ac = *a++;
        mDNSu8 bc = *b++;
        
        ac += upper_to_lower_case_table[ac];
        bc += upper_to_lower_case_table[bc];
        
        if (ac != bc)
        {
            return(mDNSfalse);
        }
        
        len_count -= 1;
    }
    return(mDNStrue);
}

// Use vectorized implementation if it is supported on this platform.
mDNSlocal void setSameDomainLabelPointer(void)
{
    if(_cpu_capabilities & kHasNeon)
    {
        // Use Neon Code
        SameDomainLabelPointer = vectorSameDomainLabel;
        LogMsg("setSameDomainLabelPointer: using vector code");
    }
    else
        LogMsg("setSameDomainLabelPointer: using scalar code");
}
#endif // TARGET_OS_IPHONE

#if TARGET_OS_OSX
#include <smmintrin.h>

// Cache line aligned table that returns 32 for the upper case letters.
// This will take up 4 cache lines.
static const __attribute__ ((aligned(64))) uint8_t upper_to_lower_case_table[256] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// SSE2 version
mDNSlocal mDNSBool vectorSameDomainLabel(const mDNSu8 *a, const mDNSu8 *b)
{
    const int len = *a++;
    
    if (len > MAX_DOMAIN_LABEL)
    {
        fprintf(stderr, "v: Malformed label (too long)\n");
        return(mDNSfalse);
    }
    
    if (len != *b++)
    {
        return(mDNSfalse);
    }
    
    uint32_t len_count = len;
    
    static const __attribute__ ((aligned(16))) unsigned char c_32[16] = { 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32 };
    static const __attribute__ ((aligned(16))) unsigned char c_37[16] = { 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37 };
    static const __attribute__ ((aligned(16))) unsigned char c_101[16] = { 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101, 101 };
    __m128i v37  = _mm_load_si128((__m128i*)c_37);
    __m128i v101 = _mm_load_si128((__m128i*)c_101);
    __m128i v32  = _mm_load_si128((__m128i*)c_32);
    
    uint32_t is_equal;
    __m128i vA, vB, vARotated, vBRotated, vMaskA, vMaskB;
    
    //AVX code that uses higher bandwidth (more elements per vector) was removed
    //to speed up the processing on the small sizes.
    //When I had them, the performance of 1 ~ 8 characters were slower by about 10% ~ 30%.
    while(len_count > 15)
    {
        vA = _mm_loadu_si128((__m128i*)a);
        vB = _mm_loadu_si128((__m128i*)b);
        a += 16;
        b += 16;
        
        //Make vA to lowercase if there is any uppercase.
        vARotated = _mm_add_epi8(vA, v37);              //Map 'A' ~ 'Z' from '65' ~ '90' to '102' ~ '127'.
        vMaskA    = _mm_cmpgt_epi8(vARotated, v101);    //Check if anything is greater than '101' which means we have uppercase letters.
        vMaskA    = _mm_and_si128(vMaskA, v32);         //Prepare 32 for the elements with uppercase letters.
        vA        = _mm_add_epi8(vA, vMaskA);           //Add 32 only to the uppercase letters to make them lowercase letters.
        
        //Make vB to lowercase if there is any uppercase.
        vBRotated = _mm_add_epi8(vB, v37);              //Map 'A' ~ 'Z' from '65' ~ '90' to '102' ~ '127'.
        vMaskB    = _mm_cmpgt_epi8(vBRotated, v101);    //Check if anything is greater than '101' which means we have uppercase letters.
        vMaskB    = _mm_and_si128(vMaskB, v32);         //Prepare 32 for the elements with uppercase letters.
        vB        = _mm_add_epi8(vB, vMaskB);           //Add 32 only to the uppercase letters to make them lowercase letters.
        
        //Compare vA & vB
        vA = _mm_cmpeq_epi8(vA, vB);
        
        //Return if any different.
        is_equal = _mm_movemask_epi8(vA);
        is_equal = is_equal & 0xffff;                 
        if(is_equal != 0xffff)
        {
            return(mDNSfalse);
        }
        
        len_count -= 16;
    }
    
    while(len_count > 0)
    {
        mDNSu8 ac = *a++;
        mDNSu8 bc = *b++;
        
        //Table will return 32 for upper case letters only.
        //0 will be returned for all others.
        ac += upper_to_lower_case_table[ac];
        bc += upper_to_lower_case_table[bc];
        
        //Return if a & b are different.
        if (ac != bc)
        {
            return(mDNSfalse);
        }
        
        len_count -= 1;
    }
    return(mDNStrue);
}

// Use vectorized implementation if it is supported on this platform.
mDNSlocal void setSameDomainLabelPointer(void)
{
    if(_cpu_capabilities & kHasSSE4_1)
    {
        // Use SSE Code
        SameDomainLabelPointer = vectorSameDomainLabel;
        LogMsg("setSameDomainLabelPointer: using vector code");
    }
    else
        LogMsg("setSameDomainLabelPointer: using scalar code");
}
#endif // TARGET_OS_OSX

// Original SameDomainLabel() implementation.
mDNSlocal mDNSBool scalarSameDomainLabel(const mDNSu8 *a, const mDNSu8 *b)
{
    int i;
    const int len = *a++;

    if (len > MAX_DOMAIN_LABEL)
    { debugf("Malformed label (too long)"); return(mDNSfalse); }

    if (len != *b++) return(mDNSfalse);
    for (i=0; i<len; i++)
    {
        mDNSu8 ac = *a++;
        mDNSu8 bc = *b++;
        if (mDNSIsUpperCase(ac)) ac += 'a' - 'A';
        if (mDNSIsUpperCase(bc)) bc += 'a' - 'A';
        if (ac != bc) return(mDNSfalse);
    }
    return(mDNStrue);
}

mDNSexport mDNSBool SameDomainLabel(const mDNSu8 *a, const mDNSu8 *b)
{
    return (*SameDomainLabelPointer)(a, b);
}

#endif // APPLE_OSX_mDNSResponder

#if MDNSRESPONDER_SUPPORTS(APPLE, RANDOM_AWDL_HOSTNAME)
mDNSexport void GetRandomUUIDLabel(domainlabel *label)
{
    uuid_t uuid;
    uuid_string_t uuidStr;
    uuid_generate_random(uuid);
    uuid_unparse_lower(uuid, uuidStr);
    MakeDomainLabelFromLiteralString(label, uuidStr);
}

mDNSexport void GetRandomUUIDLocalHostname(domainname *hostname)
{
    domainlabel uuidLabel;
    GetRandomUUIDLabel(&uuidLabel);
    hostname->c[0] = 0;
    AppendDomainLabel(hostname, &uuidLabel);
    AppendLiteralLabelString(hostname, "local");
}
#endif

#ifdef UNIT_TEST
#include "../unittests/mdns_macosx_ut.c"
#endif
