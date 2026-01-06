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
#include <limits.h>
#include <time.h>
#include <libc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mach-o/nlist.h>
#include "stuff/breakout.h"
#include "stuff/hash_string.h"
#include "stuff/allocate.h"
#include "stuff/errors.h"
#include "stuff/rnd.h"
#include "stuff/reloc.h"

char *progname = NULL;	/* name of the program for error messages (argv[0]) */

/*
 * If -arch_indr <arch> <listfile> is used then this is an allocated array of 
 * the names of the listfiles.
 */
char **list_filenames = NULL;

/*
 * Data structures to hold the symbol names (both the indirect and undefined
 * symbol names) from the list file.
 */
struct symbol {
    char *name;			/* name of the symbol */
    int32_t type;		/* type of this symbol (N_INDR or N_UNDF) */
    char *indr;			/* name for indirection if N_INDR type */
    struct indr_object *io;	/* pointer to indr_object for this */
    struct symbol *next;	/* next symbol in the hash chain */
};
/* The symbol hash table is hashed on the name field */
#define SYMBOL_HASH_SIZE	250
static struct symbol *symbol_hash[SYMBOL_HASH_SIZE];

/*
 * Data structures to hold the object names (both the names from the archive
 * and the names created for indirect symbols).  If this is for indirect symbols
 * then the indr and undef fields are non-zero.
 */
struct indr_object {
    char *membername;		/* the base name of the object file */
    char *indr;			/* the name of the indirect symbol */
    char *undef;		/* the name of the undefined symbol */
    enum bool existing_symbol;
    uint32_t index;
};

/*
 * The ordered list of the indr objects to be created.  The objects created for
 * indirect symbols are placed in first in the archive in the order the symbol
 * names appear in the list file (so to get known order).  Then all of the
 * original objects from the old archive are placed in the archive preserving
 * their order.
 */
#define INITIAL_LIST_SIZE	250
struct list {
    struct indr_object **list;	/* the order list */
    uint32_t used;		/* the number used in the list */
    uint32_t size;		/* the current size of the list */
};
static struct list indr_list;

/*
 * The string table maintained by start_string_table(), add_to_string_table()
 * and end_string_table();
 */
#define INITIAL_STRING_TABLE_SIZE	40960
static struct {
    char *strings;
    uint32_t size;
    uint32_t index;
} string_table;

struct undef_map {
    enum bool old_symbol;
    uint32_t index;
    struct nlist symbol;
};
static struct undef_map *undef_map;
static char *qsort_strings = NULL;
static struct nlist *qsort_symbols = NULL;

static void usage(
    void);

static void process_list(
    char *list_filename,
    enum bool nflag);

static void translate_input(
    struct arch *archs,
    uint32_t narchs,
    struct arch_flag *arch_flags,
    uint32_t narch_flags,
    enum bool all_archs,
    enum bool nflag);

static void translate_object(
    struct arch *arch,
    struct member *member,
    struct object *object);

static void translate_dylib(
    struct arch *arch,
    struct object *object);

static int cmp_qsort_undef_map(
    const struct undef_map *sym1,
    const struct undef_map *sym2);

static int cmp_qsort_toc(
    const struct dylib_table_of_contents *toc1,
    const struct dylib_table_of_contents *toc2);

static void make_indr_objects(
    struct arch *arch);

static void enter_symbol(
    char *name,
    int32_t type,
    char *indr,
    struct indr_object *io);

static struct symbol *lookup_symbol(
    char *name);

static struct indr_object *enter_object(
    char *membername,
    char *indr,
    char *undef,
    struct list *list);

static void start_string_table(
    void);

static int32_t add_to_string_table(
    char *p);

static void end_string_table(
    void);

static void add_list(
    struct list *list,
    struct indr_object *item);

/* apple_version is created by the libstuff/Makefile */
extern char apple_version[];
char *version = apple_version;

/*
 * The indr(l) program takes the following options:
 *
 *	% indr [-n] list_filename input_file output_file
 *
 * It builds the output file by translating each symbol name listed in
 * list file to the same name with and underbar prepended to it in all the
 * objects in the input file.  Then it if the input file is an archive and the
 * -n flag is not specified then it creates an object for each of these
 * symbols with an indirect symbol for the symbol name with an underbar and
 * adds that to the output archive.
 *
 * The -n flag is to suppress creating the indirect objects.
 */
int
main(
int argc,
char *argv[],
char *envp[])
{
    int i;
    uint32_t j;
    enum bool no_flags_left;
    char *list_filename, *output_file, *input_file;
    struct arch_flag *arch_flags;
    uint32_t narch_flags;
    enum bool all_archs;
    enum bool nflag;
    struct arch *archs;
    uint32_t narchs;
    struct stat stat_buf;

	progname = argv[0];

	arch_flags = NULL;
	narch_flags = 0;
	all_archs = FALSE;

	archs = NULL;
	narchs = 0;

	list_filenames = NULL;
	list_filename = NULL;
	input_file = NULL;
	output_file = NULL;
	nflag = FALSE;

	/*
	 * Parse the flags.
	 */
	no_flags_left = FALSE;
	for(i = 1; i < argc ; i++){
	    if(argv[i][0] != '-' || no_flags_left){
		if(list_filename == NULL && list_filenames == NULL)
		    list_filename = argv[i];
		else if(input_file == NULL)
		    input_file = argv[i];
		else if(output_file == NULL)
		    output_file = argv[i];
		else
		    usage();
		continue;
	    }
	    if(argv[i][1] == '\0'){
		no_flags_left = TRUE;
		continue;
	    }
	    if(strcmp(argv[i], "-arch") == 0){
		if(list_filenames != NULL)
		    fatal("can't mix -arch_indr and -arch arguments");
		if(i + 1 >= argc){
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
		    for(j = 0; j < narch_flags; j++){
			if(arch_flags[j].cputype ==
				arch_flags[narch_flags].cputype &&
			   (arch_flags[j].cpusubtype & ~CPU_SUBTYPE_MASK) ==
				(arch_flags[narch_flags].cpusubtype &
				 ~CPU_SUBTYPE_MASK) &&
			   strcmp(arch_flags[j].name,
				arch_flags[narch_flags].name) == 0)
			    break;
		    }
		    if(j == narch_flags)
			narch_flags++;
		}
		i++;
		continue;
	    }
	    if(strcmp(argv[i], "-arch_indr") == 0){
		if(list_filenames == NULL && narch_flags != 0)
		    fatal("can't mix -arch_indr and -arch arguments");
		if(i + 2 >= argc){
		    error("missing argument(s) to %s option", argv[i]);
		    usage();
		}
		arch_flags = reallocate(arch_flags,
			(narch_flags + 1) * sizeof(struct arch_flag));
		list_filenames = reallocate(list_filenames,
			(narch_flags + 1) * sizeof(char *));
		if(get_arch_from_flag(argv[i+1],
				      arch_flags + narch_flags) == 0){
		    error("unknown architecture specification flag: "
			  "%s %s %s", argv[i], argv[i+1], argv[i+2]);
		    arch_usage();
		    usage();
		}
		list_filenames[narch_flags] = argv[i+2];
		for(j = 0; j < narch_flags; j++){
		    if(arch_flags[j].cputype ==
			    arch_flags[narch_flags].cputype &&
		       (arch_flags[j].cpusubtype & ~CPU_SUBTYPE_MASK) ==
			    (arch_flags[narch_flags].cpusubtype & 
			     ~CPU_SUBTYPE_MASK) &&
		       strcmp(arch_flags[j].name,
			    arch_flags[narch_flags].name) == 0)
			break;
		}
		if(j == narch_flags)
		    narch_flags++;
		i += 2;
		continue;
	    }
	    for(j = 1; argv[i][j] != '\0'; j++){
		switch(argv[i][j]){
		case 'n':
		    nflag = TRUE;
		    break;
		default:
		    error("unknown flag -%c", argv[i][j]);
		    usage();
		}
	    }
	}
	if((list_filename == NULL && list_filenames == NULL) ||
	   input_file == NULL || output_file == NULL)
	    usage();

	/*
	 * Now do the work.
	 */

	/* process the list of symbols and create the data structures */
	if(list_filenames == NULL)
	    process_list(list_filename, nflag);

	/* breakout the input file for processing */
	breakout(input_file, &archs, &narchs, FALSE);
	if(errors)
	    exit(EXIT_FAILURE);

	/* checkout the input file for symbol table replacement processing */
	checkout(archs, narchs);

	/* translate the symbols in the input file */
	translate_input(archs, narchs, arch_flags, narch_flags, all_archs,
			nflag);
	if(errors)
	    exit(EXIT_FAILURE);

	/* create the output file */
	if(stat(input_file, &stat_buf) == -1)
	    system_error("can't stat input file: %s", input_file);
	writeout(archs, narchs, output_file, stat_buf.st_mode & 0777, TRUE,
		 FALSE, FALSE, FALSE, NULL);

	if(errors)
	    return(EXIT_FAILURE);
	else
	    return(EXIT_SUCCESS);
}

