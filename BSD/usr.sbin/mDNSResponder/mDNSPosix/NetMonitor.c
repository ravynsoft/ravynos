/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2002-2004 Apple Computer, Inc. All rights reserved.
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
 *
 */

//*************************************************************************************************************
// Incorporate mDNS.c functionality

// We want to use much of the functionality provided by "mDNS.c",
// except we'll steal the packets that would be sent to normal mDNSCoreReceive() routine
#define mDNSCoreReceive __NOT__mDNSCoreReceive__NOT__
#include "../mDNSCore/mDNS.c"
#undef mDNSCoreReceive

//*************************************************************************************************************
// Headers

#include <stdio.h>          // For printf()
#include <stdlib.h>         // For malloc()
#include <string.h>         // For strrchr(), strcmp()
#include <time.h>           // For "struct tm" etc.
#include <signal.h>         // For SIGINT, SIGTERM
#if defined(WIN32)
// Both mDNS.c and mDNSWin32.h declare UDPSocket_struct type resulting in a compile-time error, so
// trick the compiler when including mDNSWin32.h
#   define UDPSocket_struct _UDPSocket_struct
#   include <mDNSEmbeddedAPI.h>
#   include <mDNSWin32.h>
#   include <PosixCompat.h>
#   include <Poll.h>
#   define IFNAMSIZ 256
static HANDLE gStopEvent = INVALID_HANDLE_VALUE;
static mDNSBool gRunning;
static void CALLBACK StopNotification( HANDLE event, void *context ) { gRunning = mDNSfalse; }
static BOOL WINAPI ConsoleControlHandler( DWORD inControlEvent ) { SetEvent( gStopEvent ); return TRUE; }
void setlinebuf( FILE * fp ) {}
#else
#   include <netdb.h>           // For gethostbyname()
#   include <sys/socket.h>      // For AF_INET, AF_INET6, etc.
#   include <net/if.h>          // For IF_NAMESIZE
#   include <netinet/in.h>      // For INADDR_NONE
#   include <arpa/inet.h>       // For inet_addr()
#   include "mDNSPosix.h"      // Defines the specific types needed to run mDNS on this platform
#endif
#include "ExampleClientApp.h"

//*************************************************************************************************************
// Types and structures

enum
{
    // Primitive operations
    OP_probe        = 0,
    OP_goodbye      = 1,

    // These are meta-categories;
    // Query and Answer operations are actually subdivided into two classes:
    // Browse  query/answer and
    // Resolve query/answer
    OP_query        = 2,
    OP_answer       = 3,

    // The "Browse" variants of query/answer
    OP_browsegroup  = 2,
    OP_browseq      = 2,
    OP_browsea      = 3,

    // The "Resolve" variants of query/answer
    OP_resolvegroup = 4,
    OP_resolveq     = 4,
    OP_resolvea     = 5,

    OP_NumTypes = 6
};

typedef struct ActivityStat_struct ActivityStat;
struct ActivityStat_struct
{
    ActivityStat *next;
    domainname srvtype;
    int printed;
    int totalops;
    int stat[OP_NumTypes];
};

typedef struct FilterList_struct FilterList;
struct FilterList_struct
{
    FilterList *next;
    mDNSAddr FilterAddr;
};

//*************************************************************************************************************
// Constants

#define kReportTopServices 15
#define kReportTopHosts    15

//*************************************************************************************************************
// Globals

mDNS mDNSStorage;                       // mDNS core uses this to store its globals
static mDNS_PlatformSupport PlatformStorage;    // Stores this platform's globals
mDNSexport const char ProgramName[] = "mDNSNetMonitor";

struct timeval tv_start, tv_end, tv_interval;
static int FilterInterface = 0;
static FilterList *Filters;
#define ExactlyOneFilter (Filters && !Filters->next)
static mDNSBool AddressType = mDNSAddrType_IPv4;

static int NumPktQ, NumPktL, NumPktR, NumPktB;  // Query/Legacy/Response/Bad
static int NumProbes, NumGoodbyes, NumQuestions, NumLegacy, NumAnswers, NumAdditionals;

static ActivityStat *stats;

#define OPBanner "Total Ops   Probe   Goodbye  BrowseQ  BrowseA ResolveQ ResolveA"

mDNSexport void mDNSCoreReceive(mDNS *const m, DNSMessage *const msg, const mDNSu8 *const end, const mDNSAddr *const srcaddr, const mDNSIPPort srcport, const mDNSAddr *dstaddr, const mDNSIPPort dstport, const mDNSInterfaceID InterfaceID);

//*************************************************************************************************************
// Utilities

// Special version of printf that knows how to print IP addresses, DNS-format name strings, etc.
mDNSlocal mDNSu32 mprintf(const char *format, ...) IS_A_PRINTF_STYLE_FUNCTION(1,2);
mDNSlocal mDNSu32 mprintf(const char *format, ...)
{
    mDNSu32 length;
    unsigned char buffer[512];
    va_list ptr;
    va_start(ptr,format);
    length = mDNS_vsnprintf((char *)buffer, sizeof(buffer), format, ptr);
    va_end(ptr);
    printf("%s", buffer);
    return(length);
}

//*************************************************************************************************************
// Host Address List
//
// Would benefit from a hash

typedef enum
{
    HostPkt_Q        = 0,       // Query
    HostPkt_L        = 1,       // Legacy Query
    HostPkt_R        = 2,       // Response
    HostPkt_B        = 3,       // Bad
    HostPkt_NumTypes = 4
} HostPkt_Type;

typedef struct
{
    mDNSAddr addr;
    unsigned long pkts[HostPkt_NumTypes];
    unsigned long totalops;
    unsigned long stat[OP_NumTypes];
    domainname hostname;
    domainname revname;
    UTF8str255 HIHardware;
    UTF8str255 HISoftware;
    mDNSu32 NumQueries;
    mDNSs32 LastQuery;
} HostEntry;

#define HostEntryTotalPackets(H) ((H)->pkts[HostPkt_Q] + (H)->pkts[HostPkt_L] + (H)->pkts[HostPkt_R] + (H)->pkts[HostPkt_B])

typedef struct
{
    long num;
    long max;
    HostEntry   *hosts;
} HostList;

static HostList IPv4HostList = { 0, 0, 0 };
static HostList IPv6HostList = { 0, 0, 0 };

