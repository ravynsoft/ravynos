/* dnssd-proxy.c
 *
 * Copyright (c) 2018-2019 Apple Computer, Inc. All rights reserved.
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
 *
 * This is a Discovery Proxy module for the SRP gateway.
 *
 * The motivation here is that it makes sense to co-locate the SRP relay and the Discovery Proxy because
 * these functions are likely to co-exist on the same node, listening on the same port.  For homenet-style
 * name resolution, we need a DNS proxy that implements DNSSD Discovery Proxy for local queries, but
 * forwards other queries to an ISP resolver.  The SRP gateway is already expecting to do this.
 * This module implements the functions required to allow the SRP gateway to also do Discovery Relay.
 * 
 * The Discovery Proxy relies on Apple's DNS-SD library and the mDNSResponder DNSSD server, which is included
 * in Apple's open source mDNSResponder package, available here:
 *
 *            https://opensource.apple.com/tarballs/mDNSResponder/
 */

#define __APPLE_USE_RFC_3542

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/event.h>
#include <fcntl.h>
#include <sys/time.h>
#include <ctype.h>

#include "dns_sd.h"
#include "srp.h"
#include "dns-msg.h"
#include "srp-crypto.h"
#include "dso.h"
#include "ioloop.h"

// Enumerate the list of interfaces, map them to interface indexes, give each one a name
// Have a tree of subdomains for matching

typedef struct dnssd_query {
    io_t io;
    DNSServiceRef ref;
    char *name;						// The name we are looking up.
    const char *enclosing_domain;	// The domain the name is in, or NULL if not ours; if null, name is an FQDN.
    dns_name_pointer_t enclosing_domain_pointer;
    message_t *question;
    comm_t *connection;
    dso_activity_t *activity;
    int serviceFlags;				// Service flags to use with this query.
    bool is_dns_push;
    bool is_edns0;
    uint16_t type, qclass;			// Original query type and class.
    dns_towire_state_t towire;
    uint8_t *p_dso_length;			// Where to store the DSO length just before we write out a push notification.
    dns_wire_t response;			// This has to be at the end because we don't zero the RRdata buffer.
} dnssd_query_t;

const char push_subscription_activity_type[] = "push subscription";

const char local_suffix[] = ".local.";
#define PROXIED_DOMAIN "proxy.home.arpa."
const char proxied_domain[] = PROXIED_DOMAIN;
const char proxied_domain_ld[] = "." PROXIED_DOMAIN;
#define MY_NAME "proxy.example.com."
#define MY_IPV4_ADDR "192.0.2.1"
// #define MY_IPV6_ADDR "2001:db8::1" // for example

#define TOWIRE_CHECK(note, towire, func) { func; if ((towire)->error != 0 && failnote == NULL) failnote = (note); }

int64_t dso_transport_idle(void *context, int64_t next_event)
{
    return next_event;
}

void dnssd_query_cancel(io_t *io)
{
    dnssd_query_t *query = (dnssd_query_t *)io;
    if (query->io.sock != -1) {
        DNSServiceRefDeallocate(query->ref);
        query->io.sock = -1;
    }
    query->connection = NULL;
}

void
dns_push_finalize(dso_activity_t *activity)
{
    dnssd_query_t *query = (dnssd_query_t *)activity->context;
    INFO("dnssd_push_finalize: %s", activity->name);
    dnssd_query_cancel(&query->io);
}

void
dnssd_query_finalize(io_t *io)
{
    dnssd_query_t *query = (dnssd_query_t *)io;
    INFO("dnssd_query_finalize on %s%s", query->name, query->enclosing_domain ? ".local" : "");
    if (query->question) {
        message_free(query->question);
    }
    free(query->name);
    free(query);
}

static void
dnssd_query_callback(io_t *io)
{
    dnssd_query_t *query = (dnssd_query_t *)io;
    int status = DNSServiceProcessResult(query->ref);
    if (status != kDNSServiceErr_NoError) {
        ERROR("DNSServiceProcessResult on %s%s returned %d", query->name, query->enclosing_domain ? ".local" : "", status);
        if (query->activity != NULL && query->connection != NULL) {
            dso_drop_activity(query->connection->dso, query->activity);
        } else {
            dnssd_query_cancel(&query->io);
        }
    }
}

static void
add_dnssd_query(dnssd_query_t *query)
{
    io_t *io = &query->io;
    io->sock = DNSServiceRefSockFD(query->ref);
    io->cancel = dnssd_query_cancel;
    io->cancel_on_close = &query->connection->io;
    add_reader(io, dnssd_query_callback, dnssd_query_finalize);
}

// Parse a NUL-terminated text string into a sequence of labels.
dns_name_t *
dns_pres_name_parse(const char *pname)
{
    const char *dot = strchr(pname, '.');
    dns_label_t *ret;
    int len;
    if (dot == NULL) {
        dot = pname + strlen(pname);
    }
    len = (dot - pname) + 1 + (sizeof *ret) - DNS_MAX_LABEL_SIZE;
    ret = calloc(len, 1);
    if (ret == NULL) {
        return NULL;
    }
    ret->len = dot - pname;
    if (ret->len > 0) {
        memcpy(ret->data, pname, ret->len);
    }
    ret->data[ret->len] = 0;
    if (dot[0] == '.') {
        ret->next = dns_pres_name_parse(dot + 1);
    }
    return ret;
}

bool
dns_subdomain_of(dns_name_t *name, dns_name_t *domain, char *buf, size_t buflen)
{
    int dnum = 0, nnum = 0;
    dns_name_t *np, *dp;
    char *bufp = buf;
    size_t bytesleft = buflen;

    for (dp = domain; dp; dp = dp->next) {
        dnum++;
    }
    for (np = name; np; np = np->next) {
        nnum++;
    }
    if (nnum < dnum) {
        return false;
    }
    for (np = name; np; np = np->next) {
        if (nnum-- == dnum) {
            break;
        }
    }
    if (dns_names_equal(np, domain)) {
        for (dp = name; dp != np; dp = dp->next) {
            if (dp->len + 1 > bytesleft) {
                // It's okay to return false here because a name that overflows the buffer isn't valid.
                ERROR("dns_subdomain_of: out of buffer space!");
                return false;
            }
            memcpy(bufp, dp->data, dp->len);
            bufp += dp->len;
            bytesleft = bytesleft - dp->len;
            if (dp->next != np) {
                *bufp++ = '.';
                bytesleft -= dp->len;
            }
        }
        *bufp = 0;
        return true;
    }
    return false;
}