/*
 * Print the current usage message and exit non-zero.
 */
static
void
usage()
{
	fprintf(stderr, "Usage: %s [-n] [[-arch arch_flag] ...] "
		"<symbol list file> <input file> <output file>\n", progname);
	fprintf(stderr, "Usage: %s [-n] [[-arch_indr arch_flag "
		"<symbol list file>] ...] <input file> <output file>\n",
		progname);
	exit(EXIT_FAILURE);
}

/*
 * process the symbols listed in list_filename.  This enters each symbol as an
 * indirect symbol into the symbol hash table as well as the undefined symbol
 * for the indirection.  Then it creates the name of the object file that will
 * be used to put these symbols in.
 */ 
static
void
process_list(
char *list_filename,
enum bool nflag)
{
	FILE *list;
	char buf[BUFSIZ], *symbol_name, *_symbol_name, *membername;
	int32_t i, symbol_number;
	size_t len;
	struct indr_object *io;

	/*
	 * Reset things first.
	 */
	for(i = 0; i < SYMBOL_HASH_SIZE; i++)
	    symbol_hash[i] = NULL;
	memset(&indr_list, '\0', sizeof(struct list));

	if((list = fopen(list_filename, "r")) == NULL)
	    system_fatal("can't open: %s", list_filename);

	io = NULL;
	symbol_number = 0;
	buf[BUFSIZ-1] = '\0';
	while(fgets(buf, BUFSIZ-1, list) != NULL){
	    len = strlen(buf);
	    if(buf[len-1] != '\n')
		fatal("symbol name: %s too long from file: %s", buf,
		       list_filename);
	    buf[len-1] = '\0';

	    symbol_name = makestr(buf, NULL);
	    _symbol_name = makestr("_", symbol_name, NULL);

	    if(nflag == FALSE){
		sprintf(buf, "%05d", symbol_number++);
		membername = makestr("INDR", buf, ".o", NULL);
		io = enter_object(membername, symbol_name, _symbol_name,
				  &indr_list);
	    }

	    enter_symbol(symbol_name, N_INDR, _symbol_name, io);
	    enter_symbol(_symbol_name, N_UNDF, NULL, io);
	}
	if(ferror(list))
	    system_fatal("can't read: %s", list_filename);
	fclose(list);
}

/*
 * Translate the objects in the input file by changing the external symbols
 * that match indirect symbols in the symbol hash table to the symbol that the
 * indirect symbol is for.
 */
static
void
translate_input(
struct arch *archs,
uint32_t narchs,
struct arch_flag *arch_flags,
uint32_t narch_flags,
enum bool all_archs,
enum bool nflag)
{
    uint32_t i, j, offset, size;
    cpu_type_t cputype;
    cpu_subtype_t cpusubtype;
    struct arch_flag host_arch_flag;
    enum bool arch_process, any_processing, *arch_flag_processed;
    char *list_filename;
    struct ar_hdr h;
    char size_buf[sizeof(h.ar_size) + 1];

	arch_flag_processed = NULL;
	/*
	 * Using the specified arch_flags process specified objects for those
	 * architecures.
	 */
	any_processing = FALSE;
	if(narch_flags != 0)
	    arch_flag_processed = allocate(narch_flags * sizeof(enum bool));
	memset(arch_flag_processed, '\0', narch_flags * sizeof(enum bool));
	for(i = 0; i < narchs; i++){
	    /*
	     * Determine the architecture (cputype and cpusubtype) of arch[i]
	     */
	    cputype = 0;
	    cpusubtype = 0;
	    if(archs[i].type == OFILE_ARCHIVE){
		for(j = 0; j < archs[i].nmembers; j++){
		    if(archs[i].members[j].type == OFILE_Mach_O){
			cputype = archs[i].members[j].object->mh_cputype;
			cpusubtype = archs[i].members[j].object->mh_cpusubtype;
			break;
		    }
		}
	    }
	    else if(archs[i].type == OFILE_Mach_O){
		cputype = archs[i].object->mh_cputype;
		cpusubtype = archs[i].object->mh_cpusubtype;
	    }
	    else if(archs[i].fat_arch64 != NULL){
		cputype = archs[i].fat_arch64->cputype;
		cpusubtype = archs[i].fat_arch64->cpusubtype;
	    }
	    else if(archs[i].fat_arch != NULL){
		cputype = archs[i].fat_arch->cputype;
		cpusubtype = archs[i].fat_arch->cpusubtype;
	    }
	    arch_process = FALSE;
	    list_filename = NULL;
	    if(all_archs == TRUE){
		arch_process = TRUE;
	    }
	    else if(narch_flags != 0){
		for(j = 0; j < narch_flags; j++){
		    if(arch_flags[j].cputype == cputype &&
		       (arch_flags[j].cpusubtype & ~CPU_SUBTYPE_MASK) ==
		       (cpusubtype & ~CPU_SUBTYPE_MASK)){
			arch_process = TRUE;
			arch_flag_processed[j] = TRUE;
			if(list_filenames != NULL)
			    list_filename = list_filenames[j];
			break;
		    }
		}
	    }
	    else{
		(void)get_arch_from_host(&host_arch_flag, NULL);
		if(host_arch_flag.cputype == cputype &&
		   (host_arch_flag.cpusubtype & ~CPU_SUBTYPE_MASK) ==
		   (cpusubtype & ~CPU_SUBTYPE_MASK))
		    arch_process = TRUE;
	    }
	    if(narchs != 1 && arch_process == FALSE)
		continue;
	    any_processing = TRUE;

	    if(list_filename != NULL)
		process_list(list_filename, nflag);

	    /*
	     * Now this arch[i] has been selected to be processed so process it
	     * according to it's type.
	     */
	    if(archs[i].type == OFILE_ARCHIVE){
		for(j = 0; j < archs[i].nmembers; j++){
		    if(archs[i].members[j].type == OFILE_Mach_O){
			translate_object(archs + i, archs[i].members + j,
					 archs[i].members[j].object);
		    }
		}
		/*
		 * Make the objects for the indirect symbols in the -n flag is
		 * not specified since this architecure is an archive.
		 */
		if(nflag == FALSE)
		    make_indr_objects(archs + i);

		/*
		 * Reset the library offsets and size.
		 */
		offset = 0;
		for(j = 0; j < archs[i].nmembers; j++){
		    archs[i].members[j].offset = offset;
		    size = 0;
		    if(archs[i].members[j].member_long_name == TRUE){
			size = rnd32(archs[i].members[j].member_name_size,
				     sizeof(int32_t));
			archs[i].toc_long_name = TRUE;
		    }
		    if(archs[i].members[j].object != NULL){
			size += archs[i].members[j].object->object_size
			   - archs[i].members[j].object->input_sym_info_size
			   + archs[i].members[j].object->output_sym_info_size;
			sprintf(size_buf, "%-*ld",
			   (int)sizeof(archs[i].members[j].ar_hdr->ar_size),
			   (long)(size));
			/*
			 * This has to be done by hand because sprintf puts a
			 * null at the end of the buffer.
			 */
			memcpy(archs[i].members[j].ar_hdr->ar_size, size_buf,
			   (int)sizeof(archs[i].members[j].ar_hdr->ar_size));
		    }
		    else{
			size += archs[i].members[j].unknown_size;
		    }
		    offset += sizeof(struct ar_hdr) + size;
		}
		archs[i].library_size = offset;
	    }
	    else if(archs[i].type == OFILE_Mach_O){
		translate_object(archs + i, NULL, archs[i].object);
	    }
	    else {
		fatal_arch(archs + i, NULL, "can't process non-object and "
			   "non-archive file: ");
	    }
	}
	if(all_archs == FALSE && narch_flags != 0){
	    for(i = 0; i < narch_flags; i++){
		if(arch_flag_processed[i] == FALSE)
		    error("file: %s does not contain architecture: %s",
			  archs[0].file_name, arch_flags[i].name);
	    }
	    free(arch_flag_processed);
	}
	if(any_processing == FALSE)
	    fatal("no processing done on input file: %s (specify a -arch flag)",
		  archs[0].file_name);
}

