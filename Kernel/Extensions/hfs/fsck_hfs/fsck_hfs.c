/*
 * Copyright (c) 1999-2000, 2002-2008 Apple Inc. All rights reserved.
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
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#include <sys/ioctl.h>
#include <sys/disk.h>
#include <sys/sysctl.h>
#include <err.h>
#include <setjmp.h>

#include <hfs/hfs_mount.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>

#include <TargetConditionals.h>

#include "fsck_hfs.h"
#include "fsck_msgnums.h"
#include "fsck_hfs_msgnums.h"

#include "fsck_debug.h"
#include "dfalib/CheckHFS.h"

/*
 * These definitions are duplicated from xnu's hfs_readwrite.c, and could live
 * in a shared header file if desired. On the other hand, the freeze and thaw
 * commands are not really supposed to be public.
 */
#ifndef    F_FREEZE_FS
#define F_FREEZE_FS     53              /* "freeze" all fs operations */
#define F_THAW_FS       54              /* "thaw" all fs operations */
#endif  // F_FREEZE_FS

/* Global Variables for front end */
const char *cdevname;		/* name of device being checked */
char	*progname;
char	lflag;			/* live fsck */
char	nflag;			/* assume a no response */
char	yflag;			/* assume a yes response */
char	preen;			/* just fix normal inconsistencies */
char	force;			/* force fsck even if clean (preen only) */
char	quick;			/* quick check returns clean, dirty, or failure */
char	debug;			/* output debugging info */
char	disable_journal;	/* If debug, and set, do not simulate journal replay */
char	scanflag;		/* Scan entire disk for bad blocks */
#if	!TARGET_OS_IPHONE
char	embedded = 0;
#else
char	embedded = 1;
#endif

char	hotroot;		/* checking root device */
char	hotmount;		/* checking read-only mounted device */
char 	guiControl; 	/* this app should output info for gui control */
char	xmlControl;	/* Output XML (plist) messages -- implies guiControl as well */
char	rebuildBTree;  	/* Rebuild requested btree files */
int	rebuildOptions;	/* Options to indicate which btree should be rebuilt */
char	modeSetting;	/* set the mode when creating "lost+found" directory */
char	errorOnExit = 0;	/* Exit on first error */
int		upgrading;		/* upgrading format */
int		lostAndFoundMode = 0; /* octal mode used when creating "lost+found" directory */
uint64_t reqCacheSize;	/* Cache size requested by the caller (may be specified by the user via -c) */
int     detonator_run = 0;

int	fsmodified;		/* 1 => write done to file system */
int	fsreadfd;		/* file descriptor for reading file system */
int	fswritefd;		/* file descriptor for writing file system */
Cache_t fscache;

/*
 * Variables used to map physical block numbers to file paths
 */
enum { BLOCK_LIST_INCREMENT = 512 };
int gBlkListEntries = 0;
u_int64_t *gBlockList = NULL;
int gFoundBlockEntries = 0;
struct found_blocks *gFoundBlocksList = NULL;
long gBlockSize = 512;
static void ScanDisk(int);
static int getblocklist(const char *filepath);


static int checkfilesys __P((char * filesys));
static int setup __P(( char *dev, int *canWritePtr ));
static void usage __P((void));
static void getWriteAccess __P(( char *dev, int *canWritePtr ));
extern char *unrawname __P((char *name));

