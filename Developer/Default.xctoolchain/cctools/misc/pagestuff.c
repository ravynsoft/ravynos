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
#include <stdint.h>
#import <stdio.h>
#import <stdlib.h>
#import <string.h>
#import "stuff/ofile.h"
#import "stuff/errors.h"
#import "stuff/allocate.h"

enum file_part_type {
    FP_FAT_HEADERS,
    FP_MACH_O,
    FP_EMPTY_SPACE
};
char *file_part_type_names[] = {
    "FP_FAT_HEADERS",
    "FP_MACH_O",
    "FP_EMPTY_SPACE"
};

struct file_part {
    uint64_t offset;
    uint64_t size;
    enum file_part_type type;
    struct mach_o_part *mp;
    struct mach_header *mh;
    struct mach_header_64 *mh64;
    struct symtab_command *st;
    struct nlist *symbols;
    struct nlist_64 *symbols64;
    char *strings; 
    struct file_part *prev;
    struct file_part *next;
};
struct file_part *file_parts = NULL;

enum mach_o_part_type {
    MP_MACH_HEADERS,
    MP_SECTION,
    MP_SECTION_64,
    MP_RELOCS,
    MP_RELOCS_64,
    MP_LOCAL_SYMBOLS,
    MP_EXTDEF_SYMBOLS,
    MP_UNDEF_SYMBOLS,
    MP_TOC,
    MP_MODULE_TABLE,
    MP_REFERENCE_TABLE,
    MP_INDIRECT_SYMBOL_TABLE,
    MP_EXT_RELOCS,
    MP_LOC_RELOCS,
    MP_SPLIT_INFO,
    MP_SYMBOL_TABLE,
    MP_HINTS_TABLE,
    MP_STRING_TABLE,
    MP_EXT_STRING_TABLE,
    MP_LOC_STRING_TABLE,
    MP_CODE_SIG,
    MP_FUNCTION_STARTS,
    MP_DATA_IN_CODE,
    MP_CODE_SIGN_DRS,
    MP_LINK_OPT_HINT,
    MP_DYLD_CHAINED_FIXUPS,
    MP_DYLD_EXPORTS_TRIE,
    MP_DYLD_INFO_REBASE,
    MP_DYLD_INFO_BIND,
    MP_DYLD_INFO_WEAK_BIND,
    MP_DYLD_INFO_LAZY_BIND,
    MP_DYLD_INFO_EXPORT,
    MP_EMPTY_SPACE
};
static char *mach_o_part_type_names[] = {
    "MP_MACH_HEADERS",
    "MP_SECTION",
    "MP_SECTION_64",
    "MP_RELOCS",
    "MP_RELOCS_64",
    "MP_LOCAL_SYMBOLS",
    "MP_EXTDEF_SYMBOLS",
    "MP_UNDEF_SYMBOLS",
    "MP_TOC",
    "MP_MODULE_TABLE",
    "MP_REFERENCE_TABLE",
    "MP_INDIRECT_SYMBOL_TABLE",
    "MP_EXT_RELOCS",
    "MP_LOC_RELOCS",
    "MP_SPLIT_INFO",
    "MP_SYMBOL_TABLE",
    "MP_HINTS_TABLE",
    "MP_STRING_TABLE",
    "MP_EXT_STRING_TABLE",
    "MP_LOC_STRING_TABLE",
    "MP_CODE_SIG",
    "MP_FUNCTION_STARTS",
    "MP_DATA_IN_CODE",
    "MP_CODE_SIGN_DRS",
    "MP_LINK_OPT_HINT",
    "MP_DYLD_CHAINED_FIXUPS",
    "MP_DYLD_EXPORTS_TRIE",
    "MP_DYLD_INFO_REBASE",
    "MP_DYLD_INFO_BIND",
    "MP_DYLD_INFO_WEAK_BIND",
    "MP_DYLD_INFO_LAZY_BIND",
    "MP_DYLD_INFO_EXPORT",
    "MP_EMPTY_SPACE"
};

struct mach_o_part {
    uint64_t offset;
    uint64_t size;
    enum mach_o_part_type type;
    struct section *s;
    struct section_64 *s64;
    struct mach_o_part *prev;
    struct mach_o_part *next;
};

/* The name of the program for error messages */
char *progname = NULL;

/* The ofile for the Mach-O file argument to the program */
static struct ofile ofile;

/* The -arch flag if any, and its offset and size in the file */
static struct arch_flag *arch_flag = NULL;
uint64_t arch_offset = 0;
uint64_t arch_size = 0;
enum bool arch_found = FALSE;

static struct nlist *sorted_symbols = NULL;
static struct nlist_64 *sorted_symbols64 = NULL;

static void usage(void);

static void create_file_parts(
    char *file_name);
static struct file_part *new_file_part(
    void);
static void insert_file_part(
    struct file_part *new);
static void print_file_parts(
    void);

static void create_mach_o_parts(
    struct file_part *fp);
static struct mach_o_part *new_mach_o_part(
    void);
static void insert_mach_o_part(
    struct file_part *fp,
    struct mach_o_part *new);
static void print_mach_o_parts(
    struct mach_o_part *mp);

static void print_parts_for_page(
    uint64_t page_number);
static void print_arch(
    struct file_part *fp);
static void print_file_part(
    struct file_part *fp);
static void print_mach_o_part(
    struct mach_o_part *mp);
static void print_symbols(
    struct file_part *fp,
    uint64_t low_addr,
    uint64_t high_addr);
static void print_symbols64(
    struct file_part *fp,
    uint64_t low_addr,
    uint64_t high_addr);
static int compare(
    struct nlist *p1,
    struct nlist *p2);
static int compare64(
    struct nlist_64 *p1,
    struct nlist_64 *p2);

/* apple_version is created by the libstuff/Makefile */
extern char apple_version[];
char *version = apple_version;

/*
 * pagestuff is invoked as follows:
 *
 *	% pagestuff mach-o [-arch name] [-p] [-a] pagenumber [pagenumber ...]
 *
 * It prints out what stuff is on the page numbers listed.
 */
int
main(
int argc,
char *argv[])
{
    int i, start;
    uint64_t j, page_number;
    char *endp;
    struct arch_flag a;

	progname = argv[0];
	if(argc < 3)
	    usage();
	start = 2;

	if(strcmp(argv[start], "-arch") == 0){
	    if(start + 1 == argc){
		error("missing argument to -arch option");
		exit(EXIT_FAILURE);
	    }
	    if(get_arch_from_flag(argv[start+1], &a) == 0){
		error("unknown architecture specification flag: "
		      "%s %s", argv[start], argv[start+1]);
		exit(EXIT_FAILURE);
	    }
	    arch_flag = &a;
	    start += 2;
	    if(argc < 5)
		usage();
        }

	create_file_parts(argv[1]);
	if(arch_flag != NULL && arch_found == FALSE){
	    error("file: %s does not contain architecture: %s", argv[1],
		  arch_flag->name);
	    exit(EXIT_FAILURE);
	}
	if(errors)
	    exit(EXIT_FAILURE);
	if(strcmp(argv[start], "-p") == 0){
	    print_file_parts();
	    start++;
	}
	if(errors)
	    exit(EXIT_FAILURE);
	if(start >= argc)
	    exit(EXIT_SUCCESS);

	if(strcmp(argv[start], "-a") == 0){
	    if(arch_flag == NULL)
		page_number = (ofile.file_size + vm_page_size - 1) /
			      vm_page_size;
	    else
		page_number = (arch_size + vm_page_size - 1) /
			      vm_page_size;
	    for(j = 0; j < page_number; j++){
		print_parts_for_page(j);
	    }
	    start++;
	}
	if(start >= argc)
	    exit(EXIT_SUCCESS);

	for(i = start; i < argc; i++){
	    page_number = strtoul(argv[i], &endp, 10);
	    if(*endp != '\0')
		fatal("page number argument: %s is not a proper unsigned "
		      "number", argv[i]);
	    print_parts_for_page(page_number);
	}

	return(EXIT_SUCCESS);
}

static
void
usage(void)
{
	fprintf(stderr, "Usage: %s mach-o [-arch name] [-p] [-a] "
		"pagenumber [pagenumber ...]\n",
		progname);
	exit(EXIT_FAILURE);
}

static
void
create_file_parts(
char *file_name)
{
    static struct file_part *fp;

	if(ofile_map(file_name, arch_flag, NULL, &ofile, FALSE) == FALSE)
	    exit(EXIT_FAILURE);

	/* first create an empty space for the whole file */
	fp = new_file_part();
	fp->offset = 0;
	fp->size = ofile.file_size;
	fp->type = FP_EMPTY_SPACE;
	file_parts = fp;

	if(ofile.file_type == OFILE_FAT){
	    fp = new_file_part();
	    fp->offset = 0;
	    fp->size = sizeof(struct fat_header);
	    if(ofile.fat_header->magic == FAT_MAGIC_64)
		fp->size += ofile.fat_header->nfat_arch *
			    sizeof(struct fat_arch_64);
	    else
		fp->size += ofile.fat_header->nfat_arch *
			    sizeof(struct fat_arch);
	    fp->type = FP_FAT_HEADERS;
	    insert_file_part(fp);

	    (void)ofile_first_arch(&ofile);
	    do{
		if(ofile.arch_type == OFILE_ARCHIVE){
		    error("for architecture: %s file: %s is an archive (not "
			"supported with %s)", ofile.arch_flag.name, file_name,
			progname);
		}
		else if(ofile.arch_type == OFILE_Mach_O){
		    if(arch_flag != NULL && arch_found == FALSE &&
		       arch_flag->cputype == ofile.mh_cputype &&
		       (arch_flag->cpusubtype & ~CPU_SUBTYPE_MASK) ==
			(ofile.mh_cpusubtype & ~CPU_SUBTYPE_MASK)){
			if(ofile.fat_header->magic == FAT_MAGIC_64){
			    arch_offset =
				ofile.fat_archs64[ofile.narch].offset; 
			    arch_size = ofile.fat_archs64[ofile.narch].size; 
			}
			else{
			    arch_offset = ofile.fat_archs[ofile.narch].offset; 
			    arch_size = ofile.fat_archs[ofile.narch].size; 
			}
			arch_found = TRUE;
		    }
		    /* make mach-o parts for this */
		    fp = new_file_part();
		    if(ofile.fat_header->magic == FAT_MAGIC_64){
			fp->offset = ofile.fat_archs64[ofile.narch].offset;
			fp->size = ofile.fat_archs64[ofile.narch].size;
		    }
		    else{
			fp->offset = ofile.fat_archs[ofile.narch].offset;
			fp->size = ofile.fat_archs[ofile.narch].size;
		    }
		    fp->type = FP_MACH_O;
		    insert_file_part(fp);
		    create_mach_o_parts(fp);
		}
		else if(ofile.arch_type == OFILE_UNKNOWN){
		    error("for architecture: %s file: %s is not an object file "
			"(not supported with %s)", ofile.arch_flag.name,
			file_name, progname);
		}
	    }while(ofile_next_arch(&ofile) == TRUE);
	}
	else if(ofile.file_type == OFILE_ARCHIVE){
	    error("file: %s is an archive (not supported with %s)", file_name,
		progname);
	}
	else if(ofile.file_type == OFILE_Mach_O){
	    if(arch_flag != NULL && arch_found == FALSE &&
	       arch_flag->cputype == ofile.mh_cputype &&
	       (arch_flag->cpusubtype & ~CPU_SUBTYPE_MASK) ==
		(ofile.mh_cpusubtype & ~CPU_SUBTYPE_MASK)){
		arch_offset = 0;
		arch_size = ofile.file_size;
		arch_found = TRUE;
	    }
	    /* make mach-o parts for this */
	    fp = new_file_part();
	    fp->offset = 0;
	    fp->size = ofile.file_size;
	    fp->type = FP_MACH_O;
	    insert_file_part(fp);
	    create_mach_o_parts(fp);
	}
	else{ /* ofile.file_type == OFILE_UNKNOWN */
	    error("file: %s is not an object file (not supported with %s)",
		file_name, progname);
	}
}

static
struct file_part *
new_file_part(
void)
{
    struct file_part *fp;

	fp = allocate(sizeof(struct file_part));
	memset(fp, '\0', sizeof(struct file_part));
	return(fp);
}

static
void
insert_file_part(
struct file_part *new)
{
    struct file_part *p, *q;

	for(p = file_parts; p != NULL; p = p->next){
	    /* find an existing part that contains the new part */
	    if(new->offset >= p->offset &&
	       new->offset + new->size <= p->offset + p->size){
		if(p->type != FP_EMPTY_SPACE)
		    fatal("internal error: new file part not contained in "
			"empty space");
		/* if new is the same as p replace p with new */
		if(new->offset == p->offset &&
	           new->size == p->size){
		    new->prev = p->prev;
		    new->next = p->next;
		    if(p->next != NULL)
			p->next->prev = new;
		    if(p->prev != NULL)
			p->prev->next = new;
		    if(file_parts == p)
			file_parts = new;
		    free(p);
		    return;
		}
		/* if new starts at p put new before p and move p's offset */
		if(new->offset == p->offset){
		    p->offset = new->offset + new->size;
		    p->size = p->size - new->size;
		    if(p->prev != NULL)
			p->prev->next = new;
		    new->prev = p->prev;
		    p->prev = new;
		    new->next = p;
		    if(file_parts == p)
			file_parts = new;
		    return;
		}
		/* if new ends at p put new after p and change p's size */
		if(new->offset + new->size == p->offset + p->size){
		    p->size = new->offset - p->offset;
		    new->next = p->next;
		    if(p->next != NULL)
			p->next->prev = new;
		    p->next = new;
		    new->prev = p;
		    return;
		}
		/* new is in the middle of p */
		q = new_file_part();
		q->type = FP_EMPTY_SPACE;
		q->offset = new->offset + new->size;
		q->size = p->offset + p->size - (new->offset + new->size);
		p->size = new->offset - p->offset;
		new->prev = p;
		new->next = q;
		q->prev = new;
		q->next = p->next;
		if(p->next != NULL)
		    p->next->prev = q;
		p->next = new;
		return;
	    }
	}
	fatal("internal error: new file part not found in existing part");
}

