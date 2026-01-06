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
#include <mach/mach.h> /* first so to get rid of a precomp warning */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/stab.h>
#include <mach-o/arch.h>
#include "stuff/bool.h"
#include "stuff/ofile.h"
#include "stuff/errors.h"
#include "stuff/allocate.h"
#include "stuff/guess_short_name.h"
char *progname = NULL;

static void nm(
    struct ofile *ofile,
    char *arch_name,
    void *cookie);

static char * get_full_path(
    char *short_name,
    struct ofile *ofile);

/* Define the structures used by SymInfo */
typedef struct {
    char *name;
    const char *arch;
    char *ordinal; /* for imports only */
}_SymInfoSymbol;
typedef _SymInfoSymbol *SymInfoSymbol;

typedef struct {
    char **subUmbrellas;
    char **subFrameworks;
    unsigned int nSubFrameworks;
    unsigned int nSubUmbrellas;
}_SymInfoDependencies;
typedef _SymInfoDependencies *SymInfoDependencies;

typedef struct {
    SymInfoSymbol *exports;
    SymInfoSymbol *imports;
    SymInfoDependencies dependencies;
    unsigned int nExports;
    unsigned int nImports;
    char *path;
    char *shortName;
}_SymInfoList;

typedef _SymInfoList *SymInfoList;
#define __SymInfoTypes__
#include <cbt/libsyminfo.h>


/* flags to control the nm callback */
struct cmd_flags {
    enum bool g;        /* select only global symbols */
    enum bool u;        /* select only undefined symbols */
    enum bool d;	/* select only defined symbols, opposite of u */
    enum bool getLibDeps; /* Get the library dependency info like
			     SUB_UMBRELLA, SUB_FRAMEWORK, etc */
    enum bool import;	/* Flag to identify were looking for imported symbols
			   data */
};

/* flags set by processing a specific object file */
struct process_flags {
    uint32_t nlibs;		/* For printing the twolevel namespace */
    char **lib_names;		/*  references types, the number of libraries */
				/*  an array of pointers to library names */
};

/* the internal symbol structure used in this file */
struct symbol {
    char *name;
    char *indr_name;
    struct nlist_64 nl;
};

static void set_symbol_names(
    struct symbol *symbols,
    uint32_t nsymbols,
    char *strings,
    uint32_t strsize);
static enum bool select_symbol(
    struct symbol *symbol,
    struct cmd_flags *cmd_flags,
    struct process_flags *process_flags);
    static void make_symbol_32(struct symbol *symbol,
    struct nlist *nl);
static void make_symbol_32(
    struct symbol *symbol,
    struct nlist *nl);
static void make_symbol_64(
    struct symbol *symbol,
    struct nlist_64 *nl);
static struct symbol *select_symbols(
    struct ofile *ofile,
    struct symtab_command *st,
    struct dysymtab_command *dyst,
    struct cmd_flags *cmd_flags,
    struct process_flags *process_flags,
    uint32_t *nsymbols,
    struct nlist *all_symbols,
    struct nlist_64 *all_symbols64);

/*
 * Super hack so values can get passed back from  the nm callback.
 * The only other way I see to do this is to copy in the ofile_process code
 * and edit it to return a value but that wouldn't work to well.  This is
 * ugly but effective.
 */
struct selectedSymbolListInfo {
    vm_size_t mappedFileSize;
    void *mappedFile;
    char *cachedFileName;
    int byteSex;
    struct nlist *all_symbols;
    struct nlist_64 *all_symbols64;
    struct symtab_command *st;
};
static struct selectedSymbolListInfo *gInfo;
static SymInfoList self;

