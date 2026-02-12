/* -*- Mode: C; tab-width: 4; c-file-style: "bsd"; c-basic-offset: 4; fill-column: 108; indent-tabs-mode: nil -*-
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

#include <mach/mach.h>
#include <mach/mach_error.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <paths.h>
#include <fcntl.h>
#include <launch.h>
#include <launch_priv.h>         // for launch_socket_service_check_in()
#include <pwd.h>
#include <sys/event.h>
#include <pthread.h>
#include <sandbox.h>
#include <SystemConfiguration/SCDynamicStoreCopyDHCPInfo.h>
#include <err.h>
#include <sysexits.h>
#include <TargetConditionals.h>

#include "uDNS.h"
#include "DNSCommon.h"
#include "mDNSMacOSX.h"             // Defines the specific types needed to run mDNS on this platform

#include "uds_daemon.h"             // Interface to the server side implementation of dns_sd.h
#include "xpc_services.h"
#include "xpc_service_dns_proxy.h"
#include "xpc_service_log_utility.h"
#include "helper.h"
#include "posix_utilities.h"        // for getLocalTimestamp()

#if MDNSRESPONDER_SUPPORTS(APPLE, METRICS)
#include "Metrics.h"
#endif

#if MDNSRESPONDER_SUPPORTS(APPLE, DNSSD_XPC_SERVICE)
#include "dnssd_server.h"
#endif

// Used on OSX(10.11.x onwards) for manipulating mDNSResponder program arguments
#if APPLE_OSX_mDNSResponder
// plist file to read the user's preferences
#define kProgramArguments CFSTR("com.apple.mDNSResponder")
// possible arguments for external customers
#define kPreferencesKey_DebugLogging              CFSTR("DebugLogging")
#define kPreferencesKey_UnicastPacketLogging      CFSTR("UnicastPacketLogging")
#define kPreferencesKey_AlwaysAppendSearchDomains CFSTR("AlwaysAppendSearchDomains")
#define kPreferencesKey_EnableAllowExpired        CFSTR("EnableAllowExpired")
#define kPreferencesKey_NoMulticastAdvertisements CFSTR("NoMulticastAdvertisements")
#define kPreferencesKey_StrictUnicastOrdering     CFSTR("StrictUnicastOrdering")
#define kPreferencesKey_OfferSleepProxyService    CFSTR("OfferSleepProxyService")
#define kPreferencesKey_UseInternalSleepProxy     CFSTR("UseInternalSleepProxy")

#if ENABLE_BLE_TRIGGERED_BONJOUR
#define kPreferencesKey_EnableBLEBasedDiscovery   CFSTR("EnableBLEBasedDiscovery")
#define kPreferencesKey_DefaultToBLETriggered     CFSTR("DefaultToBLETriggered")
#endif  // ENABLE_BLE_TRIGGERED_BONJOUR

#if MDNSRESPONDER_SUPPORTS(APPLE, PREALLOCATED_CACHE)
#define kPreferencesKey_PreallocateCacheMemory    CFSTR("PreallocateCacheMemory")
#endif
#endif

//*************************************************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark - Globals
#endif

static mDNS_PlatformSupport PlatformStorage;

// Start off with a default cache of 32K (136 records of 240 bytes each)
// Each time we grow the cache we add another 136 records
// 136 * 240 = 32640 bytes.
// This fits in eight 4kB pages, with 128 bytes spare for memory block headers and similar overhead
#define RR_CACHE_SIZE ((32*1024) / sizeof(CacheRecord))
static CacheEntity rrcachestorage[RR_CACHE_SIZE];
struct CompileTimeAssertionChecks_RR_CACHE_SIZE { char a[(RR_CACHE_SIZE >= 136) ? 1 : -1]; };
#define kRRCacheGrowSize (sizeof(CacheEntity) * RR_CACHE_SIZE)


#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
mDNSlocal void PrepareForIdle(void *m_param);
#else // MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
static mach_port_t signal_port       = MACH_PORT_NULL;
#endif // MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM

static dnssd_sock_t *launchd_fds = mDNSNULL;
static size_t launchd_fds_count = 0;

static mDNSBool NoMulticastAdvertisements = mDNSfalse; // By default, advertise addresses (& other records) via multicast

extern mDNSBool StrictUnicastOrdering;
extern mDNSBool AlwaysAppendSearchDomains;
extern mDNSBool EnableAllowExpired;
mDNSexport void INFOCallback(void);
mDNSexport void dump_state_to_fd(int fd);

#if ENABLE_BLE_TRIGGERED_BONJOUR
extern mDNSBool EnableBLEBasedDiscovery;
extern mDNSBool DefaultToBLETriggered;
#endif  // ENABLE_BLE_TRIGGERED_BONJOUR

#if MDNSRESPONDER_SUPPORTS(APPLE, PREALLOCATED_CACHE)
static mDNSBool PreallocateCacheMemory = mDNSfalse;
#endif

#if MDNSRESPONDER_SUPPORTS(APPLE, CACHE_MEM_LIMIT)
#define kRRCacheMemoryLimit 1000000 // For now, we limit the cache to at most 1MB on iOS devices.
#endif

// We keep a list of client-supplied event sources in KQSocketEventSource records
typedef struct KQSocketEventSource
{
    struct  KQSocketEventSource *next;
    int fd;
    KQueueEntry kqs;
    udsEventCallback callback;
    void *context;
} KQSocketEventSource;

static KQSocketEventSource *gEventSources;

//*************************************************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - General Utility Functions
#endif

#if MDNS_MALLOC_DEBUGGING
void mDNSPlatformValidateLists()
{
    mDNS *const m = &mDNSStorage;

    KQSocketEventSource *k;
    for (k = gEventSources; k; k=k->next)
        if (k->next == (KQSocketEventSource *)~0 || k->fd < 0)
            LogMemCorruption("gEventSources: %p is garbage (%d)", k, k->fd);

    // Check platform-layer lists
    NetworkInterfaceInfoOSX     *i;
    for (i = m->p->InterfaceList; i; i = i->next)
        if (i->next == (NetworkInterfaceInfoOSX *)~0 || !i->m || i->m == (mDNS *)~0)
            LogMemCorruption("m->p->InterfaceList: %p is garbage (%p)", i, i->ifinfo.ifname);

    ClientTunnel *t;
    for (t = m->TunnelClients; t; t=t->next)
        if (t->next == (ClientTunnel *)~0 || t->dstname.c[0] > 63)
            LogMemCorruption("m->TunnelClients: %p is garbage (%d)", t, t->dstname.c[0]);
}
#endif // MDNS_MALLOC_DEBUGGING

//*************************************************************************************************************
// Registration

mDNSexport void RecordUpdatedNiceLabel(mDNSs32 delay)
{
    mDNSStorage.p->NotifyUser = NonZeroTime(mDNSStorage.timenow + delay);
}

mDNSlocal void mDNSPreferencesSetNames(int key, domainlabel *old, domainlabel *new)
{
    mDNS *const m = &mDNSStorage;
    domainlabel *prevold, *prevnew;
    switch (key)
    {
    case kmDNSComputerName:
    case kmDNSLocalHostName:
        if (key == kmDNSComputerName)
        {
            prevold = &m->p->prevoldnicelabel;
            prevnew = &m->p->prevnewnicelabel;
        }
        else
        {
            prevold = &m->p->prevoldhostlabel;
            prevnew = &m->p->prevnewhostlabel;
        }
        // There are a few cases where we need to invoke the helper.
        //
        // 1. If the "old" label and "new" label are not same, it means there is a conflict. We need
        //    to invoke the helper so that it pops up a dialogue to inform the user about the
        //    conflict
        //
        // 2. If the "old" label and "new" label are same, it means the user has set the host/nice label
        //    through the preferences pane. We may have to inform the helper as it may have popped up
        //    a dialogue previously (due to a conflict) and it needs to suppress it now. We can avoid invoking
        //    the helper in this case if the previous values (old and new) that we told helper last time
        //    are same. If the previous old and new values are same, helper does not care.
        //
        // Note: "new" can be NULL when we have repeated conflicts and we are asking helper to give up. "old"
        // is not called with NULL today, but this makes it future proof.
        if (!old || !new || !SameDomainLabelCS(old->c, new->c) ||
            !SameDomainLabelCS(old->c, prevold->c) ||
            !SameDomainLabelCS(new->c, prevnew->c))
        {
// Work around bug radar:21397654
#ifndef __clang_analyzer__
            if (old)
                *prevold = *old;
            else
                prevold->c[0] = 0;
            if (new)
                *prevnew = *new;
            else
                prevnew->c[0] = 0;
#endif
            mDNSPreferencesSetName(key, old, new);
        }
        else
        {
            LogInfo("mDNSPreferencesSetNames not invoking helper %s %#s, %s %#s, old %#s, new %#s",
                    (key == kmDNSComputerName ? "prevoldnicelabel" : "prevoldhostlabel"), prevold->c,
                    (key == kmDNSComputerName ? "prevnewnicelabel" : "prevnewhostlabel"), prevnew->c,
                    old->c, new->c);
        }
        break;
    default:
        LogMsg("mDNSPreferencesSetNames: unrecognized key: %d", key);
        return;
    }
}

mDNSlocal void mDNS_StatusCallback(mDNS *const m, mStatus result)
{
    if (result == mStatus_NoError)
    {
        if (!SameDomainLabelCS(m->p->userhostlabel.c, m->hostlabel.c))
            LogInfo("Local Hostname changed from \"%#s.local\" to \"%#s.local\"", m->p->userhostlabel.c, m->hostlabel.c);
        // One second pause in case we get a Computer Name update too -- don't want to alert the user twice
        RecordUpdatedNiceLabel(mDNSPlatformOneSecond);
    }
    else if (result == mStatus_NameConflict)
    {
        LogInfo("Local Hostname conflict for \"%#s.local\"", m->hostlabel.c);
        if (!m->p->HostNameConflict) m->p->HostNameConflict = NonZeroTime(m->timenow);
        else if (m->timenow - m->p->HostNameConflict > 60 * mDNSPlatformOneSecond)
        {
            // Tell the helper we've given up
            mDNSPreferencesSetNames(kmDNSLocalHostName, &m->p->userhostlabel, NULL);
        }
    }
    else if (result == mStatus_GrowCache)
    {
        // Allocate another chunk of cache storage
        static unsigned int allocated = 0;
#if MDNSRESPONDER_SUPPORTS(APPLE, CACHE_MEM_LIMIT)
        if (allocated >= kRRCacheMemoryLimit) return;	// For now we limit the cache to at most 1MB on iOS devices
#endif
        allocated += kRRCacheGrowSize;
        // LogMsg("GrowCache %d * %d = %d; total so far %6u", sizeof(CacheEntity), RR_CACHE_SIZE, sizeof(CacheEntity) * RR_CACHE_SIZE, allocated);
        CacheEntity *storage = mallocL("mStatus_GrowCache", sizeof(CacheEntity) * RR_CACHE_SIZE);
        //LogInfo("GrowCache %d * %d = %d", sizeof(CacheEntity), RR_CACHE_SIZE, sizeof(CacheEntity) * RR_CACHE_SIZE);
        if (storage) mDNS_GrowCache(m, storage, RR_CACHE_SIZE);
    }
    else if (result == mStatus_ConfigChanged)
    {
        // Tell the helper we've seen a change in the labels.  It will dismiss the name conflict alert if needed.
        mDNSPreferencesSetNames(kmDNSComputerName, &m->p->usernicelabel, &m->nicelabel);
        mDNSPreferencesSetNames(kmDNSLocalHostName, &m->p->userhostlabel, &m->hostlabel);

        // Then we call into the UDS daemon code, to let it do the same
        udsserver_handle_configchange(m);
    }
}


//*************************************************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Startup, shutdown, and supporting code
#endif

mDNSlocal void ExitCallback(int sig)
{
    (void)sig; // Unused
    LogMsg("%s stopping", mDNSResponderVersionString);

    if (udsserver_exit() < 0) 
        LogMsg("ExitCallback: udsserver_exit failed");

    debugf("ExitCallback: mDNS_StartExit");
    mDNS_StartExit(&mDNSStorage);
}

#ifndef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM

// Send a mach_msg to ourselves (since that is signal safe) telling us to cleanup and exit
mDNSlocal void HandleSIG(int sig)
{
    kern_return_t status;
    mach_msg_header_t header;

    // WARNING: can't call syslog or fprintf from signal handler
    header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_MAKE_SEND, 0);
    header.msgh_remote_port = signal_port;
    header.msgh_local_port = MACH_PORT_NULL;
    header.msgh_size = sizeof(header);
    header.msgh_id = sig;

    status = mach_msg(&header, MACH_SEND_MSG | MACH_SEND_TIMEOUT, header.msgh_size,
                      0, MACH_PORT_NULL, 0, MACH_PORT_NULL);

    if (status != MACH_MSG_SUCCESS)
    {
        if (status == MACH_SEND_TIMED_OUT) mach_msg_destroy(&header);
        if (sig == SIGTERM || sig == SIGINT) exit(-1);
    }
}

#endif // MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM

mDNSexport void dump_state_to_fd(int fd)
{
    mDNS *const m = &mDNSStorage;
    char buffer[1024];
    buffer[0] = '\0';

    mDNSs32 utc = mDNSPlatformUTC();
    const mDNSs32 now = mDNS_TimeNow(&mDNSStorage);
    NetworkInterfaceInfoOSX     *i;
    DNSServer *s;
    McastResolver *mr;
    char timestamp[64]; // 64 is enough to store the UTC timestmp

    LogToFD(fd, "---- BEGIN STATE LOG ---- %s %s %d", mDNSResponderVersionString, OSXVers ? "OSXVers" : "iOSVers", OSXVers ? OSXVers : iOSVers);
    getLocalTimestamp(timestamp, sizeof(timestamp));
    LogToFD(fd, "Date: %s", timestamp);
    LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_DEFAULT, "---- BEGIN STATE LOG ---- (" PUB_S ")", timestamp);

    udsserver_info_dump_to_fd(fd);

    LogToFD(fd, "----- Platform Timers -----");
    LogTimerToFD(fd, "m->NextCacheCheck       ", mDNSStorage.NextCacheCheck);
    LogTimerToFD(fd, "m->NetworkChanged       ", mDNSStorage.NetworkChanged);
    LogTimerToFD(fd, "m->p->NotifyUser        ", mDNSStorage.p->NotifyUser);
    LogTimerToFD(fd, "m->p->HostNameConflict  ", mDNSStorage.p->HostNameConflict);
    LogTimerToFD(fd, "m->p->KeyChainTimer     ", mDNSStorage.p->KeyChainTimer);

    log_dnsproxy_info_to_fd(fd, &mDNSStorage);

    LogToFD(fd, "----- KQSocketEventSources -----");
    if (!gEventSources) LogToFD(fd, "<None>");
    else
    {
        KQSocketEventSource *k;
        for (k = gEventSources; k; k=k->next)
            LogToFD(fd, "%3d %s %s", k->fd, k->kqs.KQtask, k->fd == mDNSStorage.uds_listener_skt ? "Listener for incoming UDS clients" : " ");
    }

    LogToFD(fd, "------ Network Interfaces ------");
    if (!mDNSStorage.p->InterfaceList) LogToFD(fd, "<None>");
    else
    {
        LogToFD(fd, "  Struct addr           Registered                MAC               BSSID                                Interface Address");
        for (i = mDNSStorage.p->InterfaceList; i; i = i->next)
        {
            // Allow six characters for interface name, for names like "vmnet8"
            if (!i->Exists)
                LogToFD(fd, "%p %2ld, %p,  %s %-6s %.6a %.6a %#-14a dormant for %d seconds",
                          i, i->ifinfo.InterfaceID, i->Registered,
                          i->sa_family == AF_INET ? "v4" : i->sa_family == AF_INET6 ? "v6" : "??", i->ifinfo.ifname, &i->ifinfo.MAC, &i->BSSID,
                          &i->ifinfo.ip, utc - i->LastSeen);
            else
            {
                const CacheRecord *sps[3];
                FindSPSInCache(&mDNSStorage, &i->ifinfo.NetWakeBrowse, sps);
                LogToFD(fd, "%p %2ld, %p,  %s %-6s %.6a %.6a %s %s %s %s %s %s %#a",
                          i, i->ifinfo.InterfaceID, i->Registered,
                          i->sa_family == AF_INET ? "v4" : i->sa_family == AF_INET6 ? "v6" : "??", i->ifinfo.ifname, &i->ifinfo.MAC, &i->BSSID,
                          i->ifinfo.InterfaceActive ? "Active" : "      ",
                          i->ifinfo.IPv4Available ? "v4" : "  ",
                          i->ifinfo.IPv6Available ? "v6" : "  ",
                          i->ifinfo.Advertise ? "A" : " ",
                          i->ifinfo.McastTxRx ? "M" : " ",
                          !(i->ifinfo.InterfaceActive && i->ifinfo.NetWake) ? " " : !sps[0] ? "p" : "P",
                          &i->ifinfo.ip);

                // Only print the discovered sleep proxies once for the lead/active interface of an interface set.
                if (i == i->Registered && (sps[0] || sps[1] || sps[2]))
                {
                    LogToFD(fd, "         Sleep Proxy Metric   Name");
                    if (sps[0]) LogToFD(fd, "  %13d %#s", SPSMetric(sps[0]->resrec.rdata->u.name.c), sps[0]->resrec.rdata->u.name.c);
                    if (sps[1]) LogToFD(fd, "  %13d %#s", SPSMetric(sps[1]->resrec.rdata->u.name.c), sps[1]->resrec.rdata->u.name.c);
                    if (sps[2]) LogToFD(fd, "  %13d %#s", SPSMetric(sps[2]->resrec.rdata->u.name.c), sps[2]->resrec.rdata->u.name.c);
                }
            }
        }
    }

    LogToFD(fd, "--------- DNS Servers(%d) ----------", CountOfUnicastDNSServers(&mDNSStorage));
    if (!mDNSStorage.DNSServers) LogToFD(fd, "<None>");
    else
    {
        for (s = mDNSStorage.DNSServers; s; s = s->next)
        {
            NetworkInterfaceInfoOSX *ifx = IfindexToInterfaceInfoOSX(s->interface);
            LogToFD(fd, "DNS Server %##s %s%s%#a:%d %d %s %d %d %sv4 %sv6 %scell %sexp %sconstrained %sCLAT46 %sDNSSECAware",
                    s->domain.c, ifx ? ifx->ifinfo.ifname : "", ifx ? " " : "", &s->addr, mDNSVal16(s->port),
                    s->penaltyTime ? (s->penaltyTime - mDNS_TimeNow(&mDNSStorage)) : 0, DNSScopeToString(s->scopeType),
                    s->timeout, s->resGroupID,
                    s->usableA       ? "" : "!",
                    s->usableAAAA    ? "" : "!",
                    s->isCell        ? "" : "!",
                    s->isExpensive   ? "" : "!",
                    s->isConstrained ? "" : "!",
                    s->isCLAT46      ? "" : "!",
                    s->DNSSECAware   ? "" : "!");
        }
    }

    LogToFD(fd, "v4answers %d", mDNSStorage.p->v4answers);
    LogToFD(fd, "v6answers %d", mDNSStorage.p->v6answers);
    LogToFD(fd, "Last DNS Trigger: %d ms ago", (now - mDNSStorage.p->DNSTrigger));

    LogToFD(fd, "-------- Interface Monitors --------");
    const CFIndex n = m->p->InterfaceMonitors ? CFArrayGetCount(m->p->InterfaceMonitors) : 0;
    if (n > 0)
    {
        for (CFIndex j = 0; j < n; j++)
        {
            mdns_interface_monitor_t monitor = (mdns_interface_monitor_t) CFArrayGetValueAtIndex(m->p->InterfaceMonitors, j);
            char *description = mdns_copy_description(monitor);
            if (description)
            {
                LogToFD(fd, "%s", description);
                free(description);
            }
            else
            {
                LogToFD(fd, "monitor %p (no description)", monitor);
            }
        }
    }
    else
    {
        LogToFD(fd, "No interface monitors");
    }

    LogToFD(fd, "--------- Mcast Resolvers ----------");
    if (!mDNSStorage.McastResolvers) LogToFD(fd, "<None>");
    else
    {
        for (mr = mDNSStorage.McastResolvers; mr; mr = mr->next)
            LogToFD(fd, "Mcast Resolver %##s timeout %u", mr->domain.c, mr->timeout);
    }

    LogToFD(fd, "------------ Hostnames -------------");
    if (!mDNSStorage.Hostnames) LogToFD(fd, "<None>");
    else
    {
        HostnameInfo *hi;
        for (hi = mDNSStorage.Hostnames; hi; hi = hi->next)
        {
            LogToFD(fd, "%##s v4 %d %s", hi->fqdn.c, hi->arv4.state, ARDisplayString(&mDNSStorage, &hi->arv4));
            LogToFD(fd, "%##s v6 %d %s", hi->fqdn.c, hi->arv6.state, ARDisplayString(&mDNSStorage, &hi->arv6));
        }
    }

    LogToFD(fd, "--------------- FQDN ---------------");
    if (!mDNSStorage.FQDN.c[0]) LogToFD(fd, "<None>");
    else
    {
        LogToFD(fd, "%##s", mDNSStorage.FQDN.c);
    }

    #if MDNSRESPONDER_SUPPORTS(APPLE, METRICS)
        LogMetricsToFD(fd);
    #endif

//    getLocalTimestamp(timestamp, sizeof(timestamp));
    LogToFD(fd, "Date: %s", timestamp);
    LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_DEFAULT, "---- END STATE LOG ---- (" PUB_S ")", timestamp);
    LogToFD(fd, "----  END STATE LOG  ---- %s %s %d", mDNSResponderVersionString, OSXVers ? "OSXVers" : "iOSVers", OSXVers ? OSXVers : iOSVers);
}

mDNSexport void INFOCallback(void)
{
    LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_WARNING,
        "Sending SIGINFO to mDNSResponder daemon is deprecated. To trigger state dump, please use 'dns-sd -O', enter 'dns-sd -h' for more information");
}

// Writes the state out to the dynamic store and also affects the ASL filter level
mDNSexport void UpdateDebugState()
{
    mDNSu32 one  = 1;
    mDNSu32 zero = 0;

    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (!dict)
    {
        LogMsg("UpdateDebugState: Could not create dict");
        return;
    }

    CFNumberRef numOne = CFNumberCreate(NULL, kCFNumberSInt32Type, &one);
    if (numOne == NULL)
    {
        LogMsg("UpdateDebugState: Could not create CFNumber one");
        return;
    }
    CFNumberRef numZero = CFNumberCreate(NULL, kCFNumberSInt32Type, &zero);
    if (numZero == NULL)
    {
        LogMsg("UpdateDebugState: Could not create CFNumber zero");
        CFRelease(numOne);
        return;
    }

    if (mDNS_LoggingEnabled)
        CFDictionarySetValue(dict, CFSTR("VerboseLogging"), numOne);
    else
        CFDictionarySetValue(dict, CFSTR("VerboseLogging"), numZero);

    if (mDNS_PacketLoggingEnabled)
        CFDictionarySetValue(dict, CFSTR("PacketLogging"), numOne);
    else
        CFDictionarySetValue(dict, CFSTR("PacketLogging"), numZero);

    if (mDNS_McastLoggingEnabled)
        CFDictionarySetValue(dict, CFSTR("McastLogging"), numOne);
    else
        CFDictionarySetValue(dict, CFSTR("McastLogging"), numZero);

    if (mDNS_McastTracingEnabled)
        CFDictionarySetValue(dict, CFSTR("McastTracing"), numOne);
    else 
        CFDictionarySetValue(dict, CFSTR("McastTracing"), numZero);

    CFRelease(numOne);
    CFRelease(numZero);
    mDNSDynamicStoreSetConfig(kmDNSDebugState, mDNSNULL, dict);
    CFRelease(dict);

}


#ifndef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM

mDNSlocal void SignalCallback(CFMachPortRef port, void *msg, CFIndex size, void *info)
{
    (void)port;     // Unused
    (void)size;     // Unused
    (void)info;     // Unused
    mach_msg_header_t *msg_header = (mach_msg_header_t *)msg;
    mDNS *const m = &mDNSStorage;

    // We're running on the CFRunLoop (Mach port) thread, not the kqueue thread, so we need to grab the KQueueLock before proceeding
    KQueueLock();
    switch(msg_header->msgh_id)
    {
    case SIGHUP:    {
        mDNSu32 slot;
        CacheGroup *cg;
        CacheRecord *rr;
        LogMsg("SIGHUP: Purge cache");
        mDNS_Lock(m);
        FORALL_CACHERECORDS(slot, cg, rr)
        {
            rr->resrec.mortality = Mortality_Mortal;
            mDNS_PurgeCacheResourceRecord(m, rr);
        }
        // Restart unicast and multicast queries
        mDNSCoreRestartQueries(m);
        mDNS_Unlock(m);
    } break;
    case SIGINT:
    case SIGTERM:   ExitCallback(msg_header->msgh_id); break;
    case SIGINFO:   INFOCallback(); break;
    case SIGUSR1:
#if APPLE_OSX_mDNSResponder
        mDNS_LoggingEnabled = 1;
        LogMsg("SIGUSR1: Logging %s on Apple Platforms", mDNS_LoggingEnabled ? "Enabled" : "Disabled");
#else
        mDNS_LoggingEnabled = mDNS_LoggingEnabled ? 0 : 1;
        LogMsg("SIGUSR1: Logging %s", mDNS_LoggingEnabled ? "Enabled" : "Disabled");
#endif
        WatchDogReportingThreshold = mDNS_LoggingEnabled ? 50 : 250;
        UpdateDebugState();
        LogInfo("USR1 Logging Enabled"); 
        break;
    case SIGUSR2:
#if APPLE_OSX_mDNSResponder
        mDNS_PacketLoggingEnabled = 1;
        LogMsg("SIGUSR2: Packet Logging %s on Apple Platforms", mDNS_PacketLoggingEnabled ? "Enabled" : "Disabled");
#else
        mDNS_PacketLoggingEnabled = mDNS_PacketLoggingEnabled ? 0 : 1;
        LogMsg("SIGUSR2: Packet Logging %s", mDNS_PacketLoggingEnabled ? "Enabled" : "Disabled");
#endif
        mDNS_McastTracingEnabled = (mDNS_PacketLoggingEnabled && mDNS_McastLoggingEnabled) ? mDNStrue : mDNSfalse;
        LogInfo("SIGUSR2: Multicast Tracing is %s", mDNS_McastTracingEnabled ? "Enabled" : "Disabled");
        UpdateDebugState();
        break;
    case SIGPROF:  mDNS_McastLoggingEnabled = mDNS_McastLoggingEnabled ? mDNSfalse : mDNStrue;
        LogMsg("SIGPROF: Multicast Logging %s", mDNS_McastLoggingEnabled ? "Enabled" : "Disabled");
        LogMcastStateInfo(mDNSfalse, mDNStrue, mDNStrue);
        mDNS_McastTracingEnabled = (mDNS_PacketLoggingEnabled && mDNS_McastLoggingEnabled) ? mDNStrue : mDNSfalse;
        LogMsg("SIGPROF: Multicast Tracing is %s", mDNS_McastTracingEnabled ? "Enabled" : "Disabled");
        UpdateDebugState();
        break;
    case SIGTSTP:  mDNS_LoggingEnabled = mDNS_PacketLoggingEnabled = mDNS_McastLoggingEnabled = mDNS_McastTracingEnabled = mDNSfalse;
        LogMsg("All mDNSResponder Debug Logging/Tracing Disabled (USR1/USR2/PROF)");
        UpdateDebugState();
        break;

    default: LogMsg("SignalCallback: Unknown signal %d", msg_header->msgh_id); break;
    }
    KQueueUnlock("Unix Signal");
}

// MachServerName is com.apple.mDNSResponder (Supported only till 10.9.x)
mDNSlocal kern_return_t mDNSDaemonInitialize(void)
{
    mStatus err;

    err = mDNS_Init(&mDNSStorage, &PlatformStorage,
                    rrcachestorage, RR_CACHE_SIZE,
                    !NoMulticastAdvertisements,
                    mDNS_StatusCallback, mDNS_Init_NoInitCallbackContext);

    if (err)
    {
        LogMsg("Daemon start: mDNS_Init failed %d", err);
        return(err);
    }

#if MDNSRESPONDER_SUPPORTS(APPLE, PREALLOCATED_CACHE)
    if (PreallocateCacheMemory)
    {
        const int growCount = (kRRCacheMemoryLimit + kRRCacheGrowSize - 1) / kRRCacheGrowSize;
        int i;

        for (i = 0; i < growCount; ++i)
        {
            mDNS_StatusCallback(&mDNSStorage, mStatus_GrowCache);
        }
    }
#endif

    CFMachPortRef i_port = CFMachPortCreate(NULL, SignalCallback, NULL, NULL);
    CFRunLoopSourceRef i_rls  = CFMachPortCreateRunLoopSource(NULL, i_port, 0);
    signal_port       = CFMachPortGetPort(i_port);
    CFRunLoopAddSource(CFRunLoopGetMain(), i_rls, kCFRunLoopDefaultMode);
    CFRelease(i_rls);
    
    return(err);
}

#else // MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM

// SignalDispatch is mostly just a copy/paste of entire code block from SignalCallback above.
// The common code should be a subroutine, or we end up having to fix bugs in two places all the time.
// The same applies to mDNSDaemonInitialize, much of which is just a copy/paste of chunks
// of code from above. Alternatively we could remove the duplicated source code by having
// single routines, with the few differing parts bracketed with "#ifndef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM"

mDNSlocal void SignalDispatch(dispatch_source_t source)
{
    int sig = (int)dispatch_source_get_handle(source);
    mDNS *const m = &mDNSStorage;
    KQueueLock();
    switch(sig)
    {
    case SIGHUP:    {
        mDNSu32 slot;
        CacheGroup *cg;
        CacheRecord *rr;
        LogMsg("SIGHUP: Purge cache");
        mDNS_Lock(m);
        FORALL_CACHERECORDS(slot, cg, rr)
        {
           rr->resrec.mortality = Mortality_Mortal;
           mDNS_PurgeCacheResourceRecord(m, rr);
        }
        // Restart unicast and multicast queries
        mDNSCoreRestartQueries(m);
        mDNS_Unlock(m);
    } break;
    case SIGINT:
    case SIGTERM:   ExitCallback(sig); break;
    case SIGINFO:   INFOCallback(); break;
    case SIGUSR1:   mDNS_LoggingEnabled = mDNS_LoggingEnabled ? 0 : 1;
        LogMsg("SIGUSR1: Logging %s", mDNS_LoggingEnabled ? "Enabled" : "Disabled");
        WatchDogReportingThreshold = mDNS_LoggingEnabled ? 50 : 250;
        UpdateDebugState();
        break;
    case SIGUSR2:   mDNS_PacketLoggingEnabled = mDNS_PacketLoggingEnabled ? 0 : 1;
        LogMsg("SIGUSR2: Packet Logging %s", mDNS_PacketLoggingEnabled ? "Enabled" : "Disabled");
        UpdateDebugState();
        break;
    default: LogMsg("SignalCallback: Unknown signal %d", sig); break;
    }
    KQueueUnlock("Unix Signal");
}

mDNSlocal void mDNSSetupSignal(dispatch_queue_t queue, int sig)
{
    signal(sig, SIG_IGN);
    dispatch_source_t source = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL, sig, 0, queue);

    if (source)
    {
        dispatch_source_set_event_handler(source, ^{SignalDispatch(source);});
        // Start processing signals
        dispatch_resume(source);
    }
    else
    {
        LogMsg("mDNSSetupSignal: Cannot setup signal %d", sig);
    }
}

mDNSlocal kern_return_t mDNSDaemonInitialize(void)
{
    mStatus err;
    dispatch_queue_t queue = dispatch_get_main_queue();

    err = mDNS_Init(&mDNSStorage, &PlatformStorage,
                    rrcachestorage, RR_CACHE_SIZE,
                    !NoMulticastAdvertisements,
                    mDNS_StatusCallback, mDNS_Init_NoInitCallbackContext);

    if (err)
    {
        LogMsg("Daemon start: mDNS_Init failed %d", err);
        return(err);
    }

    mDNSSetupSignal(queue, SIGHUP);
    mDNSSetupSignal(queue, SIGINT);
    mDNSSetupSignal(queue, SIGTERM);
    mDNSSetupSignal(queue, SIGINFO);
    mDNSSetupSignal(queue, SIGUSR1);
    mDNSSetupSignal(queue, SIGUSR2);

    // Create a custom handler for doing the housekeeping work. This is either triggered
    // by the timer or an event source
    PlatformStorage.custom = dispatch_source_create(DISPATCH_SOURCE_TYPE_DATA_ADD, 0, 0, queue);
    if (PlatformStorage.custom == mDNSNULL) {LogMsg("mDNSDaemonInitialize: Error creating custom source"); return -1;}
    dispatch_source_set_event_handler(PlatformStorage.custom, ^{PrepareForIdle(&mDNSStorage);});
    dispatch_resume(PlatformStorage.custom);

    // Create a timer source to trigger housekeeping work. The houskeeping work itself
    // is done in the custom handler that we set below.

    PlatformStorage.timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);
    if (PlatformStorage.timer == mDNSNULL) {LogMsg("mDNSDaemonInitialize: Error creating timer source"); return -1;}

    // As the API does not support one shot timers, we pass zero for the interval. In the custom handler, we
    // always reset the time to the new time computed. In effect, we ignore the interval
    dispatch_source_set_timer(PlatformStorage.timer, DISPATCH_TIME_NOW, 1000ull * 1000000000, 0);
    dispatch_source_set_event_handler(PlatformStorage.timer, ^{
                                          dispatch_source_merge_data(PlatformStorage.custom, 1);
                                      });
    dispatch_resume(PlatformStorage.timer);

    LogMsg("DaemonIntialize done successfully");

    return(err);
}

#endif // MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM

mDNSlocal mDNSs32 mDNSDaemonIdle(mDNS *const m)
{
    mDNSs32 now = mDNS_TimeNow(m);

    // 1. If we need to set domain secrets, do so before handling the network change
    // Detailed reason:
    // BTMM domains listed in DynStore Setup:/Network/BackToMyMac are added to the registration domains list,
    // and we need to setup the associated AutoTunnel DomainAuthInfo entries before that happens.
    if (m->p->KeyChainTimer && now - m->p->KeyChainTimer >= 0)
    {
        m->p->KeyChainTimer = 0;
        mDNS_Lock(m);
        SetDomainSecrets(m);
        mDNS_Unlock(m);
    }

    // 2. If we have network change events to handle, do them before calling mDNS_Execute()
    // Detailed reason:
    // mDNSMacOSXNetworkChanged() currently closes and re-opens its sockets. If there are received packets waiting, they are lost.
    // mDNS_Execute() generates packets, including multicasts that are looped back to ourself.
    // If we call mDNS_Execute() first, and generate packets, and then call mDNSMacOSXNetworkChanged() immediately afterwards
    // we then systematically lose our own looped-back packets.
    if (m->NetworkChanged && now - m->NetworkChanged >= 0) mDNSMacOSXNetworkChanged();

    if (m->p->RequestReSleep && now - m->p->RequestReSleep >= 0)
    {
        m->p->RequestReSleep = 0;
        mDNSPowerRequest(0, 0);
    }

    // 3. Call mDNS_Execute() to let mDNSCore do what it needs to do
    mDNSs32 nextevent = mDNS_Execute(m);

    if (m->NetworkChanged)
        if (nextevent - m->NetworkChanged > 0)
            nextevent = m->NetworkChanged;

    if (m->p->KeyChainTimer)
        if (nextevent - m->p->KeyChainTimer > 0)
            nextevent = m->p->KeyChainTimer;

    if (m->p->RequestReSleep)
        if (nextevent - m->p->RequestReSleep > 0)
            nextevent = m->p->RequestReSleep;

    
    if (m->p->NotifyUser)
    {
        if (m->p->NotifyUser - now < 0)
        {
            if (!SameDomainLabelCS(m->p->usernicelabel.c, m->nicelabel.c))
            {
                LogMsg("Name Conflict: Updated Computer Name from \"%#s\" to \"%#s\"", m->p->usernicelabel.c, m->nicelabel.c);
                mDNSPreferencesSetNames(kmDNSComputerName, &m->p->usernicelabel, &m->nicelabel);
                m->p->usernicelabel = m->nicelabel;
            }
            if (!SameDomainLabelCS(m->p->userhostlabel.c, m->hostlabel.c))
            {
                LogMsg("Name Conflict: Updated Local Hostname from \"%#s.local\" to \"%#s.local\"", m->p->userhostlabel.c, m->hostlabel.c);
                mDNSPreferencesSetNames(kmDNSLocalHostName, &m->p->userhostlabel, &m->hostlabel);
                m->p->HostNameConflict = 0; // Clear our indicator, now name change has been successful
                m->p->userhostlabel = m->hostlabel;
            }
            m->p->NotifyUser = 0;
        }
        else
        if (nextevent - m->p->NotifyUser > 0)
            nextevent = m->p->NotifyUser;
    }

    return(nextevent);
}

// Right now we consider *ALL* of our DHCP leases
// It might make sense to be a bit more selective and only consider the leases on interfaces
// (a) that are capable and enabled for wake-on-LAN, and
// (b) where we have found (and successfully registered with) a Sleep Proxy
// If we can't be woken for traffic on a given interface, then why keep waking to renew its lease?
mDNSlocal mDNSu32 DHCPWakeTime(void)
{
    mDNSu32 e = 24 * 3600;      // Maximum maintenance wake interval is 24 hours
    const CFAbsoluteTime now = CFAbsoluteTimeGetCurrent();
    if (!now) LogMsg("DHCPWakeTime: CFAbsoluteTimeGetCurrent failed");
    else
    {
        int ic, j;

        const void *pattern = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL, kSCDynamicStoreDomainState, kSCCompAnyRegex, kSCEntNetDHCP);
        if (!pattern)
        {
            LogMsg("DHCPWakeTime: SCDynamicStoreKeyCreateNetworkServiceEntity failed\n");
            return e;
        }
        CFArrayRef dhcpinfo = CFArrayCreate(NULL, (const void **)&pattern, 1, &kCFTypeArrayCallBacks);
        CFRelease(pattern);
        if (dhcpinfo)
        {
            SCDynamicStoreRef store = SCDynamicStoreCreate(NULL, CFSTR("DHCP-LEASES"), NULL, NULL);
            if (store)
            {
                CFDictionaryRef dict = SCDynamicStoreCopyMultiple(store, NULL, dhcpinfo);
                if (dict)
                {
                    ic = CFDictionaryGetCount(dict);
                    const void *vals[ic];
                    CFDictionaryGetKeysAndValues(dict, NULL, vals);

                    for (j = 0; j < ic; j++)
                    {
                        const CFDictionaryRef dhcp = (CFDictionaryRef)vals[j];
                        if (dhcp)
                        {
                            const CFDateRef start = DHCPInfoGetLeaseStartTime(dhcp);
                            const CFDataRef lease = DHCPInfoGetOptionData(dhcp, 51);    // Option 51 = IP Address Lease Time
                            if (!start || !lease || CFDataGetLength(lease) < 4)
                                LogMsg("DHCPWakeTime: SCDynamicStoreCopyDHCPInfo index %d failed "
                                       "CFDateRef start %p CFDataRef lease %p CFDataGetLength(lease) %d",
                                       j, start, lease, lease ? CFDataGetLength(lease) : 0);
                            else
                            {
                                const UInt8 *d = CFDataGetBytePtr(lease);
                                if (!d) LogMsg("DHCPWakeTime: CFDataGetBytePtr %d failed", j);
                                else
                                {
                                    const mDNSu32 elapsed   = now - CFDateGetAbsoluteTime(start);
                                    const mDNSu32 lifetime  = (mDNSs32) ((mDNSs32)d[0] << 24 | (mDNSs32)d[1] << 16 | (mDNSs32)d[2] << 8 | d[3]);
                                    const mDNSu32 remaining = lifetime - elapsed;
                                    const mDNSu32 wake      = remaining > 60 ? remaining - remaining/10 : 54;   // Wake at 90% of the lease time
                                    LogSPS("DHCP Address Lease Elapsed %6u Lifetime %6u Remaining %6u Wake %6u", elapsed, lifetime, remaining, wake);
                                    if (e > wake) e = wake;
                                }
                            }
                        }
                    }
                    CFRelease(dict);
                }
                CFRelease(store);
            }
            CFRelease(dhcpinfo);
        }
    }
    return(e);
}

// We deliberately schedule our wakeup for halfway between when we'd *like* it and when we *need* it.
// For example, if our DHCP lease expires in two hours, we'll typically renew it at the halfway point, after one hour.
// If we scheduled our wakeup for the one-hour renewal time, that might be just seconds from now, and sleeping
// for a few seconds and then waking again is silly and annoying.
// If we scheduled our wakeup for the two-hour expiry time, and we were slow to wake, we might lose our lease.
// Scheduling our wakeup for halfway in between -- 90 minutes -- avoids short wakeups while still
// allowing us an adequate safety margin to renew our lease before we lose it.

mDNSlocal mDNSBool AllowSleepNow(mDNSs32 now)
{
    mDNS *const m = &mDNSStorage;
    mDNSBool ready = mDNSCoreReadyForSleep(m, now);
    if (m->SleepState && !ready && now - m->SleepLimit < 0) return(mDNSfalse);

    m->p->WakeAtUTC = 0;
    int result = kIOReturnSuccess;
    CFDictionaryRef opts = NULL;

    // If the sleep request was cancelled, and we're no longer planning to sleep, don't need to
    // do the stuff below, but we *DO* still need to acknowledge the sleep message we received.
    if (!m->SleepState)
        LogMsg("AllowSleepNow: Sleep request was canceled with %d ticks remaining", m->SleepLimit - now);
    else
    {
        if (!m->SystemWakeOnLANEnabled || !mDNSCoreHaveAdvertisedMulticastServices(m))
            LogSPS("AllowSleepNow: Not scheduling wakeup: SystemWakeOnLAN %s enabled; %s advertised services",
                   m->SystemWakeOnLANEnabled                  ? "is" : "not",
                   mDNSCoreHaveAdvertisedMulticastServices(m) ? "have" : "no");
        else
        {
            mDNSs32 dhcp = DHCPWakeTime();
            LogSPS("ComputeWakeTime: DHCP Wake %d", dhcp);
            mDNSs32 interval = mDNSCoreIntervalToNextWake(m, now) / mDNSPlatformOneSecond;
            if (interval > dhcp) interval = dhcp;

            // If we're not ready to sleep (failed to register with Sleep Proxy, maybe because of
            // transient network problem) then schedule a wakeup in one hour to try again. Otherwise,
            // a single SPS failure could result in a remote machine falling permanently asleep, requiring
            // someone to go to the machine in person to wake it up again, which would be unacceptable.
            if (!ready && interval > 3600) interval = 3600;

            //interval = 48; // For testing

#if TARGET_OS_OSX && defined(kIOPMAcknowledgmentOptionSystemCapabilityRequirements)
            if (m->p->IOPMConnection)   // If lightweight-wake capability is available, use that
            {
                const CFDateRef WakeDate = CFDateCreate(NULL, CFAbsoluteTimeGetCurrent() + interval);
                if (!WakeDate) LogMsg("ScheduleNextWake: CFDateCreate failed");
                else
                {
                    const mDNSs32 reqs         = kIOPMSystemPowerStateCapabilityNetwork;
                    const CFNumberRef Requirements = CFNumberCreate(NULL, kCFNumberSInt32Type, &reqs);
                    if (Requirements == NULL) LogMsg("ScheduleNextWake: CFNumberCreate failed");
                    else
                    {
                        const void *OptionKeys[2] = { kIOPMAckDHCPRenewWakeDate, kIOPMAckSystemCapabilityRequirements };
                        const void *OptionVals[2] = {        WakeDate,          Requirements   };
                        opts = CFDictionaryCreate(NULL, (void*)OptionKeys, (void*)OptionVals, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
                        if (!opts) LogMsg("ScheduleNextWake: CFDictionaryCreate failed");
                        CFRelease(Requirements);
                    }
                    CFRelease(WakeDate);
                }
                LogSPS("AllowSleepNow: Will request lightweight wakeup in %d seconds", interval);
            }
            else                        // else schedule the wakeup using the old API instead to
#endif
            {
                // If we wake within +/- 30 seconds of our requested time we'll assume the system woke for us,
                // so we should put it back to sleep. To avoid frustrating the user, we always request at least
                // 60 seconds sleep, so if they immediately re-wake the system within seconds of it going to sleep,
                // we then shouldn't hit our 30-second window, and we won't attempt to re-sleep the machine.
                if (interval < 60)
                    interval = 60;

                result = mDNSPowerRequest(1, interval);

                if (result == kIOReturnNotReady)
                {
                    int r;
                    LogMsg("AllowSleepNow: Requested wakeup in %d seconds unsuccessful; retrying with longer intervals", interval);
                    // IOPMSchedulePowerEvent fails with kIOReturnNotReady (-536870184/0xe00002d8) if the
                    // requested wake time is "too soon", but there's no API to find out what constitutes
                    // "too soon" on any given OS/hardware combination, so if we get kIOReturnNotReady
                    // we just have to iterate with successively longer intervals until it doesn't fail.
                    // We preserve the value of "result" because if our original power request was deemed "too soon"
                    // for the machine to get to sleep and wake back up again, we attempt to cancel the sleep request,
                    // since the implication is that the system won't manage to be awake again at the time we need it.
                    do
                    {
                        interval += (interval < 20) ? 1 : ((interval+3) / 4);
                        r = mDNSPowerRequest(1, interval);
                    }
                    while (r == kIOReturnNotReady);
                    if (r) LogMsg("AllowSleepNow: Requested wakeup in %d seconds unsuccessful: %d %X", interval, r, r);
                    else LogSPS("AllowSleepNow: Requested later wakeup in %d seconds; will also attempt IOCancelPowerChange", interval);
                }
                else
                {
                    if (result) LogMsg("AllowSleepNow: Requested wakeup in %d seconds unsuccessful: %d %X", interval, result, result);
                    else LogSPS("AllowSleepNow: Requested wakeup in %d seconds", interval);
                }
                m->p->WakeAtUTC = mDNSPlatformUTC() + interval;
            }
        }

        m->SleepState = SleepState_Sleeping;
		// Clear our interface list to empty state, ready to go to sleep
		// As a side effect of doing this, we'll also cancel any outstanding SPS Resolve calls that didn't complete
        mDNSMacOSXNetworkChanged();
    }

    LogSPS("AllowSleepNow: %s(%lX) %s at %ld (%d ticks remaining)",
#if TARGET_OS_OSX && defined(kIOPMAcknowledgmentOptionSystemCapabilityRequirements)
           (m->p->IOPMConnection) ? "IOPMConnectionAcknowledgeEventWithOptions" :
#endif
           (result == kIOReturnSuccess) ? "IOAllowPowerChange" : "IOCancelPowerChange",
           m->p->SleepCookie, ready ? "ready for sleep" : "giving up", now, m->SleepLimit - now);

    m->SleepLimit = 0;  // Don't clear m->SleepLimit until after we've logged it above
    m->TimeSlept = mDNSPlatformUTC();

#if TARGET_OS_OSX && defined(kIOPMAcknowledgmentOptionSystemCapabilityRequirements)
    if (m->p->IOPMConnection) IOPMConnectionAcknowledgeEventWithOptions(m->p->IOPMConnection, m->p->SleepCookie, opts);
    else
#endif
    if (result == kIOReturnSuccess) IOAllowPowerChange (m->p->PowerConnection, m->p->SleepCookie);
    else IOCancelPowerChange(m->p->PowerConnection, m->p->SleepCookie);

    if (opts) CFRelease(opts);
    return(mDNStrue);
}

#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM

mDNSexport void TriggerEventCompletion()
{
    debugf("TriggerEventCompletion: Merge data");
    dispatch_source_merge_data(PlatformStorage.custom, 1);
}

mDNSlocal void PrepareForIdle(void *m_param)
{
    mDNS            *m = m_param;
    int64_t time_offset;
    dispatch_time_t dtime;

    const int multiplier = 1000000000 / mDNSPlatformOneSecond;

    // This is the main work loop:
    // (1) First we give mDNSCore a chance to finish off any of its deferred work and calculate the next sleep time
    // (2) Then we make sure we've delivered all waiting browse messages to our clients
    // (3) Then we sleep for the time requested by mDNSCore, or until the next event, whichever is sooner

    debugf("PrepareForIdle: called");
    // Run mDNS_Execute to find out the time we next need to wake up
    mDNSs32 start          = mDNSPlatformRawTime();
    mDNSs32 nextTimerEvent = udsserver_idle(mDNSDaemonIdle(m));
#if MDNSRESPONDER_SUPPORTS(COMMON, DNS_PUSH)
    if (m->DNSPushServers != mDNSNULL)
    {
        nextTimerEvent = dso_idle(m, nextTimerEvent);
    }
#endif
    mDNSs32 end            = mDNSPlatformRawTime();
    if (end - start >= WatchDogReportingThreshold)
    {
        LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_WARNING,
            "CustomSourceHandler: WARNING: Idle task took %d ms to complete", end - start);
    }

    mDNSs32 now = mDNS_TimeNow(m);

    if (m->ShutdownTime)
    {
        if (mDNSStorage.ResourceRecords)
        {
            LogInfo("Cannot exit yet; Resource Record still exists: %s", ARDisplayString(m, mDNSStorage.ResourceRecords));
            if (mDNS_LoggingEnabled) usleep(10000);     // Sleep 10ms so that we don't flood syslog with too many messages
        }
        if (mDNS_ExitNow(m, now))
        {
            LogInfo("IdleLoop: mDNS_FinalExit");
            mDNS_FinalExit(&mDNSStorage);
            usleep(1000);       // Little 1ms pause before exiting, so we don't lose our final syslog messages
            exit(0);
        }
        if (nextTimerEvent - m->ShutdownTime >= 0)
            nextTimerEvent = m->ShutdownTime;
    }

    if (m->SleepLimit)
        if (!AllowSleepNow(now))
            if (nextTimerEvent - m->SleepLimit >= 0)
                nextTimerEvent = m->SleepLimit;

    // Convert absolute wakeup time to a relative time from now
    mDNSs32 ticks = nextTimerEvent - now;
    if (ticks < 1) ticks = 1;

    static mDNSs32 RepeatedBusy = 0;    // Debugging sanity check, to guard against CPU spins
    if (ticks > 1)
        RepeatedBusy = 0;
    else
    {
        ticks = 1;
        if (++RepeatedBusy >= mDNSPlatformOneSecond) { ShowTaskSchedulingError(&mDNSStorage); RepeatedBusy = 0; }
    }

    time_offset = ((mDNSu32)ticks / mDNSPlatformOneSecond) * 1000000000 + (ticks % mDNSPlatformOneSecond) * multiplier;
    dtime = dispatch_time(DISPATCH_TIME_NOW, time_offset);
    dispatch_source_set_timer(PlatformStorage.timer, dtime, 1000ull*1000000000, 0);
    debugf("PrepareForIdle: scheduling timer with ticks %d", ticks);
    return;
}

#else // MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM

mDNSlocal void KQWokenFlushBytes(int fd, __unused short filter, __unused void *context, __unused mDNSBool encounteredEOF)
{
    // Read all of the bytes so we won't wake again.
    char buffer[100];
    while (recv(fd, buffer, sizeof(buffer), MSG_DONTWAIT) > 0) continue;
}

mDNSlocal void SetLowWater(const KQSocketSet *const k, const int r)
{
    if (k->sktv4 >=0 && setsockopt(k->sktv4, SOL_SOCKET, SO_RCVLOWAT, &r, sizeof(r)) < 0)
        LogMsg("SO_RCVLOWAT IPv4 %d error %d errno %d (%s)", k->sktv4, r, errno, strerror(errno));
    if (k->sktv6 >=0 && setsockopt(k->sktv6, SOL_SOCKET, SO_RCVLOWAT, &r, sizeof(r)) < 0)
        LogMsg("SO_RCVLOWAT IPv6 %d error %d errno %d (%s)", k->sktv6, r, errno, strerror(errno));
}

mDNSlocal void * KQueueLoop(void *m_param)
{
    mDNS            *m = m_param;
    int numevents = 0;

#if USE_SELECT_WITH_KQUEUEFD
    fd_set readfds;
    FD_ZERO(&readfds);
    const int multiplier = 1000000    / mDNSPlatformOneSecond;
#else
    const int multiplier = 1000000000 / mDNSPlatformOneSecond;
#endif

    pthread_mutex_lock(&PlatformStorage.BigMutex);
    LogInfo("Starting time value 0x%08lX (%ld)", (mDNSu32)mDNSStorage.timenow_last, mDNSStorage.timenow_last);

    // This is the main work loop:
    // (1) First we give mDNSCore a chance to finish off any of its deferred work and calculate the next sleep time
    // (2) Then we make sure we've delivered all waiting browse messages to our clients
    // (3) Then we sleep for the time requested by mDNSCore, or until the next event, whichever is sooner
    // (4) On wakeup we first process *all* events
    // (5) then when no more events remain, we go back to (1) to finish off any deferred work and do it all again
    for ( ; ; )
    {
        #define kEventsToReadAtOnce 1
        struct kevent new_events[kEventsToReadAtOnce];

        // Run mDNS_Execute to find out the time we next need to wake up
        mDNSs32 start          = mDNSPlatformRawTime();
        mDNSs32 nextTimerEvent = udsserver_idle(mDNSDaemonIdle(m));
#if MDNSRESPONDER_SUPPORTS(APPLE, DNSSD_XPC_SERVICE)
        dnssd_server_idle();
#endif
#if MDNSRESPONDER_SUPPORTS(COMMON, DNS_PUSH)
        if (m->DNSPushServers != mDNSNULL)
        {
            mDNS_Lock(m);
            nextTimerEvent = dso_idle(m, m->timenow, nextTimerEvent);
            mDNS_Unlock(m);
        }
#endif
        mDNSs32 end            = mDNSPlatformRawTime();
        if (end - start >= WatchDogReportingThreshold)
        {
            LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_WARNING, "WARNING: Idle task took %d ms to complete", end - start);
        }

#if MDNS_MALLOC_DEBUGGING >= 1
        mDNSPlatformValidateLists();
#endif

        mDNSs32 now = mDNS_TimeNow(m);

        if (m->ShutdownTime)
        {
            if (mDNSStorage.ResourceRecords)
            {
                AuthRecord *rr;
                for (rr = mDNSStorage.ResourceRecords; rr; rr=rr->next)
                {
                    LogInfo("Cannot exit yet; Resource Record still exists: %s", ARDisplayString(m, rr));
                    if (mDNS_LoggingEnabled) usleep(10000);     // Sleep 10ms so that we don't flood syslog with too many messages
                }
            }
            if (mDNS_ExitNow(m, now))
            {
                LogInfo("mDNS_FinalExit");
                mDNS_FinalExit(&mDNSStorage);
                usleep(1000);       // Little 1ms pause before exiting, so we don't lose our final syslog messages
                exit(0);
            }
            if (nextTimerEvent - m->ShutdownTime >= 0)
                nextTimerEvent = m->ShutdownTime;
        }

        if (m->SleepLimit)
            if (!AllowSleepNow(now))
                if (nextTimerEvent - m->SleepLimit >= 0)
                    nextTimerEvent = m->SleepLimit;

        // Convert absolute wakeup time to a relative time from now
        mDNSs32 ticks = nextTimerEvent - now;
        if (ticks < 1) ticks = 1;

        static mDNSs32 RepeatedBusy = 0;    // Debugging sanity check, to guard against CPU spins
        if (ticks > 1)
            RepeatedBusy = 0;
        else
        {
            ticks = 1;
            if (++RepeatedBusy >= mDNSPlatformOneSecond) { ShowTaskSchedulingError(&mDNSStorage); RepeatedBusy = 0; }
        }

        verbosedebugf("KQueueLoop: Handled %d events; now sleeping for %d ticks", numevents, ticks);
        numevents = 0;

        // Release the lock, and sleep until:
        // 1. Something interesting happens like a packet arriving, or
        // 2. The other thread writes a byte to WakeKQueueLoopFD to poke us and make us wake up, or
        // 3. The timeout expires
        pthread_mutex_unlock(&PlatformStorage.BigMutex);

        // If we woke up to receive a multicast, set low-water mark to dampen excessive wakeup rate
        if (m->p->num_mcasts)
        {
            SetLowWater(&m->p->permanentsockets, 0x10000);
            if (ticks > mDNSPlatformOneSecond / 8) ticks = mDNSPlatformOneSecond / 8;
        }

#if USE_SELECT_WITH_KQUEUEFD
        struct timeval timeout;
        timeout.tv_sec = ticks / mDNSPlatformOneSecond;
        timeout.tv_usec = (ticks % mDNSPlatformOneSecond) * multiplier;
        FD_SET(KQueueFD, &readfds);
        if (select(KQueueFD+1, &readfds, NULL, NULL, &timeout) < 0)
        { LogMsg("select(%d) failed errno %d (%s)", KQueueFD, errno, strerror(errno)); sleep(1); }
#else
        struct timespec timeout;
        timeout.tv_sec = ticks / mDNSPlatformOneSecond;
        timeout.tv_nsec = (ticks % mDNSPlatformOneSecond) * multiplier;
        // In my opinion, you ought to be able to call kevent() with nevents set to zero,
        // and have it work similarly to the way it does with nevents non-zero --
        // i.e. it waits until either an event happens or the timeout expires, and then wakes up.
        // In fact, what happens if you do this is that it just returns immediately. So, we have
        // to pass nevents set to one, and then we just ignore the event it gives back to us. -- SC
        if (kevent(KQueueFD, NULL, 0, new_events, 1, &timeout) < 0)
        { LogMsg("kevent(%d) failed errno %d (%s)", KQueueFD, errno, strerror(errno)); sleep(1); }
#endif

        pthread_mutex_lock(&PlatformStorage.BigMutex);
        // We have to ignore the event we may have been told about above, because that
        // was done without holding the lock, and between the time we woke up and the
        // time we reclaimed the lock the other thread could have done something that
        // makes the event no longer valid. Now we have the lock, we call kevent again
        // and this time we can safely process the events it tells us about.

        // If we changed UDP socket low-water mark, restore it, so we will be told about every packet
        if (m->p->num_mcasts)
        {
            SetLowWater(&m->p->permanentsockets, 1);
            m->p->num_mcasts = 0;
        }

        static const struct timespec zero_timeout = { 0, 0 };
        int events_found;
        while ((events_found = kevent(KQueueFD, NULL, 0, new_events, kEventsToReadAtOnce, &zero_timeout)) != 0)
        {
            if (events_found > kEventsToReadAtOnce || (events_found < 0 && errno != EINTR))
            {
                const int kevent_errno = errno;
                // Not sure what to do here, our kqueue has failed us - this isn't ideal
                LogMsg("ERROR: KQueueLoop - kevent failed errno %d (%s)", kevent_errno, strerror(kevent_errno));
                exit(kevent_errno);
            }

            numevents += events_found;

            int i;
            for (i = 0; i < events_found; i++)
            {
                const KQueueEntry *const kqentry = new_events[i].udata;
                mDNSs32 stime = mDNSPlatformRawTime();
                const char *const KQtask = kqentry->KQtask; // Grab a copy in case KQcallback deletes the task
                kqentry->KQcallback(new_events[i].ident, new_events[i].filter, kqentry->KQcontext, (new_events[i].flags & EV_EOF) != 0);
                mDNSs32 etime = mDNSPlatformRawTime();
                if (etime - stime >= WatchDogReportingThreshold)
                {
                    LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_WARNING, 
                        "WARNING: " PUB_S " took %d ms to complete", KQtask, etime - stime);
                }
            }
        }
    }

    return NULL;
}

#endif // MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM

mDNSlocal size_t LaunchdCheckin(void)
{
    // Ask launchd for our socket
    int result = launch_activate_socket("Listeners", &launchd_fds, &launchd_fds_count);
    if (result != 0) { LogMsg("launch_activate_socket() failed error %d (%s)", result, strerror(result)); }
    return launchd_fds_count;
}


extern int sandbox_init(const char *profile, uint64_t flags, char **errorbuf) __attribute__((weak_import));

#if APPLE_OSX_mDNSResponder
mDNSlocal mDNSBool PreferencesGetValueBool(CFStringRef key, mDNSBool defaultValue)
{
    CFBooleanRef boolean;
    mDNSBool result = defaultValue;

    boolean = CFPreferencesCopyAppValue(key, kProgramArguments);
    if (boolean != NULL)
    {
        if (CFGetTypeID(boolean) == CFBooleanGetTypeID())
            result = CFBooleanGetValue(boolean) ? mDNStrue : mDNSfalse;
        CFRelease(boolean);
    }

    return result;
}

mDNSlocal int PreferencesGetValueInt(CFStringRef key, int defaultValue)
{
    CFNumberRef number;
    int numberValue;
    int result = defaultValue;

    number = CFPreferencesCopyAppValue(key, kProgramArguments);
    if (number != NULL)
    {
        if ((CFGetTypeID(number) == CFNumberGetTypeID()) && CFNumberGetValue(number, kCFNumberIntType, &numberValue))
            result = numberValue;
        CFRelease(number);
    }

    return result;
}
#endif

mDNSlocal void SandboxProcess(void)
{
    // Invoke sandbox profile /usr/share/sandbox/mDNSResponder.sb
#if MDNS_NO_SANDBOX
    LogMsg("Note: Compiled without Apple Sandbox support");
#else // MDNS_NO_SANDBOX
    if (!sandbox_init)
        LogMsg("Note: Running without Apple Sandbox support (not available on this OS)");
    else
    {
        char *sandbox_msg;
        uint64_t sandbox_flags = SANDBOX_NAMED;

        (void)confstr(_CS_DARWIN_USER_CACHE_DIR, NULL, 0);

        int sandbox_err = sandbox_init("mDNSResponder", sandbox_flags, &sandbox_msg);
        if (sandbox_err)
        {
            LogMsg("WARNING: sandbox_init error %s", sandbox_msg);
            // If we have errors in the sandbox during development, to prevent
            // exiting, uncomment the following line.
            //sandbox_free_error(sandbox_msg);
            
            errx(EX_OSERR, "sandbox_init() failed: %s", sandbox_msg);
        }
        else LogInfo("Now running under Apple Sandbox restrictions");
    }
#endif // MDNS_NO_SANDBOX
}

#if MDNSRESPONDER_SUPPORTS(APPLE, OS_LOG)
#define MDNS_OS_LOG_CATEGORY_INIT(NAME) \
    do\
    { \
        mDNSLogCategory_ ## NAME = os_log_create("com.apple.mDNSResponder", # NAME ); \
        if (!mDNSLogCategory_ ## NAME ) \
        { \
            os_log_error(OS_LOG_DEFAULT, "Could NOT create the " # NAME " log handle in mDNSResponder"); \
            mDNSLogCategory_ ## NAME = OS_LOG_DEFAULT; \
        } \
    } \
    while (0)

os_log_t mDNSLogCategory_Default = NULL;
os_log_t mDNSLogCategory_mDNS    = NULL;
os_log_t mDNSLogCategory_uDNS    = NULL;
os_log_t mDNSLogCategory_SPS     = NULL;
os_log_t mDNSLogCategory_XPC     = NULL;

mDNSlocal void init_logging(void)
{
    MDNS_OS_LOG_CATEGORY_INIT(Default);
    MDNS_OS_LOG_CATEGORY_INIT(mDNS);
    MDNS_OS_LOG_CATEGORY_INIT(uDNS);
    MDNS_OS_LOG_CATEGORY_INIT(SPS);
    MDNS_OS_LOG_CATEGORY_INIT(XPC);
}
#endif

mDNSexport int main(int argc, char **argv)
{
    int i;
    kern_return_t status;

#if DEBUG
    bool useDebugSocket = mDNSfalse;
    bool useSandbox = mDNStrue;
#endif

#if MDNSRESPONDER_SUPPORTS(APPLE, OS_LOG)
    init_logging();
#endif

    mDNSMacOSXSystemBuildNumber(NULL);
    LogMsg("%s starting %s %d", mDNSResponderVersionString, OSXVers ? "OSXVers" : "iOSVers", OSXVers ? OSXVers : iOSVers);

#if 0
    LogMsg("CacheRecord         %5d", sizeof(CacheRecord));
    LogMsg("CacheGroup          %5d", sizeof(CacheGroup));
    LogMsg("ResourceRecord      %5d", sizeof(ResourceRecord));
    LogMsg("RData_small         %5d", sizeof(RData_small));

    LogMsg("sizeof(CacheEntity) %5d", sizeof(CacheEntity));
    LogMsg("RR_CACHE_SIZE       %5d", RR_CACHE_SIZE);
    LogMsg("block bytes used    %5d",           sizeof(CacheEntity) * RR_CACHE_SIZE);
    LogMsg("block bytes wasted  %5d", 32*1024 - sizeof(CacheEntity) * RR_CACHE_SIZE);
#endif

    if (0 == geteuid())
    {
        LogMsg("mDNSResponder cannot be run as root !! Exiting..");
        return -1;
    }

    for (i=1; i<argc; i++)
    {
        if (!strcasecmp(argv[i], "-d"                        )) mDNS_DebugMode            = mDNStrue;
        if (!strcasecmp(argv[i], "-NoMulticastAdvertisements")) NoMulticastAdvertisements = mDNStrue;
        if (!strcasecmp(argv[i], "-DisableSleepProxyClient"  )) DisableSleepProxyClient   = mDNStrue;
        if (!strcasecmp(argv[i], "-DebugLogging"             )) mDNS_LoggingEnabled       = mDNStrue;
        if (!strcasecmp(argv[i], "-UnicastPacketLogging"     )) mDNS_PacketLoggingEnabled = mDNStrue;
        if (!strcasecmp(argv[i], "-OfferSleepProxyService"   ))
            OfferSleepProxyService = (i+1 < argc && mDNSIsDigit(argv[i+1][0]) && mDNSIsDigit(argv[i+1][1]) && argv[i+1][2]==0) ? atoi(argv[++i]) : 100;
        if (!strcasecmp(argv[i], "-UseInternalSleepProxy"    ))
            UseInternalSleepProxy = (i+1<argc && mDNSIsDigit(argv[i+1][0]) && argv[i+1][1]==0) ? atoi(argv[++i]) : 1;
        if (!strcasecmp(argv[i], "-StrictUnicastOrdering"    )) StrictUnicastOrdering     = mDNStrue;
        if (!strcasecmp(argv[i], "-AlwaysAppendSearchDomains")) AlwaysAppendSearchDomains = mDNStrue;
        if (!strcasecmp(argv[i], "-DisableAllowExpired"      )) EnableAllowExpired        = mDNSfalse;
#if DEBUG
        if (!strcasecmp(argv[i], "-UseDebugSocket"))            useDebugSocket = mDNStrue;
        if (!strcasecmp(argv[i], "-NoSandbox"))                 useSandbox = mDNSfalse;
#endif    
    }


#if APPLE_OSX_mDNSResponder
/* Reads the external user's program arguments for mDNSResponder starting 10.11.x(El Capitan) on OSX. The options for external user are: 
   DebugLogging, UnicastPacketLogging, NoMulticastAdvertisements, StrictUnicastOrdering and AlwaysAppendSearchDomains

   To turn ON the particular option, here is what the user should do (as an example of setting two options)
   1] sudo defaults write /Library/Preferences/com.apple.mDNSResponder.plist AlwaysAppendSearchDomains -bool YES
   2] sudo defaults write /Library/Preferences/com.apple.mDNSResponder.plist NoMulticastAdvertisements -bool YES
   3] sudo reboot

   To turn OFF all options, here is what the user should do
   1] sudo defaults delete /Library/Preferences/com.apple.mDNSResponder.plist
   2] sudo reboot

   To view the current options set, here is what the user should do
   1] plutil -p /Library/Preferences/com.apple.mDNSResponder.plist
   OR
   1] sudo defaults read /Library/Preferences/com.apple.mDNSResponder.plist
   
*/
    