mDNSlocal HostEntry *FindHost(const mDNSAddr *addr, HostList *list)
{
    long i;

    for (i = 0; i < list->num; i++)
    {
        HostEntry *entry = list->hosts + i;
        if (mDNSSameAddress(addr, &entry->addr))
            return entry;
    }

    return NULL;
}

mDNSlocal HostEntry *AddHost(const mDNSAddr *addr, HostList *list)
{
    int i;
    HostEntry *entry;
    if (list->num >= list->max)
    {
        long newMax = list->max + 64;
        HostEntry *newHosts = realloc(list->hosts, newMax * sizeof(HostEntry));
        if (newHosts == NULL)
            return NULL;
        list->max = newMax;
        list->hosts = newHosts;
    }

    entry = list->hosts + list->num++;

    entry->addr = *addr;
    for (i=0; i<HostPkt_NumTypes; i++) entry->pkts[i] = 0;
    entry->totalops = 0;
    for (i=0; i<OP_NumTypes;      i++) entry->stat[i] = 0;
    entry->hostname.c[0] = 0;
    entry->revname.c[0] = 0;
    entry->HIHardware.c[0] = 0;
    entry->HISoftware.c[0] = 0;
    entry->NumQueries = 0;

    if (entry->addr.type == mDNSAddrType_IPv4)
    {
        mDNSv4Addr ip = entry->addr.ip.v4;
        char buffer[32];
        // Note: This is reverse order compared to a normal dotted-decimal IP address, so we can't use our customary "%.4a" format code
        mDNS_snprintf(buffer, sizeof(buffer), "%d.%d.%d.%d.in-addr.arpa.", ip.b[3], ip.b[2], ip.b[1], ip.b[0]);
        MakeDomainNameFromDNSNameString(&entry->revname, buffer);
    }

    return(entry);
}

mDNSlocal HostEntry *GotPacketFromHost(const mDNSAddr *addr, HostPkt_Type t, mDNSOpaque16 id)
{
    if (ExactlyOneFilter) return(NULL);
    else
    {
        HostList *list = (addr->type == mDNSAddrType_IPv4) ? &IPv4HostList : &IPv6HostList;
        HostEntry *entry = FindHost(addr, list);
        if (!entry) entry = AddHost(addr, list);
        if (!entry) return(NULL);
        // Don't count our own interrogation packets
        if (id.NotAnInteger != 0xFFFF) entry->pkts[t]++;
        return(entry);
    }
}

mDNSlocal void RecordHostInfo(HostEntry *entry, const ResourceRecord *const pktrr)
{
    if (!entry->hostname.c[0])
    {
        if (pktrr->rrtype == kDNSType_A || pktrr->rrtype == kDNSType_AAAA)
        {
            // Should really check that the rdata in the address record matches the source address of this packet
            entry->NumQueries = 0;
            AssignDomainName(&entry->hostname, pktrr->name);
        }

        if (pktrr->rrtype == kDNSType_PTR)
            if (SameDomainName(&entry->revname, pktrr->name))
            {
                entry->NumQueries = 0;
                AssignDomainName(&entry->hostname, &pktrr->rdata->u.name);
            }
    }
    else if (pktrr->rrtype == kDNSType_HINFO)
    {
        RDataBody *rd = &pktrr->rdata->u;
        mDNSu8 *rdend = (mDNSu8 *)rd + pktrr->rdlength;
        mDNSu8 *hw = rd->txt.c;
        mDNSu8 *sw = hw + 1 + (mDNSu32)hw[0];
        if (sw + 1 + sw[0] <= rdend)
        {
            AssignDomainName(&entry->hostname, pktrr->name);
            mDNSPlatformMemCopy(entry->HIHardware.c, hw, 1 + (mDNSu32)hw[0]);
            mDNSPlatformMemCopy(entry->HISoftware.c, sw, 1 + (mDNSu32)sw[0]);
        }
    }
}

mDNSlocal void SendUnicastQuery(mDNS *const m, HostEntry *entry, domainname *name, mDNSu16 rrtype, mDNSInterfaceID InterfaceID)
{
    const mDNSOpaque16 id = { { 0xFF, 0xFF } };
    DNSMessage query;
    mDNSu8       *qptr        = query.data;
    const mDNSu8 *const limit = query.data + sizeof(query.data);
    const mDNSAddr *target    = &entry->addr;
    InitializeDNSMessage(&query.h, id, QueryFlags);
    qptr = putQuestion(&query, qptr, limit, name, rrtype, kDNSClass_IN);
    entry->LastQuery = m->timenow;
    entry->NumQueries++;

    // Note: When there are multiple mDNSResponder agents running on a single machine
    // (e.g. Apple mDNSResponder plus a SliMP3 server with embedded mDNSResponder)
    // it is possible that unicast queries may not go to the primary system responder.
    // We try the first query using unicast, but if that doesn't work we try again via multicast.
    if (entry->NumQueries > 2)
    {
        target = &AllDNSLinkGroup_v4;
    }
    else
    {
        //mprintf("%#a Q\n", target);
        InterfaceID = mDNSInterface_Any;    // Send query from our unicast reply socket
    }

    mDNSSendDNSMessage(m, &query, qptr, InterfaceID, mDNSNULL, mDNSNULL, target, MulticastDNSPort, mDNSNULL, mDNSfalse);
}

mDNSlocal void AnalyseHost(mDNS *const m, HostEntry *entry, const mDNSInterfaceID InterfaceID)
{
    // If we've done four queries without answer, give up
    if (entry->NumQueries >= 4) return;

    // If we've done a query in the last second, give the host a chance to reply before trying again
    if (entry->NumQueries && m->timenow - entry->LastQuery < mDNSPlatformOneSecond) return;

    // If we don't know the host name, try to find that first
    if (!entry->hostname.c[0])
    {
        if (entry->revname.c[0])
        {
            SendUnicastQuery(m, entry, &entry->revname, kDNSType_PTR, InterfaceID);
            //mprintf("%##s PTR %d\n", entry->revname.c, entry->NumQueries);
        }
    }
    // If we have the host name but no HINFO, now ask for that
    else if (!entry->HIHardware.c[0])
    {
        SendUnicastQuery(m, entry, &entry->hostname, kDNSType_HINFO, InterfaceID);
        //mprintf("%##s HINFO %d\n", entry->hostname.c, entry->NumQueries);
    }
}

mDNSlocal int CompareHosts(const void *p1, const void *p2)
{
    return (int)(HostEntryTotalPackets((HostEntry *)p2) - HostEntryTotalPackets((HostEntry *)p1));
}