/*
 * translate the one object's symbols which match the symbols for which indirect
 * symbols are to be created.
 */
static
void
translate_object(
struct arch *arch,
struct member *member,
struct object *object)
{

    uint32_t i, j;

    enum byte_sex host_byte_sex;
    struct nlist *symbols, *nlistp;
    uint32_t nsyms, offset;
    char *strings;
    uint32_t strings_size;
    struct symbol *sp;

    int32_t new_nsyms;
    struct nlist *new_symbols;

	if(object->mh_filetype == MH_DYLIB ||
	   object->mh_filetype == MH_DYLIB_STUB){
	    if(member != NULL)
		fatal_arch(arch, member, "is a dynamic library which is not "
			   "allowed as a member of an archive");
	    translate_dylib(arch, object);
	    return;
	}

	host_byte_sex = get_host_byte_sex();

	if(object->st == NULL || object->st->nsyms == 0)
	    return;

	symbols = (struct nlist *)(object->object_addr + object->st->symoff);
	nsyms = object->st->nsyms;
	if(object->object_byte_sex != host_byte_sex)
	    swap_nlist(symbols, nsyms, host_byte_sex);
	strings = object->object_addr + object->st->stroff;
	strings_size = object->st->strsize;

	object->output_symbols = symbols;
	object->output_nsymbols = nsyms;
	object->input_sym_info_size = nsyms * sizeof(struct nlist) +
				      object->st->strsize;
	if(object->dyst != NULL){
	    object->input_sym_info_size +=
		    object->dyst->nindirectsyms * sizeof(uint32_t);
	}

	/*
	 * Always clear the prebind checksum if any when creating a new file.
	 */
	if(object->cs != NULL)
	    object->cs->cksum = 0;

	start_string_table();
	nlistp = symbols;
	for(i = 0; i < nsyms; i++){
	    if(nlistp->n_type & N_EXT){
		if(nlistp->n_un.n_strx){
		    if(nlistp->n_un.n_strx > 0 &&
		       (uint32_t)nlistp->n_un.n_strx < strings_size){
			sp = lookup_symbol(strings + nlistp->n_un.n_strx);
			if(sp != NULL){
			    if(sp->type == N_UNDF)
				fatal_arch(arch, member, "symbol name: "
				    "%s conflicts with symbol name created for "
				    "indirection in: ", sp->name);
			    nlistp->n_un.n_strx = add_to_string_table(sp->indr);
			}
			else{
			    nlistp->n_un.n_strx = add_to_string_table(
				    strings + nlistp->n_un.n_strx);
			}
		    }
		    else
			fatal_arch(arch, member, "bad string table "
				    "index in symbol %u in: ", i);
		}
	    }
	    else{
		if(nlistp->n_un.n_strx){
		    if(nlistp->n_un.n_strx > 0 && nlistp->n_un.n_strx <
							(int32_t)strings_size)
			nlistp->n_un.n_strx = add_to_string_table(
				strings + nlistp->n_un.n_strx);
		    else
			fatal_arch(arch, member, "bad string table "
				    "index in symbol %u in: ", i);
		}
	    }
	    nlistp++;
	}
	/*
	 * This is a hack to keep the full reference object of a host shared
	 * library correct when it is processed by this program.  To do this
	 * The name of the object, "__.FVMLIB_REF", is checked for and if this 
	 * is it an undefined symbol for each indirect symbol is added so to
	 * cause all the indrect objects to be loaded.
	 */
	new_symbols = NULL;
	if(member != NULL &&
	   strncmp(member->ar_hdr->ar_name, "__.FVMLIB_REF",
		   sizeof("__.FVMLIB_REF") - 1) == 0){
	    new_nsyms = 0;
	    for(i = 0; i < SYMBOL_HASH_SIZE; i++){
		sp = symbol_hash[i];
		while(sp != NULL){
		    if(sp->type == N_INDR)
			new_nsyms++;
		    sp = sp->next;
		}
	    }
	    new_symbols = allocate((nsyms + new_nsyms) * sizeof(struct nlist));
	    memcpy(new_symbols, symbols, nsyms * sizeof(struct nlist));
	    j = nsyms;
	    for(i = 0; i < SYMBOL_HASH_SIZE; i++){
		sp = symbol_hash[i];
		while(sp != NULL){
		    if(sp->type == N_INDR){
			new_symbols[j].n_un.n_strx =
						add_to_string_table(sp->name);
			new_symbols[j].n_type = N_UNDF | N_EXT;
			new_symbols[j].n_sect = NO_SECT;
			new_symbols[j].n_desc = 0;
			new_symbols[j].n_value = 0;
			j++;
		    }
		    sp = sp->next;
		}
	    }
	    symbols = new_symbols;
	    object->output_symbols = symbols;
	    nsyms += new_nsyms;
	    object->output_nsymbols = nsyms;
	}
	end_string_table();
	object->output_strings = allocate(string_table.index);
	memcpy(object->output_strings, string_table.strings,string_table.index);
	object->output_strings_size = string_table.index;

	object->output_sym_info_size =
		nsyms * sizeof(struct nlist) +
		string_table.index;
	if(object->dyst != NULL){
	    object->output_sym_info_size +=
		    object->dyst->nindirectsyms * sizeof(uint32_t);
	}

	if(object->seg_linkedit != NULL){
	    object->seg_linkedit->filesize += object->output_sym_info_size -
					      object->input_sym_info_size;
	    object->seg_linkedit->vmsize = object->seg_linkedit->filesize;
	}

	if(object->dyst != NULL){
	    object->st->nsyms = nsyms;
	    object->st->strsize = string_table.index;

	    offset = UINT_MAX;
	    if(object->st->nsyms != 0 &&
	       object->st->symoff < offset)
		offset = object->st->symoff;
	    if(object->dyst->nindirectsyms != 0 &&
	       object->dyst->indirectsymoff < offset)
		offset = object->dyst->indirectsymoff;
	    if(object->st->strsize != 0 &&
	       object->st->stroff < offset)
		offset = object->st->stroff;

	    if(object->st->nsyms != 0){
		object->st->symoff = offset;
		offset += object->st->nsyms * sizeof(struct nlist);
	    }
	    else
		object->st->symoff = 0;

	    if(object->dyst->nindirectsyms != 0){
		object->output_indirect_symtab = (uint32_t *)
		    (object->object_addr + object->dyst->indirectsymoff);
		object->dyst->indirectsymoff = offset;
		offset += object->dyst->nindirectsyms *
			  sizeof(uint32_t);
	    }
	    else
		object->dyst->indirectsymoff = 0;;

	    if(object->st->strsize != 0){
		object->st->stroff = offset;
		offset += object->st->strsize;
	    }
	    else
		object->st->stroff = 0;
	}
	else{
	    object->st->nsyms = nsyms;
	    object->st->stroff = object->st->symoff +
				 nsyms * sizeof(struct nlist);
	    object->st->strsize = string_table.index;
	}
}

