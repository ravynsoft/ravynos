/*
 * Copyright (c) 1999-2013 Apple Computer, Inc. All rights reserved.
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

#include <sys/types.h>

#include <sys/attr.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/vnode.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/disk.h>
#include <assert.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <hfs/hfs_mount.h>
#include <hfs/hfs_format.h>

#include <TargetConditionals.h>

/* Sensible wrappers over the byte-swapping routines */
#include "hfs_endian.h"
#if TARGET_OS_OSX  
#include "optical.h"  //only include optical headers on Macs
#endif //osx

#include <mntopts.h>


/*
 * Replay the journal.  We don't care if there are problems.
 */
static void
replay_journal(const char *device)
{
	struct vfsconf vfc;
	int mib[4];
	int fd = -1;

	fd = open(device, O_RDWR);
	if (fd == -1) {
		warn("Could not open block device %s for writing", device);
		goto done;
	}
	if (getvfsbyname("hfs", &vfc) != 0) {
		warn("Could not get hfs vfs information");
		goto done;
	}
	mib[0] = CTL_VFS;
	mib[1] = vfc.vfc_typenum;
	mib[2] = HFS_REPLAY_JOURNAL;
	mib[3] = fd;
	(void)sysctl(mib, 4, NULL, NULL, NULL, 0);

done:
	if (fd != -1)
		close(fd);
	return;
}

struct mntopt mopts[] = {
	MOPT_STDOPTS,
	MOPT_IGNORE_OWNERSHIP,
	MOPT_PERMISSIONS,
	MOPT_UPDATE,
	{ NULL }
};

#define HFS_MOUNT_TYPE				"hfs"

gid_t	a_gid __P((char *));
uid_t	a_uid __P((char *));
mode_t	a_mask __P((char *));
struct hfs_mnt_encoding * a_encoding __P((char *));
int	get_encoding_pref __P((const char *));
int	get_encoding_bias __P((void));
unsigned int  get_default_encoding(void);

void	usage __P((void));


int is_hfs_std = 0;
int wrapper_requested = 0;

typedef struct CreateDateAttrBuf {
    u_int32_t size;
    struct timespec creationTime;
} CreateDateAttrBuf;

#define	HFS_BLOCK_SIZE	512

/*
 *	This is the straight GMT conversion constant:
 *	00:00:00 January 1, 1970 - 00:00:00 January 1, 1904
 *	(3600 * 24 * ((365 * (1970 - 1904)) + (((1970 - 1904) / 4) + 1)))
 */
#define MAC_GMT_FACTOR		2082844800UL

#define KEXT_LOAD_COMMAND	"/sbin/kextload"

#define ENCODING_MODULE_PATH	"/System/Library/Filesystems/hfs.fs/Contents/Resources/Encodings/"

#define MXENCDNAMELEN	16	/* Maximun length of encoding name string */

struct hfs_mnt_encoding {
	char	encoding_name[MXENCDNAMELEN];	/* encoding type name */
	u_int32_t encoding_id;			/* encoding type number */
};


/*
 * Lookup table for hfs encoding names
 * Note: Names must be in alphabetical order
 */
struct hfs_mnt_encoding hfs_mnt_encodinglist[] = {
	{ "Arabic",	          4 },
	{ "Armenian",        24 },
	{ "Bengali",         13 },
	{ "Burmese",         19 },
	{ "Celtic",          39 },
	{ "CentralEurRoman", 29 },
	{ "ChineseSimp",     25 },
	{ "ChineseTrad",      2 },
	{ "Croatian",	     36 },
	{ "Cyrillic",	      7 },
	{ "Devanagari",       9 },
	{ "Ethiopic",        28 },
	{ "Farsi",          140 },
	{ "Gaelic",          40 },
	{ "Georgian",        23 },
	{ "Greek",	          6 },
	{ "Gujarati",        11 },
	{ "Gurmukhi",        10 },
	{ "Hebrew",	          5 },
	{ "Icelandic",	     37 },
	{ "Japanese",	      1 },
	{ "Kannada",         16 },
	{ "Khmer",           20 },
	{ "Korean",	          3 },
	{ "Laotian",         22 },
	{ "Malayalam",       17 },
	{ "Mongolian",       27 },
	{ "Oriya",           12 },
	{ "Roman",	          0 },	/* default */
	{ "Romanian",	     38 },
	{ "Sinhalese",       18 },
	{ "Tamil",           14 },
	{ "Telugu",          15 },
	{ "Thai",	         21 },
	{ "Tibetan",         26 },
	{ "Turkish",	     35 },
	{ "Ukrainian",      152 },
	{ "Vietnamese",      30 },
};


