/*
 * Copyright (c) 1998-2014 Apple Inc. All rights reserved.
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
 * This header contains the IOAppleLabelScheme class definition.
 */

#ifndef _IOAPPLELABELSCHEME_H
#define _IOAPPLELABELSCHEME_H

#include <IOKit/IOTypes.h>

/*
 * kIOAppleLabelSchemeClass is the name of the IOAppleLabelScheme class.
 */

#define kIOAppleLabelSchemeClass "IOAppleLabelScheme"

/*
 * Apple Label Scheme Definitions
 */

#pragma pack(push, 1)                        /* (enable 8-bit struct packing) */

/* Label scheme. */

struct applelabel
{
    uint8_t  al_boot0[416];               /* (reserved for boot area)         */
    uint16_t al_magic;                    /* (the magic number)               */
    uint16_t al_type;                     /* (label type)                     */
    uint32_t al_flags;                    /* (generic flags)                  */
    uint64_t al_offset;                   /* (offset of property area, bytes) */
    uint32_t al_size;                     /* (size of property area, bytes)   */
    uint32_t al_checksum;                 /* (checksum of property area)      */
    uint8_t  al_boot1[72];                /* (reserved for boot area)         */
};

/* Label scheme signature (al_magic). */

#define AL_MAGIC 0x414C

/* Label scheme version (al_type). */

#define AL_TYPE_DEFAULT 0x0000

/* Label scheme flags (al_flags). */

#define AL_FLAG_DEFAULT 0x00000000

#pragma pack(pop)                        /* (reset to default struct packing) */

#endif /* !_IOAPPLELABELSCHEME_H */
