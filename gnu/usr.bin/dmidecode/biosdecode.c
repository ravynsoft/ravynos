/*
 * BIOS Decode
 *
 *   Copyright (C) 2000-2002 Alan Cox <alan@redhat.com>
 *   Copyright (C) 2002-2017 Jean Delvare <jdelvare@suse.de>
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
 *  - DMTF "System Management BIOS (SMBIOS) Reference Specification"
 *    Version 3.0.0
 *    http://www.dmtf.org/standards/smbios
 *  - Intel "Preboot Execution Environment (PXE) Specification"
 *    Version 2.1
 *    http://www.intel.com/labs/manage/wfm/wfmspecs.htm
 *  - ACPI "Advanced Configuration and Power Interface Specification"
 *    Revision 2.0
 *    http://www.acpi.info/spec20.htm
 *  - Phoenix "BIOS32 Service Directory"
 *    Revision 0.4
 *    http://www.phoenix.com/en/support/white+papers-specs/
 *  - Microsoft "Plug and Play BIOS Specification"
 *    Version 1.0A
 *    http://www.microsoft.com/hwdev/tech/PnP/
 *  - Microsoft "PCI IRQ Routing Table Specification"
 *    Version 1.0
 *    http://www.microsoft.com/hwdev/archive/BUSBIOS/pciirq.asp
 *  - Compaq "Technical Reference Guide for Compaq Deskpro 4000 and 6000"
 *    First Edition
 *    http://h18000.www1.hp.com/support/techpubs/technical_reference_guides/113a1097.html
 *  - IBM "Using the BIOS Build ID to identify Thinkpad systems"
 *    Revision 2005-09-19
 *    http://www-307.ibm.com/pc/support/site.wss/MIGR-45120.html
 *  - Fujitsu application panel technical details
 *    As of July 23rd, 2004
 *    http://apanel.sourceforge.net/tech.php
 *  - Intel Multiprocessor Specification
 *    Version 1.4
 *    http://www.intel.com/design/archives/processors/pro/docs/242016.htm
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
	unsigned char pir;
};
static struct opt opt;

#define FLAG_VERSION            (1 << 0)
#define FLAG_HELP               (1 << 1)

#define PIR_SHORT               0
#define PIR_FULL                1

struct bios_entry {
	const char *anchor;
	size_t anchor_len; /* computed */
	off_t low_address;
	off_t high_address;
	size_t (*length)(const u8 *);
	int (*decode)(const u8*, size_t);
};


/*
 * SMBIOS
 */

static size_t smbios3_length(const u8 *p)
{
	return p[0x06];
}

static int smbios3_decode(const u8 *p, size_t len)
{
	if (len < 0x18 || !checksum(p, p[0x06]))
		return 0;

	printf("SMBIOS %u.%u.%u present.\n",
		p[0x07], p[0x08], p[0x09]);
	printf("\tStructure Table Maximum Length: %u bytes\n",
		DWORD(p + 0x0C));
	printf("\tStructure Table 64-bit Address: 0x%08X%08X\n",
		QWORD(p + 0x10).h, QWORD(p + 0x10).l);

	return 1;
}

static size_t smbios_length(const u8 *p)
{
	return p[0x05] == 0x1E ? 0x1F : p[0x05];
}

static int smbios_decode(const u8 *p, size_t len)
{
	if (len < 0x1F || !checksum(p, p[0x05])
	 || memcmp("_DMI_", p + 0x10, 5) != 0
	 || !checksum(p + 0x10, 0x0F))
		return 0;

	printf("SMBIOS %u.%u present.\n",
		p[0x06], p[0x07]);
	printf("\tStructure Table Length: %u bytes\n",
		WORD(p + 0x16));
	printf("\tStructure Table Address: 0x%08X\n",
		DWORD(p + 0x18));
	printf("\tNumber Of Structures: %u\n",
		WORD(p + 0x1C));
	printf("\tMaximum Structure Size: %u bytes\n",
		WORD(p + 0x08));

	return 1;
}

