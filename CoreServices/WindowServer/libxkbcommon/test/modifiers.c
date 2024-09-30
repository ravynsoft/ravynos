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
 *
 */

#include "config.h"

#include <assert.h>
#include <stdlib.h>

#include "test.h"
#include "keymap.h"

// Standard real modifier masks
#define ShiftMask   (1 << 0)
#define LockMask    (1 << 1)
#define ControlMask (1 << 2)
#define Mod1Mask    (1 << 3)
#define Mod2Mask    (1 << 4)
#define Mod3Mask    (1 << 5)
#define Mod4Mask    (1 << 6)
#define Mod5Mask    (1 << 7)
#define NoModifier  0

static void
test_modmap_none(void)
{
    struct xkb_context *context = test_get_context(0);
    struct xkb_keymap *keymap;
    const struct xkb_key *key;
    xkb_keycode_t keycode;

    keymap = test_compile_file(context, "keymaps/modmap-none.xkb");
    assert(keymap);

    keycode = xkb_keymap_key_by_name(keymap, "LVL3");
    assert(keycode != XKB_KEYCODE_INVALID);
    key = XkbKey(keymap, keycode);
    assert(key->modmap == NoModifier);

    keycode = xkb_keymap_key_by_name(keymap, "LFSH");
    assert(keycode != XKB_KEYCODE_INVALID);
    key = XkbKey(keymap, keycode);
    assert(key->modmap == NoModifier);

    keycode = xkb_keymap_key_by_name(keymap, "RTSH");
    assert(keycode != XKB_KEYCODE_INVALID);
    key = XkbKey(keymap, keycode);
    assert(key->modmap == NoModifier);

    keycode = xkb_keymap_key_by_name(keymap, "LWIN");
    assert(keycode != XKB_KEYCODE_INVALID);
    key = XkbKey(keymap, keycode);
    assert(key->modmap == Mod4Mask);

    keycode = xkb_keymap_key_by_name(keymap, "RWIN");
    assert(keycode != XKB_KEYCODE_INVALID);
    key = XkbKey(keymap, keycode);
    assert(key->modmap == Mod4Mask);

    keycode = xkb_keymap_key_by_name(keymap, "LCTL");
    assert(keycode != XKB_KEYCODE_INVALID);
    key = XkbKey(keymap, keycode);
    assert(key->modmap == ControlMask);

    keycode = xkb_keymap_key_by_name(keymap, "RCTL");
    assert(keycode != XKB_KEYCODE_INVALID);
    key = XkbKey(keymap, keycode);
    assert(key->modmap == ControlMask);

    keycode = xkb_keymap_key_by_name(keymap, "LALT");
    assert(keycode != XKB_KEYCODE_INVALID);
    key = XkbKey(keymap, keycode);
    assert(key->modmap == Mod1Mask);

    keycode = xkb_keymap_key_by_name(keymap, "RALT");
    assert(keycode != XKB_KEYCODE_INVALID);
    key = XkbKey(keymap, keycode);
    assert(key->modmap == (Mod2Mask | Mod5Mask));

    keycode = xkb_keymap_key_by_name(keymap, "CAPS");
    assert(keycode != XKB_KEYCODE_INVALID);
    key = XkbKey(keymap, keycode);
    assert(key->modmap == LockMask);

    keycode = xkb_keymap_key_by_name(keymap, "AD01");
    assert(keycode != XKB_KEYCODE_INVALID);
    key = XkbKey(keymap, keycode);
    assert(key->modmap == Mod1Mask);

    keycode = xkb_keymap_key_by_name(keymap, "AD02");
    assert(keycode != XKB_KEYCODE_INVALID);
    key = XkbKey(keymap, keycode);
    assert(key->modmap == NoModifier);

    keycode = xkb_keymap_key_by_name(keymap, "AD03");
    assert(keycode != XKB_KEYCODE_INVALID);
    key = XkbKey(keymap, keycode);
    assert(key->modmap == NoModifier);

    keycode = xkb_keymap_key_by_name(keymap, "AD04");
    assert(keycode != XKB_KEYCODE_INVALID);
    key = XkbKey(keymap, keycode);
    assert(key->modmap == Mod1Mask);

    keycode = xkb_keymap_key_by_name(keymap, "AD05");
    assert(keycode != XKB_KEYCODE_INVALID);
    key = XkbKey(keymap, keycode);
    assert(key->modmap == Mod2Mask);

    keycode = xkb_keymap_key_by_name(keymap, "AD06");
    assert(keycode != XKB_KEYCODE_INVALID);
    key = XkbKey(keymap, keycode);
    assert(key->modmap == Mod3Mask);

    keycode = xkb_keymap_key_by_name(keymap, "AD07");
    assert(keycode != XKB_KEYCODE_INVALID);
    key = XkbKey(keymap, keycode);
    assert(key->modmap == Mod1Mask);

    keycode = xkb_keymap_key_by_name(keymap, "AD08");
    assert(keycode != XKB_KEYCODE_INVALID);
    key = XkbKey(keymap, keycode);
    assert(key->modmap == Mod2Mask);

    keycode = xkb_keymap_key_by_name(keymap, "AD09");
    assert(keycode != XKB_KEYCODE_INVALID);
    key = XkbKey(keymap, keycode);
    assert(key->modmap == Mod3Mask);

    xkb_keymap_unref(keymap);
    xkb_context_unref(context);
}

int
main(void)
{
    test_modmap_none();

    return 0;
}
