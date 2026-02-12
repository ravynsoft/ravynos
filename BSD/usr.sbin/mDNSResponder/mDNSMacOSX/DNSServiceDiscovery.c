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
 */

#include "../mDNSMacOSX/DNSServiceDiscovery.h"
#include "DNSServiceDiscoveryDefines.h"

#include <stdlib.h>
#include <stdio.h>
#include <servers/bootstrap.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <pthread.h>

#include <netinet/in.h>

extern boolean_t DNSServiceDiscoveryReply_server(
    mach_msg_header_t *InHeadP,
    mach_msg_header_t *OutHeadP);

extern
kern_return_t DNSServiceBrowserCreate_rpc
(
    mach_port_t server,
    mach_port_t client,
    DNSCString regtype,
    DNSCString domain
);


extern
kern_return_t DNSServiceResolverResolve_rpc
(
    mach_port_t server,
    mach_port_t client,
    DNSCString name,
    DNSCString regtype,
    DNSCString domain
);


struct a_requests {
    struct a_requests       *next;
    mach_port_t client_port;
    union {
        DNSServiceBrowserReply browserCallback;
        DNSServiceDomainEnumerationReply enumCallback;
        DNSServiceRegistrationReply regCallback;
        DNSServiceResolverReply resolveCallback;
    } callout;
    void                    *context;
};

typedef struct _dns_service_discovery_t {
    mach_port_t port;
} dns_service_discovery_t;


dns_service_discovery_ref DNSServiceBrowserCreate (const char *regtype, const char *domain, DNSServiceBrowserReply callBack,void *context)
{
    
    (void) regtype;          // Unused
    (void) domain;           // Unused
    (void) callBack;         // Unused
    (void) context;          // Unused
    
    printf("DNSServiceBrowserCreate deprecated since 10.3 \n");
    return NULL;
    
}

dns_service_discovery_ref DNSServiceResolverResolve(const char *name, const char *regtype, const char *domain, DNSServiceResolverReply callBack, void *context)
{
    (void) name;             // Unused
    (void) regtype;          // Unused
    (void) domain;           // Unused
    (void) callBack;         // Unused
    (void) context;          // Unused
    
    printf("DNSServiceResolverResolve deprecated since 10.3 \n");
    return NULL;

}

void DNSServiceDiscovery_handleReply(void *replyMsg)
{
    (void) replyMsg;             // Unused
    printf("DNSServiceDiscovery_handleReply deprecated since 10.3 \n");
}

mach_port_t DNSServiceDiscoveryMachPort(dns_service_discovery_ref dnsServiceDiscovery)
{
    printf("DNSServiceDiscoveryMachPort deprecated since 10.3 \n");
    return dnsServiceDiscovery->port;
}

void DNSServiceDiscoveryDeallocate(dns_service_discovery_ref dnsServiceDiscovery)
{
    (void) dnsServiceDiscovery;             // Unused
    printf("DNSServiceDiscoveryDeallocate deprecated since 10.3 \n");
}