// Currently on Fuji/Whitetail releases we are keeping the logging always enabled.
// Hence mDNS_LoggingEnabled and mDNS_PacketLoggingEnabled is set to true below by default.
#if 0
    mDNS_LoggingEnabled       = PreferencesGetValueBool(kPreferencesKey_DebugLogging,              mDNS_LoggingEnabled);
    mDNS_PacketLoggingEnabled = PreferencesGetValueBool(kPreferencesKey_UnicastPacketLogging,      mDNS_PacketLoggingEnabled);
#endif
    
    mDNS_LoggingEnabled       = mDNStrue;
    mDNS_PacketLoggingEnabled = mDNStrue;

    NoMulticastAdvertisements = PreferencesGetValueBool(kPreferencesKey_NoMulticastAdvertisements, NoMulticastAdvertisements);
    StrictUnicastOrdering     = PreferencesGetValueBool(kPreferencesKey_StrictUnicastOrdering,     StrictUnicastOrdering);
    AlwaysAppendSearchDomains = PreferencesGetValueBool(kPreferencesKey_AlwaysAppendSearchDomains, AlwaysAppendSearchDomains);
    EnableAllowExpired        = PreferencesGetValueBool(kPreferencesKey_EnableAllowExpired,        EnableAllowExpired);
    OfferSleepProxyService    = PreferencesGetValueInt(kPreferencesKey_OfferSleepProxyService,     OfferSleepProxyService);
    UseInternalSleepProxy     = PreferencesGetValueInt(kPreferencesKey_UseInternalSleepProxy,      UseInternalSleepProxy);

