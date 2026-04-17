/*
 * Copyright (c) 1999-2011 Apple Inc. All rights reserved.
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
/* Scavenger.h */

#ifndef __SCAVENGER__
#define __SCAVENGER__

#define pascal

#include "SRuntime.h"
#include "BTree.h"
#include "BTreePrivate.h"
#include "CheckHFS.h"
#include "BTreeScanner.h"
#include "hfs_endian.h"
#include "../fsck_debug.h"
#include "../fsck_messages.h"
#include "../fsck_hfs_msgnums.h"
#include "../fsck_msgnums.h"
#include "../fsck_hfs.h"

#include <assert.h>
#include <sys/xattr.h>
#include <sys/acl.h>
#include <sys/kauth.h>
#include <sys/errno.h>
#include <sys/syslimits.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/mount.h>
#include <hfs/hfs_mount.h>

#ifdef __cplusplus
extern	"C" {
#endif


#define kFSCKMountVersion	0x6673636B	/* 'fsck' made changes */

enum {
	Log2BlkLo				= 9,					// number of left shifts to convert bytes to block.lo
	Log2BlkHi				= 23					// number of right shifts to convert bytes to block.hi
};

enum {
	kNoHint						= 0
};


//
// Misc constants
//

/* IO size for reading or writing disk blocks */
#define DISK_IOSIZE	32768

#define kMaxReScan	3	/* max times to re-scan volume on repair success */

#define	kBTreeHeaderUserBytes	128

#define	kBusErrorValue	0x50FF8001

//ее Danger! This should not be hard coded
#define	kMaxClumpSize	0x100000	/* max clump size is 1MB (2048 btree nodes) */

#define	MDB_FNum	1				/* file number representing the MDB */
#define	AMDB_FNum	-1				/* file number representing the alternate MDB */
#define	VBM_FNum	2				/* file number representing the volume bit map */
#define	MDB_BlkN	2				/* logical block number for the MDB */

#define kCalculatedExtentRefNum			( 0 )
#define kCalculatedCatalogRefNum		( 1*sizeof(SFCB) )
#define kCalculatedAllocationsRefNum	( 2*sizeof(SFCB) )
#define kCalculatedAttributesRefNum		( 3*sizeof(SFCB) )
#define kCalculatedStartupRefNum		( 4*sizeof(SFCB) )
#define kCalculatedRepairRefNum			( 5*sizeof(SFCB) )

#define	Max_ABSiz	0x7FFFFE00		/* max allocation block size (multiple of 512 */
#define	Blk_Size	512				/* size of a logical block */
#define kHFSBlockSize 512			/* HFS block size */

// only the lower 7 bits are considered to be invalid, all others are valid -djb
#define	VAtrb_Msk	0x007F			/* volume attribute mask - invalid bits */
#define	VAtrb_DFlt	0x0100			/* default volume attribute flags */
#define	VAtrb_Cons	0x0100			/* volume consistency flag */
#define kHFSCatalogNodeIDsReused 0x1000		


/*
 *	File type and creator for TextEdit documents
 */
enum {
	kTextFileType		= 0x54455854,	/* 'TEXT' */
	kTextFileCreator	= 0x74747874,	/* 'ttxt' */
};

/* 
 *	Alias type and creator for directory hard links
 */
enum {
	kHFSAliasType		= 0x66647270, 	/* 'fdrp' */
	kHFSAliasCreator	= 0x4D414353	/* 'MACS' */
};

/*------------------------------------------------------------------------------
 BTree data structures
------------------------------------------------------------------------------*/

/* misc BTree constants */

#define	BTMaxDepth	8				/* max tree depth */
#define	Num_HRecs	3				/* number of records in BTree Header node */
#define	Num_MRecs	1				/* number of records in BTree Map node */



//	DFA extensions to the HFS/HFS+ BTreeControlBlock
typedef struct BTreeExtensionsRec
{
	Ptr 				BTCBMPtr;			//	pointer to scavenger BTree bit map
	UInt32				BTCBMSize;			//	size of the bitmap, bytes
	BTreeControlBlock	*altBTCB;			//	BTCB DFA builds up
	UInt32				realFreeNodeCount;	//	Number of real free nodes, taken from disk, for more accurate progress information
} BTreeExtensionsRec;


	
/*
 * Scavenger BTree Path Record (STPR)
 */
typedef struct STPR {
	UInt32			TPRNodeN;		/* node number */
	SInt16			TPRRIndx;		/* record index */
	SInt16			unused;			/* not used - makes debugging easier */
	UInt32			TPRLtSib;		/* node number of left sibling node */
	UInt32			TPRRtSib;		/* node number of right sibling node */
	} STPR, *STPRPtr;
	
typedef	STPR SBTPT[BTMaxDepth]; 		/* BTree path table */
	
#define	LenSBTPT	( sizeof(STPR) * BTMaxDepth )	/* length of BTree Path Table */




/*------------------------------------------------------------------------------
 CM (Catalog Manager) data structures
 ------------------------------------------------------------------------------*/

//
//	Misc constants
//
#define CMMaxDepth	100				/* max catalog depth (Same as Finder 7.0) */

#define fNameLocked 4096

union CatalogName {
	Str31 							pstr;
	HFSUniStr255 					ustr;
};
typedef union CatalogName				CatalogName;
	
//
//	Scavenger Directory Path Record (SDPR)
//
typedef struct SDPR {
	UInt32				directoryID;		//	directory ID
	UInt32				offspringIndex;		//	offspring index
	UInt32				directoryHint;		//	BTree hint for directory record
	long				threadHint;			//	BTree hint for thread record
	HFSCatalogNodeID	parentDirID;		//	parent directory ID
	CatalogName			directoryName;		//	directory CName
} SDPR;
	
enum {
//	kInvalidMRUCacheKey			= -1L,							/* flag to denote current MRU cache key is invalid*/
	kDefaultNumMRUCacheBlocks	= 16							/* default number of blocks in each cache*/
};


/*
 * UTCacheReadIP and UTCacheWriteIP cacheOption
 */

enum {
	noCacheBit   = 5,	/* don't cache this please */
	noCacheMask  = 0x0020,
	rdVerifyBit  = 6,	/* read verify */
	rdVerifyMask = 0x0040
};


/*------------------------------------------------------------------------------
 Low-level File System Error codes 
------------------------------------------------------------------------------*/

/* The DCE bits are defined as follows (for the word of flags): */

enum
{
	Is_AppleTalk		= 0,
	Is_Agent			= 1,			// future use
	FollowsNewRules		= 2,			// New DRVR Rules Bit
	Is_Open				= 5,
	Is_Ram_Based		= 6,
	Is_Active			= 7,
	Read_Enable			= 8,
	Write_Enable		= 9,
	Control_Enable		= 10,
	Status_Enable		= 11,
	Needs_Goodbye		= 12,
	Needs_Time			= 13,
	Needs_Lock			= 14,

	Is_AppleTalk_Mask	= 1 << Is_AppleTalk,
	Is_Agent_Mask		= 1 << Is_Agent,
	FollowsRules_Mask	= 1 << FollowsNewRules,
	Is_Open_Mask		= 1 << Is_Open,
	Is_Ram_Based_Mask	= 1 << Is_Ram_Based,
	Is_Active_Mask		= 1 << Is_Active,
	Read_Enable_Mask	= 1 << Read_Enable,
	Write_Enable_Mask	= 1 << Write_Enable,
	Control_Enable_Mask	= 1 << Control_Enable,
	Status_Enable_Mask	= 1 << Status_Enable,
	Needs_Goodbye_Mask	= 1 << Needs_Goodbye,
	Needs_Time_Mask		= 1 << Needs_Time,
	Needs_Lock_Mask		= 1 << Needs_Lock
};

enum {
	cdInternalErr					= -1312,		//	internal CheckDisk error
	cdVolumeNotFoundErr				= -1313,		//	cound not find volume (could be offline)
	cdCannotReadErr					= -1314,		//	unable to read from disk
	cdCannotWriteErr				= -1315,		//	unable to write to disk
	cdNotHFSVolumeErr				= -1316,		//	not an HFS disk
	cdUnrepairableErr				= -1317,		//	volume needs major repairs that CheckDisk cannot fix
	cdRepairFailedErr				= -1318,		//	repair failed
	cdUserCanceledErr				= -1319,		//	user interrupt
	cdVolumeInUseErr				= -1320,		//	volume modifed by another app
	cdNeedsRepairsErr				= -1321,		//	volume needs repairs (see repairInfo for additional info)
	cdReMountErr					= -1322,		//	Cannot remount volume
	cdUnknownProcessesErr			= -1323,		//	Volume cannot be unmounted and unknown processes are running
	cdDamagedWrapperErr				= -1324,		//	HFS Wrapper damaged error.
	cdIncompatibleOSErr				= -1325,		//	Current OS version is incompatible
	cdMemoryFullErr					= -1326			//	not enough memory to check disk
};


enum {
	fsDSIntErr	= -127	/* non-hardware Internal file system error */
};

//	Repair Info - additional info returned when a repair is attempted
enum {
	kFileSharingEnabled		= 0x00000001,
	kDiskIsLocked			= 0x00000002,
	kDiskIsBoot			= 0x00000004,
	kDiskHasOpenFiles		= 0x00000008,
	kVolumeHadOverlappingExtents	= 0x00000010,	// repairLevelSomeDataLoss
	kVolumeClean			= 0x00000020,

	kRepairsWereMade		= 0x80000000
};

//	Input parameters to CheckDisk
enum
{
	ignoreRunningProcessesMask		= 0x00000001,	//	Assumes caller has shut down processes
	checkDiskVersionMask			= 0x00000004	//	Will just return back the version in repairInfo.
};

//	Message types, so the user can treat and display accordingly
enum {
	kStatusMessage					= 0x0000,
	kTitleMessage					= 0x0001,
	kErrorMessage					= 0x0002
};

//	<10> Current stage of CheckDisk passed to cancel proc.
//	File System is marked busy during kRepairStage, so WaitNextEvent and I/O cannot be done during this stage.

enum {
	kHFSStage						= 0,
	kRepairStage,
	kVerifyStage,
	kAboutToRepairStage
};

//	Resource ID of 'STR ' resource containing the name of of the folder to create aliases to damaged files.
enum {
	rDamagedFilesDirSTRid			= -20886
};

//	Type of volume
enum {
	kUnknownVolumeType 	= 0,
	kHFSVolumeType,
	kEmbededHFSPlusVolumeType,
	kPureHFSPlusVolumeType
};


enum {
	kStatusLines	= 131,
	kFirstError		= 500,
	
	kHighLevelInfo	= 1100,
	kBasicInfo		= 1200,
	kErrorInfo		= 1202,

	kErrorBase		= -1310
};


/*------------------------------------------------------------------------------
 Minor Repair Interface (records compiled during scavenge, later repaired)
 Note that not all repair types use all of these fields.
 -----------------------------------------------------------------------------*/
 
 typedef struct	RepairOrder			/* a node describing a needed minor repair */
 {
 	struct RepairOrder	*link;	/* link to next node, or NULL */
	SInt16			type;	/* type of error, as an error code (E_DirVal etc) */
	SInt16		forkType;	/* which file fork */
	UInt64		correct;	/* correct valence */
	UInt64		incorrect;	/* valence as found in volume (for consistency chk) */
	UInt32		maskBit;	/* incorrect bit */
	UInt32		hint;		/* B-tree node hint */
	UInt32		parid;		/* parent ID */
	unsigned char name[1];	/* dir or file name */
 } RepairOrder, *RepairOrderPtr;


 typedef struct	EmbededVolDescription
 {
 	SInt16				drAlBlSt;
	UInt16 				drEmbedSigWord;
 	HFSExtentDescriptor	drEmbedExtent;
 } EmbededVolDescription;


// define the correct drive queue structure
typedef struct ExtendedDrvQueue
{
	char dQVolumeLocked;
	char dQDiskInDrive;
	char dQUsedInternally;
	char dQDiskIsSingleSided;
	QElemPtr qLink;
	short qType;
	short dQDrive;
	short dQRefNum;
	short dQFSID;
	short dQDrvSz;
	short dQDrvSz2;
}ExtendedDrvQueue;


/*------------------------------------------------------------------------------
 Scavenger Global Area - (SGlob) 
------------------------------------------------------------------------------*/
typedef struct MissingThread
{
	struct MissingThread  *link;		/* link to next node, or NULL */
	UInt32                threadID;
	HFSPlusCatalogKey     nextKey;
	HFSPlusCatalogThread  thread;
} MissingThread;

#define kDataFork	0
#define kRsrcFork	(-1)
#define kEAData		1

struct ExtentInfo {
	HFSCatalogNodeID fileID;
	UInt32	startBlock;
	UInt32 	blockCount;
	UInt32	newStartBlock;
	char *  attrname;
	UInt8	forkType;
	/* didRepair stores the result of moving of overlap extent and is used 
	 * to decide which disk blocks (original blocks or blocks allocated for 
	 * for new extent location) should be marked used and free.
	 */
	Boolean didRepair;		
};
typedef struct ExtentInfo ExtentInfo;

struct ExtentsTable {
	UInt32 							count;
	ExtentInfo 						extentInfo[1];
};
typedef struct ExtentsTable ExtentsTable;


struct FileIdentifier {
	Boolean 						hasThread;
	HFSCatalogNodeID 				fileID;
	HFSCatalogNodeID 				parID;			//	Used for files on HFS volumes without threads
	Str31		 					name;			//	Used for files on HFS volumes without threads
};
typedef struct FileIdentifier FileIdentifier;

struct FileIdentifierTable {
	UInt32 							count;
	FileIdentifier 					fileIdentifier[1];
};
typedef struct FileIdentifierTable FileIdentifierTable;

/* Universal Extent Key */

union ExtentKey {
	HFSExtentKey 					hfs;
	HFSPlusExtentKey 				hfsPlus;
};
typedef union ExtentKey					ExtentKey;
/* Universal extent descriptor */

union ExtentDescriptor {
	HFSExtentDescriptor 			hfs;
	HFSPlusExtentDescriptor 		hfsPlus;
};
typedef union ExtentDescriptor			ExtentDescriptor;
/* Universal extent record */

union ExtentRecord {
	HFSExtentRecord 				hfs;
	HFSPlusExtentRecord 			hfsPlus;
};
typedef union ExtentRecord				ExtentRecord;
/* Universal catalog key */

union CatalogKey {
	HFSCatalogKey 					hfs;
	HFSPlusCatalogKey 				hfsPlus;
};
typedef union CatalogKey				CatalogKey;
/* Universal catalog data record */

union CatalogRecord {
	SInt16 							recordType;
	HFSCatalogFolder 				hfsFolder;
	HFSCatalogFile 					hfsFile;
	HFSCatalogThread 				hfsThread;
	HFSPlusCatalogFolder 			hfsPlusFolder;
	HFSPlusCatalogFile 				hfsPlusFile;
	HFSPlusCatalogThread 			hfsPlusThread;
};
typedef union CatalogRecord				CatalogRecord;

/*
  	Key for records in the attributes file.  Fields are compared in the order:
  		cnid, attributeName, startBlock
*/

struct AttributeKey {
	UInt16 							keyLength;					/* must set kBTBigKeysMask and kBTVariableIndexKeysMask in BTree header's attributes */
	UInt16 							pad;
	HFSCatalogNodeID 				cnid;						/* file or folder ID */
	UInt32 							startBlock;					/* block # relative to start of attribute */
	UInt16     attrNameLen;     /* number of unicode characters */
	UInt16     attrName[127];   /* attribute name (Unicode) */
};
typedef struct AttributeKey				AttributeKey;
enum {
	kAttributeKeyMaximumLength	= sizeof(AttributeKey) - sizeof(UInt16),
	kAttributeKeyMinimumLength	= kAttributeKeyMaximumLength - 127 * sizeof(UInt16) + sizeof(UInt16)
};

struct HIOParam {
	QElemPtr 						qLink;						/*queue link in header*/
	short 							qType;						/*type byte for safety check*/
	short 							ioTrap;						/*FS: the Trap*/
	Ptr 							ioCmdAddr;					/*FS: address to dispatch to*/
	void* 				ioCompletion;				/*completion routine addr (0 for synch calls)*/
	OSErr 							ioResult;					/*result code*/
	StringPtr 						ioNamePtr;					/*ptr to Vol:FileName string*/
	short 							ioVRefNum;					/*volume refnum (DrvNum for Eject and MountVol)*/
	short 							ioRefNum;
	SInt8 							ioVersNum;
	SInt8 							ioPermssn;
	Ptr 							ioMisc;
	Ptr 							ioBuffer;
	long 							ioReqCount;
	long 							ioActCount;
	short 							ioPosMode;
	long 							ioPosOffset;
};
typedef struct HIOParam HIOParam;

typedef HIOParam *						HIOParamPtr;


struct FCBArray {
	UInt32		length;		/* first word is FCB part length*/
	SFCB		fcb[1];		/* fcb array*/
};
typedef struct FCBArray FCBArray;

/*
	UserCancel callback routine
	
	Input:
			progress:			number from 1 to 100 indicating current progress
			progressChanged:	boolean flag that is true if progress number has been updated
			context:			pointer to context data (if any) that the caller passed to CheckDisk
			
	Output:
			return true if the user wants to cancel the CheckDisk operation
 */

typedef int (*UserCancelProcPtr)(UInt16 progress, UInt16 secondsRemaining, Boolean progressChanged, UInt16 stage, void *context, int passno);


#if  0

	//--	User Cancel Proc
	typedef UniversalProcPtr UserCancelUPP;
	
	enum {
		uppUserCancelProcInfo = kPascalStackBased
			 | RESULT_SIZE(kTwoByteCode)
			 | STACK_ROUTINE_PARAMETER(1, kTwoByteCode)
			 | STACK_ROUTINE_PARAMETER(2, kTwoByteCode)
			 | STACK_ROUTINE_PARAMETER(3, kTwoByteCode)
			 | STACK_ROUTINE_PARAMETER(4, kTwoByteCode)
			 | STACK_ROUTINE_PARAMETER(5, kFourByteCode)
	};
	
	#define NewUserCancelProc(userRoutine)		\
			(UserCancelUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppUserCancelProcInfo, GetCurrentArchitecture())
	
	#define CallUserCancelProc(userRoutine, progress, secondsRemaining, progressChanged, stage, context, p)		\
			CallUniversalProc((UniversalProcPtr)(userRoutine), uppUserCancelProcInfo, (progress), (secondsRemaining), (progressChanged), (stage), (context), (p))

#else /* not CFM */

	typedef UserCancelProcPtr UserCancelUPP;
	
	#define NewUserCancelProc(userRoutine)		\
			((UserCancelUPP) (userRoutine))
	
	#define CallUserCancelProc(userRoutine, progress, secondsRemaining, progressChanged, stage, context, p)		\
			(*(userRoutine))((progress), (secondsRemaining), (progressChanged), (stage), (context), (p))

#endif


/*
	UserMessage callback routine
	
	Input:
			message:			message from CheckDisk
			messageType:		type of message
			context:			pointer to context data (if any) that the caller passed to CheckDisk
			
	Output:
			return true if the user wants to cancel the CheckDisk operation
 */


typedef pascal void (*UserMessageProcPtr)(StringPtr message, SInt16 messageType, void *context);

#if 0

	//--	User Message Proc
	typedef UniversalProcPtr UserMessageUPP;
	
	enum {
		uppUserMessageProcInfo = kPascalStackBased
			 | STACK_ROUTINE_PARAMETER(1, kFourByteCode)
			 | STACK_ROUTINE_PARAMETER(2, kTwoByteCode)
			 | STACK_ROUTINE_PARAMETER(3, kFourByteCode)
	};
	
	#define NewUserMessageProc(userRoutine)		\
			(UserMessageUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppUserMessageProcInfo, GetCurrentArchitecture())
	
	#define CallUserMessageProc(userRoutine, message, messageType, context)		\
			CallUniversalProc((UniversalProcPtr)(userRoutine), uppUserMessageProcInfo, (message), (messageType), (context))

#else /* not CFM */

	typedef UserMessageProcPtr UserMessageUPP;
	
	#define NewUserMessageProc(userRoutine)		\
			((UserMessageUPP) (userRoutine))
	
	#define CallUserMessageProc(userRoutine, message, messageType, context)		\
			(*(userRoutine))((message), (messageType), (context))

#endif

/* 3843779 Structure to determine consistency of attribute data and 
 * corresponding bit in catalog record.  Based on Chinese Remainder
 * Theorem
 */
typedef struct PrimeBuckets {
	UInt32	n32[32];
	UInt32	n27[27];
	UInt32	n25[25];
	UInt32	n7[7];
	UInt32	n11[11];
	UInt32	n13[13];
	UInt32	n17[17];
	UInt32	n19[19];
	UInt32	n23[23];
	UInt32	n29[29];
	UInt32	n31[31];
} PrimeBuckets;

/* Record last attribute ID checked, used in CheckAttributeRecord, initialized in ScavSetup */
typedef struct attributeInfo {
	Boolean isValid;
	Boolean hasSecurity;
	int16_t	recordType;
	u_int32_t fileID;
	unsigned char attrname[XATTR_MAXNAMELEN+1];
	u_int32_t totalBlocks;
	u_int32_t calculatedTotalBlocks;
	u_int64_t logicalSize;
} attributeInfo;

/* 	
	VolumeObject encapsulates all infomration about the multiple volume anchor blocks (VHB and MSD) 
	on HFS and HFS+ volumes.  An HFS volume will have two MDBs (primary and alternate HFSMasterDirectoryBlock), 
	a pure HFS+ volume will have two VHBs (primary and alternate HFSPlusVolumeHeader), and a wrapped HFS+ 
	volume will have two MDBs and two VHBs.
*/

/* values for VolumeObject.flags */
enum {
	kVO_Inited			= 0x00000001,	   	// this structured has been initialized
	kVO_PriVHBOK		= 0x00000002,	   	// the primary Volume Header Block is valid
	kVO_AltVHBOK		= 0x00000004,	   	// the alternate Volume Header Block is valid
	kVO_PriMDBOK		= 0x00000008,	   	// the primary Master Directory Block is valid
	kVO_AltMDBOK		= 0x00000010,	   	// the alternate Master Directory Block is valid
};

typedef struct VolumeObject {
	UInt32			flags;
	SVCB * 			vcbPtr;				// pointer to VCB used for this volume
	UInt32			volumeType;			// (kHFSVolumeType or kEmbededHFSPlusVolumeType or kPureHFSPlusVolumeType)
	UInt32			embeddedOffset;		// offset of embedded HFS+ (in bytes) volume into HFS wrapper volume
										//   NOTE - UInt32 is OK since we don't support HFS Wrappers on TB volumes
	UInt32			sectorSize;			// size of a sector for this device
	UInt64			totalDeviceSectors;	// total number of sectors for this volume (from GetDeviceSize)
	UInt64			totalEmbeddedSectors; // total number of sectors for embedded volume
	// location of all possible volume anchor blocks (MDB and VHB) on this volume.  These locations
	// are the sector offset into the volume.  Only wrapped HFS+ volumes use all 4 of these.
	UInt64			primaryVHB;			// not used for HFS volumes
	UInt64			alternateVHB;		// not used for HFS volumes
	UInt64			primaryMDB;			// not used for pure HFS+ volumes
	UInt64			alternateMDB;		// not used for pure HFS+ volumes
} VolumeObject, *VolumeObjectPtr;


typedef struct SGlob {
	void *				scavStaticPtr;			// pointer to static structure allocated in ScavSetUp
	SInt16				DrvNum;					//	drive number of target drive
	SInt16				RepLevel;				//	repair level, 1 = minor repair, 2 = major repair
	SInt16				ScavRes;				//	scavenge result code
	OSErr				ErrCode;    			//	error code
	OSErr				IntErr;     			//	internal error code
	UInt16				VIStat;					//	scavenge status flags for volume info 
	UInt16				ABTStat;				//	scavenge status flags for Attributes BTree 
	UInt16				EBTStat;				//	scavenge status flags for extent BTree 
	UInt16				CBTStat;				//	scavenge status flags for catalog BTree 
	UInt32				CatStat;				//	scavenge status flags for catalog file
	UInt16				VeryMinorErrorsStat;	//	scavenge status flags for very minor errors
	UInt16				JStat;					//	scavange status flags for journal errors
	UInt16				PrintStat;				//	info about messages that should be displayed only once
	DrvQElPtr			DrvPtr;					//	pointer to driveQ element for target drive
	UInt32				TarID;					//	target ID (CNID of data structure being verified)
	UInt64				TarBlock;				//	target block/node number being verified
	SInt16				BTLevel;				//	current BTree enumeration level
	SBTPT				*BTPTPtr;				//	BTree path table pointer
	SInt16				DirLevel;				//	current directory enumeration level
	SDPR				*DirPTPtr;				//	directory path table pointer (pointer to array of SDPR)
	uint32_t			dirPathCount;			//  number of SDPR entries allocated in directory path table
	SInt16				CNType;					//	current CNode type
	UInt32				ParID;					//	current parent DirID
	CatalogName			CName;					//	current CName
	RepairOrderPtr		MinorRepairsP;			//	ptr to list of problems for later repair
	MissingThread		*missingThreadList;
	Ptr 				FCBAPtr;				//	pointer to scavenger FCB array
	UInt32				**validFilesList;		//	List of valid HFS file IDs

	ExtentsTable		**overlappedExtents;	//	List of overlapped extents
	FileIdentifierTable	**fileIdentifierTable;	//	List of files for post processing

	UInt32				inputFlags;				//	Caller can specify some DFA behaviors

	UInt32				volumeFeatures;			//	bit vector of volume and OS features
	Boolean				usersAreConnected;		//	true if user are connected
	Boolean				fileSharingOn;			//	true if file sharing is on
	UInt32				altBlockLocation;
	Boolean				checkingWrapper;
	SInt16				numExtents;				//	Number of memory resident extents.  3 or 8
	OSErr				volumeErrorCode;
	
	UserCancelUPP		userCancelProc;
	UserMessageUPP		userMessageProc;
	void				*userContext;

	UInt64				onePercent;
	UInt64				itemsToProcess;
	UInt64				itemsProcessed;
	UInt64				lastProgress;
	long				startTicks;
	UInt16				secondsRemaining;

	long				lastTickCount;

	
	SVCB			*calculatedVCB;
	SFCB			*calculatedExtentsFCB;
	SFCB			*calculatedCatalogFCB;
	SFCB			*calculatedAllocationsFCB;
	SFCB			*calculatedAttributesFCB;
	SFCB			*calculatedStartupFCB;
	SFCB			*calculatedRepairFCB;	
	BTreeControlBlock	*calculatedExtentsBTCB;
	BTreeControlBlock	*calculatedCatalogBTCB;
	BTreeControlBlock	*calculatedRepairBTCB;
	BTreeControlBlock	*calculatedAttributesBTCB;

	Boolean			cleanUnmount;
	Boolean			guiControl;
	fsck_ctx_t		context;
	int				chkLevel;
	int             repairLevel;
	int             rebuildOptions;			// options to indicate type of btree(s) to rebuild
	Boolean			minorRepairErrors;	// indicates some minor repairs failed
	Boolean		minorRepairFalseSuccess;	// indicates minor repair function is returning false success, do not delete from the list
	int				canWrite;  	// we can safely write to the block device
	int				writeRef;	// file descriptor with write access on the volume	
	int				lostAndFoundMode;  // used when creating lost+found directory
	int				liveVerifyState; // indicates if live verification is being done or not 
	BTScanState		scanState;
	int		scanCount;	/* Number of times fsck_hfs has looped */		

	unsigned char	volumeName[256]; /* volume name in ASCII or UTF-8 */
	char		deviceNode[256]; /* device node in ASCII */

	/* Extended attribute check related stuff */
	uint32_t	cat_ea_count;		/* number of catalog records that have attribute bit set */
	uint32_t	cat_acl_count;		/* number of catalog records that have security bit set */
	uint32_t	attr_ea_count;		/* number of unique fileID attributes found in attribute btree */
	uint32_t	attr_acl_count;		/* number of acls found in attribute btree */
	PrimeBuckets 	CBTAttrBucket;		/* prime number buckets for Attribute bit in Catalog btree */
	PrimeBuckets 	CBTSecurityBucket;	/* prime number buckets for Security bit in Catalog btree */
	PrimeBuckets 	ABTAttrBucket;		/* prime number buckets for Attribute bit in Attribute btree */
	PrimeBuckets 	ABTSecurityBucket;	/* prime number buckets for Security bit in Attribute btree */
	attributeInfo 	lastAttrInfo; 		/* Record last attribute ID checked, used in CheckAttributeRecord, initialized in ScavSetup */
	UInt16		securityAttrName[XATTR_MAXNAMELEN];	/* Store security attribute name in UTF16, to avoid frequent conversion */
	size_t  	securityAttrLen;

	/* File Hard Links related stuff */
	uint32_t	filelink_priv_dir_id;

	/* Directory Hard Links related stuff */
	uint32_t	dirlink_priv_dir_id;
	uint32_t	dirlink_priv_dir_valence;
	uint32_t	calculated_dirinodes;
	uint32_t	calculated_dirlinks;

	/* Journal file ID's */
	uint32_t	journal_file_id;
	uint32_t	jib_file_id;
} SGlob, *SGlobPtr;


enum
{
	supportsTrashVolumeCacheFeatureMask		= 1,
	supportsHFSPlusVolsFeatureMask			= 2,
	volumeIsMountedMask						= 4
};

/* scavenger flags */	
	
/* volume info status flags (contents of VIStat) */

#define	S_MDB					0x8000	//	MDB/VHB damaged
#define	S_AltMDB				0x4000	//	Unused	/* alternate MDB damaged */
#define	S_VBM					0x2000	//	volume bit map damaged
#define	S_WMDB					0x1000	//	wrapper MDB is damaged
#define	S_OverlappingExtents	0x0800	//	Overlapping extents found
#define	S_BadMDBdrAlBlSt		0x0400	//	Invalid drAlBlSt field in MDB
#define S_InvalidWrapperExtents	0x0200	//	Invalid catalog extent start in MDB

/* BTree status flags (contents of EBTStat, CBTStat and ABTStat) */

#define	S_BTH					0x8000	/* BTree header damaged */
#define	S_BTM					0x4000	/* BTree map damaged */
#define	S_Indx					0x2000	//	Unused	/* index structure damaged */
#define	S_Leaf					0x1000	//	Unused	/* leaf structure damaged */
#define S_Orphan				0x0800  // orphaned file
#define S_OrphanedExtent		0x0400  // orphaned extent
#define S_ReservedNotZero		0x0200  // the flags or reserved fields are not zero
#define S_RebuildBTree			0x0100  // similar to S_Indx, S_Leaf, but if one is bad we stop checking and the other may also be bad.
#define S_ReservedBTH			0x0080  // fields in the BTree header should be zero but are not
#define S_AttributeCount		0x0040	// incorrect number of xattr in attribute btree in comparison with attribute bit in catalog btree
#define S_SecurityCount			0x0020	// incorrect number of security xattrs in attribute btree in comparison with security bit in catalog btree
#define S_AttrRec				0x0010	// orphaned/unknown record in attribute BTree
#define S_ParentHierarchy		0x0008	// bad parent hierarchy, could not lookup parent directory record */
#define S_UnusedNodesNotZero	0x0004	/* Unused B-tree nodes are not filled with zeroes */

/* catalog file status flags (contents of CatStat) */

#define	S_IllName			0x00008000	/* illegal name found */
#define	S_Valence			0x00004000	/* a directory valence is out of sync */
#define	S_FThd				0x00002000	/* dangling file thread records exist */
#define	S_DFCorruption		0x00001000	/* disappearing folder corruption detected */
#define	S_NoDir				0x00000800	/* missing directory record */
#define S_LockedDirName		0x00000400  // locked dir name
#define S_MissingThread		0x00000200  /* missing thread record */
#define S_UnlinkedFile		0x00000100	/* orphaned link node */
#define S_LinkCount			0x00000080	/* data node link count needs repair */
#define S_Permissions		0x00000040	/* BSD permissions need repair */
#define S_FileAllocation	0x00000020	/* peof or leof needs adjustment */
#define S_BadExtent			0x00000010	/* invalid extent */
#define	S_LinkErrRepair		0x00000008	/* repairable file/directory hard link corruption detected */
#define	S_LinkErrNoRepair	0x00000004	/* un-repairable file/directory hard link corruptions detected */
#define	S_FileHardLinkChain	0x00000002	/* incorrect number of file hard links, doubly linked list chain needs repair */
#define S_DirHardLinkChain	0x00000001	/* incorrect number of directory hard links, doubly linked list chain needs repair */

/* VeryMinorErrorsStat */

#define S_BloatedThreadRecordFound	0x8000  // 2210409, excessivly large thread record found

/* user file status flags (contents of FilStat) */

//#define S_LockedName			0x4000  // locked file name

/* Journal status flag (contents of JStat) */
#define S_BadJournal		0x8000	/* Bad journal content */
#define	S_DirtyJournal		0x4000	/* Journal is dirty (needs to be replayed) */

/* Print status flag (contents of PrintStat) */
#define S_DamagedDir 		0x8000	/* message for M_LookDamagedDir already printed */
#define S_SymlinkCreate		0x4000	/* message for E_SymlinkCreate already printed */

/*------------------------------------------------------------------------------
 ScavCtrl Interface
------------------------------------------------------------------------------*/

//	Command Codes (commands to ScavControl)
enum
{
	scavInitialize		= 1,			//	Start initial volume check
	scavVerify,							//	Start verify operation
	scavRepair,							//	Start repair opeation
	scavTerminate,						//	Cleanup after scavenge
};


//	Repair Levels
enum
{
	repairLevelNoProblemsFound				= 0,
	repairLevelRepairIfOtherErrorsExist,		//	Bloated thread records, ...
	repairLevelVeryMinorErrors,					//	Missing Custom Icon, Locked Directory name,..., Errors that don't need fixing from CheckDisk (Installer), Non Volume corruption bugs.
	repairLevelVolumeRecoverable,				//	Minor Volume corruption exists
	repairLevelSomeDataLoss,					//	Overlapping extents, some data loss but no scavaging will get it back
	repairLevelWillCauseDataLoss,				//	Missing leaf nodes, repair will lose nodes without scavaging (proceed at your own risk, check disk with other utils)
	repairLevelCatalogBtreeRebuild,				//	Catalog Btree is damaged, repair may lose some data
	repairLevelUnrepairable						//	DFA cannot repair volume
};


/* Status messages written to summary */
enum {
	M_FirstMessage              =  1,
	M_LastMessage               = 29
};


/* Internal DFA error codes */
enum {
	errRebuildBtree				= -1001		/* BTree requires rebuilding. */
};


enum {																/*	extendFileContigMask		= 0x0002*/
	kEFContigBit				= 1,							/*	force contiguous allocation*/
	kEFContigMask				= 0x02,
	kEFAllBit					= 0,							/*	allocate all requested bytes or none*/
	kEFAllMask					= 0x01,
	kEFNoClumpBit				= 2,							/*	Don't round up requested size to multiple of clump size*/
	kEFNoClumpMask				= 0x04,							/*	TruncateFile option flags*/
	kEFNoExtOvflwBit			= 3,							/*  Don't use extens overflow file */
	kEFNoExtOvflwMask			= 0x08,

	kTFTrunExtBit				= 0,							/*	truncate to the extent containing new PEOF*/
	kTFTrunExtMask				= 1
};



// Encoding vs. Index
//
// For runtime table lookups and for the volume encoding bitmap we
// need to map some encodings to keep them in a reasonable range.
//

enum {
	kTextEncodingMacRoman		= 0L,
	kTextEncodingMacFarsi		= 0x8C,	 /* Like MacArabic but uses Farsi digits*/
																/* The following use script code 7, smCyrillic*/
	kTextEncodingMacUkrainian	= 0x98,	/* The following use script code 32, smUnimplemented*/

	kIndexMacUkrainian	= 48,		// MacUkrainian encoding is 152
	kIndexMacFarsi		= 49		// MacFarsi encoding is 140
};

#define MapEncodingToIndex(e) \
	( (e) < 48 ? (e) : ( (e) == kTextEncodingMacUkrainian ? kIndexMacUkrainian : ( (e) == kTextEncodingMacFarsi ? kIndexMacFarsi : kTextEncodingMacRoman) ) )

#define MapIndexToEncoding(i) \
	( (i) == kIndexMacFarsi ? kTextEncodingMacFarsi : ( (i) == kIndexMacUkrainian ? kTextEncodingMacUkrainian : (i) ) )

#define ValidMacEncoding(e)	\
	( ((e) < 39)  ||  ((e) == kTextEncodingMacFarsi)  ||  ((e) == kTextEncodingMacUkrainian) )




extern	void	WriteMsg( SGlobPtr GPtr, short messageID, short messageType );
extern	void	WriteError( SGlobPtr GPtr, short msgID, UInt32 tarID, UInt64 tarBlock );
extern	short	CheckPause( void );

/* ------------------------------- From SControl.c ------------------------------- */

void			ScavCtrl( SGlobPtr GPtr, UInt32 ScavOp, short *ScavRes );

extern	short	CheckForStop( SGlobPtr GPtr );


/* ------------------------------- From SRepair.c -------------------------------- */

extern	OSErr	RepairVolume( SGlobPtr GPtr );

extern	int		FixDFCorruption( const SGlobPtr GPtr, RepairOrderPtr DFOrderP );

extern	OSErr	ProcessFileExtents( SGlobPtr GPtr, SFCB *fcb, UInt8 forkType, UInt16 flags, Boolean isExtentsBTree, Boolean *hasOverflowExtents, UInt32 *blocksUsed  );

/* Function to get return file path/name given an ID */
extern 	OSErr 	GetSystemFileName(UInt32 fileID, char *filename, unsigned int *filenamelen);
extern 	OSErr 	GetFileNamePathByID(SGlobPtr GPtr, UInt32 fileID, char *fullPath, unsigned int *fullPathLen, char *fileName, unsigned int *fileNameLen, u_int16_t *status);
#define FNAME_BUF2SMALL	0x001	/* filename buffer was too small */
#define FNAME_BIGNAME	0x002	/* filename is greater than NAME_MAX bytes */
#define FPATH_BUF2SMALL	0x010	/* path buffer was too small */
#define	FPATH_BIGNAME	0x020	/* intermediate component in path is greater than NAME_MAX bytes */
#define	F_RESERVE_FILEID 0x100	/* file ID was less than kHFSFirstUserCatalogNodeID */

/* ------------------------------- From SUtils.c --------------------------------- */

extern	int		AllocBTN( SGlobPtr GPtr, short FilRefN, UInt32 NodeNum );

extern	int		IntError( SGlobPtr GPtr, OSErr ErrCode );

extern	void	RcdError( SGlobPtr GPtr, OSErr ErrCode );

extern	RepairOrderPtr AllocMinorRepairOrder( SGlobPtr GPtr, size_t extraBytes );

extern int IsDuplicateRepairOrder(SGlobPtr GPtr, RepairOrderPtr orig);

extern void DeleteRepairOrder(SGlobPtr GPtr, RepairOrderPtr orig);

extern	void	SetDFAStage( UInt32 stage );
extern	UInt32	GetDFAGlobals( void );

extern void		InitializeVolumeObject( SGlobPtr GPtr );
extern void 	CheckEmbeddedVolInfoInMDBs( SGlobPtr GPtr );
extern VolumeObjectPtr  GetVolumeObjectPtr( void );
extern OSErr	GetVolumeObjectVHB( BlockDescriptor * theBlockDescPtr );
extern void 	GetVolumeObjectBlockNum( UInt64 * theBlockNumPtr );
extern OSErr 	GetVolumeObjectAlternateBlock( BlockDescriptor * theBlockDescPtr );
extern OSErr 	GetVolumeObjectPrimaryBlock( BlockDescriptor * theBlockDescPtr );
extern void 	GetVolumeObjectAlternateBlockNum( UInt64 * theBlockNumPtr );
extern void 	GetVolumeObjectPrimaryBlockNum( UInt64 * theBlockNumPtr );
extern OSErr	GetVolumeObjectAlternateMDB( BlockDescriptor * theBlockDescPtr );
extern OSErr 	GetVolumeObjectPrimaryMDB( BlockDescriptor * theBlockDescPtr );
extern OSErr 	GetVolumeObjectVHBorMDB( BlockDescriptor * theBlockDescPtr );
extern void 	PrintName( int theCount, const UInt8 *theNamePtr, Boolean isUnicodeString );
extern void 	PrintVolumeObject( void );
extern Boolean 	VolumeObjectIsValid( SGlobPtr gptr );
extern Boolean 	VolumeObjectIsHFSPlus( void );
extern Boolean 	VolumeObjectIsHFS( void );
extern Boolean 	VolumeObjectIsEmbeddedHFSPlus( void );
extern Boolean 	VolumeObjectIsPureHFSPlus( void );
extern Boolean	VolumeObjectIsHFSX(SGlobPtr);

extern	void	InvalidateCalculatedVolumeBitMap( SGlobPtr GPtr );

extern	OSErr	GetVolumeFeatures( SGlobPtr GPtr );

OSErr	FlushAlternateVolumeControlBlock( SVCB *vcb, Boolean isHFSPlus );

extern	void	ConvertToHFSPlusExtent(const HFSExtentRecord oldExtents, HFSPlusExtentRecord newExtents);

void add_prime_bucket_uint32(PrimeBuckets *cur, uint32_t num);

void add_prime_bucket_uint64(PrimeBuckets *cur, uint64_t num);

int compare_prime_buckets(PrimeBuckets *bucket1, PrimeBuckets *bucket2); 

/* ------------------------------- From CatalogCheck.c -------------------------------- */

extern	OSErr	CheckCatalogBTree( SGlobPtr GPtr );	//	catalog btree check

extern	OSErr	CheckFolderCount( SGlobPtr GPtr );	//	Compute folderCount

extern int  RecordBadAllocation(UInt32 parID, unsigned char * filename, UInt32 forkType, UInt32 oldBlkCnt, UInt32 newBlkCnt);

extern int  RecordTruncation(UInt32 parID, unsigned char * filename, UInt32 forkType, UInt64 oldSize,  UInt64 newSize);

/* ------------------------------- From SVerify1.c -------------------------------- */

extern	OSErr	CatFlChk( SGlobPtr GPtr );		//	catalog file check
	
extern	OSErr	CatHChk( SGlobPtr GPtr );		//	catalog hierarchy check

extern	OSErr	ExtBTChk( SGlobPtr GPtr );		//	extent btree check

extern	OSErr	BadBlockFileExtentCheck( SGlobPtr GPtr );	//	bad block file extent check

extern	OSErr	AttrBTChk( SGlobPtr GPtr );		//	attributes btree check

extern	OSErr	IVChk( SGlobPtr GPtr );

/* Operation type for CheckForClean */
enum {
	kCheckVolume,		// check if volume is clean/dirty
	kMarkVolumeDirty,	// mark the volume dirty
	kMarkVolumeClean	// mark the volume clean
};
extern	int	CheckForClean( SGlobPtr GPtr, UInt8 operation, Boolean *modified );

extern  int	CheckIfJournaled(SGlobPtr GPtr, Boolean journal_bit_only);

typedef struct fsckJournalInfo {
	int	jnlfd;	// File descriptor for journal device
	off_t	jnlOffset;	// Offset of journal on journal device
	off_t	jnlSize;	// Size of journal on same
	char	*name;	// Name of journal device
} fsckJournalInfo_t;

extern	int	IsJournalEmpty(SGlobPtr, fsckJournalInfo_t *);

extern	OSErr	VInfoChk( SGlobPtr GPtr );

extern	OSErr	VLockedChk( SGlobPtr GPtr );

extern	void	BuildExtentKey( Boolean isHFSPlus, UInt8 forkType, HFSCatalogNodeID fileNumber, UInt32 blockNumber, void * key );

extern	OSErr	OrphanedFileCheck( SGlobPtr GPtr, Boolean *problemsFound );

extern	int		cmpLongs (const void *a, const void *b);

extern  int CheckAttributeRecord(SGlobPtr GPtr, const HFSPlusAttrKey *key, const HFSPlusAttrRecord *rec, UInt16 reclen);

extern void RecordXAttrBits(SGlobPtr GPtr, UInt16 flags, HFSCatalogNodeID fileid, UInt16 btreetype); 

extern  int FindOrigOverlapFiles(SGlobPtr GPtr);

extern  void PrintOverlapFiles (SGlobPtr GPtr);

/* ------------------------------- From SVerify2.c -------------------------------- */

typedef int (* CheckLeafRecordProcPtr)(SGlobPtr GPtr, void *key, void *record, UInt16 recordLen);

extern	int  BTCheck(SGlobPtr GPtr, short refNum, CheckLeafRecordProcPtr checkLeafRecord);

extern	int		BTMapChk( SGlobPtr GPtr, short FilRefN );

extern	OSErr	ChkCName( SGlobPtr GPtr, const CatalogName *name, Boolean unicode );	//	check catalog name

extern	OSErr	CmpBTH( SGlobPtr GPtr, SInt16 fileRefNum );

extern	int		CmpBTM( SGlobPtr GPtr, short FilRefN );

extern	int		CmpMDB( SGlobPtr GPtr, HFSMasterDirectoryBlock * mdbP);

extern	int		CmpVBM( SGlobPtr GPtr );

extern	OSErr	CmpBlock( void *block1P, void *block2P, size_t length ); /* same as 'memcmp', but EQ/NEQ only */
	
extern	OSErr	ChkExtRec ( SGlobPtr GPtr, UInt32 fileID, const void *extents , unsigned int *lastExtentIndex);

extern	int		BTCheckUnusedNodes(SGlobPtr GPtr, short fileRefNum, UInt16 *btStat);


/* -------------------------- From SRebuildBTree.c ------------------------- */

extern	OSErr 	RebuildBTree( SGlobPtr theSGlobPtr, int FileID );


/* -------------------------- From SCatalog.c ------------------------- */

extern OSErr	UpdateFolderCount( 	SVCB *vcb, 
									HFSCatalogNodeID pid, 
									const CatalogName *name, 
									SInt16 newType,
									UInt32 hint, 
									SInt16 valenceDelta );

/* ------------------------------- From SExtents.c -------------------------------- */
OSErr	ZeroFileBlocks( SVCB *vcb, SFCB *fcb, UInt32 startingSector, UInt32 numberOfSectors );

OSErr MapFileBlockC (
	SVCB		*vcb,				// volume that file resides on
	SFCB			*fcb,				// FCB of file
	UInt32			numberOfBytes,		// number of contiguous bytes desired
	UInt64			sectorOffset,		// starting offset within file (in 512-byte sectors)
	UInt64			*startSector,		// first 512-byte volume sector (NOT an allocation block)
	UInt32			*availableBytes);	// number of contiguous bytes (up to numberOfBytes)

OSErr DeallocateFile(SVCB *vcb, CatalogRecord * fileRec);

OSErr ExtendFileC (
	SVCB		*vcb,				// volume that file resides on
	SFCB			*fcb,				// FCB of file to truncate
	UInt32			sectorsToAdd,		// number of sectors to allocate
	UInt32			flags,				// EFContig and/or EFAll
	UInt32			*actualSectorsAdded); // number of bytes actually allocated
	
OSErr FlushExtentFile( SVCB *vcb );

void ExtDataRecToExtents(
	const HFSExtentRecord	oldExtents,
	HFSPlusExtentRecord	newExtents);

OSErr UpdateExtentRecord (
	const SVCB		*vcb,
	SFCB					*fcb,
	const HFSPlusExtentKey	*extentFileKey,
	HFSPlusExtentRecord		extentData,
	UInt32					extentBTreeHint);

OSErr ReleaseExtents(
	SVCB 					*vcb,
	const HFSPlusExtentRecord extentRecord,
	UInt32					*numReleasedAllocationBlocks,
	Boolean 				*releasedLastExtent);

OSErr	CheckFileExtents( SGlobPtr GPtr, UInt32 fileNumber, UInt8 forkType, const unsigned char *xattrName,
                          const void *extents, UInt32 *blocksUsed );
OSErr	GetBTreeHeader( SGlobPtr GPtr, SFCB* fcb, BTHeaderRec *header );
OSErr	CompareVolumeBitMap( SGlobPtr GPtr, SInt32 whichBuffer );
OSErr	CompareVolumeHeader( SGlobPtr GPtr, HFSPlusVolumeHeader *vh );
OSErr	CreateExtentsBTreeControlBlock( SGlobPtr GPtr );
OSErr	CreateCatalogBTreeControlBlock( SGlobPtr GPtr );
OSErr	CreateAttributesBTreeControlBlock( SGlobPtr GPtr );
OSErr	CreateExtendedAllocationsFCB( SGlobPtr GPtr );


OSErr	CacheWriteInPlace( SVCB *vcb, UInt32 fileRefNum,  HIOParam *iopb, UInt64 currentPosition,
	UInt32 maximumBytes, UInt32 *actualBytes );


/* Generic B-tree call back routines */
OSStatus GetBlockProc (SFCB *filePtr, UInt32 blockNum, GetBlockOptions options, BlockDescriptor *block);
OSStatus ReleaseBlockProc (SFCB *filePtr, BlockDescPtr blockPtr, ReleaseBlockOptions options);
OSStatus SetEndOfForkProc (SFCB *filePtr, FSSize minEOF, FSSize maxEOF);
OSStatus SetBlockSizeProc (SFCB *filePtr, ByteCount blockSize, ItemCount minBlockCount);

void DFA_PrepareInputName(ConstStr31Param name, Boolean isHFSPlus, CatalogName *catalogName);

extern	UInt32	CatalogNameSize( const CatalogName *name, Boolean isHFSPlus);

void	SetupFCB( SVCB *vcb, SInt16 refNum, UInt32 fileID, UInt32 fileClumpSize );


extern	void	CalculateItemCount( SGlob *GPtr, UInt64 *itemCount, UInt64 *onePercent );



//	Macros
extern		BTreeControlBlock*	GetBTreeControlBlock( short refNum );
#define		GetBTreeControlBlock(refNum)	((BTreeControlBlock*) ResolveFCB((refNum))->fcbBtree)

/*	The following macro marks a VCB as dirty by setting the upper 8 bits of the flags*/
EXTERN_API_C( void )
MarkVCBDirty					(SVCB *			vcb);

#define	MarkVCBDirty(vcb)	((void) (vcb->vcbFlags |= 0xFF00))
EXTERN_API_C( void )
MarkVCBClean					(SVCB *			vcb);

#define	MarkVCBClean(vcb)	((void) (vcb->vcbFlags &= 0x00FF))
EXTERN_API_C( Boolean )
IsVCBDirty						(SVCB *			vcb);

#define	IsVCBDirty(vcb)		((Boolean) ((vcb->vcbFlags & 0xFF00) != 0))


extern	pascal void M_Debugger(void);
extern	pascal void M_DebugStr(ConstStr255Param debuggerMsg);
#if ( DEBUG_BUILD )
	#define	M_Debuger()					Debugger()
	#define	M_DebugStr( debuggerMsg )	DebugStr( debuggerMsg )
#else
	#define	M_Debuger()
	#define	M_DebugStr( debuggerMsg )
#endif


/*	Test for error and return if error occurred*/
EXTERN_API_C( void )
ReturnIfError					(OSErr 					result);

#define	ReturnIfError(result)					if ( (result) != noErr ) return (result); else ;
/*	Test for passed condition and return if true*/
EXTERN_API_C( void )
ReturnErrorIf					(Boolean 				condition,
								 OSErr 					result);

#define	ReturnErrorIf(condition, error)			if ( (condition) )	return( (error) );
/*	Exit function on error*/
EXTERN_API_C( void )
ExitOnError						(OSErr 					result);

#define	ExitOnError( result )					if ( ( result ) != noErr )	goto ErrorExit; else ;

/*	Return the low 16 bits of a 32 bit value, pinned if too large*/
EXTERN_API_C( UInt16 )
LongToShort						(UInt32 				l);

#define	LongToShort( l )	l <= (UInt32)0x0000FFFF ? ((UInt16) l) : ((UInt16) 0xFFFF)


EXTERN_API_C( UInt32 )
GetDFAStage						(void);

EXTERN_API_C(OSErr)
DeleteCatalogNode(SVCB *vcb, UInt32 pid, const CatalogName * name, UInt32 hint, Boolean for_rename);

EXTERN_API_C(OSErr)
GetCatalogNode(SVCB *vcb, UInt32 pid, const CatalogName * name, UInt32 hint, CatalogRecord *data);

EXTERN_API_C( SInt32 )
CompareCatalogKeys				(HFSCatalogKey *		searchKey,
								 HFSCatalogKey *		trialKey);

EXTERN_API_C( SInt32 )
CompareExtendedCatalogKeys		(HFSPlusCatalogKey *	searchKey,
								 HFSPlusCatalogKey *	trialKey);
EXTERN_API_C( SInt32 )
CaseSensitiveCatalogKeyCompare (HFSPlusCatalogKey * searchKey,
                                HFSPlusCatalogKey * trialKey);

EXTERN_API_C( SInt32 )
CompareExtentKeys				(const HFSExtentKey *	searchKey,
								 const HFSExtentKey *	trialKey);

EXTERN_API_C( SInt32 )
CompareExtentKeysPlus			(const HFSPlusExtentKey * searchKey,
								 const HFSPlusExtentKey * trialKey);
EXTERN_API_C( SInt32 )
CompareAttributeKeys			(const AttributeKey * searchKey,  const AttributeKey * trialKey);
EXTERN_API( SFCB* )
ResolveFCB						(short 					fileRefNum);

EXTERN_API_C( OSErr )
ValidVolumeHeader				(HFSPlusVolumeHeader *	volumeHeader);


/* Old B-tree Manager API (going away soon!) */

EXTERN_API_C( OSErr )
SearchBTreeRecord				(SFCB 				*fcb,
								 const void *			key,
								 UInt32 				hint,
								 void *					foundKey,
								 void *					data,
								 UInt16 *				dataSize,
								 UInt32 *				newHint);

EXTERN_API_C( OSErr )
GetBTreeRecord					(SFCB 				*fcb,
								 SInt16 				selectionIndex,
								 void *					key,
								 void *					data,
								 UInt16 *				dataSize,
								 UInt32 *				newHint);

EXTERN_API_C( OSErr )
InsertBTreeRecord				(SFCB 				*fcb,
								 const void *			key,
								 const void *			data,
								 UInt16 				dataSize,
								 UInt32 *				newHint);

EXTERN_API_C( OSErr )
DeleteBTreeRecord				(SFCB 				*fcb,
								 const void *			key);

EXTERN_API_C( OSErr )
ReplaceBTreeRecord				(SFCB 				*fcb,
								 const void *			key,
								 UInt32 				hint,
								 void *					newData,
								 UInt16 				dataSize,
								 UInt32 *				newHint);

EXTERN_API_C( void )
InitBTreeHeader					(UInt32 				fileSize,
								 UInt32 				clumpSize,
								 UInt16 				nodeSize,
								 UInt16 				recordCount,
								 UInt16 				keySize,
								 UInt32 				attributes,
								 UInt32 *				mapNodes,
								 void *					buffer);

EXTERN_API_C( OSErr )
UpdateFreeCount					(SVCB *			vcb);


EXTERN_API_C(Boolean)
NodesAreContiguous(	SFCB		*fcb,
			UInt32		nodeSize);



UInt32 GetTimeUTC(void);
UInt32 GetTimeLocal(Boolean forHFS);

OSErr FlushVolumeControlBlock( SVCB *vcb );

pascal short ResolveFileRefNum(SFCB * fileCtrlBlockPtr);

extern UInt32	CatalogNameLength( const CatalogName *name, Boolean isHFSPlus);

extern void		CopyCatalogName( const CatalogName *srcName, CatalogName *dstName, Boolean isHFSPLus);

extern	void	UpdateCatalogName( ConstStr31Param srcName, Str31 destName);

extern	void	BuildCatalogKey( HFSCatalogNodeID parentID, const CatalogName *name, Boolean isHFSPlus,
								 CatalogKey *key);

extern void		UpdateVolumeEncodings( SVCB *volume, TextEncoding encoding);


OSErr BlockAllocate (SVCB *vcb, UInt32 startingBlock, UInt32 blocksRequested, UInt32 blocksMaximum,
			Boolean forceContiguous, UInt32 *actualStartBlock, UInt32 *actualNumBlocks);
OSErr	BlockDeallocate ( SVCB *vcb, UInt32 firstBlock, UInt32 numBlocks);
UInt32	DivideAndRoundUp( UInt32 numerator, UInt32 denominator);
OSErr	BlockFindAll(SFCB *fcb, UInt32 needed);

OSErr InitializeBlockCache ( UInt32 blockSize, UInt32 blockCount );

void	SetFCBSPtr( Ptr value );
Ptr	GetFCBSPtr( void );


/* 
 * UTF-8 conversion routines
 */
extern int utf_decodestr(const unsigned char *, size_t, u_int16_t *, size_t *, size_t);
extern int utf_encodestr(const u_int16_t *, size_t, unsigned char *, size_t *, size_t);

/* 
 * HardLink checking routines
 */
extern int   HardLinkCheckBegin(SGlobPtr gp, void** cookie);
extern void  HardLinkCheckEnd(void * cookie);
extern void  CaptureHardLink(void * cookie, const HFSPlusCatalogFile *file);
extern int   CheckHardLinks(void *cookie);

extern void hardlink_add_bucket(PrimeBuckets *bucket, uint32_t inode_id, uint32_t cur_link_id);
extern int inode_check(SGlobPtr, PrimeBuckets *, CatalogRecord *, CatalogKey *, Boolean);
extern void record_link_badchain(SGlobPtr, Boolean);
extern int record_link_badflags(SGlobPtr, uint32_t, Boolean, uint32_t, uint32_t);
extern int record_inode_badflags(SGlobPtr, uint32_t, Boolean, uint32_t, uint32_t, Boolean);
extern int record_dirlink_badownerflags(SGlobPtr, uint32_t, uint8_t, uint8_t, int);
extern int record_link_badfinderinfo(SGlobPtr, uint32_t, Boolean);

extern int get_first_link_id(SGlobPtr gptr, CatalogRecord *inode_rec, uint32_t inode_id, Boolean isdir, uint32_t *first_link_id);
extern int filelink_hash_inode(UInt32 inode_id, UInt32 linkCount);

/* 
 * Directory Hard Link checking routines 
 */
extern int dirhardlink_init(SGlobPtr gptr);
extern int dirhardlink_check(SGlobPtr gptr);

extern OSErr GetCatalogRecordByID(SGlobPtr GPtr, UInt32 file_id, Boolean isHFSPlus, CatalogKey *key, CatalogRecord *rec, uint16_t *recsize);

struct HardLinkInfo;
extern int RepairHardLinkChains(SGlobPtr, Boolean);

/*
 * Volume Bitmap checking routines
 */
extern int  BitMapCheckBegin(SGlobPtr g);
extern int  BitMapCheckEnd(void);
extern int  CaptureBitmapBits(UInt32 startBit, UInt32 bitCount);
extern int  ReleaseBitmapBits(UInt32 startBit, UInt32 bitCount);
extern int  CheckVolumeBitMap(SGlobPtr g, Boolean repair);
extern void UpdateFreeBlockCount(SGlobPtr g);
extern int 	AllocateContigBitmapBits (SVCB *vcb, UInt32 numBlocks, UInt32 *actualStartBlock);
extern int  IsTrimSupported(void);
extern void TrimFreeBlocks(SGlobPtr g);

/*
 * Variables and routines to support mapping a physical block number to a
 * file path
 */
struct found_blocks {
	u_int64_t block;
	u_int32_t fileID;
	u_int32_t padding;
};
#define FOUND_BLOCKS_QUANTUM	30
extern int gBlkListEntries;
extern u_int64_t *gBlockList;
extern int gFoundBlockEntries;
extern struct found_blocks *gFoundBlocksList;
extern long gBlockSize;
void CheckPhysicalMatch(SVCB *vcb, UInt32 startblk, UInt32 blkcount, UInt32 fileNumber, UInt8 forkType);
void dumpblocklist(SGlobPtr GPtr);

#ifdef __cplusplus
};
#endif

#endif /* __SCAVENGER__ */
