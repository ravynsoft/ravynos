/* srp-gw.c
 *
 * Copyright (c) 2018 Apple Computer, Inc. All rights reserved.
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
 * This is a DNSSD Service Registration Protocol gateway.   The purpose of this is to make it possible
 * for SRP clients to update DNS servers that don't support SRP.
 *
 * The way it works is that this gateway listens on port ANY:53 and forwards either to another port on
 * the same host (not recommended) or to any port (usually 53) on a different host.   Requests are accepted
 * over both TCP and UDP in principle, but UDP requests should be from constrained nodes, and rely on
 * network topology for authentication.
 *
 * Note that this is not a full DNS proxy, so you can't just put it in front of a DNS server.
 */

// Get DNS server IP address
// Get list of permitted source subnets for TCP updates
// Get list of permitted source subnet/interface tuples for UDP updates
// Set up UDP listener
// Set up TCP listener (no TCP Fast Open)
// Event loop
// Transaction processing:
//   1. If UDP, validate that it's from a subnet that is valid for the interface on which it was received.
//   2. If TCP, validate that it's from a permitted subnet
//   3. Check that the message is a valid SRP update according to the rules
//   4. Check the signature
//   5. Do a DNS Update with prerequisites to prevent overwriting a host record with the same owner name but
//      a different key.
//   6. Send back the response

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

#include "srp.h"
#include "dns-msg.h"
#include "srp-crypto.h"
#include "ioloop.h"
#include "dnssd-proxy.h"

#pragma mark structures

typedef struct subnet subnet_t;
struct subnet {
    subnet_t *NULLABLE next;
    uint8_t preflen;
    uint8_t family;
    char bytes[8];
};

typedef struct udp_validator udp_validator_t;
struct udp_validator {
    udp_validator_t *NULLABLE next;
    char *NONNULL ifname;
    int ifindex;
    subnet_t *subnets;
};

static int
usage(const char *progname)
{
    ERROR("usage: %s -s <addr> <port> -t <subnet> ... -u <ifname> <subnet> ...", progname);
    ERROR("  -s can only appear once.");
    ERROR("  -t can only appear once, and is followed by one or more subnets.");
    ERROR("  -u can appear more than once, is followed by one interface name, and");
    ERROR("     one or more subnets.");
    ERROR("  <addr> is an IPv4 address or IPv6 address.");
    ERROR("  <port> is a UDP port number.");
    ERROR("  <subnet> is an IP address followed by a slash followed by the prefix width.");
    ERROR("  <ifname> is the printable name of the interface.");
    ERROR("ex: srp-gw -s 2001:DB8::1 53 -t 2001:DB8:1300::/48 -u en0 2001:DB8:1300:1100::/56");
    return 1;
}

typedef struct delete delete_t;
struct delete {
    delete_t *next;
    dns_name_t *name;
};

typedef struct dns_host_description dns_host_description_t;
struct dns_host_description {
    dns_name_t *name;
    dns_rr_t *a, *aaaa, *key;
    delete_t *delete;
    int num_instances;
};

typedef struct service_instance service_instance_t;
struct service_instance {
    service_instance_t *next;
    dns_host_description_t *host_description;
    dns_name_t *name;
    delete_t *delete;
    int num_instances;
    dns_rr_t *srv, *txt;
};

typedef struct service service_t;
struct service {
    service_t *next;
    service_instance_t *instance;
    dns_name_t *name;
    dns_rr_t *rr;
};

