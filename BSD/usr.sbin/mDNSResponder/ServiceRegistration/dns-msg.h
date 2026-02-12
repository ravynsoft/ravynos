/* dns-msg.h
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
 * Lightweight framework for generating, sending, and unpacking DNS messages.
 * Definitions...
 */

#ifndef __DNS_MSG_H
#define __DNS_MSG_H

#ifndef DNS_MAX_UDP_PAYLOAD
#define DNS_MAX_UDP_PAYLOAD 1410
#endif

#define DNS_HEADER_SIZE           12
#define DNS_DATA_SIZE             (DNS_MAX_UDP_PAYLOAD - DNS_HEADER_SIZE)
#define DNS_MAX_POINTER           ((2 << 14) - 1)
#define DNS_MAX_LABEL_SIZE        63
#define DNS_MAX_NAME_SIZE	      255
#define DNS_MAX_NAME_SIZE_ESCAPED 1009

typedef struct dns_wire dns_wire_t;
struct dns_wire {
    uint16_t id;
    uint16_t bitfield;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
    uint8_t data[DNS_DATA_SIZE];
};

typedef struct dns_towire_state dns_towire_state_t;
struct dns_towire_state {
 	dns_wire_t *NULLABLE message;
    uint8_t *NONNULL p;
    uint8_t *NONNULL lim;
    uint8_t *NULLABLE p_rdlength;
    uint8_t *NULLABLE p_opt;
    int error;
};

typedef struct dns_transaction dns_transaction_t;
struct dns_transaction {
    dns_transaction_t *NULLABLE next;
    dns_towire_state_t towire;
    dns_wire_t *NULLABLE response;
    int response_length;
    int sock;
};    

typedef struct dns_name_pointer dns_name_pointer_t;
struct dns_name_pointer {
    uint8_t *NONNULL message_start;
    uint8_t *NONNULL name_start;
    int num_labels;
    int length;
};

typedef void (*dns_response_callback_t)(dns_transaction_t *NONNULL txn);

typedef struct dns_label dns_label_t;
typedef dns_label_t dns_name_t;
struct dns_label {
    dns_label_t *NULLABLE next;
    uint8_t len;
    char data[DNS_MAX_LABEL_SIZE];
};

typedef struct dns_txt_element dns_txt_element_t;
struct dns_txt_element {
    dns_txt_element_t *NULLABLE next;
    uint8_t len;
    char data[0];
};

typedef struct dns_rdata_unparsed dns_rdata_unparsed_t;
struct dns_rdata_unparsed {
    uint8_t *NULLABLE data;
    uint16_t len;
};

typedef struct dns_rdata_single_name dns_rdata_ptr_t;
typedef struct dns_rdata_single_name dns_rdata_cname_t;
struct dns_rdata_single_name {
    dns_label_t *NONNULL name;
};

typedef struct dns_rdata_a dns_rdata_a_t;
struct dns_rdata_a {
    struct in_addr *NONNULL addrs;
    int num;
};

typedef struct dns_rdata_aaaa dns_rdata_aaaa_t;
struct dns_rdata_aaaa {
    struct in6_addr *NONNULL addrs;
    int num;
} aaaa;

typedef struct dns_rdata_srv dns_rdata_srv_t;
struct dns_rdata_srv {
    dns_label_t *NONNULL name;
    uint16_t priority;
    uint16_t weight;
    uint16_t port;
} srv;

typedef struct dns_rdata_sig dns_rdata_sig_t;
struct dns_rdata_sig {
    uint16_t type;
    uint8_t algorithm;
    uint8_t label;
    uint32_t rrttl;
    uint32_t expiry;
    uint32_t inception;
    uint16_t key_tag;
    dns_label_t *NONNULL signer;
    int start;
    int len;
    uint8_t *NONNULL signature;
} sig;

typedef struct dns_rdata_key dns_rdata_key_t;
struct dns_rdata_key {
    uint16_t flags;
    uint8_t protocol;
    uint8_t algorithm;
    int len;
    uint8_t *NONNULL key;
} key;

typedef struct dns_rr dns_rr_t;
struct dns_rr {
    dns_label_t *NONNULL name;
    uint16_t type;
    uint16_t qclass;
    uint32_t ttl;
    union {
        dns_rdata_unparsed_t unparsed;
        dns_rdata_ptr_t ptr;
        dns_rdata_cname_t cname;
        dns_rdata_a_t a;
        dns_rdata_aaaa_t aaaa;
        dns_rdata_srv_t srv;
        dns_txt_element_t *NONNULL txt;
        dns_rdata_sig_t sig;
        dns_rdata_key_t key;
    } data;
};

