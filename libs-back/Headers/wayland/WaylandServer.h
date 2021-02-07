/* <title>WaylandServer</title>

   <abstract>Backend server using Wayland.</abstract>

   Copyright (C) 2020 Free Software Foundation, Inc.

   Author: Sergio L. Pascual <slp@sinrega.org>
   Date: February 2016

   This file is part of the GNUstep Backend.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the
   Free Software Foundation, 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _WaylandServer_h_INCLUDE
#define _WaylandServer_h_INCLUDE

#include "config.h"

#include <GNUstepGUI/GSDisplayServer.h>
#include <wayland-client.h>
#include <cairo/cairo.h>
#include <xkbcommon/xkbcommon.h>

#include "cairo/WaylandCairoSurface.h"
#include "wayland/xdg-shell-client-protocol.h"

struct pointer {
    struct wl_pointer *wlpointer;
    float x;
    float y;
    uint32_t last_click_button;
    uint32_t last_click_time;
    float last_click_x;
    float last_click_y;

    uint32_t serial;
    struct window *focus;
};

typedef struct _WaylandConfig {
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_shell *shell;
    struct wl_shm *shm;
    struct wl_seat *seat;
    struct wl_keyboard *keyboard;
    struct xdg_wm_base *wm_base;

    struct wl_list output_list;
    int output_count;
    struct wl_list window_list;
    int window_count;
    int last_window_id;

    struct pointer pointer;
    struct xkb_context *xkb_context;
    struct {
        struct xkb_keymap *keymap;
        struct xkb_state *state;
        xkb_mod_mask_t control_mask;
        xkb_mod_mask_t alt_mask;
        xkb_mod_mask_t shift_mask;
    } xkb;
    int modifiers;

    int seat_version;

    float mouse_scroll_multiplier;
} WaylandConfig;

struct output {
    WaylandConfig *wlconfig;
    struct wl_output *output;
    uint32_t server_output_id;
    struct wl_list link;
    int alloc_x;
    int alloc_y;
    int width;
    int height;
    int transform;
    int scale;
    char *make;
    char *model;

    void *user_data;
};

struct window {
    WaylandConfig *wlconfig;
    id instance;
    int window_id;
    struct wl_list link;
    BOOL configured; // surface has been configured once

    float pos_x;
    float pos_y;
    float width;
    float height;
    float saved_pos_x;
    float saved_pos_y;
    int is_out;

    unsigned char *data;
    struct wl_buffer *buffer;
    struct wl_surface *surface;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *toplevel;

    struct output *output;
    WaylandCairoSurface *wcs;
};


@interface WaylandServer : GSDisplayServer
{
    WaylandConfig *wlconfig;

    BOOL _mouseInitialized;
}
@end

#endif /* _XGServer_h_INCLUDE */
