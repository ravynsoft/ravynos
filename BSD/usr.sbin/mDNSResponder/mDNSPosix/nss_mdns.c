/*
   NICTA Public Software Licence
   Version 1.0

   Copyright © 2004 National ICT Australia Ltd

   All rights reserved.

   By this licence, National ICT Australia Ltd (NICTA) grants permission,
   free of charge, to any person who obtains a copy of this software
   and any associated documentation files ("the Software") to use and
   deal with the Software in source code and binary forms without
   restriction, with or without modification, and to permit persons
   to whom the Software is furnished to do so, provided that the
   following conditions are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimers.
   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimers in
   the documentation and/or other materials provided with the
   distribution.
   - The name of NICTA may not be used to endorse or promote products
   derived from this Software without specific prior written permission.

   EXCEPT AS EXPRESSLY STATED IN THIS LICENCE AND TO THE FULL EXTENT
   PERMITTED BY APPLICABLE LAW, THE SOFTWARE IS PROVIDED "AS-IS" AND
   NICTA MAKES NO REPRESENTATIONS, WARRANTIES OR CONDITIONS OF ANY
   KIND, EXPRESS OR IMPLIED, INCLUDING, WITHOUT LIMITATION, ANY
   REPRESENTATIONS, WARRANTIES OR CONDITIONS REGARDING THE CONTENTS
   OR ACCURACY OF THE SOFTWARE, OR OF TITLE, MERCHANTABILITY, FITNESS
   FOR A PARTICULAR PURPOSE, NONINFRINGEMENT, THE ABSENCE OF LATENT
   OR OTHER DEFECTS, OR THE PRESENCE OR ABSENCE OF ERRORS, WHETHER OR
   NOT DISCOVERABLE.

   TO THE FULL EXTENT PERMITTED BY APPLICABLE LAW, IN NO EVENT WILL
   NICTA BE LIABLE ON ANY LEGAL THEORY (INCLUDING, WITHOUT LIMITATION,
   NEGLIGENCE) FOR ANY LOSS OR DAMAGE WHATSOEVER, INCLUDING (WITHOUT
   LIMITATION) LOSS OF PRODUCTION OR OPERATION TIME, LOSS, DAMAGE OR
   CORRUPTION OF DATA OR RECORDS; OR LOSS OF ANTICIPATED SAVINGS,
   OPPORTUNITY, REVENUE, PROFIT OR GOODWILL, OR OTHER ECONOMIC LOSS;
   OR ANY SPECIAL, INCIDENTAL, INDIRECT, CONSEQUENTIAL, PUNITIVE OR
   EXEMPLARY DAMAGES ARISING OUT OF OR IN CONNECTION WITH THIS LICENCE,
   THE SOFTWARE OR THE USE OF THE SOFTWARE, EVEN IF NICTA HAS BEEN
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

   If applicable legislation implies warranties or conditions, or
   imposes obligations or liability on NICTA in respect of the Software
   that cannot be wholly or partly excluded, restricted or modified,
   NICTA's liability is limited, to the full extent permitted by the
   applicable legislation, at its option, to:

   a. in the case of goods, any one or more of the following:
   i.   the replacement of the goods or the supply of equivalent goods;
   ii.  the repair of the goods;
   iii. the payment of the cost of replacing the goods or of acquiring
       equivalent goods;
   iv.  the payment of the cost of having the goods repaired; or
   b. in the case of services:
   i.   the supplying of the services again; or
   ii.  the payment of the cost of having the services supplied
       again.
 */

/*
    NSSwitch Implementation of mDNS interface.

    Andrew White (Andrew.White@nicta.com.au)
    May 2004
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <pthread.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/inet.h>
#define BIND_8_COMPAT 1
#include <arpa/nameser.h>

#include <dns_sd.h>


//----------
// Public functions

/*
    Count the number of dots in a name string.
 */
int
count_dots (const char * name);


/*
    Test whether a domain name is local.

    Returns
        1 if name ends with ".local" or ".local."
        0 otherwise
 */
int
islocal (const char * name);


/*
    Format an address structure as a string appropriate for DNS reverse (PTR)
    lookup, based on address type.

    Parameters
        prefixlen
            Prefix length, in bits.  When formatting, this will be rounded up
            to the nearest appropriate size.  If -1, assume maximum.
        buf
            Output buffer.  Must be long enough to hold largest possible
            output.
    Returns
        Pointer to (first character of) output buffer,
        or NULL on error.
 */
char *
format_reverse_addr (int af, const void * addr, int prefixlen, char * buf);


/*
    Format an address structure as a string appropriate for DNS reverse (PTR)
    lookup for AF_INET.  Output is in .in-addr.arpa domain.

    Parameters
        prefixlen
            Prefix length, in bits.  When formatting, this will be rounded up
            to the nearest byte (8).  If -1, assume 32.
        buf
            Output buffer.  Must be long enough to hold largest possible
            output.  For AF_INET, this is 29 characters (including null).
    Returns
        Pointer to (first character of) output buffer,
        or NULL on error.
 */
char *
format_reverse_addr_in (
    const struct in_addr * addr,
    int prefixlen,
    char * buf
    );
#define DNS_PTR_AF_INET_SIZE 29

/*
    Format an address structure as a string appropriate for DNS reverse (PTR)
    lookup for AF_INET6.  Output is in .ip6.arpa domain.

    Parameters
        prefixlen
            Prefix length, in bits.  When formatting, this will be rounded up
            to the nearest nibble (4).  If -1, assume 128.
        buf
            Output buffer.  Must be long enough to hold largest possible
            output.  For AF_INET6, this is 72 characters (including null).
    Returns
        Pointer to (first character of) output buffer,
        or NULL on error.
 */
char *
format_reverse_addr_in6 (
    const struct in6_addr * addr,
    int prefixlen,
    char * buf
    );
#define DNS_PTR_AF_INET6_SIZE 72


/*
    Compare whether the given dns name has the given domain suffix.
    A single leading '.' on the name or leading or trailing '.' on the
    domain is ignored for the purposes of the comparison.
    Multiple leading or trailing '.'s are an error.  Other DNS syntax
    errors are not checked for.  The comparison is case insensitive.

    Returns
        1 on success (match)
        0 on failure (no match)
        < 0 on error
 */
int
cmp_dns_suffix (const char * name, const char * domain);
enum
{
    CMP_DNS_SUFFIX_SUCCESS = 1,
    CMP_DNS_SUFFIX_FAILURE = 0,
    CMP_DNS_SUFFIX_BAD_NAME = 1,
    CMP_DNS_SUFFIX_BAD_DOMAIN = -2
};

typedef int ns_type_t;
typedef int ns_class_t;

/*
    Convert a DNS resource record (RR) code to an address family (AF) code.

    Parameters
        rrtype
            resource record type (from nameser.h)

    Returns
        Appropriate AF code (from socket.h), or AF_UNSPEC if an appropriate
        mapping couldn't be determined
 */
int
rr_to_af (ns_type_t rrtype);


/*
    Convert an address family (AF) code to a DNS resource record (RR) code.

    Parameters
        int
            address family code (from socket.h)
    Returns
        Appropriate RR code (from nameser.h), or ns_t_invalid if an appropriate
        mapping couldn't be determined
 */
ns_type_t
af_to_rr (int af);


/*
    Convert a string to an address family (case insensitive).

    Returns
        Matching AF code, or AF_UNSPEC if no match found.
 */
int
str_to_af (const char * str);


/*
    Convert a string to an ns_class_t (case insensitive).

    Returns
        Matching ns_class_t, or ns_c_invalid if no match found.
 */
ns_class_t
str_to_ns_class (const char * str);


/*
    Convert a string to an ns_type_t (case insensitive).

    Returns
        Matching ns_type_t, or ns_t_invalid if no match found.
 */
ns_type_t
str_to_ns_type (const char * str);


/*
    Convert an address family code to a string.

    Returns
        String representation of AF,
        or NULL if address family unrecognised or invalid.
 */
const char *
af_to_str (int in);


/*
    Convert an ns_class_t code to a string.

    Returns
        String representation of ns_class_t,
        or NULL if ns_class_t unrecognised or invalid.
 */
const char *
ns_class_to_str (ns_class_t in);


/*
    Convert an ns_type_t code to a string.

    Returns
        String representation of ns_type_t,
        or NULL if ns_type_t unrecognised or invalid.
 */
const char *
ns_type_to_str (ns_type_t in);


/*
    Convert DNS rdata in label format (RFC1034, RFC1035) to a name.

    On error, partial data is written to name (as much as was successfully
    processed) and an error code is returned.  Errors include a name too
    long for the buffer and a pointer in the label (which cannot be
    resolved).

    Parameters
        rdata
            Rdata formatted as series of labels.
        rdlen
            Length of rdata buffer.
        name
            Buffer to store fully qualified result in.
            By RFC1034 section 3.1, a 255 character buffer (256 characters
            including null) is long enough for any legal name.
        name_len
            Number of characters available in name buffer, not including
            trailing null.

    Returns
        Length of name buffer (not including trailing null).
        < 0 on error.
        A return of 0 implies the empty domain.
 */
static int
dns_rdata_to_name (const unsigned char * rdata, int rdlen, char * name, unsigned int name_len);
enum
{
    DNS_RDATA_TO_NAME_BAD_FORMAT = -1,
    // Format is broken.  Usually because we ran out of data
    // (according to rdata) before the labels said we should.
    DNS_RDATA_TO_NAME_TOO_LONG = -2,
    // The converted rdata is longer than the name buffer.
    DNS_RDATA_TO_NAME_PTR = -3,
    // The rdata contains a pointer.
};

#define DNS_LABEL_MAXLEN 63
// Maximum length of a single DNS label
#define DNS_NAME_MAXLEN 256
// Maximum length of a DNS name

//----------
// Public types

typedef int errcode_t;
// Used for 0 = success, non-zero = error code functions


//----------
// Public functions

/*
    Test whether a domain name is in a domain covered by nss_mdns.
    The name is assumed to be fully qualified (trailing dot optional);
    unqualified names will be processed but may return unusual results
    if the unqualified prefix happens to match a domain suffix.

    Returns
         1 success
         0 failure
        -1 error, check errno
 */
int
config_is_mdns_suffix (const char * name);


/*
    Loads all relevant data from configuration file.  Other code should
    rarely need to call this function, since all other public configuration
    functions do so implicitly.  Once loaded, configuration info doesn't
    change.

    Returns
        0 configuration ready
        non-zero configuration error code
 */
errcode_t
init_config ();

#define ENTNAME  hostent
#define DATABASE "hosts"

#include <nss.h>
// For nss_status
#include <netdb.h>
// For hostent
#include <sys/types.h>
// For size_t

typedef enum nss_status nss_status;
typedef struct hostent hostent;

/*
   gethostbyname implementation

    name:
        name to look up
    result_buf:
        resulting entry
    buf:
        auxillary buffer
    buflen:
        length of auxillary buffer
    errnop:
        pointer to errno
    h_errnop:
        pointer to h_errno
 */
nss_status
_nss_mdns_gethostbyname_r (
    const char *name,
    hostent * result_buf,
    char *buf,
    size_t buflen,
    int *errnop,
    int *h_errnop
    );


/*
   gethostbyname2 implementation

    name:
        name to look up
    af:
        address family
    result_buf:
        resulting entry
    buf:
        auxillary buffer
    buflen:
        length of auxillary buffer
    errnop:
        pointer to errno
    h_errnop:
        pointer to h_errno
 */
nss_status
_nss_mdns_gethostbyname2_r (
    const char *name,
    int af,
    hostent * result_buf,
    char *buf,
    size_t buflen,
    int *errnop,
    int *h_errnop
    );


/*
   gethostbyaddr implementation

    addr:
        address structure to look up
    len:
        length of address structure
    af:
        address family
    result_buf:
        resulting entry
    buf:
        auxillary buffer
    buflen:
        length of auxillary buffer
    errnop:
        pointer to errno
    h_errnop:
        pointer to h_errno
 */
nss_status
_nss_mdns_gethostbyaddr_r (
    const void *addr,
    socklen_t len,
    int af,
    hostent * result_buf,
    char *buf,
    size_t buflen,
    int *errnop,
    int *h_errnop
    );


//----------
// Types and Constants

const int MDNS_VERBOSE = 0;
// This enables verbose syslog messages
// If zero, only "imporant" messages will appear in syslog

#define k_hostname_maxlen 256
// As per RFC1034 and RFC1035
#define k_aliases_max 15
#define k_addrs_max 15

typedef struct buf_header
{
    char hostname [k_hostname_maxlen + 1];
    char * aliases [k_aliases_max + 1];
    char * addrs [k_addrs_max + 1];
} buf_header_t;

typedef struct result_map
{
    int done;
    nss_status status;
    hostent * hostent;
    buf_header_t * header;
    int aliases_count;
    int addrs_count;
    char * buffer;
    int addr_idx;
    // Index for addresses - grow from low end
    // Index points to first empty space
    int alias_idx;
    // Index for aliases - grow from high end
    // Index points to lowest entry
    int r_errno;
    int r_h_errno;
} result_map_t;

static const struct timeval
k_select_time = { 0, 500000 };
// 0 seconds, 500 milliseconds

//----------
// Local prototypes

static nss_status
mdns_gethostbyname2 (
    const char *name,
    int af,
    hostent * result_buf,
    char *buf,
    size_t buflen,
    int *errnop,
    int *h_errnop
    );


/*
    Lookup name using mDNS server
 */
static nss_status
mdns_lookup_name (
    const char * fullname,
    int af,
    result_map_t * result
    );

/*
    Lookup address using mDNS server
 */
static nss_status
mdns_lookup_addr (
    const void * addr,
    socklen_t len,
    int af,
    const char * addr_str,
    result_map_t * result
    );


/*
    Handle incoming MDNS events
 */
static nss_status
handle_events (DNSServiceRef sdref, result_map_t * result, const char * str);


// Callback for mdns_lookup operations
//DNSServiceQueryRecordReply mdns_lookup_callback;
typedef void
mdns_lookup_callback_t
(
    DNSServiceRef sdref,
    DNSServiceFlags flags,
    uint32_t interface_index,
    DNSServiceErrorType error_code,
    const char          *fullname,
    uint16_t rrtype,
    uint16_t rrclass,
    uint16_t rdlen,
    const void          *rdata,
    uint32_t ttl,
    void                *context
);

mdns_lookup_callback_t mdns_lookup_callback;


static int
init_result (
    result_map_t * result,
    hostent * result_buf,
    char * buf,
    size_t buflen
    );

