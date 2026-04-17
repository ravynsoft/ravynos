/*
 * Copyright (c) 2000, 2005-2008 Apple Inc. All rights reserved.
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
 * Copyright (C) 1995, 1997 Wolfgang Solfrank
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
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <os/overflow.h>

#include "ext.h"
#include "fsutil.h"

int
readboot(dosfs, boot)
	int dosfs;
	struct bootblock *boot;
{
	u_char block[MAX_SECTOR_SIZE];
	u_char fsinfo[MAX_SECTOR_SIZE];
    u_int32_t result = 0;
	int ret = FSOK;
	
	/*
	 * [2734381] Some devices have sector sizes greater than 512 bytes.  These devices
	 * tend to return errors if you try to read less than a sector, so we try reading
	 * the maximum sector size (which may end up reading more than one sector).
	 */
	if (read(dosfs, block, MAX_SECTOR_SIZE) != MAX_SECTOR_SIZE) {
		perr("could not read boot block");
		return FSFATAL;
	}

	/* [2699033]
	*
	* The first three bytes are an Intel x86 jump instruction.  It should be one
	* of the following forms:
	*    0xE9 0x?? 0x??
	*    0xEB 0x?? 0x90
	*
	* [5016947]
	*
	* Windows doesn't actually check the third byte if the first byte is 0xEB,
	* so we don't either
	*/
	if (block[0] != 0xE9 && block[0] != 0xEB)
	{
		pfatal("Invalid BS_jmpBoot in boot block: %02x%02x%02x\n", block[0], block[1], block[2]);
		return FSFATAL;
	}

	memset(boot, 0, sizeof *boot);
	boot->ValidFat = -1;

	/* decode bios parameter block */
	boot->BytesPerSec = block[11] + (block[12] << 8);
	boot->SecPerClust = block[13];
	boot->ResSectors = block[14] + (block[15] << 8);
	boot->FATs = block[16];
	boot->RootDirEnts = block[17] + (block[18] << 8);
	boot->Sectors = block[19] + (block[20] << 8);
	boot->Media = block[21];
	boot->FATsmall = block[22] + (block[23] << 8);
	boot->SecPerTrack = block[24] + (block[25] << 8);
	boot->Heads = block[26] + (block[27] << 8);
	boot->HiddenSecs = block[28] + (block[29] << 8) + (block[30] << 16) + (block[31] << 24);
	boot->HugeSectors = block[32] + (block[33] << 8) + (block[34] << 16) + (block[35] << 24);

	boot->FATsecs = boot->FATsmall;
	boot->ClusterSize = boot->BytesPerSec * boot->SecPerClust;

	/*
	 * Make sure the sector size is a power of two, and in the range
	 * 512..MAX_SECTOR_SIZE.
	 */
	if (boot->BytesPerSec < DOSBOOTBLOCKSIZE || boot->BytesPerSec > MAX_SECTOR_SIZE ||
	    (boot->BytesPerSec & (boot->BytesPerSec - 1)) != 0)
	{
		pfatal("Invalid sector size: %u\n", boot->BytesPerSec);
		return FSFATAL;
	}
	
	/*
	 * Make sure the sectors per cluster is reasonable.  It must be a
	 * non-zero power of two.
	 *
	 * The FAT specification warns that the bytes per cluster shouldn't be
	 * allowed to be greater than 32KB (with 64KB supported on some systems),
	 * but we don't actually enforce that here.
	 */
	if (boot->SecPerClust == 0 ||
	    (boot->SecPerClust & (boot->SecPerClust - 1)) != 0)
	{
		pfatal("Invalid sectors per cluster: %u\n", boot->SecPerClust);
		return FSFATAL;
	}

	if (!boot->RootDirEnts)
		boot->flags |= FAT32;
	if (boot->flags & FAT32) {
		boot->FATsecs = block[36] + (block[37] << 8)
				+ (block[38] << 16) + (block[39] << 24);

		if (block[40] & 0x80)
			boot->ValidFat = block[40] & 0x0f;

		/* check version number: */
		if (block[42] || block[43]) {
			/* Correct?				XXX */
			pfatal("Unknown filesystem version: %x.%x\n",
			       block[43], block[42]);
			return FSFATAL;
		}
		boot->RootCl = block[44] + (block[45] << 8)
			       + (block[46] << 16) + (block[47] << 24);
		boot->FSInfo = block[48] + (block[49] << 8);
		boot->Backup = block[50] + (block[51] << 8);

		if (lseek(dosfs, boot->FSInfo * boot->BytesPerSec, SEEK_SET)
		    != boot->FSInfo * boot->BytesPerSec
		    || read(dosfs, fsinfo, boot->BytesPerSec)
		    != boot->BytesPerSec) {
			perr("could not read fsinfo block");
			return FSFATAL;
		}
		if (memcmp(fsinfo, "RRaA", 4)
		    || memcmp(fsinfo + 0x1e4, "rrAa", 4)
		    || fsinfo[0x1fc]
		    || fsinfo[0x1fd]
		    || fsinfo[0x1fe] != 0x55
		    || fsinfo[0x1ff] != 0xaa) {
			pwarn("Invalid signature in fsinfo block\n");
			if (ask(0, "fix")) {
				memcpy(fsinfo, "RRaA", 4);
				memcpy(fsinfo + 0x1e4, "rrAa", 4);
				fsinfo[0x1fc] = fsinfo[0x1fd] = 0;
				fsinfo[0x1fe] = 0x55;
				fsinfo[0x1ff] = 0xaa;
				fsinfo[0x3fc] = fsinfo[0x3fd] = 0;
				fsinfo[0x3fe] = 0x55;
				fsinfo[0x3ff] = 0xaa;
				if (lseek(dosfs, boot->FSInfo * boot->BytesPerSec, SEEK_SET)
				    != boot->FSInfo * boot->BytesPerSec
				    || write(dosfs, fsinfo, boot->BytesPerSec)
				    != boot->BytesPerSec) {
					perr("Unable to write FSInfo");
					return FSFATAL;
				}
				ret = FSBOOTMOD;
			} else {
				boot->FSInfo = 0;
				ret = FSERROR;
			}
		}
		if (boot->FSInfo) {
			boot->FSFree = fsinfo[0x1e8] + (fsinfo[0x1e9] << 8)
				       + (fsinfo[0x1ea] << 16)
				       + (fsinfo[0x1eb] << 24);
			boot->FSNext = fsinfo[0x1ec] + (fsinfo[0x1ed] << 8)
				       + (fsinfo[0x1ee] << 16)
				       + (fsinfo[0x1ef] << 24);
		}
	}

	/* sanity check the FATs and FATsecs */
	if (os_mul_overflow(boot->FATs, boot->FATsecs, &result)) {
		pfatal("Invalid boot->FATs or boot->FATsecs\n");
		return FSFATAL;
	}

	boot->ClusterOffset = (boot->RootDirEnts * 32 + boot->BytesPerSec - 1)
	    / boot->BytesPerSec
	    + boot->ResSectors
	    + result;

	if (boot->Sectors) {
		boot->HugeSectors = 0;
		boot->NumSectors = boot->Sectors;
    } else if (boot->HugeSectors) {
		boot->NumSectors = boot->HugeSectors;
    } else {
        boot->NumSectors = 0;
        u_int64_t SuperHugeSectors = (block[66] != 0x29)? 0 : (uint64_t) block[82] + ((uint64_t) block[83] << 8) + ((uint64_t) block[84] << 16) + ((uint64_t) block[85] << 24) + ((uint64_t) block[86]<<32) + ((uint64_t) block[87] << 40) + ((uint64_t) block[88] << 48) + ((uint64_t) block[89] << 54);
        if (SuperHugeSectors != 0) {
            pwarn("Encountered special FAT where total sector location is 64bit. Not Supported \n");
        } else {
            char cOEMName[9] = {0};
            strlcpy(&cOEMName[0], (char *) &block[3] , 8);
            cOEMName[8] = '\0';
            pwarn("OEMName: %s\n", cOEMName);
        }
    }

    /* Ensure NumSectors isn't zero and >= ClusterOffset */
    if ((boot->NumSectors == 0) || (boot->NumSectors < boot->ClusterOffset)) {
        pfatal("Filesystem has invalid NumSectors %u\n", boot->NumSectors);
		return FSFATAL;
	}

	/*
	 * Note: NumClusters isn't actually the *number* (or count) of clusters.  It is really
	 * the maximum cluster number plus one (which is the number of clusters plus two;
	 * it is also the number of valid FAT entries).  It is meant to be used
	 * for looping over cluster numbers, or range checking cluster numbers.
	 */
	boot->NumClusters = CLUST_FIRST + (boot->NumSectors - boot->ClusterOffset) / boot->SecPerClust;

    /* Since NumClusters is off by two, use constants that are off by two also. */
	if (boot->flags&FAT32)
		boot->ClustMask = CLUST32_MASK;
	else if (boot->NumClusters < (4085+2))
		boot->ClustMask = CLUST12_MASK;
	else if (boot->NumClusters < (65526+2))		/* Windows allows 65525 clusters, so we should, too */
		boot->ClustMask = CLUST16_MASK;
	else {
		pfatal("Filesystem too big (%u clusters) for non-FAT32 partition\n",
		       boot->NumClusters-2);
		return FSFATAL;
	}

	result = 0;

	/* sanity check FATsecs and BytesPerSec */
	if (os_mul_overflow(boot->FATsecs, boot->BytesPerSec, &result)) {
		pfatal("Invalid boot->FATsecs or boot->BytesPerSec\n");
		return FSFATAL;
	}

	switch (boot->ClustMask) {
	case CLUST32_MASK:
		boot->NumFatEntries = result / 4;
		break;
	case CLUST16_MASK:
		boot->NumFatEntries = result / 2;
		break;
	default:
        {
            u_int32_t mul2 = 0;
            if (os_mul_overflow(result, 2, &mul2)) {
                pfatal("Invalid boot->FATsecs or boot->BytesPerSec for FAT12\n");
                return FSFATAL;
            }
            boot->NumFatEntries = mul2 / 3;
            break;
        }
	}

	/*
	 * Verify that the FAT is large enough to hold the number of clusters
	 * that we think the volume has.  Some digital cameras, and our own
	 * newfs_msdos, can create volumes whose total sector count is too large.
	 */
	if (boot->NumFatEntries < boot->NumClusters) {
		pwarn("FAT size too small, %u entries won't fit into %u sectors\n",
		       boot->NumClusters, boot->FATsecs);
		boot->NumClusters = boot->NumFatEntries;
		if (ask(0, "Fix total sectors")) {
			/* Need to recompute sectors based on clusters */
			boot->NumSectors = ((boot->NumClusters - CLUST_FIRST) * boot->SecPerClust) + boot->ClusterOffset;
			if (boot->Sectors) {
				boot->Sectors = boot->NumSectors;
				block[19] = boot->NumSectors & 0xFF;
				block[20] = (boot->NumSectors >> 8) & 0xFF;
			} else {
				boot->HugeSectors = boot->NumSectors;
				block[32] = boot->NumSectors & 0xFF;
				block[33] = (boot->NumSectors >> 8) & 0xFF;
				block[34] = (boot->NumSectors >> 16) & 0xFF;
				block[35] = (boot->NumSectors >> 24) & 0xFF;
            }
			if (lseek(dosfs, 0, SEEK_SET) != 0 ||
				write(dosfs, block, boot->BytesPerSec) != boot->BytesPerSec)
			{
				perr("could not write boot sector");
				return FSFATAL;
			}
			ret |= FSBOOTMOD;	/* This flag is currently ignored by checkfilesys() */
		} else {
			pwarn("Continuing, assuming %u clusters\n", boot->NumFatEntries-2);
			/*
			 * We don't return an error here, so Mac OS X will automatically
			 * mount the volume without attempting to repair the disk just
			 * because of this problem (though it will end up fixing this
			 * problem if there was some other problem that had to be repaired
			 * before mounting).
			 */
		}
	}

	boot->NumFree = 0;

	return ret;
}