/*
 * translate the dynamic library.
 */
static
void
translate_dylib(
struct arch *arch,
struct object *object)
{
    enum byte_sex host_byte_sex;
    uint32_t i, inew_syms, inew_undefsyms, inew_mods, indr_iextdefsym;
    uint32_t new_ext_strsize, offset;
    uint32_t *map;
    size_t len;
    struct symbol *sp;
    char *p, *q;
    struct scattered_relocation_info *sreloc;

    struct nlist *symbols;
    uint32_t nsyms;
    char *strings;
    uint32_t strsize;
    struct dylib_table_of_contents *tocs;
    uint32_t ntoc;
    struct dylib_module *mods;
    uint32_t nmodtab;
    struct dylib_reference *refs;
    uint32_t nextrefsyms;
    struct relocation_info *ext_relocs;
    uint32_t *indirect_symtab;

    struct nlist *new_symbols;
    uint32_t new_nsyms;
    char *new_strings;
    uint32_t new_strsize;
    uint32_t new_nlocalsym;
    uint32_t new_nextdefsym;
    uint32_t new_nundefsym;
    struct dylib_table_of_contents *new_tocs;
    uint32_t new_ntoc;
    struct dylib_module *new_mods;
    uint32_t new_nmodtab;
    struct dylib_reference *new_refs;
    uint32_t new_nextrefsyms;


	/*
	 * Break out all the old tables.
	 */
	symbols = (struct nlist *)(object->object_addr + object->st->symoff);
	nsyms = object->st->nsyms;
	strings = object->object_addr + object->st->stroff;
	strsize = object->st->strsize;
	tocs = (struct dylib_table_of_contents *)
		(object->object_addr + object->dyst->tocoff);
	ntoc = object->dyst->ntoc;
	mods = (struct dylib_module *)
		(object->object_addr + object->dyst->modtaboff);
	nmodtab = object->dyst->nmodtab;
	refs = (struct dylib_reference *)
		(object->object_addr + object->dyst->extrefsymoff);
	nextrefsyms = object->dyst->nextrefsyms;
	ext_relocs = (struct relocation_info *)
		      (object->object_addr + object->dyst->extreloff);
	indirect_symtab = (uint32_t *)
			   (object->object_addr + object->dyst->indirectsymoff);
	/*
	 * Swap them if needed.
	 */
	host_byte_sex = get_host_byte_sex();
	if(object->object_byte_sex != host_byte_sex){
	    swap_nlist(symbols, nsyms, host_byte_sex);
	    swap_dylib_table_of_contents(tocs, ntoc, host_byte_sex);
	    swap_dylib_module(mods, nmodtab, host_byte_sex);
	    swap_dylib_reference(refs, nextrefsyms, host_byte_sex);
	    swap_relocation_info(ext_relocs, object->dyst->nextrel,
				 host_byte_sex);
	    swap_indirect_symbols(indirect_symtab, object->dyst->nindirectsyms,
				  host_byte_sex);
	}


	/*
	 * First pass, figrure out the new sizes of the new tables.
	 */
	
	/*
	 * For the symbol table and string table recalculate the their sizes
	 * with the names of the symbols listed in the indr file renamed and
	 * an indirect symbol added.
	 * Look to make sure no symbols exist that would colide with the
	 * indirect's undefined symbol names that will be created.  This is so
	 * the symbols can be renamed.
	 */
	new_nsyms = 0;
	new_strsize = sizeof(int32_t);
	new_nlocalsym = 0;
	new_nextdefsym = 0;
	new_nundefsym = 0;
	new_ext_strsize = 0;
	for(i = 0; i < nsyms; i++){
	    if(symbols[i].n_un.n_strx != 0){
		if((uint32_t)symbols[i].n_un.n_strx > strsize){
		    error_arch(arch, NULL, "bad string index for symbol "
			       "table entry %d in: ", i);
		    return;
		}
	    }
	    if((symbols[i].n_type & N_TYPE) == N_INDR){
		if(symbols[i].n_value != 0){
		    if(symbols[i].n_value > strsize){
			error_arch(arch, NULL, "bad string index for "
				   "indirect symbol table entry %d in: ", i);
			return;
		    }
		}
	    }
	    if((symbols[i].n_type & N_EXT) == 0){ /* local symbol */
		if(symbols[i].n_un.n_strx != 0)
		    new_strsize += strlen(strings + symbols[i].n_un.n_strx) + 1;
		new_nlocalsym++;
		new_nsyms++;
	    }
	    else{ /* global symbol */
		len = 0;
		if(symbols[i].n_un.n_strx != 0){
		    sp = lookup_symbol(strings + symbols[i].n_un.n_strx);
		    if(sp != NULL){
			if(sp->type == N_UNDF)
			    fatal_arch(arch, NULL, "symbol name: %s conflicts "
				"with symbol name created for indirection in: ",
				sp->name);
			else{
			    len = strlen(sp->indr) + 1;
			    sp->io->existing_symbol = TRUE;
			    sp->io->index = i;
			}
		    }
		    else
			len = strlen(strings + symbols[i].n_un.n_strx) + 1;
		}
		if((symbols[i].n_type & N_TYPE) == N_INDR)
		    len += strlen(strings + symbols[i].n_value) + 1;
		new_strsize += len;
		new_ext_strsize += len;
		new_nsyms++;
		if(((symbols[i].n_type & N_TYPE) == N_UNDF &&
		    symbols[i].n_value == 0) ||
		    (symbols[i].n_type & N_TYPE) == N_PBUD)
		    new_nundefsym++;
		else
		    new_nextdefsym++;
	    }
	}
	/*
	 * The new symbol table will have 1 new defined external for each
	 * indirect symbol name on the indr list.  And 1 new undefined symbol
	 * if the undefined symbol name does not exist.
	 */
	for(i = 0; i < indr_list.used; i++){
	    new_nsyms++;
	    new_nextdefsym++;
	    /* if there is not an existing symbol there will be a new undef */
	    if(indr_list.list[i]->existing_symbol == FALSE){
		new_nsyms++;
		new_nundefsym++;
	    }
	    len = strlen(indr_list.list[i]->indr) + 1;
	    len += strlen(indr_list.list[i]->undef) + 1;
	    new_strsize += len;
	    new_ext_strsize += len;
	}
	/*
	 * The module table's module names are placed with the external strings.
	 * So size them and add this to the external string size.
	 */
	for(i = 0; i < nmodtab; i++){
	    if(mods[i].module_name == 0 ||
	       mods[i].module_name > strsize){
		error_arch(arch, NULL, "bad string index for module_name "
			   "of module table entry %d in: ", i);
		return;
	    }
	    len = strlen(strings + mods[i].module_name) + 1;
	    new_strsize += len;
	    new_ext_strsize += len;
	}
	/*
	 * A new module will be created for each indr symbol so add the sizes
	 * of those module names to the external string size.
	 */
	for(i = 0; i < indr_list.used; i++){
	    len = strlen(indr_list.list[i]->membername) + 1;
	    new_strsize += len;
	    new_ext_strsize += len;
	}

	/*
	 * The new module table will have one extra entry for each indr symbol.
	 */
	new_nmodtab = nmodtab + indr_list.used;
	
	/*
	 * The new reference table will have two extra entries for each indr
	 * symbol.  One for the definition of the indr and one for the undefined
	 * that it refers to.
	 */
	new_nextrefsyms = nextrefsyms + 2 * indr_list.used;

	/*
	 * The new table of contents will have extra entry for each indr symbol.
	 */
	new_ntoc = ntoc + indr_list.used;

	/*
	 * Second pass, create the new tables.
	 */
	new_symbols =(struct nlist *)allocate(new_nsyms * sizeof(struct nlist));
	new_strsize = rnd32(new_strsize, sizeof(int32_t));
	new_strings = (char *)allocate(new_strsize);
	new_strings[new_strsize - 3] = '\0';
	new_strings[new_strsize - 2] = '\0';
	new_strings[new_strsize - 1] = '\0';

