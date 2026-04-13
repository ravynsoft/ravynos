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
/*
	File:		SControl.c

	Contains:	This file contains the routines which control the scavenging operations.

	Version:	xxx put version here xxx

	Written by:	Bill Bruffey

	Copyright:	й 1985, 1986, 1992-1999 by Apple Computer, Inc., all rights reserved.
*/

#define SHOW_ELAPSED_TIMES  0


#if SHOW_ELAPSED_TIMES
#include <sys/time.h>
#endif

#include "Scavenger.h"
#include "fsck_journal.h"
#include <setjmp.h>
#include <unistd.h>

#ifndef CONFIG_HFS_TRIM
#define CONFIG_HFS_TRIM 1
#endif

#define	DisplayTimeRemaining 0

/* Variable containing diskdev_cmds tag number and date/time when the binary was built.
 * This variable is populated automatically using version.pl by B&I when building the 
 * project.  For development purposes, if the current directory name looks something 
 * like a tag name or a version number is provided to buildit, buildit populates it 
 * correctly.  For all other ways to build the code, like 'make', the tag number will 
 * be left empty and only project name and build date/time will be shown.
 *
 * TODO: Get this building properly within Xcode, without need for the version.pl script!
 */
extern const unsigned char fsck_hfsVersionString[];

int gGUIControl;
extern char lflag;


// Static function prototypes

static void printVerifyStatus( SGlobPtr GPtr );
static Boolean IsBlueBoxSharedDrive ( DrvQElPtr dqPtr );
static int ScavSetUp( SGlobPtr GPtr );
static int ScavTerm( SGlobPtr GPtr );

/* this procedure receives progress calls and will allow canceling of procedures - we are using it here to print out the progress of the current operation for DFA and DiskUtility - ESP 1/10/00 */

int cancelProc(UInt16 progress, UInt16 secondsRemaining, Boolean progressChanged, UInt16 stage, void *context, int passno)
{
    if (progressChanged) {
	int base;
	int pct;
	int scale;
	static int lastPct = -1;
	if (passno < 0) {
	    base = 0;
	    scale = 100;
	} else {
	    base = (passno * 100) / kMaxReScan;	// Multiply by 100 because we're doing ints
	    scale = 100 / kMaxReScan;
	}
	pct = ((progress * scale) / 100) + base;
	if (pct != lastPct && pct != 100) {
	    fsckPrint((fsck_ctx_t)context, fsckProgress, pct);
	    lastPct = pct;
	    draw_progress(pct);
	}
    }
    return 0;
}

static const int kMaxMediumErrors = 25;

/*
 * Determine whether an error is major or minor.  The main critera we chose for
 * this is whether you can continue to use -- reading, creating, and deleting --
 * in a volume with the error present.  This should at some point go into the
 * message structure itself.
 */
static int
isMinorError(int msg, int *counts)
{
	switch (msg) {
	case hfsExtBTCheck:
	case hfsCatBTCheck:
	case hfsCatHierCheck:
	case hfsExtAttrBTCheck:
	case hfsVolBitmapCheck:
	case hfsVolInfoCheck:
	case hfsHardLinkCheck:
	case hfsRebuildExtentBTree:
	case hfsRebuildCatalogBTree:
	case hfsRebuildAttrBTree:
	case hfsCaseSensitive:
	case hfsMultiLinkDirCheck:
	case hfsJournalVolCheck:
	case hfsLiveVerifyCheck:
	case hfsVerifyVolWithWrite:
	case hfsCheckHFS:
	case hfsCheckNoJnl:
	case E_DirVal:
	case E_CName:
	case E_NoFile:
	case E_NoRtThd:
	case E_NoThd:
	case E_NoDir:
	case E_RtDirCnt:
	case E_RtFilCnt:
	case E_DirCnt:
	case E_FilCnt:
	case E_CatDepth:
	case E_NoFThdFlg:
	case E_CatalogFlagsNotZero:
	case E_BadFileName:
	case E_InvalidClumpSize:
	case E_LockedDirName:
	case E_FreeBlocks:
	case E_LeafCnt:
	case E_BadValue:
	case E_InvalidID:
	case E_DiskFull:
	case E_InvalidLinkCount:
	case E_UnlinkedFile:
	case E_InvalidPermissions:
	case E_InvalidUID_Unused:
	case E_IllegalName:
	case E_IncorrectNumThdRcd:
	case E_SymlinkCreate:
	case E_IncorrectAttrCount:
	case E_IncorrectSecurityCount:
	case E_PEOAttr:
	case E_LEOAttr:
	case E_FldCount:
	case E_HsFldCount:
	case E_BadPermPrivDir:
	case E_DirInodeBadFlags:
	case E_DirInodeBadParent:
	case E_DirInodeBadName:
	case E_DirHardLinkChain:
	case E_DirHardLinkOwnerFlags:
	case E_DirHardLinkFinderInfo:
	case E_DirLinkAncestorFlags:
	case E_DirHardLinkNesting:
	case E_InvalidLinkChainPrev:
	case E_InvalidLinkChainNext:
	case E_FileInodeBadFlags:
	case E_FileInodeBadName:
	case E_FileHardLinkChain:
	case E_FileHardLinkFinderInfo:
	case E_InvalidLinkChainFirst:
	case E_FileLinkBadFlags:
	case E_DirLinkBadFlags:
	case E_OrphanFileLink:
	case E_OrphanDirLink:
	case E_OrphanFileInode:
	case E_OrphanDirInode:
	case E_UnusedNodeNotZeroed:
	case E_VBMDamagedOverAlloc:
	case E_BadSymLink:
	case E_BadSymLinkLength:
	case E_BadSymLinkName:
		return 1;
	/*
	 * A lot of EOF errors may indicate that there were some more significant
	 * problems with the volume; just one by itself, with no other volume layout
	 * problems, won't affect the volume usage.  So we keep track of them.
	 */
	case E_PEOF:
	case E_LEOF:
		if (++counts[abs(msg)] > kMaxMediumErrors)
			return 0;
		return 1;
	default:
		return 0;
	}
}

/*------------------------------------------------------------------------------

External
 Routines:		CheckHFS	- Controls the scavenging process.

------------------------------------------------------------------------------*/