mDNSlocal void ShowSortedHostList(HostList *list, int max)
{
    HostEntry *e, *end = &list->hosts[(max < list->num) ? max : list->num];
    qsort(list->hosts, list->num, sizeof(HostEntry), CompareHosts);
    if (list->num) mprintf("\n%-25s%s%s\n", "Source Address", OPBanner, "    Pkts    Query   LegacyQ Response");
    for (e = &list->hosts[0]; e < end; e++)
    {
        int len = mprintf("%#-25a", &e->addr);
        if (len > 25) mprintf("\n%25s", "");
        mprintf("%8lu %8lu %8lu %8lu %8lu %8lu %8lu", e->totalops,
                e->stat[OP_probe], e->stat[OP_goodbye],
                e->stat[OP_browseq], e->stat[OP_browsea],
                e->stat[OP_resolveq], e->stat[OP_resolvea]);
        mprintf(" %8lu %8lu %8lu %8lu",
                HostEntryTotalPackets(e), e->pkts[HostPkt_Q], e->pkts[HostPkt_L], e->pkts[HostPkt_R]);
        if (e->pkts[HostPkt_B]) mprintf("Bad: %8lu", e->pkts[HostPkt_B]);
        mprintf("\n");
        if (!e->HISoftware.c[0] && e->NumQueries > 2)
            mDNSPlatformMemCopy(&e->HISoftware, "\x27*** Unknown (Jaguar, Windows, etc.) ***", 0x28);
        if (e->hostname.c[0] || e->HIHardware.c[0] || e->HISoftware.c[0])
            mprintf("%##-45s %#-14s %#s\n", e->hostname.c, e->HIHardware.c, e->HISoftware.c);
    }
}

//*************************************************************************************************************
// Receive and process packets

mDNSlocal mDNSBool ExtractServiceType(const domainname *const fqdn, domainname *const srvtype)
{
    int i, len;
    const mDNSu8 *src = fqdn->c;
    mDNSu8 *dst = srvtype->c;

    len = *src;
    if (len == 0 || len >= 0x40) return(mDNSfalse);
    if (src[1] != '_') src += 1 + len;

    len = *src;
    if (len == 0 || len >= 0x40 || src[1] != '_') return(mDNSfalse);
    for (i=0; i<=len; i++) *dst++ = *src++;

    len = *src;
    if (len == 0 || len >= 0x40 || src[1] != '_') return(mDNSfalse);
    for (i=0; i<=len; i++) *dst++ = *src++;

    *dst++ = 0;     // Put the null root label on the end of the service type

    return(mDNStrue);
}

mDNSlocal void recordstat(HostEntry *entry, const domainname *fqdn, int op, mDNSu16 rrtype)
{
    ActivityStat **s = &stats;
    domainname srvtype;

    if (op != OP_probe)
    {
        if (rrtype == kDNSType_SRV || rrtype == kDNSType_TXT) op = op - OP_browsegroup + OP_resolvegroup;
        else if (rrtype != kDNSType_PTR) return;
    }

    if (!ExtractServiceType(fqdn, &srvtype)) return;

    while (*s && !SameDomainName(&(*s)->srvtype, &srvtype)) s = &(*s)->next;
    if (!*s)
    {
        int i;
        *s = malloc(sizeof(ActivityStat));
        if (!*s) exit(-1);
        (*s)->next     = NULL;
        (*s)->srvtype  = srvtype;
        (*s)->printed  = 0;
        (*s)->totalops = 0;
        for (i=0; i<OP_NumTypes; i++) (*s)->stat[i] = 0;
    }

    (*s)->totalops++;
    (*s)->stat[op]++;
    if (entry)
    {
        entry->totalops++;
        entry->stat[op]++;
    }
}

mDNSlocal void printstats(int max)
{
    int i;
    if (!stats) return;
    for (i=0; i<max; i++)
    {
        int max_val = 0;
        ActivityStat *s, *m = NULL;
        for (s = stats; s; s=s->next)
            if (!s->printed && max_val < s->totalops)
            { m = s; max_val = s->totalops; }
        if (!m) return;
        m->printed = mDNStrue;
        if (i==0) mprintf("%-25s%s\n", "Service Type", OPBanner);
        mprintf("%##-25s%8d %8d %8d %8d %8d %8d %8d\n", m->srvtype.c, m->totalops, m->stat[OP_probe],
                m->stat[OP_goodbye], m->stat[OP_browseq], m->stat[OP_browsea], m->stat[OP_resolveq], m->stat[OP_resolvea]);
    }
}

mDNSlocal const mDNSu8 *FindUpdate(mDNS *const m, const DNSMessage *const query, const mDNSu8 *ptr, const mDNSu8 *const end,
                                   DNSQuestion *q, LargeCacheRecord *pkt)
{
    int i;
    for (i = 0; i < query->h.numAuthorities; i++)
    {
        const mDNSu8 *p2 = ptr;
        ptr = GetLargeResourceRecord(m, query, ptr, end, q->InterfaceID, kDNSRecordTypePacketAuth, pkt);
        if (!ptr) break;
        if (m->rec.r.resrec.RecordType != kDNSRecordTypePacketNegative && ResourceRecordAnswersQuestion(&pkt->r.resrec, q)) return(p2);
    }
    return(mDNSNULL);
}

