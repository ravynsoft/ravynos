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
/*
 * The segedit(1) program.  This program extracts and replaces sections from
 * an object file.  Only sections in segments that have been marked that they
 * have no relocation can be replaced (SG_NORELOC).  This program takes the
 * following options:
 *   -extract <segname> <sectname> <filename>
 *   -replace <segname> <sectname> <filename>
 *   -output <filename>
 */
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h> /* cctools-port: For bcmp, bzero ... */
#include <errno.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <libc.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <mach/mach.h>
#include "stuff/openstep_mach.h"
#include <mach/mach_error.h>
#include "stuff/allocate.h"
#include "stuff/errors.h"
#include "stuff/rnd.h"
#include "stuff/bytesex.h"
#include "stuff/write64.h"
#include "stuff/arch.h"

/* These variables are set from the command line arguments */
__private_extern__
char *progname = NULL;	/* name of the program for error messages (argv[0]) */

static char *input,	/* object file to extract/replace sections from */
     	    *output;	/* new object file with replaced sections, if any
			   -replace options */

/* structure for holding -extract's arguments */
struct extract {
    char *segname;		/* segment name */
    char *sectname;		/* section name */
    char *filename;		/* file to put the section contents in */
    int32_t found;		/* set when the section is found */
    struct extract *next;	/* next extract structure, NULL if last */
} *extracts;			/* first extract structure, NULL if none */

/* structure for holding -replace's arguments */
struct replace {
    char *segname;		/* segment name */
    char *sectname;		/* section name */
    char *filename;		/* file to get the section contents from */
    int32_t found;		/* set when the section is found */
    uint32_t size;		/* size of new section contents */
    struct replace *next;	/* next replace structure, NULL if last */
} *replaces;			/* first replace structure, NULL if none */

/*
 * Structures used in replace_sections in replaceing sections in the segments
 * of the input file.  There is one such structure for each segment and section.
 */
struct rep_seg {
    int32_t modified;		/* this segment has a replaced section */
    uint32_t fileoff;		/* original file offset */
    uint32_t filesize;		/* original file size */
    uint64_t vmsize;		/* original vm size */
    uint32_t padsize;		/* new pad size */
    struct segment_command *sgp;/* pointer to the segment_command */
    struct segment_command_64 *sgp64;/* pointer to the segment_command_64 */
} *segs;

struct rep_sect {
    struct replace *rp;		/* pointer to the replace structure */
    uint32_t offset;		/* original file offset */
    struct section *sp;		/* pointer to the section structure */
    struct section_64 *sp64;	/* pointer to the section_64 structure */
} *sects;

/* These variables are set in the routine map_input() */
static void *input_addr;	/* address of where the input file is mapped */
static uint32_t input_size;	/* size of the input file */
static uint32_t input_mode;	/* mode of the input file */
static struct mach_header *mhp;	/* pointer to the input file's mach header */
static struct mach_header_64
			*mhp64;	/* pointer to the input file's mach header for
				   64-bit files */
static uint32_t mh_ncmds;	/* number of load commands */
static struct load_command
		*load_commands;	/* pointer to the input file's load commands */
static enum bool swapped;	/* TRUE if the input is to be swapped */
static enum byte_sex host_byte_sex = UNKNOWN_BYTE_SEX;
static enum byte_sex target_byte_sex = UNKNOWN_BYTE_SEX;

/* Internal routines */
static void map_input(
    void);
static void extract_sections(
    void);
static void extract_section(
    char *segname,
    char *sectname,
    uint32_t flags,
    uint32_t offset,
    uint32_t size);
static void replace_sections(
    void);
static void search_for_replace_section(
    char *segname,
    char *sectname,
    uint32_t seg_flags,
    uint32_t sect_flags,
    uint32_t offset,
    uint32_t size);
static int cmp_qsort(
    const struct rep_seg *seg1,
    const struct rep_seg *seg2);
static void usage(
    void);

int
main(
int argc,
char *argv[],
char *envp[])
{
    int i;
    struct extract *ep;
    struct replace *rp;

	progname = argv[0];
	host_byte_sex = get_host_byte_sex();

	for (i = 1; i < argc; i++) {
	    if(argv[i][0] == '-'){
		if (strncmp("-extract", argv[i], 9) == 0 ||
		    strncmp("-e", argv[i], 3) == 0) {
		    if(i + 4 > argc){
			error("missing arguments to %s option", argv[i]);
			usage();
		    }
		    ep = allocate(sizeof(struct extract));
		    ep->segname =  argv[i + 1];
		    ep->sectname = argv[i + 2];
		    ep->filename = argv[i + 3];
		    ep->found = 0;
		    ep->next = extracts;
		    extracts = ep;
		    i += 3;
		}
		else if (strncmp("-replace", argv[i], 9) == 0 ||
			   strncmp("-r", argv[i], 3) == 0) {
		    if(i + 4 > argc){
			error("missing arguments to %s option", argv[i]);
			usage();
		    }
		    rp = allocate(sizeof(struct replace));
		    rp->segname =  argv[i + 1];
		    rp->sectname = argv[i + 2];
		    rp->filename = argv[i + 3];
		    rp->next = replaces;
		    replaces = rp;
		    i += 3;
		}
		else if (strncmp("-output", argv[i], 8) == 0 ||
			   strncmp("-o", argv[i], 3) == 0) {
		    if(output != NULL)
			fatal("more than one %s option", argv[i]);
		    output = argv[i + 1];
		    i += 1;
		}
		else {
		    error("unrecognized option: %s", argv[i]);
		    usage();
		}
	    }
	    else{
		if(input != NULL){
		    fatal("only one input file can be specified");
		    usage();
		}
		input = argv[i];
	    }
	}

	if(input == NULL){
	    error("no input file specified");
	    usage();
	}
	if(replaces != NULL && output == NULL)
	    fatal("output file must be specified via -o <filename> when "
		  "replacing a section");

	if(extracts == NULL && replaces == NULL){
	    error("no -extract or -replace options specified");
	    usage();
	}

	map_input();

	if(extracts != NULL)
	    extract_sections();

	if(replaces != NULL)
	    replace_sections();

	return(0);
}

/*
 * map_input maps the input file into memory.  The address it is mapped at is
 * left in input_addr and the size is left in input_size.  The input file is
 * checked to be an object file and that the headers are checked to be correct
 * enough to loop through them.  The pointer to the mach header is left in mhp
 * and the pointer to the load commands is left in load_commands.
 */
