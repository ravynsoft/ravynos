/*
 * Copyright © 2013 Ran Benita
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

#include "x11-priv.h"

XKB_EXPORT int
xkb_x11_setup_xkb_extension(xcb_connection_t *conn,
                            uint16_t major_xkb_version,
                            uint16_t minor_xkb_version,
                            enum xkb_x11_setup_xkb_extension_flags flags,
                            uint16_t *major_xkb_version_out,
                            uint16_t *minor_xkb_version_out,
                            uint8_t *base_event_out,
                            uint8_t *base_error_out)
{
    uint8_t base_event, base_error;
    uint16_t server_major, server_minor;

    if (flags & ~(XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS)) {
        /* log_err_func(ctx, "unrecognized flags: %#x\n", flags); */
        return 0;
    }

    {
        const xcb_query_extension_reply_t *reply =
            xcb_get_extension_data(conn, &xcb_xkb_id);
        if (!reply) {
            /* log_err_func(ctx, "failed to query for XKB extension\n"); */
            return 0;
        }

        if (!reply->present) {
            /* log_err_func(ctx, "failed to start using XKB extension: not available in server\n"); */
            return 0;
        }

        base_event = reply->first_event;
        base_error = reply->first_error;
    }

    {
        xcb_generic_error_t *error = NULL;
        xcb_xkb_use_extension_cookie_t cookie =
            xcb_xkb_use_extension(conn, major_xkb_version, minor_xkb_version);
        xcb_xkb_use_extension_reply_t *reply =
            xcb_xkb_use_extension_reply(conn, cookie, &error);

        if (!reply) {
            /* log_err_func(ctx, */
            /*              "failed to start using XKB extension: error code %d\n", */
            /*              error ? error->error_code : -1); */
            free(error);
            return 0;
        }

        if (!reply->supported) {
            /* log_err_func(ctx, */
            /*              "failed to start using XKB extension: server doesn't support version %d.%d\n", */
            /*              major_xkb_version, minor_xkb_version); */
            free(reply);
            return 0;
        }

        server_major = reply->serverMajor;
        server_minor = reply->serverMinor;

        free(reply);
    }

    /*
    * The XkbUseExtension() in libX11 has a *bunch* of legacy stuff, but
    * it doesn't seem like any of it is useful to us.
    */

    if (major_xkb_version_out)
        *major_xkb_version_out = server_major;
    if (minor_xkb_version_out)
        *minor_xkb_version_out = server_minor;
    if (base_event_out)
        *base_event_out = base_event;
    if (base_error_out)
        *base_error_out = base_error;

    return 1;
}

XKB_EXPORT int32_t
xkb_x11_get_core_keyboard_device_id(xcb_connection_t *conn)
{
    int32_t device_id;
    xcb_xkb_get_device_info_cookie_t cookie =
        xcb_xkb_get_device_info(conn, XCB_XKB_ID_USE_CORE_KBD,
                                0, 0, 0, 0, 0, 0);
    xcb_xkb_get_device_info_reply_t *reply =
        xcb_xkb_get_device_info_reply(conn, cookie, NULL);

    if (!reply)
        return -1;

    device_id = reply->deviceID;
    free(reply);
    return device_id;
}

struct x11_atom_cache {
    /*
     * Invalidate the cache based on the XCB connection.
     * X11 atoms are actually not per connection or client, but per X server
     * session. But better be safe just in case we survive an X server restart.
     */
    xcb_connection_t *conn;
    struct {
        xcb_atom_t from;
        xkb_atom_t to;
    } cache[256];
    size_t len;
};

static struct x11_atom_cache *
get_cache(struct xkb_context *ctx, xcb_connection_t *conn)
{
    if (!ctx->x11_atom_cache) {
        ctx->x11_atom_cache = calloc(1, sizeof(struct x11_atom_cache));
    }
    /* Can be NULL in case the malloc failed. */
    struct x11_atom_cache *cache = ctx->x11_atom_cache;
    if (cache && cache->conn != conn) {
        cache->conn = conn;
        cache->len = 0;
    }
    return cache;
}

void
x11_atom_interner_init(struct x11_atom_interner *interner,
                       struct xkb_context *ctx, xcb_connection_t *conn)
{
    interner->had_error = false;
    interner->ctx = ctx;
    interner->conn = conn;
    interner->num_pending = 0;
    interner->num_copies = 0;
    interner->num_escaped = 0;
}

