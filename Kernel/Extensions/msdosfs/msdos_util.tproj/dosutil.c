/*
 * Copyright (c) 2000-2005,2008,2010 Apple Inc. All rights reserved.
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
 * Copyright (c) 1998 Robert Nordier
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*	@(#)dosutil.c	3.0	13/09/00	(c) 2000 Apple Computer, Inc.	*/

#include <stdint.h>
#include <mach/machine/boolean.h>

#include <sys/types.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/sysctl.h>
#include <sys/loadable_fs.h>

#include <sys/disk.h>

#include <machine/byte_order.h>

#include <err.h>
#include <sysexits.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h> 
#include <pwd.h>

#include  <CommonCrypto/CommonDigest.h>

#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFStringEncodingExt.h>

#include "../msdosfs.kextproj/msdosfs.kmodproj/bootsect.h"
#include "../msdosfs.kextproj/msdosfs.kmodproj/bpb.h"
#include "../msdosfs.kextproj/msdosfs.kmodproj/direntry.h"

#define FS_TYPE			"msdos"
#define FS_NAME_FILE		"MSDOS"
#define FS_BUNDLE_NAME		"msdosfs.kext"
#define FS_KEXT_DIR			"/System/Library/Extensions/msdosfs.kext"
#define FS_KMOD_DIR			"/System/Library/Extensions/msdosfs.kext/msdosfs"
#define RAWDEV_PREFIX		"/dev/r"
#define BLOCKDEV_PREFIX		"/dev/"
#define MOUNT_COMMAND		"/sbin/mount"
#define UMOUNT_COMMAND		"/sbin/umount"
#define KEXTLOAD_COMMAND	"/sbin/kextload"
#define KMODLOAD_COMMAND	"/sbin/kmodload"
#define READWRITE_OPT		"-w"
#define READONLY_OPT		"-r"
#define SUID_OPT			"suid"
#define NOSUID_OPT			"nosuid"
#define DEV_OPT				"dev"
#define NODEV_OPT			"nodev"
#define LABEL_LENGTH		11
#define MAX_DOS_BLOCKSIZE	4096

#ifndef FSUC_GETUUID
#define FSUC_GETUUID		'k'
#endif

#define FSUC_LABEL		'n'

#define UNKNOWN_LABEL		"Unlabeled"

#define	DEVICE_SUID			"suid"
#define	DEVICE_NOSUID		"nosuid"

#define	DEVICE_DEV			"dev"
#define	DEVICE_NODEV		"nodev"

#define	CLUST_FIRST	2			/* first legal cluster number */
#define	CLUST_RSRVD	0x0ffffff6	/* reserved cluster range */


/* globals */
const char	*progname;	/* our program name, from argv[0] */
int		debug;	/* use -D to enable debug printfs */



/*
 * The following code is re-usable for all FS_util programs
 */
void usage(void);

static int fs_probe(char *devpath, int removable, int writable);
static int fs_mount(char *devpath, char *mount_point, int removable, 
	int writable, int suid, int dev);
static int fs_unmount(char *devpath);
int fs_get_uuid(char *device_path);
static int fs_label(char *devpath, char *volName);
static void fs_set_label_file(char *labelPtr);

static int safe_open(char *path, int flags, mode_t mode);
static void safe_read(int fd, void *buf, int nbytes, off_t off);
static void safe_close(int fd);
static void safe_write(int fd, char *data, int len, off_t off);
static void safe_execv(const char *args[]);

static int checkLoadable(void);
static int oklabel(const char *src);
static void mklabel(char *dest, const char *src);

int get_mount_point(char *device_path, char **mount_path);
void uuid_create_md5_from_name(uuid_t result_uuid, const uuid_t namespace, const void *name, int namelen);
int fs_get_uuid_attr(char *mount_path, uuid_t uuid);
int fs_get_uuid_raw(char *device_path, uuid_t uuid);


int ret = 0;


void usage()
{
	fprintf(stderr, "usage: %s action_arg device_arg [mount_point_arg] [Flags]\n", progname);
	fprintf(stderr, "action_arg:\n");
	fprintf(stderr, "       -%c (Probe)\n", FSUC_PROBE);
	fprintf(stderr, "       -%c (Mount)\n", FSUC_MOUNT);
	fprintf(stderr, "       -%c (Unmount)\n", FSUC_UNMOUNT);
	fprintf(stderr, "       -%c (Get UUID)\n", FSUC_GETUUID);
	fprintf(stderr, "       -%c name\n", FSUC_LABEL);
	fprintf(stderr, "device_arg:\n");
	fprintf(stderr, "       device we are acting upon (for example, 'disk0s2')\n");
	fprintf(stderr, "mount_point_arg:\n");
	fprintf(stderr, "       required for Mount and Force Mount \n");
	fprintf(stderr, "Flags:\n");
	fprintf(stderr, "       required for Mount, Force Mount and Probe\n");
	fprintf(stderr, "       indicates removable or fixed (for example 'fixed')\n");
	fprintf(stderr, "       indicates readonly or writable (for example 'readonly')\n");
	fprintf(stderr, "Examples:\n");
	fprintf(stderr, "		%s -p disk0s2 fixed writable\n", progname);
	fprintf(stderr, "		%s -m disk0s2 /my/hfs removable readonly\n", progname);
	exit(FSUR_INVAL);
}

