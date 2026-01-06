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
#include "stuff/bool.h"

/*
 * These are the tokens for the "install name" for the next addresses to use
 * when updating the table.  And also the token for fixed regions.
 */
#define NEXT_FLAT_ADDRESS_TO_ASSIGN  "<<< Next flat address to assign >>>"
#define NEXT_SPLIT_ADDRESS_TO_ASSIGN "<<< Next split address to assign >>>"
#define NEXT_DEBUG_ADDRESS_TO_ASSIGN "<<< Next debug address to assign >>>"
#define FIXED_ADDRESS_AND_SIZE "<<< Fixed address and size not to assign >>>"

/*
 * The table of dynamic library install names and their addresses they are
 * linked at.  This is used with the -seg_addr_table option from the static
 * link editor, ld(1), and the seg_addr_table(1) program.
 */
struct seg_addr_table {
    char *install_name;
    enum bool split;
    uint32_t seg1addr;
    uint32_t segs_read_only_addr;
    uint32_t segs_read_write_addr;
    uint32_t line;
};

extern struct seg_addr_table *parse_default_seg_addr_table(
    char **seg_addr_table_name,
    uint32_t *table_size);

extern struct seg_addr_table * parse_seg_addr_table(
    char *file_name,
    char *flag,
    char *argument,
    uint32_t *table_size);

extern struct seg_addr_table * search_seg_addr_table(
    struct seg_addr_table *seg_addr_table,
    char *install_name);

extern void process_seg_addr_table(
    char *file_name,
    FILE *out_fp,
    char *comment_prefix,
    void (*processor)(struct seg_addr_table *entry, FILE *out_fp, void *cookie),
    void *cookie);
