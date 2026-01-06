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
 * This header contains the IOGUIDPartitionScheme class definition.
 */

#ifndef _IOGUIDPARTITIONSCHEME_H
#define _IOGUIDPARTITIONSCHEME_H

#include <IOKit/IOTypes.h>

/*
 * kIOGUIDPartitionSchemeClass is the name of the IOGUIDPartitionScheme class.
 */

#define kIOGUIDPartitionSchemeClass "IOGUIDPartitionScheme"

/*
 * GUID Partition Map Definitions
 */

#include <uuid/uuid.h>

#pragma pack(push, 1)                        /* (enable 8-bit struct packing) */

/* Partition map. */

struct gpt_hdr
{
    uint8_t  hdr_sig[8];
    uint32_t hdr_revision;
    uint32_t hdr_size;
    uint32_t hdr_crc_self;
    uint32_t __reserved;
    uint64_t hdr_lba_self;
    uint64_t hdr_lba_alt;
    uint64_t hdr_lba_start;
    uint64_t hdr_lba_end;
    uuid_t   hdr_uuid;
    uint64_t hdr_lba_table;
    uint32_t hdr_entries;
    uint32_t hdr_entsz;
    uint32_t hdr_crc_table;
    uint32_t padding;
};

/* Partition map entry. */

struct gpt_ent
{
    uuid_t   ent_type;
    uuid_t   ent_uuid;
    uint64_t ent_lba_start;
    uint64_t ent_lba_end;
    uint64_t ent_attr;
    uint16_t ent_name[36];
};

/* Partition map signature (hdr_sig). */

#define GPT_HDR_SIG "EFI PART"

/* Partition map version (hdr_revision). */

#define GPT_HDR_REVISION 0x00010000

/* Partition map entry flags (ent_attr). */

#define GPT_ENT_ATTR_PLATFORM 0x00000001

/*!
 * @defined kIOGUIDPartitionSchemeUUIDKey
 * @abstract
 * A property of IOGUIDPartitionSchemeGUID objects
 * @discussion
 * The kIOGUIDPartitionSchemeUUIDKey property has an OSString value and contains
 * a persistent GUID for the disk define in GPT header
 */
#define kIOGUIDPartitionSchemeUUIDKey	"UUID"

/*!
 * @defined kIOMediaGPTPartitionAttributesKey
 * @abstrat
 * A property of IOMedia objects for GPT partitions
 * @discussion
 * The kIOMediaGPTPartitionAttributesKey property has an OSNumber value of 64bit
 * GPT partition attributes
 */
#define kIOMediaGPTPartitionAttributesKey "GPT Attributes"

#pragma pack(pop)                        /* (reset to default struct packing) */

#endif /* !_IOGUIDPARTITIONSCHEME_H */
