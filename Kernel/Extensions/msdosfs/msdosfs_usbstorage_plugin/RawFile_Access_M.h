/* Copyright © 2017 Apple Inc. All rights reserved.
 *
 * RawFile_Access_M.h
 *  msdosfs.kext
 *
 *  Created by Susanne Romano on 09/10/2017.
 */

#ifndef RawFile_Access_M_h
#define RawFile_Access_M_h

#include <stdio.h>
#include "Common.h"

size_t RAWFILE_read(NodeRecord_s* psFileNode, uint64_t uOffset, size_t uLength, void *pvBuf, int* piError);
size_t RAWFILE_write(NodeRecord_s* psFileNode, uint64_t uOffset, uint64_t uLength, void *pvBuf, int* piError);

#endif /* RawFile_Access_M_h */
