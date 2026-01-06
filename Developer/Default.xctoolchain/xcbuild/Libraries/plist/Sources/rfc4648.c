/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#include "rfc4648.h"

static char const g_base64_alphabet[] =
  { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" };
static char const g_base64_alphabet_safe[] =
  { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_" };
static char const g_base32_alphabet[] =
  { "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567" };
static char const g_base16_alphabet[] =
  { "0123456789ABCDEF" };
static char const g_base16_l_alphabet[] =
  { "0123456789abcdef" };

#define F_WRITE   1
#define F_RECOVER 2

size_t
rfc4648_get_encoded_size(rfc4648_type_t type,
                         size_t         size)
{
    switch (type) {
        case RFC4648_TYPE_BASE64:
        case RFC4648_TYPE_BASE64_SAFE:
            return ((size + 2) / 3) * 4;
        case RFC4648_TYPE_BASE32:
            return ((size + 4) / 5) * 8;
        case RFC4648_TYPE_BASE16:
        case RFC4648_TYPE_BASE16_LCASE:
            return (size << 1);
        default:
            return 0;
    }
}

size_t
rfc4648_get_decoded_size(rfc4648_type_t type,
                         size_t         size)
{
    switch (type) {
        case RFC4648_TYPE_BASE64:
        case RFC4648_TYPE_BASE64_SAFE:
            return ((size + 3) / 4) * 3;
        case RFC4648_TYPE_BASE32:
            return ((size + 7) / 8) * 5;
        case RFC4648_TYPE_BASE16:
        case RFC4648_TYPE_BASE16_LCASE:
            return ((size + 1) >> 1);
        default:
            return 0;
    }
}

static bool
seek_next_nonspace(uint8_t const *array, size_t *cursor, size_t length)
{
    while (*cursor < length) {
        uint8_t c = array[(*cursor)];
        if (!isspace(c)) {
            return true;
        }
        ++(*cursor);
    }
    return false;
}

static uint8_t
get_next_nonspace(uint8_t const *array, size_t *cursor, size_t length)
{
    if (seek_next_nonspace(array, cursor, length)) {
        uint8_t nonspace = array[*cursor];
        ++(*cursor);
        return nonspace;
    }
    return 0;
}

static int
encode_b16(bool        lcase,
           char       *output,
           void const *input,
           size_t      insize)
{
    char const    *a = lcase ? g_base16_l_alphabet : g_base16_alphabet;
    char          *o = output;
    uint8_t const *i = (uint8_t const *)input;
    size_t         n;

    for (n = 0; n < insize; n++, i++) {
        *o++ = a[*i >> 4];
        *o++ = a[*i & 0xf];
    }

    return (o - output);
}

static __inline int
b16_to_int(char c)
{
    if (c >= '0' && c <= '9')
        return (c - '0');
    else if (c >= 'A' && c <= 'F')
        return (c - 'A') + 10;
    else if (c >= 'a' && c <= 'f')
        return (c - 'a') + 10;
    else
        return -1;
}

static int
decode_b16(char       *output,
           void const *input,
           size_t      insize,
           uint32_t    flags)
{
    size_t n = 0;
    char  *o = output;

    seek_next_nonspace(input, &n, insize);
    while (n < insize) {
        int a, b;

        a = b16_to_int(get_next_nonspace(input, &n, insize));
        b = b16_to_int(get_next_nonspace(input, &n, insize));

        if (a < 0 || b < 0) {
            if (flags & F_RECOVER) {
                break;
            } else {
                return -1;
            }
        }

        if (flags & F_WRITE) {
            *o++ = (a << 4) | b;
        }
    }

    return o - output;
}

static int
encode_b32(char       *output,
           void const *input,
           size_t      insize)
{
    size_t         n;
    char const    *alphabet = g_base32_alphabet;
    char          *o        = output;
    uint8_t const *i        = (uint8_t const *)input;

    for (n = 0; n < insize; n += 5, i += 5) {
        bool hb, hc, hd, he;
        uint8_t a, b, c, d, e;

        hb = ((n + 1) < insize);
        hc = ((n + 2) < insize);
        hd = ((n + 3) < insize);
        he = ((n + 4) < insize);

        a = *i;
        b = hb ? *(i + 1) : 0;
        c = hc ? *(i + 2) : 0;
        d = hd ? *(i + 3) : 0;
        e = he ? *(i + 4) : 0;

        *o++ = alphabet[a >> 3];                     // A  a[0..4]
        *o++ = alphabet[(a & 0x07) << 2 | (b >> 6)]; // B  a[5..7] || b[0..1]
        if (!hb) break;
        *o++ = alphabet[(b & 0x3e) >> 1];            // C  b[2..6]
        *o++ = alphabet[(b & 0x01) << 4 | (c >> 4)]; // D  b[7] || c[0..3]
        if (!hc) break;
        *o++ = alphabet[(c & 0x0f) << 1 | (d >> 7)]; // E  c[4..7] || d[0]
        if (!hd) break;
        *o++ = alphabet[(d & 0x7c) >> 2];            // F  d[1..5]
        *o++ = alphabet[(d & 0x03) << 3 | (e >> 5)]; // G  d[6..7] || e[0..2]
        if (!he) break;
        *o++ = alphabet[e & 0x1f];                   // H  e[3..7]
    }
    while ((o - output) & 7) {
        *o++ = '=';
    }

    return (o - output);
}

static __inline int
b32_to_int(char c)
{
    if (c >= 'A' && c <= 'Z')
        return (c - 'A');
    else if (c >= '2' && c <= '7')
        return (c - '2') + 26;
    else if (c == '=')
        return -1;
    else
        return -2;
}

static int
decode_b32(char       *output,
           void const *input,
           size_t      insize,
           uint32_t    flags)
{
    size_t n = 0;
    int    a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, g = 0, h = 0;
    char  *o = output;

    seek_next_nonspace(input, &n, insize);
    while (n < insize) {
        a = b32_to_int(get_next_nonspace(input, &n, insize));
        b = b32_to_int(get_next_nonspace(input, &n, insize));
        c = b32_to_int(get_next_nonspace(input, &n, insize));
        d = b32_to_int(get_next_nonspace(input, &n, insize));
        e = b32_to_int(get_next_nonspace(input, &n, insize));
        f = b32_to_int(get_next_nonspace(input, &n, insize));
        g = b32_to_int(get_next_nonspace(input, &n, insize));
        h = b32_to_int(get_next_nonspace(input, &n, insize));

        if (a < 0) break;
        if (b < 0) { if (flags & F_RECOVER) break; else return -1; }
        if (flags & F_WRITE) *o++ = (a << 3) | (b >> 2);  // a[0..4] || b[0..2]
        if (c < 0 || d < 0) break;
        if (flags & F_WRITE) *o++ = (b << 6) | (c << 1) | (d >> 4); // b[3..4] || c[0..4] || d[0]
        if (e < 0) break;
        if (flags & F_WRITE) *o++ = (d << 4) | (e >> 1); // d[1..4] || e[0..3]
        if (f < 0 || g < 0) break;
        if (flags & F_WRITE) *o++ = (e << 7) | (f << 2) | (g >> 3); // e[4] || f || g[0..1]
        if (h < 0) break;
        if (flags & F_WRITE) *o++ = (g << 5) | h; // g[2..4] || h
    }

    if ((a == -2 || b == -2 || c == -2 || d == -2 || e == -2 || f == -2 ||
                g == -2 || h == -2) && (flags & F_RECOVER) == 0)
        return -1;

    return o - output;
}

static int
encode_b64(bool        safe,
           char       *output,
           void const *input,
           size_t      insize)
{
    size_t         n;
    char          *o = output;
    uint8_t const *i = (uint8_t const *)input;
    char const    *alphabet = safe ? g_base64_alphabet_safe : g_base64_alphabet;

    for (n = 0; n < insize; n += 3, i += 3) {
        bool hb, hc;
        uint8_t a, b, c;

        hb = ((n + 1) < insize);
        hc = ((n + 2) < insize);
        a = *i;
        b = hb ? *(i + 1) : 0;
        c = hc ? *(i + 2) : 0;

        *o++ = alphabet[a >> 2];
        *o++ = alphabet[(a & 0x03) << 4 | (b >> 4)];
        if (!hb) break;
        *o++ = alphabet[(b & 0x0f) << 2 | (c >> 6)];
        if (!hc) break;
        *o++ = alphabet[c & 0x3f];
    }
    while ((o - output) & 3) {
        *o++ = '=';
    }

    return (o - output);
}

static __inline int
b64_to_int(char c)
{
    if (c >= 'A' && c <= 'Z')
        return (c - 'A');
    else if (c >= 'a' && c <= 'z')
        return (c - 'a') + 26;
    else if (c >= '0' && c <= '9')
        return (c - '0') + 52;
    else if (c == '+' || c == '-')
        return 62;
    else if (c == '/' || c == '_')
        return 63;
    else if (c == '=')
        return -1;
    else
        return -2;
}

static int
decode_b64(char       *output,
           void const *input,
           size_t      insize,
           uint32_t    flags)
{
    size_t n = 0;
    int    a = 0, b = 0, c = 0, d = 0;
    char  *o = output;

    seek_next_nonspace(input, &n, insize);
    while (n < insize) {
        a = b64_to_int(get_next_nonspace(input, &n, insize));
        b = b64_to_int(get_next_nonspace(input, &n, insize));
        c = b64_to_int(get_next_nonspace(input, &n, insize));
        d = b64_to_int(get_next_nonspace(input, &n, insize));

        if (a < 0) break;
        if (b < 0) { if (flags & F_RECOVER) break; else return -1; }
        if (flags & F_WRITE) *o++ = (a << 2) | (b >> 4);
        if (c < 0) break;
        if (flags & F_WRITE) *o++ = (b << 4) | (c >> 2);
        if (d < 0) break;
        if (flags & F_WRITE) *o++ = (c << 6) | d;
    }

    if ((a == -2 || b == -2 || c == -2 || d == -2) && (flags & F_RECOVER) == 0)
        return -1;

    return o - output;
}

bool
rfc4648_encode(rfc4648_type_t  type,
               void const     *input,
               size_t          insize,
               char           *output,
               size_t         *outsize)
{
    int rc;

    assert(output != NULL);
    assert(input != NULL);
    if (input == NULL || output == NULL)
        return false;

    switch (type) {
        case RFC4648_TYPE_BASE64:
        case RFC4648_TYPE_BASE64_SAFE:
            rc = encode_b64(type == RFC4648_TYPE_BASE64_SAFE,
                    output, input, insize);
            break;
        case RFC4648_TYPE_BASE32:
            rc = encode_b32(output, input, insize);
            break;
        case RFC4648_TYPE_BASE16:
        case RFC4648_TYPE_BASE16_LCASE:
            rc = encode_b16(type == RFC4648_TYPE_BASE16_LCASE,
                    output, input, insize);
            break;
        default:
            return false;
    }

    if (rc < 0)
        return false;

    if (outsize != NULL) {
        *outsize = rc;
    }

    return true;
}

bool
rfc4648_can_decode(char const     *input,
                   size_t          insize,
                   rfc4648_type_t *type)
{
    int rc;

    if (insize == 0)
        return false;

    assert(input != NULL);
    rc = decode_b64(NULL, input, insize, 0);
    if (rc < 0) {
        rc = decode_b32(NULL, input, insize, 0);
        if (rc < 0) {
            rc = decode_b16(NULL, input, insize, 0);
            if (rc < 0) {
                return false;
            } else if (type != NULL) {
                *type = RFC4648_TYPE_BASE16;
            }
        } else if (type != NULL) {
            *type = RFC4648_TYPE_BASE32;
        }
    } else if (type != NULL) {
        *type = RFC4648_TYPE_BASE64;
    }

    return !(rc < 0);
}

bool
rfc4648_decode(rfc4648_type_t  type,
               void const     *input,
               size_t          insize,
               char           *output,
               size_t         *outsize,
               bool            recover)
{
    int      rc;
    uint32_t flags = F_WRITE | (recover ? F_RECOVER : 0);

    assert(output != NULL);
    assert(input != NULL);
    if (input == NULL || output == NULL)
        return false;

    switch (type) {
        case RFC4648_TYPE_BASE64:
        case RFC4648_TYPE_BASE64_SAFE:
            rc = decode_b64(output, input, insize, flags);
            break;
        case RFC4648_TYPE_BASE32:
            rc = decode_b32(output, input, insize, flags);
            break;
        case RFC4648_TYPE_BASE16:
        case RFC4648_TYPE_BASE16_LCASE:
            rc = decode_b16(output, input, insize, flags);
            break;
        default:
            return false;
    }

    if (rc < 0)
        return false;

    if (outsize != NULL) {
        *outsize = rc;
    }

    return true;
}
