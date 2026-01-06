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
 * @header IOCDPartitionScheme
 * @abstract
 * This header contains the IOCDPartitionScheme class definition.
 */

#ifndef _IOCDPARTITIONSCHEME_H
#define _IOCDPARTITIONSCHEME_H

#include <IOKit/storage/IOCDTypes.h>

/*
 * @defined kIOCDPartitionSchemeClass
 * @abstract
 * kIOCDPartitionSchemeClass is the name of the IOCDPartitionScheme class.
 * @discussion
 * kIOCDPartitionSchemeClass is the name of the IOCDPartitionScheme class.
 */

#define kIOCDPartitionSchemeClass "IOCDPartitionScheme"

/*
 * @defined kIOMediaSessionIDKey
 * @abstract
 * kIOMediaSessionIDKey is property of IOMedia objects.  It has an OSNumber
 * value.
 * @discussion
 * The kIOMediaSessionIDKey property is placed into each IOMedia instance
 * created by the CD partition scheme.  It identifies the session number
 * the track was recorded on.
 */

#define kIOMediaSessionIDKey "Session ID"

#endif /* !_IOCDPARTITIONSCHEME_H */
