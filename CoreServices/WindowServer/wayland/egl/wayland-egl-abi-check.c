/*
 * Copyright (c) 2017, NVIDIA CORPORATION. All rights reserved.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <stddef.h> /* offsetof */
#include <stdio.h>  /* printf */

#include "wayland-egl-backend.h" /* Current struct wl_egl_window implementation */

/*
 * Following are previous implementations of wl_egl_window.
 *
 * DO NOT EVER CHANGE!
 */

/* From: 214fc6e850 - Benjamin Franzke : egl: Implement libwayland-egl */
struct wl_egl_window_v0 {
    struct wl_surface *surface;

    int width;
    int height;
    int dx;
    int dy;

    int attached_width;
    int attached_height;
};

/* From: ca3ed3e024 - Ander Conselvan de Oliveira : egl/wayland: Don't invalidate drawable on swap buffers */
struct wl_egl_window_v1 {
    struct wl_surface *surface;

    int width;
    int height;
    int dx;
    int dy;

    int attached_width;
    int attached_height;

    void *private;
    void (*resize_callback)(struct wl_egl_window *, void *);
};

/* From: 690ead4a13 - Stencel, Joanna : egl/wayland-egl: Fix for segfault in dri2_wl_destroy_surface. */
#define WL_EGL_WINDOW_VERSION_v2 2
struct wl_egl_window_v2 {
    struct wl_surface *surface;

    int width;
    int height;
    int dx;
    int dy;

    int attached_width;
    int attached_height;

    void *private;
    void (*resize_callback)(struct wl_egl_window *, void *);
    void (*destroy_window_callback)(void *);
};

/* From: 2d5d61bc49 - Miguel A. Vico : wayland-egl: Make wl_egl_window a versioned struct */
#define WL_EGL_WINDOW_VERSION_v3 3
struct wl_egl_window_v3 {
    const intptr_t version;

    int width;
    int height;
    int dx;
    int dy;

    int attached_width;
    int attached_height;

    void *driver_private;
    void (*resize_callback)(struct wl_egl_window *, void *);
    void (*destroy_window_callback)(void *);

    struct wl_surface *surface;
};


/* This program checks we keep a backwards-compatible struct wl_egl_window
 * definition whenever it is modified in wayland-egl-backend.h.
 *
 * The previous definition should be added above as a new struct
 * wl_egl_window_vN, and the appropriate checks should be added below
 */

#define MEMBER_SIZE(type, member) sizeof(((type *)0)->member)