void
dp_simple_response(comm_t *comm, int rcode)
{
    if (comm->send_response) {
        struct iovec iov;
        dns_wire_t response;
        memset(&response, 0, DNS_HEADER_SIZE);

        // We take the ID and the opcode from the incoming message, because if the
        // header has been mangled, we (a) wouldn't have gotten here and (b) don't
        // have any better choice anyway.
        response.id = comm->message->wire.id;
        dns_qr_set(&response, dns_qr_response);
        dns_opcode_set(&response, dns_opcode_get(&comm->message->wire));
        dns_rcode_set(&response, rcode);
        iov.iov_base = &response;
        iov.iov_len = DNS_HEADER_SIZE; // No RRs
        comm->send_response(comm, comm->message, &iov, 1);
    }
}

bool
dp_served(dns_name_t *name, char *buf, size_t bufsize)
{
    static dns_name_t *home_dot_arpa = NULL;
    if (home_dot_arpa == NULL) {
        home_dot_arpa = dns_pres_name_parse(proxied_domain);
        if (home_dot_arpa == NULL) {
            ERROR("Unable to parse %s!", proxied_domain);
            return false;
        }
    }

     // For now we treat any query to home.arpa as local.
    return dns_subdomain_of(name, home_dot_arpa, buf, bufsize);
}

// Utility function to find "local" on the end of a string of labels.
bool
truncate_local(dns_name_t *name)
{
    dns_label_t *lp, *prev, *prevprev;
    
    prevprev = prev = NULL;
    // Find the root label.
    for (lp = name; lp && lp->len; lp = lp->next) {
        prevprev = prev;
        prev = lp;
    }
    if (lp && prev && prevprev) {
        if (prev->len == 5 && !strncasecmp(prev->data, "local", 5)) {
            dns_name_free(prev);
            prevprev->next = NULL;
            return true;
        }
    }
    dns_name_free(name);
    return false;
}    

void
dp_query_add_data_to_response(dnssd_query_t *query, const char *fullname,
                              uint16_t rrtype, uint16_t rrclass, uint16_t rdlen, const void *rdata, uint32_t ttl)
{
    dns_towire_state_t *towire = &query->towire;
    const char *failnote = NULL;

    // Rewrite the domain if it's .local.
    if (query->enclosing_domain != NULL) {
        TOWIRE_CHECK("query name", towire, dns_name_to_wire(NULL, towire, query->name));
        if (query->enclosing_domain_pointer.message_start != NULL) {
            // This happens if we are sending a DNS response, because we can always point back to the question.
            TOWIRE_CHECK("enclosing_domain_pointer", towire,
                         dns_pointer_to_wire(NULL, towire, &query->enclosing_domain_pointer));
            INFO(" dns answer:  type %02d class %02d %s.%s (p)", rrtype, rrclass, query->name, query->enclosing_domain);
        } else {
            // This happens if we are sending a DNS Push notification.
            TOWIRE_CHECK("enclosing_domain", towire, dns_full_name_to_wire(NULL, towire, query->enclosing_domain));
            INFO("push answer:  type %02d class %02d %s.%s", rrtype, rrclass, query->name, query->enclosing_domain);
        }
    } else {
        TOWIRE_CHECK("query->name", towire, dns_full_name_to_wire(NULL, towire, query->name));
        INFO("%s answer:  type %02d class %02d %s.%s (p)",
             query->is_dns_push ? "push" : " dns", rrtype, rrclass, query->name, query->enclosing_domain);
    }
    TOWIRE_CHECK("rrtype", towire, dns_u16_to_wire(towire, rrtype));
    TOWIRE_CHECK("rrclass", towire, dns_u16_to_wire(towire, rrclass));
    TOWIRE_CHECK("ttl", towire, dns_ttl_to_wire(towire, ttl));

    if (rdlen > 0) {
        // If necessary, correct domain names inside of rrdata.
        if (rrclass == dns_qclass_in && (rrtype == dns_rrtype_srv ||
                                         rrtype == dns_rrtype_ptr ||
                                         rrtype == dns_rrtype_cname)) {
            dns_rr_t answer;
            dns_name_t *name;
            unsigned offp = 0;
            answer.type = rrtype;
            answer.qclass = rrclass;
            if (!dns_rdata_parse_data(&answer, rdata, &offp, rdlen, rdlen, 0)) {
                ERROR("dp_query_add_data_to_response: rdata from mDNSResponder didn't parse!!");
                goto raw;
            }
            switch(rrtype) {
            case dns_rrtype_cname:
            case dns_rrtype_ptr:
                name = answer.data.ptr.name;
                if (!truncate_local(name)) {
                    goto raw;
                }
                TOWIRE_CHECK("rdlength begin", towire, dns_rdlength_begin(towire));
                break;
            case dns_rrtype_srv:
                name = answer.data.srv.name;
                if (!truncate_local(name)) {
                    goto raw;
                }
                TOWIRE_CHECK("rdlength begin", towire, dns_rdlength_begin(towire));
                TOWIRE_CHECK("answer.data.srv.priority", towire, dns_u16_to_wire(towire, answer.data.srv.priority));
                TOWIRE_CHECK("answer.data.srv.weight", towire, dns_u16_to_wire(towire, answer.data.srv.weight));
                TOWIRE_CHECK("answer.data.srv.port", towire, dns_u16_to_wire(towire, answer.data.srv.port));
                break;
            default:
                ERROR("dp_query_add_data_to_response: can't get here.");
                goto raw;
                break;
            }
            // If we get here, the name ended in "local."
            int bytes_written = dns_name_to_wire_canonical(towire->p, towire->lim - towire->p, name);
            towire->p += bytes_written;
            if (query->enclosing_domain_pointer.message_start != NULL) {
                TOWIRE_CHECK("enclosing_domain_pointer internal", towire,
                             dns_pointer_to_wire(NULL, towire, &query->enclosing_domain_pointer));
            } else {
                TOWIRE_CHECK("enclosing_domain internal", towire,
                             dns_full_name_to_wire(NULL, towire, query->enclosing_domain));
            }
            dns_rdlength_end(towire);
        } else {
        raw:
            TOWIRE_CHECK("rdlen", towire, dns_u16_to_wire(towire, rdlen));
            TOWIRE_CHECK("rdata", towire, dns_rdata_raw_data_to_wire(towire, rdata, rdlen));
        }
    } else {
        TOWIRE_CHECK("rdlen", towire, dns_u16_to_wire(towire, rdlen));
    }
    if (failnote) {
        ERROR("dp_query_add_data_to_response: %s", failnote);
    }
}