static
void
map_input(void)
{
    int fd;
    uint32_t i, magic, mh_sizeofcmds;
    struct stat stat_buf;
    struct load_command l, *lcp;
    struct segment_command *sgp;
    struct segment_command_64 *sgp64;
    struct section *sp;
    struct section_64 *sp64;
    struct symtab_command *stp;
    struct symseg_command *ssp;

	/* Open the input file and map it in */
	if((fd = open(input, O_RDONLY)) == -1)
	    system_fatal("can't open input file: %s", input);
	if(fstat(fd, &stat_buf) == -1)
	    system_fatal("Can't stat input file: %s", input);
	input_size = (uint32_t)stat_buf.st_size;
	input_mode = stat_buf.st_mode;
	input_addr = mmap(0, input_size, PROT_READ|PROT_WRITE,
			  MAP_FILE|MAP_PRIVATE, fd, 0);
	if((intptr_t)input_addr == -1)
	    system_error("Can't map input file: %s", input);
	close(fd);

	if(sizeof(uint32_t) > input_size)
	    fatal("truncated or malformed object (mach header would extend "
		  "past the end of the file) in: %s", input);
	magic = *(uint32_t *)input_addr;
#ifdef __BIG_ENDIAN__
	if(magic == FAT_MAGIC || magic == FAT_MAGIC_64)
#endif /* __BIG_ENDIAN__ */
#ifdef __LITTLE_ENDIAN__
	if(magic == SWAP_INT(FAT_MAGIC) || magic == SWAP_INT(FAT_MAGIC_64))
#endif /* __LITTLE_ENDIAN__ */
	    fatal("file: %s is a fat file (%s only operates on Mach-O files, "
		  "use lipo(1) on it to get a Mach-O file)", input, progname);

	mh_ncmds = 0;
	mh_sizeofcmds = 0;
	host_byte_sex = get_host_byte_sex();
	if(magic == SWAP_INT(MH_MAGIC) || magic == MH_MAGIC){
	    if(sizeof(struct mach_header) > input_size)
		fatal("truncated or malformed object (mach header would extend "
		      "past the end of the file) in: %s", input);
	    mhp = (struct mach_header *)input_addr;
	    if(magic == SWAP_INT(MH_MAGIC)){
		swapped = TRUE;
		target_byte_sex = host_byte_sex == BIG_ENDIAN_BYTE_SEX ?
				  LITTLE_ENDIAN_BYTE_SEX : BIG_ENDIAN_BYTE_SEX;
		swap_mach_header(mhp, host_byte_sex);
	    }
	    else{
		swapped = FALSE;
		target_byte_sex = host_byte_sex;
	    }
	    if(mhp->sizeofcmds + sizeof(struct mach_header) > input_size)
		fatal("truncated or malformed object (load commands would "
		      "extend past the end of the file) in: %s", input);
	    load_commands = (struct load_command *)((char *)input_addr +
				sizeof(struct mach_header));
	    mh_ncmds = mhp->ncmds;
	    mh_sizeofcmds = mhp->sizeofcmds;
	}
	else if(magic == SWAP_INT(MH_MAGIC_64) || magic == MH_MAGIC_64){
	    if(sizeof(struct mach_header_64) > input_size)
		fatal("truncated or malformed object (mach header would extend "
		      "past the end of the file) in: %s", input);
	    mhp64 = (struct mach_header_64 *)input_addr;
	    if(magic == SWAP_INT(MH_MAGIC_64)){
		swapped = TRUE;
		target_byte_sex = host_byte_sex == BIG_ENDIAN_BYTE_SEX ?
				  LITTLE_ENDIAN_BYTE_SEX : BIG_ENDIAN_BYTE_SEX;
		swap_mach_header_64(mhp64, host_byte_sex);
	    }
	    else{
		swapped = FALSE;
		target_byte_sex = host_byte_sex;
	    }
	    if(mhp64->sizeofcmds + sizeof(struct mach_header_64) > input_size)
		fatal("truncated or malformed object (load commands would "
		      "extend past the end of the file) in: %s", input);
	    load_commands = (struct load_command *)((char *)input_addr +
				sizeof(struct mach_header_64));
	    mh_ncmds = mhp64->ncmds;
	    mh_sizeofcmds = mhp64->sizeofcmds;
	}
	else
	    fatal("bad magic number (file is not a Mach-O file) in: %s", input);

	lcp = load_commands;
	for(i = 0; i < mh_ncmds; i++){
	    l = *lcp;
	    if(swapped)
		swap_load_command(&l, host_byte_sex);
	    if(l.cmdsize % sizeof(uint32_t) != 0)
		error("load command %u size not a multiple of "
		      "sizeof(uint32_t) in: %s", i, input);
	    if(l.cmdsize <= 0)
		fatal("load command %u size is less than or equal to zero "
		      "in: %s", i, input);
	    if((char *)lcp + l.cmdsize >
	       (char *)load_commands + mh_sizeofcmds)
		fatal("load command %u extends past end of all load commands "
		      "in: %s", i, input);
	    switch(l.cmd){
	    case LC_SEGMENT:
		sgp = (struct segment_command *)lcp;
		sp = (struct section *)((char *)sgp +
					sizeof(struct segment_command));
		if(swapped)
		    swap_segment_command(sgp, host_byte_sex);
		if(swapped)
		    swap_section(sp, sgp->nsects, host_byte_sex);
		break;
	    case LC_SEGMENT_64:
		sgp64 = (struct segment_command_64 *)lcp;
		sp64 = (struct section_64 *)((char *)sgp64 +
					sizeof(struct segment_command_64));
		if(swapped)
		    swap_segment_command_64(sgp64, host_byte_sex);
		if(swapped)
		    swap_section_64(sp64, sgp64->nsects, host_byte_sex);
		break;
	    case LC_SYMTAB:
		stp = (struct symtab_command *)lcp;
		if(swapped)
		    swap_symtab_command(stp, host_byte_sex);
		break;
	    case LC_SYMSEG:
		ssp = (struct symseg_command *)lcp;
		if(swapped)
		    swap_symseg_command(ssp, host_byte_sex);
		break;
	    default:
		*lcp = l;
		break;
	    }
	    lcp = (struct load_command *)((char *)lcp + l.cmdsize);
	}
}

/*
 * This routine extracts the sections in the extracts list from the input file
 * and writes then to the file specified in the list.
 */