static jmp_buf				envBuf;
int
CheckHFS( const char *rdevnode, int fsReadRef, int fsWriteRef, int checkLevel, 
	  int repairLevel, fsck_ctx_t fsckContext, int lostAndFoundMode, 
	  int canWrite, int *modified, int liveMode, int rebuildOptions )
{
	SGlob				dataArea;	// Allocate the scav globals
	short				temp; 	
	FileIdentifierTable	*fileIdentifierTable	= nil;
	OSErr				err = noErr;
	OSErr				scavError = 0;
	int					scanCount = 0;
	int					isJournaled = 0;
	Boolean 			autoRepair;
	Boolean				exitEarly = 0;
	__block int *msgCounts = NULL;
	Boolean				majorErrors = 0;

	if (checkLevel == kMajorCheck) {
		checkLevel = kForceCheck;
		exitEarly = 1;
		msgCounts = malloc(sizeof(int) * E_LastError);
	}

	autoRepair = (fsWriteRef != -1 && repairLevel != kNeverRepair);

	/* Initialize the messages only once before the verify stage */
	if (fsckContext) {
		extern fsck_message_t hfs_messages[];
		extern fsck_message_t hfs_errors[];

		if (fsckAddMessages(fsckContext, hfs_messages) == -1 || 
		    fsckAddMessages(fsckContext, hfs_errors) == -1) {
			// XXX
			return -1;
		}
	}

	/*
	 * Get the project name and version that is being built.
	 *
	 * The __fsck_hfsVersionString contents are of the form:
	 * "@(#)PROGRAM:fsck_hfs  PROJECT:hfs-557~332\n"
	 */
	if (1) {
		const char project[] = "  PROJECT:";
		char *vstr, *tmp;

		tmp = strstr((const char *)fsck_hfsVersionString, project);
		if (tmp) {
		    vstr = strdup(tmp + strlen(project));
		    tmp = strstr(vstr, "\n");
		    if (tmp)
			*tmp = 0;
		} else {
		    vstr = strdup((const char *)fsck_hfsVersionString);
		}

		fsckPrint(fsckContext, fsckInformation, "fsck_hfs", vstr);
		free(vstr);
	}

	if (setjmp(envBuf) == 1) {
		/*
		 * setjmp() returns the second argument to longjmp(), so if it returns 1, then
		 * we've hit a major error.
		 */
		dataArea.RepLevel = repairLevelVeryMinorErrors;
		majorErrors = 1;
		goto EarlyExitLabel;
	} else {
		if (exitEarly && fsckContext) {
			/*
			 * Set the after-printing block to a small bit of code that checks to see if
			 * the message in question corresponds to a major or a minor error.  If it's
			 * major, we longjmp just above, which causes us to exit out early.
			 */
			fsckSetBlock(fsckContext, fsckPhaseAfterMessage, (fsckBlock_t) ^(fsck_ctx_t c, int msgNum, va_list args) {
				if (abs(msgNum) > E_FirstError && abs(msgNum) < E_LastError) {
					if (isMinorError(abs(msgNum), msgCounts) == 1)
						return fsckBlockContinue;
					longjmp(envBuf, 1);
					return fsckBlockAbort;
				} else {
					return fsckBlockContinue;
				}
			});
		}
	}
DoAgain:
	ClearMemory( &dataArea, sizeof(SGlob) );
	if (msgCounts)
		memset(msgCounts, 0, sizeof(int) * E_LastError);

	//	Initialize some scavenger globals
	dataArea.itemsProcessed		= 0;	//	Initialize to 0% complete
	dataArea.itemsToProcess		= 1;
	dataArea.chkLevel			= checkLevel;
	dataArea.repairLevel		= repairLevel;
	dataArea.rebuildOptions		= rebuildOptions;
	dataArea.canWrite			= canWrite;
	dataArea.writeRef			= fsWriteRef;
	dataArea.lostAndFoundMode	= lostAndFoundMode;
	dataArea.DrvNum				= fsReadRef;
	dataArea.liveVerifyState 	= liveMode;
	dataArea.scanCount		= scanCount;
    	if (strlcpy(dataArea.deviceNode, rdevnode, sizeof(dataArea.deviceNode)) != strlen(rdevnode)) {
		dataArea.deviceNode[0] = '\0';
	}
    	
    /* there are cases where we cannot get the name of the volume so we */
    /* set our default name to one blank */
	dataArea.volumeName[ 0 ] = ' ';
	dataArea.volumeName[ 1 ] = '\0';

	if (fsckContext) {
		dataArea.context = fsckContext;
		dataArea.guiControl = true;
		dataArea.userCancelProc = cancelProc;
	}
	//
	//	Initialize the scavenger
	//
	ScavCtrl( &dataArea, scavInitialize, &scavError );
	if ( checkLevel == kNeverCheck || (checkLevel == kDirtyCheck && dataArea.cleanUnmount) ||
						scavError == R_NoMem || scavError == R_BadSig) {
		// also need to bail when allocate fails in ScavSetUp or we bus error!
		goto termScav;
	}

	isJournaled = CheckIfJournaled( &dataArea, false );
	if (isJournaled != 0 && 
	    scanCount == 0 && 
		checkLevel != kForceCheck && 
	    !(checkLevel == kPartialCheck && repairLevel == kForceRepairs)) {
			if (fsckGetOutputStyle(dataArea.context) == fsckOutputTraditional) {
				plog("fsck_hfs: Volume is journaled.  No checking performed.\n");
				plog("fsck_hfs: Use the -f option to force checking.\n");
			}
		scavError = 0;
		goto termScav;
	}
	dataArea.calculatedVCB->vcbDriveNumber = fsReadRef;
	dataArea.calculatedVCB->vcbDriverWriteRef = fsWriteRef;

	// Only show the progress bar if we're doing a real check.
	if (fsckContext) {
		start_progress();
	}

	//
	//	Now verify the volume
	//
	if ( scavError == noErr )
		ScavCtrl( &dataArea, scavVerify, &scavError );
        	
EarlyExitLabel:
	if (scavError == noErr && fsckGetVerbosity(dataArea.context) >= kDebugLog)
		printVerifyStatus(&dataArea);

	// Looped for maximum times for verify and repair.  This was the last verify and
	// we bail out if problems were found
	if (scanCount >= kMaxReScan && (dataArea.RepLevel != repairLevelNoProblemsFound)) {
		fsckPrint(dataArea.context, fsckVolumeNotRepairedTries, dataArea.volumeName, scanCount);
		scavError = R_RFail;
		goto termScav;
	}

	if ( dataArea.RepLevel == repairLevelUnrepairable ) 
		err = cdUnrepairableErr;

	if ( !autoRepair &&
	     (dataArea.RepLevel == repairLevelVolumeRecoverable ||
	      dataArea.RepLevel == repairLevelCatalogBtreeRebuild  ||
	      dataArea.RepLevel == repairLevelVeryMinorErrors) ) {
		fsckPrint(dataArea.context, fsckVolumeCorruptNeedsRepair, dataArea.volumeName);
		scavError = R_VFail;
		goto termScav;
	}

	if ( scavError == noErr && dataArea.RepLevel == repairLevelNoProblemsFound ) {
		if (CONFIG_HFS_TRIM &&
		    (dataArea.canWrite != 0) && (dataArea.writeRef != -1) &&
		    IsTrimSupported())
		{
			fsckPrint(dataArea.context, fsckTrimming);
			TrimFreeBlocks(&dataArea);
		}

		if (scanCount == 0) {
			fsckPrint(dataArea.context, fsckVolumeOK, dataArea.volumeName);
		} else {
			fsckPrint(dataArea.context, fsckRepairSuccessful, dataArea.volumeName);
		}
	}

	//
	//	Repair the volume if it needs repairs, its repairable and we were able to unmount it
	//
	if ( dataArea.RepLevel == repairLevelNoProblemsFound && repairLevel == kForceRepairs )
	{
		if (rebuildOptions & REBUILD_CATALOG) {
			dataArea.CBTStat |= S_RebuildBTree;
		}
		if (rebuildOptions & REBUILD_EXTENTS) {
			dataArea.EBTStat |= S_RebuildBTree;
		}
		if (rebuildOptions & REBUILD_ATTRIBUTE) {
			dataArea.ABTStat |= S_RebuildBTree;
		}
		dataArea.RepLevel = repairLevelCatalogBtreeRebuild;
	}
		
	if ( ((scavError == noErr) || (scavError == errRebuildBtree))  &&
		 (autoRepair == true)	&&
		 (dataArea.RepLevel != repairLevelUnrepairable)	&&
		 (dataArea.RepLevel != repairLevelNoProblemsFound) )
	{	
		// we cannot repair a volume when others have write access to the block device
		// for the volume 

		if ( dataArea.canWrite == 0 ) {
			scavError = R_WrErr;
			fsckPrint(dataArea.context, fsckVolumeNotRepairedInUse, dataArea.volumeName);
		}
		else
			ScavCtrl( &dataArea, scavRepair, &scavError );
		
		if ( scavError == noErr )
		{
			*modified = 1;	/* Report back that we made repairs */

			/* we just repaired a volume, so scan it again to check if it corrected everything properly */
			ScavCtrl( &dataArea, scavTerminate, &temp );
			repairLevel = kMajorRepairs;
			checkLevel = kAlwaysCheck;
			fsckPrint(dataArea.context, fsckRecheckingVolume);
			scanCount++;
			goto DoAgain;
		}
		else {
			fsckPrint(dataArea.context, fsckVolumeNotRepaired, dataArea.volumeName);
		}
	}
	else if ( scavError != noErr ) {
		// Is this correct?
		fsckPrint(dataArea.context, fsckVolumeVerifyIncomplete, dataArea.volumeName);
		if ( fsckGetVerbosity(dataArea.context) >= kDebugLog )
			plog("\tvolume check failed with error %d \n", scavError);
	}

	//	Set up structures for post processing
	if ( (autoRepair == true) && (dataArea.fileIdentifierTable != nil) )
	{
	//	*repairInfo = *repairInfo | kVolumeHadOverlappingExtents;	//	Report back that volume has overlapping extents
		fileIdentifierTable	= (FileIdentifierTable *) AllocateMemory( GetHandleSize( (Handle) dataArea.fileIdentifierTable ) );
		CopyMemory( *(dataArea.fileIdentifierTable), fileIdentifierTable, GetHandleSize( (Handle) dataArea.fileIdentifierTable ) );
	}


	//
	//	Post processing
	//
	if ( fileIdentifierTable != nil )
	{
		DisposeMemory( fileIdentifierTable );
	}

termScav:
	if (gBlkListEntries != 0)
		dumpblocklist(&dataArea);

	if (err == noErr) {
		err = scavError;
	}
		
	//
	//	Terminate the scavenger
	//

	if ( fsckGetVerbosity(dataArea.context) >= kDebugLog && 
		 (err != noErr || dataArea.RepLevel != repairLevelNoProblemsFound) )
		PrintVolumeObject();
	if (err != 0 && embedded == 1) {
		Buf_t *buf = NULL;
		off_t offset = 1024;
		uint32_t len = 512;	// the size of an HFS+ volume header
		int rv = CacheRead(&fscache, offset, len, &buf);
		if (rv == 0) {
			fprintf(stderr, "Offset %llu length %u:\n", offset, len);
			DumpData(buf->Buffer, len, NULL);
			CacheRelease(&fscache, buf, 0);
		} else {
			fprintf(stderr, "%s(%d):  rv = %d\n", __FUNCTION__, __LINE__, rv);
		}
		fflush(stderr);
	}

	// If we have write access on volume and we are allowed to write, 
	// mark the volume clean/dirty
	if ((fsWriteRef != -1) && (dataArea.canWrite != 0)) {
		Boolean update;
		if (scavError) {
			// Mark volume dirty 
			CheckForClean(&dataArea, kMarkVolumeDirty, &update);
		} else {
			// Mark volume clean
			CheckForClean(&dataArea, kMarkVolumeClean, &update);
		}
		if (update) {
			/* Report back that volume was modified */
			*modified = 1; 
		}
	}
	ScavCtrl( &dataArea, scavTerminate, &temp );		//	Note: use a temp var so that real scav error can be returned
		
	if (fsckContext) {
		fsckPrint( fsckContext, fsckProgress, 100);	// End each run with 100% message, if desired
		draw_progress(100);
		end_progress();
	}
	if (exitEarly && majorErrors)
		err = MAJOREXIT;

	if (msgCounts) {
		free(msgCounts);
	}

	return( err );
}


/*------------------------------------------------------------------------------

Function:	ScavCtrl - (Scavenger Control)

Function:	Controls the scavenging process.  Interfaces with the User Interface
			Layer (written in PASCAL).

Input:		ScavOp		-	scavenging operation to be performed:

								scavInitialize	= start initial volume check
								scavVerify	= start verify
								scavRepair	= start repair
								scavTerminate		= finished scavenge 

			GPtr		-	pointer to scavenger global area			


Output:		ScavRes		-	scavenge result code (R_xxx, or 0 if no error)

------------------------------------------------------------------------------*/

void ScavCtrl( SGlobPtr GPtr, UInt32 ScavOp, short *ScavRes )
{
	OSErr			result;
	unsigned int		stat;
#if SHOW_ELAPSED_TIMES
	struct timeval 	myStartTime;
	struct timeval 	myEndTime;
	struct timeval 	myElapsedTime;
	struct timezone zone;
#endif

	//
	//	initialize some stuff
	//
	result			= noErr;						//	assume good status
	*ScavRes		= 0;	
	GPtr->ScavRes	= 0;
	
	//		
	//	dispatch next scavenge operation
	//
	switch ( ScavOp )
	{
		case scavInitialize:								//	INITIAL VOLUME CHECK
		{
			Boolean modified; 
			int clean;

			if ( ( result = ScavSetUp( GPtr ) ) )			//	set up BEFORE CheckForStop
				break;
			if ( IsBlueBoxSharedDrive( GPtr->DrvPtr ) )
				break;
			if ( ( result = CheckForStop( GPtr ) ) )		//	in order to initialize wrCnt
				break;
		
			/* Call for all chkLevel options and check return value only 
			 * for kDirtyCheck for preen option and kNeverCheck for quick option
			 */
			clean = CheckForClean(GPtr, kCheckVolume, &modified);
			if ((GPtr->chkLevel == kDirtyCheck) || (GPtr->chkLevel == kNeverCheck)) {
				if (clean == 1) {
					/* volume was unmounted cleanly */
					GPtr->cleanUnmount = true;
					break;
				}

				if (GPtr->chkLevel == kNeverCheck) {
					if (clean == -1)
						result = R_BadSig;
					else if (clean == 0) {
						/*
					 	 * We lie for journaled file systems since
					     * they get cleaned up in mount by replaying
					 	 * the journal.
					 	 * Note: CheckIfJournaled will return negative
					     * if it finds lastMountedVersion = FSK!.
					     */
						if (CheckIfJournaled(GPtr, false))
							GPtr->cleanUnmount = true;
						else
							result = R_Dirty;
					}
					break;
				}
			}
			
			if (CheckIfJournaled(GPtr, false)
			    && GPtr->chkLevel != kForceCheck
			    && !(GPtr->chkLevel == kPartialCheck && GPtr->repairLevel == kForceRepairs)
				&& !(GPtr->chkLevel == kAlwaysCheck && GPtr->repairLevel == kMajorRepairs)) {
			    break;
			}

			if (GPtr->liveVerifyState) {
				fsckPrint(GPtr->context, hfsLiveVerifyCheck);
			} else if (GPtr->canWrite == 0 && nflag == 0) {
				fsckPrint(GPtr->context, hfsVerifyVolWithWrite);
			}

			/*
			 * In the first pass, if fsck_hfs is verifying a
			 * journaled volume, and it's not a live verification,
			 * check to see if the journal is empty.  If it is not,
			 * flag it as a journal error, and print a message.
			 * (A live verify will almost certainly have a non-empty
			 * journal, but that should be safe in this case due
			 * to the freeze command flushing everything.)
			 */
			if ((GPtr->scanCount == 0) &&
			    (CheckIfJournaled(GPtr, true) == 1) &&
			    (GPtr->canWrite == 0 || GPtr->writeRef == -1) &&
			    (lflag == 0)) {
				fsckJournalInfo_t jnlInfo = { 0 };
				UInt64 numBlocks;
				UInt32 blockSize;
				jnlInfo.jnlfd = -1;

				if (IsJournalEmpty(GPtr, &jnlInfo) == 0) {
					// disable_journal can currently only be set with debug enabled
					if (disable_journal) {
						fsckPrint(GPtr->context, E_DirtyJournal);
						GPtr->JStat |= S_DirtyJournal;
					} else {
						(void)GetDeviceSize(GPtr->calculatedVCB->vcbDriveNumber, &numBlocks, &blockSize);
#if 0
						// For debugging the cache.  WAY to verbose to run with even normal debug
						if (debug) {
							printf("Before journal replay\n");
							dumpCache(&fscache);
						}
#endif
						if (journal_open(jnlInfo.jnlfd,
								 jnlInfo.jnlOffset,
								 jnlInfo.jnlSize,
								 blockSize,
								 0,
								 jnlInfo.name,
								 ^(off_t start, void *data, size_t len) {
									 Buf_t *buf;
									 int rv;
									 rv = CacheRead(&fscache, start, (int)len, &buf);
									 if (rv != 0)
										 abort();
									 memcpy(buf->Buffer, data, len);
									 rv = CacheWrite(&fscache, buf, 0, kLockWrite);
									 if (rv != 0)
										 abort();
									 return 0;}
							    ) == -1) {
							fsckPrint(GPtr->context, E_DirtyJournal);
							GPtr->JStat |= S_DirtyJournal;
						} else if (debug) {
							plog("Journal replay simulation succeeded\n");
#if 0
							// Still way too verbose to run
							dumpCache(&fscache);
#endif
						}
					}
				} else {
					if (debug)
						plog("Journal is empty\n");
				}
				if (jnlInfo.jnlfd != -1)
					close(jnlInfo.jnlfd);
				if (jnlInfo.name != NULL)
					free(jnlInfo.name);
			}

			result = IVChk( GPtr );
			
			break;
		}
	
		case scavVerify:								//	VERIFY
		{

#if SHOW_ELAPSED_TIMES
			gettimeofday( &myStartTime, &zone );
#endif

			/* Initialize volume bitmap structure */
			if ( BitMapCheckBegin(GPtr) != 0)
				break;

#if SHOW_ELAPSED_TIMES
			gettimeofday( &myEndTime, &zone );
			timersub( &myEndTime, &myStartTime, &myElapsedTime );
			plog( "\n%s - BitMapCheck elapsed time \n", __FUNCTION__ );
			plog( "########## secs %d msecs %d \n\n", 
				myElapsedTime.tv_sec, myElapsedTime.tv_usec );
#endif

			if ( IsBlueBoxSharedDrive( GPtr->DrvPtr ) )
				break;
			if ( ( result = CheckForStop( GPtr ) ) )
				break;

#if SHOW_ELAPSED_TIMES
			gettimeofday( &myStartTime, &zone );
#endif

			/* Create calculated BTree structures */
			if ( ( result = CreateExtentsBTreeControlBlock( GPtr ) ) )	
				break;
			if ( ( result = CreateCatalogBTreeControlBlock( GPtr ) ) )
				break;
			if ( ( result = CreateAttributesBTreeControlBlock( GPtr ) ) )
				break;
			if ( ( result = CreateExtendedAllocationsFCB( GPtr ) ) )
				break;

#if SHOW_ELAPSED_TIMES
			gettimeofday( &myEndTime, &zone );
			timersub( &myEndTime, &myStartTime, &myElapsedTime );
			plog( "\n%s - create control blocks elapsed time \n", __FUNCTION__ );
			plog( ">>>>>>>>>>>>> secs %d msecs %d \n\n", 
				myElapsedTime.tv_sec, myElapsedTime.tv_usec );
#endif

			//	Now that preflight of the BTree structures is calculated, compute the CheckDisk items
			CalculateItemCount( GPtr, &GPtr->itemsToProcess, &GPtr->onePercent );
			GPtr->itemsProcessed += GPtr->onePercent;	// We do this 4 times as set up in CalculateItemCount() to smooth the scroll
			
			if ( ( result = VLockedChk( GPtr ) ) )
				break;

			GPtr->itemsProcessed += GPtr->onePercent;	// We do this 4 times as set up in CalculateItemCount() to smooth the scroll
			fsckPrint(GPtr->context, hfsExtBTCheck);

#if SHOW_ELAPSED_TIMES
			gettimeofday( &myStartTime, &zone );
#endif
				
			/* Verify extent btree structure */
			if ((result = ExtBTChk(GPtr)))
				break;

#if SHOW_ELAPSED_TIMES
			gettimeofday( &myEndTime, &zone );
			timersub( &myEndTime, &myStartTime, &myElapsedTime );
			plog( "\n%s - ExtBTChk elapsed time \n", __FUNCTION__ );
			plog( ">>>>>>>>>>>>> secs %d msecs %d \n\n", 
				myElapsedTime.tv_sec, myElapsedTime.tv_usec );
#endif
				
			if ((result = CheckForStop(GPtr)))
				break;
			
			GPtr->itemsProcessed += GPtr->onePercent;	// We do this 4 times as set up in CalculateItemCount() to smooth the scroll

			/* Check extents of bad block file */
			if ((result = BadBlockFileExtentCheck(GPtr)))
				break;
			if ((result = CheckForStop(GPtr)))
				break;
			
			GPtr->itemsProcessed += GPtr->onePercent;	// We do this 4 times as set up in CalculateItemCount() to smooth the scroll
			GPtr->itemsProcessed += GPtr->onePercent;
			fsckPrint(GPtr->context, hfsCatBTCheck);

#if SHOW_ELAPSED_TIMES
			gettimeofday( &myStartTime, &zone );
#endif
				
			if ( GPtr->chkLevel == kPartialCheck )
			{
				/* skip the rest of the verify code path the first time */
				/* through when we are rebuilding the catalog B-Tree file. */
				/* we will be back here after the rebuild.  */
				if (GPtr->rebuildOptions & REBUILD_CATALOG) {
					GPtr->CBTStat |= S_RebuildBTree;
				}
				if (GPtr->rebuildOptions & REBUILD_EXTENTS) {
					GPtr->EBTStat |= S_RebuildBTree;
				}
				if (GPtr->rebuildOptions & REBUILD_ATTRIBUTE) {
					GPtr->ABTStat |= S_RebuildBTree;
				}
				result = errRebuildBtree;
				break;
			}
			
			/* Check catalog btree.  For given fileID, the function accounts
			 * for all extents existing in catalog record as well as in
			 * overflow extent btree
			 */
			if ((result = CheckCatalogBTree(GPtr)))
				break;

#if SHOW_ELAPSED_TIMES
			gettimeofday( &myEndTime, &zone );
			timersub( &myEndTime, &myStartTime, &myElapsedTime );
			plog( "\n%s - CheckCatalogBTree elapsed time \n", __FUNCTION__ );
			plog( ">>>>>>>>>>>>> secs %d msecs %d \n\n", 
				myElapsedTime.tv_sec, myElapsedTime.tv_usec );
#endif

			if ((result = CheckForStop(GPtr)))
				break;

			if (scanflag == 0) {
				fsckPrint(GPtr->context, hfsCatHierCheck);

#if SHOW_ELAPSED_TIMES
				gettimeofday( &myStartTime, &zone );
#endif
				
				/* Check catalog hierarchy */
				if ((result = CatHChk(GPtr)))
					break;

#if SHOW_ELAPSED_TIMES
				gettimeofday( &myEndTime, &zone );
				timersub( &myEndTime, &myStartTime, &myElapsedTime );
				plog( "\n%s - CatHChk elapsed time \n", __FUNCTION__ );
				plog( ">>>>>>>>>>>>> secs %d msecs %d \n\n", 
					myElapsedTime.tv_sec, myElapsedTime.tv_usec );
#endif

				if ((result = CheckForStop(GPtr)))
					break;

				if (VolumeObjectIsHFSX(GPtr)) {
					result = CheckFolderCount(GPtr);
					if (result)
						break;

					if ((result=CheckForStop(GPtr)))
						break;
				}
			}
			/* Check attribute btree.  The function accounts for all extents
			 * for extended attributes whose values are stored in 
			 * allocation blocks
			 */
			if ((result = AttrBTChk(GPtr)))
				break;

			if ((result = CheckForStop(GPtr)))
				break;

			/* 
			 * fsck_hfs has accounted for all valid allocation blocks by 
			 * traversing all catalog records and attribute records.
			 * These traversals may have found overlapping extents.  Note
			 * that the overlapping extents are detected in CaptureBitmapBits 
			 * when it tries to set a bit corresponding to allocation block
			 * and finds that it is already set.  Therefore fsck_hfs does not
			 * know the orignal file involved overlapped extents.
			 */
			if (GPtr->VIStat & S_OverlappingExtents) {
				/* Find original files involved in overlapped extents */
				result = FindOrigOverlapFiles(GPtr);
				if (result) {
					break;
				}
		
				/* Print all unique overlapping file IDs and paths */
				(void) PrintOverlapFiles(GPtr);
			}

			if (scanflag == 0) {
				/* Directory inodes store first link information in 
				 * an extended attribute.  Therefore start directory 
				 * hard link check after extended attribute checks.
				 */
				result = dirhardlink_check(GPtr);
				/* On error or unrepairable corruption, stop the verification */
				if ((result != 0) || (GPtr->CatStat & S_LinkErrNoRepair)) {
					if (result == 0) {
						result = -1;
					}

					break;
				}
			}

			fsckPrint(GPtr->context, hfsVolBitmapCheck);

#if SHOW_ELAPSED_TIMES
			gettimeofday( &myStartTime, &zone );
#endif
				
			/* Compare in-memory volume bitmap with on-disk bitmap */
			if ((result = CheckVolumeBitMap(GPtr, false)))
				break;

#if SHOW_ELAPSED_TIMES
			gettimeofday( &myEndTime, &zone );
			timersub( &myEndTime, &myStartTime, &myElapsedTime );
			plog( "\n%s - CheckVolumeBitMap elapsed time \n", __FUNCTION__ );
			plog( ">>>>>>>>>>>>> secs %d msecs %d \n\n", 
				myElapsedTime.tv_sec, myElapsedTime.tv_usec );
#endif 

			if ((result = CheckForStop(GPtr)))
				break;

			fsckPrint(GPtr->context, hfsVolInfoCheck);

#if SHOW_ELAPSED_TIMES
			gettimeofday( &myStartTime, &zone );
#endif

			/* Verify volume level information */
			if ((result = VInfoChk(GPtr)))
				break;

#if SHOW_ELAPSED_TIMES
			gettimeofday( &myEndTime, &zone );
			timersub( &myEndTime, &myStartTime, &myElapsedTime );
			plog( "\n%s - VInfoChk elapsed time \n", __FUNCTION__ );
			plog( ">>>>>>>>>>>>> secs %d msecs %d \n\n", 
				myElapsedTime.tv_sec, myElapsedTime.tv_usec );
#endif

			stat =	GPtr->VIStat  | GPtr->ABTStat | GPtr->EBTStat | GPtr->CBTStat | 
					GPtr->CatStat | GPtr->JStat;
			
			if ( stat != 0 )
			{
				if ( (GPtr->RepLevel == repairLevelNoProblemsFound) || (GPtr->RepLevel == repairLevelVolumeRecoverable) )
				{
					//	2200106, We isolate very minor errors so that if the volume cannot be unmounted
					//	CheckDisk will just return noErr
					unsigned int minorErrors = (GPtr->CatStat & ~S_LockedDirName)  |
								GPtr->VIStat | GPtr->ABTStat | GPtr->EBTStat | GPtr->CBTStat | GPtr->JStat;
					if ( minorErrors == 0 )
						GPtr->RepLevel =  repairLevelVeryMinorErrors;
					else
						GPtr->RepLevel =  repairLevelVolumeRecoverable;
				}
			}
			else if ( GPtr->RepLevel == repairLevelNoProblemsFound )
			{
			}

			GPtr->itemsProcessed = GPtr->itemsToProcess;
			result = CheckForStop(GPtr);				//	one last check for modified volume
			break;
		}					
		
		case scavRepair:									//	REPAIR
		{
			if ( IsBlueBoxSharedDrive( GPtr->DrvPtr ) )
				break;
			if ( ( result = CheckForStop(GPtr) ) )
				break;
			if ( GPtr->CBTStat & S_RebuildBTree
				|| GPtr->EBTStat & S_RebuildBTree
				|| GPtr->ABTStat & S_RebuildBTree) {
//				fsckPrint(GPtr->context, hfsRebuildCatalogBTree);
//				fsckPrint(GPtr->context, hfsRebuildAttrBTree);
// actually print nothing yet -- we print out when we are rebuilding the trees
			} else {
				fsckPrint(GPtr->context, fsckRepairingVolume);
				if (embedded == 1 && debug == 0)
					fsckPrint(GPtr->context, fsckLimitedRepairs);
			}
			result = RepairVolume( GPtr );
			break;
		}
		
		case scavTerminate:									//	CLEANUP AFTER SCAVENGE
		{
			result = ScavTerm(GPtr);
			break;
		}
	}													//	end ScavOp switch


	//
	//	Map internal error codes to scavenger result codes
	//
	if ( (result < 0) || (result > Max_RCode) )
	{
		switch ( ScavOp )
		{	
			case scavInitialize:
			case scavVerify:
				if ( result == ioErr )
					result = R_RdErr;
				else if ( result == errRebuildBtree )
				{
					GPtr->RepLevel = repairLevelCatalogBtreeRebuild;
					break;
				}
				else
					result = R_VFail;
				GPtr->RepLevel = repairLevelUnrepairable;
				break;
			case scavRepair:
				result = R_RFail;
				break;
			default:
				result = R_IntErr;
		}
	}
	
	GPtr->ScavRes = result;

	*ScavRes = result;
		
}	//	end of ScavCtrl



