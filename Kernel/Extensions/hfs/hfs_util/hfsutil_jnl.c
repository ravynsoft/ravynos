/*
 * Copyright (c) 1999-2015 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.2 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 Copyright (c) 2002 Apple Computer, Inc.
 All Rights Reserved.

 This file contains the routine to make an HFS+ volume journaled
 and a corresponding routine to turn it off.
 
 */

#include <sys/types.h>
#include <sys/attr.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/sysctl.h>
#include <sys/resource.h>
#include <sys/vmmeter.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

#include <sys/disk.h>
#include <sys/loadable_fs.h>
#include <hfs/hfs_format.h>
#include <hfs/hfs_mount.h>    /* for hfs sysctl values */

#include "../core/hfs_fsctl.h"

#include <System/sys/content_protection.h>
#include <TargetConditionals.h>

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <architecture/byte_order.h>

// just in case these aren't in <hfs/hfs_mount.h> yet
#ifndef HFS_ENABLE_JOURNALING
#define HFS_ENABLE_JOURNALING   0x082969
#endif
#ifndef HFS_DISABLE_JOURNALING
#define HFS_DISABLE_JOURNALING 0x031272
#endif
#ifndef HFS_GET_JOURNAL_INFO
#define HFS_GET_JOURNAL_INFO    0x6a6e6c69
#endif

/* getattrlist buffers start with an extra length field */
struct ExtentsAttrBuf {
	unsigned long	infoLength;
	HFSPlusExtentRecord	extents;
};
typedef struct ExtentsAttrBuf ExtentsAttrBuf;

#ifndef HFSIOC_GET_JOURNAL_INFO
# include <sys/ioctl.h>
struct hfs_journal_info {
	off_t	jstart;
	off_t	jsize;
};
# define HFSIOC_GET_JOURNAL_INFO	_IOR('h', 17, struct hfs_journal_info)
#endif


#define kIsInvisible 0x4000

/*
 * Generic Finder file/dir data
 */
struct FinderInfo {
	u_int32_t 	opaque_1[2];
	u_int16_t 	fdFlags;	/* Finder flags */
	int16_t 	opaque_2[11];
};
typedef struct FinderInfo FinderInfo;

/* getattrlist buffers start with an extra length field */
struct FinderAttrBuf {
	unsigned long	infoLength;
	FinderInfo	finderInfo;
};
typedef struct FinderAttrBuf FinderAttrBuf;


int hide_file(const char * file)
{
    struct attrlist alist = {0};
    FinderAttrBuf finderInfoBuf = {0};
    int result;
    
    alist.bitmapcount = ATTR_BIT_MAP_COUNT;
    alist.commonattr = ATTR_CMN_FNDRINFO;

    result = getattrlist(file, &alist, &finderInfoBuf, sizeof(finderInfoBuf), 0);
    if (result) {
	return (errno);
    }
	
    if (finderInfoBuf.finderInfo.fdFlags & kIsInvisible) {
	printf("hide: %s is alreadly invisible\n", file);
	return (0);
    }

    finderInfoBuf.finderInfo.fdFlags |= kIsInvisible;

    result = setattrlist(file, &alist, &finderInfoBuf.finderInfo, sizeof(FinderInfo), 0);
    
    return (result == -1 ? errno : result);
}

off_t
get_start_block(const char *file, uint32_t fs_block_size)
{
    off_t cur_pos, phys_start, len;
    int fd, err;
    struct log2phys l2p;
    struct stat st;

    fd = open(file, O_RDONLY);
    if (fd < 0) {
	return -1;
    }

    if (fstat(fd, &st) < 0) {
	fprintf(stderr, "can't stat %s (%s)\n", file, strerror(errno));
	close(fd);
	return -1;
    }

    fs_block_size = st.st_blksize; // XXXdbg quick hack for now

    phys_start = len = 0;
    for(cur_pos=0; cur_pos < st.st_size; cur_pos += fs_block_size) {
	memset(&l2p, 0, sizeof(l2p));
	lseek(fd, cur_pos, SEEK_SET);
	err = fcntl(fd, F_LOG2PHYS, &l2p);

	if (phys_start == 0) {
	    phys_start = l2p.l2p_devoffset;
	    len = fs_block_size;
	} else if (l2p.l2p_devoffset != (phys_start + len)) {
	    // printf("    %lld : %lld - %lld\n", cur_pos, phys_start / fs_block_size, len / fs_block_size);
	    fprintf(stderr, "%s : is not contiguous!\n", file);
	    close(fd);
	    return -1;
	    // phys_start = l2p.l2p_devoffset;
	    // len = fs_block_size;
	} else {
	    len += fs_block_size;
	}
    }

    close(fd);

    //printf("%s start offset %lld; byte len %lld (blksize %d)\n",
    // file, phys_start, len, fs_block_size);

    if ((phys_start / (unsigned int)fs_block_size) & 0xffffffff00000000LL) {
	fprintf(stderr, "%s : starting block is > 32bits!\n", file);
	return -1;
    }
	
    return phys_start;
}


//
// Get the embedded offset (if any) for an hfs+ volume.
// This is pretty skanky that we have to do this but
// that's life...
//
#include <sys/disk.h>
#include <hfs/hfs_format.h>

#include <machine/endian.h>

#define HFS_PRI_SECTOR(blksize)          (1024 / (blksize))
#define HFS_PRI_OFFSET(blksize)          ((blksize) > 1024 ? 1024 : 0)

#include <libkern/OSByteOrder.h>

#define SWAP_BE16(x) ntohs(x)
#define SWAP_BE32(x) ntohl(x)
#define SWAP_BE64(x) OSSwapConstInt64(x)