void
x11_atom_interner_adopt_atom(struct x11_atom_interner *interner,
                             const xcb_atom_t atom, xkb_atom_t *out)
{
    *out = XKB_ATOM_NONE;

    if (atom == XCB_ATOM_NONE)
        return;

    /* Can be NULL in case the malloc failed. */
    struct x11_atom_cache *cache = get_cache(interner->ctx, interner->conn);

retry:

    /* Already in the cache? */
    if (cache) {
        for (size_t c = 0; c < cache->len; c++) {
            if (cache->cache[c].from == atom) {
                *out = cache->cache[c].to;
                return;
            }
        }
    }

    /* Already pending? */
    for (size_t i = 0; i < interner->num_pending; i++) {
        if (interner->pending[i].from == atom) {
            if (interner->num_copies == ARRAY_SIZE(interner->copies)) {
                x11_atom_interner_round_trip(interner);
                goto retry;
            }

            size_t idx = interner->num_copies++;
            interner->copies[idx].from = atom;
            interner->copies[idx].out = out;
            return;
        }
    }

    /* We have to send a GetAtomName request */
    if (interner->num_pending == ARRAY_SIZE(interner->pending)) {
        x11_atom_interner_round_trip(interner);
        assert(interner->num_pending < ARRAY_SIZE(interner->pending));
    }
    size_t idx = interner->num_pending++;
    interner->pending[idx].from = atom;
    interner->pending[idx].out = out;
    interner->pending[idx].cookie = xcb_get_atom_name(interner->conn, atom);
}

void
x11_atom_interner_round_trip(struct x11_atom_interner *interner) {
    struct xkb_context *ctx = interner->ctx;
    xcb_connection_t *conn = interner->conn;

    /* Can be NULL in case the malloc failed. */
    struct x11_atom_cache *cache = get_cache(ctx, conn);

    for (size_t i = 0; i < interner->num_pending; i++) {
        xcb_get_atom_name_reply_t *reply;

        reply = xcb_get_atom_name_reply(conn, interner->pending[i].cookie, NULL);
        if (!reply) {
            interner->had_error = true;
            continue;
        }
        xcb_atom_t x11_atom = interner->pending[i].from;
        xkb_atom_t atom = xkb_atom_intern(ctx,
                                          xcb_get_atom_name_name(reply),
                                          xcb_get_atom_name_name_length(reply));
        free(reply);

        if (cache && cache->len < ARRAY_SIZE(cache->cache)) {
            size_t idx = cache->len++;
            cache->cache[idx].from = x11_atom;
            cache->cache[idx].to = atom;
        }

        *interner->pending[i].out = atom;

        for (size_t j = 0; j < interner->num_copies; j++) {
            if (interner->copies[j].from == x11_atom)
                *interner->copies[j].out = atom;
        }
    }

    for (size_t i = 0; i < interner->num_escaped; i++) {
        xcb_get_atom_name_reply_t *reply;
        int length;
        char *name;
        char **out = interner->escaped[i].out;

        reply = xcb_get_atom_name_reply(conn, interner->escaped[i].cookie, NULL);
        *interner->escaped[i].out = NULL;
        if (!reply) {
            interner->had_error = true;
        } else {
            length = xcb_get_atom_name_name_length(reply);
            name = xcb_get_atom_name_name(reply);

            *out = strndup(name, length);
            free(reply);
            if (*out == NULL) {
                interner->had_error = true;
            } else {
                XkbEscapeMapName(*out);
            }
        }
    }

    interner->num_pending = 0;
    interner->num_copies = 0;
    interner->num_escaped = 0;
}

void
x11_atom_interner_get_escaped_atom_name(struct x11_atom_interner *interner,
                                        xcb_atom_t atom, char **out)
{
    if (atom == 0) {
        *out = NULL;
        return;
    }
    size_t idx = interner->num_escaped++;
    /* There can only be a fixed number of calls to this function "in-flight",
     * thus we assert this number. Increase the array size if this assert fails.
     */
    assert(idx < ARRAY_SIZE(interner->escaped));
    interner->escaped[idx].out = out;
    interner->escaped[idx].cookie = xcb_get_atom_name(interner->conn, atom);
}