	memset(new_strings, '\0', sizeof(int32_t));
	p = new_strings + sizeof(int32_t);
	q = p + new_ext_strsize;

	/*
	 * Now create the new symbol table and string table in this order
	 * symbol table
	 *	local symbols (sorted by module)
	 *	external defined symbols (sorted by module)
	 *	undefined symbols (sorted by name)
	 * string table
	 *	external strings
	 *	local strings
	 */
	map = (uint32_t *)allocate(nsyms * sizeof(uint32_t));
	memset(map, '\0', nsyms * sizeof(uint32_t));
	inew_syms = 0;
	for(i = 0; i < nsyms; i++){ /* loop for local symbols */
	    if((symbols[i].n_type & N_EXT) == 0){
		new_symbols[inew_syms] = symbols[i];
		if(symbols[i].n_un.n_strx != 0){
		    strcpy(q, strings + symbols[i].n_un.n_strx);
		    new_symbols[inew_syms].n_un.n_strx =
			(uint32_t)(q - new_strings);
		    q += strlen(q) + 1;
		}
		map[i] = inew_syms;
		inew_syms++;
	    }
	}
	for(i = 0; i < nsyms; i++){ /* loop for external defined symbols */
	    if((symbols[i].n_type & N_EXT) == N_EXT &&
	       ((symbols[i].n_type & N_TYPE) != N_UNDF &&
	        (symbols[i].n_type & N_TYPE) != N_PBUD)){
		new_symbols[inew_syms] = symbols[i];

		if(symbols[i].n_un.n_strx != 0){
		    sp = lookup_symbol(strings + symbols[i].n_un.n_strx);
		    if(sp != NULL)
			strcpy(p, sp->indr);
		    else
			strcpy(p, strings + symbols[i].n_un.n_strx);
		    new_symbols[inew_syms].n_un.n_strx =
			(uint32_t)(p - new_strings);
		    p += strlen(p) + 1;
		}
		if((symbols[i].n_type & N_TYPE) == N_INDR){
		    if(symbols[i].n_value != 0){
			strcpy(p, strings + symbols[i].n_value);
			new_symbols[inew_syms].n_value =
			    (uint32_t)(p - new_strings);
			p += strlen(p) + 1;
		    }
		}
		map[i] = inew_syms;
		inew_syms++;
	    }
	}
	indr_iextdefsym = inew_syms;
	for(i = 0; i < indr_list.used; i++){ /* loop for new defined symbols*/
	    strcpy(p, indr_list.list[i]->indr);
	    new_symbols[inew_syms].n_un.n_strx = (uint32_t)(p - new_strings);
	    p += strlen(p) + 1;
	    new_symbols[inew_syms].n_type = N_INDR | N_EXT;
	    new_symbols[inew_syms].n_desc = 0;
	    new_symbols[inew_syms].n_sect = NO_SECT;
	    /* Note this name is used below for the undefined */
	    strcpy(p, indr_list.list[i]->undef);
	    indr_list.list[i]->undef = p;
	    new_symbols[inew_syms].n_value = (uint32_t)(p - new_strings);
	    p += strlen(p) + 1;
	    inew_syms++;
	}
	/*
	 * To get the undefined symbols in order sorted by name they are first
	 * copied into the undef_map, sorted and then copied into the
	 * new_symbols in the sorted order.
	 */
	undef_map = (struct undef_map *)allocate(new_nundefsym *
						 sizeof(struct undef_map));
	inew_undefsyms = 0;
	for(i = 0; i < indr_list.used; i++){ /* loop for new undef symbols */
	    if(indr_list.list[i]->existing_symbol == FALSE){
		/* Note this name is used from above for the undefined */
		undef_map[inew_undefsyms].symbol.n_un.n_strx =
		    (uint32_t)(indr_list.list[i]->undef - new_strings);
		undef_map[inew_undefsyms].symbol.n_type = N_UNDF | N_EXT;
		undef_map[inew_undefsyms].symbol.n_desc = 0;
		undef_map[inew_undefsyms].symbol.n_sect = NO_SECT;
		undef_map[inew_undefsyms].symbol.n_value = 0;
		undef_map[inew_undefsyms].old_symbol = FALSE;
		undef_map[inew_undefsyms].index = i;
		inew_undefsyms++;
	    }
	}
	for(i = 0; i < nsyms; i++){ /* loop for undefined symbols */
	    if((symbols[i].n_type & N_EXT) == N_EXT &&
	       ((symbols[i].n_type & N_TYPE) == N_UNDF ||
	        (symbols[i].n_type & N_TYPE) == N_PBUD)){
		undef_map[inew_undefsyms].symbol = symbols[i];
		if(symbols[i].n_un.n_strx != 0){
		    sp = lookup_symbol(strings + symbols[i].n_un.n_strx);
		    if(sp != NULL)
			strcpy(p, sp->indr);
		    else
			strcpy(p, strings + symbols[i].n_un.n_strx);
		    undef_map[inew_undefsyms].symbol.n_un.n_strx =
			(uint32_t)(p - new_strings);
		    p += strlen(p) + 1;
		}
		undef_map[inew_undefsyms].old_symbol = TRUE;
		undef_map[inew_undefsyms].index = i;
		inew_undefsyms++;
	    }
	}
	/* Sort the undefined symbols by name */
	qsort_strings = new_strings;
	qsort(undef_map, new_nundefsym, sizeof(struct undef_map),
	      (int (*)(const void *, const void *))cmp_qsort_undef_map);
	/* Copy the symbols now in sorted order into new_symbols */
	for(i = 0; i < new_nundefsym; i++){
	    new_symbols[inew_syms] = undef_map[i].symbol;
	    /* update the map for these symbols */
	    if(undef_map[i].old_symbol == TRUE)
		map[undef_map[i].index] = inew_syms;
	     else
		indr_list.list[undef_map[i].index]->index = inew_syms;
	    inew_syms++;
	}

	/*
	 * Fill in the new module table.  First copy in the old table, and
	 * module names.  Then add the new module table entries for the
	 * indr modules.
	 */
	new_mods = (struct dylib_module *)allocate(
				new_nmodtab * sizeof(struct dylib_module));
	inew_mods = 0;
	for(i = 0; i < nmodtab; i++){
	    new_mods[inew_mods] = mods[i];
	    strcpy(p, strings + mods[i].module_name);
	    new_mods[inew_mods].module_name = (uint32_t)(p - new_strings);
	    p += strlen(p) + 1;
	    inew_mods++;
	}
	for(i = 0; i < indr_list.used; i++){
	    memset(new_mods + inew_mods, '\0', sizeof(struct dylib_module));
	    strcpy(p, indr_list.list[i]->membername);
	    new_mods[inew_mods].module_name = (uint32_t)(p - new_strings);
	    p += strlen(p) + 1;
	    new_mods[inew_mods].iextdefsym = indr_iextdefsym + i;
	    new_mods[inew_mods].nextdefsym = 1;
	    new_mods[inew_mods].irefsym = nextrefsyms + i * 2;
	    new_mods[inew_mods].nrefsym = 2;
	    inew_mods++;
	}

	/*
	 * Fill in the new reference table.  First copy in the old table.  Then
	 * create entries for the indr modules.
	 */
	new_refs = (struct dylib_reference *)allocate(
			new_nextrefsyms * sizeof(struct dylib_reference));
	for(i = 0; i < nextrefsyms; i++){
	    new_refs[i].isym = map[refs[i].isym];
	    new_refs[i].flags = refs[i].flags;
	}
	for(i = 0; i < indr_list.used; i++){
	    new_refs[nextrefsyms + i*2].isym = indr_iextdefsym + i;
	    new_refs[nextrefsyms + i*2].flags = REFERENCE_FLAG_DEFINED;

	    if(indr_list.list[i]->existing_symbol == TRUE)
		new_refs[nextrefsyms + i*2 + 1].isym = 
		    map[indr_list.list[i]->index];
	    else
		new_refs[nextrefsyms + i*2 + 1].isym = 
		    indr_list.list[i]->index;
	    new_refs[nextrefsyms + i*2 + 1].flags =
					    REFERENCE_FLAG_UNDEFINED_NON_LAZY;
	}

	/*
	 * Fill in the new table of contents.  First copy in the old table.
	 * Then create entries for the indr symbols.
	 */
	new_tocs = (struct dylib_table_of_contents *)allocate(
			    new_ntoc * sizeof(struct dylib_table_of_contents));
	for(i = 0; i < ntoc; i++){
	    new_tocs[i].symbol_index = map[tocs[i].symbol_index];
	    new_tocs[i].module_index = tocs[i].module_index;
	}
	for(i = 0; i < indr_list.used; i++){
	    new_tocs[ntoc + i].symbol_index = indr_iextdefsym + i;
	    new_tocs[ntoc + i].module_index = nmodtab + i;
	}
	qsort_strings = new_strings;
	qsort_symbols = new_symbols;
	qsort(new_tocs, new_ntoc, sizeof(struct dylib_table_of_contents),
	      (int (*)(const void *, const void *))cmp_qsort_toc);

	/*
	 * Remap indexes into symbol table in the external relocation entries.
	 */
	for(i = 0; i < object->dyst->nextrel; i++){
	    if((ext_relocs[i].r_address & R_SCATTERED) == 0 &&
	       ext_relocs[i].r_extern == 1){
		if(ext_relocs[i].r_symbolnum > nsyms){
		    fatal_arch(arch, NULL, "bad r_symbolnum for external "
			"relocation entry %d in: ", i);
		}
		ext_relocs[i].r_symbolnum = map[ext_relocs[i].r_symbolnum];
	    }
	    else{
		fatal_arch(arch, NULL, "bad external relocation entry "
		    "%d (not external) in: ", i);
	    }
	    if((ext_relocs[i].r_address & R_SCATTERED) == 0){
		if(reloc_has_pair(object->mh_cputype, ext_relocs[i].r_type))
		    i++;
	    }
	    else{
		sreloc = (struct scattered_relocation_info *)ext_relocs + i;
		if(reloc_has_pair(object->mh_cputype, sreloc->r_type))
		    i++;
	    }
	}

	/*
	 * Remap indexes into symbol table in the indirect symbol table.
	 */
	for(i = 0; i < object->dyst->nindirectsyms; i++){
	    if(indirect_symtab[i] != INDIRECT_SYMBOL_LOCAL &&
	       indirect_symtab[i] != INDIRECT_SYMBOL_ABS){
		if(indirect_symtab[i] > nsyms)
		    fatal_arch(arch, NULL, "indirect symbol table entry %d "
			"(past the end of the symbol table) in: ", i);
		indirect_symtab[i] = map[indirect_symtab[i]];
	    }
	}

	/*
	 * Now that all the new tables have been built and existing table
	 * updated set the object struct to use them.
	 */
	object->input_sym_info_size =
	    object->dyst->nlocrel * sizeof(struct relocation_info) +
	    object->dyst->nextrel * sizeof(struct relocation_info) +
	    object->dyst->nindirectsyms * sizeof(uint32_t) +
	    nsyms * sizeof(struct nlist) +
	    strsize +
	    ntoc * sizeof(struct dylib_table_of_contents)+
	    nmodtab * sizeof(struct dylib_module) +
	    nextrefsyms * sizeof(struct dylib_reference);

	object->output_sym_info_size =
	    object->dyst->nlocrel * sizeof(struct relocation_info) +
	    object->dyst->nextrel * sizeof(struct relocation_info) +
	    object->dyst->nindirectsyms * sizeof(uint32_t) +
	    new_nsyms * sizeof(struct nlist) +
	    new_strsize +
	    new_ntoc * sizeof(struct dylib_table_of_contents)+
	    new_nmodtab * sizeof(struct dylib_module) +
	    new_nextrefsyms * sizeof(struct dylib_reference);

	if(object->split_info_cmd != NULL){
	    object->input_sym_info_size += object->split_info_cmd->datasize;
	    object->output_sym_info_size += object->split_info_cmd->datasize;
	}

	if(object->func_starts_info_cmd != NULL){
	    object->input_sym_info_size +=
		object->func_starts_info_cmd->datasize;
	    object->output_sym_info_size +=
		object->func_starts_info_cmd->datasize;
	}

	if(object->data_in_code_cmd != NULL){
	    object->input_sym_info_size +=
		object->data_in_code_cmd->datasize;
	    object->output_sym_info_size +=
		object->data_in_code_cmd->datasize;
	}

	if(object->code_sign_drs_cmd != NULL){
	    object->input_sym_info_size +=
		object->code_sign_drs_cmd->datasize;
	    object->output_sym_info_size +=
		object->code_sign_drs_cmd->datasize;
	}

	if(object->link_opt_hint_cmd != NULL){
	    object->input_sym_info_size +=
		object->link_opt_hint_cmd->datasize;
	    object->output_sym_info_size +=
		object->link_opt_hint_cmd->datasize;
	}

	if(object->hints_cmd != NULL){ 
	    object->input_sym_info_size +=
		object->hints_cmd->nhints * sizeof(struct twolevel_hint);
	    object->output_sym_info_size +=
		object->hints_cmd->nhints * sizeof(struct twolevel_hint);
	}

	if(object->code_sig_cmd != NULL){
	    object->input_sym_info_size =
		rnd32(object->input_sym_info_size, 16);
	    object->input_sym_info_size +=
		object->code_sig_cmd->datasize;
	    object->output_sym_info_size =
		rnd32(object->output_sym_info_size, 16);
	    object->output_sym_info_size +=
		object->code_sig_cmd->datasize;
	}

	if(object->seg_linkedit != NULL){
	    object->seg_linkedit->filesize += object->output_sym_info_size -
					      object->input_sym_info_size;
	    object->seg_linkedit->vmsize = object->seg_linkedit->filesize;
	}

	object->output_symbols = new_symbols;
	object->output_nsymbols = new_nsyms;
	object->output_strings = new_strings;
	object->output_strings_size = new_strsize;
	object->output_tocs = new_tocs;
	object->output_ntoc = new_ntoc;
	object->output_mods = new_mods;
	object->output_nmodtab = new_nmodtab;
	object->output_refs = new_refs;
	object->output_nextrefsyms = new_nextrefsyms;
	object->output_loc_relocs = (struct relocation_info *)
	    (object->object_addr + object->dyst->locreloff);
	if(object->split_info_cmd != NULL){
	    object->output_split_info_data = 
	    (object->object_addr + object->split_info_cmd->dataoff);
	    object->output_split_info_data_size = 
		object->split_info_cmd->datasize;
	}
	if(object->func_starts_info_cmd != NULL){
	    object->output_func_start_info_data = 
	    (object->object_addr + object->func_starts_info_cmd->dataoff);
	    object->output_func_start_info_data_size = 
		object->func_starts_info_cmd->datasize;
	}
	if(object->data_in_code_cmd != NULL){
	    object->output_data_in_code_info_data = 
	    (object->object_addr + object->data_in_code_cmd->dataoff);
	    object->output_data_in_code_info_data_size = 
		object->data_in_code_cmd->datasize;
	}
	if(object->code_sign_drs_cmd != NULL){
	    object->output_code_sign_drs_info_data = 
	    (object->object_addr + object->code_sign_drs_cmd->dataoff);
	    object->output_code_sign_drs_info_data_size = 
		object->code_sign_drs_cmd->datasize;
	}
	if(object->link_opt_hint_cmd != NULL){
	    object->output_link_opt_hint_info_data = 
	    (object->object_addr + object->link_opt_hint_cmd->dataoff);
	    object->output_link_opt_hint_info_data_size = 
		object->link_opt_hint_cmd->datasize;
	}
	object->output_ext_relocs = ext_relocs;
	object->output_indirect_symtab = indirect_symtab;
	if(object->hints_cmd != NULL){
	    object->output_hints = (struct twolevel_hint *)
		(object->object_addr + object->hints_cmd->offset);
	}
	if(object->code_sig_cmd != NULL){
	    object->output_code_sig_data = object->object_addr +
		object->code_sig_cmd->dataoff;
	    object->output_code_sig_data_size = 
		object->code_sig_cmd->datasize;
	}

