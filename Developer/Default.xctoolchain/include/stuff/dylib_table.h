/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
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
 * The table of known dynamic library names and addresses they are linked at.
 * This is loaded from a -dylib_table option or from the default file:
 * ~rc/Data/DylibTable .
 */
struct dylib_table {
    uint32_t seg1addr;
    char *name;
};

extern struct dylib_table * parse_dylib_table(
    char *file_name,
    char *flag,
    char *argument);

extern struct dylib_table * parse_default_dylib_table(
    char **file_name);

extern struct dylib_table *search_dylib_table(
    struct dylib_table *dylib_table,
    char *name);

extern char * guess_dylib_install_name(
    char *name);