off_t
get_embedded_offset(char *devname)
{
    int fd = -1;
    off_t ret = 0;
    char *buff = NULL, rawdev[256];
    u_int64_t blkcnt;
    u_int32_t blksize;
    HFSMasterDirectoryBlock *mdbp;
    off_t embeddedOffset;
    struct statfs sfs;
    struct stat   st;
	
  restart:
    if (stat(devname, &st) != 0) {
	fprintf(stderr, "Could not access %s (%s)\n", devname, strerror(errno));
	ret = -1;
	goto out;
    }

    if (S_ISCHR(st.st_mode) == 0) {
	// hmmm, it's not the character special raw device so we
	// should try to figure out the real device.
	if (statfs(devname, &sfs) != 0) {
	    fprintf(stderr, "Can't find out any info about the fs for path %s (%s)\n",
		devname, strerror(errno));
	    ret = -1;
	    goto out;
	}

	// This assumes it begins with "/dev/".  The old code assumed
	// it began with five characters.  Should probably use strrchr or equivalent.
	snprintf(rawdev, sizeof(rawdev), "/dev/r%s", &sfs.f_mntfromname[5]);
	devname = &rawdev[0];
	goto restart;
    }

    fd = open(devname, O_RDONLY);
    if (fd < 0) {
	fprintf(stderr, "can't open: %s (%s)\n", devname, strerror(errno));
	ret = -1;
	goto out;
    }

    /* Get the real physical block size. */
    if (ioctl(fd, DKIOCGETBLOCKSIZE, (caddr_t)&blksize) != 0) {
	fprintf(stderr, "can't get the device block size (%s). assuming 512\n", strerror(errno));
	blksize = 512;
	ret = -1;
	goto out;
    }

    /* Get the number of physical blocks. */
    if (ioctl(fd, DKIOCGETBLOCKCOUNT, (caddr_t)&blkcnt)) {
	struct stat st;
	fprintf(stderr, "failed to get block count. trying stat().\n");
	if (fstat(fd, &st) != 0) {
	    ret = -1;
	    goto out;
	}

	blkcnt = st.st_size / blksize;
    }

    /*
     * At this point:
     *   blksize has our prefered physical block size
     *   blkcnt has the total number of physical blocks
     */

    buff = (char *)malloc(blksize);
	
    if (pread(fd, buff, blksize, HFS_PRI_SECTOR(blksize)*blksize) != blksize) {
		fprintf(stderr, "failed to read volume header @ offset %d (%s)\n",
				HFS_PRI_SECTOR(blksize), strerror(errno));
		ret = -1;
		goto out;
    }

	mdbp = (HFSMasterDirectoryBlock *)(buff + HFS_PRI_OFFSET(blksize));
	if (   (SWAP_BE16(mdbp->drSigWord) != kHFSSigWord) 
			&& (SWAP_BE16(mdbp->drSigWord) != kHFSPlusSigWord)
			&& (SWAP_BE16(mdbp->drSigWord) != kHFSXSigWord)) {
		printf ("get_embedded_offset: invalid volume signature \n");
		ret = -1;
		goto out;
	}

    if ((SWAP_BE16(mdbp->drSigWord) == kHFSSigWord) && (SWAP_BE16(mdbp->drEmbedSigWord) != kHFSPlusSigWord)) {
	ret = -1;
	goto out;
    } else if (SWAP_BE16(mdbp->drEmbedSigWord) == kHFSPlusSigWord) {
	/* Get the embedded Volume Header */
	embeddedOffset = SWAP_BE16(mdbp->drAlBlSt) * 512;
	embeddedOffset += (u_int64_t)SWAP_BE16(mdbp->drEmbedExtent.startBlock) *
                          (u_int64_t)SWAP_BE32(mdbp->drAlBlkSiz);

	/*
	 * If the embedded volume doesn't start on a block
	 * boundary, then switch the device to a 512-byte
	 * block size so everything will line up on a block
	 * boundary.
	 */
	if ((embeddedOffset % blksize) != 0) {
	    fprintf(stderr, "HFS Mount: embedded volume offset not"
		" a multiple of physical block size (%d);"
		" switching to 512\n", blksize);
		
	    blkcnt  *= (blksize / 512);
	    blksize  = 512;
	}

    } else { /* pure HFS+ */ 
	embeddedOffset = 0;
    }

    ret = embeddedOffset;

  out:
    if (buff) {
	free(buff);
    }
    if (fd >= 0) 
	close(fd);

    return ret;
}



static const char *journal_fname = ".journal";
static const char *jib_fname = ".journal_info_block";

