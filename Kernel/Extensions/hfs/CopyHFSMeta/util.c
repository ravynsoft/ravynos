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

/*
 * Open the source device.  In addition to opening the device,
 * this also attempts to flush the journal, and then sets up a
 * DeviceInfo_t object that will be used when doing the actual
 * reading.
 */
__private_extern__
DeviceInfo_t *
OpenDevice(const char *devname, int flushJournal)
{
	DeviceInfo_t *retval = NULL;
	int fd;
	DeviceInfo_t dev = { 0 };
	struct stat sb;
	struct vfsconf vfc;

	if (stat(devname, &sb) == -1) {
		err(kBadExit, "cannot open device %s", devname);
	}
	/*
	 * Attempt to flush the journal if requested.  If it fails, we just warn, but don't abort.
	 */
	if (flushJournal && getvfsbyname("hfs", &vfc) == 0) {
		int rv;
		int mib[4];
		char block_device[MAXPATHLEN+1];
		int jfd;

		/*
		 * The journal replay code, sadly, requires a block device.
		 * So we need to go from the raw device to block device, if
		 * necessary.
		 */
		if (strncmp(devname, "/dev/rdisk", 10) == 0) {
			snprintf(block_device, sizeof(block_device), "/dev/%s", devname+6);
		} else {
			snprintf(block_device, sizeof(block_device), "%s", devname);
		}
		jfd = open(block_device, O_RDWR);
		if (jfd == -1) {
			warn("Cannot open block device %s for read-write", block_device);
		} else {
			mib[0] = CTL_VFS;
			mib[1] = vfc.vfc_typenum;
			mib[2] = HFS_REPLAY_JOURNAL;
			mib[3] = jfd;
			if (debug)
				fprintf(stderr, "about to replay journal\n");
			rv = sysctl(mib, 4, NULL, NULL, NULL, 0);
			if (rv == -1) {
				warn("cannot replay journal");
			}
			/* This is probably not necessary, but we couldn't prove it. */
			(void)fcntl(jfd, F_FULLFSYNC, 0);
			close(jfd);
		}
	}
	/*
	 * We only allow a character device (e.g., /dev/rdisk1s2)
	 * If we're given a non-character device, we'll try to turn
	 * into a character device assuming a name pattern of /dev/rdisk*
	 */
	if ((sb.st_mode & S_IFMT) == S_IFCHR) {
		dev.devname = strdup(devname);
	} else if (strncmp(devname, "/dev/disk", 9) == 0) {
		// Turn "/dev/diskFoo" into "/dev/rdiskFoo"
		char tmpname[strlen(devname) + 2];
		(void)snprintf(tmpname, sizeof(tmpname), "/dev/rdisk%s", devname + sizeof("/dev/disk") - 1);
		if (stat(tmpname, &sb) == -1) {
			err(kBadExit, "cannot open raw device %s", tmpname);
		}
		if ((sb.st_mode & S_IFMT) != S_IFCHR) {
			errx(kBadExit, "raw device %s is not a raw device", tmpname);
		}
		dev.devname = strdup(tmpname);
	} else {
		errx(kBadExit, "device name `%s' does not fit pattern", devname);
	}
	// Open with a shared lock if we're not debugging, since the hfs estimator api is invoked from the booted system
	fd = open(dev.devname, O_RDONLY | (debug ? 0 : O_SHLOCK));
	if (fd == -1) {
		err(kBadExit, "cannot open raw device %s", dev.devname);
	}
	// Get the block size and counts for the device.
	if (ioctl(fd, DKIOCGETBLOCKSIZE, &dev.blockSize) == -1) {
		dev.blockSize = 512;	// Sane default, I hope
	}
	if (ioctl(fd, DKIOCGETBLOCKCOUNT, &dev.blockCount) == -1) {
		err(kBadExit, "cannot get size of device %s", dev.devname);
	}

	dev.size = dev.blockCount * dev.blockSize;
	dev.fd = fd;

	retval = malloc(sizeof(*retval));
	if (retval == NULL) {
		err(kBadExit, "cannot allocate device info structure");
	}
	*retval = dev;
	return retval;
}

/*
 * Get the header and alternate header for a device.
 */
__private_extern__
VolumeDescriptor_t *
VolumeInfo(DeviceInfo_t *devp)
{
	uint8_t buffer[devp->blockSize];
	VolumeDescriptor_t *vdp = NULL, vd = { 0 };
	ssize_t rv;

	vd.priOffset = 1024;	// primary volume header is at 1024 bytes
	vd.altOffset = devp->size - 1024;	// alternate header is 1024 bytes from the end

	rv = GetBlock(devp, vd.priOffset, buffer);
	if (rv == -1) {
		err(kBadExit, "cannot get primary volume header for device %s", devp->devname);
	}
	vd.priHeader = *(HFSPlusVolumeHeader*)buffer;

	rv = GetBlock(devp, vd.altOffset, buffer);
	if (rv == -1) {
		err(kBadExit, "cannot get alternate volume header for device %s", devp->devname);
	}
	vd.altHeader = *(HFSPlusVolumeHeader*)buffer;

	vdp = malloc(sizeof(*vdp));
	*vdp = vd;

	return vdp;
}

/*
 * Compare the primary and alternate volume headers.
 * We only care about the "important" bits (namely, the
 * portions related to extents).
 */
__private_extern__
int
CompareVolumeHeaders(VolumeDescriptor_t *vdp)
{
	int retval = -1;

#define CMP_FILE(v, f) memcmp(&(v)->priHeader.f, &(v)->altHeader.f, sizeof(v->priHeader.f))

	if (vdp &&
	    vdp->priHeader.journalInfoBlock == vdp->altHeader.journalInfoBlock &&
	    CMP_FILE(vdp, allocationFile) == 0 &&
	    CMP_FILE(vdp, extentsFile) == 0 &&
	    CMP_FILE(vdp, catalogFile) == 0 &&
	    CMP_FILE(vdp, attributesFile) == 0 &&
	    CMP_FILE(vdp, startupFile) == 0)
		retval = 0;
#undef CMP_FILE
	return retval;
}

/*
 * Only two (currently) types of signatures are valid: H+ and HX.
 */
static int
IsValidSigWord(uint16_t word) {
	if (word == kHFSPlusSigWord ||
	    word == kHFSXSigWord)
		return 1;
	return 0;
}

/*
 * Add the volume headers to the in-core volume information list.
 */
__private_extern__
int
AddHeaders(VolumeObjects_t *vop, int roundBlock)
{
	int retval = 1;
	HFSPlusVolumeHeader *hp;
	uint8_t buffer[vop->devp->blockSize];
	ssize_t rv;

	hp = &vop->vdp->priHeader;

	if (IsValidSigWord(S16(hp->signature)) == 0) {
		warnx("primary volume header signature = %x, invalid", S16(hp->signature));
		retval = 0;
	}
	if (roundBlock) {
		AddExtent(vop, 1024 / vop->devp->blockSize, vop->devp->blockSize);
	} else {
		AddExtent(vop, 1024, 512);
	}

	hp = &vop->vdp->altHeader;

	if (IsValidSigWord(S16(hp->signature)) == 0) {
		warnx("alternate volume header signature = %x, invalid", S16(hp->signature));
		retval = 0;
	}
	if (roundBlock) {
		AddExtent(vop, (vop->vdp->altOffset / vop->devp->blockSize) * vop->devp->blockSize, vop->devp->blockSize);
	} else {
		AddExtent(vop, vop->vdp->altOffset, 512);
	}

done:
	return retval;
}

/*
 * Add the journal information to the in-core volume list.
 * This means the journal info block, the journal itself, and
 * the contents of the same as described by the alternate volume
 * header (if it's different from the primary volume header).
 */
__private_extern__
void
AddJournal(VolumeObjects_t *vop)
{
	DeviceInfo_t *devp = vop->devp;
	uint8_t buffer[devp->blockSize];
	ssize_t rv;
	HFSPlusVolumeHeader *php, *ahp;
	JournalInfoBlock *jib;

	php = &vop->vdp->priHeader;
	ahp = &vop->vdp->altHeader;

	if (php->journalInfoBlock) {
		off_t jOffset = (off_t)S32(php->journalInfoBlock) * S32(php->blockSize);
		rv = GetBlock(devp, jOffset, buffer);
		if (rv == -1) {
			err(kBadExit, "cannot get primary header's copy of journal info block");
		}
		AddExtent(vop, jOffset, sizeof(buffer));
		jib = (JournalInfoBlock*)buffer;
		if (S32(jib->flags) & kJIJournalInFSMask) {
			AddExtent(vop, S64(jib->offset), S64(jib->size));
		}
	}

	if (ahp->journalInfoBlock &&
	    ahp->journalInfoBlock != php->journalInfoBlock) {
		off_t jOffset = (off_t)S32(ahp->journalInfoBlock) * S32(ahp->blockSize);
		rv = GetBlock(devp, jOffset, buffer);
		if (rv == -1) {
			err(kBadExit, "cannot get alternate header's copy of journal info block");
		}
		AddExtent(vop, jOffset, sizeof(buffer));
		jib = (JournalInfoBlock*)buffer;
		if (S32(jib->flags) & kJIJournalInFSMask) {
			AddExtent(vop, S64(jib->offset), S64(jib->size));
		}
	}

}

