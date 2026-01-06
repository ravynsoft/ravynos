/*
 * Copyright (c) 2006-2008 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
#ifndef __FAT_UTIL_H__
#define __FAT_UTIL_H__

#include <mach-o/fat.h>

/*!
 * @define  MAGIC32
 * @abstract  Get the 32-bit magic number from a file data pointer.
 * @param ptr A pointer to a file whose magic number you want to get.
 * @result    Returns an unsigned 32-bit integer containing
 *            the first four bytes of the file data.
 */
#define MAGIC32(ptr)          (*((uint32_t *)(ptr)))

#define ISFAT(magic)          ((magic) == OSSwapHostToBigInt32(FAT_MAGIC))

/*!
 * @typedef fat_iterator
 * @abstract An opaque type for iterating architectures
 *           in a Mach-O or universal binary file.
 * @discussion A fat_iterator allows you to access every architecture in a
 *           Macho-O or universal binary file. Initialized with either kind,
 *           you use the fat_iterator_next_arch function to scan through each
 *           architecture (which will be one for a Mach-O file, and one or more
 *           for a universal binary file).
 */
typedef struct __fat_iterator * fat_iterator;


/*!
 * @function fat_iterator_open
 * @abstract Create an iterator for a Mach-O or universal binary file.
 * @discussion
 *        The fat_iterator_open function opens and maps the named file,
 *        creates a fat_iterator for it, and returns the iterator.
 *        When finished with the iterator, use fat_iterator_close and the
 *        file will be unmapped.
 * @param path The pathname of the file to open.
 * @param macho_only Pass true to require the file data be either fat or a
 *         thin Mach-O file or
 *         false to allow a non-fat file to contain any kind of data.
 * @result Returns a fat_iterator you can use to access the architectures
 *         in the named file.
 *         Returns NULL if the file can't be opened,
 *         if macho_only is true and the file is neither fat nor a Mach-O file,
 *         or if the iterator can't be created.
 */
fat_iterator fat_iterator_open(const char * path, int macho_only);

/*!
 * @function fat_iterator_for_data
 * @abstract Create an iterator for in-memory Mach-O or universal binary data.
 * @discussion
 *        The fat_iterator_for_data function creates and returns an iterator
 *        for in-memory Mach-O or universal binary data, which must be complete.
 *        The iterator does not own the data, so be sure not to free or unmap
 *        it before you close the iterator.
 * @param file_data A pointer to the start of the file data.
 * @param file_end A pointer to the end of the file data, which the iterator
 *        uses for bounds checking.
 * @param macho_only Pass true to require the file data be either fat or a
 *         thin Mach-O file or
 *         false to allow a non-fat file to contain any kind of data.
 * @result Returns a fat_iterator you can use to access the architectures
 *         in the file data.
 *         Returns NULL if the file can't be opened,
 *         if macho_only is true and the file is neither fat nor a Mach-O file,
 *         or if the iterator can't be created.
 */
fat_iterator fat_iterator_for_data(
    const void * file_data,
    const void * file_end,
    int macho_only);

/*!
 * @function fat_iterator_close
 * @abstract Close and free a fat_iterator.
 * @discussion
 *        The fat_iterator_close function unmaps a fat_iterator's file data
 *        if necessary and frees the iterator.
 * @param iter The fat_iterator to close.
 */
void fat_iterator_close(fat_iterator iter);

/*!
 * @function fat_iterator_num_arches
 * @abstract Return how many arches are in the data of a fat iterator.
 * @param iter The fat_iterator to check.
 * @result Returns the number of arches that will be iterated over, total.
 */
int fat_iterator_num_arches(
    fat_iterator iter);

/*!
 * @function fat_iterator_is_iterable
 * @abstract Return whether a fat_iterator can actually iterate.
 * @discussion
 *        The fat_iterator_is_iterable function returns true if it represents
 *        a fat file or a thin Mach-O file, false otherwise.
 * @param iter The fat_iterator to check.
 * @result Returns true if the iterator represents
 *        a fat file or a thin Mach-O file, false otherwise.
 */
int fat_iterator_is_iterable(
    fat_iterator iter);

/*!
 * @function fat_iterator_next_arch
 * @abstract Return the next architecture in a fat_iterator.
 * @discussion
 *        The fat_iterator_next_arch function returns a pointer to the start
 * of the data for the next architecture in the iterator's file, or NULL if the
 * architectures have been fully traversed (or if the iterator is not valid).
 * @param iter The fat_iterator to iterate.
 * @param file_end If provided, this indirect pointer is filled
 *        with the address of the end of the data for the architecture returned.
 *        Other macho utility functions use this for bounds checking.
 * @result Returns a pointer to the mach_header
 * struct for the next architecture in the iterator's file, or NULL if the
 * architectures have been fully traversed (or if the iterator is not valid).
 */
void * fat_iterator_next_arch(
    fat_iterator iter,
    void ** arch_end);

/*!
 * @function fat_iterator_reset
 * @abstract Rewinds a fat_iterator.
 * @discussion
 *        The fat_iterator_reset function sets the iterator to start from
 *        the beginning of the file so that you can iterate it again.
 * @param iter The fat_iterator to reset.
 */
void fat_iterator_reset(fat_iterator iter);

/*!
 * @function fat_iterator_find_fat_arch
 * @abstract Find the specified fat_arch entry in a fat_iterator,
 *           if present.
 * @discussion
 *        The fat_iterator_find_arch function fills in a provided
 * fat_arch struct for the specified architecture
 * in the iterator's file, or NULL if that architecture is not represented.
 * @param iter The fat_iterator to get the arch data from.
 * @param cputype The CPU type code requested (see <mach/machine.h> for a list).
 * @param cpusubtype The CPU subtype requested.
 *        Use CPU_SUBTYPE_MULTIPLE for a generic processor family request.
 * @param fat_arch A pointer to the fat_arch struct to fill in.
 *        All fields filled in are in host byte order.
 * @result Returns 1 if the
 * specified architecture is found in the iterator's file,
 * or 0 if the specified architecture is not represented.
 */
int fat_iterator_find_fat_arch(
    fat_iterator iter,
    cpu_type_t cputype,
    cpu_subtype_t cpusubtype,
    struct fat_arch * fat_arch);

/*!
 * @function fat_iterator_find_arch
 * @abstract Return the specified architecture in a fat_iterator, if present.
 * @discussion
 *        The fat_iterator_find_arch function returns a pointer
 * to the start of the data for the specified architecture
 * in the iterator's file, or NULL if that architecture is not represented.
 * @param iter The fat_iterator to get the arch data from.
 * @param cputype The CPU type code specified (see <mach/machine.h> for a list).
 * @param cpusubtype The CPU subtype specified.
 *        Use CPU_SUBTYPE_MULTIPLE for a generic processor family request.
 * @param file_end If provided, this indirect pointer is filled
 *        with the address of the end of the data for the architecture returned.
 *        Other macho utility functions use this for bounds checking.
 * @result Returns a pointer to the file data
 * for the specified architecture in the iterator's file,
 * or NULL if the specified architecture is not represented.
 */
void * fat_iterator_find_arch(
    fat_iterator iter,
    cpu_type_t cputype,
    cpu_subtype_t cpusubtype,
    void ** arch_end_ptr);

/*!
 * @function fat_iterator_find_host_arch
 * @abstract Return the host architecture in a fat_iterator, if present.
 * @discussion
 *        The fat_iterator_find_host_arch function returns a pointer
 * to the start of the data for the host architecture in the iterator's file,
 * or NULL if the host architecture is not represented.
 * @param iter The fat_iterator to get the host arch data from.
 * @param file_end If provided, this indirect pointer is filled
 *        with the address of the end of the data for the architecture returned.
 *        Other macho utility functions use this for bounds checking.
 * @result Returns a pointer to the file data
 * for the host architecture in the iterator's file,
 * or NULL if the host architecture is not represented.
 */
void * fat_iterator_find_host_arch(
    fat_iterator iter,
    void ** arch_end_ptr);

/*!
 * @function fat_iterator_file_start
 * @abstract Returns a pointer to the start of a fat_iterator's file.
 * @discussion
 *        The fat_iterator_reset returns a pointer to the start
 *        of a fat_iterator's file, so that you can access it directly.
 * @param iter The fat_iterator to get the file start for.
 * @result Returns a pointer to the beginning of the file accessed
 *         by the fat_iterator.
 */
const void * fat_iterator_file_start(fat_iterator iter);

/*!
 * @function fat_iterator_file_end
 * @abstract Returns a pointer to the end of a fat_iterator's file.
 * @discussion
 *        The fat_iterator_reset returns a pointer to the end
 *        of a fat_iterator's file, so that you can do bounds checking.
 * @param iter The fat_iterator to get the file end for.
 * @result Returns a pointer to the end of the file accessed
 *         by the fat_iterator.
 */
const void * fat_iterator_file_end(fat_iterator iter);


#endif /* __FAT_UTIL_H__ */
