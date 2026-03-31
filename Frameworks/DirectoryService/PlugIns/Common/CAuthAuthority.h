/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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

#ifndef __CAUTHAUTHORITY_H__
#define __CAUTHAUTHORITY_H__

#include <CoreFoundation/CoreFoundation.h>
#include <DirectoryService/DirServicesTypes.h>

class CAuthAuthority
{
	public:
												CAuthAuthority();
												CAuthAuthority(CAuthAuthority &inAuthAuthority);
		virtual									~CAuthAuthority();
												
		virtual bool							AddValue( const char *inAuthAuthorityStr );
		virtual bool							AddValue( CFStringRef inAuthAuthorityString );
		virtual bool							AddValues( CFArrayRef inAuthAuthorityArray );
		
		virtual CFIndex							GetValueCount( void );
		virtual char *							GetValueAtIndex( int inIndex );
		virtual char *							GetValueForTag( const char *inTagStr );
		virtual CFMutableDictionaryRef			GetValueForTagAsCFDict( const char *inTagStr );
		virtual char *							GetDataForTag( const char *inTagStr, CFIndex inDataSegmentIndex );
		
		virtual bool							SetValueForTag( const char *inTagStr, const char *inAuthAuthorityStr );
		virtual bool							SetValueDisabledForTag( const char *inTagStr );
		virtual bool							SetDataForTag( const char *inTagStr, const char *inDataSegment, CFIndex inDataSegmentIndex );

		virtual void							RemoveValueForTag( const char *inTagStr );
		virtual void							ToggleDisabledAuthority( bool enable );
		virtual bool							TagDisabled( const char *inTagStr );
		
	protected:
		CFMutableArrayRef mValueArray;
		CFMutableDictionaryRef mDisabledAuthorityStorage;
		
	private:
};

#endif
