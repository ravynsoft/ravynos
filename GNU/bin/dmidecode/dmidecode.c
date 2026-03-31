/*
 * DMI Decode
 *
 *   Copyright (C) 2000-2002 Alan Cox <alan@redhat.com>
 *   Copyright (C) 2002-2024 Jean Delvare <jdelvare@suse.de>
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
 * Unless specified otherwise, all references are aimed at the "System
 * Management BIOS Reference Specification, Version 3.2.0" document,
 * available from http://www.dmtf.org/standards/smbios.
 *
 * Note to contributors:
 * Please reference every value you add or modify, especially if the
 * information does not come from the above mentioned specification.
 *
 * Additional references:
 *  - Intel AP-485 revision 36
 *    "Intel Processor Identification and the CPUID Instruction"
 *    http://www.intel.com/support/processors/sb/cs-009861.htm
 *  - DMTF Common Information Model
 *    CIM Schema version 2.19.1
 *    http://www.dmtf.org/standards/cim/
 *  - IPMI 2.0 revision 1.0
 *    "Intelligent Platform Management Interface Specification"
 *    http://developer.intel.com/design/servers/ipmi/spec.htm
 *  - AMD publication #25481 revision 2.28
 *    "CPUID Specification"
 *    http://www.amd.com/us-en/assets/content_type/white_papers_and_tech_docs/25481.pdf
 *  - BIOS Integrity Services Application Programming Interface version 1.0
 *    http://www.intel.com/design/archives/wfm/downloads/bisspec.htm
 *  - DMTF DSP0239 version 1.1.0
 *    "Management Component Transport Protocol (MCTP) IDs and Codes"
 *    http://www.dmtf.org/standards/pmci
 *  - "TPM Main, Part 2 TPM Structures"
 *    Specification version 1.2, level 2, revision 116
 *    https://trustedcomputinggroup.org/tpm-main-specification/
 *  - "PC Client Platform TPM Profile (PTP) Specification"
 *    Family "2.0", Level 00, Revision 00.43, January 26, 2015
 *    https://trustedcomputinggroup.org/pc-client-platform-tpm-profile-ptp-specification/
 *  - "RedFish Host Interface Specification" (DMTF DSP0270)
 *    https://www.dmtf.org/sites/default/files/standards/documents/DSP0270_1.3.0.pdf
 *  - LoongArch Reference Manual, volume 1
 *    https://loongson.github.io/LoongArch-Documentation/LoongArch-Vol1-EN.html#_cpucfg
 */

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#if defined(__FreeBSD__) || defined(__DragonFly__)
#include <errno.h>
#include <kenv.h>
#endif

#include "version.h"
#include "config.h"
#include "types.h"
#include "util.h"
#include "dmidecode.h"
#include "dmiopt.h"
#include "dmioem.h"
#include "dmioutput.h"

#define out_of_spec "<OUT OF SPEC>"
static const char *bad_index = "<BAD INDEX>";

enum cpuid_type cpuid_type = cpuid_none;

#define SUPPORTED_SMBIOS_VER 0x030700

#define FLAG_NO_FILE_OFFSET     (1 << 0)
#define FLAG_STOP_AT_EOT        (1 << 1)

#define SYS_FIRMWARE_DIR "/sys/firmware/dmi/tables"
#define SYS_ENTRY_FILE SYS_FIRMWARE_DIR "/smbios_entry_point"
#define SYS_TABLE_FILE SYS_FIRMWARE_DIR "/DMI"

/*
 * Type-independant Stuff
 */

/* Returns 1 if the buffer contains only printable ASCII characters */
int is_printable(const u8 *data, int len)
{
	int i;

	for (i = 0; i < len; i++)
		if (data[i] < 32 || data[i] >= 127)
			return 0;

	return 1;
}

/* Replace non-ASCII characters with dots */
static void ascii_filter(char *bp, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++)
		if (bp[i] < 32 || bp[i] >= 127)
			bp[i] = '.';
}

static char *_dmi_string(const struct dmi_header *dm, u8 s, int filter)
{
	char *bp = (char *)dm->data;

	bp += dm->length;
	while (s > 1 && *bp)
	{
		bp += strlen(bp);
		bp++;
		s--;
	}

	if (!*bp)
		return NULL;

	if (filter)
		ascii_filter(bp, strlen(bp));

	return bp;
}

const char *dmi_string(const struct dmi_header *dm, u8 s)
{
	char *bp;

	if (s == 0)
		return "Not Specified";

	bp = _dmi_string(dm, s, 1);
	if (bp == NULL)
		return bad_index;

	return bp;
}

static const char *dmi_smbios_structure_type(u8 code)
{
	static const char *type[] = {
		"BIOS", /* 0 */
		"System",
		"Base Board",
		"Chassis",
		"Processor",
		"Memory Controller",
		"Memory Module",
		"Cache",
		"Port Connector",
		"System Slots",
		"On Board Devices",
		"OEM Strings",
		"System Configuration Options",
		"BIOS Language",
		"Group Associations",
		"System Event Log",
		"Physical Memory Array",
		"Memory Device",
		"32-bit Memory Error",
		"Memory Array Mapped Address",
		"Memory Device Mapped Address",
		"Built-in Pointing Device",
		"Portable Battery",
		"System Reset",
		"Hardware Security",
		"System Power Controls",
		"Voltage Probe",
		"Cooling Device",
		"Temperature Probe",
		"Electrical Current Probe",
		"Out-of-band Remote Access",
		"Boot Integrity Services",
		"System Boot",
		"64-bit Memory Error",
		"Management Device",
		"Management Device Component",
		"Management Device Threshold Data",
		"Memory Channel",
		"IPMI Device",
		"Power Supply",
		"Additional Information",
		"Onboard Device",
		"Management Controller Host Interface",
		"TPM Device",
		"Processor",
		"Firmware",
		"String Property" /* 46 */
	};

	if (code >= 128)
		return "OEM-specific";
	if (code <= 46)
		return type[code];
	return out_of_spec;
}

static int dmi_bcd_range(u8 value, u8 low, u8 high)
{
	if (value > 0x99 || (value & 0x0F) > 0x09)
		return 0;
	if (value < low || value > high)
		return 0;
	return 1;
}

static void dmi_dump(const struct dmi_header *h)
{
	static char raw_data[48];
	int row, i;
	unsigned int off;
	char *s;

	pr_list_start("Header and Data", NULL);
	for (row = 0; row < ((h->length - 1) >> 4) + 1; row++)
	{
		off = 0;
		for (i = 0; i < 16 && i < h->length - (row << 4); i++)
			off += sprintf(raw_data + off, i ? " %02X" : "%02X",
			       (h->data)[(row << 4) + i]);
		pr_list_item(raw_data);
	}
	pr_list_end();

	if ((h->data)[h->length] || (h->data)[h->length + 1])
	{
		pr_list_start("Strings", NULL);
		i = 1;
		while ((s = _dmi_string(h, i++, !(opt.flags & FLAG_DUMP))))
		{
			if (opt.flags & FLAG_DUMP)
			{
				int j, l = strlen(s) + 1;

				for (row = 0; row < ((l - 1) >> 4) + 1; row++)
				{
					off = 0;
					for (j = 0; j < 16 && j < l - (row << 4); j++)
						off += sprintf(raw_data + off,
						       j ? " %02X" : "%02X",
						       (unsigned char)s[(row << 4) + j]);
					pr_list_item(raw_data);
				}
				/* String isn't filtered yet so do it now */
				ascii_filter(s, l - 1);
			}
			pr_list_item("%s", s);
		}
		pr_list_end();
	}
}

/* shift is 0 if the value is in bytes, 1 if it is in kilobytes */
void dmi_print_memory_size(const char *attr, u64 code, int shift)
{
	unsigned long capacity;
	u16 split[7];
	static const char *unit[8] = {
		"bytes", "kB", "MB", "GB", "TB", "PB", "EB", "ZB"
	};
	int i;

	/*
	 * We split the overall size in powers of thousand: EB, PB, TB, GB,
	 * MB, kB and B. In practice, it is expected that only one or two
	 * (consecutive) of these will be non-zero.
	 */
	split[0] = code.l & 0x3FFUL;
	split[1] = (code.l >> 10) & 0x3FFUL;
	split[2] = (code.l >> 20) & 0x3FFUL;
	split[3] = ((code.h << 2) & 0x3FCUL) | (code.l >> 30);
	split[4] = (code.h >> 8) & 0x3FFUL;
	split[5] = (code.h >> 18) & 0x3FFUL;
	split[6] = code.h >> 28;

	/*
	 * Now we find the highest unit with a non-zero value. If the following
	 * is also non-zero, we use that as our base. If the following is zero,
	 * we simply display the highest unit.
	 */
	for (i = 6; i > 0; i--)
	{
		if (split[i])
			break;
	}
	if (i > 0 && split[i - 1])
	{
		i--;
		capacity = split[i] + (split[i + 1] << 10);
	}
	else
		capacity = split[i];

	pr_attr(attr, "%lu %s", capacity, unit[i + shift]);
}

/*
 * 7.1 BIOS Information (Type 0)
 */

static void dmi_bios_runtime_size(u32 code)
{
	const char *format;

	if (code & 0x000003FF)
	{
		format = "%u bytes";
	}
	else
	{
		format = "%u kB";
		code >>= 10;
	}

	pr_attr("Runtime Size", format, code);
}

static void dmi_bios_rom_size(u8 code1, u16 code2)
{
	static const char *unit[4] = {
		"MB", "GB", out_of_spec, out_of_spec
	};

	if (code1 != 0xFF)
	{
		u64 s = { .l = (code1 + 1) << 6 };
		dmi_print_memory_size("ROM Size", s, 1);
	}
	else
		pr_attr("ROM Size", "%u %s", code2 & 0x3FFF, unit[code2 >> 14]);
}

static void dmi_bios_characteristics(u64 code)
{
	/* 7.1.1 */
	static const char *characteristics[] = {
		"BIOS characteristics not supported", /* 3 */
		"ISA is supported",
		"MCA is supported",
		"EISA is supported",
		"PCI is supported",
		"PC Card (PCMCIA) is supported",
		"PNP is supported",
		"APM is supported",
		"BIOS is upgradeable",
		"BIOS shadowing is allowed",
		"VLB is supported",
		"ESCD support is available",
		"Boot from CD is supported",
		"Selectable boot is supported",
		"BIOS ROM is socketed",
		"Boot from PC Card (PCMCIA) is supported",
		"EDD is supported",
		"Japanese floppy for NEC 9800 1.2 MB is supported (int 13h)",
		"Japanese floppy for Toshiba 1.2 MB is supported (int 13h)",
		"5.25\"/360 kB floppy services are supported (int 13h)",
		"5.25\"/1.2 MB floppy services are supported (int 13h)",
		"3.5\"/720 kB floppy services are supported (int 13h)",
		"3.5\"/2.88 MB floppy services are supported (int 13h)",
		"Print screen service is supported (int 5h)",
		"8042 keyboard services are supported (int 9h)",
		"Serial services are supported (int 14h)",
		"Printer services are supported (int 17h)",
		"CGA/mono video services are supported (int 10h)",
		"NEC PC-98" /* 31 */
	};
	int i;

	/*
	 * This isn't very clear what this bit is supposed to mean
	 */
	if (code.l & (1 << 3))
	{
		pr_list_item("%s", characteristics[0]);
		return;
	}

	for (i = 4; i <= 31; i++)
		if (code.l & (1 << i))
			pr_list_item("%s", characteristics[i - 3]);
}

static void dmi_bios_characteristics_x1(u8 code)
{
	/* 7.1.2.1 */
	static const char *characteristics[] = {
		"ACPI is supported", /* 0 */
		"USB legacy is supported",
		"AGP is supported",
		"I2O boot is supported",
		"LS-120 boot is supported",
		"ATAPI Zip drive boot is supported",
		"IEEE 1394 boot is supported",
		"Smart battery is supported" /* 7 */
	};
	int i;

	for (i = 0; i <= 7; i++)
		if (code & (1 << i))
			pr_list_item("%s", characteristics[i]);
}

static void dmi_bios_characteristics_x2(u8 code)
{
	/* 37.1.2.2 */
	static const char *characteristics[] = {
		"BIOS boot specification is supported", /* 0 */
		"Function key-initiated network boot is supported",
		"Targeted content distribution is supported",
		"UEFI is supported",
		"System is a virtual machine",
		"Manufacturing mode is supported",
		"Manufacturing mode is enabled" /* 6 */
	};
	int i;

	for (i = 0; i <= 6; i++)
		if (code & (1 << i))
			pr_list_item("%s", characteristics[i]);
}

/*
 * 7.2 System Information (Type 1)
 */