/*------------------------------------------------------------------------------

Function: 	CheckForStop

Function:	Checks for the user hitting the "STOP" button during a scavenge,
			which interrupts the operation.  Additionally, we monitor the write
			count of a mounted volume, to be sure that the volume is not
			modified by another app while we scavenge.

Input:		GPtr		-  	pointer to scavenger global area

Output:		Function result:
						0 - ok to continue
						R_UInt - STOP button hit
						R_Modified - another app has touched the volume
-------------------------------------------------------------------------------*/

short CheckForStop( SGlob *GPtr )
{
	OSErr	err			= noErr;						//	Initialize err to noErr
	long	ticks		= TickCount();
	UInt16	dfaStage	= (UInt16) GetDFAStage();

        //plog("%d, %d", dfaStage, kAboutToRepairStage);

	//if ( ((ticks - 10) > GPtr->lastTickCount) || (dfaStage == kAboutToRepairStage) )			//	To reduce cursor flicker on fast machines, call through on a timed interval
	//{
		if ( GPtr->userCancelProc != nil )
		{
			UInt64	progress = 0;
			Boolean	progressChanged;
		//	UInt16	elapsedTicks;
			
			if ( dfaStage != kRepairStage )
			{
				progress = GPtr->itemsProcessed * 100;
				progress /= GPtr->itemsToProcess;
				progressChanged = ( progress != GPtr->lastProgress );
				GPtr->lastProgress = progress;
				
				#if( DisplayTimeRemaining )					
					if ( (progressChanged) && (progress > 5) )
					{
						elapsedTicks	=	TickCount() - GPtr->startTicks;
						GPtr->secondsRemaining	= ( ( ( 100 * elapsedTicks ) / progress ) - elapsedTicks ) / 60;
					}
				#endif
				err = CallUserCancelProc( GPtr->userCancelProc, (UInt16)progress, (UInt16)GPtr->secondsRemaining, progressChanged, dfaStage, GPtr->context, GPtr->scanCount );
			}
			else
			{
				(void) CallUserCancelProc( GPtr->userCancelProc, (UInt16)progress, 0, false, dfaStage, GPtr->context, GPtr->scanCount );
			}
			
		}
		
		if ( err != noErr )
			err = R_UInt;
	#if 0	
		if ( GPtr->realVCB )							// If the volume is mounted
			if ( GPtr->realVCB->vcbWrCnt != GPtr->wrCnt )
				err = R_Modified;					// Its been modified behind our back
	#endif
		GPtr->lastTickCount	= ticks;
	//}
			
	return ( err );
}
		


