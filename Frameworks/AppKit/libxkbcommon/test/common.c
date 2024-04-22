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

#include <limits.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#include <termios.h>
#endif

#include "test.h"
#include "utils.h"

/*
 * Test a sequence of keysyms, resulting from a sequence of key presses,
 * against the keysyms they're supposed to generate.
 *
 * - Each test runs with a clean state.
 * - Each line in the test is made up of:
 *   + A keycode, given as a KEY_* from linux/input.h.
 *   + A direction - DOWN for press, UP for release, BOTH for
 *     immediate press + release, REPEAT to just get the syms.
 *   + A sequence of keysyms that should result from this keypress.
 *
 * The vararg format is:
 * <KEY_*>  <DOWN | UP | BOTH>  <XKB_KEY_* (zero or more)>  <NEXT | FINISH>
 *
 * See below for examples.
 */
int
test_key_seq_va(struct xkb_keymap *keymap, va_list ap)
{
    struct xkb_state *state;

    xkb_keycode_t kc;
    int op;
    xkb_keysym_t keysym;

    const xkb_keysym_t *syms;
    xkb_keysym_t sym;
    unsigned int nsyms, i;
    char ksbuf[64];

    fprintf(stderr, "----\n");

    state = xkb_state_new(keymap);
    assert(state);

    for (;;) {
        kc = va_arg(ap, int) + EVDEV_OFFSET;
        op = va_arg(ap, int);

        nsyms = xkb_state_key_get_syms(state, kc, &syms);
        if (nsyms == 1) {
            sym = xkb_state_key_get_one_sym(state, kc);
            syms = &sym;
        }

        fprintf(stderr, "got %u syms for keycode %u: [", nsyms, kc);

        if (op == DOWN || op == BOTH)
            xkb_state_update_key(state, kc, XKB_KEY_DOWN);
        if (op == UP || op == BOTH)
            xkb_state_update_key(state, kc, XKB_KEY_UP);

        for (i = 0; i < nsyms; i++) {
            keysym = va_arg(ap, int);
            xkb_keysym_get_name(syms[i], ksbuf, sizeof(ksbuf));
            fprintf(stderr, "%s%s", (i != 0) ? ", " : "", ksbuf);

            if (keysym == FINISH || keysym == NEXT) {
                xkb_keysym_get_name(syms[i], ksbuf, sizeof(ksbuf));
                fprintf(stderr, "Did not expect keysym: %s.\n", ksbuf);
                goto fail;
            }

            if (keysym != syms[i]) {
                xkb_keysym_get_name(keysym, ksbuf, sizeof(ksbuf));
                fprintf(stderr, "Expected keysym: %s. ", ksbuf);;
                xkb_keysym_get_name(syms[i], ksbuf, sizeof(ksbuf));
                fprintf(stderr, "Got keysym: %s.\n", ksbuf);;
                goto fail;
            }
        }

        if (nsyms == 0) {
            keysym = va_arg(ap, int);
            if (keysym != XKB_KEY_NoSymbol) {
                xkb_keysym_get_name(keysym, ksbuf, sizeof(ksbuf));
                fprintf(stderr, "Expected %s, but got no keysyms.\n", ksbuf);
                goto fail;
            }
        }

        fprintf(stderr, "]\n");

        keysym = va_arg(ap, int);
        if (keysym == NEXT)
            continue;
        if (keysym == FINISH)
            break;

        xkb_keysym_get_name(keysym, ksbuf, sizeof(ksbuf));
        fprintf(stderr, "Expected keysym: %s. Didn't get it.\n", ksbuf);
        goto fail;
    }

    xkb_state_unref(state);
    return 1;

fail:
    xkb_state_unref(state);
    return 0;
}

int
test_key_seq(struct xkb_keymap *keymap, ...)
{
    va_list ap;
    int ret;

    va_start(ap, keymap);
    ret = test_key_seq_va(keymap, ap);
    va_end(ap);

    return ret;
}

char *
test_makedir(const char *parent, const char *path)
{
    char *dirname;
    int err;

    dirname = asprintf_safe("%s/%s", parent, path);
    assert(dirname);
#ifdef _WIN32
    err = _mkdir(dirname);
#else
    err = mkdir(dirname, 0777);
#endif
    assert(err == 0);

    return dirname;
}

char *
test_maketempdir(const char *template)
{
#ifdef _WIN32
    const char *basetmp = getenv("TMP");
    if (basetmp == NULL) {
        basetmp = getenv("TEMP");
    }
    if (basetmp == NULL) {
        basetmp = getenv("top_builddir");
    }
    assert(basetmp != NULL);
    char *tmpdir = asprintf_safe("%s/%s", basetmp, template);
    assert(tmpdir != NULL);
    char *tmp = _mktemp(tmpdir);
    assert(tmp == tmpdir);
    int ret = _mkdir(tmp);
    assert(ret == 0);
    return tmpdir;
#else
    char *tmpdir = asprintf_safe("/tmp/%s", template);
    assert(tmpdir != NULL);
    char *tmp = mkdtemp(tmpdir);
    assert(tmp == tmpdir);
    return tmpdir;
#endif
}

char *
test_get_path(const char *path_rel)
{
    char *path;
    const char *srcdir;

    srcdir = getenv("top_srcdir");
    if (!srcdir)
        srcdir = ".";

    if (path_rel[0] == '/')
        return strdup(path_rel);

    path = asprintf_safe("%s/test/data%s%s", srcdir,
                         path_rel[0] ? "/" : "", path_rel);
    if (!path) {
        fprintf(stderr, "Failed to allocate path for %s\n", path_rel);
        return NULL;
    }
    return path;
}

char *
test_read_file(const char *path_rel)
{
    struct stat info;
    char *ret, *tmp, *path;
    int fd, count, remaining;

    path = test_get_path(path_rel);
    if (!path)
        return NULL;

    fd = open(path, O_RDONLY);
    free(path);
    if (fd < 0)
        return NULL;

    if (fstat(fd, &info) != 0) {
        close(fd);
        return NULL;
    }

    ret = malloc(info.st_size + 1);
    if (!ret) {
        close(fd);
        return NULL;
    }

    remaining = info.st_size;
    tmp = ret;
    while ((count = read(fd, tmp, remaining))) {
        remaining -= count;
        tmp += count;
    }
    ret[info.st_size] = '\0';
    close(fd);

    if (remaining != 0) {
        free(ret);
        return NULL;
    }

    return ret;
}

struct xkb_context *
test_get_context(enum test_context_flags test_flags)
{
    enum xkb_context_flags ctx_flags;
    struct xkb_context *ctx;
    char *path;

    ctx_flags = XKB_CONTEXT_NO_DEFAULT_INCLUDES;
    if (test_flags & CONTEXT_ALLOW_ENVIRONMENT_NAMES) {
        unsetenv("XKB_DEFAULT_RULES");
        unsetenv("XKB_DEFAULT_MODEL");
        unsetenv("XKB_DEFAULT_LAYOUT");
        unsetenv("XKB_DEFAULT_VARIANT");
        unsetenv("XKB_DEFAULT_OPTIONS");
    }
    else {
        ctx_flags |= XKB_CONTEXT_NO_ENVIRONMENT_NAMES;
    }

    ctx = xkb_context_new(ctx_flags);
    if (!ctx)
        return NULL;

    path = test_get_path("");
    if (!path) {
        xkb_context_unref(ctx);
        return NULL;
    }

    xkb_context_include_path_append(ctx, path);
    free(path);

    return ctx;
}

struct xkb_keymap *
test_compile_file(struct xkb_context *context, const char *path_rel)
{
    struct xkb_keymap *keymap;
    FILE *file;
    char *path;

    path = test_get_path(path_rel);
    if (!path)
        return NULL;

    file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open path: %s\n", path);
        free(path);
        return NULL;
    }
    assert(file != NULL);

    keymap = xkb_keymap_new_from_file(context, file,
                                      XKB_KEYMAP_FORMAT_TEXT_V1, 0);
    fclose(file);

    if (!keymap) {
        fprintf(stderr, "Failed to compile path: %s\n", path);
        free(path);
        return NULL;
    }

    fprintf(stderr, "Successfully compiled path: %s\n", path);
    free(path);

    return keymap;
}

struct xkb_keymap *
test_compile_string(struct xkb_context *context, const char *string)
{
    struct xkb_keymap *keymap;

    keymap = xkb_keymap_new_from_string(context, string,
                                        XKB_KEYMAP_FORMAT_TEXT_V1, 0);
    if (!keymap) {
        fprintf(stderr, "Failed to compile string\n");
        return NULL;
    }

    return keymap;
}

struct xkb_keymap *
test_compile_buffer(struct xkb_context *context, const char *buf, size_t len)
{
    struct xkb_keymap *keymap;

    keymap = xkb_keymap_new_from_buffer(context, buf, len,
                                        XKB_KEYMAP_FORMAT_TEXT_V1, 0);
    if (!keymap) {
        fprintf(stderr, "Failed to compile keymap from memory buffer\n");
        return NULL;
    }

    return keymap;
}

struct xkb_keymap *
test_compile_rules(struct xkb_context *context, const char *rules,
                   const char *model, const char *layout,
                   const char *variant, const char *options)
{
    struct xkb_keymap *keymap;
    struct xkb_rule_names rmlvo = {
        .rules = isempty(rules) ? NULL : rules,
        .model = isempty(model) ? NULL : model,
        .layout = isempty(layout) ? NULL : layout,
        .variant = isempty(variant) ? NULL : variant,
        .options = isempty(options) ? NULL : options
    };

    if (!rules && !model && !layout && !variant && !options)
        keymap = xkb_keymap_new_from_names(context, NULL, 0);
    else
        keymap = xkb_keymap_new_from_names(context, &rmlvo, 0);

    if (!keymap) {
        fprintf(stderr,
                "Failed to compile RMLVO: '%s', '%s', '%s', '%s', '%s'\n",
                rules, model, layout, variant, options);
        return NULL;
    }

    return keymap;
}
