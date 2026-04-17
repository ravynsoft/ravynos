/*
 * Copyright (c) 2000-2008,2010-2012 Apple Inc. All rights reserved.
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
/* $FreeBSD: src/sys/msdosfs/msdosfs_conv.c,v 1.29 1999/08/28 00:48:08 peter Exp $ */
/*	$NetBSD: msdosfs_conv.c,v 1.25 1997/11/17 15:36:40 ws Exp $	*/

/*-
 * Copyright (C) 1995, 1997 Wolfgang Solfrank.
 * Copyright (C) 1995, 1997 TooLs GmbH.
 * All rights reserved.
 * Original code by Paul Popelka (paulp@uts.amdahl.com) (see below).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by TooLs GmbH.
 * 4. The name of TooLs GmbH may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY TOOLS GMBH ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TOOLS GMBH BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Written by Paul Popelka (paulp@uts.amdahl.com)
 *
 * You can do anything you want with this software, just don't say you wrote
 * it, and don't remove this notice.
 *
 * This software is provided "as is".
 *
 * The author supplies this software to be publicly redistributed on the
 * understanding that the author is not responsible for the correct
 * functioning of this software in any circumstances and is not liable for
 * any damages caused by this software.
 *
 * October 1992
 */

/*
 * System include files.
 */

#ifndef USB_STORAGE_PLUGIN
#include <sys/param.h>
#include <sys/time.h>
#include <sys/systm.h>
#include <sys/dirent.h>
#else
#include "Conv.h"
#endif

/*
 * MSDOSFS include files.
 */
#include "direntry.h"

/*
 * Total number of days that have passed for each month in a regular year.
 */
static uint16_t regyear[] = {
	31, 59, 90, 120, 151, 181,
	212, 243, 273, 304, 334, 365
};

/*
 * Total number of days that have passed for each month in a leap year.
 */
static uint16_t leapyear[] = {
	31, 60, 91, 121, 152, 182,
	213, 244, 274, 305, 335, 366
};

/*
 * Variables used to remember parts of the last time conversion.  Maybe we
 * can avoid a full conversion.
 */
static uint32_t  lasttime;
static uint32_t  lastday;
static uint16_t lastddate;
static uint16_t lastdtime;

/*
 * This variable contains the number of seconds that local time is west of GMT
 * It is updated every time an msdosfs volume is mounted.  This value is
 * essentially tz_minuteswest * 60 - (tz_dsttime ? 3600 : 0) based on the
 * timezone returned by gettimeofday() in the mount_msdosfs tool.
 *
 * This has a problem with daylight savings time.  If the current daylight
 * savings time setting does not match the date being converted, then it
 * will be off by one hour.  The only way to properly fix this is to know
 * (inside the kernel) what dates should be adjusted for daylight savings,
 * and which should not.  That would let us return correct GMT for dates
 * throughout the year.  Of course, the GUI would have to do the opposite
 * conversion when converting to local time for display to the user.
 */
__private_extern__ int32_t msdos_secondsWest = 0;

/*
 * Convert the unix version of time to dos's idea of time to be used in
 * file timestamps. The passed in unix time is assumed to be in GMT.
 */
void msdosfs_unix2dostime(struct timespec *tsp, u_int16_t *ddp, u_int16_t *dtp, u_int8_t *dhp)
{
	uint32_t t;
	uint32_t days;
	uint32_t inc;
	uint32_t year;
	uint32_t month;
	uint16_t *months;

	/*
	 * If the time from the last conversion is the same as now, then
	 * skip the computations and use the saved result.
	 */
	t = (uint32_t)(tsp->tv_sec - msdos_secondsWest);
	t &= ~1;	/* Round down to multiple of 2 seconds */
	if (lasttime != t) {
		lasttime = t;
		lastdtime = (((t / 2) % 30) << DT_2SECONDS_SHIFT)
		    + (((t / 60) % 60) << DT_MINUTES_SHIFT)
		    + (((t / 3600) % 24) << DT_HOURS_SHIFT);

		/*
		 * If the number of days since 1970 is the same as the last
		 * time we did the computation then skip all this leap year
		 * and month stuff.
		 */
		days = t / (24 * 60 * 60);
		if (days != lastday) {
			lastday = days;
			for (year = 1970;; year++) {
				inc = year & 0x03 ? 365 : 366;
				if (days < inc)
					break;
				days -= inc;
			}
			months = year & 0x03 ? regyear : leapyear;
			for (month = 0; days >= months[month]; month++)
				;
			if (month > 0)
				days -= months[month - 1];
			lastddate = ((days + 1) << DD_DAY_SHIFT)
			    + ((month + 1) << DD_MONTH_SHIFT);
			/*
			 * Remember dos's idea of time is relative to 1980.
			 * unix's is relative to 1970.  If somehow we get a
			 * time before 1980 then don't give totally crazy
			 * results.
			 */
			if (year > 1980)
				lastddate += (year - 1980) << DD_YEAR_SHIFT;
		}
	}
	if (dtp)
		*dtp = lastdtime;
	if (dhp)
		*dhp = (tsp->tv_sec & 1) * 100 + tsp->tv_nsec / 10000000;

	*ddp = lastddate;
}