static size_t dmi_length(const u8 *p)
{
	(void) p;

	return 0x0F;
}

static int dmi_decode(const u8 *p, size_t len)
{
	if (len < 0x0F || !checksum(p, len))
		return 0;

	printf("Legacy DMI %u.%u present.\n",
		p[0x0E]>>4, p[0x0E] & 0x0F);
	printf("\tStructure Table Length: %u bytes\n",
		WORD(p + 0x06));
	printf("\tStructure Table Address: 0x%08X\n",
		DWORD(p + 0x08));
	printf("\tNumber Of Structures: %u\n",
		WORD(p + 0x0C));

	return 1;
}

/*
 * SYSID
 */

static size_t sysid_length(const u8 *p)
{
	return WORD(p + 0x08);
}

static int sysid_decode(const u8 *p, size_t len)
{
	if (len < 0x11 || !checksum(p, WORD(p + 0x08)))
		return 0;

	printf("SYSID present.\n");
	printf("\tRevision: %u\n",
		p[0x10]);
	printf("\tStructure Table Address: 0x%08X\n",
		DWORD(p + 0x0A));
	printf("\tNumber Of Structures: %u\n",
		WORD(p + 0x0E));

	return 1;
}

/*
 * PnP
 */

static size_t pnp_length(const u8 *p)
{
	return p[0x05];
}

static const char *pnp_event_notification(u8 code)
{
	static const char *notification[] = {
		"Not Supported", /* 0x0 */
		"Polling",
		"Asynchronous",
		"Unknown" /* 0x3 */
	};

	return notification[code];
}

static int pnp_decode(const u8 *p, size_t len)
{
	if (len < 0x21 || !checksum(p, p[0x05]))
		return 0;

	printf("PNP BIOS %u.%u present.\n",
		p[0x04] >> 4, p[0x04] & 0x0F);
	printf("\tEvent Notification: %s\n",
		pnp_event_notification(WORD(p + 0x06) & 0x03));
	if ((WORD(p + 0x06) & 0x03) == 0x01)
		printf("\tEvent Notification Flag Address: 0x%08X\n",
			DWORD(p + 0x09));
	printf("\tReal Mode 16-bit Code Address: %04X:%04X\n",
		WORD(p + 0x0F), WORD(p + 0x0D));
	printf("\tReal Mode 16-bit Data Address: %04X:0000\n",
		WORD(p + 0x1B));
	printf("\t16-bit Protected Mode Code Address: 0x%08X\n",
		DWORD(p + 0x13) + WORD(p + 0x11));
	printf("\t16-bit Protected Mode Data Address: 0x%08X\n",
		DWORD(p + 0x1D));
	if (DWORD(p + 0x17) != 0)
		printf("\tOEM Device Identifier: %c%c%c%02X%02X\n",
			0x40 + ((p[0x17] >> 2) & 0x1F),
			0x40 + ((p[0x17] & 0x03) << 3) + ((p[0x18] >> 5) & 0x07),
			0x40 + (p[0x18] & 0x1F), p[0x19], p[0x20]);

	return 1;
}

/*
 * ACPI
 */

static size_t acpi_length(const u8 *p)
{
	return p[15] == 2 ? 36 : 20;
}

static const char *acpi_revision(u8 code)
{
	switch (code)
	{
		case 0:
			return " 1.0";
		case 2:
			return " 2.0";
		default:
			return "";
	}
}

static int acpi_decode(const u8 *p, size_t len)
{
	if (len < 20 || !checksum(p, 20))
		return 0;

	printf("ACPI%s present.\n",
		acpi_revision(p[15]));
	printf("\tOEM Identifier: %c%c%c%c%c%c\n",
		p[9], p[10], p[11], p[12], p[13], p[14]);
	printf("\tRSD Table 32-bit Address: 0x%08X\n",
		DWORD(p + 16));

	if (len < 36)
		return 1;

	if (DWORD(p + 20) > len || !checksum(p, DWORD(p + 20)))
		return 0;

	if (DWORD(p + 20) < 32) return 1;

	printf("\tXSD Table 64-bit Address: 0x%08X%08X\n",
		QWORD(p + 24).h, QWORD(p + 24).l);

	return 1;
}