int
main(argc, argv)
 int	argc;
 char	*argv[];
{
	int ch;
	int ret;
	extern int optind;
	extern char *optarg;
	char * lastChar;
	long mode;

	if ((progname = strrchr(*argv, '/')))
		++progname;
	else
		progname = *argv;

	while ((ch = getopt(argc, argv, "b:B:c:D:e:Edfglm:npqrR:SuyxJ")) != EOF) {
		switch (ch) {
		case 'b':
			gBlockSize = atoi(optarg);
			if ((gBlockSize < 512) || (gBlockSize & (gBlockSize-1))) {
				(void) fprintf(stderr, "%s invalid block size %d\n",
					progname, gBlockSize);
				exit(2);
			}
			break;
		case 'S':
			scanflag = 1;
			break;
		case 'B':
			getblocklist(optarg);
			break;
		case 'c':
			/* Cache size to use in fsck_hfs */
			reqCacheSize = strtoull(optarg, &lastChar, 0);
			if (*lastChar) {
				switch (tolower(*lastChar)) {
					case 'g':
						reqCacheSize *= 1024ULL;
						/* fall through */
					case 'm':
						reqCacheSize *= 1024ULL;
						/* fall through */
					case 'k':
						reqCacheSize *= 1024ULL;
						break;
					default:
						reqCacheSize = 0;
						break;
				};
			}
			break;

		case 'd':
			debug++;
			break;

		case 'J':
			disable_journal++;
			break;
		case 'D':
			/* Input value should be in hex example: -D 0x5 */
			cur_debug_level = strtoul(optarg, NULL, 0);
			if (cur_debug_level == 0) {
				(void) fplog (stderr, "%s: invalid debug development argument.  Assuming zero\n", progname);
			}
			break;

		case 'e':
			if (optarg) {
				if (strcasecmp(optarg, "embedded") == 0)
					embedded = 1;
				else if (strcasecmp(optarg, "desktop") == 0)
					embedded = 0;
			}
			break;

		case 'E':
			/* Exit on first error, after logging it */
			errorOnExit = 1;
			break;
		case 'f':
			force++;
			break;

		case 'g':
			guiControl++;
			break;

		case 'x':
			guiControl = 1;
			xmlControl++;
			break;

		case 'l':
			lflag++;
			nflag++;
			yflag = 0;
			force++;
			break;
			
		case 'm':
			modeSetting++;
			mode = strtol( optarg, NULL, 8 );
			lostAndFoundMode = (int)mode;
			if ( lostAndFoundMode == 0 || mode < INT_MIN || mode > INT_MAX)
			{
				(void) fplog(stderr, "%s: %ld is invalid mode argument\n", progname, mode);
				usage();
			}
			break;
			
		case 'n':
			nflag++;
			yflag = 0;
			break;

		case 'p':
			preen++;
			break;

		case 'q':
		 	quick++;
			break;

		case 'r':
			// rebuild catalog btree
			rebuildBTree++; 	
			rebuildOptions |= REBUILD_CATALOG;
			break;

		case 'R':
			if (optarg) {
				char *cp = optarg;
				while (*cp) {
					switch (*cp) {
						case 'a':	
							// rebuild attribute btree
							rebuildBTree++; 	
							rebuildOptions |= REBUILD_ATTRIBUTE;
							break;

						case 'c':
							// rebuild catalog btree
							rebuildBTree++; 	
							rebuildOptions |= REBUILD_CATALOG;
							break;

						case 'e':
							// rebuild extents overflow btree
							rebuildBTree++; 	
							rebuildOptions |= REBUILD_EXTENTS;
							break;

						default:	
							fprintf(stderr, "%s: unknown btree rebuild code `%c' (%#x)\n", progname, *cp, *cp);
							exit(2);
					}
					cp++;
				}
				break;
			}

		case 'y':
			yflag++;
			nflag = 0;
			break;

		case 'u':
		case '?':
		default:
			usage();
		}
	}

	argc -= optind;
	argv += optind;
	
	if (debug == 0 && disable_journal != 0)
		disable_journal = 0;

	if (gBlkListEntries != 0 && gBlockSize == 0)
		gBlockSize = 512;
	
	if (guiControl)
		debug = 0; /* debugging is for command line only */

	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		(void)signal(SIGINT, catch);

	if (argc < 1) {
		(void) fplog(stderr, "%s: missing special-device\n", progname);
		usage();
	}

	ret = 0;
    while (argc-- > 0) {
        char *pcBlkChk = blockcheck(*argv++);
		ret |= checkfilesys(pcBlkChk);
    }

	exit(ret);
}

int fs_fd=-1;  // fd to the root-dir of the fs we're checking (only w/lfag == 1)

void
cleanup_fs_fd(void)
{
    if (fs_fd >= 0) {
	fcntl(fs_fd, F_THAW_FS, NULL);
	close(fs_fd);
	fs_fd = -1;
    }
}

