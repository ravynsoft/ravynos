/*
 * Copyright (c) 2008, 2010-2011 Apple Inc. All rights reserved.
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

/* fsck_msgnums.h
 *
 * This file contain fsck status message numbers associated with 
 * each fsck message string.  These status message numbers and their 
 * strings are file system independent.  
 */

#ifndef __FSCK_MSGNUMS_H
#define __FSCK_MSGNUMS_H

/* Generic fsck status message numbers.  These are file system 
 * independent messages that indicate the current state of verify or 
 * repair run or provide information about damaged files/folder.   
 *
 * The corresponding strings and the mapping array of message number 
 * and other attributes exists in fsck_strings.c
 */
enum fsck_msgnum {
    fsckUnknown                         = 100,

    fsckCheckingVolume                  = 101,  /* Checking volume */
    fsckRecheckingVolume                = 102,  /* Rechecking volume */
    fsckRepairingVolume                 = 103,  /* Repairing volume */
    fsckVolumeOK                        = 104,  /* The volume %s appears to be OK */
    fsckRepairSuccessful                = 105,  /* The volume %s was repaired successfully */
    fsckVolumeVerifyIncomplete          = 106,  /* The volume %s could not be verified completely */
    fsckVolumeVerifyIncompleteNoRepair  = 107,  /* The volume %s could not be verified completely and can not be repaired */
    fsckVolumeCorruptNoRepair           = 108,  /* The volume %s was found corrupt and can not be repaired */
    fsckVolumeCorruptNeedsRepair        = 109,  /* The volume %s was found corrupt and needs to be repaired */
    fsckVolumeNotRepaired               = 110,  /* The volume %s could not be repaired */

    fsckVolumeNotRepairedInUse          = 111,  /* The volume %s cannot be repaired when it is in use */
    fsckVolumeNotVerifiedInUse          = 112,  /* The volume %s cannot be verified when it is in use */
    fsckFileFolderDamage                = 113,  /* File/folder %s may be damaged */
    fsckFileFolderNotRepaired           = 114,  /* File/folder %s could not be repaired */
    fsckVolumeNotRepairedTries          = 115,  /* The volume %s could not be repaired after %d attempts */
    fsckLostFoundDirectory              = 116,  /* Look for missing items in %s directory */
    fsckCorruptFilesDirectory           = 117,  /* Look for links to corrupt files in %s directory */
    fsckInformation                     = 118,  /* Using %s (version %s) for checking volume %s of type %s. */
    fsckProgress                        = 119,  /* %d */
    fsckTrimming                        = 120,  /* Trimming unused blocks */
    fsckVolumeName                      = 121,	/* The volume name is %s */
    fsckVolumeModified			= 122,	/* The volume was modified */
    fsckLimitedRepairs			= 123,	/* Limited repair mode, not all repairs available */
};

#endif
