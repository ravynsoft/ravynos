/*
 * Copyright (c) 1999-2016 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.2 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 Copyright (c) 1987-99 Apple Computer, Inc.
 All Rights Reserved.

 About hfs.util.m:
 Contains code to implement hfs utility used by the WorkSpace to mount HFS.

 Change History:
     5-Jan-1999 Don Brady       Write hfs.label names in UTF-8.
    10-Dec-1998 Pat Dirks       Changed to try built-in hfs filesystem first.
     3-Sep-1998	Don Brady	Disable the daylight savings time stuff.
    28-Aug-1998 chw		Fixed parse args and verify args to indicate that the
			        flags (fixed or removable) are required in the probe case.
    22-Jun-1998	Pat Dirks	Changed HFSToUFSStr table to switch ":" and "/".
    13-Jan-1998 jwc 		first cut (derived from old NextStep macfs.util code and cdrom.util code).
 */


/* ************************************** I N C L U D E S ***************************************** */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/sysctl.h>
#include <sys/resource.h>
#include <sys/vmmeter.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/disk.h>
#include <sys/loadable_fs.h>
#include <sys/attr.h>
#include <hfs/hfs_format.h>
#include <hfs/hfs_mount.h>
#include <err.h>
#include <assert.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

/*
 * CommonCrypto provides a more stable API than OpenSSL guarantees;
 * the #define causes it to use the same API for MD5 and SHA1, so the rest of
 * the code need not change.
 */
#define COMMON_DIGEST_FOR_OPENSSL
#include  <CommonCrypto/CommonDigest.h>

#include <libkern/OSByteOrder.h>

#include <CoreFoundation/CFString.h>

#include <uuid/uuid.h>
#include <System/uuid/namespace.h>

#define READ_DEFAULT_ENCODING 1

#ifndef FSUC_ADOPT
#define FSUC_ADOPT 'a'
#endif

#ifndef FSUC_DISOWN
#define FSUC_DISOWN 'd'
#endif

#ifndef FSUC_GETUUID
#define FSUC_GETUUID 'k'
#endif

#ifndef FSUC_SETUUID
#define FSUC_SETUUID 's'
#endif

#ifndef FSUC_MKJNL
#define FSUC_MKJNL   'J'
#endif

#ifndef FSUC_UNJNL
#define FSUC_UNJNL   'U'
#endif

#ifndef FSUC_UNJNL_RAW
#define FSUC_UNJNL_RAW 'N'
#endif

#ifndef FSUC_JNLINFS_RAW
#define FSUC_JNLINFS_RAW 'e'
#endif

#ifndef FSUC_EXTJNL_RAW
#define FSUC_EXTJNL_RAW 'E'
#endif

#ifndef FSUC_JNLINFO
#define FSUC_JNLINFO 'I'
#endif
 

/* **************************************** L O C A L S ******************************************* */

#define kHFSPlusMaxFileNameBytes	(3 * 255 + 1)	/* 255 unicode characters, plus 1 NUL byte */

#define	HFS_BLOCK_SIZE			512

char gHFS_FS_NAME[] = "hfs";
char gHFS_FS_NAME_NAME[] = "HFS";

char gNewlineString[] = "\n";

char gMountCommand[] = "/sbin/mount";

char gUnmountCommand[] = "/sbin/umount";

char gReadOnlyOption[] = "-r";
char gReadWriteOption[] = "-w";

char gSuidOption[] = "suid";
char gNoSuidOption[] = "nosuid";

char gDevOption[] = "dev";
char gNoDevOption[] = "nodev";

char gUsePermissionsOption[] = "perm";
char gIgnorePermissionsOption[] = "noperm";

boolean_t gIsEjectable = 0;

int gJournalSize = 0;

#define AUTO_ADOPT_FIXED 1
#define AUTO_ENTER_FIXED 0


typedef struct FinderAttrBuf {
	u_int32_t info_length;
	u_int32_t finderinfo[8];
} FinderAttrBuf_t;


/* For requesting the UUID from the FS */
typedef struct UUIDAttrBuf {
	uint32_t info_length;
	uuid_t uu;
} UUIDAttrBuf_t;

/* HFS+ internal representation of UUID */
typedef struct hfs_UUID {
	uint32_t high;
	uint32_t low;
} hfs_UUID_t;	

/* an actual UUID */
typedef struct volUUID {
	uuid_t uuid;
} volUUID_t;


#define HFSUUIDLENGTH 16

#define VOLUME_RECORDED 0x80000000
#define VOLUME_USEPERMISSIONS 0x00000001
#define VOLUME_VALIDSTATUSBITS ( VOLUME_USEPERMISSIONS )

typedef void *VolumeStatusDBHandle;

/* UUID generation and conversion functions */
void GenerateHFSVolumeUUID(hfs_UUID_t *hfsuu);
void ConvertHFSUUIDStringToUUID (const char* UUIDString, volUUID_t *volid);
void ConvertHFSUUIDToUUID (hfs_UUID_t *hfsuuid, volUUID_t *uu);

/* 
 * Volume Database manipulation routines
 * These functions MUST manipulate the VSDB in the same way that vsdbutil does
 */
int ConvertVolumeStatusDB(VolumeStatusDBHandle DBHandle);
int OpenVolumeStatusDB(VolumeStatusDBHandle *DBHandlePtr);
int GetVolumeStatusDBEntry(VolumeStatusDBHandle DBHandle, volUUID_t *volumeID, unsigned long *VolumeStatus);
int SetVolumeStatusDBEntry(VolumeStatusDBHandle DBHandle, volUUID_t *volumeID, unsigned long VolumeStatus);
int DeleteVolumeStatusDBEntry(VolumeStatusDBHandle DBHandle, volUUID_t *volumeID);
int CloseVolumeStatusDB(VolumeStatusDBHandle DBHandle);

/* ************************************ P R O T O T Y P E S *************************************** */
static void	DoDisplayUsage( const char * argv[] );
static int	DoMount( char * theDeviceNamePtr, const char *rawName, const char * theMountPointPtr, 
					boolean_t isLocked, boolean_t isSetuid, boolean_t isDev );
static int 	DoProbe( char * rawDeviceNamePtr, char * blockDeviceNamePtr );
static int 	DoUnmount( const char * theMountPointPtr );
static int	DoGetUUIDKey( const char * theDeviceNamePtr, const char *rawName );
static int	DoChangeUUIDKey( const char * theDeviceNamePtr );
static int	DoAdopt( const char * theDeviceNamePtr, const char *rawName);
static int	DoDisown( const char * theDeviceNamePtr, const char *rawName);

extern int  DoMakeJournaled( const char * volNamePtr, int journalSize );  // XXXdbg
extern int  DoUnJournal( const char * volNamePtr );      // XXXdbg
extern int  DoGetJournalInfo( const char * volNamePtr );
extern int  RawDisableJournaling( const char *devname );
extern int  SetJournalInFSState( const char *devname, int journal_in_fs);

static int	ParseArgs( int argc, const char * argv[], const char ** actionPtr, const char ** mountPointPtr, boolean_t * isEjectablePtr, boolean_t * isLockedPtr, boolean_t * isSetuidPtr, boolean_t * isDevPtr );
static int	GetHFSMountPoint(const char *deviceNamePtr, char **pathPtr);

/* Helper functions for manipulating HFS and full UUIDs */
static int	ReadHeaderBlock(int fd, void *bufptr, off_t *startOffset, hfs_UUID_t **finderInfoUUIDPtr);
static int	GetVolumeUUIDRaw(const char *deviceNamePtr, const char *rawName, volUUID_t *volumeUUIDPtr);
static int	GetVolumeUUIDAttr(const char *path, volUUID_t *volumeUUIDPtr);
static int	GetVolumeUUID(const char *deviceNamePtr, const char *rawName, volUUID_t *volumeUUIDPtr, boolean_t generate);
static int	SetVolumeUUIDRaw(const char *deviceNamePtr, hfs_UUID_t *hfsuu);
static int	SetVolumeUUIDAttr(const char *path, hfs_UUID_t *hfsuu);
static int	SetVolumeUUID(const char *deviceNamePtr, hfs_UUID_t *hfsuu);


static int	GetEmbeddedHFSPlusVol(HFSMasterDirectoryBlock * hfsMasterDirectoryBlockPtr, off_t * startOffsetPtr);
static int	GetNameFromHFSPlusVolumeStartingAt(int fd, off_t hfsPlusVolumeOffset, unsigned char * name_o);
static int	GetBTreeNodeInfo(int fd, off_t hfsPlusVolumeOffset, u_int32_t blockSize,
							u_int32_t extentCount, const HFSPlusExtentDescriptor *extentList,
							u_int32_t *nodeSize, u_int32_t *firstLeafNode);
static int	GetCatalogOverflowExtents(int fd, off_t hfsPlusVolumeOffset, HFSPlusVolumeHeader *volHdrPtr,
									 HFSPlusExtentDescriptor **catalogExtents, u_int32_t *catalogExtCount);
static int	LogicalToPhysical(off_t logicalOffset, ssize_t length, u_int32_t blockSize,
							u_int32_t extentCount, const HFSPlusExtentDescriptor *extentList,
							off_t *physicalOffset, ssize_t *availableBytes);
static int	ReadFile(int fd, void *buffer, off_t offset, ssize_t length,
					off_t volOffset, u_int32_t blockSize,
					u_int32_t extentCount, const HFSPlusExtentDescriptor *extentList);
static ssize_t	readAt( int fd, void * buf, off_t offset, ssize_t length );
static ssize_t	writeAt( int fd, void * buf, off_t offset, ssize_t length );

static int	GetEncodingBias(void);


CF_EXPORT Boolean _CFStringGetFileSystemRepresentation(CFStringRef string, UInt8 *buffer, CFIndex maxBufLen);

static void     uuid_create_md5_from_name(uuid_t result_uuid, const uuid_t namespace, const void *name, int namelen);

/*
 * The fuction CFStringGetSystemEncoding does not work correctly in
 * our context (autodiskmount deamon).  We include a local copy here
 * so that we can derive the default encoding. Radar 2516316.
 */
#if READ_DEFAULT_ENCODING
#define __kCFUserEncodingFileName ("/.CFUserTextEncoding")

