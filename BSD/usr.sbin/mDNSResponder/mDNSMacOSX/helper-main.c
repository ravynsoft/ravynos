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

#define _FORTIFY_SOURCE 2

#include <CoreFoundation/CoreFoundation.h>
#include <sys/cdefs.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <servers/bootstrap.h>
#include <launch.h>
#include <pwd.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <Security/Security.h>
#include "helper.h"
#include "helper-server.h"
#include <xpc/private.h>

#if TARGET_OS_IPHONE
#define NO_SECURITYFRAMEWORK 1
#endif

#ifndef LAUNCH_JOBKEY_MACHSERVICES
#define LAUNCH_JOBKEY_MACHSERVICES "MachServices"
#define LAUNCH_DATA_MACHPORT 10
#define launch_data_get_machport launch_data_get_fd
#endif


int mDNSHelperLogEnabled = 0;
os_log_t  log_handle = NULL;

static dispatch_queue_t xpc_queue = NULL;
static int opt_debug;
static pthread_t idletimer_thread;

unsigned long maxidle = 15;
unsigned long actualidle = 3600;

CFRunLoopRef gRunLoop = NULL;
CFRunLoopTimerRef gTimer = NULL;


static void handle_sigterm(int sig)
{
    // os_log_debug(log_handle,"entry sig=%d", sig);	Can't use syslog from within a signal handler
    assert(sig == SIGTERM);
    helper_exit();
}

static void initialize_logging(void)
{
    log_handle   = os_log_create("com.apple.mDNSResponderHelper", "INFO");
    
    if (!log_handle)
    {
        // OS_LOG_DEFAULT is the default logging object, if you are not creating a custom subsystem/category
        os_log_error(OS_LOG_DEFAULT, "Could NOT create log handle in mDNSResponderHelper");
    }
    
}

static void initialize_id(void)
{
    static char login[] = "_mdnsresponder";
    struct passwd hardcode;
    struct passwd *pwd = &hardcode; // getpwnam(login);
    hardcode.pw_uid = 65;
    hardcode.pw_gid = 65;

    if (!pwd)
    {
        os_log(log_handle, "Could not find account name `%s'.  I will only help root.", login);
        return;
    }
    mDNSResponderUID = pwd->pw_uid;
    mDNSResponderGID = pwd->pw_gid;
}

static void diediedie(CFRunLoopTimerRef timer, void *context)
{
    os_log_info(log_handle, "entry %p %p %lu", timer, context, actualidle);
    
    assert(gTimer == timer);
    os_log_info(log_handle, "mDNSResponderHelper exiting after [%lu] seconds", actualidle);
    
    if (actualidle)
        helper_exit();
}

void pause_idle_timer(void)
{
    os_log_debug(log_handle,"entry");
    assert(gTimer);
    assert(gRunLoop);
    CFRunLoopRemoveTimer(gRunLoop, gTimer, kCFRunLoopDefaultMode);
}

void unpause_idle_timer(void)
{
    os_log_debug(log_handle,"entry");
    assert(gRunLoop);
    assert(gTimer);
    CFRunLoopAddTimer(gRunLoop, gTimer, kCFRunLoopDefaultMode);
}

void update_idle_timer(void)
{
    os_log_debug(log_handle,"entry");
    assert(gTimer);
    CFRunLoopTimerSetNextFireDate(gTimer, CFAbsoluteTimeGetCurrent() + actualidle);
}

static void *idletimer(void *context)
{
    os_log_debug(log_handle,"entry context=%p", context);
    gRunLoop = CFRunLoopGetMain();

    unpause_idle_timer();

    for (;;)
    {
        // os_log_debug(log_handle,"Running CFRunLoop");
        CFRunLoopRun();
        sleep(1);
    }

    return NULL;
}

static int initialize_timer()
{
    gTimer = CFRunLoopTimerCreate(kCFAllocatorDefault, CFAbsoluteTimeGetCurrent() + actualidle, actualidle, 0, 0, diediedie, NULL);
    int err = 0;
    os_log_info(log_handle, "mDNSResponderHelper initialize_timer() started");

    if (0 != (err = pthread_create(&idletimer_thread, NULL, idletimer, NULL)))
        os_log(log_handle, "Could not start idletimer thread: %s", strerror(err));

    return err;
}