/*
 * Sony
 */

static size_t sony_length(const u8 *p)
{
	return p[0x05];
}

static int sony_decode(const u8 *p, size_t len)
{
	if (!checksum(p, len))
		return 0;

	printf("Sony system detected.\n");

	return 1;
}

/*
 * BIOS32
 */

static size_t bios32_length(const u8 *p)
{
	return p[0x09] << 4;
}

static int bios32_decode(const u8 *p, size_t len)
{
	if (len < 0x0A || !checksum(p, p[0x09] << 4))
		return 0;

	printf("BIOS32 Service Directory present.\n");
	printf("\tRevision: %u\n",
		p[0x08]);
	printf("\tCalling Interface Address: 0x%08X\n",
		DWORD(p + 0x04));

	return 1;
}

/*
 * PIR
 */

static void pir_irqs(u16 code)
{
	if (code == 0)
		printf(" None");
	else
	{
		u8 i;

		for (i = 0; i < 16; i++)
			if (code & (1 << i))
				printf(" %u", i);
	}
}

static void pir_slot_number(u8 code)
{
	if (code == 0)
		printf(" on-board");
	else
		printf(" slot %u", code);
}

static size_t pir_length(const u8 *p)
{
	return WORD(p + 6);
}

static void pir_link_bitmap(char letter, const u8 *p)
{
	if (p[0] == 0) /* Not connected */
		return;

	printf("\t\tINT%c#: Link 0x%02x, IRQ Bitmap", letter, p[0]);
	pir_irqs(WORD(p + 1));
	printf("\n");
}

static int pir_decode(const u8 *p, size_t len)
{
	int i, n;

	if (len < 32 || !checksum(p, WORD(p + 6)))
		return 0;

	printf("PCI Interrupt Routing %u.%u present.\n",
		p[5], p[4]);
	printf("\tRouter Device: %02x:%02x.%1x\n",
		p[8], p[9]>>3, p[9] & 0x07);
	printf("\tExclusive IRQs:");
	pir_irqs(WORD(p + 10));
	printf("\n");
	if (DWORD(p + 12) != 0)
		printf("\tCompatible Router: %04x:%04x\n",
			WORD(p + 12), WORD(p + 14));
	if (DWORD(p + 16) != 0)
		printf("\tMiniport Data: 0x%08X\n",
			DWORD(p + 16));

	n = (len - 32) / 16;
	for (i = 1, p += 32; i <= n; i++, p += 16)
	{
		printf("\tDevice: %02x:%02x,", p[0], p[1] >> 3);
		pir_slot_number(p[14]);
		printf("\n");
		if (opt.pir == PIR_FULL)
		{
			pir_link_bitmap('A', p + 2);
			pir_link_bitmap('B', p + 5);
			pir_link_bitmap('C', p + 8);
			pir_link_bitmap('D', p + 11);
		}
	}

	return 1;
}

/*
 * Compaq-specific entries
 */

static size_t compaq_length(const u8 *p)
{
	return p[4] * 10 + 5;
}

static int compaq_decode(const u8 *p, size_t len)
{
	unsigned int i;
	(void) len;

	printf("Compaq-specific entries present.\n");

	/* integrity checking (lack of checksum) */
	for (i = 0; i < p[4]; i++)
	{
		/*
		 * We do not check for truncated entries, because the length
		 * was computed from the number of records in compaq_length
		 * right above, so it can't be wrong.
		 */
		if (p[5 + i * 10] != '$'
		 || !(p[6 + i * 10] >= 'A' && p[6 + i * 10] <= 'Z')
		 || !(p[7 + i * 10] >= 'A' && p[7 + i * 10] <= 'Z')
		 || !(p[8 + i * 10] >= 'A' && p[8 + i * 10] <= 'Z'))
		{
			printf("\t Abnormal entry! Please report. [%02X %02X "
				"%02X %02X]\n", p[5 + i * 10], p[6 + i * 10],
				p[7 + i * 10], p[8 + i * 10]);
			return 0;
		}
	}

	for (i = 0; i < p[4]; i++)
	{
		printf("\tEntry %u: %c%c%c%c at 0x%08X (%u bytes)\n",
			i + 1, p[5 + i * 10], p[6 + i * 10], p[7 + i * 10],
			p[8 + i * 10], DWORD(p + 9 + i * 10),
			WORD(p + 13 + i * 10));
	}

	return 1;
}

