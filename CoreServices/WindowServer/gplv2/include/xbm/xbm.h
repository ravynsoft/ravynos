/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __LABWC_XBM_H
#define __LABWC_XBM_H

#include <wlr/render/wlr_renderer.h>

#include "xbm/parse.h"

/**
 * xbm_load - load theme xbm files into global theme struct
 */
void xbm_load(struct theme *theme, struct wlr_renderer *renderer);

#endif /* __LABWC_XBM_H */
