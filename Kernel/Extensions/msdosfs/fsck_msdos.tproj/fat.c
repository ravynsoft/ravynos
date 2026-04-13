/*
 * Copyright (c) 2000,2005-2007,2010-2011 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * Copyright (C) 1995, 1996, 1997 Wolfgang Solfrank
 * Copyright (c) 1995 Martin Husemann
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Martin Husemann
 *	and Wolfgang Solfrank.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <sys/cdefs.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/param.h>

#include "ext.h"
#include "fsutil.h"

/*
 * The following value should be a multiple of the sector size in bytes.  The
 * Microsoft supported sector sizes are 512, 1024, 2048, and 4096, which means
 * this should be a multiple of 4096.  It should also be a minimum of 6K so
 * that a maximal FAT12 FAT can fit in a single chunk (so the code doesn't
 * have to handle a FAT entry crossing a chunk boundary).  It should be
 * large enough to make the I/O efficient, without occupying too much memory.
 */
#define FAT_CHUNK_SIZE (64*1024)
	

ssize_t	deblock_read(int d, void *buf, size_t nbytes);
ssize_t	deblock_write(int d, void *buf, size_t nbytes);

static cl_t fat32_get(cl_t cluster);
static int fat32_set(cl_t cluster, cl_t value);
static cl_t fat16_get(cl_t cluster);
static int fat16_set(cl_t cluster, cl_t value);
static cl_t fat12_get(cl_t cluster);
static int fat12_set(cl_t cluster, cl_t value);

cl_t (*fat_get)(cl_t cluster);
int (*fat_set)(cl_t cluster, cl_t value);

static int gFS;		/* File descriptor to read/write the volume */
static struct bootblock *gBoot;
static size_t gUseMapBytes;
static size_t gNumCacheBlocks;

/*
 * A cached FAT sector.
 */
struct fat_cache_block {
	unsigned int	dirty:1;
	unsigned int	chunk:31;
	struct fat_cache_block *next;	/* LRU list */
	uint8_t *		buffer;
	uint32_t		length;	/* Size of this block, in bytes */
};
enum { FAT_CHUNK_MAX = 0x7FFFFFFF };

static uint8_t *fat_cache_buffers;
static struct fat_cache_block *fat_cache;
static struct fat_cache_block *fat_cache_mru;

/*
 * Initialize the FAT structures and cache.
 *
 * Should we use a global, or return a structure to be passed
 * back into other routines?
 */