/*
    If path is a path to a block device, then return a path to the
    corresponding raw device.  Else return path unchanged.
*/
const char *rawdevice(const char *path)
{
	const char *devdisk = "/dev/disk";
	static char raw[MAXPATHLEN];

	if (!strncmp(path, devdisk, strlen(devdisk))) {
		/* The +5 below is strlen("/dev/"), so path+5 points to "disk..." */
		int sn_len = snprintf(raw, sizeof(raw), "/dev/r%s", path+5);
		if (sn_len < 0) {
			/* error in building string. return original. */
			return path;
		}

		if ((unsigned long) sn_len < sizeof(raw)) {
			return raw;
		}
	}
	return path;
}


/*
	GetMasterBlock
	
	Return a pointer to the Master Directory Block or Volume Header Block
	for the volume.  In the case of an HFS volume with embedded HFS Plus
	volume, this returns the HFS (wrapper) volume's Master Directory Block.
	That is, the 512 bytes at offset 1024 bytes from the start of the given
	device/partition.
	
	The master block is cached globally.  If it has previously been read in,
	the cached copy will be returned.  If this routine is called multiple times,
	it must be called with the same device string.
	
	Arguments:
		device		Path name to disk device (eg., "/dev/disk0s2")
	
	Returns:
	A pointer to the MDB or VHB.  This pointer may be in the middle of a
	malloc'ed block.  There may be more than 512 bytes of malloc'ed memory
	at the returned address.
	
	Errors:
	On error, this routine returns NULL.
*/
void *GetMasterBlock(const char *device)
{
	static char *masterBlock = NULL;
	char *buf = NULL;
	int err;
	int fd = -1;
	uint32_t blockSize;
	ssize_t amount;
	off_t offset;
	
	/*
	 * If we already read the master block, then just return it.
	 */
	if (masterBlock != NULL) {
		return masterBlock;
	}

	device = rawdevice(device);
	
	fd = open(device, O_RDONLY | O_NDELAY, 0);
	if (fd < 0) {
		fprintf(stderr, "GetMasterBlock: Error %d opening %s\n", errno, device);
		goto done;
	}

	/*
	 * Get the block size so we can read an entire block.
	 */
	err = ioctl(fd, DKIOCGETBLOCKSIZE, &blockSize);
	if (err == -1) {
		fprintf(stderr, "GetMasterBlock: Error %d getting block size\n", errno);
		goto done;
	}

	/*
	 * Figure out the offset of the start of the block which contains
	 * byte offset 1024 (the start of the master block).  This is 1024
	 * rounded down to a multiple of blockSize.  But since blockSize is
	 * always a power of two, this will be either 0 (if blockSize > 1024)
	 * or 1024 (if blockSize <= 1024).
	 */
	offset = blockSize > 1024 ? 0 : 1024;
	
	/*
	 * Allocate a buffer and read the block.
	 */
	buf = malloc(blockSize);
	if (buf == NULL) {
		fprintf(stderr, "GetMasterBlock: Could not malloc %u bytes\n", blockSize);
		goto done;
	}
	amount = pread(fd, buf, blockSize, offset);
	if (amount != blockSize) {
		fprintf(stderr, "GetMasterBlock: Error %d from read; amount=%ld, wanted=%u\n", errno, amount, blockSize);
		goto done;
	}
	
	/*
	 * Point at the part of the buffer containing the master block.
	 * Then return that pointer.
	 *
	 * Note: if blockSize <= 1024, then offset = 1024, and the master
	 * block is at the start of the buffer.  If blockSize > 1024, then
	 * offset = 0, and the master block is at offset 1024 from the start
	 * of the buffer.
	 */
	masterBlock = buf + 1024 - offset;
	buf = NULL;	/* Don't free memory that masterBlock points into. */

done:
	if (fd >= 0)
		close(fd);
	if (buf != NULL)
		free(buf);
	return masterBlock;
}


