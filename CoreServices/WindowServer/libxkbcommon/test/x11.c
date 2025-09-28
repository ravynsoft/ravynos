/*
 * Copyright © 2013 Ran Benita <ran234@gmail.com>
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

#include "test.h"
#include "xvfb-wrapper.h"
#include "xkbcommon/xkbcommon-x11.h"

X11_TEST(test_basic)
{
    struct xkb_context *ctx = test_get_context(0);
    xcb_connection_t *conn;
    int ret;
    int32_t device_id;
    struct xkb_keymap *keymap;
    struct xkb_state *state;
    char *dump;
    int exit_code = 0;

    /*
    * The next two steps depend on a running X server with XKB support.
    * If it fails, it's not necessarily an actual problem with the code.
    * So we don't want a FAIL here.
    */
    conn = xcb_connect(display, NULL);
    if (!conn || xcb_connection_has_error(conn)) {
        exit_code = SKIP_TEST;
        goto err_conn;
    }

    ret = xkb_x11_setup_xkb_extension(conn,
                                      XKB_X11_MIN_MAJOR_XKB_VERSION,
                                      XKB_X11_MIN_MINOR_XKB_VERSION,
                                      XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS,
                                      NULL, NULL, NULL, NULL);
    if (!ret) {
        exit_code = SKIP_TEST;
        goto err_conn;
    }

    device_id = xkb_x11_get_core_keyboard_device_id(conn);
    assert(device_id != -1);

    keymap = xkb_x11_keymap_new_from_device(ctx, conn, device_id,
                                            XKB_KEYMAP_COMPILE_NO_FLAGS);
    assert(keymap);

    state = xkb_x11_state_new_from_device(keymap, conn, device_id);
    assert(state);

    dump = xkb_keymap_get_as_string(keymap, XKB_KEYMAP_USE_ORIGINAL_FORMAT);
    assert(dump);
    fputs(dump, stdout);

    /* TODO: Write some X11-specific tests. */

    free(dump);
    xkb_state_unref(state);
    xkb_keymap_unref(keymap);
err_conn:
    xcb_disconnect(conn);
    xkb_context_unref(ctx);

    return exit_code;
}

int main(void) {
    return x11_tests_run();
}
