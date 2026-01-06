/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*- 
 *
 * Copyright (c) 2009-2010 Apple Inc. All rights reserved.
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


extern void* data_start  __asm("segment$start$__DATA");
extern void* data_end    __asm("segment$end$__DATA");
extern void* text_start  __asm("segment$start$__TEXT");
extern void* text_end    __asm("segment$end$__TEXT");
extern void* other_start __asm("segment$start$__OTHER");
extern void* other_end   __asm("segment$end$__OTHER");


int other[100] __attribute__ ((section ("__OTHER,__my"))) = { 1, 2 };

int mytent[1000];
static int mybss[1000];

int main()
{
	mytent[0] = 0;
	mybss[0] = 0;
	printf("text %p -> %p\n", &text_start, &text_end);
	printf("data %p -> %p\n", &data_start, &data_end);
	printf("other %p -> %p\n", &other_start, &other_end);
	return 0;
}