int 
DoMakeJournaled(char *volname, int jsize) {
	int              fd, i, block_size, journal_size = 8*1024*1024;
	char            *buf = NULL;
	int              ret;
	fstore_t         fst;
	int32_t          jstart_block, jinfo_block;
	int              sysctl_info[8];
	JournalInfoBlock jib;
	struct statfs    sfs;
	static char      tmpname[MAXPATHLEN];
	off_t            start_block, embedded_offset;

	if (statfs(volname, &sfs) != 0) {
		fprintf(stderr, "Can't stat volume %s (%s).\n", volname, strerror(errno));
		return 10;
	}

	// Make sure that we're HFS+.  First we check the fstypename.
	// If that's ok then we try to create a symlink (which won't
	// work on plain hfs volumes but will work on hfs+ volumes).
	//
	if (strcmp(sfs.f_fstypename, "devfs") == 0) {
		fprintf (stderr, "%s is a device node.  Journal enable only works on a mounted HFS+ volume.\n", volname);
		return 10;
	}
	snprintf(tmpname, sizeof(tmpname), "%s/is_vol_hfs_plus", volname);
	if (strcmp(sfs.f_fstypename, "hfs") != 0 ||
			((ret = symlink(tmpname, tmpname)) != 0 && errno == ENOTSUP)) {
		fprintf(stderr, "%s is not an HFS+ volume.  Journaling only works on HFS+ volumes.\n",
				volname);
		return 10;
	}
	unlink(tmpname);

	if (sfs.f_flags & MNT_JOURNALED) {
		fprintf(stderr, "Volume %s is already journaled.\n", volname);
		return 1;
	}

	if (jsize != 0) {
		journal_size = jsize;
	} else {
		int scale;

		//
		// we want at least 8 megs of journal for each 100 gigs of
		// disk space.  We cap the size at 512 megs though.
		//
		scale = ((long long)sfs.f_bsize * (long long)((unsigned int)sfs.f_blocks)) / (100*1024*1024*1024ULL);
		journal_size *= (scale + 1);
		if (journal_size > 512 * 1024 * 1024) {
			journal_size = 512 * 1024 * 1024;
		}
	}

	if (chdir(volname) != 0) {
		fprintf(stderr, "Can't locate volume %s to make it journaled (%s).\n",
				volname, strerror(errno));
		return 10;
	}


	embedded_offset = get_embedded_offset(volname);
	if (embedded_offset < 0) {
		fprintf(stderr, "Can't calculate the embedded offset (if any) for %s.\n", volname);
		fprintf(stderr, "Journal creation failure.\n");
		return 15;
	}
	// printf("Embedded offset == 0x%llx\n", embedded_offset);

	/* 
	 * Must use open_dprotected_np to create a class D file.  This will
	 * be the same as standard open(2) on systems that do not support content protection
	 */
	fd = open_dprotected_np (journal_fname, O_CREAT|O_TRUNC|O_RDWR, PROTECTION_CLASS_D, 0, 000);
	if (fd < 0) {
		fprintf(stderr, "Can't create journal file on volume %s (%s)\n",
				volname, strerror(errno));
		return 5;
	}

	if (fcntl(fd, F_NOCACHE, 1)) {
		fprintf(stderr, "Can't create journal file (NC)  on volume %s (%s)\n",
				volname, strerror(errno));
		return 5;	
	}


	// make sure that it has no r/w/x privs (only could happen if
	// the file already existed since open() doesn't reset the mode
	// bits).
	//
	fchmod(fd, 0);

	block_size = sfs.f_bsize;
	if ((journal_size % block_size) != 0) {
		fprintf(stderr, "Journal size %dk is not a multiple of volume %s block size (%d).\n",
				journal_size/1024, volname, block_size);
		close(fd);
		unlink(journal_fname);
		return 5;
	}

retry:
	memset(&fst, 0, sizeof(fst));
	fst.fst_flags   = F_ALLOCATECONTIG|F_ALLOCATEALL;
	fst.fst_length  = journal_size;
	fst.fst_posmode = F_PEOFPOSMODE;

	ret = fcntl(fd, F_PREALLOCATE, &fst);
	if (ret < 0) {
		if (journal_size >= 2*1024*1024) {
			fprintf(stderr, "Not enough contiguous space for a %d k journal.  Retrying.\n",
					journal_size/1024);
			journal_size /= 2;
			ftruncate(fd, 0);     // make sure the file is zero bytes long.
			goto retry;
		} else {
			fprintf(stderr, "Disk too fragmented to enable journaling.\n");
			fprintf(stderr, "Please run a defragmenter on %s.\n", volname);
			close(fd);
			unlink(journal_fname);
			return 10;
		}
	}

	printf("Allocated %lldK for journal file.\n", fst.fst_bytesalloc/1024LL);
	buf = (char *)calloc(block_size, 1);
	if (buf) {
		for(i=0; i < journal_size/block_size; i++) {
			ret = (int)write(fd, buf, block_size);
			if (ret != block_size) {
				break;
			}
		}

		if (i*block_size != journal_size) {
			fprintf(stderr, "Failed to write %dk to journal on volume %s (%s)\n",
					journal_size/1024, volname, strerror(errno));
		}
	} else {
		printf("Could not allocate memory to write to the journal on volume %s (%s)\n",
				volname, strerror(errno));
	}

	fsync(fd);
	close(fd);
	hide_file(journal_fname);

	start_block = get_start_block(journal_fname, block_size);
	if (start_block == (off_t)-1) {
		fprintf(stderr, "Failed to get start block for %s (%s)\n",
				journal_fname, strerror(errno));
		unlink(journal_fname);
		if (buf) 
			free(buf);
		return 20;
	}
	jstart_block = (int32_t)((start_block / block_size) - (embedded_offset / block_size));

	memset(&jib, 'Z', sizeof(jib));
	jib.flags  = kJIJournalInFSMask;
	jib.offset = (off_t)((unsigned int)jstart_block) * (off_t)((unsigned int)block_size);
	jib.size   = (off_t)((unsigned int)journal_size);

	/* 
	 * Use open_dprotected_np to create JIB as a class D file.  This will
	 * behave the same as a standard open(2) on systems that do not support content protection
	 */
	fd = open_dprotected_np(jib_fname, O_CREAT|O_TRUNC|O_RDWR, PROTECTION_CLASS_D, 0, 000);
	if (fd < 0) {
		fprintf(stderr, "Could not create journal info block file on volume %s (%s)\n",
				volname, strerror(errno));
		unlink(journal_fname);
		if (buf)
			free(buf);
		return 5;
	}

	if (fcntl(fd, F_NOCACHE, 1)) {
		fprintf(stderr, "Could not create journal info block (NC) file on volume %s (%s)\n",
				volname, strerror(errno));
		if (buf)
			free(buf);
		return 5;	
	} 

	// swap the data before we copy it
	jib.flags  = OSSwapBigToHostInt32(jib.flags);
	jib.offset = OSSwapBigToHostInt64(jib.offset);
	jib.size   = OSSwapBigToHostInt64(jib.size);

	memcpy(buf, &jib, sizeof(jib));

	// now put it back the way it was
	jib.size   = OSSwapBigToHostInt64(jib.size);
	jib.offset = OSSwapBigToHostInt64(jib.offset);
	jib.flags  = OSSwapBigToHostInt32(jib.flags);

	if (write(fd, buf, block_size) != block_size) {
		fprintf(stderr, "Failed to write journal info block on volume %s (%s)!\n",
				volname, strerror(errno));
		unlink(journal_fname);
		if (buf)
			free(buf);
		return 10;
	}

	fsync(fd);
	close(fd);
	hide_file(jib_fname);

	start_block = get_start_block(jib_fname, block_size);
	if (start_block == (off_t)-1) {
		fprintf(stderr, "Failed to get start block for %s (%s)\n",
				jib_fname, strerror(errno));
		unlink(journal_fname);
		unlink(jib_fname);
		if (buf)
			free(buf);
		return 20;
	}
	jinfo_block = (int32_t)((start_block / block_size) - (embedded_offset / block_size));


	//
	// Now make the volume journaled!
	//
	memset(sysctl_info, 0, sizeof(sysctl_info));
	sysctl_info[0] = CTL_VFS;
	sysctl_info[1] = sfs.f_fsid.val[1];
	sysctl_info[2] = HFS_ENABLE_JOURNALING;
	sysctl_info[3] = jinfo_block;
	sysctl_info[4] = jstart_block;
	sysctl_info[5] = journal_size;

	//printf("fs type: 0x%x\n", sysctl_info[1]);
	//printf("jinfo block : 0x%x\n", jinfo_block);
	//printf("jstart block: 0x%x\n", jstart_block);
	//printf("journal size: 0x%x\n", journal_size);

	ret = sysctl((void *)sysctl_info, 6, NULL, NULL, NULL, 0);
	if (ret != 0) {
		fprintf(stderr, "Failed to make volume %s journaled (%s)\n",
				volname, strerror(errno));
		unlink(journal_fname);
		unlink(jib_fname);
		if (buf)
			free(buf);
		return 20;
	}

	if (buf)
		free(buf);
	return 0;
}


