/*
 * Copyright © 2012 Ran Benita <ran234@gmail.com>
 * Copyright © 2019 Red Hat, Inc.
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
#include "test-config.h"

#include "test.h"
#include "xkbcomp/xkbcomp-priv.h"
#include "xkbcomp/rules.h"

struct test_data {
    /* Rules file */
    const char *rules;

    /* Input */
    const char *model;
    const char *layout;
    const char *variant;
    const char *options;

    /* Expected output */
    const char *keycodes;
    const char *types;
    const char *compat;
    const char *symbols;

    /* Or set this if xkb_components_from_rules() should fail. */
    bool should_fail;
};

static bool
test_rules(struct xkb_context *ctx, struct test_data *data)
{
    bool passed;
    const struct xkb_rule_names rmlvo = {
        data->rules, data->model, data->layout, data->variant, data->options
    };
    struct xkb_component_names kccgst;

    fprintf(stderr, "\n\nChecking : %s\t%s\t%s\t%s\t%s\n", data->rules,
            data->model, data->layout, data->variant, data->options);

    if (data->should_fail)
        fprintf(stderr, "Expecting: FAILURE\n");
    else
        fprintf(stderr, "Expecting: %s\t%s\t%s\t%s\n",
                data->keycodes, data->types, data->compat, data->symbols);

    if (!xkb_components_from_rules(ctx, &rmlvo, &kccgst)) {
        fprintf(stderr, "Received : FAILURE\n");
        return data->should_fail;
    }

    fprintf(stderr, "Received : %s\t%s\t%s\t%s\n",
            kccgst.keycodes, kccgst.types, kccgst.compat, kccgst.symbols);

    passed = streq(kccgst.keycodes, data->keycodes) &&
             streq(kccgst.types, data->types) &&
             streq(kccgst.compat, data->compat) &&
             streq(kccgst.symbols, data->symbols);

    free(kccgst.keycodes);
    free(kccgst.types);
    free(kccgst.compat);
    free(kccgst.symbols);

    return passed;
}

int
main(int argc, char *argv[])
{
    struct xkb_context *ctx;

    setenv("XKB_CONFIG_ROOT", TEST_XKB_CONFIG_ROOT, 1);

    ctx = test_get_context(0);
    assert(ctx);

    struct test_data test1 = {
        .rules = "inc-src-simple",

        .model = "my_model", .layout = "my_layout", .variant = "", .options = "",

        .keycodes = "my_keycodes", .types = "default_types",
        .compat = "default_compat", .symbols = "my_symbols",
    };
    assert(test_rules(ctx, &test1));

    struct test_data test2 = {
        .rules = "inc-src-nested",

        .model = "my_model", .layout = "my_layout", .variant = "", .options = "",

        .keycodes = "my_keycodes", .types = "default_types",
        .compat = "default_compat", .symbols = "my_symbols",
    };
    assert(test_rules(ctx, &test2));

    struct test_data test3 = {
        .rules = "inc-src-looped",

        .model = "my_model", .layout = "my_layout", .variant = "", .options = "",

        .should_fail = true,
    };
    assert(test_rules(ctx, &test3));

    struct test_data test4 = {
        .rules = "inc-src-before-after",

        .model = "before_model", .layout = "my_layout", .variant = "", .options = "",

        .keycodes = "my_keycodes", .types = "default_types",
        .compat = "default_compat", .symbols = "default_symbols",
    };
    assert(test_rules(ctx, &test4));

    struct test_data test5 = {
        .rules = "inc-src-options",

        .model = "my_model", .layout = "my_layout", .variant = "my_variant",
        .options = "option11,my_option,colon:opt,option111",

        .keycodes = "my_keycodes", .types = "default_types",
        .compat = "default_compat+substring+group(bla)|some:compat",
        .symbols = "my_symbols+extra_variant+altwin(menu)",
    };
    assert(test_rules(ctx, &test5));

    struct test_data test6 = {
        .rules = "inc-src-loop-twice",

        .model = "my_model", .layout = "my_layout", .variant = "", .options = "",

        .keycodes = "my_keycodes", .types = "default_types",
        .compat = "default_compat", .symbols = "my_symbols",
    };
    assert(test_rules(ctx, &test6));

    xkb_context_unref(ctx);
    return 0;
}
