/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright 2008 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <sys/mman.h>
#include "libelf.h"
#include "decl.h"
#include "msg.h"
#include <string.h>

/*
 * File input
 *	These functions read input files.
 *	On SVR4 and newer systems use mmap(2).  On older systems (or on
 *	file systems that don't support mmap, this code simulates mmap.
 *	When reading a file, enough memory is allocated to hold the file's
 *	image, and reads are delayed.  When another part of the library
 *	wants to use a part of the file, it "fetches" the needed regions.
 *
 *	An elf descriptor has a bit array to manage this.  Each bit
 *	represents one "page" of the file.  Pages are grouped into regions.
 *	The page size is tunable.  Its value should be at least one disk
 *	block and small enough to avoid superfluous traffic.
 *
 *	NBITS	The number of bits in an unsigned.  Each unsigned object
 *		holds a "REGION."  A byte must have at least 8 bits;
 *		it may have more, though the extra bits at the top of
 *		the unsigned will be unused.  Thus, for 9-bit bytes and
 *		36-bit words, 4 bits at the top will stay empty.
 *
 *	This mechanism gives significant performance gains for library
 *	handling (among other things), because programs typically don't
 *	need to look at entire libraries.  The fastest I/O is no I/O.
 */

/*
 * This global is used to hold the value of the PAGESIZE macro.
 *
 * This is because the PAGESIZE macro actually calls the
 * sysconfig(_CONFIG_PAGESIZE) system call and we don't want
 * to repeatedly call this through out libelf.
 */
static unsigned long	_elf_pagesize = 0;
NOTE(SCHEME_PROTECTS_DATA("read only data", _elf_pagesize))


#define	NBITS		(8 * sizeof (unsigned))
#define	REGSZ		(NBITS * _elf_pagesize)
#define	PGNUM(off)	((off % REGSZ) / _elf_pagesize)
#define	REGNUM(off)	(off / REGSZ)



Okay
_elf_vm(Elf * elf, size_t base, size_t sz)
{
	NOTE(ASSUMING_PROTECTED(*elf))
	register unsigned	*hdreg, hdbit;
	unsigned		*tlreg, tlbit;
	size_t			tail;
	off_t			off;
	Elf_Void		*iop;


	/*
	 * always validate region
	 */

	if ((base + sz) > elf->ed_fsz) {
		/*
		 * range outside of file bounds.
		 */
		_elf_seterr(EFMT_VM, 0);
		return (OK_NO);
	}

	/*
	 * If file is mmap()'d and/or the read size is 0
	 * their is nothing else for us to do.
	 */
	if (elf->ed_vm == 0 || sz == 0)
		return (OK_YES);
	/*
	 * This uses arithmetic instead of masking because
	 * sizeof (unsigned) might not be a power of 2.
	 *
	 * Tail gives one beyond the last offset that must be retrieved,
	 * NOT the last in the region.
	 */

	if (elf->ed_parent && elf->ed_parent->ed_fd == -1)
		elf->ed_fd = -1;

	base += elf->ed_baseoff;
	tail = base + sz + _elf_pagesize - 1;
	off = base - base % _elf_pagesize;
	hdbit = 1 << PGNUM(base);
	tlbit = 1 << PGNUM(tail);
	hdreg = &elf->ed_vm[REGNUM(base)];
	tlreg = &elf->ed_vm[REGNUM(tail)];
	sz = 0;

	/*
	 * Scan through the files 'page table' and make sure
	 * that all of the pages in the specified range have been
	 * loaded into memory.  As the pages are loaded the appropriate
	 * bit in the 'page table' is set.
	 *
	 * Note: This loop will only read in those pages which havn't
	 *	 been previously loaded into memory, if the page is
	 *	 already present it will not be re-loaded.
	 */
	while ((hdreg != tlreg) || (hdbit != tlbit)) {
		if (*hdreg & hdbit) {
			if (sz != 0) {
				/*
				 * Read in a 'chunk' of the elf image.
				 */
				iop = (Elf_Void *)(elf->ed_image + off);
				/*
				 * do not read past the end of the file
				 */
				if (elf->ed_imagesz - off < sz)
					sz = elf->ed_imagesz - off;
				if ((lseek(elf->ed_fd, off,
				    SEEK_SET) != off) ||
				    (read(elf->ed_fd, iop, sz) != sz)) {
					_elf_seterr(EIO_VM, errno);
					return (OK_NO);
				}
				off += sz;
				sz = 0;
			}
			off += _elf_pagesize;
		} else {
			if (elf->ed_fd < 0) {
				_elf_seterr(EREQ_NOFD, 0);
				return (OK_NO);
			}
			sz += _elf_pagesize;
			*hdreg |= hdbit;
		}
		if (hdbit == ((unsigned)1 << (NBITS - 1))) {
			hdbit = 1;
			++hdreg;
		} else
			hdbit <<= 1;
	}

	if (sz != 0) {
		iop = (Elf_Void *)(elf->ed_image + off);
		/*
		 * do not read past the end of the file
		 */
		if ((elf->ed_imagesz - off) < sz)
			sz = elf->ed_imagesz - off;
		if ((lseek(elf->ed_fd, off, SEEK_SET) != off) ||
		    (read(elf->ed_fd, iop, sz) != sz)) {
			_elf_seterr(EIO_VM, errno);
			return (OK_NO);
		}
	}
	return (OK_YES);
}


