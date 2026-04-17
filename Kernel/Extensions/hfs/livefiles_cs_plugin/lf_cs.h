//
// Copyright (c) 2019-2019 Apple Inc. All rights reserved.
//
// lf_cs.h - Defines macros for livefiles Apple_CoreStorage plugin.
//

#ifndef _LF_CS_H
#define _LF_CS_H

#include <UserFS/UserVFS.h>

//
// Device/Block alignment and rounding.
//
#define CS_HOWMANY(_n, _b) howmany(_n, _b)
#define CS_ALIGN(_n, _b, _up)\
	(((_up) ? CS_HOWMANY(_n, _b) : ((_n) / (_b))) * (_b))

#define CS_STATUS(_s)           ((_s) & 0x000FFFFF)
#define CS_STATUS_OK            0x00000000      // No error.
#define CS_STATUS_NOTCS         0x00000001      // Not a current CS volume.
#define CS_STATUS_CKSUM         0x00000002      // Cksum mismatch.
#define CS_STATUS_BLKTYPE       0x00000004      // Incorrect block type.
#define CS_STATUS_INVALID       0x00000008      // Invalid field value.
#define CS_STATUS_ADDRESS       0x00000010      // Invalid laddr/vaddr.
#define CS_STATUS_TXG           0x00000020      // Invalid transaction.
#define CS_STATUS_UUID          0x00000040      // Inconsistent UUID.


#define CS_INFO(_s)             ((_s) & 0xFFF00000)
#define CS_INFO_VERSIONITIS     0x00100000      // Incompatible CS volume format
#define CS_INFO_ZERO_VH         0x02000000      // In-progress CS volume forma

#endif /* _LF_CS_H */
