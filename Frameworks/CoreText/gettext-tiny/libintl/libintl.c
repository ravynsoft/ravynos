/* Copyright (C) 2003     Manuel Novoa III
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Trivial Stubs, Public Domain.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

char *gettext(const char *msgid)
{
	return (char *) msgid;
}

char *dgettext(const char *domainname, const char *msgid)
{
	(void) domainname;
	return (char *) msgid;
}

char *dcgettext(const char *domainname, const char *msgid, int category)
{
	(void) domainname;
	(void) category;
	return (char *) msgid;
}

char *ngettext(const char *msgid1, const char *msgid2, unsigned long n)
{
	return (char *) ((n == 1) ? msgid1 : msgid2);
}

char *dngettext(const char *domainname, const char *msgid1, const char *msgid2, unsigned long n)
{
	(void) domainname;
	return (char *) ((n == 1) ? msgid1 : msgid2);
}

char *dcngettext(const char *domainname, const char *msgid1, const char *msgid2, unsigned long n, int category)
{
	(void) domainname;
	(void) category;
	return (char *) ((n == 1) ? msgid1 : msgid2);
}

char *textdomain(const char *domainname)
{
	static const char default_str[] = "messages";

	if (domainname && *domainname && strcmp(domainname, default_str)) {
		errno = EINVAL;
		return NULL;
	}
	return (char *) default_str;
}

char *bindtextdomain(const char *domainname, const char *dirname)
{
	static const char dir[] = "/";

	if (!domainname || !*domainname
		|| (dirname && ((dirname[0] != '/') || dirname[1]))
		) {
		errno = EINVAL;
		return NULL;
	}

	return (char *) dir;
}

char *bind_textdomain_codeset(const char *domainname, const char *codeset)
{
	if (!domainname || !*domainname || (codeset && strcasecmp(codeset, "UTF-8"))) {
		errno = EINVAL;
		return NULL;
	}
	return codeset;
}

/* trick configure tests checking for gnu libintl, as in the copy included in gdb */
const char *_nl_expand_alias () { return NULL; }
int _nl_msg_cat_cntr = 0;