/* 
 Reads the user's program arguments for mDNSResponderHelper
 For now we have only one option: mDNSHelperDebugLogging which is used to turn on mDNSResponderHelperLogging
 
 To turn ON mDNSResponderHelper Verbose Logging,
 1] sudo defaults write /Library/Preferences/com.apple.mDNSResponderHelper.plist mDNSHelperDebugLogging -bool YES
 2] sudo reboot
 
 To turn OFF mDNSResponderHelper Logging,
 1] sudo defaults delete /Library/Preferences/com.apple.mDNSResponderHelper.plist

 To view the current options set,
 1] plutil -p /Library/Preferences/com.apple.mDNSResponderHelper.plist
 OR
 1] sudo defaults read /Library/Preferences/com.apple.mDNSResponderHelper.plist
*/

static mDNSBool HelperPrefsGetValueBool(CFStringRef key, mDNSBool defaultVal)
{
    CFBooleanRef boolean;
    mDNSBool result = defaultVal;
    
    boolean = CFPreferencesCopyAppValue(key, kmDNSHelperProgramArgs);
    if (boolean != NULL)
    {
        if (CFGetTypeID(boolean) == CFBooleanGetTypeID())
            result = CFBooleanGetValue(boolean) ? mDNStrue : mDNSfalse;
        CFRelease(boolean);
    }
    
    return result;
}


// Verify Client's Entitlement
static mDNSBool check_entitlement(xpc_connection_t conn, const char *password)
{
    mDNSBool entitled = mDNSfalse;
    xpc_object_t ent = xpc_connection_copy_entitlement_value(conn, password);
    
    if (ent)
    {
        if (xpc_get_type(ent) == XPC_TYPE_BOOL && xpc_bool_get_value(ent))
        {
            entitled = mDNStrue;
        }
        xpc_release(ent);
    }
    else
    {
        os_log(log_handle, "client entitlement is NULL");
    }
    
    if (!entitled)
        os_log(log_handle, "entitlement check failed -> client is missing entitlement!");
    
    return entitled;
}


