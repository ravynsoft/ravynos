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

#include "cairo-script.h"
#include "cairo-script-interpreter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

static cairo_surface_t *
_script_surface_create (void *closure,
			 cairo_content_t content,
			 double width, double height,
			 long uid)
{
    return cairo_script_surface_create (closure, content, width, height);
}

int
main (int argc, char **argv)
{
    cairo_script_interpreter_t *csi;
    cairo_script_interpreter_hooks_t hooks = {
	.surface_create = _script_surface_create,
    };
    int i;
    char buf[4096];

    csi = cairo_script_interpreter_create ();

    for (i = 1; i < argc; i++) {
        if (strcmp (argv[i], "--version") == 0) {
            printf ("%s: version %s\n", argv[0], __DATE__);
	    exit (0);
        } else if (strcmp (argv[i], "--help") == 0) {
	    printf ("usage: %s < in > out\n", argv[0]);
	    exit (0);
        }

	snprintf (buf, sizeof (buf), "%s.trace", basename (argv[i]));
	cairo_device_destroy (hooks.closure);
	hooks.closure = cairo_script_create (buf);
	cairo_script_interpreter_install_hooks (csi, &hooks);
	cairo_script_interpreter_run (csi, argv[i]);
    }
    cairo_device_destroy (hooks.closure);

    return cairo_script_interpreter_destroy (csi);
}
