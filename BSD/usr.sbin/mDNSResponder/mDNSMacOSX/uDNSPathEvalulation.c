/* -*- Mode: C; tab-width: 4; c-file-style: "bsd"; c-basic-offset: 4; fill-column: 108; indent-tabs-mode: nil; -*-
 *
 * Copyright (c) 2013-2018 Apple Inc. All rights reserved.
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

#include "mDNSMacOSX.h"
#include <libproc.h>

#if __has_include(<nw/private.h>)
    #include <nw/private.h>
#else
    #include <network/private.h>
    #define nw_release(X) network_release(X)
#endif

#include "dns_sd_internal.h"

//Gets the DNSPolicy from NW PATH EVALUATOR
mDNSexport void mDNSPlatformGetDNSRoutePolicy(DNSQuestion *q, mDNSBool *isBlocked)
{
    if (__builtin_available(macOS 10.14, *))
    {
        q->ServiceID = -1; // initialize the ServiceID to default value of -1

        // Return for non-unicast DNS queries, invalid pid, if NWPathEvaluation is already done by the client, or NWPathEvaluation not available on this OS
        if (mDNSOpaque16IsZero(q->TargetQID) || (q->pid < 0) || (q->flags & kDNSServiceFlagsPathEvaluationDone) || !nw_endpoint_create_host)
        {
            *isBlocked = mDNSfalse;
            return;
        }
        
        mDNSs32 service_id;
        mDNSu32 client_ifindex, dnspol_ifindex;
        int retval;
        struct proc_uniqidentifierinfo info;
        mDNSBool isUUIDSet;
        
        char unenc_name[MAX_ESCAPED_DOMAIN_NAME];
        ConvertDomainNameToCString(&q->qname, unenc_name);
        
        nw_endpoint_t host = nw_endpoint_create_host(unenc_name, "0");
        if (host == NULL)
            LogMsg("mDNSPlatformGetDNSRoutePolicy: DNS Route Policy: Query for %##s (%s), PID[%d], EUID[%d], ServiceID[%d] nw_endpoint_t host is NULL", q->qname.c,
                   DNSTypeName(q->qtype), q->pid, q->euid, q->ServiceID);
        
        nw_parameters_t parameters = nw_parameters_create();
        if (parameters == NULL)
            LogMsg("mDNSPlatformGetDNSRoutePolicy: DNS Route Policy: Query for %##s (%s), PID[%d], EUID[%d], ServiceID[%d] nw_endpoint_t parameters is NULL", q->qname.c,
                   DNSTypeName(q->qtype), q->pid, q->euid, q->ServiceID);

#if TARGET_OS_WATCH
        // Companion interface on watchOS does not support DNS, so we don't want path evalution to return it to us.
        nw_parameters_set_companion_preference(parameters, nw_parameters_agent_preference_avoid);
#endif // TARGET_OS_WATCH
        
        // Check for all the special (negative) internal value interface indices before initializing client_ifindex
        if (   (q->InterfaceID == mDNSInterface_Any)
            || (q->InterfaceID == mDNSInterface_LocalOnly)
            || (q->InterfaceID == mDNSInterfaceMark)
            || (q->InterfaceID == mDNSInterface_P2P)
            || (q->InterfaceID == mDNSInterface_BLE)
            || (q->InterfaceID == uDNSInterfaceMark))
        {
            client_ifindex = 0;
        }
        else
        {
            client_ifindex = (mDNSu32)(uintptr_t)q->InterfaceID;
        }

        if (client_ifindex > 0)
        {
            nw_interface_t client_intf = nw_interface_create_with_index(client_ifindex);
            nw_parameters_require_interface(parameters, client_intf);
            if (client_intf != NULL)
                nw_release(client_intf);
            else
                LogInfo("mDNSPlatformGetDNSRoutePolicy: DNS Route Policy: client_intf returned by nw_interface_create_with_index() is NULL");
        }
        
        nw_parameters_set_uid(parameters,(uid_t)q->euid);

        if (q->pid != 0)
        {
            nw_parameters_set_pid(parameters, q->pid);
            retval = proc_pidinfo(q->pid, PROC_PIDUNIQIDENTIFIERINFO, 1, &info, sizeof(info));
            if (retval == (int)sizeof(info))
            {
                nw_parameters_set_e_proc_uuid(parameters, info.p_uuid);
                isUUIDSet = mDNStrue;
            }
            else
            {
                debugf("mDNSPlatformGetDNSRoutePolicy: proc_pidinfo returned %d", retval);
                isUUIDSet = mDNSfalse;
            }
        }
        else
        {
            nw_parameters_set_e_proc_uuid(parameters, q->uuid);
            isUUIDSet = mDNStrue;
        }
        
        nw_path_evaluator_t evaluator = nw_path_create_evaluator_for_endpoint(host, parameters);
        if (evaluator == NULL)
            LogMsg("mDNSPlatformGetDNSRoutePolicy: DNS Route Policy: Query for %##s (%s), PID[%d], EUID[%d], ServiceID[%d] nw_path_evaluator_t evaluator is NULL", q->qname.c,
                    DNSTypeName(q->qtype), q->pid, q->euid, q->ServiceID);
        
        if (host != NULL)
            nw_release(host);
        if (parameters != NULL)
            nw_release(parameters);
        
        nw_path_t path = nw_path_evaluator_copy_path(evaluator);
        if (path == NULL)
            LogMsg("mDNSPlatformGetDNSRoutePolicy: DNS Route Policy: Query for %##s (%s), PID[%d], EUID[%d], ServiceID[%d] nw_path_t path is NULL", q->qname.c,
                   DNSTypeName(q->qtype), q->pid, q->euid, q->ServiceID);
        
        service_id = nw_path_get_flow_divert_unit(path);
        if (service_id != 0)
        {
            q->ServiceID = service_id;
            LogInfo("mDNSPlatformGetDNSRoutePolicy: DNS Route Policy: Query for %##s service ID is set ->service_ID:[%d] ", q->qname.c, service_id);
        }
        else
        {
            nw_interface_t nwpath_intf = nw_path_copy_scoped_interface(path);
            if (nwpath_intf != NULL)
            {
                // Use the new scoped interface given by NW PATH EVALUATOR
                dnspol_ifindex = nw_interface_get_index(nwpath_intf);
                q->InterfaceID = (mDNSInterfaceID)(uintptr_t)dnspol_ifindex;
                
                nw_release(nwpath_intf);
                
                if (dnspol_ifindex != client_ifindex)
                    LogInfo("mDNSPlatformGetDNSRoutePolicy: DNS Route Policy has changed the scoped ifindex from [%d] to [%d]",
                            client_ifindex, dnspol_ifindex);
            }
            else
            {
                debugf("mDNSPlatformGetDNSRoutePolicy: Query for %##s (%s), PID[%d], EUID[%d], ServiceID[%d] nw_interface_t nwpath_intf is NULL ", q->qname.c, DNSTypeName(q->qtype), q->pid, q->euid, q->ServiceID);
            }
        }
        
        if (isUUIDSet && path && (nw_path_get_status(path) == nw_path_status_unsatisfied) && (nw_path_get_reason(path) != nw_path_reason_no_route))
            *isBlocked = mDNStrue;
        else
            *isBlocked = mDNSfalse;

        if (path != NULL)
            nw_release(path);
        if (evaluator != NULL)
            nw_release(evaluator);
    }
    else
    {
        *isBlocked = mDNSfalse;
    }
}
