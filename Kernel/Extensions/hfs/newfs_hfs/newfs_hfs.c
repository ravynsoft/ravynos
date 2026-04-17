/*
 * Copyright (c) 1999-2015 Apple Inc. All rights reserved.
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


#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <paths.h>
#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <syslog.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/stat.h>

#include <IOKit/storage/IOMediaBSDClient.h>

#include <hfs/hfs_format.h>
#include "newfs_hfs.h"

#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#define	NOVAL       (-1)
#define UMASK       (0755)
#define	ACCESSMASK  (0777)

/*
 * The maximum HFS volume size is calculated thusly:
 * 
 * The maximum allocation block size (which must be a power of 2 value),
 * is 2GB, or 2^31 bytes
 *
 * The maximum number of allocation blocks is 2^32 -1.  
 *
 * Multiplying that out yields 2GB * ( 4GB - 1 ) == 2GB*4GB  - 2GB.
 * More explicitly, 8 exabytes - 2 gigabytes,
 * or 0x7FFFFFFF80000000 bytes.  That gives us our value below.
 */

#define MAXHFSVOLSIZE (0x7FFFFFFF80000000ULL)

#define ROUNDUP(x,y) (((x)+(y)-1)/(y)*(y))

static void getnodeopts __P((char* optlist));
static void getinitialopts __P((char* optlist));
static void getclumpopts __P((char* optlist));
#ifdef DEBUG_BUILD
static void getstartopts __P((char *optlist));
static void getextsopts __P((char* optlist));
#endif
static gid_t a_gid __P((char *));
static uid_t a_uid __P((char *));
static mode_t a_mask __P((char *));
static int hfs_newfs __P((char *device));
static void validate_hfsplus_block_size __P((UInt64 sectorCount, UInt32 sectorSize));
static void hfsplus_params __P((const DriveInfo* dip, hfsparams_t *defaults));
static UInt32 initialsizecalc __P((UInt32 initialblocks));
static UInt32 clumpsizecalc __P((UInt32 clumpblocks));
static UInt32 CalcHFSPlusBTreeClumpSize __P((UInt32 blockSize, UInt32 nodeSize, UInt64 sectors, int fileID));
static void usage __P((void));
static int get_high_bit (u_int64_t bitstring);
static int bad_disk_size (u_int64_t numsectors, u_int64_t sectorsize);



char	*progname;
char	*gVolumeName = kDefaultVolumeNameStr;
char	rawdevice[MAXPATHLEN];
char	blkdevice[MAXPATHLEN];
uint32_t gBlockSize = 0;
UInt32	gNextCNID = kHFSFirstUserCatalogNodeID;

time_t  createtime;

int	gNoCreate = FALSE;
int	gUserCatNodeSize = FALSE;
int	gCaseSensitive = FALSE;
int	gUserAttrSize = FALSE;
int	gUserAttrInitialSize = FALSE;
int	gUserCatInitialSize = FALSE;
int	gUserExtInitialSize = FALSE;
int gContentProtect = FALSE;

static UInt32	attrExtCount = 1, blkallocExtCount = 1, catExtCount = 1, extExtCount = 1;
static UInt32	attrExtStart = 0, blkallocExtStart = 0, catExtStart = 0, extExtStart = 0;
static UInt32	jibStart = 0, jnlStart = 0, allocStart = 0;

#ifdef DEBUG_BUILD
uint16_t gProtectLevel = 0;
#endif

#define JOURNAL_DEFAULT_SIZE (8*1024*1024)
int     gJournaled = FALSE;
char    *gJournalDevice = NULL;
UInt64	gJournalSize = 0;

uid_t	gUserID = (uid_t)NOVAL;
gid_t	gGroupID = (gid_t)NOVAL;
mode_t	gModeMask = (mode_t)NOVAL;

/* Starting allocation block number for the file system, 
 * all btrees, including journal will be laid down at this 
 * alloation block offset. 
 */ 
UInt32	gFSStartBlock = 0;

UInt64	gPartitionSize = 0;

UInt32	catnodesiz = 8192;
UInt32	extnodesiz = 4096;
UInt32	atrnodesiz = 8192;

UInt32	catinitialblks = 0;
UInt32	extinitialblks = 0;
UInt32	atrinitialblks = 0;

UInt32	catclumpblks = 0;
UInt32	extclumpblks = 0;
UInt32	atrclumpblks = 0;
UInt32	bmclumpblks = 0;
UInt32	rsrclumpblks = 0;
UInt32	datclumpblks = 0;
uint32_t hfsgrowblks = 0;      /* maximum growable size of wrapper */


UInt64
get_num(char *str)
{
    UInt64 num;
    char *ptr;

    num = strtoull(str, &ptr, 0);

    if (*ptr) {
	    char scale = tolower(*ptr);

	    switch(scale) {
	    case 'b':
		    num *= 512ULL;
		    break;
	    case 'p':
		    num *= 1024ULL;
		    /* fall through */
	    case 't':
		    num *= 1024ULL;
		    /* fall through */
	    case 'g':
		    num *= 1024ULL;
		    /* fall through */
	    case 'm':
		    num *= 1024ULL;
		    /* fall through */
	    case 'k':
		    num *= 1024ULL;
		    break;

	    default:
		    num = 0ULL;
		    break;
	}
    }
    return num;
}


int
main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
	extern int optind;
	int ch;
	char *cp, *special;
	struct statfs *mp;
	int n;
	
	if ((progname = strrchr(*argv, '/')))
		++progname;
	else
		progname = *argv;

// No semicolon at end of line deliberately!

	static const char *options = "G:J:D:M:N:PU:hsb:c:i:I:n:v:"
#ifdef DEBUG_BUILD
		"p:a:E:"
