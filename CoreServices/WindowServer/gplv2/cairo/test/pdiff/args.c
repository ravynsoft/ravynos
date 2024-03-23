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

#include "args.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* copyright =
"PerceptualDiff version 1.0, Copyright (C) 2006 Yangli Hector Yee\n\
PerceptualDiff comes with ABSOLUTELY NO WARRANTY;\n\
This is free software, and you are welcome\n\
to redistribute it under certain conditions;\n\
See the GPL page for details: http://www.gnu.org/copyleft/gpl.html\n\n";

static const char *usage =
"PeceptualDiff image1.tif image2.tif\n\n\
   Compares image1.tif and image2.tif using a perceptually based image metric\n\
   Options:\n\
\t-verbose       : Turns on verbose mode\n\
\t-fov deg       : Field of view in degrees (0.1 to 89.9)\n\
\t-threshold p	 : #pixels p below which differences are ignored\n\
\t-gamma g       : Value to convert rgb into linear space (default 2.2)\n\
\t-luminance l   : White luminance (default 100.0 cdm^-2)\n\
\n\
\n Note: Input files can also be in the PNG format\
\n";

void
args_init (args_t *args)
{
    args->surface_a = NULL;
    args->surface_b = NULL;
    args->Verbose = false;
    args->FieldOfView = 45.0f;
    args->Gamma = 2.2f;
    args->ThresholdPixels = 100;
    args->Luminance = 100.0f;
}

void
args_fini (args_t *args)
{
    cairo_surface_destroy (args->surface_a);
    cairo_surface_destroy (args->surface_b);
}

bool
args_parse (args_t *args, int argc, char **argv)
{
    int i;
    if (argc < 3) {
	fprintf (stderr, "%s", copyright);
	fprintf (stderr, "%s", usage);
	return false;
    }
    for (i = 0; i < argc; i++) {
	if (i == 1) {
	    args->surface_a = cairo_image_surface_create_from_png (argv[1]);
	    if (cairo_surface_status (args->surface_a))
	    {
		fprintf (stderr, "FAIL: Cannot open %s: %s\n",
			 argv[1], cairo_status_to_string (cairo_surface_status (args->surface_a)));
		return false;
	    }
	} else if (i == 2) {
	    args->surface_b = cairo_image_surface_create_from_png (argv[2]);
	    if (cairo_surface_status (args->surface_b))
	    {
		fprintf (stderr, "FAIL: Cannot open %s: %s\n",
			 argv[2], cairo_status_to_string (cairo_surface_status (args->surface_b)));
		return false;
	    }
	} else {
	    if (strstr(argv[i], "-fov")) {
		if (i + 1 < argc) {
		    args->FieldOfView = (float) atof(argv[i + 1]);
		}
	    } else if (strstr(argv[i], "-verbose")) {
		args->Verbose = true;
	    } else 	if (strstr(argv[i], "-threshold")) {
		if (i + 1 < argc) {
		    args->ThresholdPixels = atoi(argv[i + 1]);
		}
	    } else 	if (strstr(argv[i], "-gamma")) {
		if (i + 1 < argc) {
		    args->Gamma = (float) atof(argv[i + 1]);
		}
	    }else 	if (strstr(argv[i], "-luminance")) {
		if (i + 1 < argc) {
		    args->Luminance = (float) atof(argv[i + 1]);
		}
	    }
	}
    } /* i */
    return true;
}

void
args_print (args_t *args)
{
    printf("Field of view is %f degrees\n", args->FieldOfView);
    printf("Threshold pixels is %d pixels\n", args->ThresholdPixels);
    printf("The Gamma is %f\n", args->Gamma);
    printf("The Display's luminance is %f candela per meter squared\n", args->Luminance);
}
