/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1994-1996, 1998-1999, 2004
 *	Todd C. Miller <Todd.Miller@sudo.ws>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef SUDOERS_INSULTS_H
#define SUDOERS_INSULTS_H

#if defined(HAL_INSULTS) || defined(GOONS_INSULTS) || defined(CLASSIC_INSULTS) || defined(CSOPS_INSULTS) || defined(PYTHON_INSULTS)

#include <sudo_rand.h>

/*
 * Use one or more set of insults as determined by configure
 */

const char *insults[] = {

# ifdef HAL_INSULTS
#  include "ins_2001.h"
# endif

# ifdef GOONS_INSULTS
#  include "ins_goons.h"
# endif

# ifdef CLASSIC_INSULTS
#  include "ins_classic.h"
# endif

# ifdef CSOPS_INSULTS
#  include "ins_csops.h"
# endif

# ifdef PYTHON_INSULTS
#  include "ins_python.h"
# endif

    NULL

};

/*
 * How may I insult you?  Let me count the ways...
 */
#define NOFINSULTS (nitems(insults) - 1)

/*
 * return a pseudo-random insult.
 */
#define INSULT		(insults[arc4random_uniform(NOFINSULTS)])

#endif /* HAL_INSULTS || GOONS_INSULTS || CLASSIC_INSULTS || CSOPS_INSULTS || PYTHON_INSULTS */

#endif /* SUDOERS_INSULTS_H */
