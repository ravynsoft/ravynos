/*
 * Copyright (c) 2018-2019 Apple Inc. All rights reserved.
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

#ifndef __ApplePlatformFeatures_h
#define __ApplePlatformFeatures_h

#include <TargetConditionals.h>

// Feature: Bonjour-On-Demand
// Radar:   <rdar://problem/23523784>
// Enabled: Yes.

#if !defined(MDNSRESPONDER_SUPPORTS_APPLE_BONJOUR_ON_DEMAND)
    #define MDNSRESPONDER_SUPPORTS_APPLE_BONJOUR_ON_DEMAND          1
#endif

// Feature: Cache memory limit
// Radar:   <rdar://problem/15629764>
// Enabled: Yes, but only for device OSes, such as iOS, tvOS, and watchOS, i.e., when TARGET_OS_IPHONE is 1.

#if !defined(MDNSRESPONDER_SUPPORTS_APPLE_CACHE_MEM_LIMIT)
    #if !defined(TARGET_OS_IPHONE)
        #error "Expected TARGET_OS_IPHONE to be defined."
    #endif
    #if TARGET_OS_IPHONE
        #define MDNSRESPONDER_SUPPORTS_APPLE_CACHE_MEM_LIMIT        1
    #else
        #define MDNSRESPONDER_SUPPORTS_APPLE_CACHE_MEM_LIMIT        0
    #endif
#endif

// Feature: D2D
// Radar:   <rdar://problem/28062515>
// Enabled: Yes.

#if !defined(MDNSRESPONDER_SUPPORTS_APPLE_D2D)
    #define MDNSRESPONDER_SUPPORTS_APPLE_D2D                        1
#endif

// Feature: DNS64 IPv6 synthesis.
// Radar:   <rdar://problem/32297396>
// Enabled: Yes, but only for iOS and macOS, which support the DNS proxy network extension.

#if !defined(MDNSRESPONDER_SUPPORTS_APPLE_DNS64)
    #if (!defined(TARGET_OS_IOS) || !defined(TARGET_OS_OSX))
        #error "Expected TARGET_OS_IOS and TARGET_OS_OSX to be defined."
    #endif
    #if (TARGET_OS_IOS || TARGET_OS_OSX)
        #define MDNSRESPONDER_SUPPORTS_APPLE_DNS64                  1
    #else
        #define MDNSRESPONDER_SUPPORTS_APPLE_DNS64                  0
    #endif
#endif

// Feature: DNS-SD XPC service
// Radar:   <rdar://problem/43866363>
// Enabled: Yes.

#if !defined(MDNSRESPONDER_SUPPORTS_APPLE_DNSSD_XPC_SERVICE)
    #define MDNSRESPONDER_SUPPORTS_APPLE_DNSSD_XPC_SERVICE          1
#endif

// Feature: Ignore /etc/hosts file on customer builds.
// Radar:   <rdar://problem/34745220>
// Enabled: Yes, except for macOS.

#if !defined(MDNSRESPONDER_SUPPORTS_APPLE_IGNORE_HOSTS_FILE)
    #if !defined(TARGET_OS_OSX)
        #error "Expected TARGET_OS_OSX to be defined."
    #endif
    #if !TARGET_OS_OSX
        #define MDNSRESPONDER_SUPPORTS_APPLE_IGNORE_HOSTS_FILE      1
    #else
        #define MDNSRESPONDER_SUPPORTS_APPLE_IGNORE_HOSTS_FILE      0
    #endif
#endif

// Feature: AWD metrics collection
// Radar:   <rdar://problem/24146300>
// Enabled: Yes, but for iOS only.

#if !defined(MDNSRESPONDER_SUPPORTS_APPLE_METRICS)
    #if !defined(TARGET_OS_IOS)
        #error "Expected TARGET_OS_IOS to be defined."
    #endif
    #if TARGET_OS_IOS
        #define MDNSRESPONDER_SUPPORTS_APPLE_METRICS                1
    #else
        #define MDNSRESPONDER_SUPPORTS_APPLE_METRICS                0
    #endif
#endif

// Feature: No system wake for network access.
// Radar:   <rdar://problem/28079659&55038229>
// Enabled: Yes, but only for iOS and watchOS, which shouldn't act as sleep-proxy clients.

#if !defined(MDNSRESPONDER_SUPPORTS_APPLE_NO_WAKE_FOR_NET_ACCESS)
    #if (!defined(TARGET_OS_IOS) || !defined(TARGET_OS_WATCH))
        #error "Expected TARGET_OS_IOS and TARGET_OS_WATCH to be defined."
    #endif
    #if (TARGET_OS_IOS || TARGET_OS_WATCH)
        #define MDNSRESPONDER_SUPPORTS_APPLE_NO_WAKE_FOR_NET_ACCESS 1
    #else
        #define MDNSRESPONDER_SUPPORTS_APPLE_NO_WAKE_FOR_NET_ACCESS 0
    #endif
#endif

// Feature: Support for having finer granularity of log redaction, by using os_log based-log routine.
// Radar:   <rdar://problem/42814956>
// Enabled: Yes.

#if !defined(MDNSRESPONDER_SUPPORTS_APPLE_OS_LOG)
    #define MDNSRESPONDER_SUPPORTS_APPLE_OS_LOG                     1
#endif

// Feature: Preallocate mDNSResponder's cache memory. For testing purposes only.
// Radar:   <rdar://problem/29545890>
// Enabled: No.

#if !defined(MDNSRESPONDER_SUPPORTS_APPLE_PREALLOCATED_CACHE)
    #define MDNSRESPONDER_SUPPORTS_APPLE_PREALLOCATED_CACHE         0
#endif

// Feature: Randomized AWDL Hostname
// Radar:   <rdar://problem/47525004>
// Enabled: Yes.

#if !defined(MDNSRESPONDER_SUPPORTS_APPLE_RANDOM_AWDL_HOSTNAME)
    #define MDNSRESPONDER_SUPPORTS_APPLE_RANDOM_AWDL_HOSTNAME       1
#endif

// Feature: Reachability trigger
// Radar:   <rdar://problem/11374446>
// Enabled: Yes.

#if !defined(MDNSRESPONDER_SUPPORTS_APPLE_REACHABILITY_TRIGGER)
    #define MDNSRESPONDER_SUPPORTS_APPLE_REACHABILITY_TRIGGER       1
#endif

// Feature: "SlowActivation" processing for flapping interfaces.
//          Disabled to address stale Bonjour record issues during flapping network interface transitions.
// Radar:   <rdar://problem/44694746>
// Enabled: No.

#if !defined(MDNSRESPONDER_SUPPORTS_APPLE_SLOW_ACTIVATION)
    #define MDNSRESPONDER_SUPPORTS_APPLE_SLOW_ACTIVATION            0
#endif

// Feature: Suspicious Reply Defense
// Radar:   <rdar://problem/50050767>
// Enabled: Yes.

#if !defined(MDNSRESPONDER_SUPPORTS_APPLE_SUSPICIOUS_REPLY_DEFENSE)
    #define MDNSRESPONDER_SUPPORTS_APPLE_SUSPICIOUS_REPLY_DEFENSE   1
#endif

// Feature: Symptoms Reporting
// Radar:   <rdar://problem/20194922>
// Enabled: Yes.

#if !defined(MDNSRESPONDER_SUPPORTS_APPLE_SYMPTOMS)
    #define MDNSRESPONDER_SUPPORTS_APPLE_SYMPTOMS                   1
#endif

// Feature: Support for performing dot-local queries via mDNS and DNS in parallel.
// Radar:   <rdar://problem/4786302>
// Enabled: Yes.

#if !defined(MDNSRESPONDER_SUPPORTS_APPLE_UNICAST_DOTLOCAL)
    #define MDNSRESPONDER_SUPPORTS_APPLE_UNICAST_DOTLOCAL           1
#endif

// Feature: Allow browses and registrations over interfaces that aren't ready yet.
// Radar:   <rdar://problem/20181903>
// Enabled: Yes.

#if !defined(MDNSRESPONDER_SUPPORTS_APPLE_UNREADY_INTERFACES)
    #define MDNSRESPONDER_SUPPORTS_APPLE_UNREADY_INTERFACES         1
#endif

// Feature: Support for Web Content Filter
// Radar:   <rdar://problem/7409981>
// Enabled: Yes, if SDK has <WebFilterDNS/WebFilterDNS.h>.

#if !defined(MDNSRESPONDER_SUPPORTS_APPLE_WEB_CONTENT_FILTER)
    #if __has_include(<WebFilterDNS/WebFilterDNS.h>)
        #define MDNSRESPONDER_SUPPORTS_APPLE_WEB_CONTENT_FILTER     1
    #else
        #define MDNSRESPONDER_SUPPORTS_APPLE_WEB_CONTENT_FILTER     0
    #endif
#endif

#endif  // __ApplePlatformFeatures_h
