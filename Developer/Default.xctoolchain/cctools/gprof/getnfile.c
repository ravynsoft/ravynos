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
#include <stdlib.h>
#include <string.h>
#include <mach-o/loader.h>
#include <mach-o/stab.h>
#ifdef __OPENSTEP__
#include <mach-o/rld.h>
#endif /* defined(__OPENSTEP__) */
#include "stuff/ofile.h"
#include "stuff/allocate.h"
#include "stuff/errors.h"
#include "gprof.h"

struct shlib_text_range *shlib_text_ranges = NULL;
uint32_t nshlib_text_ranges = 0;

static struct ofile ofile = { 0 };

#ifdef __OPENSTEP__
static uint32_t link_edit_address;
static uint32_t address_func(
    uint32_t size,
    uint32_t headers_size);
#endif

static void count_func_symbols(
    struct nlist *symbols,
    struct nlist_64 *symbols64,
    uint32_t nsymbols,
    char *strings,
    uint32_t strsize);

static void load_func_symbols(
    struct nlist *symbols,
    struct nlist_64 *symbols64,
    uint32_t nsymbols,
    char *strings,
    uint32_t strsize,
    uint64_t vmaddr_slide);

static enum bool funcsymbol(
    uint8_t n_type,
    uint8_t n_sect,
    char *name);

static int valcmp(
    nltype *p1,
    nltype *p2);

static void count_N_SO_stabs(
    struct nlist *symbols,
    struct nlist_64 *symbols64,
    uint32_t nsymbols,
    char *strings,
    uint32_t strsize);

static void load_files(
    struct nlist *symbols,
    struct nlist_64 *symbols64,
    uint32_t nsymbols,
    char *strings,
    uint32_t strsize);

static struct arch_flag host_arch_flag;