int
writefsinfo(dosfs, boot)
	int dosfs;
	struct bootblock *boot;
{
	u_char fsinfo[MAX_SECTOR_SIZE];

	if (lseek(dosfs, boot->FSInfo * boot->BytesPerSec, SEEK_SET)
	    != boot->FSInfo * boot->BytesPerSec
	    || read(dosfs, fsinfo, boot->BytesPerSec) != boot->BytesPerSec) {
		perr("could not read fsinfo block");
		return FSFATAL;
	}
	fsinfo[0x1e8] = (u_char)boot->FSFree;
	fsinfo[0x1e9] = (u_char)(boot->FSFree >> 8);
	fsinfo[0x1ea] = (u_char)(boot->FSFree >> 16);
	fsinfo[0x1eb] = (u_char)(boot->FSFree >> 24);
	fsinfo[0x1ec] = (u_char)boot->FSNext;
	fsinfo[0x1ed] = (u_char)(boot->FSNext >> 8);
	fsinfo[0x1ee] = (u_char)(boot->FSNext >> 16);
	fsinfo[0x1ef] = (u_char)(boot->FSNext >> 24);
	if (lseek(dosfs, boot->FSInfo * boot->BytesPerSec, SEEK_SET)
	    != boot->FSInfo * boot->BytesPerSec
	    || write(dosfs, fsinfo, boot->BytesPerSec)
	    != boot->BytesPerSec) {
		perr("Unable to write FSInfo");
		return FSFATAL;
	}
	/*
	 * Technically, we should return FSBOOTMOD here.
	 *
	 * However, since Win95 OSR2 (the first M$ OS that has
	 * support for FAT32) doesn't maintain the FSINFO block
	 * correctly, it has to be fixed pretty often.
	 *
	 * Therefor, we handle the FSINFO block only informally,
	 * fixing it if neccessary, but otherwise ignoring the
	 * fact that it was incorrect.
	 */
	return 0;
}