int fat_init(int fs, struct bootblock *boot)
{
	int mod = 0;
	int i;
	cl_t temp;
	cl_t value;
	
	/*
	 * We can get called multiple times if a first
	 * repair didn't work.  Be prepared to re-use
	 * or free/reallocate memory.
	 */
	fat_uninit();
	
	gFS = fs;
	gBoot = boot;
	
	/*
	 * Point fat_get and fat_set to the appropriate routine for the volume's
	 * FAT type.  (Virtual functions in C)
	 */
	switch (boot->ClustMask)
	{
		case CLUST12_MASK:
			fat_get = fat12_get;
			fat_set = fat12_set;
			break;
		case CLUST16_MASK:
			fat_get = fat16_get;
			fat_set = fat16_set;
			break;
		case CLUST32_MASK:
			fat_get = fat32_get;
			fat_set = fat32_set;
			break;
		default:
			pfatal("Unknown cluster mask (0x%08X)\n", boot->ClustMask);
			return FSFATAL;
	}
	
	if (initUseMap(boot))
		return FSFATAL;

	/*
	 * Allocate space for the FAT cache structures
	 *
	 * If maxmem is set, limit the FAT bitmap plus buffers to that amount
	 * of memory.  TODO: If the entire FAT plus bitmap would fit in less
	 * than maxmem bytes, just allocate as much as actually needed for the
	 * entire FAT.
	 *
	 * If maxmem is not set, allocate enough to cache the entire FAT.
	 * TODO: Should we use a smaller number of bigger blocks to minimize the
	 * overhead of finding a cache block?
	 */
	if (maxmem)
	{
		gNumCacheBlocks = (maxmem-gUseMapBytes)/FAT_CHUNK_SIZE;
	}
	else
	{
		gNumCacheBlocks = (boot->FATsecs*boot->BytesPerSec+FAT_CHUNK_SIZE-1)/FAT_CHUNK_SIZE;
	}
	fat_cache = calloc(gNumCacheBlocks, sizeof(struct fat_cache_block));
	if (fat_cache == NULL)
	{
		freeUseMap();
		perr("No memory for FAT cache headers\n");
		return FSFATAL;
	}
	fat_cache_buffers = calloc(gNumCacheBlocks, FAT_CHUNK_SIZE);
	if (fat_cache_buffers == NULL)
	{
		free(fat_cache);
		fat_cache = NULL;
		freeUseMap();
		perr("No memory for FAT cache buffers\n");
		return FSFATAL;
	}
	
	/*
	 * Initialize the FAT cache buffers and buffer headers
	 */
	for (i=0; i<gNumCacheBlocks; ++i)
	{
		fat_cache[i].dirty = 0;
		fat_cache[i].chunk = FAT_CHUNK_MAX;
		fat_cache[i].buffer = fat_cache_buffers + i * FAT_CHUNK_SIZE;
		if (i != gNumCacheBlocks-1)
			fat_cache[i].next = &fat_cache[i+1];
	}
	fat_cache_mru = &fat_cache[0];

	/*
	 * Check the first (index 0) entry of the FAT.  The low byte should be
	 * the same as boot->Media.  All other bits should be set.  Note that we
	 * can't rely on fat_get() sign extending the value for us, since 0xF0
	 * is a valid media type, and is less than CLUST_RSRVD, so won't get
	 * sign extended.
	 */
	value = fat_get(0);
	if (value == CLUST_ERROR)
		return FSFATAL;
	value &= boot->ClustMask;
	temp = boot->ClustMask & (0xFFFFFF00+boot->Media);
	if (value != temp)
	{
		pwarn("FAT[0] is incorrect (is 0x%X; should be 0x%X)\n", value, temp);
		if (ask(1, "Correct"))
		{
			mod = fat_set(0, temp);
			if (!mod)
				mod = FSFATMOD;
		}
		else
		{
			mod = FSERROR;
		}
	}
	
	/*
	 * Check the second (index 1) entry of the FAT.  It should be set to an
	 * end of file mark.  For FAT16 and FAT32, the upper two bits may be cleared
	 * to indicate "volume dirty" and "hard errors"; for FAT12, it must always
	 * be an EOF value.
	 *
	 * Also check the "clean" bit.  If it is not set (i.e., the volume is dirty),
	 * set the FSDIRTY status.
	 */
	value = fat_get(1);
	if (value == CLUST_ERROR)
		return FSFATAL;
	switch (boot->ClustMask)
	{
		case CLUST16_MASK:
			if ((value & 0x8000) == 0)
				mod |= FSDIRTY;
			break;
		case CLUST32_MASK:
			if ((value & 0x08000000) == 0)
				mod |= FSDIRTY;
			break;
		default:
			break;
	}
	/*
	 * Figure out how many bits of the FAT[1] entry to compare.  FAT16 and
	 * FAT32 use the upper two bits as flags, so we don't want to compare
	 * them.  The >>2 part below leaves temp set to the cluster mask, but
	 * with the upper two bits zeroed.  (Note that the cluster mask has
	 * the lower N bits set.)  FAT12 has no flags, and uses all 12 bits.
	 *
	 * The second if statement then compares the significant bits (i.e. not
	 * the flags) of the fetched value with the same bits of the expected
	 * value (CLUST_EOFS).
	 */
	if (boot->ClustMask == CLUST12_MASK)
		temp = boot->ClustMask;
	else
		temp = boot->ClustMask >> 2;
	if ((value & temp) < (CLUST_EOFS & temp))
	{
		pwarn("FAT[1] is incorrect\n");
		if (ask(1, "Correct"))
		{
			/*
			 * Set the lower bits of FAT[1] to all ones (i.e. the
			 * value in temp), but preserving the flags from the
			 * FAT[1] entry (in the upper two significant bits of
			 * value).
			 */
			i = fat_set(1, value | temp);
			if (i)
				mod |= i;
			else
				mod |= FSFATMOD;
		}
		else
		{
			mod |= FSERROR;
		}
	}
	
	return mod;
}


/*
 * Find the FAT cache block associated with the given cluster.  If not found,
 * try to find an unused cache block.  Otherwise, recycle the least recently
 * used block (writing it out first if it was dirty).  Move the found cache
 * block to the head of the most recently used list.
 *
 * Returns NULL on I/O error
 */