typedef struct hardwired hardwired_t;
struct hardwired {
    hardwired_t *next;
    uint16_t type;
    char *name;
    char *fullname;
    uint8_t *rdata;
    uint16_t rdlen;
} *hardwired_responses;

void
dnssd_hardwired_add(const char *name, const char *domain, size_t rdlen, uint8_t *rdata, uint16_t type)
{
    hardwired_t *hp;
    int namelen = strlen(name);
    size_t total = (sizeof *hp) + rdlen + namelen * 2 + strlen(proxied_domain_ld) + 2;

    hp = calloc(1, (sizeof *hp) + rdlen + namelen * 2 + strlen(proxied_domain_ld) + 2);
    hp->rdata = (uint8_t *)(hp + 1);
    hp->rdlen = rdlen;
    memcpy(hp->rdata, rdata, rdlen);
    hp->name = (char *)hp->rdata + rdlen;
    strcpy(hp->name, name);
    hp->fullname = hp->name + namelen + 1;
    strcpy(hp->fullname, name);
    strcpy(hp->fullname + namelen, proxied_domain_ld);
    if (hp->fullname + strlen(hp->fullname) + 1 != (char *)hp + total) {
        ERROR("%p != %p", hp->fullname + strlen(hp->fullname) + 1, ((char *)hp) + total);
    }
    hp->type = type;
    hp->next = hardwired_responses;
    hardwired_responses = hp;

    INFO("hardwired_add: fullname %s name %s type %d rdlen %d", hp->fullname, hp->name, hp->type, hp->rdlen);
}

void
dnssd_hardwired_setup(void)
{
    dns_wire_t wire;
    dns_towire_state_t towire;

#define RESET \
    memset(&towire, 0, sizeof towire); \
    towire.message = &wire; \
    towire.p = wire.data; \
    towire.lim = towire.p + sizeof wire.data

    // Browsing pointers...
    RESET;
    dns_full_name_to_wire(NULL, &towire, proxied_domain);
    dnssd_hardwired_add("b._dns-sd._udp", proxied_domain_ld, towire.p - wire.data, wire.data, dns_rrtype_ptr);
    dnssd_hardwired_add("lb._dns-sd._udp", proxied_domain_ld, towire.p - wire.data, wire.data, dns_rrtype_ptr);
    
    // SRV
    // _dns-push-tls._tcp
    RESET;
    dns_u16_to_wire(&towire, 0); // priority
    dns_u16_to_wire(&towire, 0); // weight
    dns_u16_to_wire(&towire, 53); // port
    // Define MY_NAME to reference a name for this server in a different zone.
#ifndef MY_NAME
    dns_name_to_wire(NULL, &towire, "ns");
    dns_full_name_to_wire(NULL, &towire, proxied_domain);
#else
    dns_full_name_to_wire(NULL, &towire, MY_NAME);
#endif
    dnssd_hardwired_add("_dns-push-tls._tcp", proxied_domain_ld, towire.p - wire.data, wire.data, dns_rrtype_srv);
    
    // A
#ifndef MY_NAME
    // ns
#ifdef MY_IPV4_ADDR
    RESET;
    dns_rdata_a_to_wire(&towire, MY_IPV4_ADDR);
    dnssd_hardwired_add("ns", proxied_domain_ld, towire.p - wire.data, wire.data, dns_rrtype_a);
#endif

    // AAAA
#ifdef MY_IPV6_ADDR
    RESET;
    dns_rdata_aaaa_to_wire(&towire, MY_IPV6_ADDR);
    dnssd_hardwired_add("ns", proxied_domain_ld, towire.p - wire.data, wire.data, dns_rrtype_aaaa);
#endif
#endif

    // NS
    RESET;
#ifdef MY_NAME
    dns_full_name_to_wire(NULL, &towire, MY_NAME);
#else
    dns_name_to_wire(NULL, &towire, "ns");
    dns_full_name_to_wire(NULL, &towire, proxied_domain);
#endif
    dnssd_hardwired_add("", proxied_domain, towire.p - wire.data, wire.data, dns_rrtype_ns);

    // SOA (piggybacking on what we already did for NS, which starts the same.
    dns_name_to_wire(NULL, &towire, "postmaster");
    dns_full_name_to_wire(NULL, &towire, proxied_domain);
    dns_u32_to_wire(&towire, 0);     // serial 
    dns_ttl_to_wire(&towire, 7200);  // refresh
    dns_ttl_to_wire(&towire, 3600);  // retry
    dns_ttl_to_wire(&towire, 86400); // expire
    dns_ttl_to_wire(&towire, 120);    // minimum
    dnssd_hardwired_add("", proxied_domain, towire.p - wire.data, wire.data, dns_rrtype_soa);
}

