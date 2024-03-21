/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2007-2015 Todd C. Miller <Todd.Miller@sudo.ws>
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

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif /* HAVE_STRINGS_H */
#include <ctype.h>

#include <sudoers.h>

extern struct sudo_nss sudo_nss_file;
#ifdef HAVE_LDAP
extern struct sudo_nss sudo_nss_ldap;
#endif
#ifdef HAVE_SSSD
extern struct sudo_nss sudo_nss_sss;
#endif

/* Make sure we have not already inserted the nss entry. */
#define SUDO_NSS_CHECK_UNUSED(nss, tag)					       \
    if (nss.entries.tqe_next != NULL || nss.entries.tqe_prev != NULL) {      \
	sudo_warnx("internal error: nsswitch entry \"%s\" already in use",     \
	    tag);							       \
	continue;							       \
    }

#if (defined(HAVE_LDAP) || defined(HAVE_SSSD)) && defined(_PATH_NSSWITCH_CONF)
/*
 * Read in /etc/nsswitch.conf
 * Returns a tail queue of matches.
 */
struct sudo_nss_list *
sudo_read_nss(void)
{
    FILE *fp;
    char *line = NULL;
    size_t linesize = 0;
#ifdef HAVE_SSSD
    bool saw_sss = false;
#endif
#ifdef HAVE_LDAP
    bool saw_ldap = false;
#endif
    bool saw_files = false;
    bool got_match = false;
    static struct sudo_nss_list snl = TAILQ_HEAD_INITIALIZER(snl);
    debug_decl(sudo_read_nss, SUDOERS_DEBUG_NSS);

    if ((fp = fopen(_PATH_NSSWITCH_CONF, "r")) == NULL)
	goto nomatch;

    while (sudo_parseln(&line, &linesize, NULL, fp, 0) != -1) {
	char *cp, *last;

	/* Skip blank or comment lines */
	if (*line == '\0')
	    continue;

	/* Look for a line starting with "sudoers:" */
	if (strncasecmp(line, "sudoers:", 8) != 0)
	    continue;

	/* Parse line */
	for ((cp = strtok_r(line + 8, " \t", &last)); cp != NULL; (cp = strtok_r(NULL, " \t", &last))) {
	    if (strcasecmp(cp, "files") == 0 && !saw_files) {
		SUDO_NSS_CHECK_UNUSED(sudo_nss_file, "files");
		TAILQ_INSERT_TAIL(&snl, &sudo_nss_file, entries);
		got_match = saw_files = true;
#ifdef HAVE_LDAP
	    } else if (strcasecmp(cp, "ldap") == 0 && !saw_ldap) {
		SUDO_NSS_CHECK_UNUSED(sudo_nss_ldap, "ldap");
		TAILQ_INSERT_TAIL(&snl, &sudo_nss_ldap, entries);
		got_match = saw_ldap = true;
#endif
#ifdef HAVE_SSSD
	    } else if (strcasecmp(cp, "sss") == 0 && !saw_sss) {
		SUDO_NSS_CHECK_UNUSED(sudo_nss_sss, "sss");
		TAILQ_INSERT_TAIL(&snl, &sudo_nss_sss, entries);
		got_match = saw_sss = true;
#endif
	    } else if (strcasecmp(cp, "[NOTFOUND=return]") == 0 && got_match) {
		/* NOTFOUND affects the most recent entry */
		TAILQ_LAST(&snl, sudo_nss_list)->ret_if_notfound = true;
		got_match = false;
	    } else if (strcasecmp(cp, "[SUCCESS=return]") == 0 && got_match) {
		/* SUCCESS affects the most recent entry */
		TAILQ_LAST(&snl, sudo_nss_list)->ret_if_found = true;
		got_match = false;
	    } else
		got_match = false;
	}
	/* Only parse the first "sudoers:" line */
	break;
    }
    free(line);
    fclose(fp);

nomatch:
    /* Default to files only if no matches */
    if (TAILQ_EMPTY(&snl))
	TAILQ_INSERT_TAIL(&snl, &sudo_nss_file, entries);

    debug_return_ptr(&snl);
}

#else /* (HAVE_LDAP || HAVE_SSSD) && _PATH_NSSWITCH_CONF */

# if (defined(HAVE_LDAP) || defined(HAVE_SSSD)) && defined(_PATH_NETSVC_CONF)

/*
 * Read in /etc/netsvc.conf (like nsswitch.conf on AIX)
 * Returns a tail queue of matches.
 */
