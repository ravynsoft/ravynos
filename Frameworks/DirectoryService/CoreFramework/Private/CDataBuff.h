/*
 * Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
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
 * @header CDataBuff
 */

#ifndef __CDataBuff_h__
#define __CDataBuff_h__	1

#include <DirectoryServiceCore/PrivateTypes.h>
#include <stdarg.h>	//for va_list

const UInt32 kDefaultSize = 512;

// Class --------------------------------------------------------------------

class CDataBuff {
public:
				CDataBuff		( UInt32 inSize = 0 );
	virtual	   ~CDataBuff		( void );

	void		AppendString	( const char *inStr );
	void		AppendLong		( UInt32 inInt );
	void		AppendShort		( UInt16 inShort );
	void		AppendBlock		( const void *inData, UInt32 inLength );

	void		Clear			( UInt32 inSize = 0 );

	UInt32		GetSize			( void );
	UInt32		GetLength		( void );
	char*		GetData			( void );

protected:

private:
	void		GrowBuff		( UInt32 inNewSize = 0 );

	UInt32		fSize;
	UInt32		fLength;
	char	   *fData;
};

SInt32			dsCDataBuffFromAttrTypeAndStringValue( CDataBuff* inOutAttrDataBuff, CDataBuff* inOutDataBuff, bool inbAttrInfoOnly, const char* inAttrType, const char* inAttrValue );
SInt32			dsCDataBuffFromAttrTypeAndData( CDataBuff* inOutAttrDataBuff, CDataBuff* inOutDataBuff, bool inbAttrInfoOnly, const char* inAttrType, const char* inAttrValue, UInt32 inLength );
SInt32			dsCDataBuffFromAttrTypeAndStringValues( CDataBuff* inOutAttrDataBuff, CDataBuff* inOutDataBuff, bool inbAttrInfoOnly, const char* inAttrType, const char* inAttrValue, ... );
SInt32			dsCDataBuffFromAttrTypeAndStringArgValues( CDataBuff* inOutAttrDataBuff, CDataBuff* inOutDataBuff, bool inbAttrInfoOnly, const char* inAttrType, const char* inAttrValue, va_list inAttrValues );
SInt32			dsCDataBuffFromAttrTypeAndStringValues( CDataBuff* inOutAttrDataBuff, CDataBuff* inOutDataBuff, bool inbAttrInfoOnly, const char* inAttrType, const char** inAttrValues );

#endif