mDNSlocal void DisplayPacketHeader(mDNS *const m, const DNSMessage *const msg, const mDNSu8 *const end, const mDNSAddr *srcaddr, mDNSIPPort srcport, const mDNSAddr *dstaddr, const mDNSInterfaceID InterfaceID)
{
    const char *const ptype = (msg->h.flags.b[0] & kDNSFlag0_QR_Response)             ? "-R- " :
                              (srcport.NotAnInteger == MulticastDNSPort.NotAnInteger) ? "-Q- " : "-LQ-";
    const unsigned length = end - (mDNSu8 *)msg;
    struct timeval tv;
    struct tm tm;
    const mDNSu32 index = mDNSPlatformInterfaceIndexfromInterfaceID(m, InterfaceID, mDNSfalse);
    char if_name[IFNAMSIZ];     // Older Linux distributions don't define IF_NAMESIZE
    if_indextoname(index, if_name);
    gettimeofday(&tv, NULL);
    localtime_r((time_t*)&tv.tv_sec, &tm);
    mprintf("\n%d:%02d:%02d.%06d Interface %d/%s\n", tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec, index, if_name);

    mprintf("%#-16a %s             Q:%3d  Ans:%3d  Auth:%3d  Add:%3d  Size:%5d bytes",
            srcaddr, ptype, msg->h.numQuestions, msg->h.numAnswers, msg->h.numAuthorities, msg->h.numAdditionals, length);

    if (msg->h.id.NotAnInteger) mprintf("  ID:%u", mDNSVal16(msg->h.id));

    if (!mDNSAddrIsDNSMulticast(dstaddr)) mprintf("   To: %#a", dstaddr);

    if (msg->h.flags.b[0] & kDNSFlag0_TC)
    {
        if (msg->h.flags.b[0] & kDNSFlag0_QR_Response) mprintf("   Truncated");
        else mprintf("   Truncated (KA list continues in next packet)");
    }

    mprintf("\n");

    if (length < sizeof(DNSMessageHeader) + NormalMaxDNSMessageData - 192)
        if (msg->h.flags.b[0] & kDNSFlag0_TC)
            mprintf("%#-16a **** WARNING: Packet suspiciously small. Payload size (excluding IP and UDP headers)\n"
                    "%#-16a **** should usually be closer to %d bytes before truncation becomes necessary.\n",
                    srcaddr, srcaddr, sizeof(DNSMessageHeader) + NormalMaxDNSMessageData);
}

mDNSlocal void DisplaySizeCheck(const DNSMessage *const msg, const mDNSu8 *const end, const mDNSAddr *srcaddr, int num_opts)
{
    const unsigned length = end - (mDNSu8 *)msg;
    const int num_records = msg->h.numAnswers + msg->h.numAuthorities + msg->h.numAdditionals - num_opts;

    if (length > sizeof(DNSMessageHeader) + NormalMaxDNSMessageData)
        if (num_records > 1)
            mprintf("%#-16a **** ERROR: Oversized packet with %d records.\n"
                    "%#-16a **** Many network devices cannot receive packets larger than %d bytes.\n"
                    "%#-16a **** To minimize interoperability failures, oversized packets MUST be limited to a single resource record.\n",
                    srcaddr, num_records, srcaddr, 40 + 8 + sizeof(DNSMessageHeader) + NormalMaxDNSMessageData, srcaddr);
}

mDNSlocal void DisplayResourceRecord(const mDNSAddr *const srcaddr, const char *const op, const ResourceRecord *const pktrr)
{
    static const char hexchars[16] = "0123456789ABCDEF";
    #define MaxWidth 132
    char buffer[MaxWidth+8];
    char *p = buffer;

    RDataBody *rd = &pktrr->rdata->u;
    mDNSu8 *rdend = (mDNSu8 *)rd + pktrr->rdlength;
    int n = mprintf("%#-16a %-5s %-5s%5lu %##s -> ", srcaddr, op, DNSTypeName(pktrr->rrtype), pktrr->rroriginalttl, pktrr->name->c);

    if (pktrr->RecordType == kDNSRecordTypePacketNegative) { mprintf("**** ERROR: FAILED TO READ RDATA ****\n"); return; }

    // The kDNSType_OPT case below just calls GetRRDisplayString_rdb
    // Perhaps more (or all?) of the cases should do that?
    switch(pktrr->rrtype)
    {
    case kDNSType_A:    mprintf("%.4a", &rd->ipv4); break;
    case kDNSType_PTR:  mprintf("%##.*s", MaxWidth - n, rd->name.c); break;
    case kDNSType_HINFO:    // same as kDNSType_TXT below
    case kDNSType_TXT:  {
        mDNSu8 *t = rd->txt.c;
        while (t < rdend && t[0] && p < buffer+MaxWidth)
        {
            int i;
            for (i=1; i<=t[0] && p < buffer+MaxWidth; i++)
            {
                if (t[i] == '\\') *p++ = '\\';
                if (t[i] >= ' ') *p++ = t[i];
                else
                {
                    *p++ = '\\';
                    *p++ = '0';
                    *p++ = 'x';
                    *p++ = hexchars[t[i] >> 4];
                    *p++ = hexchars[t[i] & 0xF];
                }
            }
            t += 1+t[0];
            if (t < rdend && t[0]) { *p++ = '\\'; *p++ = ' '; }
        }
        *p++ = 0;
        mprintf("%.*s", MaxWidth - n, buffer);
    } break;
    case kDNSType_AAAA: mprintf("%.16a", &rd->ipv6); break;
    case kDNSType_SRV:  mprintf("%##s:%d", rd->srv.target.c, mDNSVal16(rd->srv.port)); break;
    case kDNSType_OPT:  {
        char b[MaxMsg];
        // Quick hack: we don't want the prefix that GetRRDisplayString_rdb puts at the start of its
        // string, because it duplicates the name and rrtype we already display, so we compute the
        // length of that prefix and strip that many bytes off the beginning of the string we display.
        mDNSu32 striplen = mDNS_snprintf(b, MaxMsg-1, "%4d %##s %s ", pktrr->rdlength, pktrr->name->c, DNSTypeName(pktrr->rrtype));
        GetRRDisplayString_rdb(pktrr, &pktrr->rdata->u, b);
        mprintf("%.*s", MaxWidth - n, b + striplen);
    } break;
    case kDNSType_NSEC: {
        char b[MaxMsg];
        // See the quick hack above
        mDNSu32 striplen = mDNS_snprintf(b, MaxMsg-1, "%4d %##s %s ", pktrr->rdlength, pktrr->name->c, DNSTypeName(pktrr->rrtype));
        GetRRDisplayString_rdb(pktrr, &pktrr->rdata->u, b);
        mprintf("%s", b+striplen);
    } break;
    default:            {
        mDNSu8 *s = rd->data;
        while (s < rdend && p < buffer+MaxWidth)
        {
            if (*s == '\\') *p++ = '\\';
            if (*s >= ' ') *p++ = *s;
            else
            {
                *p++ = '\\';
                *p++ = '0';
                *p++ = 'x';
                *p++ = hexchars[*s >> 4];
                *p++ = hexchars[*s & 0xF];
            }
            s++;
        }
        *p++ = 0;
        mprintf("%.*s", MaxWidth - n, buffer);
    } break;
    }

    mprintf("\n");
}

mDNSlocal void HexDump(const mDNSu8 *ptr, const mDNSu8 *const end)
{
    while (ptr < end)
    {
        int i;
        for (i=0; i<16; i++)
            if (&ptr[i] < end) mprintf("%02X ", ptr[i]);
            else mprintf("   ");
        for (i=0; i<16; i++)
            if (&ptr[i] < end) mprintf("%c", ptr[i] <= ' ' || ptr[i] >= 126 ? '.' : ptr[i]);
        ptr += 16;
        mprintf("\n");
    }
}