static
void
extract_sections(void)
{
    uint32_t i, j, errors;
    struct load_command *lcp;
    struct segment_command *sgp;
    struct segment_command_64 *sgp64;
    struct section *sp;
    struct section_64 *sp64;
    struct extract *ep;

	lcp = load_commands;
	for(i = 0; i < mh_ncmds; i++){
	    if(lcp->cmd == LC_SEGMENT){
		sgp = (struct segment_command *)lcp;
		sp = (struct section *)((char *)sgp +
					sizeof(struct segment_command));
		for(j = 0; j < sgp->nsects; j++){
		    extract_section(sp->segname, sp->sectname, sp->flags,
				    sp->offset, sp->size);
		    sp++;
		}
	    }
	    else if(lcp->cmd == LC_SEGMENT_64){
		sgp64 = (struct segment_command_64 *)lcp;
		sp64 = (struct section_64 *)((char *)sgp64 +
					sizeof(struct segment_command_64));
		for(j = 0; j < sgp64->nsects; j++){
		    extract_section(sp64->segname, sp64->sectname, sp64->flags,
				    (uint32_t)sp64->offset,
				    (uint32_t)sp64->size);
		    sp64++;
		}
	    }
	    lcp = (struct load_command *)((char *)lcp + lcp->cmdsize);
	}

	errors = 0;
	ep = extracts;
	while(ep != NULL){
	    if(ep->found == 0){
		error("section (%s,%s) not found in: %s", ep->segname,
		      ep->sectname, input);
		errors = 1;
	    }
	    ep = ep->next;
	}
	if(errors != 0)
	    exit(1);
}

static
void
extract_section(
char *segname,
char *sectname,
uint32_t flags,
uint32_t offset,
uint32_t size)
{
    struct extract *ep;
    int fd;

	ep = extracts;
	while(ep != NULL){
	    if(ep->found == 0 &&
	       strncmp(ep->segname, segname, 16) == 0 &&
	       strncmp(ep->sectname, sectname, 16) == 0){
		if(flags == S_ZEROFILL || flags == S_THREAD_LOCAL_ZEROFILL)
		    fatal("meaningless to extract zero fill "
			  "section (%s,%s) in: %s", segname,
			  sectname, input);
		if(offset + size > input_size)
		    fatal("truncated or malformed object (section "
			  "contents of (%s,%s) extends past the "
			  "end of the file) in: %s", segname,
			  sectname, input);
		 if((fd = open(ep->filename, O_WRONLY | O_CREAT |
			       O_TRUNC, 0666)) == -1)
		    system_fatal("can't create: %s", ep->filename);
		 if(write64(fd, (char *)input_addr + offset,
                            size) != (ssize_t)size)
		    system_fatal("can't write: %s", ep->filename);
		 if(close(fd) == -1)
		    system_fatal("can't close: %s", ep->filename);
		 ep->found = 1;
	    }
	    ep = ep->next;
	}
}

