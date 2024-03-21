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

#include <assert.h>
#include <stdlib.h>

#include "test.h"
#include "messages-codes.h"
#include "messages.h"

static void
test_message_get(void)
{
    const struct xkb_message_entry* entry;
    /* Invalid codes */
    /* NOTE: 0 must not be a valid message code */
    entry = xkb_message_get(0);
    assert(entry == NULL);
    entry = xkb_message_get(_XKB_LOG_MESSAGE_MIN_CODE - 1);
    assert(entry == NULL);
    entry = xkb_message_get(_XKB_LOG_MESSAGE_MAX_CODE + 1);
    assert(entry == NULL);

    /* Valid codes */
    entry = xkb_message_get(_XKB_LOG_MESSAGE_MIN_CODE);
    assert(entry != NULL);
    entry = xkb_message_get(XKB_WARNING_CANNOT_INFER_KEY_TYPE);
    assert(entry != NULL);
    entry = xkb_message_get(XKB_ERROR_INVALID_SYNTAX);
    assert(entry != NULL);
    entry = xkb_message_get(XKB_WARNING_CONFLICTING_KEY_FIELDS);
    assert(entry != NULL);
    entry = xkb_message_get(_XKB_LOG_MESSAGE_MAX_CODE);
    assert(entry != NULL);
}

int
main(void)
{
    test_message_get();
}