/*
 * The number of seconds between Jan 1, 1970 and Jan 1, 1980. In that
 * interval there were 8 regular years and 2 leap years.
 */
#define	SECONDSTO1980	(((8 * 365) + (2 * 366)) * (24 * 60 * 60))

static uint16_t lastdosdate;
static uint32_t  lastseconds;

/*
 * Convert from dos' idea of time to unix'. This will probably only be
 * called from the stat(), and fstat() system calls and so probably need
 * not be too efficient.
 */
void msdosfs_dos2unixtime(u_int dd, u_int dt, u_int dh, struct timespec *tsp)
{
	uint32_t seconds;
	uint32_t month;
	uint32_t year;
	uint32_t days;
	uint16_t *months;

	if (dd == 0) {
		/*
		 * Uninitialized field, return the epoch.
		 */
		tsp->tv_sec = 0;
		tsp->tv_nsec = 0;
		return;
	}
	seconds = (((dt & DT_2SECONDS_MASK) >> DT_2SECONDS_SHIFT) << 1)
	    + ((dt & DT_MINUTES_MASK) >> DT_MINUTES_SHIFT) * 60
	    + ((dt & DT_HOURS_MASK) >> DT_HOURS_SHIFT) * 3600
	    + dh / 100;
	/*
	 * If the year, month, and day from the last conversion are the
	 * same then use the saved value.
	 */
	if (lastdosdate != dd) {
		lastdosdate = dd;
		year = (dd & DD_YEAR_MASK) >> DD_YEAR_SHIFT;
		days = year * 365;
		days += year / 4 + 1;	/* add in leap days */
		if ((year & 0x03) == 0)
			days--;		/* if year is a leap year */
		months = year & 0x03 ? regyear : leapyear;
		month = (dd & DD_MONTH_MASK) >> DD_MONTH_SHIFT;
		if (month < 1 || month > 12) {
			month = 1;
		}
		if (month > 1)
			days += months[month - 2];
		days += ((dd & DD_DAY_MASK) >> DD_DAY_SHIFT) - 1;
		lastseconds = (days * 24 * 60 * 60) + SECONDSTO1980;
	}
	tsp->tv_sec = seconds + lastseconds + msdos_secondsWest;
	tsp->tv_nsec = (dh % 100) * 10000000;
}

/*
 * Unicode (LSB) to Win Latin1 (ANSI CodePage 1252).
 * Also converts ASCII to upper case.
 *
 * 0 - character disallowed in long file name.
 * 1 - character should be replaced by '_' in DOS file name, 
 *     and generation number inserted.
 * 2 - character ('.' and ' ') should be skipped in DOS file name,
 *     and generation number inserted.
 */
static u_char
unilsb2dos[256] = {
	0,    1,    1,    1,    1,    1,    1,    1,	/* 00-07 */
	1,    1,    1,    1,    1,    1,    1,    1,	/* 08-0f */
	1,    1,    1,    1,    1,    1,    1,    1,	/* 10-17 */
	1,    1,    1,    1,    1,    1,    1,    1,	/* 18-1f */
	2,    0x21, 1,    0x23, 0x24, 0x25, 0x26, 0x27,	/* 20-27 */
	0x28, 0x29, 1,    1,    1,    0x2d, 2,    0,	/* 28-2f */
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,	/* 30-37 */
	0x38, 0x39, 1,    1,    1,    1,    1,    1,	/* 38-3f */
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,	/* 40-47 */
	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,	/* 48-4f */
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,	/* 50-57 */
	0x58, 0x59, 0x5a, 1,    1,    1,    0x5e, 0x5f,	/* 58-5f */
	0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,	/* 60-67 */
	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,	/* 68-6f */
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,	/* 70-77 */
	0x58, 0x59, 0x5a, 0x7b, 1,    0x7d, 0x7e, 0x7f,	/* 78-7f */
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,	/* 80-87 */
	0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,	/* 88-8f */
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,	/* 90-97 */
	0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,	/* 98-9f */
	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,	/* a0-a7 */
	0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,	/* a8-af */
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,	/* b0-b7 */
	0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,	/* b8-bf */
	0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,	/* c0-c7 */
	0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,	/* c8-cf */
	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,	/* d0-d7 */
	0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,	/* d8-df */
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,	/* e0-e7 */
	0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,	/* e8-ef */
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,	/* f0-f7 */
	0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,	/* f8-ff */
};

/* Unicode punctuation marks to Win Latin1 (ANSI CodePage 1252) */
static u_char
unipunct2dos[48] = {
	1,    1,    1,    0x96, 0x97, 1,    1,    1,  /* 2010-2017 */
	0x91, 0x92, 0x82, 1,    0x93, 0x94, 0x84, 1,  /* 2018-201F */
	0x86, 0x87, 0x95, 1,    1,    1,    0x85, 1,  /* 2020-2027 */
	1,    1,    1,    1,    1,    1,    1,    1,  /* 2028-202F */
	0x89, 1,    1,    1,    1,    1,    1,    1,  /* 2030-2037 */
	1,    0x8B, 0x9B, 1,    1,    1,    1,    1   /* 2038-203F */
};

/* Win Latin1 (ANSI CodePage 1252) to Unicode */
__private_extern__ u_int16_t
dos2unicode[32] = {
  0x20AC, 0x003f, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, /* 80-87 */
  0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x003f, 0x017D, 0x003f, /* 88-8F */
  0x003f, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, /* 90-97 */
  0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x003f, 0x017E, 0x0178, /* 98-9F */
};


/*
 * Case folding table for Latin-1.
 *
 * Maps 'A'..'Z' to 'a'..'z' and 0xc0..0xd6 to 0xe0..0xf6 and
 * 0xd8..0xde to 0xf8..0xfe.
 */
__private_extern__ u_char
l2u[256] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, /* 00-07 */
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, /* 08-0f */
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, /* 10-17 */
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, /* 18-1f */
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, /* 20-27 */
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, /* 28-2f */
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, /* 30-37 */
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, /* 38-3f */
	0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, /* 40-47 */
	0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, /* 48-4f */
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, /* 50-57 */
	0x78, 0x79, 0x7a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, /* 58-5f */
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, /* 60-67 */
	0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, /* 68-6f */
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, /* 70-77 */
	0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, /* 78-7f */
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, /* 80-87 */
	0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, /* 88-8f */
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, /* 90-97 */
	0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, /* 98-9f */
	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, /* a0-a7 */
	0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, /* a8-af */
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, /* b0-b7 */
	0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, /* b8-bf */
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, /* c0-c7 */
	0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, /* c8-cf */
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xd7, /* d0-d7 */
	0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xdf, /* d8-df */
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, /* e0-e7 */
	0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, /* e8-ef */
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, /* f0-f7 */
	0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff, /* f8-ff */
};

/*
 *	Look-up table to determine whether a given ASCII character is
 *	considered upper case, lower case, or neither.  Also indicates
 *	which characters should require generation of a long name.
 *
 *	Values are bit masks composed of the following constants:
 */
enum {
	CASE_LOWER	= 1,
	CASE_UPPER	= 2,
	CASE_LONG	= 4,	/* A long name should be generated */
};

static u_char
ascii_case[128] = {
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,	/* 00-07 */
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,	/* 08-0F */
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,	/* 10-17 */
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,	/* 18-1F */
	0x04, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,	/* 20-27 */
	0x00, 0x00, 0x04, 0x04, 0x04, 0x00, 0x04, 0x04,	/* 28-2F */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* 30-37 */
	0x00, 0x00, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,	/* 38-3F */
	0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,	/* 40-47 */
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,	/* 48-4F */
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,	/* 50-57 */
	0x02, 0x02, 0x02, 0x04, 0x04, 0x04, 0x00, 0x00,	/* 58-5F */
	0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,	/* 60-67 */
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,	/* 68-6F */
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,	/* 70-77 */
	0x01, 0x01, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00,	/* 78-7F */
};

#if 0
/*
 * Macintosh Unicode (LSB) to Microsoft Services for Macintosh (SFM) Unicode
 */
static u_int16_t
mac2sfm[128] = {
	0x0,    0xf001, 0xf002, 0xf003, 0xf004, 0xf005, 0xf006, 0xf007,	/* 00-07 */
	0xf008, 0xf009, 0xf00a, 0xf00b, 0xf00c, 0xf00d, 0xf00e, 0xf00f,	/* 08-0f */
	0xf010, 0xf011, 0xf012, 0xf013, 0xf014, 0xf015, 0xf016, 0xf017,	/* 10-17 */
	0xf018, 0xf019, 0xf01a, 0xf01b, 0xf01c, 0xf01d, 0xf01e, 0xf01f,	/* 18-1f */
	0x20,   0x21,   0xf020, 0x23,   0x24,   0x25,   0x26,   0x27,	/* 20-27 */
	0x28,   0x29,   0xf021, 0x2b,   0x2c,   0x2d,   0x2e,   0x2f,  	/* 28-2f */
	0x30,   0x31,   0x32,   0x33,   0x34,   0x35,   0x36,   0x37,	/* 30-37 */
	0x38,   0x39,   0xf022, 0x3b,   0xf023, 0x3d,   0xf024, 0xf025, /* 38-3f */
	0x40,   0x41,   0x42,   0x43,   0x44,   0x45,   0x46,   0x47,	/* 40-47 */
	0x48,   0x49,   0x4a,   0x4b,   0x4c,   0x4d,   0x4e,   0x4f,	/* 48-4f */
	0x50,   0x51,   0x52,   0x53,   0x54,   0x55,   0x56,   0x57,	/* 50-57 */
	0x58,   0x59,   0x5a,   0x5b,   0xf026, 0x5d,   0x5e,   0x5f,	/* 58-5f */
	0x60,   0x61,   0x62,   0x63,   0x64,   0x65,   0x66,   0x67,	/* 60-67 */
	0x68,   0x69,   0x6a,   0x6b,   0x6c,   0x6d,   0x6e,   0x6f,	/* 68-6f */
	0x70,   0x71,   0x72,   0x73,   0x74,   0x75,   0x76,   0x77,	/* 70-77 */
	0x78,   0x79,   0x7a,   0x7b,   0xf027, 0x7d,   0x7e,   0x7f,   /* 78-7f */
};