	if(object->object_byte_sex != host_byte_sex){
	    /* the symbol table gets swapped by writeout() */
	    swap_dylib_table_of_contents(new_tocs, new_ntoc,
		object->object_byte_sex);
	    swap_dylib_module(new_mods, new_nmodtab,
		object->object_byte_sex);
	    swap_dylib_reference(new_refs, new_nextrefsyms,
		object->object_byte_sex);
	    swap_relocation_info(ext_relocs, object->dyst->nextrel,
		object->object_byte_sex);
	    swap_indirect_symbols(indirect_symtab, object->dyst->nindirectsyms,
		object->object_byte_sex);
	}

	object->st->nsyms = new_nsyms; 
	object->st->strsize = new_strsize;

	object->dyst->ilocalsym = 0;
	object->dyst->nlocalsym = new_nlocalsym;
	object->dyst->iextdefsym = new_nlocalsym;
	object->dyst->nextdefsym = new_nextdefsym;
	object->dyst->iundefsym = new_nlocalsym + new_nextdefsym;
	object->dyst->nundefsym = new_nundefsym;
	object->dyst->ntoc = new_ntoc;
	object->dyst->nmodtab = new_nmodtab;
	object->dyst->nextrefsyms = new_nextrefsyms;

	offset = object->seg_linkedit->fileoff;
	if(object->dyst->nlocrel != 0){
	    object->dyst->locreloff = offset;
	    offset += object->dyst->nlocrel * sizeof(struct relocation_info);
	}
	if(object->split_info_cmd != NULL){
	    object->split_info_cmd->dataoff = offset;
	    offset += object->split_info_cmd->datasize;
	}
	if(object->func_starts_info_cmd != NULL){
	    object->func_starts_info_cmd->dataoff = offset;
	    offset += object->func_starts_info_cmd->datasize;
	}
	if(object->data_in_code_cmd != NULL){
	    object->data_in_code_cmd->dataoff = offset;
	    offset += object->data_in_code_cmd->datasize;
	}
	if(object->code_sign_drs_cmd != NULL){
	    object->code_sign_drs_cmd->dataoff = offset;
	    offset += object->code_sign_drs_cmd->datasize;
	}
	if(object->link_opt_hint_cmd != NULL){
	    object->link_opt_hint_cmd->dataoff = offset;
	    offset += object->link_opt_hint_cmd->datasize;
	}
	if(object->st->nsyms != 0){
	    object->st->symoff = offset;
	    offset += object->st->nsyms * sizeof(struct nlist);
	}
	if(object->hints_cmd != NULL){
	    if(object->hints_cmd->nhints != 0){
		object->hints_cmd->offset = offset;
		offset += object->hints_cmd->nhints *
			  sizeof(struct twolevel_hint);
	    }
	}
	if(object->dyst->nextrel != 0){
	    object->dyst->extreloff = offset;
	    offset += object->dyst->nextrel * sizeof(struct relocation_info);
	}
	if(object->dyst->nindirectsyms != 0){
	    object->dyst->indirectsymoff = offset;
	    offset += object->dyst->nindirectsyms * sizeof(uint32_t);
	}
	if(object->dyst->ntoc != 0){
	    object->dyst->tocoff = offset;
	    offset += object->dyst->ntoc *
		      sizeof(struct dylib_table_of_contents);
	}
	if(object->dyst->nmodtab != 0){
	    object->dyst->modtaboff = offset;
	    offset += object->dyst->nmodtab * sizeof(struct dylib_module);
	}
	if(object->dyst->nextrefsyms != 0){
	    object->dyst->extrefsymoff = offset;
	    offset += object->dyst->nextrefsyms *
		      sizeof(struct dylib_reference);
	}
	if(object->st->strsize != 0){
	    object->st->stroff = offset;
	    offset += object->st->strsize;
	}
	if(object->code_sig_cmd != NULL){
	    offset = rnd32(offset, 16);
	    object->code_sig_cmd->dataoff = offset;
	    offset += object->code_sig_cmd->datasize;
	}
}

/*
 * Function for qsort for comparing undefined map entries.
 */
static
int
cmp_qsort_undef_map(
const struct undef_map *sym1,
const struct undef_map *sym2)
{
	return(strcmp(qsort_strings + sym1->symbol.n_un.n_strx,
		      qsort_strings + sym2->symbol.n_un.n_strx));
}

/*
 * Function for qsort for comparing table of contents entries.
 */
static
int
cmp_qsort_toc(
const struct dylib_table_of_contents *toc1,
const struct dylib_table_of_contents *toc2)
{
	return(strcmp(
	       qsort_strings + qsort_symbols[toc1->symbol_index].n_un.n_strx,
	       qsort_strings + qsort_symbols[toc2->symbol_index].n_un.n_strx));
}

/*
 * make_indr_objects writes an object file for each symbol name that was in the
 * list file.  The object contains an indirect symbol for the symbol_name.  The
 * symbol name used as the indirect symbol has been constructed by prepending
 * an underbar to symbol_name previously in process_list().
 */
static
void
make_indr_objects(
struct arch *arch)
{
    int32_t i;
    cpu_type_t cputype;
    cpu_subtype_t cpusubtype;
    enum byte_sex target_byte_sex;
    struct indr_object *io;
    struct nlist *output_symbols, *undef, *indr;
    char *output_strings, *object_addr;
    uint32_t object_size;
    struct mach_header *mh;
    struct symtab_command *st;
    struct ar_hdr *ar_hdr;
    uint32_t indr_time, size;
    unsigned short indr_mode;
    int oumask;
    gid_t gid;
    uid_t uid;
    struct object *object;

	/*
	 * Make room for the indr objects in this arch's member list.  The
	 * indr objects will be place first in the list so the exising members
	 * will be moved to the end of the end of the newly realloced list.
	 */
	arch->members = reallocate(arch->members,
				   (arch->nmembers + indr_list.used) *
				   sizeof(struct member));
	cputype = 0;
	cpusubtype = 0;
	target_byte_sex = UNKNOWN_BYTE_SEX;
	for(i = arch->nmembers - 1; i >= 0 ;i--){
	    if(arch->members[i].object != NULL){
		if(cputype == 0){
		    cputype = arch->members[i].object->mh_cputype;
		    cpusubtype = arch->members[i].object->mh_cpusubtype;
		}
		if(target_byte_sex == UNKNOWN_BYTE_SEX){
		    target_byte_sex = arch->members[i].object->object_byte_sex;
		}
	    }
	    arch->members[i + indr_list.used] = arch->members[i];
	}
	memset(arch->members, '\0', indr_list.used * sizeof(struct member));
	arch->nmembers += indr_list.used;
	if(cputype == 0 || target_byte_sex == UNKNOWN_BYTE_SEX)
	    fatal_arch(arch, NULL, "can't determine the cputype and/or the "
			"byte sex for: ");

