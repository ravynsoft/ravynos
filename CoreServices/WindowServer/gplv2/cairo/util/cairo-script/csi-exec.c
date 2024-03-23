/*
 * Copyright © 2008 Chris Wilson <chris@chris-wilson.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 * The Original Code is the cairo graphics library.
 *
 * The Initial Developer of the Original Code is Chris Wilson.
 *
 * Contributor(s):
 *      Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "config.h"

#include "cairo.h"
#include "cairo-script-interpreter.h"

#include <stdio.h>
#include <stdlib.h>

static cairo_surface_t *
_surface_create (void *closure,
		 cairo_content_t content,
		 double width, double height,
		 long uid)
{
    return cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
}

int
main (int argc, char **argv)
{
    const cairo_script_interpreter_hooks_t hooks = {
	.surface_create = _surface_create
    };
    cairo_script_interpreter_t *csi;
    int i;

    for (i = 1; i < argc; i++) {
	int status, line;

	csi = cairo_script_interpreter_create ();
	cairo_script_interpreter_install_hooks (csi, &hooks);
	cairo_script_interpreter_run (csi, argv[i]);
	line = cairo_script_interpreter_get_line_number (csi);
	status = cairo_script_interpreter_destroy (csi);
	if (status) {
	    fprintf (stderr, "Error during replay of '%s', line %d: %d\n",
		     argv[i], line, status);
	    return 1;
	}
    }

    return 0;
}