#define MAX_MAC2SFM			0x80
#define MAX_SFM2MAC			0x29
#define SFMCODE_PREFIX_MASK	0xf000 
/*
 * SFM Unicode (LSB) to Macintosh Unicode (LSB) 
 */
static u_char
sfm2mac[42] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,	/* 00-07 */
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,	/* 08-0F */
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,	/* 10-17 */
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,	/* 18-1F */
	0x22, 0x2a, 0x3a, 0x3c, 0x3e, 0x3f, 0x5c, 0x7c,	/* 20-27 */
	0x20, 0x2e, 	 							 	/* 28-29 */
};
#endif

/* map a Unicode char into a DOS char */
u_char msdosfs_unicode2dos(u_int16_t uc)
{
	if (uc < 0x100)
		return (unilsb2dos[uc]);

	if (uc > 0x2122)
		return (1);

	if (uc >= 0x2010 && uc <= 0x203F)
		return (unipunct2dos[uc - 0x2010]);
		
	if (uc >= 0x0152 && uc <= 0x02DC)
		switch (uc) {
		case 0x0152:
		    return (0x8C);  /* LATIN CAPITAL LIGATURE OE */
		case 0x0153:
		    return (0x9C);  /* LATIN SMALL LIGATURE OE */
		case 0x0160:
		    return (0x8A);  /* CAPITAL LETTER S WITH CARON */
		case 0x0161:
		    return (0x9A);  /* SMALL LETTER S WITH CARON */
		case 0x0178:
		    return (0x9F);  /* CAPITAL LETTER Y WITH DIAERESIS */
		case 0x017D:
		    return (0x8E);  /* CAPITAL LETTER Z WITH CARON */
		case 0x017E:
		    return (0x9E);  /* SMALL LETTER Z WITH CARON */
		case 0x0192:
		    return (0x83);  /* SMALL LETTER F WITH HOOK */
		case 0x02C6:
		    return (0x88);  /* MODIFIER LETTER CIRCUMFLEX ACCENT */
		case 0x02DC:
		    return (0x98);  /* SMALL TILDE */
		default:
			return (1);
		}

	if (uc == 0x20AC)
		return (0x80);  /* EURO SIGN */
	if (uc == 0x2122)
		return (0x99);  /* TRADE MARK SIGN */

	return (1);
}

/*
 * DOS filenames are made of 2 parts, the name part and the extension part.
 * The name part is 8 characters long and the extension part is 3
 * characters long.  They may contain trailing blanks if the name or
 * extension are not long enough to fill their respective fields.
 */

/*
 * Convert a DOS filename to a Unicode filename. And, return the number of
 * characters in the resulting unix filename.
 */
size_t msdosfs_dos2unicodefn(u_char dn[SHORT_NAME_LEN], u_int16_t *un, int lower)
{
	int i;
	u_char dc;
	int unichars = 0;

	/* 
	 * Copy the base name portion into the Unicode string.
	 */
	for (i = 0; i < 8; i++) {
		dc = *dn++;
		
		/*
		 * If first char of the filename is SLOT_E5 (0x05), then
		 * the real first char of the filename should be 0xe5.
		 * But, they couldn't just have a 0xe5 mean 0xe5 because
		 * that is used to mean a freed directory slot.
		 */
		if (i == 0 && dc == SLOT_E5)
			dc = 0xe5;

		/*
		 * If the base name was supposed to be lower case,
		 * then convert it.
		 */
		if (lower & LCASE_BASE)
			dc = l2u[dc];	/* Map to lower case equivalent */
		
		un[unichars++] = (dc < 0x80 || dc > 0x9F ? (u_int16_t)dc : dos2unicode[dc - 0x80]);
	}
	
	/*
	 * Get rid of trailing space padding in the base name.
	 */
	while (unichars > 0 && un[unichars-1]==' ')
	{
		-- unichars;
	}
	
	/*
	 * Copy the extension portion (with dot) into the Unicode string.
	 */
	un[unichars++] = '.';
	for (i = 0; i < 3; i++) {
		dc = *dn++;

		/*
		 * If the extension was supposed to be all lower case,
		 * then convert it.
		 */
		if (lower & LCASE_EXT)
			dc = l2u[dc];	/* Map to lower case equivalent */
		
		un[unichars++] = (dc < 0x80 || dc > 0x9F ? (u_int16_t)dc : dos2unicode[dc - 0x80]);
	}
	
	/*
	 * Get rid of trailing space padding in the extension (and the dot
	 * if there was no extension).
	 */
	for (i=0; i<3 && un[unichars-1]==' '; ++i)
	{
		--unichars;
	}
	if (i==3)			/* Was the extension entirely spaces? */
		--unichars;		/* Yes, so remove the dot, too. */

	return unichars;
}