static void handle_request(xpc_object_t req)
{
    mDNSu32 helper_mode = 0;
    int error_code = 0;
    
    xpc_connection_t remote_conn = xpc_dictionary_get_remote_connection(req);
    xpc_object_t response = xpc_dictionary_create_reply(req);
    
    // switch here based on dictionary to handle different requests from mDNSResponder
    if ((xpc_dictionary_get_uint64(req, kHelperMode)))
    {
        os_log_info(log_handle, "Getting mDNSResponder request mode");
        helper_mode = (mDNSu32)(xpc_dictionary_get_uint64(req, kHelperMode));
    }
   
    switch (helper_mode)
    {
        case bpf_request:
        {
            os_log_info(log_handle, "Calling new RequestBPF()");
            RequestBPF();
            break;
        }
            
        case set_name:
        {
            const char *old_name;
            const char *new_name;
            int pref_key = 0;
            
            pref_key = (int)(xpc_dictionary_get_uint64(req, kPrefsNameKey));
            old_name = xpc_dictionary_get_string(req, kPrefsOldName);
            new_name = xpc_dictionary_get_string(req, kPrefsNewName);
            
            os_log_info(log_handle, "Calling new SetName() oldname: %s newname: %s key:%d", old_name, new_name, pref_key);
            PreferencesSetName(pref_key, old_name, new_name);
            break;
        }
            
        case p2p_packetfilter:
        {
            size_t count = 0;
            pfArray_t pfports;
            pfArray_t pfprotocols;
            const char *if_name;
            uint32_t cmd;
            xpc_object_t xpc_obj_port_array;
            size_t port_array_count = 0;
            xpc_object_t xpc_obj_protocol_array;
            size_t protocol_array_count = 0;
            
            cmd = xpc_dictionary_get_uint64(req, "pf_opcode");
            if_name = xpc_dictionary_get_string(req, "pf_ifname");
            xpc_obj_port_array = xpc_dictionary_get_value(req, "xpc_obj_array_port");
            if ((void *)xpc_obj_port_array != NULL)
                port_array_count = xpc_array_get_count(xpc_obj_port_array);
            xpc_obj_protocol_array = xpc_dictionary_get_value(req, "xpc_obj_array_protocol");
            if ((void *)xpc_obj_protocol_array != NULL)
                protocol_array_count = xpc_array_get_count(xpc_obj_protocol_array);
            if (port_array_count != protocol_array_count)
                break;
            if (port_array_count > PFPortArraySize)
                break;
            count = port_array_count;

            for (size_t i = 0; i < count; i++) {
                pfports[i] = (uint16_t)xpc_array_get_uint64(xpc_obj_port_array, i);
                pfprotocols[i] = (uint16_t)xpc_array_get_uint64(xpc_obj_protocol_array, i);
            }

            os_log_info(log_handle,"Calling new PacketFilterControl()");
            PacketFilterControl(cmd, if_name, count, pfports, pfprotocols);
            break;
        }
            
        case user_notify:
        {
            const char *title;
            const char *msg;
            title = xpc_dictionary_get_string(req, "notify_title");
            msg   = xpc_dictionary_get_string(req, "notify_msg");
            
            os_log_info(log_handle,"Calling new UserNotify() title:%s msg:%s", title, msg);
            UserNotify(title, msg);
            break;
        }
            
        case power_req:
        {
            int key, interval;
            key        = xpc_dictionary_get_uint64(req, "powerreq_key");
            interval   = xpc_dictionary_get_uint64(req, "powerreq_interval");
            
            os_log_info(log_handle,"Calling new PowerRequest() key[%d] interval[%d]", key, interval);
            PowerRequest(key, interval, &error_code);
            break;
        }
            
        case send_wakepkt:
        {
            const char *ether_addr;
            const char *ip_addr;
            int iteration;
            unsigned int if_id;
            
            if_id = (unsigned int)xpc_dictionary_get_uint64(req, "interface_index");
            ether_addr = xpc_dictionary_get_string(req, "ethernet_address");
            ip_addr = xpc_dictionary_get_string(req, "ip_address");
            iteration = (int)xpc_dictionary_get_uint64(req, "swp_iteration");
            
            os_log_info(log_handle, "Calling new SendWakeupPacket() ether_addr[%s] ip_addr[%s] if_id[%d] iteration[%d]",
                           ether_addr, ip_addr, if_id, iteration);
            SendWakeupPacket(if_id, ether_addr, ip_addr, iteration);
            break;
        }
            
        case set_localaddr_cacheentry:
        {
            int if_index, family;
            size_t ip_len, eth_len;

            if_index = xpc_dictionary_get_uint64(req, "slace_ifindex");
            family   = xpc_dictionary_get_uint64(req, "slace_family");

            const uint8_t * const ip = (const uint8_t *)xpc_dictionary_get_data(req, "slace_ip", &ip_len);
            if (ip_len != sizeof(v6addr_t))
            {
                error_code = kHelperErr_ParamErr;
                break;
            }

            const uint8_t * const eth = (const uint8_t *)xpc_dictionary_get_data(req, "slace_eth", &eth_len);
            if (eth_len != sizeof(ethaddr_t))
            {
                error_code = kHelperErr_ParamErr;
                break;
            }

            os_log_info(log_handle, "Calling new SetLocalAddressCacheEntry() if_index[%d] family[%d] ", if_index, family);

            SetLocalAddressCacheEntry(if_index, family, ip, eth, &error_code);
            break;
        }
            
        case send_keepalive:
        {
            uint16_t lport, rport, win;
            uint32_t seq, ack;
            size_t sadd6_len, dadd6_len;
            
            lport = xpc_dictionary_get_uint64(req, "send_keepalive_lport");
            rport = xpc_dictionary_get_uint64(req, "send_keepalive_rport");
            seq   = xpc_dictionary_get_uint64(req, "send_keepalive_seq");
            ack   = xpc_dictionary_get_uint64(req, "send_keepalive_ack");
            win   = xpc_dictionary_get_uint64(req, "send_keepalive_win");
            
            const uint8_t * const sadd6 = (const uint8_t *)xpc_dictionary_get_data(req, "send_keepalive_sadd", &sadd6_len);
            const uint8_t * const dadd6 = (const uint8_t *)xpc_dictionary_get_data(req, "send_keepalive_dadd", &dadd6_len);
            if ((sadd6_len != sizeof(v6addr_t)) || (dadd6_len != sizeof(v6addr_t)))
            {
                error_code = kHelperErr_ParamErr;
                break;
            }

            os_log_info(log_handle, "helper-main: handle_request: send_keepalive: lport is[%d] rport is[%d] seq is[%d] ack is[%d] win is[%d]",
                           lport, rport, seq, ack, win);
            
            SendKeepalive(sadd6, dadd6, lport, rport, seq, ack, win);
            break;
        }
    
        case retreive_tcpinfo:
        {
            uint16_t lport, rport;
            int family;
            uint32_t seq, ack;
            uint16_t win;
            int32_t  intfid;
            size_t laddr_len, raddr_len;
            
            lport    = xpc_dictionary_get_uint64(req, "retreive_tcpinfo_lport");
            rport    = xpc_dictionary_get_uint64(req, "retreive_tcpinfo_rport");
            family   = xpc_dictionary_get_uint64(req, "retreive_tcpinfo_family");

            const uint8_t * const laddr = (const uint8_t *)xpc_dictionary_get_data(req, "retreive_tcpinfo_laddr", &laddr_len);
            const uint8_t * const raddr = (const uint8_t *)xpc_dictionary_get_data(req, "retreive_tcpinfo_raddr", &raddr_len);
            if ((laddr_len != sizeof(v6addr_t)) || (raddr_len != sizeof(v6addr_t)))
            {
                error_code = kHelperErr_ParamErr;
                break;
            }

            os_log_info(log_handle, "helper-main: handle_request: retreive_tcpinfo: lport is[%d] rport is[%d] family is [%d]",
                           lport, rport, family);
            
            RetrieveTCPInfo(family, laddr, lport, raddr, rport, &seq, &ack, &win, &intfid, &error_code);
            
            if (response)
            {
                xpc_dictionary_set_uint64(response, "retreive_tcpinfo_seq",  seq);
                xpc_dictionary_set_uint64(response, "retreive_tcpinfo_ack",  ack);
                xpc_dictionary_set_uint64(response, "retreive_tcpinfo_win",  win);
                xpc_dictionary_set_uint64(response, "retreive_tcpinfo_ifid", intfid);
            }
            
            os_log_info(log_handle, "helper-main: handle_request: retreive_tcpinfo: seq is[%d] ack is[%d] win is [%d] intfid is [%d]",
                           seq, ack, win, intfid);

            break;
        }
            
        case keychain_getsecrets:
        {
            unsigned int num_sec  = 0;
            unsigned long secrets = 0;
            unsigned int sec_cnt  = 0;
            
            os_log_info(log_handle,"Calling new KeyChainGetSecrets()");
            
            KeychainGetSecrets(&num_sec, &secrets, &sec_cnt, &error_code);
            
            if (response)
            {
                xpc_dictionary_set_uint64(response, "keychain_num_secrets", num_sec);
                xpc_dictionary_set_data(response, "keychain_secrets", (void *)secrets, sec_cnt);
            }
            
            os_log_info(log_handle,"helper-main: handle_request: keychain_getsecrets: num_secrets is %u, secrets is %lu, secrets_Cnt is %u",
                        num_sec, secrets, sec_cnt);
            
            if (secrets)
                vm_deallocate(mach_task_self(), secrets, sec_cnt);
            
            break;
        }
            
        default:
        {
            os_log(log_handle, "handle_request: Unrecognized mode!");
            error_code  = kHelperErr_UndefinedMode;
            break;
        }
    }
    
    // Return Response Status back to the client (essentially ACKing the request)
    if (response)
    {
        xpc_dictionary_set_uint64(response, kHelperReplyStatus, kHelperReply_ACK);
        xpc_dictionary_set_int64(response, kHelperErrCode, error_code);
        xpc_connection_send_message(remote_conn, response);
        xpc_release(response);
    }
    else
    {
        os_log(log_handle, "handle_requests: Response Dictionary could not be created!");
        return;
    }
    
}