static struct fat_cache_block *
fat_cache_find(uint32_t offset)
{
	struct fat_cache_block *found;
	struct fat_cache_block *prev;
	uint32_t chunk;
	uint32_t length;
	
	/* Figure out which chunk we're looking for */
	chunk = offset / FAT_CHUNK_SIZE;
	length = (gBoot->FATsecs * gBoot->BytesPerSec) - (chunk * FAT_CHUNK_SIZE);
	if (length > FAT_CHUNK_SIZE)
		length = FAT_CHUNK_SIZE;

	/* Find the matching buffer, else LRU buffer */
	prev = NULL;
	found = fat_cache_mru;
	while (found->chunk != chunk && found->next != NULL)
	{
		prev = found;
		found = found->next;
	}
	
	/*
	 * If we didn't find the desired sector in the cache, recycle
	 * the least recently used buffer.
	 */
	if (found->chunk != chunk)
	{
		int activeFAT;
		off_t io_offset;

		activeFAT = gBoot->ValidFat >= 0 ? gBoot->ValidFat : 0;

		if (found->dirty)
		{
//			fprintf(stderr, "Writing FAT sector %u\n", found->sector);
			
			/* Byte offset of start of active FAT */
			io_offset = (gBoot->ResSectors + activeFAT * gBoot->FATsecs) * gBoot->BytesPerSec;
			
			/* Byte offset of current chunk */
			io_offset += found->chunk * FAT_CHUNK_SIZE;
			
			if (lseek(gFS, io_offset, SEEK_SET) != io_offset)
			{
				perr("Unable to seek FAT");
				return NULL;
			}
			if (deblock_write(gFS, found->buffer, found->length) != found->length)
			{
				perr("Unable to write FAT");
				return NULL;
			}
			
			found->dirty = 0;
		}
		
		/* Figure out where the desired chunk is on disk */
		found->chunk = chunk;
		found->length = length;
		
		/* Byte offset of start of active FAT */
		io_offset = (gBoot->ResSectors + activeFAT * gBoot->FATsecs) * gBoot->BytesPerSec;
		
		/* Byte offset of desired chunk */
		io_offset += chunk * FAT_CHUNK_SIZE;
		
		/* Read in the sector */
//		fprintf(stderr, "Reading FAT sector %u\n", found->sector);
		if (lseek(gFS, io_offset, SEEK_SET) != io_offset)
		{
			perr("Unable to seek FAT");
			return NULL;
		}
		if (deblock_read(gFS, found->buffer, length) != length)
		{
			perr("Unable to read FAT");
			return NULL;
		}
	}
	
	/*
	 * Move the found buffer to the head of the MRU list.
	 */
	if (found != fat_cache_mru)
	{
		if (prev)
			prev->next = found->next;
		found->next = fat_cache_mru;
		fat_cache_mru = found;
	}
	
	return found;
}


void fat_uninit(void)
{
	if (fat_cache != NULL)
	{
		free(fat_cache);
		fat_cache = NULL;
	}
	if (fat_cache_buffers != NULL)
	{
		free(fat_cache_buffers);
		fat_cache_buffers = NULL;
	}
	freeUseMap();
}


/*
 * Returns the entry for cluster @cluster in @*value.
 * The function result is the error, if any.
 */
static cl_t fat32_get(cl_t cluster)
{
	struct fat_cache_block *block;
	uint8_t *p;
	cl_t value;
	
	if (cluster >= gBoot->NumClusters)
	{
		fprintf(stderr, "fat32_get: invalid cluster (%u)\n", cluster);
		return CLUST_ERROR;
	}
	
	block = fat_cache_find(cluster*4);
	if (block == NULL)
		return CLUST_ERROR;
	
	/* Point to the @cluster'th entry. */
	p = block->buffer + (cluster * 4) % FAT_CHUNK_SIZE;
	
	/* Extract the raw value, ignoring the reserved bits. */
	value = (p[0] + (p[1] << 8) + (p[2] << 16) + (p[3] << 24)) & CLUST32_MASK;
	
	/* Sign extended the special values for consistency. */
	if (value >= (CLUST_RSRVD & CLUST32_MASK))
		value |= ~CLUST32_MASK;

	return value;
}

static cl_t fat16_get(cl_t cluster)
{
	struct fat_cache_block *block;
	uint8_t *p;
	cl_t value;
	
	if (cluster >= gBoot->NumClusters)
	{
		fprintf(stderr, "fat16_get: invalid cluster (%u)\n", cluster);
		return CLUST_ERROR;
	}
	
	block = fat_cache_find(cluster*2);
	if (block == NULL)
		return CLUST_ERROR;
	
	/* Point to the @cluster'th entry. */
	p = block->buffer + (cluster * 2) % FAT_CHUNK_SIZE;
	
	/* Extract the value. */
	value = p[0] + (p[1] << 8);
	
	/* Sign extended the special values for consistency. */
	if (value >= (CLUST_RSRVD & CLUST16_MASK))
		value |= ~CLUST16_MASK;

	return value;
}

static cl_t fat12_get(cl_t cluster)
{
	struct fat_cache_block *block;
	uint8_t *p;
	cl_t value;

	if (cluster >= gBoot->NumClusters)
	{
		fprintf(stderr, "fat16_get: invalid cluster (%u)\n", cluster);
		return CLUST_ERROR;
	}
	
	block = fat_cache_find(cluster + cluster/2);
	if (block == NULL)
		return CLUST_ERROR;
	
	/* Point to the @cluster'th entry. */
	p = block->buffer + (cluster + cluster/2) % FAT_CHUNK_SIZE;
	
	/* Extract the value. */
	value = p[0] + (p[1] << 8);
	if (cluster & 1)
		value >>= 4;
	else
		value &= 0x0FFF;
	
	/* Sign extended the special values for consistency. */
	if (value >= (CLUST_RSRVD & CLUST12_MASK))
		value |= ~CLUST12_MASK;

	return value;
}


/*
 * Sets the entry for cluster @cluster to @value.
 * The function result is the error, if any.
 *
 * For now, this is FAT32-only.
 */
static int fat32_set(cl_t cluster, cl_t value)
{
	struct fat_cache_block *block;
	uint8_t *p;

	if (cluster >= gBoot->NumClusters)
	{
		fprintf(stderr, "fat32_set: invalid cluster (%u)\n", cluster);
		return FSFATAL;
	}

	block = fat_cache_find(cluster*4);
	if (block == NULL)
		return FSFATAL;
	
	/* Point to the @cluster'th entry in the FAT. */
	p = block->buffer + (cluster * 4) % FAT_CHUNK_SIZE;
	
	/* Store the value, preserving the reserved bits. */
	*p++ = (uint8_t) value;
	*p++ = (uint8_t) (value >> 8);
	*p++ = (uint8_t) (value >> 16);
	*p &= 0xF0;
	*p |= (value >> 24) & 0x0F;
	
	/* Update the use map. */
	if (value == CLUST_FREE)
		markFree(cluster);
	else
		markUsed(cluster);
	
	/* Remember that this block has been changed. */
	block->dirty = 1;
	
	return 0;
}

static int fat16_set(cl_t cluster, cl_t value)
{
	struct fat_cache_block *block;
	uint8_t *p;

	if (cluster >= gBoot->NumClusters)
	{
		fprintf(stderr, "fat16_set: invalid cluster (%u)\n", cluster);
		return FSFATAL;
	}
	
	block = fat_cache_find(cluster*2);
	if (block == NULL)
		return FSFATAL;
	
	/* Point to the @cluster'th entry in the FAT. */
	p = block->buffer + (cluster * 2) % FAT_CHUNK_SIZE;
	
	/* Store the value. */
	*p++ = (uint8_t) value;
	*p   = (uint8_t) (value >> 8);
	
	/* Update the use map. */
	if (value == CLUST_FREE)
		markFree(cluster);
	else
		markUsed(cluster);
	
	/* Remember that this block has been changed. */
	block->dirty = 1;
	
	return 0;
}

static int fat12_set(cl_t cluster, cl_t value)
{
	struct fat_cache_block *block;
	uint8_t *p;

	if (cluster >= gBoot->NumClusters)
	{
		fprintf(stderr, "fat16_set: invalid cluster (%u)\n", cluster);
		return FSFATAL;
	}
	
	block = fat_cache_find(cluster + cluster/2);
	if (block == NULL)
		return FSFATAL;
	
	/* Point to the @cluster'th entry in the FAT. */
	p = block->buffer + (cluster + cluster/2) % FAT_CHUNK_SIZE;

	value &= 0xFFF;
	
	/* Mix the new value with other nibble. */
	if (cluster & 1)
		value = (value << 4) | (p[0] & 0x0F);
	else
		value |= (p[1] & 0xF0) << 8;
	*p++ = (uint8_t) value;
	*p   = (uint8_t) (value >> 8);
	
	/* Update the use map. */
	if (value == CLUST_FREE)
		markFree(cluster);
	else
		markUsed(cluster);
	
	/* Remember that this block has been changed. */
	block->dirty = 1;
	
	return 0;
}


