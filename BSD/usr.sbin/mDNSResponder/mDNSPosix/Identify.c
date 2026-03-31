/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2002-2018 Apple Inc. All rights reserved.
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

// We want to use the functionality provided by "mDNS.c",
// except we'll sneak a peek at the packets before forwarding them to the normal mDNSCoreReceive() routine
#define mDNSCoreReceive __MDNS__mDNSCoreReceive
#include "mDNS.c"
#undef mDNSCoreReceive

//*************************************************************************************************************
// Headers

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>       // For n_long, required by <netinet/ip.h> below
#include <netinet/ip.h>             // For IPTOS_LOWDELAY etc.
#include <arpa/inet.h>
#include <signal.h>

#include "mDNSEmbeddedAPI.h" // Defines the interface to the mDNS core code
#include "mDNSPosix.h"    // Defines the specific types needed to run mDNS on this platform
#include "ExampleClientApp.h"

//*************************************************************************************************************
// Globals

mDNS mDNSStorage;       // mDNS core uses this to store its globals
static mDNS_PlatformSupport PlatformStorage;  // Stores this platform's globals
#define RR_CACHE_SIZE 500
static CacheEntity gRRCache[RR_CACHE_SIZE];
mDNSexport const char ProgramName[] = "mDNSIdentify";

static volatile int StopNow;    // 0 means running, 1 means stop because we got an answer, 2 means stop because of Ctrl-C
static volatile int NumAnswers, NumAddr, NumAAAA, NumHINFO;
static char hostname[MAX_ESCAPED_DOMAIN_NAME], hardware[256], software[256];
static mDNSAddr lastsrc, hostaddr, target;
static mDNSOpaque16 lastid, id;

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
// Main code

mDNSexport void mDNSCoreReceive(mDNS *const m, DNSMessage *const msg, const mDNSu8 *const end,
                                const mDNSAddr *const srcaddr, const mDNSIPPort srcport, const mDNSAddr *const dstaddr, const mDNSIPPort dstport,
                                const mDNSInterfaceID InterfaceID)
{
    (void)dstaddr; // Unused
    // Snag copy of header ID, then call through
    lastid = msg->h.id;
    lastsrc = *srcaddr;

    // We *want* to allow off-net unicast responses here.
    // For now, the simplest way to allow that is to pretend it was received via multicast so that mDNSCore doesn't reject the packet
    __MDNS__mDNSCoreReceive(m, msg, end, srcaddr, srcport, &AllDNSLinkGroup_v4, dstport, InterfaceID);
}

mDNSlocal void NameCallback(mDNS *const m, DNSQuestion *question, const ResourceRecord *const answer, QC_result AddRecord)
{
    (void)m;        // Unused
    (void)question; // Unused
    (void)AddRecord; // Unused
    if (!id.NotAnInteger) id = lastid;
    if (answer->rrtype == kDNSType_PTR || answer->rrtype == kDNSType_CNAME)
    {
        ConvertDomainNameToCString(&answer->rdata->u.name, hostname);
        StopNow = 1;
        mprintf("%##s %s %##s\n", answer->name->c, DNSTypeName(answer->rrtype), answer->rdata->u.name.c);
    }
}

mDNSlocal void InfoCallback(mDNS *const m, DNSQuestion *question, const ResourceRecord *const answer, QC_result AddRecord)
{
    (void)m;        // Unused
    (void)question; // Unused
    (void)AddRecord; // Unused
    if (answer->rrtype == kDNSType_A)
    {
        if (!id.NotAnInteger) id = lastid;
        NumAnswers++;
        NumAddr++;
        mprintf("%##s %s %.4a\n", answer->name->c, DNSTypeName(answer->rrtype), &answer->rdata->u.ipv4);
        hostaddr.type = mDNSAddrType_IPv4;  // Prefer v4 target to v6 target, for now
        hostaddr.ip.v4 = answer->rdata->u.ipv4;
    }
    else if (answer->rrtype == kDNSType_AAAA)
    {
        if (!id.NotAnInteger) id = lastid;
        NumAnswers++;
        NumAAAA++;
        mprintf("%##s %s %.16a\n", answer->name->c, DNSTypeName(answer->rrtype), &answer->rdata->u.ipv6);
        if (!hostaddr.type) // Prefer v4 target to v6 target, for now
        {
            hostaddr.type = mDNSAddrType_IPv6;
            hostaddr.ip.v6 = answer->rdata->u.ipv6;
        }
    }
    else if (answer->rrtype == kDNSType_HINFO)
    {
        mDNSu8 *p = answer->rdata->u.data;
        strncpy(hardware, (char*)(p+1), p[0]);
        hardware[p[0]] = 0;
        p += 1 + p[0];
        strncpy(software, (char*)(p+1), p[0]);
        software[p[0]] = 0;
        NumAnswers++;
        NumHINFO++;
    }

    // If we've got everything we're looking for, don't need to wait any more
    if (/*NumHINFO && */ (NumAddr || NumAAAA)) StopNow = 1;
}

