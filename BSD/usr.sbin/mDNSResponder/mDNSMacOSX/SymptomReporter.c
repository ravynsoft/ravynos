/*
 * Copyright (c) 2015-2019 Apple Inc. All rights reserved.
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

#include "SymptomReporter.h"

#include <arpa/inet.h>
#include <dlfcn.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <AvailabilityMacros.h>
#include <SymptomReporter/SymptomReporter.h>

#define SYMPTOM_REPORTER_mDNSResponder_NUMERIC_ID  101
#define SYMPTOM_REPORTER_mDNSResponder_TEXT_ID     "com.apple.mDNSResponder"

#define SYMPTOM_DNS_NO_REPLIES          0x00065001
#define SYMPTOM_DNS_RESUMED_RESPONDING  0x00065002

mDNSu32 NumUnreachableDNSServers;

static symptom_framework_t symptomReporter = mDNSNULL;
static symptom_framework_t (*symptom_framework_init_f)(symptom_ident_t id, const char *originator_string) = mDNSNULL;
static symptom_t (*symptom_new_f)(symptom_framework_t framework, symptom_ident_t id) = mDNSNULL;
static int (*symptom_set_additional_qualifier_f)(symptom_t symptom, uint32_t qualifier_type, size_t qualifier_len, void *qualifier_data) = mDNSNULL;
static int (*symptom_send_f)(symptom_t symptom) = mDNSNULL;

mDNSlocal mStatus SymptomReporterInitCheck(void)
{
    mStatus err;
    static mDNSBool triedInit = mDNSfalse;
    static void *symptomReporterLib = mDNSNULL;
    static const char path[] = "/System/Library/PrivateFrameworks/SymptomReporter.framework/SymptomReporter";

    if (!triedInit)
    {
        triedInit = mDNStrue;

        symptomReporterLib = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
        if (!symptomReporterLib)
            goto exit;

        symptom_framework_init_f = dlsym(symptomReporterLib, "symptom_framework_init");
        if (!symptom_framework_init_f)
            goto exit;

        symptom_new_f = dlsym(symptomReporterLib, "symptom_new");
        if (!symptom_new_f)
            goto exit;

        symptom_set_additional_qualifier_f = dlsym(symptomReporterLib, "symptom_set_additional_qualifier");
        if (!symptom_set_additional_qualifier_f)
            goto exit;

        symptom_send_f = dlsym(symptomReporterLib, "symptom_send");
        if (!symptom_send_f)
            goto exit;

        symptomReporter = symptom_framework_init_f(SYMPTOM_REPORTER_mDNSResponder_NUMERIC_ID, SYMPTOM_REPORTER_mDNSResponder_TEXT_ID);
    }

exit:
    err = symptomReporter ? mStatus_NoError : mStatus_NotInitializedErr;
    return err;
}

mDNSlocal mStatus SymptomReporterReportDNSReachability(const mDNSAddr *addr, mDNSBool isReachable)
{
    mStatus err;
    symptom_t symptom;
    struct sockaddr_storage sockAddr;
    size_t sockAddrSize;

    LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_INFO,
        "SymptomReporterReportDNSReachability: DNS server " PRI_IP_ADDR " is " PUB_S "reachable", addr, isReachable ? "" : "un");

    if (addr->type == mDNSAddrType_IPv4)
    {
        struct sockaddr_in *sin = (struct sockaddr_in *)&sockAddr;
        sockAddrSize = sizeof(*sin);
        mDNSPlatformMemZero(sin, sockAddrSize);
        sin->sin_len = sockAddrSize;
        sin->sin_family = AF_INET;
        sin->sin_addr.s_addr = addr->ip.v4.NotAnInteger;
    }
    else if (addr->type == mDNSAddrType_IPv6)
    {
        struct sockaddr_in6* sin6 = (struct sockaddr_in6*)&sockAddr;
        sockAddrSize = sizeof(*sin6);
        mDNSPlatformMemZero(sin6, sockAddrSize);
        sin6->sin6_len = sockAddrSize;
        sin6->sin6_family = AF_INET6;
        sin6->sin6_addr = *(struct in6_addr *)&addr->ip.v6;
    }
    else
    {
        LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_ERROR,
            "SymptomReporterReportDNSReachability: addr is not an IPv4 or IPv6 address!");
        err = mStatus_BadParamErr;
        goto exit;
    }

    symptom = symptom_new_f(symptomReporter, isReachable ? SYMPTOM_DNS_RESUMED_RESPONDING : SYMPTOM_DNS_NO_REPLIES);
    symptom_set_additional_qualifier_f(symptom, 1, sockAddrSize, (void *)&sockAddr);
    symptom_send_f(symptom);
    err = mStatus_NoError;

exit:
    return err;
}

mDNSexport mStatus SymptomReporterDNSServerReachable(mDNS *const m, const mDNSAddr *addr)
{
    mStatus err;
    DNSServer *s;
    mDNSBool found = mDNSfalse;

    err = SymptomReporterInitCheck();
    if (err != mStatus_NoError)
        goto exit;

    for (s = m->DNSServers; s; s = s->next)
    {
        if (s->flags & DNSServerFlag_Delete)
            continue;
        if ((s->flags & DNSServerFlag_Unreachable) && mDNSSameAddress(addr, &s->addr))
        {
            s->flags &= ~DNSServerFlag_Unreachable;
            NumUnreachableDNSServers--;
            found = mDNStrue;
        }
    }

    if (!found)
    {
        err = mStatus_NoSuchNameErr;
        goto exit;
    }

    err = SymptomReporterReportDNSReachability(addr, mDNStrue);

exit:
    return err;
}

mDNSexport mStatus SymptomReporterDNSServerUnreachable(DNSServer *s)
{
    mStatus err;

    err = SymptomReporterInitCheck();
    if (err != mStatus_NoError)
        goto exit;

    if ((s->flags & DNSServerFlag_Delete) || (s->flags & DNSServerFlag_Unreachable))
        goto exit;

    s->flags |= DNSServerFlag_Unreachable;
    NumUnreachableDNSServers++;

    err = SymptomReporterReportDNSReachability(&s->addr, mDNSfalse);

exit:
    return err;
}
