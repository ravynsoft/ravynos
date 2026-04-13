/*
 * Copyright (c) 1999, 2005-2011 Apple Inc. All rights reserved.
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
/* SRuntime.h */

#ifndef __SRUNTIME__
#define __SRUNTIME__

#if BSD

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include <hfs/hfs_format.h>
#else

#include <MacTypes.h>
#include <MacMemory.h>
#include <HFSVolumes.h>
#include <Errors.h>

#endif

#if BSD
/* Classic Mac OS Types */
typedef int8_t		SInt8;
typedef int16_t		SInt16;
typedef int32_t		SInt32;
typedef int64_t		SInt64;

typedef u_int8_t	UInt8;
typedef u_int16_t	UInt16;
typedef u_int32_t	UInt32;
typedef u_int64_t	UInt64;

typedef void *		LogicalAddress;
typedef char *		Ptr;
typedef Ptr *		Handle;
typedef u_int8_t	Byte;
typedef size_t 		Size;
typedef unsigned char	Boolean;
typedef u_int32_t	ItemCount;
typedef u_int32_t	ByteCount;
typedef u_int32_t	OptionBits;

typedef int16_t 	OSErr;
typedef int32_t 	OSStatus;

typedef u_int32_t	OSType;
typedef u_int32_t	ResType;

typedef u_int16_t	UniChar;
typedef u_int32_t	UniCharCount;
typedef UniChar *	UniCharArrayPtr;
typedef const UniChar *	ConstUniCharArrayPtr;
typedef u_int32_t	TextEncoding;

typedef unsigned char *	StringPtr;
typedef unsigned char	Str27[28];
typedef unsigned char	Str31[32];
typedef unsigned char	Str63[64];
typedef unsigned char	Str255[256];

typedef const unsigned char *	ConstStr31Param;
typedef const unsigned char *	ConstStr63Param;
typedef const unsigned char *	ConstStr255Param;

typedef u_int32_t	HFSCatalogNodeID;

enum {
	false		= 0,
	true		= 1
};

/* OS error codes */
enum {
	noErr		=     0,
	dskFulErr	=   -34,
	nsvErr		=   -35,
	ioErr		=   -36,
	eofErr		=   -39,
	fnfErr		=   -43,
	fBsyErr		=   -47,
	paramErr	=   -50,
	noMacDskErr	=   -57,
	badMDBErr	=   -60,
	memFullErr	=  -108,
	notBTree	=  -410,
	fileBoundsErr	= -1309,
};

/* Finder Flags */
enum {
    kIsOnDesk		= 0x0001,
    kColor		= 0x000E,
    kIsShared		= 0x0040,
    kHasBeenInited	= 0x0100,
    kHasCustomIcon	= 0x0400,
    kIsStationery	= 0x0800,
    kNameLocked		= 0x1000,
    kHasBundle		= 0x2000,
    kIsInvisible	= 0x4000,
    kIsAlias		= 0x8000
};

#define EXTERN_API(_type)	extern _type
#define EXTERN_API_C(_type)	extern _type

#define nil NULL

EXTERN_API( void )
DebugStr(ConstStr255Param debuggerMsg);

typedef void * QElemPtr;
typedef void * DrvQElPtr;


#endif



/* vcbFlags bits */
enum {
  kVCBFlagsIdleFlushBit           = 3,                                                    /* Set if volume should be flushed at idle time */
        kVCBFlagsIdleFlushMask          = 0x0008,
        kVCBFlagsHFSPlusAPIsBit         = 4,                                                    /* Set if volume implements HFS Plus APIs itself (not via emu\
												   lation) */
        kVCBFlagsHFSPlusAPIsMask        = 0x0010,
        kVCBFlagsHardwareGoneBit        = 5,                                                    /* Set if disk driver returned a hardwareGoneErr to Read or W\
												   rite */
        kVCBFlagsHardwareGoneMask       = 0x0020,
        kVCBFlagsVolumeDirtyBit         = 15,                                                   /* Set if volume information has changed since the last Flush\
												   Vol */
        kVCBFlagsVolumeDirtyMask        = 0x8000
};


/* Disk Cache constants */
/*
 * UTGetBlock options
 */
enum {
	gbDefault					= 0,							/* default value - read if not found */
																/*	bits and masks */
	gbReadBit					= 0,							/* read block from disk (forced read) */
	gbReadMask					= 0x0001,
	gbExistBit					= 1,							/* get existing cache block */
	gbExistMask					= 0x0002,
	gbNoReadBit					= 2,							/* don't read block from disk if not found in cache */
	gbNoReadMask				= 0x0004,
	gbReleaseBit				= 3,							/* release block immediately after GetBlock */
	gbReleaseMask				= 0x0008
};


/*
 * UTReleaseBlock options
 */