/*
 * Convert a Unicode filename to an 8.3-style short name.
 *
 * This function implements the Basis-Name Generation Algorithm
 * from the FAT32 specification.  It does NOT implement the numeric
 * tail generation; that part is implemented elsewhere.
 *
 * In order to support setting the LCASE_BASE and LCASE_EXT flags, this
 * routine returns the corresponding value for those flags when the name can
 * be converted (when the function result is 1).
 *
 * The test is whether the name (base or extension) contains at least one
 * lower case letter, no upper case, and zero or more case-neutral characters
 * allowed in an 8.3 name (such as digits, dollar sign, ampersand, etc.).
 *
 * By experimentation with Windows XP, it seems to always create long names
 * for names that contain extended characters, such as letter e with acute
 * (regardless of upper/lower case).  This is true even on a U.S. system
 * with characters that are representable in code page 437 (which is in
 * fact what you see in the short name).  I'm guessing it uses pure short
 * names only if all characters are pure ASCII.
 *
 * Inputs:
 *	unicode			Points to a buffer of UTF-16 (native endian)
 *					containing the original name.
 *	unicode_length	The length of the Unicode name in UTF-16 code
 *					points.
 *
 * Outputs:
 *	short_name		The corresponding 8.3-style short name is returned
 *					in this buffer.  The short name is upper case.
 *	lower_case		A pointer to returned lower case flags.  The pointer
 *					must not be NULL.
 *
 * Returns:
 *	0	if name couldn't be converted (has disallowed characters).
 *	1	if the converted name is the same as the original
 *		(no long filename entry necessary for Win95).
 *	2	if conversion was successful, and long name entries should be created,
 *		but a generation number is not necessary (for example, the name had a
 *		mix of upper and lower case, contained non-ASCII characters, contained
 *		an embedded space, or contained a dot in the base name).
 *	3	if conversion was successful, long name entries should be created, and
 *		one or more characters of the original name were removed (thus, a
 *		generation number should also be added).
 */
int msdosfs_unicode_to_dos_name(const uint16_t *unicode,
								size_t unicode_length,
								uint8_t short_name[SHORT_NAME_LEN],
								u_int8_t *lower_case)
{
	int i;					/* An index into the Unicode name. */
	int j;					/* An index into the short name. */
	int conv = 1;			/* The function's result. */
	const uint16_t *cp;		/* A pointer for looping over the Unicode name. */
	const uint16_t *dp;		/* A pointer to the first char of extension (or NULL). */
	const uint16_t *dp1;	/* A pointer after last seen dot character. */
	uint16_t c;				/* The current character being examined. */
	int case_flags;			/* Accumulates upper/low/neither case information */
	
	*lower_case = 0;	/* default to all upper case, and clear undefined bits */
	
	/*
	 * Fill the dos filename string with blanks. These are DOS's pad
	 * characters.
	 */
	memset(short_name, ' ', SHORT_NAME_LEN);
	
	/*
	 * The filenames "." and ".." are handled specially, since they
	 * don't follow dos filename rules.
	 */
	if (unicode_length == 1 && unicode[0] == '.') {
		short_name[0] = '.';
		return 1;
	}
	if (unicode_length == 2 && unicode[0] == '.' && unicode[1] == '.') {
		short_name[0] = '.';
		short_name[1] = '.';
		return 1;
	}
	
	/*
	 * Filenames with some characters are not allowed!
	 *
	 * NOTE: For a name passed into the POSIX APIs, most of the disallowed
	 * characters will have already been replaced with alternate characters
	 * according to the Services For Macintosh conversions.
	 */
	for (cp = unicode, i = (int)unicode_length; --i >= 0; cp++)
		if (msdosfs_unicode2dos(*cp) == 0)
			return 0;
	
	/*
	 * Find the extension (if any).
	 *
	 * Note: A dot as the first or last character do not count as
	 * an extension.  Trailing blanks are supposed to be ignored,
	 * but we don't (and didn't previously) do that.
	 */
	dp = dp1 = NULL;
	for (cp = unicode + 1, i = (int)unicode_length - 1; --i >= 0;) {
		switch (*cp++) {
			case '.':
				if (!dp1)
					dp1 = cp;
				break;
			default:
				if (dp1)
					dp = dp1;
				dp1 = NULL;
				break;
		}
	}
	
	/*
	 * Convert the extension (if any).
	 */
	if (dp) {
		int l;		/* The length of the extension (in UTF-16 code points). */

		if (dp1)
			l = (int)(dp1 - dp);            /* Ignore trailing dots. */
		else
			l = (int)(unicode_length - (dp - unicode));
		
		/*
		 * Convert up to 3 characters of the extension.
		 */
		for (case_flags = i = 0, j = 8; i < l && j < SHORT_NAME_LEN; i++, j++) {
			c = dp[i];
			if (c < 0x80)
				case_flags |= ascii_case[c];
			else
				case_flags |= CASE_LONG;	/* Non-ASCII always requires a long name */
			if (c < 0x100)
   				c = l2u[c];
			c = msdosfs_unicode2dos(c);
			short_name[j] = c;
			if (c == 1) {
				conv = 3;					/* Character is not allowed in short names */
				short_name[j] = '_';		/* and must be replaced with underscore */
			}
			if (c == 2) {
				conv = 3;					/* Character is not allowed in short names */
				short_name[j--] = ' ';		/* and is not substituted */
			}
		}
		
		/*
		 * Check for other conditions which might require a long name.
		 */
		if ((case_flags & CASE_LONG) != 0 && conv != 3)
			conv = 2;			/* Force a long name for things like embedded spaces or non-ASCII */
		if (conv == 1) {
			if ((case_flags & (CASE_LOWER | CASE_UPPER)) == (CASE_LOWER | CASE_UPPER))
				conv = 2;		/* Force a long name for names with mixed case */
			else if (case_flags & CASE_LOWER)
				*lower_case |= LCASE_EXT;	/* Extension has lower case */
		}
		if (i < l)
			conv = 3;			/* Extension was longer than 3 characters */
		
		dp--;					/* dp points at the dot at the start of the extension. */
	} else {
		dp = cp;				/* dp points past the end of the Unicode name. */
	}
	
	/*
	 * Now convert the base name
	 *
	 * When we get here, dp points just past the last character of the base name.
	 */
	for (case_flags = i = j = 0; unicode < dp && j < 8; i++, j++, unicode++) {
        c = *unicode;
		if (c < 0x80)
			case_flags |= ascii_case[c];
		else
			case_flags |= CASE_LONG;	/* Non-ASCII always requires a long name */
        if (c < 0x100)
            c = l2u[c];
        c = msdosfs_unicode2dos(c);
        short_name[j] = c;
		if (c == 1) {
			conv = 3;		/* Character is not allowed in short names */
			short_name[j] = '_';	/* and must be replaced with underscore */
		}
		if (c == 2) {
			conv = 3;		/* Character is not allowed in short names */
			short_name[j--] = ' ';	/* and is not substituted */
		}
	}
	if ((case_flags & CASE_LONG) != 0 && conv != 3)
		conv = 2;	/* Force a long name for things like embedded spaces */
	if (conv == 1) {
		if ((case_flags & (CASE_LOWER | CASE_UPPER)) == (CASE_LOWER | CASE_UPPER))
			conv = 2;	/* Force a long name for names with mixed case */
		else if (case_flags & CASE_LOWER)
			*lower_case |= LCASE_BASE;	/* Base name has lower case */
	}
	
	if (unicode < dp)
		conv = 3;	/* Base name was longer than 8 characters */
	
	/*
	 * If the resulting base name was empty, generate a default
	 */
	if (!j)
		short_name[0] = '_';
	
	/*
	 * The first character cannot be E5, because that means a deleted entry
	 */
	if (short_name[0] == 0xe5)
		short_name[0] = SLOT_E5;
	
	/*
	 * If the name couldn't be represented as a short name, make sure the
	 * lower case flags are clear (in case the base or extension was all
	 * lower case, but the other was not, in which case we left one of the
	 * bits set above).
	 *
	 * That is, the lower case flags are only set if the name is a valid
	 * 8.3-style short name, where any letters in the base and extension
	 * are all upper or all lower case.  (The case of the base may differ
	 * from the case of the extension.)  Non-letter ASCII characters do
	 * not affect the lower case flags.
	 */
	if (conv != 1)
		*lower_case = 0;
	
	return conv;
}