u_int32_t getVolumeCreateDate(const char *device)
{
	HFSMasterDirectoryBlock * mdbPtr;
	u_int32_t volume_create_time = 0;

	mdbPtr = GetMasterBlock(device);
	if (mdbPtr == NULL) goto exit;
	
	/* get the create date from the MDB (embedded case) or Volume Header */
	if ((mdbPtr->drSigWord == SWAP_BE16 (kHFSSigWord)) &&
	    (mdbPtr->drEmbedSigWord == SWAP_BE16 (kHFSPlusSigWord))) {
		/* Embedded volume*/
		volume_create_time = SWAP_BE32 (mdbPtr->drCrDate);
		
	} else if (mdbPtr->drSigWord == kHFSPlusSigWord ) {
		HFSPlusVolumeHeader * volHdrPtr = (HFSPlusVolumeHeader *) mdbPtr;

		volume_create_time = SWAP_BE32 (volHdrPtr->createDate);
	} else {
		goto exit;	/* cound not match signature */
	}

	if (volume_create_time > MAC_GMT_FACTOR)
		volume_create_time -= MAC_GMT_FACTOR;
	else
		volume_create_time = 0;	/* don't let date go negative! */

exit:
	return volume_create_time;
}

void syncCreateDate(const char *mntpt, u_int32_t localCreateTime)
{
	int result;
	char path[256];
	struct attrlist	attributes;
	CreateDateAttrBuf attrReturnBuffer;
	int64_t gmtCreateTime;
	int32_t gmtOffset;
	int32_t newCreateTime;

	snprintf(path, sizeof(path), "%s/", mntpt);

	attributes.bitmapcount	= ATTR_BIT_MAP_COUNT;
	attributes.reserved		= 0;
	attributes.commonattr	= ATTR_CMN_CRTIME;
	attributes.volattr 		= 0;
	attributes.dirattr 		= 0;
	attributes.fileattr 	= 0;
	attributes.forkattr 	= 0;

	result = getattrlist(path, &attributes, &attrReturnBuffer, sizeof(attrReturnBuffer), 0 );
	if (result) return;

	gmtCreateTime = attrReturnBuffer.creationTime.tv_sec;
	gmtOffset = (int32_t)(gmtCreateTime - (int64_t) localCreateTime + 900);
	if (gmtOffset > 0) {
		gmtOffset = 1800 * (gmtOffset / 1800);
	} else {
		gmtOffset = -1800 * ((-gmtOffset + 1799) / 1800);
	}
	
	newCreateTime = localCreateTime + gmtOffset;

	/*
	 * if the root directory's create date doesn't match
	 * and its within +/- 15 seconds, then update it
	 */
	if ((newCreateTime != attrReturnBuffer.creationTime.tv_sec) &&
		(( newCreateTime - attrReturnBuffer.creationTime.tv_sec) > -15) &&
		((newCreateTime - attrReturnBuffer.creationTime.tv_sec) < 15)) {

		attrReturnBuffer.creationTime.tv_sec = (time_t) newCreateTime;
		(void) setattrlist (path,
				    &attributes,
				    &attrReturnBuffer.creationTime,
				    sizeof(attrReturnBuffer.creationTime),
				    0);
	}
}

/*
 * load_encoding
 * loads an hfs encoding converter module into the kernel
 *
 * Note: unloading of encoding converter modules is done in the kernel
 */
static int
load_encoding(struct hfs_mnt_encoding *encp)
{
	int pid;
	int loaded;
	union wait status;
	struct stat sb;
	char kmodfile[MAXPATHLEN];
	
	/* MacRoman encoding (0) is built into the kernel */
	if (encp->encoding_id == 0)
		return (0);

	snprintf(kmodfile, sizeof(kmodfile), "%sHFS_Mac%s.kext", ENCODING_MODULE_PATH, encp->encoding_name);
	if (stat(kmodfile, &sb) == -1) {
		fprintf(stdout, "unable to find: %s\n", kmodfile);
		return (-1);
	}

	loaded = 0;
	pid = fork();
	if (pid == 0) {
		(void) execl(KEXT_LOAD_COMMAND, KEXT_LOAD_COMMAND, kmodfile, NULL);

		exit(1);	/* We can only get here if the exec failed */
	} else if (pid != -1) {
		if ((waitpid(pid, (int *)&status, 0) == pid) && WIFEXITED(status)) {
			/* we attempted a load */
			loaded = 1;
		}
	}

	if (!loaded) {
		fprintf(stderr, "unable to load: %s\n", kmodfile);
		return (-1);
	}
	return (0);
}

