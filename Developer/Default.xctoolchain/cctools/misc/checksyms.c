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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <libc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mach/mach.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/stab.h>
#include "stuff/bool.h"
#include "stuff/ofile.h"
#include "stuff/symbol.h"
#include "stuff/errors.h"
#include "stuff/allocate.h"
#include "stuff/guess_short_name.h"
#include "stuff/macosx_deployment_target.h"

/* used by error routines as the name of the program */
char *progname = NULL;

static int exit_status = EXIT_SUCCESS;

/* flags set from the command line arguments */
struct cmd_flags {
    uint32_t nfiles;
    enum bool rldtype;
    enum bool detail;
    enum bool verification;
    enum bool trey;
    enum bool check_dynamic_binary;
};

static void usage(
    void);

static void checksyms(
    struct ofile *ofile,
    char *arch_name,
    void *cookie);

static void check_dynamic_binary(
    struct ofile *ofile,
    char *arch_name,
    enum bool detail,
    enum bool verification);

static void check_dylib(
    struct ofile *ofile,
    char *arch_name,
    enum bool detail,
    enum bool verification,
    enum bool *debug);

/*
 * The dylib table.  This is specified with the -dylib_table option.
 */
static char *dylib_table_name = NULL;

/*
 * The segment address table.  This is specified with the -seg_addr_table
 * option.
 */
static char *seg_addr_table_name = NULL;
static enum bool seg_addr_table_specified = FALSE;
/*
 * The pathname to use instead of the install name for matching an entry in the
 * segment address table.  his is specified with the -seg_addr_table_filename
 * option.
 */
static char *seg_addr_table_filename = NULL;

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
    struct cmd_flags cmd_flags;
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
	cmd_flags.rldtype = FALSE;
	cmd_flags.detail = FALSE;
	cmd_flags.verification = FALSE;
	cmd_flags.trey = FALSE;
	cmd_flags.check_dynamic_binary = TRUE;

        files = allocate(sizeof(char *) * argc);
	for(i = 1; i < argc; i++){
	    if(argv[i][0] == '-'){
		if(argv[i][1] == '\0'){
		    for( ; i < argc; i++)
			files[cmd_flags.nfiles++] = argv[i];
		    break;
		}
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
		else if(strcmp(argv[i], "-dylib_table") == 0){
		    if(i + 1 == argc){
			error("missing argument(s) to %s option", argv[i]);
			usage();
		    }
		    if(dylib_table_name != NULL){
			error("more than one: %s option", argv[i]);
			usage();
		    }
		    dylib_table_name = argv[i+1];
		    printf("warning: -dylib_table is no longer used.\n");
		    i++;
		}
		else if(strcmp(argv[i], "-seg_addr_table") == 0){
		    if(i + 1 == argc){
			error("missing argument(s) to %s option", argv[i]);
			usage();
		    }
		    if(seg_addr_table_specified == TRUE){
			error("more than one: %s option", argv[i]);
			usage();
		    }
		    seg_addr_table_specified = TRUE;
		    seg_addr_table_name = argv[i+1];
		    printf("warning: -seg_addr_table is no longer used.\n");
		    i++;
		}
		else if(strcmp(argv[i], "-seg_addr_table_filename") == 0){
		    if(i + 1 == argc){
			error("missing argument(s) to %s option", argv[i]);
			usage();
		    }
		    if(seg_addr_table_filename != NULL){
			error("more than one: %s option", argv[i]);
			usage();
		    }
		    seg_addr_table_filename = argv[i+1];
		    printf("warning: -seg_addr_table_filename is no longer "
			   "used.\n");
		    i++;
		}
		else{
		    for(j = 1; argv[i][j] != '\0'; j++){
			switch(argv[i][j]){
			case 'r':
			    cmd_flags.rldtype = TRUE;
			    break;
			case 'd':
			    cmd_flags.detail = TRUE;
			    break;
			case 'v':
			    cmd_flags.verification = TRUE;
			    break;
			case 't':
			    cmd_flags.trey = TRUE;
			    break;
			case 'b':
			    cmd_flags.check_dynamic_binary = TRUE;
			    break;
			default:
			    error("invalid argument -%c", argv[i][j]);
			    usage();
			}
		    }
		}
		continue;
	    }
	    files[cmd_flags.nfiles++] = argv[i];
	}

	if(arch_flags == NULL)
	    all_archs = TRUE;

	if(cmd_flags.nfiles != 1)
	    usage();

	for(j = 0; j < cmd_flags.nfiles; j++)
	    ofile_process(files[j], arch_flags, narch_flags, all_archs, TRUE,
			  TRUE, FALSE, checksyms, &cmd_flags);

	if(errors == 0)
	    return(exit_status);
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
	fprintf(stderr, "Usage: %s [-r] [-d] [-t] [-b] [-] [-dylib_table file] "
		"[-seg_addr_table file] [-seg_addr_table_filename pathname] "
		"[[-arch <arch_flag>] ...] file\n",
		progname);
	exit(EXIT_FAILURE);
}

