/*
 * Copyright (c) 2010 Apple Inc. All Rights Reserved.
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

#ifndef CC_CommonBufferingPriv_h
#define CC_CommonBufferingPriv_h

#include <CommonNumerics/CommonNumerics.h>
#include <stdbool.h>

typedef struct CNBuffer_t {
    size_t chunksize;
    size_t bufferPos;
    uint8_t *buf;
} CNBuffer, *CNBufferRef;

CNBufferRef
CNBufferCreate(size_t chunksize);

CNStatus
CNBufferRelease(CNBufferRef *bufRef);

typedef int (*cnProcessFunction)(void *ctx, const void *in, size_t inLen, void *out, size_t *outLen);
typedef size_t (*cnSizeFunction)(void *ctx, size_t inLen);

CNStatus
CNBufferProcessData(CNBufferRef bufRef, 
                    void *ctx, const void *in, const size_t inLen, void *out, size_t *outLen, 
                    cnProcessFunction pFunc, cnSizeFunction sizeFunc);

CNStatus
CNBufferFlushData(CNBufferRef bufRef,
                  void *ctx, void *out, size_t *outLen,
                  cnProcessFunction pFunc, cnSizeFunction sizeFunc);


bool
CNBufferEmpty(CNBufferRef bufRef);





#endif /* CC_CommonBufferingPriv_h */
