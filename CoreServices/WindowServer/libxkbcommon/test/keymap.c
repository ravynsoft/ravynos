/*
 * Copyright © 2016 Intel Corporation
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
 * Author: Mike Blumenkrantz <zmike@osg.samsung.com>
 */

#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "test.h"
#include "keymap.h"

static void
test_garbage_key(void)
{
    struct xkb_context *context = test_get_context(0);
    struct xkb_keymap *keymap;
    xkb_keycode_t kc;
    int nsyms;
    const xkb_keysym_t *syms;
    const xkb_layout_index_t first_layout = 0;
    xkb_level_index_t nlevels;

    assert(context);

    keymap = test_compile_rules(context, NULL, NULL, "garbage", NULL, NULL);
    assert(keymap);

    /* TLDE uses the 'us' sym on the first level and is thus [grave, exclam] */
    kc = xkb_keymap_key_by_name(keymap, "TLDE");
    assert(kc != XKB_KEYCODE_INVALID);
    nlevels = xkb_keymap_num_levels_for_key(keymap, kc, first_layout);
    assert(nlevels == 2);
    nsyms = xkb_keymap_key_get_syms_by_level(keymap, kc, first_layout, 0, &syms);
    assert(nsyms == 1);
    assert(*syms == XKB_KEY_grave); /* fallback from 'us' */
    nsyms = xkb_keymap_key_get_syms_by_level(keymap, kc, first_layout, 1, &syms);
    assert(nsyms == 1);
    assert(*syms == XKB_KEY_exclam);

    /* AE13 has no 'us' fallback and ends up as [NoSymbol, asciitilde] */
    kc = xkb_keymap_key_by_name(keymap, "AE13");
    assert(kc != XKB_KEYCODE_INVALID);
    nlevels = xkb_keymap_num_levels_for_key(keymap, kc, first_layout);
    assert(nlevels == 2);
    nsyms = xkb_keymap_key_get_syms_by_level(keymap, kc, first_layout, 0, &syms);
    assert(nsyms == 0);
    nsyms = xkb_keymap_key_get_syms_by_level(keymap, kc, first_layout, 1, &syms);
    assert(nsyms == 1);
    assert(*syms == XKB_KEY_asciitilde);

    xkb_keymap_unref(keymap);
    xkb_context_unref(context);
}

