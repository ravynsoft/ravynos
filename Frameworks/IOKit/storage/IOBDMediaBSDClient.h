/*
 * Copyright (c) 2006-2014 Apple Inc. All rights reserved.
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

#ifndef _IOBDMEDIABSDCLIENT_H
#define _IOBDMEDIABSDCLIENT_H

#include <sys/ioctl.h>
#include <sys/types.h>

#include <IOKit/storage/IOBDTypes.h>

/*
 * Definitions
 *
 * ioctl                        description
 * ---------------------------- ------------------------------------------------
 * DKIOCBDREADSTRUCTURE         see IOBDMedia::readStructure()    in IOBDMedia.h
 *
 * DKIOCBDREADDISCINFO          see IOBDMedia::readDiscInfo()     in IOBDMedia.h
 * DKIOCBDREADTRACKINFO         see IOBDMedia::readTrackInfo()    in IOBDMedia.h
 *
 * DKIOCBDREPORTKEY             see IOBDMedia::reportKey()        in IOBDMedia.h
 * DKIOCBDSENDKEY               see IOBDMedia::sendKey()          in IOBDMedia.h
 *
 * DKIOCBDGETSPEED              see IOBDMedia::getSpeed()         in IOBDMedia.h
 * DKIOCBDSETSPEED              see IOBDMedia::setSpeed()         in IOBDMedia.h
 *
 *         in /System/Library/Frameworks/Kernel.framework/Headers/IOKit/storage/
 */

typedef struct
{
    uint8_t  format;

    uint8_t  reserved0008[3];                      /* reserved, clear to zero */

    uint32_t address;
    uint8_t  grantID;
    uint8_t  layer;

    uint8_t  reserved0080[4];                      /* reserved, clear to zero */

    uint16_t bufferLength;
    void *   buffer;
} dk_bd_read_structure_t;

typedef struct
{
    uint8_t  format;
    uint8_t  keyClass;
    uint8_t  blockCount;

    uint8_t  reserved0024[1];                         /* reserved, clear to zero */

    uint32_t address;
    uint8_t  grantID;

    uint8_t  reserved0072[5];                      /* reserved, clear to zero */

    uint16_t bufferLength;
    void *   buffer;
} dk_bd_report_key_t;

typedef struct
{
    uint8_t  format;
    uint8_t  keyClass;

    uint8_t  reserved0016[6];                      /* reserved, clear to zero */

    uint8_t  grantID;

    uint8_t  reserved0072[5];                      /* reserved, clear to zero */

    uint16_t bufferLength;
    void *   buffer;
} dk_bd_send_key_t;

typedef struct
{
    uint8_t  reserved0000[14];                     /* reserved, clear to zero */

    uint16_t bufferLength;                         /* actual length on return */
    void *   buffer;
} dk_bd_read_disc_info_t;

typedef struct
{
    uint8_t  reserved0000[4];                      /* reserved, clear to zero */

    uint32_t address;
    uint8_t  addressType;

    uint8_t  reserved0072[5];                      /* reserved, clear to zero */

    uint16_t bufferLength;                         /* actual length on return */
    void *   buffer;
} dk_bd_read_track_info_t;

#define DKIOCBDREADSTRUCTURE   _IOW('d', 160, dk_bd_read_structure_t)
#define DKIOCBDREPORTKEY       _IOW('d', 161, dk_bd_report_key_t)
#define DKIOCBDSENDKEY         _IOW('d', 162, dk_bd_send_key_t)

#define DKIOCBDGETSPEED        _IOR('d', 163, uint16_t)
#define DKIOCBDSETSPEED        _IOW('d', 163, uint16_t)

#define DKIOCBDREADDISCINFO    _IOWR('d', 164, dk_bd_read_disc_info_t)
#define DKIOCBDREADTRACKINFO   _IOWR('d', 165, dk_bd_read_track_info_t)

#define DKIOCBDSPLITTRACK      _IOW('d', 166, uint32_t)

#endif /* !_IOBDMEDIABSDCLIENT_H */
