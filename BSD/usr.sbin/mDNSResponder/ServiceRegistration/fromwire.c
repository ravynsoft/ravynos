/* fromwire.c
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
 * DNS wire-format functions.
 *
 * These are really simple functions for constructing DNS messages wire format.
 * The flow is that there is a transaction structure which contains pointers to both
 * a message output buffer and a response input buffer.   The structure is initialized,
 * and then the various wire format functions are called repeatedly to store data.
 * If an error occurs during this process, it's okay to just keep going, because the
 * error is recorded in the transaction; once all of the copy-in functions have been
 * called, the error status can be checked once at the end.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <ctype.h>
#include "srp.h"
#include "dns-msg.h"

bool
dns_opt_parse(dns_edns0_t *NONNULL *NULLABLE ret, dns_rr_t *rr)
{
    return true;
}

dns_label_t * NULLABLE
dns_label_parse(const uint8_t *buf, unsigned mlen, unsigned *NONNULL offp)
{
    uint8_t llen = buf[*offp];
    dns_label_t *rv;

    // Make sure that we got the data this label claims to encompass.
    if (*offp + llen + 1 > mlen) {
        DEBUG("claimed length of label is too long: %u > %u.\n", *offp + llen + 1, mlen);
        return NULL;
    }

    rv = calloc(llen + 1 - DNS_MAX_LABEL_SIZE + sizeof *rv, 1);
    if (rv == NULL) {
        DEBUG("memory allocation for %u byte label (%.*s) failed.\n",
              *offp + llen + 1, *offp + llen + 1, &buf[*offp + 1]);
        return NULL;
    }

    rv->len = llen;
    memcpy(rv->data, &buf[*offp + 1], llen);
    rv->data[llen] = 0; // We NUL-terminate the label for convenience
    *offp += llen + 1;
    return rv;
}

bool
dns_name_parse(dns_label_t *NONNULL *NULLABLE ret, const uint8_t *buf, unsigned len,
               unsigned *NONNULL offp, unsigned base)
{
    dns_label_t *rv;

    if (*offp == len) {
        return false;
    }

    // A pointer?
    if ((buf[*offp] & 0xC0) == 0xC0) {
        unsigned pointer;
        if (*offp + 2 > len) {
            DEBUG("incomplete compression pointer: %u > %u", *offp + 2, len);
            return false;
        }
        pointer = (((unsigned)buf[*offp] & 0x3f) << 8) | (unsigned)buf[*offp + 1];
        *offp += 2;
        if (pointer >= base) {
            // Don't allow a pointer forward, or to a pointer we've already visited.
            DEBUG("compression pointer points forward: %u >= %u.\n", pointer, base);
            return false;
        }
        if (pointer < DNS_HEADER_SIZE) {
            // Don't allow pointers into the header.
            DEBUG("compression pointer points into header: %u.\n", pointer);
            return false;
        }
        pointer -= DNS_HEADER_SIZE;
        if (buf[pointer] & 0xC0) {
            // If this is a pointer to a pointer, it's not valid.
            DEBUG("compression pointer points into pointer: %u %02x%02x.\n", pointer,
                  buf[pointer], pointer + 1 < len ? buf[pointer + 1] : 0xFF);
            return false;
        }
        if (buf[pointer] + pointer >= base || buf[pointer] + pointer >= *offp) {
            // Possibly this isn't worth checking.
            DEBUG("compression pointer points to something that goes past current position: %u %u\n",
                  pointer, buf[pointer]);
            return false;
        }
        return dns_name_parse(ret, buf, len, &pointer, pointer);
    }
    // We don't support binary labels, which are historical, and at this time there are no other valid
    // DNS label types.
    if (buf[*offp] & 0xC0) {
        DEBUG("invalid label type: %x\n", buf[*offp]);
        return false;
    }
    
    rv = dns_label_parse(buf, len, offp);
    if (rv == NULL) {
        return false;
    }

    *ret = rv;

    if (rv->len == 0) {
        return true;
    }
    return dns_name_parse(&rv->next, buf, len, offp, base);
}

bool
dns_u8_parse(const uint8_t *buf, unsigned len, unsigned *NONNULL offp, uint8_t *NONNULL ret)
{
    uint8_t rv;
    if (*offp + 1 > len) {
        DEBUG("dns_u8_parse: not enough room: %u > %u.\n", *offp + 1, len);
        return false;
    }

    rv = buf[*offp];
    *offp += 1;
    *ret = rv;
    return true;
}

bool
dns_u16_parse(const uint8_t *buf, unsigned len, unsigned *NONNULL offp, uint16_t *NONNULL ret)
{
    uint16_t rv;
    if (*offp + 2 > len) {
        DEBUG("dns_u16_parse: not enough room: %u > %u.\n", *offp + 2, len);
        return false;
    }

    rv = ((uint16_t)(buf[*offp]) << 8) | (uint16_t)(buf[*offp + 1]);
    *offp += 2;
    *ret = rv;
    return true;
}

bool
dns_u32_parse(const uint8_t *buf, unsigned len, unsigned *NONNULL offp, uint32_t *NONNULL ret)
{
    uint32_t rv;
    if (*offp + 4 > len) {
        DEBUG("dns_u32_parse: not enough room: %u > %u.\n", *offp + 4, len);
        return false;
    }

    rv = (((uint32_t)(buf[*offp]) << 24) | ((uint32_t)(buf[*offp + 1]) << 16) |
          ((uint32_t)(buf[*offp + 2]) << 8) | (uint32_t)(buf[*offp + 3]));
    *offp += 4;
    *ret = rv;
    return true;
}

static void
dns_name_dump(FILE *outfile, dns_label_t *name)
{
    char buf[DNS_MAX_NAME_SIZE_ESCAPED + 1];
    
    dns_name_print(name, buf, sizeof buf);
    fputs(buf, outfile);
}

static void
dns_rrdata_dump(FILE *outfile, dns_rr_t *rr)
{
    int i;
    char nbuf[80];
    dns_txt_element_t *txt;

    switch(rr->type) {
    case dns_rrtype_key:
        fprintf(outfile, "KEY <AC %d> <Z %d> <XT %d> <ZZ %d> <NAMTYPE %d> <ZZZZ %d> <ORY %d> %d %d ",
                ((rr->data.key.flags & 0xC000) >> 14 & 3), ((rr->data.key.flags & 0x2000) >> 13) & 1,
                ((rr->data.key.flags & 0x1000) >> 12) & 1, ((rr->data.key.flags & 0xC00) >> 10) & 3,
                ((rr->data.key.flags & 0x300) >> 8) & 3, ((rr->data.key.flags & 0xF0) >> 4) & 15, rr->data.key.flags & 15,
                rr->data.key.protocol, rr->data.key.algorithm);
        for (i = 0; i < rr->data.key.len; i++) {
            if (i == 0) {
                fprintf(outfile, "%d [%02x", rr->data.key.len, rr->data.key.key[i]);
            } else {
                fprintf(outfile, " %02x", rr->data.key.key[i]);
            }
        }
        fputc(']', outfile);
        break;
        
    case dns_rrtype_sig:
        fprintf(outfile, "SIG %d %d %d %lu %lu %lu %d ",
                rr->data.sig.type, rr->data.sig.algorithm, rr->data.sig.label,
                (unsigned long)rr->data.sig.rrttl, (unsigned long)rr->data.sig.expiry,
                (unsigned long)rr->data.sig.inception, rr->data.sig.key_tag);
        dns_name_dump(outfile, rr->data.sig.signer);
        for (i = 0; i < rr->data.sig.len; i++) {
            if (i == 0) {
                fprintf(outfile, "%d [%02x", rr->data.sig.len, rr->data.sig.signature[i]);
            } else {
                fprintf(outfile, " %02x", rr->data.sig.signature[i]);
            }
        }
        fputc(']', outfile);
        break;
        
    case dns_rrtype_srv:
        fprintf(outfile, "SRV %d %d %d ", rr->data.srv.priority, rr->data.srv.weight, rr->data.srv.port);
        dns_name_dump(outfile, rr->data.ptr.name);
        break;

    case dns_rrtype_ptr:
        fputs("PTR ", outfile);
        dns_name_dump(outfile, rr->data.ptr.name);
        break;

    case dns_rrtype_cname:
        fputs("CNAME ", outfile);
        dns_name_dump(outfile, rr->data.ptr.name);
        break;

    case dns_rrtype_a:
        fputs("A", outfile);
        for (i = 0; i < rr->data.a.num; i++) {
            inet_ntop(AF_INET, &rr->data.a.addrs[i], nbuf, sizeof nbuf);
            putc(' ', outfile);
            fputs(nbuf, outfile);
        }
        break;
        
    case dns_rrtype_aaaa:
        fputs("AAAA", outfile);
        for (i = 0; i < rr->data.aaaa.num; i++) {
            inet_ntop(AF_INET6, &rr->data.aaaa.addrs[i], nbuf, sizeof nbuf);
            putc(' ', outfile);
            fputs(nbuf, outfile);
        }
        break;

    case dns_rrtype_txt:
        fputs("TXT", outfile);
        for (txt = rr->data.txt; txt; txt = txt->next) {
            putc(' ', outfile);
            putc('"', outfile);
            for (i = 0; i < txt->len; i++) {
                if (isascii(txt->data[i]) && isprint(txt->data[i])) {
                    putc(txt->data[i], outfile);
                } else {
                    fprintf(outfile, "<%x>", txt->data[i]);
                }
            }
            putc('"', outfile);
        }
        break;

    default:
        fprintf(outfile, "<rrtype %d>:", rr->type);
        if (rr->data.unparsed.len == 0) {
            fputs(" <none>", outfile);
        } else {
            for (i = 0; i < rr->data.unparsed.len; i++) {
                fprintf(outfile, " %02x", rr->data.unparsed.data[i]);
            }
        }
        break;
    }
}

bool
dns_rdata_parse_data(dns_rr_t *NONNULL rr, const uint8_t *buf, unsigned *NONNULL offp, unsigned target, unsigned rdlen, unsigned rrstart)
{
    uint16_t addrlen;
    dns_txt_element_t *txt, **ptxt;

    switch(rr->type) {
    case dns_rrtype_key:
        if (!dns_u16_parse(buf, target, offp, &rr->data.key.flags) ||
            !dns_u8_parse(buf, target, offp, &rr->data.key.protocol) ||
            !dns_u8_parse(buf, target, offp, &rr->data.key.algorithm)) {
            return false;
        }
        rr->data.key.len = target - *offp;
        rr->data.key.key = malloc(rr->data.key.len);
        if (!rr->data.key.key) {
            return false;
        }
        memcpy(rr->data.key.key, &buf[*offp], rr->data.key.len);
        *offp += rr->data.key.len;
        break;

    case dns_rrtype_sig:
        rr->data.sig.start = rrstart;
        if (!dns_u16_parse(buf, target, offp, &rr->data.sig.type) ||
            !dns_u8_parse(buf, target, offp, &rr->data.sig.algorithm) ||
            !dns_u8_parse(buf, target, offp, &rr->data.sig.label) ||
            !dns_u32_parse(buf, target, offp, &rr->data.sig.rrttl) ||
            !dns_u32_parse(buf, target, offp, &rr->data.sig.expiry) ||
            !dns_u32_parse(buf, target, offp, &rr->data.sig.inception) ||
            !dns_u16_parse(buf, target, offp, &rr->data.sig.key_tag) ||
            !dns_name_parse(&rr->data.sig.signer, buf, target, offp, *offp)) {
            return false;
        }
        // The signature is what's left of the RRDATA.  It covers the message up to the signature, so we
        // remember where it starts so as to know what memory to cover to validate it.
        rr->data.sig.len = target - *offp;
        rr->data.sig.signature = malloc(rr->data.sig.len);
        if (!rr->data.sig.signature) {
            return false;
        }
        memcpy(rr->data.sig.signature, &buf[*offp], rr->data.sig.len);
        *offp += rr->data.sig.len;
        break;
        
    case dns_rrtype_srv:
        if (!dns_u16_parse(buf, target, offp, &rr->data.srv.priority) ||
            !dns_u16_parse(buf, target, offp, &rr->data.srv.weight) ||
            !dns_u16_parse(buf, target, offp, &rr->data.srv.port)) {
            return false;
        }
        // This fallthrough assumes that the first element in the srv, ptr and cname structs is
        // a pointer to a domain name.

    case dns_rrtype_ptr:
    case dns_rrtype_cname:
        if (!dns_name_parse(&rr->data.ptr.name, buf, target, offp, *offp)) {
            return false;
        }
        break;

        // We assume below that the a and aaaa structures in the data union are exact aliases of
        // each another.
    case dns_rrtype_a:
        addrlen = 4;
        goto addr_parse;
        
    case dns_rrtype_aaaa:
        addrlen = 16;
    addr_parse:
        if (rdlen & (addrlen - 1)) {
            DEBUG("dns_rdata_parse: %s rdlen not an even multiple of %u: %u",
                  addrlen == 4 ? "A" : "AAAA", addrlen, rdlen);
            return false;
        }
        rr->data.a.addrs = malloc(rdlen);
        if (rr->data.a.addrs == NULL) {
            return false;
        }
        rr->data.a.num = rdlen /  addrlen;
        memcpy(rr->data.a.addrs, &buf[*offp], rdlen);
        *offp = target;
        break;
        
    case dns_rrtype_txt:
        ptxt = &rr->data.txt;
        while (*offp < target) {
            unsigned tlen = buf[*offp];
            if (*offp + tlen + 1 > target) {
                DEBUG("dns_rdata_parse: TXT RR length is larger than available space: %u %u",
                      *offp + tlen + 1, target);
                *ptxt = NULL;
                return false;
            }
            txt = malloc(tlen + 1 + sizeof *txt);
            if (txt == NULL) {
                DEBUG("dns_rdata_parse: no memory for TXT RR");
                return false;
            }
            txt->len = tlen;
            ++*offp;
            memcpy(txt->data, &buf[*offp], tlen);
            *offp += tlen;
            txt->data[tlen] = 0;
            *ptxt = txt;
            ptxt = &txt->next;
        }
        break;

    default:
        if (rdlen > 0) {
            rr->data.unparsed.data = malloc(rdlen);
            if (rr->data.unparsed.data == NULL) {
                return false;
            }
            memcpy(rr->data.unparsed.data, &buf[*offp], rdlen);
        }
        rr->data.unparsed.len = rdlen;
        *offp = target;
        break;
    }
    if (*offp != target) {
        DEBUG("dns_rdata_parse: parse for rrtype %d not fully contained: %u %u", rr->type, target, *offp);
        return false;
    }
    return true;
}

static bool
dns_rdata_parse(dns_rr_t *NONNULL rr,
                const uint8_t *buf, unsigned len, unsigned *NONNULL offp, unsigned rrstart)
{
    uint16_t rdlen;
    unsigned target;
    
    if (!dns_u16_parse(buf, len, offp, &rdlen)) {
        return false;
    }
    target = *offp + rdlen;
    if (target > len) {
        return false;
    }
    return dns_rdata_parse_data(rr, buf, offp, target, rdlen, rrstart);
}

bool
dns_rr_parse(dns_rr_t *NONNULL rr,
             const uint8_t *buf, unsigned len, unsigned *NONNULL offp, bool rrdata_expected)
{
    int rrstart = *offp; // Needed to mark the start of the SIG RR for SIG(0).
    memset(rr, 0, sizeof *rr);
    if (!dns_name_parse(&rr->name, buf, len, offp, *offp)) {
        return false;
    }
    
    if (!dns_u16_parse(buf, len, offp, &rr->type)) {
        return false;
    }

    if (!dns_u16_parse(buf, len, offp, &rr->qclass)) {
        return false;
    }
    
    if (rrdata_expected) {
        if (!dns_u32_parse(buf, len, offp, &rr->ttl)) {
            return false;
        }
        if (!dns_rdata_parse(rr, buf, len, offp, rrstart)) {
            return false;
        }
    }
        
    printf("rrtype: %u  qclass: %u  name: ", rr->type, rr->qclass);
    dns_name_dump(stdout, rr->name);
    if (rrdata_expected) {
        printf("  rrdata: ");
        dns_rrdata_dump(stdout, rr);
    }
    printf("\n");
    return true;
}

void dns_name_free(dns_label_t *name)
{
    dns_label_t *next;
    if (name == NULL) {
        return;
    }
    next = name->next;
    free(name);
    return dns_name_free(next);
}    

void
dns_rrdata_free(dns_rr_t *rr)
{
    switch(rr->type) {
    case dns_rrtype_key:
        free(rr->data.key.key);
        break;
        
    case dns_rrtype_sig:
        dns_name_free(rr->data.sig.signer);
        free(rr->data.sig.signature);
        break;
        
    case dns_rrtype_srv:
    case dns_rrtype_ptr:
    case dns_rrtype_cname:
        dns_name_free(rr->data.ptr.name);
        rr->data.ptr.name = NULL;
        break;

    case dns_rrtype_a:
    case dns_rrtype_aaaa:
        free(rr->data.a.addrs);
        rr->data.a.addrs = NULL;
        break;
        
    case dns_rrtype_txt:
    default:
        free(rr->data.unparsed.data);
        rr->data.unparsed.data = NULL;
        break;
    }
}

void
dns_message_free(dns_message_t *message)
{
    int i;

#define FREE(count, sets)                           \
    for (i = 0; i < message->count; i++) {          \
        dns_rr_t *set = &message->sets[i];          \
        if (set->name) {                            \
            dns_name_free(set->name);               \
            set->name = NULL;                       \
        }                                           \
        dns_rrdata_free(set);                       \
    }                                               \
    if (message->sets) {                            \
        free(message->sets);                        \
    }
    FREE(qdcount, questions);
    FREE(ancount, answers);
    FREE(nscount, authority);
    FREE(arcount, additional);
#undef FREE
}

bool
dns_wire_parse(dns_message_t *NONNULL *NULLABLE ret, dns_wire_t *message, unsigned len)
{
    unsigned offset = 0;
    dns_message_t *rv = calloc(sizeof *rv, 1);
    int i;
    
    if (rv == NULL) {
        return false;
    }
    
#define PARSE(count, sets, name, rrdata_expected)                                   \
    rv->count = ntohs(message->count);                                              \
    if (rv->count > 50) {                                                           \
        dns_message_free(rv);                                                       \
        return false;                                                               \
    }                                                                               \
                                                                                    \
    if (rv->qdcount != 0) {                                                         \
        rv->sets = calloc(sizeof *rv->sets, rv->count);                             \
        if (rv->sets == NULL) {                                                     \
            dns_message_free(rv);                                                   \
            return false;                                                           \
        }                                                                           \
    }                                                                               \
                                                                                    \
    for (i = 0; i < rv->count; i++) {                                               \
        if (!dns_rr_parse(&rv->sets[i], message->data, len, &offset, rrdata_expected)) {  	\
            dns_message_free(rv);                                                   \
            fprintf(stderr, name " %d RR parse failed.\n", i);                      \
            return false;                                                           \
        }                                                                           \
    }
    PARSE(qdcount,  questions, "question", false);
    PARSE(ancount,    answers, "answers", true);
    PARSE(nscount,  authority, "authority", true);
    PARSE(arcount, additional, "additional", true);
#undef PARSE
    
    for (i = 0; i < rv->ancount; i++) {
        // Parse EDNS(0)
        if (rv->additional[i].type == dns_rrtype_opt) {
            if (!dns_opt_parse(&rv->edns0, &rv->additional[i])) {
                dns_message_free(rv);
                return false;
            }
        }
    }
    *ret = rv;
    return true;
}

const char *NONNULL
dns_name_print(dns_name_t *NONNULL name, char *buf, int bufmax)
{
    dns_label_t *lp;
    int ix = 0;
    int i;

    // Copy the labels in one at a time, putting a dot between each one; if there isn't room
    // in the buffer (shouldn't be the case), copy as much as will fit, leaving room for a NUL
    // termination.
    for (lp = name; lp; lp = lp->next) {
        if (ix != 0) {
            if (ix + 2 >= bufmax) {
                break;
            }
            buf[ix++] = '.';
        }
        for (i = 0; i < lp->len; i++) {
            if (isascii(lp->data[i]) && isprint(lp->data[i])) {
                if (ix + 2 >= bufmax) {
                    break;
                }
                buf[ix++] = lp->data[i];
            } else {
                if (ix + 5 >= bufmax) {
                    break;
                }
                buf[ix++] = '\\';
                buf[ix++] = '0' + (lp->data[i] >> 6);
                buf[ix++] = '0' + (lp->data[i] >> 3) & 3;
                buf[ix++] = '0' + lp->data[i] & 3;
            }
        }
        if (i != lp->len) {
            break;
        }
    }
    buf[ix++] = 0;
    return buf;
}

bool
labeleq(const char *label1, const char *label2, size_t len)
{
    int i;
    for (i = 0; i < len; i++) {
        if (isascii(label1[i]) && isascii(label2[i])) {
            if (tolower(label1[i]) != tolower(label2[i])) {
                return false;
            }
        }
        else {
            if (label1[i] != label2[i]) {
                return false;
            }
        }
    }
    return true;
}

bool
dns_names_equal(dns_label_t *NONNULL name1, dns_label_t *NONNULL name2)
{
    if (name1->len != name2->len) {
        return false;
    }
    if (name1->len != 0 && !labeleq(name1->data, name2->data, name1->len) != 0) {
        return false;
    }
    if (name1->next != NULL && name2->next != NULL) {
        return dns_names_equal(name1->next, name2->next);
    }
    if (name1->next == NULL && name2->next == NULL) {
        return true;
    }
    return false;
}

// Note that "foo.arpa" is not the same as "foo.arpa."
bool
dns_names_equal_text(dns_label_t *NONNULL name1, const char *NONNULL name2)
{
    const char *ndot;
    ndot = strchr(name2, '.');
    if (ndot == NULL) {
        ndot = name2 + strlen(name2);
    }
    if (name1->len != ndot - name2) {
        return false;
    }
    if (name1->len != 0 && !labeleq(name1->data, name2, name1->len) != 0) {
        return false;
    }
    if (name1->next != NULL && *ndot == '.') {
        return dns_names_equal_text(name1->next, ndot + 1);
    }
    if (name1->next == NULL && *ndot == 0) {
        return true;
    }
    return false;
}

// Find the length of a name in uncompressed wire format.
// This is in fromwire because we use it for validating signatures, and don't need it for
// sending.
static size_t
dns_name_wire_length_in(dns_label_t *NONNULL name, size_t ret)
{
    // Root label.
    if (name == NULL)
        return ret;
    return dns_name_wire_length_in(name->next, ret + name->len + 1);
}

size_t
dns_name_wire_length(dns_label_t *NONNULL name)
{
    return dns_name_wire_length_in(name, 0);
}

// Copy a name we've parsed from a message out in canonical wire format so that we can
// use it to verify a signature.   As above, not actually needed for copying to a message
// we're going to send, since in that case we want to try to compress.
static size_t
dns_name_to_wire_canonical_in(uint8_t *NONNULL buf, size_t max, size_t ret, dns_label_t *NONNULL name)
{
    INFO("dns_name_to_wire_canonical_in: buf %p max %zd ret %zd  name = %p '%.*s'",
         buf, max, ret, name, name ? name->len : 0, name ? name->data : "");
    if (name == NULL) {
        return ret;
    }
    if (max < name->len + 1) {
        return 0;
    }
    *buf = name->len;
    memcpy(buf + 1, name->data, name->len);
    return dns_name_to_wire_canonical_in(buf + name->len + 1,
                                         max - name->len - 1, ret + name->len + 1, name->next);
}

size_t
dns_name_to_wire_canonical(uint8_t *NONNULL buf, size_t max, dns_label_t *NONNULL name)
{
    return dns_name_to_wire_canonical_in(buf, max, 0, name);
}
    


// Local Variables:
// mode: C
// tab-width: 4
// c-file-style: "bsd"
// c-basic-offset: 4
// fill-column: 108
// indent-tabs-mode: nil
// End:
