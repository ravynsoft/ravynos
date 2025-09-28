/* strftime - formatted time and date to a string */
/*
 * Modified slightly by Chet Ramey for inclusion in Bash
 */
/*
 * strftime.c
 *
 * Public-domain implementation of ISO C library routine.
 *
 * If you can't do prototypes, get GCC.
 *
 * The C99 standard now specifies just about all of the formats
 * that were additional in the earlier versions of this file.
 *
 * For extensions from SunOS, add SUNOS_EXT.
 * For extensions from HP/UX, add HPUX_EXT.
 * For VMS dates, add VMS_EXT.
 * For complete POSIX semantics, add POSIX_SEMANTICS.
 *
 * The code for %c, %x, and %X follows the C99 specification for
 * the "C" locale.
 *
 * This version ignores LOCALE information.
 * It also doesn't worry about multi-byte characters.
 * So there.
 *
 * Arnold Robbins
 * January, February, March, 1991
 * Updated March, April 1992
 * Updated April, 1993
 * Updated February, 1994
 * Updated May, 1994
 * Updated January, 1995
 * Updated September, 1995
 * Updated January, 1996
 * Updated July, 1997
 * Updated October, 1999
 * Updated September, 2000
 * Updated December, 2001
 * Updated January, 2011
 * Updated April, 2012
 *
 * Fixes from ado@elsie.nci.nih.gov,
 * February 1991, May 1992
 * Fixes from Tor Lillqvist tml@tik.vtt.fi,
 * May 1993
 * Further fixes from ado@elsie.nci.nih.gov,
 * February 1994
 * %z code from chip@chinacat.unicom.com,
 * Applied September 1995
 * %V code fixed (again) and %G, %g added,
 * January 1996
 * %v code fixed, better configuration,
 * July 1997
 * Moved to C99 specification.
 * September 2000
 * Fixes from Tanaka Akira <akr@m17n.org>
 * December 2001
 */
#include <config.h>

#include <sys/types.h>

#include <stdio.h>
#include <ctype.h>
#include <posixtime.h>
#include <errno.h>

#include <stdlib.h>
#include <string.h>

/* defaults: season to taste */
#define SUNOS_EXT	1	/* stuff in SunOS strftime routine */
#define VMS_EXT		1	/* include %v for VMS date format */
#define HPUX_EXT	1	/* non-conflicting stuff in HP-UX date */
#define POSIX_SEMANTICS	1	/* call tzset() if TZ changes */
#define POSIX_2008	1	/* flag and fw for C, F, G, Y formats */

#undef strchr	/* avoid AIX weirdness */

#if !defined (errno)
extern int errno;
#endif

#if defined (SHELL)
extern char *get_string_value (const char *);
#endif

extern void tzset(void);
static int weeknumber(const struct tm *timeptr, int firstweekday);
static int iso8601wknum(const struct tm *timeptr);

#ifndef inline
#ifdef __GNUC__
#define inline	__inline__
#else
#define inline	/**/
#endif
#endif

#define range(low, item, hi)	max(low, min(item, hi))

/* Whew! This stuff is a mess. */
#if !defined(OS2) && !defined(MSDOS) && !defined(__CYGWIN__) && defined(HAVE_TZNAME)
extern char *tzname[2];
extern int daylight;
#if defined(SOLARIS) || defined(mips) || defined (M_UNIX)
extern long int timezone, altzone;
#else
#  if defined (HPUX) || defined(__hpux)
extern long int timezone;
#  else
#    if !defined(__CYGWIN__)
extern int timezone, altzone;
#    endif
#  endif
#endif
#endif

#undef min	/* just in case */

/* min --- return minimum of two numbers */

static inline int
min(int a, int b)
{
	return (a < b ? a : b);
}

#undef max	/* also, just in case */

/* max --- return maximum of two numbers */

static inline int
max(int a, int b)
{
	return (a > b ? a : b);
}

#ifdef POSIX_2008
/* iso_8601_2000_year --- format a year per ISO 8601:2000 as in 1003.1 */

static void
iso_8601_2000_year(char *buf, int year, size_t fw)
{
	int extra;
	char sign = '\0';

	if (year >= -9999 && year <= 9999) {
		sprintf(buf, "%0*d", (int) fw, year);
		return;
	}

	/* now things get weird */
	if (year > 9999) {
		sign = '+';
	} else {
		sign = '-';
		year = -year;
	}

	extra = year / 10000;
	year %= 10000;
	sprintf(buf, "%c_%04d_%d", sign, extra, year);
}
#endif /* POSIX_2008 */

/* strftime --- produce formatted time */

size_t
strftime(char *s, size_t maxsize, const char *format, const struct tm *timeptr)
{
	char *endp = s + maxsize;
	char *start = s;
	auto char tbuf[100];
	long off;
	int i, w, oerrno;
	long y;
	static short first = 1;
#ifdef POSIX_SEMANTICS
	static char *savetz = NULL;
	static int savetzlen = 0;
	char *tz;
#endif /* POSIX_SEMANTICS */
#ifndef HAVE_TM_ZONE
#ifndef HAVE_TM_NAME
#ifndef HAVE_TZNAME
#ifndef __CYGWIN__
	extern char *timezone();
	struct timeval tv;
	struct timezone zone;
#endif /* __CYGWIN__ */
#endif /* HAVE_TZNAME */
#endif /* HAVE_TM_NAME */
#endif /* HAVE_TM_ZONE */
#ifdef POSIX_2008
	int pad;
	size_t fw;
	char flag;
#endif /* POSIX_2008 */

	/* various tables, useful in North America */
	static const char *days_a[] = {
		"Sun", "Mon", "Tue", "Wed",
		"Thu", "Fri", "Sat",
	};
	static const char *days_l[] = {
		"Sunday", "Monday", "Tuesday", "Wednesday",
		"Thursday", "Friday", "Saturday",
	};
	static const char *months_a[] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
	};
	static const char *months_l[] = {
		"January", "February", "March", "April",
		"May", "June", "July", "August", "September",
		"October", "November", "December",
	};
	static const char *ampm[] = { "AM", "PM", };

	oerrno = errno;

	if (s == NULL || format == NULL || timeptr == NULL || maxsize == 0)
		return 0;

	/* quick check if we even need to bother */
	if (strchr(format, '%') == NULL && strlen(format) + 1 >= maxsize)
		return 0;

#ifndef POSIX_SEMANTICS
	if (first) {
		tzset();
		first = 0;
	}
#else	/* POSIX_SEMANTICS */
#if defined (SHELL)
	tz = get_string_value ("TZ");
#else
	tz = getenv("TZ");
#endif
	if (first) {
		if (tz != NULL) {
			int tzlen = strlen(tz);

			savetz = (char *) malloc(tzlen + 1);
			if (savetz != NULL) {
				savetzlen = tzlen + 1;
				strcpy(savetz, tz);
			}
		}
		tzset();
		first = 0;
	}
	/* if we have a saved TZ, and it is different, recapture and reset */
	if (tz && savetz && (tz[0] != savetz[0] || strcmp(tz, savetz) != 0)) {
		i = strlen(tz) + 1;
		if (i > savetzlen) {
			savetz = (char *) realloc(savetz, i);
			if (savetz) {
				savetzlen = i;
				strcpy(savetz, tz);
			}
		} else
			strcpy(savetz, tz);
		tzset();
	}