static char *
mountpoint(const char *cdev)
{
	char *retval = NULL;
	struct statfs *fsinfo;
	char *unraw = NULL;
	int result;
	int i;
    
    if (detonator_run)
        return NULL;

	unraw = strdup(cdev);
	unrawname(unraw);

	if (unraw == NULL)
		goto done;

	result = getmntinfo(&fsinfo, MNT_NOWAIT);

	for (i = 0; i < result; i++) {
		if (strcmp(unraw, fsinfo[i].f_mntfromname) == 0) {
			retval = strdup(fsinfo[i].f_mntonname);
			break;
		}
	}

done:
	if (unraw)
		free(unraw);

	return retval;
}

static int
checkfilesys(char * filesys)
{
	int flags;
	int result = 0;
	int chkLev, repLev, logLev;
	int canWrite;
	char *mntonname = NULL;
	fsck_ctx_t context = NULL;
	flags = 0;
	cdevname = filesys;
	canWrite = 0;
	hotmount = hotroot;	// hotroot will be 1 or 0 by this time

	//
	// initialize the printing/logging without actually printing anything
	// DO NOT DELETE THIS or else you can deadlock during a live fsck
	// when something is printed and we try to create the log file.
	//
	plog(""); 

	context = fsckCreate();

	mntonname = mountpoint(cdevname);
	if (hotroot) {
		if (mntonname)
			free(mntonname);
		mntonname = strdup("/");
	}

	if (lflag && !detonator_run) {
		struct stat fs_stat;

		/*
		 * Ensure that, if we're doing a live verify, that we're not trying
		 * to do input or output to the same device.  This would cause a deadlock.
		 */

		if (stat(cdevname, &fs_stat) != -1 &&
			(((fs_stat.st_mode & S_IFMT) == S_IFCHR) ||
			 ((fs_stat.st_mode & S_IFMT) == S_IFBLK))) {
			struct stat io_stat;

			if (fstat(fileno(stdin), &io_stat) != -1 &&
				(fs_stat.st_rdev == io_stat.st_dev)) {
					plog("ERROR: input redirected from target volume for live verify.\n");
					return EEXIT;
			}
			if (fstat(fileno(stdout), &io_stat) != -1 &&
				(fs_stat.st_rdev == io_stat.st_dev)) {
					plog("ERROR:  output redirected to target volume for live verify.\n");
					return EEXIT;
			}
			if (fstat(fileno(stderr), &io_stat) != -1 &&
				(fs_stat.st_rdev == io_stat.st_dev)) {
					plog("ERROR:  error output redirected to target volume for live verify.\n");
					return EEXIT;
			}

		}
	}

	/*
	 * If the device is mounted somewhere, then we need to make sure that it's
	 * a read-only device, or that a live-verify has been requested.
	 */
	if (mntonname != NULL) {
		struct statfs stfs_buf;

		if (statfs(mntonname, &stfs_buf) == 0) {
			if (lflag) {
				// Need to try to freeze it
				fs_fd = open(mntonname, O_RDONLY);
				if (fs_fd < 0) {
					plog("ERROR: could not open %s to freeze the volume.\n", mntonname);
					free(mntonname);
					return EEXIT;
				}
	
				if (fcntl(fs_fd, F_FREEZE_FS, NULL) != 0) {
					free(mntonname);
					plog("ERROR: could not freeze volume (%s)\n", strerror(errno));
					return EEXIT;
				}
			} else if (stfs_buf.f_flags & MNT_RDONLY) {
				hotmount = 1;
			} else {
				/* MNT_RDONLY is not set and this is not a live verification */
				plog("ERROR: volume %s is mounted with write access. Re-run with (-l) to freeze volume.\n", mntonname);
				return EEXIT;
			}
		}
	}

	if (debug && preen)
		pwarn("starting\n");
	
    if (setup( filesys, &canWrite ) == 0) {
		if (preen)
			pfatal("CAN'T CHECK FILE SYSTEM.");
		result = EEXIT;
		goto ExitThisRoutine;
	}

	if (preen == 0) {
		if (hotroot && !guiControl)
		    plog("** Root file system\n");
	}

	/* start with defaults for dfa back-end */
	chkLev = kAlwaysCheck;
	repLev = kMajorRepairs;
	logLev = kVerboseLog;

	if (yflag)	
		repLev = kMajorRepairs;

	if (quick) {
		chkLev = kNeverCheck;
		repLev = kNeverRepair;
		logLev = kFatalLog;
	} else if (force) {
		chkLev = kForceCheck;
	}
	if (preen) {
		repLev = kMinorRepairs;
		chkLev = force ? kAlwaysCheck : kDirtyCheck;
		logLev = kFatalLog;
	}
	if (debug)
		logLev = kDebugLog;

	if (nflag)
		repLev = kNeverRepair;
		
	if ( rebuildBTree ) {
		chkLev = kPartialCheck;
		repLev = kForceRepairs;  // this will force rebuild of B-Tree file
	}

    fsckSetVerbosity(context, logLev);
	/* All of fsck_hfs' output should go thorugh logstring */
	fsckSetOutput(context, NULL);
	/* Setup writer that will output to standard out */
	fsckSetWriter(context, &outstring);
    
	/* Setup logger that will write to log file */
	fsckSetLogger(context, &logstring);
	if (guiControl) {
		if (xmlControl)
			fsckSetOutputStyle(context, fsckOutputXML);
		else
			fsckSetOutputStyle(context, fsckOutputGUI);
	} else {
		fsckSetOutputStyle(context, fsckOutputTraditional);
	}

	if (errorOnExit && nflag) {
		chkLev = kMajorCheck;
	}

	/*
	 * go check HFS volume...
	 */

	if (rebuildOptions && canWrite == 0) {
		plog("BTree rebuild requested but writing disabled\n");
		result = EEXIT;
		goto ExitThisRoutine;
	}

	if (gBlockList != NULL && scanflag != 0) {
		plog("Cannot scan for bad blocks and ask for listed blocks to file mapping\n");
		result = EEXIT;
		goto ExitThisRoutine;
	}
	if (scanflag != 0) {
		plog("Scanning entire disk for bad blocks\n");
		ScanDisk(fsreadfd);
	}

	result = CheckHFS( filesys, fsreadfd, fswritefd, chkLev, repLev, context,
						lostAndFoundMode, canWrite, &fsmodified,
						lflag, rebuildOptions );
    if (debug)
		plog("\tCheckHFS returned %d, fsmodified = %d\n", result, fsmodified);

	if (!hotmount) {
		ckfini(1);
		if (quick) {
			if (result == 0) {
				pwarn("QUICKCHECK ONLY; FILESYSTEM CLEAN\n");
				result = 0;
				goto ExitThisRoutine;
			} else if (result == R_Dirty) {
				pwarn("QUICKCHECK ONLY; FILESYSTEM DIRTY\n");
				result = DIRTYEXIT;
				goto ExitThisRoutine;
			} else if (result == R_BadSig) {
				pwarn("QUICKCHECK ONLY; NO HFS SIGNATURE FOUND\n");
				result = DIRTYEXIT;
				goto ExitThisRoutine;
			} else {
				result = EEXIT;
				goto ExitThisRoutine;
			}
		}
	} else {
		struct statfs stfs_buf;

		/*
		 * Check to see if root is mounted read-write.
		 */
		if (statfs(mntonname, &stfs_buf) == 0)
			flags = stfs_buf.f_flags;
		else
			flags = 0;
		ckfini(flags & MNT_RDONLY);
	}

	/* XXX free any allocated memory here */

	if (hotmount && fsmodified) {
		struct hfs_mount_args args;
		/*
		 * We modified the root.  Do a mount update on
		 * it, unless it is read-write, so we can continue.
		 */
		if (!preen)
			fsckPrint(context, fsckVolumeModified);
		if (flags & MNT_RDONLY) {
			bzero(&args, sizeof(args));
			flags |= MNT_UPDATE | MNT_RELOAD;
			if (debug)
				fprintf(stderr, "doing update / reload mount for %s now\n", mntonname);
			if (mount("hfs", mntonname, flags, &args) == 0) {
				if (result != 0)
					result = EEXIT;
				goto ExitThisRoutine;
			} else {
				//if (debug)
					fprintf(stderr, "update/reload mount for %s failed: %s\n", mntonname, strerror(errno));
			}
		}
		if (!preen)
			plog("\n***** REBOOT NOW *****\n");
		sync();
		result = FIXEDROOTEXIT;
		goto ExitThisRoutine;
	}

	if (result != 0 && result != MAJOREXIT)
		result = EEXIT;
	
ExitThisRoutine:
	if (lflag) {
	    if (fs_fd >= 0) {
		fcntl(fs_fd, F_THAW_FS, NULL);
		close(fs_fd);
		fs_fd = -1;
	    }
	}
	if (mntonname)
		free(mntonname);

	if (context)
		fsckDestroy(context);

	return (result);
}


