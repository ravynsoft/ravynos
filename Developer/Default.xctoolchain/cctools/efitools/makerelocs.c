/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include "stuff/bool.h"
#include "stuff/ofile.h"
#include "stuff/errors.h"
#include "stuff/reloc.h"
#include "stuff/write64.h"
#include "coff/base_relocs.h"
#include "coff/bytesex.h"
#include "mach-o/x86_64/reloc.h"

/* used by error routines as the name of this program */
char *progname = NULL;

/* used for debugging this program */
static enum bool verbose = FALSE;

/* the bytesex of our target object file and of this host machine */
static enum byte_sex target_byte_sex;
static enum byte_sex host_byte_sex;

/*
 * This is the internal structure that we gather the base relocation in from
 * the Mach-O relocation entries.
 */
struct base_reloc {
    uint64_t addr;
    uint32_t type;
};
struct base_reloc *base_relocs = NULL;
uint32_t nbase_reloc = 0;

static void process(
    struct ofile *ofile,
    char *arch_name,
    void *cookie);
static void gather_base_reloc_info(
    uint32_t addr,
    struct relocation_info *relocs, 
    uint32_t nreloc,
    cpu_type_t cpu_type,
    uint32_t length,
    int macho_reloc_type,
    int base_reloc_type);
static void add_base_reloc(
    uint64_t addr,
    uint32_t type);
static void output_base_relocs(
    char *out);
static int cmp_base_relocs(
    struct base_reloc *x1,
    struct base_reloc *x2);
static void usage(
    void);

/* apple_version is created by the libstuff/Makefile */
extern char apple_version[];
char *version = apple_version;

/*
 * The makerelocs(1) tool makes a file of PECOFF base relocation entries from a 
 * fully linked Mach-O file compiled with dynamic code gen and relocation
 * entries saved (linked with -r).  The file of PECOFF base relocation entries
 * then are put in a Mach-O section called .reloc and is then used with
 * objcopy(1) to convert that Mach-O file into a PECOFF file. The makerelocs(1)
 * program has the current usage:
 *
 *	makerelocs [-v] input_Mach-O output_relocs
 * 
 * Where the -v flag provides verbose output used to debug the this programs
 * creation of the PECOFF base relocation entries.
 *
 * TODO: This code can be used for the basis to replace the makerelocs(1)
 * program from the efitools project.  Its current state is that it works for
 * building the efiboot project and the Bluetooth.efi hardware diag.
 */
int
main(
int argc,
char **argv,
char **envp)
{
    int i;
    char *input, *output;

	progname = argv[0];
	host_byte_sex = get_host_byte_sex();

	input = NULL;
	output = NULL;

	for(i = 1; i < argc; i++){
	    if(strcmp(argv[i], "-v") == 0)
		verbose = TRUE;
	    else if(input == NULL)
		input = argv[i];
	    else if(output == NULL)
		output = argv[i];
	    else
		usage();
	}
	if(input == NULL){
	    warning("no input file specified");
	    usage();
	}
	if(output == NULL){
	    warning("no output file specified");
	    usage();
	}

	ofile_process(input, NULL, 0, FALSE, FALSE, FALSE, FALSE,
		      process, NULL);
	if(errors != 0)
	    return(EXIT_FAILURE);

	/* create the output file */
	output_base_relocs(output);

	if(errors == 0)
	    return(EXIT_SUCCESS);
	else
	    return(EXIT_FAILURE);
}

/*
 * process() is the routine that gets called by ofile_process() to process the
 * object file and gather the info to create the base relocation entries.
 */
static
void
process(
struct ofile *ofile,
char *arch_name,
void *cookie)
{
    uint32_t ncmds, i, j;
    uint64_t addr, first_addr;
    struct load_command *lc;
    struct segment_command *sg;
    struct segment_command_64 *sg64;
    struct section *s;
    struct section_64 *s64;
    enum bool swapped;
    struct relocation_info *relocs;

    struct symtab_command *st;
    struct dysymtab_command *dyst;
    struct nlist *symbols;
    struct nlist_64 *symbols64;

	st = NULL;
	dyst = NULL;
	swapped = host_byte_sex != ofile->object_byte_sex;
	target_byte_sex = ofile->object_byte_sex;
	if(ofile->mh != NULL)
	    ncmds = ofile->mh->ncmds;
	else
	    ncmds = ofile->mh64->ncmds;

	lc = ofile->load_commands;
	for(i = 0; i < ncmds; i++){
	    if(st == NULL && lc->cmd == LC_SYMTAB){
		st = (struct symtab_command *)lc;
	    }
	    else if(dyst == NULL && lc->cmd == LC_DYSYMTAB){
		dyst = (struct dysymtab_command *)lc;
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	/* TODO this would be need to to do checking for undefined symbols */
	if(ofile->mh != NULL){
	    symbols = (struct nlist *)(ofile->object_addr + st->symoff);
	    if(swapped)
		swap_nlist(symbols, st->nsyms, host_byte_sex);
	    symbols64 = NULL;
	}
	else{
	    symbols = NULL;
	    symbols64 = (struct nlist_64 *)(ofile->object_addr +st->symoff);
	    if(swapped)
		swap_nlist_64(symbols64, st->nsyms, host_byte_sex);
	}

	first_addr = 0;
	lc = ofile->load_commands;
	for(i = 0; i < ncmds; i++){
	    if(lc->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc;
		if(first_addr == 0)
		    first_addr = sg->vmaddr;
		s = (struct section *)
		      ((char *)sg + sizeof(struct segment_command));
		for(j = 0; j < sg->nsects; j++){
		    relocs = (struct relocation_info *)(ofile->object_addr +
						        s[j].reloff);
		    if(swapped)
			swap_relocation_info(relocs, s[j].nreloc,
					     host_byte_sex);
		    if(ofile->mh_cputype == CPU_TYPE_I386)
			gather_base_reloc_info(s[j].addr, relocs, s[j].nreloc,
				CPU_TYPE_I386, 2, GENERIC_RELOC_VANILLA,
				IMAGE_REL_BASED_HIGHLOW);
		    else if(ofile->mh_cputype == CPU_TYPE_ARM)
			gather_base_reloc_info(s[j].addr, relocs, s[j].nreloc,
				CPU_TYPE_ARM, 2, GENERIC_RELOC_VANILLA,
				IMAGE_REL_BASED_HIGHLOW);
		    else
			fatal("unsupported cputype %d", ofile->mh_cputype);
		    if((s[j].flags & SECTION_TYPE) ==
			S_NON_LAZY_SYMBOL_POINTERS){
			for(addr = s[j].addr;
			    addr < s[j].addr + s[j].size;
			    addr += 4) {
			    add_base_reloc(addr, IMAGE_REL_BASED_HIGHLOW);
			}
		    }
		}
	    }
	    else if(lc->cmd == LC_SEGMENT_64){
		sg64 = (struct segment_command_64 *)lc;
		if(first_addr == 0)
		    first_addr = sg64->vmaddr;
		s64 = (struct section_64 *)
		      ((char *)sg64 + sizeof(struct segment_command_64));
		for(j = 0; j < sg64->nsects; j++){
		    relocs = (struct relocation_info *)(ofile->object_addr +
						        s64[j].reloff);
		    if(swapped)
			swap_relocation_info(relocs, s64[j].nreloc,
					     host_byte_sex);
		    if(ofile->mh_cputype == CPU_TYPE_X86_64)
			gather_base_reloc_info(s64[j].addr, relocs,
			    s64[j].nreloc, CPU_TYPE_X86_64, 3,
			    X86_64_RELOC_UNSIGNED, IMAGE_REL_BASED_DIR64);
		    else
			fatal("unsupported cputype %d", ofile->mh_cputype);
		    if((s64[j].flags & SECTION_TYPE) ==
			S_NON_LAZY_SYMBOL_POINTERS){
			for(addr = s64[j].addr;
			    addr < s64[j].addr + s64[j].size;
			    addr += 8) {
			    add_base_reloc(addr, IMAGE_REL_BASED_DIR64);
			}
		    }
		}
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	if(dyst != NULL && dyst->nlocrel != 0){
	    relocs = (struct relocation_info *)(ofile->object_addr +
						dyst->locreloff);
	    if(swapped)
		swap_relocation_info(relocs, dyst->nlocrel, host_byte_sex);
	    if(ofile->mh_cputype == CPU_TYPE_I386)
		gather_base_reloc_info(first_addr, relocs, dyst->nlocrel,
		    CPU_TYPE_I386, 2, GENERIC_RELOC_VANILLA,
		    IMAGE_REL_BASED_HIGHLOW);
	    if(ofile->mh_cputype == CPU_TYPE_ARM)
		gather_base_reloc_info(first_addr, relocs, dyst->nlocrel,
		    CPU_TYPE_ARM, 2, GENERIC_RELOC_VANILLA,
		    IMAGE_REL_BASED_HIGHLOW);
	    else if(ofile->mh_cputype == CPU_TYPE_X86_64)
		gather_base_reloc_info(first_addr, relocs, dyst->nlocrel,
		    CPU_TYPE_X86_64, 3, X86_64_RELOC_UNSIGNED,
		    IMAGE_REL_BASED_DIR64);
	    else
		fatal("unsupported cputype %d", ofile->mh_cputype);
	}
	if(dyst != NULL && dyst->nextrel != 0)
	    fatal("input Mach-O file has external relocation entries");
}

/*
 * gather_base_reloc_info() is passed the base address for the set of Mach-O
 * relocation entries. And is passed the cpu_type, length and macho_reloc_type
 * to look for and the base_reloc_type to create if found.
 */
static
void
gather_base_reloc_info(
uint32_t addr,
struct relocation_info *relocs, 
uint32_t nreloc,
cpu_type_t cpu_type,
uint32_t length,
int macho_reloc_type,
int base_reloc_type)
{
    uint32_t i, r_address, r_pcrel, r_length, r_extern, r_type;
    struct scattered_relocation_info *sreloc;

	for(i = 0; i < nreloc; i++){
	    if((relocs[i].r_address & R_SCATTERED) != 0){
		sreloc = (struct scattered_relocation_info *)(relocs + i);
		r_address = sreloc->r_address;
		r_pcrel = sreloc->r_pcrel;
		r_length = sreloc->r_length;
		r_type = (enum reloc_type_generic)sreloc->r_type;
		r_extern = 0;
	    }
	    else{
		r_address = relocs[i].r_address;
		r_pcrel = relocs[i].r_pcrel;
		r_length = relocs[i].r_length;
		r_extern = relocs[i].r_extern;
		r_type = (enum reloc_type_generic)relocs[i].r_type;
	    }

	    if(r_extern == 0 && r_pcrel == 0 &&
	       r_length == length && r_type == macho_reloc_type)
		add_base_reloc(addr + r_address, base_reloc_type);
	    else
	    	; /* TODO add checking and error messages here */

	    if((relocs[i].r_address & R_SCATTERED) == 0){
		if(reloc_has_pair(cpu_type, relocs[i].r_type))
		    i++;
	    }
	    else{
		sreloc = (struct scattered_relocation_info *)relocs + i;
		if(reloc_has_pair(cpu_type, sreloc->r_type))
		    i++;
	    }
	}
}

/*
 * add_base_reloc() is passed a addr and a type for a base relocation entry to
 * add to the list.
 */
static
void
add_base_reloc(
uint64_t addr,
uint32_t type)
{
    static int max = 0;
    struct base_reloc *new_base_relocs;
    
	if(!max){
	    max = 128;
	    base_relocs = (struct base_reloc *)
			  malloc(max * sizeof(struct base_reloc));
	}
	if(nbase_reloc >= max){
	    new_base_relocs = malloc(2 * max * sizeof(struct base_reloc));
	    memcpy(new_base_relocs, base_relocs,
		   max * sizeof(struct base_reloc));
	    max *= 2;
	    free(base_relocs);
	    base_relocs = new_base_relocs;
	}
	base_relocs[nbase_reloc].addr = addr;
        base_relocs[nbase_reloc].type = type;
	nbase_reloc++;
}

/*
 * usage() prints the current usage message and exits indicating failure.
 */
static
void
usage(
void)
{
	fprintf(stderr, "Usage: %s [-v] input_Mach-O output_relocs\n",
		progname);
	exit(EXIT_FAILURE);
}

/*
 * The base relocation table in a PECOFF file is divided into blocks. Each
 * block represents the base relocations for a 4K page. Each block must start
 * on a 32-bit boundary.  Which is why one "nop" base relocation entry may be
 * be added as padding in a block.
 */
#define MAX_BLOCK_OFFSET 0x1000
#define BLOCK_MASK (MAX_BLOCK_OFFSET-1)

/*
 * output_base_relocs() takes the info for the base relocation entries gathered
 * and outputs the fixup blocks as they would be in a PECOFF file in to the
 * specified file name.
 */
static
void
output_base_relocs(
char *out)
{
    int blockcnt;
    int i, entries;
    uint64_t base;
    int size;
    char *fb;
    struct base_relocation_block_header *h;
    struct base_relocation_entry *b;
    int f;
    uint32_t offset;
    enum bool swapped;
	
	blockcnt = 0;
	swapped = host_byte_sex != target_byte_sex;

	if(nbase_reloc == 0)
	    goto done;
	
	qsort(base_relocs, nbase_reloc, sizeof(struct base_reloc),
	      (int (*)(const void *, const void *))cmp_base_relocs);
	
	/*
	 * The size of the base relocation tables must be a multiple of 4 bytes.
	 * so we may need to add one relocation entry as padding.  We make this
	 * fixup block large enought to hold all the base relocation entries.
	 * But it will be broken up for the base relocation entries for each
	 * each group that refers to the same 4K page.
	 */
	size = sizeof(struct base_relocation_block_header) +
	       (nbase_reloc + 1) * sizeof(struct base_relocation_entry);
	fb = malloc(size);
	
	f = open(out, O_WRONLY|O_CREAT|O_TRUNC, 0644);
	if(f == -1){
	    fatal("open output file");
	}
	
	entries = 0;
	base = base_relocs[0].addr & ~BLOCK_MASK;
	h = (struct base_relocation_block_header *)fb;
	b = (struct base_relocation_entry *)
	    (fb + sizeof(struct base_relocation_block_header));
	for(i = 0; i < nbase_reloc; i++){
	    offset = base_relocs[i].addr - base;
	    if(offset >= MAX_BLOCK_OFFSET) {
		/* add padding if needed */
		if((entries % 2) != 0){
		    b[entries].type = IMAGE_REL_BASED_ABSOLUTE;
		    b[entries].offset = 0;
		    entries++;
		}
		h->page_rva = base;
		size = sizeof(struct base_relocation_block_header) +
		       entries * sizeof(struct base_relocation_entry);
		h->block_size = size;
		if(swapped){
		    swap_base_relocation_block_header(h,
						      target_byte_sex);
		    swap_base_relocation_entry(b, entries,
					       target_byte_sex);
		}
		// write out the block then start a new one
		if (write64(f, fb, size) != size)
                    fatal("failed to write block");

		entries = 0;
		blockcnt++;
		base = base_relocs[i].addr & ~BLOCK_MASK;
		offset = base_relocs[i].addr - base;
	    }
	    b[entries].type = base_relocs[i].type;
	    b[entries].offset = offset;
	    entries++;
	}
	
	/* add padding if needed */
	if((entries % 2) != 0){
	    b[entries].type = IMAGE_REL_BASED_ABSOLUTE;
	    b[entries].offset = 0;
	    entries++;
	}
	h->page_rva = base;
	size = sizeof(struct base_relocation_block_header) +
	       entries * sizeof(struct base_relocation_entry);
	h->block_size = size;
	if(swapped){
	    swap_base_relocation_block_header(h, target_byte_sex);
	    swap_base_relocation_entry(b, entries, target_byte_sex);
	}
	/* write out the last block */
        if (write64(f, fb, size) != size)
            fatal("failed to write last block");

	blockcnt++;
	close(f);
done:
	printf("wrote %d relocations in %d block%s\n", nbase_reloc, blockcnt,
	       blockcnt == 1 ? "" : "s");
}

static
int
cmp_base_relocs(
struct base_reloc *x1,
struct base_reloc *x2)
{
	if(x1->addr < x2->addr)
	    return(-1);
	if(x1->addr == x2->addr)
	    return(0);
	/* x1->addr > x2->addr */
	    return(1);
}
