/*
 * Copyright © 2012 Intel Corporation
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
 *
 * Author: Daniel Stone <daniel@fooishbar.org>
 */

#include "config.h"

#include "test.h"
#include "context.h"

#include <sys/stat.h>
#include <sys/types.h>

/* keeps a cache of all makedir/maketmpdir directories so we can free and
 * rmdir them in one go, see unmakedirs() */
char *dirnames[64];
int ndirs;

/* keeps a cache of all buffered env vars so we can restore
 * them in one go, see restore_env() */
struct env {
    char *key;
    char *value;
} environment[64];
int nenviron;

static void buffer_env(const char *key)
{
    char *v = getenv(key);

    environment[nenviron].key = strdup(key);
    environment[nenviron].value = v ? strdup(v) : NULL;
    nenviron++;
}

static void restore_env(void)
{
    for (int i = 0; i < nenviron; i++) {
        char *key = environment[i].key,
             *value = environment[i].value;

        if (value)
            setenv(key, value, 1);
        else
            unsetenv(key);

        free(key);
        free(value);
    }
    nenviron = 0;
    memset(environment, 0, sizeof(environment));
}

static const char *makedir(const char *parent, const char *path)
{
    char *dirname = test_makedir(parent, path);
    dirnames[ndirs++] = dirname;
    return dirname;
}

static const char *maketmpdir(void)
{
    char *tmpdir = test_maketempdir("xkbcommon-test.XXXXXX");
    dirnames[ndirs++] = tmpdir;
    return tmpdir;
}

static void unmakedirs(void)
{
    /* backwards order for rmdir to work */
    for (int i = ndirs - 1; i >= 0; i--) {
        char *dir = dirnames[i];
        if (!dir)
            break;
        rmdir(dir);
        free(dir);
    }
    ndirs = 0;
    memset(dirnames, 0, sizeof(dirnames));
}

static void
test_config_root_include_path(void)
{
    struct xkb_context *ctx;
    const char *tmpdir;
    const char *context_path;
    int nincludes;

    buffer_env("XKB_CONFIG_ROOT");
    buffer_env("HOME");
    buffer_env("XDG_CONFIG_HOME");

    tmpdir = maketmpdir();
    setenv("XKB_CONFIG_ROOT", tmpdir, 1);
    unsetenv("HOME");
    unsetenv("XDG_CONFIG_HOME");

    /* built-in path is last */
    ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    nincludes = xkb_context_num_include_paths(ctx);
    assert(nincludes >= 1);
    context_path = xkb_context_include_path_get(ctx, nincludes - 1);
    assert(strcmp(context_path, tmpdir) == 0);
    xkb_context_unref(ctx);

    unmakedirs();
    restore_env();
}

static void
test_config_root_include_path_fallback(void)
{
    struct xkb_context *ctx;
    const char *xkbdir = DFLT_XKB_CONFIG_ROOT;
    const char *context_path;
    int nincludes;

    /* quick and dirty check that the default directory exists.
     * It may not on a vanilla test box if we just run the test
     * suite, so where it's not there just skip this test. */
    struct stat stat_buf;
    int err = stat(xkbdir, &stat_buf);
    if (err != 0)
        return;
    if (!S_ISDIR(stat_buf.st_mode))
        return;

    buffer_env("XKB_CONFIG_ROOT");
    buffer_env("HOME");
    buffer_env("XDG_CONFIG_HOME");

    unsetenv("XKB_CONFIG_ROOT");
    unsetenv("HOME");
    unsetenv("XDG_CONFIG_HOME");

    /* built-in path is last */
    ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    nincludes = xkb_context_num_include_paths(ctx);
    assert(nincludes >= 1);
    context_path = xkb_context_include_path_get(ctx, nincludes - 1);
    assert(strcmp(context_path, xkbdir) == 0);
    xkb_context_unref(ctx);

    unmakedirs();
    restore_env();
}

static void
test_xkbdir_include_path(void)
{
    struct xkb_context *ctx;
    const char *tmpdir;
    const char *xkb_path;
    const char *context_path;

    buffer_env("HOME");
    buffer_env("XDG_CONFIG_HOME");

    tmpdir = maketmpdir();
    xkb_path = makedir(tmpdir, ".xkb");
    setenv("HOME", tmpdir, 1);
    setenv("XDG_CONFIG_HOME", tmpdir, 1);

    /* No XDG directory in our tmpdir, so we expect
     * the $HOME/.xkb to be the first include path */
    ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    assert(xkb_context_num_include_paths(ctx) >= 1);
    context_path = xkb_context_include_path_get(ctx, 0);
    assert(strcmp(context_path, xkb_path) == 0);
    xkb_context_unref(ctx);

    unmakedirs();
    restore_env();
}