/*
 * Add or update a generation number, and preceding tilde (~), to a short name.
 *
 * The base name is up to 8 characters.  We want to keep at least the first
 * character.  We need to insert a tilde before the generation number.  That
 * leaves us with room for up to 6 characters for the generation number.
 *
 * Parameters:
 *	short_name		Modified in place.  On input, a short name with no
 *					generation number, or a generation number less than
 *					or equal to the "generation" parameter.  On output,
 *					the short name with generation number.
 *	generation		The generation number to add or update.
 *
 * Result:
 *	0				The generation number was successfully added.
 *	ENAMETOOLONG	The generation number was too large to fit in the short name.
 */
int msdosfs_apply_generation_to_short_name(uint8_t short_name[SHORT_NAME_LEN],
										   unsigned generation)
{
	uint8_t	generation_text[6];			/* Note: this is stored in reverse order. */
	unsigned generation_text_length = 0;
	unsigned base_name_index;			/* Index into base name where generation string
										   should be stored. */
	/* Convert the generation number to ASCII. */
	do {
		generation_text[generation_text_length++] = '0' + (generation % 10);
		generation /= 10;
	} while (generation > 0 && generation_text_length < 6);
	
	/* Check for a too-large generation number. */
	if (generation != 0)
		return ENAMETOOLONG;
	
	/*
	 * Add or replace the generation string.
	 *
	 * We start by assuming the base name is the maximum 8 characters, then scan left
	 * to find a non-space character.  (FAT uses trailing space padding in short names.
	 * This also handles the case where the short name had an embedded space, and we'd
	 * like to avoid putting the generation string after an embedded space.)
	 */
	base_name_index = 8 - (generation_text_length + 1);		/* +1 is for the tilde. */
	while (short_name[base_name_index] == ' ' && base_name_index > 0)
	{
		--base_name_index;		/* Skip over trailing or embedded spaces. */
	}
	short_name[base_name_index++] = '~';
	while (generation_text_length > 0)
	{
		short_name[base_name_index++] = generation_text[--generation_text_length];
	}
	
	return 0;
}


/*
 * Create a Win95 long name directory entry
 * Note: assumes that the filename is valid,
 *	 i.e. doesn't consist solely of blanks and dots
 */
