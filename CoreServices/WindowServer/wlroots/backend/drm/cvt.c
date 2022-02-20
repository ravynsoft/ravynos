/*
 * Copyright 2005-2006 Luc Verhaegen.
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
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "backend/drm/cvt.h"

/* top/bottom margin size (% of height) - default: 1.8 */
#define CVT_MARGIN_PERCENTAGE 1.8

/* character cell horizontal granularity (pixels) - default 8 */
#define CVT_H_GRANULARITY 8

/* Minimum vertical porch (lines) - default 3 */
#define CVT_MIN_V_PORCH 3

/* Minimum number of vertical back porch lines - default 6 */
#define CVT_MIN_V_BPORCH 6

/* Pixel clock step (kHz) */
#define CVT_CLOCK_STEP 250

/* Minimum time of vertical sync + back porch interval (µs)
 * default 550.0 */
#define CVT_MIN_VSYNC_BP 550.0

/* Nominal hsync width (% of line period) - default 8 */
#define CVT_HSYNC_PERCENTAGE 8

/* Definition of Horizontal blanking time limitation */
/* Gradient (%/kHz) - default 600 */
#define CVT_M_FACTOR 600

/* Offset (%) - default 40 */
#define CVT_C_FACTOR 40

/* Blanking time scaling factor - default 128 */
#define CVT_K_FACTOR 128

/* Scaling factor weighting - default 20 */
#define CVT_J_FACTOR 20

#define CVT_M_PRIME CVT_M_FACTOR * CVT_K_FACTOR / 256
#define CVT_C_PRIME (CVT_C_FACTOR - CVT_J_FACTOR) * CVT_K_FACTOR / 256 + \
	CVT_J_FACTOR

/* Minimum vertical blanking interval time (µs) - default 460 */
#define CVT_RB_MIN_VBLANK 460.0

/* Fixed number of clocks for horizontal sync */
#define CVT_RB_H_SYNC 32.0

/* Fixed number of clocks for horizontal blanking */
#define CVT_RB_H_BLANK 160.0

/* Fixed number of lines for vertical front porch - default 3 */
#define CVT_RB_VFPORCH 3

/*
 * Generate a CVT standard mode from hdisplay, vdisplay and vrefresh.
 *
 * These calculations are stolen from the CVT calculation spreadsheet written
 * by Graham Loveridge. He seems to be claiming no copyright and there seems to
 * be no license attached to this. He apparently just wants to see his name
 * mentioned.
 *
 * This file can be found at http://www.vesa.org/Public/CVT/CVTd6r1.xls
 *
 * Comments and structure corresponds to the comments and structure of the xls.
 * This should ease importing of future changes to the standard (not very
 * likely though).
 *
 * This function is borrowed from xorg-xserver's xf86CVTmode.
 */