/*
 * VPD (vital product data, IBM-specific)
 */

static void vpd_print_entry(const char *name, const u8 *p, size_t len)
{
	size_t i;

	printf("\t%s: ", name);
	for (i = 0; i < len; i++)
		if (p[i] >= 32 && p[i] < 127)
			printf("%c", p[i]);
	printf("\n");
}

static size_t vpd_length(const u8 *p)
{
	return p[5];
}

static int vpd_decode(const u8 *p, size_t len)
{
	if (len < 0x30)
		return 0;

	/* XSeries have longer records. */
	if (!(len >= 0x45 && checksum(p, len))
	/* Some Netvista seem to work with this. */
	 && !checksum(p, 0x30)
	/* The Thinkpad checksum does *not* include the first 13 bytes. */
	 && !checksum(p + 0x0D, 0x30 - 0x0D))
		return 0;

	printf("VPD present.\n");

	vpd_print_entry("BIOS Build ID", p + 0x0D, 9);
	vpd_print_entry("Box Serial Number", p + 0x16, 7);
	vpd_print_entry("Motherboard Serial Number", p + 0x1D, 11);
	vpd_print_entry("Machine Type/Model", p + 0x28, 7);

	if (len < 0x45)
		return 1;

	vpd_print_entry("BIOS Release Date", p + 0x30, 8);

	return 1;
}

/*
 * Fujitsu application panel
 */

static size_t fjkeyinf_length(const u8 *p)
{
	(void) p;
	/*
	 * We don't know at this point, it's somewhere between 12 and 32.
	 * So we return the max, it shouldn't hurt.
	 */
	return 32;
}

static int fjkeyinf_decode(const u8 *p, size_t len)
{
	int i;
	(void) len;

	printf("Fujitsu application panel present.\n");

	for (i = 0; i < 6; i++)
	{
		if (*(p + 8 + i * 4) == 0)
			return 1;
		printf("\tDevice %d: type %u, chip %u", i + 1,
		       *(p + 8 + i * 4), *(p + 8 + i * 4 + 2));
		if (*(p + 8 + i * 4 + 1)) /* Access method */
			printf(", SMBus address 0x%x",
				*(p + 8 + i * 4 + 3) >> 1);
		printf("\n");
	}

	return 1;
}

/*
 * Intel Multiprocessor
 */

static size_t mp_length(const u8 *p)
{
	return 16 * p[8];
}

static int mp_decode(const u8 *p, size_t len)
{
	if (!checksum(p, len))
		return 0;

	printf("Intel Multiprocessor present.\n");
	printf("\tSpecification Revision: %s\n",
		p[9] == 0x01 ? "1.1" : p[9] == 0x04 ? "1.4" : "Invalid");
	if (p[11])
		printf("\tDefault Configuration: #%d\n", p[11]);
	else
		printf("\tConfiguration Table Address: 0x%08X\n",
			DWORD(p + 4));
	printf("\tMode: %s\n", p[12] & (1 << 7) ?
		"IMCR and PIC" : "Virtual Wire");

	return 1;
}

/*
 * Main
 */

