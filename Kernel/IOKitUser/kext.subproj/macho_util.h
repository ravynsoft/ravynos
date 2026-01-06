/*
 * Copyright (c) 2006-2008, 2012 Apple Inc. All rights reserved.
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
#ifndef __MACHO_UTIL_H__
#define __MACHO_UTIL_H__

#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <uuid/uuid.h>

/*******************************************************************************
********************************************************************************
**                   DO NOT USE THIS API. IT IS VOLATILE.                     **
********************************************************************************
*******************************************************************************/

/*! @header macho_util
Handy functions for working with Mach-O files. Very rudimentary, use at your
own risk.
*/

/*!
 * @define  MAGIC32
 * @abstract  Get the 32-bit magic number from a file data pointer.
 * @param ptr A pointer to a file whose magic number you want to get.
 * @result    Returns an unsigned 32-bit integer containing
 *            the first four bytes of the file data.
 */
#define MAGIC32(ptr)          (*((uint32_t *)(ptr)))

#define ISMACHO(magic)        (((magic) == MH_MAGIC) || \
                               ((magic) == MH_CIGAM) || \
                               ((magic) == MH_MAGIC_64) || \
                               ((magic) == MH_CIGAM_64))
#define ISSWAPPEDMACHO(magic)  (((magic) == MH_CIGAM) || \
                               ((magic) == MH_CIGAM_64))
#define ISMACHO64(magic)      (((magic) == MH_MAGIC_64) || \
                               ((magic) == MH_CIGAM_64))

/*******************************************************************************
*
*******************************************************************************/
#define CondSwapInt32(flag, value)  ((flag) ? OSSwapInt32((value)) : \
                                    (uint32_t)(value))
#define CondSwapInt64(flag, value)  ((flag) ? OSSwapInt64((value)) : \
                                    (uint64_t)(value))
/*!
 * @enum macho_seek_result
 * @abstract Results of a lookup within a Mach-O file.
 * @discussion This enum lists the possible return values for a Mach-O lookup.
 * @constant macho_seek_result_error Invalid arguments or bad Mach-O file.
 * @constant macho_seek_result_found The requested item was found.
 * @constant macho_seek_result_not_found The requested item was not found.
 *           For functions called repeatedly (such as the macho_lc_callback),
 *           this means the item may be found on a subsequent invocation.
 * @constant macho_seek_result_stop The requested item will not be found.
 *           This is only returned by functions that may be called repeatedly,
 *           when they can determine conclusively that the requested item does not exist.
 */
typedef enum {
    macho_seek_result_error          = -1,
    macho_seek_result_found          = 0,
    macho_seek_result_found_no_value = 1,
    macho_seek_result_not_found      = 2,
    macho_seek_result_stop           = 3,
} macho_seek_result;

/*!
 * @function macho_find_symbol
 * @abstract Finds symbol data in a mapped Mach-O file.
 * @discussion
 *        The macho_find_symbol function searches a memory-mapped Mach-O file
 *        for a given symbol,
 *        indirectly returning the address within that mapped file
 *        containing the data for that symbol.
 *        Only symbols of type N_SECT, N_UNDF result in an address;
 *        for N_ABS and N_INDR the result will be macho_seek_result_found_no_value.
 * @param file_start A pointer to the beginning of the mapped Mach-O file.
 * @param file_end A pointer to the end of the mapped Mach-O file.
 * @param name The name of the symbol to find.
 * @param nlist_type The address of a uint8_t that will be filled with the type
 *        of the located symbol.
 * @param symbol_address The address of a pointer that will be filled
 *        with the location of the symbol's data in the mapped file,
 *        if the address can be calculated.
 * @result Returns macho_seek_result_found if the symbol is found,
 * macho_seek_result_not_found if the symbol is not defined in the given file,
 * or macho_seek_result_error if an error occurs.
 */
macho_seek_result macho_find_symbol(
    const void          * file_start,
    const void          * file_end,
    const char          * name,
          uint8_t       * nlist_type,
    const void         ** symbol_address);

/*!
 * @function macho_find_symtab
 * @abstract Finds a mapped Mach-O file's symbol table.
 * @discussion
 *        The macho_find_symtab function locates the symbol table of a Mach-O
 *        file. Only the LC_SYMTAB load command is located, not the LC_DSYMTAB.
 * @param mach_header A pointer to the beginning of the mapped Mach-O file.
 * @param file_end A pointer to the end of the mapped Mach-O file.
 * @param symtab A pointer to the address of a symtab_command struct;
 *        if provided, this is filled with the address of the file's symbol table.
 * @result Returns macho_seek_result_found if the symbol table is found,
 * macho_seek_result_not_found if a symbol table is not defined in the given file,
 * or macho_seek_result_error if an error occurs.
 */
macho_seek_result macho_find_symtab(
    const void             * file_start,
    const void             * file_end,
    struct symtab_command ** symtab);

/*!
 * @function macho_find_dysymtab
 * @abstract Finds a mapped Mach-O file's dynamic link-edit symbol table info.
 * @discussion
 *        The macho_find_dysymtab function locates the dynamic link-edit symbol 
 *        table of a Mach-O file. Only the LC_DYSYMTAB load command is located, 
 *        not the LC_SYMTAB.
 * @param mach_header A pointer to the beginning of the mapped Mach-O file.
 * @param file_end A pointer to the end of the mapped Mach-O file.
 * @param dysymtab A pointer to the address of a dysymtab_command struct;
 *        if provided, this is filled with the address of the file's symbol table.
 * @result Returns macho_seek_result_found if the symbol table is found,
 * macho_seek_result_not_found if a symbol table is not defined in the given file,
 * or macho_seek_result_error if an error occurs.
 */
macho_seek_result macho_find_dysymtab(
                                      const void               * file_start,
                                      const void               * file_end,
                                      struct dysymtab_command ** dysymtab);

/*!
 * @function macho_find_uuid
 * @abstract Finds a mapped Mach-O file's UUID.
 * @discussion
 *        The macho_find_uuid function locates the UUID of a Mach-O file.
 * @param mach_header A pointer to the beginning of the mapped Mach-O file.
 * @param file_end A pointer to the end of the mapped Mach-O file.
 * @param uuid A pointer to the start of the UUID bytes;
 *        if provided, this is filled with the address of the UUID bytes.
 * @result Returns macho_seek_result_found if the UUID is found,
 * macho_seek_result_not_found if a UUID is not defined in the given file,
 * or macho_seek_result_error if an error occurs.
 */
macho_seek_result macho_find_uuid(
    const void * file_start,
    const void * file_end,
    char       **uuid);

/*!
 * @function macho_find_section_numbered
 * @abstract Finds an ordinal section in a Mach-O file.
 * @discussion
 *        The macho_find_section_numbered function locates an ordinally-numbered
 *        section within a Mach-O file, which is needed for calculating proper
 *        offsets and addresses of such things as nlist entries.
 *        Sections are numbered in a Mach-O file starting with 1, since zero
 *        is reserved to mean "no section".
 *        Since you will likely be passing a section number directly from other
 *        structure within the same Mach-O file, this should not be a problem.
 * @param mach_header A pointer to the beginning of the mapped Mach-O file.
 * @param file_end A pointer to the end of the mapped Mach-O file.
 * @param sect_num The ordinal number of the section to get, starting with 1.
 * @result Returns a pointer to the requested section struct if it exists;
 *         otherwise returns NULL.
 */
// cast to (struct section[_64] *)
void * macho_find_section_numbered(
    const void * file_start,
    const void * file_end,
    uint8_t sect_num);

/*!
 * @typedef macho_lc_callback
 * @abstract The callback function used when scanning Mach-O load commands
 *           with macho_scan_load_commands.
 * @discussion macho_lc_callback defines the callback function
 * invoked by macho_scan_load_commands for each load command it encounters.
 * The scan function passes a pointer to a user data struct that you can use
 * for search parameters, running information, and search results.
 * @param load_command A pointer to the Mach-O load command being processed.
 * @param file_end A pointer to the end of the mapped Mach-O file,
 *        to be used for bounds checking.
 * @param swap A boolean flag indicating whether the Mach-O file's byte order
 *        is opposite the host's.
 *        If nonzero, the callback needs to swap all multibyte values
 *        read from the Mach-O file.
 * @param user_data A pointer to user-defined data that is passed unaltered
 *        across invocations of the callback function.
 * @result Returns macho_seek_result_found if the requested item is found,
 * macho_seek_result_not_found if is not found on this call,
 * macho_seek_result_stop if the function determined that it does not exist,
 * or macho_seek_result_error if an error occurs
 * (particularly if any data structure to be accessed
 * would run past the end of the file).
 */
typedef macho_seek_result (*macho_lc_callback)(
    struct load_command * load_command,
    const void          * file_end,
    uint8_t               swap,
    void                * user_data
);

/*!
 * @function macho_find_source_version
 * @abstract Finds a mapped Mach-O file's LC_SOURCE_VERSION.
 * @discussion
 *        The macho_find_source_version function locates the source version
 *        load command of a Mach-O file.
 * @param mach_header A pointer to the beginning of the mapped Mach-O file.
 * @param file_end A pointer to the end of the mapped Mach-O file.
 * @param version A pointer to a uint64_t variable into which to store the
 *        contents of the 'version' field of the LC_SOURCE_VERSION load
 *        command from the mach-o header.
 * @result Returns macho_seek_result_found if the source version is found,
 * macho_seek_result_not_found if a source version is not defined in the
 * given file, or macho_seek_result_error if an error occurs.
 */
macho_seek_result macho_find_source_version(
                                            const void * file_start,
                                            const void * file_end,
                                            uint64_t   * version);

/*!
 * @function macho_scan_load_commands
 * @abstract Iterates over the load commands of a Mach-O file using a callback.
 * @discussion
 *        The macho_scan_load_commands iterates over the load commands within a
 *        Mach-O file, invoking a user-supplied callback until that callback
 *        returns a result indicating the scan should stop.
 * @param mach_header A pointer to the beginning of the mapped Mach-O file.
 * @param file_end A pointer to the end of the mapped Mach-O file.
 * @param lc_callback A function that is invoked on each load command
 *        of the Mach-O file until the callback function
 *        indicates the scan is finished.
 * @param user_data A pointer to user-defined data that is passed unaltered
 *        across invocations of the callback function.
 * @result Returns a pointer to the requested section struct if it exists;
 *         otherwise returns NULL.
 */
macho_seek_result macho_scan_load_commands(
    const void        * file_start,
    const void        * file_end,
    macho_lc_callback   lc_callback,
    void              * user_data);

boolean_t macho_swap(
    u_char            * file);

boolean_t macho_unswap(
    u_char            * file);

struct segment_command * macho_get_segment_by_name(
    struct mach_header    * mach_header,
    const char            * segname);

struct segment_command_64 * macho_get_segment_by_name_64(
    struct mach_header_64      * mach_header,
    const char                 * segname);

struct section * macho_get_section_by_name(
    struct mach_header    * mach_header,
    const char            * segname,
    const char            * sectname);

struct section_64 * macho_get_section_by_name_64(
    struct mach_header_64     * mach_header,
    const char                * segname,
    const char                * sectname);

boolean_t macho_remove_linkedit(
    u_char    * macho,
    u_long    * linkedit_size);

boolean_t macho_trim_linkedit(
    u_char    *macho,
    u_long    *amount_trimmed);

#endif /* __MACHO_UTIL_H__ */