SymInfoList
SymInfoCreate(
char *fileName)
{
    SymInfoList rList;
    struct cmd_flags cmd_flags = { 0 };
    kern_return_t r;

	/* Allocate return value and point the global pointer self at it */
	rList = malloc(sizeof(_SymInfoList));
	bzero(rList, sizeof(_SymInfoList));
	if(rList == NULL)
	    return(rList);
	self = rList;
	gInfo = allocate(sizeof(struct selectedSymbolListInfo));
	bzero(gInfo,sizeof(struct selectedSymbolListInfo));
	
	/*
	 * We want to process all the ofiles possible for this file.
	 * The cmd_flags are set in nm depending on what is being accomplished.
	 */
	ofile_process(fileName,	/* name */
		      NULL,	/* arch_flags */
		      0,	/* narch_flags */
		      TRUE,	/* all_archs */
		      FALSE,	/* process_non_objects */
		      FALSE,	/* dylib_flat */
		      TRUE,	/* use_member_syntax */
		      nm,	/* processor */
		      &cmd_flags); /* cookie */

	/* Clean up */
	if((r = vm_deallocate(mach_task_self(), (vm_address_t)gInfo->mappedFile,
			  (vm_size_t)gInfo->mappedFileSize)) != KERN_SUCCESS){
	    my_mach_error(r, "Can't vm_deallocate mapped memory for file: "
		   "%s",fileName);
	}
	free(gInfo->cachedFileName);
	return(rList);
}

void
SymInfoFreeSymbol(
SymInfoSymbol symbol)
{
	if(symbol == NULL){
	    printf("Null symbol, not doing a thing\n");
	    return;
	}
	if(symbol->name)
	    free(symbol->name);
	if(symbol->ordinal)
	    free(symbol->ordinal);
	if(symbol->arch)
	    free((char *)symbol->arch);
	if(symbol)
	    free(symbol);
}

void
SymInfoFree(
SymInfoList nmList)
{
    uint32_t i;
    
	if(nmList == NULL)
	    return;
	
	/* Free the Symbols */
	for(i = 0; i < nmList->nExports; i++)
	    SymInfoFreeSymbol(nmList->exports[i]);
	if(nmList->exports)
	    free(nmList->exports);
	for(i = 0; i < nmList->nImports; i++)
	    SymInfoFreeSymbol(nmList->imports[i]);
	if(nmList->imports)
	    free(nmList->imports);
	
	/* Free the Dependencies */
	SymInfoFreeDependencies(nmList->dependencies);

	/* Free the rest */
	if(nmList->path)
	    free(nmList->path);
	
	if(nmList->shortName)
	    free(nmList->shortName);
	if(nmList)
	    free(nmList);
	if(gInfo)
	    free(gInfo);
}

void
SymInfoFreeDependencies(
SymInfoDependencies deps)
{
    uint32_t i;
    
	if(deps == NULL)
	    return;
	
	/* Free the subumbrellas */
	for(i = 0; i < deps->nSubUmbrellas; i++){
	    if(deps->subUmbrellas[i])
		free(deps->subUmbrellas[i]);
	}
	if(deps->subUmbrellas)
	    free(deps->subUmbrellas);
	
	/* Free the subFrameworks */
	for(i = 0; i < deps->nSubFrameworks; i++){
	    if(deps->subFrameworks[i])
		free(deps->subFrameworks[i]);
	}
	if(deps->subFrameworks)
	    free(deps->subFrameworks);
	free(deps);
}

/* Access functions */
SymInfoSymbol *
SymInfoGetImports(
SymInfoList nmList)
{
	if(nmList == NULL)
	    return(NULL);
	return(nmList->imports);
}

SymInfoSymbol *
SymInfoGetExports(
SymInfoList nmList)
{
	if(nmList == NULL)
	    return(NULL);
	return(nmList->exports);
}

SymInfoDependencies
SymInfoGetLibraryInfo(
SymInfoList nmList)
{
	if(nmList == NULL)
	    return(NULL);
	return(nmList->dependencies);
}

unsigned int
SymInfoGetSubFrameworkCount(
SymInfoDependencies deps)
{
	if(deps == NULL)
	    return(0);
	return(deps->nSubFrameworks);
}

unsigned int
SymInfoGetSubUmbrellaCount(
SymInfoDependencies deps)
{
	if(deps == NULL)
	    return(0);
	return(deps->nSubUmbrellas);
}