static struct bios_entry bios_entries[] = {
	{ "_SM3_", 0, 0xF0000, 0xFFFFF, smbios3_length, smbios3_decode },
	{ "_SM_", 0, 0xF0000, 0xFFFFF, smbios_length, smbios_decode },
	{ "_DMI_", 0, 0xF0000, 0xFFFFF, dmi_length, dmi_decode },
	{ "_SYSID_", 0, 0xE0000, 0xFFFFF, sysid_length, sysid_decode },
	{ "$PnP", 0, 0xF0000, 0xFFFFF, pnp_length, pnp_decode },
	{ "RSD PTR ", 0, 0xE0000, 0xFFFFF, acpi_length, acpi_decode },
	{ "$SNY", 0, 0xE0000, 0xFFFFF, sony_length, sony_decode },
	{ "_32_", 0, 0xE0000, 0xFFFFF, bios32_length, bios32_decode },
	{ "$PIR", 0, 0xF0000, 0xFFFFF, pir_length, pir_decode },
	{ "32OS", 0, 0xE0000, 0xFFFFF, compaq_length, compaq_decode },
	{ "\252\125VPD", 0, 0xF0000, 0xFFFFF, vpd_length, vpd_decode },
	{ "FJKEYINF", 0, 0xF0000, 0xFFFFF, fjkeyinf_length, fjkeyinf_decode },
	{ "_MP_", 0, 0xE0000, 0xFFFFF, mp_length, mp_decode },
	{ NULL, 0, 0, 0, NULL, NULL }
};

/* Believe it or not, this is significantly faster than memcmp */
static int anchor_match(const struct bios_entry *entry, const char *p)
{
	size_t i;

	for (i = 0; i < entry->anchor_len; i++)
		if (entry->anchor[i] != p[i])
			return 0;

	return 1;
}

/* Return -1 on error, 0 on success */
static int parse_command_line(int argc, char * const argv[])
{
	int option;
	const char *optstring = "d:hV";
	struct option longopts[] = {
		{ "dev-mem", required_argument, NULL, 'd' },
		{ "pir", required_argument, NULL, 'P' },
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
			case 'P':
				if (strcmp(optarg, "full") == 0)
					opt.pir = PIR_FULL;
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
		"Usage: biosdecode [OPTIONS]\n"
		"Options are:\n"
		" -d, --dev-mem FILE     Read memory from device FILE (default: " DEFAULT_MEM_DEV ")\n"
		"     --pir full         Decode the details of the PCI IRQ routing table\n"
		" -h, --help             Display this help text and exit\n"
		" -V, --version          Display the version and exit\n";

	printf("%s", help);
}

int main(int argc, char * const argv[])
{
	u8 *buf;
	off_t fp;
	int i;

	if (sizeof(u8) != 1 || sizeof(u16) != 2 || sizeof(u32) != 4)
	{
		fprintf(stderr, "%s: compiler incompatibility\n", argv[0]);
		exit(255);
	}

	/* Set default option values */
	opt.devmem = DEFAULT_MEM_DEV;
	opt.flags = 0;

	if (parse_command_line(argc, argv) < 0)
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

	printf("# biosdecode %s\n", VERSION);

	if ((buf = mem_chunk(0xE0000, 0x20000, opt.devmem)) == NULL)
		exit(1);

	/* Compute anchor lengths once and for all */
	for (i = 0; bios_entries[i].anchor != NULL; i++)
		bios_entries[i].anchor_len = strlen(bios_entries[i].anchor);

	for (fp = 0xE0000; fp <= 0xFFFF0; fp += 16)
	{
		u8 *p = buf + fp - 0xE0000;

		for (i = 0; bios_entries[i].anchor != NULL; i++)
		{
			if (anchor_match(&bios_entries[i], (char *)p)
			 && fp >= bios_entries[i].low_address
			 && fp < bios_entries[i].high_address)
			{
				off_t len = bios_entries[i].length(p);

				if (fp + len - 1 <= bios_entries[i].high_address)
				{
					if (bios_entries[i].decode(p, len))
					{
						fp += (((len - 1) >> 4) << 4);
						break;
					}
				}
			}
		}
	}

	free(buf);

	return 0;
}