void
dp_query_send_dns_response(dnssd_query_t *query)
{
    struct iovec iov;
    dns_towire_state_t *towire = &query->towire;
    const char *failnote = NULL;

    // Send an SOA record if it's a .local query.
    if (query->enclosing_domain != NULL) {
        // DNSSD Hybrid, Section 6.1.
        TOWIRE_CHECK("&query->enclosing_domain_pointer", towire,
                     dns_pointer_to_wire(NULL, towire, &query->enclosing_domain_pointer));
        TOWIRE_CHECK("dns_rrtype_soa", towire,
                     dns_u16_to_wire(towire, dns_rrtype_soa));
        TOWIRE_CHECK("dns_qclass_in", towire,
                     dns_u16_to_wire(towire, dns_qclass_in));
        TOWIRE_CHECK("ttl", towire, dns_ttl_to_wire(towire, 3600));
        TOWIRE_CHECK("rdlength_begin ", towire, dns_rdlength_begin(towire));
#ifdef MY_NAME
        TOWIRE_CHECK(MY_NAME, towire, dns_full_name_to_wire(NULL, towire, MY_NAME));
#else
        TOWIRE_CHECK("\"ns\"", towire, dns_name_to_wire(NULL, towire, "ns"));
        TOWIRE_CHECK("&query->enclosing_domain_pointer", towire,
                     dns_pointer_to_wire(NULL, towire, &query->enclosing_domain_pointer));
#endif
        TOWIRE_CHECK("\"postmaster\"", towire,
                     dns_name_to_wire(NULL, towire, "postmaster"));
        TOWIRE_CHECK("&query->enclosing_domain_pointer", towire,
                     dns_pointer_to_wire(NULL, towire, &query->enclosing_domain_pointer));
        TOWIRE_CHECK("serial", towire,dns_u32_to_wire(towire, 0));     // serial 
        TOWIRE_CHECK("refresh", towire, dns_ttl_to_wire(towire, 7200));  // refresh
        TOWIRE_CHECK("retry", towire, dns_ttl_to_wire(towire, 3600));  // retry
        TOWIRE_CHECK("expire", towire, dns_ttl_to_wire(towire, 86400)); // expire
        TOWIRE_CHECK("minimum", towire, dns_ttl_to_wire(towire, 120));    // minimum
        dns_rdlength_end(towire);
        query->response.nscount = htons(1);

        // Response is authoritative and not recursive.
        query->response.bitfield = htons((ntohs(query->response.bitfield) | dns_flags_aa) & ~dns_flags_ra);
    } else {
        // Response is recursive and not authoritative.
        query->response.bitfield = htons((ntohs(query->response.bitfield) | dns_flags_ra) & ~dns_flags_aa);
    }
    // Not truncated, not authentic, checking not disabled.
    query->response.bitfield = htons(ntohs(query->response.bitfield) & ~(dns_flags_rd | dns_flags_tc | dns_flags_ad | dns_flags_cd));

    // This is a response
    dns_qr_set(&query->response, dns_qr_response);
    // No error.
    dns_rcode_set(&query->response, dns_rcode_noerror);
    
    // Send an OPT RR if we got one
    if (query->is_edns0) {
        TOWIRE_CHECK("Root label", towire, dns_u8_to_wire(towire, 0));     // Root label
        TOWIRE_CHECK("dns_rrtype_opt", towire, dns_u16_to_wire(towire, dns_rrtype_opt));
        TOWIRE_CHECK("UDP Payload size", towire, dns_u16_to_wire(towire, 4096)); // UDP Payload size
        TOWIRE_CHECK("extended-rcode", towire, dns_u8_to_wire(towire, 0));     // extended-rcode
        TOWIRE_CHECK("EDNS version 0", towire, dns_u8_to_wire(towire, 0));     // EDNS version 0
        TOWIRE_CHECK("No extended flags", towire, dns_u16_to_wire(towire, 0));    // No extended flags
        TOWIRE_CHECK("No payload", towire, dns_u16_to_wire(towire, 0));    // No payload
        query->response.arcount = htons(1);
    }

    if (towire->error) {
        ERROR("dp_query_send_dns_response failed on %s", failnote);
    }

    iov.iov_len = (query->towire.p - (uint8_t *)&query->response);
    iov.iov_base = &query->response;
    INFO("dp_query_send_dns_response: %s (len %zd)", query->name, iov.iov_len);

    if (query->connection != NULL) {
        query->connection->send_response(query->connection, query->question, &iov, 1);
    }

    // Free up state
    dnssd_query_cancel(&query->io);
    // Query will be freed automatically next time through the io loop.
}

void
dp_query_towire_reset(dnssd_query_t *query)
{
    query->towire.p = &query->response.data[0];  // We start storing RR data here.
    query->towire.lim = &query->response.data[DNS_DATA_SIZE]; // This is the limit to how much we can store.
    query->towire.message = &query->response;
    query->p_dso_length = NULL;
}

void
dns_push_start(dnssd_query_t *query)
{
    const char *failnote = NULL;
    
    // If we don't have a dso header yet, start one.
    if (query->p_dso_length == NULL) {
        memset(&query->response, 0, (sizeof query->response) - DNS_DATA_SIZE);
        dns_opcode_set(&query->response, dns_opcode_dso);
        // This is a unidirectional DSO message, which is marked as a query
        dns_qr_set(&query->response, dns_qr_query);
        // No error cuz not a response.
        dns_rcode_set(&query->response, dns_rcode_noerror);

        TOWIRE_CHECK("kDSOType_DNSPushUpdate", &query->towire,
                     dns_u16_to_wire(&query->towire, kDSOType_DNSPushUpdate));
        if (query->towire.p + 2 > query->towire.lim) {
            ERROR("No room for dso length in DNS Push notification message.");
            dp_query_towire_reset(query);
            return;
        }
        query->p_dso_length = query->towire.p;
        query->towire.p += 2;
    }
    if (failnote != NULL) {
        ERROR("dns_push_start: couldn't start update: %s", failnote);
    }
}

