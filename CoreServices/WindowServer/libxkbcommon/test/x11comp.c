/*
 * Copyright © 2014 Ran Benita <ran234@gmail.com>
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

#include <stdio.h>
#include <spawn.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "test.h"
#include "xvfb-wrapper.h"
#include "xkbcommon/xkbcommon-x11.h"

X11_TEST(test_basic)
{
    struct xkb_keymap *keymap;
    xcb_connection_t *conn;
    int32_t device_id;
    int ret, status;
    char *xkb_path;
    char *original, *dump;
    char *envp[] = { NULL };
    char *xkbcomp_argv[] = {
        (char *) "xkbcomp", (char *) "-I", NULL /* xkb_path */, display, NULL
    };
    pid_t xkbcomp_pid;

    conn = xcb_connect(display, NULL);
    if (xcb_connection_has_error(conn)) {
        ret = SKIP_TEST;
        goto err_conn;
    }
    ret = xkb_x11_setup_xkb_extension(conn,
                                      XKB_X11_MIN_MAJOR_XKB_VERSION,
                                      XKB_X11_MIN_MINOR_XKB_VERSION,
                                      XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS,
                                      NULL, NULL, NULL, NULL);
    if (!ret) {
        ret = SKIP_TEST;
        goto err_xcb;
    }
    device_id = xkb_x11_get_core_keyboard_device_id(conn);
    assert(device_id != -1);

    xkb_path = test_get_path("keymaps/host.xkb");
    assert(ret >= 0);
    xkbcomp_argv[2] = xkb_path;
    ret = posix_spawnp(&xkbcomp_pid, "xkbcomp", NULL, NULL, xkbcomp_argv, envp);
    free(xkb_path);
    if (ret != 0) {
        ret = SKIP_TEST;
        goto err_xcb;
    }
    ret = waitpid(xkbcomp_pid, &status, 0);
    if (ret < 0 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        ret = SKIP_TEST;
        goto err_xcb;
    }

    struct xkb_context *ctx = test_get_context(0);
    keymap = xkb_x11_keymap_new_from_device(ctx, conn, device_id,
                                            XKB_KEYMAP_COMPILE_NO_FLAGS);
    assert(keymap);

    original = test_read_file("keymaps/host.xkb");
    assert(original);

    dump = xkb_keymap_get_as_string(keymap, XKB_KEYMAP_USE_ORIGINAL_FORMAT);
    assert(dump);

    if (!streq(original, dump)) {
        fprintf(stderr,
                "round-trip test failed: dumped map differs from original\n");
        fprintf(stderr, "length: dumped %lu, original %lu\n",
                (unsigned long) strlen(dump),
                (unsigned long) strlen(original));
        fprintf(stderr, "dumped map:\n");
        fprintf(stderr, "%s\n", dump);
        ret = 1;
        goto err_dump;
    }

    ret = 0;
err_dump:
    free(original);
    free(dump);
    xkb_keymap_unref(keymap);
    xkb_context_unref(ctx);
err_xcb:
    xcb_disconnect(conn);
err_conn:
    return ret;
}

int main(void) {
    return x11_tests_run();
}