/*
 * If the FAT has been modified, write it back to disk.
 */
int fat_flush(void)
{
	int i;
	int activeFAT;
	off_t offset;
	
	activeFAT = gBoot->ValidFat >= 0 ? gBoot->ValidFat : 0;
	
	for (i=0; i<gNumCacheBlocks; ++i)
	{
		if (fat_cache[i].dirty)
		{
			/* Byte offset of start of active FAT */
			offset = (gBoot->ResSectors + activeFAT * gBoot->FATsecs) * gBoot->BytesPerSec;
			
			/* Byte offset of current chunk */
			offset += fat_cache[i].chunk * FAT_CHUNK_SIZE;
			
//			fprintf(stderr, "Flushing FAT sector %u\n", fat_cache[i].sector);
			if (lseek(gFS, offset, SEEK_SET) != offset)
			{
				perr("Unable to seek FAT");
				return FSFATAL;
			}
			if (deblock_write(gFS, fat_cache[i].buffer, fat_cache[i].length) != fat_cache[i].length)
			{
				perr("Unable to write FAT");
				return FSFATAL;
			}
			
			fat_cache[i].dirty = 0;
		}
	}

	return 0;
}


/*
 * Mark all unreferenced clusters as CLUST_FREE.  Also calculate
 * the number of free clusters.
 */
int fat_free_unused(void)
{
	int err = 0;
	cl_t cluster, value;
	cl_t count = 0;
	int fix=0;
	
	for (cluster = CLUST_FIRST; cluster < gBoot->NumClusters; ++cluster)
	{
		value = fat_get(cluster);
		if (value == CLUST_ERROR)
			break;
		if (!isUsed(cluster))
		{
			if (value == CLUST_BAD)
			{
				gBoot->NumBad++;
			}
			else if (value == CLUST_FREE)
			{
				gBoot->NumFree++;
			}
			else
			{
				if (count == 0)
				{
					pwarn("Found orphan cluster(s)\n");
					fix = ask(1, "Fix");
				}
				++count;
				if (fix)
				{
					err = fat_set(cluster, CLUST_FREE);
					if (err)
						break;
					gBoot->NumFree++;
				}
			}
		}
	}

	if (count)
	{
		if (fix)
		{
			pwarn("Marked %u clusters as free\n", count);
			err |= FSFATMOD;
		}
		else
		{
			pwarn("Found %u orphaned clusters\n", count);
			err |= FSERROR;
		}
	}

	/*
	 * Check the FSInfo sector's "free space" value.
	 *
	 * NOTE: Since the value there is documented as not reliable, we don't
	 * return a non-zero exit status if the values are unexpected.
	 */
	if (gBoot->FSInfo) {
		fix = 0;
		if (gBoot->FSFree != gBoot->NumFree) {
			if (gBoot->FSFree == 0xFFFFFFFFU)
				pwarn("Free space in FSInfo block is unset (should be %d)\n",
				      gBoot->NumFree);
			else
				pwarn("Free space in FSInfo block (%d) not correct (%d)\n",
				      gBoot->FSFree, gBoot->NumFree);
			if (ask(1, "Fix")) {
				gBoot->FSFree = gBoot->NumFree;
				fix = 1;
			}
		}
		if (fix)
			err |= writefsinfo(gFS, gBoot);
	}

	return err;
}


/*
 * Determine whether a volume is dirty, without reading the entire FAT.
 */
int isdirty(int fs, struct bootblock *boot, int fat)
{
       int             result;
       u_char  *buffer;
       off_t   offset;

       result = 1;             /* In case of error, assume volume was dirty */

       /* FAT12 volumes don't have a "clean" bit, so always assume they are dirty */
       if (boot->ClustMask == CLUST12_MASK)
               return 1;

       buffer = malloc(boot->BytesPerSec);
       if (buffer == NULL) {
               perr("No space for FAT sector");
               return 1;               /* Assume it was dirty */
       }

       offset = boot->ResSectors + fat * boot->FATsecs;
       offset *= boot->BytesPerSec;

       if (lseek(fs, offset, SEEK_SET) != offset) {
               perr("Unable to read FAT");
               goto ERROR;
       }

       if (deblock_read(fs, buffer, boot->BytesPerSec) != boot->BytesPerSec) {
               perr("Unable to read FAT");
               goto ERROR;
       }

       switch (boot->ClustMask) {
       case CLUST32_MASK:
               /* FAT32 uses bit 27 of FAT[1] */
               if ((buffer[7] & 0x08) != 0)
                       result = 0;             /* It's clean */
               break;
       case CLUST16_MASK:
               /* FAT16 uses bit 15 of FAT[1] */
               if ((buffer[3] & 0x80) != 0)
                       result = 0;             /* It's clean */
               break;
       }

ERROR:
       free(buffer);
       return result;
}


