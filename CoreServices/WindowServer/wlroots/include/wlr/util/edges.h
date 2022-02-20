/*
 * This is a stable interface of wlroots. Future changes will be limited to:
 *
 * - New functions
 * - New struct members
 * - New enum members
 *
 * Note that wlroots does not make an ABI compatibility promise - in the future,
 * the layout and size of structs used by wlroots may change, requiring code
 * depending on this header to be recompiled (but not edited).
 *
 * Breaking changes are announced in the release notes and follow a 1-year
 * deprecation schedule.
 */

#ifndef WLR_UTIL_EDGES_H
#define WLR_UTIL_EDGES_H

enum wlr_edges {
	WLR_EDGE_NONE = 0,
	WLR_EDGE_TOP = 1 << 0,
	WLR_EDGE_BOTTOM = 1 << 1,
	WLR_EDGE_LEFT = 1 << 2,
	WLR_EDGE_RIGHT = 1 << 3,
};

#endif
