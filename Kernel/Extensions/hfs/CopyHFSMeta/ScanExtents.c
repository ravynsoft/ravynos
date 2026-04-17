#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <string.h>

#include "hfsmeta.h"
#include "Data.h"

#ifndef MAX
# define MAX(a, b) \
	({ __typeof(a) __a = (a); __typeof(b) __b = (b); __a > __b ? __a : __b; })
#endif

/*
 * Functions to scan through the extents overflow file, grabbing
 * overflow extents for the special files.
 */

/*
 * Given an extent record, return the logical block address (in the volume)
 * for the requested block offset into the file.  It returns 0 if it can't
 * find it.
 */
static unsigned int
FindBlock(HFSPlusExtentRecord *erp, unsigned int blockNum)
{
	unsigned int lba = 0;
	unsigned int base = 0;
	HFSPlusExtentDescriptor *ep = &(*erp)[0];
	int i;

	for (i = 0; i < kHFSPlusExtentDensity; i++) {
		if (ep->startBlock == 0 || ep->blockCount == 0)
			break;
		if ((base + S32(ep->blockCount)) > blockNum) {
			lba = S32(ep->startBlock) + (blockNum - base);
			break;
		}
		base += S32(ep->blockCount);
		ep++;
	}
	return lba;
}

/*
 * Get the given node from the extents-overflow file.  Returns -1 on error, and
 * 0 on success.
 */
static int
GetNode(DeviceInfo_t *devp, HFSPlusVolumeHeader *hp, int nodeNum, size_t nodeSize, void *nodePtr)
{
	int retval = 0;
	unsigned char *ptr, *endPtr;
	unsigned int offset;
	HFSPlusExtentRecord *erp = &hp->extentsFile.extents;
	size_t bufferSize = MAX(nodeSize, S32(hp->blockSize));

	ptr = nodePtr;
	endPtr = ptr + bufferSize;
	/*
	 * The block number for HFS Plus is guaranteed to be 32 bits.
	 * But the multiplication could over-flow, so we cast one
	 * of the variables to off_t, and cast the whole thing back
	 * to uint32_t.
	 */
	offset = (uint32_t)(((off_t)nodeNum * nodeSize) / S32(hp->blockSize));

	/*
	 * We have two sizes to consider here:  the device blocksize, and the
	 * buffer size.  The buffer size is the larger of the btree node size
	 * and the volume allocation block size.
	 *
	 * This loop is the case where the device block size is smaller than
	 * the amount we want to read (in a common case, 8kbyte node size, and
	 * 512 byte block size).  It reads in a device block, and adds it to
	 * the buffer; it continues until an error, or until it has gotten
	 * the amount it needs.
	 *
	 * The variable "offset" is in *allocation blocks*, not *bytes*.
	 */
	while (ptr < endPtr) {
		ssize_t rv;
		off_t lba;
		unsigned int i;


		lba = FindBlock(erp, offset);
		if (lba == 0) {
			warnx("Cannot find block %u in extents overflow file", offset);
			return -1;
		}
		lba = lba * S32(hp->blockSize);
		for (i = 0; i < S32(hp->blockSize) / devp->blockSize; i++) {
//			printf("Trying to get block %lld\n", lba + i);
			rv = GetBlock(devp, lba + (i * devp->blockSize), ptr);
			if (rv == -1) {
				warnx("Cannot read block %llu in extents overflow file", lba + i);
				return -1;
			}
			ptr += devp->blockSize;
		}
		offset++;
	}

	/*
	 * Per 13080856:  if the node size is less than the allocation block size, we
	 * have to deal with having multiple nodes per block.  The code above to read it
	 * has read in either an allocation block, or a node block, depending on which
	 * is larger.  If the allocation block is larger, then we have to move the node
	 * we want to the beginning of the buffer.
	 */
	if (nodeSize < bufferSize) {
		size_t indx = nodeNum % (S32(hp->blockSize) / nodeSize);
		ptr = nodePtr;
		memmove(ptr, ptr + (indx * nodeSize), nodeSize);
	}
	return retval;
}

/*
 * Scan through an extentes overflow node, looking for File ID's less than
 * the first user file ID.  For each one it finds, it adds the extents to
 * the volume structure list.  It returns the number of the next node
 * (which will be 0 when we've hit the end of the list); it also returns 0
 * when it encounters a CNID larger than the system files'.
 */
