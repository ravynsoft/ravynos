/*
 * Copyright (c) 2007-2019 Apple Inc. All rights reserved.
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

#include <mach/mach.h>
#include <mach/mach_error.h>
#include <mach/vm_map.h>
#include <servers/bootstrap.h>
#include <IOKit/IOReturn.h>
#include <CoreFoundation/CoreFoundation.h>
#include "helper.h"
#include <dispatch/dispatch.h>
#include <arpa/inet.h>
#include <xpc/private.h>
#include <Block.h>

//
// Implementation Notes about the HelperQueue:
//
// To prevent blocking the main queue, all communications with mDNSResponderHelper should happen on
// HelperQueue. There are a few calls which are still synchronous and needs to be handled separately
// case by case.
//
// When spawning off the work to the HelperQueue, any arguments that are pointers need to be copied
// explicitly as they may cease to exist after the call returns. From within the block that is scheduled,
// arrays defined on the stack can't be referenced and hence it is enclosed them in a struct. If the array is
// an argument to the function, the blocks can reference them as they are passed in as pointers. But care should
// be taken to copy them locally as they may cease to exist when the function returns.
//


//*************************************************************************************************************
// Globals
static dispatch_queue_t HelperQueue;

static int64_t maxwait_secs = 5LL;

#define mDNSHELPER_DEBUG LogOperation

//*************************************************************************************************************
// Utility Functions

static void HelperLog(const char *prefix, xpc_object_t o)
{
    char *desc = xpc_copy_description(o);
    mDNSHELPER_DEBUG("HelperLog %s: %s", prefix, desc);
    free(desc);
}

//*************************************************************************************************************
// XPC Funcs:
//*************************************************************************************************************


mDNSlocal xpc_connection_t Create_Connection(void)
{
    xpc_connection_t connection = xpc_connection_create_mach_service(kHelperService, HelperQueue,
        XPC_CONNECTION_MACH_SERVICE_PRIVILEGED);
    if (connection)
    {
        xpc_connection_set_event_handler(connection, ^(xpc_object_t event)
        {
            mDNSHELPER_DEBUG("Create_Connection xpc: [%s] \n", xpc_dictionary_get_string(event, XPC_ERROR_KEY_DESCRIPTION));
        });
        xpc_connection_activate(connection);
    }
    return connection;
}

mDNSlocal int SendDict_ToServer(xpc_object_t msg, xpc_object_t *out_reply)
{
    xpc_connection_t connection;
    dispatch_semaphore_t sem = NULL;
    __block xpc_object_t reply = NULL;
    __block int errorcode = kHelperErr_NoResponse;
    
    HelperLog("SendDict_ToServer Sending msg to Daemon", msg);
    
    connection = Create_Connection();
    if (!connection)
    {
        goto exit;
    }

    sem = dispatch_semaphore_create(0);
    if (!sem)
    {
        goto exit;
    }
    
    dispatch_retain(sem); // for the block below
    xpc_connection_send_message_with_reply(connection, msg, HelperQueue, ^(xpc_object_t recv_msg)
    {
        const xpc_type_t type = xpc_get_type(recv_msg);
                                               
        if (type == XPC_TYPE_DICTIONARY)
        {
            HelperLog("SendDict_ToServer Received reply msg from Daemon", recv_msg);
            uint64_t reply_status = xpc_dictionary_get_uint64(recv_msg, kHelperReplyStatus);
            errorcode = (int)xpc_dictionary_get_int64(recv_msg, kHelperErrCode);
            
            switch (reply_status)
            {
                case kHelperReply_ACK:
                    mDNSHELPER_DEBUG("NoError: successful reply");
                    break;
                default:
                    LogMsg("default: Unexpected reply from Helper");
                    break;
            }
            reply = recv_msg;
            xpc_retain(reply);
        }
        else
        {
            LogMsg("SendDict_ToServer Received unexpected reply from daemon [%s]",
                    xpc_dictionary_get_string(recv_msg, XPC_ERROR_KEY_DESCRIPTION));
            HelperLog("SendDict_ToServer Unexpected Reply contents", recv_msg);
        }
        
        dispatch_semaphore_signal(sem);
        dispatch_release(sem);
    });
    
    if (dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, (maxwait_secs * NSEC_PER_SEC))) != 0)
    {
        LogMsg("SendDict_ToServer: UNEXPECTED WAIT_TIME in dispatch_semaphore_wait");

        // If we insist on using a semaphore timeout, then cancel the connection if the timeout is reached.
        // This forces the reply block to be called if a reply wasn't received to keep things serialized.
        xpc_connection_cancel(connection);
        dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);
    }
    if (out_reply)
    {
        *out_reply = reply;
        reply = NULL;
    }
    
    mDNSHELPER_DEBUG("SendDict_ToServer returning with errorcode[%d]", errorcode);
    
exit:
    if (connection)
    {
        xpc_connection_cancel(connection);
        xpc_release(connection);
    }
    if (sem)
    {
        dispatch_release(sem);
    }
    if (reply)
    {
        xpc_release(reply);
    }
    return errorcode;
}

//**************************************************************************************************************

mDNSexport mStatus mDNSHelperInit()
{
    HelperQueue = dispatch_queue_create("com.apple.mDNSResponder.HelperQueue", NULL);
    if (HelperQueue == NULL)
    {
        LogMsg("dispatch_queue_create: Helper queue NULL");
        return mStatus_NoMemoryErr;
    }
    return mStatus_NoError;
}

void mDNSPreferencesSetName(int key, domainlabel *old, domainlabel *new)
{
    struct
    {
        char oldname[MAX_DOMAIN_LABEL+1];
        char newname[MAX_DOMAIN_LABEL+1];
    } names;

    mDNSPlatformMemZero(names.oldname, MAX_DOMAIN_LABEL + 1);
    mDNSPlatformMemZero(names.newname, MAX_DOMAIN_LABEL + 1);

    ConvertDomainLabelToCString_unescaped(old, names.oldname);
    
    if (new)
        ConvertDomainLabelToCString_unescaped(new, names.newname);
    
    
    mDNSHELPER_DEBUG("mDNSPreferencesSetName: XPC IPC Test oldname %s newname %s", names.oldname, names.newname);
     
    // Create Dictionary To Send
    xpc_object_t dict = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_uint64(dict, kHelperMode, set_name);
    
    xpc_dictionary_set_uint64(dict, kPrefsNameKey, key);
    xpc_dictionary_set_string(dict, kPrefsOldName, names.oldname);
    xpc_dictionary_set_string(dict, kPrefsNewName, names.newname);
    
    SendDict_ToServer(dict, NULL);
    xpc_release(dict);
    dict = NULL;
    
}

void mDNSRequestBPF()
{
     mDNSHELPER_DEBUG("mDNSRequestBPF: Using XPC IPC");
     
     // Create Dictionary To Send
     xpc_object_t dict = xpc_dictionary_create(NULL, NULL, 0);
     xpc_dictionary_set_uint64(dict, kHelperMode, bpf_request);
     SendDict_ToServer(dict, NULL);
     xpc_release(dict);
     dict = NULL;

}

int mDNSPowerRequest(int key, int interval)
{
    int err_code = kHelperErr_NotConnected;
    
    mDNSHELPER_DEBUG("mDNSPowerRequest: Using XPC IPC calling out to Helper key is [%d] interval is [%d]", key, interval);
    
    // Create Dictionary To Send
    xpc_object_t dict = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_uint64(dict, kHelperMode, power_req);
    xpc_dictionary_set_uint64(dict, "powerreq_key", key);
    xpc_dictionary_set_uint64(dict, "powerreq_interval", interval);
    
    err_code = SendDict_ToServer(dict, NULL);
    xpc_release(dict);
    dict = NULL;
    
    mDNSHELPER_DEBUG("mDNSPowerRequest: Using XPC IPC returning error_code %d", err_code);
    return err_code;
}

int mDNSSetLocalAddressCacheEntry(int ifindex, int family, const v6addr_t ip, const ethaddr_t eth)
{
    int err_code = kHelperErr_NotConnected;
    
    mDNSHELPER_DEBUG("mDNSSetLocalAddressCacheEntry: Using XPC IPC calling out to Helper: ifindex is [%d] family is [%d]", ifindex, family);
    
    // Create Dictionary To Send
    xpc_object_t dict = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_uint64(dict, kHelperMode, set_localaddr_cacheentry);
    
    xpc_dictionary_set_uint64(dict, "slace_ifindex", ifindex);
    xpc_dictionary_set_uint64(dict, "slace_family", family);
    
    xpc_dictionary_set_data(dict, "slace_ip", (uint8_t*)ip, sizeof(v6addr_t));
    xpc_dictionary_set_data(dict, "slace_eth", (uint8_t*)eth, sizeof(ethaddr_t));
    
    err_code = SendDict_ToServer(dict, NULL);
    xpc_release(dict);
    dict = NULL;
    
    mDNSHELPER_DEBUG("mDNSSetLocalAddressCacheEntry: Using XPC IPC returning error_code %d", err_code);
    return err_code;
}


void mDNSNotify(const char *title, const char *msg) // Both strings are UTF-8 text
{
    mDNSHELPER_DEBUG("mDNSNotify() calling out to Helper XPC IPC title[%s] msg[%s]", title, msg);
    
    // Create Dictionary To Send
    xpc_object_t dict = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_uint64(dict, kHelperMode, user_notify);
    
    xpc_dictionary_set_string(dict, "notify_title", title);
    xpc_dictionary_set_string(dict, "notify_msg", msg);
    
    SendDict_ToServer(dict, NULL);
    xpc_release(dict);
    dict = NULL;
    
}


int mDNSKeychainGetSecrets(CFArrayRef *result)
{
    
    CFPropertyListRef plist = NULL;
    CFDataRef bytes = NULL;
    unsigned int numsecrets = 0;
    size_t secretsCnt = 0;
    int error_code = kHelperErr_NotConnected;
    xpc_object_t reply_dict = NULL;
    const void *sec = NULL;
    
    mDNSHELPER_DEBUG("mDNSKeychainGetSecrets: Using XPC IPC calling out to Helper");
    
    // Create Dictionary To Send
    xpc_object_t dict = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_uint64(dict, kHelperMode, keychain_getsecrets);

    SendDict_ToServer(dict, &reply_dict);
 
    if (reply_dict != NULL)
    {
        numsecrets = xpc_dictionary_get_uint64(reply_dict, "keychain_num_secrets");
        sec = xpc_dictionary_get_data(reply_dict, "keychain_secrets", &secretsCnt);
        error_code = xpc_dictionary_get_int64(reply_dict,   kHelperErrCode);
    }
 
    mDNSHELPER_DEBUG("mDNSKeychainGetSecrets: Using XPC IPC calling out to Helper: numsecrets is %u, secretsCnt is %u error_code is %d",
                     numsecrets, (unsigned int)secretsCnt, error_code);
     
    if (NULL == (bytes = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, (void*)sec, secretsCnt, kCFAllocatorNull)))
    {
        error_code = kHelperErr_ApiErr;
        LogMsg("mDNSKeychainGetSecrets: CFDataCreateWithBytesNoCopy failed");
        goto fin;
    }
    
    if (NULL == (plist = CFPropertyListCreateWithData(kCFAllocatorDefault, bytes, kCFPropertyListImmutable, NULL, NULL)))
    {
        error_code = kHelperErr_ApiErr;
        LogMsg("mDNSKeychainGetSecrets: CFPropertyListCreateFromXMLData failed");
        goto fin;
    }
    
    if (CFArrayGetTypeID() != CFGetTypeID(plist))
    {
        error_code = kHelperErr_ApiErr;
        LogMsg("mDNSKeychainGetSecrets: Unexpected result type");
        CFRelease(plist);
        plist = NULL;
        goto fin;
    }
    
    *result = (CFArrayRef)plist;
    
    
fin:
    if (bytes)
        CFRelease(bytes);
    if (dict)
        xpc_release(dict);
    if (reply_dict)
        xpc_release(reply_dict);
    
    dict = NULL;
    reply_dict = NULL;
    
    return error_code;
}

void mDNSSendWakeupPacket(unsigned int ifid, char *eth_addr, char *ip_addr, int iteration)
{
    // (void) ip_addr; // unused
    // (void) iteration; // unused

    mDNSHELPER_DEBUG("mDNSSendWakeupPacket: Entered ethernet address[%s],ip_address[%s], interface_id[%d], iteration[%d]",
           eth_addr, ip_addr, ifid, iteration);
    
    // Create Dictionary To Send
    xpc_object_t dict = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_uint64(dict, kHelperMode, send_wakepkt);
    
    xpc_dictionary_set_uint64(dict, "interface_index", ifid);
    xpc_dictionary_set_string(dict, "ethernet_address", eth_addr);
    xpc_dictionary_set_string(dict, "ip_address", ip_addr);
    xpc_dictionary_set_uint64(dict, "swp_iteration", iteration);
    
    SendDict_ToServer(dict, NULL);
    xpc_release(dict);
    dict = NULL;

}

void mDNSPacketFilterControl(uint32_t command, char * ifname, uint32_t count, pfArray_t portArray, pfArray_t protocolArray)
{
    struct
    {
        pfArray_t portArray;
        pfArray_t protocolArray;
    } pfa;
    
    mDNSPlatformMemCopy(pfa.portArray, portArray, sizeof(pfArray_t));
    mDNSPlatformMemCopy(pfa.protocolArray, protocolArray, sizeof(pfArray_t));

    mDNSHELPER_DEBUG("mDNSPacketFilterControl: XPC IPC, ifname %s", ifname);
    
    // Create Dictionary To Send
    xpc_object_t dict = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_uint64(dict, kHelperMode, p2p_packetfilter);
    
    xpc_dictionary_set_uint64(dict, "pf_opcode", command);
    if (ifname)
        xpc_dictionary_set_string(dict, "pf_ifname", ifname);

    xpc_object_t xpc_obj_portArray = xpc_array_create(NULL, 0);
    xpc_object_t xpc_obj_protocolArray = xpc_array_create(NULL, 0);

    for (size_t i = 0; i < count && i < PFPortArraySize; i++) {
        xpc_array_set_uint64(xpc_obj_portArray, XPC_ARRAY_APPEND, pfa.portArray[i]);
        xpc_array_set_uint64(xpc_obj_protocolArray, XPC_ARRAY_APPEND, pfa.protocolArray[i]);
    }
    xpc_dictionary_set_value(dict, "xpc_obj_array_port", xpc_obj_portArray);
    xpc_dictionary_set_value(dict, "xpc_obj_array_protocol", xpc_obj_protocolArray);
    xpc_release(xpc_obj_portArray);
    xpc_release(xpc_obj_protocolArray);
    
    SendDict_ToServer(dict, NULL);
    xpc_release(dict);
    dict = NULL;
    
    mDNSHELPER_DEBUG("mDNSPacketFilterControl: portArray0[%d] portArray1[%d] portArray2[%d] portArray3[%d] protocolArray0[%d] protocolArray1[%d] protocolArray2[%d] protocolArray3[%d]",
            pfa.portArray[0], pfa.portArray[1], pfa.portArray[2], pfa.portArray[3], pfa.protocolArray[0], pfa.protocolArray[1], pfa.protocolArray[2], pfa.protocolArray[3]);
    
}

void mDNSSendKeepalive(const v6addr_t sadd, const v6addr_t dadd, uint16_t lport, uint16_t rport, uint32_t seq, uint32_t ack, uint16_t win)
{

    mDNSHELPER_DEBUG("mDNSSendKeepalive: Using XPC IPC calling out to Helper: lport is[%d] rport is[%d] seq is[%d] ack is[%d] win is[%d]",
           lport, rport, seq, ack, win);
    
    char buf1[INET6_ADDRSTRLEN];
    char buf2[INET6_ADDRSTRLEN];
    
    buf1[0] = 0;
    buf2[0] = 0;
    
    inet_ntop(AF_INET6, sadd, buf1, sizeof(buf1));
    inet_ntop(AF_INET6, dadd, buf2, sizeof(buf2));
    mDNSHELPER_DEBUG("mDNSSendKeepalive: Using XPC IPC calling out to Helper: sadd is %s, dadd is %s", buf1, buf2);
    
    // Create Dictionary To Send
    xpc_object_t dict = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_uint64(dict, kHelperMode, send_keepalive);
    
    xpc_dictionary_set_data(dict, "send_keepalive_sadd", (uint8_t*)sadd, sizeof(v6addr_t));
    xpc_dictionary_set_data(dict, "send_keepalive_dadd", (uint8_t*)dadd, sizeof(v6addr_t));
    
    xpc_dictionary_set_uint64(dict, "send_keepalive_lport", lport);
    xpc_dictionary_set_uint64(dict, "send_keepalive_rport", rport);
    xpc_dictionary_set_uint64(dict, "send_keepalive_seq", seq);
    xpc_dictionary_set_uint64(dict, "send_keepalive_ack", ack);
    xpc_dictionary_set_uint64(dict, "send_keepalive_win", win);
    
    SendDict_ToServer(dict, NULL);
    xpc_release(dict);
    dict = NULL;
    
}

int mDNSRetrieveTCPInfo(int family, v6addr_t laddr, uint16_t lport, v6addr_t raddr, uint16_t rport, uint32_t *seq, uint32_t *ack, uint16_t *win, int32_t *intfid)
{
    int error_code = kHelperErr_NotConnected;
    xpc_object_t reply_dict = NULL;
    
    mDNSHELPER_DEBUG("mDNSRetrieveTCPInfo: Using XPC IPC calling out to Helper: lport is[%d] rport is[%d] family is[%d]",
           lport, rport, family);
    
    char buf1[INET6_ADDRSTRLEN];
    char buf2[INET6_ADDRSTRLEN];
    buf1[0] = 0;
    buf2[0] = 0;
    
    inet_ntop(AF_INET6, laddr, buf1, sizeof(buf1));
    inet_ntop(AF_INET6, raddr, buf2, sizeof(buf2));
    mDNSHELPER_DEBUG("mDNSRetrieveTCPInfo:: Using XPC IPC calling out to Helper: laddr is %s, raddr is %s", buf1, buf2);
    
    // Create Dictionary To Send
    xpc_object_t dict = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_uint64(dict, kHelperMode, retreive_tcpinfo);
    
    xpc_dictionary_set_data(dict, "retreive_tcpinfo_laddr", (uint8_t*)laddr, sizeof(v6addr_t));
    xpc_dictionary_set_data(dict, "retreive_tcpinfo_raddr", (uint8_t*)raddr, sizeof(v6addr_t));
    
    xpc_dictionary_set_uint64(dict, "retreive_tcpinfo_family", family);
    xpc_dictionary_set_uint64(dict, "retreive_tcpinfo_lport", lport);
    xpc_dictionary_set_uint64(dict, "retreive_tcpinfo_rport", rport);
    
    SendDict_ToServer(dict, &reply_dict);
    
    if (reply_dict != NULL)
    {
        *seq = xpc_dictionary_get_uint64(reply_dict, "retreive_tcpinfo_seq");
        *ack = xpc_dictionary_get_uint64(reply_dict, "retreive_tcpinfo_ack");
        *win = xpc_dictionary_get_uint64(reply_dict, "retreive_tcpinfo_win");
        *intfid = (int32_t)xpc_dictionary_get_uint64(reply_dict, "retreive_tcpinfo_ifid");
        error_code = xpc_dictionary_get_int64(reply_dict, kHelperErrCode);
    }
    
    mDNSHELPER_DEBUG("mDNSRetrieveTCPInfo: Using XPC IPC calling out to Helper: seq is %d, ack is %d, win is %d, intfid is %d, error is %d",
           *seq, *ack, *win, *intfid, error_code);
    
    if (dict)
        xpc_release(dict);
    if (reply_dict)
        xpc_release(reply_dict);
    dict = NULL;
    reply_dict = NULL;

    return error_code;
}
