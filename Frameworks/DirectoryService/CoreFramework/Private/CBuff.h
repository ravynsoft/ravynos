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
 * @header CBuff
 */

/*
Start of buffer:

Buffer Type	  Dat Block Cnt 	Offset 1   Offset 2  Offset n	EndTag = 'EndT'
	4			     4			   4		  4		    4		  4


	Total Rec Len	Rec Type Len	Rec Type	Rec Name Len	Rec Name
		4				2				n			2				n

	Attr Cnt	Attr Len  AttrName Len	Attr Name
		2			2			2			n
		  Value Cnt	 Value1 Len    Value1	Value2 Len	 Value2
			  2			2			n			2			n


	o) The data is stacked on at the end of the buffer.

*/

#ifndef __CBuff_h__
#define	__CBuff_h__	1

#include <DirectoryServiceCore/PrivateTypes.h>


class CBuff {
public:

enum {
	kNotInitalized		= -119,
	kBuffNull			= -120,
	kBuffFull			= -121,
	kBuffTooSmall		= -122,
	kBuffInvalFormat	= -123
};

enum {
	kEndTag		= 'EndT'
};

public:
					CBuff					( void );
	virtual		   ~CBuff					( void );

	SInt32			Initialize				( tDataBuffer *inBuff, bool inClear = false );

	SInt32			GetBuffType				( UInt32 *outType );
	SInt32			SetBuffType				( UInt32 inBuffType );

	SInt32			SetBuffLen				( UInt32 inBuffLen );

	tDataBuffer*	GetBuffer				( void );
	SInt32			GetBuffStatus			( void );

	SInt32			AddData					( char *inData, UInt32 inLen );
	SInt32			GetDataBlockCount		( UInt32 *outCount );
	char*			GetDataBlock			( UInt32 inIndex, UInt32 *outOffset );
	UInt32			GetDataBlockLength		( UInt32 inIndex );

	void			ClearBuff				( void );
	void			SetLengthToSize			( void );

protected:
	SInt32			SetNextOffset			( UInt32 inLen );

private:
	SInt32			fStatus;
	UInt32			fOffset;
	UInt32			fWhatsLeft;
	tDataBuffer	   *fBuff;
};

#endif

