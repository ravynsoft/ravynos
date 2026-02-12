//
//  xpc_client_log_utility.h
//  mDNSResponder
//
//  Copyright (c) 2019 Apple Inc. All rights reserved.
//

#ifndef XPC_CLIENT_LOG_UTILITY_H
#define XPC_CLIENT_LOG_UTILITY_H

#define kDNSLogUtilityService   "com.apple.mDNSResponder.log_utility"

typedef enum
{
    kDNSMsg_NoError = 0,
    kDNSMsg_Busy,
    kDNSMsg_UnknownRequest,
    kDNSMsg_Error
} DaemonReplyStatusCodes;

#define kDNSErrorDescription    "ErrorDescription"

#define kDNSLogLevel            "DNSLoggingVerbosity"
typedef enum
{
    log_level1 = 1, // logging off
    log_level2,     // logging USR1
    log_level3,     // logging USR2
    log_level4,     // logging USR1/2
} DNSLogLevels;

#define kDNSStateInfo           "DNSStateInfoLevels"
typedef enum
{
    full_state = 1,                     // Dump state to a plain text file
    full_state_with_compression = 2,    // Dump state to a compressed file
    full_state_to_stdout = 3,           // Dump state to STDOUT
} DNSStateInfo;

#define kmDNSResponderTests     "mDNSResponderTests"
typedef enum
{
    test_helper_ipc = 1,   // invokes mDNSResponder to send a test msg to mDNSResponderHelper
    test_mDNS_log,         // invokes mDNSResponder to log using different internal macros
} mDNSTestModes;

#define kDNSStateDump           "mDNSResponderStateDump"
#define kDNSDumpFilePath        "mDNSResponderDumpFilePath"
#define kDNSStateDumpTimeUsed   "mDNSResponderDumpTimeUsed"
#define kDNSStateDumpFD         "mDNSResponderDumpFD"

#endif /* XPC_CLIENT_LOG_UTILITY_H */
