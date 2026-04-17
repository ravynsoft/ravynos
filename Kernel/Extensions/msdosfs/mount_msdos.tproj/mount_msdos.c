/*
 * Copyright (c) 2000-2007, 2009-2011 Apple Inc. All rights reserved.
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
/*	$NetBSD: mount_msdos.c,v 1.18 1997/09/16 12:24:18 lukem Exp $	*/

/*
 * Copyright (c) 1994 Christopher G. Demetriou
 * All rights reserved.
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
 *      This product includes software developed by Christopher G. Demetriou.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Various system headers use standard int types */
#include <stdint.h>

/* Get the boolean_t type. */
#include <mach/machine/boolean.h>

#include <sys/param.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/wait.h>

#include "../msdosfs.kextproj/msdosfs.kmodproj/msdosfsmount.h"

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
/* must be after stdio to declare fparseln */
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>
#include <fcntl.h>
#include <mntopts.h>

#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFStringEncodingExt.h>
#include <CoreFoundation/CFNumber.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/storage/IOStorageProtocolCharacteristics.h>

/*
 * TODO: The -u, -g, and -m options could be expressed as ordinary mount
 * options now that we're using the new getmntopts from libutil.  That
 * would make it possible to specify those options in /etc/fstab...
 */
static struct mntopt mopts[] = {
	MOPT_STDOPTS,
	MOPT_FORCE,
	MOPT_ASYNC,
	MOPT_SYNC,
	MOPT_DEFWRITE,
	MOPT_NOATIME,
	MOPT_UPDATE,
	{ NULL }
};

static gid_t	a_gid __P((char *));
static uid_t	a_uid __P((char *));
static mode_t	a_mask __P((char *));
static void		usage __P((void));

static int checkLoadable(void);
static char *progname;
static int load_kmod(void);
static void FindVolumeName(struct msdosfs_args *args);

/* Taken from diskdev_cmds/disklib/getmntopts.c */
static void
checkpath(const char *path, char resolved[MAXPATHLEN])
{
	struct stat sb;

	if (realpath(path, resolved) != NULL && stat(resolved, &sb) == 0) {
		if (!S_ISDIR(sb.st_mode)) 
			errx(EX_USAGE, "%s: not a directory", resolved);
	} else
		errx(EX_USAGE, "%s: %s", resolved, strerror(errno));
}

/* Adapted from diskdev_cmds/disklib/getmntopts.c, with fixes. */
static void
rmslashes(const char *rrpin, char rrpout[MAXPATHLEN])
{
	char *rrpoutstart;

    for (rrpoutstart = rrpout; (*rrpin != '\0') && (rrpout - rrpoutstart < MAXPATHLEN - 1); *rrpout++ = *rrpin++) {
		/* skip all double slashes */
		while (*rrpin == '/' && *(rrpin + 1) == '/')
			 rrpin++;
	}

	/* remove trailing slash if necessary */
	if (rrpout - rrpoutstart > 1 && *(rrpout - 1) == '/')
		*(rrpout - 1) = '\0';
	else
		*rrpout = '\0';
}


/*
 * Given a BSD disk name (eg., "disk0s3" or "disk1"), return non-zero if
 * that disk is an internal disk, but not a removable disk.
 */
static int disk_default_async(char *disk)
{
    io_iterator_t iter;
	kern_return_t err;
	int result = 0;
	
	err = IOServiceGetMatchingServices(kIOMasterPortDefault,
		IOBSDNameMatching(kIOMasterPortDefault, 0, disk), &iter);
	if (err == 0)
	{
		io_object_t obj;
		obj = IOIteratorNext(iter);
		if (obj)
		{
			CFBooleanRef removableRef;
			CFDictionaryRef protocolCharacteristics;
			protocolCharacteristics = IORegistryEntrySearchCFProperty(obj,
				kIOServicePlane,
				CFSTR(kIOPropertyProtocolCharacteristicsKey),
				kCFAllocatorDefault,
				kIORegistryIterateRecursively|kIORegistryIterateParents);
			
			if (protocolCharacteristics && CFDictionaryGetTypeID() == CFGetTypeID(protocolCharacteristics))
			{
				CFStringRef location;
				location = CFDictionaryGetValue(protocolCharacteristics, CFSTR(kIOPropertyPhysicalInterconnectLocationKey));
				if(location && CFStringGetTypeID() == CFGetTypeID(location))
				{
					if(CFEqual(location, CFSTR(kIOPropertyInternalKey)))
						result = 1;		/* Internal => async */
				}
			}
			if (protocolCharacteristics)
				CFRelease(protocolCharacteristics);
			
			removableRef = IORegistryEntrySearchCFProperty(obj,
				kIOServicePlane,
				CFSTR(kIOMediaRemovableKey),
				kCFAllocatorDefault,
				kIORegistryIterateRecursively|kIORegistryIterateParents);
			
			if (removableRef)
			{
				if (CFBooleanGetTypeID() == CFGetTypeID(removableRef))
				{
					if (CFBooleanGetValue(removableRef))
						result = 0;		/* Removable => not async */
				}
				CFRelease(removableRef);
			}

			IOObjectRelease(obj);
		}
		
		IOObjectRelease(iter);
	}
	
	return result;
}


int
main(argc, argv)
	int argc;
	char **argv;
{
	struct msdosfs_args args;
	struct stat sb;
	int c, mntflags, set_gid, set_uid, set_mask;
	mntoptparse_t mp;
	char dev[MAXPATHLEN], mntpath[MAXPATHLEN];
	struct timezone local_tz;
        char *options = "u:g:m:o:";	/* getopt options */
        
	mntflags = set_gid = set_uid = set_mask = 0;
	(void)memset(&args, '\0', sizeof(args));
	args.magic = MSDOSFS_ARGSMAGIC;
	progname = argv[0];

	/*
	 * Parse through the command line options once, in order to find the
	 * block device parameter.  We'll need that to set the default value
	 * for MNT_ASYNC, before we parse the normal options (so MNT_ASYNC can
	 * be overridden either way via the command line).
	 */
	while (getopt(argc, argv, options) != -1) {
	}
	
	if (optind + 2 != argc)
		usage();

	rmslashes(argv[optind], dev);
	/* Check if <dev> is an internal, but not removable, drive; set MNT_ASYNC if so. */
	if (!strncmp(dev, "/dev/disk", 9) && disk_default_async(dev+5))
		mntflags |= MNT_ASYNC;
	
	/*
	 * Now parse the options for real.
	 */
	optreset = 1;
	optind = 1;
	while ((c = getopt(argc, argv, options)) != -1) {
		switch (c) {
		case 'u':
			args.uid = a_uid(optarg);
			set_uid = 1;
			break;
		case 'g':
			args.gid = a_gid(optarg);
			set_gid = 1;
			break;
		case 'm':
			args.mask = a_mask(optarg);
			set_mask = 1;
			break;
		case 'o':
			mp = getmntopts(optarg, mopts, &mntflags, (int *)&args.flags);
			if (mp == NULL)
				err(1, NULL);
			freemntopts(mp);
			break;
		case '?':
		default:
			usage();
			break;
		}
	}

	/*
	 * Resolve the mountpoint with realpath(3) and remove unnecessary
	 * slashes from the devicename if there are any.
	 */
	checkpath(argv[optind + 1], mntpath);

	if (!set_gid || !set_uid || !set_mask) {
		if (stat(mntpath, &sb) == -1)
			err(EX_OSERR, "stat %s", mntpath);

		if (!set_uid)
			args.uid = sb.st_uid;
		if (!set_gid)
			args.gid = sb.st_gid;
		if (!set_mask)
			args.mask = sb.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
		
		/* If NO permisions are given, then set the whole mount to unknown */
		if (!set_gid && !set_uid && !set_mask) {
			args.mask = ACCESSPERMS;
			mntflags |= MNT_UNKNOWNPERMISSIONS;
			}
	}

	args.fspec = dev;
	/* Pass the number of seconds that local time (including DST) is west of GMT */
	gettimeofday(NULL, &local_tz);
	args.secondsWest = local_tz.tz_minuteswest * 60 -
			(local_tz.tz_dsttime ? 3600 : 0);
	args.flags |= MSDOSFSMNT_SECONDSWEST;

	if ((mntflags & MNT_UPDATE) == 0) {
		FindVolumeName(&args);
	}
	
	if (checkLoadable())		/* Is it already loaded? */
		if (load_kmod())		/* Load it in */
			errx(EX_OSERR, "msdos filesystem is not available");

	if (mount("msdos", mntpath, mntflags, &args) < 0)
		err(EX_OSERR, "%s on %s", dev, mntpath);

	exit (0);
}