enum {
	rbDefault					= 0,							/* default value - just mark the buffer not in-use */
																/*	bits and masks */
	rbWriteBit					= 0,							/* force write buffer to disk */
	rbWriteMask					= 0x0001,
	rbTrashBit					= 1,							/* trash buffer contents after release */
	rbTrashMask					= 0x0002,
	rbDirtyBit					= 2,							/* mark buffer dirty */
	rbDirtyMask					= 0x0004,
	rbFreeBit					= 3,							/* free the buffer (save in the hash) */
	rbFreeMask					= 0x000A						/* rbFreeMask (rbFreeBit + rbTrashBit) works as rbTrash on < System 7.0 RamCache; on >= System 7.0, rbfreeMask overrides rbTrash */
};


/*
 * UTFlushCache options
 */
enum {
	fcDefault					= 0,							/* default value - pass this fcOption to just flush any dirty buffers */
																/*	bits and masks */
	fcTrashBit					= 0,							/* (don't pass this as fcOption, use only for testing bit) */
	fcTrashMask					= 0x0001,						/* pass this fcOption value to flush and trash cache blocks */
	fcFreeBit					= 1,							/* (don't pass this as fcOption, use only for testing bit) */
	fcFreeMask					= 0x0003						/* pass this fcOption to flush and free cache blocks (Note: both fcTrash and fcFree bits are set) */
};


/*
 * UTCacheReadIP and UTCacheWriteIP cacheOption bits and masks are the ioPosMode
 * bits and masks in Files.x
 */

/*
 * Cache routine internal error codes
 */
enum {
	chNoBuf						= 1,							/* no free cache buffers (all in use) */
	chInUse						= 2,							/* requested block in use */
	chnotfound					= 3,							/* requested block not found */
	chNotInUse					= 4								/* block being released was not in use */
};


/*
 * FCBRec.fcbFlags bits
 */
enum {
	fcbWriteBit					= 0,							/* Data can be written to this file */
	fcbWriteMask				= 0x01,
	fcbResourceBit				= 1,							/* This file is a resource fork */
	fcbResourceMask				= 0x02,
	fcbWriteLockedBit			= 2,							/* File has a locked byte range */
	fcbWriteLockedMask			= 0x04,
	fcbSharedWriteBit			= 4,							/* File is open for shared write access */
	fcbSharedWriteMask			= 0x10,
	fcbFileLockedBit			= 5,							/* File is locked (write-protected) */
	fcbFileLockedMask			= 0x20,
	fcbOwnClumpBit				= 6,							/* File has clump size specified in FCB */
	fcbOwnClumpMask				= 0x40,
	fcbModifiedBit				= 7,							/* File has changed since it was last flushed */
	fcbModifiedMask				= 0x80
};

enum {
	fcbLargeFileBit				= 3,							/* File may grow beyond 2GB; cache uses file blocks, not bytes */
	fcbLargeFileMask			= 0x08
};

#define kSectorShift  9   /* log2(kSectorSize); used for bit shifts */

/*
	Fork Level Access Method Block get options
*/
enum {
	kGetBlock	= 0x00000000,
	kForceReadBlock	= 0x00000002,
	kGetEmptyBlock	= 0x00000008,
	kSkipEndianSwap	= 0x00000010
};
typedef OptionBits  GetBlockOptions;

/*
	Fork Level Access Method Block release options
*/
enum {
	kReleaseBlock	 = 0x00000000,
	kForceWriteBlock = 0x00000001,
	kMarkBlockDirty	 = 0x00000002,
	kTrashBlock	 	 = 0x00000004
};
typedef OptionBits  ReleaseBlockOptions;

struct BlockDescriptor{
	void		*buffer;
	void		*blockHeader;
	UInt64		 blockNum;
	UInt32		 blockSize;
	Boolean		 blockReadFromDisk;
	Boolean		 fragmented;
};
typedef struct BlockDescriptor BlockDescriptor;
typedef BlockDescriptor *BlockDescPtr;



struct SFCB;

struct SVCB {
	UInt16		vcbSignature;
	UInt16		vcbVersion;
	UInt32		vcbAttributes;
	UInt32		vcbLastMountedVersion;
	UInt32		vcbReserved1;
	UInt32		vcbCreateDate;
	UInt32		vcbModifyDate;
	UInt32		vcbBackupDate;
	UInt32		vcbCheckedDate;
	UInt32		vcbFileCount;
	UInt32		vcbFolderCount;
	UInt32		vcbBlockSize;
	UInt32		vcbTotalBlocks;
	UInt32		vcbFreeBlocks;
	UInt32		vcbNextAllocation;
	UInt32 		vcbRsrcClumpSize;
	UInt32		vcbDataClumpSize;
	UInt32 		vcbNextCatalogID;
	UInt32		vcbWriteCount;
	UInt64		vcbEncodingsBitmap;
	UInt8		vcbFinderInfo[32];