int main(int argc, char **argv)
{
    char		rawdevpath[MAXPATHLEN];
    char		blockdevpath[MAXPATHLEN];
    char		opt;
    struct stat	sb;
    int			ret = FSUR_INVAL;


    /* save & strip off program name */
    progname = argv[0];
    argc--;
    argv++;

    /* secret debug flag - must be 1st flag */
    debug = (argc > 0 && !strcmp(argv[0], "-D"));
    if (debug) { /* strip off debug flag argument */
        argc--;
        argv++;
    }

    if (argc < 2 || argv[0][0] != '-')
        usage();
    opt = argv[0][1];
    if (opt != FSUC_PROBE && opt != FSUC_MOUNT && opt != FSUC_UNMOUNT && opt != FSUC_GETUUID && opt != FSUC_LABEL)
        usage(); /* Not supported action */
    if ((opt == FSUC_MOUNT || opt == FSUC_UNMOUNT || opt == FSUC_LABEL) && argc < 3)
        usage(); /* mountpoint arg missing! */

    if (snprintf(rawdevpath, sizeof(rawdevpath), "%s%s", RAWDEV_PREFIX, argv[1]) >= sizeof(rawdevpath))
    	exit(FSUR_INVAL);
    if (stat(rawdevpath, &sb) != 0) {
        fprintf(stderr, "%s: stat %s failed, %s\n", progname, rawdevpath,
                strerror(errno));
        exit(FSUR_INVAL);
    }

    if (snprintf(blockdevpath, sizeof(blockdevpath), "%s%s", BLOCKDEV_PREFIX, argv[1]) >= sizeof(blockdevpath))
    	exit(FSUR_INVAL);
    if (stat(blockdevpath, &sb) != 0) {
        fprintf(stderr, "%s: stat %s failed, %s\n", progname, blockdevpath,
                strerror(errno));
        exit(FSUR_INVAL);
    }

    switch (opt) {
        case FSUC_PROBE: {
            if (argc != 4)
                usage();
            ret = fs_probe(rawdevpath,
                        strcmp(argv[2], DEVICE_FIXED),
                        strcmp(argv[3], DEVICE_READONLY));
            break;
        }

        case FSUC_MOUNT:
        case FSUC_MOUNT_FORCE:
            if (argc != 7)
                usage();
            if (strcmp(argv[3], DEVICE_FIXED) && strcmp(argv[3], DEVICE_REMOVABLE)) {
                printf("msdosfs.util: ERROR: unrecognized flag (removable/fixed) argv[%d]='%s'\n",3,argv[3]);
                usage();
            }
                if (strcmp(argv[4], DEVICE_READONLY) && strcmp(argv[4], DEVICE_WRITABLE)) {
                    printf("msdosfs.util: ERROR: unrecognized flag (readonly/writable) argv[%d]='%s'\n",4,argv[4]);
                    usage();
                }
                if (strcmp(argv[5], DEVICE_SUID) && strcmp(argv[5], DEVICE_NOSUID)) {
                    printf("msdosfs.util: ERROR: unrecognized flag (suid/nosuid) argv[%d]='%s'\n",5,argv[5]);
                    usage();
                }
                if (strcmp(argv[6], DEVICE_DEV) && strcmp(argv[6], DEVICE_NODEV)) {
                    printf("msdosfs.util: ERROR: unrecognized flag (dev/nodev) argv[%d]='%s'\n",6,argv[6]);
                    usage();
                }
                ret = fs_mount(blockdevpath,
                            argv[2],
                            strcmp(argv[3], DEVICE_FIXED),
                            strcmp(argv[4], DEVICE_READONLY),
                            strcmp(argv[5], DEVICE_NOSUID),
                            strcmp(argv[6], DEVICE_NODEV));
            break;
        case FSUC_UNMOUNT:
            ret = fs_unmount(rawdevpath);
            break;
		case FSUC_GETUUID:
			ret = fs_get_uuid(blockdevpath);
			break;
        case FSUC_LABEL:
            ret = fs_label(rawdevpath, argv[2]);
            break;
        default:
            usage();
    }

    exit(ret);

    return(ret);
}

/*
 * Begin Filesystem-specific code
 */
