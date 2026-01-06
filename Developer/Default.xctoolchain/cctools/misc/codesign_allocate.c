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
#include "stuff/errors.h"
#include "stuff/breakout.h"
#include "stuff/rnd.h"
#include "stuff/allocate.h"

/*
 * The structure that holds the -a <arch> <size> information from the command
 * line flags.
 */
struct arch_sign {
    struct arch_flag arch_flag;
    uint32_t datasize;
    enum bool found;
};
struct arch_sign *arch_signs;
uint32_t narch_signs = 0;

enum bool rflag = FALSE;
enum bool pflag = FALSE;

/* used by error routines as the name of the program */
char *progname = NULL;

static void usage(
    void);

static void process(
    struct arch *archs,
    uint32_t narchs);

static void setup_code_signature(
    struct arch *arch,
    struct member *member,
    struct object *object);

static struct linkedit_data_command *add_code_sig_load_command(
    struct arch *arch,
    char *arch_name,
    struct member *member,
    struct object *object);

static void strip_LC_CODE_SIGNATURE_commands(
    struct arch *arch,
    struct member *member,
    struct object *object);

/* apple_version is created by the libstuff/Makefile */
extern char apple_version[];
char *version = apple_version;

static enum bool cs_alloc_debug;

/*
 * The codesign_allocate(1) tool has the following usage:
 *
 *	codesign_allocate -i oldfile [-a arch size ...] | [-r] -o newfile
 * 
 * Where the oldfile is a Mach-O file that is input for the dynamic linker
 * and it creates or adds an 
 */
int
main(
int argc,
char **argv,
char **envp)
{
    uint32_t i;
    char *input, *output, *endp;
    struct arch *archs;
    uint32_t narchs;

        /* if CS_ALLOC_DEBUG is set print the complete arguments to stderr */
        cs_alloc_debug = (NULL != getenv("CS_ALLOC_DEBUG"));
        if (cs_alloc_debug) {
            for (i = 0; i < argc; ++i) {
                if (i < (argc-1)) {
                    fprintf(stderr, "%s ", argv[i]);
                } else {
                    fprintf(stderr, "%s\n", argv[i]);
                }
            }
        }

	progname = argv[0];
	input = NULL;
	output = NULL;
	archs = NULL;
	narchs = 0;
	/*
	 * If this is being run via the symbolic link named codesign_allocate-p
	 * then set the pflag.
	 */
	i = (uint32_t)strlen(argv[0]);
	if(i >= sizeof("codesign_allocate-p") - 1 &&
	   strcmp(argv[0] + i-2, "-p") == 0)
	    pflag = TRUE;
	for(i = 1; i < argc; i++){
	    if(strcmp(argv[i], "-i") == 0){
		if(i + 1 == argc){
		    error("missing argument to: %s option", argv[i]);
		    usage();
		}
		if(input != NULL){
		    error("more than one: %s option specified", argv[i]);
		    usage();
		}
		input = argv[i+1];
		i++;
	    }
	    else if(strcmp(argv[i], "-o") == 0){
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
	    else if(strcmp(argv[i], "-a") == 0){
		if(i + 2 == argc){
		    error("missing argument(s) to: %s option", argv[i]);
		    usage();
		}
		else{
		    arch_signs = reallocate(arch_signs,
			    (narch_signs + 1) * sizeof(struct arch_sign));
		    if(get_arch_from_flag(argv[i+1],
				  &(arch_signs[narch_signs].arch_flag)) == 0){
			error("unknown architecture specification flag: "
			      "%s %s %s", argv[i], argv[i+1], argv[i+2]);
			arch_usage();
			usage();
		    }
		    arch_signs[narch_signs].datasize =
			(uint32_t)strtoul(argv[i+2], &endp, 0);
		    if(*endp != '\0')
			fatal("size for '-a %s %s' not a proper number",
			      argv[i+1], argv[i+2]);
		    if((arch_signs[narch_signs].datasize % 16) != 0)
			fatal("size for '-a %s %s' not a multiple of 16",
			      argv[i+1], argv[i+2]);
		    arch_signs[narch_signs].found = FALSE;
		    narch_signs++;
		    i += 2;
		}
	    }
	    else if(strcmp(argv[i], "-A") == 0){
		if(i + 3 == argc){
		    error("missing argument(s) to: %s option", argv[i]);
		    usage();
		}
		else{
		    arch_signs = reallocate(arch_signs,
			    (narch_signs + 1) * sizeof(struct arch_sign));

		    arch_signs[narch_signs].arch_flag.cputype = 
			(uint32_t)strtoul(argv[i+1], &endp, 0);
		    if(*endp != '\0')
			fatal("cputype for '-A %s %s %s' not a proper number",
			      argv[i+1], argv[i+2], argv[i+3]);

		    arch_signs[narch_signs].arch_flag.cpusubtype = 
			(uint32_t)strtoul(argv[i+2], &endp, 0);
		    if(*endp != '\0')
			fatal("cpusubtype for '-A %s %s %s' not a proper "
			      "number", argv[i+1], argv[i+2], argv[i+3]);

		    arch_signs[narch_signs].arch_flag.name = (char *)
			get_arch_name_from_types(
			    arch_signs[narch_signs].arch_flag.cputype,
			    arch_signs[narch_signs].arch_flag.cpusubtype);

		    arch_signs[narch_signs].datasize =
			(uint32_t)strtoul(argv[i+3], &endp, 0);
		    if(*endp != '\0')
			fatal("size for '-A %s %s %s' not a proper number",
			      argv[i+1], argv[i+2], argv[i+3]);
		    if((arch_signs[narch_signs].datasize % 16) != 0)
			fatal("size for '-A %s %s %s' not a multiple of 16",
			      argv[i+1], argv[i+2], argv[i+3]);

		    arch_signs[narch_signs].found = FALSE;
		    narch_signs++;
		    i += 3;
		}
	    }
	    else if(strcmp(argv[i], "-r") == 0){
		rflag = TRUE;
	    }
	    else if(strcmp(argv[i], "-p") == 0){
		pflag = TRUE;
	    }
	    else{
		error("unknown flag: %s", argv[i]);
		usage();
	    }
	}
	if(input == NULL || output == NULL ||
	   (narch_signs == 0 && rflag == FALSE))
	    usage();
	if(rflag && narch_signs != 0){
	    error("-r flag can't be specified with -a or -A flags");
	    usage();
	}

	breakout(input, &archs, &narchs, FALSE);
	if(errors)
	    exit(EXIT_FAILURE);

	checkout(archs, narchs);

	process(archs, narchs);

	for(i = 0; i < narch_signs; i++){
	    if(arch_signs[i].found == FALSE)
		fatal("input file: %s does not contain a matching architecture "
		      "for specified '-a %s %u' option", input,
		      arch_signs[i].arch_flag.name, arch_signs[i].datasize);
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
	fprintf(stderr, "Usage: %s -i input [[-a <arch> <size>]... "
		"[-A <cputype> <cpusubtype> <size>]... | -r] [-p] -o output\n",
		progname);
	exit(EXIT_FAILURE);
}

/*
 * process() walks the archs and calls setup_code_signature() to do the
 * work.
 */
static
void
process(
struct arch *archs,
uint32_t narchs)
{
    uint32_t i, j, offset, size;
    struct ar_hdr h;
    char size_buf[sizeof(h.ar_size) + 1];

	for(i = 0; i < narchs; i++){
	    /*
	     * Given that code signing is "meta" information about the file and 
	     * so does not really alter the "content" of the Mach-o file.
	     * codesign_allocate should never update the LC_ID_DYLIB timestamp.
	     */
	    archs[i].dont_update_LC_ID_DYLIB_timestamp = TRUE;

	    if(archs[i].type == OFILE_ARCHIVE){
		for(j = 0; j < archs[i].nmembers; j++){
		    if(archs[i].members[j].type == OFILE_Mach_O){
			setup_code_signature(archs + i, archs[i].members + j,
					     archs[i].members[j].object);
		    }
		}
		/*
		 * Reset the library offsets and size.
		 */
		offset = 0;
		for(j = 0; j < archs[i].nmembers; j++){
		    archs[i].members[j].offset = offset;
		    size = 0;
		    if(archs[i].members[j].member_long_name == TRUE){
			size = rnd32(archs[i].members[j].member_name_size,
				     sizeof(long));
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
		setup_code_signature(archs + i, NULL, archs[i].object);
	    }
	}
}

/*
 * setup_code_signature() does the work to add or update (or remove) the needed
 * LC_CODE_SIGNATURE load command for the specified broken out ofile if it
 * is of one of the architecures specifed with a -a command line options.  Or
 * removes the LC_CODE_SIGNATURE load command if the -r flag is specified.
 */
static
void
setup_code_signature(
struct arch *arch,
struct member *member,
struct object *object)
{
    uint32_t i, filetype;
    cpu_type_t cputype;
    cpu_subtype_t cpusubtype;
    uint32_t flags, linkedit_end, input_sym_info_size_before_code_sig_rnd;
    uint32_t dyld_info_start;
    uint32_t dyld_info_end;
    uint32_t align_delta, old_align_delta;
    uint32_t dataoff, rnd_dataoff;

	linkedit_end = 0;
	input_sym_info_size_before_code_sig_rnd = 0;
	if(object->mh != NULL){
	    cputype = object->mh->cputype;
	    cpusubtype = object->mh->cpusubtype & ~CPU_SUBTYPE_MASK;
	    filetype = object->mh->filetype;
	    flags = object->mh->flags;
	}
	else{
	    cputype = object->mh64->cputype;
	    cpusubtype = object->mh64->cpusubtype & ~CPU_SUBTYPE_MASK;
	    filetype = object->mh64->filetype;
	    flags = object->mh64->flags;
	}
	/*
	 * First set up all the pointers and sizes of the symbolic info.
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
	else if(object->st != NULL && object->st->strsize != 0){
	    object->output_strings =
		object->object_addr + object->st->stroff;
	    object->output_strings_size = object->st->strsize;
	    object->input_sym_info_size = object->st->strsize;
	}
	if(object->dyld_info != NULL){
	    /* there are five parts to the dyld info, but
	     codesign_allocate does not alter them, so copy as a block */
	    dyld_info_start = 0;
	    if (object->dyld_info->rebase_off != 0)
		dyld_info_start = object->dyld_info->rebase_off;
	    else if (object->dyld_info->bind_off != 0)
		dyld_info_start = object->dyld_info->bind_off;
	    else if (object->dyld_info->weak_bind_off != 0)
		dyld_info_start = object->dyld_info->weak_bind_off;
	    else if (object->dyld_info->lazy_bind_off != 0)
		dyld_info_start = object->dyld_info->lazy_bind_off;
	    else if (object->dyld_info->export_off != 0)
		dyld_info_start = object->dyld_info->export_off;
	    dyld_info_end = 0;
	    if (object->dyld_info->export_size != 0)
		dyld_info_end = object->dyld_info->export_off
		    + object->dyld_info->export_size;
	    else if (object->dyld_info->lazy_bind_size != 0)
		dyld_info_end = object->dyld_info->lazy_bind_off
		    + object->dyld_info->lazy_bind_size;
	    else if (object->dyld_info->weak_bind_size != 0)
		dyld_info_end = object->dyld_info->weak_bind_off
		    + object->dyld_info->weak_bind_size;
	    else if (object->dyld_info->bind_size != 0)
		dyld_info_end = object->dyld_info->bind_off
		    + object->dyld_info->bind_size;
	    else if (object->dyld_info->rebase_size != 0)
		dyld_info_end = object->dyld_info->rebase_off
		    + object->dyld_info->rebase_size;
	    object->output_dyld_info = object->object_addr + dyld_info_start; 
	    object->output_dyld_info_size = dyld_info_end - dyld_info_start;
	    object->output_sym_info_size += object->output_dyld_info_size;
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
	    if(object->dyld_chained_fixups != NULL){
		object->output_dyld_chained_fixups_data =
		(object->object_addr + object->dyld_chained_fixups->dataoff);
		object->output_dyld_chained_fixups_data_size =
		    object->dyld_chained_fixups->datasize;
	    }
	    if(object->dyld_exports_trie != NULL){
		object->output_dyld_exports_trie_data =
		(object->object_addr + object->dyld_exports_trie->dataoff);
		object->output_dyld_exports_trie_data_size =
		    object->dyld_exports_trie->datasize;
	    }
	    object->output_ext_relocs = (struct relocation_info *)
		(object->object_addr + object->dyst->extreloff);
	    object->output_tocs =
		(struct dylib_table_of_contents *)
		(object->object_addr + object->dyst->tocoff);
	    object->output_ntoc = object->dyst->ntoc;
	    if(object->mh != NULL){
		object->output_mods = (struct dylib_module *)
		    (object->object_addr + object->dyst->modtaboff);
		object->output_mods64 = NULL;
	    }
	    else{
		object->output_mods64 = (struct dylib_module_64 *)
		    (object->object_addr + object->dyst->modtaboff);
		object->output_mods = NULL;
	    }
	    object->output_nmodtab = object->dyst->nmodtab;
	    object->output_refs = (struct dylib_reference *)
		(object->object_addr + object->dyst->extrefsymoff);
	    object->output_nextrefsyms = object->dyst->nextrefsyms;
	    if(object->hints_cmd != NULL){
		object->output_hints = (struct twolevel_hint *)
		    (object->object_addr +
		     object->hints_cmd->offset);
	    }
	    if(object->dyld_info != NULL){
		object->input_sym_info_size += object->dyld_info->rebase_size
					    + object->dyld_info->bind_size
					    + object->dyld_info->weak_bind_size
					    + object->dyld_info->lazy_bind_size
					    + object->dyld_info->export_size;
	    }
	    object->input_sym_info_size +=
		object->dyst->nlocrel *
		    sizeof(struct relocation_info) +
		object->dyst->nextrel *
		    sizeof(struct relocation_info) +
		object->dyst->ntoc *
		    sizeof(struct dylib_table_of_contents)+
		object->dyst->nextrefsyms *
		    sizeof(struct dylib_reference);
	    if(object->split_info_cmd != NULL)
		object->input_sym_info_size += object->split_info_cmd->datasize;
	    if(object->func_starts_info_cmd != NULL)
		object->input_sym_info_size +=
		    object->func_starts_info_cmd->datasize;
	    if(object->data_in_code_cmd != NULL)
		object->input_sym_info_size +=
		    object->data_in_code_cmd->datasize;
	    if(object->code_sign_drs_cmd != NULL)
		object->input_sym_info_size +=
		    object->code_sign_drs_cmd->datasize;
	    if(object->link_opt_hint_cmd != NULL)
		object->input_sym_info_size +=
		    object->link_opt_hint_cmd->datasize;
	    if(object->dyld_chained_fixups != NULL)
		object->input_sym_info_size +=
		    object->dyld_chained_fixups->datasize;
	    if(object->dyld_exports_trie != NULL)
		object->input_sym_info_size +=
		    object->dyld_exports_trie->datasize;
	    if(object->mh != NULL){
		object->input_sym_info_size +=
		    object->dyst->nmodtab *
			sizeof(struct dylib_module) +
		    object->dyst->nindirectsyms *
			sizeof(uint32_t);
	    }
	    else{
		object->input_sym_info_size +=
		    object->dyst->nmodtab *
			sizeof(struct dylib_module_64) +
		    object->dyst->nindirectsyms *
			sizeof(uint32_t) +
		    object->input_indirectsym_pad;
	    }
	    if(object->hints_cmd != NULL){
		object->input_sym_info_size +=
		    object->hints_cmd->nhints *
		    sizeof(struct twolevel_hint);
	    }
	}
	object->output_sym_info_size = object->input_sym_info_size;
	input_sym_info_size_before_code_sig_rnd = object->input_sym_info_size;
	if(object->code_sig_cmd != NULL){
	    /*
	     * An MH_OBJECT filetype will not have any padding to get the offset
	     * of an existing code signture to a 16 byte boundary.  To get the
	     * offset to a specific byte boundary in an MH_OBJECT file the
	     * string table size before it is adjusted and there is no padding.
	     */
	    if(filetype != MH_OBJECT)
		object->input_sym_info_size = rnd32(object->input_sym_info_size,
						    16);
	    object->input_sym_info_size += object->code_sig_cmd->datasize;
	}

	/*
	 * Now see if one of the -a flags matches this object.
	 */
	for(i = 0; i < narch_signs; i++){
	    if(arch_signs[i].arch_flag.cputype == cputype &&
	       arch_signs[i].arch_flag.cpusubtype == cpusubtype)
		break;
	}

	/*
	 * If the -r flag is specified we remove the code signature and the
	 * LC_CODE_SIGNATURE load command if any.  We also do this if we have
	 * an -a flag for this arch and the size is zero.
	 */
	if(rflag || (i < narch_signs && arch_signs[i].datasize == 0)){
	    struct arch_flag arch_flag;
	    const char *arch_name;
	    arch_name = get_arch_name_from_types(cputype, cpusubtype);
	    (void)get_arch_from_flag((char *)arch_name, &arch_flag);
	    if(i < narch_signs)
		arch_signs[i].found = TRUE;
	    /*
	     * If this has a code signature load command reduce the linkedit by
	     * the size of that data and the old alignment.  But do not use the
	     * old data.
	     */
	    if(object->code_sig_cmd != NULL){
		dataoff = object->object_size - object->input_sym_info_size +
			  input_sym_info_size_before_code_sig_rnd;

		old_align_delta = object->code_sig_cmd->dataoff - dataoff;
		if(object->seg_linkedit != NULL){
		    object->seg_linkedit->filesize -=
			(old_align_delta + object->code_sig_cmd->datasize);
		    if(object->seg_linkedit->filesize >
		       object->seg_linkedit->vmsize)
			object->seg_linkedit->vmsize =
			    rnd32(object->seg_linkedit->filesize,
		                get_segalign_from_flag(&arch_flag));
		}
		else if(object->seg_linkedit64 != NULL){
		    object->seg_linkedit64->filesize -=
			old_align_delta;
		    object->seg_linkedit64->filesize -=
			object->code_sig_cmd->datasize;
		    if(object->seg_linkedit64->filesize >
		       object->seg_linkedit64->vmsize)
			object->seg_linkedit64->vmsize =
			    rnd(object->seg_linkedit64->filesize,
				get_segalign_from_flag(&arch_flag));
		}
	    }
	    object->output_code_sig_data_size = 0;
	    object->output_code_sig_data = NULL;
	    strip_LC_CODE_SIGNATURE_commands(arch, member, object);
	    return;
	}

	/*
	 * If we didn't find a matching -a flag then just use the existing
	 * code signature if any.
	 */
	if(i >= narch_signs){
	    if(object->code_sig_cmd != NULL){
		object->output_code_sig_data_size =
		    object->code_sig_cmd->datasize;
	    }
	    object->output_sym_info_size = object->input_sym_info_size;
	    return;
	}

	/*
	 * We did find a matching -a flag for this object
	 */
	arch_signs[i].found = TRUE;

	/*
	 * We now allow statically linked objects as well as objects that are
	 * input for the dynamic linker or an MH_OBJECT filetypes to have
	 * code signatures.  So no checks are done here anymore based on the
	 * flags or filetype in the mach_header.
	 */
	
	/*
	 * If this has a code signature load command reuse it and just change
	 * the size of that data.  But do not use the old data.
	 */
	if(object->code_sig_cmd != NULL){
	    /*
	     * To get the code signature data to be page aligned we have to
	     * determine the old padding incase it was just 16 byte aligned.
	     * Then determine the new padding alignment and change the string
	     * table size to add the new padding.
	     *
	     * If there is no string table then we just use the existing
	     * alignment of the old code signature data. 
	     */
	    old_align_delta = 0;
	    align_delta = 0;
	    if(object->st != NULL) {
		dataoff = object->object_size - object->input_sym_info_size +
			  input_sym_info_size_before_code_sig_rnd;

		old_align_delta = object->code_sig_cmd->dataoff - dataoff;
			
		if(pflag)
		    rnd_dataoff = rnd32(dataoff,
			    get_segalign_from_flag(&arch_signs[i].arch_flag));
		else
		    rnd_dataoff = rnd32(dataoff, 16);
		align_delta = rnd_dataoff - dataoff;

		object->code_sig_cmd->dataoff = rnd_dataoff;
		if(pflag) {
		    object->output_strings_size_pad = align_delta;
		    object->st->strsize += object->output_strings_size_pad;
		}
	    }
	    if(object->seg_linkedit != NULL){
		object->seg_linkedit->filesize +=
		    align_delta + arch_signs[i].datasize
		    - (old_align_delta + object->code_sig_cmd->datasize);
		if(object->seg_linkedit->filesize >
		   object->seg_linkedit->vmsize)
		    object->seg_linkedit->vmsize =
			rnd32(object->seg_linkedit->filesize,
			      get_segalign_from_flag(&arch_signs[i].arch_flag));
	    }
	    else if(object->seg_linkedit64 != NULL){
		object->seg_linkedit64->filesize -=
		    old_align_delta;
		object->seg_linkedit64->filesize -=
		    object->code_sig_cmd->datasize;
		object->seg_linkedit64->filesize +=
		    align_delta;
		object->seg_linkedit64->filesize +=
		    arch_signs[i].datasize;
		if(object->seg_linkedit64->filesize >
		   object->seg_linkedit64->vmsize)
		    object->seg_linkedit64->vmsize =
			rnd(object->seg_linkedit64->filesize,
			      get_segalign_from_flag(&arch_signs[i].arch_flag));
	    }

	    object->code_sig_cmd->datasize = arch_signs[i].datasize;
	    object->output_code_sig_data_size = arch_signs[i].datasize;
	    object->output_code_sig_data = NULL;

	    object->output_sym_info_size += align_delta +
					    object->code_sig_cmd->datasize;
	}
	/*
	 * The object does not have a code signature load command we add one.
	 * And if that does not fail we then set the new load command's size and
	 * offset of the code signature data to allocate in the object.  We also
	 * adjust the linkedit's segment size.
	 */
	else{
	    object->code_sig_cmd = add_code_sig_load_command(arch,
						arch_signs[i].arch_flag.name,
						member, object);
	    object->code_sig_cmd->datasize = arch_signs[i].datasize;
	    if(object->seg_linkedit != NULL)
		linkedit_end = object->seg_linkedit->fileoff +
			       object->seg_linkedit->filesize;
	    else if(object->seg_linkedit64 != NULL)
		linkedit_end = (uint32_t)(object->seg_linkedit64->fileoff +
					  object->seg_linkedit64->filesize);
	    else if(object->mh_filetype == MH_OBJECT)
		linkedit_end = object->object_size;
	    else
		fatal("can't allocate code signature data for: %s (for "
		      "architecture %s) because file does not have a "
		      SEG_LINKEDIT " segment", arch->file_name,
		      arch_signs[i].arch_flag.name);

	    /*
	     * If we have a string table and we are padding it so the code
	     * signature is page aligned determine the string table padding and
	     * adjust the string table size.  If not just pad to the older 16
	     * byte alignment.
	     */
	    if(object->st != NULL && pflag) {
		object->code_sig_cmd->dataoff = rnd32(linkedit_end,
			      get_segalign_from_flag(&arch_signs[i].arch_flag));
		object->output_strings_size_pad =
		    object->code_sig_cmd->dataoff - linkedit_end;
		object->st->strsize += object->output_strings_size_pad;
	    }
	    else {
		object->code_sig_cmd->dataoff = rnd32(linkedit_end, 16);
	    }
	    align_delta = object->code_sig_cmd->dataoff - linkedit_end;
	    object->output_code_sig_data_size = arch_signs[i].datasize;
	    object->output_code_sig_data = NULL;

	    object->output_sym_info_size = align_delta +
					   object->output_sym_info_size;
	    object->output_sym_info_size += object->code_sig_cmd->datasize;

	    if(object->seg_linkedit != NULL){
		object->seg_linkedit->filesize +=
		    align_delta +
		    object->code_sig_cmd->datasize;
		if(object->seg_linkedit->filesize >
		   object->seg_linkedit->vmsize)
		    object->seg_linkedit->vmsize =
			rnd32(object->seg_linkedit->filesize,
			      get_segalign_from_flag(&arch_signs[i].arch_flag));
	    }
	    else if(object->seg_linkedit64 != NULL){
		object->seg_linkedit64->filesize +=
		    align_delta +
		    object->code_sig_cmd->datasize;
		if(object->seg_linkedit64->filesize >
		   object->seg_linkedit64->vmsize)
		    object->seg_linkedit64->vmsize =
			rnd(object->seg_linkedit64->filesize,
			      get_segalign_from_flag(&arch_signs[i].arch_flag));
	    }
	}
}

/*
 * add_code_sig_load_command() sees if there is space to add a code signature
 * load command for the specified arch and arch_name.  If so it returns a
 * pointer to the load command which the caller will fill in the dataoff and
 * datasize fields.  If it can't be added a fatal error message is printed
 * saying to relink the file.
 */
static
struct linkedit_data_command *
add_code_sig_load_command(
struct arch *arch,
char *arch_name,
struct member *member,
struct object *object)
{
    uint32_t i, j, low_fileoff;
    uint32_t ncmds, sizeofcmds, sizeof_mach_header;
    struct load_command *lc;
    struct segment_command *sg;
    struct segment_command_64 *sg64;
    struct section *s;
    struct section_64 *s64;
    struct linkedit_data_command *code_sig;

        if(object->mh != NULL){
            ncmds = object->mh->ncmds;
	    sizeofcmds = object->mh->sizeofcmds;
	    sizeof_mach_header = sizeof(struct mach_header);
	}
	else{
            ncmds = object->mh64->ncmds;
	    sizeofcmds = object->mh64->sizeofcmds;
	    sizeof_mach_header = sizeof(struct mach_header_64);
	}

	/*
	 * The size of the new load commands that includes the added code
	 * signature load command is larger than the existing load commands, so
	 * see if they can be fitted in before the contents of the first section
	 * (or segment in the case of a LINKEDIT segment only file, in that case
         * the fileoff is non-zero, as with MH_KEXTBUNDLE it may have a __TEXT
         * segment with no sections just mapping the headers as those sections
         * would be in the __TEXT_EXEC segment).
	 */
	low_fileoff = UINT_MAX;
	lc = object->load_commands;
	for(i = 0; i < ncmds; i++){
	    if(lc->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc;
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
		    if(sg->fileoff != 0 && sg->filesize != 0 &&
                       sg->fileoff < low_fileoff)
			low_fileoff = sg->fileoff;
		}
	    }
	    else if(lc->cmd == LC_SEGMENT_64){
		sg64 = (struct segment_command_64 *)lc;
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
		    if(sg64->fileoff != 0 && sg64->filesize != 0 &&
		       sg64->fileoff < low_fileoff)
			low_fileoff = (uint32_t)sg64->fileoff;
		}
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	if(sizeofcmds + sizeof(struct linkedit_data_command) +
	   sizeof_mach_header > low_fileoff){
	    if(member)
		fatal("can't allocate code signature data for: %s(%s) (for "
		      "architecture %s) because larger updated load commands "
		      "do not fit (the program must be relinked using a larger "
		      "-headerpad value)", arch->file_name, member->member_name,
		      arch_name);
	    else
		fatal("can't allocate code signature data for: %s (for "
		      "architecture %s) because larger updated load commands "
		      "do not fit (the program must be relinked using a larger "
		      "-headerpad value)", arch->file_name, arch_name);
	}
	/*
	 * There is space for the new load commands. So just use that space for
	 * the new code signature load command and set the fields.
	 */
	code_sig = (struct linkedit_data_command *)
		   ((char *)object->load_commands + sizeofcmds);
	code_sig->cmd = LC_CODE_SIGNATURE;
	code_sig->cmdsize = sizeof(struct linkedit_data_command);
	/* these two feilds will be set by the caller */
	code_sig->dataoff = 0;
	code_sig->datasize = 0;
	
        if(object->mh != NULL){
            object->mh->sizeofcmds = sizeofcmds +
	       sizeof(struct linkedit_data_command);
            object->mh->ncmds = ncmds + 1;
        }
	else{
            object->mh64->sizeofcmds = sizeofcmds +
	       sizeof(struct linkedit_data_command);
            object->mh64->ncmds = ncmds + 1;
        }
	return(code_sig);
}

/*
 * strip_LC_CODE_SIGNATURE_commands() is called when -r is specified to remove
 * any LC_CODE_SIGNATURE load commands from the object's load commands.
 */
static
void
strip_LC_CODE_SIGNATURE_commands(
struct arch *arch,
struct member *member,
struct object *object)
{
    uint32_t i, ncmds, mh_sizeofcmds, sizeofcmds;
    struct load_command *lc1, *lc2, *new_load_commands;
    struct segment_command *sg;

	/*
	 * See if there is an LC_CODE_SIGNATURE load command and if no command
	 * just return.
	 */
	if(object->code_sig_cmd == NULL)
	    return;

	/*
	 * Allocate space for the new load commands and zero it out so any holes
	 * will be zero bytes.
	 */
        if(arch->object->mh != NULL){
            ncmds = arch->object->mh->ncmds;
	    mh_sizeofcmds = arch->object->mh->sizeofcmds;
	}
	else{
            ncmds = arch->object->mh64->ncmds;
	    mh_sizeofcmds = arch->object->mh64->sizeofcmds;
	}
	new_load_commands = allocate(mh_sizeofcmds);
	memset(new_load_commands, '\0', mh_sizeofcmds);

	/*
	 * Copy all the load commands except the LC_CODE_SIGNATURE load commands
	 * into the allocated space for the new load commands.
	 */
	lc1 = arch->object->load_commands;
	lc2 = new_load_commands;
	sizeofcmds = 0;
	for(i = 0; i < ncmds; i++){
	    if(lc1->cmd != LC_CODE_SIGNATURE){
		memcpy(lc2, lc1, lc1->cmdsize);
		sizeofcmds += lc2->cmdsize;
		lc2 = (struct load_command *)((char *)lc2 + lc2->cmdsize);
	    }
	    lc1 = (struct load_command *)((char *)lc1 + lc1->cmdsize);
	}

	/*
	 * Finally copy the updated load commands over the existing load
	 * commands.
	 */
	memcpy(arch->object->load_commands, new_load_commands, sizeofcmds);
	if(mh_sizeofcmds > sizeofcmds){
		memset((char *)arch->object->load_commands + sizeofcmds, '\0', 
			   (mh_sizeofcmds - sizeofcmds));
	}
	ncmds -= 1;
        if(arch->object->mh != NULL) {
            arch->object->mh->sizeofcmds = sizeofcmds;
            arch->object->mh->ncmds = ncmds;
        } else {
            arch->object->mh64->sizeofcmds = sizeofcmds;
            arch->object->mh64->ncmds = ncmds;
        }
	free(new_load_commands);

	/*
	 * reset the pointers into the load commands
	 *
	 * Recall that the LC_CODE_SIGNATURE load command can be anywhere in
	 * the load command array. We've just removed LC_CODE_SIGNATURE and
	 * compacted the array, and if any of the following load commands
	 * followed the code signature cmd their pointers are invalid.
	 */
	object->code_sig_cmd = NULL;
	lc1 = arch->object->load_commands;
	for(i = 0; i < ncmds; i++){
	    switch(lc1->cmd){
	    case LC_SYMTAB:
		arch->object->st = (struct symtab_command *)lc1;
	        break;
	    case LC_DYSYMTAB:
		arch->object->dyst = (struct dysymtab_command *)lc1;
		break;
	    case LC_TWOLEVEL_HINTS:
		arch->object->hints_cmd = (struct twolevel_hints_command *)lc1;
		break;
	    case LC_PREBIND_CKSUM:
		arch->object->cs = (struct prebind_cksum_command *)lc1;
		break;
	    case LC_SEGMENT:
		sg = (struct segment_command *)lc1;
		if(strcmp(sg->segname, SEG_LINKEDIT) == 0)
		    arch->object->seg_linkedit = sg;
		break;
	    case LC_SEGMENT_SPLIT_INFO:
		object->split_info_cmd = (struct linkedit_data_command *)lc1;
		break;
	    case LC_FUNCTION_STARTS:
		object->func_starts_info_cmd =
					 (struct linkedit_data_command *)lc1;
		break;
	    case LC_DATA_IN_CODE:
		object->data_in_code_cmd =
				         (struct linkedit_data_command *)lc1;
		break;
	    case LC_DYLIB_CODE_SIGN_DRS:
		object->code_sign_drs_cmd =
				         (struct linkedit_data_command *)lc1;
		break;
	    case LC_LINKER_OPTIMIZATION_HINT:
		object->link_opt_hint_cmd =
				         (struct linkedit_data_command *)lc1;
		break;
	    case LC_DYLD_EXPORTS_TRIE:
		object->dyld_exports_trie =
				         (struct linkedit_data_command *)lc1;
		break;
	    case LC_DYLD_CHAINED_FIXUPS:
		object->dyld_chained_fixups =
				         (struct linkedit_data_command *)lc1;
		break;
	    }
	    lc1 = (struct load_command *)((char *)lc1 + lc1->cmdsize);
	}
}
