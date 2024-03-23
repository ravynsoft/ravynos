/*
  Compare Args
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

#ifndef _ARGS_H
#define _ARGS_H

#include "pdiff.h"

/* Args to pass into the comparison function */
typedef struct _args
{
    cairo_surface_t	*surface_a;		/* Image A */
    cairo_surface_t	*surface_b;		/* Image B */
    bool		Verbose;		/* Print lots of text or not */
    float		FieldOfView;		/* Field of view in degrees */
    float		Gamma;			/* The gamma to convert to linear color space */
    float		Luminance;		/* the display's luminance */
    unsigned int	ThresholdPixels;	/* How many pixels different to ignore */
} args_t;

void
args_init (args_t *args);

void
args_fini (args_t *args);

bool
args_parse (args_t *args, int argc, char **argv);

void
args_print (args_t *args);

#endif