#endif
		;

	while ((ch = getopt(argc, argv, options)) != -1)
		switch (ch) {
		case 'G':
			gGroupID = a_gid(optarg);
			break;

		case 'J':
			gJournaled = TRUE;
			if (isdigit(optarg[0])) {
			    gJournalSize = get_num(optarg);
			    if (gJournalSize < 512*1024) {
					printf("%s: journal size %lldk too small.  Reset to %dk.\n",
						progname, gJournalSize/1024, JOURNAL_DEFAULT_SIZE/1024);
					gJournalSize = JOURNAL_DEFAULT_SIZE;
			    }
			} else {
				/* back up because there was no size argument */
			    optind--;
			}
			break;
			
		case 'D':
			gJournalDevice = (char *)optarg;
			break;
			
		case 'N':
			gNoCreate = TRUE;
			if (isdigit(optarg[0])) {
				gPartitionSize = get_num(optarg);
			} else {
				/* back up because there was no size argument */
				optind--;
			}
			break;

		case 'P':
			gContentProtect = TRUE;
			break;

#ifdef DEBUG_BUILD
		case 'p':
			if (isdigit (optarg[0])) {
				uint64_t level = get_num (optarg);
				gProtectLevel  = (uint16_t) level;
			}		
			else {
				/* back up because no level was provided */
				optind--;
			}
			break;
#endif

		case 'M':
			gModeMask = a_mask(optarg);
			break;

		case 'U':
			gUserID = a_uid(optarg);
			break;

#ifdef DEBUG_BUILD
		case 'a':
			getstartopts(optarg);
			break;
#endif

#ifdef DEBUG_BUILD
		case 'E':
			getextsopts(optarg);
			break;
#endif
		case 'b':
		{
			UInt64 tempBlockSize;
			
			tempBlockSize = get_num(optarg);
			if (tempBlockSize < HFSMINBSIZE)
				fatal("%s: bad allocation block size (too small)", optarg);
			if (tempBlockSize > HFSMAXBSIZE) 
				fatal("%s: bad allocation block size (too large)", optarg);
			gBlockSize = (uint32_t)tempBlockSize;
			break;
		}

		case 'c':
			getclumpopts(optarg);
			break;

		case 'i':
			gNextCNID = atoi(optarg);
			/*
			 * make sure its at least kHFSFirstUserCatalogNodeID
			 */
			if (gNextCNID < kHFSFirstUserCatalogNodeID)
				fatal("%s: starting catalog node id too small (must be > 15)", optarg);
			break;

		case 'I':
			getinitialopts(optarg);
			break;

		case 'n':
			getnodeopts(optarg);
			break;

		case 's':
			gCaseSensitive = TRUE;
			break;

		case 'v':
			if ((gVolumeName = strdup(optarg)) == NULL) {
				fatal("Could not copy volume name %s", optarg);
			}
			break;

		case '?':
		default:
			usage();
		}

	argc -= optind;
	argv += optind;

#ifdef DEBUG_BUILD
	if ((gProtectLevel) && !(gContentProtect)) {
		fatal ("content protection must be specified to set a protection level");
	}
#endif

	if (gPartitionSize != 0) {
		/*
		 * If we are given -N, a size, and a device, that's a usage error.
		 */
		if (argc != 0)
			usage();

		rawdevice[0] = blkdevice[0] = 0;
	} else {
		if (argc != 1)
			usage();

		special = argv[0];
		cp = strrchr(special, '/');
		if (cp != 0)
			special = cp + 1;
		if (*special == 'r')
			special++;
		(void) snprintf(rawdevice, sizeof(rawdevice), "%sr%s", _PATH_DEV, special);
		(void) snprintf(blkdevice, sizeof(blkdevice), "%s%s", _PATH_DEV, special);
	}

	if (gPartitionSize == 0) {
		/*
		 * Check if target device is aready mounted
		 */
		n = getmntinfo(&mp, MNT_NOWAIT);
		if (n == 0)
			fatal("%s: getmntinfo: %s", blkdevice, strerror(errno));

		while (--n >= 0) {
			if (strcmp(blkdevice, mp->f_mntfromname) == 0)
				fatal("%s is mounted on %s", blkdevice, mp->f_mntonname);
			++mp;
		}
	}

	if (hfs_newfs(rawdevice) < 0) {
		err(1, "cannot create filesystem on %s", rawdevice);
	}

	exit(0);
}


static void getnodeopts(char* optlist)
{
	char *strp = optlist;
	char *ndarg;
	char *p;
	UInt32 ndsize;
	
	while((ndarg = strsep(&strp, ",")) != NULL && *ndarg != '\0') {

		p = strchr(ndarg, '=');
		if (p == NULL)
			usage();
	
		ndsize = atoi(p+1);

		switch (*ndarg) {
		case 'c':
			if (ndsize < 4096 || ndsize > 32768 || (ndsize & (ndsize-1)) != 0)
				fatal("%s: invalid catalog b-tree node size", ndarg);
			catnodesiz = ndsize;
			gUserCatNodeSize = TRUE;
			break;

		case 'e':
			if (ndsize < 1024 || ndsize > 32768 || (ndsize & (ndsize-1)) != 0)
				fatal("%s: invalid extents b-tree node size", ndarg);
			extnodesiz = ndsize;
			break;

		case 'a':
			if (ndsize < 4096 || ndsize > 32768 || (ndsize & (ndsize-1)) != 0)
				fatal("%s: invalid atrribute b-tree node size", ndarg);
			atrnodesiz = ndsize;
			break;

		default:
			usage();
		}
	}
}


static void getinitialopts(char* optlist)
{
	char *strp = optlist;
	char *ndarg;
	char *p;
	UInt32 inblocks;
	
	while((ndarg = strsep(&strp, ",")) != NULL && *ndarg != '\0') {

		p = strchr(ndarg, '=');
		if (p == NULL)
			usage();
			
		inblocks = atoi(p+1);
		
		switch (*ndarg) {
		case 'a':
			atrinitialblks = inblocks;
			gUserAttrInitialSize = TRUE;
			gUserAttrSize = TRUE;
			break;
		case 'c':
			catinitialblks = inblocks;
			gUserCatInitialSize = TRUE;
			break;
		case 'e':
			extinitialblks = inblocks;
			gUserExtInitialSize = TRUE;
			break;

		default:
			usage();
		}
	}
}


static void getclumpopts(char* optlist)
{
	char *strp = optlist;
	char *ndarg;
	char *p;
	UInt32 clpblocks;
	
	while((ndarg = strsep(&strp, ",")) != NULL && *ndarg != '\0') {

		p = strchr(ndarg, '=');
		if (p == NULL)
			usage();
			
		clpblocks = atoi(p+1);
		
		switch (*ndarg) {
		case 'a':
			atrclumpblks = clpblocks;
			if (!gUserAttrInitialSize) {
				atrinitialblks = atrclumpblks;
			}
			gUserAttrSize = TRUE;
			break;
		case 'b':
			bmclumpblks = clpblocks;
			break;
		case 'c':
			catclumpblks = clpblocks;
			if (!gUserCatInitialSize) {
				catinitialblks = catclumpblks;
			}
			break;
		case 'd':
			datclumpblks = clpblocks;
			break;
		case 'e':
			extclumpblks = clpblocks;
			if (!gUserExtInitialSize) {
				extinitialblks = extclumpblks;
			}
			break;
		case 'r':
			rsrclumpblks = clpblocks;
			break;

		default:
			usage();
		}
	}
}

#ifdef DEBUG_BUILD
static void getextsopts(char* optlist)
{
	char *strp = optlist;
	char *ndarg;
	char *p;
	UInt32 numexts;
	
	while((ndarg = strsep(&strp, ",")) != NULL && *ndarg != '\0') {

		p = strchr(ndarg, '=');
		if (p == NULL)
			usage();
			
		numexts = atoi(p+1);
		
		switch (*ndarg) {
		case 'a':
			attrExtCount = numexts;
			break;
		case 'b':
			blkallocExtCount = numexts;
			break;
		case 'c':
			catExtCount = numexts;
			break;
		case 'e':
			extExtCount = numexts;
			break;
		default:
			usage();
		}
	}
}

static void getstartopts(char* optlist)
{
	char *strp;
	char *ndarg;
	char *p;
	unsigned long startat = 0;
    int pos;
    UInt32 upos;

	startat = strtoul(optlist, &strp, 0);
	if (startat == ULONG_MAX && errno != 0) {
		err(1, "invalid allocation start block string %s", optlist);
	}
	if (startat > UINT_MAX) {
		errx(1, "Allocation block %lu larger than max", startat);
	}
	if (strp && *strp == ',')
		strp++;

	gFSStartBlock = (UInt32)startat;

	while((ndarg = strsep(&strp, ",")) != NULL && *ndarg != '\0') {

		startat = strtoul(optlist, NULL, 0);
		p = strchr(ndarg, '=');
		if (p == NULL)
			usage();
			
		pos = atoi(p+1);
        if (pos < 0 || pos > UINT32_MAX) {
            errx(1, "pos=%d is invalid", pos);
        }
        upos = (UInt32)pos;

		switch (*ndarg) {
		case 'a':
			attrExtStart = upos;
			break;
		case 'b':
			blkallocExtStart = upos;
			break;
		case 'c':
			catExtStart = upos;
			break;
		case 'e':
			extExtStart = upos;
			break;
		case 'j':
			jibStart = upos;
			break;
		case 'J':
			jnlStart = upos;
			break;
		case 'N':
			allocStart = upos;
			break;
		default:
			usage();
		}
	}
}
#endif

gid_t
static a_gid(char *s)
{
	struct group *gr;
	char *gname;
	gid_t gid = 0;

	if ((gr = getgrnam(s)) != NULL)
		gid = gr->gr_gid;
	else {
		for (gname = s; *s && isdigit(*s); ++s);
		if (!*s)
			gid = atoi(gname);
		else
			errx(1, "unknown group id: %s", gname);
	}
	return (gid);
}

static uid_t
a_uid(char *s)
{
	struct passwd *pw;
	char *uname;
	uid_t uid = 0;

	if ((pw = getpwnam(s)) != NULL)
		uid = pw->pw_uid;
	else {
		for (uname = s; *s && isdigit(*s); ++s);
		if (!*s)
			uid = atoi(uname);
		else
			errx(1, "unknown user id: %s", uname);
	}
	return (uid);
}

static mode_t
a_mask(char *s)
{
	int done;
    long rv;
	char *ep;

	done = 0;
	rv = -1;
	if (*s >= '0' && *s <= '7') {
		done = 1;
		rv = strtol(s, &ep, 8);
	}
	if (!done || rv < 0 || rv > INT_MAX || *ep)
		errx(1, "invalid access mask: %s", s);
	return ((int)rv);
}

/*
 * Check to see if the volume is too big.
 *
 * Returns:
 *		0 if it is appropriately sized.
 *		1 if HFS+ cannot be formatted onto the disk.
 */

static int bad_disk_size (u_int64_t numsectors, u_int64_t sectorsize) {
	
	u_int32_t maxSectorBits = 0;
	u_int32_t maxSectorSizeBits = 0;
	u_int32_t maxBits = 0;
	u_int64_t bytes;
	
	/*
	 * The essential problem here is that we cannot simply multiply the sector size by the
	 * number of sectors because the product could overflow a 64 bit integer.  We do a cursory 
	 * check and then a longer check once we know the product will not overflow.
	 */ 	
	
	maxSectorBits = get_high_bit (numsectors);
	maxSectorSizeBits = get_high_bit (sectorsize);
	
	/*
	 * We get the number of bits to represent the number of sectors and the sector size.  
	 * Adding the two numbers gives us the number of bits required to represent the product.
	 * If the product is > 63 then it must be too big. 
	 */
	
	maxBits = maxSectorBits + maxSectorSizeBits;
	if (maxBits > 63) {
		return 1;
	}
	
	/* Well, now we know that the two values won't overflow.  Time to multiply */
	bytes = numsectors * sectorsize;
	
	if (bytes > MAXHFSVOLSIZE) {
		/* Too big! */
		return 1;
	}
	
	/* Otherwise, it looks good */
	return 0;
	
}

/* 
 * The allocation block size must be defined as a power of 2 value, with a floor of
 * 512 bytes.  However, we never default to anything less than 4096 bytes, so that
 * gives us 20 block size values from 4kb -> 2GB block size.
 *
 * See inline comments for how this table is used to determine the minimum fs size that
 * will use a specified allocation block size.  
 *
 * The growth boundary is used to figure out if we need a bigger block size than the
 * 4 KB default.  We get the index of the highest bit set in the FS size, then subtract the
 * growth boundary to index into the block allocation size array.
 *
 * Note that 8K appears twice in table since we want to use it for the range 2 TB < 8 TB FS size.
 * This means that when the 2TB bit or the 4TB bit is the high bit set, we prefer the 8K block size.
 */
#define NUM_ALLOC_BLOCKSIZES 21
#define GROWTH_BOUNDARY 41

u_int64_t alloc_blocksize[NUM_ALLOC_BLOCKSIZES] =  {
	/* Block Size*/ /* Min Dflt FS Size */ /* Max FS Size */
	4096,			/* 0 bytes */			/* 16 TB */
	8192,			/* 2 TB */				/* 32 TB */		/* Note that 8K appears twice in table ! */
	8192,			/* 4 TB */				/* 32 TB */		/* Note that 8K appears twice in table ! */
	16384,			/* 8 TB */				/* 64 TB */
	32768,			/* 16 TB */				/* 128 TB */
	65536,			/* 32 TB */				/* 256 TB */
	131072,			/* 64 TB */				/* 512 TB */
	262144,			/* 128 TB */			/* 1 PB */
	524288,			/* 256 TB */			/* 2 PB */
	1048576,		/* 512 TB */			/* 4 PB */
	2097152,		/* 1 PB */				/* 8 PB */
	4194304,		/* 2 PB */				/* 16 PB */
	8388608,		/* 4 PB */				/* 32 PB */
	16777216,		/* 8 PB */				/* 64 PB */	
	33554432,		/* 16 PB */				/* 128 PB */
	67108864,		/* 32 PB */				/* 256 PB */
	134217728,		/* 64 PB */				/* 512 PB */
	268435456,		/* 128 PB */			/* 1 EB */
	536870912,		/* 256 PB */			/* 2 EB */
	1073741824,		/* 512 PB */			/* 4 EB */
	2147483648ULL	/* 1 EB */				/* 8 EB */
};

