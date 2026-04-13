/* Copyright © 2017 Apple Inc. All rights reserved.
 *
 *  FSOPS_Handler.h
 *  usbstorage_plugin
 *
 *  Created by Or Haimovich on 15/10/17.
 */

#ifndef FSOPS_Handler_h
#define FSOPS_Handler_h

#include "Common.h"

extern UVFSFSOps MSDOS_fsOps;

int FSOPS_SetDirtyBitAndAcquireLck(FileSystemRecord_s* psFSRecord);
int FSOPS_FlushCacheAndFreeLck(FileSystemRecord_s* psFSRecord);

#endif /* FSOPS_Handler_h */