typedef struct dns_edns0 dns_edns0_t;
struct dns_edns0 {
    dns_edns0_t *NULLABLE next;
    uint16_t length;
    uint8_t data[0];
};

typedef struct dns_message dns_message_t;
struct dns_message {
    dns_wire_t *NULLABLE wire;
    int qdcount, ancount, nscount, arcount;
    dns_rr_t *NULLABLE questions;
    dns_rr_t *NULLABLE answers;
    dns_rr_t *NULLABLE authority;
    dns_rr_t *NULLABLE additional;
    dns_edns0_t *NULLABLE edns0;
};

// Masks for bitfield data
#define dns_qr_mask     0x8000
#define dns_opcode_mask 0x7800
#define dns_flags_mask  0x07f0
#define dns_rcode_mask  0x000f

// Shifts for bitfield data
#define dns_qr_shift     15
#define dns_opcode_shift 11
#define dns_rcode_shift  0

// Booleans
#define dns_flags_aa 0x0400
#define dns_flags_tc 0x0200
#define dns_flags_rd 0x0100
#define dns_flags_ra 0x0080
#define dns_flags_ad 0x0020
#define dns_flags_cd 0x0010

// Getters
#define dns_qr_get(w) ((ntohs((w)->bitfield) & dns_qr_mask) >> dns_qr_shift)
#define dns_opcode_get(w) ((ntohs((w)->bitfield) & dns_opcode_mask) >> dns_opcode_shift)
#define dns_rcode_get(w) ((ntohs((w)->bitfield) & dns_rcode_mask) >> dns_rcode_shift)

// Setters
#define dns_qr_set(w, value) ((w)->bitfield = htons(((ntohs((w)->bitfield) & ~dns_qr_mask) | \
                                                     ((value) << dns_qr_shift))))
#define dns_opcode_set(w, value) ((w)->bitfield = htons(((ntohs((w)->bitfield) & ~dns_opcode_mask) | \
                                                         ((value) << dns_opcode_shift))))
#define dns_rcode_set(w, value) ((w)->bitfield = htons(((ntohs((w)->bitfield) & ~dns_rcode_mask) | \
                                                        ((value) << dns_rcode_shift))))

// Query/Response
#define dns_qr_query           0
#define dns_qr_response        1

// Opcodes
#define dns_opcode_query       0
#define dns_opcode_iquery      1
#define dns_opcode_status      2
#define dns_opcode_notify      4
#define dns_opcode_update      5
#define dns_opcode_dso         6

// Response Codes
#define dns_rcode_noerror      0 // [RFC1035] No Error
#define dns_rcode_formerr      1 // [RFC1035] Format Error
#define dns_rcode_servfail     2 // [RFC1035] Server Failure
#define dns_rcode_nxdomain     3 // [RFC1035] Non-Existent Domain
#define dns_rcode_notimp       4 // [RFC1035] Not Implemented
#define dns_rcode_refused      5 // [RFC1035] Query Refused
#define dns_rcode_yxdomain     6 // [RFC2136][RFC6672] Name Exists when it should not
#define dns_rcode_yxrrset      7 // [RFC2136] RR Set Exists when it should not
#define dns_rcode_nxrrset      8 // [RFC2136] RR Set that should exist does not
#define dns_rcode_notauth      9 // [RFC2136] Server Not Authoritative for zone, or [RFC2845] Not Authorized
#define dns_rcode_notzone     10 // [RFC2136] Name not contained in zone
#define dns_rcode_dsotypeni   11 // [RFCTBD draft-ietf-dnsop-session-signal] DSO-Type Not Implemented
#define dns_rcode_badvers     16 // [RFC6891] Bad OPT Version, or [RFC2845] TSIG Signature Failure
#define dns_rcode_badkey      17 // [RFC2845] Key not recognized
#define dns_rcode_badtime     18 // [RFC2845] Signature out of time window
#define dns_rcode_badmode     19 // [RFC2930] Bad TKEY Mode
#define dns_rcode_badname     20 // [RFC2930] Duplicate key name
#define dns_rcode_badalg      21 // [RFC2930] Algorithm not supported
#define dns_rcode_badtrunc    22 // [RFC4635] Bad Truncation
#define dns_rcode_badcookie   23 // [RFC7873] Bad/missing Server Cookie

#define dns_qclass_in          1 // [RFC1035] Internet (IN)
#define dns_qclass_chaos       3 // [D. Moon, "Chaosnet"] Chaosnet (MIT)
#define dns_qclass_hesiod      4 // [MIT Project Athena Technical Plan] Hesiod service
#define dns_qclass_none      254 // [RFC2136] NONE (delete, or not in use)
#define dns_qclass_any       255 // [RFC1035] ANY (wildcard)

