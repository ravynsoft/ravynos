/*
 * Copyright (c) 2007-2013 Apple Inc.  All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <asl.h>
#include <asl_string.h>
#include <asl_core.h>
#include <asl_private.h>
#include <string.h>
#include <membership.h>
#include <pthread.h>
#include <libkern/OSAtomic.h>
#include <servers/bootstrap.h>
#include <bootstrap_priv.h>
#include <mach/kern_return.h>
#include <mach/mach_init.h>
#include <mach/mach_vm.h>
#include <mach/vm_map.h>
#include <mach/vm_param.h>
#include <dispatch/dispatch.h>

const char *ASL_LEVEL_TO_STRING[] =
{
	ASL_STRING_EMERG,
	ASL_STRING_ALERT,
	ASL_STRING_CRIT,
	ASL_STRING_ERR,
	ASL_STRING_WARNING,
	ASL_STRING_NOTICE,
	ASL_STRING_INFO,
	ASL_STRING_DEBUG
};

/*
 * Message ID generation
 */
static uint64_t _asl_core_msg_next_id = 1;
static pthread_mutex_t core_lock = PTHREAD_MUTEX_INITIALIZER;

#define mix(a, b, c) \
{ \
	a -= b; a -= c; a ^= (c>>13); \
	b -= c; b -= a; b ^= (a<< 8); \
	c -= a; c -= b; c ^= (b>>13); \
	a -= b; a -= c; a ^= (c>>12); \
	b -= c; b -= a; b ^= (a<<16); \
	c -= a; c -= b; c ^= (b>> 5); \
	a -= b; a -= c; a ^= (c>> 3); \
	b -= c; b -= a; b ^= (a<<10); \
	c -= a; c -= b; c ^= (b>>15); \
}

/*
 * Hash is used to improve string search.
 */
uint32_t
asl_core_string_hash(const char *s, uint32_t inlen)
{
	uint32_t a, b, c, l, len;

	if (s == NULL) return 0;

	l = inlen;
	if (l == 0)
	{
		if (s[0] == '\0') return 0;
		l = strlen(s);
	}

	len = l;
	a = b = 0x9e3779b9;
	c = 0;

	while (len >= 12)
	{
		a += (s[0] + ((uint32_t)s[1]<<8) + ((uint32_t)s[ 2]<<16) + ((uint32_t)s[ 3]<<24));
		b += (s[4] + ((uint32_t)s[5]<<8) + ((uint32_t)s[ 6]<<16) + ((uint32_t)s[ 7]<<24));
		c += (s[8] + ((uint32_t)s[9]<<8) + ((uint32_t)s[10]<<16) + ((uint32_t)s[11]<<24));

		mix(a, b, c);

		s += 12;
		len -= 12;
	}

	c += l;
	switch(len)
	{
		case 11: c += ((uint32_t)s[10]<<24);
		case 10: c += ((uint32_t)s[9]<<16);
		case 9 : c += ((uint32_t)s[8]<<8);

		case 8 : b += ((uint32_t)s[7]<<24);
		case 7 : b += ((uint32_t)s[6]<<16);
		case 6 : b += ((uint32_t)s[5]<<8);
		case 5 : b += s[4];

		case 4 : a += ((uint32_t)s[3]<<24);
		case 3 : a += ((uint32_t)s[2]<<16);
		case 2 : a += ((uint32_t)s[1]<<8);
		case 1 : a += s[0];
	}

	mix(a, b, c);

	if (c == 0) c = 1;
	return c;
}

const char *
asl_core_error(uint32_t code)
{
	switch (code)
	{
		case ASL_STATUS_OK: return "Operation Succeeded";
		case ASL_STATUS_INVALID_ARG: return "Invalid Argument";
		case ASL_STATUS_INVALID_STORE: return "Invalid Data Store";
		case ASL_STATUS_INVALID_STRING: return "Invalid String";
		case ASL_STATUS_INVALID_ID: return "Invalid ID Number";
		case ASL_STATUS_INVALID_MESSAGE: return "Invalid Message";
		case ASL_STATUS_NOT_FOUND: return "Not Found";
		case ASL_STATUS_READ_FAILED: return "Read Operation Failed";
		case ASL_STATUS_WRITE_FAILED: return "Write Operation Failed";
		case ASL_STATUS_NO_MEMORY: return "System Memory Allocation Failed";
		case ASL_STATUS_ACCESS_DENIED: return "Access Denied";
		case ASL_STATUS_READ_ONLY: return "Read Only Access";
		case ASL_STATUS_WRITE_ONLY: return "Write Only Access";
		case ASL_STATUS_MATCH_FAILED: return "Match Failed";
		case ASL_STATUS_NO_RECORDS: return "No More Records";
	}

	return "Operation Failed";
}

const char *
asl_core_level_to_string(uint32_t level)
{
	if (level > ASL_LEVEL_DEBUG) return "invalid";
	return ASL_LEVEL_TO_STRING[level];
}

static ASL_STATUS
asl_core_check_user_access(int32_t msgu, int32_t readu)
{
	/* -1 means anyone may read */
	if (msgu == -1) return ASL_STATUS_OK;

	/* Check for exact match */
	if (msgu == readu) return ASL_STATUS_OK;

	return ASL_STATUS_ACCESS_DENIED;
}

static ASL_STATUS
asl_core_check_group_access(int32_t msgg, int32_t readu, int32_t readg)
{
	int check;
	uuid_t uu, gu;

	/* -1 means anyone may read */
	if (msgg == -1) return ASL_STATUS_OK;

	/* Check for exact match */
	if (msgg == readg) return ASL_STATUS_OK;

	/* Check if user (u) is in read group (msgg) */
	mbr_uid_to_uuid(readu, uu);
	mbr_gid_to_uuid(msgg, gu);

	check = 0;
	mbr_check_membership(uu, gu, &check);
	if (check != 0) return ASL_STATUS_OK;

	return ASL_STATUS_ACCESS_DENIED;
}

ASL_STATUS
asl_core_check_access(int32_t msgu, int32_t msgg, int32_t readu, int32_t readg, uint16_t flags)
{
	uint16_t uset, gset;

	/* root (uid 0) may always read */
	if (readu == 0) return ASL_STATUS_OK;

	uset = flags & ASL_MSG_FLAG_READ_UID_SET;
	gset = flags & ASL_MSG_FLAG_READ_GID_SET;

	/* if no access controls are set, anyone may read */
	if ((uset | gset) == 0) return ASL_STATUS_OK;

	/* if only uid is set, then access is only by uid match */
	if ((uset != 0) && (gset == 0)) return asl_core_check_user_access(msgu, readu);

	/* if only gid is set, then access is only by gid match */
	if ((uset == 0) && (gset != 0)) return asl_core_check_group_access(msgg, readu, readg);

	/* both uid and gid are set - check user, then group */
	if ((asl_core_check_user_access(msgu, readu)) == ASL_STATUS_OK) return ASL_STATUS_OK;
	return asl_core_check_group_access(msgg, readu, readg);
}

uint64_t
asl_core_htonq(uint64_t n)
{
#ifdef __BIG_ENDIAN__
	return n;
#else
	u_int32_t t;
	union
	{
		u_int64_t q;
		u_int32_t l[2];
	} x;

	x.q = n;
	t = x.l[0];
	x.l[0] = htonl(x.l[1]);
	x.l[1] = htonl(t);

	return x.q;
#endif
}

uint64_t
asl_core_ntohq(uint64_t n)
{
#ifdef __BIG_ENDIAN__
	return n;
#else
	u_int32_t t;
	union
	{
		u_int64_t q;
		u_int32_t l[2];
	} x;

	x.q = n;
	t = x.l[0];
	x.l[0] = ntohl(x.l[1]);
	x.l[1] = ntohl(t);

	return x.q;
#endif
}

uint64_t
asl_core_new_msg_id(uint64_t start)
{
	uint64_t out;

	pthread_mutex_lock(&core_lock);

	if (start != 0) _asl_core_msg_next_id = start;

	out = _asl_core_msg_next_id;
	_asl_core_msg_next_id++;

	pthread_mutex_unlock(&core_lock);

	return out;
}

const char *
asl_filesystem_path(uint32_t place)
{
	static dispatch_once_t once;
	static char *asl_filesystem_path_database = NULL;
	static char *asl_filesystem_path_archive = NULL;

	dispatch_once(&once, ^{
		char *asl_var_log = NULL;
		const char *const_asl_var_log = "/var/log";

#if TARGET_OS_SIMULATOR
		asl_var_log = getenv("SIMULATOR_LOG_ROOT");
#endif

		if (asl_var_log != NULL) const_asl_var_log = (const char *)asl_var_log;

		asprintf(&asl_filesystem_path_database, "%s/asl", const_asl_var_log);
		asprintf(&asl_filesystem_path_archive, "%s/asl.archive", const_asl_var_log);
	});

	switch (place)
	{
		case ASL_PLACE_DATABASE:
		{
			return asl_filesystem_path_database;
		}
		case ASL_PLACE_ARCHIVE:
		{
			return asl_filesystem_path_archive;
		}
		default:
		{
			return NULL;
		}
	}

	return NULL;
}

#pragma mark -
#pragma mark data buffer encoding

/*
 * asl_core_encode_buffer
 * encode arbitrary data as a C string without embedded zero (nul) characters
 *
 * The routine computes a histogram of the input buffer and finds
 * the two least frequently used non-nul chars (L[0] and L[1]).
 *
 * L[0] is used to stand in for nul.
 * L[1] is used as the escape character.
 * Occurrences of nul in the data are encoded as L[0]
 * Occurrences of L[0] in the data are encoded as the sequence L[1] 1.
 * Occurrences of L[1] in the data are encoded as the sequence L[1] 2.
 *
 * The output string is preceded by L[0] L[1], and is nul terminated.
 * The output length is 2 + n + N(L[0]) + N(L[1]) + 1
 * where N(x) is the number of occurrences of x in the input string.
 * The worst case occurs when all characters are equally frequent,
 * In that case the output size will less that 1% larger than the input.
 */
char *
asl_core_encode_buffer(const char *in, uint32_t len)
{
	char *str;
	uint32_t i, j, k, outlen, breakit, min, hist[256];
	uint32_t lfu[2], save[2];
	uint8_t v;

	if (in == NULL) return NULL;
	if (len == 0) return NULL;

	memset(hist, 0, sizeof(hist));
	save[0] = 0;
	save[1] = 0;

	for (i = 0; i < len; i++)
	{
		v = in[i];
		hist[v]++;
	}

	for (j = 0; j < 2; j++)
	{
		lfu[j] = 1;
		min = hist[1];

		for (i = 2; i < 256; i++)
		{
			if (hist[i] < min)
			{
				lfu[j] = i;
				min = hist[i];

				/*
				 * Stop if there are no occurances or character i in the input.
				 * The minimum will never be less than zero.
				 */
				if (min == 0) break;

				/*
				 * When looking for the second least frequently used character,
				 * stop scanning if we hit the same minimum as we saw in the first
				 * pass.  There will be no smaller values.
				 */
				if ((j == 1) && (min == save[0])) break;
			}
		}

		save[j] = hist[lfu[j]];
		hist[lfu[j]] = (uint32_t)-1;
	}

	outlen = 2 + len + save[0] + save[1] + 1;

	str = malloc(outlen);
	if (str == NULL) return NULL;

	str[outlen - 1] = '\0';

	str[0] = lfu[0];
	str[1] = lfu[1];

	j = 2;

	for (i = 0; i < len; i++)
	{
		v = in[i];
		if (v == 0)
		{
			str[j++] = lfu[0];
			continue;
		}

		breakit = 0;
		for (k = 0; (k < 2) && (breakit == 0); k++)
		{
			if (v == lfu[k])
			{
				str[j++] = lfu[1];
				str[j++] = k + 1;
				breakit = 1;
			}
		}

		if (breakit == 1) continue;

		str[j++] = v;
	}

	return str;
}

/*
 * asl_core_decode_buffer
 * decode a string produced by asl_encode_buffer to recreate the original data
 */
int32_t
asl_core_decode_buffer(const char *in, char **buf, uint32_t *len)
{
	uint8_t v;
	uint32_t i, j, n, outlen;
	uint8_t lfu[2];
	char *out;

	if (buf == NULL) return -1;
	if (len == NULL) return -1;

	lfu[0] = in[0];
	lfu[1] = in[1];

	outlen = 0;

	/* strip trailing nul */
	n = strlen(in);

	/* determine output length and check for invalid input */
	for (i = 2; i < n; i++)
	{
		v = in[i];
		if (v == lfu[1])
		{
			i++;
			if (i == n) return -1;

			v = in[i];
			if ((v < 1) || (v > 2)) return -1;

			outlen++;
		}
		else outlen++;
	}

	if (outlen == 0) return -1;

	out = malloc(outlen);
	if (out == NULL) return -1;

	j = 0;
	for (i = 2; i < n; i++)
	{
		v = in[i];
		if (v == lfu[0])
		{
			out[j++] = 0;
		}
		else if (v == lfu[1])
		{
			i++;
			v = in[i];
			out[j++] = lfu[v - 1];
		}
		else out[j++] = v;
	}

	*len = outlen;
	*buf = out;
	return 0;
}

#pragma mark -
#pragma mark time parsing

/*
 * utility for converting a time string into a time_t
 * we only deal with the following formats:
 * dotted form YYYY.MM.DD hh:mm:ss UTC
 * ctime() form Mth dd hh:mm:ss (e.g. Aug 25 09:54:37)
 * absolute form - # seconds since the epoch (e.g. 1095789191)
 * relative time - seconds before or after now (e.g. -300, +43200)
 * relative time - days/hours/minutes/seconds before or after now (e.g. -1d, +6h, +30m, -10s)
 * ISO8601 - YYYY[-]MM[-]DDTHH[:]MM[:]SS optionally followed by Z or +/-HH[[:]MM]
 */

/*
 * light(er)-weight replcaement (in place of regex) for asl_core_parse_time
 */

#define MFLAG_INCLUDE 0x00000001
#define MFLAG_EXCLUDE 0x00000002

#define DIGITS "0123456789"
#define WHITESPACE "	 "

#define SECONDS_PER_MINUTE 60
#define SECONDS_PER_HOUR 3600
#define SECONDS_PER_DAY 86400
#define SECONDS_PER_WEEK 604800

/*
 * Match between mincount and maxcount characters in or not in mset.
 * maxcount == 0 means no limit.
 *
 */
bool
asl_core_str_match(const char *target, const char *mset, uint32_t mincount, uint32_t maxcount, uint32_t flags, uint32_t *length)
{
	const char *x;
	uint32_t n;

	if (length == NULL) length = &n;

	if (target == NULL) return (mincount == 0);

	for (x = target, *length = 0; *x != '\0'; x++, *length = *length + 1)
	{
		char *s;

		if ((*length == maxcount) && (maxcount > 0)) return true;
		if (mset == NULL) continue;

		s = strchr(mset, *x);
		if ((s == NULL) && (flags & MFLAG_EXCLUDE)) continue;
		if ((s != NULL) && (flags & MFLAG_INCLUDE)) continue;

		break;
	}

	return (*length >= mincount);
}

bool
asl_core_str_match_char(const char *target, const char c, uint32_t mincount, uint32_t flags, uint32_t *length)
{
	uint32_t n;

	if (length == NULL) length = &n;
	*length = 0;

	if (target == NULL) return (mincount == 0);

	if ((*target == c) && (flags & MFLAG_INCLUDE)) *length = 1;
	if ((*target != c) && (flags & MFLAG_EXCLUDE)) *length = 1;

	return (*length >= mincount);
}

uint32_t
asl_core_str_to_uint32(const char *target, uint32_t length)
{
	uint32_t i, d, out = 0;

	for (i = 0; i < length; i++)
	{
		d = target[i] - '0';
		out = (out * 10) + d;
	}

	return out;
}

size_t
asl_core_str_to_size(char *s)
{
	size_t len, n, max;
	char x;

	if (s == NULL) return 0;

	len = strlen(s);
	if (len == 0) return 0;

	n = 1;
	x = s[len - 1];
	if (x > 90) x -= 32;
	if (x == 'K') n = 1ll << 10;
	else if (x == 'M') n = 1ll << 20;
	else if (x == 'G') n = 1ll << 30;

	max = atoll(s) * n;
	return max;
}

time_t
asl_core_str_to_time(char *s, uint32_t def_n)
{
	size_t len;
	time_t n, out;
	char x;

	if (s == NULL) return 0;

	len = strlen(s);
	if (len == 0) return 0;

	n = def_n;

	x = s[len - 1];
	if (x > 90) x -= 32;

	if (x == 'S') n = 1;
	else if (x == 'M') n = SECONDS_PER_MINUTE;
	else if (x == 'H') n = SECONDS_PER_HOUR;
	else if (x == 'D') n = SECONDS_PER_DAY;

	out = atoll(s) * n;
	return out;
}

void
asl_core_time_to_str(time_t s, char *str, size_t len)
{
	char days[32], hms[32];
	uint32_t d, h, m;

	d = s / SECONDS_PER_DAY;
	s %= SECONDS_PER_DAY;

	h = s / SECONDS_PER_HOUR;
	s %= SECONDS_PER_HOUR;

	m = s / SECONDS_PER_MINUTE;
	s %= SECONDS_PER_MINUTE;

	memset(days, 0, sizeof(days));
	if (d > 0) snprintf(days, sizeof(days), "%u day%s", d, (d == 1) ? "" : "s");

	memset(hms, 0, sizeof(hms));
	snprintf(hms, sizeof(hms), "%02u:%02u:%02lld", h, m, (long long) s);

	if ((h + m + s) == 0)
	{
		if (d == 0) snprintf(str, len, "0");
		else snprintf(str, len, "%s", days);
		return;
	}

	if (d == 0) snprintf(str, len, "%s", hms);
	else snprintf(str, len, "%s %s", days, hms);
}

static bool
asl_core_str_match_absolute_or_relative_time(const char *target, time_t *tval, uint32_t *tlen)
{
	uint32_t len;
	int32_t val, sign = 1;
	bool test;
	const char *p;
	time_t start = 0;

	if (target == NULL) return false;

	/* [+-] */
	p = target;
	test = asl_core_str_match(p, "+-", 0, 1, MFLAG_INCLUDE, &len);
	if (!test) return false;

	if (len == 1)
	{
		/* relative time */
		start = time(NULL);
		if (*p == '-') sign = -1;
	}

	/* [0-9]+ */
	p += len;
	test = asl_core_str_match(p, DIGITS, 1, 0, MFLAG_INCLUDE, &len);
	if (!test) return false;
	val = asl_core_str_to_uint32(p, len);

	/* [shmdw] */
	p += len;
	test = asl_core_str_match(p, "SsMmHhDdWw", 0, 1, MFLAG_INCLUDE, &len);
	if (!test) return false;

	if ((*p == 'M') || (*p == 'm')) val *= SECONDS_PER_MINUTE;
	else if ((*p == 'H') || (*p == 'h')) val *= SECONDS_PER_HOUR;
	else if ((*p == 'D') || (*p == 'd')) val *= SECONDS_PER_DAY;
	else if ((*p == 'W') || (*p == 'w')) val *= SECONDS_PER_WEEK;

	/* matched string must be followed by space, tab, newline (not counted in length) */
	p += len;
	if (*p != '\0')
	{
		test = asl_core_str_match(p, " \t\n", 1, 1, MFLAG_INCLUDE, &len);
		if (!test) return false;
	}

	if (tlen != NULL) *tlen = p - target;
	if (tval != NULL) *tval = start + (sign * val);

	return true;
}

static int
_month_num(const char *s)
{
	if (!strncasecmp(s, "jan", 3)) return  0;
	if (!strncasecmp(s, "feb", 3)) return  1;
	if (!strncasecmp(s, "mar", 3)) return  2;
	if (!strncasecmp(s, "apr", 3)) return  3;
	if (!strncasecmp(s, "may", 3)) return  4;
	if (!strncasecmp(s, "jun", 3)) return  5;
	if (!strncasecmp(s, "jul", 3)) return  6;
	if (!strncasecmp(s, "aug", 3)) return  7;
	if (!strncasecmp(s, "sep", 3)) return  8;
	if (!strncasecmp(s, "oct", 3)) return  9;
	if (!strncasecmp(s, "nov", 3)) return 10;
	if (!strncasecmp(s, "dec", 3)) return 11;
	return -1;

}

/*
 * Match ctime() format - Mth [D]D [h]h:mm:ss
 */
bool
asl_core_str_match_c_time(const char *target, time_t *tval, uint32_t *tlen)
{
	uint32_t len, y;
	bool test;
	const char *p;
	struct tm t;
	time_t now;

	if (target == NULL) return false;
	memset(&t, 0, sizeof(t));

	/* determine current date */
	now = time(NULL);
	localtime_r(&now, &t);
	y = t.tm_year;
	memset(&t, 0, sizeof(t));
	t.tm_year = y;

	/* Mth */
	p = target;
	t.tm_mon = _month_num(p);
	len = 3;
	if (t.tm_mon == -1) return false;

	/* whitespace */
	p += len;
	test = asl_core_str_match(p, WHITESPACE, 1, 0, MFLAG_INCLUDE, &len);
	if (!test) return false;

	/* [D]D */
	p += len;
	test = asl_core_str_match(p, DIGITS, 1, 2, MFLAG_INCLUDE, &len);
	if (!test) return false;
	t.tm_mday = asl_core_str_to_uint32(p, len);
	if (t.tm_mday > 31) return false;

	/* whitespace */
	p += len;
	test = asl_core_str_match(p, WHITESPACE, 1, 0, MFLAG_INCLUDE, &len);
	if (!test) return false;

	/* [h]h */
	p += len;
	test = asl_core_str_match(p, DIGITS, 1, 2, MFLAG_INCLUDE, &len);
	if (!test) return false;
	t.tm_hour = asl_core_str_to_uint32(p, len);
	if (t.tm_hour > 23) return false;

	/* : */
	p += len;
	if (*p != ':') return false;
	len = 1;

	/* mm */
	p += len;
	test = asl_core_str_match(p, DIGITS, 2, 2, MFLAG_INCLUDE, &len);
	if (!test) return false;
	t.tm_min = asl_core_str_to_uint32(p, len);
	if (t.tm_min > 59) return false;

	/* : */
	p += len;
	if (*p != ':') return false;
	len = 1;

	/* ss */
	p += len;
	test = asl_core_str_match(p, DIGITS, 2, 2, MFLAG_INCLUDE, &len);
	if (!test) return false;
	t.tm_sec = asl_core_str_to_uint32(p, len);
	if (t.tm_sec > 59) return false;

	/* matched string must be followed by space, tab, newline (not counted in length) */
	p += len;
	if (*p != '\0')
	{
		test = asl_core_str_match(p, " \t\n", 1, 1, MFLAG_INCLUDE, &len);
		if (!test) return false;
	}

	t.tm_isdst = -1;

	if (tlen != NULL) *tlen = p - target;
	if (tval != NULL) *tval = mktime(&t);

	return true;
}

/*
 * Match YYYY.[M]M.[D]D [h]h:mm:ss UTC
 */
static bool
asl_core_str_match_dotted_time(const char *target, time_t *tval, uint32_t *tlen)
{
	uint32_t len;
	bool test;
	const char *p;
	struct tm t;

	if (target == NULL) return false;
	memset(&t, 0, sizeof(t));

	/* YYYY */
	p = target;
	test = asl_core_str_match(p, DIGITS, 4, 4, MFLAG_INCLUDE, &len);
	if (!test) return false;
	t.tm_year = asl_core_str_to_uint32(p, len) - 1900;

	/* . */
	p += len;
	if (*p != '.') return false;
	len = 1;

	/* [M]M */
	p += len;
	test = asl_core_str_match(p, DIGITS, 1, 2, MFLAG_INCLUDE, &len);
	if (!test) return false;
	t.tm_mon = asl_core_str_to_uint32(p, len);
	if (t.tm_mon < 1) return false;
	if (t.tm_mon > 12) return false;
	t.tm_mon -= 1;

	/* . */
	p += len;
	if (*p != '.') return false;
	len = 1;

	/* [D]D */
	p += len;
	test = asl_core_str_match(p, DIGITS, 1, 2, MFLAG_INCLUDE, &len);
	if (!test) return false;
	t.tm_mday = asl_core_str_to_uint32(p, len);
	if (t.tm_mday > 31) return false;

	/* whitespace */
	p += len;
	test = asl_core_str_match(p, WHITESPACE, 1, 0, MFLAG_INCLUDE, &len);
	if (!test) return false;

	/* [h]h */
	p += len;
	test = asl_core_str_match(p, DIGITS, 1, 2, MFLAG_INCLUDE, &len);
	if (!test) return false;
	t.tm_hour = asl_core_str_to_uint32(p, len);
	if (t.tm_hour > 23) return false;

	/* : */
	p += len;
	if (*p != ':') return false;
	len = 1;

	/* mm */
	p += len;
	test = asl_core_str_match(p, DIGITS, 2, 2, MFLAG_INCLUDE, &len);
	if (!test) return false;
	t.tm_min = asl_core_str_to_uint32(p, len);
	if (t.tm_min > 59) return false;

	/* : */
	p += len;
	if (*p != ':') return false;
	len = 1;

	/* ss */
	p += len;
	test = asl_core_str_match(p, DIGITS, 2, 2, MFLAG_INCLUDE, &len);
	if (!test) return false;
	t.tm_sec = asl_core_str_to_uint32(p, len);
	if (t.tm_sec > 59) return false;

	/* whitespace */
	p += len;
	test = asl_core_str_match(p, WHITESPACE, 1, 0, MFLAG_INCLUDE, &len);
	if (!test) return false;

	/* UTC */
	p += len;
	if (strncmp(p, "UTC", 3)) return false;
	len = 3;

	/* matched string must be followed by space, tab, newline (not counted in length) */
	p += len;
	if (*p != '\0')
	{
		test = asl_core_str_match(p, " \t\n", 1, 1, MFLAG_INCLUDE, &len);
		if (!test) return false;
	}

	if (tlen != NULL) *tlen = p - target;
	if (tval != NULL) *tval = timegm(&t);

	return true;
}

/*
 * Match YYYY[-]MM[-]DD[Tt]hh[:]hh[:]ss[Zz] or YYYY[-]MM[-]DD[Tt]hh[:]hh[:]ss[+-][H]H[[:]MM]
 */
static bool
asl_core_str_match_iso_8601_time(const char *target, time_t *tval, uint32_t *tlen)
{
	uint32_t len;
	bool test;
	const char *p;
	struct tm t;
	int32_t tzh, tzs, sign = -1;

	if (target == NULL) return false;
	memset(&t, 0, sizeof(t));

	/* YYYY */
	p = target;
	test = asl_core_str_match(p, DIGITS, 4, 4, MFLAG_INCLUDE, &len);
	if (!test) return false;
	t.tm_year = asl_core_str_to_uint32(p, len) - 1900;

	/* [-] */
	p += len;
	test = asl_core_str_match_char(p, '-', 0, MFLAG_INCLUDE, &len);
	if (!test) return false;

	/* MM */
	p += len;
	test = asl_core_str_match(p, DIGITS, 2, 2, MFLAG_INCLUDE, &len);
	if (!test) return false;
	t.tm_mon = asl_core_str_to_uint32(p, len);
	if (t.tm_mon < 1) return false;
	if (t.tm_mon > 12) return false;
	t.tm_mon -= 1;

	/* [-] */
	p += len;
	test = asl_core_str_match_char(p, '-', 0, MFLAG_INCLUDE, &len);
	if (!test) return false;

	/* DD */
	p += len;
	test = asl_core_str_match(p, DIGITS, 2, 2, MFLAG_INCLUDE, &len);
	if (!test) return false;
	t.tm_mday = asl_core_str_to_uint32(p, len);
	if (t.tm_mday > 31) return false;

	/* T or t */
	p += len;
	test = asl_core_str_match(p, "Tt", 1, 1, MFLAG_INCLUDE, &len);
	if (!test) return false;

	/* hh */
	p += len;
	test = asl_core_str_match(p, DIGITS, 2, 2, MFLAG_INCLUDE, &len);
	if (!test) return false;
	t.tm_hour = asl_core_str_to_uint32(p, len);
	if (t.tm_hour > 23) return false;

	/* [:] */
	p += len;
	test = asl_core_str_match_char(p, ':', 0, MFLAG_INCLUDE, &len);
	if (!test) return false;

	/* mm */
	p += len;
	test = asl_core_str_match(p, DIGITS, 2, 2, MFLAG_INCLUDE, &len);
	if (!test) return false;
	t.tm_min = asl_core_str_to_uint32(p, len);
	if (t.tm_min > 59) return false;

	/* [:] */
	p += len;
	test = asl_core_str_match_char(p, ':', 0, MFLAG_INCLUDE, &len);
	if (!test) return false;

	/* ss */
	p += len;
	test = asl_core_str_match(p, DIGITS, 2, 2, MFLAG_INCLUDE, &len);
	if (!test) return false;
	t.tm_sec = asl_core_str_to_uint32(p, len);
	if (t.tm_sec > 59) return false;

	p += len;

	/* default to local time if we hit the end of the string */
	if ((*p == '\0') || (*p == ' ') || (*p == '\t') || (*p == '\n'))
	{
		t.tm_isdst = -1;

		if (tlen != NULL) *tlen = p - target;
		if (tval != NULL) *tval = mktime(&t);

		return true;
	}

	/* Z, z, +, or - */
	test = asl_core_str_match(p, "Zz+-", 1, 1, MFLAG_INCLUDE, &len);
	if (!test) return false;

	if ((*p == 'Z') || (*p == 'z'))
	{
		/* matched string must be followed by space, tab, newline (not counted in length) */
		p += len;
		if (*p != '\0')
		{
			test = asl_core_str_match(p, " \t\n", 1, 1, MFLAG_INCLUDE, &len);
			if (!test) return false;
		}

		if (tlen != NULL) *tlen = p - target;
		if (tval != NULL) *tval = timegm(&t);

		return true;
	}

	if (*p == '-') sign = 1;

	/* [h]h */
	p += len;
	test = asl_core_str_match(p, DIGITS, 1, 2, MFLAG_INCLUDE, &len);
	if (!test) return false;
	tzh = asl_core_str_to_uint32(p, len);
	if (tzh > 23) return false;

	/* [:] */
	p += len;
	test = asl_core_str_match_char(p, ':', 0, MFLAG_INCLUDE, &len);
	if (!test) return false;

	/* mm */
	p += len;
	test = asl_core_str_match(p, DIGITS, 0, 2, MFLAG_INCLUDE, &len);
	if (!test) return false;
	tzs = asl_core_str_to_uint32(p, len);
	if (tzs > 59) return false;

	t.tm_sec += (sign * (tzh * SECONDS_PER_HOUR) + (tzs * SECONDS_PER_MINUTE));

	/* matched string must be followed by space, tab, newline (not counted in length) */
	p += len;
	if (*p != '\0')
	{
		test = asl_core_str_match(p, " \t\n", 1, 1, MFLAG_INCLUDE, &len);
		if (!test) return false;
	}

	if (tlen != NULL) *tlen = p - target;
	if (tval != NULL) *tval = timegm(&t);

	return true;
}

