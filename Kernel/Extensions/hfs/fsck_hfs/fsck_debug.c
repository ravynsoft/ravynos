/*
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
#include <stdio.h>
#include <stdarg.h>
#include "fsck_debug.h"
#include "fsck_hfs.h"

/* Current debug level of fsck_hfs for printing messages via DPRINTF */
unsigned long cur_debug_level;

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
void DPRINTF (unsigned long type, char *fmt, ...) 
{
	if (cur_debug_level & type) {
		va_list ap;

		plog ("\t");
		va_start(ap, fmt);
		vplog(fmt, ap);
		va_end(ap);
	}
}

void HexDump(const void *p_arg, unsigned length, int showOffsets)
{
	const u_int8_t *p = p_arg;
	unsigned i;
	char ascii[17];
	u_int8_t byte;
	
	ascii[16] = '\0';
	
	for (i=0; i<length; ++i)
	{
		if (showOffsets && (i & 0xF) == 0)
			plog("%08X: ", i);

		byte = p[i];
		plog("%02X ", byte);
		if (byte < 32 || byte > 126)
			ascii[i & 0xF] = '.';
		else
			ascii[i & 0xF] = byte;
		
		if ((i & 0xF) == 0xF)
		{
			plog("  %s\n", ascii);
		}
	}
	
	if (i & 0xF)
	{
		unsigned j;
		for (j = i & 0xF; j < 16; ++j)
			plog("   ");
		ascii[i & 0xF] = 0;
		plog("  %s\n", ascii);
	}
}
