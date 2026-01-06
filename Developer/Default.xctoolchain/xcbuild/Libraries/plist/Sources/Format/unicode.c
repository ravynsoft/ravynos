/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

/* $NetBSD: unicode.c,v 1.1.1.1.20.2 2007/09/03 14:40:23 yamt Exp $ */

/*-
 * Copyright (c) 2007 The NetBSD Foundation, Inc.
 * All rights reserved.
*
 * This code is derived from software contributed to The NetBSD Foundation
 * by Dieter Baron.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <plist/Format/unicode.h>

#define IS_CONT(c)  (((c)&0xc0) == 0x80)

size_t
utf8_to_utf16(uint16_t *dst, size_t dst_len, char const *src, size_t src_len,
        int flags, int *errp)
{
    uint8_t const *s;
    uint16_t       c;
    int            error;
    size_t         spos, dpos;

    error = 0;
    s = (const unsigned char *)src;
    spos = dpos = 0;
    while (spos<src_len) {
        if (s[spos] < 0x80) {
            c = s[spos++];
        } else if ((flags & UNICODE_UTF8_LATIN1_FALLBACK) &&
                (spos >= src_len || !IS_CONT(s[spos+1])) &&
                s[spos]>=0xa0) {
            /* not valid UTF-8, assume ISO 8859-1 */
            c = s[spos++];
        } else if (s[spos] < 0xc0 || s[spos] >= 0xf5) {
            /* continuation byte without lead byte
               or lead byte for codepoint above 0x10ffff */
            error++;
            spos++;
            continue;
        } else if (s[spos] < 0xe0) {
            if (spos >= src_len || !IS_CONT(s[spos+1])) {
                spos++;
                error++;
                continue;
            }
            c = ((s[spos] & 0x3f) << 6) | (s[spos+1] & 0x3f);
            spos += 2;
            if (c < 0x80) {
                /* overlong encoding */
                error++;
                continue;
            }
        } else if (s[spos] < 0xf0) {
            if (spos >= src_len-2 || !IS_CONT(s[spos+1]) ||
                    !IS_CONT(s[spos+2])) {
                spos++;
                error++;
                continue;
            }
            c = ((s[spos] & 0x0f) << 12) | ((s[spos+1] & 0x3f) << 6) |
                (s[spos+2] & 0x3f);
            spos += 3;
            if (c < 0x800 || (c & 0xdf00) == 0xd800) {
                /* overlong encoding or encoded surrogate */
                error++;
                continue;
            }
        } else {
            uint32_t cc;
            /* UTF-16 surrogate pair */

            if (spos >= src_len-3 || !IS_CONT(s[spos+1]) ||
                    !IS_CONT(s[spos+2]) || !IS_CONT(s[spos+3])) {
                spos++;
                error++;

                continue;
            }
            cc = ((s[spos] & 0x03) << 18) | ((s[spos+1] & 0x3f) << 12) |
                ((s[spos+2] & 0x3f) << 6) | (s[spos+3] & 0x3f);
            spos += 4;
            if (cc < 0x10000) {
                /* overlong encoding */
                error++;
                continue;
            }
            if (dst && dpos < dst_len) {
                dst[dpos] = (0xd800 | ((cc-0x10000)>>10));
            }
            dpos++;
            c = 0xdc00 | ((cc-0x10000) & 0x3ff);
        }

        if (dst && dpos < dst_len) {
            dst[dpos] = c;
        }
        dpos++;
    }

    if (errp) {
        *errp = error;
    }

    return dpos;
}

size_t
utf8_to_utf32(uint32_t *dst, size_t dst_len, char const *src, size_t src_len,
        int flags, int *errp)
{
    uint8_t const *s;
    uint32_t       c;
    int            error;
    size_t         spos, dpos;

    error = 0;
    s = (const unsigned char *)src;
    spos = dpos = 0;
    while (spos<src_len) {
        if (s[spos] < 0x80) {
            c = s[spos++];
        } else if ((flags & UNICODE_UTF8_LATIN1_FALLBACK) &&
                (spos >= src_len || !IS_CONT(s[spos+1])) &&
                s[spos]>=0xa0) {
            /* not valid UTF-8, assume ISO 8859-1 */
            c = s[spos++];
        } else if (s[spos] < 0xc0 || s[spos] >= 0xf5) {
            /* continuation byte without lead byte
               or lead byte for codepoint above 0x10ffff */
            error++;
            spos++;
            continue;
        } else if (s[spos] < 0xe0) {
            if (spos >= src_len || !IS_CONT(s[spos+1])) {
                spos++;
                error++;
                continue;
            }
            c = ((s[spos] & 0x3f) << 6) | (s[spos+1] & 0x3f);
            spos += 2;
            if (c < 0x80) {
                /* overlong encoding */
                error++;
                continue;
            }
        } else if (s[spos] < 0xf0) {
            if (spos >= src_len-2 || !IS_CONT(s[spos+1]) ||
                    !IS_CONT(s[spos+2])) {
                spos++;
                error++;
                continue;
            }
            c = ((s[spos] & 0x0f) << 12) | ((s[spos+1] & 0x3f) << 6) |
                (s[spos+2] & 0x3f);
            spos += 3;
        } else {
            if (spos >= src_len-3 || !IS_CONT(s[spos+1]) ||
                    !IS_CONT(s[spos+2]) || !IS_CONT(s[spos+3])) {
                spos++;
                error++;

                continue;
            }
            c = ((s[spos] & 0x03) << 18) | ((s[spos+1] & 0x3f) << 12) |
                ((s[spos+2] & 0x3f) << 6) | (s[spos+3] & 0x3f);
            spos += 4;
        }

        if (dst && dpos < dst_len) {
            dst[dpos] = c;
        }
        dpos++;
    }

    if (errp) {
        *errp = error;
    }

    return dpos;
}

