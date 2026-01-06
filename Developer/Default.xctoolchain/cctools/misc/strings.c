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
/*	$OpenBSD: strings.c,v 1.2 1996/06/26 05:39:30 deraadt Exp $	*/
/*	$NetBSD: strings.c,v 1.7 1995/02/15 15:49:19 jtc Exp $	*/

/*
 * Copyright (c) 1980, 1987, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * The NeXT Computer, Inc. strings(1) program that handles fat files, archives
 * and Mach-O objects files (no BSD a.out files).  Some lines of code were
 * taken and adapted from the BSD release.
 *
 *		CHANGES FROM THE BSD VERSION OF strings(1):
 * Object files are no longer recognized as objects if read from standard input
 * but must be a command line argument to be treated as an object file.  The
 * result is if an object is read from standard input the entire file is
 * searched for strings not just the appropate sections.  With the handling of
 * fat files any file that contains objects has it contents processed as objects
 * which includes the objects in archives which the 4.3bsd strings(1) did not
 * process as object files.  Object files may be of the form "libx.a(x.o)"
 * which refer to an archive member.
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "stuff/bool.h"
#include "stuff/ofile.h"
#include "stuff/errors.h"
#include "stuff/allocate.h"

char *progname = NULL;

struct flags {
    enum bool treat_as_data;
    enum bool print_offsets;
    char *offset_format;
    enum bool all_sections;
    uint32_t minimum_length;
};

static void usage(
    void);
static void ofile_processor(
    struct ofile *ofile,
    char *arch_name,
    void *cookie);
static void ofile_find(
    char *addr,
    uint64_t size,
    uint64_t offset,
    struct flags *flags);
static void find(
    uint32_t cnt,
    struct flags *flags);
static enum bool dirt(
    int c);

/* apple_version is created by the libstuff/Makefile */
extern char apple_version[];
char *version = apple_version;

int
main(
int argc,
char **argv,
char **envp)
{
    struct flags flags;
    int i;
    uint32_t j, nfiles;
    char *endp;
    struct arch_flag *arch_flags;
    uint32_t narch_flags;
    enum bool all_archs, rest_args_files, use_member_syntax;
    struct stat stat_buf;

	progname = argv[0];

	nfiles = 0;
	arch_flags = NULL;
	narch_flags = 0;
	all_archs = FALSE;

	flags.treat_as_data = FALSE;
	flags.print_offsets = FALSE;
	flags.offset_format = NULL;
	flags.all_sections = FALSE;
	flags.minimum_length = 4;

	rest_args_files = FALSE;
	for(i = 1; i < argc; i++){
	    if(rest_args_files == FALSE && argv[i][0] == '-'){
		if(argv[i][1] == '\0')
		    flags.treat_as_data = TRUE;
		else if(strcmp(argv[i], "--") == 0)
		    rest_args_files = TRUE;
		else if(strcmp(argv[i], "-arch") == 0){
		    if(i + 1 == argc){
			error("missing argument(s) to %s option", argv[i]);
			usage();
		    }
		    if(strcmp("all", argv[i+1]) == 0){
			all_archs = TRUE;
		    }
		    else{
			arch_flags = reallocate(arch_flags,
				(narch_flags + 1) * sizeof(struct arch_flag));
			if(get_arch_from_flag(argv[i+1],
					      arch_flags + narch_flags) == 0){
			    error("unknown architecture specification flag: "
				  "%s %s", argv[i], argv[i+1]);
			    arch_usage();
			    usage();
			}
			narch_flags++;
		    }
		    i++;
		}
		else if(strcmp(argv[i], "-n") == 0){
		    if(i + 1 == argc){
			error("missing argument to %s option", argv[i]);
			usage();
		    }
		    flags.minimum_length =
			(uint32_t)strtoul(argv[i+1], &endp, 10);
		    if(*endp != '\0'){
			error("invalid decimal number in option: %s %s",
			      argv[i], argv[i+1]);
			usage();
		    }
		    i++;
		}
		else if(strcmp(argv[i], "-t") == 0){
		    if(i + 1 == argc){
			error("missing argument to %s option", argv[i]);
			usage();
		    }
		    if(argv[i+1][1] != '\0'){
			error("invalid argument to option: %s %s",
			      argv[i], argv[i+1]);
			usage();
		    }
		    switch(argv[i+1][0]){
		    case 'd':
			flags.print_offsets = TRUE;
			flags.offset_format = "%d";
			break;
		    case 'o':
			flags.print_offsets = TRUE;
			flags.offset_format = "%o";
			break;
		    case 'x':
			flags.print_offsets = TRUE;
			flags.offset_format = "%x";
			break;
		    default:
			error("invalid argument to option: %s %s",
			      argv[i], argv[i+1]);
			usage();
		    }
		    i++;
		}
		else{
		    endp = NULL;
		    for(j = 1; argv[i][j] != '\0' && endp == NULL; j++){
			switch(argv[i][j]){
			case 'o':
			    flags.print_offsets = TRUE;
			    flags.offset_format = "%7lu";
			    break;
			case 'a':
			    flags.all_sections = TRUE;
			    break;
			default:
			    if(!isdigit(argv[i][j])){
				error("unknown flag: %s", argv[i]);
				usage();
			    }
			    flags.minimum_length =
				(uint32_t)strtoul(argv[i] + j, &endp, 10);
			    if(*endp != '\0'){
				error("invalid decimal number in flag: %s",
				argv[i]);
				usage();
			    }
			}
		    }
		}
	    }
	    else{
		nfiles++;
	    }
	}

	/*
	 * Process the file or stdin if there are no files.
	 */
	rest_args_files = FALSE;
	if(nfiles != 0){
	    for(i = 1; i < argc; i++){
		if(argv[i][0] != '-' || rest_args_files == TRUE){
		    if(flags.treat_as_data == TRUE){
			if(freopen(argv[i], "r", stdin) == NULL)
			    system_error("can't open: %s", argv[i]);
			rewind(stdin);
			find(UINT_MAX, &flags);
		    }
		    else{
			/*
			 * If there's a filename that's an exact match then use
			 * that, else fall back to the member syntax.
			 */
			if(stat(argv[i], &stat_buf) == 0)
			    use_member_syntax = FALSE;
			else
			    use_member_syntax = TRUE;
			ofile_process(argv[i], arch_flags, narch_flags,
				      all_archs, TRUE, TRUE, use_member_syntax,
				      ofile_processor,&flags);
		    }
		}
		else if(strcmp(argv[i], "-arch") == 0 ||
			strcmp(argv[i], "-n") == 0 ||
			strcmp(argv[i], "-t") == 0)
		    i++;
		else if(strcmp(argv[i], "--") == 0)
		    rest_args_files = TRUE;
	    }
	}
	else{
	    find(UINT_MAX, &flags);
	}
	if(errors == 0)
	    return(EXIT_SUCCESS);
	else
	    return(EXIT_FAILURE);
}

/*
 * usage() prints the current usage message and exits indicating failure.
 */
static
void
usage(
void)
{
	fprintf(stderr, "Usage: %s [-] [-a] [-o] [-t format] [-number] "
		"[-n number] [[-arch <arch_flag>] ...] [--] [file ...]\n",
		progname);
	exit(EXIT_FAILURE);
}

/*
 * ofile_processor() is called by ofile_process() for each ofile to process.
 * All ofiles that are object files are process by section non-object files
 * have their logical contents processed entirely.  The locical contents may
 * be a single archive member in an archive, if the name "libx.a(x.o)" was
 * used or a specific architecture if "-arch <arch_flag> fatfile" was used
 * which is not the entire physical file.  If the entire physical file is
 * wanted to be searched then the "-" option is used and this routine is not
 * used.
 */
static
void
ofile_processor(
struct ofile *ofile,
char *arch_name,
void *cookie)
{
    char *addr;
    uint64_t offset, size;
    uint32_t i, j;
    uint32_t ncmds;
    struct flags *flags;
    struct load_command *lc;
    struct segment_command *sg;
    struct segment_command_64 *sg64;
    struct section *s;
    struct section_64 *s64;

	flags = (struct flags *)cookie;

	/*
	 * If the ofile is not an object file then process it without reguard
	 * to sections.
	 */
	if(ofile->object_addr == NULL
#ifdef LTO_SUPPORT
           || ofile->member_type == OFILE_LLVM_BITCODE
#endif /* LTO_SUPPORT */
           ){
	    if(ofile->file_type == OFILE_FAT && ofile->arch_flag.cputype != 0){
		if(ofile->fat_header->magic == FAT_MAGIC_64){
		    addr = ofile->file_addr +
			   ofile->fat_archs64[ofile->narch].offset;
		    size = ofile->fat_archs64[ofile->narch].size;
		    offset = ofile->fat_archs64[ofile->narch].offset;
		}
		else{
		    addr = ofile->file_addr +
			   ofile->fat_archs[ofile->narch].offset;
		    size = ofile->fat_archs[ofile->narch].size;
		    offset = ofile->fat_archs[ofile->narch].offset;
		}
	    }
	    else{
		addr = ofile->file_addr;
		size = ofile->file_size;
		offset = 0;
	    }
	    if(ofile->member_ar_hdr != NULL) {
		addr = addr + ofile->member_offset;
		size = strtoul(ofile->member_ar_hdr->ar_size, NULL, 10);
		offset = offset + ofile->member_offset;
	    }
	    if(offset >= ofile->file_size)
		size = 0;
	    else if(offset + size > ofile->file_size)
		size = ofile->file_size - offset;
	    ofile_find(addr, size, offset, flags);
	    return;
	}

	/*
	 * The ofile is an object file so process with reguard to it's sections.
	 */
	lc = ofile->load_commands;
	if(ofile->mh != NULL)
	    ncmds = ofile->mh->ncmds;
	else
	    ncmds = ofile->mh64->ncmds;
	for(i = 0; i < ncmds; i++){
	    if(lc->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc;
		s = (struct section *)((char *)sg +
			sizeof(struct segment_command));
		for(j = 0; j < sg->nsects; j++){
		    if(flags->all_sections){
			if((s->flags & S_ZEROFILL) != S_ZEROFILL &&
			   (s->flags & S_THREAD_LOCAL_ZEROFILL) !=
				       S_THREAD_LOCAL_ZEROFILL){
			    addr = ofile->object_addr + s->offset;
			    offset = s->offset;
			    size = s->size;
			    if(offset >= ofile->file_size)
				size = 0;
			    else if(offset + size > ofile->file_size)
				size = ofile->file_size - offset;
			    ofile_find(addr, size, offset, flags);
			}
		    }
		    else{
			if((s->flags & S_ZEROFILL) != S_ZEROFILL &&
			   (s->flags & S_THREAD_LOCAL_ZEROFILL) !=
				       S_THREAD_LOCAL_ZEROFILL &&
			   (strcmp(s->sectname, SECT_TEXT) != 0 ||
			    strcmp(s->segname, SEG_TEXT) != 0)){
			    addr = ofile->object_addr + s->offset;
			    offset = s->offset;
			    size = s->size;
			    if(offset >= ofile->file_size)
				size = 0;
			    else if(offset + size > ofile->file_size)
				size = ofile->file_size - offset;
			    ofile_find(addr, size, offset, flags);
			}
		    }
		    s++;
		}
	    }
	    else if(lc->cmd == LC_SEGMENT_64){
		sg64 = (struct segment_command_64 *)lc;
		s64 = (struct section_64 *)((char *)sg64 +
			sizeof(struct segment_command_64));
		for(j = 0; j < sg64->nsects; j++){
		    if(flags->all_sections){
			if((s64->flags & S_ZEROFILL) != S_ZEROFILL &&
			   (s64->flags & S_THREAD_LOCAL_ZEROFILL) != 
				         S_THREAD_LOCAL_ZEROFILL){
			    addr = ofile->object_addr + s64->offset;
			    offset = s64->offset;
			    size = s64->size;
			    if(offset >= ofile->file_size)
				size = 0;
			    else if(offset + size > ofile->file_size)
				size = ofile->file_size - offset;
			    ofile_find(addr, size, offset, flags);
			}
		    }
		    else{
			if((s64->flags & S_ZEROFILL) != S_ZEROFILL &&
			   (s64->flags & S_THREAD_LOCAL_ZEROFILL) !=
					 S_THREAD_LOCAL_ZEROFILL &&
			   (strcmp(s64->sectname, SECT_TEXT) != 0 ||
			    strcmp(s64->segname, SEG_TEXT) != 0)){
			    addr = ofile->object_addr + s64->offset;
			    offset = s64->offset;
			    size = s64->size;
			    if(offset >= ofile->file_size)
				size = 0;
			    else if(offset + size > ofile->file_size)
				size = ofile->file_size - offset;
			    ofile_find(addr, size, offset, flags);
			}
		    }
		    s64++;
		}
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
}

/*
 * ofile_find is used by ofile_processor() to find strings in part of a ofile
 * that is memory at addr for size.  offset is the offset in the file to this
 * data for use when printing offsets.
 */
static
void
ofile_find(
char *addr,
uint64_t size,
uint64_t offset,
struct flags *flags)
{
    uint64_t i, string_length;
    char c, *string;

	string = addr;
	string_length = 0;
	for(i = 0; i < size; i++){
	    c = addr[i];
	    if(c == '\n' || dirt(c) || i == size - 1){
		if(string_length >= flags->minimum_length){
		    if(flags->print_offsets){
			printf(flags->offset_format, offset + (string - addr));
			printf(" ");
		    }
		    if(i == size - 1 && c != '\n')
			printf("%.*s\n", (int)string_length + 1, string);
		    else
			printf("%.*s\n", (int)string_length, string);
		}
		string = addr + i + 1;
		string_length = 0;
	    }
	    else{
		string_length++;
	    }
	}
}

/*
 * find() is the original 4.3bsd code that uses the stdin stream.  It searches
 * for strings through a count of cnt bytes.
 */
static
void
find(
uint32_t cnt,
struct flags *flags)
{
    static char buf[BUFSIZ];
    register char *cp;
    register int c, cc, i;

	/* <rdar://problem/54055310> Unix Conformance 2019 */
	setlocale(LC_ALL, "");

	cp = buf;
	cc = 0;
	for (i = 0; i < cnt; ++i) {
		c = getc(stdin);
		if (c == '\n' || !isprint(c) || (i + 1) == cnt) {
			if (cp > buf && cp[-1] == '\n')
				--cp;
			*cp++ = 0;
			if (cp > &buf[flags->minimum_length]) {
				if (flags->print_offsets == TRUE){
					printf(flags->offset_format,
					       i - cc);
					printf(" ");
				}
				printf("%s\n", buf);
			}
			cp = buf;
			cc = 0;
		} else {
			if (cp < &buf[sizeof buf - 2])
				*cp++ = c;
			cc++;
		}
		if (ferror(stdin) || feof(stdin))
			break;
	}
}

/*
 * dirt() is the original 4.3bsd code that returns TRUE or FALSE if the
 * character passed to it could be in a part of a printable string.
 */
static
enum bool
dirt(
int c)
{
	switch(c){
	    case '\n':
	    case '\f':
		return(FALSE);
	    case 0177:
		return(TRUE);
	    default:
		if(c > 0200 || c < ' ')
		    return(TRUE);
		else
		    return(FALSE);
	}
}