int
DoUnJournal(char *volname) {
	int           result;
	int           sysctl_info[8];
	struct statfs sfs;
	char          jbuf[MAXPATHLEN];

	if (statfs(volname, &sfs) != 0) {
		fprintf(stderr, "Can't stat volume %s (%s).\n", volname, strerror(errno));
		return 10;
	}

	if (strcmp(sfs.f_fstypename, "hfs") != 0) {
		fprintf(stderr, "Volume %s (%s) is not a HFS volume.\n", volname, sfs.f_mntfromname);
		return 1;
	}

	if ((sfs.f_flags & MNT_JOURNALED) == 0) {
		fprintf(stderr, "Volume %s (%s) is not journaled.\n", volname, sfs.f_mntfromname);
		return 1;
	}

	if (chdir(volname) != 0) {
		fprintf(stderr, "Can't locate volume %s to turn off journaling (%s).\n",
				volname, strerror(errno));
		return 10;
	}

	memset(sysctl_info, 0, sizeof(sysctl_info));
	sysctl_info[0] = CTL_VFS;
	sysctl_info[1] = sfs.f_fsid.val[1];
	sysctl_info[2] = HFS_DISABLE_JOURNALING;

	result = sysctl((void *)sysctl_info, 3, NULL, NULL, NULL, 0);
	if (result != 0) {
		fprintf(stderr, "Failed to make volume %s UN-journaled (%s)\n",
				volname, strerror(errno));
		return 20;
	}

	snprintf(jbuf, sizeof(jbuf), "%s/%s", volname, journal_fname);
	if (unlink(jbuf) != 0) {
		fprintf(stderr, "Failed to remove the journal %s (%s)\n",
				jbuf, strerror(errno));
	}

	snprintf(jbuf, sizeof(jbuf), "%s/%s", volname, jib_fname);
	if (unlink(jbuf) != 0) {
		fprintf(stderr, "Failed to remove the journal info block %s (%s)\n",
				jbuf, strerror(errno));
	}

	printf("Journaling disabled on %s mounted at %s.\n", sfs.f_mntfromname, volname);

	return 0;
}