struct sudo_nss_list *
sudo_read_nss(void)
{
    FILE *fp;
    char *cp, *ep, *last, *line = NULL;
    size_t linesize = 0;
#ifdef HAVE_SSSD
    bool saw_sss = false;
#endif
    bool saw_files = false;
    bool saw_ldap = false;
    bool got_match = false;
    static struct sudo_nss_list snl = TAILQ_HEAD_INITIALIZER(snl);
    debug_decl(sudo_read_nss, SUDOERS_DEBUG_NSS);

    if ((fp = fopen(_PATH_NETSVC_CONF, "r")) == NULL)
	goto nomatch;

    while (sudo_parseln(&line, &linesize, NULL, fp, 0) != -1) {
	/* Skip blank or comment lines */
	if (*(cp = line) == '\0')
	    continue;

	/* Look for a line starting with "sudoers = " */
	if (strncasecmp(cp, "sudoers", 7) != 0)
	    continue;
	cp += 7;
	while (isspace((unsigned char)*cp))
	    cp++;
	if (*cp++ != '=')
	    continue;

	/* Parse line */
	for ((cp = strtok_r(cp, ",", &last)); cp != NULL; (cp = strtok_r(NULL, ",", &last))) {
	    /* Trim leading whitespace. */
	    while (isspace((unsigned char)*cp))
		cp++;

	    if (!saw_files && strncasecmp(cp, "files", 5) == 0 &&
		(isspace((unsigned char)cp[5]) || cp[5] == '\0')) {
		TAILQ_INSERT_TAIL(&snl, &sudo_nss_file, entries);
		got_match = saw_files = true;
		ep = &cp[5];
#ifdef HAVE_LDAP
	    } else if (!saw_ldap && strncasecmp(cp, "ldap", 4) == 0 &&
		(isspace((unsigned char)cp[4]) || cp[4] == '\0')) {
		TAILQ_INSERT_TAIL(&snl, &sudo_nss_ldap, entries);
		got_match = saw_ldap = true;
		ep = &cp[4];
#endif
#ifdef HAVE_SSSD
	    } else if (!saw_sss && strncasecmp(cp, "sss", 3) == 0 &&
		(isspace((unsigned char)cp[3]) || cp[3] == '\0')) {
		TAILQ_INSERT_TAIL(&snl, &sudo_nss_sss, entries);
		got_match = saw_sss = true;
		ep = &cp[3];
#endif
	    } else {
		got_match = false;
	    }

	    /* check for = auth qualifier */
	    if (got_match && *ep) {
		cp = ep;
		while (isspace((unsigned char)*cp) || *cp == '=')
		    cp++;
		if (strncasecmp(cp, "auth", 4) == 0 &&
		    (isspace((unsigned char)cp[4]) || cp[4] == '\0')) {
		    TAILQ_LAST(&snl, sudo_nss_list)->ret_if_found = true;
		}
	    }
	}
	/* Only parse the first "sudoers" line */
	break;
    }
    fclose(fp);

nomatch:
    /* Default to files only if no matches */
    if (TAILQ_EMPTY(&snl))
	TAILQ_INSERT_TAIL(&snl, &sudo_nss_file, entries);

    debug_return_ptr(&snl);
}

# else /* !_PATH_NETSVC_CONF && !_PATH_NSSWITCH_CONF */

/*
 * Non-nsswitch.conf version with hard-coded order.
 */
struct sudo_nss_list *
sudo_read_nss(void)
{
    static struct sudo_nss_list snl = TAILQ_HEAD_INITIALIZER(snl);
    debug_decl(sudo_read_nss, SUDOERS_DEBUG_NSS);

#  ifdef HAVE_SSSD
    TAILQ_INSERT_TAIL(&snl, &sudo_nss_sss, entries);
#  endif
#  ifdef HAVE_LDAP
    TAILQ_INSERT_TAIL(&snl, &sudo_nss_ldap, entries);
#  endif
    TAILQ_INSERT_TAIL(&snl, &sudo_nss_file, entries);

    debug_return_ptr(&snl);
}

# endif /* !HAVE_LDAP || !_PATH_NETSVC_CONF */

#endif /* HAVE_LDAP && _PATH_NSSWITCH_CONF */

bool
sudo_nss_can_continue(const struct sudo_nss *nss, int match)
{
    debug_decl(sudo_nss_should_continue, SUDOERS_DEBUG_NSS);

    /* Handle [NOTFOUND=return] */
    if (nss->ret_if_notfound && match == UNSPEC)
	debug_return_bool(false);

    /* Handle [SUCCESS=return] */
    if (nss->ret_if_found && match != UNSPEC)
	debug_return_bool(false);

    debug_return_bool(true);
}