#if ENABLE_BLE_TRIGGERED_BONJOUR
    EnableBLEBasedDiscovery   = PreferencesGetValueBool(kPreferencesKey_EnableBLEBasedDiscovery,   EnableBLEBasedDiscovery);
    DefaultToBLETriggered     = PreferencesGetValueBool(kPreferencesKey_DefaultToBLETriggered,     DefaultToBLETriggered);
#endif  // ENABLE_BLE_TRIGGERED_BONJOUR
#endif

#if MDNSRESPONDER_SUPPORTS(APPLE, PREALLOCATED_CACHE)
    PreallocateCacheMemory    = PreferencesGetValueBool(kPreferencesKey_PreallocateCacheMemory,    PreallocateCacheMemory);
#endif

    // Note that mDNSPlatformInit will set DivertMulticastAdvertisements in the mDNS structure
    if (NoMulticastAdvertisements)
        LogMsg("-NoMulticastAdvertisements is set: Administratively prohibiting multicast advertisements");
    if (AlwaysAppendSearchDomains)
        LogMsg("-AlwaysAppendSearchDomains is set");    
    if (StrictUnicastOrdering)
        LogMsg("-StrictUnicastOrdering is set");

#ifndef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM

    signal(SIGHUP,  HandleSIG);     // (Debugging) Purge the cache to check for cache handling bugs
    signal(SIGINT,  HandleSIG);     // Ctrl-C: Detach from Mach BootstrapService and exit cleanly
    signal(SIGPIPE,   SIG_IGN);     // Don't want SIGPIPE signals -- we'll handle EPIPE errors directly
    signal(SIGTERM, HandleSIG);     // Machine shutting down: Detach from and exit cleanly like Ctrl-C
    signal(SIGINFO, HandleSIG);     // (Debugging) Write state snapshot to syslog
    signal(SIGUSR1, HandleSIG);     // (Debugging) Enable Logging
    signal(SIGUSR2, HandleSIG);     // (Debugging) Enable Packet Logging
    signal(SIGPROF, HandleSIG);     // (Debugging) Toggle Multicast Logging
    signal(SIGTSTP, HandleSIG);     // (Debugging) Disable all Debug Logging (USR1/USR2/PROF)