#define dns_rrtype_a           1 // [RFC1035] a host address
#define dns_rrtype_ns          2 // [RFC1035] an authoritative name server
#define dns_rrtype_md          3 // [RFC1035] a mail destination (OBSOLETE - use MX)
#define dns_rrtype_mf          4 // [RFC1035] a mail forwarder (OBSOLETE - use MX)
#define dns_rrtype_cname       5 // [RFC1035] the canonical name for an alias
#define dns_rrtype_soa         6 // [RFC1035] marks the start of a zone of authority
#define dns_rrtype_mb          7 // [RFC1035] a mailbox domain name (EXPERIMENTAL)
#define dns_rrtype_mg          8 // [RFC1035] a mail group member (EXPERIMENTAL)
#define dns_rrtype_mr          9 // [RFC1035] a mail rename domain name (EXPERIMENTAL)
#define dns_rrtype_null       10 // [RFC1035]	 a null RR (EXPERIMENTAL)
#define dns_rrtype_wks        11 // [RFC1035]	 a well known service description
#define dns_rrtype_ptr        12 // [RFC1035]	 a domain name pointer
#define dns_rrtype_hinfo      13 // [RFC1035]	 host information
#define dns_rrtype_minfo      14 // [RFC1035]	 mailbox or mail list information
#define dns_rrtype_mx         15 // [RFC1035]	 mail exchange
#define dns_rrtype_txt        16 // [RFC1035] text strings
#define dns_rrtype_rp         17 // [RFC1183] for Responsible Person
#define dns_rrtype_afsdb      18 // [RFC1183,RFC5864] for AFS Data Base location
#define dns_rrtype_x25        19 // [RFC1183] for X.25 PSDN address
#define dns_rrtype_isdn       20 // [RFC1183] for ISDN address
#define dns_rrtype_rt         21 // [RFC1183] for Route Through
#define dns_rrtype_nsap       22 // [RFC1706] for NSAP address, NSAP style A record
#define dns_rrtype_nsap_ptr   23 // [RFC1348,RFC1637,RFC1706] for domain name pointer, NSAP style
#define dns_rrtype_sig        24 // [RFC4034,RFC3755,RFC2535,RFC2536,RFC2537,RFC2931,RFC3110,RFC3008]
#define dns_rrtype_key        25 // [RFC4034,RFC3755,RFC2535,RFC2536,RFC2537,RFC2539,RFC3008,RFC3110]
#define dns_rrtype_px         26 // [RFC2163] X.400 mail mapping information
#define dns_rrtype_gpos       27 // [RFC1712] Geographical Position
#define dns_rrtype_aaaa       28 // [RFC3596] IP6 Address
#define dns_rrtype_loc        29 // [RFC1876] Location Information
#define dns_rrtype_nxt        30 // [RFC3755] [RFC2535] Next Domain (OBSOLETE)
#define dns_rrtype_eid        31 // [http://ana-3.lcs.mit.edu/~jnc/nimrod/dns.txt] Endpoint Identifier
#define dns_rrtype_nimloc     32 // [http://ana-3.lcs.mit.edu/~jnc/nimrod/dns.txt] Nimrod Locator
#define dns_rrtype_srv        33 // [RFC2782] Server Selection
#define dns_rrtype_atma       34 // ["ATM Name System, V2.0"] ATM Address
#define dns_rrtype_naptr      35 // [RFC2915] [RFC2168] [RFC3403] Naming Authority Pointer
#define dns_rrtype_kx         36 // [RFC2230] Key Exchanger
#define dns_rrtype_cert       37 // [RFC4398] CERT
#define dns_rrtype_a6         38 // [RFC3226] [RFC2874] [RFC6563]	 A6 (OBSOLETE - use AAAA)
#define dns_rrtype_dname      39 // [RFC6672]
#define dns_rrtype_sink       40 // [http://tools.ietf.org/html/draft-eastlake-kitchen-sink]
#define dns_rrtype_opt        41 // [RFC6891] [RFC3225]
#define dns_rrtype_apl        42 // [RFC3123]
#define dns_rrtype_ds         43 // [RFC4034] [RFC3658] Delegation Signer
#define dns_rrtype_sshfp      44 // [RFC4255] SSH Key Fingerprint
#define dns_rrtype_ipseckey   45 // [RFC4025]
#define dns_rrtype_rrsig      46 // [RFC4034] [RFC3755]
#define dns_rrtype_nsec       47 // [RFC4034] [RFC3755]
#define dns_rrtype_dnskey     48 // [RFC4034] [RFC3755]
#define dns_rrtype_dhcid      49 // [RFC4701] DHCID
#define dns_rrtype_nsec3      50 // [RFC5155] NSEC3
#define dns_rrtype_nsec3param 51 // [RFC5155] NSEC3PARAM
#define dns_rrtype_tlsa       52 // [RFC6698] TLSA
#define dns_rrtype_smimea     53 // [RFC8162] S/MIME cert association
#define dns_rrtype_hip        55 // Host Identity Protocol
#define dns_rrtype_ninfo      56 // [Jim_Reid] NINFO/ninfo-completed-template
#define dns_rrtype_rkey       57 // [Jim_Reid] RKEY/rkey-completed-template
#define dns_rrtype_talink     58 // [Wouter_Wijngaards] Trust Anchor LINK
#define dns_rrtype_cds        59 // [RFC7344] Child DS
#define dns_rrtype_cdnskey    60 // [RFC7344]	DNSKEY(s) the Child wants reflected in DS
#define dns_rrtype_openpgpkey 61 // [RFC7929]	OpenPGP Key
#define dns_rrtype_csync      62 // [RFC7477] Child-To-Parent Synchronization
#define dns_rrtype_spf        99 // [RFC7208]
#define dns_rrtype_uinfo     100 // [IANA-Reserved]		
#define dns_rrtype_uid       101 // [IANA-Reserved]		
#define dns_rrtype_gid       102 // [IANA-Reserved]		
#define dns_rrtype_unspec    103 // [IANA-Reserved]		
#define dns_rrtype_nid       104 // [RFC6742]
#define dns_rrtype_l32       105 // [RFC6742]
#define dns_rrtype_l64       106 // [RFC6742]
#define dns_rrtype_lp        107 // [RFC6742]
#define dns_rrtype_eui48     108 // an EUI-48 address [RFC7043]
#define dns_rrtype_eui64     109 // an EUI-64 address [RFC7043]
#define dns_rrtype_tkey      249 // Transaction Key	[RFC2930]		
#define dns_rrtype_tsig      250 // Transaction Signature [RFC2845]		
#define dns_rrtype_ixfr      251 // incremental transfer	[RFC1995]		
#define dns_rrtype_axfr      252 // transfer of an entire zone [RFC1035][RFC5936]		
#define dns_rrtype_mailb     253 // mailbox-related RRs (MB, MG or MR) [RFC1035]		
#define dns_rrtype_maila     254 // mail agent RRs (OBSOLETE - see MX) [RFC1035]		
#define dns_rrtype_any       255 // A request for some or all records the server has available
#define dns_rrtype_uri       256 // URI	[RFC7553]	URI/uri-completed-template
#define dns_rrtype_caa       257 // Certification Authority Restriction [RFC6844]
#define dns_rrtype_avc       258 // Application Visibility and Control [Wolfgang_Riedel]
#define dns_rrtype_doa       259 // Digital Object Architecture [draft-durand-doa-over-dns]