static
void
print_file_parts(
void)
{
    struct file_part *p, *prev;
    uint64_t offset;

	prev = NULL;
	offset = 0;
	for(p = file_parts; p != NULL; p = p->next){
	    if(arch_flag == NULL){
		printf("%s\n", file_part_type_names[p->type]);
		printf("    offset = %llu\n", p->offset);
		printf("    size = %llu\n", p->size);
	    }
	    if(prev != NULL)
		if(prev != p->prev)
		    printf("bad prev pointer\n");
	    prev = p;
	    if(offset != p->offset)
		    printf("bad offset\n");
	    offset += p->size;
	    if(p->type == FP_MACH_O){
		if(arch_flag == NULL || p->offset == arch_offset)
		    print_mach_o_parts(p->mp);
	    }
	}
}

static
void
create_mach_o_parts(
struct file_part *fp)
{
    uint32_t i, j;
    uint32_t ncmds, filetype;
    struct mach_o_part *mp;
    struct load_command *lc;
    struct symtab_command *st;
    struct dysymtab_command *dyst;
    struct dyld_info_command *dyld_info;
    struct twolevel_hints_command *hints;
    struct segment_command *sg;
    struct segment_command_64 *sg64;
    struct section *s;
    struct section_64 *s64;
    struct nlist *allocated_symbols, *symbols;
    struct nlist_64 *allocated_symbols64, *symbols64;
    uint32_t ext_low, ext_high, local_low, local_high, n_strx, n_type;
    char *strings;
    struct dylib_module *modtab;
    struct dylib_module_64 *modtab64;
    struct linkedit_data_command *split_info, *code_sig, *func_starts,
			         *data_in_code, *code_sign_drs;
    struct linkedit_data_command *link_opt_hint;
    struct linkedit_data_command *dyld_chained_fixups, *dyld_exports_trie;
    enum bool dylib_stub;

	mp = new_mach_o_part();
	mp->offset = fp->offset;
	mp->size = ofile.object_size;
	mp->type = MP_EMPTY_SPACE;
	fp->mp = mp;

	mp = new_mach_o_part();
	mp->offset = fp->offset;
	if(ofile.mh != NULL){
	    fp->mh = ofile.mh;
	    fp->mh64 = NULL;
	    mp->size = sizeof(struct mach_header) + ofile.mh->sizeofcmds;
	    ncmds = ofile.mh->ncmds;
	    dylib_stub = ofile.mh->filetype == MH_DYLIB_STUB;
	}
	else{
	    fp->mh64 = ofile.mh64;
	    fp->mh = NULL;
	    mp->size = sizeof(struct mach_header_64) + ofile.mh64->sizeofcmds;
	    ncmds = ofile.mh64->ncmds;
	    dylib_stub = ofile.mh64->filetype == MH_DYLIB_STUB;
	}
	mp->type = MP_MACH_HEADERS;
	insert_mach_o_part(fp, mp);


	st = NULL;
	dyst = NULL;
	dyld_info = NULL;
	hints = NULL;
	symbols = NULL;
	symbols64 = NULL;
	strings = NULL;
	split_info = NULL;
	code_sig = NULL;
	func_starts = NULL;
	data_in_code = NULL;
	code_sign_drs = NULL;
	link_opt_hint = NULL;
	dyld_chained_fixups = NULL;
	dyld_exports_trie = NULL;