static void
test_keymap(void)
{
    struct xkb_context *context = test_get_context(0);
    struct xkb_keymap *keymap;
    xkb_keycode_t kc;
    const char *keyname;
    xkb_mod_mask_t masks_out[4] = { 0, 0, 0, 0 };
    size_t mask_count;
    xkb_mod_mask_t shift_mask;
    xkb_mod_mask_t lock_mask;
    xkb_mod_mask_t mod2_mask;

    assert(context);

    keymap = test_compile_rules(context, "evdev", "pc104", "us,ru", NULL, "grp:menu_toggle");
    assert(keymap);

    kc = xkb_keymap_key_by_name(keymap, "AE09");
    assert(kc != XKB_KEYCODE_INVALID);
    keyname = xkb_keymap_key_get_name(keymap, kc);
    assert(streq(keyname, "AE09"));

    kc = xkb_keymap_key_by_name(keymap, "COMP");
    assert(kc != XKB_KEYCODE_INVALID);
    keyname = xkb_keymap_key_get_name(keymap, kc);
    assert(streq(keyname, "COMP"));

    kc = xkb_keymap_key_by_name(keymap, "MENU");
    assert(kc != XKB_KEYCODE_INVALID);
    keyname = xkb_keymap_key_get_name(keymap, kc);
    assert(streq(keyname, "COMP"));

    kc = xkb_keymap_key_by_name(keymap, "AC01");
    assert(kc != XKB_KEYCODE_INVALID);

    // AC01 level 0 ('a') requires no modifiers on us-pc104
    mask_count = xkb_keymap_key_get_mods_for_level(keymap, kc, 0, 0, masks_out, 4);
    assert(mask_count == 1);
    assert(masks_out[0] == 0);

    shift_mask = 1 << xkb_keymap_mod_get_index(keymap, "Shift");
    lock_mask = 1 << xkb_keymap_mod_get_index(keymap, "Lock");
    mod2_mask = 1 << xkb_keymap_mod_get_index(keymap, "Mod2");

    // AC01 level 1 ('A') requires either Shift or Lock modifiers on us-pc104
    mask_count = xkb_keymap_key_get_mods_for_level(keymap, kc, 0, 1, masks_out, 4);
    assert(mask_count == 2);
    assert(masks_out[0] == shift_mask);
    assert(masks_out[1] == lock_mask);

    kc = xkb_keymap_key_by_name(keymap, "KP1");

    // KP1 level 0 ('End') requires no modifiers or Shift+Mod2 on us-pc104
    mask_count = xkb_keymap_key_get_mods_for_level(keymap, kc, 0, 0, masks_out, 4);
    assert(mask_count == 2);
    assert(masks_out[0] == 0);
    assert(masks_out[1] == (shift_mask | mod2_mask));

    // KP1 level 1 ('1') requires either Shift or Mod2 modifiers on us-pc104
    mask_count = xkb_keymap_key_get_mods_for_level(keymap, kc, 0, 1, masks_out, 4);
    assert(mask_count == 2);
    assert(masks_out[0] == shift_mask);
    assert(masks_out[1] == mod2_mask);

    // Return key is not affected by modifiers on us-pc104
    kc = xkb_keymap_key_by_name(keymap, "RTRN");
    mask_count = xkb_keymap_key_get_mods_for_level(keymap, kc, 0, 0, masks_out, 4);
    assert(mask_count == 1);
    assert(masks_out[0] == 0);

    xkb_keymap_unref(keymap);
    xkb_context_unref(context);
}

#define Mod1Mask (1 << 3)
#define Mod2Mask (1 << 4)
#define Mod3Mask (1 << 5)

static void
test_numeric_keysyms(void)
{
    struct xkb_context *context = test_get_context(0);
    struct xkb_keymap *keymap;
    const struct xkb_key *key;
    xkb_keycode_t kc;
    int keysyms_count;
    const xkb_layout_index_t first_layout = 0;
    const xkb_keysym_t *keysyms;

    assert(context);

    keymap = test_compile_rules(context, "evdev", "pc104", "numeric_keysyms", NULL, NULL);
    assert(keymap);

    kc = xkb_keymap_key_by_name(keymap, "AD01");
    keysyms_count = xkb_keymap_key_get_syms_by_level(keymap, kc, first_layout, 0, &keysyms);
    assert(keysyms_count == 1);
    assert(keysyms[0] == 0x1ffffffd);
    key = XkbKey(keymap, kc);
    assert(key->modmap == Mod1Mask);

    kc = xkb_keymap_key_by_name(keymap, "AD02");
    keysyms_count = xkb_keymap_key_get_syms_by_level(keymap, kc, first_layout, 0, &keysyms);
    assert(keysyms_count == 1);
    assert(keysyms[0] == 0x1ffffffe);
    key = XkbKey(keymap, kc);
    assert(key->modmap == Mod2Mask);

    kc = xkb_keymap_key_by_name(keymap, "AD03");
    keysyms_count = xkb_keymap_key_get_syms_by_level(keymap, kc, first_layout, 0, &keysyms);
    assert(keysyms_count == 1);
    assert(keysyms[0] == 0x1fffffff);
    /* Invalid numeric keysym */
    keysyms_count = xkb_keymap_key_get_syms_by_level(keymap, kc, first_layout, 1, &keysyms);
    assert(keysyms_count == 0);
    key = XkbKey(keymap, kc);
    assert(key->modmap == Mod3Mask);

    xkb_keymap_unref(keymap);
    xkb_context_unref(context);
}

int
main(void)
{
    test_garbage_key();
    test_keymap();
    test_numeric_keysyms();

    return 0;
}
