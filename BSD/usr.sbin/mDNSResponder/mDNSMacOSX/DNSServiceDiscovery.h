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

/*!	@header		DNS Service Discovery (Deprecated Mach-based API)
 *
 * @discussion  Note that this API was deprecated in Mac OS X 10.3, and replaced
 *				by the portable cross-platform /usr/include/dns_sd.h API.
 */

#ifndef __DNS_SERVICE_DISCOVERY_H
#define __DNS_SERVICE_DISCOVERY_H

#include <mach/mach_types.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/cdefs.h>

#include <netinet/in.h>
#include <os/availability.h>

#define kDNSServiceDiscoveryDeprecatedMsg "This API was deprecated in Mac OS X 10.3 and replaced by the portable cross-platform /usr/include/dns_sd.h API"

__BEGIN_DECLS

/* Opaque internal data type */
typedef struct _dns_service_discovery_t * dns_service_discovery_ref;

/* possible reply flags values */

enum {
    kDNSServiceDiscoveryNoFlags         = 0,
    kDNSServiceDiscoveryMoreRepliesImmediately  = 1 << 0,
};


typedef enum
{
    DNSServiceDomainEnumerationReplyAddDomain,
    DNSServiceDomainEnumerationReplyAddDomainDefault,
    DNSServiceDomainEnumerationReplyRemoveDomain,
} DNSServiceDomainEnumerationReplyResultType;

typedef enum
{
    DNSServiceDiscoverReplyFlagsFinished,
    DNSServiceDiscoverReplyFlagsMoreComing,
} DNSServiceDiscoveryReplyFlags;

typedef void (*DNSServiceDomainEnumerationReply)(
    DNSServiceDomainEnumerationReplyResultType resultType,  // One of DNSServiceDomainEnumerationReplyResultType
    const char                                *replyDomain,
    DNSServiceDiscoveryReplyFlags              flags,       // DNS Service Discovery reply flags information
    void                                      *context
);


/* possible error code values */
typedef enum
{
    kDNSServiceDiscoveryNoError     = 0,
} DNSServiceRegistrationReplyErrorType;

typedef void (*DNSServiceRegistrationReply)(
    DNSServiceRegistrationReplyErrorType errorCode,
    void                                        *context
);

typedef uint32_t DNSRecordReference;


/*!
   @function DNSServiceResolver_handleReply
   @discussion This function should be called with the Mach message sent
   to the port returned by the call to DNSServiceResolverResolve.
   The reply message will be interpreted and will result in a
   call to the specified callout function.
   @param replyMsg The Mach message.
 */
void DNSServiceDiscovery_handleReply(void *replyMsg) API_DEPRECATED(kDNSServiceDiscoveryDeprecatedMsg, macos(10.2, 10.3));

/***************************************************************************/
/*   DNS Service Browser   */

typedef enum
{
    DNSServiceBrowserReplyAddInstance,  // Instance of service found
    DNSServiceBrowserReplyRemoveInstance    // Instance has been removed from network
} DNSServiceBrowserReplyResultType;

typedef void (*DNSServiceBrowserReply)(
    DNSServiceBrowserReplyResultType resultType,                // One of DNSServiceBrowserReplyResultType
    const char      *replyName,
    const char      *replyType,
    const char      *replyDomain,
    DNSServiceDiscoveryReplyFlags flags,                        // DNS Service Discovery reply flags information
    void            *context
    );

/*!
    @function DNSServiceBrowserCreate
    @discussion Asynchronously create a DNS Service browser
    @param regtype The type of service
    @param domain The domain in which to find the service
    @param callBack The function to be called when service instances are found or removed
    @param context A user specified context which will be passed to the callout function.
    @result A dns_registration_t
 */
dns_service_discovery_ref DNSServiceBrowserCreate
(
    const char      *regtype,
    const char      *domain,
    DNSServiceBrowserReply callBack,
    void        *context
) API_DEPRECATED(kDNSServiceDiscoveryDeprecatedMsg, macos(10.2, 10.3));

/***************************************************************************/
/* Resolver requests */

typedef void (*DNSServiceResolverReply)(
    struct sockaddr     *interface,     // Needed for scoped addresses like link-local
    struct sockaddr     *address,
    const char          *txtRecord,
    DNSServiceDiscoveryReplyFlags flags,                        // DNS Service Discovery reply flags information
    void                *context
    );

/*!
   @function DNSServiceResolverResolve
    @discussion Resolved a named instance of a service to its address, port, and
        (optionally) other demultiplexing information contained in the TXT record.
    @param name The name of the service instance
    @param regtype The type of service
    @param domain The domain in which to find the service
    @param callBack The DNSServiceResolverReply function to be called when the specified
        address has been resolved.
    @param context A user specified context which will be passed to the callout function.
    @result A dns_registration_t
 */

dns_service_discovery_ref DNSServiceResolverResolve
(
    const char      *name,
    const char      *regtype,
    const char      *domain,
    DNSServiceResolverReply callBack,
    void        *context
) API_DEPRECATED(kDNSServiceDiscoveryDeprecatedMsg, macos(10.2, 10.3));

/***************************************************************************/
/* Mach port accessor and deallocation */

/*!
    @function DNSServiceDiscoveryMachPort
    @discussion Returns the mach port for a dns_service_discovery_ref
    @param registration A dns_service_discovery_ref as returned from DNSServiceRegistrationCreate
    @result A mach reply port which will be sent messages as appropriate.
        These messages should be passed to the DNSServiceDiscovery_handleReply
        function.  A NULL value indicates that no address was
        specified or some other error occurred which prevented the
        resolution from being started.
 */
mach_port_t DNSServiceDiscoveryMachPort(dns_service_discovery_ref dnsServiceDiscovery) API_DEPRECATED(kDNSServiceDiscoveryDeprecatedMsg, macos(10.2, 10.3));

/*!
    @function DNSServiceDiscoveryDeallocate
    @discussion Deallocates the DNS Service Discovery type / closes the connection to the server
    @param dnsServiceDiscovery A dns_service_discovery_ref as returned from a creation or enumeration call
    @result void
 */
void DNSServiceDiscoveryDeallocate(dns_service_discovery_ref dnsServiceDiscovery) API_DEPRECATED(kDNSServiceDiscoveryDeprecatedMsg, macos(10.2, 10.3));

__END_DECLS

#endif  /* __DNS_SERVICE_DISCOVERY_H */
