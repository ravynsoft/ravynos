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
#define __darwin_i386_exception_state i386_exception_state
#define __darwin_i386_float_state i386_float_state
#define __darwin_i386_thread_state i386_thread_state

#include <stdarg.h>
#include "stuff/ofile.h"
#include "stuff/print.h"
#include "stuff/errors.h"

__private_extern__
void
archive_error(
struct ofile *ofile,
const char *format, ...)
{
    va_list ap;

	va_start(ap, format);
	if(ofile->file_type == OFILE_FAT){
	    print("%s: for architecture %s archive: %s ",
		  progname, ofile->arch_flag.name, ofile->file_name);
	}
	else{
	    print("%s: archive: %s ", progname, ofile->file_name);
	}
	vprint(format, ap);
        print("\n");
	va_end(ap);
	errors++;
}

__private_extern__
void
archive_member_error(
struct ofile *ofile,
const char *format, ...)
{
    va_list ap;

	va_start(ap, format);
	if(ofile->file_type == OFILE_FAT){
	    print("%s: for architecture %s archive member: %s(%.*s) ",
		  progname, ofile->arch_flag.name, ofile->file_name,
		  (int)ofile->member_name_size, ofile->member_name);
	}
	else{
	    print("%s: archive member: %s(%.*s) ", progname, ofile->file_name,
		  (int)ofile->member_name_size, ofile->member_name);
	}
	vprint(format, ap);
        print("\n");
	va_end(ap);
	errors++;
}

#ifndef OTOOL
__private_extern__
void
Mach_O_error(
struct ofile *ofile,
const char *format, ...)
{
    va_list ap;

	va_start(ap, format);
	if(ofile->file_type == OFILE_FAT){
	    if(ofile->arch_type == OFILE_ARCHIVE){
		print("%s: for architecture %s object: %s(%.*s) ", progname,
		      ofile->arch_flag.name, ofile->file_name,
		      (int)ofile->member_name_size, ofile->member_name);
	    }
	    else{
		print("%s: for architecture %s object: %s ", progname,
		      ofile->arch_flag.name, ofile->file_name);
	    }
	}
	else if(ofile->file_type == OFILE_ARCHIVE){
	    if(ofile->member_type == OFILE_FAT){
		print("%s: for object: %s(%.*s) architecture %s ", progname,
		      ofile->file_name, (int)ofile->member_name_size,
		      ofile->arch_flag.name, ofile->member_name);
	    }
	    else{
		print("%s: object: %s(%.*s) ", progname, ofile->file_name,
		      (int)ofile->member_name_size, ofile->member_name);
	    }
	}
	else{
	    print("%s: object: %s ", progname, ofile->file_name);
	}
	vprint(format, ap);
        print("\n");
	va_end(ap);
	errors++;
}
#endif /* !defined(OTOOL) */