static int valid_boot_sector(int fd, const char *devpath, char buf[MAX_DOS_BLOCKSIZE])
{
    union bootsector *bsp;
    struct byte_bpb33 *b33;
    struct byte_bpb50 *b50;
    struct byte_bpb710 *b710;
	u_int32_t dev_block_size;
    u_int16_t	bps;
    u_int8_t	spc;
	
	if (ioctl(fd, DKIOCGETBLOCKSIZE, &dev_block_size) < 0)
	{
		fprintf(stderr, "%s: ioctl(DKIOCGETBLOCKSIZE) for %s failed, %s\n",
				progname, devpath, strerror(errno));
		return FSUR_IO_FAIL;
	}
	if (dev_block_size > MAX_DOS_BLOCKSIZE)
	{
		fprintf(stderr, "%s: block size of %s is too big (%lu)\n",
				progname, devpath, (unsigned long) dev_block_size);
		return FSUR_UNRECOGNIZED;
	}
	
    /*
     * Read the boot sector of the filesystem, and then check the
     * boot signature.  If not a dos boot sector then error out.
     *
     * NOTE: 4096 is a maximum sector size in current...
     */
    safe_read(fd, buf, MAX_DOS_BLOCKSIZE, 0);
	
    bsp = (union bootsector *)buf;
    b33 = (struct byte_bpb33 *)bsp->bs33.bsBPB;
    b50 = (struct byte_bpb50 *)bsp->bs50.bsBPB;
    b710 = (struct byte_bpb710 *)bsp->bs710.bsBPB;

    /* [2699033]
     *
     * The first three bytes are an Intel x86 jump instruction.  It should be one
     * of the following forms:
     *    0xE9 0x?? 0x??
     *    0xEB 0x?? 0x90
     * where 0x?? means any byte value is OK.
     *
     * [5016947]
     *
     * Windows doesn't actually check the third byte if the first byte is 0xEB,
     * so we don't either
     */
    if (bsp->bs50.bsJump[0] != 0xE9
        && bsp->bs50.bsJump[0] != 0xEB)
    {
        return FSUR_UNRECOGNIZED;
    }
	
    /* It is possible that the above check could match a partition table, or some */
    /* non-FAT disk meant to boot a PC.  Check some more fields for sensible values. */
	
    /* We only work with 512, 1024, 2048, and 4096 byte sectors */
    bps = getuint16(b33->bpbBytesPerSec);
    if ((bps < 0x200) || (bps & (bps - 1)) || (bps > MAX_DOS_BLOCKSIZE))
	{
        return(FSUR_UNRECOGNIZED);
	}
	
    /* Check to make sure valid sectors per cluster */
    spc = b33->bpbSecPerClust;
    if ((spc == 0 ) || (spc & (spc - 1)))
	{
        return(FSUR_UNRECOGNIZED);
	}
	
	/* Make sure the number of FATs is OK; on NTFS, this will be zero */
	if (b33->bpbFATs == 0)
	{
		return(FSUR_UNRECOGNIZED);
	}
	
	/* Make sure the total sectors is non-zero */
	if (getuint16(b33->bpbSectors) == 0 && getuint32(b50->bpbHugeSectors) == 0)
	{
		return(FSUR_UNRECOGNIZED);
	}
	
	/* Make sure there is a root directory */
	if (getuint16(b33->bpbRootDirEnts) == 0 && getuint32(b710->bpbRootClust) == 0)
	{
		return(FSUR_UNRECOGNIZED);
	}
	
	return FSUR_RECOGNIZED;
}


static int fs_probe(char *devpath, int removable, int writable)
{
    int fd;
	int err;
    struct dosdirentry *dirp;
    union bootsector *bsp;
    struct byte_bpb33 *b33;
    struct byte_bpb50 *b50;
    struct byte_bpb710 *b710;
    u_int16_t	bps;
    u_int8_t	spc;
    unsigned	rootDirSectors;
    unsigned	i,j, finished;
    char diskLabel[LABEL_LENGTH];
    char buf[MAX_DOS_BLOCKSIZE];

    fd = safe_open(devpath, O_RDONLY, 0);

	err = valid_boot_sector(fd, devpath, buf);
	if (err != FSUR_RECOGNIZED)
		return err;
	
    bsp = (union bootsector *)buf;
    b33 = (struct byte_bpb33 *)bsp->bs33.bsBPB;
    b50 = (struct byte_bpb50 *)bsp->bs50.bsBPB;
    b710 = (struct byte_bpb710 *)bsp->bs710.bsBPB;
    
	/* valid_boot_sector sanity checked bytes-per-sector and sectors-per-cluster */
    bps = getuint16(b33->bpbBytesPerSec);
    spc = b33->bpbSecPerClust;

    /* we know this disk, find the volume label */
    /* First, find the root directory */
    diskLabel[0] = 0;
    finished = false;
    rootDirSectors = ((getuint16(b50->bpbRootDirEnts) * sizeof(struct dosdirentry)) +
                      (bps-1)) / bps;
    if (rootDirSectors) {			/* FAT12 or FAT16 */
    	unsigned firstRootDirSecNum;
    	char rootdirbuf[MAX_DOS_BLOCKSIZE];
    	
        firstRootDirSecNum = getuint16(b33->bpbResSectors) + (b33->bpbFATs * getuint16(b33->bpbFATsecs));
        for (i=0; i< rootDirSectors; i++) {
            safe_read(fd, rootdirbuf, bps, (firstRootDirSecNum+i)*bps);
            dirp = (struct dosdirentry *)rootdirbuf;
            for (j=0; j<bps; j+=sizeof(struct dosdirentry), dirp++) {
                if (dirp->deName[0] == SLOT_EMPTY) {
                    finished = true;
                    break;
                }
                else if (dirp->deName[0] == SLOT_DELETED)
                    continue;
                else if (dirp->deAttributes == ATTR_WIN95)
                    continue;
                else if (dirp->deAttributes & ATTR_VOLUME) {
                    strncpy(diskLabel, (char*)dirp->deName, LABEL_LENGTH);
                    finished = true;
                    break;
                }
            }	/* j */
		if (finished == true)
            break;
        }	/* i */
    }
    else {	/* FAT32 */
        u_int32_t cluster;
        u_int32_t bytesPerCluster;
        u_int8_t *rootDirBuffer;
        off_t readOffset;
        
        bytesPerCluster = (u_int32_t) bps * (u_int32_t) spc;
        rootDirBuffer = malloc(bytesPerCluster);
        cluster = getuint32(b710->bpbRootClust);
        
        finished = false;
        while (!finished && cluster >= CLUST_FIRST && cluster < CLUST_RSRVD)
        {
            /* Find sector where clusters start */
            readOffset = getuint16(b710->bpbResSectors) + (b710->bpbFATs * getuint32(b710->bpbBigFATsecs));
            /* Find sector where "cluster" starts */
            readOffset += ((off_t) cluster - CLUST_FIRST) * (off_t) spc;
            /* Convert to byte offset */
            readOffset *= (off_t) bps;
            
            /* Read in "cluster" */
            safe_read(fd, rootDirBuffer, bytesPerCluster, readOffset);
            dirp = (struct dosdirentry *) rootDirBuffer;
            
            /* Examine each directory entry in this cluster */
            for (i=0; i < bytesPerCluster; i += sizeof(struct dosdirentry), dirp++)
            {
                if (dirp->deName[0] == SLOT_EMPTY) {
                    finished = true;	// Reached end of directory (never used entry)
                    break;
                }
                else if (dirp->deName[0] == SLOT_DELETED)
                    continue;
                else if (dirp->deAttributes == ATTR_WIN95)
                    continue;
                else if (dirp->deAttributes & ATTR_VOLUME) {
                    strncpy(diskLabel, (char *)dirp->deName, LABEL_LENGTH);
                    finished = true;
                    break;
                }
            }
            if (finished)
                break;

            /* Find next cluster in the chain by reading the FAT */
            
            /* Find first sector of FAT */
            readOffset = getuint16(b710->bpbResSectors);
            /* Find sector containing "cluster" entry in FAT */
            readOffset += (cluster * 4) / bps;
            /* Convert to byte offset */
            readOffset *= bps;
            
            /* Read one sector of the FAT */
            safe_read(fd, rootDirBuffer, bps, readOffset);
            
            cluster = getuint32(rootDirBuffer + ((cluster * 4) % bps));
            cluster &= 0x0FFFFFFF;	// ignore reserved upper bits
        }
        free(rootDirBuffer);
    }	/* rootDirSectors */

	/* else look in the boot blocks */
    if (diskLabel[0] == 0) {
        if (getuint16(b50->bpbRootDirEnts) == 0) { /* Its a FAT32 */
            if (((struct extboot *)bsp->bs710.bsExt)->exBootSignature == EXBOOTSIG) {
            	strncpy(diskLabel, (char *)((struct extboot *)bsp->bs710.bsExt)->exVolumeLabel, LABEL_LENGTH);
			}
        }
        else if (((struct extboot *)bsp->bs50.bsExt)->exBootSignature == EXBOOTSIG) {
            strncpy(diskLabel, (char *)((struct extboot *)bsp->bs50.bsExt)->exVolumeLabel, LABEL_LENGTH);
        }
    }

    fs_set_label_file(diskLabel);

    safe_close(fd);

    return(FSUR_RECOGNIZED);
}


static int fs_mount(char *devpath, char *mount_point, int removable, int writable, int suid, int dev) {
    const char *kextargs[] = {KEXTLOAD_COMMAND, FS_KEXT_DIR, NULL};
    const char *mountargs[] = {MOUNT_COMMAND, READWRITE_OPT, "-o", SUID_OPT, "-o",
        DEV_OPT, "-t", FS_TYPE, devpath, mount_point, NULL};

    if (! writable)
        mountargs[1] = READONLY_OPT;

    if (! suid)
        mountargs[3] = NOSUID_OPT;

    if (! dev)
        mountargs[5] = NODEV_OPT;

    if (checkLoadable())
        safe_execv(kextargs); /* better here than in mount_udf */
    safe_execv(mountargs);
    ret = FSUR_IO_SUCCESS;

    return ret;
}

