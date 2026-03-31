/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2011-2013 Apple Inc. All rights reserved.
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

#ifndef __DNS_PROXY_H
#define __DNS_PROXY_H

#include "mDNSEmbeddedAPI.h"
#include "DNSCommon.h"

extern void ProxyUDPCallback(void *socket, DNSMessage *const msg, const mDNSu8 *const end, const mDNSAddr *const srcaddr,
                             const mDNSIPPort srcport, const mDNSAddr *dstaddr, const mDNSIPPort dstport, const mDNSInterfaceID InterfaceID, void *context);
extern void ProxyTCPCallback(void *socket, DNSMessage *const msg, const mDNSu8 *const end, const mDNSAddr *const srcaddr,
                             const mDNSIPPort srcport, const mDNSAddr *dstaddr, const mDNSIPPort dstport, const mDNSInterfaceID InterfaceID, void *context);                          
extern void DNSProxyInit(mDNSu32 IpIfArr[MaxIp], mDNSu32 OpIf);
extern void DNSProxyTerminate(void);

#endif // __DNS_PROXY_H