#define dns_opt_llq            1 // On-hold [http://files.dns-sd.org/draft-sekar-dns-llq.txt]
#define dns_opt_update_lease   2 // On-hold	[http://files.dns-sd.org/draft-sekar-dns-ul.txt]
#define dns_opt_nsid           3 // [RFC5001]
#define dns_opt_owner          4 // [draft-cheshire-edns0-owner-option]
#define dns_opt_dau            5 // [RFC6975]
#define dns_opt_dhu            6 // [RFC6975]
#define dns_opt_n3u            7 // [RFC6975]
#define dns_opt_client_subnet  8 // [RFC7871]
#define dns_opt_expire         9 // [RFC7314]
#define dns_opt_cookie        10 // [RFC7873]
#define dns_opt_keepalive     11 // [RFC7828]
#define dns_opt_padding       12 // [RFC7830]
#define dns_opt_chain         13 // [RFC7901]
#define dns_opt_key_tag       14 // [RFC8145]

// towire.c:

uint16_t srp_random16(void);
void dns_name_to_wire(dns_name_pointer_t *NULLABLE r_pointer,
                      dns_towire_state_t *NONNULL txn,
                      const char *NONNULL name);
void dns_full_name_to_wire(dns_name_pointer_t *NULLABLE r_pointer,
                           dns_towire_state_t *NONNULL txn,
                           const char *NONNULL name);
void dns_pointer_to_wire(dns_name_pointer_t *NULLABLE r_pointer,
                         dns_towire_state_t *NONNULL txn,
                         dns_name_pointer_t *NONNULL pointer);
void dns_u8_to_wire(dns_towire_state_t *NONNULL txn,
                    uint8_t val);