static int fs_unmount(char *devpath) {
        const char *umountargs[] = {UMOUNT_COMMAND, devpath, NULL};

        safe_execv(umountargs);
        return(FSUR_IO_SUCCESS);
}



/*
 * Begin Filesystem-specific code
 */

/*
 * Check to see if a file system is mounted on the given device path.
 * If it is, and it is FAT, then return a pointer to the volume's mount-on
 * path and FSUR_IO_SUCCESS.  If a volume is mounted but is not FAT, return
 * FSUR_UNRECOGNIZED.  If no volume is mounted, return a NULL pointer for the
 * mount-on path and FSUR_IO_SUCCESS.
 *
 * We use this when asked to get or set the volume UUID.  If the volume is
 * mounted, we go through the file system; otherwise, we access the device
 * directly.
 */
int get_mount_point(char *device_path, char **mount_path)
{
	int result;
	int i, num_mounts;
	struct statfs *buf;
	
	/* Assume no mounted volume is found. */
	*mount_path = NULL;
	result = FSUR_IO_SUCCESS;
	
	num_mounts = getmntinfo(&buf, MNT_NOWAIT);
	if (num_mounts == 0)
		return FSUR_IO_FAIL;
	
	for (i = 0; i < num_mounts; ++i)
	{
		if (!strcmp(device_path, buf[i].f_mntfromname))
		{
			/* Found a mounted volume.  Check the file system type. */
			if (!strcmp(buf[i].f_fstypename, FS_TYPE))
			{
				*mount_path = buf[i].f_mntonname;
			}
			else
			{
				result = FSUR_UNRECOGNIZED;
			}
			break;
		}
	}
	
	return result;
}

/*
 * Create a version 3 UUID from unique data in the SHA1 "name space".
 * Version 3 UUIDs are derived using MD5 checksum.  Here, the unique
 * data is the 4-byte volume ID and the number of sectors (normalized
 * to a 4-byte little endian value).
 */
static void msdosfs_generate_volume_uuid(uuid_t result_uuid, uint8_t volumeID[4], uint32_t totalSectors)
{
	CC_MD5_CTX c;
	uint8_t sectorsLittleEndian[4];
	
	UUID_DEFINE( kFSUUIDNamespaceSHA1, 0xB3, 0xE2, 0x0F, 0x39, 0xF2, 0x92, 0x11, 0xD6, 0x97, 0xA4, 0x00, 0x30, 0x65, 0x43, 0xEC, 0xAC );
	
	/*
	 * Normalize totalSectors to a little endian value so that this returns the
	 * same UUID regardless of endianness.
	 */
	putuint32(sectorsLittleEndian, totalSectors);
	
	/*
	 * Generate an MD5 hash of our "name space", and our unique bits of data
	 * (the volume ID and total sectors).
	 */
	CC_MD5_Init(&c);
	CC_MD5_Update(&c, kFSUUIDNamespaceSHA1, sizeof(uuid_t));
	CC_MD5_Update(&c, volumeID, 4);
	CC_MD5_Update(&c, sectorsLittleEndian, sizeof(sectorsLittleEndian));
	CC_MD5_Final(result_uuid, &c);
	
	/* Force the resulting UUID to be a version 3 UUID. */
	result_uuid[6] = (result_uuid[6] & 0x0F) | 0x30;
	result_uuid[8] = (result_uuid[8] & 0x3F) | 0x80;
}

/* Get the volume UUID of a mounted volume. */
int fs_get_uuid_attr(char *mount_path, uuid_t uuid)
{
    struct attrlist attrlist;
	struct {
		uint32_t length;
		uuid_t uuid;
    } buf;
    int result;
    
    attrlist.bitmapcount = 5;
    attrlist.reserved = 0;
    attrlist.commonattr = 0;
    attrlist.volattr = ATTR_VOL_INFO | ATTR_VOL_UUID;
    attrlist.dirattr = 0;
    attrlist.fileattr = 0;
    attrlist.forkattr = 0;
    
    result = getattrlist(mount_path, &attrlist, &buf, sizeof(buf), 0);
    if (result)
    {
		/*
		 * If the volume doesn't have a UUID (because it doesn't have an
		 * extended boot parameter block, or because the volume ID in the
		 * extended BPB was zero), then we'll get EINVAL here.  That's an
		 * "expected failure" and we shouldn't print a message about it.
		 */
		if (debug || errno != EINVAL)
			warn("fs_get_uuid_attr: getattrlist(\"%s\", ATTR_VOL_UUID)", mount_path);
		return FSUR_RECOGNIZED;
    }
    
    memcpy(uuid, buf.uuid, sizeof(uuid_t));
    
    return FSUR_IO_SUCCESS;
}