void generate_cvt_mode(drmModeModeInfo *mode, int hdisplay, int vdisplay,
		float vrefresh, bool reduced, bool interlaced) {
	bool margins = false;
	float vfield_rate, hperiod;
	int hdisplay_rnd, hmargin;
	int vdisplay_rnd, vmargin, vsync;
	float interlace; /* Please rename this */

	/* CVT default is 60.0Hz */
	if (!vrefresh) {
		vrefresh = 60.0;
	}

	/* 1. Required field rate */
	if (interlaced) {
		vfield_rate = vrefresh * 2;
	} else {
		vfield_rate = vrefresh;
	}

	/* 2. Horizontal pixels */
	hdisplay_rnd = hdisplay - (hdisplay % CVT_H_GRANULARITY);

	/* 3. Determine left and right borders */
	if (margins) {
		/* right margin is actually exactly the same as left */
		hmargin = (((float) hdisplay_rnd) * CVT_MARGIN_PERCENTAGE / 100.0);
		hmargin -= hmargin % CVT_H_GRANULARITY;
	} else {
		hmargin = 0;
	}

	/* 4. Find total active pixels */
	mode->hdisplay = hdisplay_rnd + 2 * hmargin;

	/* 5. Find number of lines per field */
	if (interlaced) {
		vdisplay_rnd = vdisplay / 2;
	} else {
		vdisplay_rnd = vdisplay;
	}

	/* 6. Find top and bottom margins */
	/* nope. */
	if (margins) {
		/* top and bottom margins are equal again. */
		vmargin = (((float) vdisplay_rnd) * CVT_MARGIN_PERCENTAGE / 100.0);
	} else {
		vmargin = 0;
	}

	mode->vdisplay = vdisplay + 2 * vmargin;

	/* 7. interlace */
	if (interlaced) {
		interlace = 0.5;
	} else {
		interlace = 0.0;
	}

	/* Determine vsync Width from aspect ratio */
	if (!(vdisplay % 3) && ((vdisplay * 4 / 3) == hdisplay)) {
		vsync = 4;
	} else if (!(vdisplay % 9) && ((vdisplay * 16 / 9) == hdisplay)) {
		vsync = 5;
	} else if (!(vdisplay % 10) && ((vdisplay * 16 / 10) == hdisplay)) {
		vsync = 6;
	} else if (!(vdisplay % 4) && ((vdisplay * 5 / 4) == hdisplay)) {
		vsync = 7;
	} else if (!(vdisplay % 9) && ((vdisplay * 15 / 9) == hdisplay)) {
		vsync = 7;
	} else { /* Custom */
		vsync = 10;
	}

	if (!reduced) { /* simplified GTF calculation */
		float hblank_percentage;
		int vsync_and_back_porch, vblank_porch;
		int hblank;

		/* 8. Estimated Horizontal period */
		hperiod = ((float) (1000000.0 / vfield_rate - CVT_MIN_VSYNC_BP)) /
			(vdisplay_rnd + 2 * vmargin + CVT_MIN_V_PORCH + interlace);

		/* 9. Find number of lines in sync + backporch */
		if (((int) (CVT_MIN_VSYNC_BP / hperiod) + 1) <
				(vsync + CVT_MIN_V_PORCH)) {
			vsync_and_back_porch = vsync + CVT_MIN_V_PORCH;
		} else {
			vsync_and_back_porch = (int) (CVT_MIN_VSYNC_BP / hperiod) + 1;
		}

		/* 10. Find number of lines in back porch */
		vblank_porch = vsync_and_back_porch - vsync;
		(void) vblank_porch;

		/* 11. Find total number of lines in vertical field */
		mode->vtotal = vdisplay_rnd + 2 * vmargin + vsync_and_back_porch + interlace
			+ CVT_MIN_V_PORCH;

		/* 12. Find ideal blanking duty cycle from formula */
		hblank_percentage = CVT_C_PRIME - CVT_M_PRIME * hperiod / 1000.0;

		/* 13. Blanking time */
		if (hblank_percentage < 20) {
			hblank_percentage = 20;
		}

		hblank = mode->hdisplay * hblank_percentage / (100.0 - hblank_percentage);
		hblank -= hblank % (2 * CVT_H_GRANULARITY);

		/* 14. Find total number of pixels in a line. */
		mode->htotal = mode->hdisplay + hblank;

		/* Fill in hsync values */
		mode->hsync_end = mode->hdisplay + hblank / 2;

		mode->hsync_start = mode->hsync_end -
			(mode->htotal * CVT_HSYNC_PERCENTAGE) / 100;
		mode->hsync_start += CVT_H_GRANULARITY -
			mode->hsync_start % CVT_H_GRANULARITY;

		/* Fill in vsync values */
		mode->vsync_start = mode->vdisplay + CVT_MIN_V_PORCH;
		mode->vsync_end = mode->vsync_start + vsync;
	} else { /* reduced blanking */
		int vbi_lines;

		/* 8. Estimate Horizontal period. */
		hperiod = ((float) (1000000.0 / vfield_rate - CVT_RB_MIN_VBLANK)) /
			(vdisplay_rnd + 2 * vmargin);

		/* 9. Find number of lines in vertical blanking */
		vbi_lines = ((float) CVT_RB_MIN_VBLANK) / hperiod + 1;

		/* 10. Check if vertical blanking is sufficient */
		if (vbi_lines < (CVT_RB_VFPORCH + vsync + CVT_MIN_V_BPORCH)) {
			vbi_lines = CVT_RB_VFPORCH + vsync + CVT_MIN_V_BPORCH;
		}

		/* 11. Find total number of lines in vertical field */
		mode->vtotal = vdisplay_rnd + 2 * vmargin + interlace + vbi_lines;

		/* 12. Find total number of pixels in a line */
		mode->htotal = mode->hdisplay + CVT_RB_H_BLANK;

		/* Fill in hsync values */
		mode->hsync_end = mode->hdisplay + CVT_RB_H_BLANK / 2;
		mode->hsync_start = mode->hsync_end - CVT_RB_H_SYNC;

		/* Fill in vsync values */
		mode->vsync_start = mode->vdisplay + CVT_RB_VFPORCH;
		mode->vsync_end = mode->vsync_start + vsync;
	}

	/* 15/13. Find pixel clock frequency (kHz for xf86) */
	mode->clock = mode->htotal * 1000.0 / hperiod;
	mode->clock -= mode->clock % CVT_CLOCK_STEP;

	/* 17/15. Find actual Field rate */
	mode->vrefresh = (1000.0 * ((float) mode->clock)) /
		((float) (mode->htotal * mode->vtotal));

	/* 18/16. Find actual vertical frame frequency */
	/* ignore - just set the mode flag for interlaced */
	if (interlaced) {
		mode->vtotal *= 2;
		mode->flags |= DRM_MODE_FLAG_INTERLACE;
	}

	snprintf(mode->name, sizeof(mode->name), "%dx%d", hdisplay, vdisplay);

	if (reduced) {
		mode->flags |= DRM_MODE_FLAG_PHSYNC | DRM_MODE_FLAG_NVSYNC;
	} else {
		mode->flags |= DRM_MODE_FLAG_NHSYNC | DRM_MODE_FLAG_PVSYNC;
	}
}
