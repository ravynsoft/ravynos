#ifndef BACKEND_DRM_CVT_H
#define BACKEND_DRM_CVT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <xf86drmMode.h>

void generate_cvt_mode(drmModeModeInfo *mode, int hdisplay, int vdisplay,
	float vrefresh, bool reduced, bool interlaced);

#endif
