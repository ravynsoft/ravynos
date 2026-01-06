/*
 * Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved.
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
/*	$OpenBSD: nm.c,v 1.4 1997/01/15 23:42:59 millert Exp $	*/
/*	$NetBSD: nm.c,v 1.7 1996/01/14 23:04:03 pk Exp $	*/

/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Hans Huebner.
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
 * The NeXT Computer, Inc. nm(1) program that handles fat files, archives and
 * Mach-O objects files (no BSD a.out files).  A few lines of code were taken
 * and adapted from the BSD release.
 *
 * When processing multiple files which are archives the BSD version of nm
 * would only print the archive member name (without the -o option) of the
 * object files before printing the symbols.  This version of nm will print the
 * archive name with the member name in ()'s in this case which makes it clear
 * which symbols belong to which arguments in the case that multiple arguments
 * are archives and have members of the same name.
 *
 * To allow the "-arch <arch_flag>" command line argument the processing of
 * command line arguments was changed to allow the options to be specified
 * in more than one group of arguments with each group preceded by a '-'.  This
 * change in behavior would only be noticed if a command line of the form
 * "nm -n -Q" was used where the BSD version would open a file of the name
 * "-Q" to process.  To do this with this version of nm the new command line
 * argument "-" would be used to treat all remaining arguments as file names.
 * So the equivalent command would be "nm -n - -Q".  This should not be a
 * problem as the BSD would treat the command "nm -Q" by saying "-Q" is an
 * invalid argument which was slightly inconsistant.
 */
#include <mach/mach.h> /* first so to get rid of a precomp warning */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> /* cctools-port: For bcmp, bzero ... */
#include <ctype.h>
#include <libc.h>
#include <dlfcn.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/stab.h>
#include "stuff/bool.h"
#include "stuff/ofile.h"
#include "stuff/errors.h"
#include "stuff/allocate.h"
#include "stuff/guess_short_name.h"
#include "stuff/write64.h"
#ifdef LTO_SUPPORT
#include "stuff/lto.h"
#include <xar/xar.h>
#endif /* LTO_SUPPORT */
#include <mach-o/dyld.h>

/* used by error routines as the name of the program */
char *progname = NULL;

/* flags set from the command line arguments */
struct cmd_flags {
    uint32_t nfiles;
    enum bool a;	/* print all symbol table entries including stabs */
    enum bool g;	/* print only global symbols */
    enum bool n;	/* sort numericly rather than alphabetically */
    enum bool o;	/* prepend file or archive element name to each line */
    enum bool p;	/* don't sort; print in symbol table order */
    enum bool r;	/* sort in reverse direction */
    enum bool u;	/* print only undefined symbols */
    enum bool U;	/* only undefined symbols */
    enum bool m;	/* print symbol in Mach-O symbol format */
    enum bool x;	/* print the symbol table entry in hex and the name */
    enum bool j;	/* just print the symbol name (no value or type) */
    enum bool s;	/* print only symbol in the following section */
    char *segname,	/*  segment name for -s */
	 *sectname;	/*  section name for -s */
    enum bool l;	/* print a .section_start symbol if none exists (-s) */
    enum bool f;	/* print a dynamic shared library flat */
    enum bool v;	/* sort and print by value diffences ,used with -n -s */
    enum bool b;	/* print only stabs for the following include */
    char *bincl_name;	/*  the begin include name for -b */
    enum bool i;	/* start searching for begin include at -iN index */
    uint32_t index;	/*  the index to start searching at */
    enum bool A;	/* pathname or library name of an object on each line */
    enum bool P;	/* portable output format */
    char *format;	/* the -t format */
#ifdef LTO_SUPPORT
    enum bool L;	/* print the symbols from (__LLVM,__bundle) section */
#endif /* LTO_SUPPORT */
};
/* These need to be static because of the qsort compare function */
static struct cmd_flags cmd_flags = { 0 };
static char *strings = NULL;
static uint32_t strsize = 0;
static enum bool compare_lto = FALSE;

/* flags set by processing a specific object file */
struct process_flags {
    uint32_t nsect;		/* The nsect, address and size for the */
    uint64_t sect_addr,		/*  section specified by the -s flag */
	     sect_size;
    enum bool sect_start_symbol;/* For processing the -l flag, set if a */
				/*  symbol with the start address of the */
				/*  section is found */
    uint32_t nsects;		/* For printing the symbol types, the number */
    struct section **sections;	/*  of sections and an array of section ptrs */
    struct section_64 **sections64;
    unsigned char text_nsect,	/* For printing symbols types, T, D, and B */
		  data_nsect,	/*  for text, data and bss symbols */
		  bss_nsect;
    uint32_t nlibs;		/* For printing the twolevel namespace */
    char **lib_names;		/*  references types, the number of libraries */
				/*  an array of pointers to library names */
};

struct symbol {
    const char *name;
    char *indr_name;
    struct nlist_64 nl;
};

struct value_diff {
    uint64_t size;
    struct symbol symbol;
};

static void usage(
    void);
static void nm(
    struct ofile *ofile,
    char *arch_name,
    void *cookie);
#ifdef LTO_SUPPORT
static void nm_lto(
    struct ofile *ofile,
    char *arch_name,
    struct cmd_flags *cmd_flags);
static void nm_llvm_bundle(
    char *llvm_bundle_pointer,
    uint64_t llvm_bundle_size,
    struct ofile *ofile,
    char *arch_name,
    struct cmd_flags *cmd_flags);
#endif /* LTO_SUPPORT */
static void print_header(
    struct ofile *ofile,
    char *arch_name,
    struct cmd_flags *cmd_flags);
static struct symbol *select_symbols(
    struct ofile *ofile,
    struct symtab_command *st,
    struct dysymtab_command *dyst,
    struct cmd_flags *cmd_flags,
    struct process_flags *process_flags,
    uint32_t *nsymbols);
static void make_symbol_32(
    struct symbol *symbol,
    struct nlist *nl);
static void make_symbol_64(
    struct symbol *symbol,
    struct nlist_64 *nl);
static enum bool select_symbol(
    struct symbol *symbol,
    struct cmd_flags *cmd_flags,
    struct process_flags *process_flags);
static void print_mach_symbols(
    struct ofile *ofile,
    struct symbol *symbols,
    uint32_t nsymbols,
    char *strings,
    uint32_t strsize,
    struct cmd_flags *cmd_flags,
    struct process_flags *process_flags,
    char *arch_name);
static void print_symbols(
    struct ofile *ofile,
    struct symbol *symbols,
    uint32_t nsymbols,
    char *strings,
    uint32_t strsize,
    struct cmd_flags *cmd_flags,
    struct process_flags *process_flags,
    char *arch_name,
    struct value_diff *value_diffs);
static char * stab(
    unsigned char n_type);
static int compare(
    struct symbol *p1,
    struct symbol *p2);
static int value_diff_compare(
    struct value_diff *p1,
    struct value_diff *p2);

/* apple_version is created by the libstuff/Makefile */
extern char apple_version[];
char *version = apple_version;