gid_t
a_gid(s)
	char *s;
{
	struct group *gr;
	char *gname;
	gid_t gid;

	if ((gr = getgrnam(s)) != NULL)
		gid = gr->gr_gid;
	else {
		for (gname = s; *s && isdigit(*s); ++s);
		if (!*s)
			gid = atoi(gname);
		else
			errx(EX_NOUSER, "unknown group id: %s", gname);
	}
	return (gid);
}

uid_t
a_uid(s)
	char *s;
{
	struct passwd *pw;
	char *uname;
	uid_t uid;

	if ((pw = getpwnam(s)) != NULL)
		uid = pw->pw_uid;
	else {
		for (uname = s; *s && isdigit(*s); ++s);
		if (!*s)
			uid = atoi(uname);
		else
			errx(EX_NOUSER, "unknown user id: %s", uname);
	}
	return (uid);
}

mode_t
a_mask(s)
	char *s;
{
	int done;
    mode_t rv;
	char *ep;

	done = 0;
	rv = -1;
	if (*s >= '0' && *s <= '7') {
		done = 1;
		rv = (mode_t)strtol(optarg, &ep, 8);
	}
	if (!done || rv < 0 || *ep)
		errx(EX_USAGE, "invalid file mode: %s", s);
	return (rv);
}

void
usage()
{
	fprintf(stderr, "%s\n%s\n", 
	"usage: mount_msdos [-o options] [-u user] [-g group] [-m mask]",
	"                   [-s] [-l] [-9] bdev dir");
	exit(EX_USAGE);
}


#define FS_TYPE			"msdos"

/* Return non-zero if the file system is not yet loaded. */
static int checkLoadable(void)
{
	int error;
	struct vfsconf vfc;
	
	error = getvfsbyname(FS_TYPE, &vfc);

	return error;
}


#define LOAD_COMMAND "/sbin/kextload"
#define MSDOS_MODULE_PATH "/System/Library/Extensions/msdosfs.kext"


static int load_kmod(void)
{

        int pid;
        int result = -1;
        union wait status;

        pid = fork();
        if (pid == 0) {
                result = execl(LOAD_COMMAND, LOAD_COMMAND, MSDOS_MODULE_PATH,NULL);
                /* We can only get here if the exec failed */
                goto Err_Exit;
        }

        if (pid == -1) {
                result = errno;
                goto Err_Exit;
        }

        /* Success! */
        if ((wait4(pid, (int *)&status, 0, NULL) == pid) && (WIFEXITED(status))) {
                result = status.w_retcode;
        }
        else {
                result = -1;
        }


Err_Exit:

                return (result);
}

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

#define MAX_DOS_BLOCKSIZE	4096

struct dosdirentry {
	u_int8_t name[11];
	u_int8_t attr;
	u_int8_t reserved;
	u_int8_t createTimeTenth;
	u_int16_t createTime;
	u_int16_t createDate;
	u_int16_t accessDate;
	u_int16_t clusterHi;
	u_int16_t modTime;
	u_int16_t modDate;
	u_int16_t clusterLo;
	u_int32_t size;
};
#define ATTR_VOLUME_NAME	0x08
#define ATTR_VOLUME_MASK	0x18
#define ATTR_LONG_NAME		0x0F
#define ATTR_MASK			0x3F