static int
callback_body_ptr (
    const char * fullname,
    result_map_t * result,
    int rdlen,
    const void * rdata
    );

static void *
add_address_to_buffer (result_map_t * result, const void * data, int len);
static char *
add_alias_to_buffer (result_map_t * result, const char * data, int len);
static char *
add_hostname_len (result_map_t * result, const char * fullname, int len);
static char *
add_hostname_or_alias (result_map_t * result, const char * data, int len);

static void *
contains_address (result_map_t * result, const void * data, int len);
static char *
contains_alias (result_map_t * result, const char * data);


static const char *
is_applicable_name (
    result_map_t * result,
    const char * name,
    char * lookup_name
    );

static const char *
is_applicable_addr (
    result_map_t * result,
    const void * addr,
    int af,
    char * addr_str
    );


// Error code functions

static nss_status
set_err (result_map_t * result, nss_status status, int err, int herr);

static nss_status set_err_notfound (result_map_t * result);
static nss_status set_err_bad_hostname (result_map_t * result);
static nss_status set_err_buf_too_small (result_map_t * result);
static nss_status set_err_internal_resource_full (result_map_t * result);
static nss_status set_err_system (result_map_t * result);
static nss_status set_err_mdns_failed (result_map_t * result);
static nss_status set_err_success (result_map_t * result);


//----------
// Global variables


//----------
// NSS functions

nss_status
_nss_mdns_gethostbyname_r (
    const char *name,
    hostent * result_buf,
    char *buf,
    size_t buflen,
    int *errnop,
    int *h_errnop
    )
{
    if (MDNS_VERBOSE)
        syslog (LOG_DEBUG,
                "mdns: Called nss_mdns_gethostbyname with %s",
                name
                );

    return
        mdns_gethostbyname2 (
            name, AF_INET, result_buf, buf, buflen, errnop, h_errnop
            );
}


nss_status
_nss_mdns_gethostbyname2_r (
    const char *name,
    int af,
    hostent * result_buf,
    char *buf,
    size_t buflen,
    int *errnop,
    int *h_errnop
    )
{
    if (MDNS_VERBOSE)
        syslog (LOG_DEBUG,
                "mdns: Called nss_mdns_gethostbyname2 with %s",
                name
                );

    return
        mdns_gethostbyname2 (
            name, af, result_buf, buf, buflen, errnop, h_errnop
            );
}


nss_status
_nss_mdns_gethostbyaddr_r (
    const void *addr,
    socklen_t len,
    int af,
    hostent * result_buf,
    char *buf,
    size_t buflen,
    int *errnop,
    int *h_errnop
    )
{
    char addr_str [NI_MAXHOST + 1];
    result_map_t result;
    int err_status;

    if (inet_ntop (af, addr, addr_str, NI_MAXHOST) == NULL)
    {
        const char * family = af_to_str (af);
        if (family == NULL)
        {
            family = "Unknown";
        }

        syslog (LOG_WARNING,
                "mdns: Couldn't covert address, family %d (%s) in nss_mdns_gethostbyaddr: %s",
                af,
                family,
                strerror (errno)
                );

        // This address family never applicable to us, so return NOT_FOUND

        *errnop = ENOENT;
        *h_errnop = HOST_NOT_FOUND;
        return NSS_STATUS_NOTFOUND;
    }
    if (MDNS_VERBOSE)
    {
        syslog (LOG_DEBUG,
                "mdns: Called nss_mdns_gethostbyaddr with %s",
                addr_str
                );
    }

    // Initialise result
    err_status = init_result (&result, result_buf, buf, buflen);
    if (err_status)
    {
        *errnop = err_status;
        *h_errnop = NETDB_INTERNAL;
        return NSS_STATUS_TRYAGAIN;
    }

    if (is_applicable_addr (&result, addr, af, addr_str))
    {
        nss_status rv;

        rv = mdns_lookup_addr (addr, len, af, addr_str, &result);
        if (rv == NSS_STATUS_SUCCESS)
        {
            return rv;
        }
    }

    // Return current error status (defaults to NOT_FOUND)

    *errnop = result.r_errno;
    *h_errnop = result.r_h_errno;
    return result.status;
}


//----------
// Local functions

nss_status
mdns_gethostbyname2 (
    const char *name,
    int af,
    hostent * result_buf,
    char *buf,
    size_t buflen,
    int *errnop,
    int *h_errnop
    )
{
    char lookup_name [k_hostname_maxlen + 1];
    result_map_t result;
    int err_status;

    // Initialise result
    err_status = init_result (&result, result_buf, buf, buflen);
    if (err_status)
    {
        *errnop = err_status;
        *h_errnop = NETDB_INTERNAL;
        return NSS_STATUS_TRYAGAIN;
    }

    if (is_applicable_name (&result, name, lookup_name))
    {
        // Try using mdns
        nss_status rv;

        if (MDNS_VERBOSE)
            syslog (LOG_DEBUG,
                    "mdns: Local name: %s",
                    name
                    );

        rv = mdns_lookup_name (name, af, &result);
        if (rv == NSS_STATUS_SUCCESS)
        {
            return rv;
        }
    }

    // Return current error status (defaults to NOT_FOUND)

    *errnop = result.r_errno;
    *h_errnop = result.r_h_errno;
    return result.status;
}


/*
    Lookup a fully qualified hostname using the default record type
    for the specified address family.

    Parameters
        fullname
            Fully qualified hostname.  If not fully qualified the code will
            still 'work', but the lookup is unlikely to succeed.
        af
            Either AF_INET or AF_INET6.  Other families are not supported.
        result
            Initialised 'result' data structure.
 */
static nss_status
mdns_lookup_name (
    const char * fullname,
    int af,
    result_map_t * result
    )
{
    // Lookup using mDNS.
    DNSServiceErrorType errcode;
    DNSServiceRef sdref;
    ns_type_t rrtype;
    nss_status status;

    if (MDNS_VERBOSE)
        syslog (LOG_DEBUG,
                "mdns: Attempting lookup of %s",
                fullname
                );

    switch (af)
    {
    case AF_INET:
        rrtype = kDNSServiceType_A;
        result->hostent->h_length = 4;
        // Length of an A record
        break;

    case AF_INET6:
        rrtype = kDNSServiceType_AAAA;
        result->hostent->h_length = 16;
        // Length of an AAAA record
        break;

    default:
        syslog (LOG_WARNING,
                "mdns: Unsupported address family %d",
                af
                );
        return set_err_bad_hostname (result);
    }
    result->hostent->h_addrtype = af;

    errcode =
        DNSServiceQueryRecord (
            &sdref,
            kDNSServiceFlagsForceMulticast,     // force multicast query
            kDNSServiceInterfaceIndexAny,   // all interfaces
            fullname,   // full name to query for
            rrtype,     // resource record type
            kDNSServiceClass_IN,    // internet class records
            mdns_lookup_callback,   // callback
            result      // Context - result buffer
            );

    if (errcode)
    {
        syslog (LOG_WARNING,
                "mdns: Failed to initialise lookup, error %d",
                errcode
                );
        return set_err_mdns_failed (result);
    }

    status = handle_events (sdref, result, fullname);
    DNSServiceRefDeallocate (sdref);
    return status;
}


