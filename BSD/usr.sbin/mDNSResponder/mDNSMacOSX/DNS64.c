/*
 * Copyright (c) 2017-2019 Apple Inc. All rights reserved.
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

#include "DNS64.h"

#if MDNSRESPONDER_SUPPORTS(APPLE, DNS64)

#include <AssertMacros.h>
#include <nw/private.h>
#include <stdlib.h>
#include <string.h>

#include "dns_sd.h"
#include "dns_sd_internal.h"
#include "mDNSMacOSX.h"
#include "uDNS.h"

//===========================================================================================================================
//  Constants
//===========================================================================================================================

#define kDNS64IPv4OnlyFQDNString    "\x8" "ipv4only" "\x4" "arpa"
#define kDNS64IPv4OnlyFQDN          ((const domainname *) kDNS64IPv4OnlyFQDNString)
#define kDNS64IPv4OnlyFQDNLength    15  // 9 bytes for first label, 5 bytes for second label, and 1 byte for the root label.

#define sizeof_field(TYPE, FIELD)   sizeof(((TYPE *)0)->FIELD)    // From CoreUtils.h

check_compile_time(sizeof(kDNS64IPv4OnlyFQDNString) == kDNS64IPv4OnlyFQDNLength);
check_compile_time(sizeof_field(DNSQuestion, qname) >= kDNS64IPv4OnlyFQDNLength);
check_compile_time(sizeof_field(DNS64, qnameStash)  == kDNS64IPv4OnlyFQDNLength);

//===========================================================================================================================
//  Local Prototypes
//===========================================================================================================================

mDNSlocal mStatus   _DNS64GetIPv6Addrs(mDNS *m, mDNSu32 inResGroupID, struct in6_addr **outAddrs, uint32_t *outAddrCount);
mDNSlocal mStatus   _DNS64GetPrefixes(mDNS *m, mDNSu32 inResGroupID, nw_nat64_prefix_t **outPrefixes, uint32_t *outPrefixCount);
mDNSlocal mDNSBool  _DNS64GetReverseIPv6Addr(const domainname *inQName, struct in6_addr *outAddr);
mDNSlocal mDNSu32   _DNS64IPv4OnlyFQDNHash(void);
mDNSlocal void      _DNS64RestartQuestion(mDNS *m, DNSQuestion *q, DNS64State newState);
mDNSlocal mDNSBool  _DNS64InterfaceSupportsNAT64(uint32_t inIfIndex);
mDNSlocal mDNSBool  _DNS64TestIPv6Synthesis(mDNS *m, mDNSu32 inResGroupID, const mDNSv4Addr *inV4Addr);

//===========================================================================================================================
//  DNS64StateMachine
//===========================================================================================================================

mDNSexport mDNSBool DNS64StateMachine(mDNS *m, DNSQuestion *inQ, const ResourceRecord *inRR, QC_result inResult)
{
    // If this is an mDNS question, then exit early. DNS64 is only for unicast DNS questions.

    if (mDNSOpaque16IsZero(inQ->TargetQID)) return (mDNSfalse);

    switch (inQ->dns64.state)
    {
    // If this question is going to be answered with a negative AAAA record and the question is not for "ipv4only.arpa." and
    // the question's DNS server's interface supports NAT64, then restart the question as an "ipv4only.arpa." AAAA question.
    // Otherwise, do nothing.

    case kDNS64State_Initial:
        if ((inRR->RecordType == kDNSRecordTypePacketNegative) && (inResult == QC_add))
        {
            if ((inQ->qtype      == kDNSType_AAAA) &&
                (inRR->rrtype    == kDNSType_AAAA) &&
                (inRR->rrclass   == kDNSClass_IN) &&
                ((inQ->qnamehash != _DNS64IPv4OnlyFQDNHash()) || !SameDomainName(&inQ->qname, kDNS64IPv4OnlyFQDN)) &&
                inQ->qDNSServer &&
                _DNS64InterfaceSupportsNAT64((uint32_t)((uintptr_t)inQ->qDNSServer->interface)))
            {
                _DNS64RestartQuestion(m, inQ, kDNS64State_PrefixDiscovery);
                return (mDNStrue);
            }
            else if ((inQ->qtype == kDNSType_PTR) &&
                (inRR->rrtype    == kDNSType_PTR) &&
                (inRR->rrclass   == kDNSClass_IN) &&
                inQ->qDNSServer &&
                _DNS64InterfaceSupportsNAT64((uint32_t)((uintptr_t)inQ->qDNSServer->interface)) &&
                _DNS64GetReverseIPv6Addr(&inQ->qname, NULL))
            {
                _DNS64RestartQuestion(m, inQ, kDNS64State_PrefixDiscoveryPTR);
                return (mDNStrue);
            }
        }
        break;

    // If the "ipv4only.arpa." question is going to be answered with a positive AAAA record, then restart it as a question
    // for an A record with the original AAAA qname.
    // Otherwise, restart the question for the original AAAA record.

    case kDNS64State_PrefixDiscovery:
        if ((inRR->RecordType != kDNSRecordTypePacketNegative) &&
            (inResult         == QC_add) &&
            (inRR->rrtype     == kDNSType_AAAA) &&
            (inRR->rrclass    == kDNSClass_IN))
        {
            _DNS64RestartQuestion(m, inQ, kDNS64State_QueryA);
            return (mDNStrue);
        }
        else
        {
            _DNS64RestartQuestion(m, inQ, kDNS64State_QueryAAAA);
            return (mDNStrue);
        }
        break;

    // The "ipv4only.arpa." question is going to be answered. Restart the question now. DNS64HandleNewQuestion() will decide
    // whether or not to change it to a reverse IPv4 question.

    case kDNS64State_PrefixDiscoveryPTR:
        _DNS64RestartQuestion(m, inQ, kDNS64State_QueryPTR);
        return (mDNStrue);
        break;

    // If this question is going to be answered with a CNAME, then do nothing.
    // If this question is going to be answered with a positive A record that's synthesizable, then set the state to
    // QueryARecord2.
    // Otherwise, restart the question for the original AAAA record.

    case kDNS64State_QueryA:
        if (inRR->rrtype != kDNSType_CNAME)
        {
            if ((inRR->RecordType != kDNSRecordTypePacketNegative) &&
                (inResult         == QC_add) &&
                (inRR->rrtype     == kDNSType_A) &&
                (inRR->rrclass    == kDNSClass_IN) &&
                inQ->qDNSServer &&
                _DNS64TestIPv6Synthesis(m, inQ->qDNSServer->resGroupID, &inRR->rdata->u.ipv4))
            {
                inQ->dns64.state = kDNS64State_QueryA2;
            }
            else
            {
                _DNS64RestartQuestion(m, inQ, kDNS64State_QueryAAAA);
                return (mDNStrue);
            }
        }
        break;

    // For all other states, do nothing.

    case kDNS64State_QueryA2:
    case kDNS64State_QueryAAAA:
    case kDNS64State_QueryPTR:
    case kDNS64State_ReverseIPv4:
    case kDNS64State_ReverseIPv6:
        break;

    default:
        LogMsg("DNS64StateMachine: unrecognized DNS64 state %d", inQ->dns64.state);
        break;
    }

    return (mDNSfalse);
}

//===========================================================================================================================
//  DNS64AnswerCurrentQuestion
//===========================================================================================================================

mDNSexport mStatus DNS64AnswerCurrentQuestion(mDNS *m, const ResourceRecord *inRR, QC_result inResult)
{
    mStatus                 err;
    ResourceRecord          newRR;
    RData                   rdata;
    nw_nat64_prefix_t *     prefixes = NULL;
    uint32_t                prefixCount;
    uint32_t                i;
    struct in_addr          v4Addr;
    struct in6_addr         synthV6;
    DNSQuestion * const     q = m->CurrentQuestion;

    require_action_quiet(q->qDNSServer, exit, err = mStatus_BadParamErr);

    err = _DNS64GetPrefixes(m, q->qDNSServer->resGroupID, &prefixes, &prefixCount);
    require_noerr_quiet(err, exit);

    newRR               = *inRR;
    newRR.rrtype        = kDNSType_AAAA;
    newRR.rdlength      = 16;
    rdata.MaxRDLength   = newRR.rdlength;
    newRR.rdata         = &rdata;

    memcpy(&v4Addr.s_addr, inRR->rdata->u.ipv4.b, 4);
    for (i = 0; i < prefixCount; i++)
    {
        if (nw_nat64_synthesize_v6(&prefixes[i], &v4Addr, &synthV6))
        {
            memcpy(rdata.u.ipv6.b, synthV6.s6_addr, 16);
            q->QuestionCallback(m, q, &newRR, inResult);
            if (m->CurrentQuestion != q) break;
        }
    }
    err = mStatus_NoError;

exit:
    if (prefixes) free(prefixes);
    return (err);
}

//===========================================================================================================================
//  DNS64HandleNewQuestion
//===========================================================================================================================

mDNSexport void DNS64HandleNewQuestion(mDNS *m, DNSQuestion *inQ)
{
    if (inQ->dns64.state == kDNS64State_QueryPTR)
    {
        struct in6_addr     v6Addr;

        inQ->dns64.state = kDNS64State_ReverseIPv6;
        if (inQ->qDNSServer && _DNS64GetReverseIPv6Addr(&inQ->qname, &v6Addr))
        {
            mStatus                 err;
            nw_nat64_prefix_t *     prefixes;
            uint32_t                prefixCount;
            uint32_t                i;
            struct in_addr          v4Addr;
            char                    qnameStr[MAX_REVERSE_MAPPING_NAME_V4];

            err = _DNS64GetPrefixes(m, inQ->qDNSServer->resGroupID, &prefixes, &prefixCount);
            require_noerr_quiet(err, exit);

            for (i = 0; i < prefixCount; i++)
            {
                if (nw_nat64_extract_v4(&prefixes[i], &v6Addr, &v4Addr))
                {
                    const mDNSu8 * const        a = (const mDNSu8 *)&v4Addr.s_addr;

                    snprintf(qnameStr, sizeof(qnameStr), "%u.%u.%u.%u.in-addr.arpa.", a[3], a[2], a[1], a[0]);
                    MakeDomainNameFromDNSNameString(&inQ->qname, qnameStr);
                    inQ->qnamehash   = DomainNameHashValue(&inQ->qname);
                    inQ->dns64.state = kDNS64State_ReverseIPv4;
                    break;
                }
            }
            free(prefixes);
        }
    }

exit:
    return;
}

//===========================================================================================================================
//  DNS64ResetState
//===========================================================================================================================

// Called from mDNS_StopQuery_internal().

mDNSexport void DNS64ResetState(DNSQuestion *inQ)
{
    switch (inQ->dns64.state)
    {
    case kDNS64State_PrefixDiscoveryPTR:
        inQ->qtype = kDNSType_PTR;  // Restore qtype to PTR and fall through.

    case kDNS64State_PrefixDiscovery:
        memcpy(&inQ->qname, inQ->dns64.qnameStash, sizeof(inQ->dns64.qnameStash));  // Restore the previous qname.
        inQ->qnamehash = DomainNameHashValue(&inQ->qname);
        break;

    case kDNS64State_QueryA:
    case kDNS64State_QueryA2:
        inQ->qtype = kDNSType_AAAA; // Restore qtype to AAAA.
        break;

    // Do nothing for the other states.

    case kDNS64State_Initial:
    case kDNS64State_QueryAAAA:
    case kDNS64State_QueryPTR:
    case kDNS64State_ReverseIPv4:
    case kDNS64State_ReverseIPv6:
        break;

    default:
        LogMsg("DNS64ResetState: unrecognized DNS64 state %d", inQ->dns64.state);
        break;
    }
    inQ->dns64.state = kDNS64State_Initial;
}

//===========================================================================================================================
//  DNS64RestartQuestions
//===========================================================================================================================

mDNSexport void DNS64RestartQuestions(mDNS *m)
{
    DNSQuestion *       q;
    DNSQuestion *       restartList = NULL;
    DNSServer *         newServer;

    m->RestartQuestion = m->Questions;
    while (m->RestartQuestion)
    {
        q = m->RestartQuestion;
        m->RestartQuestion = q->next;
        if (q->dns64.state != kDNS64State_Initial)
        {
            SetValidDNSServers(m, q);
            q->triedAllServersOnce = mDNSfalse;
            newServer = GetServerForQuestion(m, q);
            if (q->qDNSServer != newServer)
            {
                if (!CacheRecordRmvEventsForQuestion(m, q))
                {
                    LogInfo("DNS64RestartQuestions: Question deleted while delivering RMV events from cache");
                }
                else
                {
                    LogInfo("DNS64RestartQuestions: Stop question %p %##s (%s)", q, q->qname.c, DNSTypeName(q->qtype));
                    mDNS_StopQuery_internal(m, q);
                    q->next = restartList;
                    restartList = q;
                }
            }
        }
    }
    while ((q = restartList) != NULL)
    {
        restartList = restartList->next;
        q->next = NULL;
        LogInfo("DNS64RestartQuestions: Start question %p %##s (%s)", q, q->qname.c, DNSTypeName(q->qtype));
        mDNS_StartQuery_internal(m, q);
    }
}

//===========================================================================================================================
//  _DNS64GetIPv6Addrs
//===========================================================================================================================

#define IsPositiveAAAAFromResGroup(RR, RES_GROUP_ID)        \
    ((RR)->rDNSServer &&                                    \
    ((RR)->rDNSServer->resGroupID == RES_GROUP_ID) &&       \
    ((RR)->rrtype == kDNSType_AAAA) &&                      \
    ((RR)->RecordType != kDNSRecordTypePacketNegative) &&   \
    !(RR)->InterfaceID)

mDNSlocal mStatus _DNS64GetIPv6Addrs(mDNS *m, const mDNSu32 inResGroupID, struct in6_addr **outAddrs, uint32_t *outAddrCount)
{
    mStatus                 err;
    const CacheGroup *      cg;
    const CacheRecord *     cr;
    struct in6_addr *       addrs = NULL;
    uint32_t                addrCount;
    uint32_t                recordCount;

    cg = CacheGroupForName(m, _DNS64IPv4OnlyFQDNHash(), kDNS64IPv4OnlyFQDN);
    require_action_quiet(cg, exit, err = mStatus_NoSuchRecord);

    recordCount = 0;
    for (cr = cg->members; cr; cr = cr->next)
    {
        if (IsPositiveAAAAFromResGroup(&cr->resrec, inResGroupID))
        {
            recordCount++;
        }
    }
    require_action_quiet(recordCount > 0, exit, err = mStatus_NoSuchRecord);

    addrs = (struct in6_addr *)calloc(recordCount, sizeof(*addrs));
    require_action_quiet(addrs, exit, err = mStatus_NoMemoryErr);

    addrCount = 0;
    for (cr = cg->members; cr && (addrCount < recordCount); cr = cr->next)
    {
        if (IsPositiveAAAAFromResGroup(&cr->resrec, inResGroupID))
        {
            memcpy(addrs[addrCount].s6_addr, cr->resrec.rdata->u.ipv6.b, 16);
            addrCount++;
        }
    }

    *outAddrs = addrs;
    addrs = NULL;
    *outAddrCount = addrCount;
    err = mStatus_NoError;

exit:
    if (addrs) free(addrs);
    return (err);
}

//===========================================================================================================================
//  _DNS64GetPrefixes
//===========================================================================================================================

mDNSlocal mStatus _DNS64GetPrefixes(mDNS *m, mDNSu32 inResGroupID, nw_nat64_prefix_t **outPrefixes, uint32_t *outPrefixCount)
{
    mStatus                 err;
    struct in6_addr *       v6Addrs;
    uint32_t                v6AddrCount;
    nw_nat64_prefix_t *     prefixes;
    int32_t                 prefixCount;

    err = _DNS64GetIPv6Addrs(m, inResGroupID, &v6Addrs, &v6AddrCount);
    require_noerr_quiet(err, exit);

    prefixCount = nw_nat64_copy_prefixes_from_ipv4only_records(v6Addrs, v6AddrCount, &prefixes);
    free(v6Addrs);
    require_action_quiet(prefixCount > 0, exit, err = mStatus_UnknownErr);

    *outPrefixes    = prefixes;
    *outPrefixCount = prefixCount;

exit:
    return (err);
}

//===========================================================================================================================
//  _DNS64GetReverseIPv6Addr
//===========================================================================================================================

#define kReverseIPv6Domain  ((const domainname *) "\x3" "ip6" "\x4" "arpa")

mDNSlocal mDNSBool _DNS64GetReverseIPv6Addr(const domainname *inQName, struct in6_addr *outAddr)
{
    const mDNSu8 *      ptr;
    int                 i;
    unsigned int        c;
    unsigned int        nl;
    unsigned int        nu;

    // If the name is of the form "x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.ip6.arpa.", where each x
    // is a hex digit, then the sequence of 32 hex digit labels represents the nibbles of an IPv6 address in reverse order.
    // See <https://tools.ietf.org/html/rfc3596#section-2.5>.

    ptr = (const mDNSu8 *)inQName;
    for (i = 0; i < 16; i++)
    {
        if (*ptr++ != 1) return (mDNSfalse);                    // If this label's length is not 1, then fail.
        c = *ptr++;                                             // Get label byte.
        if (     (c >= '0') && (c <= '9')) nl =  c - '0';       // If it's a hex digit, get its numeric value.
        else if ((c >= 'a') && (c <= 'f')) nl = (c - 'a') + 10;
        else if ((c >= 'A') && (c <= 'F')) nl = (c - 'A') + 10;
        else                               return (mDNSfalse);  // Otherwise, fail.

        if (*ptr++ != 1) return (mDNSfalse);                    // If this label's length is not 1, then fail.
        c = *ptr++;                                             // Get label byte.
        if (     (c >= '0') && (c <= '9')) nu =  c - '0';       // If it's a hex digit, get its numeric value.
        else if ((c >= 'a') && (c <= 'f')) nu = (c - 'a') + 10;
        else if ((c >= 'A') && (c <= 'F')) nu = (c - 'A') + 10;
        else                               return (mDNSfalse);  // Otherwise, fail.

        if (outAddr) outAddr->s6_addr[15 - i] = (mDNSu8)((nu << 4) | nl);
    }

    // The rest of the name needs to be "ip6.arpa.". If it isn't, fail.

    if (!SameDomainName((const domainname *)ptr, kReverseIPv6Domain)) return (mDNSfalse);

    return (mDNStrue);
}

//===========================================================================================================================
//  _DNS64IPv4OnlyFQDNHash
//===========================================================================================================================

mDNSlocal mDNSu32 _DNS64IPv4OnlyFQDNHash(void)
{
    static dispatch_once_t      sHashOnce;
    static mDNSu32              sHash;

    dispatch_once(&sHashOnce, ^{ sHash = DomainNameHashValue(kDNS64IPv4OnlyFQDN); });

    return (sHash);
}

//===========================================================================================================================
//  _DNS64RestartQuestion
//===========================================================================================================================

mDNSlocal void _DNS64RestartQuestion(mDNS *const m, DNSQuestion *inQ, DNS64State inNewState)
{
    mDNS_StopQuery_internal(m, inQ);

    inQ->dns64.state = inNewState;
    switch (inQ->dns64.state)
    {
    case kDNS64State_Initial:
        break;

    case kDNS64State_PrefixDiscovery:
    case kDNS64State_PrefixDiscoveryPTR:
        // Save the first 15 bytes from the original qname that are displaced by setting qname to "ipv4only.arpa.".

        memcpy(inQ->dns64.qnameStash, &inQ->qname, sizeof(inQ->dns64.qnameStash));
        AssignDomainName(&inQ->qname, kDNS64IPv4OnlyFQDN);
        inQ->qnamehash = _DNS64IPv4OnlyFQDNHash();
        inQ->qtype = kDNSType_AAAA;
        break;

    case kDNS64State_QueryA:
    case kDNS64State_QueryA2:
        inQ->qtype = kDNSType_A;
        break;

    case kDNS64State_QueryPTR:
    case kDNS64State_ReverseIPv4:
    case kDNS64State_ReverseIPv6:
        inQ->qtype = kDNSType_PTR;
        break;

    case kDNS64State_QueryAAAA:
        inQ->qtype = kDNSType_AAAA;
        break;

    default:
        LogMsg("DNS64RestartQuestion: unrecognized DNS64 state %d", inQ->dns64.state);
        break;
    }

    mDNS_StartQuery_internal(m, inQ);
}

//===========================================================================================================================
//  _DNS64InterfaceSupportsNAT64
//===========================================================================================================================

mDNSlocal mDNSBool _DNS64InterfaceSupportsNAT64(uint32_t inIfIndex)
{
    mdns_interface_monitor_t monitor = GetInterfaceMonitorForIndex(inIfIndex);
    if (monitor && !mdns_interface_monitor_has_ipv4_connectivity(monitor) &&
        mdns_interface_monitor_has_ipv6_connectivity(monitor))
    {
        return (mDNStrue);
    }
    return (mDNSfalse);
}

//===========================================================================================================================
//  _DNS64TestIPv6Synthesis
//===========================================================================================================================

mDNSlocal mDNSBool _DNS64TestIPv6Synthesis(mDNS *m, mDNSu32 inResGroupID, const mDNSv4Addr *inV4Addr)
{
    mStatus                 err;
    nw_nat64_prefix_t *     prefixes    = NULL;
    uint32_t                prefixCount;
    uint32_t                i;
    struct in_addr          v4Addr;
    struct in6_addr         synthV6;
    mDNSBool                result      = mDNSfalse;

    err = _DNS64GetPrefixes(m, inResGroupID, &prefixes, &prefixCount);
    require_noerr_quiet(err, exit);

    memcpy(&v4Addr.s_addr, inV4Addr->b, 4);
    for (i = 0; i < prefixCount; i++)
    {
        if (nw_nat64_synthesize_v6(&prefixes[i], &v4Addr, &synthV6))
        {
            result = mDNStrue;
            break;
        }
    }

exit:
    if (prefixes) free(prefixes);
    return (result);
}
#endif  // MDNSRESPONDER_SUPPORTS(APPLE, DNS64)
