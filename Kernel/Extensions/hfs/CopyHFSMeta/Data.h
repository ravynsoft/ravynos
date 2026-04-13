#ifndef _DATA_H
# define _DATA_H

# include <errno.h>
/*
 * Exit status values.  We use some errno values because
 * they are convenient.
 * kGoodExit:  we finished copying, and no problems.
 * kNoSpaceExit:  Not enough space for the skeleton copy.
 * kCopyIOExit:  An I/O error occurred.  This may not be fatal,
 *	as it may just mean the source device went away.  We
 *	can continue later, perhaps.
 * kIntrExit:  The copying was interrupted.  As above, this may
 *	not be fatal.
 * kBadExit:  Any other problem.
 */
enum {
	kGoodExit = 0,
	kNoSpaceExit = ENOSPC,
	kCopyIOExit = EIO,
	kIntrExit = EINTR,
	kBadExit = 1,
};

/*
 * Data-manipulating functions and structures, used to
 * create the skeleton copy.
 */
struct DeviceInfo;
struct VolumeDescriptor;
struct IOWrapper;

/*
 * We treat each data structure in the filesystem as
 * a <start, length> pair.
 */
struct Extents {
	off_t base;
	off_t length;
	unsigned int fid;	// Optional, may not be set
};
typedef struct Extents Extents_t;

#define kExtentCount	100

/*
 * The in-core representation consists of a linked
 * list of an array of extents, up to 100 in each element.
 */
struct ExtentList {
	size_t count;
	Extents_t extents[kExtentCount];
	struct ExtentList *next;
};
typedef struct ExtentList ExtentList_t;
/*
 * The in-core description of the volume:  an input source,
 * a description of the volume, the linked list of extents,
 * the total number of bytes, and the number of linked list
 * elements.
 */
struct VolumeObjects {
	struct DeviceInfo *devp;
	struct VolumeDescriptor *vdp;
	size_t count;
	off_t byteCount;
	ExtentList_t *list;
};
typedef struct VolumeObjects VolumeObjects_t;

typedef int (^extent_handler_t)(int fid, off_t start, off_t len);

extern VolumeObjects_t *InitVolumeObject(struct DeviceInfo *devp, struct VolumeDescriptor *vdp);
extern int AddExtent(VolumeObjects_t *vop, off_t start, off_t length);
extern int AddExtentForFile(VolumeObjects_t *vop, off_t start, off_t length, unsigned int fid);
extern void PrintVolumeObject(VolumeObjects_t*);
extern int CopyObjectsToDest(VolumeObjects_t*, struct IOWrapper *wrapper, off_t skip);

extern void WriteGatheredData(const char *, VolumeObjects_t*);

extern struct DeviceInfo *OpenDevice(const char *, int);
extern struct VolumeDescriptor *VolumeInfo(struct DeviceInfo *);
extern int AddHeaders(VolumeObjects_t *, int);
extern void AddJournal(VolumeObjects_t *);
extern void AddFileExtents(VolumeObjects_t *);
extern int FindOtherMetadata(VolumeObjects_t *, extent_handler_t);
extern int CompareVolumeHeaders(struct VolumeDescriptor*);

void ReleaseDeviceInfo(struct DeviceInfo *);
void ReleaseVolumeDescriptor(struct VolumeDescriptor*);
void ReleaseVolumeObjects(VolumeObjects_t *);
#endif /* _DATA_H */