int
main(
int argc,
char **argv,
char **envp)
{
    int i;
    uint32_t j;
    struct arch_flag *arch_flags;
    uint32_t narch_flags;
    enum bool all_archs;
    char **files;

	progname = argv[0];

	arch_flags = NULL;
	narch_flags = 0;
	all_archs = FALSE;

	cmd_flags.nfiles = 0;
	cmd_flags.a = FALSE;
	cmd_flags.g = FALSE;
	cmd_flags.n = FALSE;
	cmd_flags.o = FALSE;
	cmd_flags.p = FALSE;
	cmd_flags.r = FALSE;
	cmd_flags.u = FALSE;
	cmd_flags.U = FALSE;
	cmd_flags.m = FALSE;
	cmd_flags.x = FALSE;
	cmd_flags.j = FALSE;
	cmd_flags.s = FALSE;
	cmd_flags.segname = NULL;
	cmd_flags.sectname = NULL;
	cmd_flags.l = FALSE;
	cmd_flags.f = FALSE;
	cmd_flags.bincl_name = NULL;
	cmd_flags.A = FALSE;
	cmd_flags.P = FALSE;
	cmd_flags.format = "%llx";

        files = allocate(sizeof(char *) * argc);
	for(i = 1; i < argc; i++){
	    if(argv[i][0] == '-'){
		if(argv[i][1] == '\0' ||
		   (argv[i][1] == '-' && argv[i][2] == '\0')){
		    i++;
		    for( ; i < argc; i++)
			files[cmd_flags.nfiles++] = argv[i];
		    break;
		}
		if(strcmp(argv[i], "-arch") == 0){
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
			cmd_flags.format = "%lld";
			break;
		    case 'o':
			cmd_flags.format = "%llo";
			break;
		    case 'x':
			cmd_flags.format = "%llx";
			break;
		    default:
			error("invalid argument to option: %s %s",
			      argv[i], argv[i+1]);
			usage();
		    }
		    i++;
		}
		else{
		    for(j = 1; argv[i][j] != '\0'; j++){
			switch(argv[i][j]){
			case 'a':
			    cmd_flags.a = TRUE;
			    break;
			case 'g':
			    cmd_flags.g = TRUE;
			    break;
			case 'n':
			    cmd_flags.n = TRUE;
			    break;
			case 'o':
			    cmd_flags.o = TRUE;
			    break;
			case 'p':
			    cmd_flags.p = TRUE;
			    break;
			case 'r':
			    cmd_flags.r = TRUE;
			    break;
			case 'u':
			    if(cmd_flags.U == TRUE){
				error("can't specifiy both -u and -U");
				usage();
			    }
			    cmd_flags.u = TRUE;
			    break;
			case 'U':
			    if(cmd_flags.u == TRUE){
				error("can't specifiy both -U and -u");
				usage();
			    }
			    cmd_flags.U = TRUE;
			    break;
			case 'm':
			    cmd_flags.m = TRUE;
			    break;
			case 'x':
			    cmd_flags.x = TRUE;
			    break;
			case 'j':
			    cmd_flags.j = TRUE;
			    break;
			case 's':
			    if(cmd_flags.s == TRUE){
				error("more than one -s option specified");
				usage();
			    }
			    cmd_flags.s = TRUE;
			    break;
			case 'b':
			    if(cmd_flags.b == TRUE){
				error("more than one -b option specified");
				usage();
			    }
			    cmd_flags.b = TRUE;
			    break;
			case 'i':
			    if(cmd_flags.i == TRUE){
				error("more than one -i option specified");
				usage();
			    }
			    cmd_flags.i = TRUE;
			    while(isdigit(argv[i][j+1])){
				cmd_flags.index =  cmd_flags.index * 10 +
						   (argv[i][j+1] - '0');
				j++;
			    }
			case 'l':
			    cmd_flags.l = TRUE;
			    break;
			case 'f':
			    cmd_flags.f = TRUE;
			    break;
			case 'v':
			    cmd_flags.v = TRUE;
			    break;
			case 'A':
			    cmd_flags.A = TRUE;
			    break;
			case 'P':
			    cmd_flags.P = TRUE;
			    break;
#ifdef LTO_SUPPORT
			case 'L':
			    cmd_flags.L = TRUE;
			    break;
#endif /* LTO_SUPPORT */
			default:
			    error("invalid argument -%c", argv[i][j]);
			    usage();
			}
		    }
		    if(cmd_flags.s == TRUE && cmd_flags.segname == NULL){
			if(i + 2 == argc){
			    error("missing arguments to -s");
			    usage();
			}
			cmd_flags.segname  = argv[i+1];
			cmd_flags.sectname = argv[i+2];
			i += 2;
		    }
		    if(cmd_flags.b == TRUE && cmd_flags.bincl_name == NULL){
			if(i + 1 == argc){
			    error("missing arguments to -b");
			    usage();
			}
			cmd_flags.bincl_name = argv[i+1];
			i += 1;
		    }
		}
		continue;
	    }
	    files[cmd_flags.nfiles++] = argv[i];
	}

	for(j = 0; j < cmd_flags.nfiles; j++)
	    ofile_process(files[j], arch_flags, narch_flags, all_archs, TRUE,
			  cmd_flags.f, TRUE, nm, &cmd_flags);
	if(cmd_flags.nfiles == 0)
	    ofile_process("a.out",  arch_flags, narch_flags, all_archs, TRUE,
			  cmd_flags.f, TRUE, nm, &cmd_flags);

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
	fprintf(stderr, "Usage: %s [-agnopruUmxjlfAP"
#ifdef LTO_SUPPORT
		"L"
#endif /* LTO_SUPPORT */
		"[s segname sectname] [-] "
		"[-t format] [[-arch <arch_flag>] ...] [file ...]\n", progname);
	exit(EXIT_FAILURE);
}

/*
 * nm() is the routine that gets called by ofile_process() to process single
 * object files.
 */