int
main(argc, argv)
	int argc;
	char **argv;
{
	struct hfs_mount_args args;
	int ch, mntflags;
	char *dev, dir[MAXPATHLEN];
	int mountStatus;
	struct timeval dummy_timeval; /* gettimeofday() crashes if the first argument is NULL */
	u_int32_t localCreateTime;
	struct hfs_mnt_encoding *encp;

	int do_rekey = 0;
	int tmp_mntflags = 0;

#if TARGET_OS_IPHONE 
	mntflags = MNT_NOATIME;
#else // !TARGET_OS_IPHONE
	mntflags = 0;
#endif // TARGET_OS_IPHONE

	encp = NULL;
	(void)memset(&args, '\0', sizeof(struct hfs_mount_args));

   	/*
   	 * For a mount update, the following args must be explictly
   	 * passed in as options to change their value.  On a new
   	 * mount, default values will be computed for all args.
   	 */
	args.flags = VNOVAL;
	args.hfs_uid = (uid_t)VNOVAL;
	args.hfs_gid = (gid_t)VNOVAL;
	args.hfs_mask = (mode_t)VNOVAL;
	args.hfs_encoding = (u_int32_t)VNOVAL;
	
	optind = optreset = 1;		/* Reset for parse of new argv. */
	while ((ch = getopt(argc, argv, "xu:g:m:e:o:wt:jc")) != EOF) {
		switch (ch) {
		case 't': {
			char *ptr;
			unsigned long tbufsize = strtoul(optarg, &ptr, 0);
			if (tbufsize >= UINT_MAX) {
				tbufsize = UINT_MAX;
			}
			args.journal_tbuffer_size = (unsigned int) strtoul(optarg, &ptr, 0);
			if ((args.journal_tbuffer_size == 0 || 
						((uint32_t) args.journal_tbuffer_size) == UINT_MAX) && errno != 0) {
				fprintf(stderr, "%s: Invalid tbuffer size %s\n", argv[0], optarg);
				exit(5);
			} else {
				if (*ptr == 'k')
					args.journal_tbuffer_size *= 1024;
				else if (*ptr == 'm')
					args.journal_tbuffer_size *= 1024*1024;
			}
			if (args.flags == VNOVAL){
				args.flags = 0;
			}	
			args.flags |= HFSFSMNT_EXTENDED_ARGS;
			break;
		}
		case 'j':
			/* disable the journal */
			if(args.flags == VNOVAL){
				args.flags = 0;
			}
			args.flags |= HFSFSMNT_EXTENDED_ARGS;
			args.journal_disable = 1;
			break;
		case 'c':
			// XXXdbg JOURNAL_NO_GROUP_COMMIT == 0x0001
			args.journal_flags = 0x0001;
			break;
		case 'x':
			if (args.flags == VNOVAL)
				args.flags = 0;
			args.flags |= HFSFSMNT_NOXONFILES;
			break;
		case 'u':
			args.hfs_uid = a_uid(optarg);
			break;
		case 'g':
			args.hfs_gid = a_gid(optarg);
			break;
		case 'm':
			args.hfs_mask = a_mask(optarg);
			break;
		case 'e':
			encp = a_encoding(optarg);
			break;
		case 'o':
			{
				int dummy;
				getmntopts(optarg, mopts, &mntflags, &dummy);
			}
			break;
		case 'w':
			if (args.flags == VNOVAL)
				args.flags = 0;
			args.flags |= HFSFSMNT_WRAPPER;
			wrapper_requested = 1;
			break;
		case '?':
			usage();
			break;
		default:
#if DEBUG
			printf("mount_hfs: ERROR: unrecognized ch = '%c'\n", ch);
#endif
			usage();
		}; /* switch */
	}
    
	if ((mntflags & MNT_IGNORE_OWNERSHIP) && !(mntflags & MNT_UPDATE)) {
		/*
		 * The defaults to be supplied in lieu of the on-disk permissions
		 * (could be overridden by explicit -u, -g, or -m options):
		 */
		if (args.hfs_uid == (uid_t)VNOVAL) args.hfs_uid = UNKNOWNUID;
		if (args.hfs_gid == (gid_t)VNOVAL) args.hfs_gid = UNKNOWNGID;
#if OVERRIDE_UNKNOWN_PERMISSIONS
		if (args.hfs_mask == (mode_t)VNOVAL) args.hfs_mask = ACCESSPERMS;  /* 0777 */
#endif
	}
	argc -= optind;
	argv += optind;

	if (argc != 2) {
#if DEBUG
		printf("mount_hfs: ERROR: argc == %d != 2\n", argc);
#endif
		usage();
	}

	dev = argv[0];

	if (realpath(argv[1], dir) == NULL)
		err(1, "realpath %s", dir);

	args.fspec = dev;

	/* HFS volumes need timezone info to convert local to GMT */
	(void) gettimeofday( &dummy_timeval, &args.hfs_timezone );

	/* load requested encoding (if any) for hfs volume */
	if (encp != NULL) {
		if (load_encoding(encp) != 0)
			exit(1);  /* load failure */
		args.hfs_encoding = encp->encoding_id;
	}
	
	/*
	 * For a new mount (non-update case) fill in default values for all args
	 */
	if ((mntflags & MNT_UPDATE) == 0) {

		struct stat sb;

		if (args.flags == VNOVAL)
			args.flags = 0;

		if ((args.hfs_encoding == (u_int32_t)VNOVAL) && (encp == NULL)) {
			int encoding;

			/* Find a suitable encoding preference. */
			if ((encoding = get_encoding_pref(dev)) != -1) {
				/* 
				 * Note: the encoding kext was loaded by
				 * hfs.util during the file system probe.
				 */
				args.hfs_encoding = encoding;
			} else {
				args.hfs_encoding = 0;
			}
		}
		/* when the mountpoint is root, use default values */
		if (strcmp(dir, "/") == 0) {
			sb.st_mode = 0777;
			sb.st_uid = 0;
			sb.st_gid = 0;
			
		/* otherwise inherit from the mountpoint */
		} else if (stat(dir, &sb) == -1)
			err(1, "stat %s", dir);

		if (args.hfs_uid == (uid_t)VNOVAL)
			args.hfs_uid = sb.st_uid;

		if (args.hfs_gid == (gid_t)VNOVAL)
			args.hfs_gid = sb.st_gid;

		if (args.hfs_mask == (mode_t)VNOVAL)
			args.hfs_mask = sb.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
	}

#if DEBUG
    printf("mount_hfs: calling mount: \n" );
    printf("\tdevice = %s\n", dev);
    printf("\tmount point = %s\n", dir);
    printf("\tmount flags = 0x%08x\n", mntflags);
    printf("\targ flags = 0x%x\n", args.flags);
    printf("\tuid = %d\n", args.hfs_uid);
    printf("\tgid = %d\n", args.hfs_gid);
    printf("\tmode = %o\n", args.hfs_mask);
    printf("\tencoding = %ld\n", args.hfs_encoding);

#endif

#if TARGET_OS_OSX
    /*
     * We shouldn't really be calling up to other layers, but
     * an exception was made in this case to fix the situation
     * where HFS was writable on optical media.
     */
    
    if ((_optical_is_writable(dev) & _OPTICAL_WRITABLE_PACKET)) {
	    mntflags |= MNT_RDONLY;
    }
#endif
	
    if (is_hfs_std)
	    mntflags |= MNT_RDONLY;
    
    if ((mntflags & MNT_RDONLY) == 0) {
	    /*
	     * get the volume's create date so we can synchronize
	     * it with the root directory create date
	     */
	    localCreateTime = getVolumeCreateDate(dev);
    }
    else {
	    localCreateTime = 0;
    }
    
    if ((mountStatus = mount(HFS_MOUNT_TYPE, dir, mntflags, &args)) < 0) {
	    printf("mount_hfs: error on mount(): error = %d.\n", mountStatus);
	    err(1, NULL);
    };
    
    /*
     * synchronize the root directory's create date
     * with the volume's create date
     */
    if (localCreateTime)
	    syncCreateDate(dir, localCreateTime);
    
    
    exit(0);
}