	/* MDB-specific fields... */
	SInt16		vcbNmFls;	/* number of files in root folder */
	SInt16		vcbNmRtDirs;	/* number of directories in root folder */
	UInt16		vcbVBMSt;	/* first sector of HFS volume bitmap */
	UInt16		vcbAlBlSt;	/* first allocation block in HFS volume */
	UInt16		vcbVSeqNum;	/* volume backup sequence number */
	UInt16		vcbReserved2;
	Str27		vcbVN;		/* HFS volume name */

	/* runtime fields... */
	struct SFCB * 	vcbAllocationFile;
	struct SFCB * 	vcbExtentsFile;
	struct SFCB * 	vcbCatalogFile;
	struct SFCB * 	vcbAttributesFile;
	struct SFCB * 	vcbStartupFile;

	UInt32		vcbEmbeddedOffset;	/* Byte offset where HFS+ starts */
	UInt16		vcbFlags;
	SInt16		vcbDriveNumber;
	SInt16		vcbDriverReadRef;
	SInt16		vcbDriverWriteRef;

	void *		vcbBlockCache;

	struct SGlob *	vcbGPtr;

	/* deprecated fields... */
	SInt16		vcbVRefNum;
};
typedef struct SVCB SVCB;


struct SFCB {
	UInt32			fcbFileID;
	UInt32			fcbFlags;
	struct SVCB *		fcbVolume;
	void *			fcbBtree;
	HFSExtentRecord		fcbExtents16;
	HFSPlusExtentRecord	fcbExtents32;
	UInt32			fcbCatalogHint;
	UInt32			fcbClumpSize;
	UInt64 			fcbLogicalSize;
	UInt64			fcbPhysicalSize;
	UInt32			fcbBlockSize;
};
typedef struct SFCB SFCB;


extern OSErr GetDeviceSize(int driveRefNum, UInt64 *numBlocks, UInt32 *blockSize);

extern OSErr DeviceRead(int device, int drive, void* buffer, SInt64 offset, UInt32 reqBytes, UInt32 *actBytes);

extern OSErr DeviceWrite(int device, int drive, void* buffer, SInt64 offset, UInt32 reqBytes, UInt32 *actBytes);


/*
 *  Block Cache Interface
 */
extern void      InitBlockCache(SVCB *volume);

extern OSStatus  GetVolumeBlock (SVCB *volume, UInt64 blockNum, GetBlockOptions options,
				BlockDescriptor *block);

extern OSStatus  ReleaseVolumeBlock (SVCB *volume, BlockDescriptor *block,
				ReleaseBlockOptions options);

extern OSStatus  GetFileBlock (SFCB *file, UInt32 blockNum, GetBlockOptions options,
				BlockDescriptor *block);

extern OSStatus  ReleaseFileBlock (SFCB *file, BlockDescriptor *block,
				ReleaseBlockOptions options);

extern OSStatus  SetFileBlockSize (SFCB *file, ByteCount blockSize);



#if BSD

#define AllocateMemory(size)		malloc((size_t)(size))
#define	AllocateClearMemory(size)	calloc(1,(size_t)(size))
#define ReallocateMemory(ptr,newSize)	SetPtrSize((void*)(ptr),(size_t)(newSize))
#define MemorySize(ptr)			malloc_size((void*)(ptr))
#define DisposeMemory(ptr)		free((void *)(ptr))
#define CopyMemory(src,dst,len)		bcopy((void*)(src),(void*)(dst),(size_t)(len))
#define ClearMemory(start,len)	 	bzero((void*)(start),(size_t)(len))

extern UInt32	TickCount();
extern OSErr	MemError(void);
extern Handle	NewHandleClear(Size byteCount);
extern Handle	NewHandle(Size byteCount);
extern void	DisposeHandle(Handle h);
extern Size	GetHandleSize(Handle h);
extern void	SetHandleSize(Handle h, Size newSize);
extern OSErr	PtrAndHand(const void *ptr1, Handle hand2, long size);

#else

#define AllocateMemory(size)		NewPtr((Size)(size))
#define	AllocateClearMemory(size)	NewPtrClear((Size)(size))
#define ReallocateMemory(ptr,newSize)	SetPtrSize((Ptr)(ptr),(Size)(newSize))
#define MemorySize(ptr)			GetPtrSize((Ptr)(ptr))
#define DisposeMemory(ptr)		DisposePtr((Ptr)(ptr))
#define CopyMemory(src,dst,len)		BlockMoveData((void *)(src),(void *)(dst),(Size)(len))
void ClearMemory(void* start, long len);
#endif


#endif /* __SRUNTIME__ */