/*
 * Setup for I/O to device
 * Return 1 if successful, 0 if unsuccessful.
 * canWrite - 1 if we can safely write to the raw device or 0 if not.
 */
static int
setup( char *dev, int *canWritePtr )
{
	struct stat statb;
	int devBlockSize;
	uint32_t cacheBlockSize;
	uint32_t cacheTotalBlocks;
	int preTouchMem = 0;

	fswritefd = -1;
	*canWritePtr = 0;
    
    if (!detonator_run)
    {
        if (stat(dev, &statb) < 0) {
            plog("Can't stat %s: %s\n", dev, strerror(errno));
            return (0);
        }
        if ((statb.st_mode & S_IFMT) != S_IFCHR) {
            pfatal("%s is not a character device", dev);
            if (reply("CONTINUE") == 0)
                return (0);
        }
        /* Always attempt to replay the journal */
        if (!nflag && !quick) {
            // We know we have a character device by now.
            if (strncmp(dev, "/dev/rdisk", 10) == 0) {
                char block_device[MAXPATHLEN+1];
                int rv;
                snprintf(block_device, sizeof(block_device), "/dev/%s", dev + 6);
                rv = journal_replay(block_device);
                if (debug)
                    plog("journal_replay(%s) returned %d\n", block_device, rv);
            }
        }
        /* attempt to get write access to the block device and if not check if volume is */
        /* mounted read-only.  */
        if (nflag == 0 && quick == 0) {
            getWriteAccess( dev, canWritePtr );
        }
	
        if (nflag || quick || (fswritefd = open(dev, O_RDWR | (hotmount ? 0 : O_EXLOCK))) < 0) {
            fswritefd = -1;
            if (preen) {
                pfatal("** %s (NO WRITE ACCESS)\n", dev);
            }
        }
    } else { // detonator run
        plog("fsck_hfs: detonator_run (%s).\n", dev);
        char *end_ptr;
        fswritefd = (int)strtol(dev+8, &end_ptr, 10);
        if (*end_ptr)
        {
            err(1, "fsck_hfs: Invalid file descriptor path: %s", dev);
        }

        struct stat info;
        int error = fstat(fswritefd, &info);
        if (error)
        {
            err(1, "fsck_hfs: fstat %s", dev);
        }
        
        error = (int)lseek(fswritefd, 0, SEEK_SET);
        if (error == -1)
        {
            err(1, "fsck_hfs: Could not seek %d for dev: %s, errorno %d", fswritefd, dev, errno);
        }
        
        *canWritePtr = TRUE;
    }
    
	if (preen == 0 && !guiControl) {
		if (nflag || quick || fswritefd == -1) {
			plog("** %s (NO WRITE)\n", dev);
		} else {
			plog("** %s\n", dev);
		}
	}

	if (fswritefd == -1) {
		if ((fsreadfd = open(dev, O_RDONLY)) < 0) {
			plog("Can't open %s: %s\n", dev, strerror(errno));
			return (0);
		}
	} else {
		fsreadfd = dup(fswritefd);
		if (fsreadfd < 0) {
			plog("Can't dup fd for reading on %s: %s\n", dev, strerror(errno));
			close(fswritefd);
			return(0);
		}
	}

	/* Get device block size to initialize cache */
	if (ioctl(fsreadfd, DKIOCGETBLOCKSIZE, &devBlockSize) < 0) {
		pfatal ("Can't get device block size\n");
		return (0);
	}

	 /*
	  * Calculate the cache block size and total blocks.
	  *
	  * If a quick check was requested, we'll only be checking to see if
	  * the volume was cleanly unmounted or journalled, so we won't need
	  * a lot of cache.  Since lots of quick checks can be run in parallel
	  * when a new disk with several partitions comes on line, let's avoid
	  * the memory usage when we don't need it.
	  */
	if (reqCacheSize == 0 && quick == 0) {
		/*
		 * Auto-pick the cache size.  The cache code will deal with minimum
		 * maximum values, so we just need to find out the size of memory, and
		 * how much of it we'll use.
		 *
		 * If we're looking at the root device, and it's not a live verify (lflag),
		 * then we will use half of physical memory; otherwise, we'll use an eigth.
		 *
		 */
		uint64_t memSize;
		size_t dsize = sizeof(memSize);
		int rv;

		rv = sysctlbyname("hw.memsize", &memSize, &dsize, NULL, 0);
		if (rv == -1) {
			(void)fplog(stderr, "sysctlbyname failed, not auto-setting cache size\n");
		} else {
            int d = (hotroot && !lflag) ? 2 : 8;
            if (!detonator_run) {
                int safeMode = 0;
                dsize = sizeof(safeMode);
                rv = sysctlbyname("kern.safeboot", &safeMode, &dsize, NULL, 0);
                if (rv != -1 && safeMode != 0 && hotroot && !lflag) {
    #define kMaxSafeModeMem	((size_t)2 * 1024 * 1024 * 1024)	/* 2Gbytes, means cache will max out at 1gbyte */
                    if (debug) {
                        (void)fplog(stderr, "Safe mode and single-user, setting memsize to a maximum of 2gbytes\n");
                    }
                    memSize = (memSize < kMaxSafeModeMem) ? memSize : kMaxSafeModeMem;
                }
            }
			reqCacheSize = memSize / d;
		}
	}
	
	CalculateCacheSizes(reqCacheSize, &cacheBlockSize, &cacheTotalBlocks, debug);

	preTouchMem = (hotroot != 0) && (lflag != 0);
	/* Initialize the cache */
	if (CacheInit (&fscache, fsreadfd, fswritefd, devBlockSize,
			cacheBlockSize, cacheTotalBlocks, CacheHashSize, preTouchMem) != EOK) {
		pfatal("Can't initialize disk cache\n");

		return (0);
	}	

	return (1);
}