static
void
replace_sections(void)
{
    uint32_t i, j, k, l, errors, nsegs, nsects, high_reloc_seg;
    uint32_t low_noreloc_seg, high_noreloc_seg, low_linkedit;
    uint32_t oldoffset, newoffset, oldsectsize, newsectsize;
    uint64_t oldvmaddr, newvmaddr;
    struct load_command lc, *lcp;
    struct segment_command *sgp, *linkedit_sgp;
    struct segment_command_64 *sgp64, *linkedit_sgp64;
    struct section *sp;
    struct section_64 *sp64;
    struct symtab_command *stp;
    struct symseg_command *ssp;
    struct dysymtab_command* dstp;
    struct replace *rp;
    struct stat stat_buf;
    int outfd, sectfd;
    char *sect_addr;
    vm_address_t pad_addr;
    uint32_t size;
    kern_return_t r;
    enum bool no_seg_resize;
    uint32_t pagesize;
    
	errors = 0;

	high_reloc_seg = 0;
	low_noreloc_seg = input_size;
	high_noreloc_seg = 0;
	low_linkedit = input_size;

	nsegs = 0;
	segs = allocate(mh_ncmds * sizeof(struct rep_seg));
	bzero(segs, mh_ncmds * sizeof(struct rep_seg));
	nsects = 0;

	stp = NULL;
	ssp = NULL;
	linkedit_sgp = NULL;
	linkedit_sgp64 = NULL;
	dstp = NULL;
    
	no_seg_resize = FALSE;

	/*
	 * First pass over the load commands and determine if the file is laided
	 * out in an order that the specified sections can be replaced.  Also
	 * determine if the specified sections exist in the input file and if
	 * it is marked with no relocation so it can be replaced. 
	 */
	lcp = load_commands;
	for(i = 0; i < mh_ncmds; i++){
	    switch(lcp->cmd){
	    case LC_SEGMENT:
		sgp = (struct segment_command *)lcp;
		sp = (struct section *)((char *)sgp +
					sizeof(struct segment_command));
		segs[nsegs++].sgp = sgp;
		nsects += sgp->nsects;
		if(strcmp(sgp->segname, SEG_LINKEDIT) != 0){
		    if(sgp->flags & SG_NORELOC){
			if(sgp->filesize != 0){
			    if(sgp->fileoff + sgp->filesize > high_noreloc_seg)
				high_noreloc_seg = sgp->fileoff + sgp->filesize;
			    if(sgp->fileoff < low_noreloc_seg)
				low_noreloc_seg = sgp->fileoff;
			}
		    }
		    else{
			if(sgp->filesize != 0 &&
			   sgp->fileoff + sgp->filesize > high_reloc_seg)
			    high_reloc_seg = sgp->fileoff + sgp->filesize;
		    }
		}
		else{
		    if(linkedit_sgp != NULL)
			fatal("more than one " SEG_LINKEDIT " segment found "
			      "in: %s", input);
		    linkedit_sgp = sgp;
		}
		for(j = 0; j < sgp->nsects; j++){
		    if(sp->nreloc != 0 && sp->reloff < low_linkedit)
			low_linkedit = sp->reloff;
		    search_for_replace_section(sp->segname, sp->sectname,
    			sgp->flags, sp->flags, sp->offset, sp->size);
		    sp++;
		}
		break;
	    case LC_SEGMENT_64:
		sgp64 = (struct segment_command_64 *)lcp;
		sp64 = (struct section_64 *)((char *)sgp64 +
					sizeof(struct segment_command_64));
		segs[nsegs++].sgp64 = sgp64;
		nsects += sgp64->nsects;
		if(strcmp(sgp64->segname, SEG_LINKEDIT) != 0){
		    if(sgp64->flags & SG_NORELOC){
			if(sgp64->filesize != 0){
			    if(sgp64->fileoff + sgp64->filesize >
			       high_noreloc_seg)
				high_noreloc_seg = (uint32_t)(sgp64->fileoff +
						   sgp64->filesize);
			    if(sgp64->fileoff < low_noreloc_seg)
				low_noreloc_seg = (uint32_t)sgp64->fileoff;
			}
		    }
		    else{
			if(sgp64->filesize != 0 &&
			   sgp64->fileoff + sgp64->filesize > high_reloc_seg)
			    high_reloc_seg = (uint32_t)(sgp64->fileoff +
							sgp64->filesize);
		    }
		}
		else{
		    if(linkedit_sgp64 != NULL)
			fatal("more than one " SEG_LINKEDIT " segment found "
			      "in: %s", input);
		    linkedit_sgp64 = sgp64;
		}
		for(j = 0; j < sgp64->nsects; j++){
		    if(sp64->nreloc != 0 && sp64->reloff < low_linkedit)
			low_linkedit = sp64->reloff;
		    search_for_replace_section(sp64->segname, sp64->sectname,
    			sgp64->flags, sp64->flags, sp64->offset,
					       (uint32_t)sp64->size);
		    sp64++;
		}
		break;
	    case LC_SYMTAB:
		if(stp != NULL)
		    fatal("more than one symtab_command found in: %s", input);
		stp = (struct symtab_command *)lcp;
		if(stp->nsyms != 0 && stp->symoff < low_linkedit)
		    low_linkedit = stp->symoff;
		if(stp->strsize != 0 && stp->stroff < low_linkedit)
		    low_linkedit = stp->stroff;
		break;
	    case LC_DYSYMTAB:
		if (dstp != NULL)
		    fatal("more than one dysymtab_command found in: %s", input);
		no_seg_resize = TRUE;
		dstp = (struct dysymtab_command*)lcp;
		if (dstp->tocoff != 0 && dstp->tocoff < low_linkedit)
		    low_linkedit = dstp->tocoff;
		if (dstp->modtaboff != 0 && dstp->modtaboff < low_linkedit)
		    low_linkedit = dstp->modtaboff;
		if (dstp->extrefsymoff != 0 &&
		    dstp->extrefsymoff < low_linkedit)
		    low_linkedit = dstp->extrefsymoff;
		if (dstp->indirectsymoff != 0 &&
		    dstp->indirectsymoff < low_linkedit)
		    low_linkedit = dstp->indirectsymoff;
		if (dstp->extreloff != 0 && dstp->extreloff < low_linkedit)
		    low_linkedit = dstp->extreloff;
		if (dstp->locreloff != 0 && dstp->locreloff < low_linkedit)
		    low_linkedit = dstp->locreloff;
		break;
	    case LC_SYMSEG:
		if(ssp != NULL)
		    fatal("more than one symseg_command found in: %s", input);
		ssp = (struct symseg_command *)lcp;
		if(ssp->size != 0 && ssp->offset < low_linkedit)
		    low_linkedit = ssp->offset;
		break;
	    case LC_CODE_SIGNATURE:
	    case LC_SEGMENT_SPLIT_INFO:
	    case LC_FUNCTION_STARTS:
	    case LC_DATA_IN_CODE:
	    case LC_DYLIB_CODE_SIGN_DRS:
	    case LC_LINKER_OPTIMIZATION_HINT:
		{
		    struct linkedit_data_command* ldcp =
			(struct linkedit_data_command*)lcp;
		    if (ldcp->dataoff != 0 && ldcp->dataoff < low_linkedit)
			low_linkedit = ldcp->dataoff;
		}
		break;
	    case LC_DYLD_INFO:
	    case LC_DYLD_INFO_ONLY:
		{
		    struct dyld_info_command* dicp =
			(struct dyld_info_command*)lcp;
		    no_seg_resize = TRUE;
		    if (dicp->rebase_off != 0 &&
			dicp->rebase_off < low_linkedit)
			low_linkedit = dicp->rebase_off;
		    if (dicp->bind_off != 0 && dicp->bind_off < low_linkedit)
			low_linkedit = dicp->bind_off;
		    if (dicp->weak_bind_off != 0 &&
			dicp->weak_bind_off < low_linkedit)
			low_linkedit = dicp->weak_bind_off;
		    if (dicp->lazy_bind_off != 0 &&
			dicp->lazy_bind_off < low_linkedit)
			low_linkedit = dicp->lazy_bind_off;
		    if (dicp->export_off != 0 &&
			dicp->export_off < low_linkedit)
			low_linkedit = dicp->export_off;
		}
		break;
	    case LC_DYLD_CHAINED_FIXUPS:
	    case LC_DYLD_EXPORTS_TRIE:
		{
		    struct linkedit_data_command* ldcp =
			(struct linkedit_data_command*)lcp;
		    no_seg_resize = TRUE;
		    if (ldcp->dataoff != 0 && ldcp->dataoff < low_linkedit)
			low_linkedit = ldcp->dataoff;
		}
	    case LC_THREAD:
	    case LC_UNIXTHREAD:
	    case LC_LOADFVMLIB:
	    case LC_IDFVMLIB:
	    case LC_IDENT:
	    case LC_FVMFILE:
	    case LC_PREPAGE:
	    case LC_LOAD_DYLIB:
	    case LC_LOAD_WEAK_DYLIB:
	    case LC_REEXPORT_DYLIB:
	    case LC_LOAD_UPWARD_DYLIB:
	    case LC_ID_DYLIB:
	    case LC_LOAD_DYLINKER:
	    case LC_ID_DYLINKER:
	    case LC_DYLD_ENVIRONMENT:
	    case LC_PREBOUND_DYLIB:
	    case LC_ROUTINES:
	    case LC_SUB_FRAMEWORK:
	    case LC_SUB_UMBRELLA:
	    case LC_SUB_CLIENT:
	    case LC_SUB_LIBRARY:
	    case LC_TWOLEVEL_HINTS:
	    case LC_PREBIND_CKSUM:
	    case LC_ROUTINES_64:
	    case LC_UUID:
	    case LC_RPATH:
	    case LC_LAZY_LOAD_DYLIB:
	    case LC_ENCRYPTION_INFO:
	    case LC_ENCRYPTION_INFO_64:
	    case LC_VERSION_MIN_MACOSX:
	    case LC_VERSION_MIN_IPHONEOS:
	    case LC_VERSION_MIN_TVOS:
	    case LC_VERSION_MIN_WATCHOS:
	    case LC_NOTE:
	    case LC_BUILD_VERSION:
	    case LC_MAIN:
	    case LC_SOURCE_VERSION:
	    case LC_LINKER_OPTION:
		break;
	    default:
		error("unknown load command %u (result maybe bad)", i);
		break;
	    }
	    lcp = (struct load_command *)((char *)lcp + lcp->cmdsize);
	}
	rp = replaces;
	while(rp != NULL){
	    if(rp->found == 0){
		error("section (%s,%s) not found in: %s", rp->segname,
		      rp->sectname, input);
		errors = 1;
	    }
	    else{
		if(stat(rp->filename, &stat_buf) == -1){
		    system_error("Can't stat file: %s to replace section "
				 "(%s,%s) with", rp->filename, rp->segname,
				 rp->sectname);
		    errors = 1;
		}
		rp->size = (uint32_t)stat_buf.st_size;
	    }
	    rp = rp->next;
	}
	if(errors != 0)
	    exit(1);

	if(high_reloc_seg > low_noreloc_seg ||
	   high_reloc_seg > low_linkedit ||
	   high_noreloc_seg > low_linkedit)
	    fatal("contents of input file: %s not in an order that the "
		  "specified sections can be replaced by this program", input);

	qsort(segs, nsegs, sizeof(struct rep_seg),
	      (int (*)(const void *, const void *))cmp_qsort);

	sects = allocate(nsects * sizeof(struct rep_sect));
	bzero(sects, nsects * sizeof(struct rep_sect));

	/*
	 * Set the pagesize to match that of the input file. Note that we cannot
	 * assume a single pagesize across all Mach-O files.
	 */
	pagesize = 0;
	if (0 == pagesize)
	{
	    cpu_type_t cputype = (mhp != NULL ?
				  mhp->cputype : mhp64->cputype);
	    cpu_subtype_t cpusubtype = (mhp != NULL ?
					mhp->cpusubtype : mhp64->cpusubtype);
	    const char* arch_name = get_arch_name_from_types(cputype,
							     cpusubtype);
	    if (arch_name != NULL) {
		struct arch_flag arch_flag;
		if (get_arch_from_flag((char*)arch_name, &arch_flag)) {
		    pagesize = get_segalign_from_flag(&arch_flag);
		}
		else {
		    /* unreachable? */
		    fatal("unknown cputype %u found in %s", cputype, input);
		}
	    }
	    else {
		fatal("unknown cputype %u found in %s", cputype, input);
	    }
	}
 
	/*
	 * First go through the segments and adjust the segment offsets, sizes
	 * and addresses without adjusting the offset to the relocation entries.
	 * This program can only handle object files that have contigious
	 * address spaces starting at zero and that the offsets in the file for
	 * the contents of the segments also being contiguious and in the same
	 * order as the vmaddresses.
	 *
	 * Note that in a modern Mach-O file, segedit can resize individual
	 * sections, but their overall segment sizes cannot change. There once
	 * was a time in early NeXTSTEP when that was not true...
	 */
	oldvmaddr = 0;
	newvmaddr = 0;
	if(nsegs > 1){
	    if(segs[0].sgp != NULL)
		oldoffset = segs[0].sgp->fileoff;
	    else
		oldoffset = (uint32_t)segs[0].sgp64->fileoff;
	}
	else
	    oldoffset = 0;
	newoffset = 0;
	k = 0;
	for(i = 0; i < nsegs; i++){
	    if(segs[i].sgp != NULL){ /* process the 32-bit file */
		/* verify the segment vmaddrs are contiguous */
		if(segs[i].sgp->vmaddr != oldvmaddr)
		    fatal("addresses of input file: %s not in an order that "
			  "the specified sections can be replaced by this "
			  "program", input);
		/*
		 * initialize the rep_seg struct's filesize, vmsize, and vmaddr
		 * fields. update the vmaddr if this file is resizeable (hint:
		 * it's not).
		 */
		segs[i].filesize = segs[i].sgp->filesize;
		segs[i].vmsize = segs[i].sgp->vmsize;
		if (no_seg_resize == FALSE) {
		    segs[i].sgp->vmaddr = (uint32_t)newvmaddr;
		} else {
		    if(segs[i].sgp->vmaddr != newvmaddr)
			fatal("input file: %s segment %.16s vmaddr cannot be "
			      "repositioned",
			      input, segs[i].sgp->segname);
		}
		
		if(segs[i].sgp->filesize != 0)
		{
		    /* verify the segment fileoffs are contiguous */
		    if(segs[i].sgp->fileoff != oldoffset)
			fatal("segment offsets of input file: %s not in an "
			      "order that the specified sections can be "
			      "replaced by this program", input);
		    /*
		     * initialize the rep_seg struct's fileoff field, and adjust
		     * it if this file is resizeable (hint: still no.)
		     */
		    segs[i].fileoff = segs[i].sgp->fileoff;
		    if(strcmp(segs[i].sgp->segname, SEG_LINKEDIT) != 0 ||
		       i != nsegs - 1) {
			if (no_seg_resize == FALSE) {
			    segs[i].sgp->fileoff = newoffset;
			} else {
			    if (segs[i].sgp->fileoff != newoffset) {
				fatal("input file: %s segment %.16s fileoff "
				      "cannot be repositioned",
				      input, segs[i].sgp->segname);
			    }
			}
		    }
		}

		sp = (struct section *)((char *)(segs[i].sgp) +
					sizeof(struct segment_command));
		oldsectsize = 0;
		newsectsize = 0;
		if(segs[i].sgp->flags & SG_NORELOC){
		    /*
		     * this segment contains sections that can be replaced.
		     * loop over the sections and record the new offsets
		     * and sizes for the replaced content.
		     *
		     * recall that if no_seg_resize is true the sections
		     * cannot spill beyond the segment boundary.
		     */
		    for(j = 0; j < segs[i].sgp->nsects; j++){
			sects[k + j].sp = sp;
			sects[k + j].offset = sp->offset;
			oldsectsize += sp->size;
			rp = replaces;
			while(rp != NULL){
			    if(strncmp(rp->segname, sp->segname,
				       sizeof(sp->segname)) == 0 &&
			       strncmp(rp->sectname, sp->sectname,
				       sizeof(sp->sectname)) == 0){
				sects[k + j].rp = rp;
				segs[i].modified = 1;
				sp->size = rnd32(rp->size, 1 << sp->align);
				break;
			    }
			    rp = rp->next;
			}
			sp->offset = newoffset + newsectsize;
			sp->addr   = (uint32_t)(newvmaddr + newsectsize);
			newsectsize += sp->size;
			sp++;
		    }
		    if(strcmp(segs[i].sgp->segname, SEG_LINKEDIT) != 0 ||
		       i != nsegs - 1){
			if(segs[i].sgp->filesize != rnd(oldsectsize,
							  pagesize))
			    fatal("contents of input file: %s not in a "
				  "format that the specified sections can "
				  "be replaced by this program", input);
			segs[i].padsize =
			    segs[i].sgp->filesize - newsectsize;
			if (no_seg_resize == FALSE) {
			    segs[i].sgp->filesize =
				rnd32(newsectsize, pagesize);
			    segs[i].sgp->vmsize = rnd32(newsectsize,
						      pagesize);
			} else {
			    if (segs[i].sgp->filesize !=
				rnd(newsectsize, pagesize)) {
				fatal("input file: %s segment %.16s "
				      "cannot be resized (filesize %u => "
				      "%llu)",
				      input, segs[i].sgp->segname,
				      segs[i].sgp->filesize,
				      rnd(newsectsize, pagesize));
			    }
			    if (segs[i].sgp->vmsize !=
				rnd(newsectsize, pagesize)) {
				fatal("input file: %s segment %.16s "
				      "cannot be resized (vmsize %u => "
				      "%llu)",
				      input, segs[i].sgp->segname,
				      segs[i].sgp->vmsize,
				      rnd(newsectsize, pagesize));
			    }
			}
		    }
		}
		if(strcmp(segs[i].sgp->segname, SEG_LINKEDIT) != 0 ||
		   i != nsegs - 1){
		    oldoffset += segs[i].filesize;
		    newoffset += segs[i].sgp->filesize;
		}

		oldvmaddr += segs[i].vmsize;
		newvmaddr += segs[i].sgp->vmsize;
		k += segs[i].sgp->nsects;
	    }
	    else { /* process the 64-bit file */
		/* verify the segment vmaddrs are contiguous */
		if(segs[i].sgp64->vmaddr != oldvmaddr)
		    fatal("addresses of input file: %s not in an order that "
			  "the specified sections can be replaced by this "
			  "program", input);
		/*
		 * initialize the rep_seg struct's filesize, vmsize, and vmaddr
		 * fields. update the vmaddr if this file is resizeable (hint:
		 * it's not).
		 */
		segs[i].filesize = (uint32_t)segs[i].sgp64->filesize;
		segs[i].vmsize = segs[i].sgp64->vmsize;
		if (no_seg_resize == FALSE) {
		    segs[i].sgp64->vmaddr = newvmaddr;
		} else {
		    if(segs[i].sgp64->vmaddr != newvmaddr)
			fatal("input file: %s segment %.16s vmaddr cannot be "
			      "repositioned", input, segs[i].sgp64->segname);
		}

		if(segs[i].sgp64->filesize != 0)
		{
		    /* verify the segment fileoffs are contiguous */
		    if(segs[i].sgp64->fileoff != oldoffset)
			fatal("segment offsets of input file: %s not in an "
			      "order that the specified sections can be "
			      "replaced by this program", input);
		    /*
		     * initialize the rep_seg struct's fileoff field, and adjust
		     * it if this file is resizeable (hint: still no.)
		     */
		    segs[i].fileoff = (uint32_t)segs[i].sgp64->fileoff;
		    if(strcmp(segs[i].sgp64->segname, SEG_LINKEDIT) != 0 ||
		       i != nsegs - 1) {
			if (no_seg_resize == FALSE) {
			    segs[i].sgp64->fileoff = newoffset;
			} else {
			    if (segs[i].sgp64->fileoff != newoffset) {
				fatal("input file: %s segment %.16s fileoff "
				      "cannot be repositioned",
				      input, segs[i].sgp64->segname);
			    }
			}
		    }
		}
		sp64 = (struct section_64 *)((char *)(segs[i].sgp64) +
					sizeof(struct segment_command_64));
		oldsectsize = 0;
		newsectsize = 0;
		if(segs[i].sgp64->flags & SG_NORELOC){
		    /*
		     * this segment contains sections that can be replaced.
		     * loop over the sections and record the new offsets
		     * and sizes for the replaced content.
		     *
		     * recall that if no_seg_resize is true the sections
		     * cannot spill beyond the segment boundary.
		     */
		    for(j = 0; j < segs[i].sgp64->nsects; j++){
			sects[k + j].sp64 = sp64;
			sects[k + j].offset = sp64->offset;
			oldsectsize += sp64->size;
			rp = replaces;
			while(rp != NULL){
			    if(strncmp(rp->segname, sp64->segname,
				       sizeof(sp64->segname)) == 0 &&
			       strncmp(rp->sectname, sp64->sectname,
				       sizeof(sp64->sectname)) == 0){
				sects[k + j].rp = rp;
				segs[i].modified = 1;
				sp64->size =
				   rnd(rp->size, 1 << sp64->align);
				break;
			    }
			    rp = rp->next;
			}
			sp64->offset = newoffset + newsectsize;
			sp64->addr   = newvmaddr + newsectsize;
			newsectsize += sp64->size;
			sp64++;
		    }
		    if(strcmp(segs[i].sgp64->segname, SEG_LINKEDIT) != 0 ||
		       i != nsegs - 1){
			if(segs[i].sgp64->filesize != rnd(oldsectsize,
							  pagesize))
			    fatal("contents of input file: %s not in a "
				  "format that the specified sections can "
				  "be replaced by this program", input);
			segs[i].padsize = (uint32_t)
			    (segs[i].sgp64->filesize - newsectsize);
			if (no_seg_resize == FALSE) {
			    segs[i].sgp64->filesize =
				rnd(newsectsize, pagesize);
			    segs[i].sgp64->vmsize =
				rnd(newsectsize, pagesize);
			} else {
			    if (segs[i].sgp64->filesize !=
				rnd(newsectsize, pagesize)) {
				fatal("input file: %s segment %.16s "
				      "cannot be resized (filesize %llu => "
				      "%llu)",
				      input, segs[i].sgp64->segname,
				      segs[i].sgp64->filesize,
				      rnd(newsectsize, pagesize));
			    }
			    if (segs[i].sgp64->vmsize !=
				rnd(newsectsize, pagesize)) {
				fatal("input file: %s segment %.16s "
				      "cannot be resized (vmsize %llu => "
				      "%llu)",
				       input, segs[i].sgp64->segname,
				      segs[i].sgp64->vmsize,
				      rnd(newsectsize, pagesize));
			    }
			}
		    }
		}
		if(strcmp(segs[i].sgp64->segname, SEG_LINKEDIT) != 0 ||
		   i != nsegs - 1){
		    oldoffset += segs[i].filesize;
		    newoffset += segs[i].sgp64->filesize;
		}

		oldvmaddr += segs[i].vmsize;
		newvmaddr += segs[i].sgp64->vmsize;
		k += segs[i].sgp64->nsects;
	    }
	}

	/*
	 * Now update the offsets to the linkedit information.
	 */
	if (no_seg_resize == FALSE) {
	    if(oldoffset != low_linkedit)
		fatal("contents of input file: %s not in an order that the "
		      "specified sections can be replaced by this program",
		      input);
	    for(i = 0; i < nsegs; i++){
		if(segs[i].sgp != NULL){
		    sp = (struct section *)((char *)(segs[i].sgp) +
					    sizeof(struct segment_command));
		    for(j = 0; j < segs[i].sgp->nsects; j++){
			if(sp->nreloc != 0)
			    sp->reloff += newoffset - oldoffset;
			sp++;
		    }
		}
		else{
		    sp64 = (struct section_64 *)((char *)(segs[i].sgp64) +
					    sizeof(struct segment_command_64));
		    for(j = 0; j < segs[i].sgp64->nsects; j++){
			if(sp64->nreloc != 0)
			    sp64->reloff += newoffset - oldoffset;
			sp64++;
		    }
		}
	    }
	    if(stp != NULL){
		if(stp->nsyms != 0)
		    stp->symoff += newoffset - oldoffset;
		if(stp->strsize != 0)
		    stp->stroff += newoffset - oldoffset;
	    }
	    if(ssp != NULL){
		if(ssp->size != 0)
		    ssp->offset += newoffset - oldoffset;
	    }
	    if(linkedit_sgp != NULL){
		linkedit_sgp->fileoff += newoffset - oldoffset;
	    }
	    if(linkedit_sgp64 != NULL){
		linkedit_sgp64->fileoff += newoffset - oldoffset;
	    }
	}
	/*
	 * Now write the new file by writing the header and modified load
	 * commands, then the segments with any new sections and finally
	 * the link edit info.
	 */
	if((outfd = open(output, O_CREAT | O_WRONLY | O_TRUNC ,input_mode)) 
	   == -1)
	    system_fatal("can't create output file: %s", output);

	if((r = vm_allocate(mach_task_self(), &pad_addr, pagesize, 1)) !=
	   KERN_SUCCESS)
	    mach_fatal(r, "vm_allocate() failed");

	k = 0;
	for(i = 0; i < nsegs; i++){
	    if(segs[i].modified){
		if(segs[i].sgp != NULL){
		    for(j = 0; j < segs[i].sgp->nsects; j++){
			/* if the section is replaced write the replaced
			   section */
			sp = sects[k + j].sp;
			rp = sects[k + j].rp;
			if(rp != NULL){
			    if((sectfd = open(rp->filename, O_RDONLY)) == -1)
				system_fatal("can't open file: %s to replace "
					 "section (%s,%s) with", rp->filename,
					 rp->segname, rp->sectname);
			    sect_addr = mmap(0, rp->size, PROT_READ|PROT_WRITE,
					      MAP_FILE|MAP_PRIVATE, sectfd, 0);
			    if((intptr_t)sect_addr == -1)
				system_error("Can't map file: %s",rp->filename);
			    for(l = rp->size + 1; l < sp->size; l++)
				*((char *)sect_addr + l) = '\0';
			    if(write64(outfd, (char *)sect_addr,sp->size) !=
                               sp->size)
				system_fatal("can't write new section contents "
				    "for section (%s,%s) to output file: %s", 
				     rp->segname, rp->sectname, output);
			    if(close(sectfd) == -1)
				system_error("can't close file: %s to replace "
					 "section (%s,%s) with", rp->filename,
					 rp->segname, rp->sectname);
			    if((r = vm_deallocate(mach_task_self(),
						  (vm_address_t)sect_addr,
						  rp->size)) != KERN_SUCCESS)
				mach_fatal(r, "Can't deallocate memory for "
				   "mapped file: %s", rp->filename);
			}
			else{
			    /* write the original section */
			    if(sects[k + j].offset + sp->size > input_size)
				fatal("truncated or malformed object file: %s "
				      "(section (%.16s,%.16s) extends past the "
				      "end of the file)",input, sp->segname,
				      sp->sectname);
			    if(write64(outfd,(char *)input_addr +
                                       sects[k + j].offset,
			               sp->size) != sp->size)
				system_fatal("can't write section contents for "
					 "section (%s,%s) to output file: %s", 
					 rp->segname, rp->sectname, output);
			}
			sp++;
		    }
		    /* write the segment padding */
		    if(write64(outfd, (char *)pad_addr, segs[i].padsize) !=
		       segs[i].padsize)
			system_fatal("can't write segment padding for segment "
			    "%s to output file: %s", segs[i].sgp->segname,
			     output);
		}
		else{
		    for(j = 0; j < segs[i].sgp64->nsects; j++){
			/* if the section is replaced write the replaced
			   section */
			sp64 = sects[k + j].sp64;
			rp = sects[k + j].rp;
			if(rp != NULL){
			    if((sectfd = open(rp->filename, O_RDONLY)) == -1)
				system_fatal("can't open file: %s to replace "
					 "section (%s,%s) with", rp->filename,
					 rp->segname, rp->sectname);
			    sect_addr = mmap(0, rp->size, PROT_READ|PROT_WRITE,
					      MAP_FILE|MAP_PRIVATE, sectfd, 0);
			    if((intptr_t)sect_addr == -1)
				system_error("Can't map file: %s",rp->filename);
			    for(l = rp->size + 1; l < sp64->size; l++)
				*((char *)sect_addr + l) = '\0';
			    if(write64(outfd, (char *)sect_addr,sp64->size) !=
                               sp64->size)
				system_fatal("can't write new section contents "
				    "for section (%s,%s) to output file: %s", 
				     rp->segname, rp->sectname, output);
			    if(close(sectfd) == -1)
				system_error("can't close file: %s to replace "
					 "section (%s,%s) with", rp->filename,
					 rp->segname, rp->sectname);
			    if((r = vm_deallocate(mach_task_self(),
						  (vm_address_t)sect_addr,
						  rp->size)) != KERN_SUCCESS)
				mach_fatal(r, "Can't deallocate memory for "
				   "mapped file: %s", rp->filename);
			}
			else{
			    /* write the original section */
			    if(sects[k + j].offset + sp64->size > input_size)
				fatal("truncated or malformed object file: %s "
				      "(section (%.16s,%.16s) extends past the "
				      "end of the file)",input, sp64->segname,
				      sp64->sectname);
			    if(write64(outfd,(char *)input_addr +
                                       sects[k + j].offset,
			               sp64->size) != sp64->size)
				system_fatal("can't write section contents for "
					 "section (%s,%s) to output file: %s", 
					 rp->segname, rp->sectname, output);
			}
			sp64++;
		    }
		    /* write the segment padding */
		    if(write64(outfd, (char *)pad_addr, segs[i].padsize) !=
		       segs[i].padsize)
			system_fatal("can't write segment padding for segment "
			    "%s to output file: %s", segs[i].sgp64->segname,
			     output);
		}
	    }
	    else{
		/* write the original segment */
		if(segs[i].sgp != NULL){
		    if(strcmp(segs[i].sgp->segname, SEG_LINKEDIT) != 0 ||
		       i != nsegs - 1){
			if(segs[i].fileoff + segs[i].sgp->filesize > input_size)
			    fatal("truncated or malformed object file: %s "
				  "(segment: %s extends past the end of "
				  "the file)", input, segs[i].sgp->segname);
			if(write64(outfd, (char *)input_addr + segs[i].fileoff,
                                   segs[i].sgp->filesize) !=
                           segs[i].sgp->filesize)
			    system_fatal("can't write segment contents for "
					 "segment: %s to output file: %s", 
					 segs[i].sgp->segname, output);
		    }
		}
		else{
		    if(strcmp(segs[i].sgp64->segname, SEG_LINKEDIT) != 0 ||
		       i != nsegs - 1){
			if(segs[i].fileoff + segs[i].sgp64->filesize >
			   input_size)
			    fatal("truncated or malformed object file: %s "
				  "(segment: %s extends past the end of "
				  "the file)", input, segs[i].sgp64->segname);
			if(write64(outfd, (char *)input_addr + segs[i].fileoff,
			           segs[i].sgp64->filesize) !=
                           segs[i].sgp64->filesize)
			    system_fatal("can't write segment contents for "
					 "segment: %s to output file: %s", 
					 segs[i].sgp64->segname, output);
		    }
		}
	    }
	    if(segs[i].sgp != NULL)
		k += segs[i].sgp->nsects;
	    else
		k += segs[i].sgp64->nsects;
	}
	/* write the linkedit info */
	size = input_size - low_linkedit;
	if(write64(outfd, (char *)input_addr + low_linkedit, size) != size)
	    system_fatal("can't write link edit information to output file: %s",
			 output);
	lseek(outfd, 0, L_SET);
	if(mhp != NULL)
	    size = sizeof(struct mach_header) + mhp->sizeofcmds;
	else
	    size = sizeof(struct mach_header_64) + mhp64->sizeofcmds;
	if(swapped){
	    lcp = load_commands;
	    for(i = 0; i < mh_ncmds; i++){
		lc = *lcp;
		switch(lcp->cmd){
		case LC_SEGMENT:
		    sgp = (struct segment_command *)lcp;
		    sp = (struct section *)((char *)sgp +
					    sizeof(struct segment_command));
		    swap_section(sp, sgp->nsects, host_byte_sex);
		    swap_segment_command(sgp, host_byte_sex);
		    break;
		case LC_SEGMENT_64:
		    sgp64 = (struct segment_command_64 *)lcp;
		    sp64 = (struct section_64 *)((char *)sgp64 +
					    sizeof(struct segment_command_64));
		    swap_section_64(sp64, sgp64->nsects, host_byte_sex);
		    swap_segment_command_64(sgp64, host_byte_sex);
		    break;
		case LC_SYMTAB:
		    stp = (struct symtab_command *)lcp;
		    swap_symtab_command(stp, host_byte_sex);
		    break;
		case LC_SYMSEG:
		    ssp = (struct symseg_command *)lcp;
		    swap_symseg_command(ssp, host_byte_sex);
		    break;
		default:
		    swap_load_command(lcp, host_byte_sex);
		    break;
		}
		lcp = (struct load_command *)((char *)lcp + lc.cmdsize);
	    }
	    if(mhp != NULL)
		swap_mach_header(mhp, host_byte_sex);
	    else
		swap_mach_header_64(mhp64, host_byte_sex);
	}
	if(write64(outfd, input_addr, size) != size)
	    system_fatal("can't write headers to output file: %s", output);

	if(close(outfd) == -1)
	    system_fatal("can't close output file: %s", output);
}

