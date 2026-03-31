/*
 * Compaq Ownership Tag
 *
 *   Copyright (C) 2003-2005 Jean Delvare <jdelvare@suse.de>
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
 *  - Compaq "Technical Reference Guide for Compaq Deskpro 4000 and 6000"
 *    First Edition
 *    http://h18000.www1.hp.com/support/techpubs/technical_reference_guides/113a1097.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "version.h"
#include "config.h"
#include "types.h"
#include "util.h"

/* Options are global */
struct opt
{
	const char *devmem;
	unsigned int flags;
};
static struct opt opt;

#define FLAG_VERSION            (1 << 0)
#define FLAG_HELP               (1 << 1)

static void ownership(u32 base, const char *pname, const char *devmem)
{
	u8 *buf;
	int i;

	/* read the ownership tag */
	if ((buf = mem_chunk(base, 0x51, devmem)) == NULL)
	{
		perror(pname);
		return;
	}

	/* chop the trailing garbage */
	i = 0x4f;
	while (i >= 0 && (buf[i] == 0x20 || buf[i] == 0x00))
		i--;
	buf[i + 1] = '\0';

	/* filter and print */
	if (i >= 0)
	{
		for (; i >= 0; i--)
		{
			if (buf[i] < 32 || (buf[i] >= 127 && buf[i] < 160))
				buf[i] = '?';
		}
		printf("%s\n", (char *)buf);
	}

	free(buf);
}

static u32 decode(const u8 *p)
{
	int i;

	/* integrity checking (lack of checksum) */
	for (i = 0; i < p[4]; i++)
	{
		if (p[5 + i * 10] != '$'
		 || !(p[6 + i * 10] >= 'A' && p[6 + i * 10] <= 'Z')
		 || !(p[7 + i * 10] >= 'A' && p[7 + i * 10] <= 'Z')
		 || !(p[8 + i * 10] >= 'A' && p[8 + i * 10] <= 'Z'))
		{
			printf("\t Abnormal Entry! Please report. [%02x %02x %02x %02x]\n",
				p[5 + i * 10], p[6 + i * 10],
				p[7 + i * 10], p[8 + i * 10]);
			return 0;
		}
	}

	/* search for the right entry */
	for (i = 0; i < p[4]; i++)
		if (memcmp(p + 5 + i * 10, "$ERB", 4) == 0)
			return DWORD(p + 9 + i * 10);

	return 0;
}

/* Return -1 on error, 0 on success */
static int parse_command_line(int argc, char * const argv[])
{
	int option;
	const char *optstring = "d:hV";
	struct option longopts[] = {
		{ "dev-mem", required_argument, NULL, 'd' },
		{ "help", no_argument, NULL, 'h' },
		{ "version", no_argument, NULL, 'V' },
		{ NULL, 0, NULL, 0 }
	};

	while ((option = getopt_long(argc, argv, optstring, longopts, NULL)) != -1)
		switch (option)
		{
			case 'd':
				opt.devmem = optarg;
				break;
			case 'h':
				opt.flags |= FLAG_HELP;
				break;
			case 'V':
				opt.flags |= FLAG_VERSION;
				break;
			case '?':
				return -1;
		}

	return 0;
}

static void print_help(void)
{
	static const char *help =
		"Usage: ownership [OPTIONS]\n"
		"Options are:\n"
		" -d, --dev-mem FILE     Read memory from device FILE (default: " DEFAULT_MEM_DEV ")\n"
		" -h, --help             Display this help text and exit\n"
		" -V, --version          Display the version and exit\n";

	printf("%s", help);
}

int main(int argc, char * const argv[])
{
	u8 *buf;
	off_t fp;
	int ok = 0;

	if (sizeof(u8) != 1 || sizeof(u32) != 4)
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

	if ((buf = mem_chunk(0xE0000, 0x20000, opt.devmem)) == NULL)
		exit(1);

	for (fp = 0; !ok && fp <= 0x1FFF0; fp += 16)
	{
		u8 *p = buf + fp;

		if (memcmp((char *)p, "32OS", 4) == 0)
		{
			off_t len = p[4] * 10 + 5;

			if (fp + len - 1 <= 0x1FFFF)
			{
				u32 base;

				if ((base = decode(p)))
				{
					ok = 1;
					ownership(base, argv[0], opt.devmem);
				}
			}
		}
	}

	free(buf);

	return 0;
}
