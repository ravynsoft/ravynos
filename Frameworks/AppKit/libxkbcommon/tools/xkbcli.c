/*
 * Copyright © 2020 Red Hat, Inc.
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
#include <stdio.h>
#include <stdlib.h>

#include "tools-common.h"

static void
usage(void)
{
    printf("Usage: xkbcli [--help|-h] [--version|-V] <command> [<args>]\n"
           "\n"
           "Global options:\n"
           "  -h, --help ...... show this help and exit\n"
           "  -V, --version ... show version information and exit\n"
           /* WARNING: The following is parsed by the bash completion script.
            *          Any change to the format (in particular to the indentation)
            *          should kept in the script in sync. */
           "Commands:\n"
#if HAVE_XKBCLI_LIST
           "  list\n"
           "    List available rules, models, layouts, variants and options\n"
           "\n"
#endif
#if HAVE_XKBCLI_INTERACTIVE_WAYLAND
           "  interactive-wayland\n"
           "    Interactive debugger for XKB keymaps for Wayland\n"
           "\n"
#endif
#if HAVE_XKBCLI_INTERACTIVE_X11
           "  interactive-x11\n"
           "    Interactive debugger for XKB keymaps for X11\n"
           "\n"
#endif
#if HAVE_XKBCLI_INTERACTIVE_EVDEV
           "  interactive-evdev\n"
           "    Interactive debugger for XKB keymaps for evdev\n"
           "\n"
#endif
#if HAVE_XKBCLI_COMPILE_KEYMAP
           "  compile-keymap\n"
           "    Compile an XKB keymap\n"
           "\n"
#endif
#if HAVE_XKBCLI_HOW_TO_TYPE
           "  how-to-type\n"
           "    Print key sequences to type a Unicode codepoint\n"
           "\n"
#endif
           );
}

int
main(int argc, char **argv)
{
    enum options {
        OPT_HELP = 1,
        OPT_VERSION,
    };
    int option_index = 0;

    while (1) {
        int c;
        static struct option opts[] = {
            { "help",    no_argument, 0, OPT_HELP },
            { "version", no_argument, 0, OPT_VERSION },
            { 0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, "+hV", opts, &option_index);
        if (c == -1)
            break;

        switch(c) {
            case 'h':
            case OPT_HELP:
                usage();
                return EXIT_SUCCESS;
            case 'V':
            case OPT_VERSION:
                printf("%s\n", LIBXKBCOMMON_VERSION);
                return EXIT_SUCCESS;
            default:
                usage();
                return EXIT_INVALID_USAGE;
        }
    }

    if (optind >= argc) {
        usage();
        return EXIT_INVALID_USAGE;
    }

    argv += optind;
    argc -= optind;

    return tools_exec_command("xkbcli", argc, argv);
}