void
dp_push_response(dnssd_query_t *query)
{
    struct iovec iov;

    if (query->p_dso_length != NULL) {
        int16_t dso_length = query->towire.p - query->p_dso_length - 2;
        iov.iov_len = (query->towire.p - (uint8_t *)&query->response);
        iov.iov_base = &query->response;
        INFO("dp_push_response: %s (len %zd)", query->name, iov.iov_len);

        query->towire.p = query->p_dso_length;
        dns_u16_to_wire(&query->towire, dso_length);
        if (query->connection != NULL) {
            query->connection->send_response(query->connection, query->question, &iov, 1);
        }
        dp_query_towire_reset(query);
    }
}

bool
dnssd_hardwired_response(dnssd_query_t *query, DNSServiceQueryRecordReply callback)
{
    hardwired_t *hp;
    bool got_response = false;

    for (hp = hardwired_responses; hp; hp = hp->next) {
        if ((query->type == hp->type || query->type == dns_rrtype_any) &&
            query->qclass == dns_qclass_in && !strcasecmp(hp->name, query->name)) {
            if (query->is_dns_push) {
                dns_push_start(query);
                dp_query_add_data_to_response(query, hp->fullname, hp->type, dns_qclass_in, hp->rdlen, hp->rdata, 3600);
            } else {
                // Store the response
                dp_query_add_data_to_response(query, hp->fullname, hp->type, dns_qclass_in, hp->rdlen, hp->rdata, 3600);
                query->response.ancount = htons(ntohs(query->response.ancount) + 1);
            }
            got_response = true;
        }
    }
    if (got_response) {
        if (query->is_dns_push) {
            dp_push_response(query);
        } else {
            // Steal the question
            query->question = query->connection->message;
            query->connection->message = NULL;
            // Send the answer(s).
            dp_query_send_dns_response(query);
        }
        return true;
    }
    return false;
}

// This is the callback for dns query results.
void
dns_query_callback(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode,
                   const char *fullname, uint16_t rrtype, uint16_t rrclass, uint16_t rdlen, const void *rdata,
                   uint32_t ttl, void *context)
{
    dnssd_query_t *query = context;
    
    INFO("%s %d %d %x %d", fullname, rrtype, rrclass, rdlen, errorCode);

    if (errorCode == kDNSServiceErr_NoError) {
        dp_query_add_data_to_response(query, fullname, rrtype, rrclass, rdlen, rdata,
                                      ttl > 10 ? 10 : ttl); // Per dnssd-hybrid 5.5.1, limit ttl to 10 seconds
        query->response.ancount = htons(ntohs(query->response.ancount) + 1);
        // If there isn't more coming, send the response now
        if (!(flags & kDNSServiceFlagsMoreComing)) {
            dp_query_send_dns_response(query);
        }
    } else if (errorCode == kDNSServiceErr_NoSuchRecord) {
        // If we get "no such record," we can't really do much except return the answer.
        dp_query_send_dns_response(query);
    } else {
        dns_rcode_set(&query->response, dns_rcode_servfail);
        dp_query_send_dns_response(query);
    }
}

void
dp_query_wakeup(io_t *io)
{
    dnssd_query_t *query = (dnssd_query_t *)io;
    char name[DNS_MAX_NAME_SIZE + 1];
    int namelen = strlen(query->name);

    // Should never happen.
    if ((namelen + query->enclosing_domain != NULL ? sizeof local_suffix : 0) > sizeof name) {
        ERROR("db_query_wakeup: no space to construct name.");
        dnssd_query_cancel(&query->io);
    }

    strcpy(name, query->name);
    if (query->enclosing_domain != NULL) {
        strcpy(name + namelen, local_suffix);
    }
    dp_query_send_dns_response(query);
}

bool
dp_query_start(comm_t *comm, dnssd_query_t *query, int *rcode, DNSServiceQueryRecordReply callback)
{
    char name[DNS_MAX_NAME_SIZE + 1];
    char *np;

    if (query->enclosing_domain != NULL) {
        if (dnssd_hardwired_response(query, callback)) {
            *rcode = dns_rcode_noerror;
            return true;
        }

        int len = strlen(query->name);
        if (len + sizeof local_suffix > sizeof name) {
            *rcode = dns_rcode_servfail;
            free(query->name);
            free(query);
            ERROR("question name %s is too long for .local.", name);
            return false;
        }
        memcpy(name, query->name, len);
        memcpy(&name[len], local_suffix, sizeof local_suffix);
        np = name;
    } else {
        np = query->name;
    }
        
    // Issue a DNSServiceQueryRecord call
    int err = DNSServiceQueryRecord(&query->ref, query->serviceFlags,
                                    kDNSServiceInterfaceIndexAny, np, query->type,
                                    query->qclass, callback, query);
    if (err != kDNSServiceErr_NoError) {
        ERROR("dp_query_start: DNSServiceQueryRecord failed for '%s': %d", np, err);
        *rcode = dns_rcode_servfail;
        return false;
    } else {
        INFO("dp_query_start: DNSServiceQueryRecord started for '%s': %d", np, err);
    }
    
    // If this isn't a DNS Push subscription, we need to respond quickly with as much data as we have.  It
    // turns out that dig gives us a second, but also that responses seem to come back in on the order of a
    // millisecond, so we'll wait 100ms.
    if (!query->is_dns_push && query->enclosing_domain) {
        query->io.wakeup_time = ioloop_now + IOLOOP_SECOND / 10;
        query->io.wakeup = dp_query_wakeup;
    }

    add_dnssd_query(query);
    return true;
}

