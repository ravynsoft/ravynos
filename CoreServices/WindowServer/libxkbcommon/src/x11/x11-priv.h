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

#ifndef _XKBCOMMON_X11_PRIV_H
#define _XKBCOMMON_X11_PRIV_H

#include <xcb/xkb.h>

#include "keymap.h"
#include "xkbcommon/xkbcommon-x11.h"

struct x11_atom_interner {
    struct xkb_context *ctx;
    xcb_connection_t *conn;
    bool had_error;
    /* Atoms for which we send a GetAtomName request */
    struct {
        xcb_atom_t from;
        xkb_atom_t *out;
        xcb_get_atom_name_cookie_t cookie;
    } pending[128];
    size_t num_pending;
    /* Atoms which were already pending but queried again */
    struct {
        xcb_atom_t from;
        xkb_atom_t *out;
    } copies[128];
    size_t num_copies;
    /* These are not interned, but saved directly (after XkbEscapeMapName) */
    struct {
        xcb_get_atom_name_cookie_t cookie;
        char **out;
    } escaped[4];
    size_t num_escaped;
};

void
x11_atom_interner_init(struct x11_atom_interner *interner,
                       struct xkb_context *ctx, xcb_connection_t *conn);

void
x11_atom_interner_round_trip(struct x11_atom_interner *interner);

/*
 * Make a xkb_atom_t's from X atoms. The actual write is delayed until the next
 * call to x11_atom_interner_round_trip() or when too many atoms are pending.
 */
void
x11_atom_interner_adopt_atom(struct x11_atom_interner *interner,
                             const xcb_atom_t atom, xkb_atom_t *out);

/*
 * Get a strdup'd and XkbEscapeMapName'd name of an X atom. The actual write is
 * delayed until the next call to x11_atom_interner_round_trip().
 */
void
x11_atom_interner_get_escaped_atom_name(struct x11_atom_interner *interner,
                                        xcb_atom_t atom, char **out);

#endif