int
get_journal_info(char *devname, struct JournalInfoBlock *jib)
{
	int fd = -1, ret = 0;
	char *buff = NULL, *buff2 = NULL;
	u_int64_t disksize;
	u_int64_t blkcnt;
	u_int32_t blksize;
	daddr_t   mdb_offset;
	HFSMasterDirectoryBlock *mdbp;
	HFSPlusVolumeHeader *vhp;
	off_t embeddedOffset, pos;
	struct JournalInfoBlock *myjib;

	fd = open(devname, O_RDONLY);
	if (fd < 0) {
		printf("can't open: %s (%s)\n", devname, strerror(errno));
		ret = -5;
		goto out;
	}

	/* Get the real physical block size. */
	if (ioctl(fd, DKIOCGETBLOCKSIZE, (caddr_t)&blksize) != 0) {
		printf("can't get the device block size (%s). assuming 512\n", strerror(errno));
		blksize = 512;
		ret = -1;
		goto out;
	}

	/* Get the number of physical blocks. */
	if (ioctl(fd, DKIOCGETBLOCKCOUNT, (caddr_t)&blkcnt)) {
		struct stat st;
		printf("failed to get block count. trying stat().\n");
		if (fstat(fd, &st) != 0) {
			ret = -1;
			goto out;
		}

		blkcnt = st.st_size / blksize;
	}

	/* Compute an accurate disk size */
	disksize = blkcnt * (u_int64_t)blksize;

	/*
	 * There are only 31 bits worth of block count in
	 * the buffer cache.  So for large volumes a 4K
	 * physical block size is needed.
	 */
	if (blksize == 512 && blkcnt > (u_int64_t)0x000000007fffffff) {
		blksize = 4096;
	}

	/*
	 * At this point:
	 *   blksize has our prefered physical block size
	 *   blkcnt has the total number of physical blocks
	 */

	buff  = (char *)malloc(blksize);
	buff2 = (char *)malloc(blksize);
	
	if (pread(fd, buff, blksize, HFS_PRI_SECTOR(blksize)*blksize) != blksize) {
		printf("failed to read volume header @ offset %d (%s)\n",
		       HFS_PRI_SECTOR(blksize), strerror(errno));
		ret = -1;
		goto out;
	}

	mdbp = (HFSMasterDirectoryBlock *)(buff + HFS_PRI_OFFSET(blksize));

	if (   (SWAP_BE16(mdbp->drSigWord) != kHFSSigWord)
			&& (SWAP_BE16(mdbp->drSigWord) != kHFSPlusSigWord)
			&& (SWAP_BE16(mdbp->drSigWord) != kHFSXSigWord)) {
		ret = -1;
		printf("get_journal_info: invalid volume signature\n");
		goto out;
	}

	mdbp->drSigWord = SWAP_BE16(mdbp->drSigWord);
	mdbp->drEmbedSigWord = SWAP_BE16(mdbp->drEmbedSigWord);
	mdbp->drAlBlSt = SWAP_BE16(mdbp->drAlBlSt);
	mdbp->drEmbedExtent.startBlock = SWAP_BE16(mdbp->drEmbedExtent.startBlock);
	mdbp->drAlBlkSiz = SWAP_BE32(mdbp->drAlBlkSiz);
	mdbp->drEmbedExtent.blockCount = SWAP_BE16(mdbp->drEmbedExtent.blockCount);


	if ((mdbp->drSigWord == kHFSSigWord) && (mdbp->drEmbedSigWord != kHFSPlusSigWord)) {
		// normal hfs can not ever be journaled
		goto out;
	} 
	
	/* Get the embedded Volume Header */
	if (mdbp->drEmbedSigWord == kHFSPlusSigWord) {
		embeddedOffset = mdbp->drAlBlSt * 512;
		embeddedOffset += (u_int64_t)mdbp->drEmbedExtent.startBlock *
			(u_int64_t)mdbp->drAlBlkSiz;

		/*
		 * If the embedded volume doesn't start on a block
		 * boundary, then switch the device to a 512-byte
		 * block size so everything will line up on a block
		 * boundary.
		 */
		if ((embeddedOffset % blksize) != 0) {
			printf("HFS Mount: embedded volume offset not"
			       " a multiple of physical block size (%d);"
			       " switching to 512\n", blksize);
		
			blkcnt  *= (blksize / 512);
			blksize  = 512;
		}

		disksize = (u_int64_t)mdbp->drEmbedExtent.blockCount *
			(u_int64_t)mdbp->drAlBlkSiz;

		mdb_offset = (daddr_t)((embeddedOffset / blksize) + HFS_PRI_SECTOR(blksize));
		if (pread(fd, buff, blksize, mdb_offset * blksize) != blksize) {
			printf("failed to read the embedded vhp @ offset %d\n", mdb_offset * blksize);
			ret = -1;
			goto out;
		}

		vhp = (HFSPlusVolumeHeader*) (buff + HFS_PRI_OFFSET(blksize));

		mdbp = (HFSMasterDirectoryBlock *)vhp;
		if (   (SWAP_BE16(mdbp->drSigWord) != kHFSSigWord)
				&& (SWAP_BE16(mdbp->drSigWord) != kHFSPlusSigWord)
				&& (SWAP_BE16(mdbp->drSigWord) != kHFSXSigWord)) {
			ret = -1;
			
			printf("get_journal_info: invalid embedded volume signature \n");
			goto out;
		}

	} else /* pure HFS+ */ {
		embeddedOffset = 0;
		vhp = (HFSPlusVolumeHeader*) mdbp;
	}

	if ((SWAP_BE32(vhp->attributes) & kHFSVolumeJournaledMask) == 0) {
		ret = 0;
		goto out;
	}

	//
	// Now read the journal info block... (when calculating the
	// position, make sure to cast to unsigned first, then to
	// off_t so that things don't get sign-extended improperly
	// or truncated).
	//
	pos = (off_t)((off_t)embeddedOffset +
		      (off_t)((unsigned int)SWAP_BE32(vhp->journalInfoBlock))*(off_t)((unsigned int)SWAP_BE32(vhp->blockSize)));

	if (pread(fd, buff2, blksize, pos) != blksize) {
		printf("failed to read the journal info block (%s).\n", strerror(errno));
		ret = -1;
		goto out;
	}

	myjib = (struct JournalInfoBlock *)buff2;
	myjib->flags  = SWAP_BE32(myjib->flags);
	myjib->offset = SWAP_BE64(myjib->offset);
	myjib->size   = SWAP_BE64(myjib->size);
	
	memcpy(jib, myjib, sizeof(*myjib));
	
	ret = 1;

out:
	if (buff)
		free(buff);
	if (buff2)
		free(buff2);
	if (fd >= 0) 
		close(fd);

	return ret;
}