static int get_high_bit (u_int64_t bitstring) {
	u_int64_t bits = bitstring;
	int counter = 0;
	while (bits) {
		bits = (bits >> 1);
		counter++;
	}
	return counter;
}


/*
 * Validate the HFS Plus allocation block size in gBlockSize.  If none was
 * specified, then calculate a suitable default.
 *
 * Modifies the global variable gBlockSize.
 */
static void validate_hfsplus_block_size(UInt64 sectorCount, UInt32 sectorSize)
{
	if (gBlockSize == 0) {
		
		/* Start by calculating the fs size */
		u_int64_t fs_size = sectorCount * sectorSize;
		
		/* 
		 * Determine the default based on a sliding scale.  The maximum number of 
		 * allocation blocks is always 4294967295 == (32 bits worth).  At 1 bit per 
		 * allocation block, that yields 512 MB of bitmap no matter what size we use 
		 * for the allocation block.
		 *
		 * The general default policy is to allow the filesystem to grow up to 8x the 
		 * current maximum size.  So for a 1.5TB filesystem, an 8x multiplier would be
		 * 12TB.  That means we can use the default size of 4096 bytes.  The boundary begins
		 * at 2TB, since at that point, we can no longer use the default 4096 block size to 
		 * extend the filesystem by 8x.  For a 16KB block size, the max is 64 TB, but the 8x 
		 * multiplier begins at 8 TB.  Thereafter, we increase for every power of 2 that
		 * the current filesystem size grows.
		 */
		
		gBlockSize = DFL_BLKSIZE;	/* Prefer the default of 4K */
		
		int bit_index = get_high_bit (fs_size);
		bit_index -= GROWTH_BOUNDARY;
		
		/*
		 * After subtracting the GROWTH_BOUNDARY to index into the array, we'll
		 * use the values in the static array if we have a non-negative index.  
		 * That means that if the filesystem is >= 1 TB, then we'll use the index 
		 * value. At 2TB, we grow to the 8K block size.
		 */
		if ((bit_index >= 0) && (bit_index < 22)) {
			gBlockSize = (uint32_t)alloc_blocksize[bit_index];
		}
		
		if (bit_index >= 22) {
			fatal("Error: Disk Device is too big (%llu sectors, %d bytes per sector", sectorCount, sectorSize);
		}
	} 
	else {
		/* Make sure a user-specified block size is reasonable */
		if ((gBlockSize & (gBlockSize-1)) != 0) {
			fatal("%s: bad HFS Plus allocation block size (must be a power of two)", optarg);
		}
		
		if ((sectorCount / (gBlockSize / sectorSize)) > 0xFFFFFFFF) {
			fatal("%s: block size is too small for %lld sectors", optarg, gBlockSize, sectorCount);
		}
		
		if (gBlockSize < HFSOPTIMALBLKSIZE) {
			warnx("Warning: %u is a non-optimal block size (4096 would be a better choice)", (unsigned int)gBlockSize);
		}
	}

	if (gFSStartBlock) {
		u_int64_t fs_size = sectorCount * sectorSize;
		u_int32_t totalBlocks = (u_int32_t)(fs_size/gBlockSize);

		if (gFSStartBlock >= totalBlocks) {
			warnx("Warning: %u is invalid file system start allocation block number, must be less than total allocation blocks (%u)", (unsigned int)gFSStartBlock, (unsigned int)totalBlocks);
			warnx("Warning: Resetting file system start block to zero");
			gFSStartBlock = 0;
		}
	}
}



static int
hfs_newfs(char *device)
{
	struct stat stbuf;
	DriveInfo dip = { 0 };
	int fso = -1;
	int retval = 0;
	hfsparams_t defaults = {0};
	UInt64 maxPhysPerIO = 0;
	
	if (gPartitionSize) {
		dip.sectorSize = kBytesPerSector;
		dip.physTotalSectors = dip.totalSectors = gPartitionSize / kBytesPerSector;
		dip.physSectorSize = kBytesPerSector;	/* 512-byte sectors */
		dip.fd = 0;
	} else {
		if (gNoCreate) {
			fso = open( device, O_RDONLY | O_NDELAY, 0 );
		} else {
			fso = open( device, O_RDWR | O_NDELAY, 0 );
		}
		if (fso == -1) {
			return -1;
		}

		dip.fd = fso;
		fcntl(fso, F_NOCACHE, 1);

		if (fso < 0)
			fatal("%s: %s", device, strerror(errno));

		if (fstat( fso, &stbuf) < 0)
			fatal("%s: %s", device, strerror(errno));

		if (ioctl(fso, DKIOCGETBLOCKSIZE, &dip.physSectorSize) < 0)
			fatal("%s: %s", device, strerror(errno));

		if ((dip.physSectorSize % kBytesPerSector) != 0)
			fatal("%d is an unsupported sector size\n", dip.physSectorSize);

		if (ioctl(fso, DKIOCGETBLOCKCOUNT, &dip.physTotalSectors) < 0)
			fatal("%s: %s", device, strerror(errno));

	}

	dip.physSectorsPerIO = (1024 * 1024) / dip.physSectorSize;  /* use 1M as default */

	if (fso != -1 && ioctl(fso, DKIOCGETMAXBLOCKCOUNTREAD, &maxPhysPerIO) < 0)
		fatal("%s: %s", device, strerror(errno));

	if (maxPhysPerIO)
		dip.physSectorsPerIO = MIN(dip.physSectorsPerIO, maxPhysPerIO);

	if (fso != -1 && ioctl(fso, DKIOCGETMAXBLOCKCOUNTWRITE, &maxPhysPerIO) < 0)
		fatal("%s: %s", device, strerror(errno));

	if (maxPhysPerIO)
		dip.physSectorsPerIO = MIN(dip.physSectorsPerIO, maxPhysPerIO);

	if (fso != -1 && ioctl(fso, DKIOCGETMAXBYTECOUNTREAD, &maxPhysPerIO) < 0)
		fatal("%s: %s", device, strerror(errno));

	if (maxPhysPerIO)
		dip.physSectorsPerIO = MIN(dip.physSectorsPerIO, maxPhysPerIO / dip.physSectorSize);

	if (fso != -1 && ioctl(fso, DKIOCGETMAXBYTECOUNTWRITE, &maxPhysPerIO) < 0)
		fatal("%s: %s", device, strerror(errno));

	if (maxPhysPerIO)
		dip.physSectorsPerIO = MIN(dip.physSectorsPerIO, maxPhysPerIO / dip.physSectorSize);

	dip.sectorSize = kBytesPerSector;
	dip.totalSectors = dip.physTotalSectors * dip.physSectorSize / dip.sectorSize;

	dip.sectorOffset = 0;
	time(&createtime);
	
	/* Check to see if the disk is too big */
	u_int64_t secsize = (u_int64_t) dip.sectorSize;
	if (bad_disk_size(dip.totalSectors, secsize)) {
		fatal("%s: partition is too big (maximum is %llu KB)", device, MAXHFSVOLSIZE/1024);
	}
	
	/*
	 * If we're going to make an HFS Plus disk (with or without a wrapper), validate the
	 * HFS Plus allocation block size.  This will also calculate a default allocation
	 * block size if none (or zero) was specified.
	 */
	validate_hfsplus_block_size(dip.totalSectors, dip.sectorSize);

	/* Make an HFS Plus disk */	

	if ((dip.totalSectors * dip.sectorSize ) < kMinHFSPlusVolumeSize)
		fatal("%s: partition is too small (minimum is %d KB)", device, kMinHFSPlusVolumeSize/1024);

	hfsplus_params(&dip, &defaults);
	if (gNoCreate == 0) {
		retval = make_hfsplus(&dip, &defaults);
		if (retval == 0) {
			printf("Initialized %s as a ", device);
			if (dip.totalSectors > 2048ULL*1024*1024)
				printf("%ld TB",
						(long)((dip.totalSectors + (1024ULL*1024*1024))/(2048ULL*1024*1024)));
			else if (dip.totalSectors > 2048*1024)
				printf("%ld GB",
						(long)((dip.totalSectors + (1024*1024))/(2048*1024)));
			else if (dip.totalSectors > 2048)
				printf("%ld MB",
						(long)((dip.totalSectors + 1024)/2048));
			else
				printf("%ld KB",
						(long)((dip.totalSectors + 1)/2));

			if (gCaseSensitive) {
				printf(" case-sensitive");	
			}
			else {
				printf(" case-insensitive");	
			}
			if (gJournaled)
				printf(" HFS Plus volume with a %uk journal\n",
						(u_int32_t)defaults.journalSize/1024);
			else
				printf(" HFS Plus volume\n");
		}
	}

	if (retval)
		fatal("%s: %s", device, strerror(errno));

	if ( fso > 0 ) {
		close(fso);
	}

	return retval;
}

/*
 typedef struct block_info {
	off_t       bnum;		//64 bit
	union {
		_blk_info   bi;		//64 bit
		struct buf *bp;		//64 bit on K64, 32 bit on K32
	} u;
 }__attribute__((__packed__)) block_info;
 
 total size == 16 bytes 
 */

#define BLOCK_INFO_SIZE 16

static void hfsplus_params (const DriveInfo* dip, hfsparams_t *defaults)
{
	UInt64  sectorCount = dip->totalSectors;
	UInt32  sectorSize = dip->sectorSize;
	uint32_t totalBlocks;
	UInt32	minClumpSize;
	UInt32	clumpSize;
	UInt32	initialSize;
	UInt32	oddBitmapBytes;
	
	defaults->flags = 0;
	defaults->blockSize = gBlockSize;
	defaults->fsStartBlock = gFSStartBlock;
	defaults->nextFreeFileID = gNextCNID;
    // Value will be bigger than UIN32_MAX in 2040
	defaults->createDate = (uint32_t)(createtime + MAC_GMT_FACTOR);     /* Mac OS GMT time */
	defaults->hfsAlignment = 0;
	defaults->journaledHFS = gJournaled;
	defaults->journalDevice = gJournalDevice;

	/*
	 * 8429818
	 * Always set kUseAccessPerms now; this also
	 * means we have to always have an owner, group,
	 * and mask.
	 */
	defaults->owner = (gUserID == (uid_t)NOVAL) ? geteuid() : gUserID;
	defaults->group = (gGroupID == (gid_t)NOVAL) ? getegid() : gGroupID;
	defaults->mask = (gModeMask == (mode_t)NOVAL) ? UMASK : (gModeMask & ACCESSMASK);
	defaults->flags |= kUseAccessPerms;

	/*
	 * We want at least 8 megs of journal for each 100 gigs of
	 * disk space.  We cap the size at 512 megs (64x default), unless
	 * the allocation block size is larger, in which case we use one
	 * allocation block.
	 *
	 * Only scale if it's the default, otherwise just take what
	 * the user specified, with the caveat below.
	 */
	if (gJournaled) { 
		uint32_t min_size = 0;

		/* 
		 * Check to ensure the journal size is not too small relative to the
		 * sector size of the device.  This is the check in the kernel:
		 if (tr->blhdr && (tr->blhdr->max_blocks <= 0 || 
		 tr->blhdr->max_blocks > (tr->jnl->jhdr->size/tr->jnl->jhdr->jhdr_size))) 
		 * We assume that there will be a block header and that there will be a 
		 * non-negative max_blocks value.  
		 * 
		 * The 2nd check is the problematic one.  We cannot have a journal that's too 
		 * small relative to the sector size.  max_blocks == (blhdr_size / 16).  However, 
		 * this only matters where the current block header size is smaller than the current 
		 * sector size. So, assume that the blhdr_size == sector size for now.  We look 
		 * at the condition above to get the rest of the equation -- (journal size / sector size).  
		 * Then, it's simple algebra to figure out what the new minimum journal size 
		 * should be:
		 * 
		 *	(sector_size / 16) > (journal_size / sector_size)
		 *	(sector_size / 16) = (journal_size / sector_size)
		 *	(sector_size / 16) * sector_size = (journal_size / sector_size) * sector_size
		 *	(sector_size / 16) * sector_size = journal_size
		 *
		 *  This becomes our new _floor_ for the journal_size. 
		 */

		if (dip->physSectorSize != 0) {
			min_size = dip->physSectorSize * (dip->physSectorSize / BLOCK_INFO_SIZE);
		}

		if (gJournalSize != 0) {

			/* Was the supplied journal size at least the minimum computed above? */
			if (gJournalSize < min_size) {
				printf("%s: journal size %lldk too small.  Reset to %dk.\n",
						progname, gJournalSize/1024, JOURNAL_DEFAULT_SIZE/1024);
				gJournalSize = 0;

			}
			/* defaults->journalSize will get reset below if it is 0 */
			defaults->journalSize = (uint32_t)gJournalSize;
		}

		if ((gJournalSize == 0) || (defaults->journalSize == 0)) {
			UInt32 jscale;
			uint32_t target_size;
			/* Figure out how many 100's of GBs this filesystem represents */
			jscale = (sectorCount * sectorSize) / ((UInt64)100 * 1024 * 1024 * 1024);
			if (jscale > 64) {
				jscale = 64;
			}
				
			target_size = JOURNAL_DEFAULT_SIZE * (jscale + 1);
			/* Is the target size at least the min_size computed above? */
			if (target_size < min_size) {
				target_size = min_size;
			}

			defaults->journalSize = target_size;
		} 


#ifndef DEBUG_BUILD
		// volumes that are 128 megs or less in size have such
		// a small bitmap (one 4k-block) and inherhently such
		// a small btree that we can get by with a much smaller
		// journal.  even in a worst case scenario of a catalog
		// filled with very long korean file names we should
		// never touch more than 256k of meta-data for a single
		// transaction.  therefore we'll make the journal 512k,
		// or as small as possible, given the sector size,
		// which is safe and doesn't waste much space.
		// However, be careful not to let the journal size drop BELOW
		// 512k, since the min_size computations can create an artificially
		// tiny journal (16k or so) with 512byte sectors.
		//
		if (sectorCount * sectorSize < 128*1024*1024) {
			/* This is a small (<128MB) FS */
			uint32_t small_default = (512 * 1024);

			if (small_default <= min_size) {
				/* 
				 * If 512k is too small given the sector size,
				 * then use the larger sector size 
				 */
				defaults->journalSize = min_size;
			}
			else {
				/* 512k was bigger than the min size; we can use it */
				defaults->journalSize = small_default;
			}
		}
#endif

		if (defaults->journalSize > 512 * 1024 * 1024) {
			defaults->journalSize = 512 * 1024 * 1024;
		}

		if (defaults->journalSize < defaults->blockSize) {
			defaults->journalSize = defaults->blockSize;
		}
	}
	
	defaults->volumeName = (unsigned char*)gVolumeName;

	if (rsrclumpblks == 0) {
		if (gBlockSize > DFL_BLKSIZE)
			defaults->rsrcClumpSize = ROUNDUP(kHFSPlusRsrcClumpFactor * DFL_BLKSIZE, gBlockSize);
		else
			defaults->rsrcClumpSize = kHFSPlusRsrcClumpFactor * gBlockSize;
	} else
		defaults->rsrcClumpSize = clumpsizecalc(rsrclumpblks);

	if (datclumpblks == 0) {
		if (gBlockSize > DFL_BLKSIZE)
			defaults->dataClumpSize = ROUNDUP(kHFSPlusDataClumpFactor * DFL_BLKSIZE, gBlockSize);
		else
			defaults->dataClumpSize = kHFSPlusDataClumpFactor * gBlockSize;
	} else
		defaults->dataClumpSize = clumpsizecalc(datclumpblks);

	/*
	 * The default  b-tree node size is 8K.  However, if the
	 * volume is small (< 1 GB) we use 4K instead.
	 */
	if (!gUserCatNodeSize) {
		if ((gBlockSize < HFSOPTIMALBLKSIZE) ||
		    ((UInt64)(sectorCount * sectorSize) < (UInt64)0x40000000))
			catnodesiz = 4096;
	}

	if (catclumpblks == 0) {
		clumpSize = CalcHFSPlusBTreeClumpSize(gBlockSize, catnodesiz, sectorCount, kHFSCatalogFileID);
	}
	else {
		clumpSize = clumpsizecalc(catclumpblks);
		if (clumpSize % catnodesiz != 0)
			fatal("c=%ld: clump size is not a multiple of node size\n", clumpSize/gBlockSize);
	}
	if (catinitialblks == 0) {
		initialSize = CalcHFSPlusBTreeClumpSize(gBlockSize, catnodesiz, sectorCount, kHFSCatalogFileID);
	}
	else {
		initialSize = initialsizecalc(catinitialblks);
		if (initialSize % catnodesiz != 0)
			fatal("c=%ld: initial size is not a multiple of node size\n", initialSize/gBlockSize);
	}
	if (initialSize < clumpSize) {
		fatal("c=%ld: initial size is less than clump size\n", initialSize/gBlockSize);
	}
	defaults->catalogClumpSize = clumpSize;
	defaults->catalogInitialSize = initialSize;
	defaults->catalogNodeSize = catnodesiz;
	defaults->catalogExtsCount = catExtCount;
	defaults->catalogStartBlock = catExtStart;

	if (gBlockSize < 4096 && gBlockSize < catnodesiz)
		warnx("Warning: block size %u is less than catalog b-tree node size %u", (unsigned int)gBlockSize, (unsigned int)catnodesiz);

	if (extclumpblks == 0) {
		clumpSize = CalcHFSPlusBTreeClumpSize(gBlockSize, extnodesiz, sectorCount, kHFSExtentsFileID);
	}
	else {
		clumpSize = clumpsizecalc(extclumpblks);
		if (clumpSize % extnodesiz != 0)
			fatal("e=%ld: clump size is not a multiple of node size\n", clumpSize/gBlockSize);
	}
	if (extinitialblks == 0) {
		initialSize = CalcHFSPlusBTreeClumpSize(gBlockSize, extnodesiz, sectorCount, kHFSExtentsFileID);
	}
	else {
		initialSize = initialsizecalc(extinitialblks);
		if (initialSize % extnodesiz != 0)
			fatal("e=%ld: initial size is not a multiple of node size\n", initialSize/gBlockSize);
	}
	if (initialSize < clumpSize) {
		fatal("e=%ld: initial size is less than clump size\n", initialSize/gBlockSize);
	}
	defaults->extentsClumpSize = clumpSize;
	defaults->extentsInitialSize = initialSize;
	defaults->extentsNodeSize = extnodesiz;
	defaults->extentsExtsCount = extExtCount;
	defaults->extentsStartBlock = extExtStart;

	if (gBlockSize < extnodesiz)
		warnx("Warning: block size %u is less than extents b-tree node size %u", (unsigned int)gBlockSize, (unsigned int)extnodesiz);
	if (defaults->extentsExtsCount > 8) {
		warnx("Warning:  extents overflow extent requested count %u exceeds maximum 8, capping at 8\n", defaults->extentsExtsCount);
		defaults->extentsExtsCount = 8;
	}

	if (atrclumpblks == 0) {
		clumpSize = CalcHFSPlusBTreeClumpSize(gBlockSize, atrnodesiz, sectorCount, kHFSAttributesFileID);
	}
	else {
		clumpSize = clumpsizecalc(atrclumpblks);
		if (clumpSize % atrnodesiz != 0)
			fatal("a=%ld: clump size is not a multiple of node size\n", clumpSize/gBlockSize);
	}
	if (atrinitialblks == 0) {
		if (gUserAttrSize) {
			initialSize = 0;
		}
		else {
			initialSize = CalcHFSPlusBTreeClumpSize(gBlockSize, atrnodesiz, sectorCount, kHFSAttributesFileID);
		}
	}
	else {
		initialSize = initialsizecalc(atrinitialblks);
		if (initialSize % atrnodesiz != 0)
			fatal("a=%ld: initial size is not a multiple of node size\n", initialSize/gBlockSize);
	}
	if (initialSize) {
		if (initialSize < clumpSize) {
			fatal("a=%ld: initial size is less than clump size\n", initialSize/gBlockSize);
		}
	}
	defaults->attributesClumpSize = clumpSize;
	defaults->attributesInitialSize = initialSize;
	defaults->attributesNodeSize = atrnodesiz;
	defaults->attributesExtsCount = attrExtCount;
	defaults->attributesStartBlock = attrExtStart;

	/*
	 * Calculate the number of blocks needed for bitmap (rounded up to a multiple of the block size).
	 */
	
	/*
	 * Figure out how many bytes we need for the given totalBlocks
	 * Note: this minimum value may be too large when it counts the
	 * space used by the wrapper
	 */
	totalBlocks = (uint32_t)(sectorCount / (gBlockSize / sectorSize));

	minClumpSize = totalBlocks >> 3;	/* convert bits to bytes by dividing by 8 */
	if (totalBlocks & 7)
		++minClumpSize;	/* round up to whole bytes */
	
	/* Round up to a multiple of blockSize */
	if ((oddBitmapBytes = minClumpSize % gBlockSize))
		minClumpSize = minClumpSize - oddBitmapBytes + gBlockSize;

	if (bmclumpblks == 0) {
		clumpSize = minClumpSize;
	}
	else {
		clumpSize = clumpsizecalc(bmclumpblks);

		if (clumpSize < minClumpSize)
			fatal("b=%ld: bitmap clump size is too small\n", clumpSize/gBlockSize);
	}
	defaults->allocationClumpSize = clumpSize;
	defaults->allocationExtsCount = blkallocExtCount;
	defaults->allocationStartBlock = blkallocExtStart;

	defaults->journalInfoBlock = jibStart;
	defaults->journalBlock = jnlStart;
	defaults->nextAllocBlock = allocStart;

	if (gCaseSensitive)
		defaults->flags |= kMakeCaseSensitive;
	
	if (gContentProtect)
		defaults->flags |= kMakeContentProtect;

#ifdef DEBUG_BUILD
	if (gProtectLevel) 
		defaults->protectlevel = gProtectLevel;
#endif
	
	if (gNoCreate) {
		if (gPartitionSize == 0)
			printf("%llu sectors (%u bytes per sector)\n", dip->physTotalSectors, dip->physSectorSize);
		printf("HFS Plus format parameters:\n");
		printf("\tvolume name: \"%s\"\n", gVolumeName);
		printf("\tblock-size: %u\n", defaults->blockSize);
		printf("\ttotal blocks: %u\n", totalBlocks);
		if (gJournaled)
			printf("\tjournal-size: %uk\n", defaults->journalSize/1024);
		printf("\tfirst free catalog node id: %u\n", defaults->nextFreeFileID);
		printf("\tcatalog b-tree node size: %u\n", defaults->catalogNodeSize);
		printf("\tcatalog clump size: %u\n", defaults->catalogClumpSize);
		printf("\tinitial catalog file size: %u\n", defaults->catalogInitialSize);
		printf("\textents b-tree node size: %u\n", defaults->extentsNodeSize);
		printf("\textents clump size: %u\n", defaults->extentsClumpSize);
		printf("\tinitial extents file size: %u\n", defaults->extentsInitialSize);
		printf("\tattributes b-tree node size: %u\n", defaults->attributesNodeSize);
		printf("\tattributes clump size: %u\n", defaults->attributesClumpSize);
		printf("\tinitial attributes file size: %u\n", defaults->attributesInitialSize);
		printf("\tinitial allocation file size: %u (%u blocks)\n",
			defaults->allocationClumpSize, defaults->allocationClumpSize / gBlockSize);
		printf("\tdata fork clump size: %u\n", defaults->dataClumpSize);
		printf("\tresource fork clump size: %u\n", defaults->rsrcClumpSize);
		if (defaults->flags & kUseAccessPerms) {
			printf("\tuser ID: %d\n", (int)defaults->owner);
			printf("\tgroup ID: %d\n", (int)defaults->group);
			printf("\taccess mask: %o\n", (int)defaults->mask);
		}
		printf("\tfile system start block: %u\n", defaults->fsStartBlock);
	}
}


