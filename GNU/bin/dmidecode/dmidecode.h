/*
 * This file is part of the dmidecode project.
 *
 *   Copyright (C) 2005-2023 Jean Delvare <jdelvare@suse.de>
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

#ifndef DMIDECODE_H
#define DMIDECODE_H

#include "types.h"

struct dmi_header
{
	u8 type;
	u8 length;
	u16 handle;
	u8 *data;
};

enum cpuid_type
{
	cpuid_none,
	cpuid_80386,
	cpuid_80486,
	cpuid_arm_legacy,
	cpuid_arm_soc_id,
	cpuid_x86_intel,
	cpuid_x86_amd,
	cpuid_loongarch,
};

extern enum cpuid_type cpuid_type;

int is_printable(const u8 *data, int len);
const char *dmi_string(const struct dmi_header *dm, u8 s);
void dmi_print_memory_size(const char *addr, u64 code, int shift);
void dmi_print_cpuid(void (*print_cb)(const char *name, const char *format, ...),
		     const char *label, enum cpuid_type sig, const u8 *p);

#endif
