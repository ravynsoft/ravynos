/*
 * Copyright © 2023 Pierre Le Marre <dev@wismill.eu>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "messages-codes.h"
#include "messages.h"

static xkb_message_code_t
parse_message_code(char *raw_code) {
    xkb_message_code_t code;
    code = atoi(raw_code);
    if (!code && strstr(raw_code, "XKB-")) {
        return atoi(&(raw_code[4]));
    } else {
        return code;
    }
}

static void
usage(char **argv)
{
    printf("Usage: %s MESSAGE_CODES\n"
           "\n"
           "Check whether the given message codes are supported.\n",
           argv[0]);

    const struct xkb_message_entry *xkb_messages;
    size_t count = xkb_message_get_all(&xkb_messages);

    printf("\nCurrent supported messages:\n");
    for (size_t idx = 0; idx < count; idx++) {
        printf("- XKB-%03u: %s\n", xkb_messages[idx].code, xkb_messages[idx].label);
    }
}

#define XKB_CHECK_MSG_ERROR_PREFIX "xkb-check-messages: ERROR: "
#define MALFORMED_MESSAGE   (1 << 2)
#define UNSUPPORTED_MESSAGE (1 << 3)

int main(int argc, char **argv) {
    if (argc <= 1) {
        usage(argv);
        return EXIT_INVALID_USAGE;
    }

    int rc = 0;
    xkb_message_code_t code;
    const struct xkb_message_entry* entry;
    for (int k=1; k < argc; k++) {
        code = parse_message_code(argv[k]);
        if (!code) {
            fprintf(stderr,
                    XKB_CHECK_MSG_ERROR_PREFIX "Malformed message code: %s\n",
                    argv[k]);
            rc |= MALFORMED_MESSAGE;
            continue;
        }
        entry = xkb_message_get(code);
        if (entry == NULL) {
            fprintf(stderr,
                    XKB_CHECK_MSG_ERROR_PREFIX "Unsupported message code: %s\n",
                    argv[k]);
            rc |= UNSUPPORTED_MESSAGE;
        }
    }

    return rc;
}