#define CHECK_RENAMED_MEMBER(a_ver, b_ver, a_member, b_member)                      \
    do {                                                                            \
        if (offsetof(struct wl_egl_window ## a_ver, a_member) !=                    \
            offsetof(struct wl_egl_window ## b_ver, b_member)) {                    \
            printf("Backwards incompatible change detected!\n   "                    \
                   "offsetof(struct wl_egl_window" #a_ver "::" #a_member ") != "    \
                   "offsetof(struct wl_egl_window" #b_ver "::" #b_member ")\n");    \
            return 1;                                                               \
        }                                                                           \
                                                                                    \
        if (MEMBER_SIZE(struct wl_egl_window ## a_ver, a_member) !=                 \
            MEMBER_SIZE(struct wl_egl_window ## b_ver, b_member)) {                 \
            printf("Backwards incompatible change detected!\n   "                    \
                   "MEMBER_SIZE(struct wl_egl_window" #a_ver "::" #a_member ") != " \
                   "MEMBER_SIZE(struct wl_egl_window" #b_ver "::" #b_member ")\n"); \
            return 1;                                                               \
        }                                                                           \
    } while (0)

#define CHECK_MEMBER(a_ver, b_ver, member) CHECK_RENAMED_MEMBER(a_ver, b_ver, member, member)
#define CHECK_MEMBER_CURRENT(a_ver, member) CHECK_MEMBER(a_ver,, member)

#define CHECK_SIZE(a_ver, b_ver)                                                    \
    do {                                                                            \
        if (sizeof(struct wl_egl_window ## a_ver) >                                 \
            sizeof(struct wl_egl_window ## b_ver)) {                                \
            printf("Backwards incompatible change detected!\n   "                    \
                   "sizeof(struct wl_egl_window" #a_ver ") > "                      \
                   "sizeof(struct wl_egl_window" #b_ver ")\n");                     \
            return 1;                                                               \
        }                                                                           \
    } while (0)

#define CHECK_SIZE_CURRENT(a_ver)                                                   \
    do {                                                                            \
        if (sizeof(struct wl_egl_window ## a_ver) !=                                \
            sizeof(struct wl_egl_window)) {                                         \
            printf("Backwards incompatible change detected!\n   "                    \
                   "sizeof(struct wl_egl_window" #a_ver ") != "                     \
                   "sizeof(struct wl_egl_window)\n");                               \
            return 1;                                                               \
        }                                                                           \
    } while (0)

#define CHECK_VERSION(a_ver, b_ver)                                                 \
    do {                                                                            \
        if ((WL_EGL_WINDOW_VERSION ## a_ver) >=                                     \
            (WL_EGL_WINDOW_VERSION ## b_ver)) {                                     \
            printf("Backwards incompatible change detected!\n   "                    \
                   "WL_EGL_WINDOW_VERSION" #a_ver " >= "                            \
                   "WL_EGL_WINDOW_VERSION" #b_ver "\n");                            \
            return 1;                                                               \
        }                                                                           \
    } while (0)

#define CHECK_VERSION_CURRENT(a_ver)                                                \
    do {                                                                            \
        if ((WL_EGL_WINDOW_VERSION ## a_ver) !=                                     \
            (WL_EGL_WINDOW_VERSION)) {                                              \
            printf("Backwards incompatible change detected!\n   "                    \
                   "WL_EGL_WINDOW_VERSION" #a_ver " != "                            \
                   "WL_EGL_WINDOW_VERSION\n");                                      \
            return 1;                                                               \
        }                                                                           \
    } while (0)

int main(int argc, char **argv)
{
    /* Check wl_egl_window_v1 ABI against wl_egl_window_v0 */
    CHECK_MEMBER(_v0, _v1, surface);
    CHECK_MEMBER(_v0, _v1, width);
    CHECK_MEMBER(_v0, _v1, height);
    CHECK_MEMBER(_v0, _v1, dx);
    CHECK_MEMBER(_v0, _v1, dy);
    CHECK_MEMBER(_v0, _v1, attached_width);
    CHECK_MEMBER(_v0, _v1, attached_height);

    CHECK_SIZE(_v0, _v1);

    /* Check wl_egl_window_v2 ABI against wl_egl_window_v1 */
    CHECK_MEMBER(_v1, _v2, surface);
    CHECK_MEMBER(_v1, _v2, width);
    CHECK_MEMBER(_v1, _v2, height);
    CHECK_MEMBER(_v1, _v2, dx);
    CHECK_MEMBER(_v1, _v2, dy);
    CHECK_MEMBER(_v1, _v2, attached_width);
    CHECK_MEMBER(_v1, _v2, attached_height);
    CHECK_MEMBER(_v1, _v2, private);
    CHECK_MEMBER(_v1, _v2, resize_callback);

    CHECK_SIZE(_v1, _v2);

    /* Check wl_egl_window_v3 ABI against wl_egl_window_v2 */
    CHECK_RENAMED_MEMBER(_v2, _v3, surface, version);
    CHECK_MEMBER        (_v2, _v3, width);
    CHECK_MEMBER        (_v2, _v3, height);
    CHECK_MEMBER        (_v2, _v3, dx);
    CHECK_MEMBER        (_v2, _v3, dy);
    CHECK_MEMBER        (_v2, _v3, attached_width);
    CHECK_MEMBER        (_v2, _v3, attached_height);
    CHECK_RENAMED_MEMBER(_v2, _v3, private, driver_private);
    CHECK_MEMBER        (_v2, _v3, resize_callback);
    CHECK_MEMBER        (_v2, _v3, destroy_window_callback);

    CHECK_SIZE   (_v2, _v3);
    CHECK_VERSION(_v2, _v3);

    /* Check current wl_egl_window ABI against wl_egl_window_v3 */
    CHECK_MEMBER_CURRENT(_v3, version);
    CHECK_MEMBER_CURRENT(_v3, width);
    CHECK_MEMBER_CURRENT(_v3, height);
    CHECK_MEMBER_CURRENT(_v3, dx);
    CHECK_MEMBER_CURRENT(_v3, dy);
    CHECK_MEMBER_CURRENT(_v3, attached_width);
    CHECK_MEMBER_CURRENT(_v3, attached_height);
    CHECK_MEMBER_CURRENT(_v3, driver_private);
    CHECK_MEMBER_CURRENT(_v3, resize_callback);
    CHECK_MEMBER_CURRENT(_v3, destroy_window_callback);
    CHECK_MEMBER_CURRENT(_v3, surface);

    CHECK_SIZE_CURRENT   (_v3);
    CHECK_VERSION_CURRENT(_v3);

    return 0;
}
