/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 *  DeprecatedDNSServiceDiscovery.c
 *  CFNetwork
 *
 *  Created by Jeremy Wyld on 12/15/04.
 *  Copyright 2004 Apple Computer, Inc. All rights reserved.
 *
 */

#include "DeprecatedDNSServiceDiscovery.h"



dns_service_discovery_ref DNSServiceRegistrationCreate_Deprecated
(
	const char 		*name,
	const char 		*regtype,
	const char 		*domain,
	uint16_t		port,
	const char 		*txtRecord,
	DNSServiceRegistrationReply callBack,
	void		*context
 )
{
	return DNSServiceRegistrationCreate(name, regtype, domain, port, txtRecord, callBack, context);
}


dns_service_discovery_ref DNSServiceResolverResolve_Deprecated
(
	const char 		*name,
	const char 		*regtype,
	const char 		*domain,
	DNSServiceResolverReply callBack,
	void		*context
 )
{
	return DNSServiceResolverResolve(name, regtype, domain, callBack, context);
}


mach_port_t DNSServiceDiscoveryMachPort_Deprecated(dns_service_discovery_ref dnsServiceDiscovery)
{
	return DNSServiceDiscoveryMachPort(dnsServiceDiscovery);
}


void DNSServiceDiscoveryDeallocate_Deprecated(dns_service_discovery_ref dnsServiceDiscovery)
{
	DNSServiceDiscoveryDeallocate( dnsServiceDiscovery);
}


DNSServiceRegistrationReplyErrorType DNSServiceRegistrationUpdateRecord_Deprecated(dns_service_discovery_ref ref, DNSRecordReference reference, uint16_t rdlen, const char *rdata, uint32_t ttl)
{
	return DNSServiceRegistrationUpdateRecord(ref, reference, rdlen, rdata, ttl);
}