mDNSlocal void DisplayError(const mDNSAddr *srcaddr, const mDNSu8 *ptr, const mDNSu8 *const end, char *msg)
{
    mprintf("%#-16a **** ERROR: FAILED TO READ %s ****\n", srcaddr, msg);
    HexDump(ptr, end);
}

mDNSlocal void DisplayQuery(mDNS *const m, const DNSMessage *const msg, const mDNSu8 *const end,
                            const mDNSAddr *srcaddr, mDNSIPPort srcport, const mDNSAddr *dstaddr, const mDNSInterfaceID InterfaceID)
{
    int i;
    int num_opts = 0;
    const mDNSu8 *ptr = msg->data;
    const mDNSu8 *auth = LocateAuthorities(msg, end);
    mDNSBool MQ = (srcport.NotAnInteger == MulticastDNSPort.NotAnInteger);
    HostEntry *entry = GotPacketFromHost(srcaddr, MQ ? HostPkt_Q : HostPkt_L, msg->h.id);
    LargeCacheRecord pkt;

    DisplayPacketHeader(m, msg, end, srcaddr, srcport, dstaddr, InterfaceID);
    if (msg->h.id.NotAnInteger != 0xFFFF)
    {
        if (MQ) NumPktQ++; else NumPktL++;
    }

    for (i=0; i<msg->h.numQuestions; i++)
    {
        DNSQuestion q;
        mDNSu8 *p2 = (mDNSu8 *)getQuestion(msg, ptr, end, InterfaceID, &q);
        mDNSu16 ucbit = q.qclass & kDNSQClass_UnicastResponse;
        q.qclass &= ~kDNSQClass_UnicastResponse;
        if (!p2) { DisplayError(srcaddr, ptr, end, "QUESTION"); return; }
        ptr = p2;
        p2 = (mDNSu8 *)FindUpdate(m, msg, auth, end, &q, &pkt);
        if (p2)
        {
            NumProbes++;
            DisplayResourceRecord(srcaddr, ucbit ? "(PU)" : "(PM)", &pkt.r.resrec);
            recordstat(entry, &q.qname, OP_probe, q.qtype);
            p2 = (mDNSu8 *)skipDomainName(msg, p2, end);
            // Having displayed this update record, clear type and class so we don't display the same one again.
            p2[0] = p2[1] = p2[2] = p2[3] = 0;
        }
        else
        {
            const char *ptype = ucbit ? "(QU)" : "(QM)";
            if (srcport.NotAnInteger == MulticastDNSPort.NotAnInteger) NumQuestions++;
            else { NumLegacy++; ptype = "(LQ)"; }
            mprintf("%#-16a %-5s %-5s      %##s\n", srcaddr, ptype, DNSTypeName(q.qtype), q.qname.c);
            if (msg->h.id.NotAnInteger != 0xFFFF) recordstat(entry, &q.qname, OP_query, q.qtype);
        }
    }

    for (i=0; i<msg->h.numAnswers; i++)
    {
        const mDNSu8 *ep = ptr;
        ptr = GetLargeResourceRecord(m, msg, ptr, end, InterfaceID, kDNSRecordTypePacketAns, &pkt);
        if (!ptr) { DisplayError(srcaddr, ep, end, "KNOWN ANSWER"); return; }
        DisplayResourceRecord(srcaddr, "(KA)", &pkt.r.resrec);
        if (pkt.r.resrec.rrtype == kDNSType_OPT)
            { num_opts++; mprintf("%#-16a **** ERROR: OPT RECORD IN ANSWER SECTION ****\n", srcaddr); }

        // In the case of queries with long multi-packet KA lists, we count each subsequent KA packet
        // the same as a single query, to more accurately reflect the burden on the network
        // (A query with a six-packet KA list is *at least* six times the burden on the network as a single-packet query.)
        if (msg->h.numQuestions == 0 && i == 0)
            recordstat(entry, pkt.r.resrec.name, OP_query, pkt.r.resrec.rrtype);
    }

    for (i=0; i<msg->h.numAuthorities; i++)
    {
        const mDNSu8 *ep = ptr;
        ptr = GetLargeResourceRecord(m, msg, ptr, end, InterfaceID, kDNSRecordTypePacketAuth, &pkt);
        if (!ptr) { DisplayError(srcaddr, ep, end, "AUTHORITY"); return; }
        // After we display an Update record with its matching question (above) we zero out its type and class
        // If any remain that haven't been zero'd out, display them here
        if (pkt.r.resrec.rrtype || pkt.r.resrec.rrclass) DisplayResourceRecord(srcaddr, "(AU)", &pkt.r.resrec);
        if (pkt.r.resrec.rrtype == kDNSType_OPT)
            { num_opts++; mprintf("%#-16a **** ERROR: OPT RECORD IN AUTHORITY SECTION ****\n", srcaddr); }
    }

    for (i=0; i<msg->h.numAdditionals; i++)
    {
        const mDNSu8 *ep = ptr;
        ptr = GetLargeResourceRecord(m, msg, ptr, end, InterfaceID, kDNSRecordTypePacketAdd, &pkt);
        if (!ptr) { DisplayError(srcaddr, ep, end, "ADDITIONAL"); return; }
        DisplayResourceRecord(srcaddr, pkt.r.resrec.rrtype == kDNSType_OPT ? "(OP)" : "(AD)", &pkt.r.resrec);
        if (pkt.r.resrec.rrtype == kDNSType_OPT) num_opts++;
    }

    DisplaySizeCheck(msg, end, srcaddr, num_opts);

    // We don't hexdump the DNSMessageHeader here because those six fields (id, flags, numQuestions, numAnswers, numAuthorities, numAdditionals)
    // have already been swapped to host byte order and displayed, so including them in the hexdump is confusing
    if (num_opts > 1) { mprintf("%#-16a **** ERROR: MULTIPLE OPT RECORDS ****\n", srcaddr); HexDump(msg->data, end); }

    if (entry) AnalyseHost(m, entry, InterfaceID);
}

