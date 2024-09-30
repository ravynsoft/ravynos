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

#include <assert.h>
#include <stdio.h>
#include <getopt.h>

#include "xkbcommon/xkbregistry.h"

static void
usage(const char *progname, FILE *fp)
{
    fprintf(fp,
            "Usage: %s [OPTIONS] [/path/to/xkb_base_directory [/path2]...]\n"
            "\n"
            "Options:\n"
            "  --verbose, -v .......... Increase verbosity, use multiple times for debugging output\n"
            "  --ruleset=foo .......... Load the 'foo' ruleset\n"
            "  --skip-default-paths ... Do not load the default XKB paths\n"
            "  --load-exotic .......... Load the exotic (extra) rulesets\n"
            "  --help ................. Print this help and exit\n"
            "\n"
            "Trailing arguments are treated as XKB base directory installations.\n",
            progname);
}

int
main(int argc, char **argv)
{
    int rc = 1;
    struct rxkb_context *ctx = NULL;
    struct rxkb_model *m;
    struct rxkb_layout *l;
    struct rxkb_option_group *g;
    enum rxkb_context_flags flags = RXKB_CONTEXT_NO_FLAGS;
    bool load_defaults = true;
    int verbosity = 0;
    const char *ruleset = DEFAULT_XKB_RULES;

    static const struct option opts[] = {
        {"help",                no_argument,        0, 'h'},
        {"verbose",             no_argument,        0, 'v'},
        {"load-exotic",         no_argument,        0, 'e'},
        {"skip-default-paths",  no_argument,        0, 'd'},
        {"ruleset",             required_argument,  0, 'r'},
        {0, 0, 0, 0},
    };

    while (1) {
        int c;
        int option_index = 0;

        c = getopt_long(argc, argv, "hev", opts, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'h':
                usage(argv[0], stdout);
                return 0;
            case '?':
                usage(argv[0], stderr);
                return EXIT_INVALID_USAGE;
            case 'd':
                load_defaults = false;
                break;
            case 'e':
                flags |= RXKB_CONTEXT_LOAD_EXOTIC_RULES;
                break;
            case 'r':
                ruleset = optarg;
                break;
            case 'v':
                verbosity++;
                break;
        }
    }

    if (optind < argc)
        flags |= RXKB_CONTEXT_NO_DEFAULT_INCLUDES;

    ctx = rxkb_context_new(flags);
    assert(ctx);

    switch (verbosity) {
        case 0:
            rxkb_context_set_log_level(ctx, RXKB_LOG_LEVEL_ERROR);
            break;
        case 1:
            rxkb_context_set_log_level(ctx, RXKB_LOG_LEVEL_INFO);
            break;
        default:
            rxkb_context_set_log_level(ctx, RXKB_LOG_LEVEL_DEBUG);
            break;
    }

    if (optind < argc) {
        for (int i = optind; i < argc; i++) {
            if (!rxkb_context_include_path_append(ctx, argv[i])) {
                fprintf(stderr, "Failed to append include path '%s'\n",
                        argv[i]);
                goto err;
            }
        }

        if (load_defaults) {
            if (!rxkb_context_include_path_append_default(ctx)) {
                fprintf(stderr, "Failed to include default paths.\n");
                goto err;
            }
        }
    }
    if (!rxkb_context_parse(ctx, ruleset)) {
        fprintf(stderr, "Failed to parse XKB descriptions.\n");
        goto err;
    }

    printf("models:\n");
    m = rxkb_model_first(ctx);
    assert(m); /* Empty model list is usually a bug or a bad xml file */
    while (m) {
        const char *vendor = rxkb_model_get_vendor(m);
        printf("- name: %s\n"
               "  vendor: %s\n"
               "  description: %s\n",
               rxkb_model_get_name(m),
               vendor ? vendor : "''",
               rxkb_model_get_description(m));
        m = rxkb_model_next(m);
    }

    printf("\n");
    printf("layouts:\n");
    l = rxkb_layout_first(ctx);
    assert(l); /* Empty layout list is usually a bug or a bad xml file */
    while (l) {
        struct rxkb_iso639_code *iso639;
        struct rxkb_iso3166_code *iso3166;
        const char *variant = rxkb_layout_get_variant(l);
        const char *brief = rxkb_layout_get_brief(l);

        printf("- layout: '%s'\n"
               "  variant: '%s'\n"
               "  brief: '%s'\n"
               "  description: %s\n",
               rxkb_layout_get_name(l),
               variant ? variant : "",
               brief ? brief : "''",
               rxkb_layout_get_description(l));

        printf("  iso639: [");
        iso639 = rxkb_layout_get_iso639_first(l);
        if (iso639) {
            const char *sep = "";
            while (iso639) {
                printf("%s'%s'", sep, rxkb_iso639_code_get_code(iso639));
                iso639 = rxkb_iso639_code_next(iso639);
                sep = ", ";
            }
        }
        printf("]\n");
        printf("  iso3166: [");
        iso3166 = rxkb_layout_get_iso3166_first(l);
        if (iso3166) {
            const char *sep = "";
            while (iso3166) {
                printf("%s'%s'", sep, rxkb_iso3166_code_get_code(iso3166));
                iso3166 = rxkb_iso3166_code_next(iso3166);
                sep = ", ";
            }
        }
        printf("]\n");
        l = rxkb_layout_next(l);
    }
    printf("\n");
    printf("option_groups:\n");
    g = rxkb_option_group_first(ctx);
    assert(g); /* Empty option goups list is usually a bug or a bad xml file */
    while (g) {
        struct rxkb_option *o;

        printf("- name: '%s'\n"
               "  description: %s\n"
               "  allows_multiple: %s\n"
               "  options:\n",
               rxkb_option_group_get_name(g),
               rxkb_option_group_get_description(g),
               rxkb_option_group_allows_multiple(g) ? "true" : "false");

        o = rxkb_option_first(g);
        assert(o); /* Empty option list is usually a bug or a bad xml file */
        while (o) {
            const char *brief = rxkb_option_get_brief(o);

            printf("  - name: '%s'\n"
                   "    brief: '%s'\n"
                   "    description: '%s'\n",
                   rxkb_option_get_name(o),
                   brief ? brief : "",
                   rxkb_option_get_description(o));
            o = rxkb_option_next(o);
        }

        g = rxkb_option_group_next(g);
    }

    rc = 0;

err:
    if (ctx)
        rxkb_context_unref(ctx);

    return rc;
}
