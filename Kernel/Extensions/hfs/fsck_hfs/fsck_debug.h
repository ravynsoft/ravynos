/*
 * Copyright (c) 2005-2006, 2008, 2010-2011 Apple Inc. All rights reserved.
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
#ifndef __FSCK_DEBUG__
#define __FSCK_DEBUG__

#include <sys/types.h>

enum debug_message_type {
	/* Type of information */
	d_info		=	0x0001,	/* Normal information messages during execution */
	d_error		= 	0x0002,	/* Error messages */

	/* Category of verify/repair operation */
	d_xattr		=	0x0010,	/* Extended attributes related messages */
	d_overlap	=	0x0020,	/* Overlap extents related messages */
	d_trim		=	0x0040,	/* TRIM (discard/unmap) related messages */
	
	d_dump_record = 0x0400,	/* Dump corrupt keys and records */
	d_dump_node	=	0x0800,	/* In hfs_swap_BTNode or BTCheck, dump out damaged nodes */
	d_check_slink	=	0x1000,	/* Read the contents of a symlink and check length */
};

/* Current debug level of fsck_hfs for printing messages via DPRINTF */
extern unsigned long cur_debug_level;

/* Function: DPRINTF
 *
 * Description: Debug function similar to printf except the first parameter
 * which indicates the type of message to be printed by DPRINTF. Based on 
 * current debug level and the type of message, the function decides 
 * whether to print the message or not.
 *
 * Each unique message type has a bit assigned to it.  The message type 
 * passed to DPRINTF can be one or combination (OR-ed value) of pre-defined 
 * debug message types.  Only the messages whose type have one or more similar
 * bits set in comparison with current global debug level are printed. 
 *
 * For example, if cur_debug_level = 0x11 (d_info|d_xattr)
 * ----------------------------------------
 *	message type	- 	printed/not printed
 * ----------------------------------------
 *	d_info			-	printed
 *	d_error|d_xattr	-	printed
 *	d_error			- 	not printed
 *	d_overlap		- 	not printed
 *
 * Input:
 *	message_type - type of message, to determine when to print the message
 *	variable arguments - similar to printfs
 *
 * Output:
 *	Nothing
 */
extern void DPRINTF (unsigned long message_type, char *format, ...);

void HexDump(const void *p_arg, unsigned length, int showOffsets);

#endif /* __FSCK_DEBUG__ */