mDNSlocal void ServicesCallback(mDNS *const m, DNSQuestion *question, const ResourceRecord *const answer, QC_result AddRecord)
{
    (void)m;        // Unused
    (void)question; // Unused
    (void)AddRecord; // Unused
    // Right now the mDNSCore targeted-query code is incomplete --
    // it issues targeted queries, but accepts answers from anywhere
    // For now, we'll just filter responses here so we don't get confused by responses from someone else
    if (answer->rrtype == kDNSType_PTR && mDNSSameAddress(&lastsrc, &target))
    {
        NumAnswers++;
        mprintf("%##s %s %##s\n", answer->name->c, DNSTypeName(answer->rrtype), answer->rdata->u.name.c);
    }
}

mDNSlocal void WaitForAnswer(mDNS *const m, int seconds)
{
    struct timeval end;
    gettimeofday(&end, NULL);
    end.tv_sec += seconds;
    StopNow = 0;
    NumAnswers = 0;
    while (!StopNow)
    {
        int nfds = 0;
        fd_set readfds, writefds;
        struct timeval now, remain = end;
        int result;

        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        gettimeofday(&now, NULL);
        if (remain.tv_usec < now.tv_usec) { remain.tv_usec += 1000000; remain.tv_sec--; }
        if (remain.tv_sec < now.tv_sec)
        {
            if (!NumAnswers) printf("No response after %d seconds\n", seconds);
            return;
        }
        remain.tv_usec -= now.tv_usec;
        remain.tv_sec  -= now.tv_sec;
        mDNSPosixGetFDSet(m, &nfds, &readfds, &writefds, &remain);
        result = select(nfds, &readfds, &writefds, NULL, &remain);
        if (result >= 0) mDNSPosixProcessFDSet(m, &readfds, &writefds);
        else if (errno != EINTR) StopNow = 2;
    }
}

mDNSlocal mStatus StartQuery(DNSQuestion *q, char *qname, mDNSu16 qtype, const mDNSAddr *target, mDNSQuestionCallback callback)
{
    lastsrc = zeroAddr;
    if (qname) MakeDomainNameFromDNSNameString(&q->qname, qname);
    q->InterfaceID      = mDNSInterface_Any;
    q->flags            = 0;
    q->Target           = target ? *target : zeroAddr;
    q->TargetPort       = MulticastDNSPort;
    q->TargetQID        = zeroID;
    q->qtype            = qtype;
    q->qclass           = kDNSClass_IN;
    q->LongLived        = mDNSfalse;
    q->ExpectUnique     = mDNSfalse;    // Don't want to stop after the first response packet
    q->ForceMCast       = mDNStrue;     // Query via multicast, even for apparently uDNS names like 1.1.1.17.in-addr.arpa.
    q->ReturnIntermed   = mDNStrue;
    q->SuppressUnusable = mDNSfalse;
    q->AppendSearchDomains = 0;
    q->TimeoutQuestion  = 0;
    q->ValidationRequired = 0;
    q->ValidatingResponse = 0;
    q->WakeOnResolve    = 0;
    q->UseBackgroundTraffic = mDNSfalse;
    q->ProxyQuestion    = 0;
    q->pid              = mDNSPlatformGetPID();
    q->QuestionCallback = callback;
    q->QuestionContext  = NULL;

    //mprintf("%##s %s ?\n", q->qname.c, DNSTypeName(qtype));
    return(mDNS_StartQuery(&mDNSStorage, q));
}

mDNSlocal void DoOneQuery(DNSQuestion *q, char *qname, mDNSu16 qtype, const mDNSAddr *target, mDNSQuestionCallback callback)
{
    mStatus status = StartQuery(q, qname, qtype, target, callback);
    if (status != mStatus_NoError)
        StopNow = 2;
    else
    {
        WaitForAnswer(&mDNSStorage, 4);
        mDNS_StopQuery(&mDNSStorage, q);
    }
}

mDNSlocal int DoQuery(DNSQuestion *q, char *qname, mDNSu16 qtype, const mDNSAddr *target, mDNSQuestionCallback callback)
{
    DoOneQuery(q, qname, qtype, target, callback);
    if (StopNow == 0 && NumAnswers == 0 && target && target->type)
    {
        mprintf("%##s %s Trying multicast\n", q->qname.c, DNSTypeName(q->qtype));
        DoOneQuery(q, qname, qtype, NULL, callback);
    }
    if (StopNow == 0 && NumAnswers == 0)
        mprintf("%##s %s *** No Answer ***\n", q->qname.c, DNSTypeName(q->qtype));
    return(StopNow);
}

mDNSlocal void HandleSIG(int signal)
{
    (void)signal;   // Unused
    debugf("%s","");
    debugf("HandleSIG");
    StopNow = 2;
}