Okay
_elf_inmap(Elf * elf)
{
	int		fd = elf->ed_fd;
	register size_t	sz;

	{
		register off_t	off = lseek(fd, (off_t)0, SEEK_END);

		if (off == 0)
			return (OK_YES);

		if (off == -1) {
			_elf_seterr(EIO_FSZ, errno);
			return (OK_NO);
		}

		if ((sz = (size_t)off) != off) {
			_elf_seterr(EIO_FBIG, 0);
			return (OK_NO);
		}
	}
	/*
	 *	If the file is mapped, elf->ed_vm will stay null
	 *	and elf->ed_image will need to be unmapped someday.
	 *	If the file is read, elf->ed_vm and the file image
	 *	are allocated together; free() elf->ed_vm.
	 *
	 *	If the file can be written, disallow mmap.
	 *	Otherwise, the input mapping and the output mapping
	 *	can collide.  Moreover, elf_update will truncate
	 *	the file, possibly invalidating the input mapping.
	 *	Disallowing input mmap forces the library to malloc
	 *	and read the space, which will make output mmap safe.
	 *	Using mmap for output reduces the swap space needed
	 *	for the process, so that is given preference.
	 */

	{
		register char	*p;

		/* The embedded build won't let us reprotect this memory with write
		 * permissions later unless we give it write permissions now
		 */
		if ((elf->ed_myflags & EDF_WRITE) == 0 &&
		    (p = mmap((char *)0, sz, PROT_READ|PROT_WRITE,
		    MAP_PRIVATE, fd, (off_t)0)) != (char *)-1) {
			elf->ed_image = elf->ed_ident = p;
			elf->ed_imagesz = elf->ed_fsz = elf->ed_identsz = sz;
			return (OK_YES);
		}
	}

	if (_elf_pagesize == 0)
		_elf_pagesize = PAGESIZE;

	/*
	 * If mmap fails, try read.  Some file systems don't mmap
	 */
	{
		register size_t	vmsz = sizeof (unsigned) * (REGNUM(sz) + 1);

		if (vmsz % sizeof (Elf64) != 0)
			vmsz += sizeof (Elf64) - vmsz % sizeof (Elf64);
		if ((elf->ed_vm = (unsigned *)malloc(vmsz + sz)) == 0) {
			_elf_seterr(EMEM_VM, errno);
			return (OK_NO);
		}
		(void) memset(elf->ed_vm, 0, vmsz);
		elf->ed_vmsz = vmsz / sizeof (unsigned);
		elf->ed_image = elf->ed_ident = (char *)elf->ed_vm + vmsz;
		elf->ed_imagesz = elf->ed_fsz = elf->ed_identsz = sz;
	}
	return (_elf_vm(elf, (size_t)0, (size_t)1));
}


void
_elf_unmap(char * p, size_t sz)
{
	if (p == 0 || sz == 0)
		return;
	(void) munmap(p, sz);
}
