/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*- 
 *
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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

static int foo;

int __attribute__((visibility("hidden"))) foofoo;

static	int											uninit_static;
static	int											init_static = 1;
		int __attribute__((visibility("hidden")))	uninit_hidden;
		int __attribute__((visibility("hidden")))	init_hidden = 1;
		int											uninit_global;
		int											init_global = 1;
extern	int											extern_global;
extern	int	 __attribute__((visibility("hidden")))	extern_hidden;

static	int											uninit_static_array[4];
static	int											init_static_array[4] = {1,2,3,4};
		int __attribute__((visibility("hidden")))	uninit_hidden_array[4];
		int __attribute__((visibility("hidden")))	init_hidden_array[4] = {1,2,3,4};
		int											uninit_global_array[4];
		int											init_global_array[4] = {1,2,3,4};
extern	int											extern_global_array[4];

int test1() { return uninit_static; }
int test2() { return init_static; }
int test3() { return uninit_hidden; }
int test4() { return init_hidden; }
int test5() { return uninit_global; }
int test6() { return init_global; }
int test7() { return extern_global; }
int test8() { return extern_hidden; }

int test_array1() { return uninit_static_array[2]; }
int test_array2() { return init_static_array[2]; }
int test_array3() { return uninit_hidden_array[2]; }
int test_array4() { return init_hidden_array[2]; }
int test_array5() { return uninit_global_array[2]; }
int test_array6() { return init_global_array[2]; }
int test_array7() { return extern_global_array[2]; }

static int foo2;
int test9() { return foo2; }


int* p_init_global = &init_global;
void* p_test1 = (void*)&test1;
unsigned char pad = 2;
unsigned char pad2 = 3;	 // this padding throws off alignment on compiler generated anonymous non-lazy pointers...

int func() __attribute__((visibility("hidden")));
int func() { return foo; }

int func2() { return func() + 1; }