mDNSlocal void DisplayResponse(mDNS *const m, const DNSMessage *const msg, const mDNSu8 *end,
                               const mDNSAddr *srcaddr, mDNSIPPort srcport, const mDNSAddr *dstaddr, const mDNSInterfaceID InterfaceID)
{
    int i;
    int num_opts = 0;
    const mDNSu8 *ptr = msg->data;
    HostEntry *entry = GotPacketFromHost(srcaddr, HostPkt_R, msg->h.id);
    LargeCacheRecord pkt;

    DisplayPacketHeader(m, msg, end, srcaddr, srcport, dstaddr, InterfaceID);
    if (msg->h.id.NotAnInteger != 0xFFFF) NumPktR++;

    for (i=0; i<msg->h.numQuestions; i++)
    {
        DNSQuestion q;
        const mDNSu8 *ep = ptr;
        ptr = getQuestion(msg, ptr, end, InterfaceID, &q);
        if (!ptr) { DisplayError(srcaddr, ep, end, "QUESTION"); return; }
        if (mDNSAddrIsDNSMulticast(dstaddr))
            mprintf("%#-16a (?)   **** ERROR: SHOULD NOT HAVE Q IN mDNS RESPONSE **** %-5s %##s\n", srcaddr, DNSTypeName(q.qtype), q.qname.c);
        else
            mprintf("%#-16a (Q)   %-5s      %##s\n", srcaddr, DNSTypeName(q.qtype), q.qname.c);
    }

    for (i=0; i<msg->h.numAnswers; i++)
    {
        const mDNSu8 *ep = ptr;
        ptr = GetLargeResourceRecord(m, msg, ptr, end, InterfaceID, kDNSRecordTypePacketAns, &pkt);
        if (!ptr) { DisplayError(srcaddr, ep, end, "ANSWER"); return; }
        if (pkt.r.resrec.rroriginalttl)
        {
            NumAnswers++;
            DisplayResourceRecord(srcaddr, (pkt.r.resrec.RecordType & kDNSRecordTypePacketUniqueMask) ? "(AN)" : "(AN+)", &pkt.r.resrec);
            if (msg->h.id.NotAnInteger != 0xFFFF) recordstat(entry, pkt.r.resrec.name, OP_answer, pkt.r.resrec.rrtype);
            if (entry) RecordHostInfo(entry, &pkt.r.resrec);
        }
        else
        {
            NumGoodbyes++;
            DisplayResourceRecord(srcaddr, "(DE)", &pkt.r.resrec);
            recordstat(entry, pkt.r.resrec.name, OP_goodbye, pkt.r.resrec.rrtype);
        }
        if (pkt.r.resrec.rrtype == kDNSType_OPT)
            { num_opts++; mprintf("%#-16a **** ERROR: OPT RECORD IN ANSWER SECTION ****\n", srcaddr); }
    }

    for (i=0; i<msg->h.numAuthorities; i++)
    {
        const mDNSu8 *ep = ptr;
        ptr = GetLargeResourceRecord(m, msg, ptr, end, InterfaceID, kDNSRecordTypePacketAuth, &pkt);
        if (!ptr) { DisplayError(srcaddr, ep, end, "AUTHORITY"); return; }
        DisplayResourceRecord(srcaddr, "(AU)", &pkt.r.resrec);
        if (pkt.r.resrec.rrtype == kDNSType_OPT)
            { num_opts++; mprintf("%#-16a **** ERROR: OPT RECORD IN AUTHORITY SECTION ****\n", srcaddr); }
        else if (pkt.r.resrec.rrtype != kDNSType_NSEC3)
            mprintf("%#-16a (?)  **** ERROR: SHOULD NOT HAVE AUTHORITY IN mDNS RESPONSE **** %-5s %##s\n",
                srcaddr, DNSTypeName(pkt.r.resrec.rrtype), pkt.r.resrec.name->c);
    }

    for (i=0; i<msg->h.numAdditionals; i++)
    {
        const mDNSu8 *ep = ptr;
        ptr = GetLargeResourceRecord(m, msg, ptr, end, InterfaceID, kDNSRecordTypePacketAdd, &pkt);
        if (!ptr) { DisplayError(srcaddr, ep, end, "ADDITIONAL"); return; }
        NumAdditionals++;
        if (pkt.r.resrec.rrtype == kDNSType_OPT) num_opts++;
        DisplayResourceRecord(srcaddr,
                              pkt.r.resrec.rrtype == kDNSType_OPT ? "(OP)" : (pkt.r.resrec.RecordType & kDNSRecordTypePacketUniqueMask) ? "(AD)" : "(AD+)",
                              &pkt.r.resrec);
        if (entry) RecordHostInfo(entry, &pkt.r.resrec);
    }

    DisplaySizeCheck(msg, end, srcaddr, num_opts);

    // We don't hexdump the DNSMessageHeader here because those six fields (id, flags, numQuestions, numAnswers, numAuthorities, numAdditionals)
    // have already been swapped to host byte order and displayed, so including them in the hexdump is confusing
    if (num_opts > 1) { mprintf("%#-16a **** ERROR: MULTIPLE OPT RECORDS ****\n", srcaddr); HexDump(msg->data, end); }

    if (entry) AnalyseHost(m, entry, InterfaceID);
}

mDNSlocal void ProcessUnicastResponse(mDNS *const m, const DNSMessage *const msg, const mDNSu8 *end, const mDNSAddr *srcaddr, const mDNSInterfaceID InterfaceID)
{
    int i;
    const mDNSu8 *ptr = LocateAnswers(msg, end);
    HostEntry *entry = GotPacketFromHost(srcaddr, HostPkt_R, msg->h.id);
    //mprintf("%#a R\n", srcaddr);

    for (i=0; i<msg->h.numAnswers + msg->h.numAuthorities + msg->h.numAdditionals; i++)
    {
        LargeCacheRecord pkt;
        ptr = GetLargeResourceRecord(m, msg, ptr, end, InterfaceID, kDNSRecordTypePacketAns, &pkt);
        if (ptr && pkt.r.resrec.rroriginalttl && entry) RecordHostInfo(entry, &pkt.r.resrec);
    }
}

mDNSlocal mDNSBool AddressMatchesFilterList(const mDNSAddr *srcaddr)
{
    FilterList *f;
    if (!Filters) return(srcaddr->type == AddressType);
    for (f=Filters; f; f=f->next) if (mDNSSameAddress(srcaddr, &f->FilterAddr)) return(mDNStrue);
    return(mDNSfalse);
}