static unsigned int
ScanNode(VolumeObjects_t *vop, uint8_t *nodePtr, size_t nodeSize, off_t blockSize)
{
	u_int16_t *offsetPtr;
	BTNodeDescriptor *descp;
	int indx;
	int numRecords;
	HFSPlusExtentKey *keyp;
	HFSPlusExtentRecord *datap;
	uint8_t *recp;
	unsigned int retval;

	descp = (BTNodeDescriptor*)nodePtr;

	if (descp->kind != kBTLeafNode)
		return 0;

	numRecords = S16(descp->numRecords);
	offsetPtr = (u_int16_t*)((uint8_t*)nodePtr + nodeSize);

	retval = S32(descp->fLink);
	for (indx = 1; indx <= numRecords; indx++) {
		int recOffset = S16(offsetPtr[-indx]);
		recp = nodePtr + recOffset;
		if (recp > (nodePtr + nodeSize)) {
			return -1;	// Corrupt node
		}
		keyp = (HFSPlusExtentKey*)recp;
		datap = (HFSPlusExtentRecord*)(recp + sizeof(HFSPlusExtentKey));
		if (debug > 1) printf("Node index #%d:  fileID = %u\n", indx, S32(keyp->fileID));
		if (S32(keyp->fileID) >= kHFSFirstUserCatalogNodeID) {
			if (debug) printf("Done scanning extents overflow file\n");
			retval = 0;
			break;
		} else {
			int i;
			for (i = 0; i < kHFSPlusExtentDensity; i++) {
				off_t start = S32((*datap)[i].startBlock) * (off_t)blockSize;
				off_t len = S32((*datap)[i].blockCount) * (off_t)blockSize;
				if (start && len)
					AddExtentForFile(vop, start, len, S32(keyp->fileID));
			}
		}
	}

	return retval;
}

/*
 * Given a volme structure list, scan through the extents overflow file
 * looking for system-file extents (those with a CNID < 16).  If useAltHdr
 * is set, it'll use the extents overflow descriptor in the alternate header.
 */
__private_extern__
int
ScanExtents(VolumeObjects_t *vop, int useAltHdr)
{
	int retval = -1;
	ssize_t rv;
	uint8_t buffer[vop->devp->blockSize];
	struct RootNode {
		BTNodeDescriptor desc;
		BTHeaderRec header;
	} *headerNode;
	HFSPlusVolumeHeader *hp;
	off_t vBlockSize;
	size_t nodeSize;
	size_t bufferSize;
	void *nodePtr = NULL;
	unsigned int nodeNum = 0;

	hp = useAltHdr ? &vop->vdp->altHeader : & vop->vdp->priHeader;
	vBlockSize = S32(hp->blockSize);

	rv = GetBlock(vop->devp, S32(hp->extentsFile.extents[0].startBlock) * vBlockSize, buffer);
	if (rv == -1) {
		warnx("Cannot get btree header node for extents file for %s header", useAltHdr ? "alternate" : "primary");
		retval = -1;
		goto done;
	}
	headerNode = (struct RootNode*)buffer;

	if (headerNode->desc.kind != kBTHeaderNode) {
		warnx("Root node is not a header node (%x)", headerNode->desc.kind);
		goto done;
	}

	nodeSize = S16(headerNode->header.nodeSize);
	/*
	 * There are three cases here:
	 * nodeSize == vBlockSize;
	 * nodeSize > vBlockSize;
	 * nodeSize < vBlockSize.
	 * For the first two, everything is easy:  we just need to read in a nodeSize, and that
	 * contains everything we care about.  For the third case, however, we will
	 * need to read in an allocation block size, but then have GetNode move memory
	 * around so the node we want is at the beginning of the buffer.  Note that this
	 * does mean it is less efficient than it should be.
	 */
	if (nodeSize < vBlockSize) {
		bufferSize = vBlockSize;
	} else {
		bufferSize = nodeSize;
	}

	nodePtr = malloc(bufferSize);
	if (nodePtr == NULL) {
		warn("cannot allocate buffer for node");
		goto done;
	}
	nodeNum = S32(headerNode->header.firstLeafNode);

	if (debug) printf("first leaf nodenum = %u\n", nodeNum);

	/*
	 * Iterate through the leaf nodes.
	 */
	while (nodeNum != 0) {
		if (debug) printf("Getting node %u\n", nodeNum);

		/*
		 * GetNode() puts the node we want into nodePtr;
		 * we have ensured that the buffer is large enough
		 * to contain at least one node, or one allocation block,
		 * whichever is larger.
		 */
		rv = GetNode(vop->devp, hp, nodeNum, nodeSize, nodePtr);
		if (rv == -1) {
			warnx("Cannot get node %u", nodeNum);
			retval = -1;
			goto done;
		}
		nodeNum = ScanNode(vop, nodePtr, nodeSize, vBlockSize);
	}
	retval = 0;

done:
	if (nodePtr)
		free(nodePtr);
	return retval;

}