/* Get the volume UUID from an unmounted device. */
int fs_get_uuid_raw(char *device_path, uuid_t uuid)
{
	int fd;
	int result;
    union bootsector *bsp;
    struct byte_bpb50 *b50;
    struct byte_bpb710 *b710;
	struct extboot *extboot;
    char buf[MAX_DOS_BLOCKSIZE];
	
	fd = safe_open(device_path, O_RDONLY, 0);
	
	result = valid_boot_sector(fd, device_path, buf);
	if (result != FSUR_RECOGNIZED)
		return result;

    bsp = (union bootsector *)buf;
    b50 = (struct byte_bpb50 *)bsp->bs50.bsBPB;
    b710 = (struct byte_bpb710 *)bsp->bs710.bsBPB;

	if (getuint16(b50->bpbRootDirEnts) == 0)
	{
		/* FAT32 */
		extboot = (struct extboot *)(buf + 64);
	}
	else
	{
		/* FAT12 or FAT16 */
		extboot = (struct extboot *)(buf + 36);
	}
	
	/*
	 * If it has a volume ID and it is non-zero, convert it to a UUID.
	 */
	if (extboot->exBootSignature == EXBOOTSIG &&
		(extboot->exVolumeID[0] || extboot->exVolumeID[1] ||
		 extboot->exVolumeID[2] || extboot->exVolumeID[3]))
	{
		uint32_t total_sectors;
		
		/* Get the total sectors as a 32-bit value */
		total_sectors = getuint16(b50->bpbSectors);
		if (total_sectors == 0)
			total_sectors = getuint32(b50->bpbHugeSectors);

		msdosfs_generate_volume_uuid(uuid, extboot->exVolumeID, total_sectors);
		result = FSUR_IO_SUCCESS;
	}
	/* else, result = FSUR_RECOGNIZED from above */
	
	safe_close(fd);
	
	return result;
}


/* Get the volume UUID */
int fs_get_uuid(char *device_path)
{
	char *mount_path;
	uuid_t uuid;
	char uuid_text[40];
	int result;
	
	result = get_mount_point(device_path, &mount_path);
	if (result != FSUR_IO_SUCCESS)
		return result;
	
	if (mount_path)
		result = fs_get_uuid_attr(mount_path, uuid);
	else
		result = fs_get_uuid_raw(device_path, uuid);
	
	if (result == FSUR_IO_SUCCESS)
	{
		uuid_unparse(uuid, uuid_text);
		write(1, uuid_text, strlen(uuid_text));
	}
	
	return result;
}


static int fs_label(char *devpath, char *volName)
{
        int fd;
        union bootsector *bsp;
        struct byte_bpb33 *b33;
        struct byte_bpb50 *b50;
        u_int16_t	bps;
        u_int8_t	spc;
        char		tmplabel[LABEL_LENGTH], label[LABEL_LENGTH];
        char 		buf[MAX_DOS_BLOCKSIZE];
        CFStringRef 	cfstr;


        /* First normalize the label */
        if (volName == NULL)
                errx(EX_USAGE, "No label was given");

        /* Convert it from UTF-8 */
        cfstr = CFStringCreateWithCString(kCFAllocatorDefault, volName, kCFStringEncodingUTF8);
        if (cfstr == NULL)
                errx(EX_DATAERR, "Bad UTF8 Name");
        if (! CFStringGetCString(cfstr, tmplabel, LABEL_LENGTH, kCFStringEncodingWindowsLatin1))
               errx(EX_DATAERR, "Could not convert to DOS Latin1");
        CFRelease(cfstr);

        if (! oklabel(tmplabel))
                errx(EX_DATAERR, "Label has illegal characters");

        /* Finall format it */
        mklabel(label, tmplabel);

        
        fd = safe_open(devpath, O_RDWR, 0);

        /*
         * Read the boot sector of the filesystem, and then check the
         * boot signature.  If not a dos boot sector then error out.
         *
         * NOTE: 4096 is a maximum sector size in current...
         */
        safe_read(fd, buf, MAX_DOS_BLOCKSIZE, 0);

        bsp = (union bootsector *)buf;
        b33 = (struct byte_bpb33 *)bsp->bs33.bsBPB;
        b50 = (struct byte_bpb50 *)bsp->bs50.bsBPB;

        if (bsp->bs50.bsBootSectSig0 != BOOTSIG0
            || bsp->bs50.bsBootSectSig1 != BOOTSIG1) {
                return(FSUR_UNRECOGNIZED);
        }

        /* Both partitions tables and boot sectors pass the above test, do do some more */

        /* We only work with 512, 1024, 2048 and 4096 byte sectors */
        bps = getuint16(b33->bpbBytesPerSec);
        if ((bps < 0x200) || (bps & (bps - 1)) || (bps > MAX_DOS_BLOCKSIZE))
                return(FSUR_UNRECOGNIZED);

        /* Check to make sure valid sectors per cluster */
        spc = b33->bpbSecPerClust;
        if ((spc == 0 ) || (spc & (spc - 1)))
                return(FSUR_UNRECOGNIZED);

        /* we know this disk, find the volume label */
        if (getuint16(b50->bpbRootDirEnts) == 0) {
                /* Its a FAT32 */
                strncpy((char *)((struct extboot *)bsp->bs710.bsExt)->exVolumeLabel, label, LABEL_LENGTH);
        }
        else if (((struct extboot *)bsp->bs50.bsExt)->exBootSignature == EXBOOTSIG) {
                strncpy((char *)((struct extboot *)bsp->bs50.bsExt)->exVolumeLabel, label, LABEL_LENGTH);
        }


        safe_write(fd, buf, MAX_DOS_BLOCKSIZE, 0);

        safe_close(fd);

        return(FSUR_IO_SUCCESS);
}