mDNSexport void mDNSCoreReceive(mDNS *const m, DNSMessage *const msg, const mDNSu8 *const end,
                                const mDNSAddr *srcaddr, mDNSIPPort srcport, const mDNSAddr *dstaddr, mDNSIPPort dstport, const mDNSInterfaceID InterfaceID)
{
    const mDNSu8 StdQ = kDNSFlag0_QR_Query    | kDNSFlag0_OP_StdQuery;
    const mDNSu8 StdR = kDNSFlag0_QR_Response | kDNSFlag0_OP_StdQuery;
    const mDNSu8 QR_OP = (mDNSu8)(msg->h.flags.b[0] & kDNSFlag0_QROP_Mask);
    mDNSu8 *ptr = (mDNSu8 *)&msg->h.numQuestions;
    int goodinterface = (FilterInterface == 0);

    (void)dstaddr;  // Unused
    (void)dstport;  // Unused

    // Read the integer parts which are in IETF byte-order (MSB first, LSB second)
    msg->h.numQuestions   = (mDNSu16)((mDNSu16)ptr[0] <<  8 | ptr[1]);
    msg->h.numAnswers     = (mDNSu16)((mDNSu16)ptr[2] <<  8 | ptr[3]);
    msg->h.numAuthorities = (mDNSu16)((mDNSu16)ptr[4] <<  8 | ptr[5]);
    msg->h.numAdditionals = (mDNSu16)((mDNSu16)ptr[6] <<  8 | ptr[7]);

    // For now we're only interested in monitoring IPv4 traffic.
    // All IPv6 packets should just be duplicates of the v4 packets.
    if (!goodinterface) goodinterface = (FilterInterface == (int)mDNSPlatformInterfaceIndexfromInterfaceID(m, InterfaceID, mDNSfalse));
    if (goodinterface && AddressMatchesFilterList(srcaddr))
    {
        mDNS_Lock(m);
        if (!mDNSAddrIsDNSMulticast(dstaddr))
        {
            if      (QR_OP == StdQ) mprintf("Unicast query from %#a\n", srcaddr);
            else if (QR_OP == StdR) ProcessUnicastResponse(m, msg, end, srcaddr,                   InterfaceID);
        }
        else
        {
            if      (QR_OP == StdQ) DisplayQuery          (m, msg, end, srcaddr, srcport, dstaddr, InterfaceID);
            else if (QR_OP == StdR) DisplayResponse       (m, msg, end, srcaddr, srcport, dstaddr, InterfaceID);
            else
            {
                debugf("Unknown DNS packet type %02X%02X (ignored)", msg->h.flags.b[0], msg->h.flags.b[1]);
                GotPacketFromHost(srcaddr, HostPkt_B, msg->h.id);
                NumPktB++;
            }
        }
        mDNS_Unlock(m);
    }
}

mDNSlocal mStatus mDNSNetMonitor(void)
{
    struct tm tm;
    int h, m, s, mul, div, TotPkt;
#if !defined(WIN32)
    sigset_t signals;
#endif

    mStatus status = mDNS_Init(&mDNSStorage, &PlatformStorage,
                               mDNS_Init_NoCache, mDNS_Init_ZeroCacheSize,
                               mDNS_Init_DontAdvertiseLocalAddresses,
                               mDNS_Init_NoInitCallback, mDNS_Init_NoInitCallbackContext);
    if (status) return(status);

    gettimeofday(&tv_start, NULL);

#if defined( WIN32 )
    status = SetupInterfaceList(&mDNSStorage);
    if (status) return(status);
    gStopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (gStopEvent == INVALID_HANDLE_VALUE) return mStatus_UnknownErr;
    mDNSPollRegisterEvent( gStopEvent, StopNotification, NULL );
    if (!SetConsoleCtrlHandler(ConsoleControlHandler, TRUE)) return mStatus_UnknownErr;
    gRunning = mDNStrue; while (gRunning) mDNSPoll( INFINITE );
    if (!SetConsoleCtrlHandler(ConsoleControlHandler, FALSE)) return mStatus_UnknownErr;
    CloseHandle(gStopEvent);
#else
    mDNSPosixListenForSignalInEventLoop(SIGINT);
    mDNSPosixListenForSignalInEventLoop(SIGTERM);

    do
    {
        struct timeval timeout = { FutureTime, 0 };     // wait until SIGINT or SIGTERM
        mDNSBool gotSomething;
        mDNSPosixRunEventLoopOnce(&mDNSStorage, &timeout, &signals, &gotSomething);
    }
    while ( !( sigismember( &signals, SIGINT) || sigismember( &signals, SIGTERM)));
#endif

    // Now display final summary
    TotPkt = NumPktQ + NumPktL + NumPktR;
    gettimeofday(&tv_end, NULL);
    tv_interval = tv_end;
    if (tv_start.tv_usec > tv_interval.tv_usec)
    { tv_interval.tv_usec += 1000000; tv_interval.tv_sec--; }
    tv_interval.tv_sec  -= tv_start.tv_sec;
    tv_interval.tv_usec -= tv_start.tv_usec;
    h = (tv_interval.tv_sec / 3600);
    m = (tv_interval.tv_sec % 3600) / 60;
    s = (tv_interval.tv_sec % 60);
    if (tv_interval.tv_sec > 10)
    {
        mul = 60;
        div = tv_interval.tv_sec;
    }
    else
    {
        mul = 60000;
        div = tv_interval.tv_sec * 1000 + tv_interval.tv_usec / 1000;
        if (div == 0) div=1;
    }

    mprintf("\n\n");
    localtime_r((time_t*)&tv_start.tv_sec, &tm);
    mprintf("Started      %3d:%02d:%02d.%06d\n", tm.tm_hour, tm.tm_min, tm.tm_sec, tv_start.tv_usec);
    localtime_r((time_t*)&tv_end.tv_sec, &tm);
    mprintf("End          %3d:%02d:%02d.%06d\n", tm.tm_hour, tm.tm_min, tm.tm_sec, tv_end.tv_usec);
    mprintf("Captured for %3d:%02d:%02d.%06d\n", h, m, s, tv_interval.tv_usec);
    if (!Filters)
    {
        mprintf("Unique source addresses seen on network:");
        if (IPv4HostList.num) mprintf(" %ld (IPv4)", IPv4HostList.num);
        if (IPv6HostList.num) mprintf(" %ld (IPv6)", IPv6HostList.num);
        if (!IPv4HostList.num && !IPv6HostList.num) mprintf(" None");
        mprintf("\n");
    }
    mprintf("\n");
    mprintf("Modern Query        Packets:      %7d   (avg%5d/min)\n", NumPktQ,        NumPktQ        * mul / div);
    mprintf("Legacy Query        Packets:      %7d   (avg%5d/min)\n", NumPktL,        NumPktL        * mul / div);
    mprintf("Multicast Response  Packets:      %7d   (avg%5d/min)\n", NumPktR,        NumPktR        * mul / div);
    mprintf("Total     Multicast Packets:      %7d   (avg%5d/min)\n", TotPkt,         TotPkt         * mul / div);
    mprintf("\n");
    mprintf("Total New Service Probes:         %7d   (avg%5d/min)\n", NumProbes,      NumProbes      * mul / div);
    mprintf("Total Goodbye Announcements:      %7d   (avg%5d/min)\n", NumGoodbyes,    NumGoodbyes    * mul / div);
    mprintf("Total Query Questions:            %7d   (avg%5d/min)\n", NumQuestions,   NumQuestions   * mul / div);
    mprintf("Total Queries from Legacy Clients:%7d   (avg%5d/min)\n", NumLegacy,      NumLegacy      * mul / div);
    mprintf("Total Answers/Announcements:      %7d   (avg%5d/min)\n", NumAnswers,     NumAnswers     * mul / div);
    mprintf("Total Additional Records:         %7d   (avg%5d/min)\n", NumAdditionals, NumAdditionals * mul / div);
    mprintf("\n");
    printstats(kReportTopServices);

    if (!ExactlyOneFilter)
    {
        ShowSortedHostList(&IPv4HostList, kReportTopHosts);
        ShowSortedHostList(&IPv6HostList, kReportTopHosts);
    }

    mDNS_Close(&mDNSStorage);
    return(0);
}

