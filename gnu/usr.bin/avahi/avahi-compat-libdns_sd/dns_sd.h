/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2003-2004, Apple Computer, Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1.  Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of its
 *     contributors may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _DNS_SD_H
#define _DNS_SD_H

#ifdef  __cplusplus
    extern "C" {
#endif

/* standard calling convention under Win32 is __stdcall */
/* Note: When compiling Intel EFI (Extensible Firmware Interface) under MS Visual Studio, the */
/* _WIN32 symbol is defined by the compiler even though it's NOT compiling code for Windows32 */
#if defined(_WIN32) && !defined(EFI32) && !defined(EFI64)
#define DNSSD_API __stdcall
#else
#define DNSSD_API
#endif

/* stdint.h does not exist on FreeBSD 4.x; its types are defined in sys/types.h instead */
#if defined(__FreeBSD__) && (__FreeBSD__ < 5)
#include <sys/types.h>

/* Likewise, on Sun, standard integer types are in sys/types.h */
#elif defined(__sun__)
#include <sys/types.h>

/* EFI does not have stdint.h, or anything else equivalent */
#elif defined(EFI32) || defined(EFI64)
typedef UINT8       uint8_t;
typedef INT8        int8_t;
typedef UINT16      uint16_t;
typedef INT16       int16_t;
typedef UINT32      uint32_t;
typedef INT32       int32_t;

/* Windows has its own differences */
#elif defined(_WIN32)
#include <windows.h>
#define _UNUSED
#define bzero(a, b) memset(a, 0, b)
#ifndef _MSL_STDINT_H
typedef UINT8       uint8_t;
typedef INT8        int8_t;
typedef UINT16      uint16_t;
typedef INT16       int16_t;
typedef UINT32      uint32_t;
typedef INT32       int32_t;
#endif

/* All other Posix platforms use stdint.h */
#else
#include <stdint.h>
#endif

/* DNSServiceRef, DNSRecordRef
 *
 * Opaque internal data types.
 * Note: client is responsible for serializing access to these structures if
 * they are shared between concurrent threads.
 */

typedef struct _DNSServiceRef_t *DNSServiceRef;
typedef struct _DNSRecordRef_t *DNSRecordRef;

/* General flags used in functions defined below */
enum
    {
    kDNSServiceFlagsMoreComing          = 0x1,
    /* MoreComing indicates to a callback that at least one more result is
     * queued and will be delivered following immediately after this one.
     * Applications should not update their UI to display browse
     * results when the MoreComing flag is set, because this would
     * result in a great deal of ugly flickering on the screen.
     * Applications should instead wait until until MoreComing is not set,
     * and then update their UI.
     * When MoreComing is not set, that doesn't mean there will be no more
     * answers EVER, just that there are no more answers immediately
     * available right now at this instant. If more answers become available
     * in the future they will be delivered as usual.
     */

    kDNSServiceFlagsAdd                 = 0x2,
    kDNSServiceFlagsDefault             = 0x4,
    /* Flags for domain enumeration and browse/query reply callbacks.
     * "Default" applies only to enumeration and is only valid in
     * conjuction with "Add".  An enumeration callback with the "Add"
     * flag NOT set indicates a "Remove", i.e. the domain is no longer
     * valid.
     */

    kDNSServiceFlagsNoAutoRename        = 0x8,
    /* Flag for specifying renaming behavior on name conflict when registering
     * non-shared records. By default, name conflicts are automatically handled
     * by renaming the service.  NoAutoRename overrides this behavior - with this
     * flag set, name conflicts will result in a callback.  The NoAutorename flag
     * is only valid if a name is explicitly specified when registering a service
     * (i.e. the default name is not used.)
     */

    kDNSServiceFlagsShared              = 0x10,
    kDNSServiceFlagsUnique              = 0x20,
    /* Flag for registering individual records on a connected
     * DNSServiceRef.  Shared indicates that there may be multiple records
     * with this name on the network (e.g. PTR records).  Unique indicates that the
     * record's name is to be unique on the network (e.g. SRV records).
     */

    kDNSServiceFlagsBrowseDomains       = 0x40,
    kDNSServiceFlagsRegistrationDomains = 0x80,
    /* Flags for specifying domain enumeration type in DNSServiceEnumerateDomains.
     * BrowseDomains enumerates domains recommended for browsing, RegistrationDomains
     * enumerates domains recommended for registration.
     */

    kDNSServiceFlagsLongLivedQuery      = 0x100,
    /* Flag for creating a long-lived unicast query for the DNSServiceQueryRecord call. */

    kDNSServiceFlagsAllowRemoteQuery    = 0x200,
    /* Flag for creating a record for which we will answer remote queries
     * (queries from hosts more than one hop away; hosts not directly connected to the local link).
     */

    kDNSServiceFlagsForceMulticast      = 0x400,
    /* Flag for signifying that a query or registration should be performed exclusively via multicast DNS,
     * even for a name in a domain (e.g. foo.apple.com.) that would normally imply unicast DNS.
     */

    kDNSServiceFlagsReturnCNAME         = 0x800
    /* Flag for returning CNAME records in the DNSServiceQueryRecord call. CNAME records are
     * normally followed without indicating to the client that there was a CNAME record.
     */
    };

/*
 * The values for DNS Classes and Types are listed in RFC 1035, and are available
 * on every OS in its DNS header file. Unfortunately every OS does not have the
 * same header file containing DNS Class and Type constants, and the names of
 * the constants are not consistent. For example, BIND 8 uses "T_A",
 * BIND 9 uses "ns_t_a", Windows uses "DNS_TYPE_A", etc.
 * For this reason, these constants are also listed here, so that code using
 * the DNS-SD programming APIs can use these constants, so that the same code
 * can compile on all our supported platforms.
 */

enum
    {
    kDNSServiceClass_IN       = 1       /* Internet */
    };

enum
    {
    kDNSServiceType_A         = 1,      /* Host address. */
    kDNSServiceType_NS        = 2,      /* Authoritative server. */
    kDNSServiceType_MD        = 3,      /* Mail destination. */
    kDNSServiceType_MF        = 4,      /* Mail forwarder. */
    kDNSServiceType_CNAME     = 5,      /* Canonical name. */
    kDNSServiceType_SOA       = 6,      /* Start of authority zone. */
    kDNSServiceType_MB        = 7,      /* Mailbox domain name. */
    kDNSServiceType_MG        = 8,      /* Mail group member. */
    kDNSServiceType_MR        = 9,      /* Mail rename name. */
    kDNSServiceType_NULL      = 10,     /* Null resource record. */
    kDNSServiceType_WKS       = 11,     /* Well known service. */
    kDNSServiceType_PTR       = 12,     /* Domain name pointer. */
    kDNSServiceType_HINFO     = 13,     /* Host information. */
    kDNSServiceType_MINFO     = 14,     /* Mailbox information. */
    kDNSServiceType_MX        = 15,     /* Mail routing information. */
    kDNSServiceType_TXT       = 16,     /* One or more text strings. */
    kDNSServiceType_RP        = 17,     /* Responsible person. */
    kDNSServiceType_AFSDB     = 18,     /* AFS cell database. */
    kDNSServiceType_X25       = 19,     /* X_25 calling address. */
    kDNSServiceType_ISDN      = 20,     /* ISDN calling address. */
    kDNSServiceType_RT        = 21,     /* Router. */
    kDNSServiceType_NSAP      = 22,     /* NSAP address. */
    kDNSServiceType_NSAP_PTR  = 23,     /* Reverse NSAP lookup (deprecated). */
    kDNSServiceType_SIG       = 24,     /* Security signature. */
    kDNSServiceType_KEY       = 25,     /* Security key. */
    kDNSServiceType_PX        = 26,     /* X.400 mail mapping. */
    kDNSServiceType_GPOS      = 27,     /* Geographical position (withdrawn). */
    kDNSServiceType_AAAA      = 28,     /* IPv6 Address. */
    kDNSServiceType_LOC       = 29,     /* Location Information. */
    kDNSServiceType_NXT       = 30,     /* Next domain (security). */
    kDNSServiceType_EID       = 31,     /* Endpoint identifier. */
    kDNSServiceType_NIMLOC    = 32,     /* Nimrod Locator. */
    kDNSServiceType_SRV       = 33,     /* Server Selection. */
    kDNSServiceType_ATMA      = 34,     /* ATM Address */
    kDNSServiceType_NAPTR     = 35,     /* Naming Authority PoinTeR */
    kDNSServiceType_KX        = 36,     /* Key Exchange */
    kDNSServiceType_CERT      = 37,     /* Certification record */
    kDNSServiceType_A6        = 38,     /* IPv6 Address (deprecated) */
    kDNSServiceType_DNAME     = 39,     /* Non-terminal DNAME (for IPv6) */
    kDNSServiceType_SINK      = 40,     /* Kitchen sink (experimentatl) */
    kDNSServiceType_OPT       = 41,     /* EDNS0 option (meta-RR) */
    kDNSServiceType_TKEY      = 249,    /* Transaction key */
    kDNSServiceType_TSIG      = 250,    /* Transaction signature. */
    kDNSServiceType_IXFR      = 251,    /* Incremental zone transfer. */
    kDNSServiceType_AXFR      = 252,    /* Transfer zone of authority. */
    kDNSServiceType_MAILB     = 253,    /* Transfer mailbox records. */
    kDNSServiceType_MAILA     = 254,    /* Transfer mail agent records. */
    kDNSServiceType_ANY       = 255     /* Wildcard match. */
    };


/* possible error code values */
enum
    {
    kDNSServiceErr_NoError             = 0,
    kDNSServiceErr_Unknown             = -65537,       /* 0xFFFE FFFF */
    kDNSServiceErr_NoSuchName          = -65538,
    kDNSServiceErr_NoMemory            = -65539,
    kDNSServiceErr_BadParam            = -65540,
    kDNSServiceErr_BadReference        = -65541,
    kDNSServiceErr_BadState            = -65542,
    kDNSServiceErr_BadFlags            = -65543,
    kDNSServiceErr_Unsupported         = -65544,
    kDNSServiceErr_NotInitialized      = -65545,
    kDNSServiceErr_AlreadyRegistered   = -65547,
    kDNSServiceErr_NameConflict        = -65548,
    kDNSServiceErr_Invalid             = -65549,
    kDNSServiceErr_Firewall            = -65550,
    kDNSServiceErr_Incompatible        = -65551,        /* client library incompatible with daemon */
    kDNSServiceErr_BadInterfaceIndex   = -65552,
    kDNSServiceErr_Refused             = -65553,
    kDNSServiceErr_NoSuchRecord        = -65554,
    kDNSServiceErr_NoAuth              = -65555,
    kDNSServiceErr_NoSuchKey           = -65556,
    kDNSServiceErr_NATTraversal        = -65557,
    kDNSServiceErr_DoubleNAT           = -65558,
    kDNSServiceErr_BadTime             = -65559
    /* mDNS Error codes are in the range
     * FFFE FF00 (-65792) to FFFE FFFF (-65537) */
    };


/* Maximum length, in bytes, of a service name represented as a */
/* literal C-String, including the terminating NULL at the end. */

#define kDNSServiceMaxServiceName 64

/* Maximum length, in bytes, of a domain name represented as an *escaped* C-String */
/* including the final trailing dot, and the C-String terminating NULL at the end. */

#define kDNSServiceMaxDomainName 1005

/*
 * Notes on DNS Name Escaping
 *   -- or --
 * "Why is kDNSServiceMaxDomainName 1005, when the maximum legal domain name is 255 bytes?"
 *
 * All strings used in DNS-SD are UTF-8 strings.
 * With few exceptions, most are also escaped using standard DNS escaping rules:
 *
 *   '\\' represents a single literal '\' in the name
 *   '\.' represents a single literal '.' in the name
 *   '\ddd', where ddd is a three-digit decimal value from 000 to 255,
 *        represents a single literal byte with that value.
 *   A bare unescaped '.' is a label separator, marking a boundary between domain and subdomain.
 *
 * The exceptions, that do not use escaping, are the routines where the full
 * DNS name of a resource is broken, for convenience, into servicename/regtype/domain.
 * In these routines, the "servicename" is NOT escaped. It does not need to be, since
 * it is, by definition, just a single literal string. Any characters in that string
 * represent exactly what they are. The "regtype" portion is, technically speaking,
 * escaped, but since legal regtypes are only allowed to contain letters, digits,
 * and hyphens, there is nothing to escape, so the issue is moot. The "domain"
 * portion is also escaped, though most domains in use on the public Internet
 * today, like regtypes, don't contain any characters that need to be escaped.
 * As DNS-SD becomes more popular, rich-text domains for service discovery will
 * become common, so software should be written to cope with domains with escaping.
 *
 * The servicename may be up to 63 bytes of UTF-8 text (not counting the C-String
 * terminating NULL at the end). The regtype is of the form _service._tcp or
 * _service._udp, where the "service" part is 1-14 characters, which may be
 * letters, digits, or hyphens. The domain part of the three-part name may be
 * any legal domain, providing that the resulting servicename+regtype+domain
 * name does not exceed 255 bytes.
 *
 * For most software, these issues are transparent. When browsing, the discovered
 * servicenames should simply be displayed as-is. When resolving, the discovered
 * servicename/regtype/domain are simply passed unchanged to DNSServiceResolve().
 * When a DNSServiceResolve() succeeds, the returned fullname is already in
 * the correct format to pass to standard system DNS APIs such as res_query().
 * For converting from servicename/regtype/domain to a single properly-escaped
 * full DNS name, the helper function DNSServiceConstructFullName() is provided.
 *
 * The following (highly contrived) example illustrates the escaping process.
 * Suppose you have an service called "Dr. Smith\Dr. Johnson", of type "_ftp._tcp"
 * in subdomain "4th. Floor" of subdomain "Building 2" of domain "apple.com."
 * The full (escaped) DNS name of this service's SRV record would be:
 * Dr\.\032Smith\\Dr\.\032Johnson._ftp._tcp.4th\.\032Floor.Building\0322.apple.com.
 */


/*
 * Constants for specifying an interface index
 *
 * Specific interface indexes are identified via a 32-bit unsigned integer returned
 * by the if_nametoindex() family of calls.
 *
 * If the client passes 0 for interface index, that means "do the right thing",
 * which (at present) means, "if the name is in an mDNS local multicast domain
 * (e.g. 'local.', '254.169.in-addr.arpa.', '{8,9,A,B}.E.F.ip6.arpa.') then multicast
 * on all applicable interfaces, otherwise send via unicast to the appropriate
 * DNS server." Normally, most clients will use 0 for interface index to
 * automatically get the default sensible behaviour.
 *
 * If the client passes a positive interface index, then for multicast names that
 * indicates to do the operation only on that one interface. For unicast names the
 * interface index is ignored unless kDNSServiceFlagsForceMulticast is also set.
 *
 * If the client passes kDNSServiceInterfaceIndexLocalOnly when registering
 * a service, then that service will be found *only* by other local clients
 * on the same machine that are browsing using kDNSServiceInterfaceIndexLocalOnly
 * or kDNSServiceInterfaceIndexAny.
 * If a client has a 'private' service, accessible only to other processes
 * running on the same machine, this allows the client to advertise that service
 * in a way such that it does not inadvertently appear in service lists on
 * all the other machines on the network.
 *
 * If the client passes kDNSServiceInterfaceIndexLocalOnly when browsing
 * then it will find *all* records registered on that same local machine.
 * Clients explicitly wishing to discover *only* LocalOnly services can
 * accomplish this by inspecting the interfaceIndex of each service reported
 * to their DNSServiceBrowseReply() callback function, and discarding those
 * where the interface index is not kDNSServiceInterfaceIndexLocalOnly.
 */

#define kDNSServiceInterfaceIndexAny 0
#define kDNSServiceInterfaceIndexLocalOnly ( (uint32_t) -1 )


typedef uint32_t DNSServiceFlags;
typedef int32_t DNSServiceErrorType;


/*********************************************************************************************
 *
 * Unix Domain Socket access, DNSServiceRef deallocation, and data processing functions
 *
 *********************************************************************************************/


/* DNSServiceRefSockFD()
 *
 * Access underlying Unix domain socket for an initialized DNSServiceRef.
 * The DNS Service Discovery implmementation uses this socket to communicate between
 * the client and the mDNSResponder daemon.  The application MUST NOT directly read from
 * or write to this socket.  Access to the socket is provided so that it can be used as a
 * run loop source, or in a select() loop: when data is available for reading on the socket,
 * DNSServiceProcessResult() should be called, which will extract the daemon's reply from
 * the socket, and pass it to the appropriate application callback.  By using a run loop or
 * select(), results from the daemon can be processed asynchronously.  Without using these
 * constructs, DNSServiceProcessResult() will block until the response from the daemon arrives.
 * The client is responsible for ensuring that the data on the socket is processed in a timely
 * fashion - the daemon may terminate its connection with a client that does not clear its
 * socket buffer.
 *
 * sdRef:            A DNSServiceRef initialized by any of the DNSService calls.
 *
 * return value:    The DNSServiceRef's underlying socket descriptor, or -1 on
 *                  error.
 */

int DNSSD_API DNSServiceRefSockFD(DNSServiceRef sdRef);


/* DNSServiceProcessResult()
 *
 * Read a reply from the daemon, calling the appropriate application callback.  This call will
 * block until the daemon's response is received.  Use DNSServiceRefSockFD() in
 * conjunction with a run loop or select() to determine the presence of a response from the
 * server before calling this function to process the reply without blocking.  Call this function
 * at any point if it is acceptable to block until the daemon's response arrives.  Note that the
 * client is responsible for ensuring that DNSServiceProcessResult() is called whenever there is
 * a reply from the daemon - the daemon may terminate its connection with a client that does not
 * process the daemon's responses.
 *
 * sdRef:           A DNSServiceRef initialized by any of the DNSService calls
 *                  that take a callback parameter.
 *
 * return value:    Returns kDNSServiceErr_NoError on success, otherwise returns
 *                  an error code indicating the specific failure that occurred.
 */

DNSServiceErrorType DNSSD_API DNSServiceProcessResult(DNSServiceRef sdRef);


/* DNSServiceRefDeallocate()
 *
 * Terminate a connection with the daemon and free memory associated with the DNSServiceRef.
 * Any services or records registered with this DNSServiceRef will be deregistered. Any
 * Browse, Resolve, or Query operations called with this reference will be terminated.
 *
 * Note: If the reference's underlying socket is used in a run loop or select() call, it should
 * be removed BEFORE DNSServiceRefDeallocate() is called, as this function closes the reference's
 * socket.
 *
 * Note: If the reference was initialized with DNSServiceCreateConnection(), any DNSRecordRefs
 * created via this reference will be invalidated by this call - the resource records are
 * deregistered, and their DNSRecordRefs may not be used in subsequent functions.  Similarly,
 * if the reference was initialized with DNSServiceRegister, and an extra resource record was
 * added to the service via DNSServiceAddRecord(), the DNSRecordRef created by the Add() call
 * is invalidated when this function is called - the DNSRecordRef may not be used in subsequent
 * functions.
 *
 * Note: This call is to be used only with the DNSServiceRef defined by this API.  It is
 * not compatible with dns_service_discovery_ref objects defined in the legacy Mach-based
 * DNSServiceDiscovery.h API.
 *
 * sdRef:           A DNSServiceRef initialized by any of the DNSService calls.
 *
 */

void DNSSD_API DNSServiceRefDeallocate(DNSServiceRef sdRef);


/*********************************************************************************************
 *
 * Domain Enumeration
 *
 *********************************************************************************************/

/* DNSServiceEnumerateDomains()
 *
 * Asynchronously enumerate domains available for browsing and registration.
 *
 * The enumeration MUST be cancelled via DNSServiceRefDeallocate() when no more domains
 * are to be found.
 *
 * Note that the names returned are (like all of DNS-SD) UTF-8 strings,
 * and are escaped using standard DNS escaping rules.
 * (See "Notes on DNS Name Escaping" earlier in this file for more details.)
 * A graphical browser displaying a hierarchical tree-structured view should cut
 * the names at the bare dots to yield individual labels, then de-escape each
 * label according to the escaping rules, and then display the resulting UTF-8 text.
 *
 * DNSServiceDomainEnumReply Callback Parameters:
 *
 * sdRef:           The DNSServiceRef initialized by DNSServiceEnumerateDomains().
 *
 * flags:           Possible values are:
 *                  kDNSServiceFlagsMoreComing
 *                  kDNSServiceFlagsAdd
 *                  kDNSServiceFlagsDefault
 *
 * interfaceIndex:  Specifies the interface on which the domain exists.  (The index for a given
 *                  interface is determined via the if_nametoindex() family of calls.)
 *
 * errorCode:       Will be kDNSServiceErr_NoError (0) on success, otherwise indicates
 *                  the failure that occurred (other parameters are undefined if errorCode is nonzero).
 *
 * replyDomain:     The name of the domain.
 *
 * context:         The context pointer passed to DNSServiceEnumerateDomains.
 *
 */

typedef void (DNSSD_API *DNSServiceDomainEnumReply)
    (
    DNSServiceRef                       sdRef,
    DNSServiceFlags                     flags,
    uint32_t                            interfaceIndex,
    DNSServiceErrorType                 errorCode,
    const char                          *replyDomain,
    void                                *context
    );


/* DNSServiceEnumerateDomains() Parameters:
 *
 *
 * sdRef:           A pointer to an uninitialized DNSServiceRef. If the call succeeds
 *                  then it initializes the DNSServiceRef, returns kDNSServiceErr_NoError,
 *                  and the enumeration operation will run indefinitely until the client
 *                  terminates it by passing this DNSServiceRef to DNSServiceRefDeallocate().
 *
 * flags:           Possible values are:
 *                  kDNSServiceFlagsBrowseDomains to enumerate domains recommended for browsing.
 *                  kDNSServiceFlagsRegistrationDomains to enumerate domains recommended
 *                  for registration.
 *
 * interfaceIndex:  If non-zero, specifies the interface on which to look for domains.
 *                  (the index for a given interface is determined via the if_nametoindex()
 *                  family of calls.)  Most applications will pass 0 to enumerate domains on
 *                  all interfaces. See "Constants for specifying an interface index" for more details.
 *
 * callBack:        The function to be called when a domain is found or the call asynchronously
 *                  fails.
 *
 * context:         An application context pointer which is passed to the callback function
 *                  (may be NULL).
 *
 * return value:    Returns kDNSServiceErr_NoError on succeses (any subsequent, asynchronous
 *                  errors are delivered to the callback), otherwise returns an error code indicating
 *                  the error that occurred (the callback is not invoked and the DNSServiceRef
 *                  is not initialized.)
 */

DNSServiceErrorType DNSSD_API DNSServiceEnumerateDomains
    (
    DNSServiceRef                       *sdRef,
    DNSServiceFlags                     flags,
    uint32_t                            interfaceIndex,
    DNSServiceDomainEnumReply           callBack,
    void                                *context  /* may be NULL */
    );


/*********************************************************************************************
 *
 *  Service Registration
 *
 *********************************************************************************************/

/* Register a service that is discovered via Browse() and Resolve() calls.
 *
 *
 * DNSServiceRegisterReply() Callback Parameters:
 *
 * sdRef:           The DNSServiceRef initialized by DNSServiceRegister().
 *
 * flags:           Currently unused, reserved for future use.
 *
 * errorCode:       Will be kDNSServiceErr_NoError on success, otherwise will
 *                  indicate the failure that occurred (including name conflicts,
 *                  if the kDNSServiceFlagsNoAutoRename flag was used when registering.)
 *                  Other parameters are undefined if errorCode is nonzero.
 *
 * name:            The service name registered (if the application did not specify a name in
 *                  DNSServiceRegister(), this indicates what name was automatically chosen).
 *
 * regtype:         The type of service registered, as it was passed to the callout.
 *
 * domain:          The domain on which the service was registered (if the application did not
 *                  specify a domain in DNSServiceRegister(), this indicates the default domain
 *                  on which the service was registered).
 *
 * context:         The context pointer that was passed to the callout.
 *
 */

typedef void (DNSSD_API *DNSServiceRegisterReply)
    (
    DNSServiceRef                       sdRef,
    DNSServiceFlags                     flags,
    DNSServiceErrorType                 errorCode,
    const char                          *name,
    const char                          *regtype,
    const char                          *domain,
    void                                *context
    );


/* DNSServiceRegister()  Parameters:
 *
 * sdRef:           A pointer to an uninitialized DNSServiceRef. If the call succeeds
 *                  then it initializes the DNSServiceRef, returns kDNSServiceErr_NoError,
 *                  and the registration will remain active indefinitely until the client
 *                  terminates it by passing this DNSServiceRef to DNSServiceRefDeallocate().
 *
 * interfaceIndex:  If non-zero, specifies the interface on which to register the service
 *                  (the index for a given interface is determined via the if_nametoindex()
 *                  family of calls.)  Most applications will pass 0 to register on all
 *                  available interfaces. See "Constants for specifying an interface index" for more details.
 *
 * flags:           Indicates the renaming behavior on name conflict (most applications
 *                  will pass 0).  See flag definitions above for details.
 *
 * name:            If non-NULL, specifies the service name to be registered.
 *                  Most applications will not specify a name, in which case the computer
 *                  name is used (this name is communicated to the client via the callback).
 *                  If a name is specified, it must be 1-63 bytes of UTF-8 text.
 *                  If the name is longer than 63 bytes it will be automatically truncated
 *                  to a legal length, unless the NoAutoRename flag is set,
 *                  in which case kDNSServiceErr_BadParam will be returned.
 *
 * regtype:         The service type followed by the protocol, separated by a dot
 *                  (e.g. "_ftp._tcp"). The service type must be an underscore, followed
 *                  by 1-14 characters, which may be letters, digits, or hyphens.
 *                  The transport protocol must be "_tcp" or "_udp". New service types
 *                  should be registered at <http://www.dns-sd.org/ServiceTypes.html>.
 *
 * domain:          If non-NULL, specifies the domain on which to advertise the service.
 *                  Most applications will not specify a domain, instead automatically
 *                  registering in the default domain(s).
 *
 * host:            If non-NULL, specifies the SRV target host name.  Most applications
 *                  will not specify a host, instead automatically using the machine's
 *                  default host name(s).  Note that specifying a non-NULL host does NOT
 *                  create an address record for that host - the application is responsible
 *                  for ensuring that the appropriate address record exists, or creating it
 *                  via DNSServiceRegisterRecord().
 *
 * port:            The port, in network byte order, on which the service accepts connections.
 *                  Pass 0 for a "placeholder" service (i.e. a service that will not be discovered
 *                  by browsing, but will cause a name conflict if another client tries to
 *                  register that same name).  Most clients will not use placeholder services.
 *
 * txtLen:          The length of the txtRecord, in bytes.  Must be zero if the txtRecord is NULL.
 *
 * txtRecord:       The TXT record rdata. A non-NULL txtRecord MUST be a properly formatted DNS
 *                  TXT record, i.e. <length byte> <data> <length byte> <data> ...
 *                  Passing NULL for the txtRecord is allowed as a synonym for txtLen=1, txtRecord="",
 *                  i.e. it creates a TXT record of length one containing a single empty string.
 *                  RFC 1035 doesn't allow a TXT record to contain *zero* strings, so a single empty
 *                  string is the smallest legal DNS TXT record.
 *                  As with the other parameters, the DNSServiceRegister call copies the txtRecord
 *                  data; e.g. if you allocated the storage for the txtRecord parameter with malloc()
 *                  then you can safely free that memory right after the DNSServiceRegister call returns.
 *
 * callBack:        The function to be called when the registration completes or asynchronously
 *                  fails.  The client MAY pass NULL for the callback -  The client will NOT be notified
 *                  of the default values picked on its behalf, and the client will NOT be notified of any
 *                  asynchronous errors (e.g. out of memory errors, etc.) that may prevent the registration
 *                  of the service.  The client may NOT pass the NoAutoRename flag if the callback is NULL.
 *                  The client may still deregister the service at any time via DNSServiceRefDeallocate().
 *
 * context:         An application context pointer which is passed to the callback function
 *                  (may be NULL).
 *
 * return value:    Returns kDNSServiceErr_NoError on succeses (any subsequent, asynchronous
 *                  errors are delivered to the callback), otherwise returns an error code indicating
 *                  the error that occurred (the callback is never invoked and the DNSServiceRef
 *                  is not initialized.)
 */

DNSServiceErrorType DNSSD_API DNSServiceRegister
    (
    DNSServiceRef                       *sdRef,
    DNSServiceFlags                     flags,
    uint32_t                            interfaceIndex,
    const char                          *name,         /* may be NULL */
    const char                          *regtype,
    const char                          *domain,       /* may be NULL */
    const char                          *host,         /* may be NULL */
    uint16_t                            port,
    uint16_t                            txtLen,
    const void                          *txtRecord,    /* may be NULL */
    DNSServiceRegisterReply             callBack,      /* may be NULL */
    void                                *context       /* may be NULL */
    );


/* DNSServiceAddRecord()
 *
 * Add a record to a registered service.  The name of the record will be the same as the
 * registered service's name.
 * The record can later be updated or deregistered by passing the RecordRef initialized
 * by this function to DNSServiceUpdateRecord() or DNSServiceRemoveRecord().
 *
 * Note that the DNSServiceAddRecord/UpdateRecord/RemoveRecord are *NOT* thread-safe
 * with respect to a single DNSServiceRef. If you plan to have multiple threads
 * in your program simultaneously add, update, or remove records from the same
 * DNSServiceRef, then it's the caller's responsibility to use a mutext lock
 * or take similar appropriate precautions to serialize those calls.
 *
 *
 * Parameters;
 *
 * sdRef:           A DNSServiceRef initialized by DNSServiceRegister().
 *
 * RecordRef:       A pointer to an uninitialized DNSRecordRef.  Upon succesfull completion of this
 *                  call, this ref may be passed to DNSServiceUpdateRecord() or DNSServiceRemoveRecord().
 *                  If the above DNSServiceRef is passed to DNSServiceRefDeallocate(), RecordRef is also
 *                  invalidated and may not be used further.
 *
 * flags:           Currently ignored, reserved for future use.
 *
 * rrtype:          The type of the record (e.g. kDNSServiceType_TXT, kDNSServiceType_SRV, etc)
 *
 * rdlen:           The length, in bytes, of the rdata.
 *
 * rdata:           The raw rdata to be contained in the added resource record.
 *
 * ttl:             The time to live of the resource record, in seconds.  Pass 0 to use a default value.
 *
 * return value:    Returns kDNSServiceErr_NoError on success, otherwise returns an
 *                  error code indicating the error that occurred (the RecordRef is not initialized).
 */

DNSServiceErrorType DNSSD_API DNSServiceAddRecord
    (
    DNSServiceRef                       sdRef,
    DNSRecordRef                        *RecordRef,
    DNSServiceFlags                     flags,
    uint16_t                            rrtype,
    uint16_t                            rdlen,
    const void                          *rdata,
    uint32_t                            ttl
    );


/* DNSServiceUpdateRecord
 *
 * Update a registered resource record.  The record must either be:
 *   - The primary txt record of a service registered via DNSServiceRegister()
 *   - A record added to a registered service via DNSServiceAddRecord()
 *   - An individual record registered by DNSServiceRegisterRecord()
 *
 *
 * Parameters:
 *
 * sdRef:           A DNSServiceRef that was initialized by DNSServiceRegister()
 *                  or DNSServiceCreateConnection().
 *
 * RecordRef:       A DNSRecordRef initialized by DNSServiceAddRecord, or NULL to update the
 *                  service's primary txt record.
 *
 * flags:           Currently ignored, reserved for future use.
 *
 * rdlen:           The length, in bytes, of the new rdata.
 *
 * rdata:           The new rdata to be contained in the updated resource record.
 *
 * ttl:             The time to live of the updated resource record, in seconds.
 *
 * return value:    Returns kDNSServiceErr_NoError on success, otherwise returns an
 *                  error code indicating the error that occurred.
 */

DNSServiceErrorType DNSSD_API DNSServiceUpdateRecord
    (
    DNSServiceRef                       sdRef,
    DNSRecordRef                        RecordRef,     /* may be NULL */
    DNSServiceFlags                     flags,
    uint16_t                            rdlen,
    const void                          *rdata,
    uint32_t                            ttl
    );


/* DNSServiceRemoveRecord
 *
 * Remove a record previously added to a service record set via DNSServiceAddRecord(), or deregister
 * an record registered individually via DNSServiceRegisterRecord().
 *
 * Parameters:
 *
 * sdRef:           A DNSServiceRef initialized by DNSServiceRegister() (if the
 *                  record being removed was registered via DNSServiceAddRecord()) or by
 *                  DNSServiceCreateConnection() (if the record being removed was registered via
 *                  DNSServiceRegisterRecord()).
 *
 * recordRef:       A DNSRecordRef initialized by a successful call to DNSServiceAddRecord()
 *                  or DNSServiceRegisterRecord().
 *
 * flags:           Currently ignored, reserved for future use.
 *
 * return value:    Returns kDNSServiceErr_NoError on success, otherwise returns an
 *                  error code indicating the error that occurred.
 */

DNSServiceErrorType DNSSD_API DNSServiceRemoveRecord
    (
    DNSServiceRef                 sdRef,
    DNSRecordRef                  RecordRef,
    DNSServiceFlags               flags
    );


/*********************************************************************************************
 *
 *  Service Discovery
 *
 *********************************************************************************************/

/* Browse for instances of a service.
 *
 *
 * DNSServiceBrowseReply() Parameters:
 *
 * sdRef:           The DNSServiceRef initialized by DNSServiceBrowse().
 *
 * flags:           Possible values are kDNSServiceFlagsMoreComing and kDNSServiceFlagsAdd.
 *                  See flag definitions for details.
 *
 * interfaceIndex:  The interface on which the service is advertised.  This index should
 *                  be passed to DNSServiceResolve() when resolving the service.
 *
 * errorCode:       Will be kDNSServiceErr_NoError (0) on success, otherwise will
 *                  indicate the failure that occurred.  Other parameters are undefined if
 *                  the errorCode is nonzero.
 *
 * serviceName:     The discovered service name. This name should be displayed to the user,
 *                  and stored for subsequent use in the DNSServiceResolve() call.
 *
 * regtype:         The service type, which is usually (but not always) the same as was passed
 *                  to DNSServiceBrowse(). One case where the discovered service type may
 *                  not be the same as the requested service type is when using subtypes:
 *                  The client may want to browse for only those ftp servers that allow
 *                  anonymous connections. The client will pass the string "_ftp._tcp,_anon"
 *                  to DNSServiceBrowse(), but the type of the service that's discovered
 *                  is simply "_ftp._tcp". The regtype for each discovered service instance
 *                  should be stored along with the name, so that it can be passed to
 *                  DNSServiceResolve() when the service is later resolved.
 *
 * domain:          The domain of the discovered service instance. This may or may not be the
 *                  same as the domain that was passed to DNSServiceBrowse(). The domain for each
 *                  discovered service instance should be stored along with the name, so that
 *                  it can be passed to DNSServiceResolve() when the service is later resolved.
 *
 * context:         The context pointer that was passed to the callout.
 *
 */

typedef void (DNSSD_API *DNSServiceBrowseReply)
    (
    DNSServiceRef                       sdRef,
    DNSServiceFlags                     flags,
    uint32_t                            interfaceIndex,
    DNSServiceErrorType                 errorCode,
    const char                          *serviceName,
    const char                          *regtype,
    const char                          *replyDomain,
    void                                *context
    );


/* DNSServiceBrowse() Parameters:
 *
 * sdRef:           A pointer to an uninitialized DNSServiceRef. If the call succeeds
 *                  then it initializes the DNSServiceRef, returns kDNSServiceErr_NoError,
 *                  and the browse operation will run indefinitely until the client
 *                  terminates it by passing this DNSServiceRef to DNSServiceRefDeallocate().
 *
 * flags:           Currently ignored, reserved for future use.
 *
 * interfaceIndex:  If non-zero, specifies the interface on which to browse for services
 *                  (the index for a given interface is determined via the if_nametoindex()
 *                  family of calls.)  Most applications will pass 0 to browse on all available
 *                  interfaces. See "Constants for specifying an interface index" for more details.
 *
 * regtype:         The service type being browsed for followed by the protocol, separated by a
 *                  dot (e.g. "_ftp._tcp").  The transport protocol must be "_tcp" or "_udp".
 *
 * domain:          If non-NULL, specifies the domain on which to browse for services.
 *                  Most applications will not specify a domain, instead browsing on the
 *                  default domain(s).
 *
 * callBack:        The function to be called when an instance of the service being browsed for
 *                  is found, or if the call asynchronously fails.
 *
 * context:         An application context pointer which is passed to the callback function
 *                  (may be NULL).
 *
 * return value:    Returns kDNSServiceErr_NoError on succeses (any subsequent, asynchronous
 *                  errors are delivered to the callback), otherwise returns an error code indicating
 *                  the error that occurred (the callback is not invoked and the DNSServiceRef
 *                  is not initialized.)
 */

DNSServiceErrorType DNSSD_API DNSServiceBrowse
    (
    DNSServiceRef                       *sdRef,
    DNSServiceFlags                     flags,
    uint32_t                            interfaceIndex,
    const char                          *regtype,
    const char                          *domain,    /* may be NULL */
    DNSServiceBrowseReply               callBack,
    void                                *context    /* may be NULL */
    );


/* DNSServiceResolve()
 *
 * Resolve a service name discovered via DNSServiceBrowse() to a target host name, port number, and
 * txt record.
 *
 * Note: Applications should NOT use DNSServiceResolve() solely for txt record monitoring - use
 * DNSServiceQueryRecord() instead, as it is more efficient for this task.
 *
 * Note: When the desired results have been returned, the client MUST terminate the resolve by calling
 * DNSServiceRefDeallocate().
 *
 * Note: DNSServiceResolve() behaves correctly for typical services that have a single SRV record
 * and a single TXT record. To resolve non-standard services with multiple SRV or TXT records,
 * DNSServiceQueryRecord() should be used.
 *
 * DNSServiceResolveReply Callback Parameters:
 *
 * sdRef:           The DNSServiceRef initialized by DNSServiceResolve().
 *
 * flags:           Currently unused, reserved for future use.
 *
 * interfaceIndex:  The interface on which the service was resolved.
 *
 * errorCode:       Will be kDNSServiceErr_NoError (0) on success, otherwise will
 *                  indicate the failure that occurred.  Other parameters are undefined if
 *                  the errorCode is nonzero.
 *
 * fullname:        The full service domain name, in the form <servicename>.<protocol>.<domain>.
 *                  (This name is escaped following standard DNS rules, making it suitable for
 *                  passing to standard system DNS APIs such as res_query(), or to the
 *                  special-purpose functions included in this API that take fullname parameters.
 *                  See "Notes on DNS Name Escaping" earlier in this file for more details.)
 *
 * hosttarget:      The target hostname of the machine providing the service.  This name can
 *                  be passed to functions like gethostbyname() to identify the host's IP address.
 *
 * port:            The port, in network byte order, on which connections are accepted for this service.
 *
 * txtLen:          The length of the txt record, in bytes.
 *
 * txtRecord:       The service's primary txt record, in standard txt record format.
 *
 * context:         The context pointer that was passed to the callout.
 *
 * NOTE: In earlier versions of this header file, the txtRecord parameter was declared "const char *"
 * This is incorrect, since it contains length bytes which are values in the range 0 to 255, not -128 to +127.
 * Depending on your compiler settings, this change may cause signed/unsigned mismatch warnings.
 * These should be fixed by updating your own callback function definition to match the corrected
 * function signature using "const unsigned char *txtRecord". Making this change may also fix inadvertent
 * bugs in your callback function, where it could have incorrectly interpreted a length byte with value 250
 * as being -6 instead, with various bad consequences ranging from incorrect operation to software crashes.
 * If you need to maintain portable code that will compile cleanly with both the old and new versions of
 * this header file, you should update your callback function definition to use the correct unsigned value,
 * and then in the place where you pass your callback function to DNSServiceResolve(), use a cast to eliminate
 * the compiler warning, e.g.:
 *   DNSServiceResolve(sd, flags, index, name, regtype, domain, (DNSServiceResolveReply)MyCallback, context);
 * This will ensure that your code compiles cleanly without warnings (and more importantly, works correctly)
 * with both the old header and with the new corrected version.
 *
 */

typedef void (DNSSD_API *DNSServiceResolveReply)
    (
    DNSServiceRef                       sdRef,
    DNSServiceFlags                     flags,
    uint32_t                            interfaceIndex,
    DNSServiceErrorType                 errorCode,
    const char                          *fullname,
    const char                          *hosttarget,
    uint16_t                            port,
    uint16_t                            txtLen,
    const unsigned char                 *txtRecord,
    void                                *context
    );


/* DNSServiceResolve() Parameters
 *
 * sdRef:           A pointer to an uninitialized DNSServiceRef. If the call succeeds
 *                  then it initializes the DNSServiceRef, returns kDNSServiceErr_NoError,
 *                  and the resolve operation will run indefinitely until the client
 *                  terminates it by passing this DNSServiceRef to DNSServiceRefDeallocate().
 *
 * flags:           Currently ignored, reserved for future use.
 *
 * interfaceIndex:  The interface on which to resolve the service. If this resolve call is
 *                  as a result of a currently active DNSServiceBrowse() operation, then the
 *                  interfaceIndex should be the index reported in the DNSServiceBrowseReply
 *                  callback. If this resolve call is using information previously saved
 *                  (e.g. in a preference file) for later use, then use interfaceIndex 0, because
 *                  the desired service may now be reachable via a different physical interface.
 *                  See "Constants for specifying an interface index" for more details.
 *
 * name:            The name of the service instance to be resolved, as reported to the
 *                  DNSServiceBrowseReply() callback.
 *
 * regtype:         The type of the service instance to be resolved, as reported to the
 *                  DNSServiceBrowseReply() callback.
 *
 * domain:          The domain of the service instance to be resolved, as reported to the
 *                  DNSServiceBrowseReply() callback.
 *
 * callBack:        The function to be called when a result is found, or if the call
 *                  asynchronously fails.
 *
 * context:         An application context pointer which is passed to the callback function
 *                  (may be NULL).
 *
 * return value:    Returns kDNSServiceErr_NoError on succeses (any subsequent, asynchronous
 *                  errors are delivered to the callback), otherwise returns an error code indicating
 *                  the error that occurred (the callback is never invoked and the DNSServiceRef
 *                  is not initialized.)
 */

DNSServiceErrorType DNSSD_API DNSServiceResolve
    (
    DNSServiceRef                       *sdRef,
    DNSServiceFlags                     flags,
    uint32_t                            interfaceIndex,
    const char                          *name,
    const char                          *regtype,
    const char                          *domain,
    DNSServiceResolveReply              callBack,
    void                                *context  /* may be NULL */
    );


/*********************************************************************************************
 *
 *  Special Purpose Calls (most applications will not use these)
 *
 *********************************************************************************************/

/* DNSServiceCreateConnection()
 *
 * Create a connection to the daemon allowing efficient registration of
 * multiple individual records.
 *
 *
 * Parameters:
 *
 * sdRef:           A pointer to an uninitialized DNSServiceRef.  Deallocating
 *                  the reference (via DNSServiceRefDeallocate()) severs the
 *                  connection and deregisters all records registered on this connection.
 *
 * return value:    Returns kDNSServiceErr_NoError on success, otherwise returns
 *                  an error code indicating the specific failure that occurred (in which
 *                  case the DNSServiceRef is not initialized).
 */

DNSServiceErrorType DNSSD_API DNSServiceCreateConnection(DNSServiceRef *sdRef);


/* DNSServiceRegisterRecord
 *
 * Register an individual resource record on a connected DNSServiceRef.
 *
 * Note that name conflicts occurring for records registered via this call must be handled
 * by the client in the callback.
 *
 *
 * DNSServiceRegisterRecordReply() parameters:
 *
 * sdRef:           The connected DNSServiceRef initialized by
 *                  DNSServiceCreateConnection().
 *
 * RecordRef:       The DNSRecordRef initialized by DNSServiceRegisterRecord().  If the above
 *                  DNSServiceRef is passed to DNSServiceRefDeallocate(), this DNSRecordRef is
 *                  invalidated, and may not be used further.
 *
 * flags:           Currently unused, reserved for future use.
 *
 * errorCode:       Will be kDNSServiceErr_NoError on success, otherwise will
 *                  indicate the failure that occurred (including name conflicts.)
 *                  Other parameters are undefined if errorCode is nonzero.
 *
 * context:         The context pointer that was passed to the callout.
 *
 */

 typedef void (DNSSD_API *DNSServiceRegisterRecordReply)
    (
    DNSServiceRef                       sdRef,
    DNSRecordRef                        RecordRef,
    DNSServiceFlags                     flags,
    DNSServiceErrorType                 errorCode,
    void                                *context
    );


/* DNSServiceRegisterRecord() Parameters:
 *
 * sdRef:           A DNSServiceRef initialized by DNSServiceCreateConnection().
 *
 * RecordRef:       A pointer to an uninitialized DNSRecordRef.  Upon succesfull completion of this
 *                  call, this ref may be passed to DNSServiceUpdateRecord() or DNSServiceRemoveRecord().
 *                  (To deregister ALL records registered on a single connected DNSServiceRef
 *                  and deallocate each of their corresponding DNSServiceRecordRefs, call
 *                  DNSServiceRefDealloocate()).
 *
 * flags:           Possible values are kDNSServiceFlagsShared or kDNSServiceFlagsUnique
 *                  (see flag type definitions for details).
 *
 * interfaceIndex:  If non-zero, specifies the interface on which to register the record
 *                  (the index for a given interface is determined via the if_nametoindex()
 *                  family of calls.)  Passing 0 causes the record to be registered on all interfaces.
 *                  See "Constants for specifying an interface index" for more details.
 *
 * fullname:        The full domain name of the resource record.
 *
 * rrtype:          The numerical type of the resource record (e.g. kDNSServiceType_PTR, kDNSServiceType_SRV, etc)
 *
 * rrclass:         The class of the resource record (usually kDNSServiceClass_IN)
 *
 * rdlen:           Length, in bytes, of the rdata.
 *
 * rdata:           A pointer to the raw rdata, as it is to appear in the DNS record.
 *
 * ttl:             The time to live of the resource record, in seconds.  Pass 0 to use a default value.
 *
 * callBack:        The function to be called when a result is found, or if the call
 *                  asynchronously fails (e.g. because of a name conflict.)
 *
 * context:         An application context pointer which is passed to the callback function
 *                  (may be NULL).
 *
 * return value:    Returns kDNSServiceErr_NoError on succeses (any subsequent, asynchronous
 *                  errors are delivered to the callback), otherwise returns an error code indicating
 *                  the error that occurred (the callback is never invoked and the DNSRecordRef is
 *                  not initialized.)
 */

DNSServiceErrorType DNSSD_API DNSServiceRegisterRecord
    (
    DNSServiceRef                       sdRef,
    DNSRecordRef                        *RecordRef,
    DNSServiceFlags                     flags,
    uint32_t                            interfaceIndex,
    const char                          *fullname,
    uint16_t                            rrtype,
    uint16_t                            rrclass,
    uint16_t                            rdlen,
    const void                          *rdata,
    uint32_t                            ttl,
    DNSServiceRegisterRecordReply       callBack,
    void                                *context    /* may be NULL */
    );


/* DNSServiceQueryRecord
 *
 * Query for an arbitrary DNS record.
 *
 *
 * DNSServiceQueryRecordReply() Callback Parameters:
 *
 * sdRef:           The DNSServiceRef initialized by DNSServiceQueryRecord().
 *
 * flags:           Possible values are kDNSServiceFlagsMoreComing and
 *                  kDNSServiceFlagsAdd.  The Add flag is NOT set for PTR records
 *                  with a ttl of 0, i.e. "Remove" events.
 *
 * interfaceIndex:  The interface on which the query was resolved (the index for a given
 *                  interface is determined via the if_nametoindex() family of calls).
 *                  See "Constants for specifying an interface index" for more details.
 *
 * errorCode:       Will be kDNSServiceErr_NoError on success, otherwise will
 *                  indicate the failure that occurred.  Other parameters are undefined if
 *                  errorCode is nonzero.
 *
 * fullname:        The resource record's full domain name.
 *
 * rrtype:          The resource record's type (e.g. kDNSServiceType_PTR, kDNSServiceType_SRV, etc)
 *
 * rrclass:         The class of the resource record (usually kDNSServiceClass_IN).
 *
 * rdlen:           The length, in bytes, of the resource record rdata.
 *
 * rdata:           The raw rdata of the resource record.
 *
 * ttl:             The resource record's time to live, in seconds.
 *
 * context:         The context pointer that was passed to the callout.
 *
 */

typedef void (DNSSD_API *DNSServiceQueryRecordReply)
    (
    DNSServiceRef                       DNSServiceRef,
    DNSServiceFlags                     flags,
    uint32_t                            interfaceIndex,
    DNSServiceErrorType                 errorCode,
    const char                          *fullname,
    uint16_t                            rrtype,
    uint16_t                            rrclass,
    uint16_t                            rdlen,
    const void                          *rdata,
    uint32_t                            ttl,
    void                                *context
    );


/* DNSServiceQueryRecord() Parameters:
 *
 * sdRef:           A pointer to an uninitialized DNSServiceRef. If the call succeeds
 *                  then it initializes the DNSServiceRef, returns kDNSServiceErr_NoError,
 *                  and the query operation will run indefinitely until the client
 *                  terminates it by passing this DNSServiceRef to DNSServiceRefDeallocate().
 *
 * flags:           Pass kDNSServiceFlagsLongLivedQuery to create a "long-lived" unicast
 *                  query in a non-local domain.  Without setting this flag, unicast queries
 *                  will be one-shot - that is, only answers available at the time of the call
 *                  will be returned.  By setting this flag, answers (including Add and Remove
 *                  events) that become available after the initial call is made will generate
 *                  callbacks.  This flag has no effect on link-local multicast queries.
 *
 * interfaceIndex:  If non-zero, specifies the interface on which to issue the query
 *                  (the index for a given interface is determined via the if_nametoindex()
 *                  family of calls.)  Passing 0 causes the name to be queried for on all
 *                  interfaces. See "Constants for specifying an interface index" for more details.
 *
 * fullname:        The full domain name of the resource record to be queried for.
 *
 * rrtype:          The numerical type of the resource record to be queried for
 *                  (e.g. kDNSServiceType_PTR, kDNSServiceType_SRV, etc)
 *
 * rrclass:         The class of the resource record (usually kDNSServiceClass_IN).
 *
 * callBack:        The function to be called when a result is found, or if the call
 *                  asynchronously fails.
 *
 * context:         An application context pointer which is passed to the callback function
 *                  (may be NULL).
 *
 * return value:    Returns kDNSServiceErr_NoError on succeses (any subsequent, asynchronous
 *                  errors are delivered to the callback), otherwise returns an error code indicating
 *                  the error that occurred (the callback is never invoked and the DNSServiceRef
 *                  is not initialized.)
 */

DNSServiceErrorType DNSSD_API DNSServiceQueryRecord
    (
    DNSServiceRef                       *sdRef,
    DNSServiceFlags                     flags,
    uint32_t                            interfaceIndex,
    const char                          *fullname,
    uint16_t                            rrtype,
    uint16_t                            rrclass,
    DNSServiceQueryRecordReply          callBack,
    void                                *context  /* may be NULL */
    );


/* DNSServiceReconfirmRecord
 *
 * Instruct the daemon to verify the validity of a resource record that appears to
 * be out of date (e.g. because tcp connection to a service's target failed.)
 * Causes the record to be flushed from the daemon's cache (as well as all other
 * daemons' caches on the network) if the record is determined to be invalid.
 *
 * Parameters:
 *
 * flags:           Currently unused, reserved for future use.
 *
 * interfaceIndex:  If non-zero, specifies the interface of the record in question.
 *                  Passing 0 causes all instances of this record to be reconfirmed.
 *
 * fullname:        The resource record's full domain name.
 *
 * rrtype:          The resource record's type (e.g. kDNSServiceType_PTR, kDNSServiceType_SRV, etc)
 *
 * rrclass:         The class of the resource record (usually kDNSServiceClass_IN).
 *
 * rdlen:           The length, in bytes, of the resource record rdata.
 *
 * rdata:           The raw rdata of the resource record.
 *
 */

DNSServiceErrorType DNSSD_API DNSServiceReconfirmRecord
    (
    DNSServiceFlags                    flags,
    uint32_t                           interfaceIndex,
    const char                         *fullname,
    uint16_t                           rrtype,
    uint16_t                           rrclass,
    uint16_t                           rdlen,
    const void                         *rdata
    );


/*********************************************************************************************
 *
 *  General Utility Functions
 *
 *********************************************************************************************/

/* DNSServiceConstructFullName()
 *
 * Concatenate a three-part domain name (as returned by the above callbacks) into a
 * properly-escaped full domain name. Note that callbacks in the above functions ALREADY ESCAPE
 * strings where necessary.
 *
 * Parameters:
 *
 * fullName:        A pointer to a buffer that where the resulting full domain name is to be written.
 *                  The buffer must be kDNSServiceMaxDomainName (1005) bytes in length to
 *                  accommodate the longest legal domain name without buffer overrun.
 *
 * service:         The service name - any dots or backslashes must NOT be escaped.
 *                  May be NULL (to construct a PTR record name, e.g.
 *                  "_ftp._tcp.apple.com.").
 *
 * regtype:         The service type followed by the protocol, separated by a dot
 *                  (e.g. "_ftp._tcp").
 *
 * domain:          The domain name, e.g. "apple.com.".  Literal dots or backslashes,
 *                  if any, must be escaped, e.g. "1st\. Floor.apple.com."
 *
 * return value:    Returns 0 on success, -1 on error.
 *
 */

int DNSSD_API DNSServiceConstructFullName
    (
    char                            *fullName,
    const char                      *service,      /* may be NULL */
    const char                      *regtype,
    const char                      *domain
    );


/*********************************************************************************************
 *
 *   TXT Record Construction Functions
 *
 *********************************************************************************************/

/*
 * A typical calling sequence for TXT record construction is something like:
 *
 * Client allocates storage for TXTRecord data (e.g. declare buffer on the stack)
 * TXTRecordCreate();
 * TXTRecordSetValue();
 * TXTRecordSetValue();
 * TXTRecordSetValue();
 * ...
 * DNSServiceRegister( ... TXTRecordGetLength(), TXTRecordGetBytesPtr() ... );
 * TXTRecordDeallocate();
 * Explicitly deallocate storage for TXTRecord data (if not allocated on the stack)
 */


/* TXTRecordRef
 *
 * Opaque internal data type.
 * Note: Represents a DNS-SD TXT record.
 */

typedef union _TXTRecordRef_t { char PrivateData[16]; char *ForceNaturalAlignment; } TXTRecordRef;


/* TXTRecordCreate()
 *
 * Creates a new empty TXTRecordRef referencing the specified storage.
 *
 * If the buffer parameter is NULL, or the specified storage size is not
 * large enough to hold a key subsequently added using TXTRecordSetValue(),
 * then additional memory will be added as needed using malloc().
 *
 * On some platforms, when memory is low, malloc() may fail. In this
 * case, TXTRecordSetValue() will return kDNSServiceErr_NoMemory, and this
 * error condition will need to be handled as appropriate by the caller.
 *
 * You can avoid the need to handle this error condition if you ensure
 * that the storage you initially provide is large enough to hold all
 * the key/value pairs that are to be added to the record.
 * The caller can precompute the exact length required for all of the
 * key/value pairs to be added, or simply provide a fixed-sized buffer
 * known in advance to be large enough.
 * A no-value (key-only) key requires  (1 + key length) bytes.
 * A key with empty value requires     (1 + key length + 1) bytes.
 * A key with non-empty value requires (1 + key length + 1 + value length).
 * For most applications, DNS-SD TXT records are generally
 * less than 100 bytes, so in most cases a simple fixed-sized
 * 256-byte buffer will be more than sufficient.
 * Recommended size limits for DNS-SD TXT Records are discussed in
 * <http://files.dns-sd.org/draft-cheshire-dnsext-dns-sd.txt>
 *
 * Note: When passing parameters to and from these TXT record APIs,
 * the key name does not include the '=' character. The '=' character
 * is the separator between the key and value in the on-the-wire
 * packet format; it is not part of either the key or the value.
 *
 * txtRecord:       A pointer to an uninitialized TXTRecordRef.
 *
 * bufferLen:       The size of the storage provided in the "buffer" parameter.
 *
 * buffer:          Optional caller-supplied storage used to hold the TXTRecord data.
 *                  This storage must remain valid for as long as
 *                  the TXTRecordRef.
 */

void DNSSD_API TXTRecordCreate
    (
    TXTRecordRef     *txtRecord,
    uint16_t         bufferLen,
    void             *buffer
    );


/* TXTRecordDeallocate()
 *
 * Releases any resources allocated in the course of preparing a TXT Record
 * using TXTRecordCreate()/TXTRecordSetValue()/TXTRecordRemoveValue().
 * Ownership of the buffer provided in TXTRecordCreate() returns to the client.
 *
 * txtRecord:           A TXTRecordRef initialized by calling TXTRecordCreate().
 *
 */

void DNSSD_API TXTRecordDeallocate
    (
    TXTRecordRef     *txtRecord
    );


/* TXTRecordSetValue()
 *
 * Adds a key (optionally with value) to a TXTRecordRef. If the "key" already
 * exists in the TXTRecordRef, then the current value will be replaced with
 * the new value.
 * Keys may exist in four states with respect to a given TXT record:
 *  - Absent (key does not appear at all)
 *  - Present with no value ("key" appears alone)
 *  - Present with empty value ("key=" appears in TXT record)
 *  - Present with non-empty value ("key=value" appears in TXT record)
 * For more details refer to "Data Syntax for DNS-SD TXT Records" in
 * <http://files.dns-sd.org/draft-cheshire-dnsext-dns-sd.txt>
 *
 * txtRecord:       A TXTRecordRef initialized by calling TXTRecordCreate().
 *
 * key:             A null-terminated string which only contains printable ASCII
 *                  values (0x20-0x7E), excluding '=' (0x3D). Keys should be
 *                  8 characters or less (not counting the terminating null).
 *
 * valueSize:       The size of the value.
 *
 * value:           Any binary value. For values that represent
 *                  textual data, UTF-8 is STRONGLY recommended.
 *                  For values that represent textual data, valueSize
 *                  should NOT include the terminating null (if any)
 *                  at the end of the string.
 *                  If NULL, then "key" will be added with no value.
 *                  If non-NULL but valueSize is zero, then "key=" will be
 *                  added with empty value.
 *
 * return value:    Returns kDNSServiceErr_NoError on success.
 *                  Returns kDNSServiceErr_Invalid if the "key" string contains
 *                  illegal characters.
 *                  Returns kDNSServiceErr_NoMemory if adding this key would
 *                  exceed the available storage.
 */

DNSServiceErrorType DNSSD_API TXTRecordSetValue
    (
    TXTRecordRef     *txtRecord,
    const char       *key,
    uint8_t          valueSize,        /* may be zero */
    const void       *value            /* may be NULL */
    );


/* TXTRecordRemoveValue()
 *
 * Removes a key from a TXTRecordRef.  The "key" must be an
 * ASCII string which exists in the TXTRecordRef.
 *
 * txtRecord:       A TXTRecordRef initialized by calling TXTRecordCreate().
 *
 * key:             A key name which exists in the TXTRecordRef.
 *
 * return value:    Returns kDNSServiceErr_NoError on success.
 *                  Returns kDNSServiceErr_NoSuchKey if the "key" does not
 *                  exist in the TXTRecordRef.
 */

DNSServiceErrorType DNSSD_API TXTRecordRemoveValue
    (
    TXTRecordRef     *txtRecord,
    const char       *key
    );


/* TXTRecordGetLength()
 *
 * Allows you to determine the length of the raw bytes within a TXTRecordRef.
 *
 * txtRecord:       A TXTRecordRef initialized by calling TXTRecordCreate().
 *
 * return value:    Returns the size of the raw bytes inside a TXTRecordRef
 *                  which you can pass directly to DNSServiceRegister() or
 *                  to DNSServiceUpdateRecord().
 *                  Returns 0 if the TXTRecordRef is empty.
 */

uint16_t DNSSD_API TXTRecordGetLength
    (
    const TXTRecordRef *txtRecord
    );


/* TXTRecordGetBytesPtr()
 *
 * Allows you to retrieve a pointer to the raw bytes within a TXTRecordRef.
 *
 * txtRecord:       A TXTRecordRef initialized by calling TXTRecordCreate().
 *
 * return value:    Returns a pointer to the raw bytes inside the TXTRecordRef
 *                  which you can pass directly to DNSServiceRegister() or
 *                  to DNSServiceUpdateRecord().
 */

const void * DNSSD_API TXTRecordGetBytesPtr
    (
    const TXTRecordRef *txtRecord
    );


/*********************************************************************************************
 *
 *   TXT Record Parsing Functions
 *
 *********************************************************************************************/

/*
 * A typical calling sequence for TXT record parsing is something like:
 *
 * Receive TXT record data in DNSServiceResolve() callback
 * if (TXTRecordContainsKey(txtLen, txtRecord, "key")) then do something
 * val1ptr = TXTRecordGetValuePtr(txtLen, txtRecord, "key1", &len1);
 * val2ptr = TXTRecordGetValuePtr(txtLen, txtRecord, "key2", &len2);
 * ...
 * bcopy(val1ptr, myval1, len1);
 * bcopy(val2ptr, myval2, len2);
 * ...
 * return;
 *
 * If you wish to retain the values after return from the DNSServiceResolve()
 * callback, then you need to copy the data to your own storage using bcopy()
 * or similar, as shown in the example above.
 *
 * If for some reason you need to parse a TXT record you built yourself
 * using the TXT record construction functions above, then you can do
 * that using TXTRecordGetLength and TXTRecordGetBytesPtr calls:
 * TXTRecordGetValue(TXTRecordGetLength(x), TXTRecordGetBytesPtr(x), key, &len);
 *
 * Most applications only fetch keys they know about from a TXT record and
 * ignore the rest.
 * However, some debugging tools wish to fetch and display all keys.
 * To do that, use the TXTRecordGetCount() and TXTRecordGetItemAtIndex() calls.
 */

/* TXTRecordContainsKey()
 *
 * Allows you to determine if a given TXT Record contains a specified key.
 *
 * txtLen:          The size of the received TXT Record.
 *
 * txtRecord:       Pointer to the received TXT Record bytes.
 *
 * key:             A null-terminated ASCII string containing the key name.
 *
 * return value:    Returns 1 if the TXT Record contains the specified key.
 *                  Otherwise, it returns 0.
 */

int DNSSD_API TXTRecordContainsKey
    (
    uint16_t         txtLen,
    const void       *txtRecord,
    const char       *key
    );


/* TXTRecordGetValuePtr()
 *
 * Allows you to retrieve the value for a given key from a TXT Record.
 *
 * txtLen:          The size of the received TXT Record
 *
 * txtRecord:       Pointer to the received TXT Record bytes.
 *
 * key:             A null-terminated ASCII string containing the key name.
 *
 * valueLen:        On output, will be set to the size of the "value" data.
 *
 * return value:    Returns NULL if the key does not exist in this TXT record,
 *                  or exists with no value (to differentiate between
 *                  these two cases use TXTRecordContainsKey()).
 *                  Returns pointer to location within TXT Record bytes
 *                  if the key exists with empty or non-empty value.
 *                  For empty value, valueLen will be zero.
 *                  For non-empty value, valueLen will be length of value data.
 */

const void * DNSSD_API TXTRecordGetValuePtr
    (
    uint16_t         txtLen,
    const void       *txtRecord,
    const char       *key,
    uint8_t          *valueLen
    );


/* TXTRecordGetCount()
 *
 * Returns the number of keys stored in the TXT Record.  The count
 * can be used with TXTRecordGetItemAtIndex() to iterate through the keys.
 *
 * txtLen:          The size of the received TXT Record.
 *
 * txtRecord:       Pointer to the received TXT Record bytes.
 *
 * return value:    Returns the total number of keys in the TXT Record.
 *
 */

uint16_t DNSSD_API TXTRecordGetCount
    (
    uint16_t         txtLen,
    const void       *txtRecord
    );


/* TXTRecordGetItemAtIndex()
 *
 * Allows you to retrieve a key name and value pointer, given an index into
 * a TXT Record.  Legal index values range from zero to TXTRecordGetCount()-1.
 * It's also possible to iterate through keys in a TXT record by simply
 * calling TXTRecordGetItemAtIndex() repeatedly, beginning with index zero
 * and increasing until TXTRecordGetItemAtIndex() returns kDNSServiceErr_Invalid.
 *
 * On return:
 * For keys with no value, *value is set to NULL and *valueLen is zero.
 * For keys with empty value, *value is non-NULL and *valueLen is zero.
 * For keys with non-empty value, *value is non-NULL and *valueLen is non-zero.
 *
 * txtLen:          The size of the received TXT Record.
 *
 * txtRecord:       Pointer to the received TXT Record bytes.
 *
 * index:           An index into the TXT Record.
 *
 * keyBufLen:       The size of the string buffer being supplied.
 *
 * key:             A string buffer used to store the key name.
 *                  On return, the buffer contains a null-terminated C string
 *                  giving the key name. DNS-SD TXT keys are usually
 *                  8 characters or less. To hold the maximum possible
 *                  key name, the buffer should be 256 bytes long.
 *
 * valueLen:        On output, will be set to the size of the "value" data.
 *
 * value:           On output, *value is set to point to location within TXT
 *                  Record bytes that holds the value data.
 *
 * return value:    Returns kDNSServiceErr_NoError on success.
 *                  Returns kDNSServiceErr_NoMemory if keyBufLen is too short.
 *                  Returns kDNSServiceErr_Invalid if index is greater than
 *                  TXTRecordGetCount()-1.
 */

DNSServiceErrorType DNSSD_API TXTRecordGetItemAtIndex
    (
    uint16_t         txtLen,
    const void       *txtRecord,
    uint16_t         index,
    uint16_t         keyBufLen,
    char             *key,
    uint8_t          *valueLen,
    const void       **value
    );

#ifdef __APPLE_API_PRIVATE

/*
 * Mac OS X specific functionality
 * 3rd party clients of this API should not depend on future support or availability of this routine
 */

/* DNSServiceSetDefaultDomainForUser()
 *
 * Set the default domain for the caller's UID.  Future browse and registration
 * calls by this user that do not specify an explicit domain will browse and
 * register in this wide-area domain in addition to .local.  In addition, this
 * domain will be returned as a Browse domain via domain enumeration calls.
 *
 *
 * Parameters:
 *
 * flags:           Pass kDNSServiceFlagsAdd to add a domain for a user.  Call without
 *                  this flag set to clear a previously added domain.
 *
 * domain:          The domain to be used for the caller's UID.
 *
 * return value:    Returns kDNSServiceErr_NoError on succeses, otherwise returns
 *                  an error code indicating the error that occurred
 */

DNSServiceErrorType DNSSD_API DNSServiceSetDefaultDomainForUser
    (
    DNSServiceFlags                    flags,
    const char                         *domain
    );

#endif //__APPLE_API_PRIVATE

// Some C compiler cleverness. We can make the compiler check certain things for us,
// and report errors at compile-time if anything is wrong. The usual way to do this would
// be to use a run-time "if" statement or the conventional run-time "assert" mechanism, but
// then you don't find out what's wrong until you run the software. This way, if the assertion
// condition is false, the array size is negative, and the complier complains immediately.

struct DNS_SD_CompileTimeAssertionChecks
	{
	char assert0[(sizeof(union _TXTRecordRef_t) == 16) ? 1 : -1];
	};

#ifdef  __cplusplus
    }
#endif

#endif  /* _DNS_SD_H */
