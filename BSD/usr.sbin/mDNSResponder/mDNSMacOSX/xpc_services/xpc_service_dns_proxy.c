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

#include <xpc/private.h>
#include "xpc_service_dns_proxy.h"
#include "xpc_clients.h"

#include "dnsproxy.h"
#include "mDNSMacOSX.h"
#include "xpc_services.h"

extern mDNS mDNSStorage;
static int dps_client_pid; // To track current active client using DNS Proxy Service
static dispatch_queue_t dps_queue = NULL;

mDNSlocal void accept_dps_client(xpc_connection_t conn);
mDNSlocal void handle_dps_request(xpc_object_t req);
mDNSlocal void handle_dps_terminate();
mDNSlocal void ActivateDNSProxy(mDNSu32 IpIfArr[MaxIp], mDNSu32 OpIf, mDNSBool proxy_off);

mDNSexport void log_dnsproxy_info(mDNS *const m)
{
    LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_DEFAULT, "----- Active XPC Clients -----");
    if (dps_client_pid) {
        LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_DEFAULT, "DNSProxy->Client[%d]: InputIntfs[%d, %d, %d, %d, %d] Output[%d]",
                  dps_client_pid, m->dp_ipintf[0], m->dp_ipintf[1], m->dp_ipintf[2], m->dp_ipintf[3], m->dp_ipintf[4],
                  m->dp_opintf);
    } else {
        LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_DEFAULT, "<None>");
    }
}

mDNSexport void log_dnsproxy_info_to_fd(int fd, mDNS *const m)
{
    LogToFD(fd, "----- Active XPC Clients -----");
    if (dps_client_pid) {
        LogToFD(fd, "DNSProxy->Client[%d]: InputIntfs[%d, %d, %d, %d, %d] Output[%d]",
                  dps_client_pid, m->dp_ipintf[0], m->dp_ipintf[1], m->dp_ipintf[2], m->dp_ipintf[3], m->dp_ipintf[4],
                  m->dp_opintf);
    } else {
        LogToFD(fd, "<None>");
    }
}

mDNSexport void init_dnsproxy_service(void)
{
    xpc_connection_t dps_listener = xpc_connection_create_mach_service(kDNSProxyService, NULL, XPC_CONNECTION_MACH_SERVICE_LISTENER);
    if (!dps_listener || xpc_get_type(dps_listener) != XPC_TYPE_CONNECTION)
    {
        LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_DEFAULT,
                  "init_dnsproxy_service: Error Creating XPC Listener for DNSProxyService !!");
        return;
    }

    dps_queue = dispatch_queue_create("com.apple.mDNSResponder.dnsproxyservice_queue", NULL);

    xpc_connection_set_event_handler(dps_listener, ^(xpc_object_t eventmsg)
        {
            xpc_type_t type = xpc_get_type(eventmsg);

            if (type == XPC_TYPE_CONNECTION) {
                LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_DEFAULT,
                          "init_dnsproxy_service: New DNSProxyService Client %p", eventmsg);
                accept_dps_client(eventmsg);
            }
            else if (type == XPC_TYPE_ERROR) // Ideally, we would never hit these cases
            {
                LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_DEFAULT, "init_dnsproxy_service: XPCError: " PUB_S,
                          xpc_dictionary_get_string(eventmsg, XPC_ERROR_KEY_DESCRIPTION));
                return;
            }
            else
            {
                LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_DEFAULT, "init_dnsproxy_service: Unknown EventMsg type");
                return;
            }
        });

    xpc_connection_resume(dps_listener);
}

mDNSlocal void accept_dps_client(xpc_connection_t conn)
{
    uid_t c_euid;
    int   c_pid;
    c_euid  = xpc_connection_get_euid(conn);
    c_pid   = xpc_connection_get_pid(conn);

    if (c_euid != 0 || !IsEntitled(conn, kDNSProxyService))
    {
        LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_DEFAULT,
                  "accept_dps_client: DNSProxyService Client PID[%d] is missing Entitlement or is not running as root!",
                  c_pid);
        xpc_connection_cancel(conn);
        return;
    }

    xpc_retain(conn);
    xpc_connection_set_target_queue(conn, dps_queue);
    xpc_connection_set_event_handler(conn, ^(xpc_object_t req_msg)
        {
            xpc_type_t type = xpc_get_type(req_msg);

            if (type == XPC_TYPE_DICTIONARY)
            {
                handle_dps_request(req_msg);
            }
            else // We hit this case ONLY if Client Terminated DPS Connection OR Crashed
            {
                LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_DEFAULT,
                          "accept_dps_client: DPS Client %p teared down the connection or Crashed", (void *) conn);
                // Only the Client that has activated DPS should be able to terminate it
                if (c_pid == dps_client_pid)
                    handle_dps_terminate();
                xpc_release(conn);
            }
        });

    xpc_connection_resume(conn);
}