/*
    Reverse (PTR) lookup for the specified address.

    Parameters
        addr
            Either a struct in_addr or a struct in6_addr
        addr_len
            size of the address
        af
            Either AF_INET or AF_INET6.  Other families are not supported.
            Must match addr
        addr_str
            Address in format suitable for PTR lookup.
            AF_INET: a.b.c.d -> d.c.b.a.in-addr.arpa
            AF_INET6: reverse nibble format, x.x.x...x.ip6.arpa
        result
            Initialised 'result' data structure.
 */
static nss_status
mdns_lookup_addr (
    const void * addr,
    socklen_t addr_len,
    int af,
    const char * addr_str,
    result_map_t * result
    )
{
    DNSServiceErrorType errcode;
    DNSServiceRef sdref;
    nss_status status;

    if (MDNS_VERBOSE)
        syslog (LOG_DEBUG,
                "mdns: Attempting lookup of %s",
                addr_str
                );

    result->hostent->h_addrtype = af;
    result->hostent->h_length = addr_len;

    // Query address becomes "address" in result.
    if (!add_address_to_buffer (result, addr, addr_len))
    {
        return result->status;
    }

    result->hostent->h_name [0] = 0;

    errcode =
        DNSServiceQueryRecord (
            &sdref,
            kDNSServiceFlagsForceMulticast,     // force multicast query
            kDNSServiceInterfaceIndexAny,   // all interfaces
            addr_str,   // address string to query for
            kDNSServiceType_PTR,    // pointer RRs
            kDNSServiceClass_IN,    // internet class records
            mdns_lookup_callback,   // callback
            result      // Context - result buffer
            );

    if (errcode)
    {
        syslog (LOG_WARNING,
                "mdns: Failed to initialise mdns lookup, error %d",
                errcode
                );
        return set_err_mdns_failed (result);
    }

    status = handle_events (sdref, result, addr_str);
    DNSServiceRefDeallocate (sdref);
    return status;
}


/*
    Wait on result of callback, and process it when it arrives.

    Parameters
        sdref
            dns-sd reference
        result
            Initialised 'result' data structure.
        str
            lookup string, used for status/error reporting.
 */
static nss_status
handle_events (DNSServiceRef sdref, result_map_t * result, const char * str)
{
    int dns_sd_fd = DNSServiceRefSockFD(sdref);
    int nfds = dns_sd_fd + 1;
    fd_set readfds;
    struct timeval tv;
    int select_result;

    while (!result->done)
    {
        FD_ZERO(&readfds);
        FD_SET(dns_sd_fd, &readfds);

        tv = k_select_time;

        select_result =
            select (nfds, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);
        if (select_result > 0)
        {
            if (FD_ISSET(dns_sd_fd, &readfds))
            {
                if (MDNS_VERBOSE)
                    syslog (LOG_DEBUG,
                            "mdns: Reply received for %s",
                            str
                            );
                DNSServiceProcessResult(sdref);
            }
            else
            {
                syslog (LOG_WARNING,
                        "mdns: Unexpected return from select on lookup of %s",
                        str
                        );
            }
        }
        else
        {
            // Terminate loop due to timer expiry
            if (MDNS_VERBOSE)
                syslog (LOG_DEBUG,
                        "mdns: %s not found - timer expired",
                        str
                        );
            set_err_notfound (result);
            break;
        }
    }

    return result->status;
}


/*
    Examine incoming data and add to relevant fields in result structure.
    This routine is called from DNSServiceProcessResult where appropriate.
 */
void
mdns_lookup_callback
(
    DNSServiceRef sdref,
    DNSServiceFlags flags,
    uint32_t interface_index,
    DNSServiceErrorType error_code,
    const char          *fullname,
    uint16_t rrtype,
    uint16_t rrclass,
    uint16_t rdlen,
    const void          *rdata,
    uint32_t ttl,
    void                *context
)
{
    // A single record is received

    result_map_t * result = (result_map_t *) context;

    (void)sdref; // Unused
    (void)interface_index; // Unused
    (void)ttl; // Unused

    if (!(flags & kDNSServiceFlagsMoreComing) )
    {
        result->done = 1;
    }

    if (error_code == kDNSServiceErr_NoError)
    {
        ns_type_t expected_rr_type =
            af_to_rr (result->hostent->h_addrtype);

        // Idiot check class
        if (rrclass != C_IN)
        {
            syslog (LOG_WARNING,
                    "mdns: Received bad RR class: expected %d (%s),"
                    " got %d (%s), RR type %d (%s)",
                    C_IN,
                    ns_class_to_str (C_IN),
                    rrclass,
                    ns_class_to_str (rrclass),
                    rrtype,
                    ns_type_to_str (rrtype)
                    );
            return;
        }

        // If a PTR
        if (rrtype == kDNSServiceType_PTR)
        {
            if (callback_body_ptr (fullname, result, rdlen, rdata) < 0)
                return;
        }
        else if (rrtype == expected_rr_type)
        {
            if (!
                add_hostname_or_alias (
                    result,
                    fullname,
                    strlen (fullname)
                    )
                )
            {
                result->done = 1;
                return;
                // Abort on error
            }

            if (!add_address_to_buffer (result, rdata, rdlen) )
            {
                result->done = 1;
                return;
                // Abort on error
            }
        }
        else
        {
            syslog (LOG_WARNING,
                    "mdns: Received bad RR type: expected %d (%s),"
                    " got %d (%s)",
                    expected_rr_type,
                    ns_type_to_str (expected_rr_type),
                    rrtype,
                    ns_type_to_str (rrtype)
                    );
            return;
        }

        if (result->status != NSS_STATUS_SUCCESS)
            set_err_success (result);
    }
    else
    {
        // For now, dump message to syslog and continue
        syslog (LOG_WARNING,
                "mdns: callback returned error %d",
                error_code
                );
    }
}

static int
callback_body_ptr (
    const char * fullname,
    result_map_t * result,
    int rdlen,
    const void * rdata
    )
{
    char result_name [k_hostname_maxlen + 1];
    int rv;

    // Fullname should be .in-addr.arpa or equivalent, which we're
    // not interested in.  Ignore it.

    rv = dns_rdata_to_name (rdata, rdlen, result_name, k_hostname_maxlen);
    if (rv < 0)
    {
        const char * errmsg;

        switch (rv)
        {
        case DNS_RDATA_TO_NAME_BAD_FORMAT:
            errmsg = "mdns: PTR '%s' result badly formatted ('%s...')";
            break;

        case DNS_RDATA_TO_NAME_TOO_LONG:
            errmsg = "mdns: PTR '%s' result too long ('%s...')";
            break;

        case DNS_RDATA_TO_NAME_PTR:
            errmsg = "mdns: PTR '%s' result contained pointer ('%s...')";
            break;

        default:
            errmsg = "mdns: PTR '%s' result conversion failed ('%s...')";
        }

        syslog (LOG_WARNING,
                errmsg,
                fullname,
                result_name
                );

        return -1;
    }

    if (MDNS_VERBOSE)
    {
        syslog (LOG_DEBUG,
                "mdns: PTR '%s' resolved to '%s'",
                fullname,
                result_name
                );
    }

    // Data should be a hostname
    if (!
        add_hostname_or_alias (
            result,
            result_name,
            rv
            )
        )
    {
        result->done = 1;
        return -1;
    }

    return 0;
}