// This routine will attempt to open the block device with write access for the target 
// volume in order to block others from mounting the volume with write access while we
// check / repair it.  If we cannot get write access then we check to see if the volume
// has been mounted read-only.  If it is read-only then we should be OK to write to 
// the raw device.  Note that this does not protect use from someone upgrading the mount
// from read-only to read-write.

static void getWriteAccess( char *dev, int *canWritePtr )
{
	int					i;
	int					myMountsCount;
	void *				myPtr;
	char *				myCharPtr;
	struct statfs *			myBufPtr;
	void *				myNamePtr;
	int				blockDevice_fd = -1;

	myPtr = NULL;
	myNamePtr = malloc( strlen(dev) + 2 );
	if ( myNamePtr == NULL ) 
		return;
		
	strcpy( (char *)myNamePtr, dev );
	if ( (myCharPtr = strrchr( (char *)myNamePtr, '/' )) != 0 ) {
		if ( myCharPtr[1] == 'r' ) {
			memmove(&myCharPtr[1], &myCharPtr[2], strlen(&myCharPtr[2]) + 1);
			blockDevice_fd = open( (char *)myNamePtr, O_WRONLY | (hotmount ? 0 : O_EXLOCK) );
		}
	}
	
	if ( blockDevice_fd > 0 ) {
		// we got write access to the block device so we can safely write to raw device
		*canWritePtr = 1;
		goto ExitThisRoutine;
	}
	
	// get count of mounts then get the info for each 
	myMountsCount = getfsstat( NULL, 0, MNT_NOWAIT );
	if ( myMountsCount < 0 )
		goto ExitThisRoutine;

	myPtr = (void *) malloc( sizeof(struct statfs) * myMountsCount );
	if ( myPtr == NULL ) 
		goto ExitThisRoutine;
	myMountsCount = getfsstat( 	myPtr, 
								(int)(sizeof(struct statfs) * myMountsCount), 
								MNT_NOWAIT );
	if ( myMountsCount < 0 )
		goto ExitThisRoutine;

	myBufPtr = (struct statfs *) myPtr;
	for ( i = 0; i < myMountsCount; i++ )
	{
		if ( strcmp( myBufPtr->f_mntfromname, myNamePtr ) == 0 ) {
			if ( myBufPtr->f_flags & MNT_RDONLY )
				*canWritePtr = 1;
			goto ExitThisRoutine;
		}
		myBufPtr++;
	}
	*canWritePtr = 1;  // single user will get us here, f_mntfromname is not /dev/diskXXXX 
	
ExitThisRoutine:
	if ( myPtr != NULL )
		free( myPtr );
		
	if ( myNamePtr != NULL )
		free( myNamePtr );
	
	if (blockDevice_fd != -1) {
		close(blockDevice_fd);
	}

	return;
	
} /* getWriteAccess */