#endif	/* POSIX_SEMANTICS */

	for (; *format && s < endp - 1; format++) {
		tbuf[0] = '\0';
		if (*format != '%') {
			*s++ = *format;
			continue;
		}
#ifdef POSIX_2008
		pad = '\0';
		fw = 0;
		flag = '\0';
		switch (*++format) {
		case '+':
			flag = '+';
			/* fall through */
		case '0':
			pad = '0';
			format++;
			break;

		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			break;

		default:
			format--;
			goto again;
		}
		for (; isdigit(*format); format++) {
			fw = fw * 10 + (*format - '0');
		}
		format--;
#endif /* POSIX_2008 */

	again:
		switch (*++format) {
		case '\0':
			*s++ = '%';
			goto out;

		case '%':
			*s++ = '%';
			continue;

		case 'a':	/* abbreviated weekday name */
			if (timeptr->tm_wday < 0 || timeptr->tm_wday > 6)
				strcpy(tbuf, "?");
			else
				strcpy(tbuf, days_a[timeptr->tm_wday]);
			break;

		case 'A':	/* full weekday name */
			if (timeptr->tm_wday < 0 || timeptr->tm_wday > 6)
				strcpy(tbuf, "?");
			else
				strcpy(tbuf, days_l[timeptr->tm_wday]);
			break;

		case 'b':	/* abbreviated month name */
		short_month:
			if (timeptr->tm_mon < 0 || timeptr->tm_mon > 11)
				strcpy(tbuf, "?");
			else
				strcpy(tbuf, months_a[timeptr->tm_mon]);
			break;

		case 'B':	/* full month name */
			if (timeptr->tm_mon < 0 || timeptr->tm_mon > 11)
				strcpy(tbuf, "?");
			else
				strcpy(tbuf, months_l[timeptr->tm_mon]);
			break;

		case 'c':	/* appropriate date and time representation */
			/*
			 * This used to be:
			 *
			 * strftime(tbuf, sizeof tbuf, "%a %b %e %H:%M:%S %Y", timeptr);
			 *
			 * Now, per the ISO 1999 C standard, it this:
			 */
			strftime(tbuf, sizeof tbuf, "%A %B %d %T %Y", timeptr);
			break;

		case 'C':
#ifdef POSIX_2008
			if (pad != '\0' && fw > 0) {
				size_t min_fw = (flag ? 3 : 2);

				fw = max(fw, min_fw);
				sprintf(tbuf, flag
						? "%+0*ld"
						: "%0*ld", (int) fw,
						(timeptr->tm_year + 1900L) / 100);
			} else
#endif /* POSIX_2008 */
		century:
				sprintf(tbuf, "%02ld", (timeptr->tm_year + 1900L) / 100);
			break;

		case 'd':	/* day of the month, 01 - 31 */
			i = range(1, timeptr->tm_mday, 31);
			sprintf(tbuf, "%02d", i);
			break;

		case 'D':	/* date as %m/%d/%y */
			strftime(tbuf, sizeof tbuf, "%m/%d/%y", timeptr);
			break;

		case 'e':	/* day of month, blank padded */
			sprintf(tbuf, "%2d", range(1, timeptr->tm_mday, 31));
			break;

		case 'E':
			/* POSIX (now C99) locale extensions, ignored for now */
			goto again;

		case 'F':	/* ISO 8601 date representation */
		{
#ifdef POSIX_2008
			/*
			 * Field width for %F is for the whole thing.
			 * It must be at least 10.
			 */
			char m_d[10];
			strftime(m_d, sizeof m_d, "-%m-%d", timeptr);
			size_t min_fw = 10;

			if (pad != '\0' && fw > 0) {
				fw = max(fw, min_fw);
			} else {
				fw = min_fw;
			}

			fw -= 6;	/* -XX-XX at end are invariant */

			iso_8601_2000_year(tbuf, timeptr->tm_year + 1900, fw);
			strcat(tbuf, m_d);
#else
			strftime(tbuf, sizeof tbuf, "%Y-%m-%d", timeptr);
#endif /* POSIX_2008 */
		}
			break;

		case 'g':
		case 'G':
			/*
			 * Year of ISO week.
			 *
			 * If it's December but the ISO week number is one,
			 * that week is in next year.
			 * If it's January but the ISO week number is 52 or
			 * 53, that week is in last year.
			 * Otherwise, it's this year.
			 */
			w = iso8601wknum(timeptr);
			if (timeptr->tm_mon == 11 && w == 1)
				y = 1900L + timeptr->tm_year + 1;
			else if (timeptr->tm_mon == 0 && w >= 52)
				y = 1900L + timeptr->tm_year - 1;
			else
				y = 1900L + timeptr->tm_year;

			if (*format == 'G') {
#ifdef POSIX_2008
				if (pad != '\0' && fw > 0) {
					size_t min_fw = 4;

					fw = max(fw, min_fw);
					sprintf(tbuf, flag
							? "%+0*ld"
							: "%0*ld", (int) fw,
							y);
				} else
#endif /* POSIX_2008 */
					sprintf(tbuf, "%ld", y);
			}
			else
				sprintf(tbuf, "%02ld", y % 100);
			break;

		case 'h':	/* abbreviated month name */
			goto short_month;

		case 'H':	/* hour, 24-hour clock, 00 - 23 */
			i = range(0, timeptr->tm_hour, 23);
			sprintf(tbuf, "%02d", i);
			break;

		case 'I':	/* hour, 12-hour clock, 01 - 12 */
			i = range(0, timeptr->tm_hour, 23);
			if (i == 0)
				i = 12;
			else if (i > 12)
				i -= 12;
			sprintf(tbuf, "%02d", i);
			break;

		case 'j':	/* day of the year, 001 - 366 */
			sprintf(tbuf, "%03d", timeptr->tm_yday + 1);
			break;

		case 'm':	/* month, 01 - 12 */
			i = range(0, timeptr->tm_mon, 11);
			sprintf(tbuf, "%02d", i + 1);
			break;

		case 'M':	/* minute, 00 - 59 */
			i = range(0, timeptr->tm_min, 59);
			sprintf(tbuf, "%02d", i);
			break;

		case 'n':	/* same as \n */
			tbuf[0] = '\n';
			tbuf[1] = '\0';
			break;

		case 'O':
			/* POSIX (now C99) locale extensions, ignored for now */
			goto again;

		case 'p':	/* am or pm based on 12-hour clock */
			i = range(0, timeptr->tm_hour, 23);
			if (i < 12)
				strcpy(tbuf, ampm[0]);
			else
				strcpy(tbuf, ampm[1]);
			break;

		case 'r':	/* time as %I:%M:%S %p */
			strftime(tbuf, sizeof tbuf, "%I:%M:%S %p", timeptr);
			break;

		case 'R':	/* time as %H:%M */
			strftime(tbuf, sizeof tbuf, "%H:%M", timeptr);
			break;

#if defined(HAVE_MKTIME)
		case 's':	/* time as seconds since the Epoch */
		{
			struct tm non_const_timeptr;

			non_const_timeptr = *timeptr;
			sprintf(tbuf, "%ld", mktime(& non_const_timeptr));
			break;
		}
#endif /* defined(HAVE_MKTIME) */

		case 'S':	/* second, 00 - 60 */
			i = range(0, timeptr->tm_sec, 60);
			sprintf(tbuf, "%02d", i);
			break;

		case 't':	/* same as \t */
			tbuf[0] = '\t';
			tbuf[1] = '\0';
			break;

		case 'T':	/* time as %H:%M:%S */
		the_time:
			strftime(tbuf, sizeof tbuf, "%H:%M:%S", timeptr);
			break;

		case 'u':
		/* ISO 8601: Weekday as a decimal number [1 (Monday) - 7] */
			sprintf(tbuf, "%d", timeptr->tm_wday == 0 ? 7 :
					timeptr->tm_wday);
			break;

		case 'U':	/* week of year, Sunday is first day of week */
			sprintf(tbuf, "%02d", weeknumber(timeptr, 0));
			break;

		case 'V':	/* week of year according ISO 8601 */
			sprintf(tbuf, "%02d", iso8601wknum(timeptr));
			break;

		case 'w':	/* weekday, Sunday == 0, 0 - 6 */
			i = range(0, timeptr->tm_wday, 6);
			sprintf(tbuf, "%d", i);
			break;

		case 'W':	/* week of year, Monday is first day of week */
			sprintf(tbuf, "%02d", weeknumber(timeptr, 1));
			break;

		case 'x':	/* appropriate date representation */
			strftime(tbuf, sizeof tbuf, "%A %B %d %Y", timeptr);
			break;

		case 'X':	/* appropriate time representation */
			goto the_time;
			break;

		case 'y':	/* year without a century, 00 - 99 */
		year:
			i = timeptr->tm_year % 100;
			sprintf(tbuf, "%02d", i);
			break;

		case 'Y':	/* year with century */
#ifdef POSIX_2008
			if (pad != '\0' && fw > 0) {
				size_t min_fw = 4;

				fw = max(fw, min_fw);
				sprintf(tbuf, flag
						? "%+0*ld"
						: "%0*ld", (int) fw,
						1900L + timeptr->tm_year);
			} else
#endif /* POSIX_2008 */
			sprintf(tbuf, "%ld", 1900L + timeptr->tm_year);
			break;

		/*
		 * From: Chip Rosenthal <chip@chinacat.unicom.com>
		 * Date: Sun, 19 Mar 1995 00:33:29 -0600 (CST)
		 * 
		 * Warning: the %z [code] is implemented by inspecting the
		 * timezone name conditional compile settings, and
		 * inferring a method to get timezone offsets. I've tried
		 * this code on a couple of machines, but I don't doubt
		 * there is some system out there that won't like it.
		 * Maybe the easiest thing to do would be to bracket this
		 * with an #ifdef that can turn it off. The %z feature
		 * would be an admittedly obscure one that most folks can
		 * live without, but it would be a great help to those of
		 * us that muck around with various message processors.
		 */
 		case 'z':	/* time zone offset east of GMT e.g. -0600 */
 			if (timeptr->tm_isdst < 0)
 				break;
#ifdef HAVE_TM_NAME
			/*
			 * Systems with tm_name probably have tm_tzadj as
			 * secs west of GMT.  Convert to mins east of GMT.
			 */
			off = -timeptr->tm_tzadj / 60;
#else /* !HAVE_TM_NAME */
#ifdef HAVE_TM_ZONE
			/*
			 * Systems with tm_zone probably have tm_gmtoff as
			 * secs east of GMT.  Convert to mins east of GMT.
			 */
			off = timeptr->tm_gmtoff / 60;
#else /* !HAVE_TM_ZONE */
#if HAVE_TZNAME
			/*
			 * Systems with tzname[] probably have timezone as
			 * secs west of GMT.  Convert to mins east of GMT.
			 */
#  if defined(__hpux) || defined (HPUX) || defined(__CYGWIN__)
			off = -timezone / 60;
#  else
			/* ADR: 4 August 2001, fixed this per gazelle@interaccess.com */
			off = -(daylight ? altzone : timezone) / 60;
#  endif
#else /* !HAVE_TZNAME */
			gettimeofday(& tv, & zone);
			off = -zone.tz_minuteswest;
#endif /* !HAVE_TZNAME */
#endif /* !HAVE_TM_ZONE */
#endif /* !HAVE_TM_NAME */
			if (off < 0) {
				tbuf[0] = '-';
				off = -off;
			} else {
				tbuf[0] = '+';
			}
			sprintf(tbuf+1, "%02ld%02ld", off/60, off%60);
			break;

		case 'Z':	/* time zone name or abbreviation */
#ifdef HAVE_TZNAME
			i = (daylight && timeptr->tm_isdst > 0); /* 0 or 1 */
			strcpy(tbuf, tzname[i]);
#else
#ifdef HAVE_TM_ZONE
			strcpy(tbuf, timeptr->tm_zone);
#else
#ifdef HAVE_TM_NAME
			strcpy(tbuf, timeptr->tm_name);
#else
			gettimeofday(& tv, & zone);
			strcpy(tbuf, timezone(zone.tz_minuteswest,
						timeptr->tm_isdst > 0));
#endif /* HAVE_TM_NAME */
#endif /* HAVE_TM_ZONE */
#endif /* HAVE_TZNAME */
			break;

#ifdef SUNOS_EXT
		case 'k':	/* hour, 24-hour clock, blank pad */
			sprintf(tbuf, "%2d", range(0, timeptr->tm_hour, 23));
			break;

		case 'l':	/* hour, 12-hour clock, 1 - 12, blank pad */
			i = range(0, timeptr->tm_hour, 23);
			if (i == 0)
				i = 12;
			else if (i > 12)
				i -= 12;
			sprintf(tbuf, "%2d", i);
			break;
#endif

#ifdef HPUX_EXT
		case 'N':	/* Emperor/Era name */
			/* this is essentially the same as the century */
			goto century;	/* %C */

		case 'o':	/* Emperor/Era year */
			goto year;	/* %y */
#endif /* HPUX_EXT */


#ifdef VMS_EXT
		case 'v':	/* date as dd-bbb-YYYY */
			sprintf(tbuf, "%2d-%3.3s-%4ld",
				range(1, timeptr->tm_mday, 31),
				months_a[range(0, timeptr->tm_mon, 11)],
				timeptr->tm_year + 1900L);
			for (i = 3; i < 6; i++)
				if (islower(tbuf[i]))
					tbuf[i] = toupper(tbuf[i]);
			break;
#endif

		default:
			tbuf[0] = '%';
			tbuf[1] = *format;
			tbuf[2] = '\0';
			break;
		}
		i = strlen(tbuf);
		if (i) {
			if (s + i < endp - 1) {
				strcpy(s, tbuf);
				s += i;
			} else
				return 0;
		}
	}
out:
	if (s < endp && *format == '\0') {
		*s = '\0';
		if (s == start)
			errno = oerrno;
		return (s - start);
	} else
		return 0;
}

/* isleap --- is a year a leap year? */

static int
isleap(long year)
{
	return ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0);
}


/* iso8601wknum --- compute week number according to ISO 8601 */

static int
iso8601wknum(const struct tm *timeptr)
{
	/*
	 * From 1003.2:
	 *	If the week (Monday to Sunday) containing January 1
	 *	has four or more days in the new year, then it is week 1;
	 *	otherwise it is the highest numbered week of the previous
	 *	year (52 or 53), and the next week is week 1.
	 *
	 * ADR: This means if Jan 1 was Monday through Thursday,
	 *	it was week 1, otherwise week 52 or 53.
	 *
	 * XPG4 erroneously included POSIX.2 rationale text in the
	 * main body of the standard. Thus it requires week 53.
	 */

	int weeknum, jan1day, diff;

	/* get week number, Monday as first day of the week */
	weeknum = weeknumber(timeptr, 1);

	/*
	 * With thanks and tip of the hatlo to tml@tik.vtt.fi
	 *
	 * What day of the week does January 1 fall on?
	 * We know that
	 *	(timeptr->tm_yday - jan1.tm_yday) MOD 7 ==
	 *		(timeptr->tm_wday - jan1.tm_wday) MOD 7
	 * and that
	 * 	jan1.tm_yday == 0
	 * and that
	 * 	timeptr->tm_wday MOD 7 == timeptr->tm_wday
	 * from which it follows that. . .
 	 */
	jan1day = timeptr->tm_wday - (timeptr->tm_yday % 7);
	if (jan1day < 0)
		jan1day += 7;

	/*
	 * If Jan 1 was a Monday through Thursday, it was in
	 * week 1.  Otherwise it was last year's highest week, which is
	 * this year's week 0.
	 *
	 * What does that mean?
	 * If Jan 1 was Monday, the week number is exactly right, it can
	 *	never be 0.
	 * If it was Tuesday through Thursday, the weeknumber is one
	 *	less than it should be, so we add one.
	 * Otherwise, Friday, Saturday or Sunday, the week number is
	 * OK, but if it is 0, it needs to be 52 or 53.
	 */
	switch (jan1day) {
	case 1:		/* Monday */
		break;
	case 2:		/* Tuesday */
	case 3:		/* Wednesday */
	case 4:		/* Thursday */
		weeknum++;
		break;
	case 5:		/* Friday */
	case 6:		/* Saturday */
	case 0:		/* Sunday */
		if (weeknum == 0) {
#ifdef USE_BROKEN_XPG4
			/* XPG4 (as of March 1994) says 53 unconditionally */
			weeknum = 53;
#else
			/* get week number of last week of last year */
			struct tm dec31ly;	/* 12/31 last year */
			dec31ly = *timeptr;
			dec31ly.tm_year--;
			dec31ly.tm_mon = 11;
			dec31ly.tm_mday = 31;
			dec31ly.tm_wday = (jan1day == 0) ? 6 : jan1day - 1;
			dec31ly.tm_yday = 364 + isleap(dec31ly.tm_year + 1900L);
			weeknum = iso8601wknum(& dec31ly);
#endif
		}
		break;
	}

	if (timeptr->tm_mon == 11) {
		/*
		 * The last week of the year
		 * can be in week 1 of next year.
		 * Sigh.
		 *
		 * This can only happen if
		 *	M   T  W
		 *	29  30 31
		 *	30  31
		 *	31
		 */
		int wday, mday;

		wday = timeptr->tm_wday;
		mday = timeptr->tm_mday;
		if (   (wday == 1 && (mday >= 29 && mday <= 31))
		    || (wday == 2 && (mday == 30 || mday == 31))
		    || (wday == 3 &&  mday == 31))
			weeknum = 1;
	}

	return weeknum;
}

/* weeknumber --- figure how many weeks into the year */

/* With thanks and tip of the hatlo to ado@elsie.nci.nih.gov */

static int
weeknumber(const struct tm *timeptr, int firstweekday)
{
	int wday = timeptr->tm_wday;
	int ret;

	if (firstweekday == 1) {
		if (wday == 0)	/* sunday */
			wday = 6;
		else
			wday--;
	}
	ret = ((timeptr->tm_yday + 7 - wday) / 7);
	if (ret < 0)
		ret = 0;
	return ret;
}

#if 0
/* ADR --- I'm loathe to mess with ado's code ... */

Date:         Wed, 24 Apr 91 20:54:08 MDT
From: Michal Jaegermann <audfax!emory!vm.ucs.UAlberta.CA!NTOMCZAK>
To: arnold@audiofax.com

Hi Arnold,
in a process of fixing of strftime() in libraries on Atari ST I grabbed
some pieces of code from your own strftime.  When doing that it came
to mind that your weeknumber() function compiles a little bit nicer
in the following form:
/*
 * firstweekday is 0 if starting in Sunday, non-zero if in Monday
 */
{
    return (timeptr->tm_yday - timeptr->tm_wday +
	    (firstweekday ? (timeptr->tm_wday ? 8 : 1) : 7)) / 7;
}
How nicer it depends on a compiler, of course, but always a tiny bit.

   Cheers,
   Michal
   ntomczak@vm.ucs.ualberta.ca