static unsigned int __CFStringGetDefaultEncodingForHFSUtil() {
    struct passwd *passwdp;

    if ((passwdp = getpwuid(0))) { // root account
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
    return 0; // Fallback to smRoman
}
#endif


#define MXENCDNAMELEN	16	/* Maximun length of encoding name string */

struct hfs_mnt_encoding {
	char				encoding_name[MXENCDNAMELEN];	/* encoding type name */
	CFStringEncoding	encoding_id;					/* encoding type number */
};

static struct hfs_mnt_encoding hfs_mnt_encodinglist[] = {
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

#define KEXT_LOAD_COMMAND	"/sbin/kextload"
#define ENCODING_MODULE_PATH	"/System/Library/Filesystems/hfs.fs/Contents/Resources/Encodings/"

static int load_encoding(CFStringEncoding encoding)
{
	int i;
	int numEncodings;
	int pid;
	char *encodingName;
	struct stat sb;
	union wait status;
	char kmodfile[MAXPATHLEN];

	/* Find the encoding that matches the one passed in */
	numEncodings = sizeof(hfs_mnt_encodinglist) / sizeof(struct hfs_mnt_encoding);
	encodingName = NULL;
	for (i=0; i<numEncodings; ++i)
	{
		if (hfs_mnt_encodinglist[i].encoding_id == encoding)
		{
			encodingName = hfs_mnt_encodinglist[i].encoding_name;
			break;
		}
	}
	
	if (encodingName == NULL)
	{
		/* Couldn't figure out which encoding KEXT to load */
		syslog(LOG_ERR, "Couldn't find name for encoding #%d", encoding);
		return FSUR_LOADERR;
	}
	
	snprintf(kmodfile, sizeof(kmodfile), "%sHFS_Mac%s.kext", ENCODING_MODULE_PATH, encodingName);
	if (stat(kmodfile, &sb) == -1)
	{
		/* We recognized the encoding, but couldn't find the KEXT */
		syslog(LOG_ERR, "Couldn't stat HFS_Mac%s.kext: %s", encodingName, strerror(errno));
		return FSUR_LOADERR;
	}
	
	pid = fork();
	if (pid == 0)
	{
		(void) execl(KEXT_LOAD_COMMAND, KEXT_LOAD_COMMAND, "-q", kmodfile, NULL);

		exit(1);	/* We can only get here if the exec failed */
	}
	else if (pid != -1)
	{
		if ((waitpid(pid, (int *)&status, 0) == pid) && WIFEXITED(status))
		{
			if (WEXITSTATUS(status) != 0)
			{
				/* kextload returned an error.  Too bad its output doesn't get logged. */
				syslog(LOG_ERR, "Couldn't load HFS_Mac%s.kext", encodingName);
				return FSUR_LOADERR;
			}
		}
	}
	
	return FSUR_IO_SUCCESS;
}


/* ******************************************** main ************************************************
Purpose -
This our main entry point to this utility.  We get called by the WorkSpace.  See ParseArgs
for detail info on input arguments.
Input -
argc - the number of arguments in argv.
argv - array of arguments.
Output -
returns FSUR_IO_SUCCESS if OK else one of the other FSUR_xyz errors in loadable_fs.h.
*************************************************************************************************** */

int main (int argc, const char *argv[])
{
    const char			*	actionPtr = NULL;
    char				rawDeviceName[MAXPATHLEN];
    char				blockDeviceName[MAXPATHLEN];
    const char			*	mountPointPtr = NULL;
    int						result = FSUR_IO_SUCCESS;
    boolean_t				isLocked = 0;	/* reasonable assumptions */
    boolean_t				isSetuid = 0;	/* reasonable assumptions */
    boolean_t				isDev = 0;	/* reasonable assumptions */

	openlog("hfs.util", LOG_PID, LOG_DAEMON);

    /* Verify our arguments */
    if ( (result = ParseArgs( argc, argv, & actionPtr, & mountPointPtr, & gIsEjectable, & isLocked, &isSetuid, &isDev )) != 0 ) {
        goto AllDone;
    }

    /*
    -- Build our device name (full path), should end up with something like:
    --   "/dev/disk0s2"
    */

    snprintf(rawDeviceName, sizeof(rawDeviceName), "/dev/r%s", argv[2]);
    snprintf(blockDeviceName, sizeof(blockDeviceName), "/dev/%s", argv[2]);

    /* call the appropriate routine to handle the given action argument after becoming root */

    switch( * actionPtr ) {
        case FSUC_PROBE:
            result = DoProbe(rawDeviceName, blockDeviceName);
            break;

        case FSUC_MOUNT:
        case FSUC_MOUNT_FORCE:
            result = DoMount(blockDeviceName, rawDeviceName, mountPointPtr, isLocked, isSetuid, isDev);
            break;

        case FSUC_UNMOUNT:
            result = DoUnmount( mountPointPtr );
            break;
		case FSUC_GETUUID:
			result = DoGetUUIDKey( blockDeviceName, rawDeviceName);
			break;
		
		case FSUC_SETUUID:
			result = DoChangeUUIDKey( blockDeviceName );
			break;
		case FSUC_ADOPT:
			result = DoAdopt( blockDeviceName, rawDeviceName);
			break;
		
		case FSUC_DISOWN:
			result = DoDisown( blockDeviceName, rawDeviceName );
			break;

		case FSUC_MKJNL:
			if (gJournalSize) {
				result = DoMakeJournaled( argv[3], gJournalSize );
			} else {
				result = DoMakeJournaled( argv[2], gJournalSize );
			}
			break;

		case FSUC_UNJNL:
			result = DoUnJournal( argv[2] );
			break;
			
		case FSUC_UNJNL_RAW:
			result = RawDisableJournaling( argv[2] );
			break;
			
		case FSUC_JNLINFS_RAW:
			// argv[2] has the device for the external journal.  however
			// we don't need it so we ignore it and just pass argv[3]
			// which is the hfs volume whose state we're going to change
			//
			result = SetJournalInFSState( argv[3], 1 );
			break;
			
		case FSUC_EXTJNL_RAW:
			// see the comment for FSUC_JNLINFS_RAW
			result = SetJournalInFSState( argv[3], 0 );
			break;
			
		case FSUC_JNLINFO:
			result = DoGetJournalInfo( argv[2] );
			break;

        default:
            /* should never get here since ParseArgs should handle this situation */
            DoDisplayUsage( argv );
            result = FSUR_INVAL;
            break;
    }

AllDone:

    exit(result);

    return result;	/*...and make main fit the ANSI spec. */
}


/* ***************************** DoMount ********************************
Purpose -
This routine will fire off a system command to mount the given device at the given mountpoint.
autodiskmount will make sure the mountpoint exists and will remove it at Unmount time.
Input -
deviceNamePtr - pointer to the device name (full path, like /dev/disk0s2).
mountPointPtr - pointer to the mount point.
isLocked - a flag
Output -
returns FSUR_IO_SUCCESS everything is cool else one of several other FSUR_xyz error codes.
*********************************************************************** */
static int
DoMount(char *deviceNamePtr, const char *rawName, const char *mountPointPtr,
		boolean_t isLocked, boolean_t isSetuid, boolean_t isDev)
{
	int pid;
        char *isLockedstr;
        char *isSetuidstr;
        char *isDevstr;
	char *permissionsOption;
	int result = FSUR_IO_FAIL;
	union wait status;
	char encodeopt[16] = "";
	CFStringEncoding encoding;
	volUUID_t targetVolUUID;
	VolumeStatusDBHandle vsdbhandle = NULL;
	unsigned long targetVolumeStatus;

	if (mountPointPtr == NULL || *mountPointPtr == '\0')
		return (FSUR_IO_FAIL);

	/* get the volume UUID to check if permissions should be used: */
	targetVolumeStatus = 0;
	if (((result = GetVolumeUUID(deviceNamePtr, rawName, &targetVolUUID, FALSE)) != FSUR_IO_SUCCESS) ||
			(uuid_is_null(targetVolUUID.uuid))) {
#if TRACE_HFS_UTIL
		fprintf(stderr, "hfs.util: DoMount: GetVolumeUUID returned %d.\n", result);
#endif
#if AUTO_ADOPT_FIXED
		if (gIsEjectable == 0) {
			result = DoAdopt( deviceNamePtr, rawName);
#if TRACE_HFS_UTIL
			fprintf(stderr, "hfs.util: DoMount: Auto-adopting %s; result = %d.\n", deviceNamePtr, result);
#endif
			targetVolumeStatus = VOLUME_USEPERMISSIONS;
		} else {
#if TRACE_HFS_UTIL
			fprintf(stderr, "hfs.util: DoMount: Not adopting ejectable %s.\n", deviceNamePtr);
#endif
			targetVolumeStatus = 0;
		}
#endif
	} else {
		/* We've got a real volume UUID! */
		if ((result = OpenVolumeStatusDB(&vsdbhandle)) != 0) {
			/* Can't even get access to the volume info db; assume permissions are OK. */
#if TRACE_HFS_UTIL
			fprintf(stderr, "hfs.util: DoMount: OpenVolumeStatusDB returned %d; ignoring permissions.\n", result);
#endif
			targetVolumeStatus = VOLUME_USEPERMISSIONS;
		} else {
#if TRACE_HFS_UTIL
			fprintf(stderr, "hfs.util: DoMount: Looking up volume status...\n");
#endif
			if ((result = GetVolumeStatusDBEntry(vsdbhandle, &targetVolUUID, &targetVolumeStatus)) != 0) {
#if TRACE_HFS_UTIL
				fprintf(stderr, "hfs.util: DoMount: GetVolumeStatusDBEntry returned %d.\n", result);
#endif
#if AUTO_ENTER_FIXED
				if (gIsEjectable == 0) {
					result = DoAdopt( deviceNamePtr, rawName );
#if TRACE_HFS_UTIL
					fprintf(stderr, "hfs.util: DoMount: Auto-adopting %s; result = %d.\n", deviceNamePtr, result);
#endif
					targetVolumeStatus = VOLUME_USEPERMISSIONS;
				} else {
#if TRACE_HFS_UTIL
					fprintf(stderr, "hfs.util: DoMount: Not adopting ejectable %s.\n", deviceNamePtr);
#endif
					targetVolumeStatus = 0;
				}
#else
				targetVolumeStatus = 0;
#endif
			}
			(void)CloseVolumeStatusDB(vsdbhandle);
			vsdbhandle = NULL;
		}
	}
	
	pid = fork();
	if (pid == 0) {
                isLockedstr = isLocked ? gReadOnlyOption : gReadWriteOption;
                isSetuidstr = isSetuid ? gSuidOption : gNoSuidOption;
                isDevstr = isDev ? gDevOption : gNoDevOption;
		
		permissionsOption =
			(targetVolumeStatus & VOLUME_USEPERMISSIONS) ? gUsePermissionsOption : gIgnorePermissionsOption;

		/* get default encoding value (for hfs volumes) */
#if READ_DEFAULT_ENCODING
		encoding = __CFStringGetDefaultEncodingForHFSUtil();
#else
		encoding = CFStringGetSystemEncoding();
#endif
		snprintf(encodeopt, sizeof(encodeopt), "-e=%d", (int)encoding);
#if TRACE_HFS_UTIL
		fprintf(stderr, "hfs.util: %s %s -o -x -o %s -o %s -o -u=unknown,-g=unknown,-m=0777 -t %s %s %s ...\n",
							gMountCommand, isLockedstr, encodeopt, permissionsOption, gHFS_FS_NAME, deviceNamePtr, mountPointPtr);
#endif
                (void) execl(gMountCommand, gMountCommand, isLockedstr, "-o", isSetuidstr, "-o", isDevstr,
                                        "-o", encodeopt, "-o", permissionsOption,
                                        "-o", "-u=unknown,-g=unknown,-m=0777",
                                        "-t", gHFS_FS_NAME, deviceNamePtr, mountPointPtr, NULL);
                

		/* IF WE ARE HERE, WE WERE UNSUCCESFUL */
		return (FSUR_IO_FAIL);
	}

	if (pid == -1)
		return (FSUR_IO_FAIL);

	/* Success! */
	if ((wait4(pid, (int *)&status, 0, NULL) == pid) && (WIFEXITED(status)))
		result = status.w_retcode;
	else
		result = -1;

	return (result == 0) ? FSUR_IO_SUCCESS : FSUR_IO_FAIL;
}


/* ****************************************** DoUnmount *********************************************
Purpose -
    This routine will fire off a system command to unmount the given device.
Input -
    theDeviceNamePtr - pointer to the device name (full path, like /dev/disk0s2).
Output -
    returns FSUR_IO_SUCCESS everything is cool else FSUR_IO_FAIL.
*************************************************************************************************** */
static int
DoUnmount(const char * theMountPointPtr)
{
	int pid;
	union wait status;
	int result;

	if (theMountPointPtr == NULL || *theMountPointPtr == '\0') return (FSUR_IO_FAIL);

	pid = fork();
	if (pid == 0) {
#if TRACE_HFS_UTIL
		fprintf(stderr, "hfs.util: %s %s ...\n", gUnmountCommand, theMountPointPtr);
#endif
		(void) execl(gUnmountCommand, gUnmountCommand, theMountPointPtr, NULL);

		/* IF WE ARE HERE, WE WERE UNSUCCESFULL */
		return (FSUR_IO_FAIL);
	}

	if (pid == -1)
		return (FSUR_IO_FAIL);

	/* Success! */
	if ((wait4(pid, (int *)&status, 0, NULL) == pid) && (WIFEXITED(status)))
		result = status.w_retcode;
	else
		result = -1;

	return (result == 0 ? FSUR_IO_SUCCESS : FSUR_IO_FAIL);

} /* DoUnmount */


/*
	PrintVolumeNameAttr
	
	Get the volume name of the volume mounted at "path".  Print that volume
	name to standard out.

	Returns: FSUR_RECOGNIZED, FSUR_IO_FAIL
*/
struct VolumeNameBuf {
	u_int32_t	info_length;
	attrreference_t	name_ref;
	char		buffer[1024];
};

static int
PrintVolumeNameAttr(const char *path)
{
	struct attrlist alist;
	struct VolumeNameBuf volNameInfo;
	int result;

	/* Set up the attrlist structure to get the volume's Finder Info */
	memset (&alist, 0, sizeof(alist));
	alist.bitmapcount = 5;
	alist.reserved = 0;
	alist.commonattr = 0;
	alist.volattr = ATTR_VOL_INFO | ATTR_VOL_NAME;
	alist.dirattr = 0;
	alist.fileattr = 0;
	alist.forkattr = 0;
	

	/* Get the Finder Info */
	result = getattrlist(path, &alist, &volNameInfo, sizeof(volNameInfo), 0);
	if (result) {
		result = FSUR_IO_FAIL;
		goto Err_Exit;
	}

	/* Print the name to standard out */
	printf("%.*s", (int) volNameInfo.name_ref.attr_length, ((char *) &volNameInfo.name_ref) + volNameInfo.name_ref.attr_dataoffset);
	result = FSUR_RECOGNIZED;

Err_Exit:
	return result;
}


/* ******************************************* DoProbe **********************************************
Purpose -
    This routine will open the given device and check to make sure there is media that looks
    like an HFS.  If it is HFS, then print the volume name to standard output.
Input -
    rawDeviceNamePtr - pointer to the full path of the raw device (like /dev/rdisk0s2).
    blockDeviceNamePtr - pointer to the full path of the non-raw device (like /dev/disk0s2).
Output -
    returns FSUR_RECOGNIZED if we can handle the media else one of the FSUR_xyz error codes.
*************************************************************************************************** */
static int
DoProbe(char *rawDeviceNamePtr, char *blockDeviceNamePtr)
{
	int result = FSUR_UNRECOGNIZED;
	int fd = 0;
	char * bufPtr;
	HFSMasterDirectoryBlock * mdbPtr;
	HFSPlusVolumeHeader * volHdrPtr;
	u_char volnameUTF8[kHFSPlusMaxFileNameBytes];

	/*
	 * Determine if there is a volume already mounted from this device.  If
	 * there is, and it is HFS, then we need to get the volume name via
	 * getattrlist.
	 *
	 * NOTE: We're using bufPtr to hold a pointer to a path.
	 */
	bufPtr = NULL;
	result = GetHFSMountPoint(blockDeviceNamePtr, &bufPtr);
	if (result != FSUR_IO_SUCCESS) {
		goto Err_Exit;
	}
	if (bufPtr != NULL) {
		/* There is an HFS volume mounted from the device. */
		result = PrintVolumeNameAttr(bufPtr);
		goto Err_Exit;
	}
	
	/*
	 * If we get here, there is no volume mounted from this device, so
	 * go probe the raw device directly.
	 */
	
	bufPtr = (char *)malloc(HFS_BLOCK_SIZE);
	if ( ! bufPtr ) {
		result = FSUR_UNRECOGNIZED;
		goto Return;
	}

	mdbPtr = (HFSMasterDirectoryBlock *) bufPtr;
	volHdrPtr = (HFSPlusVolumeHeader *) bufPtr;

	fd = open( rawDeviceNamePtr, O_RDONLY, 0 );
	if( fd <= 0 ) {
		result = FSUR_IO_FAIL;
		goto Return;
	}

	/*
	 * Read the HFS Master Directory Block from sector 2
	 */
	result = (int)readAt(fd, bufPtr, (off_t)(2 * HFS_BLOCK_SIZE), HFS_BLOCK_SIZE);
	if (FSUR_IO_FAIL == result)
		goto Return;

	/* get classic HFS volume name (from MDB) */
	if (OSSwapBigToHostInt16(mdbPtr->drSigWord) == kHFSSigWord &&
	    OSSwapBigToHostInt16(mdbPtr->drEmbedSigWord) != kHFSPlusSigWord) {
	    	Boolean cfOK;
		CFStringRef cfstr;
		CFStringEncoding encoding;

		/* Some poorly mastered HFS CDs have an empty MDB name field! */
		if (mdbPtr->drVN[0] == '\0') {
			strcpy((char *)&mdbPtr->drVN[1], gHFS_FS_NAME_NAME);
			mdbPtr->drVN[0] = strlen(gHFS_FS_NAME_NAME);
		}

		/* Check for an encoding hint in the Finder Info (field 4). */
		encoding = GET_HFS_TEXT_ENCODING(OSSwapBigToHostInt32(mdbPtr->drFndrInfo[4]));
		if (encoding == kCFStringEncodingInvalidId) {
			/* Next try the encoding bias in the kernel. */
			encoding = GetEncodingBias();
			if (encoding == 0 || encoding == kCFStringEncodingInvalidId)
				encoding = __CFStringGetDefaultEncodingForHFSUtil();
		}

		cfstr = CFStringCreateWithPascalString(kCFAllocatorDefault,
			    mdbPtr->drVN, encoding);
		if (cfstr == NULL) {
			result = FSUR_INVAL;
			goto Return;
		}
		cfOK = _CFStringGetFileSystemRepresentation(cfstr, volnameUTF8, NAME_MAX);
		CFRelease(cfstr);

		if (!cfOK && encoding != kCFStringEncodingMacRoman) {

		    	/* default to MacRoman on conversion errors */
			cfstr = CFStringCreateWithPascalString(kCFAllocatorDefault,
				    mdbPtr->drVN, kCFStringEncodingMacRoman);
			_CFStringGetFileSystemRepresentation(cfstr, volnameUTF8, NAME_MAX);
			CFRelease(cfstr);
			encoding = kCFStringEncodingMacRoman;
		}
 
 		/* Preload the encoding converter so mount_hfs can run as an ordinary user. */
 		if (encoding != kCFStringEncodingMacRoman) {
 			if (load_encoding(encoding) != FSUR_IO_SUCCESS) {
 				encoding = kCFStringEncodingMacRoman;
				cfstr = CFStringCreateWithPascalString(kCFAllocatorDefault, mdbPtr->drVN, encoding);
				_CFStringGetFileSystemRepresentation(cfstr, volnameUTF8, NAME_MAX);
				CFRelease(cfstr);
 			}
 		}
 
 	/* get HFS Plus volume name (from Catalog) */
	} else if ((OSSwapBigToHostInt16(volHdrPtr->signature) == kHFSPlusSigWord)  ||
	           (OSSwapBigToHostInt16(volHdrPtr->signature) == kHFSXSigWord)  ||
		   (OSSwapBigToHostInt16(mdbPtr->drSigWord) == kHFSSigWord &&
		    OSSwapBigToHostInt16(mdbPtr->drEmbedSigWord) == kHFSPlusSigWord)) {
		off_t startOffset;

		if (OSSwapBigToHostInt16(volHdrPtr->signature) == kHFSSigWord) {
			/* embedded volume, first find offset */
			result = GetEmbeddedHFSPlusVol(mdbPtr, &startOffset);
			if ( result != FSUR_IO_SUCCESS )
				goto Return;
		} else {
			startOffset = 0;
		}

		result = GetNameFromHFSPlusVolumeStartingAt(fd, startOffset,
			    volnameUTF8);
	} else {
		result = FSUR_UNRECOGNIZED;
	}

	if (FSUR_IO_SUCCESS == result) {
		/* Print the volume name to standard output */
		write(1, volnameUTF8, strlen((char *)volnameUTF8));
		result = FSUR_RECOGNIZED;
	}

Return:

	if ( bufPtr )
		free( bufPtr );

	if (fd > 0)
		close(fd);
Err_Exit:
	return result;

} /* DoProbe */

/*
 * Create a version 3 UUID from a unique "name" in the given "name space".  
 * Version 3 UUID are derived using "name" via MD5 checksum.
 *
 * Parameters:
 *	result_uuid	- resulting UUID.
 *	namespace	- namespace in which given name exists and UUID should be created.
 *	name		- unique string used to create version 3 UUID.
 *	namelen     - length of the name string.
 */
static void
uuid_create_md5_from_name(uuid_t result_uuid, const uuid_t namespace, const void *name, int namelen)
{
    MD5_CTX c;

    MD5_Init(&c);
    MD5_Update(&c, namespace, sizeof(uuid_t));
    MD5_Update(&c, name, namelen);
    MD5_Final(result_uuid, &c);

    result_uuid[6] = (result_uuid[6] & 0x0F) | 0x30;
    result_uuid[8] = (result_uuid[8] & 0x3F) | 0x80;
}


/* **************************************** DoGetUUIDKey *******************************************
Purpose -
    This routine will open the given block device and return the 128-bit volume UUID in text form written to stdout.
Input -
    theDeviceNamePtr - pointer to the device name (full path, like /dev/disk0s2).
Output -
    returns FSUR_IO_SUCCESS or else one of the FSUR_xyz error codes.
*************************************************************************************************** */
static int
DoGetUUIDKey( const char * theDeviceNamePtr, const char *rawName) {
	int result;
	volUUID_t targetVolumeUUID;
	uuid_string_t uustr;

	result = GetVolumeUUID(theDeviceNamePtr, rawName, &targetVolumeUUID, FALSE);
	if (result == FSUR_IO_SUCCESS) {
		uuid_unparse (targetVolumeUUID.uuid, uustr);
		/* for compatibility, must write out the string to stdout, with NO newline */
		write(STDOUT_FILENO, uustr, strlen(uustr));

		/* for debugging */
		// fprintf(stderr, "device %s UUID : %s\n", rawName, uustr);
	}

	return result;
}



/* *************************************** DoChangeUUIDKey ******************************************
Purpose -
    This routine will change the UUID on the specified block device.
Input -
    theDeviceNamePtr - pointer to the device name (full path, like /dev/disk0s2).
Output -
    returns FSUR_IO_SUCCESS or else one of the FSUR_xyz error codes.
*************************************************************************************************** */
static int
DoChangeUUIDKey( const char * theDeviceNamePtr ) {
	int result;
	hfs_UUID_t newVolumeUUID;
	
	GenerateHFSVolumeUUID(&newVolumeUUID);
#if 0
	// for testing purposes, may want to set a NULL UUID from command line.
	memset (&newVolumeUUID, 0, sizeof(newVolumeUUID));
#endif
	result = SetVolumeUUID(theDeviceNamePtr, &newVolumeUUID);
	
	//fprintf(stderr, "device %s has new UUID \n", theDeviceNamePtr);

	return result;
}



/* **************************************** DoAdopt *******************************************
Purpose -
    This routine will add the UUID of the specified block device to the list of local volumes.
Input -
    theDeviceNamePtr - pointer to the device name (full path, like /dev/disk0s2).
Output -
    returns FSUR_IO_SUCCESS or else one of the FSUR_xyz error codes.
*************************************************************************************************** */
static int
DoAdopt( const char * theDeviceNamePtr, const char *rawName) {
	int result, closeresult;
	volUUID_t targetVolumeUUID;
	VolumeStatusDBHandle vsdbhandle = NULL;
	unsigned long targetVolumeStatus;
	
	if ((result = GetVolumeUUID(theDeviceNamePtr, rawName, &targetVolumeUUID, TRUE)) != FSUR_IO_SUCCESS) goto Err_Return;
	
	if ((result = OpenVolumeStatusDB(&vsdbhandle)) != 0) goto Err_Exit;
	if ((result = GetVolumeStatusDBEntry(vsdbhandle, &targetVolumeUUID, &targetVolumeStatus)) != 0) {
		targetVolumeStatus = 0;
	}
	targetVolumeStatus = (targetVolumeStatus & VOLUME_VALIDSTATUSBITS) | VOLUME_USEPERMISSIONS;
	if ((result = SetVolumeStatusDBEntry(vsdbhandle, &targetVolumeUUID, targetVolumeStatus)) != 0) goto Err_Exit;

	result = FSUR_IO_SUCCESS;
	
Err_Exit:
	if (vsdbhandle) {
		closeresult = CloseVolumeStatusDB(vsdbhandle);
		vsdbhandle = NULL;
		if (result == FSUR_IO_SUCCESS) result = closeresult;
	}
	
	if ((result != 0) && (result != FSUR_IO_SUCCESS)) result = FSUR_IO_FAIL;
	
Err_Return:
#if TRACE_HFS_UTIL
	if (result != FSUR_IO_SUCCESS) fprintf(stderr, "DoAdopt: returning %d...\n", result);
#endif
	return result;
}



/* **************************************** DoDisown *******************************************
Purpose -
    This routine will change the status of the specified block device to ignore its permissions.
Input -
    theDeviceNamePtr - pointer to the device name (full path, like /dev/disk0s2).
Output -
    returns FSUR_IO_SUCCESS or else one of the FSUR_xyz error codes.
*************************************************************************************************** */
static int
DoDisown( const char * theDeviceNamePtr, const char *rawName) {
	int result, closeresult;
	volUUID_t targetVolumeUUID;
	VolumeStatusDBHandle vsdbhandle = NULL;
	unsigned long targetVolumeStatus;
	
	if ((result = GetVolumeUUID(theDeviceNamePtr, rawName, &targetVolumeUUID, TRUE)) != FSUR_IO_SUCCESS) goto Err_Return;
	
	if ((result = OpenVolumeStatusDB(&vsdbhandle)) != 0) goto Err_Exit;
	if ((result = GetVolumeStatusDBEntry(vsdbhandle, &targetVolumeUUID, &targetVolumeStatus)) != 0) {
		targetVolumeStatus = 0;
	}
	targetVolumeStatus = (targetVolumeStatus & VOLUME_VALIDSTATUSBITS) & ~VOLUME_USEPERMISSIONS;
	if ((result = SetVolumeStatusDBEntry(vsdbhandle, &targetVolumeUUID, targetVolumeStatus)) != 0) goto Err_Exit;

	result = FSUR_IO_SUCCESS;
	
Err_Exit:
	if (vsdbhandle) {
		closeresult = CloseVolumeStatusDB(vsdbhandle);
		vsdbhandle = NULL;
		if (result == FSUR_IO_SUCCESS) result = closeresult;
	}
	
	if ((result != 0) && (result != FSUR_IO_SUCCESS)) {
#if TRACE_HFS_UTIL
		if (result != 0) fprintf(stderr, "DoDisown: result = %d; changing to %d...\n", result, FSUR_IO_FAIL);
#endif
		result = FSUR_IO_FAIL;
	}
	
Err_Return:
#if TRACE_HFS_UTIL
	if (result != FSUR_IO_SUCCESS) fprintf(stderr, "DoDisown: returning %d...\n", result);
#endif
	return result;
}


static int
get_multiplier(char c)
{
	if (tolower(c) == 'k') {
		return 1024;
	} else if (tolower(c) == 'm') {
		return 1024 * 1024;
	} else if (tolower(c) == 'g') {
		return 1024 * 1024 * 1024;
	} 

	return 1;
}

/* **************************************** ParseArgs ********************************************
Purpose -
    This routine will make sure the arguments passed in to us are cool.
    Here is how this utility is used:

usage: hfs.util actionArg deviceArg [mountPointArg] [flagsArg]
actionArg:
        -p (Probe for mounting)
       	-P (Probe for initializing - not supported)
       	-m (Mount)
        -r (Repair - not supported)
        -u (Unmount)
        -M (Force Mount)
        -i (Initialize - not supported)

deviceArg:
        disk0s2 (for example)

mountPointArg:
        /foo/bar/ (required for Mount and Force Mount actions)

flagsArg:
        (these are ignored for CDROMs)
        either "readonly" OR "writable"
        either "removable" OR "fixed"
        either "nosuid" or "suid"
	either "nodev" or "dev"

examples:
	hfs.util -p disk0s2 removable writable
	hfs.util -p disk0s2 removable readonly
	hfs.util -m disk0s2 /my/hfs

Input -
    argc - the number of arguments in argv.
    argv - array of arguments.
Output -
    returns FSUR_INVAL if we find a bad argument else 0.
*************************************************************************************************** */
static int
ParseArgs(int argc, const char *argv[], const char ** actionPtr,
	const char ** mountPointPtr, boolean_t * isEjectablePtr,
          boolean_t * isLockedPtr, boolean_t * isSetuidPtr, boolean_t * isDevPtr)
{
    size_t      deviceLength;
    int			result = FSUR_INVAL;
    int			doLengthCheck = 1;
    int			index;
    int 		mounting = 0;

    /* Must have at least 3 arguments and the action argument must start with a '-' */
    if ( (argc < 3) || (argv[1][0] != '-') ) {
        DoDisplayUsage( argv );
        goto Return;
    }

    /* we only support actions Probe, Mount, Force Mount, and Unmount */

    * actionPtr = & argv[1][1];

    switch ( argv[1][1] ) {
        case FSUC_PROBE:
        /* action Probe and requires 5 arguments (need the flags) */
	     if ( argc < 5 ) {
             	DoDisplayUsage( argv );
             	goto Return;
	     } else {
            	index = 3;
	     }
            break;
            
        case FSUC_UNMOUNT:
        	/* Note: the device argument in argv[2] is checked further down but ignored. */
            * mountPointPtr = argv[3];
            index = 0; /* No isEjectable/isLocked flags for unmount. */
            break;
            
        case FSUC_MOUNT:
        case FSUC_MOUNT_FORCE:
            /* action Mount and ForceMount require 8 arguments (need the mountpoint and the flags) */
            if ( argc < 8 ) {
                DoDisplayUsage( argv );
                goto Return;
            } else {
                * mountPointPtr = argv[3];
                index = 4;
                mounting = 1;
            }
            break;
        
        case FSUC_GETUUID:
        	index = 0;
			break;
		
		case FSUC_SETUUID:
			index = 0;
			break;
			
		case FSUC_ADOPT:
			index = 0;
			break;
		
		case FSUC_DISOWN:
			index = 0;
			break;
		
		// XXXdbg
		case FSUC_MKJNL:
			index = 0;
			doLengthCheck = 0;
			if (isdigit(argv[2][0])) {
				char *ptr;
                unsigned long size = strtoul(argv[2], &ptr, 0);

                assert(size < INT_MAX);
				gJournalSize = (int)size;
				if (ptr) {
					gJournalSize *= get_multiplier(*ptr);
				}
				return 0;
			}
			break;

		case FSUC_UNJNL:
			index = 0;
			doLengthCheck = 0;
			break;

		case FSUC_UNJNL_RAW:
			index = 0;
			doLengthCheck = 0;
			break;

		case FSUC_JNLINFS_RAW:
			index = 0;
			doLengthCheck = 0;
			break;

		case FSUC_EXTJNL_RAW:
			index = 0;
			doLengthCheck = 0;
			break;

		case FSUC_JNLINFO:
			index = 0;
			doLengthCheck = 0;
			break;
		// XXXdbg

        default:
            DoDisplayUsage( argv );
            goto Return;
            break;
    }

    /* Make sure device (argv[2]) is something reasonable */
    deviceLength = strlen( argv[2] );
    if ( doLengthCheck && (deviceLength < 3 || deviceLength > NAME_MAX) ) {
        DoDisplayUsage( argv );
        goto Return;
    }

    if ( index ) {
        /* Flags: removable/fixed. */
        if ( 0 == strcmp(argv[index],"removable") ) {
            * isEjectablePtr = 1;
        } else if ( 0 == strcmp(argv[index],"fixed") ) {
            * isEjectablePtr = 0;
        } else {
            printf("hfs.util: ERROR: unrecognized flag (removable/fixed) argv[%d]='%s'\n",index,argv[index]);
        }

        /* Flags: readonly/writable. */
        if ( 0 == strcmp(argv[index+1],"readonly") ) {
            * isLockedPtr = 1;
        } else if ( 0 == strcmp(argv[index+1],"writable") ) {
            * isLockedPtr = 0;
        } else {
            printf("hfs.util: ERROR: unrecognized flag (readonly/writable) argv[%d]='%s'\n",index,argv[index+1]);
        }

        if (mounting) {
                    /* Flags: suid/nosuid. */
                    if ( 0 == strcmp(argv[index+2],"suid") ) {
                        * isSetuidPtr = 1;
                    } else if ( 0 == strcmp(argv[index+2],"nosuid") ) {
                        * isSetuidPtr = 0;
                    } else {
                        printf("hfs.util: ERROR: unrecognized flag (suid/nosuid) argv[%d]='%s'\n",index,argv[index+2]);
                    }

                    /* Flags: dev/nodev. */
                    if ( 0 == strcmp(argv[index+3],"dev") ) {
                        * isDevPtr = 1;
                    } else if ( 0 == strcmp(argv[index+3],"nodev") ) {
                        * isDevPtr = 0;
                    } else {
                        printf("hfs.util: ERROR: unrecognized flag (dev/nodev) argv[%d]='%s'\n",index,argv[index+3]);
                    }
        }


    }

    result = 0;

Return:
        return result;

} /* ParseArgs */


/* *************************************** DoDisplayUsage ********************************************
Purpose -
    This routine will do a printf of the correct usage for this utility.
Input -
    argv - array of arguments.
Output -
    NA.
*************************************************************************************************** */
static void
DoDisplayUsage(const char *argv[])
{
    printf("usage: %s action_arg device_arg [mount_point_arg] [Flags] \n", argv[0]);
    printf("action_arg:\n");
    printf("       -%c (Probe for mounting)\n", FSUC_PROBE);
    printf("       -%c (Mount)\n", FSUC_MOUNT);
    printf("       -%c (Unmount)\n", FSUC_UNMOUNT);
    printf("       -%c (Force Mount)\n", FSUC_MOUNT_FORCE);
#ifdef HFS_UUID_SUPPORT
    printf("       -%c (Get UUID Key)\n", FSUC_GETUUID);
    printf("       -%c (Set UUID Key)\n", FSUC_SETUUID);
#endif //HFS_UUID_SUPPORT
    printf("       -%c (Adopt permissions)\n", FSUC_ADOPT);
	printf("       -%c (Make a file system journaled)\n", FSUC_MKJNL);
	printf("       -%c (Turn off journaling on a file system)\n", FSUC_UNJNL);
	printf("       -%c (Turn off journaling on a raw device)\n", FSUC_UNJNL_RAW);
	printf("       -%c (Disable use of an external journal on a raw device)\n", FSUC_JNLINFS_RAW);
	printf("       -%c (Enable the use of an external journal on a raw device)\n", FSUC_EXTJNL_RAW);
	printf("       -%c (Get size & location of journaling on a file system)\n", FSUC_JNLINFO);
    printf("device_arg:\n");
    printf("       device we are acting upon (for example, 'disk0s2')\n");
    printf("       if '-%c' or '-%c' is specified, this should be the\n", FSUC_MKJNL, FSUC_UNJNL);
	printf("       name of the file system we're to act on (for example, '/Volumes/foo' or '/')\n");
    printf("mount_point_arg:\n");
    printf("       required for Mount and Force Mount \n");
    printf("Flags:\n");
    printf("       required for Mount, Force Mount and Probe\n");
    printf("       indicates removable or fixed (for example 'fixed')\n");
    printf("       indicates readonly or writable (for example 'readonly')\n");
    printf("       indicates suid or nosuid (for example 'suid')\n");
    printf("       indicates dev or nodev (for example 'dev')\n");
    printf("Examples:\n");
    printf("       %s -p disk0s2 fixed writable\n", argv[0]);
    printf("       %s -m disk0s2 /my/hfs removable readonly nosuid nodev\n", argv[0]);

    return;

} /* DoDisplayUsage */


/*
	GetHFSMountPoint
	
	Given a path to a device, determine if a volume is mounted on that
	device.  If there is an HFS volume, return its path and FSUR_IO_SUCCESS.
	If there is a non-HFS volume, return FSUR_UNRECOGNIZED.  If there is
	no volume mounted on the device, set *pathPtr to NULL and return
	FSUR_IO_SUCCESS.

	Returns: FSUR_IO_SUCCESS, FSUR_IO_FAIL, FSUR_UNRECOGNIZED
*/
static int
GetHFSMountPoint(const char *deviceNamePtr, char **pathPtr)
{
	int result;
	int i, numMounts;
	struct statfs *buf;
	
	/* Assume no mounted volume found */	
	*pathPtr = NULL;
	result = FSUR_IO_SUCCESS;	
	
	numMounts = getmntinfo(&buf, MNT_NOWAIT);
	if (numMounts == 0)
		return FSUR_IO_FAIL;
	
	for (i=0; i<numMounts; ++i) {
		if (!strcmp(deviceNamePtr, buf[i].f_mntfromname)) {
			/* Found a mounted volume; check the type */
			if (!strcmp(buf[i].f_fstypename, "hfs")) {
				*pathPtr = buf[i].f_mntonname;
				/* result = FSUR_IO_SUCCESS, above */
			} else {
				result = FSUR_UNRECOGNIZED;
			}
			break;
		}
	}
	
	return result;
}


/*
	ReadHeaderBlock
	
	Read the Master Directory Block or Volume Header Block from an HFS,
	HFS Plus, or HFSX volume into a caller-supplied buffer.  Return the
	offset of an embedded HFS Plus volume (or 0 if not embedded HFS Plus).
	Return a pointer to the volume UUID in the Finder Info.

	Returns: FSUR_IO_SUCCESS, FSUR_IO_FAIL, FSUR_UNRECOGNIZED
*/
static int
ReadHeaderBlock(int fd, void *bufPtr, off_t *startOffset, hfs_UUID_t **finderInfoUUIDPtr)
{
	int result;
	HFSMasterDirectoryBlock * mdbPtr;
	HFSPlusVolumeHeader * volHdrPtr;

	mdbPtr = bufPtr;
	volHdrPtr = bufPtr;

	/*
	 * Read the HFS Master Directory Block or Volume Header from sector 2
	 */
	*startOffset = 0;
	result = (int)readAt(fd, bufPtr, (off_t)(2 * HFS_BLOCK_SIZE), HFS_BLOCK_SIZE);
	if (result != FSUR_IO_SUCCESS)
		goto Err_Exit;

	/*
	 * If this is a wrapped HFS Plus volume, read the Volume Header from
	 * sector 2 of the embedded volume.
	 */
	if (OSSwapBigToHostInt16(mdbPtr->drSigWord) == kHFSSigWord &&
		OSSwapBigToHostInt16(mdbPtr->drEmbedSigWord) == kHFSPlusSigWord) {
		result = GetEmbeddedHFSPlusVol(mdbPtr, startOffset);
		if (result != FSUR_IO_SUCCESS)
			goto Err_Exit;
		result = (int)readAt(fd, bufPtr, *startOffset + (off_t)(2*HFS_BLOCK_SIZE), HFS_BLOCK_SIZE);
		if (result != FSUR_IO_SUCCESS)
			goto Err_Exit;
	}
	
	/*
	 * At this point, we have the MDB for plain HFS, or VHB for HFS Plus and HFSX
	 * volumes (including wrapped HFS Plus).  Verify the signature and grab the
	 * UUID from the Finder Info.
	 */
	if (OSSwapBigToHostInt16(mdbPtr->drSigWord) == kHFSSigWord) {
	    *finderInfoUUIDPtr = (hfs_UUID_t *)(&mdbPtr->drFndrInfo[6]);
	} else if (OSSwapBigToHostInt16(volHdrPtr->signature) == kHFSPlusSigWord ||
				OSSwapBigToHostInt16(volHdrPtr->signature) == kHFSXSigWord) {
	    *finderInfoUUIDPtr = (hfs_UUID_t *)&volHdrPtr->finderInfo[24];
	} else {
		result = FSUR_UNRECOGNIZED;
	}

Err_Exit:
	return result;
}


/*
	GetVolumeUUIDRaw
	
	Read the UUID from an unmounted volume, by doing direct access to the device.
	Assumes the caller has already determined that a volume is not mounted
	on the device.  Once we have the HFS UUID from the finderinfo, convert it to a
	full UUID and then write it into the output argument provided (volUUIDPtr)

	Returns: FSUR_IO_SUCCESS, FSUR_IO_FAIL, FSUR_UNRECOGNIZED
*/
static int
GetVolumeUUIDRaw(const char *deviceNamePtr, const char *rawName, volUUID_t *volUUIDPtr)
{
	int fd = 0;
	char * bufPtr;
	off_t startOffset;
	hfs_UUID_t *finderInfoUUIDPtr;
	hfs_UUID_t hfs_uuid;
	volUUID_t fullUUID;
	int result;
	int error; 

	bufPtr = (char *)malloc(HFS_BLOCK_SIZE);
	if ( ! bufPtr ) {
		result = FSUR_UNRECOGNIZED;
		goto Err_Exit;
	}

	fd = open( deviceNamePtr, O_RDONLY, 0);
	if (fd <= 0) {
		error = errno;
#if TRACE_HFS_UTIL
		fprintf(stderr, "hfs.util: GetVolumeUUIDRaw: device (%s)  open failed (errno = %d).\n", deviceNamePtr, errno);
#endif
		if (error == EBUSY) {
			/* If it was busy, then retry, this time using the raw device */
			fd = open (rawName, O_RDONLY, 0);
			if (fd <= 0) {
#if TRACE_HFS_UTIL
				fprintf(stderr, "hfs.util: GetVolumeUUIDRaw: device (%s) open failed (errno = %d).\n", rawName, errno);
#endif
				result = FSUR_IO_FAIL;
				goto Err_Exit;
			}
		}
		else {
			result = FSUR_IO_FAIL;
			goto Err_Exit;
		}
	}

	/*
	 * Get the pointer to the volume UUID in the Finder Info*/
	result = ReadHeaderBlock(fd, bufPtr, &startOffset, &finderInfoUUIDPtr);
	if (result != FSUR_IO_SUCCESS)
		goto Err_Exit;

	/*
	 * Copy the volume UUID out of the Finder Info. Note that the FinderInfo
	 * stores the UUID in big-endian so we have to convert to native
	 * endianness. 
	 */
	hfs_uuid.high = OSSwapBigToHostInt32(finderInfoUUIDPtr->high);
	hfs_uuid.low = OSSwapBigToHostInt32(finderInfoUUIDPtr->low);

	/* 
	 * Now convert to a full UUID using the same algorithm as HFS+ 
	 * This makes sure to construct a full NULL-UUID if necessary.
	 */
	ConvertHFSUUIDToUUID (&hfs_uuid, &fullUUID);	

	/* Copy it out into the caller's buffer */
	uuid_copy(volUUIDPtr->uuid, fullUUID.uuid); 

Err_Exit:
	if (fd > 0) close(fd);
	if (bufPtr) free(bufPtr);
	
#if TRACE_HFS_UTIL
	if (result != FSUR_IO_SUCCESS) fprintf(stderr, "hfs.util: GetVolumeUUIDRaw: result = %d...\n", result);
#endif
	return (result == FSUR_IO_SUCCESS) ? FSUR_IO_SUCCESS : FSUR_IO_FAIL;
}



void ConvertHFSUUIDStringToUUID(const char *UUIDString, volUUID_t *volumeID) {
    int i;
    char c;
    u_int32_t nextdigit;
    u_int32_t high = 0;
    u_int32_t low = 0;
    u_int32_t carry;
	hfs_UUID_t hfsuu;

    for (i = 0; (i < HFSUUIDLENGTH) && ((c = UUIDString[i]) != (char)0) ; ++i) {
        if ((c >= '0') && (c <= '9')) {
            nextdigit = c - '0';
        } else if ((c >= 'A') && (c <= 'F')) {
            nextdigit = c - 'A' + 10;
        } else if ((c >= 'a') && (c <= 'f')) {
            nextdigit = c - 'a' + 10;
        } else {
            nextdigit = 0;
        }
        carry = ((low & 0xF0000000) >> 28) & 0x0000000F;
        high = (high << 4) | carry;
        low = (low << 4) | nextdigit;
    }

    hfsuu.high = high;
    hfsuu.low = low;

	/* now convert to a full UUID */
	ConvertHFSUUIDToUUID(&hfsuu, volumeID);

	return;
}



/*
 * Convert an HFS+ UUID in binary form to a full UUID 
 * 
 * Assumes that the HFS UUID argument is stored in native endianness
 * If the input UUID is zeroes, then it will emit a NULL'd out UUID.
 */
void ConvertHFSUUIDToUUID (hfs_UUID_t *hfsuuid, volUUID_t *uu)
{
	uint8_t rawUUID[8];

	/* if either high or low is 0, then return the NULL uuid */
	if ((hfsuuid->high == 0) || (hfsuuid->low == 0)) {
		uuid_clear (uu->uuid);
		return;
	}
	/*
	 * If the input UUID was not zeroes, then run it through the normal md5
	 *
	 * NOTE: When using MD5 to compute the "full" UUID, we must pass in the
	 * big-endian values of the two 32-bit fields.  In the kernel, HFS uses the
	 * raw 4-byte fields of the finderinfo directly, without running them through
	 * an endian-swap.  As a result, we must endian-swap back to big endian here.
	 */
	((uint32_t*)rawUUID)[0] = OSSwapHostToBigInt32(hfsuuid->high);
	((uint32_t*)rawUUID)[1] = OSSwapHostToBigInt32(hfsuuid->low);	
	uuid_create_md5_from_name(uu->uuid, kFSUUIDNamespaceSHA1, rawUUID, sizeof(rawUUID));	
}

/*
	SetVolumeUUIDRaw
	
	Write a previously generated UUID to an unmounted volume, by doing direct
	access to the device.  Assumes the caller has already determined that a
	volume is not mounted on the device.

	Returns: FSUR_IO_SUCCESS, FSUR_IO_FAIL, FSUR_UNRECOGNIZED
*/
static int
SetVolumeUUIDRaw(const char *deviceNamePtr, hfs_UUID_t *volumeUUIDPtr)
{
	int fd = 0;
	char * bufPtr;
	off_t startOffset;
	hfs_UUID_t *finderInfoUUIDPtr;
	int result;

	bufPtr = (char *)malloc(HFS_BLOCK_SIZE);
	if ( ! bufPtr ) {
		result = FSUR_UNRECOGNIZED;
		goto Err_Exit;
	}

	fd = open( deviceNamePtr, O_RDWR, 0);
	if (fd <= 0) {
#if TRACE_HFS_UTIL
		fprintf(stderr, "hfs.util: SetVolumeUUIDRaw: device open failed (errno = %d).\n", errno);
#endif
		result = FSUR_IO_FAIL;
		goto Err_Exit;
	}

	/*
	 * Get the pointer to the volume UUID in the Finder Info
	 */
	result = ReadHeaderBlock(fd, bufPtr, &startOffset, &finderInfoUUIDPtr);
	if (result != FSUR_IO_SUCCESS)
		goto Err_Exit;

	/*
	 * Update the UUID in the Finder Info. Make sure to write out big endian.
	 */
	finderInfoUUIDPtr->high = OSSwapHostToBigInt32(volumeUUIDPtr->high);
	finderInfoUUIDPtr->low = OSSwapHostToBigInt32(volumeUUIDPtr->low);

	/*
	 * Write the modified MDB or VHB back to disk
	 */
	result = (int)writeAt(fd, bufPtr, startOffset + (off_t)(2*HFS_BLOCK_SIZE), HFS_BLOCK_SIZE);

Err_Exit:
	if (fd > 0) close(fd);
	if (bufPtr) free(bufPtr);
	
#if TRACE_HFS_UTIL
	if (result != FSUR_IO_SUCCESS) fprintf(stderr, "hfs.util: SetVolumeUUIDRaw: result = %d...\n", result);
#endif
	return (result == FSUR_IO_SUCCESS) ? FSUR_IO_SUCCESS : FSUR_IO_FAIL;
}


/*
	GetVolumeUUIDAttr
	
	Read the UUID from a mounted volume, by calling getattrlist().
	Assumes the path is the mount point of an HFS volume. Note that this will
	return the full-length UUID to the caller, as emitted by the underlying
	filesystem.  On HFS+ this means that we use the hfs_vfsops.c implementation
	to construct the UUID

	Returns: FSUR_IO_SUCCESS, FSUR_IO_FAIL
*/
static int
GetVolumeUUIDAttr(const char *path, volUUID_t *volUUIDPtr) 
{
	struct attrlist alist;
	UUIDAttrBuf_t uuidattr;
	FinderAttrBuf_t finderinfo;
	int result;

	/*
	 * This is a little bit dodgy.  In order to detect whether or not the
	 * volume has a valid UUID, we need to call getattrlist() and examine
	 * the FinderInfo, which is what this function has historically done, even if
	 * we ultimately want the full UUID, which is what is returned if one requests
	 * ATTR_VOL_UUID. 
	 * 
	 * The reason is that if the UUID does not exist, it will be stored
	 * as 8 bytes of zeroes in the UUID portion of the finder info. However, if 
	 * you request ATTR_VOL_UUID, it will run the 8 bytes of zeroes through 
	 * the MD5 function, where they will be manipulated into a full UUID. It 
	 * doesn't look like that guarantees the resulting UUID will also be a 
	 * NULL-uuid (i.e. all zeroes).
	 * 
	 * All of this to say we need to check the finder info first, then check 
	 * ATTR_VOL_UUID as needed afterwards. 
	 */

	/* First set up for a call to getattrlist for the finderinfo */
	memset (&alist, 0, sizeof(alist));
	alist.bitmapcount = ATTR_BIT_MAP_COUNT;
	alist.reserved = 0;
	alist.commonattr = ATTR_CMN_FNDRINFO;
	alist.volattr = ATTR_VOL_INFO;
	alist.dirattr = 0;
	alist.fileattr = 0;
	alist.forkattr = 0;

	/* Get the finderinfo */
	result = getattrlist(path, &alist, &finderinfo, sizeof(finderinfo), 0);
	if (result) {
		return FSUR_IO_FAIL;
	}

	/* Now we need to check if the finderinfo UUID is NULL */
	hfs_UUID_t* hfs_finderinfo = (hfs_UUID_t*)(&finderinfo.finderinfo[6]);
	
	/* 
	 * We should really endian-swap these, but if a uint32_t is 0, 
	 * the endianness doesn't matter 
	 */
	if ((hfs_finderinfo->high == 0) || (hfs_finderinfo->low == 0)) {
		/* Then it is an uninitialized/NULL UUID. Zap the caller buffer and bail out */
		uuid_clear (volUUIDPtr->uuid);
		return FSUR_IO_SUCCESS;
	}	

	/* OK, now set up the attrlist structure to get the volume's UUID */
	memset (&alist, 0, sizeof(alist));
	alist.bitmapcount = ATTR_BIT_MAP_COUNT;
	alist.reserved = 0;
	alist.commonattr = 0;
	alist.volattr = (ATTR_VOL_INFO | ATTR_VOL_UUID);
	alist.dirattr = 0;
	alist.fileattr = 0;
	alist.forkattr = 0;

	/* Get the full UUID from the kernel */
	result = getattrlist(path, &alist, &uuidattr, sizeof(uuidattr), 0);
	if (result) {
		return FSUR_IO_FAIL;
	}

	/* Copy the UUID from the buf to caller's buffer */
	uuid_copy (volUUIDPtr->uuid, uuidattr.uu);
	result = FSUR_IO_SUCCESS;

	return result;
}


/*
	SetVolumeUUIDAttr
	
	Write a UUID to a mounted volume, by calling setattrlist().
	Assumes the path is the mount point of an HFS volume.

	Returns: FSUR_IO_SUCCESS, FSUR_IO_FAIL
*/
static int
SetVolumeUUIDAttr(const char *path, hfs_UUID_t *volumeUUIDPtr)
{
	struct attrlist alist;
	struct FinderAttrBuf volFinderInfo;
	hfs_UUID_t *finderInfoUUIDPtr;
	int result;

	/* Set up the attrlist structure to get the volume's Finder Info */
	memset (&alist, 0, sizeof(alist));
	alist.bitmapcount = ATTR_BIT_MAP_COUNT;
	alist.reserved = 0;
	alist.commonattr = ATTR_CMN_FNDRINFO;
	alist.volattr = ATTR_VOL_INFO;
	alist.dirattr = 0;
	alist.fileattr = 0;
	alist.forkattr = 0;

	/* Get the Finder Info */
	result = getattrlist(path, &alist, &volFinderInfo, sizeof(volFinderInfo), 0);
	if (result) {
		result = FSUR_IO_FAIL;
		goto Err_Exit;
	}

	/* Update the UUID in the Finder Info. Make sure to swap back to big endian */
	finderInfoUUIDPtr = (hfs_UUID_t *)(&volFinderInfo.finderinfo[6]);
	finderInfoUUIDPtr->high = OSSwapHostToBigInt32(volumeUUIDPtr->high);
	finderInfoUUIDPtr->low = OSSwapHostToBigInt32(volumeUUIDPtr->low);

	/* Write the Finder Info back to the volume */
	result = setattrlist(path, &alist, &volFinderInfo.finderinfo, sizeof(volFinderInfo.finderinfo), 0);
	if (result) {
		result = FSUR_IO_FAIL;
		goto Err_Exit;
	}

	result = FSUR_IO_SUCCESS;

Err_Exit:
	return result;
}


/*
	GetVolumeUUID
	
	Return the UUID of an HFS, HFS Plus or HFSX volume.  If there is no UUID and
	we were asked to generate one, then generate a new UUID and write it to the
	volume.
	
	Determine whether an HFS volume is mounted on the given device.  If so, we
	need to use GetVolumeUUIDAttr and SetVolumeUUIDAttr to access the UUID through
	the filesystem.  If there is no mounted volume, then do direct device access
	with GetVolumeUUIDRaw and SetVolumeUUIDRaw.
	
	Returns: FSUR_IO_SUCCESS, FSUR_IO_FAIL, FSUR_UNRECOGNIZED
 */

static int
GetVolumeUUID(const char *deviceNamePtr, const char *rawName, volUUID_t *voluu, boolean_t generate)
{
	int result;
	char *path = NULL;
	
	/*
	 * Determine whether a volume is mounted on this device.  If it is HFS, then
	 * get the mount point's path.  If it is non-HFS, then we can exit immediately
	 * with FSUR_UNRECOGNIZED.
	 */
	result = GetHFSMountPoint(deviceNamePtr, &path);
	if (result != FSUR_IO_SUCCESS) {
		return result;
	}

	/*
	 * Get any existing UUID.
	 */
	if (path) {
		result = GetVolumeUUIDAttr(path, voluu);
	}
	else {
		result = GetVolumeUUIDRaw(deviceNamePtr, rawName, voluu);
	}

	if (result != FSUR_IO_SUCCESS) {
		return result;
	}

	/*
	 * If there was no valid UUID, and we were asked to generate one, then
	 * generate it and write it back to disk.
	 */
	if (generate && (uuid_is_null(voluu->uuid))) {
		hfs_UUID_t hfsuu;

		GenerateHFSVolumeUUID(&hfsuu);
		if (path) {
			result = SetVolumeUUIDAttr(path, &hfsuu);
		}
		else {
			result = SetVolumeUUIDRaw(deviceNamePtr, &hfsuu);
		}
	}
	return result;
}



/*
	SetVolumeUUID
	
	Write a UUID to an HFS, HFS Plus or HFSX volume.
	
	Determine whether an HFS volume is mounted on the given device.  If so, we
	need to use SetVolumeUUIDAttr to access the UUID through the filesystem.
	If there is no mounted volume, then do direct device access SetVolumeUUIDRaw.
	
	Returns: FSUR_IO_SUCCESS, FSUR_IO_FAIL, FSUR_UNRECOGNIZED
 */
static int
SetVolumeUUID(const char *deviceNamePtr, hfs_UUID_t *volumeUUIDPtr) {
	int result;
	char *path = NULL;
	
	/*
	 * Determine whether a volume is mounted on this device.  If it is HFS, then
	 * get the mount point's path.  If it is non-HFS, then we can exit immediately
	 * with FSUR_UNRECOGNIZED.
	 */
	result = GetHFSMountPoint(deviceNamePtr, &path);
	if (result != FSUR_IO_SUCCESS)
		goto Err_Exit;

	/*
	 * Update the UUID.
	 */
	if (path)
		result = SetVolumeUUIDAttr(path, volumeUUIDPtr);
	else
		result = SetVolumeUUIDRaw(deviceNamePtr, volumeUUIDPtr);

Err_Exit:
	return result;
}



/*
 --	GetEmbeddedHFSPlusVol
 --
 --	In: hfsMasterDirectoryBlockPtr
 --	Out: startOffsetPtr - the disk offset at which the HFS+ volume starts
 				(that is, 2 blocks before the volume header)
 --
 */

static int
GetEmbeddedHFSPlusVol (HFSMasterDirectoryBlock * hfsMasterDirectoryBlockPtr, off_t * startOffsetPtr)
{
    int		result = FSUR_IO_SUCCESS;
    u_int32_t	allocationBlockSize, firstAllocationBlock, startBlock, blockCount;

    if (OSSwapBigToHostInt16(hfsMasterDirectoryBlockPtr->drSigWord) != kHFSSigWord) {
        result = FSUR_UNRECOGNIZED;
        goto Return;
    }

    allocationBlockSize = OSSwapBigToHostInt32(hfsMasterDirectoryBlockPtr->drAlBlkSiz);
    firstAllocationBlock = OSSwapBigToHostInt16(hfsMasterDirectoryBlockPtr->drAlBlSt);

    if (OSSwapBigToHostInt16(hfsMasterDirectoryBlockPtr->drEmbedSigWord) != kHFSPlusSigWord) {
        result = FSUR_UNRECOGNIZED;
        goto Return;
    }

    startBlock = OSSwapBigToHostInt16(hfsMasterDirectoryBlockPtr->drEmbedExtent.startBlock);
    blockCount = OSSwapBigToHostInt16(hfsMasterDirectoryBlockPtr->drEmbedExtent.blockCount);

    if ( startOffsetPtr )
        *startOffsetPtr = ((u_int64_t)startBlock * (u_int64_t)allocationBlockSize) +
        	((u_int64_t)firstAllocationBlock * (u_int64_t)HFS_BLOCK_SIZE);

Return:
        return result;

}



/*
 --	GetNameFromHFSPlusVolumeStartingAt
 --
 --	Caller's responsibility to allocate and release memory for the converted string.
 --
 --	Returns: FSUR_IO_SUCCESS, FSUR_IO_FAIL
 */

static int
GetNameFromHFSPlusVolumeStartingAt(int fd, off_t hfsPlusVolumeOffset, unsigned char * name_o)
{
    int					result = FSUR_IO_SUCCESS;
    u_int32_t				blockSize;
    char			*	bufPtr = NULL;
    HFSPlusVolumeHeader		*	volHdrPtr;
    BTNodeDescriptor		*	bTreeNodeDescriptorPtr;
    u_int32_t				catalogNodeSize;
	u_int32_t leafNode;
	u_int32_t catalogExtCount;
	HFSPlusExtentDescriptor *catalogExtents = NULL;

    volHdrPtr = (HFSPlusVolumeHeader *)malloc(HFS_BLOCK_SIZE);
    if ( ! volHdrPtr ) {
        result = FSUR_IO_FAIL;
        goto Return;
    }

    /*
     * Read the Volume Header
     * (This is a little redundant for a pure, unwrapped HFS+ volume)
     */
    result = (int)readAt( fd, volHdrPtr, hfsPlusVolumeOffset + (off_t)(2*HFS_BLOCK_SIZE), HFS_BLOCK_SIZE );
    if (result == FSUR_IO_FAIL) {
#if TRACE_HFS_UTIL
        fprintf(stderr, "hfs.util: GetNameFromHFSPlusVolumeStartingAt: readAt failed\n");
#endif
        goto Return; // return FSUR_IO_FAIL
    }

    /* Verify that it is an HFS+ volume. */

    if (OSSwapBigToHostInt16(volHdrPtr->signature) != kHFSPlusSigWord &&
        OSSwapBigToHostInt16(volHdrPtr->signature) != kHFSXSigWord) {
        result = FSUR_IO_FAIL;
#if TRACE_HFS_UTIL
        fprintf(stderr, "hfs.util: GetNameFromHFSPlusVolumeStartingAt: volHdrPtr->signature != kHFSPlusSigWord\n");
#endif
        goto Return;
    }

    blockSize = OSSwapBigToHostInt32(volHdrPtr->blockSize);
    catalogExtents = (HFSPlusExtentDescriptor *) malloc(sizeof(HFSPlusExtentRecord));
    if ( ! catalogExtents ) {
        result = FSUR_IO_FAIL;
        goto Return;
    }
	bcopy(volHdrPtr->catalogFile.extents, catalogExtents, sizeof(HFSPlusExtentRecord));
	catalogExtCount = kHFSPlusExtentDensity;

	/* if there are overflow catalog extents, then go get them */
	if (OSSwapBigToHostInt32(catalogExtents[7].blockCount) != 0) {
		result = GetCatalogOverflowExtents(fd, hfsPlusVolumeOffset, volHdrPtr, &catalogExtents, &catalogExtCount);
		if (result != FSUR_IO_SUCCESS)
			goto Return;
	}

	/* Read the header node of the catalog B-Tree */

	result = GetBTreeNodeInfo(fd, hfsPlusVolumeOffset, blockSize,
							catalogExtCount, catalogExtents,
							&catalogNodeSize, &leafNode);
	if (result != FSUR_IO_SUCCESS)
        goto Return;

	/* Read the first leaf node of the catalog b-tree */

    bufPtr = (char *)malloc(catalogNodeSize);
    if ( ! bufPtr ) {
        result = FSUR_IO_FAIL;
        goto Return;
    }

    bTreeNodeDescriptorPtr = (BTNodeDescriptor *)bufPtr;

	result = ReadFile(fd, bufPtr, (off_t) leafNode * (off_t) catalogNodeSize, catalogNodeSize,
						hfsPlusVolumeOffset, blockSize,
						catalogExtCount, catalogExtents);
    if (result == FSUR_IO_FAIL) {
#if TRACE_HFS_UTIL
        fprintf(stderr, "hfs.util: ERROR: reading first leaf failed\n");
#endif
        goto Return; // return FSUR_IO_FAIL
    }

    {
        u_int16_t			*	v;
        char			*	p;
        HFSPlusCatalogKey	*	k;
	CFStringRef cfstr;

        if ( OSSwapBigToHostInt16(bTreeNodeDescriptorPtr->numRecords) < 1) {
            result = FSUR_IO_FAIL;
#if TRACE_HFS_UTIL
			fprintf(stderr, "hfs.util: ERROR: bTreeNodeDescriptorPtr->numRecords < 1\n");
#endif
            goto Return;
        }

	// Get the offset (in bytes) of the first record from the list of offsets at the end of the node.

        p = bufPtr + catalogNodeSize - sizeof(u_int16_t); // pointer arithmetic in bytes
        v = (u_int16_t *)p;

	// Get a pointer to the first record.

        p = bufPtr + OSSwapBigToHostInt16(*v); // pointer arithmetic in bytes
        k = (HFSPlusCatalogKey *)p;

	// There should be only one record whose parent is the root parent.  It should be the first record.

        if (OSSwapBigToHostInt32(k->parentID) != kHFSRootParentID) {
            result = FSUR_IO_FAIL;
#if TRACE_HFS_UTIL
            fprintf(stderr, "hfs.util: ERROR: k->parentID != kHFSRootParentID\n");
#endif
            goto Return;
        }

	if ((OSSwapBigToHostInt16(k->nodeName.length) >
		(sizeof(k->nodeName.unicode) / sizeof(k->nodeName.unicode[0]))) ||
		OSSwapBigToHostInt16(k->nodeName.length) > 255) {
		result = FSUR_IO_FAIL;
#if TRACE_HFS_UTIL
		fprintf(stderr, "hfs.util: ERROR:  k->nodeName.length is a bad size (%d)\n", OSSwapBigToHostInt16(k->nodeName.length));
#endif
		goto Return;
	}

	/* Extract the name of the root directory */

	{
	    HFSUniStr255 *swapped;
	    int i;
    
	    swapped = (HFSUniStr255 *)malloc(sizeof(HFSUniStr255));
	    if (swapped == NULL) {
		result = FSUR_IO_FAIL;
		goto Return;
	    }
	    swapped->length = OSSwapBigToHostInt16(k->nodeName.length);
	    
	    for (i=0; i<swapped->length; i++) {
		swapped->unicode[i] = OSSwapBigToHostInt16(k->nodeName.unicode[i]);
	    }
	    swapped->unicode[i] = 0;
	    cfstr = CFStringCreateWithCharacters(kCFAllocatorDefault, swapped->unicode, swapped->length);
	    (void) CFStringGetCString(cfstr, (char *)name_o, NAME_MAX * 3 + 1, kCFStringEncodingUTF8);
	    CFRelease(cfstr);
	    free(swapped);
	}
    }

    result = FSUR_IO_SUCCESS;

Return:
	if (volHdrPtr)
		free((char*) volHdrPtr);

	if (catalogExtents)
		free((char*) catalogExtents);
		
	if (bufPtr)
		free((char*)bufPtr);

    return result;

} /* GetNameFromHFSPlusVolumeStartingAt */


typedef struct {
	BTNodeDescriptor	node;
	BTHeaderRec		header;
} __attribute__((aligned(2), packed)) HeaderRec, *HeaderPtr;

/*
 --	
 --
 --	Returns: FSUR_IO_SUCCESS, FSUR_IO_FAIL
 --
 */
static int
GetBTreeNodeInfo(int fd, off_t hfsPlusVolumeOffset, u_int32_t blockSize,
				u_int32_t extentCount, const HFSPlusExtentDescriptor *extentList,
				u_int32_t *nodeSize, u_int32_t *firstLeafNode)
{
	int result;
	HeaderRec * bTreeHeaderPtr = NULL;

	bTreeHeaderPtr = (HeaderRec *) malloc(HFS_BLOCK_SIZE);
	if (bTreeHeaderPtr == NULL)
		return (FSUR_IO_FAIL);
    
	/* Read the b-tree header node */

	result = ReadFile(fd, bTreeHeaderPtr, 0, HFS_BLOCK_SIZE,
					hfsPlusVolumeOffset, blockSize,
					extentCount, extentList);
	if ( result == FSUR_IO_FAIL ) {
#if TRACE_HFS_UTIL
		fprintf(stderr, "hfs.util: ERROR: reading header node failed\n");
#endif
		goto free;
	}

	if ( bTreeHeaderPtr->node.kind != kBTHeaderNode ) {
		result = FSUR_IO_FAIL;
#if TRACE_HFS_UTIL
		fprintf(stderr, "hfs.util: ERROR: bTreeHeaderPtr->node.kind != kBTHeaderNode\n");
#endif
		goto free;
	}

	*nodeSize = OSSwapBigToHostInt16(bTreeHeaderPtr->header.nodeSize);

	if (OSSwapBigToHostInt32(bTreeHeaderPtr->header.leafRecords) == 0)
		*firstLeafNode = 0;
	else
		*firstLeafNode = OSSwapBigToHostInt32(bTreeHeaderPtr->header.firstLeafNode);

free:;
	free((char*) bTreeHeaderPtr);

	return result;

} /* GetBTreeNodeInfo */


/*
 --
 --
 --	Returns: FSUR_IO_SUCCESS, FSUR_IO_FAIL
 --
 */
static int
GetCatalogOverflowExtents(int fd, off_t hfsPlusVolumeOffset,
		HFSPlusVolumeHeader *volHdrPtr,
		HFSPlusExtentDescriptor **catalogExtents,
		u_int32_t *catalogExtCount)
{
	off_t offset;
	u_int32_t numRecords;
	u_int32_t nodeSize;
	u_int32_t leafNode;
	u_int32_t blockSize;
    BTNodeDescriptor * bTreeNodeDescriptorPtr;
	HFSPlusExtentDescriptor * extents;
	size_t listsize;
    char *	bufPtr = NULL;
	uint32_t i;
	int result;

	blockSize = OSSwapBigToHostInt32(volHdrPtr->blockSize);
	listsize = *catalogExtCount * sizeof(HFSPlusExtentDescriptor);
	extents = *catalogExtents;
	offset = (off_t)OSSwapBigToHostInt32(volHdrPtr->extentsFile.extents[0].startBlock) *
		    (off_t)blockSize;

	/* Read the header node of the extents B-Tree */

	result = GetBTreeNodeInfo(fd, hfsPlusVolumeOffset, blockSize,
			kHFSPlusExtentDensity, volHdrPtr->extentsFile.extents,
		    &nodeSize, &leafNode);
	if (result != FSUR_IO_SUCCESS || leafNode == 0)
		goto Return;

	/* Calculate the logical position of the first leaf node */

	offset = (off_t) leafNode * (off_t) nodeSize;

	/* Read the first leaf node of the extents b-tree */

    bufPtr = (char *)malloc(nodeSize);
	if (! bufPtr) {
		result = FSUR_IO_FAIL;
		goto Return;
	}

	bTreeNodeDescriptorPtr = (BTNodeDescriptor *)bufPtr;

again:
	result = ReadFile(fd, bufPtr, offset, nodeSize,
					hfsPlusVolumeOffset, blockSize,
					kHFSPlusExtentDensity, volHdrPtr->extentsFile.extents);
	if ( result == FSUR_IO_FAIL ) {
#if TRACE_HFS_UTIL
		fprintf(stderr, "hfs.util: ERROR: reading first leaf failed\n");
#endif
		goto Return;
	}

	if (bTreeNodeDescriptorPtr->kind != kBTLeafNode) {
		result = FSUR_IO_FAIL;
		goto Return;
	}

	numRecords = OSSwapBigToHostInt16(bTreeNodeDescriptorPtr->numRecords);
	for (i = 1; i <= numRecords; ++i) {
		u_int16_t * v;
		char * p;
		HFSPlusExtentKey * k;

		/*
		 * Get the offset (in bytes) of the record from the
		 * list of offsets at the end of the node
		 */
		p = bufPtr + nodeSize - (sizeof(u_int16_t) * i);
		v = (u_int16_t *)p;

		/* Get a pointer to the record */

		p = bufPtr + OSSwapBigToHostInt16(*v); /* pointer arithmetic in bytes */
		k = (HFSPlusExtentKey *)p;

		if (OSSwapBigToHostInt32(k->fileID) != kHFSCatalogFileID)
			goto Return;

		/* grow list and copy additional extents */
		listsize += sizeof(HFSPlusExtentRecord);
		extents = (HFSPlusExtentDescriptor *) realloc(extents, listsize);
		bcopy(p + OSSwapBigToHostInt16(k->keyLength) + sizeof(u_int16_t),
			&extents[*catalogExtCount], sizeof(HFSPlusExtentRecord));

		*catalogExtCount += kHFSPlusExtentDensity;
		*catalogExtents = extents;
	}
	
	if ((leafNode = OSSwapBigToHostInt32(bTreeNodeDescriptorPtr->fLink)) != 0) {
	
		offset = (off_t) leafNode * (off_t) nodeSize;
		
		goto again;
	}

Return:;
	if (bufPtr)
		free(bufPtr);

	return (result);
}



/*
 *	LogicalToPhysical - Map a logical file position and size to volume-relative physical
 *	position and number of contiguous bytes at that position.
 *
 *	Inputs:
 *		logicalOffset	Logical offset in bytes from start of file
 *		length			Maximum number of bytes to map
 *		blockSize		Number of bytes per allocation block
 *		extentCount		Number of extents in file
 *		extentList		The file's extents
 *
 *	Outputs:
 *		physicalOffset	Physical offset in bytes from start of volume
 *		availableBytes	Number of bytes physically contiguous (up to length)
 *
 *	Returns: FSUR_IO_SUCCESS, FSUR_IO_FAIL
 */
static int	LogicalToPhysical(off_t offset, ssize_t length, u_int32_t blockSize,
							u_int32_t extentCount, const HFSPlusExtentDescriptor *extentList,
							off_t *physicalOffset, ssize_t *availableBytes)
{
	off_t		temp;
	u_int32_t	logicalBlock;
	u_int32_t	extent;
	u_int32_t	blockCount = 0;
	
	/* Determine allocation block containing logicalOffset */
	logicalBlock = (u_int32_t)(offset / blockSize);	/* This can't overflow for valid volumes */
	offset %= blockSize;	/* Offset from start of allocation block */
	
	/* Find the extent containing logicalBlock */
	for (extent = 0; extent < extentCount; ++extent)
	{
		blockCount = OSSwapBigToHostInt32(extentList[extent].blockCount);
		
		if (blockCount == 0)
			return FSUR_IO_FAIL;	/* Tried to map past physical end of file */
		
		if (logicalBlock < blockCount)
			break;				/* Found it! */
		
		logicalBlock -= blockCount;
	}

	if (extent >= extentCount)
		return FSUR_IO_FAIL;		/* Tried to map past physical end of file */

	/*
	 *	When we get here, extentList[extent] is the extent containing logicalOffset.
	 *	The desired allocation block is logicalBlock blocks into the extent.
	 */
	
	/* Compute the physical starting position */
	temp = OSSwapBigToHostInt32(extentList[extent].startBlock) + logicalBlock;	/* First physical block */
	temp *= blockSize;	/* Byte offset of first physical block */
	*physicalOffset = temp + offset;

	/* Compute the available contiguous bytes. */
	temp = blockCount - logicalBlock;	/* Number of blocks available in extent */
	temp *= blockSize;
	temp -= offset;						/* Number of bytes available */
	
	if (temp < length)
		*availableBytes = temp;
	else
		*availableBytes = length;
	
	return FSUR_IO_SUCCESS;
}



/*
 *	ReadFile - Read bytes from a file.  Handles cases where the starting and/or
 *	ending position are not allocation or device block aligned.
 *
 *	Inputs:
 *		fd			Descriptor for reading the volume
 *		buffer		The bytes are read into here
 *		offset		Offset in file to start reading
 *		length		Number of bytes to read
 *		volOffset	Byte offset from start of device to start of volume
 *		blockSize	Number of bytes per allocation block
 *		extentCount	Number of extents in file
 *		extentList	The file's exents
 *
 *	Returns: FSUR_IO_SUCCESS, FSUR_IO_FAIL
 */
static int	ReadFile(int fd, void *buffer, off_t offset, ssize_t length,
					off_t volOffset, u_int32_t blockSize,
					u_int32_t extentCount, const HFSPlusExtentDescriptor *extentList)
{
	int		result = FSUR_IO_SUCCESS;
	off_t	physOffset;
	ssize_t	physLength;
	
	while (length > 0)
	{
		result = LogicalToPhysical(offset, length, blockSize, extentCount, extentList,
									&physOffset, &physLength);
		if (result != FSUR_IO_SUCCESS)
			break;
		
		result = (int)readAt(fd, buffer, volOffset+physOffset, physLength);
		if (result != FSUR_IO_SUCCESS)
			break;
		
		length -= physLength;
		offset += physLength;
		buffer = (char *) buffer + physLength;
	}
	
	return result;
}

/*
 --	readAt = lseek() + read()
 --
 --	Returns: FSUR_IO_SUCCESS, FSUR_IO_FAIL
 --
 */

static ssize_t
readAt( int fd, void * bufPtr, off_t offset, ssize_t length )
{
    int			blocksize;
    off_t		lseekResult;
    ssize_t		readResult;
    void *		rawData = NULL;
    off_t		rawOffset;
    ssize_t		rawLength;
    ssize_t		dataOffset = 0;
    int			result = FSUR_IO_SUCCESS;

    if (ioctl(fd, DKIOCGETBLOCKSIZE, &blocksize) < 0) {
#if TRACE_HFS_UTIL
    	fprintf(stderr, "hfs.util: readAt: couldn't determine block size of device.\n");
#endif
		result = FSUR_IO_FAIL;
		goto Return;
    }
    /* put offset and length in terms of device blocksize */
    rawOffset = offset / blocksize * blocksize;
    dataOffset = offset - rawOffset;
    rawLength = ((length + dataOffset + blocksize - 1) / blocksize) * blocksize;
    rawData = malloc(rawLength);
    if (rawData == NULL) {
		result = FSUR_IO_FAIL;
		goto Return;
    }

    lseekResult = lseek( fd, rawOffset, SEEK_SET );
    if ( lseekResult != rawOffset ) {
        result = FSUR_IO_FAIL;
        goto Return;
    }

    readResult = read(fd, rawData, rawLength);
    if ( readResult != rawLength ) {
#if TRACE_HFS_UTIL
    		fprintf(stderr, "hfs.util: readAt: attempt to read data from device failed (errno = %d)?\n", errno);
#endif
        result = FSUR_IO_FAIL;
        goto Return;
    }
    bcopy(rawData + dataOffset, bufPtr, length);

Return:
    if (rawData) {
        free(rawData);
    }
    return result;

} /* readAt */

/*
 --	writeAt = lseek() + write()
 --
 --	Returns: FSUR_IO_SUCCESS, FSUR_IO_FAIL
 --
 */

static ssize_t
writeAt( int fd, void * bufPtr, off_t offset, ssize_t length )
{
    int			blocksize;
    off_t		deviceoffset;
    ssize_t		bytestransferred;
    void *		rawData = NULL;
    off_t		rawOffset;
    ssize_t		rawLength;
    ssize_t		dataOffset = 0;
    int			result = FSUR_IO_SUCCESS;

    if (ioctl(fd, DKIOCGETBLOCKSIZE, &blocksize) < 0) {
#if TRACE_HFS_UTIL
    	fprintf(stderr, "hfs.util: couldn't determine block size of device.\n");
#endif
		result = FSUR_IO_FAIL;
		goto Return;
    }
    /* put offset and length in terms of device blocksize */
    rawOffset = offset / blocksize * blocksize;
    dataOffset = offset - rawOffset;
    rawLength = ((length + dataOffset + blocksize - 1) / blocksize) * blocksize;
    rawData = malloc(rawLength);
    if (rawData == NULL) {
		result = FSUR_IO_FAIL;
		goto Return;
    }

    deviceoffset = lseek( fd, rawOffset, SEEK_SET );
    if ( deviceoffset != rawOffset ) {
        result = FSUR_IO_FAIL;
        goto Return;
    }

	/* If the write isn't block-aligned, read the existing data before writing the new data: */
	if (((rawOffset % blocksize) != 0) || ((rawLength % blocksize) != 0)) {
		bytestransferred = read(fd, rawData, rawLength);
	    if ( bytestransferred != rawLength ) {
#if TRACE_HFS_UTIL
    		fprintf(stderr, "writeAt: attempt to pre-read data from device failed (errno = %d)\n", errno);
#endif
	        result = FSUR_IO_FAIL;
	        goto Return;
	    }
	}
	
    bcopy(bufPtr, rawData + dataOffset, length);	/* Copy in the new data */
    
    deviceoffset = lseek( fd, rawOffset, SEEK_SET );
    if ( deviceoffset != rawOffset ) {
        result = FSUR_IO_FAIL;
        goto Return;
    }

    bytestransferred = write(fd, rawData, rawLength);
    if ( bytestransferred != rawLength ) {
#if TRACE_HFS_UTIL
    		fprintf(stderr, "writeAt: attempt to write data to device failed?!");
#endif
        result = FSUR_IO_FAIL;
        goto Return;
    }

Return:
    if (rawData) free(rawData);

    return result;

} /* writeAt */


/*
 * Get kernel's encoding bias.
 */
static int
GetEncodingBias()
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

/******************************************************************************
 *
 *  V O L U M E   S T A T U S   D A T A B A S E   R O U T I N E S
 *
 *****************************************************************************/

#define DBHANDLESIGNATURE 0x75917737

/* Flag values for operation options: */
#define DBMARKPOSITION 1

static char gVSDBPath[] = "/var/db/volinfo.database";

#define MAXIOMALLOC 16384

/* Database layout: */

typedef struct VSDBKey {
	char uuid[16];
} VSDBKey_t;

typedef struct VSDBKeyUUID {
	uuid_string_t uuid_string;
} VSDBKeyUUID_t;

struct VSDBRecord {
	char statusFlags[8];
};

/* A VSDB Entry using a uuid_str (36 byte) instead of HFS UUID string (8 byte) */
typedef struct VSDBEntryUUID {
	VSDBKeyUUID_t key;
	char keySeparator;
	char space;
	struct VSDBRecord record;
	char terminator;
} VSDBEntryUUID_t;

/* a VSDB entry using the HFS UUID */
typedef struct VSDBEntryHFS {
	VSDBKey_t key;
	char keySeparator;
	char space;
	struct VSDBRecord record;
	char terminator;
} VSDBEntryHFS_t;

#define DBKEYSEPARATOR ':'
#define DBBLANKSPACE ' '
#define DBRECORDTERMINATOR '\n'

/* In-memory data structures: */

struct VSDBState {
    unsigned long signature;
    int dbfile;
    int dbmode;
    off_t recordPosition;
};

typedef struct VSDBState *VSDBStatePtr;



/* Internal function prototypes: */
static int LockDB(VSDBStatePtr dbstateptr, int lockmode);
static int UnlockDB(VSDBStatePtr dbstateptr);

static int FindVolumeRecordByUUID(VSDBStatePtr dbstateptr, volUUID_t *volumeID, VSDBEntryUUID_t *dbentry, unsigned long options);
static int AddVolumeRecord(VSDBStatePtr dbstateptr, VSDBEntryUUID_t *dbentry);
static int UpdateVolumeRecord(VSDBStatePtr dbstateptr, VSDBEntryUUID_t *dbentry);
static int GetVSDBEntry(VSDBStatePtr dbstateptr, VSDBEntryUUID_t *dbentry);
static int CompareVSDBKeys(VSDBKeyUUID_t *key1, VSDBKeyUUID_t *key2);


static void FormatULong(unsigned long u, char *s);
static void FormatDBKey(volUUID_t *volumeID, VSDBKeyUUID_t *dbkey);
static void FormatDBRecord(unsigned long volumeStatusFlags, struct VSDBRecord *dbrecord);
static void FormatDBEntry(volUUID_t *volumeID, unsigned long volumeStatusFlags, VSDBEntryUUID_t *dbentry);
static unsigned long ConvertHexStringToULong(const char *hs, long maxdigits);



/******************************************************************************
 *
 *  P U B L I S H E D   I N T E R F A C E   R O U T I N E S
 *
 *****************************************************************************/

void GenerateHFSVolumeUUID(hfs_UUID_t *newuuid) {
	SHA_CTX context;
	char randomInputBuffer[26];
	unsigned char digest[20];
	time_t now;
	clock_t uptime;
	int mib[2];
	int sysdata;
	char sysctlstring[128];
	size_t datalen;
	double sysloadavg[3];
	struct vmtotal sysvmtotal;
	hfs_UUID_t hfsuuid;

	memset (&hfsuuid, 0, sizeof(hfsuuid));

	do {
		/* Initialize the SHA-1 context for processing: */
		SHA1_Init(&context);
		
		/* Now process successive bits of "random" input to seed the process: */
		
		/* The current system's uptime: */
		uptime = clock();
		SHA1_Update(&context, &uptime, sizeof(uptime));
		
		/* The kernel's boot time: */
		mib[0] = CTL_KERN;
		mib[1] = KERN_BOOTTIME;
		datalen = sizeof(sysdata);
		sysctl(mib, 2, &sysdata, &datalen, NULL, 0);
		SHA1_Update(&context, &sysdata, datalen);
		
		/* The system's host id: */
		mib[0] = CTL_KERN;
		mib[1] = KERN_HOSTID;
		datalen = sizeof(sysdata);
		sysctl(mib, 2, &sysdata, &datalen, NULL, 0);
		SHA1_Update(&context, &sysdata, datalen);

		/* The system's host name: */
		mib[0] = CTL_KERN;
		mib[1] = KERN_HOSTNAME;
		datalen = sizeof(sysctlstring);
		sysctl(mib, 2, sysctlstring, &datalen, NULL, 0);
		SHA1_Update(&context, sysctlstring, datalen);

		/* The running kernel's OS release string: */
		mib[0] = CTL_KERN;
		mib[1] = KERN_OSRELEASE;
		datalen = sizeof(sysctlstring);
		sysctl(mib, 2, sysctlstring, &datalen, NULL, 0);
		SHA1_Update(&context, sysctlstring, datalen);

		/* The running kernel's version string: */
		mib[0] = CTL_KERN;
		mib[1] = KERN_VERSION;
		datalen = sizeof(sysctlstring);
		sysctl(mib, 2, sysctlstring, &datalen, NULL, 0);
		SHA1_Update(&context, sysctlstring, datalen);

		/* The system's load average: */
		datalen = sizeof(sysloadavg);
		getloadavg(sysloadavg, 3);
		SHA1_Update(&context, &sysloadavg, datalen);

		/* The system's VM statistics: */
		mib[0] = CTL_VM;
		mib[1] = VM_METER;
		datalen = sizeof(sysvmtotal);
		sysctl(mib, 2, &sysvmtotal, &datalen, NULL, 0);
		SHA1_Update(&context, &sysvmtotal, datalen);

		/* The current GMT (26 ASCII characters): */
		time(&now);
		strncpy(randomInputBuffer, asctime(gmtime(&now)), 26);	/* "Mon Mar 27 13:46:26 2000" */
		SHA1_Update(&context, randomInputBuffer, 26);
		
		/* Pad the accumulated input and extract the final digest hash: */
		SHA1_Final(digest, &context);
	
		memcpy(&hfsuuid, digest, sizeof(hfsuuid));
	} while ((hfsuuid.high == 0) || (hfsuuid.low == 0));

	/* now copy out the hfs uuid */
	memcpy (newuuid, &hfsuuid, sizeof (hfsuuid));

	return;
}

int OpenVolumeStatusDB(VolumeStatusDBHandle *DBHandlePtr) {
	VSDBStatePtr dbstateptr;
	
	*DBHandlePtr = NULL;

	dbstateptr = (VSDBStatePtr)malloc(sizeof(*dbstateptr));
	if (dbstateptr == NULL) {
		return ENOMEM;
	}
	
	dbstateptr->dbmode = O_RDWR;
	dbstateptr->dbfile = open(gVSDBPath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (dbstateptr->dbfile == -1) {
		/*
		   The file couldn't be opened for read/write access:
		   try read-only access before giving up altogether.
		 */
		dbstateptr->dbmode = O_RDONLY;
		dbstateptr->dbfile = open(gVSDBPath, O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		if (dbstateptr->dbfile == -1) {
			errno_t local_errno = errno;
			free(dbstateptr);
			return local_errno;
		}
	}
	
	dbstateptr->signature = DBHANDLESIGNATURE;
	*DBHandlePtr = (VolumeStatusDBHandle)dbstateptr;
	
	/* VSDBUtil converts the status DB, so we do it here, too */
	ConvertVolumeStatusDB(*DBHandlePtr);

	return 0;
}

/* Convert the volume status DB from 64-bit (HFS-style) entries into full UUIDs */
int ConvertVolumeStatusDB(VolumeStatusDBHandle DBHandle) {
    VSDBStatePtr dbstateptr = (VSDBStatePtr)DBHandle;
    struct VSDBEntryHFS entry64;
    struct stat dbinfo;
    int result;
    u_int32_t iobuffersize;
    void *iobuffer = NULL;
    int i;

    if (dbstateptr->signature != DBHANDLESIGNATURE) return EINVAL;

    if ((result = LockDB(dbstateptr, LOCK_EX)) != 0) return result;


	/* 
	 * This function locks the database file then tries to read in
	 * the size of a old-style HFS UUID database entry.  If what it finds
	 * is a well-formatted HFS entry, then it will convert the entire database.
	 * If it finds that the file isn't long enough or isn't actually the
	 * format for the 64-bit UUID struct on-disk, then it will bail out and not 
	 * touch it. Otherwise it will read the whole file and convert it
	 *
	 * In practice this means that if the file is empty or if we have already
	 * converted it then do nothing.
	 */
    lseek(dbstateptr->dbfile, 0, SEEK_SET);
    result = (int)read(dbstateptr->dbfile, &entry64, sizeof(entry64));
    if ((result != sizeof(entry64)) ||
        (entry64.keySeparator != DBKEYSEPARATOR) ||
        (entry64.space != DBBLANKSPACE) ||
        (entry64.terminator != DBRECORDTERMINATOR)) {
        result = 0;
        goto ErrExit;
    } else {
        off_t buf_size = dbinfo.st_size;

		/* Read in a giant buffer */
        if ((result = stat(gVSDBPath, &dbinfo)) != 0) goto ErrExit;
        if (buf_size > UINT32_MAX) {
            result = EINVAL;
            goto ErrExit;
        }
        iobuffersize = (u_int32_t)buf_size;
        iobuffer = malloc(iobuffersize);
        if (iobuffer == NULL) {
            result = ENOMEM;
            goto ErrExit;
        };

        lseek(dbstateptr->dbfile, 0, SEEK_SET);
        result = (int)read(dbstateptr->dbfile, iobuffer, iobuffersize);
        if (result != iobuffersize) {
            result = errno;
            goto ErrExit;
        };
        if ((result = ftruncate(dbstateptr->dbfile, 0)) != 0) {
            goto ErrExit;
        };
        for (i = 0; i < iobuffersize / sizeof(entry64); i++) {
            volUUID_t volumeID;
            u_int32_t VolumeStatus;
            struct VSDBEntryUUID dbentry;

			/* 
			 * now iterate through the contents of the 64-bit entries in RAM
			 * and write them on top of the existing database file
			 */
            entry64 = *(((struct VSDBEntryHFS *)iobuffer) + i);
            if ((entry64.keySeparator != DBKEYSEPARATOR) ||
                (entry64.space != DBBLANKSPACE) ||
                (entry64.terminator != DBRECORDTERMINATOR)) {
                continue;
            }

            ConvertHFSUUIDToUUID(entry64.key.uuid, &volumeID);
            VolumeStatus = (u_int32_t)ConvertHexStringToULong(entry64.record.statusFlags, sizeof(entry64.record.statusFlags));

            FormatDBEntry(&volumeID, VolumeStatus, &dbentry);
            if ((result = AddVolumeRecord(dbstateptr, &dbentry)) != sizeof(dbentry)) {
                warnx("couldn't convert volume status database: %s", strerror(result));
                goto ErrExit;
            };
        };

        fsync(dbstateptr->dbfile);

        result = 0;
    };

ErrExit:
    if (iobuffer) free(iobuffer);
    UnlockDB(dbstateptr);
    return result;
}




int GetVolumeStatusDBEntry(VolumeStatusDBHandle DBHandle, volUUID_t *volumeID, unsigned long *VolumeStatus) {
	VSDBStatePtr dbstateptr = (VSDBStatePtr)DBHandle;
	struct VSDBEntryUUID dbentry;
	int result;

	if (dbstateptr->signature != DBHANDLESIGNATURE) return EINVAL;

	if ((result = LockDB(dbstateptr, LOCK_SH)) != 0) return result;
	
	if ((result = FindVolumeRecordByUUID(dbstateptr, volumeID, &dbentry, 0)) != 0) {
		goto ErrExit;
	}
	*VolumeStatus = VOLUME_RECORDED | ConvertHexStringToULong(dbentry.record.statusFlags, sizeof(dbentry.record.statusFlags));
	
	result = 0;

ErrExit:
	UnlockDB(dbstateptr);
	return result;
}



int SetVolumeStatusDBEntry(VolumeStatusDBHandle DBHandle, volUUID_t *volumeID, unsigned long VolumeStatus) {
	VSDBStatePtr dbstateptr = (VSDBStatePtr)DBHandle;
	struct VSDBEntryUUID dbentry;
	int result;
	
	if (dbstateptr->signature != DBHANDLESIGNATURE) return EINVAL;
	if (VolumeStatus & ~VOLUME_VALIDSTATUSBITS) return EINVAL;
	
	if ((result = LockDB(dbstateptr, LOCK_EX)) != 0) return result;
	
	FormatDBEntry(volumeID, VolumeStatus, &dbentry);
	if ((result = FindVolumeRecordByUUID(dbstateptr, volumeID, NULL, DBMARKPOSITION)) == 0) {
#if DEBUG_TRACE
		fprintf(stderr,"AddLocalVolumeUUID: found record in database; updating in place.\n");
#endif
		result = UpdateVolumeRecord(dbstateptr, &dbentry);
	} else if (result == -1) {
#if DEBUG_TRACE
		fprintf(stderr,"AddLocalVolumeUUID: record not found in database; appending at end.\n");
#endif
		result = AddVolumeRecord(dbstateptr, &dbentry);
	} else {
		goto ErrExit;
	}
	
	fsync(dbstateptr->dbfile);
	
	result = 0;

ErrExit:	
	UnlockDB(dbstateptr);
	return result;
}



int DeleteVolumeStatusDBEntry(VolumeStatusDBHandle DBHandle, volUUID_t *volumeID) {
	VSDBStatePtr dbstateptr = (VSDBStatePtr)DBHandle;
	struct stat dbinfo;
	int result;
	unsigned long iobuffersize;
	void *iobuffer = NULL;
	off_t dataoffset;
	unsigned long iotransfersize;
	unsigned long bytestransferred;

	if (dbstateptr->signature != DBHANDLESIGNATURE) return EINVAL;

	if ((result = LockDB(dbstateptr, LOCK_EX)) != 0) return result;
	
	if ((result = FindVolumeRecordByUUID(dbstateptr, volumeID, NULL, DBMARKPOSITION)) != 0) {
#if DEBUG_TRACE
		fprintf(stderr, "DeleteLocalVolumeUUID: No record with matching volume UUID in DB (result = %d).\n", result);
#endif
		if (result == -1) result = 0;	/* Entry wasn't in the database to begin with? */
		goto StdEdit;
	} else {
#if DEBUG_TRACE
		fprintf(stderr, "DeleteLocalVolumeUUID: Found record with matching volume UUID...\n");
#endif
		if ((result = stat(gVSDBPath, &dbinfo)) != 0) goto ErrExit;
		if ((dbinfo.st_size - dbstateptr->recordPosition - sizeof(struct VSDBEntryUUID)) <= MAXIOMALLOC) {
			iobuffersize = dbinfo.st_size - dbstateptr->recordPosition - sizeof(struct VSDBEntryUUID);
		} else {
			iobuffersize = MAXIOMALLOC;
		}
#if DEBUG_TRACE
		fprintf(stderr, "DeleteLocalVolumeUUID: DB size = 0x%08lx; recordPosition = 0x%08lx;\n", 
							(unsigned long)dbinfo.st_size, (unsigned long)dbstateptr->recordPosition);
		fprintf(stderr, "DeleteLocalVolumeUUID: I/O buffer size = 0x%lx\n", iobuffersize);
#endif
		if (iobuffersize > 0) {
			iobuffer = malloc(iobuffersize);
			if (iobuffer == NULL) {
				result = ENOMEM;
				goto ErrExit;
			}
			
			dataoffset = dbstateptr->recordPosition + sizeof(struct VSDBEntryUUID);
			do {
				iotransfersize = dbinfo.st_size - dataoffset;
				if (iotransfersize > 0) {
					if (iotransfersize > iobuffersize) iotransfersize = iobuffersize;
	
	#if DEBUG_TRACE
					fprintf(stderr, "DeleteLocalVolumeUUID: reading 0x%08lx bytes starting at 0x%08lx ...\n", iotransfersize, (unsigned long)dataoffset);
	#endif
					lseek(dbstateptr->dbfile, dataoffset, SEEK_SET);
					bytestransferred = read(dbstateptr->dbfile, iobuffer, iotransfersize);
					if (bytestransferred != iotransfersize) {
						result = errno;
						goto ErrExit;
					}
	
	#if DEBUG_TRACE
					fprintf(stderr, "DeleteLocalVolumeUUID: writing 0x%08lx bytes starting at 0x%08lx ...\n", iotransfersize, (unsigned long)(dataoffset - (off_t)sizeof(struct VSDBEntryUUID)));
	#endif
					lseek(dbstateptr->dbfile, dataoffset - (off_t)sizeof(struct VSDBEntryUUID), SEEK_SET);
					bytestransferred = write(dbstateptr->dbfile, iobuffer, iotransfersize);
					if (bytestransferred != iotransfersize) {
						result = errno;
						goto ErrExit;
					}
					
					dataoffset += (off_t)iotransfersize;
				}
			} while (iotransfersize > 0);
		}
#if DEBUG_TRACE
		fprintf(stderr, "DeleteLocalVolumeUUID: truncating database file to 0x%08lx bytes.\n", (unsigned long)(dbinfo.st_size - (off_t)(sizeof(struct VSDBEntryUUID))));
#endif
		if ((result = ftruncate(dbstateptr->dbfile, dbinfo.st_size - (off_t)(sizeof(struct VSDBEntryUUID)))) != 0) {
			goto ErrExit;
		}
		
		fsync(dbstateptr->dbfile);
		
		result = 0;
	}

ErrExit:
	if (iobuffer) free(iobuffer);
	UnlockDB(dbstateptr);
	
StdEdit:
	return result;
}



int CloseVolumeStatusDB(VolumeStatusDBHandle DBHandle) {
	VSDBStatePtr dbstateptr = (VSDBStatePtr)DBHandle;

	if (dbstateptr->signature != DBHANDLESIGNATURE) return EINVAL;

	dbstateptr->signature = 0;
	
	close(dbstateptr->dbfile);		/* Nothing we can do about any errors... */
	dbstateptr->dbfile = 0;
	
	free(dbstateptr);
	
	return 0;
}



/******************************************************************************
 *
 *  I N T E R N A L   O N L Y   D A T A B A S E   R O U T I N E S
 *
 *****************************************************************************/

static int LockDB(VSDBStatePtr dbstateptr, int lockmode) {
#if DEBUG_TRACE
	fprintf(stderr, "LockDB: Locking VSDB file...\n");
#endif
	return flock(dbstateptr->dbfile, lockmode);
}

	

static int UnlockDB(VSDBStatePtr dbstateptr) {
#if DEBUG_TRACE
	fprintf(stderr, "UnlockDB: Unlocking VSDB file...\n");
#endif
	return flock(dbstateptr->dbfile, LOCK_UN);
}



static int FindVolumeRecordByUUID(VSDBStatePtr dbstateptr, volUUID_t *volumeID, 
		VSDBEntryUUID_t *targetEntry, unsigned long options) {
	VSDBKeyUUID_t searchkey;
	struct VSDBEntryUUID dbentry;
	int result;
	
	FormatDBKey(volumeID, &searchkey);
	lseek(dbstateptr->dbfile, 0, SEEK_SET);
	
	do {
		result = GetVSDBEntry(dbstateptr, &dbentry);
		if ((result == 0) && (CompareVSDBKeys(&dbentry.key, &searchkey) == 0)) {
			if (targetEntry != NULL) {
#if DEBUG_TRACE
				fprintf(stderr, "FindVolumeRecordByUUID: copying %d. bytes from %08xl to %08l...\n", sizeof(*targetEntry), &dbentry, targetEntry);
#endif
				memcpy(targetEntry, &dbentry, sizeof(*targetEntry));
			}
			return 0;
		}
	} while (result == 0);
	
	return -1;
}



static int AddVolumeRecord(VSDBStatePtr dbstateptr, VSDBEntryUUID_t *dbentry) {
	lseek(dbstateptr->dbfile, 0, SEEK_END);
	return (int)write(dbstateptr->dbfile, dbentry, sizeof(struct VSDBEntryUUID));
}


static int UpdateVolumeRecord(VSDBStatePtr dbstateptr, VSDBEntryUUID_t *dbentry) {
	lseek(dbstateptr->dbfile, dbstateptr->recordPosition, SEEK_SET);
	return (int)write(dbstateptr->dbfile, dbentry, sizeof(*dbentry));
}

static int GetVSDBEntry(VSDBStatePtr dbstateptr, VSDBEntryUUID_t *dbentry) {
	struct VSDBEntryUUID entry;
	int result;
	
	dbstateptr->recordPosition = lseek(dbstateptr->dbfile, 0, SEEK_CUR);
	result = (int)read(dbstateptr->dbfile, &entry, sizeof(entry));
	if ((result != sizeof(entry)) ||
		(entry.keySeparator != DBKEYSEPARATOR) ||
		(entry.space != DBBLANKSPACE) ||
		(entry.terminator != DBRECORDTERMINATOR)) {
		return -1;
	}
	
	memcpy(dbentry, &entry, sizeof(*dbentry));
	return 0;
}



static int CompareVSDBKeys(VSDBKeyUUID_t *key1, VSDBKeyUUID_t *key2) {
	
	return strcmp(key1->uuid_string, key2->uuid_string);
}



/******************************************************************************
 *
 *  F O R M A T T I N G   A N D   C O N V E R S I O N   R O U T I N E S
 *
 *****************************************************************************/

static void FormatULong(unsigned long u, char *s) {
	unsigned long d;
	int i;
	char *digitptr = s;

	for (i = 0; i < 8; ++i) {
		d = ((u & 0xF0000000) >> 28) & 0x0000000F;
		if (d < 10) {
			*digitptr++ = (char)(d + '0');
		} else {
			*digitptr++ = (char)(d - 10 + 'A');
		}
		u = u << 4;
	}
}


static void FormatDBKey(volUUID_t *volumeID, VSDBKeyUUID_t *dbkey) {
	uuid_string_t uuid_str;

	uuid_unparse (volumeID->uuid, uuid_str);
	memcpy (dbkey->uuid_string, uuid_str, sizeof (uuid_str));
}



static void FormatDBRecord(unsigned long volumeStatusFlags, struct VSDBRecord *dbrecord) {
	FormatULong(volumeStatusFlags, dbrecord->statusFlags);
}


static void FormatDBEntry(volUUID_t *volumeID, unsigned long volumeStatusFlags, struct VSDBEntryUUID *dbentry) {
	FormatDBKey(volumeID, &dbentry->key);
	dbentry->keySeparator = DBKEYSEPARATOR;
	dbentry->space = DBBLANKSPACE;
	FormatDBRecord(volumeStatusFlags, &dbentry->record);

	dbentry->terminator = DBRECORDTERMINATOR;
}



static unsigned long ConvertHexStringToULong(const char *hs, long maxdigits) {
	int i;
	char c;
	unsigned long nextdigit;
	unsigned long n;
	
	n = 0;
	for (i = 0; (i < 8) && ((c = hs[i]) != (char)0) ; ++i) {
		if ((c >= '0') && (c <= '9')) {
			nextdigit = c - '0';
		} else if ((c >= 'A') && (c <= 'F')) {
			nextdigit = c - 'A' + 10;
		} else if ((c >= 'a') && (c <= 'f')) {
			nextdigit = c - 'a' + 10;
		} else {
			nextdigit = 0;
		}
		n = (n << 4) + nextdigit;
	}
	
	return n;
}