static void accept_client(xpc_connection_t conn)
{
    int c_pid = xpc_connection_get_pid(conn);
    
    if (!(check_entitlement(conn, kHelperService)))
    {
        os_log(log_handle, "accept_client: Helper Client PID[%d] is missing Entitlement. Cancelling connection", c_pid);
        xpc_connection_cancel(conn);
        return;
    }
    
    xpc_retain(conn);
    xpc_connection_set_target_queue(conn, xpc_queue);
    xpc_connection_set_event_handler(conn, ^(xpc_object_t req_msg)
    {
        xpc_type_t type = xpc_get_type(req_msg);
                                         
        if (type == XPC_TYPE_DICTIONARY)
        {
            os_log_info(log_handle,"accept_client:conn:[%p] client[%d](mDNSResponder) requesting service", (void *) conn, c_pid);
            handle_request(req_msg);
        }
        else // We hit this case ONLY if Client Terminated Connection OR Crashed
        {
            os_log(log_handle, "accept_client:conn:[%p] client[%d](mDNSResponder) teared down the connection (OR Crashed)", (void *) conn, c_pid);
            // handle_termination();
            xpc_release(conn);
        }
    });
    
    xpc_connection_resume(conn);
}


static void init_helper_service(const char *service_name)
{
    
    xpc_connection_t xpc_listener = xpc_connection_create_mach_service(service_name, NULL, XPC_CONNECTION_MACH_SERVICE_LISTENER);
    if (!xpc_listener || xpc_get_type(xpc_listener) != XPC_TYPE_CONNECTION)
    {
        os_log(log_handle, "init_helper_service: Error Creating XPC Listener for mDNSResponderHelperService !!");
        return;
    }
    
    os_log_info(log_handle,"init_helper_service: XPC Listener for mDNSResponderHelperService Listening");
    
    xpc_queue = dispatch_queue_create("com.apple.mDNSHelper.service_queue", NULL);
    
    xpc_connection_set_event_handler(xpc_listener, ^(xpc_object_t eventmsg)
    {
        xpc_type_t type = xpc_get_type(eventmsg);
                                         
        if (type == XPC_TYPE_CONNECTION)
        {
            os_log_info(log_handle,"init_helper_service: new mDNSResponderHelper Client %p", eventmsg);
            accept_client(eventmsg);
        }
        else if (type == XPC_TYPE_ERROR) // Ideally, we would never hit these cases below
        {
            os_log(log_handle, "init_helper_service: XPCError: %s", xpc_dictionary_get_string(eventmsg, XPC_ERROR_KEY_DESCRIPTION));
            return;
        }
        else
        {
            os_log(log_handle, "init_helper_service: Unknown EventMsg type");
            return;
        }
    });
    
    xpc_connection_resume(xpc_listener);
}


