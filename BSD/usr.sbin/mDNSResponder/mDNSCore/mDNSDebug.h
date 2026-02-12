/*
 * Copyright (c) 2002-2019 Apple Inc. All rights reserved.
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

#ifndef __mDNSDebug_h
#define __mDNSDebug_h

#include "mDNSFeatures.h"

#if MDNSRESPONDER_SUPPORTS(APPLE, OS_LOG)
#include <os/log.h>
#endif

// Set MDNS_DEBUGMSGS to 0 to optimize debugf() calls out of the compiled code
// Set MDNS_DEBUGMSGS to 1 to generate normal debugging messages
// Set MDNS_DEBUGMSGS to 2 to generate verbose debugging messages
// MDNS_DEBUGMSGS is normally set in the project options (or makefile) but can also be set here if desired
// (If you edit the file here to turn on MDNS_DEBUGMSGS while you're debugging some code, be careful
// not to accidentally check-in that change by mistake when you check in your other changes.)

//#undef MDNS_DEBUGMSGS
//#define MDNS_DEBUGMSGS 2

// Set MDNS_CHECK_PRINTF_STYLE_FUNCTIONS to 1 to enable extra GCC compiler warnings
// Note: You don't normally want to do this, because it generates a bunch of
// spurious warnings for the following custom extensions implemented by mDNS_vsnprintf:
//    warning: `#' flag used with `%s' printf format    (for %#s              -- pascal string format)
//    warning: repeated `#' flag in format              (for %##s             -- DNS name string format)
//    warning: double format, pointer arg (arg 2)       (for %.4a, %.16a, %#a -- IP address formats)
#define MDNS_CHECK_PRINTF_STYLE_FUNCTIONS 0

#if MDNSRESPONDER_SUPPORTS(APPLE, OS_LOG)
typedef os_log_t mDNSLogCategory_t;

typedef os_log_type_t mDNSLogLevel_t;
#define MDNS_LOG_FAULT      OS_LOG_TYPE_FAULT
#define MDNS_LOG_ERROR      OS_LOG_TYPE_ERROR
#define MDNS_LOG_WARNING    OS_LOG_TYPE_DEFAULT
#define MDNS_LOG_DEFAULT    OS_LOG_TYPE_DEFAULT
#define MDNS_LOG_INFO       OS_LOG_TYPE_DEFAULT
#define MDNS_LOG_DEBUG      OS_LOG_TYPE_DEBUG
#else
typedef const char * mDNSLogCategory_t;
typedef enum
{
    MDNS_LOG_FAULT   = 1,
    MDNS_LOG_ERROR   = 2,
    MDNS_LOG_WARNING = 3,
    MDNS_LOG_DEFAULT = 4,
    MDNS_LOG_INFO    = 5,
    MDNS_LOG_DEBUG   = 6
} mDNSLogLevel_t;
#endif

#if MDNSRESPONDER_SUPPORTS(APPLE, OS_LOG)
    extern os_log_t mDNSLogCategory_Default;
    extern os_log_t mDNSLogCategory_mDNS;
    extern os_log_t mDNSLogCategory_uDNS;
    extern os_log_t mDNSLogCategory_SPS;
    extern os_log_t mDNSLogCategory_XPC;

    #define MDNS_LOG_CATEGORY_DEFINITION(NAME)  mDNSLogCategory_ ## NAME
#else
    #define MDNS_LOG_CATEGORY_DEFINITION(NAME)  # NAME
#endif

#define MDNS_LOG_CATEGORY_DEFAULT   MDNS_LOG_CATEGORY_DEFINITION(Default)
#define MDNS_LOG_CATEGORY_MDNS      MDNS_LOG_CATEGORY_DEFINITION(mDNS)
#define MDNS_LOG_CATEGORY_UDNS      MDNS_LOG_CATEGORY_DEFINITION(uDNS)
#define MDNS_LOG_CATEGORY_SPS       MDNS_LOG_CATEGORY_DEFINITION(SPS)
#define MDNS_LOG_CATEGORY_XPC       MDNS_LOG_CATEGORY_DEFINITION(XPC)

// Set this symbol to 1 to answer remote queries for our Address, and reverse mapping PTR
#define ANSWER_REMOTE_HOSTNAME_QUERIES 0

// Set this symbol to 1 to do extra debug checks on malloc() and free()
// Set this symbol to 2 to write a log message for every malloc() and free()
// #define MDNS_MALLOC_DEBUGGING 1

#if (MDNS_MALLOC_DEBUGGING > 0) && defined(WIN32)
#error "Malloc debugging does not yet work on Windows"
#endif

//#define ForceAlerts 1
//#define LogTimeStamps 1

// Developer-settings section ends here

#if MDNS_CHECK_PRINTF_STYLE_FUNCTIONS
#define IS_A_PRINTF_STYLE_FUNCTION(F,A) __attribute__ ((format(printf,F,A)))
#else
#define IS_A_PRINTF_STYLE_FUNCTION(F,A)
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Variable argument macro support. Use ANSI C99 __VA_ARGS__ where possible. Otherwise, use the next best thing.

#if (defined(__GNUC__))
    #if ((__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 2)))
        #define MDNS_C99_VA_ARGS        1
        #define MDNS_GNU_VA_ARGS        0
    #else
        #define MDNS_C99_VA_ARGS        0
        #define MDNS_GNU_VA_ARGS        1
    #endif
    #define MDNS_HAS_VA_ARG_MACROS      1
#elif (_MSC_VER >= 1400) // Visual Studio 2005 and later
    #define MDNS_C99_VA_ARGS            1
    #define MDNS_GNU_VA_ARGS            0
    #define MDNS_HAS_VA_ARG_MACROS      1
#elif (defined(__MWERKS__))
    #define MDNS_C99_VA_ARGS            1
    #define MDNS_GNU_VA_ARGS            0
    #define MDNS_HAS_VA_ARG_MACROS      1
#else
    #define MDNS_C99_VA_ARGS            0
    #define MDNS_GNU_VA_ARGS            0
    #define MDNS_HAS_VA_ARG_MACROS      0
#endif

#if (MDNS_HAS_VA_ARG_MACROS)
    #if (MDNS_C99_VA_ARGS)
        #define MDNS_LOG_DEFINITION(LEVEL, ...) \
            do { if (mDNS_LoggingEnabled) LogMsgWithLevel(MDNS_LOG_CATEGORY_DEFAULT, LEVEL, __VA_ARGS__); } while (0)

        #define debug_noop(...)   do {} while(0)
        #define LogMsg(...)       LogMsgWithLevel(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_DEFAULT, __VA_ARGS__)
        #define LogOperation(...) MDNS_LOG_DEFINITION(MDNS_LOG_INFO,  __VA_ARGS__)
        #define LogSPS(...)       MDNS_LOG_DEFINITION(MDNS_LOG_INFO,  __VA_ARGS__)
        #define LogInfo(...)      MDNS_LOG_DEFINITION(MDNS_LOG_INFO,  __VA_ARGS__)
        #define LogDebug(...)     MDNS_LOG_DEFINITION(MDNS_LOG_DEBUG, __VA_ARGS__)
    #elif (MDNS_GNU_VA_ARGS)
        #define MDNS_LOG_DEFINITION(LEVEL, ARGS...) \
            do { if (mDNS_LoggingEnabled) LogMsgWithLevel(MDNS_LOG_CATEGORY_DEFAULT, LEVEL, ARGS); } while (0)

        #define debug_noop(ARGS...)   do {} while (0)
        #define LogMsg(ARGS... )      LogMsgWithLevel(MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_DEFAULT, ARGS)
        #define LogOperation(ARGS...) MDNS_LOG_DEFINITION(MDNS_LOG_INFO,  ARGS)
        #define LogSPS(ARGS...)       MDNS_LOG_DEFINITION(MDNS_LOG_INFO,  ARGS)
        #define LogInfo(ARGS...)      MDNS_LOG_DEFINITION(MDNS_LOG_INFO,  ARGS)
        #define LogDebug(ARGS...)     MDNS_LOG_DEFINITION(MDNS_LOG_DEBUG, ARGS)
    #else
        #error "Unknown variadic macros"
    #endif
#else
// If your platform does not support variadic macros, you need to define the following variadic functions.
// See mDNSShared/mDNSDebug.c for sample implementation
    #define debug_noop 1 ? (void)0 : (void)
    #define LogMsg LogMsg_
    #define LogOperation (mDNS_LoggingEnabled == 0) ? ((void)0) : LogOperation_
    #define LogSPS       (mDNS_LoggingEnabled == 0) ? ((void)0) : LogSPS_
    #define LogInfo      (mDNS_LoggingEnabled == 0) ? ((void)0) : LogInfo_
    #define LogDebug     (mDNS_LoggingEnabled == 0) ? ((void)0) : LogDebug_
extern void LogMsg_(const char *format, ...)       IS_A_PRINTF_STYLE_FUNCTION(1,2);
extern void LogOperation_(const char *format, ...) IS_A_PRINTF_STYLE_FUNCTION(1,2);
extern void LogSPS_(const char *format, ...)       IS_A_PRINTF_STYLE_FUNCTION(1,2);
extern void LogInfo_(const char *format, ...)      IS_A_PRINTF_STYLE_FUNCTION(1,2);
extern void LogDebug_(const char *format, ...)     IS_A_PRINTF_STYLE_FUNCTION(1,2);
#endif


#if MDNS_DEBUGMSGS
#define debugf debugf_
extern void debugf_(const char *format, ...) IS_A_PRINTF_STYLE_FUNCTION(1,2);
#else
#define debugf debug_noop
#endif

#if MDNS_DEBUGMSGS > 1
#define verbosedebugf verbosedebugf_
extern void verbosedebugf_(const char *format, ...) IS_A_PRINTF_STYLE_FUNCTION(1,2);
#else
#define verbosedebugf debug_noop
#endif

extern int mDNS_LoggingEnabled;
extern int mDNS_PacketLoggingEnabled;
extern int mDNS_McastLoggingEnabled;
extern int mDNS_McastTracingEnabled;
extern int mDNS_DebugMode;          // If non-zero, LogMsg() writes to stderr instead of syslog
extern const char ProgramName[];

extern void LogMsgWithLevel(mDNSLogCategory_t category, mDNSLogLevel_t level, const char *format, ...) IS_A_PRINTF_STYLE_FUNCTION(3,4);
// LogMsgNoIdent needs to be fixed so that it logs without the ident prefix like it used to
// (or completely overhauled to use the new "log to a separate file" facility)
#define LogMsgNoIdent LogMsg

#if APPLE_OSX_mDNSResponder
extern void LogFatalError(const char *format, ...);
#else
#define LogFatalError LogMsg
#endif

#if MDNS_MALLOC_DEBUGGING >= 1
extern void *mallocL(const char *msg, mDNSu32 size);
extern void *callocL(const char *msg, mDNSu32 size);
extern void freeL(const char *msg, void *x);
#if APPLE_OSX_mDNSResponder
extern void LogMemCorruption(const char *format, ...);
#else
#define LogMemCorruption LogMsg
#endif
#else
#define mallocL(MSG, SIZE) malloc(SIZE)
#define callocL(MSG, SIZE) calloc(1, SIZE)
#define freeL(MSG, PTR) free(PTR)
#endif

#ifdef __cplusplus
}
#endif

#if MDNSRESPONDER_SUPPORTS(APPLE, OS_LOG)
/** @brief Write a log message to system's log storage(memory or disk).
 *
 *  On Apple platform, os_log() will be called to log a message.
 *
 *  @param CATEGORY         A custom log object previously created by the os_log_create function, and such an object is
 *                          used to specify "subsystem" and "category". For mDNSResponder, the subsystem should always
 *                          be set to "com.apple.mDNSResponder"; and the category is used for categorization and
 *                          filtering of related log messages within the subsystem’s settings. We have 4 categories that
 *                          are pre-defined: MDNS_LOG_CATEGORY_DEFAULT, MDNS_LOG_CATEGORY_MDNS, MDNS_LOG_CATEGORY_UDNS,
 *                          MDNS_LOG_CATEGORY_SPS. If these categories are not enough, use os_log_create to create more.
 *
 *  @param LEVEL            The log level that determines the importance of the message. The levels are, in order of
 *                          decreasing importance:
 *                              MDNS_LOG_FAULT      Fault-level messages are intended for capturing system-level errors
 *                                                  that are critical to the system. They are always saved in the data store.
 *                              MDNS_LOG_ERROR      Error-level messages are intended for reporting process-level errors
 *                                                  that are unexpected and incorrect during the normal operation. They
 *                                                  are always saved in the data store.
 *                              MDNS_LOG_WARNING    Warning-level messages are intended for capturing unexpected and
 *                                                  possible incorrect behavior that might be used later to root cause
 *                                                  an error or fault. They are are initially stored in memory buffers
 *                                                  and then moved to a data store.
 *                              MDNS_LOG_DEFAULT    Default-level messages are intended for reporting things that might
 *                                                  result a failure. They are are initially stored in memory buffers
 *                                                  and then moved to a data store.
 *                              MDNS_LOG_INFO       Info-level messages are intended for capturing information that may
 *                                                  be helpful, but isn’t essential, for troubleshooting errors. They
 *                                                  are initially stored in memory buffers, but will only be moved into
 *                                                  data store when faults and, optionally, errors occur.
 *                              MDNS_LOG_DEBUG      Debug-level messages are intended for information that may be useful
 *                                                  during development or while troubleshooting a specific problem, Debug
 *                                                  logging should not be used in shipping software. They are only
 *                                                  captured in memory when debug logging is enabled through a
 *                                                  configuration change.
 *
 *  @param FORMAT           A constant string or format string that produces a human-readable log message. The format
 *                          string follows the IEEE printf specification, besides the following customized format specifiers:
 *                              %{mdnsresponder:domain_name}.*P     the pointer to a DNS lable sequence
 *                              %{mdnsresponder:ip_addr}.20P        the pointer to a mDNSAddr variable
 *                              %{network:in_addr}.4P               the pointer to a mDNSv4Addr variable
 *                              %{network:in6_addr}.16P             the pointer to a mDNSv6Addr variable
 *                              %{mdnsresponder:mac_addr}.6P        the pointer to a 6-byte-length MAC address
 *
 *  @param ...              The parameter list that will be formated by the format string. Note that if the customized
 *                          format specifiers are used and the data length is not specified in the format string, the
 *                          size should be listed before the pointer to the data, for example:
 *                              "%{mdnsresponder:domain_name}.*P", (name ? (int)DomainNameLength((const domainname *)name) : 0), <the pointer to a DNS label sequence>
 *
 */
    #define LogRedact(CATEGORY, LEVEL, FORMAT, ...) os_log_with_type(CATEGORY, LEVEL, FORMAT, ## __VA_ARGS__)
#else
    #if (MDNS_HAS_VA_ARG_MACROS)
        #if (MDNS_C99_VA_ARGS)
            #define LogRedact(CATEGORY, LEVEL, ...) \
                do { if (mDNS_LoggingEnabled) LogMsgWithLevel(CATEGORY, LEVEL, __VA_ARGS__); } while (0)
        #elif (MDNS_GNU_VA_ARGS)
            #define LogRedact(CATEGORY, LEVEL, ARGS...) \
                do { if (mDNS_LoggingEnabled) LogMsgWithLevel(CATEGORY, LEVEL, ARGS); } while (0)
        #else
            #error "Unknown variadic macros"
        #endif
    #else
        #define LogRedact      (mDNS_LoggingEnabled == 0) ? ((void)0) : LogRedact_
        extern void LogRedact_(const char *format, ...) IS_A_PRINTF_STYLE_FUNCTION(1,2);
    #endif
#endif // MDNSRESPONDER_SUPPORTS(APPLE, OS_LOG)

// The followings are the customized log specifier defined in os_log. For compatibility, we have to define it when it is
// not on the Apple platform, for example, the Posix platform. The keyword "public" or "private" is used to control whether
// the content would be redacted when the redaction is turned on: "public" means the content will always be printed;
// "private" means the content will be printed as <private> if the redaction is turned on, only when the redaction is
// turned off, the content will be printed as what it should be.

