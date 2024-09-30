/*
 * Copyright © 2012 Ran Benita <ran234@gmail.com>
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

#include "evdev-scancodes.h"
#include "test.h"

int
main(void)
{
    struct xkb_context *ctx = test_get_context(0);
    struct xkb_keymap *keymap;

    assert(ctx);
    keymap = test_compile_rules(ctx, "evdev", "evdev",
                                "us,il,ru,de", ",,phonetic,neo",
                                "grp:alt_shift_toggle,grp:menu_toggle");
    assert(keymap);

    assert(test_key_seq(keymap,
                        KEY_H,  BOTH,  XKB_KEY_h,  NEXT,
                        KEY_E,  BOTH,  XKB_KEY_e,  NEXT,
                        KEY_L,  BOTH,  XKB_KEY_l,  NEXT,
                        KEY_L,  BOTH,  XKB_KEY_l,  NEXT,
                        KEY_O,  BOTH,  XKB_KEY_o,  FINISH));

    /* Simple shifted level. */
    assert(test_key_seq(keymap,
                        KEY_H,          BOTH,  XKB_KEY_h,        NEXT,
                        KEY_LEFTSHIFT,  DOWN,  XKB_KEY_Shift_L,  NEXT,
                        KEY_E,          BOTH,  XKB_KEY_E,        NEXT,
                        KEY_L,          BOTH,  XKB_KEY_L,        NEXT,
                        KEY_LEFTSHIFT,  UP,    XKB_KEY_Shift_L,  NEXT,
                        KEY_L,          BOTH,  XKB_KEY_l,        NEXT,
                        KEY_O,          BOTH,  XKB_KEY_o,        FINISH));

    /* Key repeat shifted and unshifted in the middle. */
    assert(test_key_seq(keymap,
                        KEY_H,           DOWN,    XKB_KEY_h,        NEXT,
                        KEY_H,           REPEAT,  XKB_KEY_h,        NEXT,
                        KEY_H,           REPEAT,  XKB_KEY_h,        NEXT,
                        KEY_LEFTSHIFT,   DOWN,    XKB_KEY_Shift_L,  NEXT,
                        KEY_H,           REPEAT,  XKB_KEY_H,        NEXT,
                        KEY_H,           REPEAT,  XKB_KEY_H,        NEXT,
                        KEY_LEFTSHIFT,   UP,      XKB_KEY_Shift_L,  NEXT,
                        KEY_H,           REPEAT,  XKB_KEY_h,        NEXT,
                        KEY_H,           REPEAT,  XKB_KEY_h,        NEXT,
                        KEY_H,           UP,      XKB_KEY_h,        NEXT,
                        KEY_H,           BOTH,    XKB_KEY_h,        FINISH));

    /* Base modifier cleared on key release... */
    assert(test_key_seq(keymap,
                        KEY_H,          BOTH,  XKB_KEY_h,        NEXT,
                        KEY_LEFTSHIFT,  DOWN,  XKB_KEY_Shift_L,  NEXT,
                        KEY_E,          BOTH,  XKB_KEY_E,        NEXT,
                        KEY_L,          BOTH,  XKB_KEY_L,        NEXT,
                        KEY_LEFTSHIFT,  DOWN,  XKB_KEY_Shift_L,  NEXT,
                        KEY_L,          BOTH,  XKB_KEY_L,        NEXT,
                        KEY_O,          BOTH,  XKB_KEY_O,        FINISH));

    /* ... But only by the keycode that set it. */
    assert(test_key_seq(keymap,
                        KEY_H,           BOTH,  XKB_KEY_h,        NEXT,
                        KEY_LEFTSHIFT,   DOWN,  XKB_KEY_Shift_L,  NEXT,
                        KEY_E,           BOTH,  XKB_KEY_E,        NEXT,
                        KEY_L,           BOTH,  XKB_KEY_L,        NEXT,
                        KEY_RIGHTSHIFT,  UP,    XKB_KEY_Shift_R,  NEXT,
                        KEY_L,           BOTH,  XKB_KEY_L,        NEXT,
                        KEY_O,           BOTH,  XKB_KEY_O,        FINISH));

    /*
     * A base modifier should only be cleared when no other key affecting
     * the modifier is down.
     */
    assert(test_key_seq(keymap,
                        KEY_H,           BOTH,  XKB_KEY_h,        NEXT,
                        KEY_LEFTSHIFT,   DOWN,  XKB_KEY_Shift_L,  NEXT,
                        KEY_E,           BOTH,  XKB_KEY_E,        NEXT,
                        KEY_RIGHTSHIFT,  DOWN,  XKB_KEY_Shift_R,  NEXT,
                        KEY_L,           BOTH,  XKB_KEY_L,        NEXT,
                        KEY_RIGHTSHIFT,  UP,    XKB_KEY_Shift_R,  NEXT,
                        KEY_L,           BOTH,  XKB_KEY_L,        NEXT,
                        KEY_LEFTSHIFT,   UP,    XKB_KEY_Shift_L,  NEXT,
                        KEY_O,           BOTH,  XKB_KEY_o,        FINISH));

    /*
     * Two key presses from the same key (e.g. if two keyboards use the
     * same xkb_state) should only be released after two releases.
     */
    assert(test_key_seq(keymap,
                        KEY_H,           BOTH,  XKB_KEY_h,        NEXT,
                        KEY_LEFTSHIFT,   DOWN,  XKB_KEY_Shift_L,  NEXT,
                        KEY_H,           BOTH,  XKB_KEY_H,        NEXT,
                        KEY_LEFTSHIFT,   DOWN,  XKB_KEY_Shift_L,  NEXT,
                        KEY_H,           BOTH,  XKB_KEY_H,        NEXT,
                        KEY_LEFTSHIFT,   UP,    XKB_KEY_Shift_L,  NEXT,
                        KEY_H,           BOTH,  XKB_KEY_H,        NEXT,
                        KEY_LEFTSHIFT,   UP,    XKB_KEY_Shift_L,  NEXT,
                        KEY_H,           BOTH,  XKB_KEY_h,        FINISH));

    /* Same as above with locked modifiers. */
    assert(test_key_seq(keymap,
                        KEY_H,           BOTH,  XKB_KEY_h,          NEXT,
                        KEY_CAPSLOCK,    DOWN,  XKB_KEY_Caps_Lock,  NEXT,
                        KEY_H,           BOTH,  XKB_KEY_H,          NEXT,
                        KEY_CAPSLOCK,    DOWN,  XKB_KEY_Caps_Lock,  NEXT,
                        KEY_H,           BOTH,  XKB_KEY_H,          NEXT,
                        KEY_CAPSLOCK,    UP,    XKB_KEY_Caps_Lock,  NEXT,
                        KEY_H,           BOTH,  XKB_KEY_H,          NEXT,
                        KEY_CAPSLOCK,    UP,    XKB_KEY_Caps_Lock,  NEXT,
                        KEY_H,           BOTH,  XKB_KEY_H,          NEXT,
                        KEY_CAPSLOCK,    BOTH,  XKB_KEY_Caps_Lock,  NEXT,
                        KEY_H,           BOTH,  XKB_KEY_h,          FINISH));

    /* Group switching / locking. */
    assert(test_key_seq(keymap,
                        KEY_H,        BOTH,  XKB_KEY_h,               NEXT,
                        KEY_E,        BOTH,  XKB_KEY_e,               NEXT,
                        KEY_COMPOSE,  BOTH,  XKB_KEY_ISO_Next_Group,  NEXT,
                        KEY_K,        BOTH,  XKB_KEY_hebrew_lamed,    NEXT,
                        KEY_F,        BOTH,  XKB_KEY_hebrew_kaph,     NEXT,
                        KEY_COMPOSE,  BOTH,  XKB_KEY_ISO_Next_Group,  NEXT,
                        KEY_COMPOSE,  BOTH,  XKB_KEY_ISO_Next_Group,  NEXT,
                        KEY_COMPOSE,  BOTH,  XKB_KEY_ISO_Next_Group,  NEXT,
                        KEY_O,        BOTH,  XKB_KEY_o,               FINISH));

    assert(test_key_seq(keymap,
                        KEY_LEFTSHIFT, DOWN, XKB_KEY_Shift_L,        NEXT,
                        KEY_LEFTALT,   DOWN, XKB_KEY_ISO_Next_Group, NEXT,
                        KEY_LEFTALT,   UP,   XKB_KEY_ISO_Next_Group, NEXT,
                        KEY_LEFTSHIFT, UP,   XKB_KEY_Shift_L,        FINISH));

    assert(test_key_seq(keymap,
                        KEY_LEFTALT,   DOWN, XKB_KEY_Alt_L,          NEXT,
                        KEY_LEFTSHIFT, DOWN, XKB_KEY_ISO_Next_Group, NEXT,
                        KEY_LEFTSHIFT, UP,   XKB_KEY_ISO_Next_Group, NEXT,
                        KEY_LEFTALT,   UP,   XKB_KEY_Alt_L,          FINISH));

    /* Locked modifiers. */
    assert(test_key_seq(keymap,
                        KEY_CAPSLOCK,  BOTH,  XKB_KEY_Caps_Lock,  NEXT,
                        KEY_H,         BOTH,  XKB_KEY_H,          NEXT,
                        KEY_E,         BOTH,  XKB_KEY_E,          NEXT,
                        KEY_L,         BOTH,  XKB_KEY_L,          NEXT,
                        KEY_L,         BOTH,  XKB_KEY_L,          NEXT,
                        KEY_O,         BOTH,  XKB_KEY_O,          FINISH));

    assert(test_key_seq(keymap,
                        KEY_H,         BOTH,  XKB_KEY_h,          NEXT,
                        KEY_E,         BOTH,  XKB_KEY_e,          NEXT,
                        KEY_CAPSLOCK,  BOTH,  XKB_KEY_Caps_Lock,  NEXT,
                        KEY_L,         BOTH,  XKB_KEY_L,          NEXT,
                        KEY_L,         BOTH,  XKB_KEY_L,          NEXT,
                        KEY_CAPSLOCK,  BOTH,  XKB_KEY_Caps_Lock,  NEXT,
                        KEY_O,         BOTH,  XKB_KEY_o,          FINISH));

    assert(test_key_seq(keymap,
                        KEY_H,         BOTH,  XKB_KEY_h,          NEXT,
                        KEY_CAPSLOCK,  DOWN,  XKB_KEY_Caps_Lock,  NEXT,
                        KEY_E,         BOTH,  XKB_KEY_E,          NEXT,
                        KEY_L,         BOTH,  XKB_KEY_L,          NEXT,
                        KEY_L,         BOTH,  XKB_KEY_L,          NEXT,
                        KEY_CAPSLOCK,  UP,    XKB_KEY_Caps_Lock,  NEXT,
                        KEY_O,         BOTH,  XKB_KEY_O,          FINISH));

    assert(test_key_seq(keymap,
                        KEY_H,         BOTH,  XKB_KEY_h,          NEXT,
                        KEY_E,         BOTH,  XKB_KEY_e,          NEXT,
                        KEY_CAPSLOCK,  UP,    XKB_KEY_Caps_Lock,  NEXT,
                        KEY_L,         BOTH,  XKB_KEY_l,          NEXT,
                        KEY_L,         BOTH,  XKB_KEY_l,          NEXT,
                        KEY_O,         BOTH,  XKB_KEY_o,          FINISH));

    /*
     * A key release affecting a locked modifier should clear it
     * regardless of the key press.
     */
    /* assert(test_key_seq(keymap, */
    /*                     KEY_H,         BOTH,  XKB_KEY_h,          NEXT, */
    /*                     KEY_CAPSLOCK,  DOWN,  XKB_KEY_Caps_Lock,  NEXT, */
    /*                     KEY_E,         BOTH,  XKB_KEY_E,          NEXT, */
    /*                     KEY_L,         BOTH,  XKB_KEY_L,          NEXT, */
    /*                     KEY_CAPSLOCK,  UP,    XKB_KEY_Caps_Lock,  NEXT, */
    /*                     KEY_L,         BOTH,  XKB_KEY_L,          NEXT, */
    /*                     KEY_CAPSLOCK,  UP,    XKB_KEY_Caps_Lock,  NEXT, */
    /*                     KEY_O,         BOTH,  XKB_KEY_o,          FINISH)); */

    /* Simple Num Lock sanity check. */
    assert(test_key_seq(keymap,
                        KEY_KP1,      BOTH,  XKB_KEY_KP_End,    NEXT,
                        KEY_NUMLOCK,  BOTH,  XKB_KEY_Num_Lock,  NEXT,
                        KEY_KP1,      BOTH,  XKB_KEY_KP_1,      NEXT,
                        KEY_KP2,      BOTH,  XKB_KEY_KP_2,      NEXT,
                        KEY_NUMLOCK,  BOTH,  XKB_KEY_Num_Lock,  NEXT,
                        KEY_KP2,      BOTH,  XKB_KEY_KP_Down,   FINISH));

    /* Test that the aliases in the ru(phonetic) symbols map work. */
    assert(test_key_seq(keymap,
                        KEY_COMPOSE,     BOTH,  XKB_KEY_ISO_Next_Group,  NEXT,
                        KEY_COMPOSE,     BOTH,  XKB_KEY_ISO_Next_Group,  NEXT,
                        KEY_1,           BOTH,  XKB_KEY_1,               NEXT,
                        KEY_Q,           BOTH,  XKB_KEY_Cyrillic_ya,     NEXT,
                        KEY_LEFTSHIFT,   DOWN,  XKB_KEY_Shift_L,         NEXT,
                        KEY_1,           BOTH,  XKB_KEY_exclam,          NEXT,
                        KEY_Q,           BOTH,  XKB_KEY_Cyrillic_YA,     NEXT,
                        KEY_LEFTSHIFT,   UP,    XKB_KEY_Shift_L,         NEXT,
                        KEY_V,           BOTH,  XKB_KEY_Cyrillic_zhe,    NEXT,
                        KEY_CAPSLOCK,    BOTH,  XKB_KEY_Caps_Lock,       NEXT,
                        KEY_1,           BOTH,  XKB_KEY_1,               NEXT,
                        KEY_V,           BOTH,  XKB_KEY_Cyrillic_ZHE,    NEXT,
                        KEY_RIGHTSHIFT,  DOWN,  XKB_KEY_Shift_R,         NEXT,
                        KEY_V,           BOTH,  XKB_KEY_Cyrillic_zhe,    NEXT,
                        KEY_RIGHTSHIFT,  UP,    XKB_KEY_Shift_R,         NEXT,
                        KEY_V,           BOTH,  XKB_KEY_Cyrillic_ZHE,    FINISH));

