/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
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

#ifndef __DNS64State_h
#define __DNS64State_h

typedef enum
{
    kDNS64State_Initial             = 0,    // Initial state.
    kDNS64State_PrefixDiscovery     = 1,    // Querying for "ipv4only.arpa." AAAA records to discover NAT64 IPv6 prefix(es).
    kDNS64State_PrefixDiscoveryPTR  = 2,    // Same as PrefixDiscovery, but discoverying for "ip6.arpa." PTR record queries.
    kDNS64State_QueryA              = 3,    // Querying for A record with same QNAME as AAAA record query.
    kDNS64State_QueryA2             = 4,    // Continuing A record query after being answered with a synthesizable A record.
    kDNS64State_QueryAAAA           = 5,    // Querying for original AAAA record.
    kDNS64State_QueryPTR            = 6,    // Determining whether to query for reverse IPV4 or reverse IPv6 PTR record.
    kDNS64State_ReverseIPv4         = 7,    // Querying for reverse IPV4 (in-addr.arpa.) PTR record.
    kDNS64State_ReverseIPv6         = 8     // Querying for the original reverse IPv6 (ip6.arpa.) PTR record.

}   DNS64State;

typedef struct
{
    DNS64State      state;          // Current state.
    mDNSu8          qnameStash[15]; // Temporary space to hold the up to 15 bytes that are displaced in a DNSQuestion's qname
                                    // when it's set to "ipv4only.arpa." during prefix discovery.
}   DNS64;

#endif // __DNS64State_h
