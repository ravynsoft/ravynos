/*
 * Copyright (c) 2007, Oracle and/or its affiliates.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#ifndef _SYS_PCI_TOOLS_H
#define	_SYS_PCI_TOOLS_H

#pragma ident	"@(#)pci_tools.h	1.4	05/09/28 SMI"

#include <sys/modctl.h>

#ifdef	__cplusplus
extern "C" {
#endif

/*
 * Versioning. Have different versions for userland program and drivers, so
 * they can all stay in sync with each other.
 */
#define	PCITOOL_USER_VERSION	1
#define	PCITOOL_DRVR_VERSION	1

/* File suffixes for nexus pcitool nodes. */
#define	PCI_MINOR_REG	"reg"
#define	PCI_MINOR_INTR	"intr"

/*
 * Ioctls for PCI tools.
 */
#define	PCITOOL_IOC		(('P' << 24) | ('C' << 16) | ('T' << 8))

/* Read/write a device on a PCI bus, in physical space. */
#define	PCITOOL_DEVICE_GET_REG	(PCITOOL_IOC | 1)
#define	PCITOOL_DEVICE_SET_REG	(PCITOOL_IOC | 2)

/* Read/write the PCI nexus bridge, in physical space. */
#define	PCITOOL_NEXUS_GET_REG	(PCITOOL_IOC | 3)
#define	PCITOOL_NEXUS_SET_REG	(PCITOOL_IOC | 4)

/* Get/set interrupt-CPU mapping for PCI devices. */
#define	PCITOOL_DEVICE_GET_INTR	(PCITOOL_IOC | 5)
#define	PCITOOL_DEVICE_SET_INTR	(PCITOOL_IOC | 6)

/* Return the number of supported interrupts on a PCI bus. */
#define	PCITOOL_DEVICE_NUM_INTR	(PCITOOL_IOC | 7)


/*
 * This file contains data structures for the pci tool.
 */
#define	PCITOOL_CONFIG	0
#define	PCITOOL_BAR0	1
#define	PCITOOL_BAR1	2
#define	PCITOOL_BAR2	3
#define	PCITOOL_BAR3	4
#define	PCITOOL_BAR4	5
#define	PCITOOL_BAR5	6
#define	PCITOOL_ROM	7

/*
 * Pass this through barnum to signal to use a base addr instead.
 * This is for platforms which do not have a way to automatically map
 * a selected bank to a base addr.
 */
#define	PCITOOL_BASE	0xFF

/*
 * BAR corresponding to space desired.
 */
typedef enum {
    config = PCITOOL_CONFIG,
    bar0 = PCITOOL_BAR0,
    bar1 = PCITOOL_BAR1,
    bar2 = PCITOOL_BAR2,
    bar3 = PCITOOL_BAR3,
    bar4 = PCITOOL_BAR4,
    bar5 = PCITOOL_BAR5,
    rom = PCITOOL_ROM
} pcitool_bars_t;


/*
 * PCITOOL error numbers.
 */

typedef enum {
	PCITOOL_SUCCESS = 0x0,
	PCITOOL_INVALID_CPUID,
	PCITOOL_INVALID_INO,
	PCITOOL_PENDING_INTRTIMEOUT,
	PCITOOL_REGPROP_NOTWELLFORMED,
	PCITOOL_INVALID_ADDRESS,
	PCITOOL_NOT_ALIGNED,
	PCITOOL_OUT_OF_RANGE,
	PCITOOL_END_OF_RANGE,
	PCITOOL_ROM_DISABLED,
	PCITOOL_ROM_WRITE,
	PCITOOL_IO_ERROR,
	PCITOOL_INVALID_SIZE
} pcitool_errno_t;


/*
 * PCITOOL_DEVICE_SET_INTR ioctl data structure to re-assign the interrupts.
 */
typedef struct pcitool_intr_set {
	uint16_t user_version;	/* Userland program version - to krnl */
	uint16_t drvr_version;	/* Driver version - from kernel */
	uint32_t ino;		/* interrupt to set - to kernel */
	uint32_t cpu_id;	/* to: cpu to set / from: old cpu returned */
	pcitool_errno_t status;	/* from kernel */
} pcitool_intr_set_t;


/*
 * PCITOOL_DEVICE_GET_INTR ioctl data structure to dump out the
 * ino mapping information.
 */

typedef struct pcitool_intr_dev {
	uint32_t	dev_inst;	/* device instance - from kernel */
	char		driver_name[MAXMODCONFNAME];	/* from kernel */
	char		path[MAXPATHLEN]; /* device path - from kernel */
} pcitool_intr_dev_t;


typedef struct pcitool_intr_get {
	uint16_t user_version;		/* Userland program version - to krnl */
	uint16_t drvr_version;		/* Driver version - from kernel */
	uint32_t	ino;		/* interrupt number - to kernel */
	uint8_t		num_devs_ret;	/* room for this # of devs to be */
					/* returned - to kernel */
					/* # devs returned - from kernel */
	uint8_t		num_devs;	/* # devs on this ino - from kernel */
					/* intrs enabled for devs if > 0 */
	uint8_t		ctlr;		/* controller number - from kernel */
	uint32_t	cpu_id;		/* cpu of interrupt - from kernel */
	pcitool_errno_t status;		/* returned status - from kernel */
	pcitool_intr_dev_t	dev[1];	/* start of variable device list */
					/* from kernel */
} pcitool_intr_get_t;

/*
 * Get the size needed to return the number of devices wanted.
 * Can't say num_devs - 1 as num_devs may be unsigned.
 */
#define	PCITOOL_IGET_SIZE(num_devs) \
	(sizeof (pcitool_intr_get_t) - \
	sizeof (pcitool_intr_dev_t) + \
	((num_devs) * sizeof (pcitool_intr_dev_t)))

/*
 * Size and endian fields for acc_attr bitmask.
 */
#define	PCITOOL_ACC_ATTR_SIZE_MASK	0x3
#define	PCITOOL_ACC_ATTR_SIZE_1		0x0
#define	PCITOOL_ACC_ATTR_SIZE_2		0x1
#define	PCITOOL_ACC_ATTR_SIZE_4		0x2
#define	PCITOOL_ACC_ATTR_SIZE_8		0x3
#define	PCITOOL_ACC_ATTR_SIZE(x)	(1 << ((x) & PCITOOL_ACC_ATTR_SIZE_MASK))

#define	PCITOOL_ACC_ATTR_ENDN_MASK	0x100
#define	PCITOOL_ACC_ATTR_ENDN_LTL	0x0
#define	PCITOOL_ACC_ATTR_ENDN_BIG	0x100
#define	PCITOOL_ACC_IS_BIG_ENDIAN(x)	((x) & PCITOOL_ACC_ATTR_ENDN_BIG)

/*
 * Data structure to read and write to pci device registers.
 * This is the argument to the following ioctls:
 *	PCITOOL_DEVICE_SET/GET_REG
 *	PCITOOL_NEXUS_SET/GET_REG
 */
typedef struct pcitool_reg {
	uint16_t	user_version;	/* Userland program version - to krnl */
	uint16_t	drvr_version;	/* Driver version - from kernel */
	uint8_t		bus_no;		/* pci bus - to kernel */
	uint8_t		dev_no;		/* pci dev - to kernel */
	uint8_t		func_no;	/* pci function - to kernel */
	uint8_t		barnum;		/* bank (DEVCTL_NEXUS_SET/GET_REG) or */
					/*   BAR from pcitools_bar_t */
					/*   (DEVCTL_DEVICE_SET/GET_REG) */
					/*   to kernel */
	uint64_t	offset;		/* to kernel */
	uint32_t	acc_attr;	/* access attributes - to kernel */
	uint32_t	padding1;	/* 8-byte align next uint64_t for X86 */
	uint64_t	data;		/* to/from kernel, 64-bit alignment */
	uint32_t	status;		/* from kernel */
	uint32_t	padding2;	/* 8-byte align next uint64_t for X86 */
	uint64_t	phys_addr;	/* from kernel, 64-bit alignment */
} pcitool_reg_t;


#ifdef	__cplusplus
}
#endif

#endif	/* _SYS_PCI_TOOLS_H */
