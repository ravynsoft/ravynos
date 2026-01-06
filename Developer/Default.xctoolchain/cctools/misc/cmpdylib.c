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
#import <stdio.h>
#import <stdlib.h>

#import "stuff/ofile.h"
#import "stuff/errors.h"
#import "stuff/bytesex.h"
#import "stuff/allocate.h"

/* name of the program for error messages (argv[0]) */
__private_extern__ char *progname = NULL;

/* The filenames of the old and new dylib */
static char *old_dylib = NULL;
static char *new_dylib = NULL;

/*
 * This is a flag use by process_old() and compare() to make sure each
 * architecture in the old library is in the new library.
 */
static enum bool arch_processed = FALSE;
static char *arch_name_being_processed = NULL;

/* The result of the dylib comparison */
static enum bool compatible = TRUE;

/* the byte sex of the machine this program is running on */
static enum byte_sex host_byte_sex = UNKNOWN_BYTE_SEX;

/*
 * These are pointers to strings and symbols used to search of the table of
 * contents of a library.  These have to be static and not local so that
 * check_global_symbols() can set them and that dylib_bsearch() can use them.
 */
static char *strings = NULL;
static struct nlist *symbols = NULL;
static struct nlist_64 *symbols64 = NULL;

static void process_old(
    struct ofile *ofile,
    char *arch_name,
    void *cookie);

static void compare(
    struct ofile *new_ofile,
    char *arch_name,
    void *cookie);

static void check_dylib(
    struct ofile *ofile,
    char *arch_name);

static void check_global_symbols(
    struct ofile *new_ofile,
    struct ofile *old_ofile,
    char *arch_name,
    enum bool new_api_allowed);

static int dylib_bsearch(
    const char *symbol_name,
    const struct dylib_table_of_contents *toc);

static int dylib_bsearch64(
    const char *symbol_name,
    const struct dylib_table_of_contents *toc);

static int nlist_bsearch(
    const char *symbol_name,
    const struct nlist *sym);

static int nlist_bsearch64(
    const char *symbol_name,
    const struct nlist_64 *sym64);

/* apple_version is created by the libstuff/Makefile */
extern char apple_version[];
char *version = apple_version;

/*
 * The program cmpdylib.  This compares an old and an new dynamic shared library
 * for compatiblity.  Usage:
 *
 *	cmpdylib old_dylib new_dylib
 * 
 * It exits non-zero for incompatible libraries and prints why the libraries are
 * incompatible.  It exit zero and prints nothing for compatible libraries.
 */
int
main(
int argc,
char *argv[],
char *envp[])
{
	progname = argv[0];
	host_byte_sex = get_host_byte_sex();

	if(argc != 3){
	    fprintf(stderr, "Usage: %s old_dylib new_dylib\n", progname);
	    exit(EXIT_FAILURE);
	}
	old_dylib = argv[1];
	new_dylib = argv[2];

	ofile_process(old_dylib, NULL, 0, TRUE, TRUE, TRUE, FALSE, process_old, 
		      NULL);

	if(compatible == TRUE)
	    return(EXIT_SUCCESS);
	else
	    return(EXIT_FAILURE);
}

/*
 * process_old() is called once for each architecture in the old dynamic shared
 * library and then causes compare() to be called for the same architecture
 * in the new dynamic shared library.
 */
static
void
process_old(
struct ofile *old_ofile,
char *arch_name,
void *cookie)
{
    struct arch_flag arch_flag;

	/* check to make sure this is a dynamic shared library */
	check_dylib(old_ofile, arch_name);

	/* fill in the architecure info */
	arch_flag.name = (char *)get_arch_name_from_types(
						old_ofile->mh_cputype,
						old_ofile->mh_cpusubtype);
	arch_flag.cputype = old_ofile->mh_cputype;
	arch_flag.cpusubtype = old_ofile->mh_cpusubtype;

	arch_processed = FALSE;
	arch_name_being_processed = arch_name;

	ofile_process(new_dylib, &arch_flag, 1, FALSE, TRUE, TRUE, FALSE,
		      compare, old_ofile);

	if(arch_processed == FALSE)
	    fatal("new dynamic shared library: %s  does not contain "
		  "architecture %s\n", new_dylib, arch_flag.name);
}