/*
 * Mark a FAT16 or FAT32 volume "clean."  Ignored for FAT12.
 */
int fat_mark_clean(void)
{
	cl_t value;
	
	/*
	 * FAT12 does not have a "dirty" bit, so do nothing.
	 */
	if (gBoot->ClustMask == CLUST12_MASK)
		return 0;
	
	value = fat_get(1);
	if (value == CLUST_ERROR)
		return FSERROR;
	
	if (gBoot->ClustMask == CLUST16_MASK)
		value |= 0x8000;
	else
		value |= 0x08000000;
	return fat_set(1, value);
}


/*
 * Get type of reserved cluster
 */
char *
rsrvdcltype(cl)
	cl_t cl;
{
	if (cl == CLUST_FREE)
		return "free";
	if (cl < CLUST_BAD)
		return "reserved";
	if (cl > CLUST_BAD)
		return "as EOF";
	return "bad";
}

#define DEBLOCK_SIZE 		(MAXPHYSIO>>2)
ssize_t	deblock_read(int d, void *buf, size_t nbytes) {
    ssize_t		totbytes = 0, readbytes;
    char		*b = buf;

    while (nbytes > 0) {
        size_t 		rbytes = nbytes < DEBLOCK_SIZE? nbytes : DEBLOCK_SIZE;
        readbytes = read(d, b, rbytes);
        if (readbytes < 0)
            return readbytes;
        else if (readbytes == 0)
            break;
        else {
            nbytes-=readbytes;
            totbytes += readbytes;
            b += readbytes;
        }
    }

    return totbytes;
}

ssize_t	deblock_write(int d, void *buf, size_t nbytes) {
    ssize_t		totbytes = 0, writebytes;
    char		*b = buf;

    while (nbytes > 0) {
        size_t 		wbytes = nbytes < DEBLOCK_SIZE ? nbytes : DEBLOCK_SIZE;
        writebytes = write(d, b, wbytes);
        if (writebytes < 0)
            return writebytes;
        else if (writebytes == 0)
            break;
        else {
            nbytes-=writebytes;
            totbytes += writebytes;
            b += writebytes;
        }
    }

    return totbytes;
}


/*======================================================================
	Cluster Use Map Routines
	
	These routines keep track of which clusters have been referenced
	(by a directory entry).
	
	Right now, the implementation is a simple bitmap.  But it could be
	replaced with something else (list of allocated ranges, etc.).
======================================================================*/

static uint32_t *useMap = NULL;

int initUseMap(struct bootblock *boot)
{
	/* Round up clusters to a multiple of 32 */
	cl_t clusters = (boot->NumClusters + 31) & ~31;
	if (useMap != NULL)
		free(useMap);
	gUseMapBytes = clusters/8;
	if (maxmem != 0 && maxmem < (gUseMapBytes + FAT_CHUNK_SIZE))
	{
		pfatal("Cannot allocate %zd bytes for usemap (maxmem=%zd, clusters=%d)\n"
			"maxmem must be at least %zd\n",
			gUseMapBytes, maxmem, clusters, gUseMapBytes + FAT_CHUNK_SIZE);
		useMap = NULL;
	}
	else
	{
		useMap = calloc(clusters/32, sizeof(uint32_t));
	}
	return useMap==NULL;
}

void freeUseMap(void)
{
	if (useMap != NULL)
		free(useMap);
	useMap = NULL;
}

int isUsed(cl_t cluster)
{
	return (useMap[cluster/32] >> cluster%32) & 1;
}

int markUsed(cl_t cluster)
{
	int error = 0;
	cl_t index = cluster / 32;
	uint32_t mask = 1 << (cluster % 32);
	
	if (useMap[index] & mask)
		error = 1;
	else
		useMap[index] |= mask;

	return error;
}

int markFree(cl_t cluster)
{
	int error = 0;
	cl_t index = cluster / 32;
	uint32_t mask = 1 << (cluster % 32);
	
	if (useMap[index] & mask)
		useMap[index] &= ~mask;
	else
		error = 1;

	return error;
}