mDNSlocal void handle_dps_request(xpc_object_t req)
{
    int dps_tmp_client;
    mDNSBool proxy_off = mDNSfalse;
    xpc_connection_t remote_conn = xpc_dictionary_get_remote_connection(req);
    dps_tmp_client = (int) xpc_connection_get_pid(remote_conn);

    LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_DEFAULT, "handle_dps_request: Handler for DNS Proxy Requests");

    if (dps_client_pid <= 0)
    {
        LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_INFO, "handle_dps_request: DNSProxy is not engaged (New Client)");
        // No Active Client, save new Client's PID (also indicates DNS Proxy was OFF)
        dps_client_pid = dps_tmp_client;
        proxy_off = mDNStrue;
    }
    else
    {
        // We already have an active DNS Proxy Client and until that client does not terminate the connection
        // or crashes, a new client cannot change/override the current DNS Proxy settings.
        if (dps_client_pid != dps_tmp_client)
        {
            LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_DEFAULT,
                      "handle_dps_request: A Client is already using DNS Proxy and your request cannot be handled at this time");
            // Return Engaged Status to the client
            xpc_object_t reply = xpc_dictionary_create_reply(req);
            if (reply)
            {
                xpc_dictionary_set_uint64(reply, kDNSDaemonReply, kDNSMsg_Busy);
                xpc_connection_send_message(remote_conn, reply);
                xpc_release(reply);
            }
            else
            {
                LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_DEFAULT,
                          "handle_dps_request: Reply Dictionary could not be created");
                return;
            }
            // We do not really need to terminate the connection with the client
            // as it may try again later which is fine
            return;
        }
    }


    xpc_object_t response = xpc_dictionary_create_reply(req);
    // Return Success Status to the client
    if (response)
    {
        xpc_dictionary_set_uint64(response, kDNSDaemonReply, kDNSMsg_NoError);
        xpc_connection_send_message(remote_conn, response);
        xpc_release(response);
    }
    else
    {
        LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_DEFAULT,
                  "handle_dps_request: Response Dictionary could not be created");
        return;
    }

    // Proceed to get DNS Proxy Settings from the Client
    if (xpc_dictionary_get_uint64(req, kDNSProxyParameters))
    {
        mDNSu32 inIf[MaxIp], outIf;

        inIf[0]   = (mDNSu32)xpc_dictionary_get_uint64(req, kDNSInIfindex0);
        inIf[1]   = (mDNSu32)xpc_dictionary_get_uint64(req, kDNSInIfindex1);
        inIf[2]   = (mDNSu32)xpc_dictionary_get_uint64(req, kDNSInIfindex2);
        inIf[3]   = (mDNSu32)xpc_dictionary_get_uint64(req, kDNSInIfindex3);
        inIf[4]   = (mDNSu32)xpc_dictionary_get_uint64(req, kDNSInIfindex4);
        outIf     = (mDNSu32)xpc_dictionary_get_uint64(req, kDNSOutIfindex);

        ActivateDNSProxy(inIf, outIf, proxy_off);
    }
}

mDNSlocal void handle_dps_terminate()
{

    LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_INFO,
              "handle_dps_terminate: Client PID[%d] terminated connection or crashed. Proceed to terminate DNSProxy",
              dps_client_pid);
    // Clear the Client's PID, so that we can now accept new DPS requests
    dps_client_pid = 0;

    KQueueLock();
    mDNSPlatformCloseDNSProxySkts(&mDNSStorage);
    // TBD: Close TCP Sockets
    DNSProxyTerminate();
    KQueueUnlock("DNSProxy Deactivated");
}

mDNSlocal void ActivateDNSProxy(mDNSu32 IpIfArr[MaxIp], mDNSu32 OpIf, mDNSBool proxy_off)
{
    LogRedact(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_INFO,
              "ActivateDNSProxy: InterfaceIndex List by Client: Input[%d, %d, %d, %d, %d] Output[%d]",
              IpIfArr[0], IpIfArr[1], IpIfArr[2], IpIfArr[3], IpIfArr[4], OpIf);

    KQueueLock();
    DNSProxyInit(IpIfArr, OpIf);
    if (proxy_off) // Open skts only if proxy was OFF else we may end up opening extra skts
        mDNSPlatformInitDNSProxySkts(ProxyUDPCallback, ProxyTCPCallback);
    KQueueUnlock("DNSProxy Activated");
}