/*
 * Add the extents for the special files in the volume header.  Compare
 * them with the alternate volume header's versions, and if they're different,
 * add that as well.
 */
__private_extern__
void
AddFileExtents(VolumeObjects_t *vop)
{
	int useAlt = 0;
#define ADDEXTS(vop, file, fid)			\
	do { \
		off_t pSize = S32(vop->vdp->priHeader.blockSize);	\
		off_t aSize = S32(vop->vdp->altHeader.blockSize);	\
		int i;							\
		if (debug) printf("Adding " #file " extents\n");		\
		for (i = 0; i < kHFSPlusExtentDensity; i++) {		\
			HFSPlusExtentDescriptor *ep = &vop->vdp->priHeader. file .extents[i]; \
			HFSPlusExtentDescriptor *ap = &vop->vdp->altHeader. file .extents[i]; \
			if (debug) printf("\tExtent <%u, %u>\n", S32(ep->startBlock), S32(ep->blockCount)); \
			if (ep->startBlock && ep->blockCount) {		\
				AddExtentForFile(vop, S32(ep->startBlock) * pSize, S32(ep->blockCount) * pSize, fid); \
				if (memcmp(ep, ap, sizeof(*ep)) != 0) { \
					AddExtentForFile(vop, S32(ap->startBlock) * aSize, S32(ap->blockCount) * aSize, fid); \
					useAlt = 1;			\
				}					\
			}						\
		}							\
	} while (0)

	ADDEXTS(vop, allocationFile, kHFSAllocationFileID);
	ADDEXTS(vop, extentsFile, kHFSExtentsFileID);
	ADDEXTS(vop, catalogFile, kHFSCatalogFileID);
	ADDEXTS(vop, attributesFile, kHFSAttributesFileID);
	ADDEXTS(vop, startupFile, kHFSStartupFileID);

#undef ADDEXTS

	ScanExtents(vop, 0);
	if (useAlt)
		ScanExtents(vop, useAlt);

	return;
}

static int
ScanCatalogNode(VolumeObjects_t *vop, uint8_t *buffer, size_t nodeSize, extent_handler_t handler)
{
	BTNodeDescriptor *ndp = (BTNodeDescriptor *)buffer;
	uint16_t *indices = (uint16_t*)(buffer + nodeSize);
	size_t counter;
	off_t blockSize = S32(vop->vdp->priHeader.blockSize);
	int retval = 0;

	if (ndp->kind != kBTLeafNode)	// Skip if it's not a leaf node
		return 0;

	if (debug)
		fprintf(stderr, "%s:  scanning catalog node\n", __FUNCTION__);

	for (counter = 1; counter <= S16(ndp->numRecords); counter++) {
		// Need to get past the end of the key
		uint16_t recOffset = S16(indices[-counter]);
		HFSPlusCatalogKey *keyp = (HFSPlusCatalogKey*)(buffer + recOffset);
		size_t keyLength = S16(keyp->keyLength);
		// Add two because the keyLength field is not included.
		HFSPlusCatalogFile *fp = (HFSPlusCatalogFile*)(((uint8_t*)keyp) +  2 + keyLength + (keyLength & 1));

		if (S16(fp->recordType) != kHFSPlusFileRecord) {
			if (debug)
				fprintf(stderr, "%s:  skipping node record %zu because it is type %#x, at offset %u keyLength %zu\n", __FUNCTION__, counter, S16(fp->recordType), recOffset, keyLength);
			continue;
		}

		if (debug)
			fprintf(stderr, "%s:  node record %zu, file id = %u\n", __FUNCTION__, counter, S32(fp->fileID));
		if (S32(fp->userInfo.fdType) == kSymLinkFileType &&
		    S32(fp->userInfo.fdCreator) == kSymLinkCreator) {
			unsigned int fid = S32(fp->fileID);
			HFSPlusExtentDescriptor *extPtr = fp->dataFork.extents;
			int i;

			for (i = 0; i < 8; i++) {
				if (extPtr[i].startBlock &&
				    extPtr[i].blockCount) {
					off_t start = blockSize * S32(extPtr[i].startBlock);
					off_t length = blockSize * S32(extPtr[i].blockCount);
					retval = handler(fid, start, length);
					if (retval != 0)
						return retval;
				} else {
					break;
				}
			}
		}
	}
	return retval;
}

static int
ScanAttrNode(VolumeObjects_t *vop, uint8_t *buffer, size_t nodeSize, extent_handler_t handler)
{
	BTNodeDescriptor *ndp = (BTNodeDescriptor *)buffer;
	uint16_t *indices = (uint16_t*)(buffer + nodeSize);
	size_t counter;
	off_t blockSize = S32(vop->vdp->priHeader.blockSize);
	int retval = 0;

	if (ndp->kind != kBTLeafNode)
		return 0;	// Skip if it's not a leaf node

	/*
	 * Look for records of type kHFSPlusForkData and kHFSPlusAttrExtents
	 */
	for (counter = 1; counter <= S16(ndp->numRecords); counter++) {
		// Need to get past the end of the key
		unsigned int fid;
		HFSPlusAttrKey *keyp = (HFSPlusAttrKey*)(buffer + S16(indices[-counter]));
		size_t keyLength = S16(keyp->keyLength);
		// Add two because the keyLength field is not included.
		HFSPlusAttrRecord *ap = (HFSPlusAttrRecord*)(((uint8_t*)keyp) + 2 + keyLength + (keyLength & 1));
		HFSPlusExtentDescriptor *theExtents = NULL;
		switch (S32(ap->recordType)) {
		case kHFSPlusAttrForkData:
			theExtents = ap->forkData.theFork.extents;
			break;
		case kHFSPlusAttrExtents:
			theExtents = ap->overflowExtents.extents;
			break;
		default:
			break;
		}
		if (theExtents != NULL) {
			HFSPlusExtentDescriptor *extPtr = theExtents;
			int i;
			fid = S32(keyp->fileID);

			for (i = 0; i < 8; i++) {
				if (extPtr[i].startBlock &&
				    extPtr[i].blockCount) {
					off_t start = blockSize * S32(extPtr[i].startBlock);
					off_t length = blockSize * S32(extPtr[i].blockCount);
					retval = handler(fid, start, length);
					if (retval != 0)
						return retval;
				} else {
					break;
				}
			}
		}
	}
	return retval;
}


/*
 * Given a VolumeObject_t, search for the other metadata that
 * aren't described by the system files, but rather in the
 * system files.  This includes symbolic links, and large EA
 * extents.  We can do this at one of two times -- while copying
 * the data, or while setting up the list of extents.  The
 * former is going to be more efficient, but the latter will
 * mean the estimates and continuation will be less likely to
 * be wrong as we add extents to the list.
 */
__private_extern__
int
FindOtherMetadata(VolumeObjects_t *vop, extent_handler_t handler)
{
	size_t catNodeSize = 0, attrNodeSize = 0;
	off_t node0_location = 0;
	uint8_t *tBuffer;
	BTHeaderRec *hdp;
	BTNodeDescriptor *ndp;
	int retval = 0;

	tBuffer = calloc(1, vop->devp->blockSize);
	if (tBuffer == NULL) {
		warn("Could not allocate memory to collect extra metadata");
		goto done;
	}
	/*
	 * First, do the catalog file
	 */
	if (vop->vdp->priHeader.catalogFile.logicalSize) {

		node0_location = S32(vop->vdp->priHeader.catalogFile.extents[0].startBlock);
		node0_location = node0_location * S32(vop->vdp->priHeader.blockSize);
		if (GetBlock(vop->devp, node0_location, tBuffer) == -1) {
			warn("Could not read catalog header node");
		} else {
			ndp = (BTNodeDescriptor*)tBuffer;
			hdp = (BTHeaderRec*)(tBuffer + sizeof(BTNodeDescriptor));
			
			if (ndp->kind != kBTHeaderNode) {
				warnx("Did not read header node for catalog as expected");
			} else {
				catNodeSize = S16(hdp->nodeSize);
			}
		}
	}
	/*
	 * Now, the attributes file.
	 */
	if (vop->vdp->priHeader.attributesFile.logicalSize) {

		node0_location = S32(vop->vdp->priHeader.attributesFile.extents[0].startBlock);
		node0_location = node0_location * S32(vop->vdp->priHeader.blockSize);
		if (GetBlock(vop->devp, node0_location, tBuffer) == -1) {
			warn("Could not read attributes file header node");
		} else {
			ndp = (BTNodeDescriptor*)tBuffer;
			hdp = (BTHeaderRec*)(tBuffer + sizeof(BTNodeDescriptor));
			
			if (ndp->kind != kBTHeaderNode) {
				warnx("Did not read header node for attributes file as expected");
			} else {
				attrNodeSize = S16(hdp->nodeSize);
			}
		}
	}
	if (debug)
		fprintf(stderr, "Catalog node size = %zu, attributes node size = %zu\n", catNodeSize, attrNodeSize);

	/*
	 * We start reading the extents now.
	 *
	 * This is a lot of duplicated code, unfortunately.
	 */
	ExtentList_t *exts;
	for (exts = vop->list;
	     exts;
	     exts = exts->next) {
		size_t indx;
		
		for (indx = 0; indx < exts->count; indx++) {
			off_t start = exts->extents[indx].base;
			off_t len = exts->extents[indx].length;
			off_t nread = 0;
			if (exts->extents[indx].fid == 0) {
				continue;	// Unknown file, skip
			} else {
				if (debug) fprintf(stderr, "%s:  fid = %u, start = %llu, len = %llu\n", __FUNCTION__, exts->extents[indx].fid, start, len);
				while (nread < len) {
					size_t bufSize;
					uint8_t *buffer;
					bufSize = MIN(len - nread, 1024 * 1024);	// Read 1mbyte max
					buffer = calloc(1, bufSize);
					if (buffer == NULL) {
						warn("Cannot allocate %zu bytes for buffer, skipping node scan", bufSize);
					} else {
						ssize_t t = UnalignedRead(vop->devp, buffer, bufSize, start + nread);
						if (t != bufSize) {
							warn("Attempted to read %zu bytes, only read %zd, skipping node scan", bufSize, t);
						} else {
							uint8_t *curPtr = buffer, *endPtr = (buffer + bufSize);
							size_t nodeSize = 0;
							int (*func)(VolumeObjects_t *, uint8_t *, size_t, extent_handler_t) = NULL;
							if (exts->extents[indx].fid == kHFSCatalogFileID) {
								func = ScanCatalogNode;
								nodeSize = catNodeSize;
							} else if (exts->extents[indx].fid == kHFSAttributesFileID) {
								func = ScanAttrNode;
								nodeSize = attrNodeSize;
							}
							if (func) {
								while (curPtr < endPtr && retval == 0) {
									retval = (*func)(vop, curPtr, nodeSize, handler);
									curPtr += nodeSize;
								}
							}
						}
						free(buffer);
					}
					if (retval != 0)
						goto done;
					nread += bufSize;
				}
			}
		}
	}

done:
	if (tBuffer)
		free(tBuffer);
	return retval;
}

/*
 * Perform a (potentially) unaligned read from a given input device.
 */
__private_extern__
ssize_t
UnalignedRead(DeviceInfo_t *devp, void *buffer, size_t size, off_t offset)
{
	ssize_t nread = -1;
	size_t readSize = ((size + devp->blockSize - 1) / devp->blockSize) * devp->blockSize;
	off_t baseOffset = (offset / devp->blockSize) * devp->blockSize;
	size_t off = offset - baseOffset;
	char *tmpbuf = NULL;
	
	if ((baseOffset == offset) && (readSize == size)) {
		/*
		 * The read is already properly aligned, so call pread.
		 */
		return pread(devp->fd, buffer, size, offset);
	}
	
	tmpbuf = malloc(readSize);
	if (!tmpbuf) {
		goto done;
	}
	
	nread = pread(devp->fd, tmpbuf, readSize, baseOffset);
	if (nread == -1) {
		goto done;
	}
	
	nread -= off;
	if (nread > (ssize_t)size) {
		nread = size;
	}
	memcpy(buffer, tmpbuf + off, nread);
	
done:
	free(tmpbuf);
	return nread;
}

__private_extern__
void
ReleaseDeviceInfo(DeviceInfo_t *devp)
{
	if (devp) {
		if (devp->fd != -1) {
			close(devp->fd);
		}
		if (devp->devname)
			free(devp->devname);
		free(devp);
	}
	return;
}

__private_extern__
void
ReleaseVolumeDescriptor(VolumeDescriptor_t *vdp)
{
	if (vdp)
		free(vdp);	// No contained pointers!
	return;
}

__private_extern__
void
ReleaseVolumeObjects(VolumeObjects_t *vop)
{
	if (vop) {
		if (vop->devp) {
			ReleaseDeviceInfo(vop->devp);
		}
		if (vop->vdp) {
			ReleaseVolumeDescriptor(vop->vdp);
		}
		ExtentList_t *extList;
		for (extList = vop->list;
		     extList;
			) {
			ExtentList_t *next = extList->next;
			free(extList);
			extList = next;
		}
		free(vop);
	}
}
