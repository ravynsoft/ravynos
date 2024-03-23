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

static cairo_status_t
write_func (void *closure,
	    const unsigned char *data,
	    unsigned int length)
{
    if (fwrite (data, length, 1, closure) != 1)
	return CAIRO_STATUS_WRITE_ERROR;

    return CAIRO_STATUS_SUCCESS;
}

int
main (int argc, char **argv)
{
    FILE *in = stdin, *out = stdout;
    cairo_status_t status;
    int i;

    if (argc >= 3) {
	if (strcmp (argv[argc-1], "-")) {
	    out = fopen (argv[argc-1], "w");
	    if (out == NULL) {
		fprintf (stderr, "Failed to open output '%s'\n", argv[argc-1]);
		return 1;
	    }
	}
    }

    if (argc > 2) {
	for (i = 1; i < argc - 1; i++) {
	    in = fopen (argv[i], "r");
	    if (in == NULL) {
		fprintf (stderr, "Failed to open input '%s'\n", argv[i]);
		return 1;
	    }

	    status = cairo_script_interpreter_translate_stream (in, write_func, out);
	    fclose (in);

	    if (status)
		break;
	}
    } else {
	if (argc > 1) {
	    if (strcmp (argv[1], "-")) {
		in = fopen (argv[1], "r");
		if (in == NULL) {
		    fprintf (stderr, "Failed to open input '%s'\n", argv[1]);
		    return 1;
		}
	    }
	}

	status = cairo_script_interpreter_translate_stream (in, write_func, out);

	if (in != stdin)
	    fclose (in);
    }

    if (out != stdout)
	fclose (out);

    if (status) {
	fprintf (stderr, "Translation failed: %s\n",
		cairo_status_to_string (status));
	return status;
    }

    return status;
}
