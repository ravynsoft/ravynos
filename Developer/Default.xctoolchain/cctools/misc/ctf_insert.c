/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "stuff/allocate.h"
#include "stuff/errors.h"
#include "stuff/breakout.h"
#include "stuff/rnd.h"

/*
 * The structure that holds the -arch <arch> <file> information from the command
 * line flags.
 */
struct arch_ctf {
    struct arch_flag arch_flag;
    enum bool arch_found;
    char *filename;
    char *contents;
    uint32_t size;
};
struct arch_ctf *arch_ctfs;
uint32_t narch_ctfs = 0;

/* used by error routines as the name of the program */
char *progname = NULL;

static void usage(
    void);

static void process(
    struct arch *archs,
    uint32_t narchs);

static void ctf_insert(
    struct arch *arch,
    struct member *member,
    struct object *object);

static void add_ctf_section(
    struct arch *arch,
    char *arch_name,
    uint32_t offset,
    uint64_t addr,
    uint32_t size);

/* apple_version is created by the libstuff/Makefile */
extern char apple_version[];
char *version = apple_version;

/*
 * The ctf_insert(1) tool has the following usage:
 *
 *	ctf_insert input -arch arch ctf_file ...  -o output
 * 
 * Where the input is a Mach-O file that is the ctf_file(s) are to be inserted
 * into and output is the file to be created.
 */
int
main(
int argc,
char **argv,
char **envp)
{
    uint32_t i;
    char *input, *output, *contents;
    struct arch *archs;
    uint32_t narchs;
    struct stat stat_buf;
    int fd;

	progname = argv[0];
	input = NULL;
	output = NULL;
	archs = NULL;
	narchs = 0;
	for(i = 1; i < argc; i++){
	    if(strcmp(argv[i], "-o") == 0){
		if(i + 1 == argc){
		    error("missing argument to: %s option", argv[i]);
		    usage();
		}
		if(output != NULL){
		    error("more than one: %s option specified", argv[i]);
		    usage();
		}
		output = argv[i+1];
		i++;
	    }
	    else if(strcmp(argv[i], "-arch") == 0){
		if(i + 2 == argc){
		    error("missing argument(s) to: %s option", argv[i]);
		    usage();
		}
		else{
		    arch_ctfs = reallocate(arch_ctfs,
			    (narch_ctfs + 1) * sizeof(struct arch_ctf));
		    if(get_arch_from_flag(argv[i+1],
				  &(arch_ctfs[narch_ctfs].arch_flag)) == 0){
			error("unknown architecture specification flag: "
			      "%s %s %s", argv[i], argv[i+1], argv[i+2]);
			arch_usage();
			usage();
		    }
		    if((fd = open(argv[i+2], O_RDONLY, 0)) == -1)
			system_fatal("can't open file: %s", argv[i+2]);
		    if(fstat(fd, &stat_buf) == -1)
			system_fatal("can't stat file: %s", argv[i+2]);
		    /*
		     * For some reason mapping files with zero size fails
		     * so it has to be handled specially.
		     */
		    contents = NULL;
		    if(stat_buf.st_size != 0){
			contents = mmap(0, stat_buf.st_size,
					PROT_READ|PROT_WRITE,
				        MAP_FILE|MAP_PRIVATE, fd, 0);
			if((intptr_t)contents == -1)
			    system_error("can't map file : %s", argv[i+2]);
		    }
		    arch_ctfs[narch_ctfs].filename = argv[i+2];
		    arch_ctfs[narch_ctfs].contents = contents;
		    arch_ctfs[narch_ctfs].size = (uint32_t)stat_buf.st_size;
		    arch_ctfs[narch_ctfs].arch_found = FALSE;
		    narch_ctfs++;
		    i += 2;
		}
	    }
	    else{
		if(input != NULL){
		    error("more than one input file file: %s specified",
			  input);
		    usage();
		}
		input = argv[i];
	    }
	}
	if(input == NULL || output == NULL || narch_ctfs == 0)
	    usage();

	breakout(input, &archs, &narchs, FALSE);
	if(errors)
	    exit(EXIT_FAILURE);

	checkout(archs, narchs);

	process(archs, narchs);

	for(i = 0; i < narch_ctfs; i++){
	    if(arch_ctfs[i].arch_found == FALSE)
		fatal("input file: %s does not contain a matching architecture "
		      "for specified '-arch %s %s' option", input,
		      arch_ctfs[i].arch_flag.name, arch_ctfs[i].filename);
	}

	writeout(archs, narchs, output, 0777, TRUE, FALSE, FALSE, FALSE, NULL);

	if(errors)
	    return(EXIT_FAILURE);
	else
	    return(EXIT_SUCCESS);
}