void
getnfile(
void)
{
    uint32_t i, j, ncmds;
    uint64_t text_highpc;
    struct load_command *lc;
    struct segment_command *sg;
    struct segment_command_64 *sg64;
    struct section *s;
    struct section_64 *s64;
    struct symtab_command *st;
    struct nlist *symbols;
    struct nlist_64 *symbols64;
#ifdef notdef
    struct ofile lib_ofile;
    uint32_t k;
    struct fvmlib_command *fl;
    struct load_command *lib_lc;
    struct symtab_command *lib_st;
    char *lib_name;
#endif

	nname = 0;
	n_files = 0;

	if(get_arch_from_host(&host_arch_flag, NULL) == 0)
	    fatal("can't determine the host architecture");

	if(ofile_map(a_outname, NULL, NULL, &ofile, FALSE) == FALSE)
	    return;

	/*
	 * Pick the host architecture first if it is there, or the 64-bit
	 * version of the host architecture next if it is there.
	 */
	if(ofile.file_type == OFILE_FAT){
	    (void)ofile_first_arch(&ofile);
	    do{
		if(host_arch_flag.cputype == ofile.mh_cputype)
		    goto good;
	    }while(ofile_next_arch(&ofile) == TRUE);

	    (void)ofile_first_arch(&ofile);
	    do{
		if((host_arch_flag.cputype | CPU_ARCH_ABI64) ==
		   ofile.mh_cputype){
		    host_arch_flag.cputype |= CPU_ARCH_ABI64;
		    goto good;
		}
	    }while(ofile_next_arch(&ofile) == TRUE);

	    error("file: %s does not contain the host architecture", a_outname);
	    return;
	}
	else if(ofile.file_type == OFILE_ARCHIVE){
	    error("file: %s is not an Mach-O file executable", a_outname);
	    return;
	}
	else if(ofile.file_type == OFILE_Mach_O){
	    if(host_arch_flag.cputype == ofile.mh_cputype)
		goto good;
	    if((host_arch_flag.cputype | CPU_ARCH_ABI64) == ofile.mh_cputype){
		host_arch_flag.cputype |= CPU_ARCH_ABI64;
		goto good;
	    }
	    error("file: %s is not of the host architecture", a_outname);
	    return;
	}
	else{ /* ofile.file_type == OFILE_UNKNOWN */
	    error("file: %s is not an Mach-O file executable", a_outname);
	    return;
	}
good:

	if((ofile.mh == NULL && ofile.mh64 == NULL) ||
	   (ofile.mh_filetype != MH_EXECUTE &&
	    ofile.mh_filetype != MH_DYLIB &&
	    ofile.mh_filetype != MH_DYLINKER))
	    fatal("file: %s is not a Mach-O executable file", a_outname);

	/*
	 * Pass 1 count symbols and files.
	 */
	st = NULL;
	lc = ofile.load_commands;
	if(ofile.mh != NULL){
	    text_highpc = 0xfffffffe;
	    ncmds = ofile.mh->ncmds;
	}
	else{
	    text_highpc = 0xfffffffffffffffeULL;
	    ncmds = ofile.mh64->ncmds;
	}
	for(i = 0; i < ncmds; i++){
	    if(lc->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc;
		s = (struct section *)
		      ((char *)sg + sizeof(struct segment_command));
		for(j = 0; j < sg->nsects; j++){
		    if(strcmp(s->sectname, SECT_TEXT) == 0 &&
		       strcmp(s->segname, SEG_TEXT) == 0){
			textspace = (unsigned char *)ofile.object_addr +
				    s->offset;
			text_highpc = s->addr + s->size;
		    }
		    s++;
		}
	    }
	    else if(lc->cmd == LC_SEGMENT_64){
		sg64 = (struct segment_command_64 *)lc;
		s64 = (struct section_64 *)
		      ((char *)sg64 + sizeof(struct segment_command_64));
		for(j = 0; j < sg64->nsects; j++){
		    if(strcmp(s64->sectname, SECT_TEXT) == 0 &&
		       strcmp(s64->segname, SEG_TEXT) == 0){
			textspace = (unsigned char *)ofile.object_addr +
				    s64->offset;
			text_highpc = s64->addr + s64->size;
		    }
		    s64++;
		}
	    }
#ifdef notdef
	    else if(lc->cmd == LC_LOADFVMLIB){
		fl = (struct fvmlib_command *)lc;
		lib_name = (char *)fl + fl->fvmlib.name.offset;
		if(ofile_map(lib_name, &host_arch_flag, NULL, &lib_ofile,
			     FALSE) == FALSE)
		    goto done1;
		if(lib_ofile.mh == NULL || lib_ofile.mh->filetype != MH_FVMLIB){
		    warning("file: %s is not a shared library file", lib_name);
		    goto done_and_unmap1;
		}
		lib_st = NULL;
		lib_lc = lib_ofile.load_commands;
		for(j = 0; j < lib_ofile.mh->ncmds; j++){
		    if(lib_st == NULL && lib_lc->cmd == LC_SYMTAB){
			lib_st = (struct symtab_command *)lib_lc;
			count_func_symbols((struct nlist *)
				(lib_ofile.object_addr + lib_st->symoff),
				lib_st->nsyms,
				lib_ofile.object_addr + lib_st->stroff,
				lib_st->strsize);
			break;
		    }
		    else if(lib_lc->cmd == LC_SEGMENT){
			sg = (struct segment_command *)lib_lc;
			s = (struct section *)
			      ((char *)sg + sizeof(struct segment_command));
			for(k = 0; k < sg->nsects; k++){
			    if(strcmp(s->sectname, SECT_TEXT) == 0 &&
			       strcmp(s->segname, SEG_TEXT) == 0){
				shlib_text_ranges =
				    reallocate(shlib_text_ranges,
					       (nshlib_text_ranges + 1) *
					       sizeof(struct shlib_text_range));
				shlib_text_ranges[nshlib_text_ranges].lowpc = 
				    s->addr;
				shlib_text_ranges[nshlib_text_ranges].highpc = 
				    s->addr + s->size;
				nshlib_text_ranges++;
			    }
			    s++;
			}
		    }
		    lib_lc = (struct load_command *)
				((char *)lib_lc + lib_lc->cmdsize);
		}
done_and_unmap1:
		ofile_unmap(&lib_ofile);
done1:		;
	    }
#endif
	    else if(st == NULL && lc->cmd == LC_SYMTAB){
		st = (struct symtab_command *)lc;
		if(ofile.mh != NULL){
		    symbols = (struct nlist *)
			      (ofile.object_addr + st->symoff);
		    symbols64 = NULL;
		}
		else{
		    symbols64 = (struct nlist_64 *)
				(ofile.object_addr + st->symoff);
		    symbols = NULL;
		}
		count_func_symbols(symbols, symbols64, st->nsyms,
				   ofile.object_addr + st->stroff, st->strsize);
		count_N_SO_stabs(symbols, symbols64, st->nsyms,
				 ofile.object_addr + st->stroff, st->strsize);
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}

	if(nname == 0)
	    fatal("executable file %s: has no symbols", a_outname);
	/*
	 * Allocate the data structures for the symbols and files.
	 */
	nl = (nltype *)calloc(nname + 2, sizeof(nltype));
	if(nl == NULL)
	    fatal("No room for %lu bytes of symbol table\n",
		  (nname + 2) * sizeof(nltype));
	npe = nl;
	files = (struct file *)calloc(n_files/2, sizeof(struct file));
	if(files == NULL)
	    fatal("No room for %lu bytes of file table\n",
		  n_files/2 * sizeof(struct file));
	n_files = 0;

	/*
	 * Pass 2 load symbols and files.
	 */
	if(st != NULL){
	    if(ofile.mh != NULL){
		symbols = (struct nlist *)
			  (ofile.object_addr + st->symoff);
		symbols64 = NULL;
	    }
	    else{
		symbols64 = (struct nlist_64 *)
			    (ofile.object_addr + st->symoff);
		symbols = NULL;
	    }
	    load_func_symbols(symbols, symbols64, st->nsyms,
			      ofile.object_addr + st->stroff, st->strsize, 0);
	    load_files(symbols, symbols64, st->nsyms,
		       ofile.object_addr + st->stroff, st->strsize);
	}
#ifdef notdef
	lc = ofile.load_commands;
	for(i = 0; i < ofile.mh->ncmds; i++){
	    if(lc->cmd == LC_LOADFVMLIB){
		fl = (struct fvmlib_command *)lc;
		lib_name = (char *)fl + fl->fvmlib.name.offset;
		if(ofile_map(lib_name, &host_arch_flag, NULL, &lib_ofile, 
			     FALSE) == TRUE){
		    lib_st = NULL;
		    lib_lc = lib_ofile.load_commands;
		    for(j = 0; j < lib_ofile.mh->ncmds; j++){
			lib_lc = (struct load_command *)
				    ((char *)lib_lc + lib_lc->cmdsize);
			if(lib_st == NULL && lib_lc->cmd == LC_SYMTAB){
			    lib_st = (struct symtab_command *)lib_lc;
			    load_func_symbols((struct nlist *)
				    (lib_ofile.object_addr + lib_st->symoff),
				    lib_st->nsyms,
				    lib_ofile.object_addr + lib_st->stroff,
				    lib_st->strsize, 0);
			    break;
			}
		    }
		}
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
#endif

	npe->value = text_highpc;
	npe->name = "past end of text";
	npe++;
	nname++;

	if(ofile.mh != NULL)
	    npe->value = npe->svalue = 0xffffffff;
	else
	    npe->value = npe->svalue = 0xffffffffffffffffULL;
	npe->name = "top of memory";

	qsort(nl, nname, sizeof(nltype),
	      (int (*)(const void *, const void *))valcmp);
#ifdef DEBUG
	if(debug & AOUTDEBUG){
	    for(i = 0; i < nname; i++){
		printf("[getnfile] ");
		if(ofile.mh != NULL)
		    printf("0x%08x", (unsigned int)nl[i].value);
		else
		    printf("%016llx", nl[i].value);
		printf("\t%s\n", nl[i].name);
	    }
	}
#endif /* DEBUG */
}

#ifdef __OPENSTEP__
void
get_rld_state_symbols(
void)
{
    uint32_t i, j, save_nname;
    NXStream *stream;
    struct mach_header **headers;
    char *object_addr;
    struct load_command *lc;
    struct symtab_command *st;

	if(grld_nloaded_states == 0)
	    return;
	headers = allocate(grld_nloaded_states * sizeof(struct mach_header *));

	/*
	 * Load the a_outname file as the base file.
	 */
	stream = NXOpenFile(fileno(stdout), NX_WRITEONLY);
	if(rld_load_basefile(stream, a_outname) == 0){
	    NXFlush(stream);
	    fflush(stdout);
	    fatal("can't load: %s as base file", a_outname);
	}
	/*
	 * Preform an rld_load() for each state at the state's address.
	 */
	for(i = 0; i < grld_nloaded_states; i++){
	    link_edit_address = (uint32_t)grld_loaded_state[i].header_addr;
	    rld_address_func(address_func);
	    if(rld_load(stream, &(headers[i]),
			grld_loaded_state[i].object_filenames,
			RLD_DEBUG_OUTPUT_FILENAME) == 0){
		NXFlush(stream);
		fflush(stdout);
		fatal("rld_load() failed");
	    }
	}

	/*
	 * Pass 1 count symbols
	 */
	save_nname = nname;
	for(i = 0; i < grld_nloaded_states; i++){
	    st = NULL;
	    object_addr = (char *)headers[i];
	    lc = (struct load_command *)
		 (object_addr + sizeof(struct mach_header));
	    for(j = 0; j < headers[i]->ncmds; j++){
		if(st == NULL && lc->cmd == LC_SYMTAB){
		    st = (struct symtab_command *)lc;
		    count_func_symbols((struct nlist *)
				       (object_addr + st->symoff),
				       st->nsyms,
				       object_addr + st->stroff,
				       st->strsize);
		}
		lc = (struct load_command *)((char *)lc + lc->cmdsize);
	    }
	}
	/*
	 * Reallocate the data structures for the symbols.
	 */
	nl = (nltype *)realloc(nl, (nname + 1) * sizeof(nltype));
	if(nl == NULL)
	    fatal("No room for %lu bytes of symbol table\n",
		  (nname + 1) * sizeof(nltype));
	npe = nl + (save_nname + 1);
	memset(npe, '\0', (nname - save_nname) * sizeof(nltype));
	/*
	 * Pass 2 load symbols.
	 */
	for(i = 0; i < grld_nloaded_states; i++){
	    st = NULL;
	    object_addr = (char *)headers[i];
	    lc = (struct load_command *)
		 (object_addr + sizeof(struct mach_header));
	    for(j = 0; j < headers[i]->ncmds; j++){
		if(st == NULL && lc->cmd == LC_SYMTAB){
		    st = (struct symtab_command *)lc;
		    load_func_symbols((struct nlist *)
				      (object_addr + st->symoff),
				      st->nsyms,
				      object_addr + st->stroff,
				      st->strsize,
				      0);
		}
		lc = (struct load_command *)((char *)lc + lc->cmdsize);
	    }
	}
#ifdef DEBUG
	if(debug & RLDDEBUG){
	    for(i = save_nname + 1; i < nname + 1; i++){
		printf("[get_rld_state_symbols] 0x%08x\t%s\n",
			(unsigned int)nl[i].value, nl[i].name);
	    }
	}
#endif /* DEBUG */
	/*
	 * Resort the symbol table.
	 */
	qsort(nl, nname + 1, sizeof(nltype),
	      (int (*)(const void *, const void *))valcmp);
	free(headers);
}

static
uint32_t
address_func(
uint32_t size,
uint32_t headers_size)
{
	return(link_edit_address);
}
#endif /* defined(__OPENSTEP__) */

void
get_dyld_state_symbols(
void)
{
    uint32_t i, j, save_nname, ncmds;
    struct ofile *ofiles;
    struct load_command *lc;
    struct segment_command *sg;
    struct segment_command_64 *sg64;
    struct symtab_command *st;
    struct nlist *symbols;
    struct nlist_64 *symbols64;

	if(image_count == 0)
	    return;
	/*
	 * Create an ofile for each image.
	 */
	ofiles = allocate(image_count * sizeof(struct ofile));
	for(i = 0; i < image_count; i++){
	    if(ofile_map(dyld_images[i].name, &host_arch_flag, NULL, ofiles + i,
			 FALSE) == 0)
		fatal("ofile_map() failed");
	    if(ofiles[i].mh == NULL && ofiles[i].mh64 == NULL)
		fatal("file from dyld loaded state: %s is not a Mach-O file",
		      dyld_images[i].name);
	}

	/*
	 * Pass 1 count symbols
	 */
	save_nname = nname;
	for(i = 0; i < image_count; i++){
	    st = NULL;
	    lc = ofiles[i].load_commands;
	    if(ofiles[i].mh != NULL)
		ncmds = ofiles[i].mh->ncmds;
	    else
		ncmds = ofiles[i].mh64->ncmds;
	    for(j = 0; j < ncmds; j++){
		if(st == NULL && lc->cmd == LC_SYMTAB){
		    st = (struct symtab_command *)lc;
		    if(ofiles[i].mh != NULL){
			symbols = (struct nlist *)
				  (ofiles[i].object_addr + st->symoff);
			symbols64 = NULL;
		    }
		    else{
			symbols64 = (struct nlist_64 *)
				    (ofiles[i].object_addr + st->symoff);
			symbols = NULL;
		    }
		    count_func_symbols(symbols, symbols64, st->nsyms,
				       ofiles[i].object_addr + st->stroff,
				       st->strsize);
		}
		if(dyld_images[i].image_header != 0 &&
		   dyld_images[i].vmaddr_slide == 0){
		    if(lc->cmd == LC_SEGMENT){
			sg = (struct segment_command *)lc;
			if(sg->filesize != 0){
			    dyld_images[i].vmaddr_slide = 
				dyld_images[i].image_header - sg->vmaddr;
			}
		    }
		    else if(lc->cmd == LC_SEGMENT_64){
			sg64 = (struct segment_command_64 *)lc;
			if(sg64->filesize != 0){
			    dyld_images[i].vmaddr_slide = 
				dyld_images[i].image_header - sg64->vmaddr;
			}
		    }
		}
		lc = (struct load_command *)((char *)lc + lc->cmdsize);
	    }
	}
	/*
	 * Reallocate the data structures for the symbols.
	 */
	nl = (nltype *)realloc(nl, (nname + 1) * sizeof(nltype));
	if(nl == NULL)
	    fatal("No room for %lu bytes of symbol table\n",
		  (nname + 1) * sizeof(nltype));
	npe = nl + (save_nname + 1);
	memset(npe, '\0', (nname - save_nname) * sizeof(nltype));
	/*
	 * Pass 2 load symbols.
	 */
	for(i = 0; i < image_count; i++){
	    st = NULL;
	    lc = ofiles[i].load_commands;
	    if(ofiles[i].mh != NULL)
		ncmds = ofiles[i].mh->ncmds;
	    else
		ncmds = ofiles[i].mh64->ncmds;
	    for(j = 0; j < ncmds; j++){
		if(st == NULL && lc->cmd == LC_SYMTAB){
		    st = (struct symtab_command *)lc;
		    if(ofiles[i].mh != NULL){
			symbols = (struct nlist *)
				  (ofiles[i].object_addr + st->symoff);
			symbols64 = NULL;
		    }
		    else{
			symbols64 = (struct nlist_64 *)
				    (ofiles[i].object_addr + st->symoff);
			symbols = NULL;
		    }
		    load_func_symbols(symbols, symbols64, st->nsyms,
				      ofiles[i].object_addr + st->stroff,
				      st->strsize,
				      dyld_images[i].vmaddr_slide);
		}
		lc = (struct load_command *)((char *)lc + lc->cmdsize);
	    }
	}
#ifdef DEBUG
	if(debug & DYLDDEBUG){
	    for(i = save_nname + 1; i < nname + 1; i++){
		printf("[get_dyld_state_symbols] ");
		if(ofiles[0].mh != NULL)
		    printf("0x%08x", (unsigned int)nl[i].value);
		else
		    printf("%016llx", nl[i].value);
		printf("\t%s\n", nl[i].name);
	    }
	}
#endif /* DEBUG */
	/*
	 * Resort the symbol table.
	 */
	qsort(nl, nname + 1, sizeof(nltype),
	      (int (*)(const void *, const void *))valcmp);
	free(ofiles);
}

static
void
count_func_symbols(
struct nlist *symbols,
struct nlist_64 *symbols64,
uint32_t nsymbols,
char *strings,
uint32_t strsize)
{
    uint32_t i;
    uint32_t n_strx;
    uint8_t n_type;
    uint8_t n_sect;

	for(i = 0; i < nsymbols; i++){
	    if(symbols != NULL){
		n_strx = symbols[i].n_un.n_strx;
		n_type = symbols[i].n_type;
		n_sect = symbols[i].n_sect;
	    }
	    else{
		n_strx = symbols64[i].n_un.n_strx;
		n_type = symbols64[i].n_type;
		n_sect = symbols64[i].n_sect;
	    }
	    if(n_strx != 0 && n_strx < strsize){
		if(funcsymbol(n_type, n_sect, strings + n_strx))
		    nname++;
	    }
	}
}

static
void
load_func_symbols(
struct nlist *symbols,
struct nlist_64 *symbols64,
uint32_t nsymbols,
char *strings,
uint32_t strsize,
uint64_t vmaddr_slide)
{
    uint32_t i;
    uint32_t n_strx;
    uint8_t n_type;
    uint8_t n_sect;
    uint64_t n_value;

	for(i = 0; i < nsymbols; i++){
	    if(symbols != NULL){
		n_strx = symbols[i].n_un.n_strx;
		n_type = symbols[i].n_type;
		n_sect = symbols[i].n_sect;
		n_value = symbols[i].n_value;
	    }
	    else{
		n_strx = symbols64[i].n_un.n_strx;
		n_type = symbols64[i].n_type;
		n_sect = symbols64[i].n_sect;
		n_value = symbols64[i].n_value;
	    }
	    if(n_strx != 0 && n_strx < strsize){
		if(funcsymbol(n_type, n_sect, strings + n_strx)){
		    npe->value = n_value + vmaddr_slide;
		    npe->name = strings + n_strx;
		    npe++;
		}
	    }
	}
}

static
enum bool
funcsymbol(
uint8_t n_type,
uint8_t n_sect,
char *name)
{
    int type;

	/*
	 *	must be a text symbol,
	 *	and static text symbols don't qualify if aflag set.
	 */
	if(n_type & N_STAB)
	    return(FALSE);
	type = n_type & N_TYPE;
	if(type == N_SECT && n_sect == 1)
	    type = N_TEXT;
	if(type != N_TEXT)
	    return FALSE;
	if((!(n_type & N_EXT)) && aflag)
	    return(FALSE);
	/*
	 * can't have any `funny' characters in name,
	 * where `funny' includes	`.', .o file names
	 *			and	`$', pascal labels.
	 */
	for( ; *name ; name += 1 ){
	    if(*name == '.' || *name == '$'){
		return(FALSE);
	    }
	}
	return(TRUE);
}

static
int
valcmp(
nltype *p1,
nltype *p2)
{
	if(p1->value < p2->value){
	    return(LESSTHAN);
	}
	if(p1->value > p2->value){
	    return(GREATERTHAN);
	}
	return(EQUALTO);
}

static
void
count_N_SO_stabs(
struct nlist *symbols,
struct nlist_64 *symbols64,
uint32_t nsymbols,
char *strings,
uint32_t strsize)
{
    uint32_t i, len;
    char *name;
    uint32_t n_strx;
    uint8_t n_type;
    uint64_t n_value;

	for(i = 0; i < nsymbols; i++){
	    if(symbols != NULL){
		n_strx = symbols[i].n_un.n_strx;
		n_type = symbols[i].n_type;
		n_value = symbols[i].n_value;
	    }
	    else{
		n_strx = symbols64[i].n_un.n_strx;
		n_type = symbols64[i].n_type;
		n_value = symbols64[i].n_value;
	    }
	    if(n_type == N_SO){
		/* skip the N_SO for the directory name that ends in a '/' */
		if(n_strx != 0 && n_strx < strsize){
		    name = strings + n_strx;
		    len = strlen(name);
		    if(len != 0 && name[len-1] == '/')
			continue;
		}
		n_files++;
	    }
	}
}

static
void
load_files(
struct nlist *symbols,
struct nlist_64 *symbols64,
uint32_t nsymbols,
char *strings,
uint32_t strsize)
{
    uint32_t i;
    char *s, *name;
    int len;
    int oddeven;
    uint32_t n_strx;
    uint8_t n_type;
    uint64_t n_value;

	oddeven = 0;
	for(i = 0; i < nsymbols; i++){
	    if(symbols != NULL){
		n_strx = symbols[i].n_un.n_strx;
		n_type = symbols[i].n_type;
		n_value = symbols[i].n_value;
	    }
	    else{
		n_strx = symbols64[i].n_un.n_strx;
		n_type = symbols64[i].n_type;
		n_value = symbols64[i].n_value;
	    }
	    if(n_type == N_SO){
		/* skip the N_SO for the directory name that ends in a '/' */
		if(n_strx != 0 && n_strx < strsize){
		    name = strings + n_strx;
		    len = strlen(name);
		    if(len != 0 && name[len-1] == '/')
			continue;
		}
		if(oddeven){
		    files[n_files++].lastpc = n_value;
		    oddeven = 0;
		}
		else {
		    s = strings + n_strx;
		    len = strlen(s);
		    if(len > 0)
			s[len-1] = 'o';
		    files[n_files].name = files[n_files].what_name = s;
		    files[n_files].firstpc = n_value; 
		    oddeven = 1;
		}
	    }
	}
#ifdef notdef
	for(i = 0; i < n_files; i++){
	    fprintf(stderr, "files[%lu] firstpc = 0x%x name = %s\n", i,
		    (unsigned int)(files[i].firstpc), files[i].name);
	}
#endif
}

void
get_text_min_max(
uint64_t *text_min,
uint64_t *text_max)
{
    uint32_t i, j, ncmds;
    struct load_command *lc;
    struct segment_command *sg;
    struct segment_command_64 *sg64;
    struct section *s;
    struct section_64 *s64;

	*text_min = 0;
	if(ofile.mh != NULL){
	    *text_max = 0xffffffff;
	    ncmds = ofile.mh->ncmds;
	}
	else{
	    *text_max = 0xffffffffffffffffULL;
	    ncmds = ofile.mh64->ncmds;
	}

	lc = ofile.load_commands;
	for (i = 0; i < ncmds; i++){
	    if(lc->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc;
		s = (struct section *)
		      ((char *)sg + sizeof(struct segment_command));
		for(j = 0; j < sg->nsects; j++){
		    if(strcmp(s->sectname, SECT_TEXT) == 0 &&
		       strcmp(s->segname, SEG_TEXT) == 0){
			*text_min = s->addr;
			*text_max = s->addr + s->size;
			return;
		    }
		    s++;
		}
	    }
	    else if(lc->cmd == LC_SEGMENT_64){
		sg64 = (struct segment_command_64 *)lc;
		s64 = (struct section_64 *)
		      ((char *)sg64 + sizeof(struct segment_command_64));
		for(j = 0; j < sg64->nsects; j++){
		    if(strcmp(s64->sectname, SECT_TEXT) == 0 &&
		       strcmp(s64->segname, SEG_TEXT) == 0){
			*text_min = s64->addr;
			*text_max = s64->addr + s64->size;
			return;
		    }
		    s++;
		}
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
}