gid_t
a_gid(s)
    char *s;
{
    struct group *gr;
    char *gname, *orig = s;
    gid_t gid = 0;

    if (*s == '-')
	s++;
    for (gname = s; *s && isdigit(*s); ++s);
    if (!*s) {
	gid = atoi(gname);
    } else {
	gr = getgrnam(orig);
	if (gr == NULL)
	    errx(1, "unknown group id: %s", orig);
	gid = gr->gr_gid;
    }
    return (gid);
}

uid_t
a_uid(s)
    char *s;
{
    struct passwd *pw;
    char *uname, *orig = s;
    uid_t uid = 0;

    if (*s == '-')
	s++;
    for (uname = s; *s && isdigit(*s); ++s);
    if (!*s) {
	uid = atoi(uname);
    } else {
	pw = getpwnam(orig);
	if (pw == NULL)
	    errx(1, "unknown user id: %s", orig);
	uid = pw->pw_uid;
    }
    return (uid);
}

mode_t
a_mask(s)
    char *s;
{
    int done, rv;
    char *ep;

    done = 0;
    rv = -1;
    if (*s >= '0' && *s <= '7') {
        long mask;

        done = 1;
        mask = strtol(optarg, &ep, 8);
        if (mask >= 0 && mask <= INT_MAX)
            rv = (int)mask;
    }
    if (!done || rv < 0 || *ep)
        errx(1, "invalid file mode: %s", s);
    return (rv);
}