/*------------------------------------------------------------------------------

Function:	ScavSetUp - (Scavenger Set Up)

Function:	Sets up scavenger globals for a new scavenge operation.  Memory is 
			allocated for the Scavenger's static data structures (VCB, FCBs,
			BTCBs, and TPTs). The contents of the data structures are
			initialized to zero.
			
Input:		GPtr		-	pointer to scavenger global area

Output:		ScavSetUp	-	function result:			
								0	= no error
								n 	= error code
------------------------------------------------------------------------------*/

struct ScavStaticStructures {
	SVCB				vcb;
	SFCB				fcbList[6];		
	BTreeControlBlock	btcb[4];			// 4 btcb's
	SBTPT				btreePath;			// scavenger BTree path table
};
typedef struct ScavStaticStructures ScavStaticStructures;


static int ScavSetUp( SGlob *GPtr)
{
	OSErr  err;
	SVCB * vcb;
#if !BSD
	DrvQEl	*drvP;
	short	ioRefNum;
#endif

	GPtr->MinorRepairsP = nil;
	
	GPtr->itemsProcessed = 0;
	GPtr->lastProgress = 0;
	GPtr->startTicks = TickCount();
	
	//
	//	allocate the static data structures (VCB, FCB's, BTCB'S, DPT and BTPT)
	//
 	{
		ScavStaticStructures	*pointer;
		
		pointer = (ScavStaticStructures *) AllocateClearMemory( sizeof(ScavStaticStructures) );
		if ( pointer == nil ) {
			if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) {
				plog( "\t error %d - could not allocate %ld bytes of memory \n",
					R_NoMem, sizeof(ScavStaticStructures) );
			}
			return( R_NoMem );
		}
		GPtr->scavStaticPtr = pointer;

		GPtr->DirPTPtr = AllocateClearMemory(sizeof(SDPR) * CMMaxDepth);
		if ( GPtr->DirPTPtr == nil ) {
			if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) {
				plog( "\t error %d - could not allocate %ld bytes of memory \n",
					R_NoMem, sizeof(SDPR) * CMMaxDepth );
			}
			return( R_NoMem );
		}
		GPtr->dirPathCount = CMMaxDepth;

		GPtr->calculatedVCB = vcb	= &pointer->vcb;
		vcb->vcbGPtr = GPtr;

		GPtr->FCBAPtr				= (Ptr) &pointer->fcbList;
		GPtr->calculatedExtentsFCB		= &pointer->fcbList[0];
		GPtr->calculatedCatalogFCB		= &pointer->fcbList[1];
		GPtr->calculatedAllocationsFCB	= &pointer->fcbList[2];
		GPtr->calculatedAttributesFCB	= &pointer->fcbList[3];
		GPtr->calculatedStartupFCB		= &pointer->fcbList[4];
		GPtr->calculatedRepairFCB		= &pointer->fcbList[5];
		
		GPtr->calculatedExtentsBTCB		= &pointer->btcb[0];
		GPtr->calculatedCatalogBTCB		= &pointer->btcb[1];
		GPtr->calculatedRepairBTCB		= &pointer->btcb[2];
		GPtr->calculatedAttributesBTCB	= &pointer->btcb[3];
		
		GPtr->BTPTPtr					= (SBTPT*) &pointer->btreePath;
	}
	
	
	SetDFAStage( kVerifyStage );
	SetFCBSPtr( GPtr->FCBAPtr );

	//
	//	locate the driveQ element for drive being scavenged
	//
 	GPtr->DrvPtr	= 0;							//	<8> initialize so we can know if drive disappears

	//
	//	Set up Real structures
	//
