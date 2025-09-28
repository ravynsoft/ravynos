/*
 * Copyright © 2020 Ran Benita <ran@unusedvar.com>
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

#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "xkbcommon/xkbcommon.h"

#define ARRAY_SIZE(arr) ((sizeof(arr) / sizeof(*(arr))))

static void
usage(const char *argv0, FILE *fp)
{
    fprintf(fp, "Usage: %s [--keysym] [--rules <rules>] [--model <model>] "
                "[--layout <layout>] [--variant <variant>] [--options <options>]"
                " <unicode codepoint/keysym>\n", argv0);
}

int
main(int argc, char *argv[])
{
    const char *rules = NULL;
    const char *model = NULL;
    const char *layout_ = NULL;
    const char *variant = NULL;
    const char *options = NULL;
    bool keysym_mode = false;
    int err = EXIT_FAILURE;
    struct xkb_context *ctx = NULL;
    char *endp;
    long val;
    uint32_t codepoint;
    xkb_keysym_t keysym;
    int ret;
    char name[200];
    struct xkb_keymap *keymap = NULL;
    xkb_keycode_t min_keycode, max_keycode;
    xkb_mod_index_t num_mods;
    enum options {
        OPT_KEYSYM,
        OPT_RULES,
        OPT_MODEL,
        OPT_LAYOUT,
        OPT_VARIANT,
        OPT_OPTIONS,
    };
    static struct option opts[] = {
        {"help",                 no_argument,            0, 'h'},
        {"keysym",               no_argument,            0, OPT_KEYSYM},
        {"rules",                required_argument,      0, OPT_RULES},
        {"model",                required_argument,      0, OPT_MODEL},
        {"layout",               required_argument,      0, OPT_LAYOUT},
        {"variant",              required_argument,      0, OPT_VARIANT},
        {"options",              required_argument,      0, OPT_OPTIONS},
        {0, 0, 0, 0},
    };

    while (1) {
        int opt;
        int option_index = 0;

        opt = getopt_long(argc, argv, "h", opts, &option_index);
        if (opt == -1)
            break;

        switch (opt) {
        case OPT_KEYSYM:
            keysym_mode = true;
            break;
        case OPT_RULES:
            rules = optarg;
            break;
        case OPT_MODEL:
            model = optarg;
            break;
        case OPT_LAYOUT:
            layout_ = optarg;
            break;
        case OPT_VARIANT:
            variant = optarg;
            break;
        case OPT_OPTIONS:
            options = optarg;
            break;
        case 'h':
            usage(argv[0], stdout);
            exit(EXIT_SUCCESS);
        default:
            usage(argv[0], stderr);
            exit(EXIT_INVALID_USAGE);
        }
    }
    if (argc - optind != 1) {
        usage(argv[0], stderr);
        exit(EXIT_INVALID_USAGE);
    }

    if (keysym_mode) {
        keysym = xkb_keysym_from_name(argv[optind], XKB_KEYSYM_NO_FLAGS);
        if (keysym == XKB_KEY_NoSymbol) {
            fprintf(stderr, "Failed to convert argument to keysym\n");
            goto err;
        }
    } else {
        errno = 0;
        val = strtol(argv[optind], &endp, 0);
        if (errno != 0 || endp == argv[optind] || val < 0 || val > 0x10FFFF) {
            usage(argv[0], stderr);
            exit(EXIT_INVALID_USAGE);
        }
        codepoint = (uint32_t) val;

        keysym = xkb_utf32_to_keysym(codepoint);
        if (keysym == XKB_KEY_NoSymbol) {
            fprintf(stderr, "Failed to convert codepoint to keysym\n");
            goto err;
        }
    }

    ret = xkb_keysym_get_name(keysym, name, sizeof(name));
    if (ret < 0 || (size_t) ret >= sizeof(name)) {
        fprintf(stderr, "Failed to get name of keysym\n");
        goto err;
    }

    ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!ctx) {
        fprintf(stderr, "Failed to create XKB context\n");
        goto err;
    }

    struct xkb_rule_names names = {
        .rules = rules,
        .model = model,
        .layout = layout_,
        .variant = variant,
        .options = options,
    };
    keymap = xkb_keymap_new_from_names(ctx, &names,
                                       XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (!keymap) {
        fprintf(stderr, "Failed to create XKB keymap\n");
        goto err;
    }

    printf("keysym: %s (%#x)\n", name, keysym);
    printf("%-8s %-9s %-8s %-20s %-7s %-s\n",
           "KEYCODE", "KEY NAME", "LAYOUT", "LAYOUT NAME", "LEVEL#", "MODIFIERS");

    min_keycode = xkb_keymap_min_keycode(keymap);
    max_keycode = xkb_keymap_max_keycode(keymap);
    num_mods = xkb_keymap_num_mods(keymap);
    for (xkb_keycode_t keycode = min_keycode; keycode <= max_keycode; keycode++) {
        const char *key_name;
        xkb_layout_index_t num_layouts;

        key_name = xkb_keymap_key_get_name(keymap, keycode);
        if (!key_name) {
            continue;
        }

        num_layouts = xkb_keymap_num_layouts_for_key(keymap, keycode);
        for (xkb_layout_index_t layout = 0; layout < num_layouts; layout++) {
            const char *layout_name;
            xkb_level_index_t num_levels;

            layout_name = xkb_keymap_layout_get_name(keymap, layout);
            if (!layout_name) {
                layout_name = "?";
            }

            num_levels = xkb_keymap_num_levels_for_key(keymap, keycode, layout);
            for (xkb_level_index_t level = 0; level < num_levels; level++) {
                int num_syms;
                const xkb_keysym_t *syms;
                size_t num_masks;
                xkb_mod_mask_t masks[100];

                num_syms = xkb_keymap_key_get_syms_by_level(
                    keymap, keycode, layout, level, &syms
                );
                if (num_syms != 1) {
                    continue;
                }
                if (syms[0] != keysym) {
                    continue;
                }

                num_masks = xkb_keymap_key_get_mods_for_level(
                    keymap, keycode, layout, level, masks, ARRAY_SIZE(masks)
                );
                for (size_t i = 0; i < num_masks; i++) {
                    xkb_mod_mask_t mask = masks[i];

                    printf("%-8u %-9s %-8u %-20s %-7u [ ",
                           keycode, key_name, layout + 1, layout_name, level + 1);
                    for (xkb_mod_index_t mod = 0; mod < num_mods; mod++) {
                        if ((mask & (1 << mod)) == 0) {
                            continue;
                        }
                        printf("%s ", xkb_keymap_mod_get_name(keymap, mod));
                    }
                    printf("]\n");
                }
            }
        }
    }

    err = EXIT_SUCCESS;
err:
    xkb_keymap_unref(keymap);
    xkb_context_unref(ctx);
    return err;
}