static CFStringEncoding GetDefaultDOSEncoding(void)
{
	CFStringEncoding encoding;
    struct passwd *passwdp;
	int fd;
	ssize_t size;
	char buffer[MAXPATHLEN + 1];

	/*
	 * Get a default (Mac) encoding.  We use the CFUserTextEncoding
	 * file since CFStringGetSystemEncoding() always seems to
	 * return 0 when msdos.util is executed via disk arbitration.
	 */
	encoding = kCFStringEncodingMacRoman;	/* Default to Roman/Latin */
	if ((passwdp = getpwuid(getuid()))) {
		strlcpy(buffer, passwdp->pw_dir, sizeof(buffer));
		strlcat(buffer, "/.CFUserTextEncoding", sizeof(buffer));

		if ((fd = open(buffer, O_RDONLY, 0)) > 0) {
			size = read(fd, buffer, MAXPATHLEN);
			buffer[(size < 0 ? 0 : size)] = '\0';
			close(fd);
			encoding = (CFStringEncoding)strtol(buffer, NULL, 0);
		}
	}

	/* Convert the Mac encoding to a DOS/Windows encoding. */
	switch (encoding) {
		case kCFStringEncodingMacRoman:
			encoding = kCFStringEncodingDOSLatin1;
			break;
		case kCFStringEncodingMacJapanese:
			encoding = kCFStringEncodingDOSJapanese;
			break;
		case kCFStringEncodingMacChineseTrad:
			encoding = kCFStringEncodingDOSChineseTrad;
			break;
		case kCFStringEncodingMacKorean:
			encoding = kCFStringEncodingDOSKorean;
			break;
		case kCFStringEncodingMacArabic:
			encoding = kCFStringEncodingDOSArabic;
			break;
		case kCFStringEncodingMacHebrew:
			encoding = kCFStringEncodingDOSHebrew;
			break;
		case kCFStringEncodingMacGreek:
			encoding = kCFStringEncodingDOSGreek;
			break;
		case kCFStringEncodingMacCyrillic:
		case kCFStringEncodingMacUkrainian:
			encoding = kCFStringEncodingDOSCyrillic;
			break;
		case kCFStringEncodingMacThai:
			encoding = kCFStringEncodingDOSThai;
			break;
		case kCFStringEncodingMacChineseSimp:
			encoding = kCFStringEncodingDOSChineseSimplif;
			break;
		case kCFStringEncodingMacCentralEurRoman:
		case kCFStringEncodingMacCroatian:
		case kCFStringEncodingMacRomanian:
			encoding = kCFStringEncodingDOSLatin2;
			break;
		case kCFStringEncodingMacTurkish:
			encoding = kCFStringEncodingDOSTurkish;
			break;
		case kCFStringEncodingMacIcelandic:
			encoding = kCFStringEncodingDOSIcelandic;
			break;
		case kCFStringEncodingMacFarsi:
			encoding = kCFStringEncodingDOSArabic;
			break;
		default:
			encoding = kCFStringEncodingInvalidId;	/* Error: no corresponding Windows encoding */
			break;
	}
	
	return encoding;
}