int
DoGetJournalInfo(char *volname)
{
    struct statfs sfs;
    struct hfs_journal_info jinfo;
	
    if (strncmp(volname, "/dev/", 5) == 0 || strncmp(volname, "disk", 4) == 0 || strncmp(volname, "rdisk", 5) == 0) {
	    struct JournalInfoBlock jib;
	    int ret;
	    char fulldevname[256];

	    if (strncmp(volname, "disk", 4) == 0 || strncmp(volname, "rdisk", 5) == 0) {
		    snprintf(fulldevname, sizeof(fulldevname), "/dev/%s", volname);
		    volname = &fulldevname[0];
	    }
	    
	    // try the name as a device name...
	    ret = get_journal_info(volname, &jib);
	    if (ret == 0) {
		    printf("Volume %s is not journaled.\n", volname);
		    return 0;
	    } else if (ret < 0) {
		    printf("Volume %s does not appear to be an HFS+ volume.\n", volname);
		    return 10;
	    } else {
		if (jib.flags & kJIJournalInFSMask) {
			printf("%s : journal size %lld k at offset 0x%llx\n", volname, jib.size/1024, jib.offset);
		} else {
			printf("%s: external journal stored on partition with uuid %s on machine w/serial number %s\n",
			       volname, jib.ext_jnl_uuid, jib.machine_serial_num);
		}

		return 0;
	    }

    }

    if (statfs(volname, &sfs) != 0) {
	    fprintf(stderr, "Unable to get fs info for %s\n", volname);
	    return 10;
    }

    if ((sfs.f_flags & MNT_JOURNALED) == 0) {
	fprintf(stderr, "Volume %s is not journaled.\n", volname);
	return 1;
    }

    if (fsctl(volname, HFSIOC_GET_JOURNAL_INFO,  &jinfo, 0) != 0) {
	fprintf(stderr, "Failed to get journal info for volume %s (%s)\n",
		volname, strerror(errno));
	return 20;
    }

    if (jinfo.jstart == 0) {
	    char rawdev[256];

	    snprintf(rawdev, sizeof(rawdev), "/dev/r%s", &sfs.f_mntfromname[5]);

	    // it's an external journal so get the info the
	    // other way.
	    return DoGetJournalInfo(&rawdev[0]);
    }

    if (jinfo.jsize == 0) {
	printf("%s : not journaled.\n", volname);
    } else {
	printf("%s : journal size %lld k at offset 0x%llx\n", volname, jinfo.jsize/1024, jinfo.jstart);
    }

    return 0;
}