#endif // MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM

    mDNSStorage.p = &PlatformStorage;   // Make sure mDNSStorage.p is set up, because validatelists uses it

#ifndef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM

    // Create the kqueue, mutex and thread to support KQSockets
    KQueueFD = kqueue();
    if (KQueueFD == -1)
    {
        const int kqueue_errno = errno;
        LogMsg("kqueue() failed errno %d (%s)", kqueue_errno, strerror(kqueue_errno));
        status = kqueue_errno;
        goto exit;
    }

    i = pthread_mutex_init(&PlatformStorage.BigMutex, NULL);
    if (i != 0) { LogMsg("pthread_mutex_init() failed error %d (%s)", i, strerror(i)); status = i; goto exit; }

    int fdpair[2] = {0, 0};
    i = socketpair(AF_UNIX, SOCK_STREAM, 0, fdpair);
    if (i == -1)
    {
        const int socketpair_errno = errno;
        LogMsg("socketpair() failed errno %d (%s)", socketpair_errno, strerror(socketpair_errno));
        status = socketpair_errno;
        goto exit;
    }

    // Socket pair returned us two identical sockets connected to each other
    // We will use the first socket to send the second socket. The second socket
    // will be added to the kqueue so it will wake when data is sent.
    static const KQueueEntry wakeKQEntry = { KQWokenFlushBytes, NULL, "kqueue wakeup after CFRunLoop event" };

    PlatformStorage.WakeKQueueLoopFD = fdpair[0];
    KQueueSet(fdpair[1], EV_ADD, EVFILT_READ, &wakeKQEntry);

