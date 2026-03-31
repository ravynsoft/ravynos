/* srp-simple.c
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
 * Simple Service Registration Protocol Client
 *
 * This is intended for the constrained node solution for SRP.   It's intended to be flexible and
 * understandable while linking in the minimum possible support code to reduce code size.  It does
 * no mallocs, does not put anything big on the stack, and doesn't require an event loop.
 */

#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#include "srp.h"
#include "dns-msg.h"
#include "srp-crypto.h"

static void
dns_response_callback(dns_transaction_t *txn)
{
}

int
main(int argc, char **argv)
{
    const char *host_name = "thread-demo";
    const char *zone_name = "default.service.arpa";
    const char *host_fqdn = "thread-demo.default.service.arpa";
    const char *service_type = "_ipps._tcp";
    const char *a_record = "127.0.0.1";
    const char *aaaa_record = "::1";
    const char *txt_record = "0";
    const char *anycast_address = "127.0.0.1";
//    const char *anycast_address = "73.186.137.119"; // cer.fugue.com
    const char *keyfile_name = "srp-simple.key";
    int port = 9992;
    srp_key_t *key;
    dns_wire_t message, response;
    uint16_t key_tag;
    static dns_transaction_t txn;
    dns_towire_state_t *towire = &txn.towire;
    dns_name_pointer_t p_host_name;
    dns_name_pointer_t p_zone_name;
    dns_name_pointer_t p_service_name;
    dns_name_pointer_t p_service_instance_name;
    int line;

    key = srp_load_keypair(keyfile_name);
    if (key == NULL) {
        key = srp_generate_key();
        if (key == NULL) {
            printf("Unable to load or generate a key.");
            exit(1);
        }
        if (!srp_write_key_to_file(keyfile_name, key)) {
            printf("Unable to safe generated key.");
            exit(1);
        }
    }

#define CH if (towire->error) { line = __LINE__; goto fail; }

    // Generate a random UUID.
#ifdef NOTYET
    message.id = srp_random16();
#else
    srandomdev();
    message.id = (uint32_t)(random()) & 65535;
#endif
    message.bitfield = 0;
    dns_qr_set(&message, dns_qr_query);
    dns_opcode_set(&message, dns_opcode_update);

    // Message data...
    memset(&txn, 0, sizeof txn);
    towire->p = &message.data[0];  // We start storing RR data here.
    towire->lim = &message.data[DNS_DATA_SIZE]; // This is the limit to how much we can store.
    towire->message = &message;
    txn.response = &response;
    txn.response_length = (int)(sizeof response);

    message.qdcount = htons(1); // ZOCOUNT = 1
    // Copy in Zone name (and save pointer)
    // ZTYPE = SOA
    // ZCLASS = IN
    dns_full_name_to_wire(&p_zone_name, towire, zone_name); CH;
    dns_u16_to_wire(towire, dns_rrtype_soa); CH;
    dns_u16_to_wire(towire, dns_qclass_in); CH;

    message.ancount = 0;
    // PRCOUNT = 0

    message.nscount = 0;
    // UPCOUNT = ...

    // Host Description:
    //  * Delete all RRsets from <hostname>; remember the pointer to hostname
    //      NAME = hostname label followed by pointer to SOA name.
    //      TYPE = ANY
    //      CLASS = ANY
    //      TTL = 0
    //      RDLENGTH = 0
    dns_name_to_wire(&p_host_name, towire, host_name); CH;
    dns_pointer_to_wire(&p_host_name, towire, &p_zone_name); CH;
    dns_u16_to_wire(towire, dns_rrtype_any); CH;
    dns_u16_to_wire(towire, dns_qclass_any); CH;
    dns_ttl_to_wire(towire, 0); CH;
    dns_u16_to_wire(towire, 0); CH;
    message.nscount++;
    //  * Add either or both of an A or AAAA RRset, each of which contains one
    //    or more A or AAAA RRs.
    //      NAME = pointer to hostname from Delete (above)
    //      TYPE = A or AAAA
    //      CLASS = IN
    //      TTL = 3600 ?
    //      RDLENGTH = number of RRs * RR length (4 or 16)
    //      RDATA = <the data>
    dns_pointer_to_wire(NULL, towire, &p_host_name); CH;
    dns_u16_to_wire(towire, dns_rrtype_a); CH;
    dns_u16_to_wire(towire, dns_qclass_in); CH;
    dns_ttl_to_wire(towire, 3600); CH;
    dns_rdlength_begin(towire); CH;
    dns_rdata_a_to_wire(towire, a_record); CH;
    dns_rdlength_end(towire); CH;
    message.nscount++;
    
    dns_pointer_to_wire(NULL, towire, &p_host_name); CH;
    dns_u16_to_wire(towire, dns_rrtype_aaaa); CH;
    dns_u16_to_wire(towire, dns_qclass_in); CH;
    dns_ttl_to_wire(towire, 3600); CH;
    dns_rdlength_begin(towire); CH;
    dns_rdata_aaaa_to_wire(towire, aaaa_record); CH;
    dns_rdlength_end(towire); CH;
    message.nscount++;
    
    //  * Exactly one KEY RR:
    //      NAME = pointer to hostname from Delete (above)
    //      TYPE = KEY
    //      CLASS = IN
    //      TTL = 3600
    //      RDLENGTH = length of key + 4 (32 bits)
    //      RDATA = <flags(16) = 0000 0010 0000 0001, protocol(8) = 3, algorithm(8) = 8?, public key(variable)>
    dns_pointer_to_wire(NULL, towire, &p_host_name); CH;
    dns_u16_to_wire(towire, dns_rrtype_key); CH;
    dns_u16_to_wire(towire, dns_qclass_in); CH;
    dns_ttl_to_wire(towire, 3600); CH;
    dns_rdlength_begin(towire); CH;
    key_tag = dns_rdata_key_to_wire(towire, 0, 2, 1, key); CH;
    dns_rdlength_end(towire); CH;
    message.nscount++;

    // Service Discovery:
    //   * Update PTR RR
    //     NAME = service name (_a._b.service.arpa)
    //     TYPE = PTR
    //     CLASS = IN
    //     TTL = 3600
    //     RDLENGTH = 2
    //     RDATA = service instance name
    dns_name_to_wire(&p_service_name, towire, service_type); CH;
    dns_pointer_to_wire(&p_service_name, towire, &p_zone_name); CH;
    dns_u16_to_wire(towire, dns_rrtype_ptr); CH;
    dns_u16_to_wire(towire, dns_qclass_in); CH;
    dns_ttl_to_wire(towire, 3600); CH;
    dns_rdlength_begin(towire); CH;
    dns_name_to_wire(&p_service_instance_name, towire, host_name); CH;
    dns_pointer_to_wire(&p_service_instance_name, towire, &p_service_name); CH;
    dns_rdlength_end(towire); CH;
    message.nscount++;

    // Service Description:
    //   * Delete all RRsets from service instance name
    //      NAME = service instance name (save pointer to service name, which is the second label)
    //      TYPE = ANY
    //      CLASS = ANY
    //      TTL = 0
    //      RDLENGTH = 0
    dns_pointer_to_wire(NULL, towire, &p_service_instance_name); CH;
    dns_u16_to_wire(towire, dns_rrtype_any); CH;
    dns_u16_to_wire(towire, dns_qclass_any); CH;
    dns_ttl_to_wire(towire, 0); CH;
    dns_u16_to_wire(towire, 0); CH;
    message.nscount++;

    //   * Add one SRV RRset pointing to Host Description
    //      NAME = pointer to service instance name from above
    //      TYPE = SRV
    //      CLASS = IN
    //      TTL = 3600
    //      RDLENGTH = 8
    //      RDATA = <priority(16) = 0, weight(16) = 0, port(16) = service port, target = pointer to hostname>
    dns_pointer_to_wire(NULL, towire, &p_service_instance_name); CH;
    dns_u16_to_wire(towire, dns_rrtype_srv); CH;
    dns_u16_to_wire(towire, dns_qclass_in); CH;
    dns_ttl_to_wire(towire, 3600); CH;
    dns_rdlength_begin(towire); CH;
    dns_u16_to_wire(towire, 0); // priority CH;
    dns_u16_to_wire(towire, 0); // weight CH;
    dns_u16_to_wire(towire, port); // port CH;
    dns_pointer_to_wire(NULL, towire, &p_host_name); CH;
    dns_rdlength_end(towire); CH;
    message.nscount++;

    //   * Add one or more TXT records
    //      NAME = pointer to service instance name from above
    //      TYPE = TXT
    //      CLASS = IN
    //      TTL = 3600
    //      RDLENGTH = <length of text>
    //      RDATA = <text>
    dns_pointer_to_wire(NULL, towire, &p_service_instance_name); CH;
    dns_u16_to_wire(towire, dns_rrtype_txt); CH;
    dns_u16_to_wire(towire, dns_qclass_in); CH;
    dns_ttl_to_wire(towire, 3600); CH;
    dns_rdlength_begin(towire); CH;
    dns_rdata_txt_to_wire(towire, txt_record); CH;
    dns_rdlength_end(towire); CH;
    message.nscount++;
    message.nscount = htons(message.nscount);
    
    // What about services with more than one name?   Are these multiple service descriptions?

    // ARCOUNT = 2
    //   EDNS(0) options
    //     ...
    //   SIG(0)
    
    message.arcount = htons(1);
    dns_edns0_header_to_wire(towire, DNS_MAX_UDP_PAYLOAD, 0, 0, 1); CH;   // XRCODE = 0; VERSION = 0; DO=1
    dns_rdlength_begin(towire); CH;
    dns_u16_to_wire(towire, dns_opt_update_lease); CH;  // OPTION-CODE
    dns_edns0_option_begin(towire); CH;                 // OPTION-LENGTH
    dns_u32_to_wire(towire, 3600); CH;                  // LEASE (1 hour)
    dns_u32_to_wire(towire, 604800); CH;                // KEY-LEASE (7 days)
    dns_edns0_option_end(towire); CH;                   // Now we know OPTION-LENGTH
    dns_rdlength_end(towire); CH;

    dns_sig0_signature_to_wire(towire, key, key_tag, &p_host_name, host_fqdn); CH;
    // The signature is computed before counting the signature RR in the header counts.
    message.arcount = htons(ntohs(message.arcount) + 1);

    // Send the update
    if (dns_send_to_server(&txn, anycast_address, 53, dns_response_callback) < 0) {
        line = __LINE__;
    fail:
        printf("dns_send_to_server failed: %s at line %d\n", strerror(towire->error), line);
    }
}

// Local Variables:
// mode: C
// tab-width: 4
// c-file-style: "bsd"
// c-basic-offset: 4
// fill-column: 108
// indent-tabs-mode: nil
// End:
