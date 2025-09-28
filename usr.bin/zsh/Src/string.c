/*
 * string.c - string manipulation
 *
 * This file is part of zsh, the Z shell.
 *
 * Copyright (c) 2000 Peter Stephenson
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and to distribute modified versions of this software for any
 * purpose, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * In no event shall Peter Stephenson or the Zsh Development Group be liable
 * to any party for direct, indirect, special, incidental, or consequential
 * damages arising out of the use of this software and its documentation,
 * even if Peter Stephenson and the Zsh Development Group have been advised of
 * the possibility of such damage.
 *
 * Peter Stephenson and the Zsh Development Group specifically disclaim any
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose.  The software
 * provided hereunder is on an "as is" basis, and Peter Stephenson and the
 * Zsh Development Group have no obligation to provide maintenance,
 * support, updates, enhancements, or modifications.
 */

#include "zsh.mdh"

/**/
mod_export char *
dupstring(const char *s)
{
    char *t;

    if (!s)
	return NULL;
    t = (char *) zhalloc(strlen((char *)s) + 1);
    strcpy(t, s);
    return t;
}

/* Duplicate string on heap when length is known */

/**/
mod_export char *
dupstring_wlen(const char *s, unsigned len)
{
    char *t;

    if (!s)
	return NULL;
    t = (char *) zhalloc(len + 1);
    memcpy(t, s, len);
    t[len] = '\0';
    return t;
}

/* Duplicate string on heap, returning length of string */

/**/
mod_export char *
dupstring_glen(const char *s, unsigned *len_ret)
{
    char *t;

    if (!s)
	return NULL;
    t = (char *) zhalloc((*len_ret = strlen((char *)s)) + 1);
    strcpy(t, s);
    return t;
}

/**/
mod_export char *
ztrdup(const char *s)
{
    char *t;

    if (!s)
	return NULL;
    t = (char *)zalloc(strlen((char *)s) + 1);
    strcpy(t, s);
    return t;
}

/**/
#ifdef MULTIBYTE_SUPPORT
/**/
mod_export wchar_t *
wcs_ztrdup(const wchar_t *s)
{
    wchar_t *t;

    if (!s)
	return NULL;
    t = (wchar_t *)zalloc(sizeof(wchar_t) * (wcslen((wchar_t *)s) + 1));
    wcscpy(t, s);
    return t;
}
/**/
#endif /* MULTIBYTE_SUPPORT */


/* Concatenate s1, s2, and s3 into dynamically allocated buffer.
 *
 * To concatenate four or more strings, see zjoin().
 */

/**/
mod_export char *
tricat(char const *s1, char const *s2, char const *s3)
{
    /* This version always uses permanently-allocated space. */
    char *ptr;
    size_t l1 = strlen(s1);
    size_t l2 = strlen(s2);

    ptr = (char *)zalloc(l1 + l2 + strlen(s3) + 1);
    strcpy(ptr, s1);
    strcpy(ptr + l1, s2);
    strcpy(ptr + l1 + l2, s3);
    return ptr;
}

/**/
mod_export char *
zhtricat(char const *s1, char const *s2, char const *s3)
{
    char *ptr;
    size_t l1 = strlen(s1);
    size_t l2 = strlen(s2);

    ptr = (char *)zhalloc(l1 + l2 + strlen(s3) + 1);
    strcpy(ptr, s1);
    strcpy(ptr + l1, s2);
    strcpy(ptr + l1 + l2, s3);
    return ptr;
}

/* concatenate s1 and s2 in dynamically allocated buffer */

/**/
mod_export char *
dyncat(const char *s1, const char *s2)
{
    /* This version always uses space from the current heap. */
    char *ptr;
    size_t l1 = strlen(s1);

    ptr = (char *)zhalloc(l1 + strlen(s2) + 1);
    strcpy(ptr, s1);
    strcpy(ptr + l1, s2);
    return ptr;
}

/**/
mod_export char *
bicat(const char *s1, const char *s2)
{
    /* This version always uses permanently-allocated space. */
    char *ptr;
    size_t l1 = strlen(s1);

    ptr = (char *)zalloc(l1 + strlen(s2) + 1);
    strcpy(ptr, s1);
    strcpy(ptr + l1, s2);
    return ptr;
}

/* like dupstring(), but with a specified length */

/**/
mod_export char *
dupstrpfx(const char *s, int len)
{
    char *r = zhalloc(len + 1);

    memcpy(r, s, len);
    r[len] = '\0';
    return r;
}

/**/
mod_export char *
ztrduppfx(const char *s, int len)
{
    /* This version always uses permanently-allocated space. */
    char *r = zalloc(len + 1);

    memcpy(r, s, len);
    r[len] = '\0';
    return r;
}

/* Append a string to an allocated string, reallocating to make room. */

/**/
mod_export char *
appstr(char *base, char const *append)
{
    return strcat(realloc(base, strlen(base) + strlen(append) + 1), append);
}

/* Return a pointer to the last character of a string,
   unless the string is empty. */

/**/
mod_export char *
strend(char *str)
{
    if (*str == '\0')
	return str;
    return str + strlen (str) - 1;
}
