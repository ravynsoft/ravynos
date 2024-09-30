/*
 * Copyright © 2009 Dan Nicholson <dbn.lists@gmail.com>
 * Copyright © 2012 Intel Corporation
 * Copyright © 2012 Ran Benita <ran234@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the names of the authors or their
 * institutions shall not be used in advertising or otherwise to promote the
 * sale, use or other dealings in this Software without prior written
 * authorization from the authors.
 *
 * Author: Dan Nicholson <dbn.lists@gmail.com>
 *         Daniel Stone <daniel@fooishbar.org>
 *         Ran Benita <ran234@gmail.com>
 */

#include "config.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#include <termios.h>
#endif

#include "tools-common.h"

static void
print_keycode(struct xkb_keymap *keymap, const char* prefix,
              xkb_keycode_t keycode, const char *suffix) {
    const char *keyname = xkb_keymap_key_get_name(keymap, keycode);
    if (keyname) {
        printf("%s%-4s%s", prefix, keyname, suffix);
    } else {
        printf("%s%-4d%s", prefix, keycode, suffix);
    }
}

#ifdef ENABLE_PRIVATE_APIS
#include "src/keymap.h"

void
print_keymap_modmaps(struct xkb_keymap *keymap) {
    printf("Modifiers mapping:\n");
    for (xkb_mod_index_t vmod = 0; vmod < xkb_keymap_num_mods(keymap); vmod++) {
        if (keymap->mods.mods[vmod].type & MOD_REAL)
            continue;
        printf("- %s: ", xkb_keymap_mod_get_name(keymap, vmod));
        if (keymap->mods.mods[vmod].mapping) {
            bool first = true;
            for (xkb_mod_index_t mod = 0; mod < xkb_keymap_num_mods(keymap); mod++) {
                if (keymap->mods.mods[vmod].mapping & (1u << mod)) {
                    if (first) {
                        first = false;
                        printf("%s", xkb_keymap_mod_get_name(keymap, mod));
                    } else {
                        printf("+ %s", xkb_keymap_mod_get_name(keymap, mod));
                    }
                }
            }
        } else {
            printf("(unmapped)");
        }
        printf("\n");
    }
}

#define MODMAP_PADDING  7
#define VMODMAP_PADDING 9
static void
print_key_modmaps(struct xkb_keymap *keymap, xkb_keycode_t keycode) {
    const struct xkb_key *key = XkbKey(keymap, keycode);
    if (key != NULL) {
        xkb_mod_index_t mod;

        printf("modmap [ ");
        if (key->modmap) {
            for (mod = 0; mod < xkb_keymap_num_mods(keymap); mod++) {
                if (key->modmap & (1u << mod)) {
                    printf("%-*s", (int) MODMAP_PADDING,
                           xkb_keymap_mod_get_name(keymap, mod));
                    break;
                }
            }
        } else {
            printf("%*c", (int) MODMAP_PADDING, ' ');
        }

        printf(" ] vmodmap [ ");
        int length = 0;
        const char *mod_name;
        for (mod = 0; mod < xkb_keymap_num_mods(keymap); mod++) {
            if (key->vmodmap & (1u << mod)) {
                mod_name = xkb_keymap_mod_get_name(keymap, mod);
                length += strlen(mod_name) + 1;
                printf("%s ", mod_name);
            }
        }
        if (length < VMODMAP_PADDING) {
            printf("%*c", (int) VMODMAP_PADDING - length, ' ');
        }
        printf("] ");
    }
}

void
print_keys_modmaps(struct xkb_keymap *keymap) {
    const struct xkb_key *key;
    printf("Keys modmaps:\n");
    xkb_keys_foreach(key, keymap) {
        if (key->modmap || key->vmodmap) {
            print_keycode(keymap, "- ", key->keycode, ": ");
            print_key_modmaps(keymap, key->keycode);
            putchar('\n');
        }
    }
}
#endif

