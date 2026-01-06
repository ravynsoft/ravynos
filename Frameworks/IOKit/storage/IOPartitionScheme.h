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
 * @header IOPartitionScheme
 * @abstract
 * This header contains the IOPartitionScheme class definition.
 */

#ifndef _IOPARTITIONSCHEME_H
#define _IOPARTITIONSCHEME_H

/*!
 * @defined kIOPartitionSchemeClass
 * @abstract
 * The name of the IOPartitionScheme class.
 * @discussion
 * kIOPartitionSchemeClass is the name of the IOPartitionScheme class.
 */

#define kIOPartitionSchemeClass "IOPartitionScheme"

/*!
 * @defined kIOMediaBaseKey
 * @abstract
 * A property of IOMedia objects.
 * @discussion
 * The kIOMediaBaseKey property has an OSNumber value and is placed into an
 * IOMedia instance created via the partition scheme. It describes the byte
 * offset of the partition relative to the provider media.
 */

#define kIOMediaBaseKey "Base"

/*!
 * @defined kIOMediaLiveKey
 * @abstract
 * A property of IOMedia objects.
 * @discussion
 * The kIOMediaLiveKey property has an OSBoolean
 * value and is placed into an IOMedia instance
 * created via the partition scheme.  It describes whether the
 * partition is live, that is, it is up-to-date with respect
 * to the on-disk partition table.
 */

#define kIOMediaLiveKey "Live"

/*!
 * @defined kIOMediaPartitionIDKey
 * @abstract
 * A property of IOMedia objects.
 * @discussion
 * The kIOMediaPartitionIDKey property has an OSNumber
 * value and is placed into an IOMedia instance
 * created via the partition scheme.  It is an ID that differentiates one 
 * partition from the other (within a given scheme).  It is typically an index
 * into the on-disk partition table.
 */

#define kIOMediaPartitionIDKey "Partition ID"

#endif /* !_IOPARTITIONSCHEME_H */