#endif // MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM

#if DEBUG
    if (useSandbox)
#endif
    SandboxProcess();

#if MDNSRESPONDER_SUPPORTS(APPLE, METRICS)
    status = MetricsInit();
    if (status) { LogMsg("Daemon start: MetricsInit failed (%d)", status); }
#endif

    status = mDNSDaemonInitialize();
    if (status) { LogMsg("Daemon start: mDNSDaemonInitialize failed"); goto exit; }

    // Need to Start XPC Server Before LaunchdCheckin() (Reason: radar:11023750)
    xpc_server_init();
#if MDNSRESPONDER_SUPPORTS(APPLE, DNSSD_XPC_SERVICE)
    dnssd_server_init();
#endif
#if DEBUG
    if (!useDebugSocket) {
        if (LaunchdCheckin() == 0)
            useDebugSocket = mDNStrue;
    }
    if (useDebugSocket)
        SetDebugBoundPath();
#else
    LaunchdCheckin();
#endif

    status = udsserver_init(launchd_fds, launchd_fds_count);
    if (status) { LogMsg("Daemon start: udsserver_init failed"); goto exit; }

    mDNSMacOSXNetworkChanged();
    UpdateDebugState();

#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
    LogInfo("Daemon Start: Using LibDispatch");
    // CFRunLoopRun runs both CFRunLoop sources and dispatch sources
    CFRunLoopRun();
#else // MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
      // Start the kqueue thread
    pthread_t KQueueThread;
    i = pthread_create(&KQueueThread, NULL, KQueueLoop, &mDNSStorage);
    if (i != 0) { LogMsg("pthread_create() failed error %d (%s)", i, strerror(i)); status = i; goto exit; }
    if (status == 0)
    {
        CFRunLoopRun();
        LogMsg("ERROR: CFRunLoopRun Exiting.");
        mDNS_Close(&mDNSStorage);
    }
#endif // MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM

    LogMsg("%s exiting", mDNSResponderVersionString);

exit:
    return(status);
}

// uds_daemon.c support routines /////////////////////////////////////////////

mDNSlocal void kqUDSEventCallback(int fd, short filter, void *context, mDNSBool encounteredEOF)
{
    const KQSocketEventSource *const source = context;
    (void)filter; // unused
    (void)encounteredEOF; // unused
    
    source->callback(fd, source->context);
}

// Arrange things so that when data appears on fd, callback is called with context
mDNSexport mStatus udsSupportAddFDToEventLoop(int fd, udsEventCallback callback, void *context, void **platform_data)
{
    KQSocketEventSource **p = &gEventSources;
    (void) platform_data;
    while (*p && (*p)->fd != fd) p = &(*p)->next;
    if (*p) { LogMsg("udsSupportAddFDToEventLoop: ERROR fd %d already has EventLoop source entry", fd); return mStatus_AlreadyRegistered; }

    KQSocketEventSource *newSource = (KQSocketEventSource*) callocL("KQSocketEventSource", sizeof(*newSource));
    if (!newSource) return mStatus_NoMemoryErr;

    newSource->next           = mDNSNULL;
    newSource->fd             = fd;
    newSource->callback       = callback;
    newSource->context        = context;
    newSource->kqs.KQcallback = kqUDSEventCallback;
    newSource->kqs.KQcontext  = newSource;
    newSource->kqs.KQtask     = "UDS client";
#ifdef MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM
    newSource->kqs.readSource  = mDNSNULL;
    newSource->kqs.writeSource = mDNSNULL;
    newSource->kqs.fdClosed    = mDNSfalse;
#endif // MDNSRESPONDER_USES_LIB_DISPATCH_AS_PRIMARY_EVENT_LOOP_MECHANISM

    if (KQueueSet(fd, EV_ADD, EVFILT_READ, &newSource->kqs) == 0)
    {
        *p = newSource;
        return mStatus_NoError;
    }

    LogMsg("KQueueSet failed for fd %d errno %d (%s)", fd, errno, strerror(errno));
    freeL("KQSocketEventSource", newSource);
    return mStatus_BadParamErr;
}