int
RawDisableJournaling(char *devname)
{
    int fd = -1, ret = 0;
    char *buff = NULL, rawdev[256], unrawdev[256];
    u_int64_t disksize;
    u_int64_t blkcnt;
    u_int32_t blksize;
    daddr_t   mdb_offset;
    HFSMasterDirectoryBlock *mdbp;
    HFSPlusVolumeHeader *vhp;
    off_t embeddedOffset, hdr_offset;
    struct stat   st;
    struct statfs *fsinfo;
	
	// assume that the name provided is a raw device name
	strlcpy(rawdev, devname, sizeof(rawdev));

restart:
	if (stat(rawdev, &st) != 0) {
		fprintf(stderr, "Could not access %s (%s)\n", devname, strerror(errno));
		ret = -1;
		goto out;
	}
	
	if (S_ISCHR(st.st_mode) == 0) {
		if (S_ISBLK(st.st_mode)) {
			// this is a block device, convert the name to 
			// raw character device and try again
			snprintf(rawdev, sizeof(rawdev), "/dev/r%s", devname + 5);
			goto restart;
		} else {
			// probably it is a mount point 
			return DoUnJournal(devname);
		}
	} else {
		// convert the character raw device name to 
		// block device name to compare with getmntinfo output
		snprintf(unrawdev, sizeof(unrawdev), "/dev/%s", rawdev + 6);
	}
	
	// make sure that the file system on the device node is not mounted
	ret = getmntinfo(&fsinfo, MNT_NOWAIT);
	if (ret == 0) {
		fprintf (stderr, "Error getting list of mounted filesystems\n"); 
		ret = -1;
		goto out;
	}
	
	while (ret--) {
		// the file system on this device node is currently mounted
		if (strcmp(unrawdev, fsinfo[ret].f_mntfromname) == 0) {
			return DoUnJournal(fsinfo[ret].f_mntonname);
		}
	}

    fd = open(rawdev, O_RDWR);
    if (fd < 0) {
	fprintf(stderr, "can't open: %s (%s)\n", devname, strerror(errno));
	ret = -1;
	goto out;
    }

    /* Get the real physical block size. */
    if (ioctl(fd, DKIOCGETBLOCKSIZE, (caddr_t)&blksize) != 0) {
	fprintf(stderr, "can't get the device block size (%s). assuming 512\n", strerror(errno));
	blksize = 512;
	ret = -1;
	goto out;
    }

    /* Get the number of physical blocks. */
    if (ioctl(fd, DKIOCGETBLOCKCOUNT, (caddr_t)&blkcnt)) {
	struct stat st;

	if (fstat(fd, &st) != 0) {
	    ret = -1;
	    goto out;
	}

	blkcnt = st.st_size / blksize;
    }

    /* Compute an accurate disk size */
    disksize = blkcnt * (u_int64_t)blksize;

    /*
     * There are only 31 bits worth of block count in
     * the buffer cache.  So for large volumes a 4K
     * physical block size is needed.
     */
    if (blksize == 512 && blkcnt > (u_int64_t)0x000000007fffffff) {
	blksize = 4096;
    }

    /*
     * At this point:
     *   blksize has our prefered physical block size
     *   blkcnt has the total number of physical blocks
     */

    buff  = (char *)malloc(blksize);
	
    hdr_offset = HFS_PRI_SECTOR(blksize)*blksize;
    if (pread(fd, buff, blksize, hdr_offset) != blksize) {
		fprintf(stderr, "RawDisableJournaling: failed to read volume header @ offset %lld (%s)\n",
				hdr_offset, strerror(errno));
		ret = -1;
		goto out;
    }

	mdbp = (HFSMasterDirectoryBlock *)(buff + HFS_PRI_OFFSET(blksize));
	if (   (SWAP_BE16(mdbp->drSigWord) != kHFSSigWord)
		&& (SWAP_BE16(mdbp->drSigWord) != kHFSPlusSigWord)
		&& (SWAP_BE16(mdbp->drSigWord) != kHFSXSigWord)) {
		ret = -1;
		printf("RawDisableJournaling: Invalid Volume Signature \n");
		goto out;
	}
	
    if ((SWAP_BE16(mdbp->drSigWord) == kHFSSigWord) && (SWAP_BE16(mdbp->drEmbedSigWord) != kHFSPlusSigWord)) {
	// normal hfs can not ever be journaled
	fprintf(stderr, "disable_journaling: volume is only regular HFS, not HFS+\n");
	goto out;
    } 
	
    /* Get the embedded Volume Header */
    if (SWAP_BE16(mdbp->drEmbedSigWord) == kHFSPlusSigWord) {
		embeddedOffset = SWAP_BE16(mdbp->drAlBlSt) * 512;
		embeddedOffset += (u_int64_t)SWAP_BE16(mdbp->drEmbedExtent.startBlock) * (u_int64_t)SWAP_BE32(mdbp->drAlBlkSiz);
		
		/*
		 * If the embedded volume doesn't start on a block
		 * boundary, then switch the device to a 512-byte
		 * block size so everything will line up on a block
		 * boundary.
		 */
		if ((embeddedOffset % blksize) != 0) {
			fprintf(stderr, "HFS Mount: embedded volume offset not"
					" a multiple of physical block size (%d);"
					" switching to 512\n", blksize);
			
			blkcnt  *= (blksize / 512);
			blksize  = 512;
		}
		
		disksize = (u_int64_t)SWAP_BE16(mdbp->drEmbedExtent.blockCount) * (u_int64_t)SWAP_BE32(mdbp->drAlBlkSiz);
		
		mdb_offset = (daddr_t)((embeddedOffset / blksize) + HFS_PRI_SECTOR(blksize));
		hdr_offset = mdb_offset * blksize;
		if (pread(fd, buff, blksize, hdr_offset) != blksize) {
			fprintf(stderr, "failed to read the embedded vhp @ offset %d\n", mdb_offset * blksize);
			ret = -1;
			goto out;
		}
		
		vhp = (HFSPlusVolumeHeader*) (buff + HFS_PRI_OFFSET(blksize));
		
		mdbp = (HFSMasterDirectoryBlock *)vhp;
		if (   (SWAP_BE16(mdbp->drSigWord) != kHFSSigWord)
			&& (SWAP_BE16(mdbp->drSigWord) != kHFSPlusSigWord)
			&& (SWAP_BE16(mdbp->drSigWord) != kHFSXSigWord)) {
			ret = -1;
			
			printf("RawDisableJournaling: invalid embedded volume signature \n");
			goto out;
		}
		
    } else /* pure HFS+ */ {
		embeddedOffset = 0;
		vhp = (HFSPlusVolumeHeader*) mdbp;
    }


    if ((SWAP_BE32(vhp->attributes) & kHFSVolumeJournaledMask) != 0) {
		unsigned int tmp = SWAP_BE32(vhp->attributes);
		
		tmp &= ~kHFSVolumeJournaledMask;
		vhp->attributes = SWAP_BE32(tmp);
		if ((tmp = (unsigned int)pwrite(fd, buff, blksize, hdr_offset)) != blksize) {
			fprintf(stderr, "Update of super-block on %s failed! (%d != %d, %s)\n",
					devname, tmp, blksize, strerror(errno));
		} else {
			fprintf(stderr, "Turned off the journaling bit for %s\n", devname);
		}
    } else {
		fprintf(stderr, "disable_journaling: %s is not journaled.\n", devname);
    }
	
	
  out:
    if (buff)
	free(buff);
    if (fd >= 0) 
	close(fd);

    return ret;
}