#if MDNSRESPONDER_SUPPORTS(APPLE, OS_LOG)
    #define PUB_S "%{public}s"
    #define PRI_S "%{private}s"
#else
    #define PUB_S "%s"
    #define PRI_S PUB_S
#endif

#if MDNSRESPONDER_SUPPORTS(APPLE, OS_LOG)
    #define PUB_DM_NAME "%{public, mdnsresponder:domain_name}.*P"
    #define PRI_DM_NAME "%{private, mdnsresponder:domain_name}.*P"
    // When DM_NAME_PARAM is used, the file where the function is defined must include DNSEmbeddedAPI.h
    #define DM_NAME_PARAM(name) ((name) ? ((int)DomainNameLength((const domainname *)(name))) : 0), (name)
#else
    #define PUB_DM_NAME "%##s"
    #define PRI_DM_NAME PUB_DM_NAME
    #define DM_NAME_PARAM(name) (name)
#endif

#if MDNSRESPONDER_SUPPORTS(APPLE, OS_LOG)
    #define PUB_IP_ADDR "%{public, mdnsresponder:ip_addr}.20P"
    #define PRI_IP_ADDR "%{private, mdnsresponder:ip_addr}.20P"

    #define PUB_IPv4_ADDR "%{public, network:in_addr}.4P"
    #define PRI_IPv4_ADDR "%{private, network:in_addr}.4P"

    #define PUB_IPv6_ADDR "%{public, network:in6_addr}.16P"
    #define PRI_IPv6_ADDR "%{private, network:in6_addr}.16P"
#else
    #define PUB_IP_ADDR "%#a"
    #define PRI_IP_ADDR PUB_IP_ADDR

    #define PUB_IPv4_ADDR "%.4a"
    #define PRI_IPv4_ADDR PUB_IPv4_ADDR

    #define PUB_IPv6_ADDR "%.16a"
    #define PRI_IPv6_ADDR PUB_IPv6_ADDR
#endif

#if MDNSRESPONDER_SUPPORTS(APPLE, OS_LOG)
    #define PUB_MAC_ADDR "%{public, mdnsresponder:mac_addr}.6P"
    #define PRI_MAC_ADDR "%{private, mdnsresponder:mac_addr}.6P"
#else
    #define PUB_MAC_ADDR "%.6a"
    #define PRI_MAC_ADDR PUB_MAC_ADDR
#endif

extern void LogToFD(int fd, const char *format, ...);

#endif // __mDNSDebug_h