static void
usage()
{
	(void) fplog(stderr, "usage: %s [-b [size] B [path] c [size] e [mode] ESdfglx m [mode] npqruy] special-device\n", progname);
	(void) fplog(stderr, "  b size = size of physical blocks (in bytes) for -B option\n");
	(void) fplog(stderr, "  B path = file containing physical block numbers to map to paths\n");
	(void) fplog(stderr, "  c size = cache size (ex. 512m, 1g)\n");
	(void) fplog(stderr, "  e mode = emulate 'embedded' or 'desktop'\n");
	(void) fplog(stderr, "  E = exit on first major error\n");
	(void) fplog(stderr, "  d = output debugging info\n");
	(void) fplog(stderr, "  f = force fsck even if clean (preen only) \n");
	(void) fplog(stderr, "  g = GUI output mode\n");
	(void) fplog(stderr, "  x = XML output mode\n");
	(void) fplog(stderr, "  l = live fsck (lock down and test-only)\n");
	(void) fplog(stderr, "  m arg = octal mode used when creating lost+found directory \n");
	(void) fplog(stderr, "  n = assume a no response \n");
	(void) fplog(stderr, "  p = just fix normal inconsistencies \n");
	(void) fplog(stderr, "  q = quick check returns clean, dirty, or failure \n");
	(void) fplog(stderr, "  r = rebuild catalog btree \n");
	(void) fplog(stderr, "  S = Scan disk for bad blocks\n");
	(void) fplog(stderr, "  u = usage \n");
	(void) fplog(stderr, "  y = assume a yes response \n");
	
	exit(1);
}