dnssd_query_t *
dp_query_generate(comm_t *comm, dns_rr_t *question, bool dns_push, int *rcode)
{
    char name[DNS_MAX_NAME_SIZE + 1];
    const char *enclosing_domain;

    // If it's a query for a name served by the local discovery proxy, do an mDNS lookup.
    if ((dp_served(question->name, name, sizeof name))) {
        enclosing_domain = proxied_domain;
        INFO("%s question: type %d class %d %s%s -> %s.local", dns_push ? "push" : " dns",
             question->type, question->qclass, name, proxied_domain, name);
    } else {
        dns_name_print(question->name, name, sizeof name);
        enclosing_domain = NULL;
        INFO("%s question: type %d class %d %s",
             dns_push ? "push" : " dns", question->type, question->qclass, name);
    }

    dnssd_query_t *query = malloc(sizeof *query);
    if (query == NULL) {
        ERROR("Unable to allocate memory for query on %s", name);
        *rcode = dns_rcode_servfail;
        return NULL;
    }
    // Zero out everything except the message data buffer, which is large and doesn't need it.
    memset(query, 0, (sizeof *query) - (sizeof query->response) + DNS_HEADER_SIZE);

    // Steal the data from the question.   If subdomain is not null, this is a local mDNS query; otherwise
    // we are recursing.
    INFO("name = %s", name);
    query->name = strdup(name);
    if (!query->name) {
        *rcode = dns_rcode_servfail;
        free(query);
        ERROR("unable to allocate memory for question name on %s", name);
        return NULL;
    }
    // It is safe to assume that enclosing domain will not be freed out from under us.
    query->enclosing_domain = enclosing_domain;
    query->serviceFlags = 0;

    // If this is a local query, add ".local" to the end of the name and require multicast.
    if (enclosing_domain != NULL) {
        query->serviceFlags |= kDNSServiceFlagsForceMulticast;
    } else {
        query->serviceFlags |= kDNSServiceFlagsReturnIntermediates;
    }
    // Name now contains the name we want mDNSResponder to look up.

    // XXX make sure finalize does the right thing.
    query->connection = comm;

    // Remember whether this is a long-lived query.
    query->is_dns_push = dns_push;

    // Start writing the response
    dp_query_towire_reset(query);

    query->type = question->type;
    query->qclass = question->qclass;

    // Just in case we don't need to do a DNSServiceQueryRecord query to satisfy it.
    query->io.sock = -1;

    *rcode = dns_rcode_noerror;
    return query;
}

// This is the callback for DNS push query results, as opposed to push updates.
void
dns_push_query_callback(DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode,
                        const char *fullname, uint16_t rrtype, uint16_t rrclass, uint16_t rdlen, const void *rdata,
                        uint32_t ttl, void *context)
{
    dnssd_query_t *query = context;
    
    // From DNSSD-Hybrid, for mDNS queries:
    // If we have cached answers, respond immediately, because we probably have all the answers.
    // If we don't have cached answers, respond as soon as we get an answer (presumably more-coming will be false).

    // The spec says to not query if we have cached answers.   We trust the DNSServiceQueryRecord call to handle this.

    // If we switch to using a single connection to mDNSResponder, we could have !more-coming trigger a flush of
    // all outstanding queries that aren't waiting on a time trigger.   This is because more-coming isn't
    // query-specific

    INFO("PUSH %s %d %d %x %d", fullname, rrtype, rrclass, rdlen, errorCode);

    // query_state_waiting means that we're answering a regular DNS question
    if (errorCode == kDNSServiceErr_NoError) {
        dns_push_start(query);

        // If kDNSServiceFlagsAdd is set, it's an add, otherwise a delete.
        if (flags & kDNSServiceFlagsAdd) {
            dp_query_add_data_to_response(query, fullname, rrtype, rrclass, rdlen, rdata, ttl);
        } else {
            // I think if this happens it means delete all RRs of this type.
            if (rdlen == 0) {
                dp_query_add_data_to_response(query, fullname, rrtype, dns_qclass_any, rdlen, rdata, 0);
            } else {
                dp_query_add_data_to_response(query, fullname, rrtype, dns_qclass_none, rdlen, rdata, 0);
            }
        }
        // If there isn't more coming, send a DNS Push notification now.
        // XXX If enough comes to fill the response, send the message.
        if (!(flags & kDNSServiceFlagsMoreComing)) {
            dp_push_response(query);
        }
    } else {
        ERROR("dns_push_query_callback: unexpected error code %d", errorCode);
        if (query->connection != NULL) {
            dso_drop_activity(query->connection->dso, query->activity);
        }
    }
}

void
dns_push_subscribe(comm_t *comm, dns_wire_t *header, dso_state_t *dso, dns_rr_t *question,
                   const char *activity_name, const char *opcode_name)
{
    int rcode;
    dnssd_query_t *query = dp_query_generate(comm, question, true, &rcode);
    
    if (!query) {
        dp_simple_response(comm, rcode);
        return;
    }

    dso_activity_t *activity = dso_add_activity(dso, activity_name, push_subscription_activity_type, query, dns_push_finalize);
    query->activity = activity;
    if (!dp_query_start(comm, query, &rcode, dns_push_query_callback)) {
        dso_drop_activity(dso, activity);
        dp_simple_response(comm, rcode);
        return;
    }
    dp_simple_response(comm, dns_rcode_noerror);
}

void
dns_push_reconfirm(comm_t *comm, dns_wire_t *header, dso_state_t *dso)
{
    dns_rr_t question;
    char name[DNS_MAX_NAME_SIZE + 1];
    uint16_t rdlen;

    // The TLV offset should always be pointing into the message.
    unsigned offp = dso->primary.payload - &header->data[0];
    int len = offp + dso->primary.length;
    
    // Parse the name, rrtype and class.   We say there's no rdata even though there is
    // because there's no ttl and also we want the raw rdata, not parsed rdata.
    if (!dns_rr_parse(&question, header->data, len, &offp, false) ||
        !dns_u16_parse(header->data, len, &offp, &rdlen)) {
        dp_simple_response(comm, dns_rcode_formerr);
        ERROR("dns_push_reconfirm: RR parse from %s failed", dso->remote_name);
        return;
    }
    if (rdlen + offp != len) {
        dp_simple_response(comm, dns_rcode_formerr);
        ERROR("dns_push_reconfirm: RRdata parse from %s failed: length mismatch (%d != %d)",
              dso->remote_name, rdlen + offp, len);
        return;
    }

    if ((dp_served(question.name, name, sizeof name))) {
        int len = strlen(name);
        if (len + sizeof local_suffix > sizeof name) {
            dp_simple_response(comm, dns_rcode_formerr);
            ERROR("dns_push_reconfirm: name is too long for .local suffix: %s", name);
            return;
        }
        memcpy(&name[len], local_suffix, sizeof local_suffix);
    } else {
        dns_name_print(question.name, &name[8], sizeof name - 8);
    }
    // transmogrify name.
    DNSServiceReconfirmRecord(0, kDNSServiceInterfaceIndexAny, name,
                              question.type, question.qclass, rdlen, &header->data[offp]);
    dp_simple_response(comm, dns_rcode_noerror);
}

