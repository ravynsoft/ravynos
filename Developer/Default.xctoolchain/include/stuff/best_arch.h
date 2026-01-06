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

#include <mach/machine.h>
#include <stuff/bool.h>

/*
 * cpusubtype_findbestarch_64() is passed a cputype and cpusubtype and a set of
 * fat_arch_64 structs and selects the best one that matches (if any) and
 * returns a pointer to that fat_arch_64 struct (or NULL).  The fat_arch_64
 * structs must be in the host byte sex and correct such that the fat_archs64
 * really points to enough memory for nfat_arch_64 structs.  It is possible
 * that this routine could fail if new cputypes or cpusubtypes are added and an
 * old version of this routine is used.  But if there is an exact match between
 * the cputype and cpusubtype and one of the fat_arch_64 structs this routine
 * will always succeed.
 */
__private_extern__ struct fat_arch_64 * cpusubtype_findbestarch_64(
    cpu_type_t cputype,
    cpu_subtype_t cpusubtype,
    struct fat_arch_64 *fat_archs64,
    uint32_t nfat_archs);

/*
 * cpusubtype_findbestarch() is passed a cputype and cpusubtype and a set of
 * fat_arch structs and selects the best one that matches (if any) and returns
 * a pointer to that fat_arch struct (or NULL).  The fat_arch structs must be
 * in the host byte sex and correct such that the fat_archs really points to
 * enough memory for nfat_arch structs.  It is possible that this routine could
 * fail if new cputypes or cpusubtypes are added and an old version of this
 * routine is used.  But if there is an exact match between the cputype and
 * cpusubtype and one of the fat_arch structs this routine will always succeed.
 */
__private_extern__ struct fat_arch * cpusubtype_findbestarch(
    cpu_type_t cputype,
    cpu_subtype_t cpusubtype,
    struct fat_arch *fat_archs,
    uint32_t nfat_archs);

/*
 * cpusubtype_combine() returns the resulting cpusubtype when combining two
 * differnet cpusubtypes for the specified cputype.  If the two cpusubtypes
 * can't be combined (the specific subtypes are mutually exclusive) -1 is
 * returned indicating it is an error to combine them.  This can also fail and
 * return -1 if new cputypes or cpusubtypes are added and an old version of
 * this routine is used.  But if the cpusubtypes are the same they can always
 * be combined and this routine will return the cpusubtype pass in.
 */
__private_extern__ cpu_subtype_t cpusubtype_combine(
    cpu_type_t cputype,
    cpu_subtype_t cpusubtype1,
    cpu_subtype_t cpusubtype2);

/*
 * cpusubtype_execute() returns TRUE if the exec_cpusubtype can be used for
 * execution on the host_cpusubtype for the specified cputype.  If the
 * exec_cpusubtype can't be run on the host_cpusubtype FALSE is returned
 * indicating it is an error to combine them.  This can also return FALSE and
 * if new cputypes or cpusubtypes are added and an old version of this routine
 * is used.  But if the cpusubtypes are the same they can always be executed
 * and this routine will return TRUE.
 */
__private_extern__ enum bool cpusubtype_execute(
    cpu_type_t host_cputype,
    cpu_subtype_t host_cpusubtype, /* can NOT be the ALL type */
    cpu_subtype_t exec_cpusubtype);/* can be the ALL type */