int msdosfs_unicode2winfn(const u_int16_t *un, int unlen, struct winentry *wep, int cnt, int chksum)
{
	u_int8_t *wcp;
	int i;
	u_int16_t code;

	un += (cnt - 1) * WIN_CHARS;
	unlen -= (cnt - 1) * WIN_CHARS;

	/*
	 * Initialize winentry to some useful default
	 */
	for (wcp = (u_int8_t *)wep, i = sizeof(*wep); --i >= 0; *wcp++ = 0xff);
	wep->weCnt = cnt;
	wep->weAttributes = ATTR_WIN95;
	wep->weReserved1 = 0;
	wep->weChksum = chksum;
	wep->weReserved2 = 0;

	/*
	 * Now convert the filename parts
	 */
	for (wcp = wep->wePart1, i = sizeof(wep->wePart1)/2; --i >= 0;) {
		if (--unlen < 0)
			goto done;
		code = *un++;
		*wcp++ = code & 0x00ff;
		*wcp++ = code >> 8;
	}
	for (wcp = wep->wePart2, i = sizeof(wep->wePart2)/2; --i >= 0;) {
		if (--unlen < 0)
			goto done;
		code = *un++;
		*wcp++ = code & 0x00ff;
		*wcp++ = code >> 8;
	}
	for (wcp = wep->wePart3, i = sizeof(wep->wePart3)/2; --i >= 0;) {
		if (--unlen < 0)
			goto done;
		code = *un++;
		*wcp++ = code & 0x00ff;
		*wcp++ = code >> 8;
	}
	if (!unlen)
		wep->weCnt |= WIN_LAST;
	return unlen;

done:
	*wcp++ = 0;
	*wcp   = 0;
	wep->weCnt |= WIN_LAST;
	return 0;
}



/*
 * Convert a Unicode character to a single known case.  Upper and lower case
 * variants of the same character produce the same result.
 *
 * As an implementation detail, we convert the character to lower case.
 *
 * Note: this currently only handles case folding of ASCII characters.  The
 * Unicode standard defines case equivalence for other characters (such as
 * precomposed characters), but I don't know whether Windows considers them
 * case equivalents.
 */
static inline u_int16_t case_fold(u_int16_t ch)
{
	if (ch < 0x100)
		return l2u[ch];
	else
		return ch;
}


/*
 * Compare a single long name entry to the corresponding portion of a
 * filename.  If the name matches, we return the on-disk name and a
 * flag indicating whether the name needed to be case folded.
 *
 * We also make sure all long name entries have the same checksum
 * value; if not, the long name is invalid and is not a match.
 *
 * Inputs:
 *	un[]		The name being looked up (the search name).
 *	ucslen		The number of code points in the name (length of un[] in elements).
 *	wep			The current long name entry being compared.
 *	chksum		The checksum field from previous long name entries.
 *
 * Outputs:
 *	found_name	The actual on-disk name that matched.  OPTIONAL.  MAY BE NULL.
 *	case_folded	Set to true if the on-disk name is a case variant of un[] (that is
 *				NOT a case-sensitive match), or if the on-disk name is not a match
 *				to un[].  Unchanged if the on-disk name is exactly equal to un[].
 *
 * Function result:
 *	-1			The name or checksums did not match
 * <checksum>	The name matched.  <checksum> is the checksum stored in all
 *				of the long name entries.
 */
int msdosfs_winChkName(const u_int16_t *un, int ucslen, struct winentry *wep, int chksum,
					   u_int16_t *found_name, boolean_t *case_folded)
{
	u_int8_t *cp;
	int i;
	u_int16_t code;
	
	/*
	 * First compare checksums
	 */
	if (wep->weCnt&WIN_LAST)
		chksum = wep->weChksum;
	else if (chksum != wep->weChksum)
		chksum = -1;
	if (chksum == -1)
		return -1;

	/*
	 * Offset of this entry
	 */
	i = ((wep->weCnt&WIN_CNT) - 1) * WIN_CHARS;
	un += i;
	if (found_name)
		found_name += i;
	
	if ((ucslen -= i) < 0)	/* Was "<=".  See below. */
		return -1;	/* More long name entries than the name would need */
	if ((wep->weCnt&WIN_LAST) && ucslen > WIN_CHARS)
		return -1;	/* Too few long name entries to hold the name */

        /*
         * [2865792] Some FAT implementations have a bug when the long
         * is an exact multiple of WIN_CHARS long.  They make an extra
         * long name entry containing only a terminating 0x0000 and
         * the 0xFFFF pad characters.  While this is out-of-spec
         * (i.e. corrupt), we can be graceful and handle it anyway,
         * like Windows does.
         *
         * We handle this case by falling through here with ucslen == 0.
         * We then expect to return during the first iteration of the
         * following for() loop where --ucslen goes negative, and
         * "cp" points to two zero bytes.
         */

	/*
	 * Compare the name parts
	 */
	for (cp = wep->wePart1, i = sizeof(wep->wePart1)/2; --i >= 0;) {
		code = (cp[1] << 8) | cp[0];
		if (--ucslen < 0) {
			/* Got to end of input name.  Are we also at end of on-disk name? */
			if (code == 0)
				return chksum;
			else
				return -1;
		}
		if (found_name)
			*found_name++ = code;
		if (code != *un)
		{
			/* Not an exact match.  Try case-insensitive match. */
			*case_folded = TRUE;
			if (case_fold(code) != case_fold(*un))
				return (-1);
		}
		cp += 2;
		un++;
	}
	for (cp = wep->wePart2, i = sizeof(wep->wePart2)/2; --i >= 0;) {
		code = (cp[1] << 8) | cp[0];
		if (--ucslen < 0) {
			/* Got to end of input name.  Are we also at end of on-disk name? */
			if (code == 0)
				return chksum;
			else
				return -1;
		}
		if (found_name)
			*found_name++ = code;
		if (code != *un)
		{
			/* Not an exact match.  Try case-insensitive match. */
			*case_folded = TRUE;
			if (case_fold(code) != case_fold(*un))
				return (-1);
		}
		cp += 2;
		un++;
	}
	for (cp = wep->wePart3, i = sizeof(wep->wePart3)/2; --i >= 0;) {
		code = (cp[1] << 8) | cp[0];
		if (--ucslen < 0) {
			/* Got to end of input name.  Are we also at end of on-disk name? */
			if (code == 0)
				return chksum;
			else
				return -1;
		}
		if (found_name)
			*found_name++ = code;
		if (code != *un)
		{
			/* Not an exact match.  Try case-insensitive match. */
			*case_folded = TRUE;
			if (case_fold(code) != case_fold(*un))
				return (-1);
		}
		cp += 2;
		un++;
	}
	return chksum;
}