static UInt32
initialsizecalc(UInt32 initialblocks)
{
	UInt64 initialsize;

	initialsize = (UInt64)initialblocks * (UInt64)gBlockSize;
		
	if (initialsize & (UInt64)(0xFFFFFFFF00000000ULL))
		fatal("=%ld: too many blocks for initial size!", initialblocks);

	return ((UInt32)initialsize);
}


static UInt32
clumpsizecalc(UInt32 clumpblocks)
{
	UInt64 clumpsize;

	clumpsize = (UInt64)clumpblocks * (UInt64)gBlockSize;
		
	if (clumpsize & (UInt64)(0xFFFFFFFF00000000ULL))
		fatal("=%ld: too many blocks for clump size!", clumpblocks);

	return ((UInt32)clumpsize);
}


#define CLUMP_ENTRIES	15

short clumptbl[CLUMP_ENTRIES * 3] = {
/*
 *	    Volume	Attributes	 Catalog	 Extents
 *	     Size	Clump (MB)	Clump (MB)	Clump (MB)
 */
	/*   1GB */	  4,		  4,		 4,
	/*   2GB */	  6,		  6,		 4,
	/*   4GB */	  8,		  8,		 4,
	/*   8GB */	 11,		 11,		 5,
	/*
	 * For volumes 16GB and larger, we want to make sure that a full OS
	 * install won't require fragmentation of the Catalog or Attributes
	 * B-trees.  We do this by making the clump sizes sufficiently large,
	 * and by leaving a gap after the B-trees for them to grow into.
	 *
	 * For SnowLeopard 10A298, a FullNetInstall with all packages selected
	 * results in:
	 * Catalog B-tree Header
	 *	nodeSize:          8192
	 *	totalNodes:       31616
	 *	freeNodes:         1978
	 * (used = 231.55 MB)
	 * Attributes B-tree Header
	 *	nodeSize:          8192
	 *	totalNodes:       63232
	 *	freeNodes:          958
	 * (used = 486.52 MB)
	 *
	 * We also want Time Machine backup volumes to have a sufficiently
	 * large clump size to reduce fragmentation.
	 *
	 * The series of numbers for Catalog and Attribute form a geometric series.
	 * For Catalog (16GB to 512GB), each term is 8**(1/5) times the previous
	 * term.  For Attributes (16GB to 512GB), each term is 4**(1/5) times
	 * the previous term.  For 1TB to 16TB, each term is 2**(1/5) times the
	 * previous term.
	 */
	/*  16GB */	 64,		 32,		 5,
	/*  32GB */	 84,		 49,		 6,
	/*  64GB */	111,		 74,		 7,
	/* 128GB */	147,		111,		 8,
	/* 256GB */	194,		169,		 9,
	/* 512GB */	256,		256,		11,
	/*   1TB */	294,		294,		14,
	/*   2TB */	338,		338,		16,
	/*   4TB */	388,		388,		20,
	/*   8TB */	446,		446,		25,
	/*  16TB */	512,		512,		32
};

