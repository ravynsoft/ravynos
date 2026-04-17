/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/* This file has been modified to support C++ initialization for the ravynOS
 * project in March 2026. This notice is in support of Clause 2.2 of the
 * License.
 */

#include <mach/mach_types.h>
#include <libkern/OSKextLib.h>

// These global symbols will be defined by CreateInfo script's info.c file.
extern kmod_start_func_t *_realmain;
extern kmod_info_t KMOD_INFO_NAME;

struct mach_header_64 {
	uint32_t	magic;		/* mach magic number identifier */
	cpu_type_t	cputype;	/* cpu specifier */
	cpu_subtype_t	cpusubtype;	/* machine specifier */
	uint32_t	filetype;	/* type of file */
	uint32_t	ncmds;		/* number of load commands */
	uint32_t	sizeofcmds;	/* the size of all the load commands */
	uint32_t	flags;		/* flags */
	uint32_t	reserved;	/* reserved */
};

struct segment_command_64 { /* for 64-bit architectures */
	uint32_t	cmd;		/* LC_SEGMENT_64 */
	uint32_t	cmdsize;	/* includes sizeof section_64 structs */
	char		segname[16];	/* segment name */
	uint64_t	vmaddr;		/* memory address of this segment */
	uint64_t	vmsize;		/* memory size of this segment */
	uint64_t	fileoff;	/* file offset of this segment */
	uint64_t	filesize;	/* amount to map from the file */
	vm_prot_t	maxprot;	/* maximum VM protection */
	vm_prot_t	initprot;	/* initial VM protection */
	uint32_t	nsects;		/* number of sections in segment */
	uint32_t	flags;		/* flags */
};

struct section_64 { /* for 64-bit architectures */
	char		sectname[16];	/* name of this section */
	char		segname[16];	/* segment this section goes in */
	uint64_t	addr;		/* memory address of this section */
	uint64_t	size;		/* size in bytes of this section */
	uint32_t	offset;		/* file offset of this section */
	uint32_t	align;		/* section alignment (power of 2) */
	uint32_t	reloff;		/* file offset of relocation entries */
	uint32_t	nreloc;		/* number of relocation entries */
	uint32_t	flags;		/* flags (section type and attributes)*/
	uint32_t	reserved1;	/* reserved (for offset or index) */
	uint32_t	reserved2;	/* reserved (for count or sizeof) */
	uint32_t	reserved3;	/* reserved */
};

#define LC_SEGMENT_64 0x19
typedef void (*structor_t)(void);
extern void kprintf(const char *fmt, ...);

static void
__call_constructors(kmod_info_t *ki)
{
    struct mach_header_64 * mh = (struct mach_header_64 *) ki->address;
    struct segment_command_64 * seg =
        (struct segment_command_64 *) ((uint64_t) mh + sizeof(struct mach_header_64));

    void * end = (void *) ((uint64_t)mh + mh->sizeofcmds);

    while (seg < end) {
        if (seg->cmd == LC_SEGMENT_64) {
            if (!strcmp(seg->segname, "__DATA")) {
                struct section_64 * sect =
                    (struct section_64 *) ((uint64_t)seg + sizeof(struct segment_command_64));
                while (sect < (uint64_t)seg + seg->cmdsize) {
                    if (!strcmp(sect->sectname, "__mod_init_func")) {
                        structor_t *ctor = (structor_t *) sect->addr;
                        while (ctor < (uint64_t)sect->addr + sect->size) {
                            if (*ctor) {
                                (**ctor)();
                            }
                            ctor++;
                        }
                        return;
                    }
                    sect = (struct section_64 *) ((uint64_t)sect + sizeof(struct section_64));
                }
            }
        }
        seg = (struct segment_command_64 *) ((uint64_t)seg + seg->cmdsize);
    }
}

/*********************************************************************
*********************************************************************/
__private_extern__ kern_return_t
_start(kmod_info_t *ki, void *data)
{
        __call_constructors(ki);
        
	if (_realmain) {
		return (*_realmain)(ki, data);
	} else {
		return KERN_SUCCESS;
	}
}

/*********************************************************************
*********************************************************************/
__private_extern__ const char *
OSKextGetCurrentIdentifier(void)
{
	return KMOD_INFO_NAME.name;
}

/*********************************************************************
*********************************************************************/
__private_extern__ const char *
OSKextGetCurrentVersionString(void)
{
	return KMOD_INFO_NAME.version;
}

/*********************************************************************
*********************************************************************/
__private_extern__ OSKextLoadTag
OSKextGetCurrentLoadTag(void)
{
	return (OSKextLoadTag)KMOD_INFO_NAME.id;
}

__private_extern__ void
__cxa_atexit(void)
{
	return;
}
