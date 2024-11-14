/*
 * Generic output functions
 * This file is part of the dmidecode project.
 *
 *   Copyright (C) 2020 Jean Delvare <jdelvare@suse.de>
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

#include <stdarg.h>
#include <stdio.h>
#include "dmioutput.h"

void pr_comment(const char *format, ...)
{
	va_list args;

	printf("# ");
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
}

void pr_info(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
}

void pr_handle(const struct dmi_header *h)
{
	printf("Handle 0x%04X, DMI type %d, %d bytes\n",
	       h->handle, h->type, h->length);
}

void pr_handle_name(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
}

void pr_attr(const char *name, const char *format, ...)
{
	va_list args;

	printf("\t%s: ", name);

	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
}

void pr_subattr(const char *name, const char *format, ...)
{
	va_list args;

	printf("\t\t%s: ", name);

	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
}

void pr_list_start(const char *name, const char *format, ...)
{
	va_list args;

	printf("\t%s:", name);

	/* format is optional, skip value if not provided */
	if (format)
	{
		printf(" ");
		va_start(args, format);
		vprintf(format, args);
		va_end(args);
	}
	printf("\n");

}

void pr_list_item(const char *format, ...)
{
	va_list args;

	printf("\t\t");

	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
}

void pr_list_end(void)
{
	/* a no-op for text output */
}

void pr_sep(void)
{
	printf("\n");
}

void pr_struct_err(const char *format, ...)
{
	va_list args;

	printf("\t");

	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
}
