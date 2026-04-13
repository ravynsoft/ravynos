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
#include "fsck_messages.h"
#include "fsck_msgnums.h"

/*
 * Standard fsck message strings (not specific to any filesystem).
 *
 * The message numbers (first field) for these are defined
 * in fsck_msgnums.h; messages can be added to the array in
 * any order, as fsckAddMessages will sort them based on the
 * message number field.  The array needs to end with an all-0
 * field, and no message string can be NULL.
 *
 * The last field in the structure is a pointer to a constant,
 * variable-length array describing the arguments to the message.
 * Most messages have no arguments; if a message does have arguments,
 * it needs to be one of the types defined in fsck_msgnums.h (enum
 * fsck_arg_type).  The format specifier in the message string can be a
 * SIMPLE printf-style:  %d, %i, %u, %o, %x, %s, %c, %p; it needs to be
 * converted at run-time to a Cocoa-style specifier, and the conversion
 * routine does not handle all of the possible printf variations.
 * (See convertfmt() in fsck_messages.c for details.)
 */

fsck_message_t
fsck_messages_common[] = {
    /* Message Number                       Message                                                                     Type                Verbosity   Arguments */
    /* 101 - 110 */
    { fsckCheckingVolume,                   "Checking volume.",                                                         fsckMsgVerify,      fsckLevel0,   0, },
    { fsckRecheckingVolume,                 "Rechecking volume.",                                                       fsckMsgVerify,      fsckLevel0,   0, },
    { fsckRepairingVolume,                  "Repairing volume.",                                                        fsckMsgRepair,      fsckLevel0,   0, },
    { fsckVolumeOK,                         "The volume %s appears to be OK.",                                          fsckMsgSuccess,     fsckLevel0,   1, (const int[]) { fsckTypeVolume } },
    { fsckRepairSuccessful,                 "The volume %s was repaired successfully.",                                 fsckMsgSuccess,     fsckLevel0,   1, (const int[]) { fsckTypeVolume } },
    { fsckVolumeVerifyIncomplete,           "The volume %s could not be verified completely.",                          fsckMsgFail,        fsckLevel0,   1, (const int[]) { fsckTypeVolume } },
    { fsckVolumeVerifyIncompleteNoRepair,   "The volume %s could not be verified completely and can not be repaired.",  fsckMsgFail,        fsckLevel0,   1, (const int[]) { fsckTypeVolume } },
    { fsckVolumeCorruptNoRepair,            "The volume %s was found corrupt and can not be repaired.",                 fsckMsgFail,        fsckLevel0,   1, (const int[]) { fsckTypeVolume } },
    { fsckVolumeCorruptNeedsRepair,         "The volume %s was found corrupt and needs to be repaired.",                fsckMsgFail,        fsckLevel0,   1, (const int[]) { fsckTypeVolume } },
    { fsckVolumeNotRepaired,                "The volume %s could not be repaired.",                                     fsckMsgFail,        fsckLevel0,   1, (const int[]) { fsckTypeVolume } },

    /* 111 - 122 */
    { fsckVolumeNotRepairedInUse,           "The volume %s cannot be repaired when it is in use.",                      fsckMsgFail,        fsckLevel0,   1, (const int[]) { fsckTypeVolume } },
    { fsckVolumeNotVerifiedInUse,           "The volume %s cannot be verified when it is in use.",                      fsckMsgFail,        fsckLevel0,   1, (const int[]) { fsckTypeVolume } },
    { fsckFileFolderDamage,                 "File/folder %s may be damaged.",                                           fsckMsgDamageInfo,  fsckLevel0,   1, (const int[]) { fsckTypePath } },
    { fsckFileFolderNotRepaired,            "File/folder %s could not be repaired.",                                    fsckMsgDamageInfo,  fsckLevel0,   1, (const int[]) { fsckTypePath } },
    { fsckVolumeNotRepairedTries,           "The volume %s could not be repaired after %d attempts.",                   fsckMsgFail,        fsckLevel0,   2, (const int[]) { fsckTypeVolume, fsckTypeInt }},
    { fsckLostFoundDirectory,               "Look for missing items in %s directory.",                                  fsckMsgRepair,	    fsckLevel0,   1, (const int[]) { fsckTypeDirectory } },
    { fsckCorruptFilesDirectory,            "Look for links to corrupt files in %s directory.",                         fsckMsgDamageInfo,  fsckLevel0,   1, (const int[]) { fsckTypeDirectory }},
    { fsckInformation,                      "Executing %s (version %s).",                                               fsckMsgInfo,        fsckLevel1,   2, (const int[]) { fsckTypeString, fsckTypeString }},
    { fsckProgress,                         "%d %%",                                                                    fsckMsgProgress,    fsckLevel0,   1, (const int[]) { fsckTypeProgress } },
    { fsckTrimming,                         "Trimming unused blocks.",                                                  fsckMsgVerify,      fsckLevel0,   0 },
    { fsckVolumeName,                       "The volume name is %s",                                                    fsckMsgInfo,        fsckLevel0,   1, (const int[]) { fsckTypeVolume } },
    { fsckVolumeModified,                   "The volume was modified",                                                  fsckMsgNotice,      fsckLevel0,   0 },
    { fsckLimitedRepairs,                   "Limited repair mode, not all repairs available",                           fsckMsgInfo,        fsckLevel0,   0 },
    { 0, },
};

