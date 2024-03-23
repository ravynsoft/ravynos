/*
  Laplacian Pyramid
  Copyright (C) 2006 Yangli Hector Yee

  This program is free software; you can redistribute it and/or modify it under the terms of the
  GNU General Public License as published by the Free Software Foundation; either version 2 of the License,
  or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with this program;
  if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA
*/

#include "lpyramid.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct _lpyramid {
    /* Successively blurred versions of the original image */
    float *levels[MAX_PYR_LEVELS];

    int width;
    int height;
};

static void
convolve (lpyramid_t *pyramid, float *a, const float *b)
/* convolves image b with the filter kernel and stores it in a */
{
    int y,x,i,j;
    const float Kernel[] = {0.05f, 0.25f, 0.4f, 0.25f, 0.05f};
    int width = pyramid->width;
    int height = pyramid->height;

    for (y=0; y<height; y++) {
	for (x=0; x<width; x++) {
	    float sum = 0.f;
	    for (j=-2; j<=2; j++) {
		float sum_i = 0.f;
		int ny=y+j;
		if (ny<0) ny=-ny;
		if (ny>=height) ny=2*height - ny - 1;
		ny *= width;
		for (i=-2; i<=2; i++) {
		    int nx=x+i;
		    if (nx<0) nx=-nx;
		    if (nx>=width) nx=2*width - nx - 1;
		    sum_i += Kernel[i+2] * b[ny + nx];
		}
		sum += sum_i * Kernel[j+2];
	    }
	    *a++ = sum;
	}
    }
}

/*
 * Construction/Destruction
 */

lpyramid_t *
lpyramid_create (float *image, int width, int height)
{
    lpyramid_t *pyramid;
    int i;

    pyramid = malloc (sizeof (lpyramid_t));
    if (pyramid == NULL) {
	fprintf (stderr, "Out of memory.\n");
	exit (1);
    }
    pyramid->width = width;
    pyramid->height = height;

    /* Make the Laplacian pyramid by successively
     * copying the earlier levels and blurring them */
    for (i=0; i<MAX_PYR_LEVELS; i++) {
	pyramid->levels[i] = malloc (width * height * sizeof (float));
	if (pyramid->levels[i] == NULL) {
	    fprintf (stderr, "Out of memory.\n");
	    exit (1);
	}
	if (i == 0) {
	    memcpy (pyramid->levels[i], image, width * height * sizeof (float));
	} else {
	    convolve(pyramid, pyramid->levels[i], pyramid->levels[i - 1]);
	}
    }

    return pyramid;
}

void
lpyramid_destroy (lpyramid_t *pyramid)
{
    int i;

    for (i=0; i<MAX_PYR_LEVELS; i++)
	free (pyramid->levels[i]);

    free (pyramid);
}

float
lpyramid_get_value (lpyramid_t *pyramid, int x, int y, int level)
{
    int index = x + y * pyramid->width;
    int l = level;
    if (l > MAX_PYR_LEVELS)
        l = MAX_PYR_LEVELS;
    return pyramid->levels[l][index];
}