bool
srp_relay(comm_t *comm, dns_message_t *message)
{
    dns_name_t *update_zone;
    bool updating_services_dot_arpa = false;
    int i;
    dns_host_description_t *host_description = NULL;
    delete_t *deletes = NULL, *dp, **dpp = &deletes;
    service_instance_t *service_instances = NULL, *sip, **sipp = &service_instances;
    service_t *services = NULL, *sp, **spp = &services;
    dns_rr_t *signature;
    char namebuf[DNS_MAX_NAME_SIZE + 1], namebuf1[DNS_MAX_NAME_SIZE + 1];
    bool ret = false;
    struct timeval now;

    // Update requires a single SOA record as the question
    if (message->qdcount != 1) {
        ERROR("srp_relay: update received with qdcount > 1");
        return false;
    }

    // Update should contain zero answers.
    if (message->ancount != 0) {
        ERROR("srp_relay: update received with ancount > 0");
        return false;
    }

    if (message->questions[0].type != dns_rrtype_soa) {
        ERROR("srp_relay: update received with rrtype %d instead of SOA in question section.",
              message->questions[0].type);
        return false;
    }
    update_zone = message->questions[0].name;

    // What zone are we updating?
    if (dns_names_equal_text(update_zone, "services.arpa")) {
        updating_services_dot_arpa = true;
    }

    // Scan over the authority RRs; do the delete consistency check.  We can't do other consistency checks
    // because we can't assume a particular order to the records other than that deletes have to come before
    // adds.
    for (i = 0; i < message->nscount; i++) {
        dns_rr_t *rr = &message->authority[i];

        // If this is a delete for all the RRs on a name, record it in the list of deletes.
        if (rr->type == dns_rrtype_any && rr->qclass == dns_qclass_any && rr->ttl == 0) {
            for (dp = deletes; dp; dp = dp->next) {
                if (dns_names_equal(dp->name, rr->name)) {
                    ERROR("srp_relay: two deletes for the same name: %s",
                          dns_name_print(rr->name, namebuf, sizeof namebuf));
                    goto out;
                }
            }
            dp = calloc(sizeof *dp, 1);
            if (!dp) {
                ERROR("srp_relay: no memory.");
                goto out;
            }
            dp->name = rr->name;
            *dpp = dp;
            dpp = &dp->next;
        }

        // Otherwise if it's an A or AAAA record, it's part of a hostname entry.
        else if (rr->type == dns_rrtype_a || rr->type == dns_rrtype_aaaa || rr->type == dns_rrtype_key) {
            // Allocate the hostname record
            if (!host_description) {
                host_description = calloc(sizeof *host_description, 1);
                if (!host_description) {
                    ERROR("srp_relay: no memory");
                    goto out;
                }
            }

            // Make sure it's preceded by a deletion of all the RRs on the name.
            if (!host_description->delete) {
                for (dp = deletes; dp; dp = dp->next) {
                    if (dns_names_equal(dp->name, rr->name)) {
                        break;
                    }
                }
                if (dp == NULL) {
                    ERROR("srp_relay: ADD for hostname %s without a preceding delete.",
                          dns_name_print(rr->name, namebuf, sizeof namebuf));
                    goto out;
                }
                host_description->delete = dp;
                host_description->name = dp->name;
            }
                          
            if (rr->type == dns_rrtype_a) {
                if (host_description->a != NULL) {
                    ERROR("srp_relay: more than one A rrset received for name: %s",
                          dns_name_print(rr->name, namebuf, sizeof namebuf));
                    goto out;
                }
                host_description->a = rr;
            } else if (rr->type == dns_rrtype_aaaa) {
                if (host_description->aaaa != NULL) {
                    ERROR("srp_relay: more than one AAAA rrset received for name: %s",
                          dns_name_print(rr->name, namebuf, sizeof namebuf));
                    goto out;
                }
                host_description->aaaa = rr;
            } else if (rr->type == dns_rrtype_key) {
                if (host_description->key != NULL) {
                    ERROR("srp_relay: more than one KEY rrset received for name: %s",
                          dns_name_print(rr->name, namebuf, sizeof namebuf));
                    goto out;
                }
                host_description->key =  rr;
            }
        }

        // Otherwise if it's an SRV entry, that should be a service instance name.
        else if (rr->type == dns_rrtype_srv || rr->type == dns_rrtype_txt) {
            // Should be a delete that precedes this service instance.
            for (dp = deletes; dp; dp = dp->next) {
                if (dns_names_equal(dp->name, rr->name)) {
                    break;
                }
            }
            if (dp == NULL) {
                ERROR("srp_relay: ADD for service instance not preceded by delete: %s",
                      dns_name_print(rr->name, namebuf, sizeof namebuf));
                goto out;
            }
            for (sip = service_instances; sip; sip = sip->next) {
                if (dns_names_equal(sip->name, rr->name)) {
                    break;
                }
            }
            if (!sip) {
                sip = calloc(sizeof *sip, 1);
                if (sip == NULL) {
                    ERROR("srp_relay: no memory");
                    goto out;
                }
                sip->delete = dp;
                sip->name = dp->name;
                *sipp = sip;
                sipp = &sip->next;
            }
            if (rr->type == dns_rrtype_srv) {
                if (sip->srv != NULL) {
                    ERROR("srp_relay: more than one SRV rr received for service instance: %s",
                          dns_name_print(rr->name, namebuf, sizeof namebuf));
                    goto out;
                }
                sip->srv = rr;
            } else if (rr->type == dns_rrtype_txt) {
                if (sip->txt != NULL) {
                    ERROR("srp_relay: more than one SRV rr received for service instance: %s",
                          dns_name_print(rr->name, namebuf, sizeof namebuf));
                }
                sip->txt = rr;
            }
        }

        // Otherwise if it's a PTR entry, that should be a service name
        else if (rr->type == dns_rrtype_ptr) {
            sp = calloc(sizeof *sp, 1);
            if (sp == NULL) {
                ERROR("srp_relay: no memory");
                goto out;
            }
            sp->rr = rr;
            *spp = sp;
            spp = &sp->next;
        }            

        // Otherwise it's not a valid update
        else {
            ERROR("srp_relay: unexpected rrtype %d on %s in update.", rr->type,
                      dns_name_print(rr->name, namebuf, sizeof namebuf));
            goto out;
        }
    }

    // Now that we've scanned the whole update, do the consistency checks for updates that might
    // not have come in order.
    
    // First, make sure there's a host description.
    if (host_description == NULL) {
        ERROR("srp_relay: SRP update does not include a host description.");
        goto out;
    }

    // Make sure that each service add references a service instance that's in the same update.
    for (sp = services; sp; sp = sp->next) {
        for (sip = service_instances; sip; sip = sip->next) {
            if (dns_names_equal(sip->name, sp->rr->data.ptr.name)) {
                // Note that we have already verified that there is only one service instance
                // with this name, so this could only ever happen once in this loop even without
                // the break statement.
                sp->instance = sip;
                sip->num_instances++;
                break;
            }
        }
        // If this service doesn't point to a service instance that's in the update, then the
        // update fails validation.
        if (sip == NULL) {
            ERROR("srp_relay: service %s points to an instance that's not included: %s",
                  dns_name_print(sp->name, namebuf, sizeof namebuf),
                  dns_name_print(sip->name, namebuf1, sizeof namebuf1));
            goto out;
        }
    }

    for (sip = service_instances; sip; sip = sip->next) {
        // For each service instance, make sure that at least one service references it
        if (sip->num_instances == 0) {
            ERROR("srp_relay: service instance update for %s is not referenced by a service update.",
                  dns_name_print(sip->name, namebuf, sizeof namebuf));
            goto out;
        }

        // For each service instance, make sure that it references the host description
        if (dns_names_equal(host_description->name, sip->srv->data.srv.name)) {
            sip->host_description = host_description;
            host_description->num_instances++;
        }
    }

    // Make sure that at least one service instance references the host description
    if (host_description->num_instances == 0) {
        ERROR("srp_relay: host description %s is not referenced by any service instances.",
              dns_name_print(host_description->name, namebuf, sizeof namebuf));
        goto out;
    }

    // Make sure the host description has at least one address record.
    if (host_description->a == NULL && host_description->aaaa == NULL) {
        ERROR("srp_relay: host description %s doesn't contain any IP addresses.",
              dns_name_print(host_description->name, namebuf, sizeof namebuf));
        goto out;
    }
    // And make sure it has a key record
    if (host_description->key == NULL) {
        ERROR("srp_relay: host description %s doesn't contain a key.",
              dns_name_print(host_description->name, namebuf, sizeof namebuf));
        goto out;
    }

    // The signature should be the last thing in the additional section.   Even if the signature
    // is valid, if it's not at the end we reject it.   Note that we are just checking for SIG(0)
    // so if we don't find what we're looking for, we forward it to the DNS auth server which
    // will either accept or reject it.
    if (message->arcount < 1) {
        ERROR("srp_relay: signature not present");
        goto out;
    }
    signature = &message->additional[message->arcount -1];
    if (signature->type != dns_rrtype_sig) {
        ERROR("srp_relay: signature is not at the end or is not present");
        goto out;
    }

    // Make sure that the signer name is the hostname.   If it's not, it could be a legitimate
    // update with a different key, but it's not an SRP update, so we pass it on.
    if (!dns_names_equal(signature->data.sig.signer, host_description->name)) {
        ERROR("srp_relay: signer %s doesn't match host %s", 
              dns_name_print(signature->data.sig.signer, namebuf, sizeof namebuf),
              dns_name_print(host_description->name, namebuf1, sizeof namebuf1));
        goto out;
    }
    
    // Make sure we're in the time limit for the signature.   Zeroes for the inception and expiry times
    // mean the host that send this doesn't have a working clock.   One being zero and the other not isn't
    // valid unless it's 1970.
    if (signature->data.sig.inception != 0 || signature->data.sig.expiry != 0) {
        gettimeofday(&now, NULL);
        // The sender does the bracketing, so we can just do a simple comparison.
        if (now.tv_sec > signature->data.sig.expiry || now.tv_sec < signature->data.sig.inception) {
            ERROR("signature is not timely: %lu < %lu < %lu does not hold",
                  (unsigned long)signature->data.sig.inception, (unsigned long)now.tv_sec,
                  (unsigned long)signature->data.sig.expiry);
            goto badsig;
        }
    }

    // Now that we have the key, we can validate the signature.   If the signature doesn't validate,
    // there is no need to pass the message on.
    if (!srp_sig0_verify(message->wire, host_description->key, signature)) {
        ERROR("signature is not valid");
        goto badsig;
    }

badsig:
    // True means we consumed it, not that it was valid.
    ret = true;

out:
    // free everything we allocated but (it turns out) aren't going to use
    for (dp = deletes; dp; ) {
        delete_t *next = dp->next;
        free(dp);
        dp = next;
    }
    for (sip = service_instances; sip; ) {
        service_instance_t *next = sip->next;
        free(sip);
        sip = next;
    }
    for (sp = services; sp; ) {
        service_t *next = sp->next;
        free(sp);
        sp = next;
    }
    if (host_description != NULL) {
        free(host_description);
    }
    return ret;
}

