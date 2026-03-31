/*
 * Common "util" functions
 * This file is part of the dmidecode project.
 *
 *   Copyright (C) 2002-2023 Jean Delvare <jdelvare@suse.de>
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
 */

#include <sys/types.h>
#include <sys/stat.h>

#include "config.h"

#ifdef USE_MMAP
#include <sys/mman.h>
#ifndef MAP_FAILED
#define MAP_FAILED ((void *) -1)
#endif /* !MAP_FAILED */
#endif /* USE MMAP */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "types.h"
#include "util.h"

static int myread(int fd, u8 *buf, size_t count, const char *prefix)
{
	ssize_t r = 1;
	size_t r2 = 0;

	while (r2 != count && r != 0)
	{
		r = read(fd, buf + r2, count - r2);
		if (r == -1)
		{
			if (errno != EINTR)
			{
				perror(prefix);
				return -1;
			}
		}
		else
			r2 += r;
	}

	if (r2 != count)
	{
		fprintf(stderr, "%s: Unexpected end of file\n", prefix);
		return -1;
	}

	return 0;
}

int checksum(const u8 *buf, size_t len)
{
	u8 sum = 0;
	size_t a;

	for (a = 0; a < len; a++)
		sum += buf[a];
	return (sum == 0);
}

/*
 * Reads all of file from given offset, up to max_len bytes.
 * A buffer of at most max_len bytes is allocated by this function, and
 * needs to be freed by the caller.
 * This provides a similar usage model to mem_chunk()
 *
 * Returns a pointer to the allocated buffer, or NULL on error, and
 * sets max_len to the length actually read.
 */
void *read_file(off_t base, size_t *max_len, const char *filename)
{
	struct stat statbuf;
	int fd;
	u8 *p;

	/*
	 * Don't print error message on missing file, as we will try to read
	 * files that may or may not be present.
	 */
	if ((fd = open(filename, O_RDONLY)) == -1)
	{
		if (errno != ENOENT)
			perror(filename);
		return NULL;
	}

	/*
	 * Check file size, don't allocate more than can be read.
	 */
	if (fstat(fd, &statbuf) == 0)
	{
		if (base >= statbuf.st_size)
		{
			fprintf(stderr, "%s: Can't read data beyond EOF\n",
				filename);
			p = NULL;
			goto out;
		}
		if (*max_len > (size_t)statbuf.st_size - base)
			*max_len = statbuf.st_size - base;
	}

	if ((p = malloc(*max_len)) == NULL)
	{
		perror("malloc");
		goto out;
	}

	if (lseek(fd, base, SEEK_SET) == -1)
	{
		fprintf(stderr, "%s: ", filename);
		perror("lseek");
		goto err_free;
	}

	if (myread(fd, p, *max_len, filename) == 0)
		goto out;

err_free:
	free(p);
	p = NULL;

out:
	if (close(fd) == -1)
		perror(filename);

	return p;
}

#ifdef USE_MMAP
static void safe_memcpy(void *dest, const void *src, size_t n)
{
#ifdef USE_SLOW_MEMCPY
	size_t i;

	for (i = 0; i < n; i++)
		*((u8 *)dest + i) = *((const u8 *)src + i);
#else
	memcpy(dest, src, n);
#endif /* USE_SLOW_MEMCPY */
}
#endif /* USE_MMAP */

/*
 * Copy a physical memory chunk into a memory buffer.
 * This function allocates memory.
 */
void *mem_chunk(off_t base, size_t len, const char *devmem)
{
	struct stat statbuf;
	void *p = NULL;
	int fd;
#ifdef USE_MMAP
	off_t mmoffset;
	void *mmp;
#endif

	/*
	 * Safety check: if running as root, devmem is expected to be a
	 * character device file.
	 */
	if ((fd = open(devmem, O_RDONLY)) == -1
	 || fstat(fd, &statbuf) == -1
	 || (geteuid() == 0 && !S_ISCHR(statbuf.st_mode)))
	{
		fprintf(stderr, "Can't read memory from %s\n", devmem);
		if (fd == -1)
			return NULL;
		goto out;
	}

	if ((p = malloc(len)) == NULL)
	{
		perror("malloc");
		goto out;
	}

#ifdef USE_MMAP
	/*
	 * mmap() will fail with SIGBUS if trying to map beyond the end of
	 * the file.
	 */
	if (S_ISREG(statbuf.st_mode) && base + (off_t)len > statbuf.st_size)
	{
		fprintf(stderr, "mmap: Can't map beyond end of file %s\n",
			devmem);
		goto err_free;
	}

#ifdef _SC_PAGESIZE
	mmoffset = base % sysconf(_SC_PAGESIZE);
#else
	mmoffset = base % getpagesize();
#endif /* _SC_PAGESIZE */
	/*
	 * Please note that we don't use mmap() for performance reasons here,
	 * but to workaround problems many people encountered when trying
	 * to read from /dev/mem using regular read() calls.
	 */
	mmp = mmap(NULL, mmoffset + len, PROT_READ, MAP_SHARED, fd, base - mmoffset);
	if (mmp == MAP_FAILED)
		goto try_read;

	safe_memcpy(p, (u8 *)mmp + mmoffset, len);

	if (munmap(mmp, mmoffset + len) == -1)
	{
		fprintf(stderr, "%s: ", devmem);
		perror("munmap");
	}

	goto out;

try_read:
#endif /* USE_MMAP */
	if (lseek(fd, base, SEEK_SET) == -1)
	{
		fprintf(stderr, "%s: ", devmem);
		perror("lseek");
		goto err_free;
	}

	if (myread(fd, p, len, devmem) == 0)
		goto out;

err_free:
	free(p);
	p = NULL;

out:
	if (close(fd) == -1)
		perror(devmem);

	return p;
}

/* Returns end - start + 1, assuming start < end */
u64 u64_range(u64 start, u64 end)
{
	u64 res;

	res.h = end.h - start.h;
	res.l = end.l - start.l;

	if (end.l < start.l)
		res.h--;
	if (++res.l == 0)
		res.h++;

	return res;
}
