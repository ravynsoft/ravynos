/*
 * Copyright (c) 2012-2019 Apple Inc. All rights reserved.
 *
 * PRIVATE DNSX CLIENT LIBRARY --FOR Apple Platforms ONLY OSX/iOS--
 * Resides in /usr/lib/libdns_services.dylib
 */

#include "dns_services.h"
#include <xpc/xpc.h>
#include <Block.h>
#include <os/log.h>
#include "xpc_clients.h"

//*************************************************************************************************************
// Globals

#define connection_t xpc_connection_t

struct _DNSXConnRef_t
{
    connection_t          conn_ref;      // xpc_connection between client and daemon
    dispatch_queue_t      lib_q;         // internal queue created in library itself
    DNSXEnableProxyReply  AppCallBack;   // Callback function ptr for Client
    dispatch_queue_t      client_q;      // Queue specified by client for scheduling its Callback
};

//*************************************************************************************************************
// Utility Functions

static bool LogDebugEnabled()
{
    return true;
}

static void LogDebug(const char *prefix, xpc_object_t o)
{
    if (!LogDebugEnabled())
        return;
    
    char *desc = xpc_copy_description(o);
    os_log_info(OS_LOG_DEFAULT, "%s: %s", prefix, desc);
    free(desc);
}

//**************************************************************************************************************

void DNSXRefDeAlloc(DNSXConnRef connRef)
{
    if (connRef == NULL)
    {
        os_log(OS_LOG_DEFAULT, "dns_services: DNSXRefDeAlloc called with NULL DNSXConnRef");
        return;
    }
    
    // Schedule this work on the internal library queue
    dispatch_sync(connRef->lib_q, ^{
        xpc_connection_set_event_handler((connRef)->conn_ref, ^(__unused xpc_object_t event){}); // ignore any more events
        xpc_release(connRef->conn_ref);
        connRef->conn_ref = NULL;
        dispatch_release(connRef->lib_q);
        connRef->lib_q = NULL;
        connRef->AppCallBack = NULL;
        os_log_info(OS_LOG_DEFAULT, "dns_services: DNSXRefDeAlloc successfully DeAllocated conn_ref & lib_q");
        
        dispatch_async((connRef)->client_q, ^{
            dispatch_release(connRef->client_q);
            connRef->client_q = NULL;
            free(connRef);
            os_log_info(OS_LOG_DEFAULT, "dns_services: DNSXRefDeAlloc successfully DeAllocated client_q & freed connRef");
        });
    });
    
    // DO NOT reference connRef after this comment, as it may have been freed
    os_log_info(OS_LOG_DEFAULT, "dns_services: DNSXRefDeAlloc successfully DeAllocated connRef");
    
}

// Sends the Msg(Dictionary) to the Server Daemon
static DNSXErrorType SendMsgToServer(DNSXConnRef connRef, xpc_object_t msg)
{
    DNSXErrorType errx = kDNSX_NoError;
    
    LogDebug("dns_services: SendMsgToServer Sending msg to Daemon", msg);
    
    xpc_connection_send_message_with_reply((connRef)->conn_ref, msg, (connRef)->lib_q, ^(xpc_object_t recv_msg)
    {
        xpc_type_t type = xpc_get_type(recv_msg);
                                               
        if (type == XPC_TYPE_DICTIONARY)
        {
            LogDebug("dns_services: SendMsgToServer Received reply msg from Daemon", recv_msg);
            uint64_t daemon_status = xpc_dictionary_get_uint64(recv_msg, kDNSDaemonReply);
                                                   
            if (connRef == NULL || connRef->client_q == NULL || connRef->AppCallBack == NULL)
            {
                // If connRef is bad, do not schedule any callbacks to the client
                os_log(OS_LOG_DEFAULT, "dns_services: SendMsgToServer: connRef is BAD Daemon status code [%llu]", daemon_status);
            }
            else
            {
                switch (daemon_status)
                {
                    case kDNSMsg_NoError:
                        dispatch_async((connRef)->client_q, ^{
                        if (connRef->AppCallBack != NULL)
                            connRef->AppCallBack(connRef, kDNSX_NoError);
                        });
                        break;
                                                             
                    case kDNSMsg_Busy:
                        os_log(OS_LOG_DEFAULT, "dns_services: SendMsgToServer: DNS Proxy already in use");
                        dispatch_async((connRef)->client_q, ^{
                        if (connRef->AppCallBack != NULL)
                            connRef->AppCallBack(connRef, kDNSX_Busy);
                        });
                        break;
                                                               
                    default:
                        os_log(OS_LOG_DEFAULT, "dns_services: SendMsgToServer: Unknown error");
                        dispatch_async((connRef)->client_q, ^{
                        if (connRef->AppCallBack != NULL)
                            connRef->AppCallBack(connRef, kDNSX_UnknownErr);
                        });
                        break;
                }
            }
        }
        else
        {
            os_log(OS_LOG_DEFAULT, "dns_services: SendMsgToServer Received unexpected reply from daemon [%s]",
                                xpc_dictionary_get_string(recv_msg, XPC_ERROR_KEY_DESCRIPTION));
            LogDebug("dns_services: SendMsgToServer Unexpected Reply contents", recv_msg);
        }
    });
    
    return errx;
}

