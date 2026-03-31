/* -*- Mode: C; tab-width: 4 -*-
 * 
 * Copyright (c) 2015-2019 Apple Inc. All rights reserved.
 */

#ifndef _DNS_SD_PRIVATE_H
#define _DNS_SD_PRIVATE_H

#include <dns_sd.h>

// Private flags (kDNSServiceFlagsPrivateOne, kDNSServiceFlagsPrivateTwo, kDNSServiceFlagsPrivateThree, kDNSServiceFlagsPrivateFour, kDNSServiceFlagsPrivateFive) from dns_sd.h
enum
{
    kDNSServiceFlagsDenyConstrained        = 0x2000,
    /*
     * This flag is meaningful only for Unicast DNS queries. When set, the daemon will restrict
     * DNS resolutions on interfaces defined as constrained for that request.
     */

    kDNSServiceFlagsDenyCellular           = 0x8000000,
    /*
     * This flag is meaningful only for Unicast DNS queries. When set, the daemon will restrict
     * DNS resolutions on the cellular interface for that request.
     */
    kDNSServiceFlagsServiceIndex           = 0x10000000,
    /*
     * This flag is meaningful only for DNSServiceGetAddrInfo() for Unicast DNS queries.
     * When set, DNSServiceGetAddrInfo() will interpret the "interfaceIndex" argument of the call
     * as the "serviceIndex".
     */

    kDNSServiceFlagsDenyExpensive          = 0x20000000,
    /*
     * This flag is meaningful only for Unicast DNS queries. When set, the daemon will restrict
     * DNS resolutions on interfaces defined as expensive for that request.
     */

    kDNSServiceFlagsPathEvaluationDone     = 0x40000000
    /*
     * This flag is meaningful for only Unicast DNS queries.
     * When set, it indicates that Network PathEvaluation has already been performed.
     */
};

#if !defined(DNSSD_NO_CREATE_DELEGATE_CONNECTION) || !DNSSD_NO_CREATE_DELEGATE_CONNECTION
/* DNSServiceCreateDelegateConnection()
 *
 * Parameters:
 *
 * sdRef:           A pointer to an uninitialized DNSServiceRef. Deallocating
 *                  the reference (via DNSServiceRefDeallocate()) severs the
 *                  connection and deregisters all records registered on this connection.
 *
 * pid :            Process ID of the delegate
 *
 * uuid:            UUID of the delegate
 *
 *                  Note that only one of the two arguments (pid or uuid) can be specified. If pid
 *                  is zero, uuid will be assumed to be a valid value; otherwise pid will be used.
 *
 * return value:    Returns kDNSServiceErr_NoError on success, otherwise returns
 *                  an error code indicating the specific failure that occurred (in which
 *                  case the DNSServiceRef is not initialized). kDNSServiceErr_NotAuth is
 *                  returned to indicate that the calling process does not have entitlements
 *                  to use this API.
 */
DNSSD_EXPORT
DNSServiceErrorType DNSSD_API DNSServiceCreateDelegateConnection(DNSServiceRef *sdRef, int32_t pid, uuid_t uuid);
#endif

// Map the source port of the local UDP socket that was opened for sending the DNS query
// to the process ID of the application that triggered the DNS resolution.
//
/* DNSServiceGetPID() Parameters:
 *
 * srcport:         Source port (in network byte order) of the UDP socket that was created by
 *                  the daemon to send the DNS query on the wire.
 *
 * pid:             Process ID of the application that started the name resolution which triggered
 *                  the daemon to send the query on the wire. The value can be -1 if the srcport
 *                  cannot be mapped.
 *
 * return value:    Returns kDNSServiceErr_NoError on success, or kDNSServiceErr_ServiceNotRunning
 *                  if the daemon is not running. The value of the pid is undefined if the return
 *                  value has error.
 */
DNSSD_EXPORT
DNSServiceErrorType DNSSD_API DNSServiceGetPID
(
    uint16_t srcport,
    int32_t *pid
);

DNSSD_EXPORT
DNSServiceErrorType DNSSD_API DNSServiceSetDefaultDomainForUser(DNSServiceFlags flags, const char *domain);

SPI_AVAILABLE(macos(10.15.4), ios(13.2.2), watchos(6.2), tvos(13.2))
DNSServiceErrorType DNSSD_API DNSServiceSleepKeepalive_sockaddr
(
    DNSServiceRef *                 sdRef,
    DNSServiceFlags                 flags,
    const struct sockaddr *         localAddr,
    const struct sockaddr *         remoteAddr,
    unsigned int                    timeout,
    DNSServiceSleepKeepaliveReply   callBack,
    void *                          context
);

#define kDNSServiceCompPrivateDNS   "PrivateDNS"
#define kDNSServiceCompMulticastDNS "MulticastDNS"

#endif