/* Set the name of this file system */
static void fs_set_label_file(char *labelPtr)
{
    int				i;
    CFStringEncoding encoding;
    char			label[LABEL_LENGTH+1];
    char			labelUTF8[LABEL_LENGTH*3];
    CFStringRef 	cfstr;

	/* Make a local copy of the label */
	strncpy(label, labelPtr, LABEL_LENGTH);
	label[LABEL_LENGTH] = 0;

	/* Convert leading 0x05 to 0xE5 for multibyte languages like Japanese */
	if (label[0] == 0x05)
		label[0] = 0xE5;

	/* Check for illegal characters */
	if (!oklabel(label))
		label[0] = 0;

	/* Remove any trailing spaces */
	for (i=LABEL_LENGTH-1; i>=0; --i) {
		if (label[i] == ' ')
			label[i] = 0;
		else
			break;
	}

    /* Convert it to UTF-8 */
    encoding = GetDefaultDOSEncoding();
    cfstr = CFStringCreateWithCString(NULL, label, encoding);
    if (cfstr == NULL && encoding != kCFStringEncodingDOSLatin1)
        cfstr = CFStringCreateWithCString(NULL, label, kCFStringEncodingDOSLatin1);
	if (cfstr == NULL)
		labelUTF8[0] = 0;
	else {
		CFMutableStringRef mutable;
		
		mutable = CFStringCreateMutableCopy(NULL, 0, cfstr);
		if (mutable != NULL) {
			CFStringNormalize(mutable, kCFStringNormalizationFormD);
			CFStringGetCString(mutable, labelUTF8, sizeof(labelUTF8), kCFStringEncodingUTF8);
			CFRelease(mutable);
		}
		
		CFRelease(cfstr);
	}

    /* At this point, labelUTF8 should contain a correctly formatted name (possibly empty) */
    write(1, labelUTF8, strlen(labelUTF8));
}

/*
 * Based from newfs_msdos....to support the same 'functionality'...thanks
 */
        	
/*
 * Check a volume label.
 */
static int
oklabel(const char *src)
{
    int c, i;

    for (i = 0, c = 0; i <= 11; i++) {
        c = (u_char)*src++;
        if (c < ' ' + !i || strchr("\"*+,./:;<=>?[\\]|", c))
            break;
    }
    return i && !c;
}

/*
 * Make a volume label.
 */
static void
mklabel(char *dest, const char *src)
{
    int c, i;

    for (i = 0; i < 11; i++) {
	c = *src ? toupper(*src++) : ' ';
	*dest++ = !i && c == '\xe5' ? 5 : c;
    }
}

static int
safe_open(char *path, int flags, mode_t mode)
{
	int fd = open(path, flags, mode);

	if (fd < 0) {
		fprintf(stderr, "%s: open %s failed, %s\n", progname, path,
			strerror(errno));
		exit(FSUR_IO_FAIL);
	}
	return(fd);
}


static void
safe_close(int fd)
{
	if (close(fd)) {
		fprintf(stderr, "%s: safe_close failed, %s\n", progname,
			strerror(errno));
		exit(FSUR_IO_FAIL);
	}
}

void
safe_execv(const char *args[])
{
	int		pid;
	union wait	status;

	pid = fork();
	if (pid == 0) {
		(void)execv(args[0], (char *const *) args);
		fprintf(stderr, "%s: execv %s failed, %s\n", progname, args[0],
			strerror(errno));
		exit(FSUR_IO_FAIL);
	}
	if (pid == -1) {
		fprintf(stderr, "%s: fork failed, %s\n", progname,
			strerror(errno));
		exit(FSUR_IO_FAIL);
	}
	if (wait4(pid, (int *)&status, 0, NULL) != pid) {
		fprintf(stderr, "%s: BUG executing %s command\n", progname,
			args[0]);
		exit(FSUR_IO_FAIL);
	} else if (!WIFEXITED(status)) {
		fprintf(stderr, "%s: %s command aborted by signal %d\n",
			progname, args[0], WTERMSIG(status));
		exit(FSUR_IO_FAIL);
	} else if (WEXITSTATUS(status)) {
		fprintf(stderr, "%s: %s command failed, exit status %d: %s\n",
			progname, args[0], WEXITSTATUS(status),
			strerror(WEXITSTATUS(status)));
		exit(FSUR_IO_FAIL);
	}
}


static void
safe_read(int fd, void *buf, int nbytes, off_t off)
{
	if (lseek(fd, off, SEEK_SET) == -1) {
		fprintf(stderr, "%s: device seek error @ %qu, %s\n", progname,
			off, strerror(errno));
		exit(FSUR_IO_FAIL);
	}
	if (read(fd, buf, nbytes) != nbytes) {
		fprintf(stderr, "%s: device safe_read error @ %qu, %s\n", progname,
			off, strerror(errno));
		exit(FSUR_IO_FAIL);
	}
}


void
safe_write(int fd, char *buf, int nbytes, off_t off)
{
        if (lseek(fd, off, SEEK_SET) == -1) {
                fprintf(stderr, "%s: device seek error @ %qu, %s\n", progname,
                        off, strerror(errno));
                exit(FSUR_IO_FAIL);
        }
        if (write(fd, buf, nbytes) != nbytes) {
                fprintf(stderr, "%s: write failed, %s\n", progname,
                        strerror(errno));
                exit(FSUR_IO_FAIL);
        }
}


/* Return non-zero if the file system is not yet loaded. */
static int checkLoadable(void)
{
	int error;
	struct vfsconf vfc;
	
	error = getvfsbyname(FS_TYPE, &vfc);

	return error;
}

/* end of DOS.util.c */
