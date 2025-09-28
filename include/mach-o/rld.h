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
/*
 * This is the interface to the rld package as described in rld(3).
 */

#ifndef _MACHO_RLD_H_
#define _MACHO_RLD_H_

#include <streams/streams.h>
#include <mach-o/loader.h>

extern long rld_load(
    NXStream *stream,
    struct mach_header **header_addr,
    const char * const *object_filenames,
    const char *output_filename);

extern long rld_load_from_memory(
    NXStream *stream,
    struct mach_header **header_addr,
    const char *object_name,
    char *object_addr,
    long object_size,
    const char *output_filename);

extern long rld_unload(
    NXStream *stream);

extern long rld_lookup(
    NXStream *stream,
    const char *symbol_name,
    unsigned long *value);

extern long rld_forget_symbol(
    NXStream *stream,
    const char *symbol_name);

extern long rld_unload_all(
    NXStream *stream,
    long deallocate_sets);

extern long rld_load_basefile(
    NXStream *stream,
    const char *base_filename);

extern void rld_address_func(
    unsigned long (*func)(unsigned long size, unsigned long headers_size));

extern long rld_write_symfile(
    NXStream *stream,
    const char *output_filename);
#endif /* _MACHO_RLD_H_ */
