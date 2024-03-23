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
#ifndef _LPYRAMID_H
#define _LPYRAMID_H

#define MAX_PYR_LEVELS 8

typedef struct _lpyramid lpyramid_t;

lpyramid_t *
lpyramid_create (float *image, int width, int height);

void
lpyramid_destroy (lpyramid_t *pyramid);

float
lpyramid_get_value (lpyramid_t *pyramid, int x, int y, int level);

#endif /* _LPYRAMID_H */