/*
 * usage() prints the current usage message and exits indicating failure.
 */
static
void
usage(
void)
{
	fprintf(stderr, "Usage: %s input [-arch <arch> <file>]... -o output\n",
		progname);
	exit(EXIT_FAILURE);
}

/*
 * process() walks the archs and calls ctf_insert() to do the work.
 */
static
void
process(
struct arch *archs,
uint32_t narchs)
{
    uint32_t i;

	for(i = 0; i < narchs; i++){
	    if(archs[i].type == OFILE_Mach_O)
		ctf_insert(archs + i, NULL, archs[i].object);
	    else
		fatal_arch(archs + i, NULL, "file type not valid input for "
			   "this this program to process: ");
	}
}

/*
 * ctf_insert() does the work to add the ctf section in the specified broken
 * out ofile for the architecure specifed with a -arch command line option.
 */
static
void
ctf_insert(
struct arch *arch,
struct member *member,
struct object *object)
{
    uint32_t i, move_size;
    cpu_type_t cputype;
    cpu_subtype_t cpusubtype;
    uint32_t flags, offset;
    uint64_t addr;

	if(object->mh != NULL){
	    cputype = object->mh->cputype;
	    cpusubtype = object->mh->cpusubtype & ~CPU_SUBTYPE_MASK;
	    flags = object->mh->flags;
	    offset = object->seg_linkedit->fileoff;
	    addr = object->seg_linkedit->vmaddr;
	}
	else{
	    cputype = object->mh64->cputype;
	    cpusubtype = object->mh64->cpusubtype & ~CPU_SUBTYPE_MASK;
	    flags = object->mh64->flags;
	    offset = (uint32_t)object->seg_linkedit64->fileoff;
	    addr = object->seg_linkedit64->vmaddr;
	}

	/*
	 * Make sure this object is valid to process.  Since the input should
	 * be a mach_kernel that is statically linked we should not see any
	 * dynamic symbol table info. Or a code signature at the point this
	 * program is called in the build process.
	 */
	if((flags & MH_DYLDLINK) == MH_DYLDLINK ||
	   object->dyld_info != NULL ||
	   object->split_info_cmd != NULL ||
	   object->hints_cmd != NULL)
	     fatal_arch(arch, member, "file is input for the dynamic linker so "
			"not a valid input for this program to process: ");
	/*
	 * Allow a dynamic symbol table load command where it only has an
	 * indirect symbol table and no other tables.
	 */
        if(object->dyst != NULL &&
	   (object->dyst->ntoc != 0 ||
	    object->dyst->nmodtab != 0 ||
	    object->dyst->nmodtab != 0 ||
	    object->dyst->nextrefsyms != 0 ||
	    object->dyst->nextrel != 0))
	     fatal_arch(arch, member, "file is input for the dynamic linker so "
			"not a valid input for this program to process: ");
	if(object->mh_filetype != MH_EXECUTE)
	     fatal_arch(arch, member, "file type is not MH_EXECUTE so "
			"not a valid input for this program to process: ");
	if(object->seg_linkedit == NULL && object->seg_linkedit64 == NULL)
	     fatal_arch(arch, member, "file type does not have a __LINKEDIT "
			"segment so not a valid input for this program to "
			"process: ");
	if(object->code_sig_cmd != NULL)
	     fatal_arch(arch, member, "file type has code signature "
			"so not a valid input for this program to process: ");

	/*
	 * Now see if one of the -arch flags matches this object.
	 */
	for(i = 0; i < narch_ctfs; i++){
	    if(arch_ctfs[i].arch_flag.cputype == cputype &&
	       arch_ctfs[i].arch_flag.cpusubtype == cpusubtype)
		break;
	}
	/*
	 * If we didn't find a matching -arch flag it is an error.
	 */
	if(i >= narch_ctfs){
	     fatal_arch(arch, member, "no matching -arch option for this slice "
			"of file: ");
	     return;
	}
	arch_ctfs[i].arch_found = TRUE;

	/*
	 * Add the section for the ctf data for this arch.  It is placed in
	 * the file where the linkedit info was and that info will then be
	 * moved.
	 */
	add_ctf_section(arch, arch_ctfs[i].arch_flag.name,
			offset, addr, arch_ctfs[i].size);

