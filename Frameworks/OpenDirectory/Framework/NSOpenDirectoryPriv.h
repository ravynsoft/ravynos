/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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
    @header		NSOpenDirectoryPriv
    @abstract   Contains private functions that are not public API
    @discussion Contains private functions that are not public API
*/

#import <OpenDirectory/OpenDirectory.h>

@interface ODRecord (PrivateExtensions)

/*!
	@method     isMemberRecordRefresh: error:
	@abstract   Will use membership APIs to determine if inRecord is a member of the group ignoring the cache
	@discussion Will use membership APIs to determine if inRecord is a member of the group ignoring the cache.  
				If the receiving object is not a group then NO will still be returned.  outError is optional parameter, 
				nil can be passed if error details are not needed.
 */
- (BOOL)isMemberRecordRefresh:(ODRecord *)inRecord error:(NSError **)outError;

@end