#undef IS_CONT

#define CHECK_LENGTH(l) (dpos > dst_len-(l) ? dst=NULL : NULL)
#define ADD_BYTE(b) (dst ? dst[dpos] = (b) : 0, dpos++)

size_t
utf16_to_utf8(char *dst, size_t dst_len, uint16_t const *src, size_t src_len,
        int flags, int *errp)
{
    size_t spos, dpos;
    int    error;

    error = 0;
    dpos  = 0;
    for (spos = 0; spos < src_len; spos++) {
        if (src[spos] < 0x80) {
            CHECK_LENGTH(1);
            ADD_BYTE((char)src[spos]);
        } else if (src[spos] < 0x800) {
            CHECK_LENGTH(2);
            ADD_BYTE(0xc0 | (src[spos]>>6));
            ADD_BYTE(0x80 | (src[spos] & 0x3f));
        } else if ((src[spos] & 0xdc00) == 0xd800) {
            uint32_t c;
            /* first surrogate */
            if (spos == src_len - 1 || (src[spos+1] & 0xdc00) != 0xdc00) {
                /* no second surrogate present */
                error++;
                continue;
            }
            CHECK_LENGTH(4);
            c = (((src[spos]&0x3ff) << 10) | (src[spos+1]&0x3ff)) + 0x10000;
            spos++;
            ADD_BYTE(0xf0 | ((c>>18) & 0x7));
            ADD_BYTE(0x80 | ((c>>12) & 0x3f));
            ADD_BYTE(0x80 | ((c>>6) & 0x3f));
            ADD_BYTE(0x80 | (c & 0x3f));
        } else if ((src[spos] & 0xdc00) == 0xdc00) {
            /* second surrogate without preceding first surrogate */
            error++;
        } else {
            CHECK_LENGTH(3);
            ADD_BYTE(0xe0 | src[spos]>>12);
            ADD_BYTE(0x80 | ((src[spos]>>6) & 0x3f));
            ADD_BYTE(0x80 | (src[spos] & 0x3f));
        }
    }

    if (errp) {
        *errp = error;
    }

    return dpos;
}

size_t
utf32_to_utf8(char *dst, size_t dst_len, uint32_t const *src, size_t src_len,
        int flags, int *errp)
{
    size_t spos, dpos;
    int    error;

    error = 0;
    dpos  = 0;
    for (spos = 0; spos < src_len; spos++) {
        if (src[spos] < 0x80) {
            CHECK_LENGTH(1);
            ADD_BYTE(src[spos]);
        } else if (src[spos] < 0x800) {
            CHECK_LENGTH(2);
            ADD_BYTE(0xc0 | (src[spos]>>6));
            ADD_BYTE(0x80 | (src[spos] & 0x3f));
        } else if (src[spos] < 0x10000) {
            CHECK_LENGTH(3);
            ADD_BYTE(0xe0 | ((src[spos]>>12) & 0x3f));
            ADD_BYTE(0x80 | ((src[spos]>>6) & 0x3f));
            ADD_BYTE(0x80 | (src[spos] & 0x3f));
        } else if (src[spos] < 0x200000) {
            CHECK_LENGTH(4);
            ADD_BYTE(0xf0 | ((src[spos]>>18) & 0x7));
            ADD_BYTE(0x80 | ((src[spos]>>12) & 0x3f));
            ADD_BYTE(0x80 | ((src[spos]>>6) & 0x3f));
            ADD_BYTE(0x80 | (src[spos] & 0x3f));
        } else {
            /* out of range */
            error++;
        }
    }

    if (errp) {
        *errp = error;
    }

    return dpos;
}

#undef ADD_BYTE
#undef CHECK_LENGTH
