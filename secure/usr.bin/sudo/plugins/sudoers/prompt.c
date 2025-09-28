/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 1993-1996,1998-2005, 2007-2015
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
 *
 * Sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and Air Force Research Laboratory, Air Force
 * Materiel Command, USAF, under agreement number F39502-99-1-0512.
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>

#include <sudoers.h>

/*
 * Expand %h and %u escapes (if present) in the prompt and pass back
 * the dynamically allocated result.
 */
char *
expand_prompt(const struct sudoers_context *ctx,
        const char *restrict old_prompt, const char *restrict auth_user)
{
    size_t len, n;
    int subst;
    const char *p;
    char *np, *new_prompt;
    debug_decl(expand_prompt, SUDOERS_DEBUG_AUTH);

    /* How much space do we need to malloc for the prompt? */
    subst = 0;
    for (p = old_prompt, len = strlen(old_prompt); *p != '\0'; p++) {
	if (p[0] =='%') {
	    switch (p[1]) {
		case 'h':
		    p++;
		    len += strlen(ctx->user.shost) - 2;
		    subst = 1;
		    break;
		case 'H':
		    p++;
		    len += strlen(ctx->user.host) - 2;
		    subst = 1;
		    break;
		case 'p':
		    p++;
		    len += strlen(auth_user) - 2;
		    subst = 1;
		    break;
		case 'u':
		    p++;
		    len += strlen(ctx->user.name) - 2;
		    subst = 1;
		    break;
		case 'U':
		    p++;
		    len += strlen(ctx->runas.pw->pw_name) - 2;
		    subst = 1;
		    break;
		case '%':
		    p++;
		    len--;
		    subst = 1;
		    break;
		default:
		    break;
	    }
	}
    }

    if ((new_prompt = malloc(++len)) == NULL) {
	sudo_warnx(U_("%s: %s"), __func__, U_("unable to allocate memory"));
	debug_return_str(NULL);
    }

    if (subst) {
	for (p = old_prompt, np = new_prompt; *p != '\0'; p++) {
	    if (p[0] =='%') {
		switch (p[1]) {
		    case 'h':
			p++;
			n = strlcpy(np, ctx->user.shost, len);
			if (n >= len)
			    goto oflow;
			np += n;
			len -= n;
			continue;
		    case 'H':
			p++;
			n = strlcpy(np, ctx->user.host, len);
			if (n >= len)
			    goto oflow;
			np += n;
			len -= n;
			continue;
		    case 'p':
			p++;
			n = strlcpy(np, auth_user, len);
			if (n >= len)
			    goto oflow;
			np += n;
			len -= n;
			continue;
		    case 'u':
			p++;
			n = strlcpy(np, ctx->user.name, len);
			if (n >= len)
			    goto oflow;
			np += n;
			len -= n;
			continue;
		    case 'U':
			p++;
			n = strlcpy(np,  ctx->runas.pw->pw_name, len);
			if (n >= len)
			    goto oflow;
			np += n;
			len -= n;
			continue;
		    case '%':
			/* convert %% -> % */
			p++;
			break;
		    default:
			/* no conversion */
			break;
		}
	    }
	    if (len < 2)			/* len includes NUL */
		goto oflow;
	    *np++ = *p;
	    len--;
	}
	if (len != 1)
	    goto oflow;
	*np = '\0';
    } else {
	/* Nothing to expand. */
	memcpy(new_prompt, old_prompt, len);	/* len includes NUL */
    }

    debug_return_str(new_prompt);

oflow:
    /* We pre-allocate enough space, so this should never happen. */
    free(new_prompt);
    sudo_warnx(U_("internal error, %s overflow"), __func__);
    debug_return_str(NULL);
}
