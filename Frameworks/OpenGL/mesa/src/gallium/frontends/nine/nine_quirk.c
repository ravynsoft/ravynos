/*
 * Copyright 2011 Joakim Sindholt <opensource@zhasha.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "nine_quirk.h"

#include "util/u_debug.h"

static const struct debug_named_value nine_quirk_table[] = {
    { "fakecaps", QUIRK_FAKE_CAPS,
      "Fake caps to emulate D3D specs regardless of hardware caps." },
    { "lenientshader", QUIRK_LENIENT_SHADER,
      "Be lenient when translating shaders." },
    { "all", ~0U,
      "Enable all quirks." },
    DEBUG_NAMED_VALUE_END
};

bool
_nine_get_quirk( unsigned quirk )
{
    static bool first = true;
    static unsigned long flags = 0;

    if (first) {
        first = false;
        flags = debug_get_flags_option("NINE_QUIRKS", nine_quirk_table, 0);
    }

    return !!(flags & quirk);
}