#define KS(name) xkb_keysym_from_name(name, 0)

    /* Test that levels (1-5) in de(neo) symbols map work. */
    assert(test_key_seq(keymap,
                        /* Switch to the group. */
                        KEY_COMPOSE,     BOTH,  XKB_KEY_ISO_Next_Group,    NEXT,
                        KEY_COMPOSE,     BOTH,  XKB_KEY_ISO_Next_Group,    NEXT,
                        KEY_COMPOSE,     BOTH,  XKB_KEY_ISO_Next_Group,    NEXT,

                        /* Level 1. */
                        KEY_1,           BOTH,  XKB_KEY_1,                 NEXT,
                        KEY_Q,           BOTH,  XKB_KEY_x,                 NEXT,
                        KEY_KP7,         BOTH,  XKB_KEY_KP_7,              NEXT,
                        KEY_ESC,         BOTH,  XKB_KEY_Escape,            NEXT,

                        /* Level 2 with Shift. */
                        KEY_LEFTSHIFT,   DOWN,  XKB_KEY_Shift_L,           NEXT,
                        KEY_1,           BOTH,  XKB_KEY_degree,            NEXT,
                        KEY_Q,           BOTH,  XKB_KEY_X,                 NEXT,
                        KEY_KP7,         BOTH,  KS("U2714"),               NEXT,
                        KEY_ESC,         BOTH,  XKB_KEY_Escape,            NEXT,
                        /*
                         * XXX: de(neo) uses shift(both_capslock) which causes
                         * the interesting result in the next line. Since it's
                         * a key release, it doesn't actually lock the modifier,
                         * and applications by-and-large ignore the keysym on
                         * release(?). Is this a problem?
                         */
                        KEY_LEFTSHIFT,   UP,    XKB_KEY_Caps_Lock,         NEXT,

                        /* Level 2 with the Lock modifier. */
                        KEY_LEFTSHIFT,   DOWN,  XKB_KEY_Shift_L,           NEXT,
                        KEY_RIGHTSHIFT,  BOTH,  XKB_KEY_Caps_Lock,         NEXT,
                        KEY_LEFTSHIFT,   UP,    XKB_KEY_Caps_Lock,         NEXT,
                        KEY_6,           BOTH,  XKB_KEY_6,                 NEXT,
                        KEY_H,           BOTH,  XKB_KEY_S,                 NEXT,
                        KEY_KP3,         BOTH,  XKB_KEY_KP_3,              NEXT,
                        KEY_ESC,         BOTH,  XKB_KEY_Escape,            NEXT,
                        KEY_LEFTSHIFT,   DOWN,  XKB_KEY_Shift_L,           NEXT,
                        KEY_RIGHTSHIFT,  BOTH,  XKB_KEY_Caps_Lock,         NEXT,
                        KEY_LEFTSHIFT,   UP,    XKB_KEY_Caps_Lock,         NEXT,

                        /* Level 3. */
                        KEY_CAPSLOCK,    DOWN,  XKB_KEY_ISO_Level3_Shift,  NEXT,
                        KEY_6,           BOTH,  XKB_KEY_cent,              NEXT,
                        KEY_Q,           BOTH,  XKB_KEY_ellipsis,          NEXT,
                        KEY_KP7,         BOTH,  KS("U2195"),               NEXT,
                        KEY_ESC,         BOTH,  XKB_KEY_Escape,            NEXT,
                        KEY_CAPSLOCK,    UP,    XKB_KEY_ISO_Level3_Shift,  NEXT,

                        /* Level 4. */
                        KEY_CAPSLOCK,    DOWN,  XKB_KEY_ISO_Level3_Shift,  NEXT,
                        KEY_LEFTSHIFT,   DOWN,  XKB_KEY_Shift_L,           NEXT,
                        KEY_5,           BOTH,  XKB_KEY_malesymbol,        NEXT,
                        KEY_E,           BOTH,  XKB_KEY_Greek_lambda,      NEXT,
                        KEY_SPACE,       BOTH,  XKB_KEY_nobreakspace,      NEXT,
                        KEY_KP8,         BOTH,  XKB_KEY_intersection,      NEXT,
                        KEY_ESC,         BOTH,  XKB_KEY_Escape,            NEXT,
                        KEY_LEFTSHIFT,   UP,    XKB_KEY_Caps_Lock,         NEXT,
                        KEY_CAPSLOCK,    UP,    XKB_KEY_ISO_Level3_Shift,  NEXT,

                        /* Level 5. */
                        KEY_RIGHTALT,    DOWN,  XKB_KEY_ISO_Level5_Shift,  NEXT,
                        /* XXX: xkeyboard-config is borked when de(neo) is
                         *      not the first group - not our fault. We test
                         *      Level5 seprately below with only de(neo). */
                        /* KEY_5,           BOTH,  XKB_KEY_periodcentered,    NEXT, */
                        /* KEY_E,           BOTH,  XKB_KEY_Up,                NEXT, */
                        /* KEY_SPACE,       BOTH,  XKB_KEY_KP_0,              NEXT, */
                        /* KEY_KP8,         BOTH,  XKB_KEY_KP_Up,             NEXT, */
                        KEY_ESC,         BOTH,  XKB_KEY_Escape,            NEXT,
                        KEY_RIGHTALT,    UP,    XKB_KEY_ISO_Level5_Shift,  NEXT,

                        KEY_V,           BOTH,  XKB_KEY_p,               FINISH));

    xkb_keymap_unref(keymap);
    keymap = test_compile_rules(ctx, "evdev", "", "de", "neo", "");
    assert(keymap);
    assert(test_key_seq(keymap,
                        /* Level 5. */
                        KEY_RIGHTALT,    DOWN,  XKB_KEY_ISO_Level5_Shift,  NEXT,
                        KEY_5,           BOTH,  XKB_KEY_periodcentered,    NEXT,
                        KEY_E,           BOTH,  XKB_KEY_Up,                NEXT,
                        KEY_SPACE,       BOTH,  XKB_KEY_KP_0,              NEXT,
                        KEY_KP8,         BOTH,  XKB_KEY_KP_Up,             NEXT,
                        KEY_ESC,         BOTH,  XKB_KEY_Escape,            NEXT,
                        KEY_RIGHTALT,    UP,    XKB_KEY_ISO_Level5_Lock,   NEXT,

                        /* Level 6. */
                        KEY_RIGHTALT,    DOWN,  XKB_KEY_ISO_Level5_Shift,  NEXT,
                        KEY_RIGHTSHIFT,  DOWN,  XKB_KEY_Shift_R,           NEXT,
                        KEY_5,           BOTH,  XKB_KEY_NoSymbol,          NEXT,
                        KEY_8,           BOTH,  XKB_KEY_ISO_Left_Tab,      NEXT,
                        KEY_E,           BOTH,  XKB_KEY_Up,                NEXT,
                        KEY_SPACE,       BOTH,  XKB_KEY_KP_0,              NEXT,
                        KEY_KP8,         BOTH,  XKB_KEY_KP_Up,             NEXT,
                        KEY_ESC,         BOTH,  XKB_KEY_Escape,            NEXT,
                        KEY_RIGHTSHIFT,  UP,    XKB_KEY_Caps_Lock,         NEXT,
                        KEY_RIGHTALT,    UP,    XKB_KEY_ISO_Level5_Lock,   NEXT,

                        /* Level 7. */
                        KEY_RIGHTALT,    DOWN,  XKB_KEY_ISO_Level5_Shift,  NEXT,
                        KEY_CAPSLOCK,    DOWN,  XKB_KEY_ISO_Level3_Shift,  NEXT,
                        KEY_5,           BOTH,  KS("U2221"),               NEXT,
                        KEY_E,           BOTH,  XKB_KEY_Greek_LAMBDA,      NEXT,
                        KEY_SPACE,       BOTH,  KS("U202F"),               NEXT,
                        KEY_KP8,         BOTH,  KS("U22C2"),               NEXT,
                        KEY_ESC,         BOTH,  XKB_KEY_Escape,            NEXT,
                        KEY_CAPSLOCK,    UP,    XKB_KEY_ISO_Level3_Shift,  NEXT,
                        KEY_RIGHTALT,    UP,    XKB_KEY_ISO_Level5_Lock,   NEXT,

                        /* Level 8. */
                        KEY_RIGHTALT,    DOWN,  XKB_KEY_ISO_Level5_Shift,  NEXT,
                        KEY_CAPSLOCK,    DOWN,  XKB_KEY_ISO_Level3_Shift,  NEXT,
                        KEY_RIGHTSHIFT,  DOWN,  XKB_KEY_Shift_R,           NEXT,
                        KEY_TAB,         BOTH,  XKB_KEY_ISO_Level5_Lock,   NEXT,
                        KEY_V,           BOTH,  XKB_KEY_Greek_pi,          NEXT,
                        KEY_RIGHTSHIFT,  UP,    XKB_KEY_Caps_Lock,         NEXT,
                        KEY_V,           BOTH,  XKB_KEY_asciitilde,        NEXT,
                        KEY_CAPSLOCK,    UP,    XKB_KEY_ISO_Level3_Shift,  NEXT,
                        KEY_V,           BOTH,  XKB_KEY_p,                 NEXT,
                        KEY_RIGHTALT,    UP,    XKB_KEY_ISO_Level5_Lock,   NEXT,
                        /* Locks Level 5. */

                        KEY_V,           BOTH,  XKB_KEY_Return,            FINISH));


    xkb_keymap_unref(keymap);
    keymap = test_compile_rules(ctx, "evdev", "", "us,il,ru", "",
                                "grp:alt_shift_toggle_bidir,grp:menu_toggle");
    assert(keymap);

    assert(test_key_seq(keymap,
                        KEY_LEFTSHIFT, DOWN, XKB_KEY_Shift_L,        NEXT,
                        KEY_LEFTALT,   DOWN, XKB_KEY_ISO_Prev_Group, NEXT,
                        KEY_LEFTALT,   UP,   XKB_KEY_ISO_Prev_Group, NEXT,
                        KEY_LEFTSHIFT, UP,   XKB_KEY_Shift_L,        FINISH));

    assert(test_key_seq(keymap,
                        KEY_LEFTALT,   DOWN, XKB_KEY_Alt_L,          NEXT,
                        KEY_LEFTSHIFT, DOWN, XKB_KEY_ISO_Prev_Group, NEXT,
                        KEY_LEFTSHIFT, UP,   XKB_KEY_ISO_Prev_Group, NEXT,
                        KEY_LEFTALT,   UP,   XKB_KEY_Alt_L,          FINISH));

    /* Check backwards (negative) group switching and wrapping. */
    assert(test_key_seq(keymap,
                        KEY_H,         BOTH, XKB_KEY_h,              NEXT,
                        KEY_COMPOSE,   BOTH, XKB_KEY_ISO_Next_Group, NEXT,
                        KEY_H,         BOTH, XKB_KEY_hebrew_yod,     NEXT,
                        KEY_COMPOSE,   BOTH, XKB_KEY_ISO_Next_Group, NEXT,
                        KEY_H,         BOTH, XKB_KEY_Cyrillic_er,    NEXT,
                        KEY_LEFTSHIFT, DOWN, XKB_KEY_Shift_L,        NEXT,
                        KEY_LEFTALT,   BOTH, XKB_KEY_ISO_Prev_Group, NEXT,
                        KEY_LEFTSHIFT, UP,   XKB_KEY_Shift_L,        NEXT,
                        KEY_H,         BOTH, XKB_KEY_hebrew_yod,     NEXT,
                        KEY_LEFTSHIFT, DOWN, XKB_KEY_Shift_L,        NEXT,
                        KEY_LEFTALT,   BOTH, XKB_KEY_ISO_Prev_Group, NEXT,
                        KEY_LEFTSHIFT, UP,   XKB_KEY_Shift_L,        NEXT,
                        KEY_H,         BOTH, XKB_KEY_h,              NEXT,
                        KEY_LEFTSHIFT, DOWN, XKB_KEY_Shift_L,        NEXT,
                        KEY_LEFTALT,   BOTH, XKB_KEY_ISO_Prev_Group, NEXT,
                        KEY_LEFTSHIFT, UP,   XKB_KEY_Shift_L,        NEXT,
                        KEY_H,         BOTH, XKB_KEY_Cyrillic_er,    NEXT,
                        KEY_COMPOSE,   BOTH, XKB_KEY_ISO_Next_Group, NEXT,
                        KEY_H,         BOTH, XKB_KEY_h,              FINISH));

    xkb_keymap_unref(keymap);
    keymap = test_compile_rules(ctx, "evdev", "", "us,il,ru", "",
                                "grp:switch,grp:lswitch,grp:menu_toggle");
    assert(keymap);

    /* Test depressed group works (Mode_switch). */
    assert(test_key_seq(keymap,
                        KEY_H,         BOTH, XKB_KEY_h,                 NEXT,
                        KEY_RIGHTALT,  DOWN, XKB_KEY_Mode_switch,       NEXT,
                        KEY_H,         BOTH, XKB_KEY_hebrew_yod,        NEXT,
                        KEY_RIGHTALT,  UP,   XKB_KEY_ISO_Level3_Shift,  NEXT,
                        KEY_H,         BOTH, XKB_KEY_h,                 NEXT,
                        KEY_RIGHTALT,  DOWN, XKB_KEY_Mode_switch,       NEXT,
                        KEY_H,         BOTH, XKB_KEY_hebrew_yod,        NEXT,
                        KEY_RIGHTALT,  UP,   XKB_KEY_ISO_Level3_Shift,  NEXT,
                        KEY_H,         BOTH, XKB_KEY_h,                 FINISH));

    /* Test locked+depressed group works, with wrapping and accumulation. */
    assert(test_key_seq(keymap,
                        KEY_H,         BOTH, XKB_KEY_h,                 NEXT,
                        KEY_COMPOSE,   BOTH, XKB_KEY_ISO_Next_Group,    NEXT,
                        KEY_LEFTALT,   DOWN, XKB_KEY_Mode_switch,       NEXT,
                        KEY_H,         BOTH, XKB_KEY_Cyrillic_er,       NEXT,
                        KEY_LEFTALT,   UP,   XKB_KEY_Mode_switch,       NEXT,
                        KEY_H,         BOTH, XKB_KEY_hebrew_yod,        NEXT,
                        KEY_COMPOSE,   BOTH, XKB_KEY_ISO_Next_Group,    NEXT,
                        KEY_LEFTALT,   DOWN, XKB_KEY_Mode_switch,       NEXT,
                        /* Should wrap back to first group. */
                        KEY_H,         BOTH, XKB_KEY_h,                 NEXT,
                        KEY_LEFTALT,   UP,   XKB_KEY_Mode_switch,       NEXT,
                        KEY_H,         BOTH, XKB_KEY_Cyrillic_er,       NEXT,
                        KEY_COMPOSE,   BOTH, XKB_KEY_ISO_Next_Group,    NEXT,
                        KEY_H,         BOTH, XKB_KEY_h,                 NEXT,
                        /* Two SetGroup(+1)'s should add up. */
                        KEY_RIGHTALT,  DOWN, XKB_KEY_Mode_switch,       NEXT,
                        KEY_H,         BOTH, XKB_KEY_hebrew_yod,        NEXT,
                        KEY_LEFTALT,   DOWN, XKB_KEY_Mode_switch,       NEXT,
                        KEY_H,         BOTH, XKB_KEY_Cyrillic_er,       NEXT,
                        KEY_LEFTALT,   UP,   XKB_KEY_Mode_switch,       NEXT,
                        KEY_H,         BOTH, XKB_KEY_hebrew_yod,        NEXT,
                        KEY_RIGHTALT,  UP,   XKB_KEY_ISO_Level3_Shift,  NEXT,
                        KEY_H,         BOTH, XKB_KEY_h,                 FINISH));

    xkb_keymap_unref(keymap);
    keymap = test_compile_rules(ctx, "evdev", "", "us", "euro", "");
    assert(keymap);

    assert(test_key_seq(keymap,
                        KEY_5,         BOTH, XKB_KEY_5,                 NEXT,
                        KEY_RIGHTALT,  DOWN, XKB_KEY_ISO_Level3_Shift,  NEXT,
                        KEY_5,         BOTH, XKB_KEY_EuroSign,          NEXT,
                        KEY_RIGHTALT,  UP,   XKB_KEY_ISO_Level3_Shift,  FINISH));

    xkb_keymap_unref(keymap);
    keymap = test_compile_file(ctx, "keymaps/unbound-vmod.xkb");
    assert(keymap);

    assert(test_key_seq(keymap,
                        KEY_H,         BOTH, XKB_KEY_h,                 NEXT,
                        KEY_Z,         BOTH, XKB_KEY_y,                 NEXT,
                        KEY_MINUS,     BOTH, XKB_KEY_ssharp,            NEXT,
                        KEY_Z,         BOTH, XKB_KEY_y,                 FINISH));

    xkb_keymap_unref(keymap);
    keymap = test_compile_rules(ctx, "evdev", "applealu_ansi", "us", "",
                                "terminate:ctrl_alt_bksp");
    assert(keymap);

    assert(test_key_seq(keymap,
                        KEY_5,         BOTH, XKB_KEY_5,                 NEXT,
                        KEY_KP1,       BOTH, XKB_KEY_KP_1,              NEXT,
                        KEY_NUMLOCK,   BOTH, XKB_KEY_Clear,             NEXT,
                        KEY_LEFTSHIFT, DOWN, XKB_KEY_Shift_L,           NEXT,
                        KEY_KP1,       BOTH, XKB_KEY_KP_1,              NEXT,
                        KEY_LEFTSHIFT, UP,   XKB_KEY_Shift_L,           NEXT,
                        KEY_CAPSLOCK,  BOTH, XKB_KEY_Caps_Lock,         NEXT,
                        KEY_KP1,       BOTH, XKB_KEY_KP_1,              NEXT,
                        KEY_LEFTSHIFT, DOWN, XKB_KEY_Shift_L,           NEXT,
                        KEY_KP1,       BOTH, XKB_KEY_KP_1,              NEXT,
                        KEY_LEFTSHIFT, UP,   XKB_KEY_Shift_L,           NEXT,
                        KEY_CAPSLOCK,  BOTH, XKB_KEY_Caps_Lock,         NEXT,
                        KEY_A,         BOTH, XKB_KEY_a,                 FINISH));

    xkb_keymap_unref(keymap);
    xkb_context_unref(ctx);
    return 0;
}