static
void
search_for_replace_section(
char *segname,
char *sectname,
uint32_t seg_flags,
uint32_t sect_flags,
uint32_t offset,
uint32_t size)
{
    struct replace *rp;

	rp = replaces;
	while(rp != NULL){
	    if(rp->found == 0 &&
	       strncmp(rp->segname, segname, 16) == 0 &&
	       strncmp(rp->sectname, sectname, 16) == 0){
		if(sect_flags == S_ZEROFILL ||
		   sect_flags == S_THREAD_LOCAL_ZEROFILL){
		    error("can't replace zero fill section (%.16s,"
			  "%.16s) in: %s", segname,
			  sectname, input);
		    errors = 1;
		}
		if((seg_flags & SG_NORELOC) == 0){
		    error("can't replace section (%.16s,%.16s) "
			  "in: %s because it requires relocation",
			  segname, sectname, input);
		    errors = 1;
		}
		if(offset + size > input_size)
		    fatal("truncated or malformed object (section "
			  "contents of (%.16s,%.16s) extends "
			  "past the end of the file) in: %s",
			  segname, sectname, input);
		rp->found = 1;
	    }
	    rp = rp->next;
	}
}

/*
 * Function for qsort for comparing segment's vmaddrs
 */
static
int
cmp_qsort(
const struct rep_seg *seg1,
const struct rep_seg *seg2)
{
	if(seg1->sgp != NULL){
	    if(seg1->sgp->vmaddr > seg2->sgp->vmaddr)
		return(1);
	    if(seg1->sgp->vmaddr < seg2->sgp->vmaddr)
		return(-1);
	    /* seg1->sgp->vmaddr == seg2->sgp->vmaddr */
		return(0);
	}
	else{
	    if(seg1->sgp64->vmaddr > seg2->sgp64->vmaddr)
		return(1);
	    if(seg1->sgp64->vmaddr < seg2->sgp64->vmaddr)
		return(-1);
	    /* seg1->sgp64->vmaddr == seg2->sgp64->vmaddr */
		return(0);
	}
}

/*
 * Print the usage message and exit non-zero.
 */
static
void
usage(void)
{
    /* Just print the basename for the usage. */
    const char* basename = strrchr(progname, '/');
    if (basename) {
	basename += 1;
    } else {
	basename = progname;
    }
    fprintf(stderr,
	    "usage: %s input_file [-extract seg_name sect_name data_file] "
	    "...\n",
	    basename);
    fprintf(stderr,
	    "       %s input_file [-replace seg_name sect_name data_file] "
	    "... -output output_file\n",
	    basename);
    exit(1);
}
