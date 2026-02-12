/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2015-2018 Apple Inc. All rights reserved.
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

#ifndef _D2D_H_
#define _D2D_H_

#include "mDNSEmbeddedAPI.h"        // Defines the interface provided to the client layer above
#include "dnssd_ipc.h"
#include <DeviceToDeviceManager/DeviceToDeviceManager.h>

extern void internal_start_browsing_for_service(mDNSInterfaceID InterfaceID, const domainname *const type, DNS_TypeValues qtype, DNSServiceFlags flags);
extern void internal_stop_browsing_for_service(mDNSInterfaceID InterfaceID, const domainname *const type, DNS_TypeValues qtype, DNSServiceFlags flags);
extern void internal_start_advertising_service(const ResourceRecord *const resourceRecord, DNSServiceFlags flags);
extern void internal_stop_advertising_service(const ResourceRecord *const resourceRecord, DNSServiceFlags flags);

void xD2DAddToCache(D2DStatus result, D2DServiceInstance instanceHandle, D2DTransportType transportType, const Byte *key, size_t keySize, const Byte *value, size_t valueSize);
void xD2DRemoveFromCache(D2DStatus result, D2DServiceInstance instanceHandle, D2DTransportType transportType, const Byte *key, size_t keySize, const Byte *value, size_t valueSize);

extern mDNSBool callExternalHelpers(mDNSInterfaceID InterfaceID, const domainname *const domain, DNSServiceFlags flags);
extern mDNSu32 deriveD2DFlagsFromAuthRecType(AuthRecType authRecType);
extern void external_start_browsing_for_service(mDNSInterfaceID InterfaceID, const domainname *const type, DNS_TypeValues qtype, DNSServiceFlags flags);
extern void external_stop_browsing_for_service(mDNSInterfaceID InterfaceID, const domainname *const type, DNS_TypeValues qtype, DNSServiceFlags flags);
extern void external_start_advertising_service(const ResourceRecord *const resourceRecord, DNSServiceFlags flags);
extern void external_stop_advertising_service(const ResourceRecord *const resourceRecord, DNSServiceFlags flags);
extern void external_start_resolving_service(mDNSInterfaceID InterfaceID, const domainname *const fqdn, DNSServiceFlags flags);
extern void external_stop_resolving_service(mDNSInterfaceID InterfaceID, const domainname *const fqdn, DNSServiceFlags flags);
extern void external_connection_release(const domainname *instance);

extern void D2D_start_advertising_interface(NetworkInterfaceInfo *interface);
extern void D2D_stop_advertising_interface(NetworkInterfaceInfo *interface);
extern void D2D_start_advertising_record(AuthRecord *ar);
extern void D2D_stop_advertising_record(AuthRecord *ar);

#if ENABLE_BLE_TRIGGERED_BONJOUR
// Just define as the current max value for now for BLE.c prototype.
// TODO: Will need to define in DeviceToDeviceManager.framework if we convert the
// BLE discovery code to a D2D plugin.
#define D2DBLETransport D2DTransportMax
#endif // ENABLE_BLE_TRIGGERED_BONJOUR

#ifdef UNIT_TEST
#pragma mark - Unit test declarations

// Unit test entry points, which are not used in the mDNSResponder runtime code paths.
void D2D_unitTest(void);

#endif  //  UNIT_TEST

#endif /* _D2D_H_ */