void dns_u16_to_wire(dns_towire_state_t *NONNULL txn,
                      uint16_t val);
void dns_u32_to_wire(dns_towire_state_t *NONNULL txn,
                      uint32_t val);
void dns_ttl_to_wire(dns_towire_state_t *NONNULL txn,
                     int32_t val);
void dns_rdlength_begin(dns_towire_state_t *NONNULL txn);
void dns_rdlength_end(dns_towire_state_t *NONNULL txn);
void dns_rdata_a_to_wire(dns_towire_state_t *NONNULL txn,
                         const char *NONNULL ip_address);
void dns_rdata_aaaa_to_wire(dns_towire_state_t *NONNULL txn,
                            const char *NONNULL ip_address);
uint16_t dns_rdata_key_to_wire(dns_towire_state_t *NONNULL txn,
                               unsigned key_type,
                               unsigned name_type,
                               unsigned signatory,
                               srp_key_t *NONNULL key);
void dns_rdata_txt_to_wire(dns_towire_state_t *NONNULL txn,
                           const char *NONNULL txt_record);
void dns_rdata_raw_data_to_wire(dns_towire_state_t *NONNULL txn, const void *NONNULL raw_data, size_t length);
void dns_edns0_header_to_wire(dns_towire_state_t *NONNULL txn,
                              int mtu,
                              int xrcode,
                              int version,
                              int DO);
void dns_edns0_option_begin(dns_towire_state_t *NONNULL txn);
void dns_edns0_option_end(dns_towire_state_t *NONNULL txn);
void dns_sig0_signature_to_wire(dns_towire_state_t *NONNULL txn,
                                srp_key_t *NONNULL key, uint16_t key_tag,
                                dns_name_pointer_t *NONNULL signer,
                                const char *NONNULL signer_fqdn);
int dns_send_to_server(dns_transaction_t *NONNULL txn,
                       const char *NONNULL anycast_address, uint16_t port,
                       dns_response_callback_t NONNULL callback);

// fromwire.c:
dns_label_t *NULLABLE dns_label_parse(const uint8_t *NONNULL buf, unsigned mlen, unsigned *NONNULL offp);
bool dns_opt_parse(dns_edns0_t *NONNULL *NULLABLE ret, dns_rr_t *NONNULL rrset);
bool dns_name_parse(dns_label_t *NONNULL *NULLABLE ret, const uint8_t *NONNULL buf, unsigned len,
                    unsigned *NONNULL offp, unsigned base);
bool dns_u8_parse(const uint8_t *NONNULL buf, unsigned len, unsigned *NONNULL offp, uint8_t *NONNULL ret);
bool dns_u16_parse(const uint8_t *NONNULL buf, unsigned len, unsigned *NONNULL offp, uint16_t *NONNULL ret);
bool dns_u32_parse(const uint8_t *NONNULL buf, unsigned len, unsigned *NONNULL offp, uint32_t *NONNULL ret);
bool dns_rdata_parse_data(dns_rr_t *NONNULL rr, const uint8_t *NONNULL buf, unsigned *NONNULL offp,
                          unsigned target, unsigned rdlen, unsigned rrstart);
bool dns_rr_parse(dns_rr_t *NONNULL rrset,
                  const uint8_t *NONNULL buf, unsigned len, unsigned *NONNULL offp, bool rrdata_permitted);
void dns_name_free(dns_label_t *NONNULL name);
void dns_rrdata_free(dns_rr_t *NONNULL rr);
void dns_message_free(dns_message_t *NONNULL message);
bool dns_rdata_parse_data(dns_rr_t *NONNULL rr, const uint8_t *NONNULL buf, unsigned *NONNULL offp,
                          unsigned target, unsigned rdlen, unsigned rrstart);
bool dns_wire_parse(dns_message_t *NONNULL *NULLABLE ret, dns_wire_t *NONNULL message, unsigned len);
bool dns_names_equal(dns_label_t *NONNULL name1, dns_label_t *NONNULL name2);
const char *NONNULL dns_name_print(dns_name_t *NONNULL name, char *NONNULL buf, int bufmax);
bool dns_names_equal_text(dns_label_t *NONNULL name1, const char *NONNULL name2);
size_t dns_name_wire_length(dns_label_t *NONNULL name);
size_t dns_name_to_wire_canonical(uint8_t *NONNULL buf, size_t max, dns_label_t *NONNULL name);
#endif // _DNS_MSG_H

// Local Variables:
// mode: C
// tab-width: 4
// c-file-style: "bsd"
// c-basic-offset: 4
// fill-column: 108
// indent-tabs-mode: nil
// End:
