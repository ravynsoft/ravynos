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
#ifndef _STUFF_ARCH_H_
#define _STUFF_ARCH_H_

#if defined(__MWERKS__) && !defined(__private_extern__)
#define __private_extern__ __declspec(private_extern)
#endif
/*
 * This file contains the current known set of flags and constants for the
 * known architectures.
 */
#include <mach/machine.h>
#include <stuff/bytesex.h>

/*
 * The structure describing an architecture flag with the string of the flag
 * name, and the cputype and cpusubtype.
 */
struct arch_flag {
    char *name;
    cpu_type_t cputype;
    cpu_subtype_t cpusubtype;
};

/*
 * get_arch_from_flag() is passed a name of an architecture flag and returns
 * zero if that flag is not known and non-zero if the flag is known.
 * If the pointer to the arch_flag is not NULL it is filled in with the
 * arch_flag struct that matches the name.
 */
__private_extern__ int get_arch_from_flag(
    char *name,
    struct arch_flag *arch_flag);

/*
 * get_arch_from_host() gets the architecture from the host this is running on
 * and returns zero if the architecture is not known and zero if the
 * architecture is known.  If the parameters family_arch_flag and
 * specific_arch_flag are not NULL they get fill in with the family
 * architecture and specific architecure for the host.  If the architecture
 * is unknown and the parameters are not NULL then all fields are set to zero.
 */
__private_extern__ int get_arch_from_host(
    struct arch_flag *family_arch_flag,
    struct arch_flag *specific_arch_flag);

/*
 * get_arch_flags() returns a pointer to an array of all currently know
 * architecture flags (terminated with an entry with all zeros).
 */
__private_extern__ const struct arch_flag *get_arch_flags(
    void);

/*
 * arch_usage() is called when an unknown architecture flag is encountered.
 * It prints the currently know architecture flags on stderr.
 */
__private_extern__ void arch_usage(
    void);

/*
 * set_arch_flag_name() sets the name field of the specified arch_flag to
 * match it's cputype and cpusubtype.  The string is allocated via malloc by
 * the routines in "allocate.h" and errors are handled by the routines in
 * "error.h".
 */
__private_extern__ void set_arch_flag_name(
    struct arch_flag *p);

/*
 * get_arch_name_from_types() returns the name of the architecture for the
 * specified cputype and cpusubtype if known.  If unknown it returns a pointer
 * to the string "unknown".
 */
__private_extern__ const char *get_arch_name_from_types(
    cpu_type_t cputype,
    cpu_subtype_t cpusubtype);

/*
 * get_arch_name_if_known() returns the name of the architecture for the
 * specified cputype and cpusubtype if known.  If unknown it returns NULL.
 */
__private_extern__ const char *get_arch_name_if_known(
    cpu_type_t cputype,
    cpu_subtype_t cpusubtype);

/*
 * get_arch_family_from_cputype() returns the family architecture for the
 * specified cputype if known.  If unknown it returns NULL.
 */
__private_extern__ const struct arch_flag *get_arch_family_from_cputype(
    cpu_type_t cputype);

/*
 * get_byte_sex_from_flag() returns the byte sex of the architecture for the
 * specified cputype and cpusubtype if known.  If unknown it will abort().
 * If the bytesex can be determined directly as in the case of reading a magic
 * number from a file that should be done and this routine should not be used
 * as it could be out of date.
 */
__private_extern__ enum byte_sex get_byte_sex_from_flag(
    const struct arch_flag *flag);

/*
 * get_stack_addr_from_flag() returns the default starting address of the user
 * stack. If unknown it will abort().
 */
__private_extern__ uint64_t get_stack_addr_from_flag(
    const struct arch_flag *flag);

/*
 * get_segalign_from_flag() returns the default segment alignment (page size).
 * If unknown it will abort().
 */
__private_extern__ uint32_t get_segalign_from_flag(
    const struct arch_flag *flag);

/*
 * get_shared_region_size_from_flag() returns the size of the read only shared
 * region.
 */
__private_extern__ uint32_t get_shared_region_size_from_flag(
    const struct arch_flag *flag);

/*
 * force_cpusubtype_ALL_for_cputype() takes a cputype and returns TRUE if for
 * that cputype the cpusubtype should always be forced to the ALL cpusubtype,
 * otherwise it returns FALSE.
 */
__private_extern__ enum bool force_cpusubtype_ALL_for_cputype(
    cpu_type_t cputype);
#endif /* _STUFF_ARCH_H_ */