static void
test_xdg_include_path(void)
{
    struct xkb_context *ctx;
    const char *tmpdir;
    const char *xdg_path;
    const char *context_path;

    buffer_env("XDG_CONFIG_HOME");

    tmpdir = maketmpdir();
    xdg_path = makedir(tmpdir, "xkb");
    setenv("XDG_CONFIG_HOME", tmpdir, 1);

    /* XDG path is always first */
    ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    assert(xkb_context_num_include_paths(ctx) >= 1);
    context_path = xkb_context_include_path_get(ctx, 0);
    assert(strcmp(context_path, xdg_path) == 0);
    xkb_context_unref(ctx);

    unmakedirs();
    restore_env();
}

static void
test_xdg_include_path_fallback(void)
{
    struct xkb_context *ctx;
    const char *tmpdir;
    const char *xdg_root, *xdg_path;
    const char *context_path;

    buffer_env("XDG_CONFIG_HOME");
    buffer_env("HOME");

    tmpdir = maketmpdir();
    xdg_root = makedir(tmpdir, ".config");
    xdg_path = makedir(xdg_root, "xkb");
    setenv("HOME", tmpdir, 1);
    unsetenv("XDG_CONFIG_HOME");

    /* XDG path is always first, even if fallback */
    ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    assert(xkb_context_num_include_paths(ctx) >= 1);
    context_path = xkb_context_include_path_get(ctx, 0);
    assert(strcmp(context_path, xdg_path) == 0);
    xkb_context_unref(ctx);

    unmakedirs();
    restore_env();
}

static void
test_include_order(void)
{
    struct xkb_context *ctx;
    const char *tmpdir;
    const char *xdg_path;
    const char *xkb_home_path;
    const char *xkb_root_path;
    const char *context_path;

    buffer_env("XKB_CONFIG_ROOT");
    buffer_env("XDG_CONFIG_HOME");
    buffer_env("HOME");

    tmpdir = maketmpdir();
    xdg_path = makedir(tmpdir, "xkb");
    xkb_home_path = makedir(tmpdir, ".xkb");
    xkb_root_path = makedir(tmpdir, "xkbroot");
    setenv("HOME", tmpdir, 1);
    setenv("XDG_CONFIG_HOME", tmpdir, 1);
    setenv("XKB_CONFIG_ROOT", xkb_root_path, 1);

    ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    assert(xkb_context_num_include_paths(ctx) >= 3);
    /* XDG is first */
    context_path = xkb_context_include_path_get(ctx, 0);
    assert(strcmp(context_path, xdg_path) == 0);
    /* $HOME/.xkb is second */
    context_path = xkb_context_include_path_get(ctx, 1);
    assert(strcmp(context_path, xkb_home_path) == 0);
    /* CONFIG_ROOT is last */
    context_path = xkb_context_include_path_get(ctx, 2);
    assert(strcmp(context_path, xkb_root_path) == 0);

    xkb_context_unref(ctx);

    unmakedirs();
    restore_env();
}

int
main(void)
{
    struct xkb_context *context = test_get_context(0);
    xkb_atom_t atom;

    assert(context);

    assert(xkb_context_num_include_paths(context) == 1);
    assert(!xkb_context_include_path_append(context, "¡NONSENSE!"));
    assert(xkb_context_num_include_paths(context) == 1);

    atom = xkb_atom_intern(context, "HELLOjunkjunkjunk", 5);
    assert(atom != XKB_ATOM_NONE);
    assert(streq(xkb_atom_text(context, atom), "HELLO"));

    atom = xkb_atom_intern_literal(context, "HELLOjunkjunkjunk");
    assert(atom != XKB_ATOM_NONE);
    assert(streq(xkb_atom_text(context, atom), "HELLOjunkjunkjunk"));

    xkb_context_unref(context);

    test_config_root_include_path();
    test_config_root_include_path_fallback();
    test_xkbdir_include_path();
    test_xdg_include_path();
    test_xdg_include_path_fallback();
    test_include_order();

    return 0;
}
