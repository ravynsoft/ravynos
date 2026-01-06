/*
 * Copyright (c) 2004 Rob Braun
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Rob Braun nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * 1-Oct-2004
 * DRI: Rob Braun <bbraun@synack.net>
 */

#include <stdlib.h>
#include <string.h>

#ifdef _BTEST_
int main(int argc, char* argv[]) {
    unsigned char* enc = benc(argv[1], strlen(argv[1]));
    printf("%s", enc);
    printf("%s\n", bdec(enc, strlen(enc)));
}
#endif


/*
 * The code below derives from "Secure Programming Cookbook for C and
 * C++"* and adapted by Kogule, Ryo (kogule@opendarwin.org).
 *
 * *John Viega and Matt Messier, O'Reilly, 2003
 *  http://www.secureprogramming.com/
 */

static char b64revtb[256] = {
  -3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*0-15*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*16-31*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, /*32-47*/
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -2, -1, -1, /*48-63*/
  -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, /*64-79*/
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, /*80-95*/
  -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, /*96-111*/
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, /*112-127*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*128-143*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*144-159*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*160-175*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*176-191*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*192-207*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*208-223*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*224-239*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  /*240-255*/
};

static unsigned int raw_base64_decode(
  const unsigned char *input, unsigned char *output, unsigned int len,
  unsigned int *olen)
{

    unsigned int  x, i = 0, ignr = 0, dummy;
    unsigned char buf[3], pad = 0;

    if (!olen) olen = &dummy;
    *olen = 0;
    while (i < len && !pad) {
        x = b64revtb[input[i++]];
        switch (x) {
            case -3: /* NULL TERMINATOR */
                if ((i - ignr - 1) % 4) return 1;
                return 0;
            case -2: /* PADDING CHARACTER */
                if ((i - ignr - 1) % 4 < 2) {
                    /* Invalid here */
                    return 1;
                } else if ((i - ignr - 1) % 4 == 2) {
                    /* Make sure there's appropriate padding */
                    if (input[i] != '=') return 1;
                    buf[2] = 0;
                    pad = 2;
                    break;
                } else {
                    pad = 1;
                    break;
                }
                return 0;
            case -1:
                ignr++;
                break;
            default:
                switch ((i - ignr - 1) % 4) {
                    case 0:
                        buf[0] = x << 2;
                        break;
                    case 1:
                        buf[0] |= (x >> 4);
                        buf[1] = x << 4;
                        break;
                    case 2:
                        buf[1] |= (x >> 2);
                        buf[2] = x << 6;
                        break;
                    case 3:
                        buf[2] |= x;
                        for (x = 0;  x < 3U - pad;  x++) {
                            *output++ = buf[x];
                            (*olen)++;
                        }
                        break;
                }
                break;
        }
    }
    if (i > len) return 2;
    if (!pad) {
        if ((i - ignr) % 4) return 1;
    } else {
        for (x = 0;  x < 3U - pad;  x++) {
            *output++ = buf[x];
            (*olen)++;
        }
    }
    return 0;
}

unsigned char* xar_from_base64(
  const unsigned char* input, unsigned int len, unsigned int *olen
)
{
    int err;
    unsigned char *output;

    output = malloc(3 * (len / 4 + 1));
    if (!output) return NULL;

    err = raw_base64_decode(input, output, len, olen);

    if (err) {
        free(output);
        return NULL;
    }
    return output;
}