struct hfs_mnt_encoding *
a_encoding(s)
	char *s;
{
	char *uname;
	int i;
	u_int32_t encoding;
	struct hfs_mnt_encoding *p, *q, *enclist;
	int elements = sizeof(hfs_mnt_encodinglist) / sizeof(struct hfs_mnt_encoding);
	int compare;

	/* Use a binary search to find an encoding match */
	p = hfs_mnt_encodinglist;
	q = p + (elements - 1);
	while (p <= q) {
		enclist = p + ((q - p) >> 1);	/* divide by 2 */
		compare = strcmp(s, enclist->encoding_name);
		if (compare < 0)
			q = enclist - 1;
		else if (compare > 0)
			p = enclist + 1;
		else
			return (enclist);
	}

	for (uname = s; *s && isdigit(*s); ++s);

	if (*s) goto unknown;

	encoding = atoi(uname);
	for (i=0, enclist = hfs_mnt_encodinglist; i < elements; i++, enclist++) {
		if (enclist->encoding_id == encoding)
			return (enclist);
	}

unknown:
	errx(1, "unknown encoding: %s", uname);
	return (NULL);
}


/*
 * Get file system's encoding preference.
 */
int
get_encoding_pref(const char *device)
{
	struct hfs_mnt_encoding *enclist;
	HFSMasterDirectoryBlock * mdbp;
	int encoding = -1;
	int elements;
	int i;

	mdbp = GetMasterBlock(device);
	if (mdbp == NULL)
		return 0;

	if (SWAP_BE16(mdbp->drSigWord) != kHFSSigWord ||
	    (SWAP_BE16(mdbp->drEmbedSigWord) == kHFSPlusSigWord && (!wrapper_requested))) {
		return (-1);
	}
	else {
		is_hfs_std = 1;
	}
	encoding = GET_HFS_TEXT_ENCODING(SWAP_BE32(mdbp->drFndrInfo[4]));

	if (encoding == -1) {
		encoding = get_encoding_bias();
		if (encoding == 0 || encoding == -1)
			encoding = get_default_encoding();
	}

	/* Check if this is a supported encoding. */
	elements = sizeof(hfs_mnt_encodinglist) / sizeof(struct hfs_mnt_encoding);
	for (i=0, enclist = hfs_mnt_encodinglist; i < elements; i++, enclist++) {
		if (enclist->encoding_id == encoding)
			return (encoding);
	}

	return (0);
}

/*
 * Get kernel's encoding bias.
 */
int
get_encoding_bias()
{
        int mib[3];
        size_t buflen = sizeof(int);
        struct vfsconf vfc;
        int hint = 0;

        if (getvfsbyname("hfs", &vfc) < 0)
		goto error;

        mib[0] = CTL_VFS;
        mib[1] = vfc.vfc_typenum;
        mib[2] = HFS_ENCODINGBIAS;
 
	if (sysctl(mib, 3, &hint, &buflen, NULL, 0) < 0)
 		goto error;
	return (hint);
error:
	return (-1);
}

#define __kCFUserEncodingFileName ("/.CFUserTextEncoding")

unsigned int
get_default_encoding()
{
	struct passwd *passwdp;

	if ((passwdp = getpwuid(0))) {	/* root account */
		char buffer[MAXPATHLEN + 1];
		int fd;

		strlcpy(buffer, passwdp->pw_dir, sizeof(buffer));
		strlcat(buffer, __kCFUserEncodingFileName, sizeof(buffer));

		if ((fd = open(buffer, O_RDONLY, 0)) > 0) {
			ssize_t readSize;
            long encoding;

			readSize = read(fd, buffer, MAXPATHLEN);
			buffer[(readSize < 0 ? 0 : readSize)] = '\0';
			close(fd);
            encoding = strtol(buffer, NULL, 0);
            assert(encoding > -1 && encoding <= UINT_MAX);
			return (unsigned int)encoding;
		}
	}
	return (0);	/* Fallback to smRoman */
}


void
usage()
{
	(void)fprintf(stderr,
               "usage: mount_hfs [-xw] [-u user] [-g group] [-m mask] [-e encoding] [-t tbuffer-size] [-j] [-c] [-o options] special-device filesystem-node\n");
	
	exit(1);
}