int udsSupportReadFD(dnssd_sock_t fd, char *buf, int len, int flags, void *platform_data)
{
    (void) platform_data;
    return recv(fd, buf, len, flags);
}

mDNSexport mStatus udsSupportRemoveFDFromEventLoop(int fd, void *platform_data)     // Note: This also CLOSES the file descriptor
{
    KQSocketEventSource **p = &gEventSources;
    (void) platform_data;
    while (*p && (*p)->fd != fd) p = &(*p)->next;
    if (*p)
    {
        KQSocketEventSource *s = *p;
        *p = (*p)->next;
        // We don't have to explicitly do a kqueue EV_DELETE here because closing the fd
        // causes the kernel to automatically remove any associated kevents
        mDNSPlatformCloseFD(&s->kqs, s->fd);
        freeL("KQSocketEventSource", s);
        return mStatus_NoError;
    }
    LogMsg("udsSupportRemoveFDFromEventLoop: ERROR fd %d not found in EventLoop source list", fd);
    return mStatus_NoSuchNameErr;
}

#ifdef UNIT_TEST
#include "../unittests/daemon_ut.c"
#endif // UNIT_TEST

#if _BUILDING_XCODE_PROJECT_
// If mDNSResponder crashes, then this string will be magically included in the automatically-generated crash log
const char *__crashreporter_info__ = mDNSResponderVersionString;
asm (".desc ___crashreporter_info__, 0x10");
#endif

// For convenience when using the "strings" command, this is the last thing in the file
// The "@(#) " pattern is a special prefix the "what" command looks for
mDNSexport const char mDNSResponderVersionString_SCCS[] = "@(#) mDNSResponder " STRINGIFY(mDNSResponderVersion) " (" __DATE__ " " __TIME__ ")";
