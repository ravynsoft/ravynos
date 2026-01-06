/*
 * Copyright (c) 1998-2015 Apple Inc. All rights reserved.
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
 * @header IOStorage
 * @abstract
 * This header contains the IOStorage class definition.
 */

#ifndef _IOSTORAGE_H
#define _IOSTORAGE_H

#include <sys/kernel_types.h>
#include <IOKit/IOTypes.h>

/*!
 * @defined kIOStorageClass
 * @abstract
 * The name of the IOStorage class.
 */

#define kIOStorageClass "IOStorage"

/*!
 * @defined kIOStorageCategory
 * @abstract
 * kIOStorageCategory is a value for IOService's kIOMatchCategoryKey property.
 * @discussion
 * The kIOStorageCategory value is the standard value for the IOService property
 * kIOMatchCategoryKey ("IOMatchCategory") for all storage drivers.  All storage
 * objects that expect to drive new content (that is, produce new media objects)
 * are expected to compete within the kIOStorageCategory namespace.
 *
 * See the IOService documentation for more information on match categories.
 */

#define kIOStorageCategory "IOStorage"                /* (as IOMatchCategory) */

/*!
 * @defined kIOStorageFeaturesKey
 * @abstract
 * A property of any object in the storage stack.
 * @discussion
 * kIOStorageFeaturesKey is a property of any object in the storage stack that
 * wishes to express support of additional features, such as Force Unit Access.
 * It is typically defined in the device object below the block storage driver
 * object.  It has an OSDictionary value, where each entry describes one given
 * feature.
 */

#define kIOStorageFeaturesKey "IOStorageFeatures"

/*!
 * @defined kIOStorageFeatureBarrier
 * @abstract
 * Describes the presence of the Barrier feature.
 * @discussion
 * This property describes the ability of the storage stack to honor a write
 * barrier, guaranteeing that on power loss, writes after the barrier will not
 * be visible until all writes before the barrier are visible.  It is one of the
 * feature entries listed under the top-level kIOStorageFeaturesKey property
 * table.  It has an OSBoolean value.
 */

#define kIOStorageFeatureBarrier "Barrier"

/*!
 * @defined kIOStorageFeatureForceUnitAccess
 * @abstract
 * Describes the presence of the Force Unit Access feature.
 * @discussion
 * This property describes the ability of the storage stack to force a request
 * to access the media.  It is one of the feature entries listed under the top-
 * level kIOStorageFeaturesKey property table.  It has an OSBoolean value.
 */

#define kIOStorageFeatureForceUnitAccess "Force Unit Access"

/*!
 * @defined kIOStorageFeaturePriority
 * @abstract
 * Describes the presence of the Priority feature.
 * @discussion
 * This property describes the ability of the storage stack to enforce the
 * priority of a request.  It is one of the feature entries listed under the
 * top-level kIOStorageFeaturesKey property table.  It has an OSBoolean value.
 */

#define kIOStorageFeaturePriority "Priority"

/*!
 * @defined kIOStorageFeatureUnmap
 * @abstract
 * Describes the presence of the Unmap feature.
 * @discussion
 * This property describes the ability of the storage stack to delete unused
 * data from the media.  It is one of the feature entries listed under the top-
 * level kIOStorageFeaturesKey property table.  It has an OSBoolean value.
 */

#define kIOStorageFeatureUnmap "Unmap"

#endif /* !_IOSTORAGE_H */