static void dmi_system_uuid(void (*print_cb)(const char *name, const char *format, ...),
			    const char *attr, const u8 *p, u16 ver)
{
	int only0xFF = 1, only0x00 = 1;
	int i;

	for (i = 0; i < 16 && (only0x00 || only0xFF); i++)
	{
		if (p[i] != 0x00) only0x00 = 0;
		if (p[i] != 0xFF) only0xFF = 0;
	}

	if (only0xFF)
	{
		if (print_cb)
			print_cb(attr, "Not Present");
		else
			printf("Not Present\n");
		return;
	}
	if (only0x00)
	{
		if (print_cb)
			print_cb(attr, "Not Settable");
		else
			printf("Not Settable\n");
		return;
	}

	/*
	 * As of version 2.6 of the SMBIOS specification, the first 3
	 * fields of the UUID are supposed to be encoded on little-endian.
	 * The specification says that this is the defacto standard,
	 * however I've seen systems following RFC 4122 instead and use
	 * network byte order, so I am reluctant to apply the byte-swapping
	 * for older versions.
	 */
	if (ver >= 0x0206)
	{
		if (print_cb)
			print_cb(attr,
				"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
				p[3], p[2], p[1], p[0], p[5], p[4], p[7], p[6],
				p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
		else
			printf("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n",
				p[3], p[2], p[1], p[0], p[5], p[4], p[7], p[6],
				p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
	}
	else
	{
		if (print_cb)
			print_cb(attr,
				"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
				p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
				p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
		else
			printf("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n",
				p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
				p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
	}
}

static const char *dmi_system_wake_up_type(u8 code)
{
	/* 7.2.2 */
	static const char *type[] = {
		"Reserved", /* 0x00 */
		"Other",
		"Unknown",
		"APM Timer",
		"Modem Ring",
		"LAN Remote",
		"Power Switch",
		"PCI PME#",
		"AC Power Restored" /* 0x08 */
	};

	if (code <= 0x08)
		return type[code];
	return out_of_spec;
}

/*
 * 7.3 Base Board Information (Type 2)
 */

static void dmi_base_board_features(u8 code)
{
	/* 7.3.1 */
	static const char *features[] = {
		"Board is a hosting board", /* 0 */
		"Board requires at least one daughter board",
		"Board is removable",
		"Board is replaceable",
		"Board is hot swappable" /* 4 */
	};

	if ((code & 0x1F) == 0)
		pr_list_start("Features", "%s", "None");
	else
	{
		int i;

		pr_list_start("Features", NULL);
		for (i = 0; i <= 4; i++)
			if (code & (1 << i))
				pr_list_item("%s", features[i]);
	}
	pr_list_end();
}

static const char *dmi_base_board_type(u8 code)
{
	/* 7.3.2 */
	static const char *type[] = {
		"Unknown", /* 0x01 */
		"Other",
		"Server Blade",
		"Connectivity Switch",
		"System Management Module",
		"Processor Module",
		"I/O Module",
		"Memory Module",
		"Daughter Board",
		"Motherboard",
		"Processor+Memory Module",
		"Processor+I/O Module",
		"Interconnect Board" /* 0x0D */
	};

	if (code >= 0x01 && code <= 0x0D)
		return type[code - 0x01];
	return out_of_spec;
}

static void dmi_base_board_handles(u8 count, const u8 *p)
{
	int i;

	pr_list_start("Contained Object Handles", "%u", count);
	for (i = 0; i < count; i++)
		pr_list_item("0x%04X", WORD(p + sizeof(u16) * i));
	pr_list_end();
}

/*
 * 7.4 Chassis Information (Type 3)
 */

static const char *dmi_chassis_type(u8 code)
{
	/* 7.4.1 */
	static const char *type[] = {
		"Other", /* 0x01 */
		"Unknown",
		"Desktop",
		"Low Profile Desktop",
		"Pizza Box",
		"Mini Tower",
		"Tower",
		"Portable",
		"Laptop",
		"Notebook",
		"Hand Held",
		"Docking Station",
		"All In One",
		"Sub Notebook",
		"Space-saving",
		"Lunch Box",
		"Main Server Chassis", /* CIM_Chassis.ChassisPackageType says "Main System Chassis" */
		"Expansion Chassis",
		"Sub Chassis",
		"Bus Expansion Chassis",
		"Peripheral Chassis",
		"RAID Chassis",
		"Rack Mount Chassis",
		"Sealed-case PC",
		"Multi-system",
		"CompactPCI",
		"AdvancedTCA",
		"Blade",
		"Blade Enclosing",
		"Tablet",
		"Convertible",
		"Detachable",
		"IoT Gateway",
		"Embedded PC",
		"Mini PC",
		"Stick PC" /* 0x24 */
	};

	code &= 0x7F; /* bits 6:0 are chassis type, 7th bit is the lock bit */

	if (code >= 0x01 && code <= 0x24)
		return type[code - 0x01];
	return out_of_spec;
}

static const char *dmi_chassis_lock(u8 code)
{
	static const char *lock[] = {
		"Not Present", /* 0x00 */
		"Present" /* 0x01 */
	};

	return lock[code];
}

static const char *dmi_chassis_state(u8 code)
{
	/* 7.4.2 */
	static const char *state[] = {
		"Other", /* 0x01 */
		"Unknown",
		"Safe",
		"Warning",
		"Critical",
		"Non-recoverable" /* 0x06 */
	};

	if (code >= 0x01 && code <= 0x06)
		return state[code - 0x01];
	return out_of_spec;
}

static const char *dmi_chassis_security_status(u8 code)
{
	/* 7.4.3 */
	static const char *status[] = {
		"Other", /* 0x01 */
		"Unknown",
		"None",
		"External Interface Locked Out",
		"External Interface Enabled" /* 0x05 */
	};

	if (code >= 0x01 && code <= 0x05)
		return status[code - 0x01];
	return out_of_spec;
}

static void dmi_chassis_height(u8 code)
{
	if (code == 0x00)
		pr_attr("Height", "Unspecified");
	else
		pr_attr("Height", "%u U", code);
}

static void dmi_chassis_power_cords(u8 code)
{
	if (code == 0x00)
		pr_attr("Number Of Power Cords", "Unspecified");
	else
		pr_attr("Number Of Power Cords", "%u", code);
}

static void dmi_chassis_elements(u8 count, u8 len, const u8 *p)
{
	int i;

	pr_list_start("Contained Elements", "%u", count);
	for (i = 0; i < count; i++)
	{
		if (len >= 0x03)
		{
			const char *type;

			type = (p[i * len] & 0x80) ?
				dmi_smbios_structure_type(p[i * len] & 0x7F) :
				dmi_base_board_type(p[i * len] & 0x7F);

			if (p[1 + i * len] == p[2 + i * len])
				pr_list_item("%s (%u)", type, p[1 + i * len]);
			else
				pr_list_item("%s (%u-%u)", type, p[1 + i * len],
					     p[2 + i * len]);
		}
	}
	pr_list_end();
}

/*
 * 7.5 Processor Information (Type 4)
 */

static const char *dmi_processor_type(u8 code)
{
	/* 7.5.1 */
	static const char *type[] = {
		"Other", /* 0x01 */
		"Unknown",
		"Central Processor",
		"Math Processor",
		"DSP Processor",
		"Video Processor" /* 0x06 */
	};

	if (code >= 0x01 && code <= 0x06)
		return type[code - 0x01];
	return out_of_spec;
}

static const char *dmi_processor_family(const struct dmi_header *h, u16 ver)
{
	const u8 *data = h->data;
	unsigned int i, low, high;
	u16 code;

	/* 7.5.2 */
	static struct {
		int value;
		const char *name;
	} family2[] = {
		{ 0x01, "Other" },
		{ 0x02, "Unknown" },
		{ 0x03, "8086" },
		{ 0x04, "80286" },
		{ 0x05, "80386" },
		{ 0x06, "80486" },
		{ 0x07, "8087" },
		{ 0x08, "80287" },
		{ 0x09, "80387" },
		{ 0x0A, "80487" },
		{ 0x0B, "Pentium" },
		{ 0x0C, "Pentium Pro" },
		{ 0x0D, "Pentium II" },
		{ 0x0E, "Pentium MMX" },
		{ 0x0F, "Celeron" },
		{ 0x10, "Pentium II Xeon" },
		{ 0x11, "Pentium III" },
		{ 0x12, "M1" },
		{ 0x13, "M2" },
		{ 0x14, "Celeron M" },
		{ 0x15, "Pentium 4 HT" },
		{ 0x16, "Intel" },

		{ 0x18, "Duron" },
		{ 0x19, "K5" },
		{ 0x1A, "K6" },
		{ 0x1B, "K6-2" },
		{ 0x1C, "K6-3" },
		{ 0x1D, "Athlon" },
		{ 0x1E, "AMD29000" },
		{ 0x1F, "K6-2+" },
		{ 0x20, "Power PC" },
		{ 0x21, "Power PC 601" },
		{ 0x22, "Power PC 603" },
		{ 0x23, "Power PC 603+" },
		{ 0x24, "Power PC 604" },
		{ 0x25, "Power PC 620" },
		{ 0x26, "Power PC x704" },
		{ 0x27, "Power PC 750" },
		{ 0x28, "Core Duo" },
		{ 0x29, "Core Duo Mobile" },
		{ 0x2A, "Core Solo Mobile" },
		{ 0x2B, "Atom" },
		{ 0x2C, "Core M" },
		{ 0x2D, "Core m3" },
		{ 0x2E, "Core m5" },
		{ 0x2F, "Core m7" },
		{ 0x30, "Alpha" },
		{ 0x31, "Alpha 21064" },
		{ 0x32, "Alpha 21066" },
		{ 0x33, "Alpha 21164" },
		{ 0x34, "Alpha 21164PC" },
		{ 0x35, "Alpha 21164a" },
		{ 0x36, "Alpha 21264" },
		{ 0x37, "Alpha 21364" },
		{ 0x38, "Turion II Ultra Dual-Core Mobile M" },
		{ 0x39, "Turion II Dual-Core Mobile M" },
		{ 0x3A, "Athlon II Dual-Core M" },
		{ 0x3B, "Opteron 6100" },
		{ 0x3C, "Opteron 4100" },
		{ 0x3D, "Opteron 6200" },
		{ 0x3E, "Opteron 4200" },
		{ 0x3F, "FX" },
		{ 0x40, "MIPS" },
		{ 0x41, "MIPS R4000" },
		{ 0x42, "MIPS R4200" },
		{ 0x43, "MIPS R4400" },
		{ 0x44, "MIPS R4600" },
		{ 0x45, "MIPS R10000" },
		{ 0x46, "C-Series" },
		{ 0x47, "E-Series" },
		{ 0x48, "A-Series" },
		{ 0x49, "G-Series" },
		{ 0x4A, "Z-Series" },
		{ 0x4B, "R-Series" },
		{ 0x4C, "Opteron 4300" },
		{ 0x4D, "Opteron 6300" },
		{ 0x4E, "Opteron 3300" },
		{ 0x4F, "FirePro" },
		{ 0x50, "SPARC" },
		{ 0x51, "SuperSPARC" },
		{ 0x52, "MicroSPARC II" },
		{ 0x53, "MicroSPARC IIep" },
		{ 0x54, "UltraSPARC" },
		{ 0x55, "UltraSPARC II" },
		{ 0x56, "UltraSPARC IIi" },
		{ 0x57, "UltraSPARC III" },
		{ 0x58, "UltraSPARC IIIi" },

		{ 0x60, "68040" },
		{ 0x61, "68xxx" },
		{ 0x62, "68000" },
		{ 0x63, "68010" },
		{ 0x64, "68020" },
		{ 0x65, "68030" },
		{ 0x66, "Athlon X4" },
		{ 0x67, "Opteron X1000" },
		{ 0x68, "Opteron X2000" },
		{ 0x69, "Opteron A-Series" },
		{ 0x6A, "Opteron X3000" },
		{ 0x6B, "Zen" },

		{ 0x70, "Hobbit" },

		{ 0x78, "Crusoe TM5000" },
		{ 0x79, "Crusoe TM3000" },
		{ 0x7A, "Efficeon TM8000" },

		{ 0x80, "Weitek" },

		{ 0x82, "Itanium" },
		{ 0x83, "Athlon 64" },
		{ 0x84, "Opteron" },
		{ 0x85, "Sempron" },
		{ 0x86, "Turion 64" },
		{ 0x87, "Dual-Core Opteron" },
		{ 0x88, "Athlon 64 X2" },
		{ 0x89, "Turion 64 X2" },
		{ 0x8A, "Quad-Core Opteron" },
		{ 0x8B, "Third-Generation Opteron" },
		{ 0x8C, "Phenom FX" },
		{ 0x8D, "Phenom X4" },
		{ 0x8E, "Phenom X2" },
		{ 0x8F, "Athlon X2" },
		{ 0x90, "PA-RISC" },
		{ 0x91, "PA-RISC 8500" },
		{ 0x92, "PA-RISC 8000" },
		{ 0x93, "PA-RISC 7300LC" },
		{ 0x94, "PA-RISC 7200" },
		{ 0x95, "PA-RISC 7100LC" },
		{ 0x96, "PA-RISC 7100" },

		{ 0xA0, "V30" },
		{ 0xA1, "Quad-Core Xeon 3200" },
		{ 0xA2, "Dual-Core Xeon 3000" },
		{ 0xA3, "Quad-Core Xeon 5300" },
		{ 0xA4, "Dual-Core Xeon 5100" },
		{ 0xA5, "Dual-Core Xeon 5000" },
		{ 0xA6, "Dual-Core Xeon LV" },
		{ 0xA7, "Dual-Core Xeon ULV" },
		{ 0xA8, "Dual-Core Xeon 7100" },
		{ 0xA9, "Quad-Core Xeon 5400" },
		{ 0xAA, "Quad-Core Xeon" },
		{ 0xAB, "Dual-Core Xeon 5200" },
		{ 0xAC, "Dual-Core Xeon 7200" },
		{ 0xAD, "Quad-Core Xeon 7300" },
		{ 0xAE, "Quad-Core Xeon 7400" },
		{ 0xAF, "Multi-Core Xeon 7400" },
		{ 0xB0, "Pentium III Xeon" },
		{ 0xB1, "Pentium III Speedstep" },
		{ 0xB2, "Pentium 4" },
		{ 0xB3, "Xeon" },
		{ 0xB4, "AS400" },
		{ 0xB5, "Xeon MP" },
		{ 0xB6, "Athlon XP" },
		{ 0xB7, "Athlon MP" },
		{ 0xB8, "Itanium 2" },
		{ 0xB9, "Pentium M" },
		{ 0xBA, "Celeron D" },
		{ 0xBB, "Pentium D" },
		{ 0xBC, "Pentium EE" },
		{ 0xBD, "Core Solo" },
		/* 0xBE handled as a special case */
		{ 0xBF, "Core 2 Duo" },
		{ 0xC0, "Core 2 Solo" },
		{ 0xC1, "Core 2 Extreme" },
		{ 0xC2, "Core 2 Quad" },
		{ 0xC3, "Core 2 Extreme Mobile" },
		{ 0xC4, "Core 2 Duo Mobile" },
		{ 0xC5, "Core 2 Solo Mobile" },
		{ 0xC6, "Core i7" },
		{ 0xC7, "Dual-Core Celeron" },
		{ 0xC8, "IBM390" },
		{ 0xC9, "G4" },
		{ 0xCA, "G5" },
		{ 0xCB, "ESA/390 G6" },
		{ 0xCC, "z/Architecture" },
		{ 0xCD, "Core i5" },
		{ 0xCE, "Core i3" },
		{ 0xCF, "Core i9" },

		{ 0xD2, "C7-M" },
		{ 0xD3, "C7-D" },
		{ 0xD4, "C7" },
		{ 0xD5, "Eden" },
		{ 0xD6, "Multi-Core Xeon" },
		{ 0xD7, "Dual-Core Xeon 3xxx" },
		{ 0xD8, "Quad-Core Xeon 3xxx" },
		{ 0xD9, "Nano" },
		{ 0xDA, "Dual-Core Xeon 5xxx" },
		{ 0xDB, "Quad-Core Xeon 5xxx" },

		{ 0xDD, "Dual-Core Xeon 7xxx" },
		{ 0xDE, "Quad-Core Xeon 7xxx" },
		{ 0xDF, "Multi-Core Xeon 7xxx" },
		{ 0xE0, "Multi-Core Xeon 3400" },

		{ 0xE4, "Opteron 3000" },
		{ 0xE5, "Sempron II" },
		{ 0xE6, "Embedded Opteron Quad-Core" },
		{ 0xE7, "Phenom Triple-Core" },
		{ 0xE8, "Turion Ultra Dual-Core Mobile" },
		{ 0xE9, "Turion Dual-Core Mobile" },
		{ 0xEA, "Athlon Dual-Core" },
		{ 0xEB, "Sempron SI" },
		{ 0xEC, "Phenom II" },
		{ 0xED, "Athlon II" },
		{ 0xEE, "Six-Core Opteron" },
		{ 0xEF, "Sempron M" },

		{ 0xFA, "i860" },
		{ 0xFB, "i960" },

		{ 0x100, "ARMv7" },
		{ 0x101, "ARMv8" },
		{ 0x102, "ARMv9" },
		{ 0x103, "ARM" },
		{ 0x104, "SH-3" },
		{ 0x105, "SH-4" },
		{ 0x118, "ARM" },
		{ 0x119, "StrongARM" },
		{ 0x12C, "6x86" },
		{ 0x12D, "MediaGX" },
		{ 0x12E, "MII" },
		{ 0x140, "WinChip" },
		{ 0x15E, "DSP" },
		{ 0x1F4, "Video Processor" },

		{ 0x200, "RV32" },
		{ 0x201, "RV64" },
		{ 0x202, "RV128" },

		{ 0x258, "LoongArch" },
		{ 0x259, "Loongson 1" },
		{ 0x25A, "Loongson 2" },
		{ 0x25B, "Loongson 3" },
		{ 0x25C, "Loongson 2K" },
		{ 0x25D, "Loongson 3A" },
		{ 0x25E, "Loongson 3B" },
		{ 0x25F, "Loongson 3C" },
		{ 0x260, "Loongson 3D" },
		{ 0x261, "Loongson 3E" },
		{ 0x262, "Dual-Core Loongson 2K 2xxx" },
		{ 0x26C, "Quad-Core Loongson 3A 5xxx" },
		{ 0x26D, "Multi-Core Loongson 3A 5xxx" },
		{ 0x26E, "Quad-Core Loongson 3B 5xxx" },
		{ 0x26F, "Multi-Core Loongson 3B 5xxx" },
		{ 0x270, "Multi-Core Loongson 3C 5xxx" },
		{ 0x271, "Multi-Core Loongson 3D 5xxx" },
	};
	/*
	 * Note to developers: when adding entries to this list, check if
	 * function dmi_processor_id below needs updating too.
	 */

	/* Special case for ambiguous value 0x30 (SMBIOS 2.0 only) */
	if (ver == 0x0200 && data[0x06] == 0x30 && h->length >= 0x08)
	{
		const char *manufacturer = dmi_string(h, data[0x07]);

		if (strstr(manufacturer, "Intel") != NULL
		 || strncasecmp(manufacturer, "Intel", 5) == 0)
			return "Pentium Pro";
	}

	code = (data[0x06] == 0xFE && h->length >= 0x2A) ?
		WORD(data + 0x28) : data[0x06];

	/* Special case for ambiguous value 0xBE */
	if (code == 0xBE)
	{
		if (h->length >= 0x08)
		{
			const char *manufacturer = dmi_string(h, data[0x07]);

			/* Best bet based on manufacturer string */
			if (strstr(manufacturer, "Intel") != NULL
			 || strncasecmp(manufacturer, "Intel", 5) == 0)
				return "Core 2";
			if (strstr(manufacturer, "AMD") != NULL
			 || strncasecmp(manufacturer, "AMD", 3) == 0)
				return "K7";
		}

		return "Core 2 or K7";
	}

	/* Perform a binary search */
	low = 0;
	high = ARRAY_SIZE(family2) - 1;

	while (1)
	{
		i = (low + high) / 2;
		if (family2[i].value == code)
			return family2[i].name;
		if (low == high) /* Not found */
			return out_of_spec;

		if (code < family2[i].value)
			high = i;
		else
			low = i + 1;
	}
}

static enum cpuid_type dmi_get_cpuid_type(const struct dmi_header *h)
{
	const u8 *data = h->data;
	const u8 *p = data + 0x08;
	u16 type;

	type = (data[0x06] == 0xFE && h->length >= 0x2A) ?
		WORD(data + 0x28) : data[0x06];

	if (type == 0x05) /* 80386 */
	{
		return cpuid_80386;
	}
	else if (type == 0x06) /* 80486 */
	{
		u16 dx = WORD(p);
		/*
		 * Not all 80486 CPU support the CPUID instruction, we have to find
		 * whether the one we have here does or not. Note that this trick
		 * works only because we know that 80486 must be little-endian.
		 */
		if ((dx & 0x0F00) == 0x0400
		 && ((dx & 0x00F0) == 0x0040 || (dx & 0x00F0) >= 0x0070)
		 && ((dx & 0x000F) >= 0x0003))
			return cpuid_x86_intel;
		else
			return cpuid_80486;
	}
	else if ((type >= 0x100 && type <= 0x102) /* ARM */
	      || (type >= 0x118 && type <= 0x119)) /* ARM */
	{
		/*
		 * The field's format depends on the processor's support of
		 * the SMCCC_ARCH_SOC_ID architectural call. Software can determine
		 * the support for SoC ID by examining the Processor Characteristics field
		 * for "Arm64 SoC ID" bit.
		 */
		if (h->length >= 0x28
		 && (WORD(data + 0x26) & (1 << 9)))
			return cpuid_arm_soc_id;
		else
			return cpuid_arm_legacy;
	}
	else if ((type >= 0x0B && type <= 0x15) /* Intel, Cyrix */
	      || (type >= 0x28 && type <= 0x2F) /* Intel */
	      || (type >= 0xA1 && type <= 0xB3) /* Intel */
	      || type == 0xB5 /* Intel */
	      || (type >= 0xB9 && type <= 0xC7) /* Intel */
	      || (type >= 0xCD && type <= 0xCF) /* Intel */
	      || (type >= 0xD2 && type <= 0xDB) /* VIA, Intel */
	      || (type >= 0xDD && type <= 0xE0)) /* Intel */
		return cpuid_x86_intel;
	else if ((type >= 0x18 && type <= 0x1D) /* AMD */
	      || type == 0x1F /* AMD */
	      || (type >= 0x38 && type <= 0x3F) /* AMD */
	      || (type >= 0x46 && type <= 0x4F) /* AMD */
	      || (type >= 0x66 && type <= 0x6B) /* AMD */
	      || (type >= 0x83 && type <= 0x8F) /* AMD */
	      || (type >= 0xB6 && type <= 0xB7) /* AMD */
	      || (type >= 0xE4 && type <= 0xEF)) /* AMD */
		return cpuid_x86_amd;
	else if ((type >= 0x258 && type <= 0x262) /* Loongarch */
	      || (type >= 0x26C && type <= 0x271)) /* Loongarch */
		return cpuid_loongarch;

	/* neither X86 nor ARM */
	return cpuid_none;
}

void dmi_print_cpuid(void (*print_cb)(const char *name, const char *format, ...),
		     const char *label, enum cpuid_type sig, const u8 *p)
{
	u32 eax, midr, jep106, soc_revision;
	u16 dx;

	switch (sig)
	{
		case cpuid_80386:
			dx = WORD(p);
			/*
			 * 80386 have a different signature.
			 */
			print_cb(label,
				 "Type %u, Family %u, Major Stepping %u, Minor Stepping %u",
				 dx >> 12, (dx >> 8) & 0xF,
				 (dx >> 4) & 0xF, dx & 0xF);
			return;

		case cpuid_80486:
			dx = WORD(p);
			print_cb(label,
				 "Type %u, Family %u, Model %u, Stepping %u",
				 (dx >> 12) & 0x3, (dx >> 8) & 0xF,
				 (dx >> 4) & 0xF, dx & 0xF);
			return;

		case cpuid_arm_legacy: /* ARM before SOC ID */
			midr = DWORD(p);
			/*
			 * The format of this field was not defined for ARM processors
			 * before version 3.1.0 of the SMBIOS specification, so we
			 * silently skip it if it reads all zeroes.
			 */
			if (midr == 0)
				return;
			print_cb(label,
				 "Implementor 0x%02x, Variant 0x%x, Architecture %u, Part 0x%03x, Revision %u",
				 midr >> 24, (midr >> 20) & 0xF,
				 (midr >> 16) & 0xF, (midr >> 4) & 0xFFF, midr & 0xF);
			return;

		case cpuid_arm_soc_id: /* ARM with SOC ID */
			/*
			 * If Soc ID is supported, the first DWORD is the JEP-106 code;
			 * the second DWORD is the SoC revision value.
			 */
			jep106 = DWORD(p);
			soc_revision = DWORD(p + 4);
			/*
			 * According to SMC Calling Convention (SMCCC) v1.3 specification
			 * (https://developer.arm.com/documentation/den0028/d/), the format
			 * of the values returned by the SMCCC_ARCH_SOC_ID call is as follows:
			 *
			 * JEP-106 code for the SiP (SoC_ID_type == 0)
			 *   Bit[31] must be zero
			 *   Bits[30:24] JEP-106 bank index for the SiP
			 *   Bits[23:16] JEP-106 identification code with parity bit for the SiP
			 *   Bits[15:0] Implementation defined SoC ID
			 *
			 * SoC revision (SoC_ID_type == 1)
			 *   Bit[31] must be zero
			 *   Bits[30:0] SoC revision
			 */
			pr_attr("Signature",
				"JEP-106 Bank 0x%02x Manufacturer 0x%02x, SoC ID 0x%04x, SoC Revision 0x%08x",
				(jep106 >> 24) & 0x7F, (jep106 >> 16) & 0x7F, jep106 & 0xFFFF, soc_revision);
			return;

		case cpuid_x86_intel: /* Intel */
			eax = DWORD(p);
			/*
			 * Extra flags are now returned in the ECX register when
			 * one calls the CPUID instruction. Their meaning is
			 * explained in table 3-5, but DMI doesn't support this
			 * yet.
			 */
			print_cb(label,
				 "Type %u, Family %u, Model %u, Stepping %u",
				 (eax >> 12) & 0x3,
				 ((eax >> 20) & 0xFF) + ((eax >> 8) & 0x0F),
				 ((eax >> 12) & 0xF0) + ((eax >> 4) & 0x0F),
				 eax & 0xF);
			break;

		case cpuid_x86_amd: /* AMD, publication #25481 revision 2.28 */
			eax = DWORD(p);
			print_cb(label, "Family %u, Model %u, Stepping %u",
				 ((eax >> 8) & 0xF) + (((eax >> 8) & 0xF) == 0xF ? (eax >> 20) & 0xFF : 0),
				 ((eax >> 4) & 0xF) | (((eax >> 8) & 0xF) == 0xF ? (eax >> 12) & 0xF0 : 0),
				 eax & 0xF);
			break;

		case cpuid_loongarch: /* LoongArch Reference Manual, volume 1 */
			eax = DWORD(p);
			print_cb(label, "Processor Identity 0x%08x\n", eax);
			break;
		default:
			return;
	}
}

static void dmi_processor_id(const struct dmi_header *h)
{
	/* Intel AP-485 revision 36, table 2-4 */
	static const char *flags[32] = {
		"FPU (Floating-point unit on-chip)", /* 0 */
		"VME (Virtual mode extension)",
		"DE (Debugging extension)",
		"PSE (Page size extension)",
		"TSC (Time stamp counter)",
		"MSR (Model specific registers)",
		"PAE (Physical address extension)",
		"MCE (Machine check exception)",
		"CX8 (CMPXCHG8 instruction supported)",
		"APIC (On-chip APIC hardware supported)",
		NULL, /* 10 */
		"SEP (Fast system call)",
		"MTRR (Memory type range registers)",
		"PGE (Page global enable)",
		"MCA (Machine check architecture)",
		"CMOV (Conditional move instruction supported)",
		"PAT (Page attribute table)",
		"PSE-36 (36-bit page size extension)",
		"PSN (Processor serial number present and enabled)",
		"CLFSH (CLFLUSH instruction supported)",
		NULL, /* 20 */
		"DS (Debug store)",
		"ACPI (ACPI supported)",
		"MMX (MMX technology supported)",
		"FXSR (FXSAVE and FXSTOR instructions supported)",
		"SSE (Streaming SIMD extensions)",
		"SSE2 (Streaming SIMD extensions 2)",
		"SS (Self-snoop)",
		"HTT (Multi-threading)",
		"TM (Thermal monitor supported)",
		NULL, /* 30 */
		"PBE (Pending break enabled)" /* 31 */
	};
	const u8 *data = h->data;
	const u8 *p = data + 0x08;
	enum cpuid_type sig = dmi_get_cpuid_type(h);
	u32 edx;

	/*
	 * This might help learn about new processors supporting the
	 * CPUID instruction or another form of identification.
	 */
	if (!(opt.flags & FLAG_QUIET))
		pr_attr("ID", "%02X %02X %02X %02X %02X %02X %02X %02X",
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);

	dmi_print_cpuid(pr_attr, "Signature", sig, p);

	if (sig != cpuid_x86_intel && sig != cpuid_x86_amd)
		return;

	edx = DWORD(p + 4);
	if ((edx & 0xBFEFFBFF) == 0)
		pr_list_start("Flags", "None");
	else
	{
		int i;

		pr_list_start("Flags", NULL);
		for (i = 0; i <= 31; i++)
			if (flags[i] != NULL && edx & (1 << i))
				pr_list_item("%s", flags[i]);
	}
	pr_list_end();
}

static void dmi_processor_voltage(const char *attr, u8 code)
{
	/* 7.5.4 */
	static const char *voltage[] = {
		"5.0 V", /* 0 */
		"3.3 V",
		"2.9 V" /* 2 */
	};
	int i;

	if (code & 0x80)
		pr_attr(attr, "%.1f V", (float)(code & 0x7f) / 10);
	else if ((code & 0x07) == 0x00)
		pr_attr(attr, "Unknown");
	else
	{
		char voltage_str[18];
		int off = 0;

		for (i = 0; i <= 2; i++)
		{
			if (code & (1 << i))
			{
				/* Insert space if not the first value */
				off += sprintf(voltage_str + off,
					       off ? " %s" :"%s",
					       voltage[i]);
			}
		}
		if (off)
			pr_attr(attr, voltage_str);
	}
}

static void dmi_processor_frequency(const char *attr, const u8 *p)
{
	u16 code = WORD(p);

	if (code)
	{
		if (attr)
			pr_attr(attr, "%u MHz", code);
		else
			printf("%u MHz\n", code);
	}
	else
	{
		if (attr)
			pr_attr(attr, "Unknown");
		else
			printf("Unknown\n");
	}
}

/* code is assumed to be a 3-bit value */
static const char *dmi_processor_status(u8 code)
{
	static const char *status[] = {
		"Unknown", /* 0x00 */
		"Enabled",
		"Disabled By User",
		"Disabled By BIOS",
		"Idle", /* 0x04 */
		out_of_spec,
		out_of_spec,
		"Other" /* 0x07 */
	};

	return status[code];
}

static const char *dmi_processor_upgrade(u8 code)
{
	/* 7.5.5 */
	static const char *upgrade[] = {
		"Other", /* 0x01 */
		"Unknown",
		"Daughter Board",
		"ZIF Socket",
		"Replaceable Piggy Back",
		"None",
		"LIF Socket",
		"Slot 1",
		"Slot 2",
		"370-pin Socket",
		"Slot A",
		"Slot M",
		"Socket 423",
		"Socket A (Socket 462)",
		"Socket 478",
		"Socket 754",
		"Socket 940",
		"Socket 939",
		"Socket mPGA604",
		"Socket LGA771",
		"Socket LGA775",
		"Socket S1",
		"Socket AM2",
		"Socket F (1207)",
		"Socket LGA1366",
		"Socket G34",
		"Socket AM3",
		"Socket C32",
		"Socket LGA1156",
		"Socket LGA1567",
		"Socket PGA988A",
		"Socket BGA1288",
		"Socket rPGA988B",
		"Socket BGA1023",
		"Socket BGA1224",
		"Socket BGA1155",
		"Socket LGA1356",
		"Socket LGA2011",
		"Socket FS1",
		"Socket FS2",
		"Socket FM1",
		"Socket FM2",
		"Socket LGA2011-3",
		"Socket LGA1356-3",
		"Socket LGA1150",
		"Socket BGA1168",
		"Socket BGA1234",
		"Socket BGA1364",
		"Socket AM4",
		"Socket LGA1151",
		"Socket BGA1356",
		"Socket BGA1440",
		"Socket BGA1515",
		"Socket LGA3647-1",
		"Socket SP3",
		"Socket SP3r2",
		"Socket LGA2066",
		"Socket BGA1392",
		"Socket BGA1510",
		"Socket BGA1528",
		"Socket LGA4189",
		"Socket LGA1200",
		"Socket LGA4677",
		"Socket LGA1700",
		"Socket BGA1744",
		"Socket BGA1781",
		"Socket BGA1211",
		"Socket BGA2422",
		"Socket LGA1211",
		"Socket LGA2422",
		"Socket LGA5773",
		"Socket BGA5773",
		"Socket AM5",
		"Socket SP5",
		"Socket SP6",
		"Socket BGA883",
		"Socket BGA1190",
		"Socket BGA4129",
		"Socket LGA4710",
		"Socket LGA7529" /* 0x50 */
	};

	if (code >= 0x01 && code <= 0x50)
		return upgrade[code - 0x01];
	return out_of_spec;
}

static void dmi_processor_cache(const char *attr, u16 code, const char *level,
				u16 ver)
{
	if (code == 0xFFFF)
	{
		if (ver >= 0x0203)
			pr_attr(attr, "Not Provided");
		else
			pr_attr(attr, "No %s Cache", level);
	}
	else
		pr_attr(attr, "0x%04X", code);
}

static void dmi_processor_characteristics(const char *attr, u16 code)
{
	/* 7.5.9 */
	static const char *characteristics[] = {
		"64-bit capable", /* 2 */
		"Multi-Core",
		"Hardware Thread",
		"Execute Protection",
		"Enhanced Virtualization",
		"Power/Performance Control",
		"128-bit Capable",
		"Arm64 SoC ID" /* 9 */
	};

	if ((code & 0x00FC) == 0)
		pr_attr(attr, "None");
	else
	{
		int i;

		pr_list_start(attr, NULL);
		for (i = 2; i <= 9; i++)
			if (code & (1 << i))
				pr_list_item("%s", characteristics[i - 2]);
		pr_list_end();
	}
}

/*
 * 7.6 Memory Controller Information (Type 5)
 */

static const char *dmi_memory_controller_ed_method(u8 code)
{
	/* 7.6.1 */
	static const char *method[] = {
		"Other", /* 0x01 */
		"Unknown",
		"None",
		"8-bit Parity",
		"32-bit ECC",
		"64-bit ECC",
		"128-bit ECC",
		"CRC" /* 0x08 */
	};

	if (code >= 0x01 && code <= 0x08)
		return method[code - 0x01];
	return out_of_spec;
}

static void dmi_memory_controller_ec_capabilities(const char *attr, u8 code)
{
	/* 7.6.2 */
	static const char *capabilities[] = {
		"Other", /* 0 */
		"Unknown",
		"None",
		"Single-bit Error Correcting",
		"Double-bit Error Correcting",
		"Error Scrubbing" /* 5 */
	};

	if ((code & 0x3F) == 0)
		pr_attr(attr, "None");
	else
	{
		int i;

		pr_list_start(attr, NULL);
		for (i = 0; i <= 5; i++)
			if (code & (1 << i))
				pr_list_item("%s", capabilities[i]);
		pr_list_end();
	}
}

static const char *dmi_memory_controller_interleave(u8 code)
{
	/* 7.6.3 */
	static const char *interleave[] = {
		"Other", /* 0x01 */
		"Unknown",
		"One-way Interleave",
		"Two-way Interleave",
		"Four-way Interleave",
		"Eight-way Interleave",
		"Sixteen-way Interleave" /* 0x07 */
	};

	if (code >= 0x01 && code <= 0x07)
		return interleave[code - 0x01];
	return out_of_spec;
}

static void dmi_memory_controller_speeds(const char *attr, u16 code)
{
	/* 7.6.4 */
	const char *speeds[] = {
		"Other", /* 0 */
		"Unknown",
		"70 ns",
		"60 ns",
		"50 ns" /* 4 */
	};

	if ((code & 0x001F) == 0)
		pr_attr(attr, "None");
	else
	{
		int i;

		pr_list_start(attr, NULL);
		for (i = 0; i <= 4; i++)
			if (code & (1 << i))
				pr_list_item("%s", speeds[i]);
		pr_list_end();
	}
}

static void dmi_memory_controller_slots(u8 count, const u8 *p)
{
	int i;

	pr_list_start("Associated Memory Slots", "%u", count);
	for (i = 0; i < count; i++)
		pr_list_item("0x%04X", WORD(p + sizeof(u16) * i));
	pr_list_end();
}

/*
 * 7.7 Memory Module Information (Type 6)
 */

static void dmi_memory_module_types(const char *attr, u16 code, int flat)
{
	/* 7.7.1 */
	static const char *types[] = {
		"Other", /* 0 */
		"Unknown",
		"Standard",
		"FPM",
		"EDO",
		"Parity",
		"ECC",
		"SIMM",
		"DIMM",
		"Burst EDO",
		"SDRAM" /* 10 */
	};

	if ((code & 0x07FF) == 0)
		pr_attr(attr, "None");
	else if (flat)
	{
		char type_str[68];
		int i, off = 0;

		for (i = 0; i <= 10; i++)
		{
			if (code & (1 << i))
			{
				/* Insert space if not the first value */
				off += sprintf(type_str + off,
					       off ? " %s" :"%s",
					       types[i]);
			}
		}
		if (off)
			pr_attr(attr, type_str);
	}
	else
	{
		int i;

		pr_list_start(attr, NULL);
		for (i = 0; i <= 10; i++)
			if (code & (1 << i))
				pr_list_item("%s", types[i]);
		pr_list_end();
	}
}

static void dmi_memory_module_connections(u8 code)
{
	if (code == 0xFF)
		pr_attr("Bank Connections", "None");
	else if ((code & 0xF0) == 0xF0)
		pr_attr("Bank Connections", "%u", code & 0x0F);
	else if ((code & 0x0F) == 0x0F)
		pr_attr("Bank Connections", "%u", code >> 4);
	else
		pr_attr("Bank Connections", "%u %u", code >> 4, code & 0x0F);
}

static void dmi_memory_module_speed(const char *attr, u8 code)
{
	if (code == 0)
		pr_attr(attr, "Unknown");
	else
		pr_attr(attr, "%u ns", code);
}

static void dmi_memory_module_size(const char *attr, u8 code)
{
	const char *connection;

	/* 7.7.2 */
	if (code & 0x80)
		connection = " (Double-bank Connection)";
	else
		connection = " (Single-bank Connection)";

	switch (code & 0x7F)
	{
		case 0x7D:
			pr_attr(attr, "Not Determinable%s", connection);
			break;
		case 0x7E:
			pr_attr(attr, "Disabled%s", connection);
			break;
		case 0x7F:
			pr_attr(attr, "Not Installed");
			return;
		default:
			pr_attr(attr, "%u MB%s", 1 << (code & 0x7F),
				connection);
	}
}

static void dmi_memory_module_error(u8 code)
{
	static const char *status[] = {
		"OK", /* 0x00 */
		"Uncorrectable Errors",
		"Correctable Errors",
		"Correctable and Uncorrectable Errors" /* 0x03 */
	};

	if (code & (1 << 2))
		pr_attr("Error Status", "See Event Log");
	else
		pr_attr("Error Status", "%s", status[code & 0x03]);
}

/*
 * 7.8 Cache Information (Type 7)
 */

static const char *dmi_cache_mode(u8 code)
{
	static const char *mode[] = {
		"Write Through", /* 0x00 */
		"Write Back",
		"Varies With Memory Address",
		"Unknown" /* 0x03 */
	};

	return mode[code];
}

/* code is assumed to be a 2-bit value */
static const char *dmi_cache_location(u8 code)
{
	static const char *location[4] = {
		"Internal", /* 0x00 */
		"External",
		out_of_spec, /* 0x02 */
		"Unknown" /* 0x03 */
	};

	return location[code];
}

static void dmi_cache_size_2(const char *attr, u32 code)
{
	u64 size;

	if (code & 0x80000000)
	{
		code &= 0x7FFFFFFFLU;
		size.l = code << 6;
		size.h = code >> 26;
	}
	else
	{
		size.l = code;
		size.h = 0;
	}

	/* Use a more convenient unit for large cache size */
	dmi_print_memory_size(attr, size, 1);
}

static void dmi_cache_size(const char *attr, u16 code)
{
	dmi_cache_size_2(attr,
			 (((u32)code & 0x8000LU) << 16) | (code & 0x7FFFLU));
}

static void dmi_cache_types(const char *attr, u16 code, int flat)
{
	/* 7.8.2 */
	static const char *types[] = {
		"Other", /* 0 */
		"Unknown",
		"Non-burst",
		"Burst",
		"Pipeline Burst",
		"Synchronous",
		"Asynchronous" /* 6 */
	};

	if ((code & 0x007F) == 0)
		pr_attr(attr, "None");
	else if (flat)
	{
		char type_str[70];
		int i, off = 0;

		for (i = 0; i <= 6; i++)
		{
			if (code & (1 << i))
			{
				/* Insert space if not the first value */
				off += sprintf(type_str + off,
					       off ? " %s" :"%s",
					       types[i]);
			}
		}
		if (off)
			pr_attr(attr, type_str);
	}
	else
	{
		int i;

		pr_list_start(attr, NULL);
		for (i = 0; i <= 6; i++)
			if (code & (1 << i))
				pr_list_item("%s", types[i]);
		pr_list_end();
	}
}

static const char *dmi_cache_ec_type(u8 code)
{
	/* 7.8.3 */
	static const char *type[] = {
		"Other", /* 0x01 */
		"Unknown",
		"None",
		"Parity",
		"Single-bit ECC",
		"Multi-bit ECC" /* 0x06 */
	};

	if (code >= 0x01 && code <= 0x06)
		return type[code - 0x01];
	return out_of_spec;
}

static const char *dmi_cache_type(u8 code)
{
	/* 7.8.4 */
	static const char *type[] = {
		"Other", /* 0x01 */
		"Unknown",
		"Instruction",
		"Data",
		"Unified" /* 0x05 */
	};

	if (code >= 0x01 && code <= 0x05)
		return type[code - 0x01];
	return out_of_spec;
}

static const char *dmi_cache_associativity(u8 code)
{
	/* 7.8.5 */
	static const char *type[] = {
		"Other", /* 0x01 */
		"Unknown",
		"Direct Mapped",
		"2-way Set-associative",
		"4-way Set-associative",
		"Fully Associative",
		"8-way Set-associative",
		"16-way Set-associative",
		"12-way Set-associative",
		"24-way Set-associative",
		"32-way Set-associative",
		"48-way Set-associative",
		"64-way Set-associative",
		"20-way Set-associative" /* 0x0E */
	};

	if (code >= 0x01 && code <= 0x0E)
		return type[code - 0x01];
	return out_of_spec;
}

/*
 * 7.9 Port Connector Information (Type 8)
 */

static const char *dmi_port_connector_type(u8 code)
{
	/* 7.9.2 */
	static const char *type[] = {
		"None", /* 0x00 */
		"Centronics",
		"Mini Centronics",
		"Proprietary",
		"DB-25 male",
		"DB-25 female",
		"DB-15 male",
		"DB-15 female",
		"DB-9 male",
		"DB-9 female",
		"RJ-11",
		"RJ-45",
		"50 Pin MiniSCSI",
		"Mini DIN",
		"Micro DIN",
		"PS/2",
		"Infrared",
		"HP-HIL",
		"Access Bus (USB)",
		"SSA SCSI",
		"Circular DIN-8 male",
		"Circular DIN-8 female",
		"On Board IDE",
		"On Board Floppy",
		"9 Pin Dual Inline (pin 10 cut)",
		"25 Pin Dual Inline (pin 26 cut)",
		"50 Pin Dual Inline",
		"68 Pin Dual Inline",
		"On Board Sound Input From CD-ROM",
		"Mini Centronics Type-14",
		"Mini Centronics Type-26",
		"Mini Jack (headphones)",
		"BNC",
		"IEEE 1394",
		"SAS/SATA Plug Receptacle",
		"USB Type-C Receptacle" /* 0x23 */
	};
	static const char *type_0xA0[] = {
		"PC-98", /* 0xA0 */
		"PC-98 Hireso",
		"PC-H98",
		"PC-98 Note",
		"PC-98 Full" /* 0xA4 */
	};

	if (code <= 0x23)
		return type[code];
	if (code >= 0xA0 && code <= 0xA4)
		return type_0xA0[code - 0xA0];
	if (code == 0xFF)
		return "Other";
	return out_of_spec;
}

static const char *dmi_port_type(u8 code)
{
	/* 7.9.3 */
	static const char *type[] = {
		"None", /* 0x00 */
		"Parallel Port XT/AT Compatible",
		"Parallel Port PS/2",
		"Parallel Port ECP",
		"Parallel Port EPP",
		"Parallel Port ECP/EPP",
		"Serial Port XT/AT Compatible",
		"Serial Port 16450 Compatible",
		"Serial Port 16550 Compatible",
		"Serial Port 16550A Compatible",
		"SCSI Port",
		"MIDI Port",
		"Joystick Port",
		"Keyboard Port",
		"Mouse Port",
		"SSA SCSI",
		"USB",
		"Firewire (IEEE P1394)",
		"PCMCIA Type I",
		"PCMCIA Type II",
		"PCMCIA Type III",
		"Cardbus",
		"Access Bus Port",
		"SCSI II",
		"SCSI Wide",
		"PC-98",
		"PC-98 Hireso",
		"PC-H98",
		"Video Port",
		"Audio Port",
		"Modem Port",
		"Network Port",
		"SATA",
		"SAS",
		"MFDP (Multi-Function Display Port)",
		"Thunderbolt" /* 0x23 */
	};
	static const char *type_0xA0[] = {
		"8251 Compatible", /* 0xA0 */
		"8251 FIFO Compatible" /* 0xA1 */
	};

	if (code <= 0x23)
		return type[code];
	if (code >= 0xA0 && code <= 0xA1)
		return type_0xA0[code - 0xA0];
	if (code == 0xFF)
		return "Other";
	return out_of_spec;
}

/*
 * 7.10 System Slots (Type 9)
 */

static const char *dmi_slot_type(u8 code)
{
	/* 7.10.1 */
	static const char *type[] = {
		"Other", /* 0x01 */
		"Unknown",
		"ISA",
		"MCA",
		"EISA",
		"PCI",
		"PC Card (PCMCIA)",
		"VLB",
		"Proprietary",
		"Processor Card",
		"Proprietary Memory Card",
		"I/O Riser Card",
		"NuBus",
		"PCI-66",
		"AGP",
		"AGP 2x",
		"AGP 4x",
		"PCI-X",
		"AGP 8x",
		"M.2 Socket 1-DP",
		"M.2 Socket 1-SD",
		"M.2 Socket 2",
		"M.2 Socket 3",
		"MXM Type I",
		"MXM Type II",
		"MXM Type III",
		"MXM Type III-HE",
		"MXM Type IV",
		"MXM 3.0 Type A",
		"MXM 3.0 Type B",
		"PCI Express 2 SFF-8639 (U.2)",
		"PCI Express 3 SFF-8639 (U.2)",
		"PCI Express Mini 52-pin with bottom-side keep-outs",
		"PCI Express Mini 52-pin without bottom-side keep-outs",
		"PCI Express Mini 76-pin",
		"PCI Express 4 SFF-8639 (U.2)",
		"PCI Express 5 SFF-8639 (U.2)",
		"OCP NIC 3.0 Small Form Factor (SFF)",
		"OCP NIC 3.0 Large Form Factor (LFF)",
		"OCP NIC Prior to 3.0" /* 0x28 */
	};
	static const char *type_0x30[] = {
		"CXL FLexbus 1.0" /* 0x30 */
	};
	static const char *type_0xA0[] = {
		"PC-98/C20", /* 0xA0 */
		"PC-98/C24",
		"PC-98/E",
		"PC-98/Local Bus",
		"PC-98/Card",
		"PCI Express",
		"PCI Express x1",
		"PCI Express x2",
		"PCI Express x4",
		"PCI Express x8",
		"PCI Express x16",
		"PCI Express 2",
		"PCI Express 2 x1",
		"PCI Express 2 x2",
		"PCI Express 2 x4",
		"PCI Express 2 x8",
		"PCI Express 2 x16",
		"PCI Express 3",
		"PCI Express 3 x1",
		"PCI Express 3 x2",
		"PCI Express 3 x4",
		"PCI Express 3 x8",
		"PCI Express 3 x16",
		out_of_spec, /* 0xB7 */
		"PCI Express 4",
		"PCI Express 4 x1",
		"PCI Express 4 x2",
		"PCI Express 4 x4",
		"PCI Express 4 x8",
		"PCI Express 4 x16",
		"PCI Express 5",
		"PCI Express 5 x1",
		"PCI Express 5 x2",
		"PCI Express 5 x4",
		"PCI Express 5 x8",
		"PCI Express 5 x16",
		"PCI Express 6+",
		"EDSFF E1",
		"EDSFF E3" /* 0xC6 */
	};
	/*
	 * Note to developers: when adding entries to these lists, check if
	 * function dmi_slot_id below needs updating too.
	 */

	if (code >= 0x01 && code <= 0x28)
		return type[code - 0x01];
	if (code == 0x30)
		return type_0x30[code - 0x30];
	if (code >= 0xA0 && code <= 0xC6)
		return type_0xA0[code - 0xA0];
	return out_of_spec;
}

static const char *dmi_slot_bus_width(u8 code)
{
	/* 7.10.2 */
	static const char *width[] = {
		"Other", /* 0x01 */
		"Unknown",
		"8 bit",
		"16 bit",
		"32 bit",
		"64 bit",
		"128 bit",
		"1x or x1",
		"2x or x2",
		"4x or x4",
		"8x or x8",
		"12x or x12",
		"16x or x16",
		"32x or x32" /* 0x0E */
	};

	if (code >= 0x01 && code <= 0x0E)
		return width[code - 0x01];
	return out_of_spec;
}

static const char *dmi_slot_current_usage(u8 code)
{
	/* 7.10.3 */
	static const char *usage[] = {
		"Other", /* 0x01 */
		"Unknown",
		"Available",
		"In Use",
		"Unavailable" /* 0x05 */
	};

	if (code >= 0x01 && code <= 0x05)
		return usage[code - 0x01];
	return out_of_spec;
}

static const char *dmi_slot_length(u8 code)
{
	/* 7.10.4 */
	static const char *length[] = {
		"Other", /* 0x01 */
		"Unknown",
		"Short",
		"Long",
		"2.5\" drive form factor",
		"3.5\" drive form factor" /* 0x06 */
	};

	if (code >= 0x01 && code <= 0x06)
		return length[code - 0x01];
	return out_of_spec;
}

static void dmi_slot_id(u8 code1, u8 code2, u8 type)
{
	/* 7.10.5 */
	switch (type)
	{
		case 0x04: /* MCA */
			pr_attr("ID", "%u", code1);
			break;
		case 0x05: /* EISA */
			pr_attr("ID", "%u", code1);
			break;
		case 0x06: /* PCI */
		case 0x0E: /* PCI */
		case 0x0F: /* AGP */
		case 0x10: /* AGP */
		case 0x11: /* AGP */
		case 0x12: /* PCI-X */
		case 0x13: /* AGP */
		case 0x1F: /* PCI Express 2 */
		case 0x20: /* PCI Express 3 */
		case 0x21: /* PCI Express Mini */
		case 0x22: /* PCI Express Mini */
		case 0x23: /* PCI Express Mini */
		case 0xA5: /* PCI Express */
		case 0xA6: /* PCI Express */
		case 0xA7: /* PCI Express */
		case 0xA8: /* PCI Express */
		case 0xA9: /* PCI Express */
		case 0xAA: /* PCI Express */
		case 0xAB: /* PCI Express 2 */
		case 0xAC: /* PCI Express 2 */
		case 0xAD: /* PCI Express 2 */
		case 0xAE: /* PCI Express 2 */
		case 0xAF: /* PCI Express 2 */
		case 0xB0: /* PCI Express 2 */
		case 0xB1: /* PCI Express 3 */
		case 0xB2: /* PCI Express 3 */
		case 0xB3: /* PCI Express 3 */
		case 0xB4: /* PCI Express 3 */
		case 0xB5: /* PCI Express 3 */
		case 0xB6: /* PCI Express 3 */
		case 0xB8: /* PCI Express 4 */
		case 0xB9: /* PCI Express 4 */
		case 0xBA: /* PCI Express 4 */
		case 0xBB: /* PCI Express 4 */
		case 0xBC: /* PCI Express 4 */
		case 0xBD: /* PCI Express 4 */
		case 0xBE: /* PCI Express 5 */
		case 0xBF: /* PCI Express 5 */
		case 0xC0: /* PCI Express 5 */
		case 0xC1: /* PCI Express 5 */
		case 0xC2: /* PCI Express 5 */
		case 0xC3: /* PCI Express 5 */
		case 0xC4: /* PCI Express 6+ */
			pr_attr("ID", "%u", code1);
			break;
		case 0x07: /* PCMCIA */
			pr_attr("ID", "Adapter %u, Socket %u", code1, code2);
			break;
	}
}

static void dmi_slot_characteristics(const char *attr, u8 code1, u8 code2)
{
	/* 7.10.6 */
	static const char *characteristics1[] = {
		"5.0 V is provided", /* 1 */
		"3.3 V is provided",
		"Opening is shared",
		"PC Card-16 is supported",
		"Cardbus is supported",
		"Zoom Video is supported",
		"Modem ring resume is supported" /* 7 */
	};
	/* 7.10.7 */
	static const char *characteristics2[] = {
		"PME signal is supported", /* 0 */
		"Hot-plug devices are supported",
		"SMBus signal is supported",
		"PCIe slot bifurcation is supported",
		"Async/surprise removal is supported",
		"Flexbus slot, CXL 1.0 capable",
		"Flexbus slot, CXL 2.0 capable",
		"Flexbus slot, CXL 3.0 capable" /* 7 */
	};

	if (code1 & (1 << 0))
		pr_attr(attr, "Unknown");
	else if ((code1 & 0xFE) == 0 && code2 == 0)
		pr_attr(attr, "None");
	else
	{
		int i;

		pr_list_start(attr, NULL);
		for (i = 1; i <= 7; i++)
			if (code1 & (1 << i))
				pr_list_item("%s", characteristics1[i - 1]);
		for (i = 0; i <= 7; i++)
			if (code2 & (1 << i))
				pr_list_item("%s", characteristics2[i]);
		pr_list_end();
	}
}

static void dmi_slot_segment_bus_func(u16 code1, u8 code2, u8 code3)
{
	/* 7.10.8 */
	if (!(code1 == 0xFFFF && code2 == 0xFF && code3 == 0xFF))
		pr_attr("Bus Address", "%04x:%02x:%02x.%x",
			code1, code2, code3 >> 3, code3 & 0x7);
}

static void dmi_slot_peers(u8 n, const u8 *data)
{
	char attr[16];
	int i;

	for (i = 1; i <= n; i++, data += 5)
	{
		sprintf(attr, "Peer Device %hhu", (u8)i);
		pr_attr(attr, "%04x:%02x:%02x.%x (Width %u)",
			WORD(data), data[2], data[3] >> 3, data[3] & 0x07,
			data[4]);
	}
}

static void dmi_slot_information(u8 type, u8 code)
{
	switch (type)
	{
		case 0x1F: /* PCI Express 2 */
		case 0x20: /* PCI Express 3 */
		case 0x21: /* PCI Express Mini */
		case 0x22: /* PCI Express Mini */
		case 0x23: /* PCI Express Mini */
		case 0xA5: /* PCI Express */
		case 0xA6: /* PCI Express */
		case 0xA7: /* PCI Express */
		case 0xA8: /* PCI Express */
		case 0xA9: /* PCI Express */
		case 0xAA: /* PCI Express */
		case 0xAB: /* PCI Express 2 */
		case 0xAC: /* PCI Express 2 */
		case 0xAD: /* PCI Express 2 */
		case 0xAE: /* PCI Express 2 */
		case 0xAF: /* PCI Express 2 */
		case 0xB0: /* PCI Express 2 */
		case 0xB1: /* PCI Express 3 */
		case 0xB2: /* PCI Express 3 */
		case 0xB3: /* PCI Express 3 */
		case 0xB4: /* PCI Express 3 */
		case 0xB5: /* PCI Express 3 */
		case 0xB6: /* PCI Express 3 */
		case 0xB8: /* PCI Express 4 */
		case 0xB9: /* PCI Express 4 */
		case 0xBA: /* PCI Express 4 */
		case 0xBB: /* PCI Express 4 */
		case 0xBC: /* PCI Express 4 */
		case 0xBD: /* PCI Express 4 */
		case 0xBE: /* PCI Express 5 */
		case 0xBF: /* PCI Express 5 */
		case 0xC0: /* PCI Express 5 */
		case 0xC1: /* PCI Express 5 */
		case 0xC2: /* PCI Express 5 */
		case 0xC3: /* PCI Express 5 */
		case 0xC4: /* PCI Express 6+ */
			if (code)
				pr_attr("PCI Express Generation", "%u", code);
			break;
	}
}

static void dmi_slot_physical_width(u8 code)
{
	if (code)
		pr_attr("Slot Physical Width", "%s",
			dmi_slot_bus_width(code));
}

static void dmi_slot_pitch(u16 code)
{
	if (code)
		pr_attr("Pitch", "%u.%02u mm", code / 100, code % 100);
}

static const char *dmi_slot_height(u8 code)
{
	/* 7.10.3 */
	static const char *height[] = {
		"Not applicable", /* 0x00 */
		"Other",
		"Unknown",
		"Full height",
		"Low-profile" /* 0x04 */
	};

	if (code <= 0x04)
		return height[code];
	return out_of_spec;
}

/*
 * 7.11 On Board Devices Information (Type 10)
 */

static const char *dmi_on_board_devices_type(u8 code)
{
	/* 7.11.1 and 7.42.2 */
	static const char *type[] = {
		"Other", /* 0x01 */
		"Unknown",
		"Video",
		"SCSI Controller",
		"Ethernet",
		"Token Ring",
		"Sound",
		"PATA Controller",
		"SATA Controller",
		"SAS Controller",
		"Wireless LAN",
		"Bluetooth",
		"WWAN",
		"eMMC",
		"NVMe Controller",
		"UFS Controller" /* 0x10 */
	};

	if (code >= 0x01 && code <= 0x10)
		return type[code - 0x01];
	return out_of_spec;
}

static void dmi_on_board_devices(const struct dmi_header *h)
{
	u8 *p = h->data + 4;
	u8 count = (h->length - 0x04) / 2;
	int i;

	for (i = 0; i < count; i++)
	{
		if (count == 1)
			pr_handle_name("On Board Device Information");
		else
			pr_handle_name("On Board Device %d Information",
				       i + 1);
		pr_attr("Type", "%s",
			dmi_on_board_devices_type(p[2 * i] & 0x7F));
		pr_attr("Status", "%s",
			p[2 * i] & 0x80 ? "Enabled" : "Disabled");
		pr_attr("Description", "%s", dmi_string(h, p[2 * i + 1]));
	}
}

/*
 * 7.12 OEM Strings (Type 11)
 */

static void dmi_oem_strings(const struct dmi_header *h)
{
	char attr[11];
	u8 *p = h->data + 4;
	u8 count = p[0x00];
	int i;

	for (i = 1; i <= count; i++)
	{
		sprintf(attr, "String %hhu", (u8)i);
		pr_attr(attr, "%s",dmi_string(h, i));
	}
}

/*
 * 7.13 System Configuration Options (Type 12)
 */

static void dmi_system_configuration_options(const struct dmi_header *h)
{
	char attr[11];
	u8 *p = h->data + 4;
	u8 count = p[0x00];
	int i;

	for (i = 1; i <= count; i++)
	{
		sprintf(attr, "Option %hhu", (u8)i);
		pr_attr(attr, "%s",dmi_string(h, i));
	}
}

/*
 * 7.14 BIOS Language Information (Type 13)
 */

static void dmi_bios_languages(const struct dmi_header *h)
{
	u8 *p = h->data + 4;
	u8 count = p[0x00];
	int i;

	for (i = 1; i <= count; i++)
		pr_list_item("%s", dmi_string(h, i));
}

static const char *dmi_bios_language_format(u8 code)
{
	if (code & 0x01)
		return "Abbreviated";
	else
		return "Long";
}

/*
 * 7.15 Group Associations (Type 14)
 */

static void dmi_group_associations_items(u8 count, const u8 *p)
{
	int i;

	for (i = 0; i < count; i++)
	{
		pr_list_item("0x%04X (%s)",
			WORD(p + 3 * i + 1),
			dmi_smbios_structure_type(p[3 * i]));
	}
}

/*
 * 7.16 System Event Log (Type 15)
 */

static const char *dmi_event_log_method(u8 code)
{
	static const char *method[] = {
		"Indexed I/O, one 8-bit index port, one 8-bit data port", /* 0x00 */
		"Indexed I/O, two 8-bit index ports, one 8-bit data port",
		"Indexed I/O, one 16-bit index port, one 8-bit data port",
		"Memory-mapped physical 32-bit address",
		"General-purpose non-volatile data functions" /* 0x04 */
	};

	if (code <= 0x04)
		return method[code];
	if (code >= 0x80)
		return "OEM-specific";
	return out_of_spec;
}

static void dmi_event_log_status(u8 code)
{
	static const char *valid[] = {
		"Invalid", /* 0 */
		"Valid" /* 1 */
	};
	static const char *full[] = {
		"Not Full", /* 0 */
		"Full" /* 1 */
	};

	pr_attr("Status", "%s, %s",
		valid[(code >> 0) & 1], full[(code >> 1) & 1]);
}

static void dmi_event_log_address(u8 method, const u8 *p)
{
	/* 7.16.3 */
	switch (method)
	{
		case 0x00:
		case 0x01:
		case 0x02:
			pr_attr("Access Address", "Index 0x%04X, Data 0x%04X",
				WORD(p), WORD(p + 2));
			break;
		case 0x03:
			pr_attr("Access Address", "0x%08X", DWORD(p));
			break;
		case 0x04:
			pr_attr("Access Address", "0x%04X", WORD(p));
			break;
		default:
			pr_attr("Access Address", "Unknown");
	}
}

static const char *dmi_event_log_header_type(u8 code)
{
	static const char *type[] = {
		"No Header", /* 0x00 */
		"Type 1" /* 0x01 */
	};

	if (code <= 0x01)
		return type[code];
	if (code >= 0x80)
		return "OEM-specific";
	return out_of_spec;
}

static const char *dmi_event_log_descriptor_type(u8 code)
{
	/* 7.16.6.1 */
	static const char *type[] = {
		NULL, /* 0x00 */
		"Single-bit ECC memory error",
		"Multi-bit ECC memory error",
		"Parity memory error",
		"Bus timeout",
		"I/O channel block",
		"Software NMI",
		"POST memory resize",
		"POST error",
		"PCI parity error",
		"PCI system error",
		"CPU failure",
		"EISA failsafe timer timeout",
		"Correctable memory log disabled",
		"Logging disabled",
		NULL, /* 0x0F */
		"System limit exceeded",
		"Asynchronous hardware timer expired",
		"System configuration information",
		"Hard disk information",
		"System reconfigured",
		"Uncorrectable CPU-complex error",
		"Log area reset/cleared",
		"System boot" /* 0x17 */
	};

	if (code <= 0x17 && type[code] != NULL)
		return type[code];
	if (code >= 0x80 && code <= 0xFE)
		return "OEM-specific";
	if (code == 0xFF)
		return "End of log";
	return out_of_spec;
}

static const char *dmi_event_log_descriptor_format(u8 code)
{
	/* 7.16.6.2 */
	static const char *format[] = {
		"None", /* 0x00 */
		"Handle",
		"Multiple-event",
		"Multiple-event handle",
		"POST results bitmap",
		"System management",
		"Multiple-event system management" /* 0x06 */
	};

	if (code <= 0x06)
		return format[code];
	if (code >= 0x80)
		return "OEM-specific";
	return out_of_spec;
}

static void dmi_event_log_descriptors(u8 count, u8 len, const u8 *p)
{
	/* 7.16.1 */
	char attr[16];
	int i;

	for (i = 0; i < count; i++)
	{
		if (len >= 0x02)
		{
			sprintf(attr, "Descriptor %d", i + 1);
			pr_attr(attr, "%s",
				dmi_event_log_descriptor_type(p[i * len]));
			sprintf(attr, "Data Format %d", i + 1);
			pr_attr(attr, "%s",
				dmi_event_log_descriptor_format(p[i * len + 1]));
		}
	}
}

/*
 * 7.17 Physical Memory Array (Type 16)
 */

static const char *dmi_memory_array_location(u8 code)
{
	/* 7.17.1 */
	static const char *location[] = {
		"Other", /* 0x01 */
		"Unknown",
		"System Board Or Motherboard",
		"ISA Add-on Card",
		"EISA Add-on Card",
		"PCI Add-on Card",
		"MCA Add-on Card",
		"PCMCIA Add-on Card",
		"Proprietary Add-on Card",
		"NuBus" /* 0x0A */
	};
	static const char *location_0xA0[] = {
		"PC-98/C20 Add-on Card", /* 0xA0 */
		"PC-98/C24 Add-on Card",
		"PC-98/E Add-on Card",
		"PC-98/Local Bus Add-on Card",
		"CXL Add-on Card" /* 0xA4 */
	};

	if (code >= 0x01 && code <= 0x0A)
		return location[code - 0x01];
	if (code >= 0xA0 && code <= 0xA4)
		return location_0xA0[code - 0xA0];
	return out_of_spec;
}

static const char *dmi_memory_array_use(u8 code)
{
	/* 7.17.2 */
	static const char *use[] = {
		"Other", /* 0x01 */
		"Unknown",
		"System Memory",
		"Video Memory",
		"Flash Memory",
		"Non-volatile RAM",
		"Cache Memory" /* 0x07 */
	};

	if (code >= 0x01 && code <= 0x07)
		return use[code - 0x01];
	return out_of_spec;
}

static const char *dmi_memory_array_ec_type(u8 code)
{
	/* 7.17.3 */
	static const char *type[] = {
		"Other", /* 0x01 */
		"Unknown",
		"None",
		"Parity",
		"Single-bit ECC",
		"Multi-bit ECC",
		"CRC" /* 0x07 */
	};

	if (code >= 0x01 && code <= 0x07)
		return type[code - 0x01];
	return out_of_spec;
}

static void dmi_memory_array_error_handle(u16 code)
{
	if (code == 0xFFFE)
		pr_attr("Error Information Handle", "Not Provided");
	else if (code == 0xFFFF)
		pr_attr("Error Information Handle", "No Error");
	else
		pr_attr("Error Information Handle", "0x%04X", code);
}

/*
 * 7.18 Memory Device (Type 17)
 */

static void dmi_memory_device_width(const char *attr, u16 code)
{
	/*
	 * If no memory module is present, width may be 0
	 */
	if (code == 0xFFFF || (code == 0 && !(opt.flags & FLAG_NO_QUIRKS)))
		pr_attr(attr, "Unknown");
	else
		pr_attr(attr, "%u bits", code);
}

static void dmi_memory_device_size(u16 code)
{
	if (code == 0)
		pr_attr("Size", "No Module Installed");
	else if (code == 0xFFFF)
		pr_attr("Size", "Unknown");
	else
	{
		u64 s = { .l = code & 0x7FFF };
		if (!(code & 0x8000))
			s.l <<= 10;
		dmi_print_memory_size("Size", s, 1);
	}
}

static void dmi_memory_device_extended_size(u32 code)
{
	code &= 0x7FFFFFFFUL;

	/*
	 * Use the greatest unit for which the exact value can be displayed
	 * as an integer without rounding
	 */
	if (code & 0x3FFUL)
		pr_attr("Size", "%lu MB", (unsigned long)code);
	else if (code & 0xFFC00UL)
		pr_attr("Size", "%lu GB", (unsigned long)code >> 10);
	else
		pr_attr("Size", "%lu TB", (unsigned long)code >> 20);
}

static void dmi_memory_voltage_value(const char *attr, u16 code)
{
	if (code == 0)
		pr_attr(attr, "Unknown");
	else
		pr_attr(attr, code % 100 ? "%g V" : "%.1f V",
			(float)code / 1000);
}

static const char *dmi_memory_device_form_factor(u8 code)
{
	/* 7.18.1 */
	static const char *form_factor[] = {
		"Other", /* 0x01 */
		"Unknown",
		"SIMM",
		"SIP",
		"Chip",
		"DIP",
		"ZIP",
		"Proprietary Card",
		"DIMM",
		"TSOP",
		"Row Of Chips",
		"RIMM",
		"SODIMM",
		"SRIMM",
		"FB-DIMM",
		"Die" /* 0x10 */
	};

	if (code >= 0x01 && code <= 0x10)
		return form_factor[code - 0x01];
	return out_of_spec;
}

static void dmi_memory_device_set(u8 code)
{
	if (code == 0)
		pr_attr("Set", "None");
	else if (code == 0xFF)
		pr_attr("Set", "Unknown");
	else
		pr_attr("Set", "%u", code);
}

static const char *dmi_memory_device_type(u8 code)
{
	/* 7.18.2 */
	static const char *type[] = {
		"Other", /* 0x01 */
		"Unknown",
		"DRAM",
		"EDRAM",
		"VRAM",
		"SRAM",
		"RAM",
		"ROM",
		"Flash",
		"EEPROM",
		"FEPROM",
		"EPROM",
		"CDRAM",
		"3DRAM",
		"SDRAM",
		"SGRAM",
		"RDRAM",
		"DDR",
		"DDR2",
		"DDR2 FB-DIMM",
		"Reserved",
		"Reserved",
		"Reserved",
		"DDR3",
		"FBD2",
		"DDR4",
		"LPDDR",
		"LPDDR2",
		"LPDDR3",
		"LPDDR4",
		"Logical non-volatile device",
		"HBM",
		"HBM2",
		"DDR5",
		"LPDDR5",
		"HBM3" /* 0x24 */
	};

	if (code >= 0x01 && code <= 0x24)
		return type[code - 0x01];
	return out_of_spec;
}

static void dmi_memory_device_type_detail(u16 code)
{
	/* 7.18.3 */
	static const char *detail[] = {
		"Other", /* 1 */
		"Unknown",
		"Fast-paged",
		"Static Column",
		"Pseudo-static",
		"RAMBus",
		"Synchronous",
		"CMOS",
		"EDO",
		"Window DRAM",
		"Cache DRAM",
		"Non-Volatile",
		"Registered (Buffered)",
		"Unbuffered (Unregistered)",
		"LRDIMM"  /* 15 */
	};
	char list[172];		/* Update length if you touch the array above */

	if ((code & 0xFFFE) == 0)
		pr_attr("Type Detail", "None");
	else
	{
		int i, off = 0;

		list[0] = '\0';
		for (i = 1; i <= 15; i++)
			if (code & (1 << i))
				off += sprintf(list + off, off ? " %s" : "%s",
					       detail[i - 1]);
		pr_attr("Type Detail", list);
	}
}

static void dmi_memory_device_speed(const char *attr, u16 code1, u32 code2)
{
	if (code1 == 0xFFFF)
	{
		if (code2 == 0)
			pr_attr(attr, "Unknown");
		else
			pr_attr(attr, "%lu MT/s", code2);
	}
	else
	{
		if (code1 == 0)
			pr_attr(attr, "Unknown");
		else
			pr_attr(attr, "%u MT/s", code1);
	}
}

static void dmi_memory_technology(u8 code)
{
	/* 7.18.6 */
	static const char * const technology[] = {
		"Other", /* 0x01 */
		"Unknown",
		"DRAM",
		"NVDIMM-N",
		"NVDIMM-F",
		"NVDIMM-P",
		"Intel Optane DC persistent memory" /* 0x07 */
	};
	if (code >= 0x01 && code <= 0x07)
		pr_attr("Memory Technology", "%s", technology[code - 0x01]);
	else
		pr_attr("Memory Technology", "%s", out_of_spec);
}

static void dmi_memory_operating_mode_capability(u16 code)
{
	/* 7.18.7 */
	static const char * const mode[] = {
		"Other", /* 1 */
		"Unknown",
		"Volatile memory",
		"Byte-accessible persistent memory",
		"Block-accessible persistent memory" /* 5 */
	};
	char list[99];		/* Update length if you touch the array above */

	if ((code & 0xFFFE) == 0)
		pr_attr("Memory Operating Mode Capability", "None");
	else {
		int i, off = 0;

		list[0] = '\0';
		for (i = 1; i <= 5; i++)
			if (code & (1 << i))
				off += sprintf(list + off, off ? " %s" : "%s",
					       mode[i - 1]);
		pr_attr("Memory Operating Mode Capability", list);
	}
}

static void dmi_memory_manufacturer_id(const char *attr, u16 code)
{
	/* 7.18.8 */
	/* 7.18.10 */
	/* 7.18.15 */
	/* 7.17.17 */
	/* LSB is 7-bit Odd Parity number of continuation codes */
	if (code == 0)
		pr_attr(attr, "Unknown");
	else
		pr_attr(attr, "Bank %d, Hex 0x%02X",
			(code & 0x7F) + 1, code >> 8);
}

static void dmi_memory_product_id(const char *attr, u16 code)
{
	/* 7.18.9 */
	/* 7.18.11 */
	if (code == 0)
		pr_attr(attr, "Unknown");
	else
		pr_attr(attr, "0x%04X", code);
}

static void dmi_memory_size(const char *attr, u64 code)
{
	/* 7.18.12 */
	/* 7.18.13 */
	if (code.h == 0xFFFFFFFF && code.l == 0xFFFFFFFF)
		pr_attr(attr, "Unknown");
	else if (code.h == 0x0 && code.l == 0x0)
		pr_attr(attr, "None");
	else
		dmi_print_memory_size(attr, code, 0);
}

static void dmi_memory_revision(const char *attr_type, u16 code, u8 mem_type)
{
	/* 7.18.16 */
	/* 7.18.18 */
	char attr[22];

	if (code == 0xFF00)
	{
		snprintf(attr, sizeof(attr), "%s Revision Number", attr_type);
		pr_attr(attr, "Unknown");
	}
	else if (mem_type == 0x22 || mem_type == 0x23)	/* DDR5 */
	{
		u8 dev_type = (code >> 8) & 0x0F;
		u8 dev_rev = code & 0xFF;

		if (code & 0x8000)			/* Installed */
		{
			snprintf(attr, sizeof(attr), "%s Device Type",
				 attr_type);
			pr_attr(attr, "%hu", dev_type);
			snprintf(attr, sizeof(attr), "%s Device Revision",
				 attr_type);
			pr_attr(attr, "%hu.%hu", dev_rev >> 4, dev_rev & 0x0F);
		}
		else
		{
			snprintf(attr, sizeof(attr), "%s Device Type",
				 attr_type);
			pr_attr(attr, "Not Installed");
		}
	}
	else						/* Generic fallback */
	{
		snprintf(attr, sizeof(attr), "%s Revision Number", attr_type);
		pr_attr(attr, "0x%04x", code);
	}
}

/*
 * 7.19 32-bit Memory Error Information (Type 18)
 */

static const char *dmi_memory_error_type(u8 code)
{
	/* 7.19.1 */
	static const char *type[] = {
		"Other", /* 0x01 */
		"Unknown",
		"OK",
		"Bad Read",
		"Parity Error",
		"Single-bit Error",
		"Double-bit Error",
		"Multi-bit Error",
		"Nibble Error",
		"Checksum Error",
		"CRC Error",
		"Corrected Single-bit Error",
		"Corrected Error",
		"Uncorrectable Error" /* 0x0E */
	};

	if (code >= 0x01 && code <= 0x0E)
		return type[code - 0x01];
	return out_of_spec;
}

static const char *dmi_memory_error_granularity(u8 code)
{
	/* 7.19.2 */
	static const char *granularity[] = {
		"Other", /* 0x01 */
		"Unknown",
		"Device Level",
		"Memory Partition Level" /* 0x04 */
	};

	if (code >= 0x01 && code <= 0x04)
		return granularity[code - 0x01];
	return out_of_spec;
}

static const char *dmi_memory_error_operation(u8 code)
{
	/* 7.19.3 */
	static const char *operation[] = {
		"Other", /* 0x01 */
		"Unknown",
		"Read",
		"Write",
		"Partial Write" /* 0x05 */
	};

	if (code >= 0x01 && code <= 0x05)
		return operation[code - 0x01];
	return out_of_spec;
}

static void dmi_memory_error_syndrome(u32 code)
{
	if (code == 0x00000000)
		pr_attr("Vendor Syndrome", "Unknown");
	else
		pr_attr("Vendor Syndrome", "0x%08X", code);
}

static void dmi_32bit_memory_error_address(const char *attr, u32 code)
{
	if (code == 0x80000000)
		pr_attr(attr, "Unknown");
	else
		pr_attr(attr, "0x%08X", code);
}

/*
 * 7.20 Memory Array Mapped Address (Type 19)
 */

static void dmi_mapped_address_size(u32 code)
{
	if (code == 0)
		pr_attr("Range Size", "Invalid");
	else
	{
		u64 size;

		size.h = 0;
		size.l = code;
		dmi_print_memory_size("Range Size", size, 1);
	}
}

static void dmi_mapped_address_extended_size(u64 start, u64 end)
{
	if (start.h == end.h && start.l == end.l)
		pr_attr("Range Size", "Invalid");
	else
		dmi_print_memory_size("Range Size", u64_range(start, end), 0);
}

/*
 * 7.21 Memory Device Mapped Address (Type 20)
 */

static void dmi_mapped_address_row_position(u8 code)
{
	if (code == 0)
		pr_attr("Partition Row Position", "%s", out_of_spec);
	else if (code == 0xFF)
		pr_attr("Partition Row Position", "Unknown");
	else
		pr_attr("Partition Row Position", "%u", code);
}

static void dmi_mapped_address_interleave_position(u8 code)
{
	if (code != 0)
	{
		if (code == 0xFF)
			pr_attr("Interleave Position", "Unknown");
		else
			pr_attr("Interleave Position", "%u", code);
	}
}

static void dmi_mapped_address_interleaved_data_depth(u8 code)
{
	if (code != 0)
	{
		if (code == 0xFF)
			pr_attr("Interleaved Data Depth", "Unknown");
		else
			pr_attr("Interleaved Data Depth", "%u", code);
	}
}

/*
 * 7.22 Built-in Pointing Device (Type 21)
 */

static const char *dmi_pointing_device_type(u8 code)
{
	/* 7.22.1 */
	static const char *type[] = {
		"Other", /* 0x01 */
		"Unknown",
		"Mouse",
		"Track Ball",
		"Track Point",
		"Glide Point",
		"Touch Pad",
		"Touch Screen",
		"Optical Sensor" /* 0x09 */
	};

	if (code >= 0x01 && code <= 0x09)
		return type[code - 0x01];
	return out_of_spec;
}

static const char *dmi_pointing_device_interface(u8 code)
{
	/* 7.22.2 */
	static const char *interface[] = {
		"Other", /* 0x01 */
		"Unknown",
		"Serial",
		"PS/2",
		"Infrared",
		"HIP-HIL",
		"Bus Mouse",
		"ADB (Apple Desktop Bus)" /* 0x08 */
	};
	static const char *interface_0xA0[] = {
		"Bus Mouse DB-9", /* 0xA0 */
		"Bus Mouse Micro DIN",
		"USB",
		"I2C",
		"SPI" /* 0xA4 */
	};

	if (code >= 0x01 && code <= 0x08)
		return interface[code - 0x01];
	if (code >= 0xA0 && code <= 0xA4)
		return interface_0xA0[code - 0xA0];
	return out_of_spec;
}

/*
 * 7.23 Portable Battery (Type 22)
 */

static const char *dmi_battery_chemistry(u8 code)
{
	/* 7.23.1 */
	static const char *chemistry[] = {
		"Other", /* 0x01 */
		"Unknown",
		"Lead Acid",
		"Nickel Cadmium",
		"Nickel Metal Hydride",
		"Lithium Ion",
		"Zinc Air",
		"Lithium Polymer" /* 0x08 */
	};

	if (code >= 0x01 && code <= 0x08)
		return chemistry[code - 0x01];
	return out_of_spec;
}

static void dmi_battery_capacity(u16 code, u8 multiplier)
{
	if (code == 0)
		pr_attr("Design Capacity", "Unknown");
	else
		pr_attr("Design Capacity", "%u mWh", code * multiplier);
}

static void dmi_battery_voltage(u16 code)
{
	if (code == 0)
		pr_attr("Design Voltage", "Unknown");
	else
		pr_attr("Design Voltage", "%u mV", code);
}

static void dmi_battery_maximum_error(u8 code)
{
	if (code == 0xFF)
		pr_attr("Maximum Error", "Unknown");
	else
		pr_attr("Maximum Error", "%u%%", code);
}

/*
 * 7.24 System Reset (Type 23)
 */

/* code is assumed to be a 2-bit value */
static const char *dmi_system_reset_boot_option(u8 code)
{
	static const char *option[] = {
		out_of_spec, /* 0x0 */
		"Operating System", /* 0x1 */
		"System Utilities",
		"Do Not Reboot" /* 0x3 */
	};

	return option[code];
}

static void dmi_system_reset_count(const char *attr, u16 code)
{
	if (code == 0xFFFF)
		pr_attr(attr, "Unknown");
	else
		pr_attr(attr, "%u", code);
}

static void dmi_system_reset_timer(const char *attr, u16 code)
{
	if (code == 0xFFFF)
		pr_attr(attr, "Unknown");
	else
		pr_attr(attr, "%u min", code);
}

/*
 * 7.25 Hardware Security (Type 24)
 */

static const char *dmi_hardware_security_status(u8 code)
{
	static const char *status[] = {
		"Disabled", /* 0x00 */
		"Enabled",
		"Not Implemented",
		"Unknown" /* 0x03 */
	};

	return status[code];
}

/*
 * 7.26 System Power Controls (Type 25)
 */

static void dmi_power_controls_power_on(const u8 *p)
{
	char time[15];
	int off = 0;

	/* 7.26.1 */
	if (dmi_bcd_range(p[0], 0x01, 0x12))
		off += sprintf(time + off, "%02X", p[0]);
	else
		off += sprintf(time + off, "*");
	if (dmi_bcd_range(p[1], 0x01, 0x31))
		off += sprintf(time + off, "-%02X", p[1]);
	else
		off += sprintf(time + off, "-*");
	if (dmi_bcd_range(p[2], 0x00, 0x23))
		off += sprintf(time + off, " %02X", p[2]);
	else
		off += sprintf(time + off, " *");
	if (dmi_bcd_range(p[3], 0x00, 0x59))
		off += sprintf(time + off, ":%02X", p[3]);
	else
		off += sprintf(time + off, ":*");
	if (dmi_bcd_range(p[4], 0x00, 0x59))
		off += sprintf(time + off, ":%02X", p[4]);
	else
		off += sprintf(time + off, ":*");

	pr_attr("Next Scheduled Power-on", time);
}

/*
 * 7.27 Voltage Probe (Type 26)
 */

static const char *dmi_voltage_probe_location(u8 code)
{
	/* 7.27.1 */
	static const char *location[] = {
		"Other", /* 0x01 */
		"Unknown",
		"Processor",
		"Disk",
		"Peripheral Bay",
		"System Management Module",
		"Motherboard",
		"Memory Module",
		"Processor Module",
		"Power Unit",
		"Add-in Card" /* 0x0B */
	};

	if (code >= 0x01 && code <= 0x0B)
		return location[code - 0x01];
	return out_of_spec;
}

static const char *dmi_probe_status(u8 code)
{
	/* 7.27.1 */
	static const char *status[] = {
		"Other", /* 0x01 */
		"Unknown",
		"OK",
		"Non-critical",
		"Critical",
		"Non-recoverable" /* 0x06 */
	};

	if (code >= 0x01 && code <= 0x06)
		return status[code - 0x01];
	return out_of_spec;
}

static void dmi_voltage_probe_value(const char *attr, u16 code)
{
	if (code == 0x8000)
		pr_attr(attr, "Unknown");
	else
		pr_attr(attr, "%.3f V", (float)(i16)code / 1000);
}

static void dmi_voltage_probe_resolution(u16 code)
{
	if (code == 0x8000)
		pr_attr("Resolution", "Unknown");
	else
		pr_attr("Resolution", "%.1f mV", (float)code / 10);
}

static void dmi_probe_accuracy(u16 code)
{
	if (code == 0x8000)
		pr_attr("Accuracy", "Unknown");
	else
		pr_attr("Accuracy", "%.2f%%", (float)code / 100);
}

/*
 * 7.28 Cooling Device (Type 27)
 */

static const char *dmi_cooling_device_type(u8 code)
{
	/* 7.28.1 */
	static const char *type[] = {
		"Other", /* 0x01 */
		"Unknown",
		"Fan",
		"Centrifugal Blower",
		"Chip Fan",
		"Cabinet Fan",
		"Power Supply Fan",
		"Heat Pipe",
		"Integrated Refrigeration" /* 0x09 */
	};
	static const char *type_0x10[] = {
		"Active Cooling", /* 0x10 */
		"Passive Cooling" /* 0x11 */
	};

	if (code >= 0x01 && code <= 0x09)
		return type[code - 0x01];
	if (code >= 0x10 && code <= 0x11)
		return type_0x10[code - 0x10];
	return out_of_spec;
}

static void dmi_cooling_device_speed(u16 code)
{
	if (code == 0x8000)
		pr_attr("Nominal Speed", "Unknown Or Non-rotating");
	else
		pr_attr("Nominal Speed", "%u rpm", code);
}

/*
 * 7.29 Temperature Probe (Type 28)
 */

static const char *dmi_temperature_probe_location(u8 code)
{
	/* 7.29.1 */
	static const char *location[] = {
		"Other", /* 0x01 */
		"Unknown",
		"Processor",
		"Disk",
		"Peripheral Bay",
		"System Management Module",
		"Motherboard",
		"Memory Module",
		"Processor Module",
		"Power Unit",
		"Add-in Card",
		"Front Panel Board",
		"Back Panel Board",
		"Power System Board",
		"Drive Back Plane" /* 0x0F */
	};

	if (code >= 0x01 && code <= 0x0F)
		return location[code - 0x01];
	return out_of_spec;
}

static void dmi_temperature_probe_value(const char *attr, u16 code)
{
	if (code == 0x8000)
		pr_attr(attr, "Unknown");
	else
		pr_attr(attr, "%.1f deg C", (float)(i16)code / 10);
}

static void dmi_temperature_probe_resolution(u16 code)
{
	if (code == 0x8000)
		pr_attr("Resolution", "Unknown");
	else
		pr_attr("Resolution", "%.3f deg C", (float)code / 1000);
}

/*
 * 7.30 Electrical Current Probe (Type 29)
 */

static void dmi_current_probe_value(const char *attr, u16 code)
{
	if (code == 0x8000)
		pr_attr(attr, "Unknown");
	else
		pr_attr(attr, "%.3f A", (float)(i16)code / 1000);
}

static void dmi_current_probe_resolution(u16 code)
{
	if (code == 0x8000)
		pr_attr("Resolution", "Unknown");
	else
		pr_attr("Resolution", "%.1f mA", (float)code / 10);
}

/*
 * 7.33 System Boot Information (Type 32)
 */

static const char *dmi_system_boot_status(u8 code)
{
	static const char *status[] = {
		"No errors detected", /* 0 */
		"No bootable media",
		"Operating system failed to load",
		"Firmware-detected hardware failure",
		"Operating system-detected hardware failure",
		"User-requested boot",
		"System security violation",
		"Previously-requested image",
		"System watchdog timer expired" /* 8 */
	};

	if (code <= 8)
		return status[code];
	if (code >= 128 && code <= 191)
		return "OEM-specific";
	if (code >= 192)
		return "Product-specific";
	return out_of_spec;
}

/*
 * 7.34 64-bit Memory Error Information (Type 33)
 */

static void dmi_64bit_memory_error_address(const char *attr, u64 code)
{
	if (code.h == 0x80000000 && code.l == 0x00000000)
		pr_attr(attr, "Unknown");
	else
		pr_attr(attr, "0x%08X%08X", code.h, code.l);
}

/*
 * 7.35 Management Device (Type 34)
 */

/*
 * Several boards have a bug where some type 34 structures have their
 * length incorrectly set to 0x10 instead of 0x0B. This causes the
 * first 5 characters of the device name to be trimmed. It's easy to
 * check and fix, so do it, but warn.
 */
static void dmi_fixup_type_34(struct dmi_header *h, int display)
{
	u8 *p = h->data;

	/* Make sure the hidden data is ASCII only */
	if (h->length == 0x10
	 && is_printable(p + 0x0B, 0x10 - 0x0B))
	{
		if (!(opt.flags & FLAG_QUIET) && display)
			fprintf(stderr,
				"Invalid entry length (%u). Fixed up to %u.\n",
				0x10, 0x0B);
		h->length = 0x0B;
	}
}

static const char *dmi_management_device_type(u8 code)
{
	/* 7.35.1 */
	static const char *type[] = {
		"Other", /* 0x01 */
		"Unknown",
		"LM75",
		"LM78",
		"LM79",
		"LM80",
		"LM81",
		"ADM9240",
		"DS1780",
		"MAX1617",
		"GL518SM",
		"W83781D",
		"HT82H791" /* 0x0D */
	};

	if (code >= 0x01 && code <= 0x0D)
		return type[code - 0x01];
	return out_of_spec;
}

static const char *dmi_management_device_address_type(u8 code)
{
	/* 7.35.2 */
	static const char *type[] = {
		"Other", /* 0x01 */
		"Unknown",
		"I/O Port",
		"Memory",
		"SMBus" /* 0x05 */
	};

	if (code >= 0x01 && code <= 0x05)
		return type[code - 0x01];
	return out_of_spec;
}

/*
 * 7.38 Memory Channel (Type 37)
 */

static const char *dmi_memory_channel_type(u8 code)
{
	/* 7.38.1 */
	static const char *type[] = {
		"Other", /* 0x01 */
		"Unknown",
		"RamBus",
		"SyncLink" /* 0x04 */
	};

	if (code >= 0x01 && code <= 0x04)
		return type[code - 0x01];
	return out_of_spec;
}

static void dmi_memory_channel_devices(u8 count, const u8 *p)
{
	char attr[18];
	int i;

	for (i = 1; i <= count; i++)
	{
		sprintf(attr, "Device %hhu Load", (u8)i);
		pr_attr(attr, "%u", p[3 * i]);
		if (!(opt.flags & FLAG_QUIET))
		{
			sprintf(attr, "Device %hhu Handle", (u8)i);
			pr_attr(attr, "0x%04X", WORD(p + 3 * i + 1));
		}
	}
}

/*
 * 7.39 IPMI Device Information (Type 38)
 */

static const char *dmi_ipmi_interface_type(u8 code)
{
	/* 7.39.1 and IPMI 2.0, appendix C1, table C1-2 */
	static const char *type[] = {
		"Unknown", /* 0x00 */
		"KCS (Keyboard Control Style)",
		"SMIC (Server Management Interface Chip)",
		"BT (Block Transfer)",
		"SSIF (SMBus System Interface)" /* 0x04 */
	};

	if (code <= 0x04)
		return type[code];
	return out_of_spec;
}

static void dmi_ipmi_base_address(u8 type, const u8 *p, u8 lsb)
{
	if (type == 0x04) /* SSIF */
	{
		pr_attr("Base Address", "0x%02X (SMBus)", (*p) >> 1);
	}
	else
	{
		u64 address = QWORD(p);
		pr_attr("Base Address", "0x%08X%08X (%s)",
			address.h, (address.l & ~1) | lsb,
			address.l & 1 ? "I/O" : "Memory-mapped");
	}
}

/* code is assumed to be a 2-bit value */
static const char *dmi_ipmi_register_spacing(u8 code)
{
	/* IPMI 2.0, appendix C1, table C1-1 */
	static const char *spacing[] = {
		"Successive Byte Boundaries", /* 0x00 */
		"32-bit Boundaries",
		"16-byte Boundaries", /* 0x02 */
		out_of_spec /* 0x03 */
	};

	return spacing[code];
}

/*
 * 7.40 System Power Supply (Type 39)
 */

static void dmi_power_supply_power(u16 code)
{
	if (code == 0x8000)
		pr_attr("Max Power Capacity", "Unknown");
	else
		pr_attr("Max Power Capacity", "%u W", (unsigned int)code);
}

static const char *dmi_power_supply_type(u8 code)
{
	/* 7.40.1 */
	static const char *type[] = {
		"Other", /* 0x01 */
		"Unknown",
		"Linear",
		"Switching",
		"Battery",
		"UPS",
		"Converter",
		"Regulator" /* 0x08 */
	};

	if (code >= 0x01 && code <= 0x08)
		return type[code - 0x01];
	return out_of_spec;
}

static const char *dmi_power_supply_status(u8 code)
{
	/* 7.40.1 */
	static const char *status[] = {
		"Other", /* 0x01 */
		"Unknown",
		"OK",
		"Non-critical",
		"Critical" /* 0x05 */
	};

	if (code >= 0x01 && code <= 0x05)
		return status[code - 0x01];
	return out_of_spec;
}

static const char *dmi_power_supply_range_switching(u8 code)
{
	/* 7.40.1 */
	static const char *switching[] = {
		"Other", /* 0x01 */
		"Unknown",
		"Manual",
		"Auto-switch",
		"Wide Range",
		"N/A" /* 0x06 */
	};

	if (code >= 0x01 && code <= 0x06)
		return switching[code - 0x01];
	return out_of_spec;
}

/*
 * 7.41 Additional Information (Type 40)
 *
 * Proper support of this entry type would require redesigning a large part of
 * the code, so I am waiting to see actual implementations of it to decide
 * whether it's worth the effort.
 */

static void dmi_additional_info(const struct dmi_header *h)
{
	u8 *p = h->data + 4;
	u8 count = *p++;
	u8 length;
	int i, offset = 5;

	for (i = 0; i < count; i++)
	{
		pr_handle_name("Additional Information %d", i + 1);

		/* Check for short entries */
		if (h->length < offset + 1) break;
		length = p[0x00];
		if (length < 0x05 || h->length < offset + length) break;

		pr_attr("Referenced Handle", "0x%04x",
			WORD(p + 0x01));
		pr_attr("Referenced Offset", "0x%02x",
			p[0x03]);
		pr_attr("String", "%s",
			dmi_string(h, p[0x04]));

		switch (length - 0x05)
		{
			case 1:
				pr_attr("Value", "0x%02x", p[0x05]);
				break;
			case 2:
				pr_attr("Value", "0x%04x", WORD(p + 0x05));
				break;
			case 4:
				pr_attr("Value", "0x%08x", DWORD(p + 0x05));
				break;
			default:
				pr_attr("Value", "Unexpected size");
				break;
		}

		p += length;
		offset += length;
	}
}

/*
 * 7.43 Management Controller Host Interface (Type 42)
 */

static const char *dmi_management_controller_host_type(u8 code)
{
	/* DMTF DSP0239 (MCTP) version 1.1.0 */
	static const char *type[] = {
		"KCS: Keyboard Controller Style", /* 0x02 */
		"8250 UART Register Compatible",
		"16450 UART Register Compatible",
		"16550/16550A UART Register Compatible",
		"16650/16650A UART Register Compatible",
		"16750/16750A UART Register Compatible",
		"16850/16850A UART Register Compatible" /* 0x08 */
	};

	if (code >= 0x02 && code <= 0x08)
		return type[code - 0x02];
	if (code <= 0x3F)
		return "MCTP";
	if (code == 0x40)
		return "Network";
	if (code == 0xF0)
		return "OEM";
	return out_of_spec;
}

/*
 * 7.43.2: Protocol Record Types
 */
static const char *dmi_protocol_record_type(u8 type)
{
	const char *protocol[] = {
		"Reserved",		/* 0x0 */
		"Reserved",
		"IPMI",
		"MCTP",
		"Redfish over IP",	/* 0x4 */
	};

	if (type <= 0x4)
		return protocol[type];
	if (type == 0xF0)
		return "OEM";
	return out_of_spec;
}

/*
 * DSP0270: 8.4.2: Protocol IP Assignment types
 */
static const char *dmi_protocol_assignment_type(u8 type)
{
	const char *assignment[] = {
		"Unknown",		/* 0x0 */
		"Static",
		"DHCP",
		"AutoConf",
		"Host Selected",	/* 0x4 */
	};

	if (type <= 0x4)
		return assignment[type];
	return out_of_spec;
}

/*
 * DSP0270: 8.4.3: Protocol IP Address type
 */
static const char *dmi_address_type(u8 type)
{
	const char *addressformat[] = {
		"Unknown",	/* 0x0 */
		"IPv4",
		"IPv6",		/* 0x2 */
	};

	if (type <= 0x2)
		return addressformat[type];
	return out_of_spec;
}

/*
 *  DSP0270: 8.4.3 Protocol Address decode
 */
static const char *dmi_address_decode(u8 *data, char *storage, u8 addrtype)
{
	if (addrtype == 0x1) /* IPv4 */
		return inet_ntop(AF_INET, data, storage, 64);
	if (addrtype == 0x2) /* IPv6 */
		return inet_ntop(AF_INET6, data, storage, 64);
	return out_of_spec;
}

/*
 * DSP0270: 8.4: Parse the protocol record format
 */
static void dmi_parse_protocol_record(u8 *rec)
{
	u8 rid;
	u8 rlen;
	u8 *rdata;
	char buf[64];
	u8 assign_val;
	u8 addrtype;
	u8 hlen;
	const char *addrstr;
	const char *hname;
	char attr[38];

	/* DSP0270: 8.4: Protocol Identifier */
	rid = rec[0x0];
	/* DSP0270: 8.4: Protocol Record Length */
	rlen = rec[0x1];
	/* DSP0270: 8.4: Protocol Record Data */
	rdata = &rec[0x2];

	pr_attr("Protocol ID", "%02x (%s)", rid,
		dmi_protocol_record_type(rid));

	/*
	 * Don't decode anything other than Redfish for now
	 * Note 0x4 is Redfish over IP in 7.43.2
	 * and DSP0270: 8.4
	 */
	if (rid != 0x4)
		return;

	/*
	 * Ensure that the protocol record is of sufficient length
	 * For RedFish that means rlen must be at least 91 bytes
	 * other protcols will need different length checks
	 */
	if (rlen < 91)
		return;

	/*
	 * DSP0270: 8.4.1: Redfish Over IP Service UUID
	 * Note: ver is hardcoded to 0x311 here just for
	 * convenience.  It could get passed from the SMBIOS
	 * header, but that's a lot of passing of pointers just
	 * to get that info, and the only thing it is used for is
	 * to determine the endianness of the field.  Since we only
	 * do this parsing on versions of SMBIOS after 3.1.1, and the
	 * endianness of the field is always little after version 2.6.0
	 * we can just pick a sufficiently recent version here.
	 */
	dmi_system_uuid(pr_subattr, "Service UUID", &rdata[0], 0x311);

	/*
	 * DSP0270: 8.4.1: Redfish Over IP Host IP Assignment Type
	 * Note, using decimal indices here, as the DSP0270
	 * uses decimal, so as to make it more comparable
	 */
	assign_val = rdata[16];
	pr_subattr("Host IP Assignment Type", "%s",
		dmi_protocol_assignment_type(assign_val));

	/* DSP0270: 8.4.1: Redfish Over IP Host Address format */
	addrtype = rdata[17];
	addrstr = dmi_address_type(addrtype);
	pr_subattr("Host IP Address Format", "%s",
		addrstr);

	/* DSP0270: 8.4.1 IP Assignment types */
	/* We only use the Host IP Address and Mask if the assignment type is static */
	if (assign_val == 0x1 || assign_val == 0x3)
	{
		/* DSP0270: 8.4.1: the Host IPv[4|6] Address */
		sprintf(attr, "%s Address", addrstr);
		pr_subattr(attr, "%s",
			dmi_address_decode(&rdata[18], buf, addrtype));

		/* DSP0270: 8.4.1: Prints the Host IPv[4|6] Mask */
		sprintf(attr, "%s Mask", addrstr);
		pr_subattr(attr, "%s",
			dmi_address_decode(&rdata[34], buf, addrtype));
	}

	/* DSP0270: 8.4.1: Get the Redfish Service IP Discovery Type */
	assign_val = rdata[50];
	/* Redfish Service IP Discovery type mirrors Host IP Assignment type */
	pr_subattr("Redfish Service IP Discovery Type", "%s",
		dmi_protocol_assignment_type(assign_val));

	/* DSP0270: 8.4.1: Get the Redfish Service IP Address Format */
	addrtype = rdata[51];
	addrstr = dmi_address_type(addrtype);
	pr_subattr("Redfish Service IP Address Format", "%s",
		addrstr);

	if (assign_val == 0x1 || assign_val == 0x3)
	{
		u16 port;
		u32 vlan;

		/* DSP0270: 8.4.1: Prints the Redfish IPv[4|6] Service Address */
		sprintf(attr, "%s Redfish Service Address", addrstr);
		pr_subattr(attr, "%s",
			dmi_address_decode(&rdata[52], buf,
			addrtype));

		/* DSP0270: 8.4.1: Prints the Redfish IPv[4|6] Service Mask */
		sprintf(attr, "%s Redfish Service Mask", addrstr);
		pr_subattr(attr, "%s",
			dmi_address_decode(&rdata[68], buf,
			addrtype));

		/* DSP0270: 8.4.1: Redfish vlan and port info */
		port = WORD(&rdata[84]);
		vlan = DWORD(&rdata[86]);
		pr_subattr("Redfish Service Port", "%hu", port);
		pr_subattr("Redfish Service Vlan", "%u", vlan);
	}

	/* DSP0270: 8.4.1: Redfish host length and name */
	hlen = rdata[90];

	/*
	 * DSP0270: 8.4.1: The length of the host string + 91 (the minimum
	 * size of a protocol record) cannot exceed the record length
	 * (rec[0x1])
	 */
	hname = (const char *)&rdata[91];
	if (hlen + 91 > rlen)
	{
		hname = out_of_spec;
		hlen = strlen(out_of_spec);
	}
	pr_subattr("Redfish Service Hostname", "%.*s", hlen, hname);
}

/*
 * DSP0270: 8.3: Device type ennumeration
 */
static const char *dmi_parse_device_type(u8 type)
{
	const char *devname[] = {
		"USB",		/* 0x2 */
		"PCI/PCIe",	/* 0x3 */
		"USB v2",	/* 0x4 */
		"PCI/PCIe v2",	/* 0x5 */
	};

	if (type >= 0x2 && type <= 0x5)
		return devname[type - 0x2];
	if (type >= 0x80)
		return "OEM";
	return out_of_spec;
}

/*
 * DSP0270: 8.3.7: Device Characteristics
 */
static void dmi_device_characteristics(u16 code)
{
	const char *characteristics[] = {
		"Credential bootstrapping via IPMI is supported", /* 0 */
		/* Reserved */
	};

	if ((code & 0x1) == 0)
		pr_list_item("None");
	else
	{
		int i;

		for (i = 0; i < 1; i++)
			if (code & (1 << i))
				pr_list_item("%s", characteristics[i]);
	}
}

static void dmi_parse_controller_structure(const struct dmi_header *h)
{
	int i;
	u8 *data = h->data;
	/* Host interface type */
	u8 type;
	/* Host Interface specific data length */
	u8 len;
	u8 count;
	u32 total_read;

	/*
	 * Minimum length of this struct is 0xB bytes
	 */
	if (h->length < 0xB)
		return;

	/*
	 * Also need to ensure that the interface specific data length
	 * plus the size of the structure to that point don't exceed
	 * the defined length of the structure, or we will overrun its
	 * bounds
	 */
	len = data[0x5];
	total_read = len + 0x6;

	if (total_read > h->length)
		return;

	type = data[0x4];
	pr_attr("Host Interface Type", "%s",
		dmi_management_controller_host_type(type));

	/*
	 * The following decodes are code for Network interface host types only
	 * As defined in DSP0270
	 */
	if (type != 0x40)
		return;

	if (len != 0)
	{
		/* DSP0270: 8.3.1 Table 3: Device Type values */
		type = data[0x6];

		pr_attr("Device Type", "%s",
			dmi_parse_device_type(type));
		if (type == 0x2 && len >= 5)
		{
			/* USB Device Type - need at least 6 bytes */
			u8 *usbdata = &data[0x7];
			/* USB Device Descriptor: idVendor */
			pr_attr("idVendor", "0x%04x",
				WORD(&usbdata[0x0]));
			/* USB Device Descriptor: idProduct */
			pr_attr("idProduct", "0x%04x",
				WORD(&usbdata[0x2]));
			/*
			 * USB Serial number is here, but its useless, don't
			 * bother decoding it
			 */
		}
		else if (type == 0x3 && len >= 9)
		{
			/* PCI Device Type - Need at least 8 bytes */
			u8 *pcidata = &data[0x7];
			/* PCI Device Descriptor: VendorID */
			pr_attr("VendorID", "0x%04x",
				WORD(&pcidata[0x0]));
			/* PCI Device Descriptor: DeviceID */
			pr_attr("DeviceID", "0x%04x",
				WORD(&pcidata[0x2]));
			/* PCI Device Descriptor: PCI SubvendorID */
			pr_attr("SubVendorID", "0x%04x",
				WORD(&pcidata[0x4]));
			/* PCI Device Descriptor: PCI SubdeviceID */
			pr_attr("SubDeviceID", "0x%04x",
				WORD(&pcidata[0x6]));
		}
		else if (type == 0x4 && len >= 0x0d)
		{
			/* USB Device Type v2 - need at least 12 bytes */
			u8 *usbdata = &data[7];
			/* USB Device Descriptor v2: idVendor */
			pr_attr("idVendor", "0x%04x",
				WORD(&usbdata[0x1]));
			/* USB Device Descriptor v2: idProduct */
			pr_attr("idProduct", "0x%04x",
				WORD(&usbdata[0x3]));

			/*
			 * USB Serial number is here, but its useless, don't
			 * bother decoding it
			 */

			/* USB Device Descriptor v2: MAC Address */
			pr_attr("MAC Address", "%02x:%02x:%02x:%02x:%02x:%02x",
				usbdata[0x6], usbdata[0x7], usbdata[0x8],
				usbdata[0x9], usbdata[0xa], usbdata[0xb]);

			/* DSP0270 v1.3.0 support */
			if (len >= 0x11)
			{
				/* USB Device Descriptor v2: Device Characteristics */
				pr_list_start("Device Characteristics", NULL);
				dmi_device_characteristics(WORD(&usbdata[0xc]));
				pr_list_end();

				/* USB Device Descriptor v2: Credential Bootstrapping Handle */
				if (WORD(&usbdata[0x0c]) & 0x1)
				{
					pr_attr("Credential Bootstrapping Handle", "0x%04x",
							WORD(&usbdata[0xe]));
				}
			}
		}
		else if (type == 0x5 && len >= 0x14)
		{
			/* PCI Device Type v2 - Need at least 19 bytes */
			u8 *pcidata = &data[0x7];
			/* PCI Device Descriptor v2: VendorID */
			pr_attr("VendorID", "0x%04x",
				WORD(&pcidata[0x1]));
			/* PCI Device Descriptor v2: DeviceID */
			pr_attr("DeviceID", "0x%04x",
				WORD(&pcidata[0x3]));
			/* PCI Device Descriptor v2: PCI SubvendorID */
			pr_attr("SubVendorID", "0x%04x",
				WORD(&pcidata[0x5]));
			/* PCI Device Descriptor v2: PCI SubdeviceID */
			pr_attr("SubDeviceID", "0x%04x",
				WORD(&pcidata[0x7]));
			/* PCI Device Descriptor v2: MAC Address */
			pr_attr("MAC Address", "%02x:%02x:%02x:%02x:%02x:%02x",
				pcidata[0x9], pcidata[0xa], pcidata[0xb],
				pcidata[0xc], pcidata[0xd], pcidata[0xe]);
			/* PCI Device Descriptor v2:
			 *		Segment Group Number, Bus Number, Device/Function Number
			 */
			dmi_slot_segment_bus_func(WORD(&pcidata[0xf]), pcidata[0x11], pcidata[0x12]);

			/* DSP0270 v1.3.0 support */
			if (len >= 0x18)
			{
				/* PCI Device Descriptor v2: Device Characteristics */
				pr_list_start("Device Characteristics", NULL);
				dmi_device_characteristics(WORD(&pcidata[0x13]) );
				pr_list_end();
				/* PCI Device Descriptor v2: Credential Bootstrapping Handle */
				if (WORD(&pcidata[0x13]) & 0x1)
				{
					pr_attr("Credential Bootstrapping Handle", "0x%04x",
							WORD(&pcidata[0x15]));
				}
			}
		}
		else if (type >= 0x80 && len >= 5)
		{
			/* OEM Device Type - Need at least 4 bytes */
			u8 *oemdata = &data[0x7];
			/* OEM Device Descriptor: IANA */
			pr_attr("Vendor ID", "0x%02x:0x%02x:0x%02x:0x%02x",
				oemdata[0x0], oemdata[0x1],
				oemdata[0x2], oemdata[0x3]);
		}
		/* Don't mess with unknown types for now */
	}

	/*
	 * DSP0270: 8.2 and 8.4: Protocol record count and protocol records
	 * Move to the Protocol Count.
	 */
	data = &data[total_read];

	/*
	 * We've validated up to 0x6 + len bytes, but we need to validate
	 * the next byte below, the count value.
	 */
	total_read++;
	if (total_read > h->length)
	{
		fprintf(stderr,
			"Total read length %d exceeds total structure length %d (handle 0x%04hx)\n",
			total_read, h->length, h->handle);
		return;
	}

	/* Get the protocol records count */
	count = data[0x0];
	if (count)
	{
		u8 *rec = &data[0x1];
		for (i = 0; i < count; i++)
		{
			/*
			 * Need to ensure that this record doesn't overrun
			 * the total length of the type 42 struct.  Note the +2
			 * is added for the two leading bytes of a protocol
			 * record representing the type and length bytes.
			 */
			total_read += rec[1] + 2;
			if (total_read > h->length)
			{
				fprintf(stderr,
					"Total read length %d exceeds total structure length %d (handle 0x%04hx, record %d)\n",
					total_read, h->length, h->handle, i + 1);
				return;
			}

			dmi_parse_protocol_record(rec);

			/*
			 * DSP0270: 8.4.1
			 * Each record is rec[1] bytes long, starting at the
			 * data byte immediately following the length field.
			 * That means we need to add the byte for the rec id,
			 * the byte for the length field, and the value of the
			 * length field itself.
			 */
			rec += rec[1] + 2;
		}
	}
}

/*
 * 7.44 TPM Device (Type 43)
 */

static void dmi_tpm_vendor_id(const u8 *p)
{
	char vendor_id[5];
	int i;

	/* ASCII filtering */
	for (i = 0; i < 4 && p[i] != 0; i++)
	{
		if (p[i] < 32 || p[i] >= 127)
			vendor_id[i] = '.';
		else
			vendor_id[i] = p[i];
	}

	/* Terminate the string */
	vendor_id[i] = '\0';

	pr_attr("Vendor ID", "%s", vendor_id);
}

static void dmi_tpm_characteristics(u64 code)
{
	/* 7.1.1 */
	static const char *characteristics[] = {
		"TPM Device characteristics not supported", /* 2 */
		"Family configurable via firmware update",
		"Family configurable via platform software support",
		"Family configurable via OEM proprietary mechanism" /* 5 */
	};
	int i;

	/*
	 * This isn't very clear what this bit is supposed to mean
	 */
	if (code.l & (1 << 2))
	{
		pr_list_item("%s", characteristics[0]);
		return;
	}

	for (i = 3; i <= 5; i++)
		if (code.l & (1 << i))
			pr_list_item("%s", characteristics[i - 2]);
}

/*
 * 7.46 Firmware Inventory Information (Type 45)
 */

static void dmi_firmware_characteristics(u16 code)
{
	/* 7.46.3 */
	static const char *characteristics[] = {
		"Updatable", /* 0 */
		"Write-Protect" /* 1 */
	};
	int i;

	for (i = 0; i <= 1; i++)
		pr_list_item("%s: %s", characteristics[i],
			     (code & (1 << i)) ? "Yes" : "No");
}

static const char *dmi_firmware_state(u8 code)
{
	/* 7.46.4 */
	static const char *state[] = {
		"Other", /* 0x01 */
		"Unknown",
		"Disabled",
		"Enabled",
		"Absent",
		"Stand-by Offline",
		"Stand-by Spare",
		"Unavailable Offline" /* 0x08 */
	};

	if (code >= 0x01 && code <= 0x08)
		return state[code - 0x01];
	return out_of_spec;
}

static void dmi_firmware_components(u8 count, const u8 *p)
{
	int i;

	pr_list_start("Associated Components", "%u", count);
	for (i = 0; i < count; i++)
		pr_list_item("0x%04X", WORD(p + sizeof(u16) * i));
	pr_list_end();
}

/*
 * Main
 */

static void dmi_decode(const struct dmi_header *h, u16 ver)
{
	const u8 *data = h->data;

	/*
	 * Note: DMI types 37 and 42 are untested
	 */
	switch (h->type)
	{
		case 0: /* 7.1 BIOS Information */
			pr_handle_name("BIOS Information");
			if (h->length < 0x12) break;
			pr_attr("Vendor", "%s",
				dmi_string(h, data[0x04]));
			pr_attr("Version", "%s",
				dmi_string(h, data[0x05]));
			pr_attr("Release Date", "%s",
				dmi_string(h, data[0x08]));
			/*
			 * On IA-64 and UEFI-based systems, the BIOS base
			 * address will read 0 because there is no BIOS. Skip
			 * the base address and the runtime size in this case.
			 */
			if (WORD(data + 0x06) != 0)
			{
				pr_attr("Address", "0x%04X0",
					WORD(data + 0x06));
				dmi_bios_runtime_size((0x10000 - WORD(data + 0x06)) << 4);
			}
			dmi_bios_rom_size(data[0x09], h->length < 0x1A ? 16 : WORD(data + 0x18));
			pr_list_start("Characteristics", NULL);
			dmi_bios_characteristics(QWORD(data + 0x0A));
			pr_list_end();
			if (h->length < 0x13) break;
			dmi_bios_characteristics_x1(data[0x12]);
			if (h->length < 0x14) break;
			dmi_bios_characteristics_x2(data[0x13]);
			if (h->length < 0x18) break;
			if (data[0x14] != 0xFF && data[0x15] != 0xFF)
				pr_attr("BIOS Revision", "%u.%u",
					data[0x14], data[0x15]);
			if (data[0x16] != 0xFF && data[0x17] != 0xFF)
				pr_attr("Firmware Revision", "%u.%u",
					data[0x16], data[0x17]);
			break;

		case 1: /* 7.2 System Information */
			pr_handle_name("System Information");
			if (h->length < 0x08) break;
			pr_attr("Manufacturer", "%s",
				dmi_string(h, data[0x04]));
			pr_attr("Product Name", "%s",
				dmi_string(h, data[0x05]));
			pr_attr("Version", "%s",
				dmi_string(h, data[0x06]));
			pr_attr("Serial Number", "%s",
				dmi_string(h, data[0x07]));
			if (h->length < 0x19) break;
			dmi_system_uuid(pr_attr, "UUID", data + 0x08, ver);
			pr_attr("Wake-up Type", "%s",
				dmi_system_wake_up_type(data[0x18]));
			if (h->length < 0x1B) break;
			pr_attr("SKU Number", "%s",
				dmi_string(h, data[0x19]));
			pr_attr("Family", "%s",
				dmi_string(h, data[0x1A]));
			break;

		case 2: /* 7.3 Base Board Information */
			pr_handle_name("Base Board Information");
			if (h->length < 0x08) break;
			pr_attr("Manufacturer", "%s",
				dmi_string(h, data[0x04]));
			pr_attr("Product Name", "%s",
				dmi_string(h, data[0x05]));
			pr_attr("Version", "%s",
				dmi_string(h, data[0x06]));
			pr_attr("Serial Number", "%s",
				dmi_string(h, data[0x07]));
			if (h->length < 0x09) break;
			pr_attr("Asset Tag", "%s",
				dmi_string(h, data[0x08]));
			if (h->length < 0x0A) break;
			dmi_base_board_features(data[0x09]);
			if (h->length < 0x0E) break;
			pr_attr("Location In Chassis", "%s",
				dmi_string(h, data[0x0A]));
			if (!(opt.flags & FLAG_QUIET))
				pr_attr("Chassis Handle", "0x%04X",
					WORD(data + 0x0B));
			pr_attr("Type", "%s",
				dmi_base_board_type(data[0x0D]));
			if (h->length < 0x0F) break;
			if (h->length < 0x0F + data[0x0E] * sizeof(u16)) break;
			if (!(opt.flags & FLAG_QUIET))
				dmi_base_board_handles(data[0x0E], data + 0x0F);
			break;

		case 3: /* 7.4 Chassis Information */
			pr_handle_name("Chassis Information");
			if (h->length < 0x09) break;
			pr_attr("Manufacturer", "%s",
				dmi_string(h, data[0x04]));
			pr_attr("Type", "%s",
				dmi_chassis_type(data[0x05]));
			pr_attr("Lock", "%s",
				dmi_chassis_lock(data[0x05] >> 7));
			pr_attr("Version", "%s",
				dmi_string(h, data[0x06]));
			pr_attr("Serial Number", "%s",
				dmi_string(h, data[0x07]));
			pr_attr("Asset Tag", "%s",
				dmi_string(h, data[0x08]));
			if (h->length < 0x0D) break;
			pr_attr("Boot-up State", "%s",
				dmi_chassis_state(data[0x09]));
			pr_attr("Power Supply State", "%s",
				dmi_chassis_state(data[0x0A]));
			pr_attr("Thermal State", "%s",
				dmi_chassis_state(data[0x0B]));
			pr_attr("Security Status", "%s",
				dmi_chassis_security_status(data[0x0C]));
			if (h->length < 0x11) break;
			pr_attr("OEM Information", "0x%08X",
				DWORD(data + 0x0D));
			if (h->length < 0x13) break;
			dmi_chassis_height(data[0x11]);
			dmi_chassis_power_cords(data[0x12]);
			if (h->length < 0x15) break;
			if (h->length < 0x15 + data[0x13] * data[0x14]) break;
			dmi_chassis_elements(data[0x13], data[0x14], data + 0x15);
			if (h->length < 0x16 + data[0x13] * data[0x14]) break;
			pr_attr("SKU Number", "%s",
				dmi_string(h, data[0x15 + data[0x13] * data[0x14]]));
			break;

		case 4: /* 7.5 Processor Information */
			pr_handle_name("Processor Information");
			if (h->length < 0x1A) break;
			pr_attr("Socket Designation", "%s",
				dmi_string(h, data[0x04]));
			pr_attr("Type", "%s",
				dmi_processor_type(data[0x05]));
			pr_attr("Family", "%s",
				dmi_processor_family(h, ver));
			pr_attr("Manufacturer", "%s",
				dmi_string(h, data[0x07]));
			dmi_processor_id(h);
			pr_attr("Version", "%s",
				dmi_string(h, data[0x10]));
			dmi_processor_voltage("Voltage", data[0x11]);
			dmi_processor_frequency("External Clock", data + 0x12);
			dmi_processor_frequency("Max Speed", data + 0x14);
			dmi_processor_frequency("Current Speed", data + 0x16);
			if (data[0x18] & (1 << 6))
				pr_attr("Status", "Populated, %s",
					dmi_processor_status(data[0x18] & 0x07));
			else
				pr_attr("Status", "Unpopulated");
			pr_attr("Upgrade", "%s",
				dmi_processor_upgrade(data[0x19]));
			if (h->length < 0x20) break;
			if (!(opt.flags & FLAG_QUIET))
			{
				dmi_processor_cache("L1 Cache Handle",
						    WORD(data + 0x1A), "L1", ver);
				dmi_processor_cache("L2 Cache Handle",
						    WORD(data + 0x1C), "L2", ver);
				dmi_processor_cache("L3 Cache Handle",
						    WORD(data + 0x1E), "L3", ver);
			}
			if (h->length < 0x23) break;
			pr_attr("Serial Number", "%s",
				dmi_string(h, data[0x20]));
			pr_attr("Asset Tag", "%s",
				dmi_string(h, data[0x21]));
			pr_attr("Part Number", "%s",
				dmi_string(h, data[0x22]));
			if (h->length < 0x28) break;
			if (data[0x23] != 0)
				pr_attr("Core Count", "%u",
					h->length >= 0x2C && data[0x23] == 0xFF ?
					WORD(data + 0x2A) : data[0x23]);
			if (data[0x24] != 0)
				pr_attr("Core Enabled", "%u",
					h->length >= 0x2E && data[0x24] == 0xFF ?
					WORD(data + 0x2C) : data[0x24]);
			if (data[0x25] != 0)
				pr_attr("Thread Count", "%u",
					h->length >= 0x30 && data[0x25] == 0xFF ?
					WORD(data + 0x2E) : data[0x25]);
			if (h->length >= 0x32 && WORD(data + 0x30) != 0)
				pr_attr("Thread Enabled", "%u",
					WORD(data + 0x30));
			dmi_processor_characteristics("Characteristics",
						      WORD(data + 0x26));
			break;

		case 5: /* 7.6 Memory Controller Information */
			pr_handle_name("Memory Controller Information");
			if (h->length < 0x0F) break;
			pr_attr("Error Detecting Method", "%s",
				dmi_memory_controller_ed_method(data[0x04]));
			dmi_memory_controller_ec_capabilities("Error Correcting Capabilities",
							      data[0x05]);
			pr_attr("Supported Interleave", "%s",
				dmi_memory_controller_interleave(data[0x06]));
			pr_attr("Current Interleave", "%s",
				dmi_memory_controller_interleave(data[0x07]));
			pr_attr("Maximum Memory Module Size", "%u MB",
				1 << data[0x08]);
			pr_attr("Maximum Total Memory Size", "%u MB",
				data[0x0E] * (1 << data[0x08]));
			dmi_memory_controller_speeds("Supported Speeds",
						     WORD(data + 0x09));
			dmi_memory_module_types("Supported Memory Types",
						WORD(data + 0x0B), 0);
			dmi_processor_voltage("Memory Module Voltage", data[0x0D]);
			if (h->length < 0x0F + data[0x0E] * sizeof(u16)) break;
			dmi_memory_controller_slots(data[0x0E], data + 0x0F);
			if (h->length < 0x10 + data[0x0E] * sizeof(u16)) break;
			dmi_memory_controller_ec_capabilities("Enabled Error Correcting Capabilities",
							      data[0x0F + data[0x0E] * sizeof(u16)]);
			break;

		case 6: /* 7.7 Memory Module Information */
			pr_handle_name("Memory Module Information");
			if (h->length < 0x0C) break;
			pr_attr("Socket Designation", "%s",
				dmi_string(h, data[0x04]));
			dmi_memory_module_connections(data[0x05]);
			dmi_memory_module_speed("Current Speed", data[0x06]);
			dmi_memory_module_types("Type", WORD(data + 0x07), 1);
			dmi_memory_module_size("Installed Size", data[0x09]);
			dmi_memory_module_size("Enabled Size", data[0x0A]);
			dmi_memory_module_error(data[0x0B]);
			break;

		case 7: /* 7.8 Cache Information */
			pr_handle_name("Cache Information");
			if (h->length < 0x0F) break;
			pr_attr("Socket Designation", "%s",
				dmi_string(h, data[0x04]));
			pr_attr("Configuration", "%s, %s, Level %u",
				WORD(data + 0x05) & 0x0080 ? "Enabled" : "Disabled",
				WORD(data + 0x05) & 0x0008 ? "Socketed" : "Not Socketed",
				(WORD(data + 0x05) & 0x0007) + 1);
			pr_attr("Operational Mode", "%s",
				dmi_cache_mode((WORD(data + 0x05) >> 8) & 0x0003));
			pr_attr("Location", "%s",
				dmi_cache_location((WORD(data + 0x05) >> 5) & 0x0003));
			if (h->length >= 0x1B)
				dmi_cache_size_2("Installed Size", DWORD(data + 0x17));
			else
				dmi_cache_size("Installed Size", WORD(data + 0x09));
			if (h->length >= 0x17)
				dmi_cache_size_2("Maximum Size", DWORD(data + 0x13));
			else
				dmi_cache_size("Maximum Size", WORD(data + 0x07));
			dmi_cache_types("Supported SRAM Types", WORD(data + 0x0B), 0);
			dmi_cache_types("Installed SRAM Type", WORD(data + 0x0D), 1);
			if (h->length < 0x13) break;
			dmi_memory_module_speed("Speed", data[0x0F]);
			pr_attr("Error Correction Type", "%s",
				dmi_cache_ec_type(data[0x10]));
			pr_attr("System Type", "%s",
				dmi_cache_type(data[0x11]));
			pr_attr("Associativity", "%s",
				dmi_cache_associativity(data[0x12]));
			break;

		case 8: /* 7.9 Port Connector Information */
			pr_handle_name("Port Connector Information");
			if (h->length < 0x09) break;
			pr_attr("Internal Reference Designator", "%s",
				dmi_string(h, data[0x04]));
			pr_attr("Internal Connector Type", "%s",
				dmi_port_connector_type(data[0x05]));
			pr_attr("External Reference Designator", "%s",
				dmi_string(h, data[0x06]));
			pr_attr("External Connector Type", "%s",
				dmi_port_connector_type(data[0x07]));
			pr_attr("Port Type", "%s",
				dmi_port_type(data[0x08]));
			break;

		case 9: /* 7.10 System Slots */
			pr_handle_name("System Slot Information");
			if (h->length < 0x0C) break;
			pr_attr("Designation", "%s",
				dmi_string(h, data[0x04]));
			pr_attr("Type", "%s", dmi_slot_type(data[0x05]));
			pr_attr("Data Bus Width", "%s", dmi_slot_bus_width(data[0x06]));
			pr_attr("Current Usage", "%s",
				dmi_slot_current_usage(data[0x07]));
			pr_attr("Length", "%s",
				dmi_slot_length(data[0x08]));
			dmi_slot_id(data[0x09], data[0x0A], data[0x05]);
			if (h->length < 0x0D)
				dmi_slot_characteristics("Characteristics", data[0x0B], 0x00);
			else
				dmi_slot_characteristics("Characteristics", data[0x0B], data[0x0C]);
			if (h->length < 0x11) break;
			dmi_slot_segment_bus_func(WORD(data + 0x0D), data[0x0F], data[0x10]);
			if (h->length < 0x13) break;
			pr_attr("Data Bus Width (Base)", "%u", data[0x11]);
			pr_attr("Peer Devices", "%u", data[0x12]);
			if (h->length < 0x13 + data[0x12] * 5) break;
			dmi_slot_peers(data[0x12], data + 0x13);
			if (h->length < 0x17 + data[0x12] * 5) break;
			dmi_slot_information(data[0x05], data[0x13 + data[0x12] * 5]);
			dmi_slot_physical_width(data[0x14 + data[0x12] * 5]);
			dmi_slot_pitch(WORD(data + 0x15 + data[0x12] * 5));
			if (h->length < 0x18 + data[0x12] * 5) break;
			pr_attr("Height", "%s",
				dmi_slot_height(data[0x17 + data[0x12] * 5]));
			break;

		case 10: /* 7.11 On Board Devices Information */
			dmi_on_board_devices(h);
			break;

		case 11: /* 7.12 OEM Strings */
			pr_handle_name("OEM Strings");
			if (h->length < 0x05) break;
			dmi_oem_strings(h);
			break;

		case 12: /* 7.13 System Configuration Options */
			pr_handle_name("System Configuration Options");
			if (h->length < 0x05) break;
			dmi_system_configuration_options(h);
			break;

		case 13: /* 7.14 BIOS Language Information */
			pr_handle_name("BIOS Language Information");
			if (h->length < 0x16) break;
			if (ver >= 0x0201)
			{
				pr_attr("Language Description Format", "%s",
					dmi_bios_language_format(data[0x05]));
			}
			pr_list_start("Installable Languages", "%u", data[0x04]);
			dmi_bios_languages(h);
			pr_list_end();
			pr_attr("Currently Installed Language", "%s",
				dmi_string(h, data[0x15]));
			break;

		case 14: /* 7.15 Group Associations */
			pr_handle_name("Group Associations");
			if (h->length < 0x05) break;
			pr_attr("Name", "%s",
				dmi_string(h, data[0x04]));
			pr_list_start("Items", "%u",
				(h->length - 0x05) / 3);
			dmi_group_associations_items((h->length - 0x05) / 3, data + 0x05);
			pr_list_end();
			break;

		case 15: /* 7.16 System Event Log */
			pr_handle_name("System Event Log");
			if (h->length < 0x14) break;
			pr_attr("Area Length", "%u bytes",
				WORD(data + 0x04));
			pr_attr("Header Start Offset", "0x%04X",
				WORD(data + 0x06));
			if (WORD(data + 0x08) - WORD(data + 0x06))
				pr_attr("Header Length", "%u byte%s",
					WORD(data + 0x08) - WORD(data + 0x06),
					WORD(data + 0x08) - WORD(data + 0x06) > 1 ? "s" : "");
			pr_attr("Data Start Offset", "0x%04X",
				WORD(data + 0x08));
			pr_attr("Access Method", "%s",
				dmi_event_log_method(data[0x0A]));
			dmi_event_log_address(data[0x0A], data + 0x10);
			dmi_event_log_status(data[0x0B]);
			pr_attr("Change Token", "0x%08X",
				DWORD(data + 0x0C));
			if (h->length < 0x17) break;
			pr_attr("Header Format", "%s",
				dmi_event_log_header_type(data[0x14]));
			pr_attr("Supported Log Type Descriptors", "%u",
				data[0x15]);
			if (h->length < 0x17 + data[0x15] * data[0x16]) break;
			dmi_event_log_descriptors(data[0x15], data[0x16], data + 0x17);
			break;

		case 16: /* 7.17 Physical Memory Array */
			pr_handle_name("Physical Memory Array");
			if (h->length < 0x0F) break;
			pr_attr("Location", "%s",
				dmi_memory_array_location(data[0x04]));
			pr_attr("Use", "%s",
				dmi_memory_array_use(data[0x05]));
			pr_attr("Error Correction Type", "%s",
				dmi_memory_array_ec_type(data[0x06]));
			if (DWORD(data + 0x07) == 0x80000000)
			{
				if (h->length < 0x17)
					pr_attr("Maximum Capacity", "Unknown");
				else
					dmi_print_memory_size("Maximum Capacity",
							      QWORD(data + 0x0F), 0);
			}
			else
			{
				u64 capacity;

				capacity.h = 0;
				capacity.l = DWORD(data + 0x07);
				dmi_print_memory_size("Maximum Capacity",
						      capacity, 1);
			}
			if (!(opt.flags & FLAG_QUIET))
				dmi_memory_array_error_handle(WORD(data + 0x0B));
			pr_attr("Number Of Devices", "%u",
				WORD(data + 0x0D));
			break;

		case 17: /* 7.18 Memory Device */
			pr_handle_name("Memory Device");
			if (h->length < 0x15) break;
			if (!(opt.flags & FLAG_QUIET))
			{
				pr_attr("Array Handle", "0x%04X",
					WORD(data + 0x04));
				dmi_memory_array_error_handle(WORD(data + 0x06));
			}
			dmi_memory_device_width("Total Width", WORD(data + 0x08));
			dmi_memory_device_width("Data Width", WORD(data + 0x0A));
			if (h->length >= 0x20 && WORD(data + 0x0C) == 0x7FFF)
				dmi_memory_device_extended_size(DWORD(data + 0x1C));
			else
				dmi_memory_device_size(WORD(data + 0x0C));
			pr_attr("Form Factor", "%s",
				dmi_memory_device_form_factor(data[0x0E]));
			dmi_memory_device_set(data[0x0F]);
			pr_attr("Locator", "%s",
				dmi_string(h, data[0x10]));
			pr_attr("Bank Locator", "%s",
				dmi_string(h, data[0x11]));
			pr_attr("Type", "%s",
				dmi_memory_device_type(data[0x12]));
			dmi_memory_device_type_detail(WORD(data + 0x13));
			if (h->length < 0x17) break;
			/* If no module is present, the remaining fields are irrelevant */
			if (WORD(data + 0x0C) == 0 && !(opt.flags & FLAG_NO_QUIRKS))
				break;
			dmi_memory_device_speed("Speed", WORD(data + 0x15),
						h->length >= 0x5C ?
						DWORD(data + 0x54) : 0);
			if (h->length < 0x1B) break;
			pr_attr("Manufacturer", "%s",
				dmi_string(h, data[0x17]));
			pr_attr("Serial Number", "%s",
				dmi_string(h, data[0x18]));
			pr_attr("Asset Tag", "%s",
				dmi_string(h, data[0x19]));
			pr_attr("Part Number", "%s",
				dmi_string(h, data[0x1A]));
			if (h->length < 0x1C) break;
			if ((data[0x1B] & 0x0F) == 0)
				pr_attr("Rank", "Unknown");
			else
				pr_attr("Rank", "%u", data[0x1B] & 0x0F);
			if (h->length < 0x22) break;
			dmi_memory_device_speed("Configured Memory Speed",
						WORD(data + 0x20),
						h->length >= 0x5C ?
						DWORD(data + 0x58) : 0);
			if (h->length < 0x28) break;
			dmi_memory_voltage_value("Minimum Voltage",
						 WORD(data + 0x22));
			dmi_memory_voltage_value("Maximum Voltage",
						 WORD(data + 0x24));
			dmi_memory_voltage_value("Configured Voltage",
						 WORD(data + 0x26));
			if (h->length < 0x34) break;
			dmi_memory_technology(data[0x28]);
			dmi_memory_operating_mode_capability(WORD(data + 0x29));
			pr_attr("Firmware Version", "%s",
				dmi_string(h, data[0x2B]));
			dmi_memory_manufacturer_id("Module Manufacturer ID",
						   WORD(data + 0x2C));
			dmi_memory_product_id("Module Product ID",
					      WORD(data + 0x2E));
			dmi_memory_manufacturer_id("Memory Subsystem Controller Manufacturer ID",
						   WORD(data + 0x30));
			dmi_memory_product_id("Memory Subsystem Controller Product ID",
					      WORD(data + 0x32));
			if (h->length < 0x3C) break;
			dmi_memory_size("Non-Volatile Size", QWORD(data + 0x34));
			if (h->length < 0x44) break;
			dmi_memory_size("Volatile Size", QWORD(data + 0x3C));
			if (h->length < 0x4C) break;
			dmi_memory_size("Cache Size", QWORD(data + 0x44));
			if (h->length < 0x54) break;
			dmi_memory_size("Logical Size", QWORD(data + 0x4C));
			if (h->length < 0x64) break;
			dmi_memory_manufacturer_id("PMIC0 Manufacturer ID",
						   WORD(data + 0x5C));
			dmi_memory_revision("PMIC0", WORD(data + 0x5E),
					    data[0x12]);
			dmi_memory_manufacturer_id("RCD Manufacturer ID",
						   WORD(data + 0x60));
			dmi_memory_revision("RCD", WORD(data + 0x62),
					    data[0x12]);
			break;

		case 18: /* 7.19 32-bit Memory Error Information */
			pr_handle_name("32-bit Memory Error Information");
			if (h->length < 0x17) break;
			pr_attr("Type", "%s",
				dmi_memory_error_type(data[0x04]));
			pr_attr("Granularity", "%s",
				dmi_memory_error_granularity(data[0x05]));
			pr_attr("Operation", "%s",
				dmi_memory_error_operation(data[0x06]));
			dmi_memory_error_syndrome(DWORD(data + 0x07));
			dmi_32bit_memory_error_address("Memory Array Address",
						       DWORD(data + 0x0B));
			dmi_32bit_memory_error_address("Device Address",
						       DWORD(data + 0x0F));
			dmi_32bit_memory_error_address("Resolution",
						       DWORD(data + 0x13));
			break;

		case 19: /* 7.20 Memory Array Mapped Address */
			pr_handle_name("Memory Array Mapped Address");
			if (h->length < 0x0F) break;
			if (h->length >= 0x1F && DWORD(data + 0x04) == 0xFFFFFFFF)
			{
				u64 start, end;

				start = QWORD(data + 0x0F);
				end = QWORD(data + 0x17);

				pr_attr("Starting Address", "0x%08X%08Xk",
					start.h, start.l);
				pr_attr("Ending Address", "0x%08X%08Xk",
					end.h, end.l);
				dmi_mapped_address_extended_size(start, end);
			}
			else
			{
				pr_attr("Starting Address", "0x%08X%03X",
					DWORD(data + 0x04) >> 2,
					(DWORD(data + 0x04) & 0x3) << 10);
				pr_attr("Ending Address", "0x%08X%03X",
					DWORD(data + 0x08) >> 2,
					((DWORD(data + 0x08) & 0x3) << 10) + 0x3FF);
				dmi_mapped_address_size(DWORD(data + 0x08) - DWORD(data + 0x04) + 1);
			}
			if (!(opt.flags & FLAG_QUIET))
				pr_attr("Physical Array Handle", "0x%04X",
					WORD(data + 0x0C));
			pr_attr("Partition Width", "%u",
				data[0x0E]);
			break;

		case 20: /* 7.21 Memory Device Mapped Address */
			pr_handle_name("Memory Device Mapped Address");
			if (h->length < 0x13) break;
			if (h->length >= 0x23 && DWORD(data + 0x04) == 0xFFFFFFFF)
			{
				u64 start, end;

				start = QWORD(data + 0x13);
				end = QWORD(data + 0x1B);

				pr_attr("Starting Address", "0x%08X%08Xk",
					start.h, start.l);
				pr_attr("Ending Address", "0x%08X%08Xk",
					end.h, end.l);
				dmi_mapped_address_extended_size(start, end);
			}
			else
			{
				pr_attr("Starting Address", "0x%08X%03X",
					DWORD(data + 0x04) >> 2,
					(DWORD(data + 0x04) & 0x3) << 10);
				pr_attr("Ending Address", "0x%08X%03X",
					DWORD(data + 0x08) >> 2,
					((DWORD(data + 0x08) & 0x3) << 10) + 0x3FF);
				dmi_mapped_address_size(DWORD(data + 0x08) - DWORD(data + 0x04) + 1);
			}
			if (!(opt.flags & FLAG_QUIET))
			{
				pr_attr("Physical Device Handle", "0x%04X",
					WORD(data + 0x0C));
				pr_attr("Memory Array Mapped Address Handle", "0x%04X",
					WORD(data + 0x0E));
			}
			dmi_mapped_address_row_position(data[0x10]);
			dmi_mapped_address_interleave_position(data[0x11]);
			dmi_mapped_address_interleaved_data_depth(data[0x12]);
			break;

		case 21: /* 7.22 Built-in Pointing Device */
			pr_handle_name("Built-in Pointing Device");
			if (h->length < 0x07) break;
			pr_attr("Type", "%s",
				dmi_pointing_device_type(data[0x04]));
			pr_attr("Interface", "%s",
				dmi_pointing_device_interface(data[0x05]));
			pr_attr("Buttons", "%u",
				data[0x06]);
			break;

		case 22: /* 7.23 Portable Battery */
			pr_handle_name("Portable Battery");
			if (h->length < 0x10) break;
			pr_attr("Location", "%s",
				dmi_string(h, data[0x04]));
			pr_attr("Manufacturer", "%s",
				dmi_string(h, data[0x05]));
			if (data[0x06] || h->length < 0x1A)
				pr_attr("Manufacture Date", "%s",
					dmi_string(h, data[0x06]));
			if (data[0x07] || h->length < 0x1A)
				pr_attr("Serial Number", "%s",
					dmi_string(h, data[0x07]));
			pr_attr("Name", "%s",
				dmi_string(h, data[0x08]));
			if (data[0x09] != 0x02 || h->length < 0x1A)
				pr_attr("Chemistry", "%s",
					dmi_battery_chemistry(data[0x09]));
			if (h->length < 0x16)
				dmi_battery_capacity(WORD(data + 0x0A), 1);
			else
				dmi_battery_capacity(WORD(data + 0x0A), data[0x15]);
			dmi_battery_voltage(WORD(data + 0x0C));
			pr_attr("SBDS Version", "%s",
				dmi_string(h, data[0x0E]));
			dmi_battery_maximum_error(data[0x0F]);
			if (h->length < 0x1A) break;
			if (data[0x07] == 0)
				pr_attr("SBDS Serial Number", "%04X",
					WORD(data + 0x10));
			if (data[0x06] == 0)
				pr_attr("SBDS Manufacture Date", "%u-%02u-%02u",
					1980 + (WORD(data + 0x12) >> 9),
					(WORD(data + 0x12) >> 5) & 0x0F,
					WORD(data + 0x12) & 0x1F);
			if (data[0x09] == 0x02)
				pr_attr("SBDS Chemistry", "%s",
					dmi_string(h, data[0x14]));
			pr_attr("OEM-specific Information", "0x%08X",
				DWORD(data + 0x16));
			break;

		case 23: /* 7.24 System Reset */
			pr_handle_name("System Reset");
			if (h->length < 0x0D) break;
			pr_attr("Status", "%s",
				data[0x04] & (1 << 0) ? "Enabled" : "Disabled");
			pr_attr("Watchdog Timer", "%s",
				data[0x04] & (1 << 5) ? "Present" : "Not Present");
			if (!(data[0x04] & (1 << 5)))
				break;
			pr_attr("Boot Option", "%s",
				dmi_system_reset_boot_option((data[0x04] >> 1) & 0x3));
			pr_attr("Boot Option On Limit", "%s",
				dmi_system_reset_boot_option((data[0x04] >> 3) & 0x3));
			dmi_system_reset_count("Reset Count", WORD(data + 0x05));
			dmi_system_reset_count("Reset Limit", WORD(data + 0x07));
			dmi_system_reset_timer("Timer Interval", WORD(data + 0x09));
			dmi_system_reset_timer("Timeout", WORD(data + 0x0B));
			break;

		case 24: /* 7.25 Hardware Security */
			pr_handle_name("Hardware Security");
			if (h->length < 0x05) break;
			pr_attr("Power-On Password Status", "%s",
				dmi_hardware_security_status(data[0x04] >> 6));
			pr_attr("Keyboard Password Status", "%s",
				dmi_hardware_security_status((data[0x04] >> 4) & 0x3));
			pr_attr("Administrator Password Status", "%s",
				dmi_hardware_security_status((data[0x04] >> 2) & 0x3));
			pr_attr("Front Panel Reset Status", "%s",
				dmi_hardware_security_status(data[0x04] & 0x3));
			break;

		case 25: /* 7.26 System Power Controls */
			pr_handle_name("System Power Controls");
			if (h->length < 0x09) break;
			dmi_power_controls_power_on(data + 0x04);
			break;

		case 26: /* 7.27 Voltage Probe */
			pr_handle_name("Voltage Probe");
			if (h->length < 0x14) break;
			pr_attr("Description", "%s",
				dmi_string(h, data[0x04]));
			pr_attr("Location", "%s",
				dmi_voltage_probe_location(data[0x05] & 0x1f));
			pr_attr("Status", "%s",
				dmi_probe_status(data[0x05] >> 5));
			dmi_voltage_probe_value("Maximum Value", WORD(data + 0x06));
			dmi_voltage_probe_value("Minimum Value", WORD(data + 0x08));
			dmi_voltage_probe_resolution(WORD(data + 0x0A));
			dmi_voltage_probe_value("Tolerance", WORD(data + 0x0C));
			dmi_probe_accuracy(WORD(data + 0x0E));
			pr_attr("OEM-specific Information", "0x%08X",
				DWORD(data + 0x10));
			if (h->length < 0x16) break;
			dmi_voltage_probe_value("Nominal Value", WORD(data + 0x14));
			break;

		case 27: /* 7.28 Cooling Device */
			pr_handle_name("Cooling Device");
			if (h->length < 0x0C) break;
			if (!(opt.flags & FLAG_QUIET) && WORD(data + 0x04) != 0xFFFF)
				pr_attr("Temperature Probe Handle", "0x%04X",
					WORD(data + 0x04));
			pr_attr("Type", "%s",
				dmi_cooling_device_type(data[0x06] & 0x1f));
			pr_attr("Status", "%s",
				dmi_probe_status(data[0x06] >> 5));
			if (data[0x07] != 0x00)
				pr_attr("Cooling Unit Group", "%u",
					data[0x07]);
			pr_attr("OEM-specific Information", "0x%08X",
				DWORD(data + 0x08));
			if (h->length < 0x0E) break;
			dmi_cooling_device_speed(WORD(data + 0x0C));
			if (h->length < 0x0F) break;
			pr_attr("Description", "%s", dmi_string(h, data[0x0E]));
			break;

		case 28: /* 7.29 Temperature Probe */
			pr_handle_name("Temperature Probe");
			if (h->length < 0x14) break;
			pr_attr("Description", "%s",
				dmi_string(h, data[0x04]));
			pr_attr("Location", "%s",
				dmi_temperature_probe_location(data[0x05] & 0x1F));
			pr_attr("Status", "%s",
				dmi_probe_status(data[0x05] >> 5));
			dmi_temperature_probe_value("Maximum Value",
						    WORD(data + 0x06));
			dmi_temperature_probe_value("Minimum Value",
						    WORD(data + 0x08));
			dmi_temperature_probe_resolution(WORD(data + 0x0A));
			dmi_temperature_probe_value("Tolerance",
						    WORD(data + 0x0C));
			dmi_probe_accuracy(WORD(data + 0x0E));
			pr_attr("OEM-specific Information", "0x%08X",
				DWORD(data + 0x10));
			if (h->length < 0x16) break;
			dmi_temperature_probe_value("Nominal Value",
						    WORD(data + 0x14));
			break;

		case 29: /* 7.30 Electrical Current Probe */
			pr_handle_name("Electrical Current Probe");
			if (h->length < 0x14) break;
			pr_attr("Description", "%s",
				dmi_string(h, data[0x04]));
			pr_attr("Location", "%s",
				dmi_voltage_probe_location(data[5] & 0x1F));
			pr_attr("Status", "%s",
				dmi_probe_status(data[0x05] >> 5));
			dmi_current_probe_value("Maximum Value",
						WORD(data + 0x06));
			dmi_current_probe_value("Minimum Value",
						WORD(data + 0x08));
			dmi_current_probe_resolution(WORD(data + 0x0A));
			dmi_current_probe_value("Tolerance",
						WORD(data + 0x0C));
			dmi_probe_accuracy(WORD(data + 0x0E));
			pr_attr("OEM-specific Information", "0x%08X",
				DWORD(data + 0x10));
			if (h->length < 0x16) break;
			dmi_current_probe_value("Nominal Value",
						WORD(data + 0x14));
			break;

		case 30: /* 7.31 Out-of-band Remote Access */
			pr_handle_name("Out-of-band Remote Access");
			if (h->length < 0x06) break;
			pr_attr("Manufacturer Name", "%s",
				dmi_string(h, data[0x04]));
			pr_attr("Inbound Connection", "%s",
				data[0x05] & (1 << 0) ? "Enabled" : "Disabled");
			pr_attr("Outbound Connection", "%s",
				data[0x05] & (1 << 1) ? "Enabled" : "Disabled");
			break;

		case 31: /* 7.32 Boot Integrity Services Entry Point */
			pr_handle_name("Boot Integrity Services Entry Point");
			if (h->length < 0x1C) break;
			pr_attr("Checksum", "%s",
				checksum(data, h->length) ? "OK" : "Invalid");
			pr_attr("16-bit Entry Point Address", "%04X:%04X",
				DWORD(data + 0x08) >> 16,
				DWORD(data + 0x08) & 0xFFFF);
			pr_attr("32-bit Entry Point Address", "0x%08X",
				DWORD(data + 0x0C));
			break;

		case 32: /* 7.33 System Boot Information */
			pr_handle_name("System Boot Information");
			if (h->length < 0x0B) break;
			pr_attr("Status", "%s",
				dmi_system_boot_status(data[0x0A]));
			break;

		case 33: /* 7.34 64-bit Memory Error Information */
			pr_handle_name("64-bit Memory Error Information");
			if (h->length < 0x1F) break;
			pr_attr("Type", "%s",
				dmi_memory_error_type(data[0x04]));
			pr_attr("Granularity", "%s",
				dmi_memory_error_granularity(data[0x05]));
			pr_attr("Operation", "%s",
				dmi_memory_error_operation(data[0x06]));
			dmi_memory_error_syndrome(DWORD(data + 0x07));
			dmi_64bit_memory_error_address("Memory Array Address",
						       QWORD(data + 0x0B));
			dmi_64bit_memory_error_address("Device Address",
						       QWORD(data + 0x13));
			dmi_32bit_memory_error_address("Resolution",
						       DWORD(data + 0x1B));
			break;

		case 34: /* 7.35 Management Device */
			pr_handle_name("Management Device");
			if (h->length < 0x0B) break;
			pr_attr("Description", "%s",
				dmi_string(h, data[0x04]));
			pr_attr("Type", "%s",
				dmi_management_device_type(data[0x05]));
			pr_attr("Address", "0x%08X",
				DWORD(data + 0x06));
			pr_attr("Address Type", "%s",
				dmi_management_device_address_type(data[0x0A]));
			break;

		case 35: /* 7.36 Management Device Component */
			pr_handle_name("Management Device Component");
			if (h->length < 0x0B) break;
			pr_attr("Description", "%s",
				dmi_string(h, data[0x04]));
			if (!(opt.flags & FLAG_QUIET))
			{
				pr_attr("Management Device Handle", "0x%04X",
					WORD(data + 0x05));
				pr_attr("Component Handle", "0x%04X",
					WORD(data + 0x07));
				if (WORD(data + 0x09) != 0xFFFF)
					pr_attr("Threshold Handle", "0x%04X",
						WORD(data + 0x09));
			}
			break;

		case 36: /* 7.37 Management Device Threshold Data */
			pr_handle_name("Management Device Threshold Data");
			if (h->length < 0x10) break;
			if (WORD(data + 0x04) != 0x8000)
				pr_attr("Lower Non-critical Threshold", "%d",
					(i16)WORD(data + 0x04));
			if (WORD(data + 0x06) != 0x8000)
				pr_attr("Upper Non-critical Threshold", "%d",
					(i16)WORD(data + 0x06));
			if (WORD(data + 0x08) != 0x8000)
				pr_attr("Lower Critical Threshold", "%d",
					(i16)WORD(data + 0x08));
			if (WORD(data + 0x0A) != 0x8000)
				pr_attr("Upper Critical Threshold", "%d",
					(i16)WORD(data + 0x0A));
			if (WORD(data + 0x0C) != 0x8000)
				pr_attr("Lower Non-recoverable Threshold", "%d",
					(i16)WORD(data + 0x0C));
			if (WORD(data + 0x0E) != 0x8000)
				pr_attr("Upper Non-recoverable Threshold", "%d",
					(i16)WORD(data + 0x0E));
			break;

		case 37: /* 7.38 Memory Channel */
			pr_handle_name("Memory Channel");
			if (h->length < 0x07) break;
			pr_attr("Type", "%s",
				dmi_memory_channel_type(data[0x04]));
			pr_attr("Maximal Load", "%u",
				data[0x05]);
			pr_attr("Devices", "%u",
				data[0x06]);
			if (h->length < 0x07 + 3 * data[0x06]) break;
			dmi_memory_channel_devices(data[0x06], data + 0x07);
			break;

		case 38: /* 7.39 IPMI Device Information */
			/*
			 * We use the word "Version" instead of "Revision", conforming to
			 * the IPMI specification.
			 */
			pr_handle_name("IPMI Device Information");
			if (h->length < 0x10) break;
			pr_attr("Interface Type", "%s",
				dmi_ipmi_interface_type(data[0x04]));
			pr_attr("Specification Version", "%u.%u",
				data[0x05] >> 4, data[0x05] & 0x0F);
			pr_attr("I2C Slave Address", "0x%02x",
				data[0x06] >> 1);
			if (data[0x07] != 0xFF)
				pr_attr("NV Storage Device Address", "%u",
					data[0x07]);
			else
				pr_attr("NV Storage Device", "Not Present");
			dmi_ipmi_base_address(data[0x04], data + 0x08,
				h->length < 0x11 ? 0 : (data[0x10] >> 4) & 1);
			if (h->length < 0x12) break;
			if (data[0x04] != 0x04)
			{
				pr_attr("Register Spacing", "%s",
					dmi_ipmi_register_spacing(data[0x10] >> 6));
				if (data[0x10] & (1 << 3))
				{
					pr_attr("Interrupt Polarity", "%s",
						data[0x10] & (1 << 1) ? "Active High" : "Active Low");
					pr_attr("Interrupt Trigger Mode", "%s",
						data[0x10] & (1 << 0) ? "Level" : "Edge");
				}
			}
			if (data[0x11] != 0x00)
			{
				pr_attr("Interrupt Number", "%u",
					data[0x11]);
			}
			break;

		case 39: /* 7.40 System Power Supply */
			pr_handle_name("System Power Supply");
			if (h->length < 0x10) break;
			if (data[0x04] != 0x00)
				pr_attr("Power Unit Group", "%u",
					data[0x04]);
			pr_attr("Location", "%s",
				dmi_string(h, data[0x05]));
			pr_attr("Name", "%s",
				dmi_string(h, data[0x06]));
			pr_attr("Manufacturer", "%s",
				dmi_string(h, data[0x07]));
			pr_attr("Serial Number", "%s",
				dmi_string(h, data[0x08]));
			pr_attr("Asset Tag", "%s",
				dmi_string(h, data[0x09]));
			pr_attr("Model Part Number", "%s",
				dmi_string(h, data[0x0A]));
			pr_attr("Revision", "%s",
				dmi_string(h, data[0x0B]));
			dmi_power_supply_power(WORD(data + 0x0C));
			if (WORD(data + 0x0E) & (1 << 1))
				pr_attr("Status", "Present, %s",
					dmi_power_supply_status((WORD(data + 0x0E) >> 7) & 0x07));
			else
				pr_attr("Status", "Not Present");
			pr_attr("Type", "%s",
				dmi_power_supply_type((WORD(data + 0x0E) >> 10) & 0x0F));
			pr_attr("Input Voltage Range Switching", "%s",
				dmi_power_supply_range_switching((WORD(data + 0x0E) >> 3) & 0x0F));
			pr_attr("Plugged", "%s",
				WORD(data + 0x0E) & (1 << 2) ? "No" : "Yes");
			pr_attr("Hot Replaceable", "%s",
				WORD(data + 0x0E) & (1 << 0) ? "Yes" : "No");
			if (h->length < 0x16) break;
			if (!(opt.flags & FLAG_QUIET))
			{
				if (WORD(data + 0x10) != 0xFFFF)
					pr_attr("Input Voltage Probe Handle", "0x%04X",
						WORD(data + 0x10));
				if (WORD(data + 0x12) != 0xFFFF)
					pr_attr("Cooling Device Handle", "0x%04X",
						WORD(data + 0x12));
				if (WORD(data + 0x14) != 0xFFFF)
					pr_attr("Input Current Probe Handle", "0x%04X",
						WORD(data + 0x14));
			}
			break;

		case 40: /* 7.41 Additional Information */
			if (h->length < 0x0B) break;
			if (opt.flags & FLAG_QUIET)
				return;
			dmi_additional_info(h);
			break;

		case 41: /* 7.42 Onboard Device Extended Information */
			pr_handle_name("Onboard Device");
			if (h->length < 0x0B) break;
			pr_attr("Reference Designation", "%s", dmi_string(h, data[0x04]));
			pr_attr("Type", "%s",
				dmi_on_board_devices_type(data[0x05] & 0x7F));
			pr_attr("Status", "%s",
				data[0x05] & 0x80 ? "Enabled" : "Disabled");
			pr_attr("Type Instance", "%u", data[0x06]);
			dmi_slot_segment_bus_func(WORD(data + 0x07), data[0x09], data[0x0A]);
			break;

		case 42: /* 7.43 Management Controller Host Interface */
			pr_handle_name("Management Controller Host Interface");
			if (ver < 0x0302)
			{
				if (h->length < 0x05) break;
				pr_attr("Interface Type", "%s",
					dmi_management_controller_host_type(data[0x04]));
				/*
				 * There you have a type-dependent, variable-length
				 * part in the middle of the structure, with no
				 * length specifier, so no easy way to decode the
				 * common, final part of the structure. What a pity.
				 */
				if (h->length < 0x09) break;
				if (data[0x04] == 0xF0)		/* OEM */
				{
					pr_attr("Vendor ID", "0x%02X%02X%02X%02X",
						data[0x05], data[0x06], data[0x07],
						data[0x08]);
				}
			}
			else
				dmi_parse_controller_structure(h);
			break;

		case 43: /* 7.44 TPM Device */
			pr_handle_name("TPM Device");
			if (h->length < 0x1B) break;
			dmi_tpm_vendor_id(data + 0x04);
			pr_attr("Specification Version", "%d.%d", data[0x08], data[0x09]);
			switch (data[0x08])
			{
				case 0x01:
					/*
					 * We skip the first 2 bytes, which are
					 * redundant with the above, and uncoded
					 * in a silly way.
					 */
					pr_attr("Firmware Revision", "%u.%u",
						data[0x0C], data[0x0D]);
					break;
				case 0x02:
					pr_attr("Firmware Revision", "%u.%u",
						DWORD(data + 0x0A) >> 16,
						DWORD(data + 0x0A) & 0xFFFF);
					/*
					 * We skip the next 4 bytes, as their
					 * format is not standardized and their
					 * usefulness seems limited anyway.
					 */
					break;
			}
			pr_attr("Description", "%s", dmi_string(h, data[0x12]));
			pr_list_start("Characteristics", NULL);
			dmi_tpm_characteristics(QWORD(data + 0x13));
			pr_list_end();
			if (h->length < 0x1F) break;
			pr_attr("OEM-specific Information", "0x%08X",
				DWORD(data + 0x1B));
			break;

		case 45: /* 7.46 Firmware Inventory Information */
			pr_handle_name("Firmware Inventory Information");
			if (h->length < 0x18) break;
			pr_attr("Firmware Component Name", "%s",
				dmi_string(h, data[0x04]));
			pr_attr("Firmware Version", "%s",
				dmi_string(h, data[0x05]));
			pr_attr("Firmware ID", "%s", dmi_string(h, data[0x07]));
			pr_attr("Release Date", "%s", dmi_string(h, data[0x09]));
			pr_attr("Manufacturer", "%s", dmi_string(h, data[0x0A]));
			pr_attr("Lowest Supported Firmware Version", "%s",
				dmi_string(h, data[0x0B]));
			dmi_memory_size("Image Size", QWORD(data + 0x0C));
			pr_list_start("Characteristics", NULL);
			dmi_firmware_characteristics(WORD(data + 0x14));
			pr_list_end();
			pr_attr("State", "%s", dmi_firmware_state(data[0x16]));
			if (h->length < 0x18 + data[0x17] * 2) break;
			if (!(opt.flags & FLAG_QUIET))
				dmi_firmware_components(data[0x17], data + 0x18);
			break;

		case 126:
			pr_handle_name("Inactive");
			break;

		case 127:
			pr_handle_name("End Of Table");
			break;

		default:
			if (dmi_decode_oem(h))
				break;
			if (opt.flags & FLAG_QUIET)
				return;
			pr_handle_name("%s Type",
				h->type >= 128 ? "OEM-specific" : "Unknown");
			dmi_dump(h);
	}
	pr_sep();
}

static void to_dmi_header(struct dmi_header *h, u8 *data)
{
	h->type = data[0];
	h->length = data[1];
	h->handle = WORD(data + 2);
	h->data = data;
}

static void dmi_table_string(const struct dmi_header *h, const u8 *data, u16 ver)
{
	int key;
	u8 offset = opt.string->offset;

	if (opt.string->type == 11) /* OEM strings */
	{
		if (h->length < 5 || offset > data[4])
		{
			fprintf(stderr, "No OEM string number %u\n", offset);
			return;
		}

		if (offset)
			printf("%s\n", dmi_string(h, offset));
		else
			printf("%u\n", data[4]);	/* count */
		return;
	}

	if (offset >= h->length)
		return;

	key = (opt.string->type << 8) | offset;
	switch (key)
	{
		case 0x015: /* -s bios-revision */
			if (data[offset - 1] != 0xFF && data[offset] != 0xFF)
				printf("%u.%u\n", data[offset - 1], data[offset]);
			break;
		case 0x017: /* -s firmware-revision */
			if (data[offset - 1] != 0xFF && data[offset] != 0xFF)
				printf("%u.%u\n", data[offset - 1], data[offset]);
			break;
		case 0x108:
			dmi_system_uuid(NULL, NULL, data + offset, ver);
			break;
		case 0x305:
			printf("%s\n", dmi_chassis_type(data[offset]));
			break;
		case 0x406:
			printf("%s\n", dmi_processor_family(h, ver));
			break;
		case 0x416:
			dmi_processor_frequency(NULL, data + offset);
			break;
		default:
			printf("%s\n", dmi_string(h, data[offset]));
	}
}

static int dmi_table_dump(const u8 *ep, u32 ep_len, const u8 *table,
			  u32 table_len)
{
	int fd;
	FILE *f;

	fd = open(opt.dumpfile, O_WRONLY|O_CREAT|O_EXCL, 0666);
	if (fd == -1)
	{
		fprintf(stderr, "%s: ", opt.dumpfile);
		perror("open");
		return -1;
	}

	f = fdopen(fd, "wb");
	if (!f)
	{
		fprintf(stderr, "%s: ", opt.dumpfile);
		perror("fdopen");
		return -1;
	}

	if (!(opt.flags & FLAG_QUIET))
		pr_comment("Writing %d bytes to %s.", ep_len, opt.dumpfile);
	if (fwrite(ep, ep_len, 1, f) != 1)
	{
		fprintf(stderr, "%s: ", opt.dumpfile);
		perror("fwrite");
		goto err_close;
	}

	if (fseek(f, 32, SEEK_SET) != 0)
	{
		fprintf(stderr, "%s: ", opt.dumpfile);
		perror("fseek");
		goto err_close;
	}

	if (!(opt.flags & FLAG_QUIET))
		pr_comment("Writing %d bytes to %s.", table_len, opt.dumpfile);
	if (fwrite(table, table_len, 1, f) != 1)
	{
		fprintf(stderr, "%s: ", opt.dumpfile);
		perror("fwrite");
		goto err_close;
	}

	if (fclose(f))
	{
		fprintf(stderr, "%s: ", opt.dumpfile);
		perror("fclose");
		return -1;
	}

	return 0;

err_close:
	fclose(f);
	return -1;
}

static void dmi_table_decode(u8 *buf, u32 len, u16 num, u16 ver, u32 flags)
{
	u8 *data;
	int i = 0;

	/* First pass: Save specific values needed to decode OEM types */
	data = buf;
	while ((i < num || !num)
	    && data + 4 <= buf + len) /* 4 is the length of an SMBIOS structure header */
	{
		u8 *next;
		struct dmi_header h;

		to_dmi_header(&h, data);

		/*
		 * If a short entry is found (less than 4 bytes), not only it
		 * is invalid, but we cannot reliably locate the next entry.
		 * Also stop at end-of-table marker if so instructed.
		 */
		if (h.length < 4 ||
		    (h.type == 127 &&
		     (opt.flags & (FLAG_QUIET | FLAG_STOP_AT_EOT))))
			break;
		i++;

		/* Look for the next handle */
		next = data + h.length;
		while ((unsigned long)(next - buf + 1) < len
		    && (next[0] != 0 || next[1] != 0))
			next++;
		next += 2;

		/* Make sure the whole structure fits in the table */
		if ((unsigned long)(next - buf) > len)
			break;

		/* Assign vendor for vendor-specific decodes later */
		if (h.type == 1 && h.length >= 6)
			dmi_set_vendor(_dmi_string(&h, data[0x04], 0),
				       _dmi_string(&h, data[0x05], 0));

		/* Remember CPUID type for HPE type 199 */
		if (h.type == 4 && h.length >= 0x1A && cpuid_type == cpuid_none)
			cpuid_type = dmi_get_cpuid_type(&h);
		data = next;
	}

	/* Second pass: Actually decode the data */
	i = 0;
	data = buf;
	while ((i < num || !num)
	    && data + 4 <= buf + len) /* 4 is the length of an SMBIOS structure header */
	{
		u8 *next;
		struct dmi_header h;
		int display;

		to_dmi_header(&h, data);
		display = ((opt.type == NULL || opt.type[h.type])
			&& (opt.handle == ~0U || opt.handle == h.handle)
			&& !((opt.flags & FLAG_QUIET) && (h.type == 126 || h.type == 127))
			&& !opt.string);

		/*
		 * If a short entry is found (less than 4 bytes), not only it
		 * is invalid, but we cannot reliably locate the next entry.
		 * Better stop at this point, and let the user know his/her
		 * table is broken.
		 */
		if (h.length < 4)
		{
			if (!(opt.flags & FLAG_QUIET))
			{
				fprintf(stderr,
					"Invalid entry length (%u). DMI table "
					"is broken! Stop.\n\n",
					(unsigned int)h.length);
				opt.flags |= FLAG_QUIET;
			}
			break;
		}
		i++;

		/* In quiet mode, stop decoding at end of table marker */
		if ((opt.flags & FLAG_QUIET) && h.type == 127)
			break;

		if (display
		 && (!(opt.flags & FLAG_QUIET) || (opt.flags & FLAG_DUMP)))
			pr_handle(&h);

		/* Look for the next handle */
		next = data + h.length;
		while ((unsigned long)(next - buf + 1) < len
		    && (next[0] != 0 || next[1] != 0))
			next++;
		next += 2;

		/* Make sure the whole structure fits in the table */
		if ((unsigned long)(next - buf) > len)
		{
			if (display && !(opt.flags & FLAG_QUIET))
				pr_struct_err("<TRUNCATED>");
			pr_sep();
			data = next;
			break;
		}

		/* Fixup a common mistake */
		if (h.type == 34 && !(opt.flags & FLAG_NO_QUIRKS))
			dmi_fixup_type_34(&h, display);

		if (display)
		{
			if (opt.flags & FLAG_DUMP)
			{
				dmi_dump(&h);
				pr_sep();
			}
			else
				dmi_decode(&h, ver);
		}
		else if (opt.string != NULL
		      && opt.string->type == h.type)
			dmi_table_string(&h, data, ver);

		data = next;

		/* SMBIOS v3 requires stopping at this marker */
		if (h.type == 127 && (flags & FLAG_STOP_AT_EOT))
			break;
	}

	/*
	 * SMBIOS v3 64-bit entry points do not announce a structures count,
	 * and only indicate a maximum size for the table.
	 */
	if (!(opt.flags & FLAG_QUIET))
	{
		if (num && i != num)
			fprintf(stderr, "Wrong DMI structures count: %d announced, "
				"only %d decoded.\n", num, i);
		if ((unsigned long)(data - buf) > len
		 || (num && (unsigned long)(data - buf) < len))
			fprintf(stderr, "Wrong DMI structures length: %u bytes "
				"announced, structures occupy %lu bytes.\n",
				len, (unsigned long)(data - buf));
	}
}

/* Allocates a buffer for the table, must be freed by the caller */
static u8 *dmi_table_get(off_t base, u32 *len, u16 num, u32 ver,
			 const char *devmem, u32 flags)
{
	u8 *buf;

	if (ver > SUPPORTED_SMBIOS_VER && !(opt.flags & FLAG_QUIET))
	{
		pr_comment("SMBIOS implementations newer than version %u.%u.%u are not",
			   SUPPORTED_SMBIOS_VER >> 16,
			   (SUPPORTED_SMBIOS_VER >> 8) & 0xFF,
			   SUPPORTED_SMBIOS_VER & 0xFF);
		pr_comment("fully supported by this version of dmidecode.");
	}

	if (!(opt.flags & FLAG_QUIET))
	{
		if (opt.type == NULL)
		{
			if (num)
				pr_info("%u structures occupying %u bytes.",
					num, *len);
			if (!(opt.flags & FLAG_FROM_DUMP))
				pr_info("Table at 0x%08llX.",
					(unsigned long long)base);
		}
		pr_sep();
	}

	if ((flags & FLAG_NO_FILE_OFFSET) || (opt.flags & FLAG_FROM_DUMP))
	{
		/*
		 * When reading from sysfs or from a dump file, the file may be
		 * shorter than announced. For SMBIOS v3 this is expcted, as we
		 * only know the maximum table size, not the actual table size.
		 * For older implementations (and for SMBIOS v3 too), this
		 * would be the result of the kernel truncating the table on
		 * parse error.
		 */
		size_t size = *len;
		buf = read_file(flags & FLAG_NO_FILE_OFFSET ? 0 : base,
			&size, devmem);
		if (!(opt.flags & FLAG_QUIET) && num && size != (size_t)*len)
		{
			fprintf(stderr, "Wrong DMI structures length: %u bytes "
				"announced, only %lu bytes available.\n",
				*len, (unsigned long)size);
		}
		*len = size;
	}
	else
		buf = mem_chunk(base, *len, devmem);

	if (buf == NULL)
	{
		fprintf(stderr, "Failed to read table, sorry.\n");
#ifndef USE_MMAP
		if (!(flags & FLAG_NO_FILE_OFFSET))
			fprintf(stderr,
				"Try compiling dmidecode with -DUSE_MMAP.\n");
#endif
	}

	return buf;
}


/*
 * Build a crafted entry point with table address hard-coded to 32,
 * as this is where we will put it in the output file. We adjust the
 * DMI checksum appropriately. The SMBIOS checksum needs no adjustment.
 */
static void overwrite_dmi_address(u8 *buf)
{
	buf[0x05] += buf[0x08] + buf[0x09] + buf[0x0A] + buf[0x0B] - 32;
	buf[0x08] = 32;
	buf[0x09] = 0;
	buf[0x0A] = 0;
	buf[0x0B] = 0;
}

/* Same thing for SMBIOS3 entry points */
static void overwrite_smbios3_address(u8 *buf)
{
	buf[0x05] += buf[0x10] + buf[0x11] + buf[0x12] + buf[0x13]
		   + buf[0x14] + buf[0x15] + buf[0x16] + buf[0x17] - 32;
	buf[0x10] = 32;
	buf[0x11] = 0;
	buf[0x12] = 0;
	buf[0x13] = 0;
	buf[0x14] = 0;
	buf[0x15] = 0;
	buf[0x16] = 0;
	buf[0x17] = 0;
}

static int smbios3_decode(u8 *buf, size_t buf_len, const char *devmem, u32 flags)
{
	u32 ver, len;
	u64 offset;
	u8 *table;

	/* Don't let checksum run beyond the buffer */
	if (buf[0x06] > buf_len)
	{
		fprintf(stderr,
			"Entry point length too large (%u bytes, expected %u).\n",
			(unsigned int)buf[0x06], 0x18U);
		return 0;
	}

	if (buf[0x06] < 0x18
	 || !checksum(buf, buf[0x06]))
		return 0;

	ver = (buf[0x07] << 16) + (buf[0x08] << 8) + buf[0x09];
	if (!(opt.flags & FLAG_QUIET))
		pr_info("SMBIOS %u.%u.%u present.",
			buf[0x07], buf[0x08], buf[0x09]);

	offset = QWORD(buf + 0x10);
	if (!(flags & FLAG_NO_FILE_OFFSET) && offset.h && sizeof(off_t) < 8)
	{
		fprintf(stderr, "64-bit addresses not supported, sorry.\n");
		return 0;
	}

	/* Maximum length, may get trimmed */
	len = DWORD(buf + 0x0C);
	table = dmi_table_get(((off_t)offset.h << 32) | offset.l, &len, 0, ver,
			      devmem, flags | FLAG_STOP_AT_EOT);
	if (table == NULL)
		return 1;

	if (opt.flags & FLAG_DUMP_BIN)
	{
		u8 crafted[32];

		memcpy(crafted, buf, 32);
		overwrite_smbios3_address(crafted);

		dmi_table_dump(crafted, crafted[0x06], table, len);
	}
	else
	{
		dmi_table_decode(table, len, 0, ver >> 8,
				 flags | FLAG_STOP_AT_EOT);
	}

	free(table);

	return 1;
}

static void dmi_fixup_version(u16 *ver)
{
	/* Some BIOS report weird SMBIOS version, fix that up */
	switch (*ver)
	{
		case 0x021F:
		case 0x0221:
			if (!(opt.flags & FLAG_QUIET))
				fprintf(stderr,
					"SMBIOS version fixup (2.%d -> 2.%d).\n",
					*ver & 0xFF, 3);
			*ver = 0x0203;
			break;
		case 0x0233:
			if (!(opt.flags & FLAG_QUIET))
				fprintf(stderr,
					"SMBIOS version fixup (2.%d -> 2.%d).\n",
					51, 6);
			*ver = 0x0206;
			break;
	}
}

static int smbios_decode(u8 *buf, size_t buf_len, const char *devmem, u32 flags)
{
	u16 ver, num;
	u32 len;
	u8 *table;

	/* Don't let checksum run beyond the buffer */
	if (buf[0x05] > buf_len)
	{
		fprintf(stderr,
			"Entry point length too large (%u bytes, expected %u).\n",
			(unsigned int)buf[0x05], 0x1FU);
		return 0;
	}

	/*
	 * The size of this structure is 0x1F bytes, but we also accept value
	 * 0x1E due to a mistake in SMBIOS specification version 2.1.
	 */
	if (buf[0x05] < 0x1E
	 || !checksum(buf, buf[0x05])
	 || memcmp(buf + 0x10, "_DMI_", 5) != 0
	 || !checksum(buf + 0x10, 0x0F))
		return 0;

	ver = (buf[0x06] << 8) + buf[0x07];
	if (!(opt.flags & FLAG_NO_QUIRKS))
		dmi_fixup_version(&ver);
	if (!(opt.flags & FLAG_QUIET))
		pr_info("SMBIOS %u.%u present.",
			ver >> 8, ver & 0xFF);

	/* Maximum length, may get trimmed */
	len = WORD(buf + 0x16);
	num = WORD(buf + 0x1C);
	table = dmi_table_get(DWORD(buf + 0x18), &len, num, ver << 8,
			      devmem, flags);
	if (table == NULL)
		return 1;

	if (opt.flags & FLAG_DUMP_BIN)
	{
		u8 crafted[32];

		memcpy(crafted, buf, 32);
		overwrite_dmi_address(crafted + 0x10);

		dmi_table_dump(crafted, crafted[0x05], table, len);
	}
	else
	{
		dmi_table_decode(table, len, num, ver, flags);
	}

	free(table);

	return 1;
}

static int legacy_decode(u8 *buf, const char *devmem, u32 flags)
{
	u16 ver, num;
	u32 len;
	u8 *table;

	if (!checksum(buf, 0x0F))
		return 0;

	ver = ((buf[0x0E] & 0xF0) << 4) + (buf[0x0E] & 0x0F);
	if (!(opt.flags & FLAG_QUIET))
		pr_info("Legacy DMI %u.%u present.",
			buf[0x0E] >> 4, buf[0x0E] & 0x0F);

	/* Maximum length, may get trimmed */
	len = WORD(buf + 0x06);
	num = WORD(buf + 0x0C);
	table = dmi_table_get(DWORD(buf + 0x08), &len, num, ver << 8,
			      devmem, flags);
	if (table == NULL)
		return 1;

	if (opt.flags & FLAG_DUMP_BIN)
	{
		u8 crafted[16];

		memcpy(crafted, buf, 16);
		overwrite_dmi_address(crafted);

		dmi_table_dump(crafted, 0x0F, table, len);
	}
	else
	{
		dmi_table_decode(table, len, num, ver, flags);
	}

	free(table);

	return 1;
}

/*
 * Probe for EFI interface
 */
#define EFI_NOT_FOUND   (-1)
#define EFI_NO_SMBIOS   (-2)
static int address_from_efi(off_t *address)
{
#if defined(__linux__)
	FILE *efi_systab;
	const char *filename;
	char linebuf[64];
#elif defined(__FreeBSD__) || defined(__DragonFly__)
	char addrstr[KENV_MVALLEN + 1];
#endif
	const char *eptype;
	int ret;

	*address = 0; /* Prevent compiler warning */

#if defined(__linux__)
	/*
	 * Linux up to 2.6.6: /proc/efi/systab
	 * Linux 2.6.7 and up: /sys/firmware/efi/systab
	 */
	if ((efi_systab = fopen(filename = "/sys/firmware/efi/systab", "r")) == NULL
	 && (efi_systab = fopen(filename = "/proc/efi/systab", "r")) == NULL)
	{
		/* No EFI interface, fallback to memory scan */
		return EFI_NOT_FOUND;
	}
	ret = EFI_NO_SMBIOS;
	while ((fgets(linebuf, sizeof(linebuf) - 1, efi_systab)) != NULL)
	{
		char *addrp = strchr(linebuf, '=');
		*(addrp++) = '\0';
		if (strcmp(linebuf, "SMBIOS3") == 0
		 || strcmp(linebuf, "SMBIOS") == 0)
		{
			*address = strtoull(addrp, NULL, 0);
			eptype = linebuf;
			ret = 0;
			break;
		}
	}
	if (fclose(efi_systab) != 0)
		perror(filename);

	if (ret == EFI_NO_SMBIOS)
		fprintf(stderr, "%s: SMBIOS entry point missing\n", filename);
#elif defined(__FreeBSD__) || defined(__DragonFly__)
	/*
	 * On FreeBSD, SMBIOS anchor base address in UEFI mode is exposed
	 * via kernel environment:
	 * https://svnweb.freebsd.org/base?view=revision&revision=307326
	 *
	 * DragonFly BSD adopted the same method as FreeBSD, see commit
	 * 5e488df32cb01056a5b714a522e51c69ab7b4612
	 */
	ret = kenv(KENV_GET, "hint.smbios.0.mem", addrstr, sizeof(addrstr));
	if (ret == -1)
	{
		if (errno != ENOENT)
			perror("kenv");
		return EFI_NOT_FOUND;
	}

	*address = strtoull(addrstr, NULL, 0);
	eptype = "SMBIOS";
	ret = 0;
#else
	ret = EFI_NOT_FOUND;
#endif

	if (ret == 0 && !(opt.flags & FLAG_QUIET))
		pr_comment("%s entry point at 0x%08llx",
			   eptype, (unsigned long long)*address);

	return ret;
}

int main(int argc, char * const argv[])
{
	int ret = 0;                /* Returned value */
	int found = 0;
	off_t fp;
	size_t size;
	int efi;
	u8 *buf = NULL;

	/*
	 * We don't want stdout and stderr to be mixed up if both are
	 * redirected to the same file.
	 */
	setlinebuf(stdout);
	setlinebuf(stderr);

	if (sizeof(u8) != 1 || sizeof(u16) != 2 || sizeof(u32) != 4 || '\0' != 0)
	{
		fprintf(stderr, "%s: compiler incompatibility\n", argv[0]);
		exit(255);
	}

	/* Set default option values */
	opt.devmem = DEFAULT_MEM_DEV;
	opt.flags = 0;
	opt.handle = ~0U;

	if (parse_command_line(argc, argv)<0)
	{
		ret = 2;
		goto exit_free;
	}

	if (opt.flags & FLAG_LIST)
	{
		/* Already handled in parse_command_line() */
		goto exit_free;
	}

	if (opt.flags & FLAG_HELP)
	{
		print_help();
		goto exit_free;
	}

	if (opt.flags & FLAG_VERSION)
	{
		printf("%s\n", VERSION);
		goto exit_free;
	}

	if (!(opt.flags & FLAG_QUIET))
		pr_comment("dmidecode %s", VERSION);

	/* Read from dump if so instructed */
	size = 0x20;
	if (opt.flags & FLAG_FROM_DUMP)
	{
		if (!(opt.flags & FLAG_QUIET))
			pr_info("Reading SMBIOS/DMI data from file %s.",
				opt.dumpfile);
		if ((buf = read_file(0, &size, opt.dumpfile)) == NULL)
		{
			ret = 1;
			goto exit_free;
		}

		/* Truncated entry point can't be processed */
		if (size < 0x20)
		{
			ret = 1;
			goto done;
		}

		if (memcmp(buf, "_SM3_", 5) == 0)
		{
			if (smbios3_decode(buf, size, opt.dumpfile, 0))
				found++;
		}
		else if (memcmp(buf, "_SM_", 4) == 0)
		{
			if (smbios_decode(buf, size, opt.dumpfile, 0))
				found++;
		}
		else if (memcmp(buf, "_DMI_", 5) == 0)
		{
			if (legacy_decode(buf, opt.dumpfile, 0))
				found++;
		}
		goto done;
	}

	/*
	 * First try reading from sysfs tables.  The entry point file could
	 * contain one of several types of entry points, so read enough for
	 * the largest one, then determine what type it contains.
	 */
	if (!(opt.flags & FLAG_NO_SYSFS)
	 && (buf = read_file(0, &size, SYS_ENTRY_FILE)) != NULL)
	{
		if (!(opt.flags & FLAG_QUIET))
			pr_info("Getting SMBIOS data from sysfs.");
		if (size >= 24 && memcmp(buf, "_SM3_", 5) == 0)
		{
			if (smbios3_decode(buf, size, SYS_TABLE_FILE, FLAG_NO_FILE_OFFSET))
				found++;
		}
		else if (size >= 31 && memcmp(buf, "_SM_", 4) == 0)
		{
			if (smbios_decode(buf, size, SYS_TABLE_FILE, FLAG_NO_FILE_OFFSET))
				found++;
		}
		else if (size >= 15 && memcmp(buf, "_DMI_", 5) == 0)
		{
			if (legacy_decode(buf, SYS_TABLE_FILE, FLAG_NO_FILE_OFFSET))
				found++;
		}

		if (found)
			goto done;
		if (!(opt.flags & FLAG_QUIET))
			pr_info("Failed to get SMBIOS data from sysfs.");
	}

	/* Next try EFI (ia64, Intel-based Mac, arm64) */
	efi = address_from_efi(&fp);
	switch (efi)
	{
		case EFI_NOT_FOUND:
			goto memory_scan;
		case EFI_NO_SMBIOS:
			ret = 1;
			goto exit_free;
	}

	if (!(opt.flags & FLAG_QUIET))
		pr_info("Found SMBIOS entry point in EFI, reading table from %s.",
			opt.devmem);
	if ((buf = mem_chunk(fp, 0x20, opt.devmem)) == NULL)
	{
		ret = 1;
		goto exit_free;
	}

	if (memcmp(buf, "_SM3_", 5) == 0)
	{
		if (smbios3_decode(buf, 0x20, opt.devmem, 0))
			found++;
	}
	else if (memcmp(buf, "_SM_", 4) == 0)
	{
		if (smbios_decode(buf, 0x20, opt.devmem, 0))
			found++;
	}
	goto done;

memory_scan:
#if defined __i386__ || defined __x86_64__
	if (!(opt.flags & FLAG_QUIET))
		pr_info("Scanning %s for entry point.", opt.devmem);
	/* Fallback to memory scan (x86, x86_64) */
	if ((buf = mem_chunk(0xF0000, 0x10000, opt.devmem)) == NULL)
	{
		ret = 1;
		goto exit_free;
	}

	/* Look for a 64-bit entry point first */
	for (fp = 0; fp <= 0xFFE0; fp += 16)
	{
		if (memcmp(buf + fp, "_SM3_", 5) == 0)
		{
			if (smbios3_decode(buf + fp, 0x20, opt.devmem, 0))
			{
				found++;
				goto done;
			}
		}
	}

	/* If none found, look for a 32-bit entry point */
	for (fp = 0; fp <= 0xFFF0; fp += 16)
	{
		if (memcmp(buf + fp, "_SM_", 4) == 0 && fp <= 0xFFE0)
		{
			if (smbios_decode(buf + fp, 0x20, opt.devmem, 0))
			{
				found++;
				goto done;
			}
		}
		else if (memcmp(buf + fp, "_DMI_", 5) == 0)
		{
			if (legacy_decode(buf + fp, opt.devmem, 0))
			{
				found++;
				goto done;
			}
		}
	}
#endif

done:
	if (!found && !(opt.flags & FLAG_QUIET))
		pr_comment("No SMBIOS nor DMI entry point found, sorry.");

	free(buf);
exit_free:
	free(opt.type);

	return ret;
}