void
dns_push_unsubscribe(comm_t *comm, dns_wire_t *header, dso_state_t *dso, dns_rr_t *question,
                   dso_activity_t *activity, const char *opcode_name)
{
    dso_drop_activity(dso, activity);
    // No response, unsubscribe is unidirectional.
}

void
dns_push_subscription_change(const char *opcode_name, comm_t *comm, dns_wire_t *header, dso_state_t *dso)
{
    // type-in-hex/class-in-hex/name-to-subscribe
    char activity_name[DNS_MAX_NAME_SIZE_ESCAPED + 3 + 4 + 4];
    dso_activity_t *activity;
    
    // The TLV offset should always be pointing into the message.
    unsigned offp = dso->primary.payload - &header->data[0];
    // Get the question
    dns_rr_t question;

    if (!dns_rr_parse(&question, header->data, offp + dso->primary.length, &offp, false)) {
        // Unsubscribes are unidirectional, so no response can be sent
        if (dso->primary.opcode != kDSOType_DNSPushUnsubscribe) {
            dp_simple_response(comm, dns_rcode_formerr);
        }
        ERROR("RR parse for %s from %s failed", dso->remote_name, opcode_name);
        return;
    }

    // Concoct an activity name.
    snprintf(activity_name, sizeof activity_name, "%04x%04x", question.type, question.qclass);
    if ((dp_served(question.name, &activity_name[8], (sizeof activity_name) - 8))) {
        int len = strlen(activity_name);
        if (len + sizeof local_suffix + 8 > sizeof (activity_name)) {
            ERROR("activity name overflow for %s", activity_name);
            return;
        }
        strncpy(&activity_name[len], local_suffix, sizeof local_suffix);
    } else {
        dns_name_print(question.name, &activity_name[8], (sizeof activity_name) - 8);
    }
    
    activity = dso_find_activity(dso, activity_name, push_subscription_activity_type, NULL);
    if (activity == NULL) {
        // Unsubscribe with no activity means no work to do; just return noerror.
        if (dso->primary.opcode != kDSOType_DNSPushSubscribe) {
            ERROR("dso_message: %s for %s when no subscription exists.", opcode_name, activity_name);
            if (dso->primary.opcode == kDSOType_DNSPushReconfirm) {
                dp_simple_response(comm, dns_rcode_noerror);
            }
        } else {
            // In this case we have a push subscribe for which no subscription exists, which means we can do it.
            dns_push_subscribe(comm, header, dso, &question, activity_name, opcode_name);
        }
    } else {
        // Subscribe with a matching activity means no work to do; just return noerror.
        if (dso->primary.opcode == kDSOType_DNSPushSubscribe) {
            dp_simple_response(comm, dns_rcode_noerror);
        }            
        // Otherwise cancel the subscription.
        else {
            dns_push_unsubscribe(comm, header, dso, &question, activity, opcode_name);
        }
    }
}

static void dso_message(comm_t *comm, dns_wire_t *header, dso_state_t *dso)
{
    switch(dso->primary.opcode) {
    case kDSOType_DNSPushSubscribe:
        dns_push_subscription_change("DNS Push Subscribe", comm, header, dso);
        break;
    case kDSOType_DNSPushUnsubscribe:
        dns_push_subscription_change("DNS Push Unsubscribe", comm, header, dso);
        break;

    case kDSOType_DNSPushReconfirm:
        dns_push_reconfirm(comm, header, dso);
        break;
        
    case kDSOType_DNSPushUpdate:
        INFO("dso_message: bogus push update message %d", dso->primary.opcode);
        dso_drop(dso);
        break;

    default:
        INFO("dso_message: unexpected primary TLV %d", dso->primary.opcode);
        dp_simple_response(comm, dns_rcode_dsotypeni);
        break;
    }
    // XXX free the message if we didn't consume it.
}

static void dns_push_callback(void *context, void *header_context,
                              dso_state_t *dso, dso_event_type_t eventType)
{
    dns_wire_t *header = header_context;
	switch(eventType)
    {
	case kDSOEventType_DNSMessage:
        // We shouldn't get here because we already handled any DNS messages
		INFO("dns_push_callback: DNS Message (opcode=%d) received from %s", dns_opcode_get(header), dso->remote_name);
		break;
	case kDSOEventType_DNSResponse:
        // We shouldn't get here because we already handled any DNS messages
		INFO("dns_push_callback: DNS Response (opcode=%d) received from %s", dns_opcode_get(header), dso->remote_name);
		break;
	case kDSOEventType_DSOMessage:
		INFO("dns_push_callback: DSO Message (Primary TLV=%d) received from %s",
               dso->primary.opcode, dso->remote_name);
        dso_message((comm_t *)context, (dns_wire_t *)header, dso);
		break;
	case kDSOEventType_DSOResponse:
		INFO("dns_push_callback: DSO Response (Primary TLV=%d) received from %s",
               dso->primary.opcode, dso->remote_name);
		break;

	case kDSOEventType_Finalize:
		INFO("dns_push_callback: Finalize");
		break;

	case kDSOEventType_Connected:
		INFO("dns_push_callback: Connected to %s", dso->remote_name);
		break;

	case kDSOEventType_ConnectFailed:
		INFO("dns_push_callback: Connection to %s failed", dso->remote_name);
		break;

	case kDSOEventType_Disconnected:
		INFO("dns_push_callback: Connection to %s disconnected", dso->remote_name);
		break;
	}
}

