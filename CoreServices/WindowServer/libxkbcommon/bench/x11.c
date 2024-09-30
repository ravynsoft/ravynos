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

#include <assert.h>
#include <stdlib.h>

#include <xcb/xkb.h>

#include "xkbcommon/xkbcommon.h"
#include "xkbcommon/xkbcommon-x11.h"

#include "bench.h"

#define BENCHMARK_ITERATIONS 2500

int
main(void)
{
    int ret;
    xcb_connection_t *conn;
    int32_t device_id;
    struct xkb_context *ctx;
    struct bench bench;
    char *elapsed;

    conn = xcb_connect(NULL, NULL);
    if (!conn || xcb_connection_has_error(conn)) {
        fprintf(stderr, "Couldn't connect to X server: error code %d\n",
                conn ? xcb_connection_has_error(conn) : -1);
        ret = -1;
        goto err_out;
    }

    ret = xkb_x11_setup_xkb_extension(conn,
                                      XKB_X11_MIN_MAJOR_XKB_VERSION,
                                      XKB_X11_MIN_MINOR_XKB_VERSION,
                                      XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS,
                                      NULL, NULL, NULL, NULL);
    if (!ret) {
        fprintf(stderr, "Couldn't setup XKB extension\n");
        goto err_conn;
    }

    device_id = xkb_x11_get_core_keyboard_device_id(conn);
    if (device_id == -1) {
        ret = -1;
        fprintf(stderr, "Couldn't find core keyboard device\n");
        goto err_conn;
    }

    ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!ctx) {
        ret = -1;
        fprintf(stderr, "Couldn't create xkb context\n");
        goto err_conn;
    }

    bench_start(&bench);
    for (int i = 0; i < BENCHMARK_ITERATIONS; i++) {
        struct xkb_keymap *keymap;
        struct xkb_state *state;

        keymap = xkb_x11_keymap_new_from_device(ctx, conn, device_id,
                                                XKB_KEYMAP_COMPILE_NO_FLAGS);
        assert(keymap);

        state = xkb_x11_state_new_from_device(keymap, conn, device_id);
        assert(state);

        xkb_state_unref(state);
        xkb_keymap_unref(keymap);
    }
    bench_stop(&bench);
    ret = 0;

    elapsed = bench_elapsed_str(&bench);
    fprintf(stderr, "retrieved %d keymaps from X in %ss\n",
            BENCHMARK_ITERATIONS, elapsed);
    free(elapsed);

    xkb_context_unref(ctx);
err_conn:
    xcb_disconnect(conn);
err_out:
    return ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