mDNSexport int main(int argc, char **argv)
{
    const char *progname = strrchr(argv[0], '/') ? strrchr(argv[0], '/') + 1 : argv[0];
    int this_arg = 1;
    mStatus status;
    struct in_addr s4;
#if HAVE_IPV6
    struct in6_addr s6;
#endif
    char buffer[256];
    DNSQuestion q;

    if (argc < 2) goto usage;

    // Since this is a special command-line tool, we want LogMsg() errors to go to stderr, not syslog
    mDNS_DebugMode = mDNStrue;

    // Initialise the mDNS core.
    status = mDNS_Init(&mDNSStorage, &PlatformStorage,
                       gRRCache, RR_CACHE_SIZE,
                       mDNS_Init_DontAdvertiseLocalAddresses,
                       mDNS_Init_NoInitCallback, mDNS_Init_NoInitCallbackContext);
    if (status) { fprintf(stderr, "Daemon start: mDNS_Init failed %d\n", (int)status); return(status); }

    signal(SIGINT, HandleSIG);  // SIGINT is what you get for a Ctrl-C
    signal(SIGTERM, HandleSIG);

    while (this_arg < argc)
    {
        char *arg = argv[this_arg++];
        if (this_arg > 2) printf("\n");

        lastid = id = zeroID;
        hostaddr = target = zeroAddr;
        hostname[0] = hardware[0] = software[0] = 0;
        NumAddr = NumAAAA = NumHINFO = 0;

        if (inet_pton(AF_INET, arg, &s4) == 1)
        {
            mDNSu8 *p = (mDNSu8 *)&s4;
            // Note: This is reverse order compared to a normal dotted-decimal IP address, so we can't use our customary "%.4a" format code
            mDNS_snprintf(buffer, sizeof(buffer), "%d.%d.%d.%d.in-addr.arpa.", p[3], p[2], p[1], p[0]);
            printf("%s\n", buffer);
            target.type = mDNSAddrType_IPv4;
            target.ip.v4.NotAnInteger = s4.s_addr;
            DoQuery(&q, buffer, kDNSType_PTR, &target, NameCallback);
            if (StopNow == 2) break;
        }
#if HAVE_IPV6
        else if (inet_pton(AF_INET6, arg, &s6) == 1)
        {
            int i;
            mDNSu8 *p = (mDNSu8 *)&s6;
            for (i = 0; i < 16; i++)
            {
                static const char hexValues[] = "0123456789ABCDEF";
                buffer[i * 4    ] = hexValues[p[15-i] & 0x0F];
                buffer[i * 4 + 1] = '.';
                buffer[i * 4 + 2] = hexValues[p[15-i] >> 4];
                buffer[i * 4 + 3] = '.';
            }
            mDNS_snprintf(&buffer[64], sizeof(buffer)-64, "ip6.arpa.");
            target.type = mDNSAddrType_IPv6;
            mDNSPlatformMemCopy(&target.ip.v6, &s6, sizeof(target.ip.v6));
            DoQuery(&q, buffer, kDNSType_PTR, &target, NameCallback);
            if (StopNow == 2) break;
        }
#endif
        else
        {
            if (strlen(arg) >= sizeof(hostname))
            {
                fprintf(stderr, "hostname must be < %d characters\n", (int)sizeof(hostname));
                goto usage;
            }
            strcpy(hostname, arg);
        }

        // Now we have the host name; get its A, AAAA, and HINFO
        if (hostname[0]) DoQuery(&q, hostname, kDNSQType_ANY, &target, InfoCallback);
        if (StopNow == 2) break;

        if (hardware[0] || software[0])
        {
            printf("HINFO Hardware: %s\n", hardware);
            printf("HINFO Software: %s\n", software);
        }
        else if (NumAnswers) printf("%s has no HINFO record\n", hostname);
        else printf("Incorrect dot-local hostname, address, or no mDNSResponder running on that machine\n");

        if (NumAnswers)
        {
            // Because of the way we use lastsrc in ServicesCallback, we need to clear the cache to make sure we're getting fresh answers
            mDNS *const m = &mDNSStorage;
            mDNSu32 slot;
            CacheGroup *cg;
            CacheRecord *rr;
            FORALL_CACHERECORDS(slot, cg, rr)
            {
                mDNS_PurgeCacheResourceRecord(m, rr);
            }
            if (target.type == 0) target = hostaddr;        // Make sure the services query is targeted
            DoQuery(&q, "_services._dns-sd._udp.local.", kDNSType_PTR, &target, ServicesCallback);
            if (StopNow == 2) break;
        }
    }

    mDNS_Close(&mDNSStorage);
    return(0);

usage:
    fprintf(stderr, "Usage: %s <dot-local hostname> or <IPv4 address> or <IPv6 address> ...\n", progname);
    return(-1);
}