char **
SymInfoGetSubUmbrellas(
SymInfoDependencies deps)
{
	if(deps == NULL)
	    return(NULL);
	return(deps->subUmbrellas);
}

char **
SymInfoGetSubFrameworks(
SymInfoDependencies deps)
{
	if(deps == NULL)
	    return(NULL);
	return(deps->subFrameworks);
}

char *
SymInfoGetSymbolName(
SymInfoSymbol symbol)
{
	if(symbol == NULL)
	    return(NULL);
	return(symbol->name);
}

const char *
SymInfoGetSymbolArch(
SymInfoSymbol symbol)
{
	if(symbol == NULL)
	    return(NULL);    
	return(symbol->arch);
}

char *
SymInfoGetSymbolOrdinal(
SymInfoSymbol symbol)
{
	if(symbol == NULL)
	    return(NULL);
	return(symbol->ordinal);
}

unsigned int
SymInfoGetExportCount(
SymInfoList nmList)
{
	if(nmList == NULL)
	    return(0);
	return(nmList->nExports);
}

unsigned int
SymInfoGetImportCount(
SymInfoList nmList)
{
	if(nmList == NULL)
	    return(0);
	return(nmList->nImports);
}

char *
SymInfoGetShortName(
SymInfoList nmList)
{
	if(nmList == NULL)
	    return(NULL);
	return(nmList->shortName);
}

/* Function for creating SymInfoSymbol */
SymInfoSymbol
SymInfoCreateSymbols(
char *name,
char *arch,
char *ordinal) 
{
	SymInfoSymbol symbol;
	symbol = malloc(sizeof(_SymInfoSymbol));
	symbol->name = name;
	symbol->arch = arch;
	symbol->ordinal = ordinal;
	return(symbol);
}

SymInfoDependencies
SymInfoCreateDependencies(
char **subUmbrellas,
char **subFrameworks,
int nSubUmbrellas,
int nSubFrameworks)
{
    SymInfoDependencies deps;

	deps = malloc(sizeof(_SymInfoDependencies));
	bzero(deps,sizeof(_SymInfoDependencies));
	deps->subUmbrellas = subUmbrellas;
	deps->subFrameworks = subFrameworks;
	deps->nSubUmbrellas = nSubUmbrellas;
	deps->nSubFrameworks = nSubFrameworks;
	return(deps);
}

/*
 * nm() is the processor routine that will extract export and import info as
 * well as the library information.
 */
