#ifndef _corby_corby_h
#define _corby_corby_h

/*
 * Copyright 2003, 2004 Porchdog Software. All rights reserved.
 *
 *	Redistribution and use in source and binary forms, with or without modification,
 *	are permitted provided that the following conditions are met:
 *
 *		1. Redistributions of source code must retain the above copyright notice,
 *		   this list of conditions and the following disclaimer.
 *		2. Redistributions in binary form must reproduce the above copyright notice,
 *		   this list of conditions and the following disclaimer in the documentation
 *		   and/or other materials provided with the distribution.
 *
 *	THIS SOFTWARE IS PROVIDED BY PORCHDOG SOFTWARE ``AS IS'' AND ANY
 *	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *	IN NO EVENT SHALL THE HOWL PROJECT OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 *	INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *	BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 *	OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *	OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 *	OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	The views and conclusions contained in the software and documentation are those
 *	of the authors and should not be interpreted as representing official policies,
 *	either expressed or implied, of Porchdog Software.
 */

#include <salt/platform.h>

/*
 * corby limits
 */
#define SW_CORBY_OID_LEN		32


/*
 * protocol tags.  the only standard one is TAG_INTERNET_IIOP.
 * the others are proprietary pandora corby protocols.
 */
#define SW_TAG_INTERNET_IOP	0
#define SW_TAG_UIOP				250
#define SW_TAG_MIOP				251
#define SW_MIOP_ADDR				"231.255.255.250"
typedef sw_uint32				sw_corby_protocol_tag;


/*
 * error codes
 */
#define SW_E_CORBY_BASE					0x80000500
#define SW_E_CORBY_UNKNOWN				(SW_E_CORBY_BASE + 0)
#define SW_E_CORBY_BAD_CONFIG			(SW_E_CORBY_BASE + 1)
#define SW_E_CORBY_NO_INTERFACE		(SW_E_CORBY_BASE + 2)
#define SW_E_CORBY_BAD_URL				(SW_E_CORBY_BASE + 3)
#define SW_E_CORBY_BAD_NAME			(SW_E_CORBY_BASE + 4)
#define SW_E_CORBY_BAD_MESSAGE		(SW_E_CORBY_BASE + 5)
#define SW_E_CORBY_BAD_VERSION		(SW_E_CORBY_BASE + 6)
#define SW_E_CORBY_BAD_OID				(SW_E_CORBY_BASE + 7)
#define SW_E_CORBY_BAD_OPERATION		(SW_E_CORBY_BASE + 8)
#define SW_E_CORBY_MARSHAL				(SW_E_CORBY_BASE + 10)
#define SW_E_CORBY_OBJECT_NOT_EXIST	(SW_E_CORBY_BASE + 11)


#endif
