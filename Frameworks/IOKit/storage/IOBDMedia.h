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

/*!
 * @header IOBDMedia
 * @abstract
 * This header contains the IOBDMedia class definition.
 */

#ifndef _IOBDMEDIA_H
#define _IOBDMEDIA_H

/*!
 * @defined kIOBDMediaClass
 * @abstract
 * kIOBDMediaClass is the name of the IOBDMedia class.
 * @discussion
 * kIOBDMediaClass is the name of the IOBDMedia class.
 */

#define kIOBDMediaClass "IOBDMedia"

/*!
 * @defined kIOBDMediaTypeKey
 * @abstract
 * kIOBDMediaTypeKey is a property of IOBDMedia objects.  It has an OSString
 * value.
 * @discussion
 * The kIOBDMediaTypeKey property identifies the BD media type (BD-ROM, BD-R,
 * BD-RE, etc).  See the kIOBDMediaType contants for possible values.
 */

#define kIOBDMediaTypeKey "Type"

/*!
 * @defined kIOBDMediaTypeROM
 * The kIOBDMediaTypeKey constant for BD-ROM media.
 */

#define kIOBDMediaTypeROM "BD-ROM"

/*!
 * @defined kIOBDMediaTypeR
 * The kIOBDMediaTypeKey constant for BD-R media.
 */

#define kIOBDMediaTypeR "BD-R"

/*!
 * @defined kIOBDMediaTypeRE
 * The kIOBDMediaTypeKey constant for BD-RE media.
 */

#define kIOBDMediaTypeRE "BD-RE"

#endif /* !_IOBDMEDIA_H */
