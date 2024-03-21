/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2013-2015, 2019-2020 Todd C. Miller <Todd.Miller@sudo.ws>
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

#include <ctype.h>
#include <errno.h>

#include <sudo_compat.h>
#include <sudo_gettext.h>
#include <sudo_util.h>

enum strtonum_err {
    STN_INITIAL,
    STN_VALID,
    STN_INVALID,
    STN_TOOSMALL,
    STN_TOOBIG
};

/*
 * Convert a string to a number in the range [minval, maxval]
 * Unlike strtonum(), this returns the first non-digit in endp (if not NULL).
 */
long long
sudo_strtonumx(const char *str, long long minval, long long maxval, char **endp,
    const char **errstrp)
{
    enum strtonum_err errval = STN_INITIAL;
    long long lastval, result = 0;
    const char *cp = str;
    int remainder;
    char ch, sign;

    if (minval > maxval) {
	errval = STN_INVALID;
	goto done;
    }

    /* Trim leading space and check sign, if any. */
    do {
	ch = *cp++;
    } while (isspace((unsigned char)ch));
    switch (ch) {
    case '-':
	sign = '-';
	ch = *cp++;
	break;
    case '+':
	ch = *cp++;
	FALLTHROUGH;
    default:
	sign = '+';
	break;
    }

    /*
     * To prevent overflow we determine the highest (or lowest in
     * the case of negative numbers) value result can have *before*
     * if its multiplied (divided) by 10 as well as the remainder.
     * If result matches this value and the next digit is larger than
     * the remainder, we know the result is out of range.
     * The remainder is always positive since it is compared against
     * an unsigned digit.
     */
    if (sign == '-') {
	lastval = minval / 10;
	remainder = -(int)(minval % 10);
	if (remainder < 0) {
	    lastval += 1;
	    remainder += 10;
	}
	for (;; ch = *cp++) {
	    if (!isdigit((unsigned char)ch))
		break;
	    ch -= '0';
	    if (result < lastval || (result == lastval && ch > remainder)) {
		/* Skip remaining digits. */
		do {
		    ch = *cp++;
		} while (isdigit((unsigned char)ch));
		errval = STN_TOOSMALL;
		break;
	    } else {
		result *= 10;
		result -= ch;
		errval = STN_VALID;
	    }
	}
	if (result > maxval)
	    errval = STN_TOOBIG;
    } else {
	lastval = maxval / 10;
	remainder = (int)(maxval % 10);
	for (;; ch = *cp++) {
	    if (!isdigit((unsigned char)ch))
		break;
	    ch -= '0';
	    if (result > lastval || (result == lastval && ch > remainder)) {
		/* Skip remaining digits. */
		do {
		    ch = *cp++;
		} while (isdigit((unsigned char)ch));
		errval = STN_TOOBIG;
		break;
	    } else {
		result *= 10;
		result += ch;
		errval = STN_VALID;
	    }
	}
	if (result < minval)
	    errval = STN_TOOSMALL;
    }

done:
    switch (errval) {
    case STN_INITIAL:
    case STN_VALID:
	if (errstrp != NULL)
	    *errstrp = NULL;
	break;
    case STN_INVALID:
	result = 0;
	errno = EINVAL;
	if (errstrp != NULL)
	    *errstrp = N_("invalid value");
	break;
    case STN_TOOSMALL:
	result = 0;
	errno = ERANGE;
	if (errstrp != NULL)
	    *errstrp = N_("value too small");
	break;
    case STN_TOOBIG:
	result = 0;
	errno = ERANGE;
	if (errstrp != NULL)
	    *errstrp = N_("value too large");
	break;
    }
    if (endp != NULL) {
	if (errval == STN_INITIAL || errval == STN_INVALID)
	    *endp = (char *)str;
	else
	    *endp = (char *)(cp - 1);
    }
    return result;
}

/*
 * Convert a string to a number in the range [minval, maxval]
 */
long long
sudo_strtonum(const char *str, long long minval, long long maxval,
    const char **errstrp)
{
    const char *errstr;
    char *ep;
    long long ret;

    ret = sudo_strtonumx(str, minval, maxval, &ep, &errstr);
    /* Check for empty string and terminating NUL. */
    if (str == ep || *ep != '\0') {
	errno = EINVAL;
	errstr = N_("invalid value");
	ret = 0;
    }
    if (errstrp != NULL)
	*errstrp = errstr;
    return ret;
}