#if !BSD 
	err = FindDrive( &ioRefNum, &(GPtr->DrvPtr), GPtr->DrvNum );
#endif	
	if ( IsBlueBoxSharedDrive( GPtr->DrvPtr ) )
		return noErr;
	
	err = GetVolumeFeatures( GPtr );				//	Sets up GPtr->volumeFeatures and GPtr->realVCB

#if !BSD
	if ( GPtr->DrvPtr == NULL )						//	<8> drive is no longer there!
		return ( R_NoVol );
	else
		drvP = GPtr->DrvPtr;

	//	Save current value of vcbWrCnt, to detect modifications to volume by other apps etc
	if ( GPtr->volumeFeatures & volumeIsMountedMask )
	{
		FlushVol( nil, GPtr->realVCB->vcbVRefNum );	//	Ask HFS to update all changes to disk
		GPtr->wrCnt = GPtr->realVCB->vcbWrCnt;		//	Remember write count after writing changes
	}
#endif

	//	Finish initializing the VCB

	//	The calculated structures
#if BSD
	InitBlockCache(vcb);
	vcb->vcbDriveNumber = GPtr->DrvNum;
	vcb->vcbDriverReadRef = GPtr->DrvNum;
	vcb->vcbDriverWriteRef = -1;	/* XXX need to get real fd here */