mDNSexport int main(int argc, char **argv)
{
    const char *progname = strrchr(argv[0], '/') ? strrchr(argv[0], '/') + 1 : argv[0];
    int i;
    mStatus status;

#if defined(WIN32)
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
#endif

    setlinebuf(stdout);             // Want to see lines as they appear, not block buffered

    for (i=1; i<argc; i++)
    {
		if (i+1 < argc && !strcmp(argv[i], "-i"))
        {
			FilterInterface = if_nametoindex(argv[i+1]);
			if (!FilterInterface) FilterInterface = atoi(argv[i+1]);
			if (!FilterInterface) {
				fprintf(stderr, "Unknown interface %s\n", argv[i+1]);
				goto usage;
			}
            printf("Monitoring interface %d/%s\n", FilterInterface, argv[i+1]);
			i += 1;
        }
        else if (!strcmp(argv[i], "-6"))
        {
            AddressType = mDNSAddrType_IPv6;
            printf("Monitoring IPv6 traffic\n");
        }
        else
        {
            struct in_addr s4;
            struct in6_addr s6;
            FilterList *f;
            mDNSAddr a;
            a.type = mDNSAddrType_IPv4;

            if (inet_pton(AF_INET, argv[i], &s4) == 1)
                a.ip.v4.NotAnInteger = s4.s_addr;
            else if (inet_pton(AF_INET6, argv[i], &s6) == 1)
            {
                a.type = mDNSAddrType_IPv6;
                mDNSPlatformMemCopy(&a.ip.v6, &s6, sizeof(a.ip.v6));
            }
            else
            {
                struct hostent *h = gethostbyname(argv[i]);
                if (h) a.ip.v4.NotAnInteger = *(long*)h->h_addr;
                else goto usage;
            }

            f = malloc(sizeof(*f));
            f->FilterAddr = a;
            f->next = Filters;
            Filters = f;
        }
    }

    status = mDNSNetMonitor();
    if (status) { fprintf(stderr, "%s: mDNSNetMonitor failed %d\n", progname, (int)status); return(status); }
    return(0);

usage:
    fprintf(stderr, "\nmDNS traffic monitor\n");
    fprintf(stderr, "Usage: %s [-i index] [-6] [host]\n", progname);
    fprintf(stderr, "Optional [-i index] parameter displays only packets from that interface index\n");
	fprintf(stderr, "Optional [-6] parameter displays only ipv6 packets (defaults to only ipv4 packets)\n");
    fprintf(stderr, "Optional [host] parameter displays only packets from that host\n");

    fprintf(stderr, "\nPer-packet header output:\n");
    fprintf(stderr, "-Q-            Multicast Query from mDNS client that accepts multicast responses\n");
    fprintf(stderr, "-R-            Multicast Response packet containing answers/announcements\n");
    fprintf(stderr, "-LQ-           Multicast Query from legacy client that does *not* listen for multicast responses\n");
    fprintf(stderr, "Q/Ans/Auth/Add Number of questions, answers, authority records and additional records in packet\n");

    fprintf(stderr, "\nPer-record display:\n");
    fprintf(stderr, "(PM)           Probe Question (new service starting), requesting multicast response\n");
    fprintf(stderr, "(PU)           Probe Question (new service starting), requesting unicast response\n");
    fprintf(stderr, "(DE)           Deletion/Goodbye (service going away)\n");
    fprintf(stderr, "(LQ)           Legacy Query Question\n");
    fprintf(stderr, "(QM)           Query Question, requesting multicast response\n");
    fprintf(stderr, "(QU)           Query Question, requesting unicast response\n");
    fprintf(stderr, "(KA)           Known Answer (information querier already knows)\n");
    fprintf(stderr, "(AN)           Unique Answer to question (or periodic announcment) (entire RR Set)\n");
    fprintf(stderr, "(AN+)          Answer to question (or periodic announcment) (add to existing RR Set members)\n");
    fprintf(stderr, "(AD)           Unique Additional Record Set (entire RR Set)\n");
    fprintf(stderr, "(AD+)          Additional records (add to existing RR Set members)\n");

    fprintf(stderr, "\nFinal summary, sorted by service type:\n");
    fprintf(stderr, "Probe          Probes for this service type starting up\n");
    fprintf(stderr, "Goodbye        Goodbye (deletion) packets for this service type shutting down\n");
    fprintf(stderr, "BrowseQ        Browse questions from clients browsing to find a list of instances of this service\n");
    fprintf(stderr, "BrowseA        Browse answers/announcments advertising instances of this service\n");
    fprintf(stderr, "ResolveQ       Resolve questions from clients actively connecting to an instance of this service\n");
    fprintf(stderr, "ResolveA       Resolve answers/announcments giving connection information for an instance of this service\n");
    fprintf(stderr, "\n");
    return(-1);
}