	/*
	 * Now set up all the pointers and sizes of the symbol and string table.
	 */
	if(object->st != NULL && object->st->nsyms != 0){
	    if(object->mh != NULL){
		object->output_symbols = (struct nlist *)
		    (object->object_addr + object->st->symoff);
		if(object->object_byte_sex != get_host_byte_sex())
		    swap_nlist(object->output_symbols,
			       object->st->nsyms,
			       get_host_byte_sex());
		object->output_symbols64 = NULL;
	    }
	    else{
		object->output_symbols64 = (struct nlist_64 *)
		    (object->object_addr + object->st->symoff);
		if(object->object_byte_sex != get_host_byte_sex())
		    swap_nlist_64(object->output_symbols64,
				  object->st->nsyms,
				  get_host_byte_sex());
		object->output_symbols = NULL;
	    }
	    object->output_nsymbols = object->st->nsyms;
	    object->output_strings =
		object->object_addr + object->st->stroff;
	    object->output_strings_size = object->st->strsize;
	    if(object->mh != NULL){
		object->input_sym_info_size =
		    object->st->nsyms * sizeof(struct nlist) +
		    object->st->strsize;
	    }
	    else{
		object->input_sym_info_size =
		    object->st->nsyms * sizeof(struct nlist_64) +
		    object->st->strsize;
	    }
	}
	if(object->dyst != NULL){
	    object->output_ilocalsym = object->dyst->ilocalsym;
	    object->output_nlocalsym = object->dyst->nlocalsym;
	    object->output_iextdefsym = object->dyst->iextdefsym;
	    object->output_nextdefsym = object->dyst->nextdefsym;
	    object->output_iundefsym = object->dyst->iundefsym;
	    object->output_nundefsym = object->dyst->nundefsym;
	    object->output_indirect_symtab = (uint32_t *)
		(object->object_addr + object->dyst->indirectsymoff);
	    object->output_loc_relocs = (struct relocation_info *)
		(object->object_addr + object->dyst->locreloff);
	    if(object->mh != NULL){
		object->input_sym_info_size +=
		    object->dyst->nindirectsyms *
			sizeof(uint32_t);
	    }
	    else{
		object->input_sym_info_size +=
		    object->dyst->nindirectsyms *
			sizeof(uint32_t) +
		    object->input_indirectsym_pad;
	    }
	    object->input_sym_info_size +=
		object->dyst->nlocrel *
		    sizeof(struct relocation_info);
	}
	if(object->func_starts_info_cmd != NULL){
	    object->output_func_start_info_data = object->object_addr +
		object->func_starts_info_cmd->dataoff;
	    object->output_func_start_info_data_size = 
		object->func_starts_info_cmd->datasize;
	    object->input_sym_info_size +=
		object->func_starts_info_cmd->datasize;
	}
	if(object->data_in_code_cmd != NULL){
	    object->output_data_in_code_info_data = object->object_addr +
		object->data_in_code_cmd->dataoff;
	    object->output_data_in_code_info_data_size = 
		object->data_in_code_cmd->datasize;
	    object->input_sym_info_size +=
		object->data_in_code_cmd->datasize;
	}
	if(object->code_sign_drs_cmd != NULL){
	    object->output_code_sign_drs_info_data = object->object_addr +
		object->code_sign_drs_cmd->dataoff;
	    object->output_code_sign_drs_info_data_size = 
		object->code_sign_drs_cmd->datasize;
	    object->input_sym_info_size +=
		object->code_sign_drs_cmd->datasize;
	}
	if(object->link_opt_hint_cmd != NULL){
	    object->output_link_opt_hint_info_data = object->object_addr +
		object->link_opt_hint_cmd->dataoff;
	    object->output_link_opt_hint_info_data_size = 
		object->link_opt_hint_cmd->datasize;
	    object->input_sym_info_size +=
		object->link_opt_hint_cmd->datasize;
	}
	object->output_sym_info_size = object->input_sym_info_size;

	/*
	 * Now move the link edit info by the size of the ctf for this arch
	 * rounded to the load command size for this arch.
	 */
	if(object->mh != NULL){
	    move_size = rnd32(arch_ctfs[i].size, sizeof(uint32_t));
	    object->seg_linkedit->fileoff += move_size;
	}
	else{
	    move_size = rnd32(arch_ctfs[i].size, sizeof(uint64_t));
	    object->seg_linkedit64->fileoff += move_size;
	}
	if(object->st != NULL && object->st->nsyms != 0){
	    object->st->symoff += move_size;
	    object->st->stroff += move_size;
	}
	if(object->dyst != NULL){
	    if(object->dyst->nindirectsyms != 0)
	        object->dyst->indirectsymoff += move_size;
	    if(object->dyst->nlocrel != 0)
		object->dyst->locreloff += move_size;
	}
	if(object->func_starts_info_cmd != NULL)
	    object->func_starts_info_cmd->dataoff += move_size;
	if(object->data_in_code_cmd != NULL)
	    object->data_in_code_cmd->dataoff += move_size;
	if(object->code_sign_drs_cmd != NULL)
	    object->code_sign_drs_cmd->dataoff += move_size;
	if(object->link_opt_hint_cmd != NULL)
	    object->link_opt_hint_cmd->dataoff += move_size;

	/*
	 * Record the new content for writeout() to put in to the output file.
	 */
	object->output_new_content = arch_ctfs[i].contents;
	object->output_new_content_size = move_size;
}

/*
 * add_ctf_section() sees if there is space to add an LC_SEGMENT load command
 * and one section stucture in the padding after the headers for the specified
 * arch and arch_name.  If so it adds a segment load command and section struct
 * filled in with the offset, size and addr fields.  If it can't be added or
 * one already exists a fatal error message is printed.
 */