#else
	vcb->vcbDriveNumber = drvP->dQDrive;
	vcb->vcbDriverReadRef = drvP->dQRefNum;
	vcb->vcbDriverWriteRef = drvP->dQRefNum;
	vcb->vcbFSID = drvP->dQFSID;
#endif
//	vcb->vcbVRefNum = Vol_RefN;

	//
	//	finish initializing the FCB's
	//
	{
		SFCB *fcb;
		
		// Create Calculated Extents FCB
		fcb			= GPtr->calculatedExtentsFCB;
		fcb->fcbFileID		= kHFSExtentsFileID;
		fcb->fcbVolume		= vcb;
		fcb->fcbBtree		= GPtr->calculatedExtentsBTCB;
		vcb->vcbExtentsFile	= fcb;
			
		// Create Calculated Catalog FCB
		fcb			= GPtr->calculatedCatalogFCB;
		fcb->fcbFileID 		= kHFSCatalogFileID;
		fcb->fcbVolume		= vcb;
		fcb->fcbBtree		= GPtr->calculatedCatalogBTCB;
		vcb->vcbCatalogFile	= fcb;
	
		// Create Calculated Allocations FCB
		fcb			= GPtr->calculatedAllocationsFCB;
		fcb->fcbFileID		= kHFSAllocationFileID;
		fcb->fcbVolume 		= vcb;
		fcb->fcbBtree		= NULL;		//	no BitMap B-Tree
		vcb->vcbAllocationFile	= fcb;
	
		// Create Calculated Attributes FCB
		fcb			= GPtr->calculatedAttributesFCB;
		fcb->fcbFileID		= kHFSAttributesFileID;
		fcb->fcbVolume 		= vcb;
		fcb->fcbBtree		= GPtr->calculatedAttributesBTCB;
		vcb->vcbAttributesFile	= fcb;

		/* Create Calculated Startup FCB */
		fcb			= GPtr->calculatedStartupFCB;
		fcb->fcbFileID		= kHFSStartupFileID;
		fcb->fcbVolume 		= vcb;
		fcb->fcbBtree		= NULL;
		vcb->vcbStartupFile	= fcb;
	}
	
	//	finish initializing the BTCB's
	{
		BTreeControlBlock	*btcb;
		
		btcb			= GPtr->calculatedExtentsBTCB;		// calculatedExtentsBTCB
		btcb->fcbPtr		= GPtr->calculatedExtentsFCB;
		btcb->getBlockProc	= GetFileBlock;
		btcb->releaseBlockProc	= ReleaseFileBlock;
		btcb->setEndOfForkProc	= SetEndOfForkProc;
		
		btcb			= GPtr->calculatedCatalogBTCB;		// calculatedCatalogBTCB
		btcb->fcbPtr		= GPtr->calculatedCatalogFCB;
		btcb->getBlockProc	= GetFileBlock;
		btcb->releaseBlockProc	= ReleaseFileBlock;
		btcb->setEndOfForkProc	= SetEndOfForkProc;
	
		btcb			= GPtr->calculatedAttributesBTCB;	// calculatedAttributesBTCB
		btcb->fcbPtr		= GPtr->calculatedAttributesFCB;
		btcb->getBlockProc	= GetFileBlock;
		btcb->releaseBlockProc	= ReleaseFileBlock;
		btcb->setEndOfForkProc	= SetEndOfForkProc;
	}

	
	//
	//	Initialize some global stuff
	//

	GPtr->RepLevel			= repairLevelNoProblemsFound;
	GPtr->ErrCode			= 0;
	GPtr->IntErr			= noErr;
	GPtr->VIStat			= 0;
	GPtr->ABTStat			= 0;
	GPtr->EBTStat			= 0;
	GPtr->CBTStat			= 0;
	GPtr->CatStat			= 0;
	GPtr->VeryMinorErrorsStat	= 0;
	GPtr->JStat			= 0;

	/* Assume that the volume is dirty unmounted */
	GPtr->cleanUnmount		= false;

 	// 
 	// Initialize VolumeObject
 	//
 	
 	InitializeVolumeObject( GPtr );

	/* Check if the volume type of initialized object is valid.  If not, return error */
	if (VolumeObjectIsValid(GPtr) == false) {
		return (R_BadSig);
	}
 	
	// Keep a valid file id list for HFS volumes
	GPtr->validFilesList = (UInt32**)NewHandle( 0 );
	if ( GPtr->validFilesList == nil ) {
		if ( fsckGetVerbosity(GPtr->context) >= kDebugLog ) {
			plog( "\t error %d - could not allocate file ID list \n", R_NoMem );
		}
		return( R_NoMem );
	}

	// Convert the security attribute name from utf8 to utf16.  This will
	// avoid repeated conversion of all extended attributes to compare with
	// security attribute name
	(void) utf_decodestr((unsigned char *)KAUTH_FILESEC_XATTR, strlen(KAUTH_FILESEC_XATTR), GPtr->securityAttrName, &GPtr->securityAttrLen, sizeof(GPtr->securityAttrName));

	return( noErr );

} /* end of ScavSetUp */

		


/*------------------------------------------------------------------------------

Function:	ScavTerm - (Scavenge Termination))

Function:	Terminates the current scavenging operation.  Memory for the
			VCB, FCBs, BTCBs, volume bit map, and BTree bit maps is
			released.
			
Input:		GPtr		-	pointer to scavenger global area

Output:		ScavTerm	-	function result:			
								0	= no error
								n 	= error code
------------------------------------------------------------------------------*/