	lc = ofile.load_commands;
	for(i = 0; i < ncmds; i++){
	    if(st == NULL && lc->cmd == LC_SYMTAB){
		st = (struct symtab_command *)lc;
	    }
	    else if(dyst == NULL && lc->cmd == LC_DYSYMTAB){
		dyst = (struct dysymtab_command *)lc;
	    }
	    else if(dyld_info == NULL && lc->cmd == LC_DYLD_INFO_ONLY){
		dyld_info = (struct dyld_info_command *)lc;
	    }
	    else if(hints == NULL && lc->cmd == LC_TWOLEVEL_HINTS){
		hints = (struct twolevel_hints_command *)lc;
	    }
	    else if(split_info == NULL && lc->cmd == LC_SEGMENT_SPLIT_INFO){
		split_info = (struct linkedit_data_command *)lc;
	    }
	    else if(code_sig == NULL && lc->cmd == LC_CODE_SIGNATURE){
		code_sig = (struct linkedit_data_command *)lc;
	    }
	    else if(func_starts == NULL && lc->cmd == LC_FUNCTION_STARTS){
		func_starts = (struct linkedit_data_command *)lc;
	    }
	    else if(data_in_code == NULL && lc->cmd == LC_DATA_IN_CODE){
		data_in_code = (struct linkedit_data_command *)lc;
	    }
	    else if(code_sign_drs == NULL && lc->cmd == LC_DYLIB_CODE_SIGN_DRS){
		code_sign_drs = (struct linkedit_data_command *)lc;
	    }
	    else if(link_opt_hint == NULL &&
		    lc->cmd == LC_LINKER_OPTIMIZATION_HINT){
		link_opt_hint = (struct linkedit_data_command *)lc;
	    }
	    else if (dyld_chained_fixups == NULL &&
		     lc->cmd == LC_DYLD_CHAINED_FIXUPS) {
		dyld_chained_fixups = (struct linkedit_data_command*)lc;
	    }
	    else if (dyld_exports_trie == NULL &&
		     lc->cmd == LC_DYLD_EXPORTS_TRIE) {
		dyld_exports_trie = (struct linkedit_data_command*)lc;
	    }
	    else if(lc->cmd == LC_SEGMENT && dylib_stub == FALSE){
		sg = (struct segment_command *)lc;
		s = (struct section *)
		      ((char *)sg + sizeof(struct segment_command));
		for(j = 0; j < sg->nsects; j++){
		    if(s->nreloc != 0){
			mp = new_mach_o_part();
			mp->offset = fp->offset + s->reloff;
			mp->size = s->nreloc * sizeof(struct relocation_info);
			mp->type = MP_RELOCS;
			mp->s = s;
			mp->s64 = NULL;
			insert_mach_o_part(fp, mp);
		    }
		    if((s->flags & SECTION_TYPE) != S_ZEROFILL &&
		       (s->flags & SECTION_TYPE) != S_THREAD_LOCAL_ZEROFILL &&
		       s->size != 0){
			mp = new_mach_o_part();
			mp->offset = fp->offset + s->offset;
			mp->size = s->size;
			mp->type = MP_SECTION;
			mp->s = s;
			mp->s64 = NULL;
			insert_mach_o_part(fp, mp);
		    }
		    if(((s->flags & SECTION_TYPE) == S_ZEROFILL ||
			(s->flags & SECTION_TYPE) == S_THREAD_LOCAL_ZEROFILL) &&
		       s->size != 0){
			if(s->addr - sg->vmaddr < sg->filesize){
			    mp = new_mach_o_part();
			    mp->offset = fp->offset + sg->fileoff +
					 s->addr - sg->vmaddr;
			    if(s->addr - sg->vmaddr + s->size <= sg->filesize)
				mp->size = s->size;
			    else
				mp->size = sg->filesize -(s->addr - sg->vmaddr);
			    mp->type = MP_SECTION;
			    mp->s = s;
			    mp->s64 = NULL;
			    insert_mach_o_part(fp, mp);
			}
		    }
		    s++;
		}
	    }
	    else if(lc->cmd == LC_SEGMENT_64 && dylib_stub == FALSE){
		sg64 = (struct segment_command_64 *)lc;
		s64 = (struct section_64 *)
		      ((char *)sg64 + sizeof(struct segment_command_64));
		for(j = 0; j < sg64->nsects; j++){
		    if(s64->nreloc != 0){
			mp = new_mach_o_part();
			mp->offset = fp->offset + s64->reloff;
			mp->size = s64->nreloc * sizeof(struct relocation_info);
			mp->type = MP_RELOCS_64;
			mp->s64 = s64;
			mp->s = NULL;
			insert_mach_o_part(fp, mp);
		    }
		    if((s64->flags & SECTION_TYPE) != S_ZEROFILL &&
		       (s64->flags & SECTION_TYPE) != S_THREAD_LOCAL_ZEROFILL &&
		       s64->size != 0){
			mp = new_mach_o_part();
			mp->offset = fp->offset + s64->offset;
			mp->size = s64->size;
			mp->type = MP_SECTION_64;
			mp->s64 = s64;
			mp->s = NULL;
			insert_mach_o_part(fp, mp);
		    }
		    if(((s64->flags & SECTION_TYPE) == S_ZEROFILL ||
			(s64->flags & SECTION_TYPE) ==
						    S_THREAD_LOCAL_ZEROFILL) && 
		       s64->size != 0){
			if(s64->addr - sg64->vmaddr < sg64->filesize){
			    mp = new_mach_o_part();
			    mp->offset = fp->offset + sg64->fileoff +
					 s64->addr - sg64->vmaddr;
			    if(s64->addr - sg64->vmaddr + s64->size <=
			       sg64->filesize)
				mp->size = s64->size;
			    else
				mp->size = sg64->filesize -
					   (s64->addr - sg64->vmaddr);
			    mp->type = MP_SECTION_64;
			    mp->s64 = s64;
			    mp->s = NULL;
			    insert_mach_o_part(fp, mp);
			}
		    }
		    s64++;
		}
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	if(st != NULL){
	    if(ofile.mh != NULL){
		allocated_symbols = NULL;
		symbols = (struct nlist *)(ofile.object_addr + st->symoff);
		if(ofile.object_byte_sex != get_host_byte_sex()){
		    allocated_symbols = allocate(sizeof(struct nlist) *
						 st->nsyms);
		    memcpy(allocated_symbols, symbols,
			   sizeof(struct nlist) * st->nsyms);
		    swap_nlist(allocated_symbols, st->nsyms,
			       get_host_byte_sex());
		    symbols = allocated_symbols;
		}
		fp->symbols = symbols;
		fp->symbols64 = NULL;
	    }
	    else{
		allocated_symbols64 = NULL;
		symbols64 = (struct nlist_64 *)(ofile.object_addr + st->symoff);
		if(ofile.object_byte_sex != get_host_byte_sex()){
		    allocated_symbols64 = allocate(sizeof(struct nlist_64) *
						   st->nsyms);
		    memcpy(allocated_symbols64, symbols64,
			   sizeof(struct nlist_64) * st->nsyms);
		    swap_nlist_64(allocated_symbols64, st->nsyms,
			       get_host_byte_sex());
		    symbols64 = allocated_symbols64;
		}
		fp->symbols64 = symbols64;
		fp->symbols = NULL;
	    }
	    strings = ofile.object_addr + st->stroff;
	    fp->st = st;
	    fp->strings = strings;
	}

	if(dyst != NULL && st != NULL){
	    if(dyst->nlocalsym != 0){
		mp = new_mach_o_part();
		if(ofile.mh != NULL){
		    mp->offset = fp->offset + st->symoff +
				 dyst->ilocalsym * sizeof(struct nlist);
		    mp->size = dyst->nlocalsym * sizeof(struct nlist);
		}
		else{
		    mp->offset = fp->offset + st->symoff +
				 dyst->ilocalsym * sizeof(struct nlist_64);
		    mp->size = dyst->nlocalsym * sizeof(struct nlist_64);
		}
		mp->type = MP_LOCAL_SYMBOLS;
		insert_mach_o_part(fp, mp);
	    }
	    if(dyst->nextdefsym != 0){
		mp = new_mach_o_part();
		if(ofile.mh != NULL){
		    mp->offset = fp->offset + st->symoff +
				 dyst->iextdefsym * sizeof(struct nlist);
		    mp->size = dyst->nextdefsym * sizeof(struct nlist);
		}
		else{
		    mp->offset = fp->offset + st->symoff +
				 dyst->iextdefsym * sizeof(struct nlist_64);
		    mp->size = dyst->nextdefsym * sizeof(struct nlist_64);
		}
		mp->type = MP_EXTDEF_SYMBOLS;
		insert_mach_o_part(fp, mp);
	    }
	    if(dyst->nundefsym != 0){
		mp = new_mach_o_part();
		if(ofile.mh != NULL){
		    mp->offset = fp->offset + st->symoff +
				 dyst->iundefsym * sizeof(struct nlist);
		    mp->size = dyst->nundefsym * sizeof(struct nlist);
		}
		else{
		    mp->offset = fp->offset + st->symoff +
				 dyst->iundefsym * sizeof(struct nlist_64);
		    mp->size = dyst->nundefsym * sizeof(struct nlist_64);
		}
		mp->type = MP_UNDEF_SYMBOLS;
		insert_mach_o_part(fp, mp);
	    }
	    if(hints != NULL && hints->nhints != 0){
		mp = new_mach_o_part();
		mp->offset = fp->offset + hints->offset;
		mp->size = hints->nhints *
			   sizeof(struct twolevel_hint);
		mp->type = MP_HINTS_TABLE;
		insert_mach_o_part(fp, mp);
	    }
	    if(split_info != NULL && split_info->datasize != 0){
		mp = new_mach_o_part();
		mp->offset = fp->offset + split_info->dataoff;
		mp->size = split_info->datasize;
		mp->type = MP_SPLIT_INFO;
		insert_mach_o_part(fp, mp);
	    }
	    if(dyst->ntoc != 0){
		mp = new_mach_o_part();
		mp->offset = fp->offset + dyst->tocoff;
		mp->size = dyst->ntoc * sizeof(struct dylib_table_of_contents);
		mp->type = MP_TOC;
		insert_mach_o_part(fp, mp);
	    }
	    if(dyst->nmodtab != 0){
		mp = new_mach_o_part();
		mp->offset = fp->offset + dyst->modtaboff;
		if(ofile.mh != NULL)
		    mp->size = dyst->nmodtab * sizeof(struct dylib_module);
		else
		    mp->size = dyst->nmodtab * sizeof(struct dylib_module_64);
		mp->type = MP_MODULE_TABLE;
		insert_mach_o_part(fp, mp);
	    }
	    if(dyst->nextrefsyms != 0){
		mp = new_mach_o_part();
		mp->offset = fp->offset + dyst->extrefsymoff;
		mp->size = dyst->nextrefsyms * sizeof(struct dylib_reference);
		mp->type = MP_REFERENCE_TABLE;
		insert_mach_o_part(fp, mp);
	    }
	    if(dyst->nindirectsyms != 0){
		mp = new_mach_o_part();
		mp->offset = fp->offset + dyst->indirectsymoff;
		mp->size = dyst->nindirectsyms * sizeof(uint32_t);
		mp->type = MP_INDIRECT_SYMBOL_TABLE;
		insert_mach_o_part(fp, mp);
	    }
	    if(dyst->nextrel != 0){
		mp = new_mach_o_part();
		mp->offset = fp->offset + dyst->extreloff;
		mp->size = dyst->nextrel * sizeof(struct relocation_info);
		mp->type = MP_EXT_RELOCS;
		insert_mach_o_part(fp, mp);
	    }
	    if(dyst->nlocrel != 0){
		mp = new_mach_o_part();
		mp->offset = fp->offset + dyst->locreloff;
		mp->size = dyst->nlocrel * sizeof(struct relocation_info);
		mp->type = MP_LOC_RELOCS;
		insert_mach_o_part(fp, mp);
	    }
	    if(dyst->nlocalsym != 0 &&
	       (dyst->nextdefsym != 0 || dyst->nundefsym != 0)){
		/*
		 * Try to break the string table into the local and external
		 * parts knowing the tools should put all the external strings
		 * first and all the local string last.
		 */
		ext_low = st->strsize;
		local_low = st->strsize;
		ext_high = 0;
		local_high = 0;
		for(i = 0; i < st->nsyms; i++){
		    if(ofile.mh != NULL){
			n_strx = symbols[i].n_un.n_strx;
			n_type = symbols[i].n_type;
		        filetype = fp->mh->filetype;
		    }
		    else{
			n_strx = symbols64[i].n_un.n_strx;
			n_type = symbols64[i].n_type;
		        filetype = fp->mh64->filetype;
		    }
		    if(n_strx == 0)
			continue;
		    /*
		     * If this object has any bad string indexes don't try to
		     * break the string table into the local and external
		     * parts.
		     */
		    if(n_strx > st->strsize){
			ext_low = st->strsize;
			local_low = st->strsize;
			ext_high = 0;
			local_high = 0;
			break;
		    }
		    if(n_type & N_EXT ||
		       (filetype == MH_EXECUTE && n_type & N_PEXT)){
			if(n_strx > ext_high)
			    ext_high = n_strx;
			if(n_strx < ext_low)
			    ext_low = n_strx;
		    }
		    else{
			if(n_strx > local_high)
			    local_high = n_strx;
			if(n_strx < local_low)
			    local_low = n_strx;
		    }
		}

		if(ofile.mh != NULL){
		    modtab = (struct dylib_module *)(ofile.object_addr +
						     dyst->modtaboff);
		    if(ofile.object_byte_sex != get_host_byte_sex())
			swap_dylib_module(modtab, dyst->nmodtab,
			       get_host_byte_sex());
		    for(i = 0; i < dyst->nmodtab; i++){
			if(modtab[i].module_name > local_high)
			    local_high = modtab[i].module_name;
			if(modtab[i].module_name < local_low)
			    local_low = modtab[i].module_name;
		    }
		}
		else{
		    modtab64 = (struct dylib_module_64 *)(ofile.object_addr +
						          dyst->modtaboff);
		    if(ofile.object_byte_sex != get_host_byte_sex())
			swap_dylib_module_64(modtab64, dyst->nmodtab,
			       get_host_byte_sex());
		    for(i = 0; i < dyst->nmodtab; i++){
			if(modtab64[i].module_name > local_high)
			    local_high = modtab64[i].module_name;
			if(modtab64[i].module_name < local_low)
			    local_low = modtab64[i].module_name;
		    }
		}

		if(ext_high < local_low && local_low < local_high){
		    mp = new_mach_o_part();
		    mp->offset = fp->offset + st->stroff + ext_low;
		    mp->size = ext_high - ext_low +
			       strlen(strings + ext_high) + 1;
		    mp->type = MP_EXT_STRING_TABLE;
		    insert_mach_o_part(fp, mp);

		    mp = new_mach_o_part();
		    mp->offset = fp->offset + st->stroff + local_low;
		    mp->size = local_high - local_low +
			       strlen(strings + local_high) + 1;
		    mp->type = MP_LOC_STRING_TABLE;
		    insert_mach_o_part(fp, mp);
		}
		else{
		    mp = new_mach_o_part();
		    mp->offset = fp->offset + st->stroff;
		    mp->size = st->strsize;
		    mp->type = MP_STRING_TABLE;
		    insert_mach_o_part(fp, mp);
		}
	    }
	    else if(dyst->nextdefsym != 0 || dyst->nundefsym != 0){
		if(st->strsize != 0){
		    mp = new_mach_o_part();
		    mp->offset = fp->offset + st->stroff;
		    mp->size = st->strsize;
		    mp->type = MP_EXT_STRING_TABLE;
		    insert_mach_o_part(fp, mp);
		}
	    }
	    else{
		if(st->strsize != 0){
		    mp = new_mach_o_part();
		    mp->offset = fp->offset + st->stroff;
		    mp->size = st->strsize;
		    mp->type = MP_LOC_STRING_TABLE;
		    insert_mach_o_part(fp, mp);
		}
	    }
	}
	else if(st != NULL){
	    if(st->nsyms != 0){
		mp = new_mach_o_part();
		mp->offset = fp->offset + st->symoff;
		if(ofile.mh != NULL)
		    mp->size = st->nsyms * sizeof(struct nlist);
		else
		    mp->size = st->nsyms * sizeof(struct nlist_64);
		mp->type = MP_SYMBOL_TABLE;
		insert_mach_o_part(fp, mp);
	    }
	    if(st->strsize != 0){
		mp = new_mach_o_part();
		mp->offset = fp->offset + st->stroff;
		mp->size = st->strsize;
		mp->type = MP_STRING_TABLE;
		insert_mach_o_part(fp, mp);
	    }
	}
	if(dyld_info != NULL){
	    if(dyld_info->rebase_size != 0){
		mp = new_mach_o_part();
		mp->offset = fp->offset + dyld_info->rebase_off;
		mp->size = dyld_info->rebase_size;
		mp->type = MP_DYLD_INFO_REBASE;
		insert_mach_o_part(fp, mp);
	    }
	    if(dyld_info->bind_size != 0){
		mp = new_mach_o_part();
		mp->offset = fp->offset + dyld_info->bind_off;
		mp->size = dyld_info->bind_size;
		mp->type = MP_DYLD_INFO_BIND;
		insert_mach_o_part(fp, mp);
	    }
	    if(dyld_info->weak_bind_size != 0){
		mp = new_mach_o_part();
		mp->offset = fp->offset + dyld_info->weak_bind_off;
		mp->size = dyld_info->weak_bind_size;
		mp->type = MP_DYLD_INFO_WEAK_BIND;
		insert_mach_o_part(fp, mp);
	    }
	    if(dyld_info->lazy_bind_size != 0){
		mp = new_mach_o_part();
		mp->offset = fp->offset + dyld_info->lazy_bind_off;
		mp->size = dyld_info->lazy_bind_size;
		mp->type = MP_DYLD_INFO_LAZY_BIND;
		insert_mach_o_part(fp, mp);
	    }
	    if(dyld_info->export_size != 0){
		mp = new_mach_o_part();
		mp->offset = fp->offset + dyld_info->export_off;
		mp->size = dyld_info->export_size;
		mp->type = MP_DYLD_INFO_EXPORT;
		insert_mach_o_part(fp, mp);
	    }
	}
	if(code_sig != NULL && code_sig->datasize != 0){
	    mp = new_mach_o_part();
	    mp->offset = fp->offset + code_sig->dataoff;
	    mp->size = code_sig->datasize;
	    mp->type = MP_CODE_SIG;
	    insert_mach_o_part(fp, mp);
	}
	if(func_starts != NULL && func_starts->datasize != 0){
	    mp = new_mach_o_part();
	    mp->offset = fp->offset + func_starts->dataoff;
	    mp->size = func_starts->datasize;
	    mp->type = MP_FUNCTION_STARTS;
	    insert_mach_o_part(fp, mp);
	}
	if(data_in_code != NULL && data_in_code->datasize != 0){
	    mp = new_mach_o_part();
	    mp->offset = fp->offset + data_in_code->dataoff;
	    mp->size = data_in_code->datasize;
	    mp->type = MP_DATA_IN_CODE;
	    insert_mach_o_part(fp, mp);
	}
	if(code_sign_drs != NULL && code_sign_drs->datasize != 0){
	    mp = new_mach_o_part();
	    mp->offset = fp->offset + code_sign_drs->dataoff;
	    mp->size = code_sign_drs->datasize;
	    mp->type = MP_CODE_SIGN_DRS;
	    insert_mach_o_part(fp, mp);
	}
	if(link_opt_hint != NULL && link_opt_hint->datasize != 0){
	    mp = new_mach_o_part();
	    mp->offset = fp->offset + link_opt_hint->dataoff;
	    mp->size = link_opt_hint->datasize;
	    mp->type = MP_LINK_OPT_HINT;
	    insert_mach_o_part(fp, mp);
	}
	if(dyld_chained_fixups != NULL && dyld_chained_fixups->datasize != 0){
	    mp = new_mach_o_part();
	    mp->offset = fp->offset + dyld_chained_fixups->dataoff;
	    mp->size = dyld_chained_fixups->datasize;
	    mp->type = MP_DYLD_CHAINED_FIXUPS;
	    insert_mach_o_part(fp, mp);
	}
	if(dyld_exports_trie != NULL && dyld_exports_trie->datasize != 0){
	    mp = new_mach_o_part();
	    mp->offset = fp->offset + dyld_exports_trie->dataoff;
	    mp->size = dyld_exports_trie->datasize;
	    mp->type = MP_DYLD_EXPORTS_TRIE;
	    insert_mach_o_part(fp, mp);
	}
}

static
struct mach_o_part *
new_mach_o_part(
void)
{
    struct mach_o_part *mp;

	mp = allocate(sizeof(struct mach_o_part));
	memset(mp, '\0', sizeof(struct mach_o_part));
	return(mp);
}

static
void
insert_mach_o_part(
struct file_part *fp,
struct mach_o_part *new)
{
    struct mach_o_part *p, *q;

	for(p = fp->mp; p != NULL; p = p->next){
	    /* find an existing part that contains the new part */
	    if(new->offset >= p->offset &&
	       new->offset + new->size <= p->offset + p->size){
		if(p->type != MP_EMPTY_SPACE)
		    fatal("internal error: new mach_o part not contained in "
			"empty space");
		/* if new is the same as p replace p with new */
		if(new->offset == p->offset &&
	           new->size == p->size){
		    new->prev = p->prev;
		    new->next = p->next;
		    if(p->next != NULL)
			p->next->prev = new;
		    if(p->prev != NULL)
			p->prev->next = new;
		    if(fp->mp == p)
			fp->mp = new;
		    free(p);
		    return;
		}
		/* if new starts at p put new before p and move p's offset */
		if(new->offset == p->offset){
		    p->offset = new->offset + new->size;
		    p->size = p->size - new->size;
		    new->prev = p->prev;
		    if(p->prev != NULL)
			p->prev->next = new;
		    p->prev = new;
		    new->next = p;
		    if(fp->mp == p)
			fp->mp = new;
		    return;
		}
		/* if new ends at p put new after p and change p's size */
		if(new->offset + new->size == p->offset + p->size){
		    p->size = new->offset - p->offset;
		    new->next = p->next;
		    if(p->next != NULL)
			p->next->prev = new;
		    p->next = new;
		    new->prev = p;
		    return;
		}
		/* new is in the middle of p */
		q = new_mach_o_part();
		q->type = MP_EMPTY_SPACE;
		q->offset = new->offset + new->size;
		q->size = p->offset + p->size - (new->offset + new->size);
		p->size = new->offset - p->offset;
		new->prev = p;
		new->next = q;
		q->prev = new;
		q->next = p->next;
		if(p->next != NULL)
		    p->next->prev = q;
		p->next = new;
		return;
	    }
	}
	fatal("internal error: new mach_o part not found in existing part");
}

static
void
print_mach_o_parts(
struct mach_o_part *mp)
{
    struct mach_o_part *p, *prev;
    uint64_t offset;
    char *indent;

	if(arch_flag == NULL)
	    indent = "    ";
	else
	    indent = "";
	offset = 0;
	prev = NULL;
	if(mp != NULL)
	    offset = mp->offset;
	for(p = mp; p != NULL; p = p->next){
	    if(p->type == MP_SECTION)
		printf("%sMP_SECTION (%.16s,%.16s)\n", indent,
		p->s->segname, p->s->sectname);
	    else if(p->type == MP_SECTION_64)
		printf("%sMP_SECTION_64 (%.16s,%.16s)\n", indent,
		p->s64->segname, p->s64->sectname);
	    else
		printf("%s%s\n", indent, mach_o_part_type_names[p->type]);
	    printf("%s    offset = %llu\n", indent, p->offset - arch_offset);
	    printf("%s    size = %llu\n", indent, p->size);
	    if(prev != NULL)
		if(prev != p->prev)
		    printf("bad prev pointer\n");
	    prev = p;
	    if(offset != p->offset)
		    printf("bad offset\n");
	    offset += p->size;
	}
}

static
void
print_parts_for_page(
uint64_t page_number)
{
    uint64_t offset, size, low_addr, high_addr, new_low_addr, new_high_addr;
    struct file_part *fp;
    struct mach_o_part *mp;
    enum bool printed;
    enum bool sections, sections64;
    const char *arch_name;

	offset = page_number * vm_page_size;
	size = vm_page_size;
	low_addr = 0;
	high_addr = 0;

	if(arch_flag == NULL){
	    if(offset > ofile.file_size){
		printf("File has no page %llu (file has only %u pages)\n",
		       page_number, (uint32_t)((ofile.file_size +
					        vm_page_size -1) /
					      vm_page_size));
	        return;
	    }
	}
	else{
	    if(offset > arch_size){
		printf("File for architecture %s has no page %llu (has only %u "
		       "pages)\n", arch_flag->name,
		       page_number, (uint32_t)((arch_size +
					        vm_page_size -1) /
					      vm_page_size));
	        return;
	    }
	}

	/*
	 * First pass through the file printing whats on the page ignoring the
	 * empty spaces.
	 */
	printed = FALSE;
	for(fp = file_parts; fp != NULL; fp = fp->next){
	    if(offset + size <= fp->offset - arch_offset)
		continue;
	    if(offset > fp->offset - arch_offset + fp->size)
		continue;
	    switch(fp->type){
	    case FP_FAT_HEADERS:
		printf("File Page %llu contains fat file headers\n",
		       page_number);
		printed = TRUE;
		break;
	    case FP_MACH_O:
		sections = FALSE;
		sections64 = FALSE;
		for(mp = fp->mp; mp != NULL; mp = mp->next){
		    if(offset + size <= mp->offset - arch_offset)
			continue;
		    if(offset > mp->offset - arch_offset + mp->size)
			continue;
		    switch(mp->type){
		    case MP_MACH_HEADERS:
			printf("File Page %llu contains Mach-O headers",
			       page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_SECTION:
			printf("File Page %llu contains contents of "
			       "section (%.16s,%.16s)", page_number,
			       mp->s->segname, mp->s->sectname);
			print_arch(fp);
			printed = TRUE;
			if(offset < mp->offset - arch_offset)
			    new_low_addr = mp->s->addr;
			else
			    new_low_addr = mp->s->addr + offset - mp->offset - 
					   arch_offset;
			if(offset + size > mp->offset - arch_offset + mp->size)
			    new_high_addr = mp->s->addr + mp->s->size;
			else
			    new_high_addr = mp->s->addr +
				(offset + size - (mp->offset - arch_offset));
			if(sections == FALSE){
			    low_addr = new_low_addr;
			    high_addr = new_high_addr;
			}
			else{
			    if(new_low_addr < low_addr)
				low_addr = new_low_addr;
			    if(new_high_addr > high_addr)
				high_addr = new_high_addr;
		        }
			sections = TRUE;
			break;
		    case MP_SECTION_64:
			printf("File Page %llu contains contents of "
			       "section (%.16s,%.16s)", page_number,
			       mp->s64->segname, mp->s64->sectname);
			print_arch(fp);
			printed = TRUE;
			if(offset < mp->offset - arch_offset)
			    new_low_addr = mp->s64->addr;
			else
			    new_low_addr = mp->s64->addr + offset -
					   (mp->offset - arch_offset);
			if(offset + size > (mp->offset - arch_offset) +
					   mp->size)
			    new_high_addr = mp->s64->addr + mp->s64->size;
			else
			    new_high_addr = mp->s64->addr +
				(offset + size - (mp->offset - arch_offset));
			if(sections64 == FALSE){
			    low_addr = new_low_addr;
			    high_addr = new_high_addr;
			}
			else{
			    if(new_low_addr < low_addr)
				low_addr = new_low_addr;
			    if(new_high_addr > high_addr)
				high_addr = new_high_addr;
		        }
			sections64 = TRUE;
			break;
		    case MP_RELOCS:
			printf("File Page %llu contains relocation entries for "
			       "section (%.16s,%.16s)", page_number,
			       mp->s->segname, mp->s->sectname);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_RELOCS_64:
			printf("File Page %llu contains relocation entries for "
			       "section (%.16s,%.16s)", page_number,
			       mp->s64->segname, mp->s64->sectname);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_SPLIT_INFO:
			printf("File Page %llu contains local of info to split "
			       "segments", page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_LOCAL_SYMBOLS:
			printf("File Page %llu contains symbol table for "
			       "non-global symbols", page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_EXTDEF_SYMBOLS:
			printf("File Page %llu contains symbol table for "
			       "defined global symbols", page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_UNDEF_SYMBOLS:
			printf("File Page %llu contains symbol table for "
			       "undefined symbols", page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_TOC:
			printf("File Page %llu contains table of contents",
			       page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_MODULE_TABLE:
			printf("File Page %llu contains module table",
			       page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_REFERENCE_TABLE:
			printf("File Page %llu contains reference table",
			       page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_INDIRECT_SYMBOL_TABLE:
			printf("File Page %llu contains indirect symbols table",
			       page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_EXT_RELOCS:
			printf("File Page %llu contains external relocation "
			       "entries", page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_LOC_RELOCS:
			printf("File Page %llu contains local relocation "
			       "entries", page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_SYMBOL_TABLE:
			printf("File Page %llu contains symbol table",
			       page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_HINTS_TABLE:
			printf("File Page %llu contains hints table",
			       page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_STRING_TABLE:
			printf("File Page %llu contains string table",
			       page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_EXT_STRING_TABLE:
			printf("File Page %llu contains string table for "
			       "external symbols", page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_LOC_STRING_TABLE:
			printf("File Page %llu contains string table for "
			       "local symbols", page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_CODE_SIG:
			printf("File Page %llu contains data of code signature",
			       page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_FUNCTION_STARTS:
			printf("File Page %llu contains data of function starts",
			       page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_DATA_IN_CODE:
			printf("File Page %llu contains info for data in code",
			       page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_CODE_SIGN_DRS:
			printf("File Page %llu contains info for code signing "
			       "DRs copied from linked dylib", page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_LINK_OPT_HINT:
			printf("File Page %llu contains info for linker "
			       "optimization hints", page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_DYLD_CHAINED_FIXUPS:
			printf("File Page %llu contains dyld info for chained "
			       "fixups", page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_DYLD_EXPORTS_TRIE:
			printf("File Page %llu contains dyld info for exported "
			       "symbols (TRIE)", page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_DYLD_INFO_REBASE:
			printf("File Page %llu contains dyld info for sliding "
			       "an image", page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_DYLD_INFO_BIND:
			printf("File Page %llu contains dyld info for binding "
			       "symbols", page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_DYLD_INFO_WEAK_BIND:
			printf("File Page %llu contains dyld info for weak bound "
			       "symbols", page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_DYLD_INFO_LAZY_BIND:
			printf("File Page %llu contains dyld info for lazy bound "
			       "symbols", page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_DYLD_INFO_EXPORT:
			printf("File Page %llu contains dyld info for symbols "
			       "exported by a dylib", page_number);
			print_arch(fp);
			printed = TRUE;
			break;
		    case MP_EMPTY_SPACE:
			break;
		    }
		}
		if(sections == TRUE || sections64 == TRUE){
		    printf("Symbols on file page %llu virtual address 0x%llx to "
			   "0x%llx\n", page_number, low_addr, high_addr);
		    if(sections == TRUE)
			print_symbols(fp, low_addr, high_addr);
		    else
			print_symbols64(fp, low_addr, high_addr);
		}
		break;

	    case FP_EMPTY_SPACE:
		break;
	    }
	}

	/*
	 * Make a second pass through the file if nothing got printing on the
	 * first pass because the page covers empty space in the file.
	 */
	if(printed == TRUE)
	    return;
	for(fp = file_parts; fp != NULL; fp = fp->next){
	    if(offset + size <= fp->offset)
		continue;
	    if(offset > fp->offset + fp->size)
		continue;
	    if(fp->type == FP_MACH_O){
		for(mp = fp->mp; mp != NULL; mp = mp->next){
		    if(offset + size <= mp->offset - arch_offset)
			continue;
		    if(offset > (mp->offset - arch_offset) + mp->size)
			continue;
		    if(fp->mh != NULL)
			arch_name = get_arch_name_from_types(fp->mh->cputype,
						    	fp->mh->cpusubtype);
		    else
			arch_name = get_arch_name_from_types(fp->mh64->cputype,
						    	fp->mh64->cpusubtype);
		    printf("File Page %llu contains empty space in the Mach-O "
			   "file for %s between:\n", page_number, arch_name);
		    if(mp->prev == NULL)
			printf("    the start of the Mach-O file");
		    else{
			printf("    ");
			print_mach_o_part(mp->prev);
		    }
		    printf(" and\n");
		    if(mp->next == NULL)
			printf("    the end of the Mach-O file\n");
		    else{
			printf("    ");
			print_mach_o_part(mp->next);
		    }
		    printf("\n");
		    return;
		}
		break;
	    }
	    printf("File Page %llu contains empty space in the file between:\n",
		   page_number);
	    if(fp->prev == NULL)
		printf("    the start of the file");
	    else{
		printf("    ");
		print_file_part(fp->prev);
	    }
	    printf(" and\n");
	    if(fp->next == NULL)
		printf("    the end of the file\n");
	    else{
		printf("    ");
		print_file_part(fp->next);
	    }
	    printf("\n");
	    return;
	}
}

static
void
print_arch(
struct file_part *fp)
{
	if(ofile.file_type == OFILE_FAT){
	    if(fp->mh != NULL)
		printf(" (%s)\n",
		   get_arch_name_from_types(fp->mh->cputype,
					    fp->mh->cpusubtype));
	    else
		printf(" (%s)\n",
		   get_arch_name_from_types(fp->mh64->cputype,
					    fp->mh64->cpusubtype));
	}
	else
	    printf("\n");
}

static
void
print_file_part(
struct file_part *fp)
{
    const char *arch_name;

	switch(fp->type){
	case FP_FAT_HEADERS:
	    printf("fat file headers");
	    break;
	case FP_MACH_O:
	    if(fp->mh != NULL)
		arch_name = get_arch_name_from_types(fp->mh->cputype,
						fp->mh->cpusubtype);
	    else
		arch_name = get_arch_name_from_types(fp->mh64->cputype,
						fp->mh64->cpusubtype);
	    printf("Mach-O file for %s", arch_name);
	    break;
	case FP_EMPTY_SPACE:
	    printf("empty space");
	    break;
	}
}

static
void
print_mach_o_part(
struct mach_o_part *mp)
{
	switch(mp->type){
	case MP_MACH_HEADERS:
	    printf("Mach-O headers");
	    break;
	case MP_SECTION:
	    printf("contents of section (%.16s,%.16s)",
		   mp->s->segname, mp->s->sectname);
	    break;
	case MP_SECTION_64:
	    printf("contents of section (%.16s,%.16s)",
		   mp->s64->segname, mp->s64->sectname);
	    break;
	case MP_RELOCS:
	    printf("relocation entries for section (%.16s,%.16s)",
		   mp->s->segname, mp->s->sectname);
	    break;
	case MP_RELOCS_64:
	    printf("relocation entries for section (%.16s,%.16s)",
		   mp->s64->segname, mp->s64->sectname);
	    break;
	case MP_SPLIT_INFO:
	    printf("local of info to split segments");
	    break;
	case MP_LOCAL_SYMBOLS:
	    printf("symbol table for non-global symbols");
	    break;
	case MP_EXTDEF_SYMBOLS:
	    printf("symbol table for defined global symbols");
	    break;
	case MP_UNDEF_SYMBOLS:
	    printf("symbol table for undefined symbols");
	    break;
	case MP_TOC:
	    printf("table of contents");
	    break;
	case MP_MODULE_TABLE:
	    printf("module table");
	    break;
	case MP_REFERENCE_TABLE:
	    printf("reference table");
	    break;
	case MP_INDIRECT_SYMBOL_TABLE:
	    printf("indirect symbol table");
	    break;
	case MP_EXT_RELOCS:
	    printf("external relocation entries");
	    break;
	case MP_LOC_RELOCS:
	    printf("local relocation entries");
	    break;
	case MP_SYMBOL_TABLE:
	    printf("symbol table");
	    break;
	case MP_HINTS_TABLE:
	    printf("hints table");
	    break;
	case MP_STRING_TABLE:
	    printf("string table");
	    break;
	case MP_EXT_STRING_TABLE:
	    printf("string table for external symbols");
	    break;
	case MP_LOC_STRING_TABLE:
	    printf("string table for local symbols");
	    break;
	case MP_CODE_SIG:
	    printf("data of code signature");
	    break;
	case MP_FUNCTION_STARTS:
	    printf("data of function starts");
	    break;
	case MP_DATA_IN_CODE:
	    printf("info for data in code");
	    break;
	case MP_CODE_SIGN_DRS:
	    printf("info for code signing DRs copied from linked dylibs");
	    break;
	case MP_LINK_OPT_HINT:
	    printf("info for linker optimization hints");
	    break;
	case MP_DYLD_CHAINED_FIXUPS:
	    printf("dyld info for chained fixups");
	    break;
	case MP_DYLD_EXPORTS_TRIE:
	    printf("dyld info for exported symbols (TRIE)");
	    break;
	case MP_DYLD_INFO_REBASE:
	    printf("dyld info for sliding an image");
	    break;
	case MP_DYLD_INFO_BIND:
	    printf("dyld info for binding symbols");
	    break;
	case MP_DYLD_INFO_WEAK_BIND:
	    printf("dyld info for binding weak symbols");
	    break;
	case MP_DYLD_INFO_LAZY_BIND:
	    printf("dyld info for binding lazy symbols");
	    break;
	case MP_DYLD_INFO_EXPORT:
	    printf("dyld info for exported symbols");
	    break;
	case MP_EMPTY_SPACE:
	    printf("empty space");
	    break;
	}
}

static
void
print_symbols(
struct file_part *fp,
uint64_t low_addr,
uint64_t high_addr)
{
    uint32_t i, count;

	if(fp->st == NULL)
	    return;

	if(sorted_symbols == NULL)
	    sorted_symbols = allocate(sizeof(struct nlist) * fp->st->nsyms);

	count = 0;
	for(i = 0; i < fp->st->nsyms; i++){
	    if((fp->symbols[i].n_type & N_STAB) != 0)
		continue;
	    if(fp->symbols[i].n_value >= low_addr &&
	       fp->symbols[i].n_value <= high_addr){
		sorted_symbols[count] = fp->symbols[i];
		count++;
	    }
	}

	qsort(sorted_symbols, count, sizeof(struct nlist),
	      (int (*)(const void *, const void *))compare);

	for(i = 0; i < count; i++){
	    printf("  0x%08x %s\n", (unsigned int)sorted_symbols[i].n_value,
		fp->strings + sorted_symbols[i].n_un.n_strx);
	}
}

static
void
print_symbols64(
struct file_part *fp,
uint64_t low_addr,
uint64_t high_addr)
{
    uint32_t i, count;

	if(fp->st == NULL)
	    return;

	if(sorted_symbols64 == NULL)
	    sorted_symbols64 = allocate(sizeof(struct nlist_64) *
					fp->st->nsyms);

	count = 0;
	for(i = 0; i < fp->st->nsyms; i++){
	    if((fp->symbols64[i].n_type & N_STAB) != 0)
		continue;
	    if(fp->symbols64[i].n_value >= low_addr &&
	       fp->symbols64[i].n_value <= high_addr){
		sorted_symbols64[count] = fp->symbols64[i];
		count++;
	    }
	}

	qsort(sorted_symbols64, count, sizeof(struct nlist_64),
	      (int (*)(const void *, const void *))compare64);

	for(i = 0; i < count; i++){
	    printf("  0x%016llx %s\n", sorted_symbols64[i].n_value,
		fp->strings + sorted_symbols64[i].n_un.n_strx);
	}
}

static
int
compare(
struct nlist *p1,
struct nlist *p2)
{
	if(p1->n_value > p2->n_value)
	    return(1);
	else if(p1->n_value < p2->n_value)
	    return(-1);
	else
	    return(0);
}

static
int
compare64(
struct nlist_64 *p1,
struct nlist_64 *p2)
{
	if(p1->n_value > p2->n_value)
	    return(1);
	else if(p1->n_value < p2->n_value)
	    return(-1);
	else
	    return(0);
}