static
void
nm(
struct ofile *ofile,
char *arch_name,
void *cookie)
{
    uint32_t ncmds, mh_flags, mh_filetype;
    struct cmd_flags *cmd_flags;
    struct process_flags process_flags;
    uint32_t i, j, k;
    struct load_command *lc;
    struct symtab_command *st;
    struct dysymtab_command *dyst;
    struct segment_command *sg;
    struct segment_command_64 *sg64;
    struct section *s;
    struct section_64 *s64;
    struct dylib_command *dl;

    struct symbol *symbols;
    uint32_t nsymbols;
    struct value_diff *value_diffs;

    char *short_name, *has_suffix;
    enum bool is_framework;
#ifdef LTO_SUPPORT
    char *llvm_bundle_pointer;
    uint64_t llvm_bundle_size;
    enum bool llvm_bundle_found;
	llvm_bundle_found = FALSE;
#endif /* LTO_SUPPORT */

	/* cctools-port start */
	memset(&process_flags, '\0', sizeof(process_flags));
	/* cctools-port end */

	cmd_flags = (struct cmd_flags *)cookie;

	process_flags.nsect = -1;
	process_flags.sect_addr = 0;
	process_flags.sect_size = 0;
	process_flags.sect_start_symbol = FALSE;
	process_flags.nsects = 0;
	process_flags.sections = NULL;
	process_flags.text_nsect = NO_SECT;
	process_flags.data_nsect = NO_SECT;
	process_flags.bss_nsect = NO_SECT;
	process_flags.nlibs = 0;
	process_flags.lib_names = NULL;

#ifdef LTO_SUPPORT
	llvm_bundle_pointer = NULL;
	llvm_bundle_size = 0;
#endif /* LTO_SUPPORT */

	if(ofile->mh == NULL && ofile->mh64 == NULL){
#ifdef LTO_SUPPORT
	    if(ofile->lto != NULL)
		nm_lto(ofile, arch_name, cmd_flags);
#endif /* LTO_SUPPORT */
	    return;
	}
	st = NULL;
	dyst = NULL;
	lc = ofile->load_commands;
	if(ofile->mh != NULL){
	    ncmds = ofile->mh->ncmds;
	    mh_flags = ofile->mh->flags;
	    mh_filetype = ofile->mh->filetype;
	}
	else{
	    ncmds = ofile->mh64->ncmds;
	    mh_flags = ofile->mh64->flags;
	    mh_filetype = ofile->mh64->filetype;
	}
	for(i = 0; i < ncmds; i++){
	    if(st == NULL && lc->cmd == LC_SYMTAB){
		st = (struct symtab_command *)lc;
	    }
	    else if(dyst == NULL && lc->cmd == LC_DYSYMTAB){
		dyst = (struct dysymtab_command *)lc;
	    }
	    else if(lc->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc;
		process_flags.nsects += sg->nsects;
	    }
	    else if(lc->cmd == LC_SEGMENT_64){
		sg64 = (struct segment_command_64 *)lc;
		process_flags.nsects += sg64->nsects;
	    }
	    else if((mh_flags & MH_TWOLEVEL) == MH_TWOLEVEL &&
		    (lc->cmd == LC_LOAD_DYLIB ||
		     lc->cmd == LC_LOAD_WEAK_DYLIB ||
		     lc->cmd == LC_LAZY_LOAD_DYLIB ||
		     lc->cmd == LC_REEXPORT_DYLIB ||
		     lc->cmd == LC_LOAD_UPWARD_DYLIB)){
		process_flags.nlibs++;
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	if(process_flags.nsects > 0){
	    if(ofile->mh != NULL){
		process_flags.sections = (struct section **)
			   malloc(sizeof(struct section *) *
			   process_flags.nsects);
		process_flags.sections64 = NULL;
	    }
	    else{
		process_flags.sections64 = (struct section_64 **)
			   malloc(sizeof(struct section_64 *) *
			   process_flags.nsects);
		process_flags.sections = NULL;
	    }
	    k = 0;
	    lc = ofile->load_commands;
	    for (i = 0; i < ncmds; i++){
		if(lc->cmd == LC_SEGMENT){
		    sg = (struct segment_command *)lc;
		    s = (struct section *)
			  ((char *)sg + sizeof(struct segment_command));
		    for(j = 0; j < sg->nsects; j++){
			if(strcmp((s + j)->sectname, SECT_TEXT) == 0 &&
			   strcmp((s + j)->segname, SEG_TEXT) == 0)
			    process_flags.text_nsect = k + 1;
			else if(strcmp((s + j)->sectname, SECT_DATA) == 0 &&
				strcmp((s + j)->segname, SEG_DATA) == 0)
			    process_flags.data_nsect = k + 1;
			else if(strcmp((s + j)->sectname, SECT_BSS) == 0 &&
				strcmp((s + j)->segname, SEG_DATA) == 0)
			    process_flags.bss_nsect = k + 1;
#ifdef LTO_SUPPORT
			else if(strcmp((s + j)->sectname, "__bundle") == 0 &&
				strcmp((s + j)->segname, "__LLVM") == 0){
			    if(((st == NULL || st->nsyms == 0) ||
				cmd_flags->L) &&
                               (((s + j)->flags & SECTION_TYPE) != S_ZEROFILL)){
				if((s + j)->offset > ofile->object_size){
				    Mach_O_error(ofile, "section offset for "
					     "section (__LLVM,__bundle) is "
					     "past end of file");
				}
				else if((s + j)->size > ofile->object_size ||
				        (s + j)->offset + (s + j)->size >
							ofile->object_size){
				    Mach_O_error(ofile, "section "
					     "(__LLVM,__bundle) extends past "
					     "end of file");
				}
				else{
				    llvm_bundle_pointer = ofile->object_addr +
							  (s + j)->offset;
				    llvm_bundle_size = (s + j)->size;
				    llvm_bundle_found = TRUE;
				}
			    }
			}
#endif /* LTO_SUPPORT */
			if(cmd_flags->segname != NULL){
			    if(strncmp((s + j)->sectname, cmd_flags->sectname,
				       sizeof(s->sectname)) == 0 &&
			       strncmp((s + j)->segname, cmd_flags->segname,
				       sizeof(s->segname)) == 0){
				process_flags.nsect = k + 1;
				process_flags.sect_addr = (s + j)->addr;
				process_flags.sect_size = (s + j)->size;
			    }
			}
			process_flags.sections[k++] = s + j;
		    }
		}
		else if(lc->cmd == LC_SEGMENT_64){
		    sg64 = (struct segment_command_64 *)lc;
		    s64 = (struct section_64 *)
			  ((char *)sg64 + sizeof(struct segment_command_64));
		    for(j = 0; j < sg64->nsects; j++){
			if(strcmp((s64 + j)->sectname, SECT_TEXT) == 0 &&
			   strcmp((s64 + j)->segname, SEG_TEXT) == 0)
			    process_flags.text_nsect = k + 1;
			else if(mh_filetype == MH_KEXT_BUNDLE &&
			   strcmp((s64 + j)->sectname, SECT_TEXT) == 0 &&
			   strcmp((s64 + j)->segname, "__TEXT_EXEC") == 0)
			    process_flags.text_nsect = k + 1;
			else if(strcmp((s64 + j)->sectname, SECT_DATA) == 0 &&
				strcmp((s64 + j)->segname, SEG_DATA) == 0)
			    process_flags.data_nsect = k + 1;
			else if(strcmp((s64 + j)->sectname, SECT_BSS) == 0 &&
				strcmp((s64 + j)->segname, SEG_DATA) == 0)
			    process_flags.bss_nsect = k + 1;
#ifdef LTO_SUPPORT
			else if(strcmp((s64 + j)->sectname, "__bundle") == 0 &&
				strcmp((s64 + j)->segname, "__LLVM") == 0)
			    if(((st == NULL || st->nsyms == 0) ||
				cmd_flags->L) &&
                               (((s64 + j)->flags & SECTION_TYPE)
							      != S_ZEROFILL)){
				if((s64 + j)->offset > ofile->object_size){
				    Mach_O_error(ofile, "section offset for "
					     "section (__LLVM,__bundle) is "
					     "past end of file");
				}
				else if((s64 + j)->size > ofile->object_size ||
				        (s64 + j)->offset + (s64 + j)->size >
							ofile->object_size){
				    Mach_O_error(ofile, "section "
					     "(__LLVM,__bundle) extends past "
					     "end of file");
				}
				else{
				    llvm_bundle_pointer = ofile->object_addr +
							  (s64 + j)->offset;
				    llvm_bundle_size = (s64 + j)->size;
				    llvm_bundle_found = TRUE;
				}
			    }
#endif /* LTO_SUPPORT */
			if(cmd_flags->segname != NULL){
			    if(strncmp((s64 + j)->sectname, cmd_flags->sectname,
				       sizeof(s64->sectname)) == 0 &&
			       strncmp((s64 + j)->segname, cmd_flags->segname,
				       sizeof(s64->segname)) == 0){
				process_flags.nsect = k + 1;
				process_flags.sect_addr = (s64 + j)->addr;
				process_flags.sect_size = (s64 + j)->size;
			    }
			}
			process_flags.sections64[k++] = s64 + j;
		    }
		}
		lc = (struct load_command *)
		      ((char *)lc + lc->cmdsize);
	    }
	}
	if(st == NULL || st->nsyms == 0){
#ifdef LTO_SUPPORT
	    if(llvm_bundle_found == TRUE)
		nm_llvm_bundle(llvm_bundle_pointer, llvm_bundle_size,
			       ofile, arch_name, cmd_flags);
	    else
#endif /* LTO_SUPPORT */
		warning("no name list");
	    return;
	}
#ifdef LTO_SUPPORT
	else if(cmd_flags->L && llvm_bundle_found == TRUE){
	    nm_llvm_bundle(llvm_bundle_pointer, llvm_bundle_size,
			   ofile, arch_name, cmd_flags);
	    return;
	}
#endif /* LTO_SUPPORT */
	if((mh_flags & MH_TWOLEVEL) == MH_TWOLEVEL &&
	   process_flags.nlibs > 0){
	    process_flags.lib_names = (char **)
		       malloc(sizeof(char *) * process_flags.nlibs);
	    j = 0;
	    lc = ofile->load_commands;
	    for (i = 0; i < ncmds; i++){
		if(lc->cmd == LC_LOAD_DYLIB ||
		   lc->cmd == LC_LOAD_WEAK_DYLIB ||
		   lc->cmd == LC_LAZY_LOAD_DYLIB ||
		   lc->cmd == LC_REEXPORT_DYLIB ||
		   lc->cmd == LC_LOAD_UPWARD_DYLIB){
		    dl = (struct dylib_command *)lc;
		    process_flags.lib_names[j] =
			(char *)dl + dl->dylib.name.offset;
		    short_name = guess_short_name(process_flags.lib_names[j],
						  &is_framework, &has_suffix);
		    if(short_name != NULL)
			process_flags.lib_names[j] = short_name;
		    j++;
		}
		lc = (struct load_command *)
		      ((char *)lc + lc->cmdsize);
	    }
	}

	/* select symbols to print */
	symbols = select_symbols(ofile, st, dyst, cmd_flags, &process_flags,
				 &nsymbols);

	/* set names in the symbols to be printed */
	strings = ofile->object_addr + st->stroff;
	strsize = st->strsize;
	compare_lto = FALSE;
	if(cmd_flags->x == FALSE){
	    for(i = 0; i < nsymbols; i++){
		if(symbols[i].nl.n_un.n_strx == 0)
		    symbols[i].name = "";
		else if((int)symbols[i].nl.n_un.n_strx < 0 ||
			(uint32_t)symbols[i].nl.n_un.n_strx > st->strsize)
		    symbols[i].name = "bad string index";
		else
		    symbols[i].name = symbols[i].nl.n_un.n_strx + strings;

		if((symbols[i].nl.n_type & N_STAB) == 0 &&
		   (symbols[i].nl.n_type & N_TYPE) == N_INDR){
		    if(symbols[i].nl.n_value == 0)
			symbols[i].indr_name = "";
		    else if(symbols[i].nl.n_value > st->strsize)
			symbols[i].indr_name = "bad string index";
		    else
			symbols[i].indr_name = strings + symbols[i].nl.n_value;
		}
	    }
	    if(cmd_flags->l == TRUE &&
	       (int32_t)process_flags.nsect != -1 &&
	       process_flags.sect_start_symbol == FALSE &&
	       process_flags.sect_size != 0){
		symbols = reallocate(symbols,
				     (nsymbols + 1) * sizeof(struct symbol));
		symbols[nsymbols].name = ".section_start";
		symbols[nsymbols].nl.n_type = N_SECT;
		symbols[nsymbols].nl.n_sect = process_flags.nsect;
		symbols[nsymbols].nl.n_value = process_flags.sect_addr;
		nsymbols++;
	    }
	}

	/* print header if needed */
	print_header(ofile, arch_name, cmd_flags);

	/* sort the symbols if needed */
	if(cmd_flags->p == FALSE && cmd_flags->b == FALSE)
	    qsort(symbols, nsymbols, sizeof(struct symbol),
		  (int (*)(const void *, const void *))compare);

	value_diffs = NULL;
	if(cmd_flags->v == TRUE && cmd_flags->n == TRUE &&
	   cmd_flags->r == FALSE && cmd_flags->s == TRUE &&
	   nsymbols != 0){
	    value_diffs = allocate(sizeof(struct value_diff) * nsymbols);
	    for(i = 0; i < nsymbols - 1; i++){
		value_diffs[i].symbol = symbols[i];
		value_diffs[i].size = symbols[i+1].nl.n_value -
				      symbols[i].nl.n_value;
	    }
	    value_diffs[i].symbol = symbols[i];
	    value_diffs[i].size =
		process_flags.sect_addr + process_flags.sect_size -
		symbols[i].nl.n_value;
	    qsort(value_diffs, nsymbols, sizeof(struct value_diff), 
		  (int (*)(const void *, const void *))value_diff_compare);
	    for(i = 0; i < nsymbols; i++)
		symbols[i] = value_diffs[i].symbol;
	}

	/* now print the symbols as specified by the flags */
	if(cmd_flags->m == TRUE)
	    print_mach_symbols(ofile, symbols, nsymbols, strings, st->strsize,
			       cmd_flags, &process_flags, arch_name);
	else
	    print_symbols(ofile, symbols, nsymbols, strings, st->strsize,
			  cmd_flags, &process_flags, arch_name, value_diffs);

	free(symbols);
	if(process_flags.sections != NULL){
	    if(process_flags.sections != NULL){
		free(process_flags.sections);
		process_flags.sections = NULL;
	    }
	    if(process_flags.sections64 != NULL){
		free(process_flags.sections64);
		process_flags.sections64 = NULL;
	    }
	}
}

#ifdef LTO_SUPPORT
/*
 * In translating the information in an lto bitcode file to something that looks
 * like what would be in a Mach-O file for use by print_mach_symbols() we use
 * these sections for the CODE, DATA and RODATA defined symbols.
 */
static struct section lto_code_section = { "CODE", "LTO" };
static struct section lto_data_section = { "DATA", "LTO" };
static struct section lto_rodata_section = { "RODATA", "LTO" };
static struct section *lto_sections[3] = {
	&lto_code_section,
	&lto_data_section,
	&lto_rodata_section
};
static struct section_64 lto_code_section64 = { "CODE", "LTO" };
static struct section_64 lto_data_section64 = { "DATA", "LTO" };
static struct section_64 lto_rodata_section64 = { "RODATA", "LTO" };
static struct section_64 *lto_sections64[3] = {
	&lto_code_section64,
	&lto_data_section64,
	&lto_rodata_section64
};

/*
 * nm_lto() is called by nm() to process an lto bitcode file.
 */
static
void
nm_lto(
struct ofile *ofile,
char *arch_name,
struct cmd_flags *cmd_flags)
{
    uint32_t nsyms, nsymbols, i;
    struct symbol symbol, *symbols;
    struct process_flags process_flags;

	process_flags.nsect = -1;
	if(cmd_flags->segname != NULL &&
	   strcmp(cmd_flags->segname, "LTO") == 0){
	    if(strcmp(cmd_flags->sectname, "CODE") == 0)
		process_flags.nsect = 1;
	    else if(strcmp(cmd_flags->sectname, "DATA") == 0)
		process_flags.nsect = 2;
	    else if(strcmp(cmd_flags->sectname, "RODATA") == 0)
		process_flags.nsect = 3;
	}
	process_flags.sect_addr = 0;
	process_flags.sect_size = 0;
	process_flags.sect_start_symbol = FALSE;
	process_flags.nsects = 3;
	if((ofile->lto_cputype & CPU_ARCH_ABI64) != CPU_ARCH_ABI64){
	    process_flags.sections = lto_sections;
	    process_flags.sections64 = NULL;
	}
	else{
	    process_flags.sections64 = lto_sections64;
	    process_flags.sections = NULL;
	}
	process_flags.text_nsect = 1;
	process_flags.data_nsect = 2;
	process_flags.bss_nsect = NO_SECT;
	process_flags.nlibs = 0;
	process_flags.lib_names = NULL;

	nsyms = lto_get_nsyms(ofile->lto);
	symbols = allocate(sizeof(struct symbol) * nsyms);

	nsymbols = 0;
	for(i = 0; i < nsyms; i++){
	    symbol.name = lto_symbol_name(ofile->lto, i);
	    symbol.indr_name = NULL;
	    lto_get_nlist_64(&(symbol.nl), ofile->lto, i);
	    if(select_symbol(&symbol, cmd_flags, &process_flags))
		symbols[nsymbols++] = symbol;
	}

	print_header(ofile, arch_name, cmd_flags);

	/* reset these as the can be used by compare() with -x */
	strings = NULL;
	strsize = 0;
	compare_lto = TRUE;

	/* sort the symbols if needed */
	if(cmd_flags->p == FALSE)
	    qsort(symbols, nsymbols, sizeof(struct symbol),
		  (int (*)(const void *, const void *))compare);

	/* now print the symbols as specified by the flags */
	if(cmd_flags->m == TRUE)
	    print_mach_symbols(ofile, symbols, nsymbols, NULL, 0,
			       cmd_flags, &process_flags, arch_name);
	else
	    print_symbols(ofile, symbols, nsymbols, NULL, 0,
			  cmd_flags, &process_flags, arch_name, NULL);

	free(symbols);
}

/*
 * These are pointers to the xar API's so that libxar.dylib can be loaded
 * dynamically with dlopen() and then the symbols for these API's can be
 * looked up via dlsym() and if things are mising the the code does nothing
 * and does not fail.
 */
static enum bool tried_to_load_libxar = FALSE;
static void *xar_handle = NULL;
static xar_t (*ptr_xar_open)(const char *file, int32_t flags) = NULL;
static void (*ptr_xar_serialize)(xar_t x, const char *file) = NULL;
static int (*ptr_xar_close)(xar_t x) = NULL;
static xar_file_t (*ptr_xar_file_first)(xar_t x, xar_iter_t i) = NULL;
static xar_file_t (*ptr_xar_file_next)(xar_iter_t i) = NULL;
static void (*ptr_xar_iter_free)(xar_iter_t i) = NULL;
static xar_iter_t (*ptr_xar_iter_new)(void) = NULL;
static const char * (*ptr_xar_prop_first)(xar_file_t f, xar_iter_t i) = NULL;
static int32_t (*ptr_xar_prop_get)(xar_file_t f, const char *key,
				   const char **value) = NULL;
static const char * (*ptr_xar_prop_next)(xar_iter_t i) = NULL;
static void (*ptr_xar_prop_unset)(xar_file_t f, const char *key) = NULL;
static int32_t (*ptr_xar_extract_tobuffersz)(xar_t x, xar_file_t f,
                char **buffer, size_t *size) = NULL;

/*
 * nm_llvm_bundle() is called by nm() to process the contents of the
 * (__LLVM,__bundle) which is a xar file.  And also called recursively when one
 * of the xar files members contents is a possible xar file.  It expects to
 * find a xar file with bitcode file members (or a nested xar file).
 */
static
void
nm_llvm_bundle(
char *llvm_bundle_pointer,
uint64_t llvm_bundle_size,
struct ofile *ofile,
char *arch_name,
struct cmd_flags *cmd_flags)
{
    uint32_t r, bufsize;
    char *p, *prefix, *xar_path, buf[MAXPATHLEN], resolved_name[PATH_MAX];
    char xar_filename[] = "/tmp/temp.XXXXXX";
    int xar_fd;
    xar_t xar;
    xar_iter_t i;
    xar_file_t f;

	/*
	 * Note this check is also preformed before we are called recursively,
	 * so if it fails we know it was from the top level and called directly
	 * for the contents of a Mach-O file with the (__LLVM,__bundle)
	 * contents.
	 */
	if(llvm_bundle_size < sizeof(struct xar_header)){
	    Mach_O_error(ofile, "size of (__LLVM,__bundle) section too "
			 "small (smaller than size of struct xar_header)");
	    return;
	}

	if(tried_to_load_libxar == FALSE){
	    tried_to_load_libxar = TRUE;
	    /*
	     * Construct the prefix to this executable assuming it is in a bin
	     * directory relative to a lib directory of the matching xar library
	     * and first try to load that.  If not then fall back to trying
	     * "/usr/lib/libxar.dylib". 
	     */
	    bufsize = MAXPATHLEN;
	    p = buf;
	    r = _NSGetExecutablePath(p, &bufsize);
	    if(r == -1){
		p = allocate(bufsize);
		_NSGetExecutablePath(p, &bufsize);
	    }
	    prefix = realpath(p, resolved_name);
	    p = rindex(prefix, '/');
	    if(p != NULL)
		p[1] = '\0';
	    xar_path = makestr(prefix, "../lib/libxar.dylib", NULL);

	    xar_handle = dlopen(xar_path, RTLD_NOW);
	    if(xar_handle == NULL){
		free(xar_path);
		xar_path = NULL;
		xar_handle = dlopen("/usr/lib/libxar.dylib", RTLD_NOW);
	    }
	    if(xar_handle == NULL)
		return;

	    ptr_xar_open = dlsym(xar_handle, "xar_open");
	    ptr_xar_serialize = dlsym(xar_handle, "xar_serialize");
	    ptr_xar_close = dlsym(xar_handle, "xar_close");
	    ptr_xar_file_first = dlsym(xar_handle, "xar_file_first");
	    ptr_xar_file_next = dlsym(xar_handle, "xar_file_next");
	    ptr_xar_iter_free = dlsym(xar_handle, "xar_iter_free");
	    ptr_xar_iter_new = dlsym(xar_handle, "xar_iter_new");
	    ptr_xar_prop_first = dlsym(xar_handle, "xar_prop_first");
	    ptr_xar_prop_get = dlsym(xar_handle, "xar_prop_get");
	    ptr_xar_prop_next = dlsym(xar_handle, "xar_prop_next");
	    ptr_xar_prop_unset = dlsym(xar_handle, "xar_prop_unset");
	    ptr_xar_extract_tobuffersz =
				dlsym(xar_handle, "xar_extract_tobuffersz");
	    if(ptr_xar_open == NULL ||
	       ptr_xar_serialize == NULL ||
	       ptr_xar_close == NULL ||
	       ptr_xar_file_first == NULL ||
	       ptr_xar_file_next == NULL ||
	       ptr_xar_iter_free == NULL ||
	       ptr_xar_iter_new == NULL ||
	       ptr_xar_prop_first == NULL ||
	       ptr_xar_prop_get == NULL ||
	       ptr_xar_prop_next == NULL ||
	       ptr_xar_prop_unset == NULL ||
	       ptr_xar_extract_tobuffersz == NULL)
		return;
	}
	if(xar_handle == NULL)
	    return;

	xar_fd = mkstemp(xar_filename);
	if(write64(xar_fd, llvm_bundle_pointer, llvm_bundle_size) !=
	        llvm_bundle_size){
	    if(ofile->xar_member_name != NULL)
		system_error("Can't write (__LLVM,__bundle) section contents "
		    "to temporary file: %s\n", xar_filename);
	    else
		system_error("Can't write (__LLVM,__bundle) xar file %s "
		    "contents to temporary file: %s\n", ofile->xar_member_name,
		    xar_filename);
	    close(xar_fd);
	    return;
	}
	close(xar_fd);
	xar = ptr_xar_open(xar_filename, READ);
	if(!xar){
	    system_error("Can't create temporary xar archive %s\n",
			 xar_filename);
	    unlink(xar_filename);
	    return;
	}

	i = ptr_xar_iter_new();
	if(!i){
	    error("Can't obtain an xar iterator for xar archive %s\n",
		  xar_filename);
	    ptr_xar_close(xar);
	    unlink(xar_filename);
	    return;
	}

	/*
	 * Go through the xar's files.
	 */
	for(f = ptr_xar_file_first(xar, i); f; f = ptr_xar_file_next(i)){
	    const char *key;
	    xar_iter_t p;
	    const char *xar_member_name, *xar_member_type,
		       *xar_member_size_string;
	    size_t xar_member_size;
uint32_t nprops, maxprops;
        
	    p = ptr_xar_iter_new();
	    if(!p){
		error("Can't obtain an xar iterator for xar archive %s\n",
		      xar_filename);
		ptr_xar_close(xar);
		unlink(xar_filename);
		return;
	    }
	    xar_member_name = NULL;
	    xar_member_type = NULL;
	    xar_member_size_string = NULL;

	    /*
	     * See comment below about keys with multiple values, why knowing
	     * the maximum number of "props" is needed.
	     */
	    maxprops = 0;
	    for(key = ptr_xar_prop_first(f, p); key; key = ptr_xar_prop_next(p))
    	        maxprops++;

	    nprops = 0;
	    for(key = ptr_xar_prop_first(f, p);
		key;
		key = ptr_xar_prop_next(p)){
		nprops++;

		const char *val = NULL; 
		ptr_xar_prop_get(f, key, &val);
#if 0
		printf("key: %s, value: %s\n", key, val);
#endif
		if(strcmp(key, "name") == 0)
		    xar_member_name = val;
		if(strcmp(key, "type") == 0)
		    xar_member_type = val;
		if(strcmp(key, "data/size") == 0)
		    xar_member_size_string = val;

		/*
		 * These specific keys used for bitcode files may have multiple
		 * values.  But the xar_prop_get() API will always get the first
		 * value even though xar_prop_next() will advance to the next
		 * key.  The workaround used to get the next value for the
		 * same key is to delete the "prop" with xar_prop_unset(), but
		 * care has to be taken not to delete the last "prop" or then
		 * xar_prop_next() will crash.  There are also specific keys
		 * used for linker subdoc that have multiple values but since
		 * this is looping on file "props" they are not listed here.
		 */ 
		if((strcmp(key, "file-type") == 0 ||
		    strcmp(key, "clang/cmd") == 0 ||
		    strcmp(key, "swift-cmd") == 0) &&
		    nprops != maxprops)
		    ptr_xar_prop_unset(f, key);
	    }
 	    /*
	     * If we found a file with a name, date/size and type properties
	     * and with the type being "file" see if that is a bitcode file.
	     */
	    if(xar_member_name != NULL &&
	       xar_member_type != NULL &&
		   strcmp(xar_member_type, "file") == 0 &&
	       xar_member_size_string != NULL){
		/*
		 * Extract the file into a buffer.
		 */
		char *endptr;
		xar_member_size = strtoul(xar_member_size_string, &endptr, 10);
		if(*endptr == '\0' && xar_member_size != 0){
		    char *buffer;
		    buffer = allocate(xar_member_size);
		    if(ptr_xar_extract_tobuffersz(xar, f, &buffer,
					          &xar_member_size) == 0){
#if 0
			printf("xar member: %s extracted\n", xar_member_name);
#endif
			/*
			 * Set the ofile->xar_member_name we want to see
			 * printed in the header, which nm_lto() will caused
			 * to be printed.
			 */
			const char *old_xar_member_name;
			/*
			 * If ofile->xar_member_name is already set this is
			 * nested. So save the old name and create the nested
			 * name.
			 */
			if(ofile->xar_member_name != NULL){
			    old_xar_member_name = ofile->xar_member_name;
			    ofile->xar_member_name =
				makestr("[", ofile->xar_member_name, "]",
					xar_member_name, NULL);
			}
			else {
			    old_xar_member_name = NULL;
			    ofile->xar_member_name = xar_member_name;
			}
			/*
			 * Now see if this is bitcode, and if so finally call
			 * nm_lto() to get its symbols printed.
			 */
			if(is_llvm_bitcode(ofile, buffer, xar_member_size)){
			    nm_lto(ofile, arch_name, cmd_flags);
			    lto_free(ofile->lto);
			    ofile->lto = NULL;
			    ofile->lto_cputype = 0;
			    ofile->lto_cpusubtype = 0;
			}
			else{
			    /* See if this is could be a xar file (nested). */
			    if(xar_member_size >= sizeof(struct xar_header)){
#if 0
				printf("could be a xar file: %s\n",
				       ofile->xar_member_name);
#endif
				nm_llvm_bundle(buffer, xar_member_size,
					       ofile, arch_name, cmd_flags);
			    }
			}
			if(old_xar_member_name != NULL)
			    free((void *)ofile->xar_member_name);
			ofile->xar_member_name = old_xar_member_name;
		    }
		    free(buffer);
		}
	    }
	    ptr_xar_iter_free(p);
	}
	ptr_xar_close(xar);
	unlink(xar_filename);
}
#endif /* LTO_SUPPORT */

/* print header if needed */
static
void
print_header(
struct ofile *ofile,
char *arch_name,
struct cmd_flags *cmd_flags)
{
	if((ofile->member_ar_hdr != NULL ||
	    ofile->dylib_module_name != NULL ||
	    ofile->xar_member_name != NULL ||
	    cmd_flags->nfiles > 1 ||
	    arch_name != NULL) &&
	    (cmd_flags->o == FALSE && cmd_flags->A == FALSE)){
	    if(ofile->dylib_module_name != NULL){
		printf("\n%s(%s)", ofile->file_name, ofile->dylib_module_name);
	    }
	    else if(ofile->member_ar_hdr != NULL){
		printf("\n%s(%.*s)", ofile->file_name,
		       (int)ofile->member_name_size, ofile->member_name);
	    }
	    else if(ofile->xar_member_name != NULL){
		printf("\n%s[%s]", ofile->file_name, ofile->xar_member_name);
	    }
	    else
		printf("\n%s", ofile->file_name);
	    if(arch_name != NULL)
		printf(" (for architecture %s):\n", arch_name);
	    else
		printf(":\n");
	}
}

/*
 * select_symbols returns an allocated array of symbol structs as the symbols
 * that are to be printed based on the flags.  The number of symbols in the
 * array returned in returned indirectly through nsymbols.
 */
static
struct symbol *
select_symbols(
struct ofile *ofile,
struct symtab_command *st,
struct dysymtab_command *dyst,
struct cmd_flags *cmd_flags,
struct process_flags *process_flags,
uint32_t *nsymbols)
{
    uint32_t i, flags, nest;
    struct nlist *all_symbols;
    struct nlist_64 *all_symbols64;
    struct symbol *selected_symbols, symbol;
    struct dylib_module m;
    struct dylib_module_64 m64;
    struct dylib_reference *refs;
    enum bool found;
    uint32_t irefsym, nrefsym, nextdefsym, iextdefsym, nlocalsym, ilocalsym;

	if(ofile->mh != NULL){
	    all_symbols = (struct nlist *)(ofile->object_addr + st->symoff);
	    all_symbols64 = NULL;
	}
	else{
	    all_symbols = NULL;
	    all_symbols64 = (struct nlist_64 *)(ofile->object_addr +st->symoff);
	}
	selected_symbols = allocate(sizeof(struct symbol) * st->nsyms);
	*nsymbols = 0;

	if(ofile->object_byte_sex != get_host_byte_sex()){
	    if(ofile->mh != NULL)
		swap_nlist(all_symbols, st->nsyms, get_host_byte_sex());
	    else
		swap_nlist_64(all_symbols64, st->nsyms, get_host_byte_sex());
	}

	if(ofile->dylib_module != NULL){
	    if(ofile->mh != NULL){
		m = *ofile->dylib_module;
		if(ofile->object_byte_sex != get_host_byte_sex())
		    swap_dylib_module(&m, 1, get_host_byte_sex());
		irefsym = m.irefsym;
		nrefsym = m.nrefsym;
		nextdefsym = m.nextdefsym;
		iextdefsym = m.iextdefsym;
		nlocalsym = m.nlocalsym;
		ilocalsym = m.ilocalsym;
	    }
	    else{
		m64 = *ofile->dylib_module64;
		if(ofile->object_byte_sex != get_host_byte_sex())
		    swap_dylib_module_64(&m64, 1, get_host_byte_sex());
		irefsym = m64.irefsym;
		nrefsym = m64.nrefsym;
		nextdefsym = m64.nextdefsym;
		iextdefsym = m64.iextdefsym;
		nlocalsym = m64.nlocalsym;
		ilocalsym = m64.ilocalsym;
	    }
	    refs = (struct dylib_reference *)(ofile->object_addr +
					      dyst->extrefsymoff);
	    if(ofile->object_byte_sex != get_host_byte_sex()){
		swap_dylib_reference(refs + irefsym, nrefsym,
				     get_host_byte_sex());
	    }
	    for(i = 0; i < nrefsym; i++){
		flags = refs[i + irefsym].flags;
		if(flags == REFERENCE_FLAG_UNDEFINED_NON_LAZY ||
		   flags == REFERENCE_FLAG_UNDEFINED_LAZY ||
		   flags == REFERENCE_FLAG_PRIVATE_UNDEFINED_NON_LAZY ||
		   flags == REFERENCE_FLAG_PRIVATE_UNDEFINED_LAZY){
		    if(ofile->mh != NULL)
			make_symbol_32(&symbol,
				      all_symbols + refs[i + irefsym].isym);
		    else
			make_symbol_64(&symbol,
				      all_symbols64 + refs[i + irefsym].isym);
		    if(flags == REFERENCE_FLAG_UNDEFINED_NON_LAZY ||
		       flags == REFERENCE_FLAG_UNDEFINED_LAZY ||
		       cmd_flags->m == TRUE)
			symbol.nl.n_type = N_UNDF | N_EXT;
		    else
			symbol.nl.n_type = N_UNDF;
		    symbol.nl.n_desc = (symbol.nl.n_desc &~ REFERENCE_TYPE) |
				       flags;
		    symbol.nl.n_value = 0;
		    if(select_symbol(&symbol, cmd_flags, process_flags))
			selected_symbols[(*nsymbols)++] = symbol;
		}
	    }
	    for(i = 0; i < nextdefsym && iextdefsym + i < st->nsyms; i++){
		if(ofile->mh != NULL)
		    make_symbol_32(&symbol, all_symbols + iextdefsym + i);
		else
		    make_symbol_64(&symbol, all_symbols64 + iextdefsym + i);
		if(select_symbol(&symbol, cmd_flags, process_flags))
		    selected_symbols[(*nsymbols)++] = symbol;
	    }
	    for(i = 0; i < nlocalsym && ilocalsym + i < st->nsyms; i++){
		if(ofile->mh != NULL)
		    make_symbol_32(&symbol, all_symbols + ilocalsym + i);
		else
		    make_symbol_64(&symbol, all_symbols64 + ilocalsym + i);
		if(select_symbol(&symbol, cmd_flags, process_flags))
		    selected_symbols[(*nsymbols)++] = symbol;
	    }
	}
	else if(cmd_flags->b == TRUE){
	    found = FALSE;
	    strings = ofile->object_addr + st->stroff;
	    if(cmd_flags->i == TRUE)
		i = cmd_flags->index;
	    else
		i = 0;
	    for( ; i < st->nsyms; i++){
		if(ofile->mh != NULL)
		    make_symbol_32(&symbol, all_symbols + i);
		else
		    make_symbol_64(&symbol, all_symbols64 + i);
		if(symbol.nl.n_type == N_BINCL &&
		   symbol.nl.n_un.n_strx != 0 &&
		   (uint32_t)symbol.nl.n_un.n_strx < st->strsize &&
		   strcmp(cmd_flags->bincl_name,
			  strings + symbol.nl.n_un.n_strx) == 0){
		    selected_symbols[(*nsymbols)++] = symbol;
		    found = TRUE;
		    nest = 0;
		    for(i = i + 1 ; i < st->nsyms; i++){
			if(ofile->mh != NULL)
			    make_symbol_32(&symbol, all_symbols + i);
			else
			    make_symbol_64(&symbol, all_symbols64 + i);
			if(symbol.nl.n_type == N_BINCL)
			    nest++;
			else if(symbol.nl.n_type == N_EINCL){
			    if(nest == 0){
				selected_symbols[(*nsymbols)++] = symbol;
				break;
			    }
			    nest--;
			}
			else if(nest == 0)
			    selected_symbols[(*nsymbols)++] = symbol;
		    }
		}
		if(found == TRUE)
		    break;
	    }
	}
	else{
	    for(i = 0; i < st->nsyms; i++){
		if(ofile->mh != NULL)
		    make_symbol_32(&symbol, all_symbols + i);
		else
		    make_symbol_64(&symbol, all_symbols64 + i);
		if(select_symbol(&symbol, cmd_flags, process_flags))
		    selected_symbols[(*nsymbols)++] = symbol;
	    }
	}
	if(ofile->object_byte_sex != get_host_byte_sex()){
	    if(ofile->mh != NULL)
		swap_nlist(all_symbols, st->nsyms, ofile->object_byte_sex);
	    else
		swap_nlist_64(all_symbols64, st->nsyms, ofile->object_byte_sex);
	}
	/*
	 * Could reallocate selected symbols to the exact size but it is more
	 * of a time waste than a memory savings.
	 */
	return(selected_symbols);
}

static
void
make_symbol_32(
struct symbol *symbol,
struct nlist *nl)
{
	symbol->nl.n_un.n_strx = nl->n_un.n_strx;
	symbol->nl.n_type = nl->n_type;
	symbol->nl.n_sect = nl->n_sect;
	symbol->nl.n_desc = nl->n_desc;
	symbol->nl.n_value = nl->n_value;
}

static
void
make_symbol_64(
struct symbol *symbol,
struct nlist_64 *nl)
{
	symbol->nl = *nl;
}

/*
 * select_symbol() returns TRUE or FALSE if the specified symbol is to be
 * printed based on the flags.
 */
static
enum bool
select_symbol(
struct symbol *symbol,
struct cmd_flags *cmd_flags,
struct process_flags *process_flags)
{
	if(cmd_flags->u == TRUE){
	    if((symbol->nl.n_type == (N_UNDF | N_EXT) &&
		symbol->nl.n_value == 0) ||
	       symbol->nl.n_type == (N_PBUD | N_EXT))
		return(TRUE);
	    else
		return(FALSE);
	}
	if(cmd_flags->U == TRUE){
	    if((symbol->nl.n_type == (N_UNDF | N_EXT) &&
		symbol->nl.n_value == 0) ||
	       symbol->nl.n_type == (N_PBUD | N_EXT))
		return(FALSE);
	}
	if(cmd_flags->g == TRUE && (symbol->nl.n_type & N_EXT) == 0)
	    return(FALSE);
	if(cmd_flags->s == TRUE){
	    if(((symbol->nl.n_type & N_TYPE) == N_SECT) &&
		(symbol->nl.n_sect == process_flags->nsect)){
		if(cmd_flags->l &&
		   symbol->nl.n_value == process_flags->sect_addr){
		    process_flags->sect_start_symbol = TRUE;
		}
	    }
	    else
		return(FALSE);
	}
	if((symbol->nl.n_type & N_STAB) &&
	   (cmd_flags->a == FALSE || cmd_flags->g == TRUE ||
	    cmd_flags->u == TRUE))
		return(FALSE);
	return(TRUE);
}

/*
 * print_mach_symbols() is called when the -m flag is specified and prints
 * symbols in the extended Mach-O style format.
 */
static
void
print_mach_symbols(
struct ofile *ofile,
struct symbol *symbols,
uint32_t nsymbols,
char *strings,
uint32_t strsize,
struct cmd_flags *cmd_flags,
struct process_flags *process_flags,
char *arch_name)
{
    uint32_t i, library_ordinal;
    char *ta_xfmt, *i_xfmt, *dashes, *spaces;
    uint32_t mh_flags;

	mh_flags = 0;
	if(ofile->mh != NULL ||
	   (ofile->lto != NULL &&
	    (ofile->lto_cputype & CPU_ARCH_ABI64) != CPU_ARCH_ABI64)){
	    ta_xfmt = "%08llx";
	    i_xfmt =  "%08x";
	    if(ofile->mh != NULL)
		mh_flags = ofile->mh->flags;
	    spaces = "        ";
	    dashes = "--------";
	}
	else{
	    ta_xfmt = "%016llx";
	    i_xfmt =  "%016x";
	    if(ofile->mh64 != NULL)
		mh_flags = ofile->mh64->flags;
	    spaces = "                ";
	    dashes = "----------------";
	}
	for(i = 0; i < nsymbols; i++){
	    if(cmd_flags->x == TRUE){
		printf(ta_xfmt, symbols[i].nl.n_value);
		printf(" %02x %02x %04x ",
		       (unsigned int)(symbols[i].nl.n_type & 0xff),
		       (unsigned int)(symbols[i].nl.n_sect & 0xff),
		       (unsigned int)(symbols[i].nl.n_desc & 0xffff));
		if(symbols[i].nl.n_un.n_strx == 0){
		    printf(i_xfmt, symbols[i].nl.n_un.n_strx);
		    if(ofile->lto != NULL)
			printf(" %s", symbols[i].name);
		    else
			printf(" (null)");
		}
		else if((uint32_t)symbols[i].nl.n_un.n_strx > strsize){
		    printf("%08x", symbols[i].nl.n_un.n_strx);
		    printf(" (bad string index)");
		}
		else{
		    printf("%08x", symbols[i].nl.n_un.n_strx);
		    printf(" %s", symbols[i].nl.n_un.n_strx + strings);
		}
		if((symbols[i].nl.n_type & N_STAB) == 0 &&
		   (symbols[i].nl.n_type & N_TYPE) == N_INDR){
		    if(symbols[i].nl.n_value == 0){
			printf(" (indirect for ");
			printf(ta_xfmt, symbols[i].nl.n_value);
			printf(" (null))\n");
		    }
		    else if(symbols[i].nl.n_value > strsize){
			printf(" (indirect for ");
			printf(ta_xfmt, symbols[i].nl.n_value);
			printf(" (bad string index))\n");
		    }
		    else{
			printf(" (indirect for ");
			printf(ta_xfmt, symbols[i].nl.n_value);
			printf(" %s)\n", symbols[i].indr_name);
		    }
		}
		else
		    printf("\n");
		continue;
	    }

	    if(symbols[i].nl.n_type & N_STAB){
		if(cmd_flags->o == TRUE || cmd_flags->A == TRUE){
		    if(arch_name != NULL)
			printf("(for architecture %s):", arch_name);
		    if(ofile->dylib_module_name != NULL){
			printf("%s:%s: ", ofile->file_name,
			       ofile->dylib_module_name);
		    }
		    else if(ofile->member_ar_hdr != NULL){
			printf("%s:%.*s: ", ofile->file_name,
			       (int)ofile->member_name_size,
			       ofile->member_name);
		    }
		    else
			printf("%s: ", ofile->file_name);
		}
		printf(ta_xfmt, symbols[i].nl.n_value);
		printf(" - %02x %04x %5.5s %s\n",
		       (unsigned int)symbols[i].nl.n_sect & 0xff,
		       (unsigned int)symbols[i].nl.n_desc & 0xffff,
		       stab(symbols[i].nl.n_type), symbols[i].name);
		continue;
	    }

	    if(cmd_flags->o == TRUE || cmd_flags->A == TRUE){
		if(arch_name != NULL)
		    printf("(for architecture %s):", arch_name);
		if(ofile->dylib_module_name != NULL){
		    printf("%s:%s: ", ofile->file_name,
			   ofile->dylib_module_name);
		}
		else if(ofile->member_ar_hdr != NULL){
		    printf("%s:%.*s: ", ofile->file_name,
			   (int)ofile->member_name_size,
			   ofile->member_name);
		}
		else
		    printf("%s: ", ofile->file_name);
	    }

	    if(((symbols[i].nl.n_type & N_TYPE) == N_UNDF &&
		 symbols[i].nl.n_value == 0) ||
		 (symbols[i].nl.n_type & N_TYPE) == N_INDR)
		printf("%s", spaces);
	    else{
		if(ofile->lto)
		    printf("%s", dashes);
		else
		    printf(ta_xfmt, symbols[i].nl.n_value);
	    }

	    switch(symbols[i].nl.n_type & N_TYPE){
	    case N_UNDF:
	    case N_PBUD:
		if((symbols[i].nl.n_type & N_TYPE) == N_UNDF &&
		   symbols[i].nl.n_value != 0){
		    printf(" (common) ");
		    if(GET_COMM_ALIGN(symbols[i].nl.n_desc) != 0)
			printf("(alignment 2^%d) ",
			       GET_COMM_ALIGN(symbols[i].nl.n_desc));
		}
		else{
		    if((symbols[i].nl.n_type & N_TYPE) == N_PBUD)
			printf(" (prebound ");
		    else
			printf(" (");
		    if((symbols[i].nl.n_desc & REFERENCE_TYPE) ==
		       REFERENCE_FLAG_UNDEFINED_LAZY)
			printf("undefined [lazy bound]) ");
		    else if((symbols[i].nl.n_desc & REFERENCE_TYPE) ==
			    REFERENCE_FLAG_PRIVATE_UNDEFINED_LAZY)
			printf("undefined [private lazy bound]) ");
		    else if((symbols[i].nl.n_desc & REFERENCE_TYPE) ==
			    REFERENCE_FLAG_PRIVATE_UNDEFINED_NON_LAZY)
			printf("undefined [private]) ");
		    else
			printf("undefined) ");
		}
		break;
	    case N_ABS:
		printf(" (absolute) ");
		
		break;
	    case N_INDR:
		printf(" (indirect) ");
		break;
	    case N_SECT:
		if(symbols[i].nl.n_sect >= 1 &&
		   symbols[i].nl.n_sect <= process_flags->nsects){
		    if(ofile->mh != NULL ||
	   	       (ofile->lto != NULL &&
	    		(ofile->lto_cputype & CPU_ARCH_ABI64) !=
			 CPU_ARCH_ABI64)){
			printf(" (%.16s,%.16s) ",
			       process_flags->sections[
				    symbols[i].nl.n_sect-1]->segname,
			       process_flags->sections[
				    symbols[i].nl.n_sect-1]->sectname);
		    }
		    else{
			printf(" (%.16s,%.16s) ",
			       process_flags->sections64[
				    symbols[i].nl.n_sect-1]->segname,
			       process_flags->sections64[
				    symbols[i].nl.n_sect-1]->sectname);
		    }
		}
		else
		    printf(" (?,?) ");
		break;
	    default:
		    printf(" (?) ");
		    break;
	    }

	    if(symbols[i].nl.n_type & N_EXT){
		if(symbols[i].nl.n_desc & REFERENCED_DYNAMICALLY)
		    printf("[referenced dynamically] ");
		if(symbols[i].nl.n_type & N_PEXT){
		    if((symbols[i].nl.n_desc & N_WEAK_DEF) == N_WEAK_DEF)
			printf("weak private external ");
		    else
			printf("private external ");
		}
		else{
		    if((symbols[i].nl.n_desc & N_WEAK_REF) == N_WEAK_REF ||
		       (symbols[i].nl.n_desc & N_WEAK_DEF) == N_WEAK_DEF){
			if((symbols[i].nl.n_desc & (N_WEAK_REF | N_WEAK_DEF)) ==
			   (N_WEAK_REF | N_WEAK_DEF))
			    printf("weak external automatically hidden ");
			else
			    printf("weak external ");
		    }
		    else
			printf("external ");
		}
	    }
	    else{
		if(symbols[i].nl.n_type & N_PEXT)
		    printf("non-external (was a private external) ");
		else
		    printf("non-external ");
	    }
	    
	    if(ofile->mh_filetype == MH_OBJECT &&
	       (symbols[i].nl.n_desc & N_NO_DEAD_STRIP) == N_NO_DEAD_STRIP)
		    printf("[no dead strip] ");

	    if(ofile->mh_filetype == MH_OBJECT &&
	       ((symbols[i].nl.n_type & N_TYPE) != N_UNDF) &&
	       (symbols[i].nl.n_desc & N_SYMBOL_RESOLVER) == N_SYMBOL_RESOLVER)
		    printf("[symbol resolver] ");

	    if(ofile->mh_filetype == MH_OBJECT &&
	       ((symbols[i].nl.n_type & N_TYPE) != N_UNDF) &&
	       (symbols[i].nl.n_desc & N_ALT_ENTRY) == N_ALT_ENTRY)
		    printf("[alt entry] ");

	    if(ofile->mh_filetype == MH_OBJECT &&
	       ((symbols[i].nl.n_type & N_TYPE) != N_UNDF) &&
	       (symbols[i].nl.n_desc & N_COLD_FUNC) == N_COLD_FUNC)
		    printf("[cold func] ");

	    if((symbols[i].nl.n_desc & N_ARM_THUMB_DEF) == N_ARM_THUMB_DEF)
		    printf("[Thumb] ");

	    if((symbols[i].nl.n_type & N_TYPE) == N_INDR)
		printf("%s (for %s)", symbols[i].name, symbols[i].indr_name);
	    else
		printf("%s", symbols[i].name);

	    if((mh_flags & MH_TWOLEVEL) == MH_TWOLEVEL &&
	       (((symbols[i].nl.n_type & N_TYPE) == N_UNDF &&
		 symbols[i].nl.n_value == 0) ||
	        (symbols[i].nl.n_type & N_TYPE) == N_PBUD)){
		library_ordinal = GET_LIBRARY_ORDINAL(symbols[i].nl.n_desc);
		if(library_ordinal != 0){
		    if(library_ordinal == EXECUTABLE_ORDINAL)
			printf(" (from executable)");
		    else if(process_flags->nlibs != DYNAMIC_LOOKUP_ORDINAL &&
			    library_ordinal == DYNAMIC_LOOKUP_ORDINAL)
			printf(" (dynamically looked up)");
		    else if(library_ordinal-1 >= process_flags->nlibs)
			printf(" (from bad library ordinal %u)",
			       library_ordinal);
		    else
			printf(" (from %s)", process_flags->lib_names[
						library_ordinal-1]);
		}
	    }
	    printf("\n");
	}
}

/*
 * print_symbols() is called with the -m flag is not specified and prints
 * symbols in the standard BSD format.
 */
static
void
print_symbols(
struct ofile *ofile,
struct symbol *symbols,
uint32_t nsymbols,
char *strings,
uint32_t strsize,
struct cmd_flags *cmd_flags,
struct process_flags *process_flags,
char *arch_name,
struct value_diff *value_diffs)
{
    uint32_t i;
    unsigned char c;
    char *ta_xfmt, *i_xfmt, *spaces, *dashes;
    const char *p;

	if(ofile->mh != NULL ||
	   (ofile->lto != NULL &&
	    (ofile->lto_cputype & CPU_ARCH_ABI64) != CPU_ARCH_ABI64)){
	    ta_xfmt = "%08llx";
	    i_xfmt =  "%08x";
	    spaces = "        ";
	    dashes = "--------";
	}
	else{
	    ta_xfmt = "%016llx";
	    i_xfmt =  "%016x";
	    spaces = "                ";
	    dashes = "----------------";
	}

	for(i = 0; i < nsymbols; i++){
	    if(cmd_flags->x == TRUE){
		printf(ta_xfmt, symbols[i].nl.n_value);
		printf(" %02x %02x %04x ",
		       (unsigned int)(symbols[i].nl.n_type & 0xff),
		       (unsigned int)(symbols[i].nl.n_sect & 0xff),
		       (unsigned int)(symbols[i].nl.n_desc & 0xffff));
		if(symbols[i].nl.n_un.n_strx == 0){
		    printf(i_xfmt, symbols[i].nl.n_un.n_strx);
		    if(ofile->lto != NULL)
			printf(" %s", symbols[i].name);
		    else
			printf(" (null)");
		}
		else if((uint32_t)symbols[i].nl.n_un.n_strx > strsize){
		    printf("%08x", symbols[i].nl.n_un.n_strx);
		    printf(" (bad string index)");
		}
		else{
		    printf("%08x", symbols[i].nl.n_un.n_strx);
		    printf(" %s", symbols[i].nl.n_un.n_strx + strings);
		}
		if((symbols[i].nl.n_type & N_STAB) == 0 &&
		   (symbols[i].nl.n_type & N_TYPE) == N_INDR){
		    if(symbols[i].nl.n_value == 0){
			printf(" (indirect for ");
			printf(ta_xfmt, symbols[i].nl.n_value);
			printf(" (null))\n");
		    }
		    else if(symbols[i].nl.n_value > strsize){
			printf(" (indirect for ");
			printf(ta_xfmt, symbols[i].nl.n_value);
			printf(" (bad string index))\n");
		    }
		    else{
			printf(" (indirect for ");
			printf(ta_xfmt, symbols[i].nl.n_value);
			printf(" %s)\n", symbols[i].nl.n_value + strings);
		    }
		}
		else
		    printf("\n");
		continue;
	    }
	    if(cmd_flags->P == TRUE){
		if(cmd_flags->A == TRUE){
		    if(arch_name != NULL)
			printf("(for architecture %s): ", arch_name);
		    if(ofile->dylib_module_name != NULL){
			printf("%s[%s]: ", ofile->file_name,
			       ofile->dylib_module_name);
		    }
		    else if(ofile->member_ar_hdr != NULL){
			printf("%s[%.*s]: ", ofile->file_name,
			       (int)ofile->member_name_size,
			       ofile->member_name);
		    }
		    else
			printf("%s: ", ofile->file_name);
		}
		printf("%s ", symbols[i].name);

		/* type */
		c = symbols[i].nl.n_type;
		if(c & N_STAB)
		    c = '-';
		else{
		    switch(c & N_TYPE){
		    case N_UNDF:
			c = 'u';
			if(symbols[i].nl.n_value != 0)
			    c = 'c';
			break;
		    case N_PBUD:
			c = 'u';
			break;
		    case N_ABS:
			c = 'a';
			break;
		    case N_SECT:
			if(symbols[i].nl.n_sect ==
			   process_flags->text_nsect)
			    c = 't';
			else if(symbols[i].nl.n_sect ==
				process_flags->data_nsect)
			    c = 'd';
			else if(symbols[i].nl.n_sect ==
				process_flags->bss_nsect)
			    c = 'b';
			else
			    c = 's';
			break;
		    case N_INDR:
			c = 'i';
			break;
		    default:
			c = '?';
			break;
		    }
		}
		if((symbols[i].nl.n_type & N_EXT) && c != '?')
		    c = toupper(c);
		printf("%c ", c);
		printf(cmd_flags->format, symbols[i].nl.n_value);
		printf(" 0\n"); /* the 0 is the size for conformance */
		continue;
	    }
	    c = symbols[i].nl.n_type;
	    if(c & N_STAB){
		if(cmd_flags->o == TRUE || cmd_flags->A == TRUE){
		    if(arch_name != NULL)
			printf("(for architecture %s):", arch_name);
		    if(ofile->dylib_module_name != NULL){
			printf("%s:%s: ", ofile->file_name,
			       ofile->dylib_module_name);
		    }
		    else if(ofile->member_ar_hdr != NULL){
			printf("%s:%.*s: ", ofile->file_name,
			       (int)ofile->member_name_size,
			       ofile->member_name);
		    }
		    else
			printf("%s: ", ofile->file_name);
		}
		printf(ta_xfmt, symbols[i].nl.n_value);
		printf(" - %02x %04x %5.5s ",
		       (unsigned int)symbols[i].nl.n_sect & 0xff,
		       (unsigned int)symbols[i].nl.n_desc & 0xffff,
		       stab(symbols[i].nl.n_type));
		if(cmd_flags->b == TRUE){
		    for(p = symbols[i].name; *p != '\0'; p++){
			printf("%c", *p);
			if(*p == '('){
			    p++;
			    while(isdigit((unsigned char)*p))
				p++;
			    p--;
			}
			if(*p == '.' && p[1] != '\0' && p[1] == '_'){
			    p++; /* one for the '.' */
			    p++; /* and one for the '_' */
			    while(isdigit((unsigned char)*p))
				p++;
			    p--;
			}
		    }
		    printf("\n");
		}
		else{
		    printf("%s\n", symbols[i].name);
		}
		continue;
	    }
	    switch(c & N_TYPE){
	    case N_UNDF:
		c = 'u';
		if(symbols[i].nl.n_value != 0)
		    c = 'c';
		break;
	    case N_PBUD:
		c = 'u';
		break;
	    case N_ABS:
		c = 'a';
		break;
	    case N_SECT:
		if(symbols[i].nl.n_sect == process_flags->text_nsect)
		    c = 't';
		else if(symbols[i].nl.n_sect == process_flags->data_nsect)
		    c = 'd';
		else if(symbols[i].nl.n_sect == process_flags->bss_nsect)
		    c = 'b';
		else
		    c = 's';
		break;
	    case N_INDR:
		c = 'i';
		break;
	    default:
		c = '?';
		break;
	    }
	    if(cmd_flags->u == TRUE && c != 'u')
		continue;
	    if(cmd_flags->o == TRUE || cmd_flags->A == TRUE){
		if(arch_name != NULL)
		    printf("(for architecture %s):", arch_name);
		if(ofile->dylib_module_name != NULL){
		    printf("%s:%s: ", ofile->file_name,
			   ofile->dylib_module_name);
		}
		else if(ofile->member_ar_hdr != NULL){
		    printf("%s:%.*s: ", ofile->file_name,
			   (int)ofile->member_name_size,
			   ofile->member_name);
		}
		else
		    printf("%s: ", ofile->file_name);
	    }
	    if((symbols[i].nl.n_type & N_EXT) && c != '?')
		c = toupper(c);
	    if(cmd_flags->u == FALSE && cmd_flags->j == FALSE){
		if(c == 'u' || c == 'U' || c == 'i' || c == 'I')
		    printf("%s", spaces);
		else{
		    if(cmd_flags->v && value_diffs != NULL){
			printf(ta_xfmt, value_diffs[i].size);
			printf(" ");
		    }
		    if(ofile->lto)
			printf("%s", dashes);
		    else
			printf(ta_xfmt, symbols[i].nl.n_value);
		}
		printf(" %c ", c);
	    }
	    if(cmd_flags->j == FALSE &&
	       (symbols[i].nl.n_type & N_TYPE) == N_INDR)
		printf("%s (indirect for %s)\n", symbols[i].name,
		       symbols[i].indr_name);
	    else 
		printf("%s\n", symbols[i].name);
	}
}

struct stabnames {
    unsigned char n_type;
    char *name;
};
static const struct stabnames stabnames[] = {
    { N_GSYM,  "GSYM" },
    { N_FNAME, "FNAME" },
    { N_FUN,   "FUN" },
    { N_STSYM, "STSYM" },
    { N_LCSYM, "LCSYM" },
    { N_BNSYM, "BNSYM" },
    { N_AST,   "AST" },
    { N_OPT,   "OPT" },
    { N_RSYM,  "RSYM" },
    { N_SLINE, "SLINE" },
    { N_ENSYM, "ENSYM" },
    { N_SSYM,  "SSYM" },
    { N_SO,    "SO" },
    { N_OSO,   "OSO" },
    { N_LSYM,  "LSYM" },
    { N_BINCL, "BINCL" },
    { N_SOL,   "SOL" },
    { N_PARAMS,"PARAM" },
    { N_VERSION,"VERS" },
    { N_OLEVEL,"OLEV" },
    { N_PSYM,  "PSYM" },
    { N_EINCL, "EINCL" },
    { N_ENTRY, "ENTRY" },
    { N_LBRAC, "LBRAC" },
    { N_EXCL,  "EXCL" },
    { N_RBRAC, "RBRAC" },
    { N_BCOMM, "BCOMM" },
    { N_ECOMM, "ECOMM" },
    { N_ECOML, "ECOML" },
    { N_LENG,  "LENG" },
    { N_PC,    "PC" },
    { 0, 0 }
};

/*
 * stab() returns the name of the specified stab n_type.
 */
static
char *
stab(
unsigned char n_type)
{
    const struct stabnames *p;
    static char prbuf[32];

	for(p = stabnames; p->name; p++)
	    if(p->n_type == n_type)
		return(p->name);
	sprintf(prbuf, "%02x", (unsigned int)n_type);
	return(prbuf);
}

/*
 * compare is the function used by qsort if any sorting of symbols is to be
 * done.
 */
static
int
compare(
struct symbol *p1,
struct symbol *p2)
{
    int r;

	r = 0;
	if(cmd_flags.n == TRUE){
	    if(p1->nl.n_value > p2->nl.n_value)
		return(cmd_flags.r == FALSE ? 1 : -1);
	    else if(p1->nl.n_value < p2->nl.n_value)
		return(cmd_flags.r == FALSE ? -1 : 1);
	    /*
	     * If p1->nl.n_value == p2->nl.n_value fall through
	     * and sort by name
	     */
	}

	if(cmd_flags.x == TRUE && compare_lto == FALSE){
	    if((uint32_t)p1->nl.n_un.n_strx > strsize ||
	       (uint32_t)p2->nl.n_un.n_strx > strsize){
		if((uint32_t)p1->nl.n_un.n_strx > strsize)
		    r = -1;
		else if((uint32_t)p2->nl.n_un.n_strx > strsize)
		    r = 1;
	    }
	    else
		r = strcmp(p1->nl.n_un.n_strx + strings,
			   p2->nl.n_un.n_strx + strings);
	}
	else
	    r = strcmp(p1->name, p2->name);

	if(cmd_flags.r == TRUE)
	    return(-r);
	else
	    return(r);
}

static
int
value_diff_compare(
struct value_diff *p1,
struct value_diff *p2)
{
	if(p1->size < p2->size)
	    return(-1);
	else if(p1->size > p2->size)
	    return(1);
	/* if p1->size == p2->size */
	return(0);
}
