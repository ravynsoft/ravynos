/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __RESISTANCE_H
#define __RESISTANCE_H
#include "labwc.h"

void resistance_move_apply(struct view *view, double *x, double *y);
void resistance_resize_apply(struct view *view, struct wlr_box *new_view_geo);

#endif /* __RESISTANCE_H */
