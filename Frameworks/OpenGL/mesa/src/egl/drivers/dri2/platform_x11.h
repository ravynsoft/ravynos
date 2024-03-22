/*
 * Copyright 2022 Yonggang Luo
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef EGL_X11_INCLUDED
#define EGL_X11_INCLUDED

#include "egl_dri2.h"

uint32_t
dri2_format_for_depth(struct dri2_egl_display *dri2_dpy, uint32_t depth);

EGLBoolean
dri2_x11_get_msc_rate(_EGLDisplay *display, _EGLSurface *surface,
                      EGLint *numerator, EGLint *denominator);

#endif