/*
 * Collect Win95 filename Unicode chars into ucfn.  Stores the number
 * of characters in *unichars.  Returns the checksum, or -1 if impossible.
 */
int msdosfs_getunicodefn(struct winentry *wep, u_int16_t ucfn[WIN_MAXLEN], u_int16_t *unichars, int chksum)
{
	u_int8_t *cp;
	u_int16_t *np;
	u_int16_t *ep = &ucfn[WIN_MAXLEN];
	u_int16_t code;
	int i;

	if ((wep->weCnt&WIN_CNT) > howmany(WIN_MAXLEN, WIN_CHARS)
	    || !(wep->weCnt&WIN_CNT))
	{
		return -1;
	}

	/*
	 * First compare checksums
	 */
	if (wep->weCnt&WIN_LAST)
	{
		/*
		 * The "last" entry is the one we encounter first in the directory,
		 * so save off the checksum to compare against the other entries.
		 *
		 * The length we return here assumes the name is an exact multiple
		 * of WIN_CHARS long, and therefore has no terminator in the "last"
		 * entry.  If we in fact find a terminator, we'll adjust the length.
		 */
		chksum = wep->weChksum;
		*unichars = (wep->weCnt&WIN_CNT) * WIN_CHARS;
	}
	else if (chksum != wep->weChksum)
	{
		chksum = -1;
	}
	if (chksum == -1)
		return -1;

	/*
	 * Find the offset within ucfn where the first character of this
	 * long name entry should get stored.
	 */
	np = &ucfn[((wep->weCnt&WIN_CNT) - 1) * WIN_CHARS];

	/*
	 * Extract the characters from the three discontiguous parts.
	 *
	 * A maximum name (255 characters) will occupy 19 full entries
	 * with 13 characters per entry, and a partial (20th) entry
	 * containing 8 characters.  In that case, we'd better see the
	 * terminating 0-character in wePart2.  We only need to check
	 * for maximum name length while checking wePart2.
	 */
	for (cp = wep->wePart1, i = sizeof(wep->wePart1)/2; --i >= 0;)
	{
		code = (cp[1] << 8) | cp[0];
		switch (code) {
		case 0:
			*unichars = np - ucfn;
			return chksum;
		case '/':
			return -1;
		default:
			*np++ = code;
			break;
		}
		cp += 2;
	}
	for (cp = wep->wePart2, i = sizeof(wep->wePart2)/2; --i >= 0;)
	{
		code = (cp[1] << 8) | cp[0];
		switch (code) {
		case 0:
			*unichars = np - ucfn;
			return chksum;
		case '/':
			return -1;
		default:
			if (np >= ep)
			{
				/* The name is too long.  Return error. */
				return -1;
			}
			*np++ = code;
			break;
		}
		cp += 2;
	}
	for (cp = wep->wePart3, i = sizeof(wep->wePart3)/2; --i >= 0;)
	{
		code = (cp[1] << 8) | cp[0];
		switch (code) {
		case 0:
			*unichars = np - ucfn;
			return chksum;
		case '/':
			return -1;
		default:
			*np++ = code;
			break;
		}
		cp += 2;
	}
	return chksum;
}


/*
 * Compute the checksum of a DOS filename for Win95 use
 */
u_int8_t msdosfs_winChksum(u_int8_t *name)
{
	int i;
	u_int8_t s;

	for (s = 0, i = SHORT_NAME_LEN; --i >= 0; s += *name++)
		s = (s << 7)|(s >> 1);
	return s;
}

/*
 * Determine the number of slots necessary for Win95 names
 */
int msdosfs_winSlotCnt(const u_int16_t *un, int unlen)
{
#pragma unused (un)
	if (unlen > WIN_MAXLEN)
		return 0;
	return howmany(unlen, WIN_CHARS);
}