// Creates a new DNSX Connection Reference(DNSXConnRef)
static DNSXErrorType InitConnection(DNSXConnRef *connRefOut, const char *servname, dispatch_queue_t clientq, DNSXEnableProxyReply AppCallBack)
{
    if (connRefOut == NULL)
    {
        os_log(OS_LOG_DEFAULT, "dns_services: InitConnection() connRef cannot be NULL");
        return kDNSX_BadParam;
    }
    
    // Use a DNSXConnRef on the stack to be captured in the blocks below, rather than capturing the DNSXConnRef* owned by the client
    DNSXConnRef connRef = malloc(sizeof(struct _DNSXConnRef_t));
    if (connRef == NULL)
    {
        os_log(OS_LOG_DEFAULT, "dns_services: InitConnection() No memory to allocate!");
        return kDNSX_NoMem;
    }
    
    // Initialize the DNSXConnRef
    dispatch_retain(clientq);
    connRef->client_q     = clientq;
    connRef->AppCallBack  = AppCallBack;
    connRef->lib_q        = dispatch_queue_create("com.apple.mDNSResponder.libdns_services.q", DISPATCH_QUEUE_SERIAL);
    connRef->conn_ref     = xpc_connection_create_mach_service(servname, connRef->lib_q, XPC_CONNECTION_MACH_SERVICE_PRIVILEGED);
    
    if (connRef->conn_ref == NULL || connRef->lib_q == NULL)
    {
        os_log(OS_LOG_DEFAULT, "dns_services: InitConnection() conn_ref/lib_q is NULL");
        if (connRef != NULL)
            free(connRef);
        return kDNSX_NoMem;
    }
    
    xpc_connection_set_event_handler(connRef->conn_ref, ^(xpc_object_t event)
    {
        if (connRef == NULL || connRef->client_q == NULL || connRef->AppCallBack == NULL)
        {
            // If connRef is bad, do not schedule any callbacks to the client
            os_log(OS_LOG_DEFAULT, "dns_services: InitConnection: connRef is BAD Unexpected Connection Error [%s]",
                                xpc_dictionary_get_string(event, XPC_ERROR_KEY_DESCRIPTION));
        }
        else
        {
            os_log(OS_LOG_DEFAULT, "dns_services: InitConnection: Unexpected Connection Error [%s] Ping the client",
                                 xpc_dictionary_get_string(event, XPC_ERROR_KEY_DESCRIPTION));
            dispatch_async(connRef->client_q, ^{
            if (connRef->AppCallBack != NULL)
                connRef->AppCallBack(connRef, kDNSX_DaemonNotRunning);
            });
        }
                                         
    });
    xpc_connection_resume(connRef->conn_ref);
    
    *connRefOut = connRef;
    
    return kDNSX_NoError;
}

DNSXErrorType DNSXEnableProxy(DNSXConnRef *connRef, DNSProxyParameters proxyparam, IfIndex inIfindexArr[MaxInputIf],
                              IfIndex outIfindex, dispatch_queue_t clientq, DNSXEnableProxyReply callBack)
{
    
    DNSXErrorType errx = kDNSX_NoError;
    
    // Sanity Checks
    if (connRef == NULL || callBack == NULL || clientq == NULL)
    {
        os_log(OS_LOG_DEFAULT, "dns_services: DNSXEnableProxy called with NULL DNSXConnRef OR Callback OR ClientQ parameter");
        return kDNSX_BadParam;
    }
    
    // Get connRef from InitConnection()
    if (*connRef == NULL)
    {
        errx = InitConnection(connRef, kDNSProxyService, clientq, callBack);
        if (errx) // On error InitConnection() leaves *connRef set to NULL
        {
            os_log(OS_LOG_DEFAULT, "dns_services: Since InitConnection() returned %d error returning w/o sending msg", errx);
            return errx;
        }
    }
    else // Client already has a connRef and this is not valid use for this SPI
    {
        os_log(OS_LOG_DEFAULT, "dns_services: Client already has a valid connRef! This is incorrect usage from the client");
        return kDNSX_BadParam;
    }
    
    // Create Dictionary To Send
    xpc_object_t dict = xpc_dictionary_create(NULL, NULL, 0);
    if (dict == NULL)
    {
        os_log(OS_LOG_DEFAULT, "dns_services: DNSXEnableProxy could not create the Msg Dict To Send!");
        DNSXRefDeAlloc(*connRef);
        return kDNSX_NoMem;
    }
    
    xpc_dictionary_set_uint64(dict, kDNSProxyParameters, proxyparam);
    
    xpc_dictionary_set_uint64(dict, kDNSInIfindex0,      inIfindexArr[0]);
    xpc_dictionary_set_uint64(dict, kDNSInIfindex1,      inIfindexArr[1]);
    xpc_dictionary_set_uint64(dict, kDNSInIfindex2,      inIfindexArr[2]);
    xpc_dictionary_set_uint64(dict, kDNSInIfindex3,      inIfindexArr[3]);
    xpc_dictionary_set_uint64(dict, kDNSInIfindex4,      inIfindexArr[4]);
    
    xpc_dictionary_set_uint64(dict, kDNSOutIfindex,      outIfindex);
    
    errx = SendMsgToServer(*connRef, dict);
    xpc_release(dict);
    dict = NULL;
    
    return errx;
}

