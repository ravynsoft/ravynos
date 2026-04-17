#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <err.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/disk.h>
#include <sys/sysctl.h>
#include <hfs/hfs_mount.h>
#include <Block.h>
#include "hfsmeta.h"
#include "Data.h"
#include "Sparse.h"

/*
 * Used to automatically run a corruption program after the
 * copying is done.  Only used during development.  Uncomment
 * to use.
 */
//#define TESTINJECT	1

static const char *kAppleInternal = "/AppleInternal";
static const char *kTestProgram = "HC-Inject-Errors";

int verbose;
int debug;
int printProgress;


/*
 * Compare two volume headers to see if they're the same.  Some fields
 * we may not care about, so we only compare specific fields.  Note that
 * since we're looking for equality, we don't need to byte swap.
 * (The function CompareVolumeHeaders() will compare the two volume
 * headers to see if the extents they describe are the same.)
 */
static int
CheckVolumeHeaders(HFSPlusVolumeHeader *left, HFSPlusVolumeHeader *right)
{
	if (left->signature != right->signature ||
	    left->version != right->version ||
	    left->modifyDate != right->modifyDate ||
	    left->fileCount != right->fileCount ||
	    left->folderCount != right->folderCount ||
	    left->nextAllocation != right->nextAllocation ||
	    left->nextCatalogID != right->nextCatalogID ||
	    left->writeCount != right->writeCount)
		return 0;
	return 1;
}

static void
usage(const char *progname)
{

	errx(kBadExit, "usage: %s [-vdpS] [-g gatherFile] [-C] [-r <bytes>] <src device> <destination>", progname);
}

int
main(int ac, char **av)
{
	char *src = NULL;
	char *dst = NULL;
	DeviceInfo_t *devp = NULL;
	VolumeDescriptor_t *vdp = NULL;
	VolumeObjects_t *vop = NULL;
	int ch;
	off_t restart = 0;
	int printEstimate = 0;
	const char *progname = av[0];
	char *gather = NULL;
	int force = 0;
	int retval = kGoodExit;
	int find_all_metadata = 0;

	while ((ch = getopt(ac, av, "fvdg:Spr:CA")) != -1) {
		switch (ch) {
		case 'A':	find_all_metadata = 1; break;
		case 'v':	verbose++; break;
		case 'd':	debug++; verbose++; break;
		case 'S':	printEstimate = 1; break;
		case 'p':	printProgress = 1; break;
		case 'r':	restart = strtoull(optarg, NULL, 0); break;
		case 'g':	gather = strdup(optarg); break;
		case 'f':	force = 1; break;
		default:	usage(progname);
		}
	}

	ac -= optind;
	av += optind;

	if (ac == 0 || ac > 2) {
		usage(progname);
	}
	src = av[0];
	if (ac == 2)
		dst = av[1];

	// Start by opening the input device
	devp = OpenDevice(src, 1);
	if (devp == NULL) {
		errx(kBadExit, "cannot get device information for %s", src);
	}

	// Get the volume information.
	vdp = VolumeInfo(devp);

	// Start creating the in-core volume list
	vop = InitVolumeObject(devp, vdp);

	// Add the volume headers
	if (AddHeaders(vop, 0) == 0) {
		errx(kBadExit, "Invalid volume header(s) for %s", src);
	}
	// Add the journal and file extents
	AddJournal(vop);
	AddFileExtents(vop);

	/*
	 * find_all_metadata requires scanning through
	 * the catalog and attributes files, looking for
	 * other bits treated as metadata.
	 */
	if (find_all_metadata)
		FindOtherMetadata(vop, ^(int fid, off_t start, off_t len) {
				AddExtentForFile(vop, start, len, fid);
//				fprintf(stderr, "AddExtentForFile(%p, %llu, %llu, %u)\n", vop, start, len, fid);
				return 0;
			});

	if (debug)
		PrintVolumeObject(vop);

	if (printEstimate) {
		printf("Estimate %llu\n", vop->byteCount);
	}

	// Create a gatherHFS-compatible file, if requested.
	if (gather) {
		WriteGatheredData(gather, vop);
	}

	/*
	 * If we're given a destination, initialize it.
 	 */
	if (dst) {
		IOWrapper_t *wrapper = InitSparseBundle(dst, devp);

		if (wrapper == NULL) {
			err(kBadExit, "cannot initialize destination container %s", dst);
		}

		// See if we're picking up from a previous copy
		if (restart == 0) {
			restart = wrapper->getprog(wrapper);
			if (debug) {
				fprintf(stderr, "auto-restarting at offset %lld\n", restart);
			}
		}
		// "force" in this case means try even if the space estimate says we won't succeed.
		if (force == 0) {
			struct statfs sfs;
			if (statfs(dst, &sfs) != -1) {
				off_t freeSpace = (off_t)sfs.f_bsize * (off_t)sfs.f_bfree;
				if (freeSpace < (vop->byteCount - restart)) {
					errx(kNoSpaceExit, "free space (%lld) < required space (%lld)", freeSpace, vop->byteCount - restart);
				}
			}
		}

		/*
		 * If we're restarting, we need to compare the volume headers and see if
		 * they're the same.  If they're not, we need to start from the beginning.
		 */
		if (restart) {
			HFSPlusVolumeHeader priHeader, altHeader;

			if (wrapper->reader(wrapper, 1024, &priHeader, sizeof(priHeader)) != -1) {
				if (CheckVolumeHeaders(&priHeader, &vop->vdp->priHeader) == 0) {
					restart = 0;
				} else {
					if (wrapper->reader(wrapper, vop->vdp->altOffset, &altHeader, sizeof(altHeader)) != -1) {
						if (CheckVolumeHeaders(&altHeader, &vop->vdp->altHeader) == 0) {
							restart = 0;
						}
					}
				}
			}
			if (restart == 0) {
				if (verbose)
					warnx("Destination volume does not match source, starting from beginning");
			}
		}

		// And start copying the objects.
		if (CopyObjectsToDest(vop, wrapper, restart) == -1) {
			if (errno == EIO)
				retval = kCopyIOExit;
			else if (errno == EINTR)
				retval = kIntrExit;
			else
				retval = kBadExit;
			err(retval, "CopyObjectsToDest failed");
		} else {
#if TESTINJECT
			// Copy finished, let's see if we should run a test program
			if (access(kAppleInternal, 0) != -1) {
				char *home = getenv("HOME");
				if (home) {
					char *pName;
					pName = malloc(strlen(home) + strlen(kTestProgram) + 2);	// '/' and NUL
					if (pName) {
						sprintf(pName, "%s/%s", home, kTestProgram);
						execl(pName, kTestProgram, dst, NULL);
					}
				}
			}
#endif
		}
	}

	return retval;
}

