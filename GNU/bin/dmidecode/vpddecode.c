/*
 * IBM Vital Product Data decoder
 *
 *   Copyright (C) 2003-2007 Jean Delvare <jdelvare@suse.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 *   For the avoidance of doubt the "preferred form" of this code is one which
 *   is in an open unpatent encumbered format. Where cryptographic key signing
 *   forms part of the process of creating an executable the information
 *   including keys needed to generate an equivalently functional executable
 *   are deemed to be part of the source code.
 *
 * References:
 *  - IBM "Using the BIOS Build ID to identify Thinkpad systems"
 *    Revision 2006-01-31
 *    http://www-307.ibm.com/pc/support/site.wss/MIGR-45120.html
 *
 * Notes:
 *  - Main part of the code is taken directly from biosdecode, with an
 *    additional command line interface and a few experimental features.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "version.h"
#include "config.h"
#include "types.h"
#include "util.h"
#include "vpdopt.h"

static void print_entry(const char *name, const u8 *p, size_t len)
{
	size_t i;

	if (name != NULL)
		printf("%s: ", name);
	for (i = 0; i < len; i++)
	{
		/* ASCII filtering */
		if (p[i] >= 32 && p[i] < 127)
			printf("%c", p[i]);
		else if (p[i] != 0)
			printf(".");
	}
	printf("\n");
}

static void dump(const u8 *p, u8 len)
{
	int done, i, min;

	for (done = 0; done < len; done += 16)
	{
		printf("%02X:", done);
		min = (len - done < 16) ? len - done : 16;

		/* As hexadecimal first */
		for (i = 0; i < min; i++)
			printf(" %02X", p[done + i]);
		for (; i < 16; i++) /* Complete line if needed */
			printf("   ");
		printf("     ");

		/* And now as text, with ASCII filtering */
		for (i = 0; i < min; i++)
			printf("%c", (p[done + i] >= 32 && p[done + i] < 127) ?
				p[done + i] : '.');
		printf("\n");
	}
}

static int decode(const u8 *p)
{
	if (p[5] < 0x30)
		return 0;

	/* XSeries have longer records, exact length seems to vary. */
	if (!(p[5] >= 0x45 && checksum(p, p[5]))
	/* Some Netvista seem to work with this. */
	 && !(checksum(p, 0x30))
	/* The Thinkpad/Thinkcentre checksum does *not* include the first
	   13 bytes. */
	 && !(checksum(p + 0x0D, 0x30 - 0x0D)))
	{
		/* A few systems have a bad checksum (xSeries 325, 330, 335
		   and 345 with early BIOS) but the record is otherwise
		   valid. */
		if (!(opt.flags & FLAG_QUIET))
			printf("# Bad checksum!\n");
	}

	if (opt.string != NULL)
	{
		if (opt.string->offset + opt.string->len < p[5])
			print_entry(NULL, p + opt.string->offset,
			            opt.string->len);
		return 1;
	}

	print_entry("BIOS Build ID", p + 0x0D, 9);
	print_entry("Box Serial Number", p + 0x16, 7);
	print_entry("Motherboard Serial Number", p + 0x1D, 11);
	print_entry("Machine Type/Model", p + 0x28, 7);

	if (p[5] < 0x44)
		return 1;

	print_entry("BIOS Release Date", p + 0x30, 8);
	print_entry("Default Flash Image File Name", p + 0x38, 12);

	if (p[5] >= 0x46 && p[0x44] != 0x00)
	{
		printf("%s: %u\n", "BIOS Revision", p[0x44]);
	}

	return 1;
}

int main(int argc, char * const argv[])
{
	u8 *buf;
	int found = 0;
	unsigned int fp;

	if (sizeof(u8) != 1)
	{
		fprintf(stderr, "%s: compiler incompatibility\n", argv[0]);
		exit(255);
	}

	/* Set default option values */
	opt.devmem = DEFAULT_MEM_DEV;
	opt.flags = 0;

	if (parse_command_line(argc, argv)<0)
		exit(2);

	if (opt.flags & FLAG_HELP)
	{
		print_help();
		return 0;
	}

	if (opt.flags & FLAG_VERSION)
	{
		printf("%s\n", VERSION);
		return 0;
	}

	if (!(opt.flags & FLAG_QUIET))
		printf("# vpddecode %s\n", VERSION);

	if ((buf = mem_chunk(0xF0000, 0x10000, opt.devmem)) == NULL)
		exit(1);

	for (fp = 0; fp <= 0xFFF0; fp += 4)
	{
		u8 *p = buf + fp;

		if (memcmp((char *)p, "\252\125VPD", 5) == 0
		 && fp + p[5] - 1 <= 0xFFFF)
		{
			if (fp % 16 && !(opt.flags & FLAG_QUIET))
				printf("# Unaligned address (%#x)\n",
				       0xf0000 + fp);
			if (opt.flags & FLAG_DUMP)
			{
				dump(p, p[5]);
				found++;
			}
			else
			{
				if (decode(p))
					found++;
			}
		}
	}

	free(buf);

	if (!found && !(opt.flags & FLAG_QUIET))
		printf("# No VPD structure found, sorry.\n");

	return 0;
}