void
tools_print_keycode_state(struct xkb_state *state,
                          struct xkb_compose_state *compose_state,
                          xkb_keycode_t keycode,
                          enum xkb_consumed_mode consumed_mode,
                          print_state_fields_mask_t fields)
{
    struct xkb_keymap *keymap;

    xkb_keysym_t sym;
    const xkb_keysym_t *syms;
    int nsyms;
    char s[16];
    xkb_layout_index_t layout;
    enum xkb_compose_status status;

    keymap = xkb_state_get_keymap(state);

    nsyms = xkb_state_key_get_syms(state, keycode, &syms);

    if (nsyms <= 0)
        return;

    status = XKB_COMPOSE_NOTHING;
    if (compose_state)
        status = xkb_compose_state_get_status(compose_state);

    if (status == XKB_COMPOSE_COMPOSING || status == XKB_COMPOSE_CANCELLED)
        return;

    if (status == XKB_COMPOSE_COMPOSED) {
        sym = xkb_compose_state_get_one_sym(compose_state);
        syms = &sym;
        nsyms = 1;
    }
    else if (nsyms == 1) {
        sym = xkb_state_key_get_one_sym(state, keycode);
        syms = &sym;
    }

    print_keycode(keymap, "keycode [ ", keycode, " ] ");

#ifdef ENABLE_PRIVATE_APIS
    if (fields & PRINT_MODMAPS) {
        print_key_modmaps(keymap, keycode);
    }
#endif

    printf("keysyms [ ");
    for (int i = 0; i < nsyms; i++) {
        xkb_keysym_get_name(syms[i], s, sizeof(s));
        printf("%-*s ", (int) sizeof(s), s);
    }
    printf("] ");

    if (fields & PRINT_UNICODE) {
        if (status == XKB_COMPOSE_COMPOSED)
            xkb_compose_state_get_utf8(compose_state, s, sizeof(s));
        else
            xkb_state_key_get_utf8(state, keycode, s, sizeof(s));
        /* HACK: escape single control characters from C0 set using the
        * Unicode codepoint convention. Ideally we would like to escape
        * any non-printable character in the string.
        */
        if (!*s) {
            printf("unicode [   ] ");
        } else if (strlen(s) == 1 && (*s <= 0x1F || *s == 0x7F)) {
            printf("unicode [ U+%04hX ] ", *s);
        } else {
            printf("unicode [ %s ] ", s);
        }
    }

    layout = xkb_state_key_get_layout(state, keycode);
    if (fields & PRINT_LAYOUT) {
        printf("layout [ %s (%d) ] ",
               xkb_keymap_layout_get_name(keymap, layout), layout);
    }

    printf("level [ %d ] ",
           xkb_state_key_get_level(state, keycode, layout));

    printf("mods [ ");
    for (xkb_mod_index_t mod = 0; mod < xkb_keymap_num_mods(keymap); mod++) {
        if (xkb_state_mod_index_is_active(state, mod,
                                          XKB_STATE_MODS_EFFECTIVE) <= 0)
            continue;
        if (xkb_state_mod_index_is_consumed2(state, keycode, mod,
                                             consumed_mode))
            printf("-%s ", xkb_keymap_mod_get_name(keymap, mod));
        else
            printf("%s ", xkb_keymap_mod_get_name(keymap, mod));
    }
    printf("] ");

    printf("leds [ ");
    for (xkb_led_index_t led = 0; led < xkb_keymap_num_leds(keymap); led++) {
        if (xkb_state_led_index_is_active(state, led) <= 0)
            continue;
        printf("%s ", xkb_keymap_led_get_name(keymap, led));
    }
    printf("] ");

    printf("\n");
}

void
tools_print_state_changes(enum xkb_state_component changed)
{
    if (changed == 0)
        return;

    printf("changed [ ");
    if (changed & XKB_STATE_LAYOUT_EFFECTIVE)
        printf("effective-layout ");
    if (changed & XKB_STATE_LAYOUT_DEPRESSED)
        printf("depressed-layout ");
    if (changed & XKB_STATE_LAYOUT_LATCHED)
        printf("latched-layout ");
    if (changed & XKB_STATE_LAYOUT_LOCKED)
        printf("locked-layout ");
    if (changed & XKB_STATE_MODS_EFFECTIVE)
        printf("effective-mods ");
    if (changed & XKB_STATE_MODS_DEPRESSED)
        printf("depressed-mods ");
    if (changed & XKB_STATE_MODS_LATCHED)
        printf("latched-mods ");
    if (changed & XKB_STATE_MODS_LOCKED)
        printf("locked-mods ");
    if (changed & XKB_STATE_LEDS)
        printf("leds ");
    printf("]\n");
}

#ifdef _WIN32
void
tools_disable_stdin_echo(void)
{
    HANDLE stdin_handle = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(stdin_handle, &mode);
    SetConsoleMode(stdin_handle, mode & ~ENABLE_ECHO_INPUT);
}

void
tools_enable_stdin_echo(void)
{
    HANDLE stdin_handle = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(stdin_handle, &mode);
    SetConsoleMode(stdin_handle, mode | ENABLE_ECHO_INPUT);
}
#else
void
tools_disable_stdin_echo(void)
{
    /* Same as `stty -echo`. */
    struct termios termios;
    if (tcgetattr(STDIN_FILENO, &termios) == 0) {
        termios.c_lflag &= ~ECHO;
        (void) tcsetattr(STDIN_FILENO, TCSADRAIN, &termios);
    }
}

void
tools_enable_stdin_echo(void)
{
    /* Same as `stty echo`. */
    struct termios termios;
    if (tcgetattr(STDIN_FILENO, &termios) == 0) {
        termios.c_lflag |= ECHO;
        (void) tcsetattr(STDIN_FILENO, TCSADRAIN, &termios);
    }
}

#endif

int
tools_exec_command(const char *prefix, int real_argc, char **real_argv)
{
    char *argv[64] = {NULL};
    char executable[PATH_MAX];
    const char *command;
    int rc;

    if (((size_t)real_argc >= ARRAY_SIZE(argv))) {
        fprintf(stderr, "Too many arguments\n");
        return EXIT_INVALID_USAGE;
    }

    command = real_argv[0];

    rc = snprintf(executable, sizeof(executable),
                  "%s/%s-%s", LIBXKBCOMMON_TOOL_PATH, prefix, command);
    if (rc < 0 || (size_t) rc >= sizeof(executable)) {
        fprintf(stderr, "Failed to assemble command\n");
        return EXIT_FAILURE;
    }

    argv[0] = executable;
    for (int i = 1; i < real_argc; i++)
        argv[i] = real_argv[i];

    execv(executable, argv);
    if (errno == ENOENT) {
        fprintf(stderr, "Command '%s' is not available\n", command);
        return EXIT_INVALID_USAGE;
    } else {
        fprintf(stderr, "Failed to execute '%s' (%s)\n",
                command, strerror(errno));
    }

    return EXIT_FAILURE;
}