int
SetJournalInFSState(const char *devname, int journal_in_fs)
{
	int fd = -1, ret = 0;
	char *buff = NULL, *buff2 = NULL;
	u_int64_t blkcnt;
	u_int32_t blksize;
	daddr_t   mdb_offset;
	HFSMasterDirectoryBlock *mdbp;
	HFSPlusVolumeHeader *vhp;
	off_t embeddedOffset, pos;
	struct JournalInfoBlock *myjib;

	fd = open(devname, O_RDWR);
	if (fd < 0) {
		printf("can't open: %s (%s)\n", devname, strerror(errno));
		ret = -1;
		goto out;
	}

	/* Get the real physical block size. */
	if (ioctl(fd, DKIOCGETBLOCKSIZE, (caddr_t)&blksize) != 0) {
		printf("can't get the device block size (%s). assuming 512\n", strerror(errno));
		blksize = 512;
		ret = -1;
		goto out;
	}

	/* Get the number of physical blocks. */
	if (ioctl(fd, DKIOCGETBLOCKCOUNT, (caddr_t)&blkcnt)) {
		struct stat st;
		printf("failed to get block count. trying stat().\n");
		if (fstat(fd, &st) != 0) {
			ret = -1;
			goto out;
		}

		blkcnt = st.st_size / blksize;
	}

	/*
	 * There used to only be 31 bits worth of block count in
	 * the buffer cache.  So for large volumes a 4K
	 * physical block size is needed.
	 */
	if (blksize == 512 && blkcnt > (u_int64_t)0x000000007fffffff) {
		blksize = 4096;
	}

	/*
	 * At this point:
	 *   blksize has our prefered physical block size
	 *   blkcnt has the total number of physical blocks
	 */

	buff  = (char *)malloc(blksize);
	buff2 = (char *)malloc(blksize);
	
	if (pread(fd, buff, blksize, HFS_PRI_SECTOR(blksize)*blksize) != blksize) {
		printf("failed to read volume header @ offset %d (%s)\n",
			   HFS_PRI_SECTOR(blksize), strerror(errno));
		ret = -1;
		goto out;
	}

	mdbp = (HFSMasterDirectoryBlock *)(buff + HFS_PRI_OFFSET(blksize));


	if (   (SWAP_BE16(mdbp->drSigWord) != kHFSSigWord)
			&& (SWAP_BE16(mdbp->drSigWord) != kHFSPlusSigWord)
			&& (SWAP_BE16(mdbp->drSigWord) != kHFSXSigWord)) {
		ret = -1;
		printf ("SetJournalInFSState: Invalid Volume Signature \n");
		goto out;
	}

	if ((SWAP_BE16(mdbp->drSigWord) == kHFSSigWord) && (SWAP_BE16(mdbp->drEmbedSigWord) != kHFSPlusSigWord)) {
		// normal hfs can not ever be journaled
		goto out;
	} 
	
	/* Get the embedded Volume Header */
	if (SWAP_BE16(mdbp->drEmbedSigWord) == kHFSPlusSigWord) {
	    embeddedOffset = SWAP_BE16(mdbp->drAlBlSt) * 512;
	    embeddedOffset += (u_int64_t)SWAP_BE16(mdbp->drEmbedExtent.startBlock) *
			      (u_int64_t)SWAP_BE32(mdbp->drAlBlkSiz);

	    /*
	     * If the embedded volume doesn't start on a block
	     * boundary, then switch the device to a 512-byte
	     * block size so everything will line up on a block
	     * boundary.
	     */
	    if ((embeddedOffset % blksize) != 0) {
		printf("HFS Mount: embedded volume offset not"
		       " a multiple of physical block size (%d);"
		       " switching to 512\n", blksize);
		
		blkcnt  *= (blksize / 512);
		blksize  = 512;
	    }

	    mdb_offset = (daddr_t)((embeddedOffset / blksize) + HFS_PRI_SECTOR(blksize));
	    if (pread(fd, buff, blksize, mdb_offset * blksize) != blksize) {
		printf("failed to read the embedded vhp @ offset %d\n", mdb_offset * blksize);
		ret = -1;
		goto out;
	    }

	    vhp = (HFSPlusVolumeHeader*) (buff + HFS_PRI_OFFSET(blksize));
		
		mdbp = (HFSMasterDirectoryBlock *)(vhp);
		if (   (SWAP_BE16(mdbp->drSigWord) != kHFSSigWord)
			&& (SWAP_BE16(mdbp->drSigWord) != kHFSPlusSigWord)
			&& (SWAP_BE16(mdbp->drSigWord) != kHFSXSigWord)) {
			ret = -1;
			printf("SetJournalInFSState: Invalid Embedded Volume Signature \n");
			goto out;
		}
		
		
	} else /* pure HFS+ */ {
	    embeddedOffset = 0;
	    vhp = (HFSPlusVolumeHeader*) mdbp;
	}

	//printf("vol header attributes: 0x%x\n", SWAP_BE32(vhp->attributes));
	if ((SWAP_BE32(vhp->attributes) & kHFSVolumeJournaledMask) == 0) {
	    ret = 0;
	    goto out;
	}

	//
	// Now read the journal info block... (when calculating the
	// position, make sure to cast to unsigned first, then to
	// off_t so that things don't get sign-extended improperly
	// or truncated).
	//
	pos = (off_t)((off_t)embeddedOffset +
		      (off_t)((unsigned int )SWAP_BE32(vhp->journalInfoBlock))*(off_t)((unsigned int)SWAP_BE32(vhp->blockSize)));

	if (pread(fd, buff2, blksize, pos) != blksize) {
	    printf("failed to read the journal info block (%s).\n", strerror(errno));
	    ret = -1;
	    goto out;
	}

	myjib = (struct JournalInfoBlock *)buff2;

	// swap this to host native format so we can diddle with the bits
	myjib->flags  = SWAP_BE32(myjib->flags);
	
	if (journal_in_fs) {
		myjib->flags |= kJIJournalInFSMask;
	} else {
		myjib->flags &= ~kJIJournalInFSMask;
	}
	myjib->flags |= kJIJournalNeedInitMask;

	// and now swap back before writing it out
	myjib->flags  = SWAP_BE32(myjib->flags);
	
	if (pwrite(fd, buff2, blksize, pos) != blksize) {
	    printf("failed to re-write the journal info block (%s).\n", strerror(errno));
	    ret = -1;
	    goto out;
	}

	ret = 0;

  out:
	if (buff)
		free(buff);
	if (buff2)
		free(buff2);
	if (fd >= 0) 
		close(fd);

	return ret;
}