int main(int ac, char *av[])
{
    char *p = NULL;
    long n;
    int ch;

    while ((ch = getopt(ac, av, "dt:")) != -1)
    {
        switch (ch)
        {
            case 'd':
                opt_debug = 1;
                break;
            case 't':
                n = strtol(optarg, &p, 0);
                if ('\0' == optarg[0] || '\0' != *p || n > LONG_MAX || n < 0)
                {
                    fprintf(stderr, "Invalid idle timeout: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }
                maxidle = n;
                break;
            case '?':
            default:
                fprintf(stderr, "Usage: mDNSResponderHelper [-d] [-t maxidle]\n");
                exit(EXIT_FAILURE);
        }
    }
    ac -= optind;
    av += optind;
    (void)ac; // Unused
    (void)av; // Unused

    initialize_logging();
    initialize_id();

    mDNSHelperLogEnabled = HelperPrefsGetValueBool(kPreferencesKey_mDNSHelperLog, mDNSHelperLogEnabled);

// Currently on Fuji/Whitetail releases we are keeping the logging always enabled.
// Hence mDNSHelperLogEnabled is set to true below by default.
    mDNSHelperLogEnabled      = 1;

    os_log_info(log_handle,"mDNSResponderHelper Starting to run");

#ifndef NO_SECURITYFRAMEWORK
    // We should normally be running as a system daemon.  However, that might not be the case in some scenarios (e.g. debugging).
    // Explicitly ensure that our Keychain operations utilize the system domain.
    if (opt_debug)
        SecKeychainSetPreferenceDomain(kSecPreferencesDomainSystem);
#endif

    if (maxidle)
        actualidle = maxidle;

    signal(SIGTERM, handle_sigterm);

    if (initialize_timer())
        exit(EXIT_FAILURE);
    for (n=0; n<100000; n++)
        if (!gRunLoop)
            usleep(100);
    
    if (!gRunLoop)
    {
        os_log(log_handle, "gRunLoop not set after waiting");
        exit(EXIT_FAILURE);
    }

    init_helper_service(kHelperService);
    os_log_info(log_handle,"mDNSResponderHelper is now running");
    dispatch_main();
    
}

// Note: The C preprocessor stringify operator ('#') makes a string from its argument, without macro expansion
// e.g. If "version" is #define'd to be "4", then STRINGIFY_AWE(version) will return the string "version", not "4"
// To expand "version" to its value before making the string, use STRINGIFY(version) instead
#define STRINGIFY_ARGUMENT_WITHOUT_EXPANSION(s) # s
#define STRINGIFY(s) STRINGIFY_ARGUMENT_WITHOUT_EXPANSION(s)

// For convenience when using the "strings" command, this is the last thing in the file
// The "@(#) " pattern is a special prefix the "what" command looks for
const char VersionString_SCCS[] = "@(#) mDNSResponderHelper " STRINGIFY(mDNSResponderVersion) " (" __DATE__ " " __TIME__ ")";

#if _BUILDING_XCODE_PROJECT_
// If the process crashes, then this string will be magically included in the automatically-generated crash log
const char *__crashreporter_info__ = VersionString_SCCS + 5;
asm (".desc ___crashreporter_info__, 0x10");
#endif