static int ScavTerm( SGlobPtr GPtr )
{
	SFCB			*fcbP;
	BTreeControlBlock	*btcbP;
	RepairOrderPtr		rP;
	OSErr			err;
	ExtentsTable 		**extentsTableH;
	ExtentInfo		*curExtentInfo;
	int			i;

	(void) BitMapCheckEnd();

	while( (rP = GPtr->MinorRepairsP) != nil )		//	loop freeing leftover (undone) repair orders
	{
		GPtr->MinorRepairsP = rP->link;				//	(in case repairs were not made)
		DisposeMemory(rP);
		err = MemError();
	}
	
	if( GPtr->validFilesList != nil )
		DisposeHandle( (Handle) GPtr->validFilesList );
	
	if( GPtr->overlappedExtents != nil ) {
 		extentsTableH = GPtr->overlappedExtents;
 	
		/* Overlapped extents list also allocated memory for attribute name */
		for (i=0; i<(**extentsTableH).count; i++) {
			curExtentInfo = &((**extentsTableH).extentInfo[i]);

			/* Deallocate memory for attribute name, if any */
			if (curExtentInfo->attrname) {
				free(curExtentInfo->attrname);
			}
		}

		DisposeHandle( (Handle) GPtr->overlappedExtents );
	}
	
	if( GPtr->fileIdentifierTable != nil )
		DisposeHandle( (Handle) GPtr->fileIdentifierTable );
	
	if( GPtr->calculatedVCB == nil )								//	already freed?
		return( noErr );

	//	If the FCB's and BTCB's have been set up, dispose of them
	fcbP = GPtr->calculatedExtentsFCB;	// release extent file BTree bit map
	if ( fcbP != nil )
	{
		btcbP = (BTreeControlBlock*)fcbP->fcbBtree;
		if ( btcbP != nil)
		{
			if( btcbP->refCon != nil )
			{
				if(((BTreeExtensionsRec*)btcbP->refCon)->BTCBMPtr != nil)
				{
					DisposeMemory(((BTreeExtensionsRec*)btcbP->refCon)->BTCBMPtr);
					err = MemError();
				}
				DisposeMemory( (Ptr)btcbP->refCon );
				err = MemError();
				btcbP->refCon = nil;
			}
				
			fcbP = GPtr->calculatedCatalogFCB;	//	release catalog BTree bit map
			btcbP = (BTreeControlBlock*)fcbP->fcbBtree;
				
			if( btcbP->refCon != nil )
			{
				if(((BTreeExtensionsRec*)btcbP->refCon)->BTCBMPtr != nil)
				{
					DisposeMemory(((BTreeExtensionsRec*)btcbP->refCon)->BTCBMPtr);
					err = MemError();
				}
				DisposeMemory( (Ptr)btcbP->refCon );
				err = MemError();
				btcbP->refCon = nil;
			}
		}
	}

	DisposeMemory(GPtr->DirPTPtr);
	DisposeMemory((ScavStaticStructures *)GPtr->scavStaticPtr);
	GPtr->scavStaticPtr = nil;
	GPtr->calculatedVCB = nil;

	return( noErr );
}

#define BLUE_BOX_SHARED_DRVR_NAME "\p.BlueBoxShared"
#define BLUE_BOX_FLOPPY_WHERE_STRING "\pdisk%d (Shared)"
#define SONY_DRVR_NAME "\p.Sony"

/*------------------------------------------------------------------------------

Routine:	IsBlueBoxSharedDrive

Function: 	Given a DQE address, return a boolean that determines whether
			or not a drive is a Blue Box disk being accessed via Shared mode.
			Such drives do not support i/o and cannot be scavenged.
			
Input:		Arg 1	- DQE pointer

Output:		D0.L -	0 if drive not to be used
					1 otherwise			
------------------------------------------------------------------------------*/

struct IconAndStringRec {
	char		icon[ 256 ];
	Str255		string;
};
typedef struct IconAndStringRec IconAndStringRec, * IconAndStringRecPtr;


Boolean IsBlueBoxSharedDrive ( DrvQElPtr dqPtr )
{
#if 0
	Str255			blueBoxSharedDriverName		= BLUE_BOX_SHARED_DRVR_NAME;
	Str255			blueBoxFloppyWhereString	= BLUE_BOX_FLOPPY_WHERE_STRING;
	Str255			sonyDriverName				= SONY_DRVR_NAME;
	DCtlHandle		driverDCtlHandle;
	DCtlPtr			driverDCtlPtr;
	DRVRHeaderPtr	drvrHeaderPtr;
	StringPtr		driverName;
	
	if ( dqPtr == NULL )
		return false;

	// Now look at the name of the Driver name. If it is .BlueBoxShared keep it out of the list of available disks.
	driverDCtlHandle = GetDCtlEntry(dqPtr->dQRefNum);
	driverDCtlPtr = *driverDCtlHandle;
	if((((driverDCtlPtr->dCtlFlags) & Is_Native_Mask) == 0) && (driverDCtlPtr->dCtlDriver != nil))
	{
		if (((driverDCtlPtr->dCtlFlags) & Is_Ram_Based_Mask) == 0)
		{
			drvrHeaderPtr = (DRVRHeaderPtr)driverDCtlPtr->dCtlDriver;
		}	
		else
		{
			//еее bek - lock w/o unlock/restore?  should be getstate/setstate?
			HLock((Handle)(driverDCtlPtr)->dCtlDriver);
			drvrHeaderPtr = (DRVRHeaderPtr)*((Handle)(driverDCtlPtr->dCtlDriver));
			
		}
		driverName = (StringPtr)&(drvrHeaderPtr->drvrName);
		if (!(IdenticalString(driverName,blueBoxSharedDriverName,nil)))
		{
			return( true );
		}

		// Special case for the ".Sony" floppy driver which might be accessed in Shared mode inside the Blue Box
		// Test its "where" string instead of the driver name.
		if (!(IdenticalString(driverName,sonyDriverName,nil)))
		{
			CntrlParam			paramBlock;
		
			paramBlock.ioCompletion	= nil;
			paramBlock.ioNamePtr	= nil;
			paramBlock.ioVRefNum	= dqPtr->dQDrive;
			paramBlock.ioCRefNum	= dqPtr->dQRefNum;
			paramBlock.csCode		= kDriveIcon;						// return physical icon
			
			// If PBControl(kDriveIcon) returns an error then the driver is not the Blue Box driver.
			if ( noErr == PBControlSync( (ParmBlkPtr) &paramBlock ) )
			{
				IconAndStringRecPtr		iconAndStringRecPtr;
				StringPtr				whereStringPtr;
				
				iconAndStringRecPtr = * (IconAndStringRecPtr*) & paramBlock.csParam;
				whereStringPtr = (StringPtr) & iconAndStringRecPtr->string;
				if (!(IdenticalString(whereStringPtr,blueBoxFloppyWhereString,nil)))
				{
					return( true );
				}
			}
		}
	}
#endif
	
	return false;
}

		


/*------------------------------------------------------------------------------

Function:	printVerifyStatus - (Print Verify Status)

Function:	Prints out the Verify Status words.
			
Input:		GPtr	-	pointer to scavenger global area

Output:		None.
------------------------------------------------------------------------------*/
static 
void printVerifyStatus(SGlobPtr GPtr)
{
    UInt32 stat;
                    
    stat = GPtr->VIStat | GPtr->ABTStat | GPtr->EBTStat | GPtr->CBTStat | GPtr->CatStat;
                    
    if ( stat != 0 ) {
       	plog("   Verify Status: VIStat = 0x%04x, ABTStat = 0x%04x EBTStat = 0x%04x\n", 
                GPtr->VIStat, GPtr->ABTStat, GPtr->EBTStat);     
       	plog("                  CBTStat = 0x%04x CatStat = 0x%08x\n", 
                GPtr->CBTStat, GPtr->CatStat);
    }
}
