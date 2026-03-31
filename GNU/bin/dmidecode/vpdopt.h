/*
 * Command line handling of vpddecode
 * This file is part of the dmidecode project.
 *
 *   Copyright (C) 2005-2006 Jean Delvare <jdelvare@suse.de>
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
 */

#include <sys/types.h>

struct string_keyword
{
	const char *keyword;
	off_t offset;
	size_t len;
};

struct opt
{
	const char *devmem;
	unsigned int flags;
	const struct string_keyword *string;
};
extern struct opt opt;

#define FLAG_VERSION            (1 << 0)
#define FLAG_HELP               (1 << 1)
#define FLAG_DUMP               (1 << 2)
#define FLAG_QUIET              (1 << 3)

int parse_command_line(int argc, char * const argv[]);
void print_help(void);
