/*
 * Copyright (c) 2019 Apple Inc. All rights reserved.
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

#include "xpc_services.h"
#include <xpc/private.h>                // xpc_connection_copy_entitlement_value

#include "mDNSMacOSX.h"                 // KQueueLock/KQueueUnlock
#include "dnsproxy.h"                   // DNSProxyInit/ProxyUDPCallback/ProxyTCPCallback
#include "xpc_service_dns_proxy.h"      // init_dnsproxy_service
#include "xpc_service_log_utility.h"    // init_dnsctl_service

extern mDNS mDNSStorage;

mDNSexport void xpc_server_init()
{
    // add XPC Services here
    init_dnsproxy_service();
    init_log_utility_service();
}

// Utilities

mDNSexport mDNSBool IsEntitled(xpc_connection_t conn, const char *entitlement_name)
{
    mDNSBool        entitled = mDNSfalse;
    xpc_object_t    entitled_obj = xpc_connection_copy_entitlement_value(conn, entitlement_name);

    if (entitled_obj) {
        if (xpc_get_type(entitled_obj) == XPC_TYPE_BOOL && xpc_bool_get_value(entitled_obj)) {
            entitled = mDNStrue;
        }
        xpc_release(entitled_obj);
    } else {
        LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_DEFAULT, "IsEntitled: Client Entitlement is NULL");
    }

    if (!entitled) {
        LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_DEFAULT, "IsEntitled: Client is missing Entitlement!");
    }

    return entitled;
}
