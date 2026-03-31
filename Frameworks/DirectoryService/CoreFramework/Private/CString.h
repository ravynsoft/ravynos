/*
 * Copyright (c) 1996-98, 2000-2002 Apple Computer, Inc. All rights reserved.
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
 * @header CString
 * Interface for a reasonable String Object.
 */

#ifndef _CSTRING_H
#define _CSTRING_H 1

#include <stdarg.h>		// for va_list type for Vsprintf
#include <string.h>		// for strcmp()

const int kCStringDefSize = 128 ;

//-----------------------------------------------------------------------------
//	* CString: Your Basic String Class
//	NOTE: static CStrings will show up in the leaks.log because destructors for
//	static objects are called after main exits, so the DebugNewReportLeaks()
//	will report them even though they do get deleted on exit.
//-----------------------------------------------------------------------------

class CString
{
public:
	/**** Typedefs, enums, and constants. ****/
	// Exceptions
	enum {
		kUnknownEscapeErr	= 'str0'	// bad %x in sprintf.
	};

public:
	/**** Instance methods. ****/
	// ctor and dtor.
				CString			( int sz = 0 );
				CString			( const char *str );
				CString			( const char *str, int len );
				CString			( const CString& cs );
				CString			( const char *pattern, va_list args );
	virtual	   ~CString			( void );
		
	// Inline accessors.
	int			GetLength		( void ) const ; 
					// { return mLength ; }
	int			GetAllocSize	( void ) const
					{ return mSize ; }
	char		*GetData		( void ) const
					{ return mData ; }
	void		GetPascal		( unsigned char *pstr ) const ;

				operator const char*	(  ) const
					{ return mData ; }
	int			operator==		( const CString& other ) const
					{ return !::strcmp (mData, other.mData) ; }
	CString&	operator=		( const CString& other )
					{ this->Set (other) ; return *this ; }

	// Data modifiers.
	void		Sprintf			( const char *pattern, ... );
	void		Vsprintf		( const char *pattern, va_list args );

	void		Set				( const char *str );
	void		Set				( const char *str, int len );
	void		Set				( const unsigned char *pstr );
	void		Set				( const CString& cs );

	void		Append			( char inChar );
	void		Append			( const char *str );
	void		Append			( const char *str, int len );
	void		Append			( const unsigned char *pstr );
	void		Append			( const CString& cs );

	void		Prepend			( const char *str );
	void		Clear			( int inNewSize = 0 );

protected:
	/**** Instance methods accessible only to class and subclasses. ****/
	void		Grow			( int newSz = 0 );

	/**** Instance data. ****/
	int			mSize;		// Alloc size, *not* length of string
	int			mLength;	// length of string
	char		*mData;		// ptr to string data.
};

#endif	/* _CSTRING_H */