void
dns_evaluate(comm_t *comm)
{
    dns_message_t *message;

    // Drop incoming responses--we're a server, so we only accept queries.
    if (dns_qr_get(&comm->message->wire) == dns_qr_response) {
        return;
    }

    // Forward incoming messages that are queries but not updates.
    // XXX do this later--for now we operate only as a translator, not a proxy.
    if (dns_opcode_get(&comm->message->wire) != dns_opcode_update) {
        return;
    }
    
    // Parse the UPDATE message.
    if (!dns_wire_parse(&message, &comm->message->wire, comm->message->length)) {
        ERROR("dns_wire_parse failed.");
        return;
    }
    
    // We need the wire message to validate the signature...
    message->wire = &comm->message->wire;
    if (!srp_relay(comm, message)) {
        // The message wasn't invalid, but wasn't an SRP message.
        // dns_forward(comm)
    }
    // But we don't save it.
    message->wire = NULL;

    //dns_message_free(message);
}

void dns_input(comm_t *comm)
{
    dns_evaluate(comm);
    message_free(comm->message);
    comm->message = NULL;
}

int
main(int argc, char **argv)
{
    int i;
    subnet_t *tcp_validators = NULL;
    udp_validator_t *udp_validators = NULL;
    udp_validator_t *NULLABLE *NONNULL up = &udp_validators;
    subnet_t *NULLABLE *NONNULL nt = &tcp_validators;
    subnet_t *NULLABLE *NONNULL sp;
    addr_t server, pref;
    uint16_t port;
    socklen_t len, prefalen;
    char *s, *p;
    int width;
    uint16_t listen_port;

    listen_port = htons(53);

    // Read the configuration from the command line.
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-s")) {
            if (i++ == argc) {
                ERROR("-s is missing server IP address.");
                return usage(argv[0]);
            }
            len = getipaddr(&server, argv[i]);
            if (!len) {
                ERROR("Invalid IP address: %s.", argv[i]);
                return usage(argv[0]);
            }
            server.sa.sa_len = len;
            if (i++ == argc) {
                ERROR("-s is missing server port.");
                return usage(argv[0]);
            }
            port = strtol(argv[i], &s, 10);
            if (s == argv[i] || s[0] != '\0') {
                ERROR("Invalid port number: %s", argv[i]);
                return usage(argv[0]);
            }
            if (server.sa.sa_family == AF_INET) {
                server.sin.sin_port = htons(port);
            } else {
                server.sin6.sin6_port = htons(port);
            }
            i += 2;
        } else if (!strcmp(argv[i], "-t") || !strcmp(argv[i], "-u")) {
            if (!strcmp(argv[i], "-u")) {
                if (i++ == argc) {
                    ERROR("-u is missing interface name.");
                    return usage(argv[0]);
                }
                *up = calloc(1, sizeof **up);
                if (*up == NULL) {
                    ERROR("udp_validators: out of memory.");
                    return usage(argv[0]);
                }
                (*up)->ifname = strdup(argv[i]);
                if ((*up)->ifname == NULL) {
                    ERROR("udp validators: ifname: out of memory.");
                    return usage(argv[0]);
                }
                sp = &((*up)->subnets);
            } else {
                sp = nt;
            }

            if (i++ == argc) {
                ERROR("%s requires at least one prefix.", argv[i - 1]);
                return usage(argv[0]);
            }
            s = strchr(argv[i], '/');
            if (s == NULL) {
                ERROR("%s is not a prefix.", argv[i]);
                return usage(argv[0]);
            }
            *s = 0;
            ++s;
            prefalen = getipaddr(&pref, argv[i]);
            if (!prefalen) {
                ERROR("%s is not a valid prefix address.", argv[i]);
                return usage(argv[0]);
            }
            width = strtol(s, &p, 10);
            if (s == p || p[0] != '\0') {
                ERROR("%s (prefix width) is not a number.", p);
                return usage(argv[0]);
            }
            if (width < 0 ||
                (pref.sa.sa_family == AF_INET && width > 32) ||
                (pref.sa.sa_family == AF_INET6 && width > 64)) {
                ERROR("%s is not a valid prefix length for %s", p,
                        pref.sa.sa_family == AF_INET ? "IPv4" : "IPv6");
                return usage(argv[0]);
            }

            *nt = calloc(1, sizeof **nt);
            if (!*nt) {
                ERROR("tcp_validators: out of memory.");
                return 1;
            }

            (*nt)->preflen = width;
            (*nt)->family = pref.sa.sa_family;
            if (pref.sa.sa_family == AF_INET) {
                memcpy((*nt)->bytes, &pref.sin.sin_addr, 4);
            } else {
                memcpy((*nt)->bytes, &pref.sin6.sin6_addr, 8);
            }

            // *up will be non-null for -u and null for -t.
            if (*up) {
                up = &((*up)->next);
            } else {
                nt = sp;
            }
        }
    }

    if (!ioloop_init()) {
        return 1;
    }

    // Set up listeners
    if (!setup_listener_socket(AF_INET, IPPROTO_UDP, listen_port, "UDPv4 listener", dns_input, 0, 0)) {
        ERROR("UDPv4 listener: fail.");
        return 1;
    }
    if (!setup_listener_socket(AF_INET6, IPPROTO_UDP, listen_port, "UDPv6 listener", dns_input, 0, 0)) {
        ERROR("UDPv6 listener: fail.");
        return 1;
    }
    if (!setup_listener_socket(AF_INET, IPPROTO_TCP, listen_port, "TCPv4 listener", dns_input, 0, 0)) {
        ERROR("TCPv4 listener: fail.");
        return 1;
    }
    if (!setup_listener_socket(AF_INET6, IPPROTO_TCP, listen_port, "TCPv6 listener", dns_input, 0, 0)) {
        ERROR("TCPv4 listener: fail.");
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
