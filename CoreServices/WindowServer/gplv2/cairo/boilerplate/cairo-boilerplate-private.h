/*
 * Copyright © 2009 Chris Wilson
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Red Hat, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. Red Hat, Inc. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * RED HAT, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL RED HAT, INC. BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Chris Wilson <chris@chris-wilson.co.uk>
 */

#ifndef _CAIRO_BOILERPLATE_PRIVATE_H_
#define _CAIRO_BOILERPLATE_PRIVATE_H_

#include "cairo-boilerplate.h"

CAIRO_BEGIN_DECLS

void
_cairo_boilerplate_register_all (void);

void
_cairo_boilerplate_register_backend (const cairo_boilerplate_target_t *targets,
				     unsigned int		       count);

#define CAIRO_BOILERPLATE(name__, targets__) \
void _register_##name__ (void); \
void _register_##name__ (void) { \
    _cairo_boilerplate_register_backend (targets__, \
					 ARRAY_LENGTH(targets__)); \
}

#define CAIRO_NO_BOILERPLATE(name__) \
void _register_##name__ (void); \
void _register_##name__ (void) { }

CAIRO_END_DECLS

#endif /* _CAIRO_BOILERPLATE_PRIVATE_H_ */