/*
    Add an address to the buffer.

    Parameter
        result
            Result structure to write to
        data
            Incoming address data buffer
            Must be 'int' aligned
        len
            Length of data buffer (in bytes)
            Must match data alignment

    Result
        Pointer to start of newly written data,
        or NULL on error.
        If address already exists in buffer, returns pointer to that instead.
 */
static void *
add_address_to_buffer (result_map_t * result, const void * data, int len)
{
    int new_addr;
    void * start;
    void * temp;

    if ((temp = contains_address (result, data, len)))
    {
        return temp;
    }

    if (result->addrs_count >= k_addrs_max)
    {
        // Not enough addr slots
        set_err_internal_resource_full (result);
        syslog (LOG_ERR,
                "mdns: Internal address buffer full; increase size"
                );
        return NULL;
    }

    // Idiot check
    if (len != result->hostent->h_length)
    {
        syslog (LOG_WARNING,
                "mdns: Unexpected rdata length for address.  Expected %d, got %d",
                result->hostent->h_length,
                len
                );
        // XXX And continue for now.
    }

    new_addr = result->addr_idx + len;

    if (new_addr > result->alias_idx)
    {
        // Not enough room
        set_err_buf_too_small (result);
        if (MDNS_VERBOSE)
            syslog (LOG_DEBUG,
                    "mdns: Ran out of buffer when adding address %d",
                    result->addrs_count + 1
                    );
        return NULL;
    }

    start = result->buffer + result->addr_idx;
    memcpy (start, data, len);
    result->addr_idx = new_addr;
    result->header->addrs [result->addrs_count] = start;
    result->addrs_count++;
    result->header->addrs [result->addrs_count] = NULL;

    return start;
}


static void *
contains_address (result_map_t * result, const void * data, int len)
{
    int i;

    // Idiot check
    if (len != result->hostent->h_length)
    {
        syslog (LOG_WARNING,
                "mdns: Unexpected rdata length for address.  Expected %d, got %d",
                result->hostent->h_length,
                len
                );
        // XXX And continue for now.
    }

    for (i = 0; result->header->addrs [i]; i++)
    {
        if (memcmp (result->header->addrs [i], data, len) == 0)
        {
            return result->header->addrs [i];
        }
    }

    return NULL;
}


/*
    Add an alias to the buffer.

    Parameter
        result
            Result structure to write to
        data
            Incoming alias (null terminated)
        len
            Length of data buffer (in bytes), including trailing null

    Result
        Pointer to start of newly written data,
        or NULL on error
        If alias already exists in buffer, returns pointer to that instead.
 */
static char *
add_alias_to_buffer (result_map_t * result, const char * data, int len)
{
    int new_alias;
    char * start;
    char * temp;

    if ((temp = contains_alias (result, data)))
    {
        return temp;
    }

    if (result->aliases_count >= k_aliases_max)
    {
        // Not enough alias slots
        set_err_internal_resource_full (result);
        syslog (LOG_ERR,
                "mdns: Internal alias buffer full; increase size"
                );
        return NULL;
    }

    new_alias = result->alias_idx - len;

    if (new_alias < result->addr_idx)
    {
        // Not enough room
        set_err_buf_too_small (result);
        if (MDNS_VERBOSE)
            syslog (LOG_DEBUG,
                    "mdns: Ran out of buffer when adding alias %d",
                    result->aliases_count + 1
                    );
        return NULL;
    }

    start = result->buffer + new_alias;
    memcpy (start, data, len);
    result->alias_idx = new_alias;
    result->header->aliases [result->aliases_count] = start;
    result->aliases_count++;
    result->header->aliases [result->aliases_count] = NULL;

    return start;
}


static char *
contains_alias (result_map_t * result, const char * alias)
{
    int i;

    for (i = 0; result->header->aliases [i]; i++)
    {
        if (strcmp (result->header->aliases [i], alias) == 0)
        {
            return result->header->aliases [i];
        }
    }

    return NULL;
}


/*
    Add fully qualified hostname to result.

    Parameter
        result
            Result structure to write to
        fullname
            Fully qualified hostname

    Result
        Pointer to start of hostname buffer,
        or NULL on error (usually hostname too long)
 */

static char *
add_hostname_len (result_map_t * result, const char * fullname, int len)
{
    if (len >= k_hostname_maxlen)
    {
        set_err_bad_hostname (result);
        syslog (LOG_WARNING,
                "mdns: Hostname too long '%.*s': len %d, max %d",
                len,
                fullname,
                len,
                k_hostname_maxlen
                );
        return NULL;
    }

    result->hostent->h_name =
        strcpy (result->header->hostname, fullname);

    return result->header->hostname;
}


/*
    Add fully qualified name as hostname or alias.

    If hostname is not fully qualified this is not an error, but the data
    returned may be not what the application wanted.

    Parameter
        result
            Result structure to write to
        data
            Incoming alias (null terminated)
        len
            Length of data buffer (in bytes), including trailing null

    Result
        Pointer to start of newly written data,
        or NULL on error
        If alias or hostname already exists, returns pointer to that instead.
 */
static char *
add_hostname_or_alias (result_map_t * result, const char * data, int len)
{
    char * hostname = result->hostent->h_name;

    if (*hostname)
    {
        if (strcmp (hostname, data) == 0)
        {
            return hostname;
        }
        else
        {
            return add_alias_to_buffer (result, data, len);
        }
    }
    else
    {
        return add_hostname_len (result, data, len);
    }
}


static int
init_result (
    result_map_t * result,
    hostent * result_buf,
    char * buf,
    size_t buflen
    )
{
    if (buflen < sizeof (buf_header_t))
    {
        return ERANGE;
    }

    result->hostent = result_buf;
    result->header = (buf_header_t *) buf;
    result->header->hostname[0] = 0;
    result->aliases_count = 0;
    result->header->aliases[0] = NULL;
    result->addrs_count = 0;
    result->header->addrs[0] = NULL;
    result->buffer = buf + sizeof (buf_header_t);
    result->addr_idx = 0;
    result->alias_idx = buflen - sizeof (buf_header_t);
    result->done = 0;
    set_err_notfound (result);

    // Point hostent to the right buffers
    result->hostent->h_name = result->header->hostname;
    result->hostent->h_aliases = result->header->aliases;
    result->hostent->h_addr_list = result->header->addrs;

    return 0;
}

/*
    Set the status in the result.

    Parameters
        result
            Result structure to update
        status
            New nss_status value
        err
            New errno value
        herr
            New h_errno value

    Returns
        New status value
 */
static nss_status
set_err (result_map_t * result, nss_status status, int err, int herr)
{
    result->status = status;
    result->r_errno = err;
    result->r_h_errno = herr;

    return status;
}

static nss_status
set_err_notfound (result_map_t * result)
{
    return set_err (result, NSS_STATUS_NOTFOUND, ENOENT, HOST_NOT_FOUND);
}

static nss_status
set_err_bad_hostname (result_map_t * result)
{
    return set_err (result, NSS_STATUS_TRYAGAIN, ENOENT, NO_RECOVERY);
}

static nss_status
set_err_buf_too_small (result_map_t * result)
{
    return set_err (result, NSS_STATUS_TRYAGAIN, ERANGE, NETDB_INTERNAL);
}

static nss_status
set_err_internal_resource_full (result_map_t * result)
{
    return set_err (result, NSS_STATUS_RETURN, ERANGE, NO_RECOVERY);
}

static nss_status
set_err_system (result_map_t * result)
{
    return set_err (result, NSS_STATUS_UNAVAIL, errno, NETDB_INTERNAL);
}

static nss_status
set_err_mdns_failed (result_map_t * result)
{
    return set_err (result, NSS_STATUS_TRYAGAIN, EAGAIN, TRY_AGAIN);
}

static nss_status
set_err_success (result_map_t * result)
{
    result->status = NSS_STATUS_SUCCESS;
    return result->status;
}


/*
    Test whether name is applicable for mdns to process, and if so copy into
    lookup_name buffer (if non-NULL).

    Returns
        Pointer to name to lookup up, if applicable, or NULL otherwise.
 */
static const char *
is_applicable_name (
    result_map_t * result,
    const char * name,
    char * lookup_name
    )
{
    int match = config_is_mdns_suffix (name);
    if (match > 0)
    {
        if (lookup_name)
        {
            strncpy (lookup_name, name, k_hostname_maxlen + 1);
            return lookup_name;
        }
        else
        {
            return name;
        }
    }
    else
    {
        if (match < 0)
        {
            set_err_system (result);
        }
        return NULL;
    }
}

/*
    Test whether address is applicable for mdns to process, and if so copy into
    addr_str buffer as an address suitable for ptr lookup.

    Returns
        Pointer to name to lookup up, if applicable, or NULL otherwise.
 */
static const char *
is_applicable_addr (
    result_map_t * result,
    const void * addr,
    int af,
    char * addr_str
    )
{
    int match;

    if (!format_reverse_addr (af, addr, -1, addr_str))
    {
        if (MDNS_VERBOSE)
            syslog (LOG_DEBUG,
                    "mdns: Failed to create reverse address"
                    );
        return NULL;
    }

    if (MDNS_VERBOSE)
        syslog (LOG_DEBUG,
                "mdns: Reverse address: %s",
                addr_str
                );

    match = config_is_mdns_suffix (addr_str);
    if (match > 0)
    {
        return addr_str;
    }
    else
    {
        if (match < 0)
        {
            set_err_system (result);
        }
        return NULL;
    }
}

//----------
// Types and Constants

const char * k_conf_file = "/etc/nss_mdns.conf";
#define CONF_LINE_SIZE 1024

const char k_comment_char = '#';

const char * k_keyword_domain = "domain";

const char * k_default_domains [] =
{
    "local",
    "254.169.in-addr.arpa",
    "8.e.f.ip6.int",
    "8.e.f.ip6.arpa",
    "9.e.f.ip6.int",
    "9.e.f.ip6.arpa",
    "a.e.f.ip6.int",
    "a.e.f.ip6.arpa",
    "b.e.f.ip6.int",
    "b.e.f.ip6.arpa",
    NULL
    // Always null terminated
};

// Linked list of domains
typedef struct domain_entry
{
    char * domain;
    struct domain_entry * next;
} domain_entry_t;


// Config
typedef struct
{
    domain_entry_t * domains;
} config_t;

const config_t k_empty_config =
{
    NULL
};


// Context - tracks position in config file, used for error reporting
typedef struct
{
    const char * filename;
    int linenum;
} config_file_context_t;


//----------
// Local prototypes

static errcode_t
load_config (config_t * conf);

static errcode_t
process_config_line (
    config_t * conf,
    char * line,
    config_file_context_t * context
    );

static char *
get_next_word (char * input, char **next);

static errcode_t
default_config (config_t * conf);

static errcode_t
add_domain (config_t * conf, const char * domain);

static int
contains_domain (const config_t * conf, const char * domain);

static int
contains_domain_suffix (const config_t * conf, const char * addr);


//----------
// Global variables

static config_t * g_config = NULL;
// Configuration info

pthread_mutex_t g_config_mutex =
#ifdef PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP
    PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
#else
    PTHREAD_MUTEX_INITIALIZER;
#endif


//----------
// Configuration functions


/*
    Initialise the configuration from the config file.

    Returns
        0 success
        non-zero error code on failure
 */
errcode_t
init_config ()
{
    if (g_config)
    {
        /*
            Safe to test outside mutex.
            If non-zero, initialisation is complete and g_config can be
            safely used read-only.  If zero, then we do proper mutex
            testing before initialisation.
         */
        return 0;
    }
    else
    {
        int errcode = -1;
        int presult;
        config_t * temp_config;

        // Acquire mutex
        presult = pthread_mutex_lock (&g_config_mutex);
        if (presult)
        {
            syslog (LOG_ERR,
                    "mdns: Fatal mutex lock error in nss_mdns:init_config, %s:%d: %d: %s",
                    __FILE__, __LINE__, presult, strerror (presult)
                    );
            return presult;
        }

        // Test again now we have mutex, in case initialisation occurred while
        // we were waiting
        if (!g_config)
        {
            temp_config = (config_t *) malloc (sizeof (config_t));
            if (temp_config)
            {
                // Note: This code will leak memory if initialisation fails
                // repeatedly.  This should only happen in the case of a memory
                // error, so I'm not sure if it's a meaningful problem. - AW
                *temp_config = k_empty_config;
                errcode = load_config (temp_config);

                if (!errcode)
                {
                    g_config = temp_config;
                }
            }
            else
            {
                syslog (LOG_ERR,
                        "mdns: Can't allocate memory in nss_mdns:init_config, %s:%d",
                        __FILE__, __LINE__
                        );
                errcode = errno;
            }
        }

        presult = pthread_mutex_unlock (&g_config_mutex);
        if (presult)
        {
            syslog (LOG_ERR,
                    "mdns: Fatal mutex unlock error in nss_mdns:init_config, %s:%d: %d: %s",
                    __FILE__, __LINE__, presult, strerror (presult)
                    );
            errcode = presult;
        }

        return errcode;
    }
}


int
config_is_mdns_suffix (const char * name)
{
    int errcode = init_config ();
    if (!errcode)
    {
        return contains_domain_suffix (g_config, name);
    }
    else
    {
        errno = errcode;
        return -1;
    }
}


//----------
// Local functions

static errcode_t
load_config (config_t * conf)
{
    FILE * cf;
    char line [CONF_LINE_SIZE];
    config_file_context_t context;

    context.filename = k_conf_file;
    context.linenum = 0;


    cf = fopen (context.filename, "r");
    if (!cf)
    {
        syslog (LOG_INFO,
                "mdns: Couldn't open nss_mdns configuration file %s, using default.",
                context.filename
                );
        return default_config (conf);
    }

    while (fgets (line, CONF_LINE_SIZE, cf))
    {
        int errcode;
        context.linenum++;
        errcode = process_config_line (conf, line, &context);
        if (errcode)
        {
            // Critical error, give up
            fclose(cf);
            return errcode;
        }
    }

    fclose (cf);

    return 0;
}


/*
    Parse a line of the configuration file.
    For each keyword recognised, perform appropriate handling.
    If the keyword is not recognised, print a message to syslog
    and continue.

    Returns
        0 success, or recoverable config file error
        non-zero serious system error, processing aborted
 */
static errcode_t
process_config_line (
    config_t * conf,
    char * line,
    config_file_context_t * context
    )
{
    char * curr = line;
    char * word;

    word = get_next_word (curr, &curr);
    if (!word || word [0] == k_comment_char)
    {
        // Nothing interesting on this line
        return 0;
    }

    if (strcmp (word, k_keyword_domain) == 0)
    {
        word = get_next_word (curr, &curr);
        if (word)
        {
            int errcode = add_domain (conf, word);
            if (errcode)
            {
                // something badly wrong, bail
                return errcode;
            }

            if (get_next_word (curr, NULL))
            {
                syslog (LOG_WARNING,
                        "%s, line %d: ignored extra text found after domain",
                        context->filename,
                        context->linenum
                        );
            }
        }
        else
        {
            syslog (LOG_WARNING,
                    "%s, line %d: no domain specified",
                    context->filename,
                    context->linenum
                    );
        }
    }
    else
    {
        syslog (LOG_WARNING,
                "%s, line %d: unknown keyword %s - skipping",
                context->filename,
                context->linenum,
                word
                );
    }

    return 0;
}


/*
    Get next word (whitespace separated) from input string.
    A null character is written into the first whitespace character following
    the word.

    Parameters
        input
            Input string.  This string is modified by get_next_word.
        next
            If non-NULL and the result is non-NULL, a pointer to the
            character following the end of the word (after the null)
            is written to 'next'.
            If no word is found, the original value is unchanged.
            If the word extended to the end of the string, 'next' points
            to the trailling NULL.
            It is safe to pass 'str' as 'input' and '&str' as 'next'.
    Returns
        Pointer to the first non-whitespace character (and thus word) found.
        if no word is found, returns NULL.
 */
static char *
get_next_word (char * input, char **next)
{
    char * curr = input;
    char * result;

    while (isspace (*curr))
    {
        curr++;
    }

    if (*curr == 0)
    {
        return NULL;
    }

    result = curr;
    while (*curr && !isspace (*curr))
    {
        curr++;
    }
    if (*curr)
    {
        *curr = 0;
        if (next)
        {
            *next = curr+1;
        }
    }
    else
    {
        if (next)
        {
            *next = curr;
        }
    }

    return result;
}


static errcode_t
default_config (config_t * conf)
{
    int i;
    for (i = 0; k_default_domains [i]; i++)
    {
        int errcode =
            add_domain (conf, k_default_domains [i]);
        if (errcode)
        {
            // Something has gone (badly) wrong - let's bail
            return errcode;
        }
    }

    return 0;
}


static errcode_t
add_domain (config_t * conf, const char * domain)
{
    if (!contains_domain (conf, domain))
    {
        domain_entry_t * d =
            (domain_entry_t *) malloc (sizeof (domain_entry_t));
        if (!d)
        {
            syslog (LOG_ERR,
                    "mdns: Can't allocate memory in nss_mdns:init_config, %s:%d",
                    __FILE__, __LINE__
                    );
            return ENOMEM;
        }

        d->domain = strdup (domain);
        if (!d->domain)
        {
            syslog (LOG_ERR,
                    "mdns: Can't allocate memory in nss_mdns:init_config, %s:%d",
                    __FILE__, __LINE__
                    );
            free (d);
            return ENOMEM;
        }
        d->next = conf->domains;
        conf->domains = d;
    }

    return 0;
}


static int
contains_domain (const config_t * conf, const char * domain)
{
    const domain_entry_t * curr = conf->domains;

    while (curr != NULL)
    {
        if (strcasecmp (curr->domain, domain) == 0)
        {
            return 1;
        }

        curr = curr->next;
    }

    return 0;
}


static int
contains_domain_suffix (const config_t * conf, const char * addr)
{
    const domain_entry_t * curr = conf->domains;

    while (curr != NULL)
    {
        if (cmp_dns_suffix (addr, curr->domain) > 0)
        {
            return 1;
        }

        curr = curr->next;
    }

    return 0;
}

//----------
// Types and Constants

static const char * k_local_suffix = "local";
static const char k_dns_separator = '.';

static const unsigned int k_label_maxlen = DNS_LABEL_MAXLEN;
// Label entries longer than this are actually pointers.

typedef struct
{
    int value;
    const char * name;
    const char * comment;
} table_entry_t;

static const table_entry_t k_table_af [] =
{
    { AF_UNSPEC, NULL, NULL },
    { AF_LOCAL, "LOCAL", NULL },
    { AF_UNIX, "UNIX", NULL },
    { AF_INET, "INET", NULL },
    { AF_INET6, "INET6", NULL }
};
static const int k_table_af_size =
    sizeof (k_table_af) / sizeof (*k_table_af);

static const char * k_table_ns_class [] =
{
    NULL,
    "IN"
};
static const int k_table_ns_class_size =
    sizeof (k_table_ns_class) / sizeof (*k_table_ns_class);

static const char * k_table_ns_type [] =
{
    NULL,
    "A",
    "NS",
    "MD",
    "MF",
    "CNAME",
    "SOA",
    "MB",
    "MG",
    "MR",
    "NULL",
    "WKS",
    "PTR",
    "HINFO",
    "MINFO",
    "MX",
    "TXT",
    "RP",
    "AFSDB",
    "X25",
    "ISDN",
    "RT",
    "NSAP",
    NULL,
    "SIG",
    "KEY",
    "PX",
    "GPOS",
    "AAAA",
    "LOC",
    "NXT",
    "EID",
    "NIMLOC",
    "SRV",
    "ATMA",
    "NAPTR",
    "KX",
    "CERT",
    "A6",
    "DNAME",
    "SINK",
    "OPT"
};
static const int k_table_ns_type_size =
    sizeof (k_table_ns_type) / sizeof (*k_table_ns_type);


//----------
// Local prototypes

static int
simple_table_index (const char * table [], int size, const char * str);

static int
table_index_name (const table_entry_t table [], int size, const char * str);

static int
table_index_value (const table_entry_t table [], int size, int n);


//----------
// Global variables


//----------
// Util functions

int
count_dots (const char * name)
{
    int count = 0;
    int i;
    for (i = 0; name[i]; i++)
    {
        if (name [i] == k_dns_separator)
            count++;
    }

    return count;
}


int
islocal (const char * name)
{
    return cmp_dns_suffix (name, k_local_suffix) > 0;
}


int
rr_to_af (ns_type_t rrtype)
{
    switch (rrtype)
    {
    case kDNSServiceType_A:
        return AF_INET;

    case kDNSServiceType_AAAA:
        return AF_INET6;

    default:
        return AF_UNSPEC;
    }
}


ns_type_t
af_to_rr (int af)
{
    switch (af)
    {
    case AF_INET:
        return kDNSServiceType_A;

    case AF_INET6:
        return kDNSServiceType_AAAA;

    default:
        //return ns_t_invalid;
        return 0;
    }
}


int
str_to_af (const char * str)
{
    int result =
        table_index_name (k_table_af, k_table_af_size, str);
    if (result < 0)
        result = 0;

    return k_table_af [result].value;
}


ns_class_t
str_to_ns_class (const char * str)
{
    return (ns_class_t)
           simple_table_index (k_table_ns_class, k_table_ns_class_size, str);
}


ns_type_t
str_to_ns_type (const char * str)
{
    return (ns_type_t)
           simple_table_index (k_table_ns_type, k_table_ns_type_size, str);
}


const char *
af_to_str (int in)
{
    int result =
        table_index_value (k_table_af, k_table_af_size, in);
    if (result < 0)
        result = 0;

    return k_table_af [result].name;
}


const char *
ns_class_to_str (ns_class_t in)
{
    if (in < k_table_ns_class_size)
        return k_table_ns_class [in];
    else
        return NULL;
}


const char *
ns_type_to_str (ns_type_t in)
{
    if (in < k_table_ns_type_size)
        return k_table_ns_type [in];
    else
        return NULL;
}


char *
format_reverse_addr_in (
    const struct in_addr * addr,
    int prefixlen,
    char * buf
    )
{
    char * curr = buf;
    int i;

    const uint8_t * in_addr_a = (uint8_t *) addr;

    if (prefixlen > 32)
        return NULL;
    if (prefixlen < 0)
        prefixlen = 32;

    i = (prefixlen + 7) / 8;
    // divide prefixlen into bytes, rounding up

    while (i > 0)
    {
        i--;
        curr += sprintf (curr, "%d.", in_addr_a [i]);
    }
    sprintf (curr, "in-addr.arpa");

    return buf;
}


char *
format_reverse_addr_in6 (
    const struct in6_addr * addr,
    int prefixlen,
    char * buf
    )
{
    char * curr = buf;
    int i;

    const uint8_t * in_addr_a = (uint8_t *) addr;

    if (prefixlen > 128)
        return NULL;
    if (prefixlen < 0)
        prefixlen = 128;

    i = (prefixlen + 3) / 4;
    // divide prefixlen into nibbles, rounding up

    // Special handling for first
    if (i % 2)
    {
        curr += sprintf (curr, "%d.", (in_addr_a [i/2] >> 4) & 0x0F);
    }
    i >>= 1;
    // Convert i to bytes (divide by 2)

    while (i > 0)
    {
        uint8_t val;

        i--;
        val = in_addr_a [i];
        curr += sprintf (curr, "%x.%x.", val & 0x0F, (val >> 4) & 0x0F);
    }
    sprintf (curr, "ip6.arpa");

    return buf;
}


char *
format_reverse_addr (
    int af,
    const void * addr,
    int prefixlen,
    char * buf
    )
{
    switch (af)
    {
    case AF_INET:
        return
            format_reverse_addr_in (
                (struct in_addr *) addr, prefixlen, buf
                );
        break;

    case AF_INET6:
        return
            format_reverse_addr_in6 (
                (struct in6_addr *) addr, prefixlen, buf
                );
        break;

    default:
        return NULL;
    }
}


int
cmp_dns_suffix (const char * name, const char * domain)
{
    const char * nametail;
    const char * domaintail;

    // Idiot checks
    if (*name == 0 || *name == k_dns_separator)
    {
        // Name can't be empty or start with separator
        return CMP_DNS_SUFFIX_BAD_NAME;
    }

    if (*domain == 0)
    {
        return CMP_DNS_SUFFIX_SUCCESS;
        // trivially true
    }

    if (*domain == k_dns_separator)
    {
        // drop leading separator from domain
        domain++;
        if (*domain == k_dns_separator)
        {
            return CMP_DNS_SUFFIX_BAD_DOMAIN;
        }
    }

    // Find ends of strings
    for (nametail = name; *nametail; nametail++)
        ;
    for (domaintail = domain; *domaintail; domaintail++)
        ;

    // Shuffle back to last real character, and drop any trailing '.'
    // while we're at it.
    nametail--;
    if (*nametail == k_dns_separator)
    {
        nametail--;
        if (*nametail == k_dns_separator)
        {
            return CMP_DNS_SUFFIX_BAD_NAME;
        }
    }
    domaintail--;
    if (*domaintail == k_dns_separator)
    {
        domaintail--;
        if (*domaintail == k_dns_separator)
        {
            return CMP_DNS_SUFFIX_BAD_DOMAIN;
        }
    }

    // Compare.
    while (
        nametail >= name
        && domaintail >= domain
        && tolower(*nametail) == tolower(*domaintail))
    {
        nametail--;
        domaintail--;
    }

    /* A successful finish will be one of the following:
        (leading and trailing . ignored)

        name  :  domain2.domain1
        domain:  domain2.domain1
                ^

        name  : domain3.domain2.domain1
        domain:         domain2.domain1
                       ^
     */
    if (
        domaintail < domain
        && (nametail < name || *nametail == k_dns_separator)
        )
    {
        return CMP_DNS_SUFFIX_SUCCESS;
    }
    else
    {
        return CMP_DNS_SUFFIX_FAILURE;
    }
}


static int
dns_rdata_to_name (const unsigned char * rdata, int rdlen, char * name, unsigned int name_len)
{
    int i = 0;
    // Index into 'name'
    const unsigned char * rdata_curr = rdata;

    if (rdlen == 0) return DNS_RDATA_TO_NAME_BAD_FORMAT;

    /*
        In RDATA, a DNS name is stored as a series of labels.
        Each label consists of a length octet (max value 63)
        followed by the data for that label.
        The series is terminated with a length 0 octet.
        A length octet beginning with bits 11 is a pointer to
        somewhere else in the payload, but we don't support these
        since we don't have access to the entire payload.

        See RFC1034 section 3.1 and RFC1035 section 3.1.
     */
    while (1)
    {
        unsigned int term_len = *rdata_curr;
        rdata_curr++;

        if (term_len == 0)
        {
            break;
            // 0 length record terminates label
        }
        else if (term_len > k_label_maxlen)
        {
            name [i] = 0;
            return DNS_RDATA_TO_NAME_PTR;
        }
        else if (rdata_curr + term_len > rdata + rdlen)
        {
            name [i] = 0;
            return DNS_RDATA_TO_NAME_BAD_FORMAT;
        }

        if (name_len < i + term_len + 1)
        // +1 is separator
        {
            name [i] = 0;
            return DNS_RDATA_TO_NAME_TOO_LONG;
        }

        memcpy (name + i, rdata_curr, term_len);

        i += term_len;
        rdata_curr += term_len;

        name [i] = k_dns_separator;
        i++;
    }

    name [i] = 0;
    return i;
}


//----------
// Local functions

/*
    Find the index of an string entry in a table.  A case insenitive match
    is performed.  If no entry is found, 0 is returned.

    Parameters
        table
            Lookup table
            Table entries may be NULL.  NULL entries will never match.
        size
            number of entries in table
        str
            lookup string

    Result
        index of first matching entry, or 0 if no matches
 */
static int
simple_table_index (const char * table [], int size, const char * str)
{
    int i;
    for (i = 0; i < size; i++)
    {
        if (
            table [i]
            && (strcasecmp (table [i], str) == 0)
            )
        {
            return i;
        }
    }

    return 0;
}


/*
    Find the index of a name in a table.

    Parameters
        table
            array of table_entry_t records.  The name field is compared
            (ignoring case) to the input string.
        size
            number of entries in table
        str
            lookup string

    Result
        index of first matching entry, or -1 if no matches
 */
static int
table_index_name (const table_entry_t table [], int size, const char * str)
{
    int i;
    for (i = 0; i < size; i++)
    {
        if (
            table [i].name
            && (strcasecmp (table [i].name, str) == 0)
            )
        {
            return i;
        }
    }

    return -1;
}


/*
    Find the index of a value a table.

    Parameters
        table
            array of table_entry_t records.  The value field is compared to
            the input value
        size
            number of entries in table
        n
            lookup value

    Result
        index of first matching entry, or -1 if no matches
 */
static int
table_index_value (const table_entry_t table [], int size, int n)
{
    int i;
    for (i = 0; i < size; i++)
    {
        if (table [i].value == n)
        {
            return i;
        }
    }

    return -1;
}
