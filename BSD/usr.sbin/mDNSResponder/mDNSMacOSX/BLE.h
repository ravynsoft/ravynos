/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2015-2016 Apple Inc. All rights reserved.
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

#ifndef _BLE_H_
#define _BLE_H_

#if ENABLE_BLE_TRIGGERED_BONJOUR

#include "dns_sd.h"
#include "dns_sd_internal.h"

typedef unsigned long serviceHash_t;

bool shouldUseBLE(mDNSInterfaceID interfaceID,  DNS_TypeValues rrtype, domainname *serviceType, domainname *domain);

void start_BLE_browse(mDNSInterfaceID InterfaceID, const domainname *const domain, DNS_TypeValues type, DNSServiceFlags flags,
                       mDNSu8 *key, size_t keySize);
bool stop_BLE_browse(mDNSInterfaceID InterfaceID, const domainname *const domain, DNS_TypeValues type, DNSServiceFlags flags);

void start_BLE_advertise(const ResourceRecord *const resourceRecord, const domainname *const domain, DNS_TypeValues type, DNSServiceFlags flags);
void stop_BLE_advertise(const domainname *const domain, DNS_TypeValues type, DNSServiceFlags flags);

void responseReceived(serviceHash_t peerBloomFilter, mDNSEthAddr *ptrToMAC);

void serviceBLE(void);

// C interfaces to Objective-C beacon management code.
void updateBLEBeacon(serviceHash_t bloomFilter);
void stopBLEBeacon(void);
void startBLEScan(void);
void stopBLEScan(void);
bool currentlyBeaconing(void);

extern bool suppressBeacons;
extern bool finalBeacon;

extern mDNS mDNSStorage;
extern mDNSBool EnableBLEBasedDiscovery;
extern mDNSBool DefaultToBLETriggered;

extern mDNSInterfaceID AWDLInterfaceID;
#define applyToBLE(interface, flags) ((interface == mDNSInterface_BLE) || (((interface == mDNSInterface_Any) || (interface == AWDLInterfaceID)) && (flags & kDNSServiceFlagsAutoTrigger)))

#ifdef UNIT_TEST
#pragma mark - Unit test declarations

// Unit test entry points, which are not used in the mDNSResponder runtime code paths.
void BLE_unitTest(void);

#endif  //  UNIT_TEST

#endif  // ENABLE_BLE_TRIGGERED_BONJOUR

#endif /* _BLE_H_ */