#endif

#ifdef	TEST_STRFTIME

/*
 * NAME:
 *	tst
 *
 * SYNOPSIS:
 *	tst
 *
 * DESCRIPTION:
 *	"tst" is a test driver for the function "strftime".
 *
 * OPTIONS:
 *	None.
 *
 * AUTHOR:
 *	Karl Vogel
 *	Control Data Systems, Inc.
 *	vogelke@c-17igp.wpafb.af.mil
 *
 * BUGS:
 *	None noticed yet.
 *
 * COMPILE:
 *	cc -o tst -DTEST_STRFTIME strftime.c
 */

/* ADR: I reformatted this to my liking, and deleted some unneeded code. */

#ifndef NULL
#include	<stdio.h>
#endif
#include	<sys/time.h>
#include	<string.h>

#define		MAXTIME		132

/*
 * Array of time formats.
 */

static char *array[] =
{
	"(%%A)      full weekday name, var length (Sunday..Saturday)  %A",
	"(%%B)       full month name, var length (January..December)  %B",
	"(%%C)                                               Century  %C",
	"(%%D)                                       date (%%m/%%d/%%y)  %D",
	"(%%E)                           Locale extensions (ignored)  %E",
	"(%%F)       full month name, var length (January..December)  %F",
	"(%%H)                          hour (24-hour clock, 00..23)  %H",
	"(%%I)                          hour (12-hour clock, 01..12)  %I",
	"(%%M)                                       minute (00..59)  %M",
	"(%%N)                                      Emperor/Era Name  %N",
	"(%%O)                           Locale extensions (ignored)  %O",
	"(%%R)                                 time, 24-hour (%%H:%%M)  %R",
	"(%%S)                                       second (00..60)  %S",
	"(%%T)                              time, 24-hour (%%H:%%M:%%S)  %T",
	"(%%U)    week of year, Sunday as first day of week (00..53)  %U",
	"(%%V)                    week of year according to ISO 8601  %V",
	"(%%W)    week of year, Monday as first day of week (00..53)  %W",
	"(%%X)     appropriate locale time representation (%H:%M:%S)  %X",
	"(%%Y)                           year with century (1970...)  %Y",
	"(%%Z) timezone (EDT), or blank if timezone not determinable  %Z",
	"(%%a)          locale's abbreviated weekday name (Sun..Sat)  %a",
	"(%%b)            locale's abbreviated month name (Jan..Dec)  %b",
	"(%%c)           full date (Sat Nov  4 12:02:33 1989)%n%t%t%t  %c",
	"(%%d)                             day of the month (01..31)  %d",
	"(%%e)               day of the month, blank-padded ( 1..31)  %e",
	"(%%h)                                should be same as (%%b)  %h",
	"(%%j)                            day of the year (001..366)  %j",
	"(%%k)               hour, 24-hour clock, blank pad ( 0..23)  %k",
	"(%%l)               hour, 12-hour clock, blank pad ( 0..12)  %l",
	"(%%m)                                        month (01..12)  %m",
	"(%%o)                                      Emperor/Era Year  %o",
	"(%%p)              locale's AM or PM based on 12-hour clock  %p",
	"(%%r)                   time, 12-hour (same as %%I:%%M:%%S %%p)  %r",
	"(%%u) ISO 8601: Weekday as decimal number [1 (Monday) - 7]   %u",
	"(%%v)                                VMS date (dd-bbb-YYYY)  %v",
	"(%%w)                       day of week (0..6, Sunday == 0)  %w",
	"(%%x)                appropriate locale date representation  %x",
	"(%%y)                      last two digits of year (00..99)  %y",
	"(%%z)      timezone offset east of GMT as HHMM (e.g. -0500)  %z",
	(char *) NULL
};

/* main routine. */

int
main(argc, argv)
int argc;
char **argv;
{
	long time();

	char *next;
	char string[MAXTIME];

	int k;
	int length;

	struct tm *tm;

	long clock;

	/* Call the function. */

	clock = time((long *) 0);
	tm = localtime(&clock);

	for (k = 0; next = array[k]; k++) {
		length = strftime(string, MAXTIME, next, tm);
		printf("%s\n", string);
	}

	exit(0);
}
#endif	/* TEST_STRFTIME */
