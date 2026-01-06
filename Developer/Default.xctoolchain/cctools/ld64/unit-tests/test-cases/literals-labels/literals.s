/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
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

	.literal16
L01:.long 12345678
	.long 87654321
	.long 12345678
	.long 87654321
		
L02:.long 12345678
	.long 87654321
	.long 12345678
	.long 87654322

_foo16:
_bar16:
	.long 22345678
	.long 87654321
	.long 12345678
	.long 87654323

L04:.long 12345678
	.long 87654321
	.long 12345678
	.long 87654324


	.literal8
L1:	.long 12345678
	.long 87654321
	
L2:	.long 12345678
	.long 87654322
	
_foo8:
_bar8:
	.long 22345678
	.long 87654323
	
L4:	.long 12345678
	.long 87654324
	
	.literal4	
L11:.long 12345678
L12:.long 12345679
_foo4:
_bar4:
	.long 22345670
L14:.long 12345671
	
	.cstring
L21: .ascii "hello\0"
L22: .ascii "hello,there\0"
_string1:
_string2:
	.ascii "there\0"
L24: .ascii "bye\0"
