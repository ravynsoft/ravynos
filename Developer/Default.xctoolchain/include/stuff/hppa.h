/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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
#if defined(__MWERKS__) && !defined(__private_extern__)
#define __private_extern__ __declspec(private_extern)
#endif

__private_extern__ void calc_hppa_HILO(
    uint32_t base,
    uint32_t offset,
    uint32_t *left21,
    uint32_t *right14);

__private_extern__ uint32_t assemble_17(
    uint32_t x,
    uint32_t y,
    uint32_t z);

__private_extern__ uint32_t assemble_21(
    uint32_t x);

__private_extern__ uint32_t assemble_12(
    uint32_t x,
    uint32_t y);

__private_extern__ uint32_t assemble_3(
    uint32_t x);

__private_extern__ uint32_t sign_ext(
    uint32_t x,
    uint32_t len);

__private_extern__ uint32_t low_sign_ext(
    uint32_t x,
    uint32_t len);

__private_extern__ uint32_t dis_assemble_21(
    uint32_t as21);

__private_extern__ uint32_t low_sign_unext(
    uint32_t x,
    uint32_t len);

__private_extern__ void dis_assemble_17(
    uint32_t as17,
    uint32_t *x,
    uint32_t *y,
    uint32_t *z);

__private_extern__ uint32_t sign_unext(
    uint32_t x,
    uint32_t len);

__private_extern__ uint32_t dis_assemble_3(
    uint32_t x);

__private_extern__ void dis_assemble_12(
    uint32_t as12,
    uint32_t *x,
    uint32_t *y);