static void
AddBlockToList(long long block)
{

	if ((gBlkListEntries % BLOCK_LIST_INCREMENT) == 0) {
		void *tmp;

//		gBlkListEntries += BLOCK_LIST_INCREMENT;
		tmp = realloc(gBlockList, (gBlkListEntries + BLOCK_LIST_INCREMENT) * sizeof(u_int64_t));
		if (tmp == NULL) {
			pfatal("Can't allocate memory for block list (%llu entries).\n", gBlkListEntries);
		}
		gBlockList = (u_int64_t*)tmp;
	}
	gBlockList[gBlkListEntries++] = block;
	return;
}

static int printStatus;
static void
siginfo(int signo)
{
	printStatus = 1;
}

static void
ScanDisk(int fd)
{
	uint32_t devBlockSize = 512;
	uint64_t devBlockTotal;
	off_t diskSize;
	uint8_t *buffer = NULL;
	size_t bufSize = 1024 * 1024;
	ssize_t nread;
	off_t curPos = 0;
	void (*oldhandler)(int);
	uint32_t numErrors = 0;
	uint32_t maxErrors = 40;	// Something more variable?

	oldhandler = signal(SIGINFO, &siginfo);

#define PRSTAT \
	do { \
		if (diskSize) { \
			fprintf(stderr, "Scanning offset %lld of %lld (%d%%)\n", \
				curPos, diskSize, (int)((curPos * 100) / diskSize)); \
		} else { \
			fprintf(stderr, "Scanning offset %lld\n", curPos); \
		} \
		printStatus = 0; \
	} while (0)

	if (ioctl(fd, DKIOCGETBLOCKSIZE, &devBlockSize) == -1) {
		devBlockSize = 512;
	}

	if (ioctl(fd, DKIOCGETBLOCKCOUNT, &devBlockTotal) == -1) {
		diskSize = 0;
	} else
		diskSize = devBlockTotal * devBlockSize;

	while (buffer == NULL && bufSize >= devBlockSize) {
		buffer = malloc(bufSize);
		if (buffer == NULL) {
			bufSize /= 2;
		}
	}
	if (buffer == NULL) {
		pfatal("Cannot allocate buffer for disk scan.\n");
	}

loop:

	if (printStatus) {
		PRSTAT;
	}
	while ((nread = pread(fd, buffer, bufSize, curPos)) == bufSize) {
		curPos += bufSize;
		if (printStatus) {
			PRSTAT;
		}
	}

	if (nread == 0) {
		/* We're done with the disk */
		goto done;
	}
	if (nread == -1) {
		if (errno == EIO) {
			/* Try reading devBlockSize blocks */
			size_t total;
			for (total = 0; total < bufSize; total += devBlockSize) {
				nread = pread(fd, buffer, devBlockSize, curPos + total);
				if (nread == -1) {
					if (errno == EIO) {
						if (debug)
							fprintf(stderr, "Bad block at offset %lld\n", curPos + total);
						AddBlockToList((curPos + total) / gBlockSize);
						if (++numErrors > maxErrors) {
							if (debug)
								fprintf(stderr, "Got %u errors, maxing out so stopping scan\n", numErrors);
							goto done;
						}
						continue;
					} else {
						pfatal("Got a non I/O error reading disk at offset %llu: %s\n",
							curPos + total, strerror(errno));
						// Hey, pfatal wasn't fatal!
						// But that seems to work out for us for some reason.
					}
				}
				if (nread == 0) {
					/* End of disk, somehow. */
					goto done;
				}
				if (nread != devBlockSize) {
					pwarn("During disk scan, did not get block size (%zd) read, got %zd instead.  Skipping rest of this block.\n", (size_t)devBlockSize, nread);
					continue;
				}
			}
			curPos += total;
			goto loop;
		} else if (errno == EINTR) {
			goto loop;
		} else {
			pfatal("Got a non I/O error reading disk at offset %llu:  %s\n", curPos, strerror(errno));
			exit(EEXIT);
		}
	}
	if (nread < bufSize) {
		if ((nread % devBlockSize) == 0) {
			curPos += nread;
		} else {
			curPos = curPos + (((nread % devBlockSize) + 1) * devBlockSize);
		}
		goto loop;
	}
	goto loop;
done:
	if (buffer)
		free(buffer);
	signal(SIGINFO, oldhandler);
	return;

}

static int
getblocklist(const char *filepath)
{
	FILE * file;
	long long block;
	size_t blockListCount;	/* Number of elements allocated to gBlockList array */
	
	blockListCount = BLOCK_LIST_INCREMENT;
	gBlockList = (u_int64_t *) malloc(blockListCount * sizeof(u_int64_t));
	if (gBlockList == NULL)
		pfatal("Can't allocate memory for block list.\n");
	
//	printf("getblocklist: processing blocklist %s...\n", filepath);

	if ((file = fopen(filepath, "r")) == NULL)
		pfatal("Can't open %s\n", filepath);

	while (fscanf(file, "%lli", &block) > 0) {
		AddBlockToList(block);
	}

	(void) fclose(file);

	printf("%d blocks to match:\n", gBlkListEntries);
	
	return (0);
}
