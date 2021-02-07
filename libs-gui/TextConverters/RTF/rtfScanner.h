/* rtcScanner.h

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author:  Stefan Boehringer (stefan.boehringer@uni-bochum.de)
   Date: Dec 1999

   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#ifndef rtfScanner_h_INCLUDE
#define rtfScanner_h_INCLUDE

#include	<objc/objc.h>

#if !defined(YES)
	typedef	char	BOOL;
	enum { NO, YES };
#endif

typedef enum { NoError, LEXoutOfMemory, LEXsyntaxError } GSLexError;

typedef struct _RTFscannerCtxt {
	int	(*lgetchar)(void *);
	char	pushbackBuffer[4];	// gaurantee 4 chars of pushback
	int	pushbackCount;
	int	streamPosition;
	int	streamLineNumber;
	void	*customContext;
} RTFscannerCtxt;

typedef struct {
	BOOL	isEmpty;
	int	parameter;
	int	token;
	const char	*name;
} RTFcmd;

typedef enum {
	GSRTFfamilyNil, GSRTFfamilyRoman, GSRTFfamilySwiss,
	GSRTFfamilyModern, GSRTFfamilyScript, GSRTFfamilyDecor,
	GSRTFfamilyTech
} RTFfontFamily;


void	lexInitContext(RTFscannerCtxt *lctxt, void *customContext, int (*getcharFunction)());

/*	external symbols from the grammer	*/
/*int	GSRTFparse(void *ctxt, RTFscannerCtxt *lctxt);*/
int	GSRTFparse(void *ctxt, void *lctxt);

#endif