#define SLOT_EMPTY			0x00
#define SLOT_DELETED		0xE5U
#define SLOT_E5				0x05

#define CLUST_FIRST			2
#define CLUST_RESERVED		0x0FFFFFF7

static void FindVolumeName(struct msdosfs_args *args)
{
	int fd;
	u_int32_t i;
	struct dosdirentry *dir;
	void *rootBuffer;
	unsigned bytesPerSector;
	unsigned sectorsPerCluster;
	unsigned rootDirEntries;
	unsigned reservedSectors;
	unsigned numFATs;
	u_int32_t sectorsPerFAT;
	off_t	readOffset;		/* Byte offset of current sector */
	ssize_t	readAmount;
	unsigned char buf[MAX_DOS_BLOCKSIZE];
	char label[12];
	CFStringRef cfstr;

	bzero(label, sizeof(label));	/* Default to no label */
	rootBuffer = NULL;

	fd = open(args->fspec, O_RDONLY, 0);
	if (fd<0)
		err(EX_OSERR, "%s", args->fspec);

	/* Read the boot sector */
	if (pread(fd, buf, MAX_DOS_BLOCKSIZE, 0) != MAX_DOS_BLOCKSIZE)
		err(EX_OSERR, "%s", args->fspec);
	
	/* Check the jump field (first 3 bytes)? */
	
	/* Get the bytes per sector */
	bytesPerSector = buf[11] + buf[12]*256;
	if (bytesPerSector < 512 || bytesPerSector > MAX_DOS_BLOCKSIZE || (bytesPerSector & (bytesPerSector-1)))
		errx(EX_OSERR, "Unsupported sector size (%u)", bytesPerSector);

	/* Get the sectors per cluster */
	sectorsPerCluster = buf[13];
	if (sectorsPerCluster==0 || (sectorsPerCluster & (sectorsPerCluster-1)))
		errx(EX_OSERR, "Unsupported sectors per cluster (%u)", sectorsPerCluster);

	reservedSectors = buf[14] + buf[15]*256;
	numFATs = buf[16];
	
	/* Get the size of the root directory, in sectors */
	rootDirEntries = buf[17] + buf[18]*256;

	/* If there is a label in the boot parameter block, copy it */
	if (rootDirEntries == 0) {
		bcopy(&buf[71], label, 11);
	} else {
		if (buf[38] == 0x29)
			bcopy(&buf[43], label, 11);
	}
	
	/* If there is a label in the root directory, copy it */
	if (rootDirEntries != 0) {
		/* FAT12 or FAT16 */
		u_int32_t	firstRootSector;
		
		sectorsPerFAT = buf[22] + buf[23]*256;
		firstRootSector = reservedSectors + numFATs * sectorsPerFAT;

		readOffset = firstRootSector * bytesPerSector;
		readAmount = (rootDirEntries * sizeof(struct dosdirentry) + bytesPerSector-1) / bytesPerSector;
		readAmount *= bytesPerSector;

		rootBuffer = malloc(readAmount);
		if (rootBuffer == NULL)
			errx(EX_OSERR, "Out of memory");

		/* Read the root directory */
		if (pread(fd, rootBuffer, readAmount, readOffset) != readAmount)
			err(EX_OSERR, "%s", args->fspec);

		/* Loop over root directory entries */
		for (i=0,dir=rootBuffer; i<rootDirEntries; ++i,++dir) {
			if (dir->name[0] == SLOT_EMPTY)
				goto end_of_dir;
			if (dir->name[0] == SLOT_DELETED)
				continue;
			if ((dir->attr & ATTR_MASK) == ATTR_LONG_NAME)
				continue;
			if ((dir->attr & ATTR_VOLUME_MASK) == ATTR_VOLUME_NAME) {
				bcopy(dir->name, label, 11);
				goto end_of_dir;
			}
		}
	} else {
		/* FAT32 */
		u_int32_t	cluster;		/* Current cluster number */
		u_int32_t	clusterOffset;	/* Sector where cluster data starts */

		sectorsPerFAT = buf[36] + (buf[37]<<8L) + (buf[38]<<16L) + (buf[39]<<24L);
		clusterOffset = reservedSectors + numFATs * sectorsPerFAT;

		readAmount = bytesPerSector * sectorsPerCluster;
		rootBuffer = malloc(readAmount);
		if (rootBuffer == NULL)
			errx(EX_OSERR, "Out of memory");
		
		/* Figure out the number of directory entries per cluster */
		rootDirEntries = (unsigned)(readAmount / sizeof(struct dosdirentry));
		
		/* Start with the first cluster of the root directory */
		cluster = buf[44] + (buf[45]<<8L) + (buf[46]<<16L) + (buf[47]<<24L);
		
		/* Loop over clusters in the root directory */
		while (cluster >= CLUST_FIRST && cluster < CLUST_RESERVED) {
			readOffset = (cluster - CLUST_FIRST) * sectorsPerCluster + clusterOffset;
			readOffset *= bytesPerSector;

			/* Read the cluster */
			if (pread(fd, rootBuffer, readAmount, readOffset) != readAmount)
				err(EX_OSERR, "%s", args->fspec);

			/* Loop over every directory entry in the cluster */
			for (i=0,dir=rootBuffer; i<rootDirEntries; ++i,++dir) {
				if (dir->name[0] == SLOT_EMPTY)
					goto end_of_dir;
				if (dir->name[0] == SLOT_DELETED)
					continue;
				if ((dir->attr & ATTR_MASK) == ATTR_LONG_NAME)
					continue;
				if ((dir->attr & ATTR_VOLUME_MASK) == ATTR_VOLUME_NAME) {
					bcopy(dir->name, label, 11);
					goto end_of_dir;
				}
			}

			/* Read the FAT so we can find the next cluster */
			readOffset = reservedSectors + ((cluster * 4) / bytesPerSector);
			readOffset *= bytesPerSector;
			
			if (pread(fd, buf, bytesPerSector, readOffset) != bytesPerSector)
				err(EX_OSERR, "%s", args->fspec);
			
			/* Determine byte offset in FAT sector for "cluster" */
			i = (cluster * 4) % bytesPerSector;
			cluster = buf[i] + (buf[i+1]<<8L) + (buf[i+2]<<16L) + (buf[i+3]<<24L);
			cluster &= 0x0FFFFFFF;	/* Ignore the reserved upper bits */
		}
	}

end_of_dir:
	if (rootBuffer)
		free(rootBuffer);
	close(fd);

	/* Convert a leading 0x05 to 0xE5 for multibyte encodings */
	if (label[0] == 0x05)
		label[0] = 0xE5;

	/* Check for illegal characters */
	if (!oklabel(label))
		label[0] = 0;
	
	/* Remove any trailing spaces. */
	i = 11;
	do {
		--i;
		if (label[i] == ' ')
			label[i] = 0;
		else
			break;
	} while (i != 0);

	/* Convert using default encoding, or Latin1 */
	cfstr = CFStringCreateWithCString(NULL, label, GetDefaultDOSEncoding());
	if (cfstr == NULL)
		cfstr = CFStringCreateWithCString(NULL, label, kCFStringEncodingDOSLatin1);
	if (cfstr == NULL)
		args->label[0] = 0;
	else {
		CFMutableStringRef mutable;
		
		mutable = CFStringCreateMutableCopy(NULL, 0, cfstr);
		if (mutable != NULL) {
			CFStringNormalize(mutable, kCFStringNormalizationFormD);
			CFStringGetCString(mutable, (char *)args->label, sizeof(args->label), kCFStringEncodingUTF8);
			CFRelease(mutable);
		}
		
		CFRelease(cfstr);
	}
	args->flags |= MSDOSFSMNT_LABEL;
}