/*
 * CalcHFSPlusBTreeClumpSize
 *	
 * This routine calculates the file clump size for either
 * the catalog file or the extents overflow file.
 */
static UInt32
CalcHFSPlusBTreeClumpSize(UInt32 blockSize, UInt32 nodeSize, UInt64 sectors, int fileID)
{
	UInt32 mod = MAX(nodeSize, blockSize);
	UInt32 clumpSize;
	int column;
	int i;

	/* Figure out which column of the above table to use for this file. */
	switch (fileID) {
		case kHFSAttributesFileID:
			column = 0;
			break;
		case kHFSCatalogFileID:
			column = 1;
			break;
		default:
			column = 2;
			break;
	}
	
	/*
	 * The default clump size is 0.8% of the volume size. And
	 * it must also be a multiple of the node and block size.
	 */
	if (sectors < 0x200000) {
		clumpSize = (UInt32)(sectors << 2);	/*  0.8 %  */
		if (clumpSize < (8 * nodeSize))
			clumpSize = 8 * nodeSize;
	} else {
		/*
		 * XXX This should scale more smoothly!
		 */
		/* turn exponent into table index... */
		for (i = 0, sectors = sectors >> 22;
		     sectors && (i < CLUMP_ENTRIES-1);
		     ++i, sectors = sectors >> 1);
		
		clumpSize = clumptbl[column + (i) * 3] * 1024 * 1024;
	}
	
	/*
	 * Round the clump size to a multiple of node and block size.
	 * NOTE: This rounds down.
	 */
	clumpSize /= mod;
	clumpSize *= mod;
	
	/*
	 * Rounding down could have rounded down to 0 if the block size was
	 * greater than the clump size.  If so, just use one block or node.
	 */
	if (clumpSize == 0)
		clumpSize = mod;
		
	return (clumpSize);
}


