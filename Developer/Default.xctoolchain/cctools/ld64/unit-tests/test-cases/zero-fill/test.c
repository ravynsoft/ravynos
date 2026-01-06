/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*- 
 *
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
#include <stdio.h>

// if we used one big array, the linker would page align it
// but we want to test a non-page align big chunk of zero-fill data
int bigarray1[256];
int bigarray2[256];
int bigarray3[256];
int bigarray4[256];
int bigarray5[256];
int bigarray6[256];
static int staticbigarray1[256];
static int staticbigarray2[256];
static int staticbigarray3[256];
static int staticbigarray4[256];
static int staticbigarray5[256];
static int staticbigarray6[256];

int main()
{
	staticbigarray1[10] = 4;
	staticbigarray2[10] = 4;
	staticbigarray3[10] = 4;
	staticbigarray4[10] = 4;
	staticbigarray5[10] = 4;
	staticbigarray6[10] = 4;
	return 0;
}