time_t
asl_core_parse_time(const char *in, uint32_t *tlen)
{
	time_t tval = 0;
	uint32_t inlen;

	if (tlen != NULL) *tlen = 0;

	if (in == NULL) return -1;

	/*
	 * Heuristics to determine the string format.
	 * Warning: this code must be checked and may need to be adjusted if new formats are added.
	 */
	inlen = strlen(in);
	if (inlen == 0) return -1;

	/* leading plus or minus means it must be a relative time */
	if ((in[0] == '+') || (in[0] == '-'))
	{
		if (asl_core_str_match_absolute_or_relative_time(in, &tval, tlen)) return tval;
		return -1;
	}

	/* leading alphabetic char means it must be ctime() format */
	if (((in[0] >= 'a') && (in[0] <= 'z')) || ((in[0] >= 'A') && (in[0] <= 'Z')))
	{
		if (asl_core_str_match_c_time(in, &tval, tlen)) return tval;
		return -1;
	}

	/* only absolute, dotted, or iso8601 formats at this point */

	/* one to for chars means it must be absolute */
	if (inlen < 5)
	{
		if (asl_core_str_match_absolute_or_relative_time(in, &tval, tlen)) return tval;
		return -1;
	}

	/* check for dot */
	if (in[4] == '.')
	{
		if (asl_core_str_match_dotted_time(in, &tval, tlen)) return tval;
		return -1;
	}

	/* only absolute or iso8601 at this point */

	/* check for absolute first, since that's quicker */
	if (asl_core_str_match_absolute_or_relative_time(in, &tval, tlen)) return tval;

	if (asl_core_str_match_iso_8601_time(in, &tval, tlen)) return tval;

	return -1;
}

/*
 * asl_parse_time is old SPI used all over the place.
 */
time_t
asl_parse_time(const char *in)
{
	return asl_core_parse_time(in, NULL);
}
