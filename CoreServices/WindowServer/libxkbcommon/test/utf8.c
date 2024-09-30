/*
 * Copyright © 2014 Ran Benita <ran234@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "config.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "utf8.h"
#include "utils.h"

#define VALID(lit) assert(is_valid_utf8(lit, sizeof(lit)-1))
#define INVALID(lit) assert(!is_valid_utf8(lit, sizeof(lit)-1))

static void
test_is_valid_utf8(void)
{
    /*
     * Mostly taken from:
     * https://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-test.txt
     */

    VALID("ascii");
    VALID("\xCE\xBA\xE1\xBD\xB9\xCF\x83\xCE\xBC\xCE\xB5");

    VALID("");
    VALID("\x00");
    VALID("\x00\x00");

    VALID("\x50");
    VALID("\xC2\x80");
    VALID("\xE0\xA0\x80");
    VALID("\xF0\x90\x80\x80");

    /* 5/6-byte continuations aren't allowed (unlike UTF-8-test). */
    INVALID("\xF8\x88\x80\x80\x80");
    INVALID("\xFC\x84\x80\x80\x80\x80");

    VALID("\x7F");
    VALID("\xDF\xBF");
    VALID("\xEF\xBF\xBF");
    /* VALID("\xF7\xBF\xBF\xBF"); */

    /* 5/6-byte continuations aren't allowed (unlike UTF-8-test). */
    INVALID("\xFB\xBF\xBF\xBF\xBF");
    INVALID("\xFD\xBFxBF\xBF\xBF");

    VALID("\xED\x9F\xBF");
    VALID("\xEE\x80\x80");
    VALID("\xEF\xBF\xBD");
    VALID("\xF4\x8F\xBF\xBF");
    /* VALID("\xF4\x90\x80\x80"); */

    INVALID("\x80");
    INVALID("\xBF");
    INVALID("\x80\xBF");
    INVALID("\x80\xBF\x80");
    INVALID("\x80\xBF\x80\xBF");
    INVALID("\x80\xBF\x80\xBF\x80");
    INVALID("\x80\xBF\x80\xBF\x80\xBF");
    INVALID("\x80\xBF\x80\xBF\x80\xBF\x80");
    INVALID("\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8A\x8B\x8C\x8D\x8E\x8F"
            "\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9A\x9B\x9C\x9D\x9E\x9F"
            "\xA0\xA1\xA2\xA3\xA4\xA5\xA6\xA7\xA8\xA9\xAA\xAB\xAC\xAD\xAE\xAF"
            "\xB0\xB1\xB2\xB3\xB4\xB5\xB6\xB7\xB8\xB9\xBA\xBB\xBC\xBD\xBE\xBF");

    INVALID("\xC0 \xC1 \xC2 \xC3 \xC4 \xC5 \xC6 \xC7 \xC8 \xC9 \xCA \xCB \xCC "
            "\xCD \xCE \xCF "
            "\xD0 \xD1 \xD2 \xD3 \xD4 \xD5 \xD6 \xD7 \xD8 \xD9 \xDA \xDB \xDD "
            "\xDD \xDE \xDF ");
    INVALID("\xF0 \xF1 \xF2 \xF3 \xF4 \xF5 \xF6 \xF7 ");
    INVALID("\xF8 \xF9 \xFA \xFB ");
    INVALID("\xFC \xFD ");

    INVALID("\xC0");
    INVALID("\xE0\x80");
    INVALID("\xF0\x80\x80");
    INVALID("\xF8\x80\x80\x80");
    INVALID("\xFC\x80\x80\x80\x80");
    INVALID("\xDF");
    INVALID("\xEF\xBF");
    INVALID("\xF7\xBF\xBF");
    INVALID("\xFB\xBF\xBF\xBF");
    INVALID("\xFD\xBF\xBF\xBF\xBF");

    INVALID("\xC0\xE0\x80\xF0\x80\x80\xF8\x80\x80\x80\xFC\x80\x80\x80\x80"
            "\xDF\xEF\xBF\xF7\xBF\xBF\xFB\xBF\xBF\xBF\xFD\xBF\xBF\xBF\xBF");

    INVALID("\xFE");
    INVALID("\xFF");
    INVALID("\xFE\xFE\xFF\xFF");

    INVALID("\xC0\xAF");
    INVALID("\xE0\x80\xAF");
    INVALID("\xF0\x80\x80\xAF");
    INVALID("\xF8\x80\x80\x80\xAF");
    INVALID("\xFC\x80\x80\x80\x80\xAF");

    INVALID("\xC1\xBF");
    INVALID("\xE0\x9F\xBF");
    INVALID("\xF0\x8F\xBF\xBF");
    INVALID("\xF8\x87\xBF\xBF\xBF");
    INVALID("\xFC\x83\xBF\xBF\xBF\xBF");

    INVALID("\xC0\x80");
    INVALID("\xE0\x80\x80");
    INVALID("\xF0\x80\x80\x80");
    INVALID("\xF8\x80\x80\x80\x80");
    INVALID("\xFC\x80\x80\x80\x80\x80");

    INVALID("\xED\xA0\x80");
    INVALID("\xED\xAD\xBF");
    INVALID("\xED\xAE\x80");
    INVALID("\xED\xAF\xBF");
    INVALID("\xED\xB0\x80");
    INVALID("\xED\xBE\x80");
    INVALID("\xED\xBF\xBF");

    INVALID("\xED\xA0\x80\xED\xB0\x80");
    INVALID("\xED\xA0\x80\xED\xBF\xBF");
    INVALID("\xED\xAD\xBF\xED\xB0\x80");
    INVALID("\xED\xAD\xBF\xED\xBF\xBF");
    INVALID("\xED\xAE\x80\xED\xB0\x80");
    INVALID("\xED\xAE\x80\xED\xBF\xBF");
    INVALID("\xED\xAF\xBF\xED\xB0\x80");
    INVALID("\xED\xAF\xBF\xED\xBF\xBF");

    /* INVALID("\xEF\xBF\xBE"); */
    /* INVALID("\xEF\xBF\xBF"); */
}

static void
check_utf32_to_utf8(uint32_t unichar, int expected_length, const char *expected) {
    char buffer[7];
    int length;

    length = utf32_to_utf8(unichar, buffer);

    assert(length == expected_length);
    assert(streq(buffer, expected));
}

static void
test_utf32_to_utf8(void)
{
    check_utf32_to_utf8(0x0, 2, "");
    check_utf32_to_utf8(0x40, 2, "\x40");
    check_utf32_to_utf8(0xA1, 3, "\xc2\xa1");
    check_utf32_to_utf8(0x2701, 4, "\xe2\x9c\x81");
    check_utf32_to_utf8(0xd800, 0, ""); // Unicode surrogates
    check_utf32_to_utf8(0xdfff, 0, ""); // Unicode surrogates
    check_utf32_to_utf8(0x1f004, 5, "\xf0\x9f\x80\x84");
    check_utf32_to_utf8(0x110000, 0, "");
    check_utf32_to_utf8(0xffffffff, 0, "");
}

int
main(void)
{
    test_is_valid_utf8();
    test_utf32_to_utf8();

    return 0;
}