void
dp_dns_query(comm_t *comm, dns_rr_t *question)
{
    int rcode;
    dnssd_query_t *query = dp_query_generate(comm, question, false, &rcode);
    const char *failnote = NULL;
    if (!query) {
        dp_simple_response(comm, rcode);
        return;
    }

    // For regular DNS queries, copy the ID, etc.
    query->response.id = comm->message->wire.id;
    query->response.bitfield = comm->message->wire.bitfield;
    dns_rcode_set(&query->response, dns_rcode_noerror);

    // For DNS queries, we need to return the question.
    query->response.qdcount = htons(1);
    if (query->enclosing_domain != NULL) {
        TOWIRE_CHECK("name", &query->towire, dns_name_to_wire(NULL, &query->towire, query->name));
        TOWIRE_CHECK("enclosing_domain", &query->towire,
                     dns_full_name_to_wire(&query->enclosing_domain_pointer,
                                           &query->towire, query->enclosing_domain));
    } else {
        TOWIRE_CHECK("full name", &query->towire, dns_full_name_to_wire(NULL, &query->towire, query->name));
    }        
    TOWIRE_CHECK("TYPE", &query->towire, dns_u16_to_wire(&query->towire, question->type));    // TYPE
    TOWIRE_CHECK("CLASS", &query->towire, dns_u16_to_wire(&query->towire, question->qclass));  // CLASS
    if (failnote != NULL) {
        ERROR("dp_dns_query: failure encoding question: %s", failnote);
        goto fail;
    }
    
    // We should check for OPT RR, but for now assume it's there.
    query->is_edns0 = true;

    if (!dp_query_start(comm, query, &rcode, dns_query_callback)) {
    fail:
        dp_simple_response(comm, rcode);
        free(query->name);
        free(query);
        return;
    }
    
    // XXX make sure that finalize frees this.
    query->question = comm->message;
    comm->message = NULL;
}

void dso_transport_finalize(comm_t *comm)
{
    dso_state_t *dso = comm->dso;
    INFO("dso_transport_finalize: %s", dso->remote_name);
    if (comm) {
        ioloop_close(&comm->io);
    }
    free(dso);
    comm->dso = NULL;
}

void dns_evaluate(comm_t *comm)
{
    dns_rr_t question;
    unsigned offset = 0;

    // Drop incoming responses--we're a server, so we only accept queries.
    if (dns_qr_get(&comm->message->wire) == dns_qr_response) {
        return;
    }

    // If this is a DSO message, see if we have a session yet.
    switch(dns_opcode_get(&comm->message->wire)) {
    case dns_opcode_dso:
        if (!comm->tcp_stream) {
            ERROR("DSO message received on non-tcp socket %s", comm->name);
            dp_simple_response(comm, dns_rcode_notimp);
            return;
        }
        
        if (!comm->dso) {
            comm->dso = dso_create(true, 0, comm->name, dns_push_callback, comm, comm);
            if (!comm->dso) {
                ERROR("Unable to create a dso context for %s", comm->name);
                dp_simple_response(comm, dns_rcode_servfail);
                ioloop_close(&comm->io);
                return;
            }
            comm->dso->transport_finalize = dso_transport_finalize;
        }
        dso_message_received(comm->dso, (uint8_t *)&comm->message->wire, comm->message->length);
        break;

    case dns_opcode_query:
        // In theory this is permitted but it can't really be implemented because there's no way
        // to say "here's the answer for this, and here's why that failed.
        if (ntohs(comm->message->wire.qdcount) != 1) {
            dp_simple_response(comm, dns_rcode_formerr);
            return;
        }
        if (!dns_rr_parse(&question, comm->message->wire.data, comm->message->length, &offset, 0)) {
            dp_simple_response(comm, dns_rcode_formerr);
            return;
        }
        dp_dns_query(comm, &question);
        dns_rrdata_free(&question);
        break;

        // No support for other opcodes yet.
    default:
        dp_simple_response(comm, dns_rcode_notimp);
        break;
    }
}

void dns_input(comm_t *comm)
{
    dns_evaluate(comm);
    if (comm->message != NULL) {
        message_free(comm->message);
        comm->message = NULL;
    }
}

static int
usage(const char *progname)
{
    ERROR("usage: %s", progname);
    ERROR("ex: dnssd-proxy");
    return 1;
}

// Called whenever we get a connection.
void
connected(comm_t *comm)
{
    INFO("connection from %s", comm->name);
    return;
}

int
main(int argc, char **argv)
{
    int i;
    int16_t port;
    comm_t *tcp4_listener;
    comm_t *udp4_listener;

    port = htons(53);

    // Read the configuration from the command line.
    for (i = 1; i < argc; i++) {
        return usage(argv[0]);
    }

    if (!ioloop_init()) {
        return 1;
    }

    // Set up hardwired answers
    dnssd_hardwired_setup();

    // XXX Support IPv6!
    tcp4_listener = setup_listener_socket(AF_INET, IPPROTO_TCP, port, "IPv4 DNS Push Listener", dns_input, connected, 0);
    if (tcp4_listener == NULL) {
        ERROR("TCPv4 listener: fail.");
        return 1;
    }
    
    udp4_listener = setup_listener_socket(AF_INET, IPPROTO_UDP, port, "IPv4 DNS UDP Listener", dns_input, 0, 0);
    if (udp4_listener == NULL) {
        ERROR("UDP4 listener: fail.");
        return 1;
    }
    
    do {
        int something = 0;
        something = ioloop_events(0);
        INFO("dispatched %d events.", something);
    } while (1);
}

// Local Variables:
// mode: C
// tab-width: 4
// c-file-style: "bsd"
// c-basic-offset: 4
// fill-column: 108
// indent-tabs-mode: nil
// End:
