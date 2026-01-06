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

/*!
 * @header IOCDMedia
 * @abstract
 * This header contains the IOCDMedia class definition.
 */

#ifndef _IOCDMEDIA_H
#define _IOCDMEDIA_H

/*!
 * @defined kIOCDMediaClass
 * @abstract
 * kIOCDMediaClass is the name of the IOCDMedia class.
 * @discussion
 * kIOCDMediaClass is the name of the IOCDMedia class.
 */

#define kIOCDMediaClass "IOCDMedia"

/*!
 * @defined kIOCDMediaTOCKey
 * @abstract
 * kIOCDMediaTOCKey is a property of IOCDMedia objects.  It has an OSData value
 * and a CDTOC structure.
 * @discussion
 * The kIOCDMediaTOCKey property contains the CD's full table of contents,
 * formatted as a CDTOC structure.  The CDTOC structure is same as what is
 * returned by a READ TOC command, format 0x02.  All fields in the TOC are
 * guaranteed to be binary-encoded (no BCD-encoded numbers are ever passed).
 */

#define kIOCDMediaTOCKey "TOC"

/*!
 * @defined kIOCDMediaTypeKey
 * @abstract
 * kIOCDMediaTypeKey is a property of IOCDMedia objects.  It has an OSString
 * value.
 * @discussion
 * The kIOCDMediaTypeKey property identifies the CD media type (CD-ROM, CD-R,
 * CD-RW, etc).  See the kIOCDMediaType contants for possible values.
 */

#define kIOCDMediaTypeKey "Type"

/*!
 * @defined kIOCDMediaTypeROM
 * The kIOCDMediaTypeKey constant for CD-ROM media (inclusive of the CD-I,
 * CD-ROM XA, and CD Audio standards, and mixed mode combinations thereof).
 */

#define kIOCDMediaTypeROM "CD-ROM"

/*!
 * @defined kIOCDMediaTypeR
 * The kIOCDMediaTypeKey constant for CD Recordable (CD-R) media.
 */

#define kIOCDMediaTypeR "CD-R"

/*!
 * @defined kIOCDMediaTypeRW
 * The kIOCDMediaTypeKey constant for CD ReWritable (CD-RW) media.
 */

#define kIOCDMediaTypeRW "CD-RW"

#endif /* !_IOCDMEDIA_H */