static
void
nm(
struct ofile *ofile,
char *arch_name,
void *cookie)
{
    struct cmd_flags *cmd_flags;
    struct process_flags process_flags;
    uint32_t i, j;
    struct load_command *lc;
    struct symtab_command *st;
    struct dysymtab_command *dyst;
    struct dylib_command *dl;
    uint32_t library_ordinal;
    uint32_t strsize = 0;
    char *strings = NULL; 
    struct symbol *symbols = NULL;
    uint32_t nsymbols;
    uint32_t nlibnames = 0;
    char *short_name, *has_suffix;
    enum bool is_framework;
    int symbolIndex;
    struct nlist *all_symbols;
    struct nlist_64 *all_symbols64;
    struct dylib_module m;
    struct dylib_module_64 m64;
    struct dylib_reference *refs;
    uint32_t ncmds, mh_flags;
    
	cmd_flags = (struct cmd_flags *)cookie;
	process_flags.nlibs = 0;
	process_flags.lib_names = NULL;

	if(ofile->mh != NULL){
	    ncmds = ofile->mh->ncmds;
	    mh_flags = ofile->mh->flags;
	}
	else{
	    ncmds = ofile->mh64->ncmds;
	    mh_flags = ofile->mh64->flags;
	}
	st = NULL;
	dyst = NULL;
	lc = ofile->load_commands;
	for(i = 0; i < ncmds; i++){
	    if(st == NULL && lc->cmd == LC_SYMTAB){
		st = (struct symtab_command *)lc;
	    }
	    else if(dyst == NULL && lc->cmd == LC_DYSYMTAB){
		dyst = (struct dysymtab_command *)lc;
	    }
	    else if((mh_flags & MH_TWOLEVEL) == MH_TWOLEVEL &&
		    (lc->cmd == LC_LOAD_DYLIB ||
		     lc->cmd == LC_LOAD_WEAK_DYLIB ||
		     lc->cmd == LC_REEXPORT_DYLIB ||
		     lc->cmd == LC_LOAD_UPWARD_DYLIB)){
		process_flags.nlibs++;
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	if(st == NULL || st->nsyms == 0){
	    /* If there is no name list we don't really care */
	    return;
	}
	if((mh_flags & MH_TWOLEVEL) == MH_TWOLEVEL &&
	    process_flags.nlibs > 0){
	    process_flags.lib_names = (char **)
			malloc(sizeof(char *) * process_flags.nlibs);
	    j = 0;
	    lc = ofile->load_commands;
	    for(i = 0; i < ncmds; i++){
		if(lc->cmd == LC_LOAD_DYLIB ||
		   lc->cmd == LC_LOAD_WEAK_DYLIB ||
		   lc->cmd == LC_REEXPORT_DYLIB ||
		   lc->cmd == LC_LOAD_UPWARD_DYLIB){
		    dl = (struct dylib_command *)lc;
		    process_flags.lib_names[j] =
			savestr((char *)dl + dl->dylib.name.offset);
		    short_name = guess_short_name(process_flags.lib_names[j],
						    &is_framework, &has_suffix);
		    if(has_suffix)
			free(has_suffix);

		    if(short_name != NULL)
			process_flags.lib_names[j] = short_name;
		    j++;
		}
		lc = (struct load_command *)((char *)lc + lc->cmdsize);
	    }
	    nlibnames = j;
	}
	/* Correct the endianness of this ofile */
	if(ofile->mh != NULL){
	    all_symbols = (struct nlist *)(ofile->object_addr + st->symoff);
	    all_symbols64 = NULL;
	}
	else{
	    all_symbols = NULL;
	    all_symbols64 = (struct nlist_64 *)(ofile->object_addr +st->symoff);
	}

	if(ofile->object_byte_sex != get_host_byte_sex()){
	    /* 
	     * Cache the symbols whose endianness has been flipped so it doesn't
	     * have to be done again
	     */
	    if(gInfo->cachedFileName &&
		  (strcmp(gInfo->cachedFileName,ofile->file_name) == 0) &&
		  gInfo->byteSex == (int)ofile->object_byte_sex && 
		  st->symoff == gInfo->st->symoff &&
		  st->nsyms == gInfo->st->nsyms &&
		  st->stroff == gInfo->st->stroff &&
		  st->strsize == gInfo->st->strsize){
		all_symbols = gInfo->all_symbols;
		all_symbols64 = gInfo->all_symbols64;
	    }
	    else{
		if(ofile->mh != NULL)
		    swap_nlist(all_symbols, st->nsyms, get_host_byte_sex());
		else
		    swap_nlist_64(all_symbols64, st->nsyms,get_host_byte_sex());
		gInfo->byteSex =  ofile->object_byte_sex;
		if(gInfo->cachedFileName){
		    free(gInfo->cachedFileName);
		    gInfo->cachedFileName = NULL;
		}
		gInfo->cachedFileName = savestr(ofile->file_name);
		gInfo->all_symbols = all_symbols;
		gInfo->all_symbols64 = all_symbols64;
		gInfo->st = st;
	    }
	}
	if(ofile->dylib_module != NULL){
	    m = *ofile->dylib_module;
	    refs = (struct dylib_reference *)(ofile->object_addr +
					dyst->extrefsymoff);
	    if(ofile->object_byte_sex != get_host_byte_sex()){
		swap_dylib_module(&m, 1, get_host_byte_sex());
		swap_dylib_reference(refs + m.irefsym, m.nrefsym,
			get_host_byte_sex());
	    }
	}
	else if(ofile->dylib_module64 != NULL){
	    m64 = *ofile->dylib_module64;
	    refs = (struct dylib_reference *)(ofile->object_addr +
					dyst->extrefsymoff);
	    if(ofile->object_byte_sex != get_host_byte_sex()){
		swap_dylib_module_64(&m64, 1, get_host_byte_sex());
		swap_dylib_reference(refs + m64.irefsym, m64.nrefsym,
			get_host_byte_sex());
	    }
	}

	/* select export symbols to return */
	cmd_flags->g = TRUE;
	cmd_flags->d = TRUE;
	symbols = select_symbols(ofile, st, dyst, cmd_flags, &process_flags,
				 &nsymbols, all_symbols, all_symbols64);
	strings = ofile->object_addr + st->stroff;
	strsize = st->strsize;
	set_symbol_names(symbols, nsymbols, strings, strsize);

	self->nExports += nsymbols;
	
	/* Store all this info so that it can be cleaned up later */
	gInfo->mappedFile = ofile->file_addr;
	gInfo->mappedFileSize = ofile->file_size;

	/* Reallocate the array of SymInfoSymbol export structs in self */
	self->exports = reallocate(self->exports,
				   sizeof(SymInfoSymbol *) * self->nExports);

	/*
	 * Loop through saving the symbol information in SymInfoSymbol structs.
	 */
	for(i = self->nExports - nsymbols; i < self->nExports; i++){
	    const NXArchInfo *archInfo;
	    symbolIndex = i - (self->nExports - nsymbols);
	    self->exports[i] = malloc(sizeof(_SymInfoSymbol));

	    /* Save the symbol info */
	    archInfo = NXGetArchInfoFromCpuType(ofile->mh_cputype,
						CPU_SUBTYPE_MULTIPLE);
	    self->exports[i]->name = savestr(symbols[symbolIndex].name);
	    /* If we don't know the arch name use the number */
	    if(archInfo == NULL){
		char archString[10];
		sprintf(archString, "%d", ofile->mh_cputype);
		self->exports[i]->arch = savestr(archString);
	    }
	    else{
                self->exports[i]->arch = savestr(archInfo->name);
	    }
	    self->exports[i]->ordinal = NULL;
	}
	free(symbols);

	/* Unset the cmd_flags used to find exports */
	cmd_flags->d = FALSE;

	/* Set up the flags to get the symbol info for the imports */
	cmd_flags->g = TRUE;
	cmd_flags->u = TRUE;
	cmd_flags->import = TRUE;
	
	symbols = select_symbols(ofile, st, dyst, cmd_flags, &process_flags,
				 &nsymbols, all_symbols, all_symbols64);
	strings = ofile->object_addr + st->stroff;
	strsize = st->strsize;
	set_symbol_names(symbols, nsymbols, strings, strsize);

	self->nImports += nsymbols;

	/* Reallocate the array of SymInfoSymbol imports structs in self */
	self->imports = reallocate(self->imports,
				   sizeof(SymInfoSymbol *) * self->nImports);

	/*
	 * Loop through saving the symbol information in SymInfoSymbol structs.
	 */
	for(i = self->nImports - nsymbols; i < self->nImports; i++){
	    const NXArchInfo *archInfo;
	    symbolIndex = i - (self->nImports - nsymbols);
	    self->imports[i] = malloc(sizeof(_SymInfoSymbol));
	    archInfo = NXGetArchInfoFromCpuType(ofile->mh_cputype, 
						CPU_SUBTYPE_MULTIPLE);

	    /* Save the name and arch */
	    self->imports[i]->name = savestr(symbols[symbolIndex].name);
	    if(archInfo == NULL){
		char archString[10];
		sprintf(archString, "%d", ofile->mh_cputype);
		self->imports[i]->arch = savestr(archString);
	    }
	    else{
                self->imports[i]->arch = savestr(archInfo->name);
	    }

	    /* Now extract the ordinal info and save the short library name */
	    library_ordinal =
		GET_LIBRARY_ORDINAL(symbols[symbolIndex].nl.n_desc);
	    if(library_ordinal != 0){
		if(library_ordinal == EXECUTABLE_ORDINAL)
		    self->imports[i]->ordinal = savestr("from executable");
		else if(library_ordinal == DYNAMIC_LOOKUP_ORDINAL)
		    self->imports[i]->ordinal =savestr("dynamically looked up");
		else if(library_ordinal-1 >= process_flags.nlibs)
		    self->imports[i]->ordinal = savestr("bad library ordinal");
		else 
		    /* Find the full path to the library */
		    self->imports[i]->ordinal = 
		       get_full_path(process_flags.lib_names[library_ordinal-1],
				     ofile);
	    }
	    else{
		self->imports[i]->ordinal = NULL;
	    }
	}
	free(symbols);

	cmd_flags->g = FALSE;
	cmd_flags->u = FALSE;
	cmd_flags->import = FALSE;

	/* Now get the dependency information from the load commands */
	if(self->dependencies == NULL){
	    /* Allocate the memory */
	    self->dependencies = malloc(sizeof(_SymInfoDependencies));
	    bzero(self->dependencies, sizeof(_SymInfoDependencies));
	    lc = ofile->load_commands;

	    /* Now get the dependency info */
	    for(j = 0; j < ncmds; j++){
		char *p;
		if(lc->cmd == LC_SUB_UMBRELLA || lc->cmd == LC_SUB_LIBRARY){
		    struct sub_umbrella_command *usub = (struct sub_umbrella_command *)lc;
		    p = (char *)lc + usub->sub_umbrella.offset;
		    self->dependencies->subUmbrellas =
			reallocate(self->dependencies->subUmbrellas,
		    (self->dependencies->nSubUmbrellas+1)*
		    sizeof(char **));
		    self->dependencies ->
			subUmbrellas[self->dependencies->nSubUmbrellas] =
								 savestr(p);
		    self->dependencies->nSubUmbrellas++;
		}
		else if(lc->cmd == LC_SUB_FRAMEWORK){ 
		    struct sub_framework_command *subFramework = (struct sub_framework_command *)lc;
		    p = (char *)lc + subFramework->umbrella.offset;
		    self->dependencies-> subFrameworks =
			    reallocate(self->dependencies->subFrameworks,
		    		       (self->dependencies->nSubFrameworks+1) *
				       sizeof(char **));
		    self->dependencies->
			subFrameworks[self->dependencies->nSubFrameworks] =
								    savestr(p);
		    self->dependencies->nSubFrameworks++;
		}
		lc = (struct load_command *)((char *)lc + lc->cmdsize);
	    }

	    /*
	    * Save the short name for this binary, to be used
	    * to map short name to full path
	    */
	    short_name =
		guess_short_name(ofile->file_name, &is_framework, &has_suffix);
	    if(has_suffix)
		free(has_suffix);
	    if(short_name){
		self->shortName = short_name;
	    }
	}
	
	/* Free the memory that was malloced in this function */
	for(i = 0; i < process_flags.nlibs; i++)
	    free(process_flags.lib_names[i]);
	if(process_flags.lib_names)
	    free(process_flags.lib_names);
        if(ofile->arch_flag.name != NULL) {
           free(ofile->arch_flag.name);
           ofile->arch_flag.name = NULL;
        }
}

/*
 * set_symbol_names() sets the name and the indr_name fields of the symbols
 * passed to it from the symbol table pass to it.
 */
static
void
set_symbol_names(
struct symbol *symbols,
uint32_t nsymbols,
char *strings,
uint32_t strsize)
{
    uint32_t i;

	for(i = 0; i < nsymbols; i++){
	    if(symbols[i].nl.n_un.n_strx == 0)
		symbols[i].name = "";
	    else if((int)symbols[i].nl.n_un.n_strx < -1 ||
		    (uint32_t)symbols[i].nl.n_un.n_strx > strsize){
		symbols[i].name = "bad string index";
		printf ("Setting bad string index in exports\n");
	    }
	    else
		symbols[i].name = strings + symbols[i].nl.n_un.n_strx;
	    if((symbols[i].nl.n_type & N_TYPE) == N_INDR){
		if(symbols[i].nl.n_value == 0)
		    symbols[i].indr_name = "";
		else if(symbols[i].nl.n_value > strsize){
		    symbols[i].indr_name = "bad string index";
		    printf ("Setting bad string index in exports2\n");
		}
		else
		    symbols[i].indr_name = strings + symbols[i].nl.n_value;
	    }
	}
}

static
char *
get_full_path(
char *short_name,
struct ofile *ofile) 
{
    uint32_t j;
    struct dylib_command *dl;
    struct load_command *lc;
    char *has_suffix;
    enum bool is_framework;
    uint32_t ncmds;
    
	lc = ofile->load_commands;
	if(ofile->mh != NULL)
	    ncmds = ofile->mh->ncmds;
	else
	    ncmds = ofile->mh64->ncmds;

	for(j = 0; j < ncmds; j++) {
	    if(lc->cmd == LC_LOAD_DYLIB ||
	       lc->cmd == LC_LOAD_WEAK_DYLIB ||
	       lc->cmd == LC_REEXPORT_DYLIB ||
	       lc->cmd == LC_LOAD_UPWARD_DYLIB){
		char *longName;
		char *shortName;
		char *returnLongName;
		dl = (struct dylib_command *)lc;
		longName = (char *)lc + dl->dylib.name.offset;
		shortName = guess_short_name(longName, &is_framework,
					     &has_suffix);
		if(shortName && strcmp(shortName, short_name) == 0){
		    free(shortName);
		    returnLongName = malloc(strlen(longName) + 2);
		    returnLongName[0] = '.';
		    returnLongName[1] = '\0';
		    strcat(returnLongName, longName);
		    return(returnLongName);
		}
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	/*
	fprintf(stderr,
		"WARNING: Couldn't find full path for: %s\n", short_name);
	*/
     
	return(savestr(short_name));
}

static
struct symbol *
select_symbols(
struct ofile *ofile,
struct symtab_command *st,
struct dysymtab_command *dyst,
struct cmd_flags *cmd_flags,
struct process_flags *process_flags,
uint32_t *nsymbols,
struct nlist *all_symbols,
struct nlist_64 *all_symbols64)
{
    uint32_t i, flags;
    struct symbol *selected_symbols, symbol;
    struct dylib_module m;
    struct dylib_module_64 m64;
    struct dylib_reference *refs;
    uint32_t irefsym, nrefsym, nextdefsym, iextdefsym, nlocalsym, ilocalsym;

	selected_symbols = allocate(sizeof(struct symbol) * st->nsyms);
	*nsymbols = 0;

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
		       flags == REFERENCE_FLAG_UNDEFINED_LAZY)
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
 * select based on the flags.
 */
static
enum bool
select_symbol(
struct symbol *symbol,
struct cmd_flags *cmd_flags,
struct process_flags *process_flags)
{
	if((cmd_flags->import == TRUE) &&
	   (process_flags->nlibs > 0) &&
	   GET_LIBRARY_ORDINAL(symbol->nl.n_desc) == 0)
	    return(FALSE);
	
	if(cmd_flags->u == TRUE){
	    if((symbol->nl.n_type == (N_UNDF | N_EXT) &&
		symbol->nl.n_value == 0) ||
		symbol->nl.n_type == (N_PBUD | N_EXT))
		return(TRUE);
	    else
		return(FALSE);
	}

	if(cmd_flags->d == TRUE){
	    if((((symbol->nl.n_type & N_TYPE) != N_UNDF)
		    && ((symbol->nl.n_type & N_EXT) != 0))
		    && ((symbol->nl.n_type & N_TYPE) != N_PBUD))
		return(TRUE);
	    else
		return(FALSE);
	}

	if(cmd_flags->g == TRUE && (symbol->nl.n_type & N_EXT) == 0)
	    return(FALSE);

	return(TRUE);
}