/*
 * compare() checks the new dynamic shared library against the old one for the
 * same architecture.  The old dynamic shared library's ofile struct is passed
 * as the cookie.
 */
static
void
compare(
struct ofile *new_ofile,
char *arch_name,
void *cookie)
{
    uint32_t i;
    struct load_command *lc;
    struct ofile *old_ofile;
    struct dylib_command *old_dl, *new_dl;
    char *old_install_name, *new_install_name;
    enum bool new_api_allowed;
    uint32_t ncmds;

	arch_name = arch_name_being_processed;

	/* check to make sure this is a dynamic shared library */
	check_dylib(new_ofile, arch_name);

	old_ofile = (struct ofile *)cookie;

	/* Get the LC_ID_DYLIB from the old dylib */
	old_dl = NULL;
	lc = old_ofile->load_commands;
	if(old_ofile->mh != NULL)
	    ncmds = old_ofile->mh->ncmds;
	else
	    ncmds = old_ofile->mh64->ncmds;
	for(i = 0; i < ncmds; i++){
	    if(old_dl == NULL && lc->cmd == LC_ID_DYLIB){
		old_dl = (struct dylib_command *)lc;
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	if(old_dl == NULL){
	    if(arch_name != NULL)
		fatal("malformed dynamic shared library: %s (for architecture "
		      "%s) (has no dylib id command)", old_dylib, arch_name);
	    else
		fatal("malformed dynamic shared library: %s (has no dylib id "
		      "command)", old_dylib);
	}
	old_install_name = (char *)old_dl + old_dl->dylib.name.offset;
	if(old_dl->dylib.current_version <
	   old_dl->dylib.compatibility_version){
	    if(arch_name != NULL)
		fatal("malformed dynamic shared library: %s (for architecture "
		      "%s) (current version less than compatibility_version)",
		      old_dylib, arch_name);
	    else
		fatal("malformed dynamic shared library: %s (current version "
		      "less than compatibility_version)", old_dylib);
	}

	/* Get the LC_ID_DYLIB from the new dylib */
	new_dl = NULL;
	lc = new_ofile->load_commands;
	if(new_ofile->mh != NULL)
	    ncmds = new_ofile->mh->ncmds;
	else
	    ncmds = new_ofile->mh64->ncmds;
	for(i = 0; i < ncmds; i++){
	    if(new_dl == NULL && lc->cmd == LC_ID_DYLIB){
		new_dl = (struct dylib_command *)lc;
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	if(new_dl == NULL){
	    if(arch_name != NULL)
		fatal("malformed dynamic shared library: %s (for architecture "
		      "%s) (has no dylib id command)", new_dylib, arch_name);
	    else
		fatal("malformed dynamic shared library: %s (has no dylib id "
		      "command)", new_dylib);
	}
	new_install_name = (char *)new_dl + new_dl->dylib.name.offset;
	if(new_dl->dylib.current_version <
	   new_dl->dylib.compatibility_version){
	    if(arch_name != NULL)
		fatal("malformed dynamic shared library: %s (for architecture "
		      "%s) (current version less than compatibility_version)",
		      new_dylib, arch_name);
	    else
		fatal("malformed dynamic shared library: %s (current version "
		      "less than compatibility_version)", new_dylib);
	}

	/* check the values of the LC_ID_DYLIB's */
	if(strcmp(old_install_name, new_install_name) != 0){
	    if(arch_name != NULL)
		printf("For architecture %s ", arch_name);
	    printf("dynamic shared libraries have different install names (%s "
		   "and %s)\n", old_install_name, new_install_name);
	    compatible = FALSE;
	}
	if(old_dl->dylib.current_version >
	   new_dl->dylib.current_version){
	    if(arch_name != NULL)
		printf("For architecture %s ", arch_name);
	    printf("current version of old dynamic shared library (%u) "
		   "greater than new dynamic shared library (%u)\n",
		   old_dl->dylib.current_version,new_dl->dylib.current_version);
	    compatible = FALSE;
	}
	if(old_dl->dylib.compatibility_version >
	   new_dl->dylib.compatibility_version){
	    if(arch_name != NULL)
		printf("For architecture %s ", arch_name);
	    printf("compatibility version of old dynamic shared library (%u) "
		   "greater than new dynamic shared library (%u)\n",
		   old_dl->dylib.compatibility_version,
		   new_dl->dylib.compatibility_version);
	    compatible = FALSE;
	    new_api_allowed = TRUE;
	}
	else{
	    if(new_dl->dylib.compatibility_version !=
	       old_dl->dylib.compatibility_version)
		new_api_allowed = TRUE;
	    else
		new_api_allowed = FALSE;
	}

	check_global_symbols(new_ofile, old_ofile, arch_name, new_api_allowed);

	arch_processed = TRUE;
}

/*
 * check_dylib() checks to make sure this is a dynamic shared library.  If not
 * it prints an error and exits.
 */
static
void
check_dylib(
struct ofile *ofile,
char *arch_name)
{
	if(ofile->file_type == OFILE_FAT){
	    if(ofile->arch_type != OFILE_Mach_O ||
	       (ofile->mh_filetype != MH_DYLIB &&
	        ofile->mh_filetype != MH_DYLIB_STUB))
	    fatal("for architecture %s file: %s is not a dynamic shared "
		  "library", ofile->arch_flag.name, ofile->file_name);
	}
	else
	    if(ofile->file_type != OFILE_Mach_O ||
	       (ofile->mh_filetype != MH_DYLIB &&
	        ofile->mh_filetype != MH_DYLIB_STUB))
		fatal("file: %s is not a dynamic shared library",
		      ofile->file_name);
}

/*
 * check_global_symbols() checks to see if all the global symbols defined in the
 * old library are defined in the new library.  If not it prints the ones not
 * defined and sets compatible to FALSE.  If new_api_allowed is FALSE then it
 * checks to see if there are any global symbols that are in the new library
 * that are not in the old library.  If so it prints those and sets compatible
 * to FALSE.
 */
static
void
check_global_symbols(
struct ofile *new_ofile,
struct ofile *old_ofile,
char *arch_name,
enum bool new_api_allowed)
{
    uint32_t i;
    struct load_command *lc;
    enum bool new_api, missing_symbols, found;

    struct symtab_command *old_st, *new_st;
    struct dysymtab_command *old_dyst, *new_dyst;
    struct nlist *old_symbols, *new_symbols, *sym;
    struct nlist_64 *old_symbols64, *new_symbols64, *sym64;
    char *old_strings, *new_strings;
    struct dylib_table_of_contents *old_tocs, *new_tocs, *toc;
    char *symbol_name;
    uint32_t ncmds, n_strx;

	/*
	 * Pickup the symbolic info for the old dylib.
	 */
	old_st = NULL;
	old_dyst = NULL;
	lc = old_ofile->load_commands;
	if(old_ofile->mh != NULL)
	    ncmds = old_ofile->mh->ncmds;
	else
	    ncmds = old_ofile->mh64->ncmds;
	for(i = 0; i < ncmds; i++){
	    if(old_st == NULL && lc->cmd == LC_SYMTAB){
		old_st = (struct symtab_command *)lc;
	    }
	    else if(old_dyst == NULL && lc->cmd == LC_DYSYMTAB){
		old_dyst = (struct dysymtab_command *)lc;
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	if(old_st == NULL || old_st->nsyms == 0){
	    if(arch_name != NULL)
		fatal("old dynamic shared library: %s (for architecture %s) "
		      "has no symbol table", old_dylib, arch_name);
	    else
		fatal("old dynamic shared library: %s has no symbol table",
		      old_dylib);
	}
	if(old_ofile->mh != NULL){
	    old_symbols = (struct nlist *)
		(old_ofile->object_addr + old_st->symoff);
	    old_symbols64 = NULL;
	}
	else{
	    old_symbols64 = (struct nlist_64 *)
		(old_ofile->object_addr + old_st->symoff);
	    old_symbols = NULL;
	}
	old_tocs = (struct dylib_table_of_contents *)
		(old_ofile->object_addr + old_dyst->tocoff);
	old_strings = (char *)
		(old_ofile->object_addr + old_st->stroff);
	if(old_ofile->object_byte_sex != host_byte_sex){
	    if(old_ofile->mh != NULL)
		swap_nlist(old_symbols, old_st->nsyms, host_byte_sex);
	    else
		swap_nlist_64(old_symbols64, old_st->nsyms, host_byte_sex);
	    swap_dylib_table_of_contents(old_tocs, old_dyst->ntoc,
		    host_byte_sex);
	}
	for(i = 0; i < old_st->nsyms; i++){
	    if(old_ofile->mh != NULL)
		n_strx = old_symbols[i].n_un.n_strx;
	    else
		n_strx = old_symbols64[i].n_un.n_strx;
	    if(n_strx != 0 && n_strx > old_st->strsize){
		if(arch_name != NULL)
		    fatal("malformed dynamic shared library: %s (for "
			"architecture %s) (bad string table index for symbol "
			"%u)", old_dylib, arch_name, i);
		else
		    fatal("malformed dynamic shared library: %s (bad string "
			"table index for symbol %u)", old_dylib, i);
	    }
	}
	for(i = 0; i < old_dyst->ntoc; i++){
	    if(old_tocs[i].symbol_index > old_st->nsyms){
		if(arch_name != NULL)
		    fatal("malformed dynamic shared library: %s (for "
			"architecture %s) (symbol_index field of table of "
			"contents entry %u past the end of the symbol table)",
			old_dylib, arch_name, i);
		else
		    fatal("malformed dynamic shared library: %s (symbol_index "
			"field of table of contents entry %u past the end of "
			"the symbol table)", old_dylib, i);
	    }
	}

	/*
	 * Pickup the symbolic info for the new dylib.
	 */
	new_st = NULL;
	new_dyst = NULL;
	lc = new_ofile->load_commands;
	if(new_ofile->mh != NULL)
	    ncmds = new_ofile->mh->ncmds;
	else
	    ncmds = new_ofile->mh64->ncmds;
	for(i = 0; i < ncmds; i++){
	    if(new_st == NULL && lc->cmd == LC_SYMTAB){
		new_st = (struct symtab_command *)lc;
	    }
	    else if(new_dyst == NULL && lc->cmd == LC_DYSYMTAB){
		new_dyst = (struct dysymtab_command *)lc;
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	if(new_st == NULL || new_st->nsyms == 0){
	    if(arch_name != NULL)
		fatal("new dynamic shared library: %s (for architecture %s) "
		      "has no symbol table", new_dylib, arch_name);
	    else
		fatal("new dynamic shared library: %s has no symbol table",
		      new_dylib);
	}
	if(new_ofile->mh != NULL){
	    new_symbols = (struct nlist *)
		(new_ofile->object_addr + new_st->symoff);
	    new_symbols64 = NULL;
	}
	else{
	    new_symbols64 = (struct nlist_64 *)
		(new_ofile->object_addr + new_st->symoff);
	    new_symbols = NULL;
	}
	new_strings = (char *)
		(new_ofile->object_addr + new_st->stroff);
	new_tocs = (struct dylib_table_of_contents *)
		(new_ofile->object_addr + new_dyst->tocoff);
	if(new_ofile->object_byte_sex != host_byte_sex){
	    if(new_ofile->mh != NULL)
		swap_nlist(new_symbols, new_st->nsyms, host_byte_sex);
	    else
		swap_nlist_64(new_symbols64, new_st->nsyms, host_byte_sex);
	    swap_dylib_table_of_contents(new_tocs, new_dyst->ntoc,
		    host_byte_sex);
	}
	for(i = 0; i < new_st->nsyms; i++){
	    if(new_ofile->mh != NULL)
		n_strx = new_symbols[i].n_un.n_strx;
	    else
		n_strx = new_symbols64[i].n_un.n_strx;
	    if(n_strx != 0 && n_strx > new_st->strsize){
		if(arch_name != NULL)
		    fatal("malformed dynamic shared library: %s (for "
			"architecture %s) (bad string table index for symbol "
			"%u)", new_dylib, arch_name, i);
		else
		    fatal("malformed dynamic shared library: %s (bad string "
			"table index for symbol %u)", new_dylib, i);
	    }
	}
	for(i = 0; i < new_dyst->ntoc; i++){
	    if(new_tocs[i].symbol_index > new_st->nsyms){
		if(arch_name != NULL)
		    fatal("malformed dynamic shared library: %s (for "
			"architecture %s) (symbol_index field of table of "
			"contents entry %u past the end of the symbol table)",
			new_dylib, arch_name, i);
		else
		    fatal("malformed dynamic shared library: %s (symbol_index "
			"field of table of contents entry %u past the end of "
			"the symbol table)", new_dylib, i);
	    }
	}

	/*
	 * Now check to see if all the global symbols defined in the old library
	 * are defined in the new library.  Prints the ones that are not and
	 * sets compatible to FALSE.
	 */
	missing_symbols = FALSE;
	strings = new_strings;
	if(new_dyst->ntoc != 0){
	    symbols = new_symbols;
	    symbols64 = new_symbols64;
	}
	else{
	    if(new_ofile->mh != NULL){
		symbols = new_symbols + new_dyst->iextdefsym;
		symbols64 = NULL;
	    }
	    else{
		symbols64 = new_symbols64 + new_dyst->iextdefsym;
		symbols = NULL;
	    }
	}
	for(i = 0; i < old_dyst->nextdefsym; i++){
	    if(new_ofile->mh != NULL)
		n_strx = old_symbols[i + old_dyst->iextdefsym].n_un.n_strx;
	    else
		n_strx = old_symbols64[i + old_dyst->iextdefsym].n_un.n_strx;
	    symbol_name = old_strings + n_strx;
	    if(new_dyst->ntoc != 0){
		if(new_ofile->mh != NULL){
		    toc = bsearch(symbol_name, new_tocs, new_dyst->ntoc,
			          sizeof(struct dylib_table_of_contents),
			          (int (*)(const void *, const void *))
				  dylib_bsearch);
		}
		else{
		    toc = bsearch(symbol_name, new_tocs, new_dyst->ntoc,
			          sizeof(struct dylib_table_of_contents),
			          (int (*)(const void *, const void *))
				  dylib_bsearch64);
		}
		found = toc != NULL;
	    }
	    else{
		if(new_ofile->mh != NULL){
		    sym = bsearch(symbol_name, symbols, new_dyst->nextdefsym,
				  sizeof(struct nlist),
				  (int (*)(const void *, const void *))
				  nlist_bsearch);
		    found = sym != NULL;
		}
		else{
		    sym64 = bsearch(symbol_name, symbols64,new_dyst->nextdefsym,
				  sizeof(struct nlist_64),
				  (int (*)(const void *, const void *))
				  nlist_bsearch64);
		    found = sym64 != NULL;
		}
	    }
	    if(found == FALSE){
		if(missing_symbols == FALSE){
		    if(arch_name != NULL)
			printf("For architecture %s symbols defined in %s "
			       "not defined in %s:\n", arch_name, old_dylib,
			       new_dylib);
		    else
			printf("symbols defined in %s not defined in %s:\n",
			    old_dylib, new_dylib);
		    missing_symbols = TRUE;
		    compatible = FALSE;
		}
		printf("%s\n", symbol_name);
	    }
	}

	/*
	 * If new api is allowed then checking of global symbols is done.
	 */
	if(new_api_allowed == TRUE)
	    return;

	/*
	 * New api is not allowed so check to make sure no symbols in the new
	 * library are not in the old library.
	 */
	new_api = FALSE;
	strings = old_strings;
	if(old_dyst->ntoc != 0){
	    symbols = old_symbols;
	    symbols64 = old_symbols64;
	}
	else{
	    if(old_ofile->mh != NULL){
		symbols = old_symbols + old_dyst->iextdefsym;
		symbols64 = NULL;
	    }
	    else{
		symbols64 = old_symbols64 + old_dyst->iextdefsym;
		symbols = NULL;
	    }
	}
	for(i = 0; i < new_dyst->nextdefsym; i++){
	    if(new_ofile->mh != NULL)
		n_strx = new_symbols[i + new_dyst->iextdefsym].n_un.n_strx;
	    else
		n_strx = new_symbols64[i + new_dyst->iextdefsym].n_un.n_strx;
	    symbol_name = new_strings + n_strx;
	    if(old_dyst->ntoc != 0){
		if(new_ofile->mh != NULL){
		    toc = bsearch(symbol_name, old_tocs, old_dyst->ntoc,
			          sizeof(struct dylib_table_of_contents),
			          (int (*)(const void *, const void *))
				  dylib_bsearch);
		}
		else{
		    toc = bsearch(symbol_name, old_tocs, old_dyst->ntoc,
			          sizeof(struct dylib_table_of_contents),
			          (int (*)(const void *, const void *))
				  dylib_bsearch64);
		}
		found = toc != NULL;
	    }
	    else{
		if(new_ofile->mh != NULL){
		    sym = bsearch(symbol_name, symbols, old_dyst->nextdefsym,
				  sizeof(struct nlist),
				  (int (*)(const void *, const void *))
				  nlist_bsearch);
		    found = sym != NULL;
		}
		else{
		    sym64 = bsearch(symbol_name, symbols64,old_dyst->nextdefsym,
				  sizeof(struct nlist_64),
				  (int (*)(const void *, const void *))
				  nlist_bsearch64);
		    found = sym64 != NULL;
		}
	    }
	    if(found == FALSE){
		if(new_api == FALSE){
		    if(arch_name != NULL)
			printf("For architecture %s compatibility versions are "
			    "the same but new symbols defined in %s not "
			    "defined in %s:\n",arch_name, new_dylib, old_dylib);
		    else
			printf("compatibility versions are the same but new "
			    "symbols defined in %s not defined in %s:\n",
			    new_dylib, old_dylib);
		    new_api = TRUE;
		    compatible = FALSE;
		}
		printf("%s\n", symbol_name);
	    }
	}
}

/*
 * Function for bsearch() for finding a symbol name in a dylib table of
 * contents.
 */
static
int
dylib_bsearch(
const char *symbol_name,
const struct dylib_table_of_contents *toc)
{
	return(strcmp(symbol_name,
		      strings + symbols[toc->symbol_index].n_un.n_strx));
}

static
int
dylib_bsearch64(
const char *symbol_name,
const struct dylib_table_of_contents *toc)
{
	return(strcmp(symbol_name,
		      strings + symbols64[toc->symbol_index].n_un.n_strx));
}

/*
 * Function for bsearch() for finding a symbol name in the sorted list of
 * defined external symbols.
 */
static
int
nlist_bsearch(
const char *symbol_name,
const struct nlist *sym)
{
	return(strcmp(symbol_name, strings + sym->n_un.n_strx));
}

static
int
nlist_bsearch64(
const char *symbol_name,
const struct nlist_64 *sym64)
{
	return(strcmp(symbol_name, strings + sym64->n_un.n_strx));
}