	/*
	 * Now loop through the indr list creating the objects for each indirect
	 * symbol to be added to this archive.
	 */
	indr_time = (uint32_t)time(0);
	oumask = umask(0);
	indr_mode = S_IFREG | (0666 & ~oumask);
	(void)umask(oumask);
	gid = getgid();
	uid = getuid();
	for(i = 0; i < indr_list.used; i++){
	    io = indr_list.list[i];

	    output_symbols = allocate(2 * sizeof(struct nlist));
	    undef = output_symbols + 0;
	    indr = output_symbols + 1;

	    start_string_table();
	    undef->n_un.n_strx = add_to_string_table(io->undef);
	    undef->n_type = N_UNDF | N_EXT;
	    undef->n_sect = NO_SECT;
	    undef->n_desc = 0;
	    undef->n_value = 0;

	    indr->n_un.n_strx = add_to_string_table(io->indr);
	    indr->n_type = N_INDR | N_EXT;
	    indr->n_sect = NO_SECT;
	    indr->n_desc = 0;
	    indr->n_value = undef->n_un.n_strx;
	    end_string_table();

	    output_strings = allocate(string_table.index);
	    memcpy(output_strings, string_table.strings, string_table.index);

	    object_size = sizeof(struct mach_header) +
			  sizeof(struct symtab_command);
	    object_addr = allocate(object_size);
	    mh = (struct mach_header *)object_addr;
	    st = (struct symtab_command *)(object_addr +
					   sizeof(struct mach_header));
	    mh->magic = MH_MAGIC;
	    mh->cputype = cputype;
	    mh->cpusubtype = cpusubtype;
	    mh->filetype = MH_OBJECT;
	    mh->ncmds = 1;
	    mh->sizeofcmds = sizeof(struct symtab_command);
	    mh->flags = 0;

	    st->cmd = LC_SYMTAB;
	    st->cmdsize = sizeof(struct symtab_command);
	    st->symoff = sizeof(struct mach_header) +
			 sizeof(struct symtab_command);
	    st->nsyms = 2;
	    st->stroff = sizeof(struct mach_header) +
			 sizeof(struct symtab_command) +
			 2 * sizeof(struct nlist);
	    st->strsize = string_table.index;
	    size = sizeof(struct mach_header) +
		   sizeof(struct symtab_command) +
		   2 * sizeof(struct nlist) +
		   string_table.index;

	    arch->members[i].type = OFILE_Mach_O;
	    ar_hdr = allocate(sizeof(struct ar_hdr) + 1);
	    arch->members[i].ar_hdr = ar_hdr;
	    sprintf((char *)ar_hdr, "%-*s%-*ld%-*u%-*u%-*o%-*ld%-*s",
	       (int)sizeof(ar_hdr->ar_name),
		   io->membername,
	       (int)sizeof(ar_hdr->ar_date),
		   (long)indr_time,
	       (int)sizeof(ar_hdr->ar_uid),
		   (unsigned int)uid,
	       (int)sizeof(ar_hdr->ar_gid),
		   (unsigned int)gid,
	       (int)sizeof(ar_hdr->ar_mode),
		   (unsigned int)indr_mode,
	       (int)sizeof(ar_hdr->ar_size),
		   (long)(size),
	       (int)sizeof(ar_hdr->ar_fmag),
		   ARFMAG);

	    object = allocate(sizeof(struct object));
	    memset(object, '\0', sizeof(struct object));
	    arch->members[i].object = object;
	    object->object_addr = object_addr;
	    object->object_size = object_size;
	    object->object_byte_sex = target_byte_sex;
	    object->mh = mh;
	    object->load_commands = (struct load_command *)st;
	    object->st = NULL;
	    object->input_sym_info_size = 0;
	    object->output_sym_info_size = 2 * sizeof(struct nlist) +
					   string_table.index;
	    object->output_symbols = output_symbols;
	    object->output_nsymbols = 2;
	    object->output_strings = output_strings;
	    object->output_strings_size = string_table.index;

	    arch->members[i].input_file_name = arch->file_name;
	    arch->members[i].input_ar_hdr = ar_hdr;
	}
}

/*
 * enter a symbol and it's type into the symbol hash table checking for
 * duplicates.  Duplicates cause a fatal error to be printed.
 */
static
void
enter_symbol(
char *name,
int32_t type,
char *indr,
struct indr_object *io)
{
    int32_t hash_key;
    struct symbol *sp;

	hash_key = hash_string(name) % SYMBOL_HASH_SIZE;
	sp = symbol_hash[hash_key];
	while(sp != NULL){
	    if(strcmp(name, sp->name) == 0){
		fatal("to create %s symbol: %s would conflict with also to be "
		      "created %s symbol: %s",
		      type == N_INDR ? "indirect" : "undefined", name,
		      sp->type == N_INDR ? "indirect" : "undefined", sp->name);
	    }
	    sp = sp->next;
	}
	sp = (struct symbol *)allocate(sizeof(struct symbol));
	sp->name = name;
	sp->type = type;
	sp->indr = indr;
	sp->io = io;
	sp->next = symbol_hash[hash_key];
	symbol_hash[hash_key] = sp;
}

/*
 * lookup a symbol name in the symbol hash table returning a pointer to the
 * symbol structure for it.  A NULL pointer is returned if not found.
 */
static
struct symbol *
lookup_symbol(
char *name)
{
    int32_t hash_key;
    struct symbol *sp;

	hash_key = hash_string(name) % SYMBOL_HASH_SIZE;
	sp = symbol_hash[hash_key];
	while(sp != NULL){
	    if(strcmp(name, sp->name) == 0){
		return(sp);
	    }
	    sp = sp->next;
	}
	return(NULL);
}

/*
 * Create and enter an indr object and it's informaton into the specified list.
 */
static
struct indr_object *
enter_object(
char *membername,
char *indr,
char *undef,
struct list *list)
{
    struct indr_object *io;

	io = allocate(sizeof(struct indr_object));
	io->membername = membername;
	io->indr = indr;
	io->undef = undef;
	io->existing_symbol = FALSE;
	io->index = 0;
	add_list(list, io);
	return(io);
}

/*
 * This routine is called before calls to add_to_string_table() are made to
 * setup or reset the string table structure.  The first four bytes string
 * table are zeroed and the first string is placed after that  (this was for
 * the string table length in a 4.3bsd a.out along time ago).  The first four
 * bytes are kept zero even thought only the first byte can't be used as valid
 * string offset (because that is defined to be a NULL string) but to avoid
 * breaking programs that don't know this the first byte is left zero and the
 * first 4 bytes are not stuffed with the size because on a little endian
 * machine that first byte is likely to be non-zero.
 */
static
void
start_string_table()
{
	if(string_table.size == 0){
	    string_table.size = INITIAL_STRING_TABLE_SIZE;
	    string_table.strings = (char *)allocate(string_table.size);
	}
	memset(string_table.strings, '\0', sizeof(int32_t));
	string_table.index = sizeof(int32_t);
}

/*
 * This routine adds the specified string to the string table structure and
 * returns the index of the string in the table.
 */
static
int32_t
add_to_string_table(
char *p)
{
    size_t len;
    uint32_t index;

	len = strlen(p) + 1;
	if(string_table.size < string_table.index + len){
	    string_table.strings = (char *)reallocate(string_table.strings,
						      string_table.size * 2);
	    string_table.size *= 2;
	}
	index = string_table.index;
	strcpy(string_table.strings + string_table.index, p);
	string_table.index += len;
	return(index);
}

/*
 * This routine is called after all calls to add_to_string_table() are made
 * to round off the size of the string table.  It zeros the rounded bytes.
 */
static
void
end_string_table()
{
    uint32_t length;

	length = rnd32(string_table.index, sizeof(uint32_t));
	memset(string_table.strings + string_table.index, '\0',
	       length - string_table.index);
	string_table.index = length;
}

/*
 * Add the item to the specified list.  Lists can be reused just by setting
 * list->used to zero.  The item after the last item is allways set to NULL
 * so that list->list can be used for things like execv() calls directly.
 */
static
void
add_list(
struct list *list,
struct indr_object *item)
{
	if(list->used + 1 >= list->size){
	    if(list->size == 0){
		list->list = allocate(INITIAL_LIST_SIZE * sizeof(void *));
		list->size = INITIAL_LIST_SIZE;
		list->used = 0;
	    }
	    else{
		list->list = reallocate(list->list,
					list->size * 2 * sizeof(void *));
		list->size *= 2;
	    }
	}
	list->list[list->used++] = item;
	list->list[list->used] = NULL;
}