/* VARARGS */
void
#if __STDC__
fatal(const char *fmt, ...)
#else
fatal(fmt, va_alist)
	char *fmt;
	va_dcl
#endif
{
	va_list ap;

#if __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	if (fcntl(STDERR_FILENO, F_GETFL) < 0) {
		openlog(progname, LOG_CONS, LOG_DAEMON);
		vsyslog(LOG_ERR, fmt, ap);
		closelog();
	} else {
		vwarnx(fmt, ap);
	}
	va_end(ap);
	exit(1);
	/* NOTREACHED */
}


void usage()
{
	fprintf(stderr, "usage: %s [-N [partition-size]] [hfsplus-options] special-device\n", progname);

	fprintf(stderr, "  options:\n");
	fprintf(stderr, "\t-N do not create file system, just print out parameters\n");
	fprintf(stderr, "\t-s use case-sensitive filenames (default is case-insensitive)\n");

	fprintf(stderr, "  where hfsplus-options are:\n");
	fprintf(stderr, "\t-J [journal-size] make this HFS+ volume journaled\n");
	fprintf(stderr, "\t-D journal-dev use 'journal-dev' for an external journal\n");
	fprintf(stderr, "\t-G group-id (for root directory)\n");
	fprintf(stderr, "\t-U user-id (for root directory)\n");
	fprintf(stderr, "\t-M octal access-mask (for root directory)\n");
	fprintf(stderr, "\t-b allocation block size (4096 optimal)\n");
	fprintf(stderr, "\t-c clump size list (comma separated)\n");
	fprintf(stderr, "\t\ta=blocks (attributes file)\n");
	fprintf(stderr, "\t\tb=blocks (bitmap file)\n");
	fprintf(stderr, "\t\tc=blocks (catalog file)\n");
	fprintf(stderr, "\t\td=blocks (user data fork)\n");
	fprintf(stderr, "\t\te=blocks (extents file)\n");
	fprintf(stderr, "\t\tr=blocks (user resource fork)\n");
	fprintf(stderr, "\t-i starting catalog node id\n");
	fprintf(stderr, "\t-I initial size list (comma separated)\n");
	fprintf(stderr, "\t\ta=size (attributes b-tree)\n");
	fprintf(stderr, "\t\tc=size (catalog b-tree)\n");
	fprintf(stderr, "\t\te=size (extents b-tree)\n");
	fprintf(stderr, "\t-n b-tree node size list (comma separated)\n");
	fprintf(stderr, "\t\ta=size (attributes b-tree)\n");
	fprintf(stderr, "\t\tc=size (catalog b-tree)\n");
	fprintf(stderr, "\t\te=size (extents b-tree)\n");
	fprintf(stderr, "\t-v volume name (in ascii or UTF-8)\n");
#ifdef DEBUG_BUILD
	fprintf(stderr, "\t-E extent count list (comma separated)\n");
	fprintf(stderr, "\t\ta=count (attributes file)\n");
	fprintf(stderr, "\t\tb=count (bitmap file)\n");
	fprintf(stderr, "\t\tc=count (catalog file)\n");
	fprintf(stderr, "\t\te=count (extents file)\n");
	fprintf(stderr, "\t-a <num>[,list] metadata start allocation block, all btrees and journal will be created starting at this allocation block offset\n");
	fprintf(stderr, "\t\tlist is as with -E above, plus:\n");
	fprintf(stderr, "\t\tj=addr (JournalInfoBlock)\n");
	fprintf(stderr, "\t\tJ=addr (Journal)\n");
	fprintf(stderr, "\t\tN=addr (Next Allocation Block)\n");
	fprintf(stderr, "\t\tExample: -a 100,e=200,c=500\n");
#endif

	fprintf(stderr, "  examples:\n");
	fprintf(stderr, "\t%s -v Untitled /dev/rdisk0s7 \n", progname);
	fprintf(stderr, "\t%s -v Untitled -n c=4096,e=1024 /dev/rdisk0s7 \n", progname);
	fprintf(stderr, "\t%s -v Untitled -c b=64,c=1024 /dev/rdisk0s7 \n\n", progname);

	exit(1);
}