/*
 * checksyms() is the routine that gets called by ofile_process() to process
 * single object files.
 */
static
void
checksyms(
struct ofile *ofile,
char *arch_name,
void *cookie)
{
    struct cmd_flags *cmd_flags;
    uint32_t i, mh_flags, mh_ncmds, n_type;
    struct load_command *lc;
    struct symtab_command *st;
    struct nlist *symbols;
    struct nlist_64 *symbols64;
    struct symbol *syms;
    uint32_t nsymbols;
    char *strings;
    uint32_t strsize;
    uint32_t nfiledefs, ncats, nlocal, nstabs, nfun;
    uint32_t filedef_strings, cat_strings, local_strings, stab_strings;
    enum bool debug;

	if(ofile->mh != NULL){
	    mh_flags = ofile->mh->flags;
	    mh_ncmds = ofile->mh->ncmds;
	}
	else if(ofile->mh64 != NULL){
	    mh_flags = ofile->mh64->flags;
	    mh_ncmds = ofile->mh64->ncmds;
	}
	else {
	    printf("internal error: no mach header\n");
	    exit_status = EXIT_FAILURE;
	    return;
	}

	debug = FALSE;
	cmd_flags = (struct cmd_flags *)cookie;

	if(cmd_flags->check_dynamic_binary == TRUE)
	    if((mh_flags & MH_DYLDLINK) == MH_DYLDLINK &&
		ofile->mh_filetype != MH_KEXT_BUNDLE)
		check_dynamic_binary(ofile, arch_name, cmd_flags->detail,
				     cmd_flags->verification);

	if(ofile->mh_filetype == MH_DYLIB ||
	   ofile->mh_filetype == MH_DYLIB_STUB)
	    check_dylib(ofile, arch_name, cmd_flags->detail,
			cmd_flags->verification, &debug);

	st = NULL;
	lc = ofile->load_commands;
	for(i = 0; i < mh_ncmds; i++){
	    if(st == NULL && lc->cmd == LC_SYMTAB){
		st = (struct symtab_command *)lc;
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	if(st == NULL || st->nsyms == 0){
	    return;
	}

	if(cmd_flags->rldtype == FALSE &&
	   cmd_flags->trey == FALSE &&
	   (mh_flags & MH_DYLDLINK) == 0 &&
	    (ofile->file_type != OFILE_FAT ||
	     ofile->arch_type != OFILE_ARCHIVE) &&
	    ofile->file_type != OFILE_ARCHIVE &&
	    ofile->mh_filetype != MH_FVMLIB){
	    if(st->nsyms == 0)
		return;

	    if(cmd_flags->detail == TRUE){
		if(arch_name != NULL)
		    printf("(for architecture %s):", arch_name);
		if(ofile->member_ar_hdr != NULL){
		    printf("%s:%.*s:", ofile->file_name,
			   (int)ofile->member_name_size, ofile->member_name);
		}
		else
		    printf("%s:", ofile->file_name);
		printf(" has %u symbols and %u string bytes\n", st->nsyms,
		       st->strsize);
	    }
	    if(cmd_flags->verification == TRUE)
		printf("unstripped_binary\n");
	    exit_status = EXIT_FAILURE;
	    return;
	}

	nsymbols = st->nsyms;
	symbols = NULL;
	symbols64 = NULL;
	if(ofile->mh != NULL){
	    symbols = (struct nlist *)(ofile->object_addr + st->symoff);
	    if(ofile->object_byte_sex != get_host_byte_sex())
		swap_nlist(symbols, nsymbols, get_host_byte_sex());
	}
	else{
	    symbols64 = (struct nlist_64 *)(ofile->object_addr + st->symoff);
	    if(ofile->object_byte_sex != get_host_byte_sex())
		swap_nlist_64(symbols64, nsymbols, get_host_byte_sex());
	}
	syms = allocate(nsymbols * sizeof(struct symbol));

	strings = ofile->object_addr + st->stroff;
	strsize = st->strsize;
	for(i = 0; i < nsymbols; i++){
	    if(ofile->mh != NULL){
		if(symbols[i].n_un.n_strx == 0)
		    syms[i].name = "";
		else if((uint32_t)symbols[i].n_un.n_strx > st->strsize)
		    syms[i].name = "bad string index";
		else
		    syms[i].name = symbols[i].n_un.n_strx + strings;

		if((symbols[i].n_type & N_TYPE) == N_INDR){
		    if(symbols[i].n_value == 0)
			syms[i].indr_name = NULL;
		    else if(symbols[i].n_value > st->strsize)
			syms[i].indr_name = "bad string index";
		    else
			syms[i].indr_name = strings + symbols[i].n_value;
		}
		syms[i].n_value = symbols[i].n_value;
	    }
	    else{
		if(symbols64[i].n_un.n_strx == 0)
		    syms[i].name = "";
		else if((int)symbols64[i].n_un.n_strx < 0 ||
			(uint32_t)symbols64[i].n_un.n_strx > st->strsize)
		    syms[i].name = "bad string index";
		else
		    syms[i].name = symbols64[i].n_un.n_strx + strings;

		if((symbols64[i].n_type & N_TYPE) == N_INDR){
		    if(symbols64[i].n_value == 0)
			syms[i].indr_name = NULL;
		    else if(symbols64[i].n_value > st->strsize)
			syms[i].indr_name = "bad string index";
		    else
			syms[i].indr_name = strings + symbols64[i].n_value;
		}
		syms[i].n_value = symbols64[i].n_value;
	    }
	}

	nfiledefs = 0;
	ncats = 0;
	nlocal = 0;
	nstabs = 0;
	nfun = 0;
	filedef_strings = 0;
	cat_strings = 0;
	local_strings = 0;
	stab_strings = 0;
	for(i = 0; i < nsymbols; i++){
	    if(ofile->mh != NULL)
		n_type = symbols[i].n_type;
	    else
		n_type = symbols64[i].n_type;
	    if(ofile->mh_filetype == MH_EXECUTE){
		if(n_type == (N_ABS | N_EXT) && syms[i].n_value == 0){
		    if(strncmp(syms[i].name, ".file_definition_",
			       sizeof(".file_definition_") - 1) == 0){
			nfiledefs++;
			filedef_strings += strlen(syms[i].name);
		    }
		    if(strncmp(syms[i].name, ".objc_category_name_",
			       sizeof(".objc_category_name_") - 1) == 0){
			ncats++;
			cat_strings += strlen(syms[i].name);
		    }
		}
	    }
	    /*
	     * We need to allow the symbol created by strip(1) for radar bug
	     * 5614542 (see that radar and related for more information).
	     */
	    if(strcmp(syms[i].name, "radr://5614542") != 0){
		if((n_type & N_EXT) == 0){
		    nlocal++;
		    local_strings += strlen(syms[i].name);
		}
		if(n_type & N_STAB){
		    nstabs++;
		    stab_strings += strlen(syms[i].name);
		    if(n_type == N_FUN)
			nfun++;
		}
	    }
	}

	if(nfiledefs == 0 && ncats == 0 && nlocal == 0 && nstabs == 0)
	    return;
	if(cmd_flags->rldtype == TRUE && nstabs == 0)
	    return;
	if((mh_flags & MH_DYLDLINK) == MH_DYLDLINK &&
	   (nstabs == 0 && nlocal == 0))
	    return;
	if(nstabs == 0 &&
	   ((ofile->file_type == OFILE_FAT &&
	     ofile->arch_type == OFILE_ARCHIVE) ||
	    ofile->file_type == OFILE_ARCHIVE ||
	    ofile->mh_filetype == MH_FVMLIB))
	    return;
	if((ofile->mh_filetype == MH_DYLIB ||
	    ofile->mh_filetype == MH_DYLIB_STUB ||
	    ofile->mh_filetype == MH_FVMLIB) &&
	    (nfun == 0 || debug == TRUE))
	    return;

	if(cmd_flags->detail == TRUE){
	    if(arch_name != NULL)
		printf("(for architecture %s):", arch_name);
	    if(ofile->member_ar_hdr != NULL){
		printf("%s:%.*s:", ofile->file_name,
		       (int)ofile->member_name_size, ofile->member_name);
	    }
	    else
		printf("%s:", ofile->file_name);
	    printf("\n");
	    if(nfiledefs != 0)
		printf(" has %u .file_definition_ symbols and %u string "
		       "bytes\n", nfiledefs, filedef_strings);
	    if(ncats != 0)
		printf(" has %u .objc_category_name_ symbols and %u string "
		       "bytes\n", ncats, cat_strings);
	    if(nlocal != 0)
		printf(" has %u local symbols and %u string "
		       "bytes\n", nlocal, local_strings);
	    if(nstabs != 0)
		printf(" has %u debugging symbols and %u string "
		       "bytes\n", nstabs, stab_strings);
	}
	if(cmd_flags->verification == TRUE)
	    printf("unstripped_binary\n");
	if(cmd_flags->trey == TRUE && nstabs == 0)
	    return;

	exit_status = EXIT_FAILURE;
	return;
}

/*
 * check_dynamic_binary checks to see a dynamic is built correctly.  That is it
 * has no read-only-relocs, and is prebound.
 */
static
void
check_dynamic_binary(
struct ofile *ofile,
char *arch_name,
enum bool detail,
enum bool verification)
{
    uint32_t i, j, section_attributes, section_type, mh_flags, mh_ncmds;
    struct load_command *lc;
    struct segment_command *sg;
    struct segment_command_64 *sg64;
    struct section *s;
    struct section_64 *s64;
    struct macosx_deployment_target macosx_deployment_target;

	if(ofile->mh != NULL){
	    mh_ncmds = ofile->mh->ncmds;
	    mh_flags = ofile->mh->flags;
	}
	else{
	    mh_ncmds = ofile->mh64->ncmds;
	    mh_flags = ofile->mh64->flags;
	}
	/*
	 * First check for relocation entries in read only segments.
	 */
	lc = ofile->load_commands;
	for(i = 0; i < mh_ncmds; i++){
	    if(lc->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc;
		s = (struct section *)((char *)lc +
					sizeof(struct segment_command));
		for(j = 0; j < sg->nsects; j++){
		    section_attributes = s->flags & SECTION_ATTRIBUTES;
		    section_type = s->flags & SECTION_TYPE;
		    if((sg->initprot & VM_PROT_WRITE) == 0 &&
		       ((section_attributes & S_ATTR_EXT_RELOC) != 0 ||
		        (section_attributes & S_ATTR_LOC_RELOC) != 0)){
			/* read-only relocs are ok in i386 and arm stubs and 
			    stub helper sections */
			if((ofile->mh != NULL) && 
			   (ofile->mh->cputype == CPU_TYPE_I386 ||
			    ofile->mh->cputype == CPU_TYPE_ARM) &&
			   (section_type == S_SYMBOL_STUBS ||
			    (strcmp(s->sectname,"__symbol_stub")==0) ||
			    (strcmp(s->sectname,"__stub_helper")==0)))
				continue;
			if(detail == TRUE){
			    if(arch_name != NULL)
				printf("(for architecture %s):", arch_name);
			    printf("%s: relocation entries in read-only section"
				   " (%.16s,%.16s)\n", ofile->file_name,
				   s->segname, s->sectname);
			}
			if(verification == TRUE)
			    printf("read_only_relocs\n");
			exit_status = EXIT_FAILURE;
		    }
		    s++;
		}
	    }
	    else if(lc->cmd == LC_SEGMENT_64){
		sg64 = (struct segment_command_64 *)lc;
		s64 = (struct section_64 *)((char *)lc +
					sizeof(struct segment_command_64));
		for(j = 0; j < sg64->nsects; j++){
		    section_attributes = s64->flags & SECTION_ATTRIBUTES;
		    if((sg64->initprot & VM_PROT_WRITE) == 0 &&
		       ((section_attributes & S_ATTR_EXT_RELOC) != 0 ||
		        (section_attributes & S_ATTR_LOC_RELOC) != 0)){
			if(detail == TRUE){
			    if(arch_name != NULL)
				printf("(for architecture %s):", arch_name);
			    printf("%s: relocation entries in read-only section"
				   " (%.16s,%.16s)\n", ofile->file_name,
				   s64->segname, s64->sectname);
			}
			if(verification == TRUE)
			    printf("read_only_relocs\n");
			exit_status = EXIT_FAILURE;
		    }
		    s64++;
		}
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}

	/*
	 * If the file is 32-bit and an executable or a dynamic library and has
	 * no undefined references it should be prebound unless
	 * MACOSX_DEPLOYMENT_TARGET is 10.4 or greater.
	 */
	if(ofile->mh64 != NULL)
	    return;
	get_macosx_deployment_target(&macosx_deployment_target);
	if(macosx_deployment_target.major >= 4)
	    return;

	if((ofile->mh_filetype == MH_EXECUTE ||
	    ofile->mh_filetype == MH_DYLIB ||
	    ofile->mh_filetype == MH_DYLIB_STUB) &&
	   (mh_flags & MH_NOUNDEFS) == MH_NOUNDEFS){

	    if((mh_flags & MH_PREBOUND) != MH_PREBOUND){
		if(detail == TRUE){
		    if(arch_name != NULL)
			printf("(for architecture %s):", arch_name);
		    printf("%s: is not prebound\n", ofile->file_name);
		}
		if(verification == TRUE)
		    printf("not_prebound\n");
		exit_status = EXIT_FAILURE;
	    }
	}
}

/*
 * check_dylib() checks the dynamic library against the Apple conventions.
 * This includes
ifdef CHECK_ADDRESS
 * the linked address and
endif CHECK_ADDRESS
 * the setting of compatibility and current versions.
 */
static
void
check_dylib(
struct ofile *ofile,
char *arch_name,
enum bool detail,
enum bool verification,
enum bool *debug)
{
    uint32_t i, seg1addr, segs_read_only_addr, segs_read_write_addr, mh_flags,
	     mh_ncmds;
    struct load_command *lc;
    struct segment_command *sg;
    struct dylib_command *dlid;
    char *install_name;
#ifdef CHECK_ADDRESS
    uint32_t table_size;
    char *suffix;
    struct seg_addr_table *entry;
    char *short_name;
    enum bool is_framework;
#endif /* CHECK_ADDRESS */

	*debug = FALSE;

	/*
	 * First pick up the linked address (for 32-bit dylibs) and the dylib
	 * id command.
	 */
	seg1addr = UINT_MAX;
	segs_read_only_addr = UINT_MAX;
	segs_read_write_addr = UINT_MAX;
	dlid = NULL;
	install_name = NULL;
	lc = ofile->load_commands;
	if(ofile->mh != NULL){
	    mh_ncmds = ofile->mh->ncmds;
	    mh_flags = ofile->mh->flags;
	}
	else{
	    mh_ncmds = ofile->mh64->ncmds;
	    mh_flags = ofile->mh64->flags;
	}
	for(i = 0; i < mh_ncmds; i++){
	    switch(lc->cmd){
	    case LC_SEGMENT:
		sg = (struct segment_command *)lc;
		if(mh_flags & MH_SPLIT_SEGS){
		    if((sg->initprot & VM_PROT_WRITE) == 0){
			if(sg->vmaddr < segs_read_only_addr)
			    segs_read_only_addr = sg->vmaddr;
		    }
		    else{
			if(sg->vmaddr < segs_read_write_addr)
			    segs_read_write_addr = sg->vmaddr;
		    }
		}
		else{
		    if(sg->vmaddr < seg1addr)
			seg1addr = sg->vmaddr;
		}
		break;
	    case LC_ID_DYLIB:
		if(dlid == NULL){
		    dlid = (struct dylib_command *)lc;
		    install_name = (char *)dlid + dlid->dylib.name.offset;
		}
		break;
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	if(dlid == NULL){
	    if(ofile->mh_filetype != MH_DYLIB_STUB){
		printf("%s: ", ofile->file_name);
		if(arch_name != NULL)
		    printf("(for architecture %s): ", arch_name);
		printf("malformed dynamic library (no LC_ID_DYLIB command)\n");
		exit_status = EXIT_FAILURE;
	    }
	    return;
	}

	/*
	 * Check for compatibility and current version being set (non-zero).
	 */
	if(dlid->dylib.compatibility_version == 0){
	    if(detail == TRUE){
		printf("%s: ", ofile->file_name);
		if(arch_name != NULL)
		    printf("(for architecture %s): ", arch_name);
		printf("compatibility_version for dynamic library not set\n");
	    }
	    if(verification == TRUE)
		printf("no_compatibility_version\n");
	    exit_status = EXIT_FAILURE;
	}
	if(dlid->dylib.current_version == 0){
	    if(detail == TRUE){
		printf("%s: ", ofile->file_name);
		if(arch_name != NULL)
		    printf("(for architecture %s): ", arch_name);
		printf("current_version for dynamic library not set\n");
	    }
	    if(verification == TRUE)
		printf("no_current_version\n");
	    exit_status = EXIT_FAILURE;
	}

	/*
	 * With the change in B&I to slide addresses at make logical time this
	 * check is no longer needed and this code is not used.
	 */
#ifdef CHECK_ADDRESS
	short_name = guess_short_name(install_name, &is_framework, &suffix);
	if(suffix != NULL && strcmp(suffix, "_debug") == 0)
	    *debug = TRUE;
	else
	    *debug = FALSE;

	/*
	 * If there is no -seg_addr_table argument then open the default segment
	 * address table.
	 */
	if(seg_addr_table == NULL)
	    seg_addr_table = parse_default_seg_addr_table(
				&seg_addr_table_name, &table_size);
	/*
	 * Use the segment address table if there is one to check the addresses
	 * if not fall back to using the the dylib table.
	 */
	if(seg_addr_table != NULL){
	    if(seg_addr_table_filename != NULL)
		entry = search_seg_addr_table(seg_addr_table,
					      seg_addr_table_filename);
	    else
		entry = search_seg_addr_table(seg_addr_table, install_name);
	    if(entry != NULL){
		if(entry->split == TRUE){
		    if(entry->segs_read_only_addr != segs_read_only_addr ||
		       entry->segs_read_write_addr != segs_read_write_addr){
			if(detail == TRUE){
			    printf("%s: ", ofile->file_name);
			    if(arch_name != NULL)
				printf("(for architecture %s): ", arch_name);
			    printf("dynamic library (%s) not linked at its "
				   "expected segs_read_only_addr (0x%x) and "
				   "segs_read_write_addr (0x%x)\n",
				   seg_addr_table_filename != NULL ?
				   seg_addr_table_filename : install_name, 
				   (unsigned int)entry->segs_read_only_addr,
				   (unsigned int)entry->segs_read_write_addr);
			}
			if(verification == TRUE)
			    printf("dylib_wrong_address\n");
			exit_status = EXIT_FAILURE;
		    }
		    return;
		}
		else{
		    if(entry->seg1addr != seg1addr){
			if(detail == TRUE){
			    printf("%s: ", ofile->file_name);
			    if(arch_name != NULL)
				printf("(for architecture %s): ", arch_name);
			    printf("dynamic library (%s) not linked at its "
				   "expected seg1addr (0x%x)\n",
				   seg_addr_table_filename != NULL ?
				   seg_addr_table_filename : install_name, 
				   (unsigned int)entry->seg1addr);
			}
			if(verification == TRUE)
			    printf("dylib_wrong_address\n");
			exit_status = EXIT_FAILURE;
		    }
		    return;
		}
	    }
	    else{
		/*
		 * This case is that there was a segment address table but this
		 * library did not have an entry in it.  In this case the B&I
		 * tools will automaticly add it to the table and assign it an
		 * address.  So NO error is to be generated in this case and
		 * the dylib table is not to be used as the segment address
		 * table exists.
		 */
		return;
	    }
	}

	/*
	 * If there is no dylib table open the default table and use it.
	 */
	if(dylib_table == NULL)
	    dylib_table = parse_default_dylib_table(&dylib_table_name);

	/*
	 * Now short_name points to the name of the library.  This short_name 
	 * must be found in the table and the seg1addr must match with what is
	 * in the table.
	 */
	if(short_name != NULL){
	    for(i = 0; dylib_table[i].name != NULL; i++){
		if(strcmp(dylib_table[i].name, short_name) == 0)
		    break;
	    }
	}
	if(short_name == NULL || dylib_table[i].name == NULL){
	    if(detail == TRUE){
		printf("%s: ", ofile->file_name);
		if(arch_name != NULL)
		    printf("(for architecture %s): ", arch_name);
		printf("dynamic library name (%s) unknown to %s and "
		    "checking of its seg1addr can't be done ",
		    short_name != NULL ? short_name : install_name, progname);
		printf("(dylib table: %s must be updated to include this "
		       "library, contact Build & Integration to assign an "
		       "address to this library)\n", dylib_table_name);
	    }
	    if(verification == TRUE)
		printf("unknown_dylib\n");
	    exit_status = EXIT_FAILURE;
	}
	else if(dylib_table[i].seg1addr != seg1addr){
	    if(detail == TRUE){
		printf("%s: ", ofile->file_name);
		if(arch_name != NULL)
		    printf("(for architecture %s): ", arch_name);
		printf("dynamic library (%s) not linked at its expected "
		       "seg1addr (0x%x)\n", short_name,
		       (unsigned int)dylib_table[i].seg1addr);
	    }
	    if(verification == TRUE)
		printf("dylib_wrong_address\n");
	    exit_status = EXIT_FAILURE;
	}
#endif /* CHECK_ADDRESS */

	return;
}