static
void
add_ctf_section(
struct arch *arch,
char *arch_name,
uint32_t offset,
uint64_t addr,
uint32_t size)
{
    uint32_t i, j, low_fileoff, mach_header_size, added_header_size;
    uint32_t ncmds, sizeofcmds;
    struct load_command *lc;
    struct segment_command *sg;
    struct segment_command_64 *sg64;
    struct section *s;
    struct section_64 *s64;
    struct segment_command *sg_CTF;
    struct section *s_ctf;
    struct segment_command_64 *sg64_CTF;
    struct section_64 *s64_ctf;

        if(arch->object->mh != NULL){
            ncmds = arch->object->mh->ncmds;
	    sizeofcmds = arch->object->mh->sizeofcmds;
	    mach_header_size = sizeof(struct mach_header);
	    added_header_size = sizeof(struct segment_command) +
				sizeof(struct section);
	}
	else{
            ncmds = arch->object->mh64->ncmds;
	    sizeofcmds = arch->object->mh64->sizeofcmds;
	    mach_header_size = sizeof(struct mach_header_64);
	    added_header_size = sizeof(struct segment_command_64) +
				sizeof(struct section_64);
	}

	/*
	 * The size of the new load commands that includes the added segment
	 * load command and section structure is larger than the existing load
	 * commands, so see if they can be fitted in before the contents of the
	 * first section (or segment in the case of a LINKEDIT segment only
	 * file).
	 */
	low_fileoff = UINT_MAX;
	lc = arch->object->load_commands;
	for(i = 0; i < ncmds; i++){
	    if(lc->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc;
		if(strcmp(sg->segname, "__CTF") == 0)
		    fatal("can't insert __CTF segment for: %s (for "
			  "architecture %s) because it already contains "
			  "this segment", arch->file_name, arch_name);
		s = (struct section *)
		    ((char *)sg + sizeof(struct segment_command));
		if(sg->nsects != 0){
		    for(j = 0; j < sg->nsects; j++){
			if(s->size != 0 &&
			(s->flags & S_ZEROFILL) != S_ZEROFILL &&
			(s->flags & S_THREAD_LOCAL_ZEROFILL) !=
				    S_THREAD_LOCAL_ZEROFILL &&
			s->offset < low_fileoff)
			    low_fileoff = s->offset;
			s++;
		    }
		}
		else{
		    if(sg->filesize != 0 && sg->fileoff < low_fileoff)
			low_fileoff = sg->fileoff;
		}
	    }
	    else if(lc->cmd == LC_SEGMENT_64){
		sg64 = (struct segment_command_64 *)lc;
		if(strcmp(sg64->segname, "__CTF") == 0)
		    fatal("can't insert __CTF segment for: %s (for "
			  "architecture %s) because it already contains "
			  "this segment", arch->file_name, arch_name);
		s64 = (struct section_64 *)
		    ((char *)sg64 + sizeof(struct segment_command_64));
		if(sg64->nsects != 0){
		    for(j = 0; j < sg64->nsects; j++){
			if(s64->size != 0 &&
			(s64->flags & S_ZEROFILL) != S_ZEROFILL &&
			(s64->flags & S_THREAD_LOCAL_ZEROFILL) !=
				      S_THREAD_LOCAL_ZEROFILL &&
			s64->offset < low_fileoff)
			    low_fileoff = s64->offset;
			s64++;
		    }
		}
		else{
		    if(sg64->filesize != 0 && sg64->fileoff < low_fileoff)
			low_fileoff = (uint32_t)sg64->fileoff;
		}
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	if(sizeofcmds + mach_header_size + added_header_size > low_fileoff)
{
printf("added_header_size = %d\n", added_header_size);
printf("space available = %d\n", low_fileoff - (sizeofcmds + mach_header_size));
	    fatal("can't insert (__CTF,__ctf) section for: %s (for architecture"
		  " %s) because larger updated load commands do not fit (the "
		  "program must be relinked using a larger -headerpad value)", 
		  arch->file_name, arch_name);
}
	/*
	 * There is space for the new load command. So just use that space for
	 * the new segment and section and set the fields.
	 */
        if(arch->object->mh != NULL){
	    sg_CTF = (struct segment_command *)
		     ((char *)arch->object->load_commands + sizeofcmds);
	    memset(sg_CTF, '\0', sizeof(struct segment_command));
	    sg_CTF->cmd = LC_SEGMENT;
	    sg_CTF->cmdsize = sizeof(struct segment_command) +
			      sizeof(struct section);
	    strcpy(sg_CTF->segname, "__CTF");
	    sg_CTF->vmaddr = (uint32_t)addr;
	    sg_CTF->vmsize = 0;
	    sg_CTF->fileoff = offset;
	    sg_CTF->filesize = size;
	    sg_CTF->maxprot = VM_PROT_READ;
	    sg_CTF->initprot = VM_PROT_READ;
	    sg_CTF->nsects = 1;
	    sg_CTF->flags = 0;
	    s_ctf = (struct section *)
		    ((char *)sg_CTF + sizeof(struct segment_command));
	    memset(s_ctf, '\0', sizeof(struct section));
	    strcpy(s_ctf->sectname, "__ctf");
	    strcpy(s_ctf->segname, "__CTF");
	    s_ctf->addr = (uint32_t)addr;
	    s_ctf->size = size;
	    s_ctf->offset = offset;
	    s_ctf->align = 0;
	    s_ctf->reloff = 0;
	    s_ctf->nreloc = 0;
	    s_ctf->flags = S_REGULAR;
	    s_ctf->reserved1 = 0;
	    s_ctf->reserved2 = 0;
            arch->object->mh->sizeofcmds = sizeofcmds +
					   sizeof(struct segment_command) +
					   sizeof(struct section);
            arch->object->mh->ncmds = ncmds + 1;
        }
	else{
	    sg64_CTF = (struct segment_command_64 *)
		   ((char *)arch->object->load_commands + sizeofcmds);
	    memset(sg64_CTF, '\0', sizeof(struct segment_command_64));
	    sg64_CTF->cmd = LC_SEGMENT_64;
	    sg64_CTF->cmdsize = sizeof(struct segment_command_64) +
				sizeof(struct section_64);
	    strcpy(sg64_CTF->segname, "__CTF");
	    sg64_CTF->vmaddr = addr;
	    sg64_CTF->vmsize = 0;
	    sg64_CTF->fileoff = offset;
	    sg64_CTF->filesize = size;
	    sg64_CTF->maxprot = VM_PROT_READ;
	    sg64_CTF->initprot = VM_PROT_READ;
	    sg64_CTF->nsects = 1;
	    sg64_CTF->flags = 0;
	    s64_ctf = (struct section_64 *)
		  ((char *)sg64_CTF + sizeof(struct segment_command_64));
	    memset(s64_ctf, '\0', sizeof(struct section_64));
	    strcpy(s64_ctf->sectname, "__ctf");
	    strcpy(s64_ctf->segname, "__CTF");
	    s64_ctf->addr = addr;
	    s64_ctf->size = size;
	    s64_ctf->offset = offset;
	    s64_ctf->align = 0;
	    s64_ctf->reloff = 0;
	    s64_ctf->nreloc = 0;
	    s64_ctf->flags = S_REGULAR;
	    s64_ctf->reserved1 = 0;
	    s64_ctf->reserved2 = 0;
            arch->object->mh64->sizeofcmds = sizeofcmds +
					     sizeof(struct segment_command_64) +
					     sizeof(struct section_64);
            arch->object->mh64->ncmds = ncmds + 1;
        }
}
