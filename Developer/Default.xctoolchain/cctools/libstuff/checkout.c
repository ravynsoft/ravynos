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
#ifndef RLD
#include <stdio.h>
#include <string.h>
#include "../include/xar/xar.h" /* cctools-port: 
				   force the use of the bundled xar header */
#include "stuff/ofile.h"
#include "stuff/breakout.h"
#include "stuff/rnd.h"

static void check_object(
    struct arch *arch,
    struct member *member,
    struct object *object);

static void symbol_string_at_end(
    struct arch *arch,
    struct member *member,
    struct object *object);

static void dyld_order(
    struct arch *arch,
    struct member *member,
    struct object *object);

static void order_error(
    struct arch *arch,
    struct member *member,
    char *reason);

__private_extern__
void
checkout(
struct arch *archs,
uint32_t narchs)
{
    uint32_t i, j;

	for(i = 0; i < narchs; i++){
	    if(archs[i].type == OFILE_ARCHIVE){
		for(j = 0; j < archs[i].nmembers; j++){
		    if(archs[i].members[j].type == OFILE_Mach_O){
			check_object(archs + i, archs[i].members + j,
					 archs[i].members[j].object);
		    }
		}
	    }
	    else if(archs[i].type == OFILE_Mach_O){
		check_object(archs + i, NULL, archs[i].object);
	    }
	}
}

static
void
check_object(
struct arch *arch,
struct member *member,
struct object *object)
{
    uint32_t i, ncmds, flags;
    struct load_command *lc;
    struct segment_command *sg;
    struct segment_command_64 *sg64;
    struct dylib_command *dl_id;

	/*
	 * Set up the symtab load command field and link edit segment feilds in
	 * the object structure.
	 */
	object->st = NULL;
	object->dyst = NULL;
	object->hints_cmd = NULL;
	object->seg_bitcode = NULL;
	object->seg_bitcode64 = NULL;
	object->seg_linkedit = NULL;
	object->seg_linkedit64 = NULL;
	object->code_sig_cmd = NULL;
	dl_id = NULL;
	lc = object->load_commands;
	if(object->mh != NULL){
	    ncmds = object->mh->ncmds;
	    flags = object->mh->flags;
	}
	else{
	    ncmds = object->mh64->ncmds;
	    flags = object->mh64->flags;
	}
	for(i = 0; i < ncmds; i++){
	    if(lc->cmd == LC_SYMTAB){
		if(object->st != NULL)
		    fatal_arch(arch, member, "malformed file (more than one "
			"LC_SYMTAB load command): ");
		object->st = (struct symtab_command *)lc;
	    }
	    else if(lc->cmd == LC_DYSYMTAB){
		if(object->dyst != NULL)
		    fatal_arch(arch, member, "malformed file (more than one "
			"LC_DYSYMTAB load command): ");
		object->dyst = (struct dysymtab_command *)lc;
	    }
	    else if(lc->cmd == LC_TWOLEVEL_HINTS){
		if(object->hints_cmd != NULL)
		    fatal_arch(arch, member, "malformed file (more than one "
			"LC_TWOLEVEL_HINTS load command): ");
		object->hints_cmd = (struct twolevel_hints_command *)lc;
	    }
	    else if(lc->cmd == LC_CODE_SIGNATURE){
		if(object->code_sig_cmd != NULL)
		    fatal_arch(arch, member, "malformed file (more than one "
			"LC_CODE_SIGNATURE load command): ");
		object->code_sig_cmd = (struct linkedit_data_command *)lc;
	    }
	    else if(lc->cmd == LC_SEGMENT_SPLIT_INFO){
		if(object->split_info_cmd != NULL)
		    fatal_arch(arch, member, "malformed file (more than one "
			"LC_SEGMENT_SPLIT_INFO load command): ");
		object->split_info_cmd = (struct linkedit_data_command *)lc;
	    }
	    else if(lc->cmd == LC_FUNCTION_STARTS){
		if(object->func_starts_info_cmd != NULL)
		    fatal_arch(arch, member, "malformed file (more than one "
			"LC_FUNCTION_STARTS load command): ");
		object->func_starts_info_cmd =
			(struct linkedit_data_command *)lc;
	    }
	    else if(lc->cmd == LC_DATA_IN_CODE){
		if(object->data_in_code_cmd != NULL)
		    fatal_arch(arch, member, "malformed file (more than one "
			"LC_DATA_IN_CODE load command): ");
		object->data_in_code_cmd =
			(struct linkedit_data_command *)lc;
	    }
	    else if(lc->cmd == LC_DYLIB_CODE_SIGN_DRS){
		if(object->code_sign_drs_cmd != NULL)
		    fatal_arch(arch, member, "malformed file (more than one "
			"LC_DYLIB_CODE_SIGN_DRS load command): ");
		object->code_sign_drs_cmd =
			(struct linkedit_data_command *)lc;
	    }
	    else if(lc->cmd == LC_LINKER_OPTIMIZATION_HINT){
		if(object->link_opt_hint_cmd != NULL)
		    fatal_arch(arch, member, "malformed file (more than one "
			"LC_LINKER_OPTIMIZATION_HINT load command): ");
		object->link_opt_hint_cmd =
			(struct linkedit_data_command *)lc;
	    }
	    else if((lc->cmd == LC_DYLD_INFO) ||(lc->cmd == LC_DYLD_INFO_ONLY)){
		if(object->dyld_info != NULL)
		    fatal_arch(arch, member, "malformed file (more than one "
			"LC_DYLD_INFO load command): ");
		object->dyld_info = (struct dyld_info_command *)lc;
	    }
	    else if(lc->cmd == LC_DYLD_EXPORTS_TRIE){
		if(object->dyld_exports_trie != NULL)
		    fatal_arch(arch, member, "malformed file (more than one "
			"LC_DYLD_EXPORTS_TRIE load command): ");
		object->dyld_exports_trie =
			(struct linkedit_data_command *)lc;
	    }
	    else if(lc->cmd == LC_DYLD_CHAINED_FIXUPS){
		if(object->dyld_chained_fixups != NULL)
		    fatal_arch(arch, member, "malformed file (more than one "
			       "LC_DYLD_CHAINED_FIXUPS load command): ");
		object->dyld_chained_fixups =
		(struct linkedit_data_command *)lc;
	    }
	    else if(lc->cmd == LC_SEGMENT){
		sg = (struct segment_command *)lc;
		if(strcmp(sg->segname, SEG_LINKEDIT) == 0){
		    if(object->seg_linkedit != NULL)
			fatal_arch(arch, member, "malformed file (more than "
			    "one " SEG_LINKEDIT "segment): ");
		    object->seg_linkedit = sg;
		}
		else if(strcmp(sg->segname, "__LLVM") == 0){
		    if(object->seg_bitcode != NULL)
			fatal_arch(arch, member, "malformed file (more than "
			    "one __LLVM segment): ");
		    object->seg_bitcode = sg;
		}
	    }
	    else if(lc->cmd == LC_SEGMENT_64){
		sg64 = (struct segment_command_64 *)lc;
		if(strcmp(sg64->segname, SEG_LINKEDIT) == 0){
		    if(object->seg_linkedit64 != NULL)
			fatal_arch(arch, member, "malformed file (more than "
			    "one " SEG_LINKEDIT "segment): ");
		    object->seg_linkedit64 = sg64;
		}
		else if(strcmp(sg64->segname, "__LLVM") == 0){
		    if(object->seg_bitcode64 != NULL)
			fatal_arch(arch, member, "malformed file (more than "
			    "one __LLVM segment): ");
		    object->seg_bitcode64 = sg64;
		}
	    }
	    else if(lc->cmd == LC_ID_DYLIB){
		if(dl_id != NULL)
		    fatal_arch(arch, member, "malformed file (more than one "
			"LC_ID_DYLIB load command): ");
		dl_id = (struct dylib_command *)lc;
		if(dl_id->dylib.name.offset >= dl_id->cmdsize)
		    fatal_arch(arch, member, "malformed file (name.offset of "
			"load command %u extends past the end of the load "
			"command): ", i);
	    }
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}
	if((object->mh_filetype == MH_DYLIB ||
 	    (object->mh_filetype == MH_DYLIB_STUB && ncmds > 0)) &&
		dl_id == NULL)
	    fatal_arch(arch, member, "malformed file (no LC_ID_DYLIB load "
		"command in %s file): ", object->mh_filetype == MH_DYLIB ?
		"MH_DYLIB" : "MH_DYLIB_STUB");
	if(object->hints_cmd != NULL){
	    if(object->dyst == NULL && object->hints_cmd->nhints != 0)
		fatal_arch(arch, member, "malformed file (LC_TWOLEVEL_HINTS "
		"load command present without an LC_DYSYMTAB load command):");
	    if(object->hints_cmd->nhints != 0 &&
	       object->hints_cmd->nhints != object->dyst->nundefsym)
		fatal_arch(arch, member, "malformed file (LC_TWOLEVEL_HINTS "
		"load command's nhints does not match LC_DYSYMTAB load "
		"command's nundefsym):");
	}

	/*
	 * For objects without a dynamic symbol table check to see that the
	 * string table is at the end of the file and that the symbol table is
	 * just before it.
	 */
	if(object->dyst == NULL){
	    symbol_string_at_end(arch, member, object);
	}
	else{
	    /*
	     * This file has a dynamic symbol table command.  We handle three
	     * cases, a dynamic shared library, a file for the dynamic linker,
	     * and a relocatable object file.  Since it has a dynamic symbol
	     * table command it could have an indirect symbol table.
	     */
	    if(object->mh_filetype == MH_DYLIB /* ||
	       object->mh_filetype == MH_DYLIB_STUB */ ){
		/*
		 * This is a dynamic shared library.
		 * The order of the symbolic info is:
		 * 	local relocation entries
		 *	symbol table
		 *		local symbols
		 *		defined external symbols
		 *		undefined symbols
		 *	two-level namespace hints
		 * 	external relocation entries
		 *	indirect symbol table
		 *	table of contents
		 * 	module table
		 *	reference table
		 *	string table
		 *		strings for external symbols
		 *		strings for local symbols
		 *	code signature data (16 byte aligned)
		 */
		dyld_order(arch, member, object);
	    }
	    else if(flags & MH_DYLDLINK ||
		    object->mh_filetype == MH_KEXT_BUNDLE){
		/*
		 * This is a file for the dynamic linker (output of ld(1) with
		 * -output_for_dyld .  That is the relocation entries are split
		 * into local and external and hanging off the dysymtab not off
		 * the sections.
		 * The order of the symbolic info is:
		 * 	local relocation entries
		 *	symbol table
		 *		local symbols (in order as appeared in stabs)
		 *		defined external symbols (sorted by name)
		 *		undefined symbols (sorted by name)
		 * 	external relocation entries
		 *	indirect symbol table
		 *	string table
		 *		strings for external symbols
		 *		strings for local symbols
		 *	code signature data (16 byte aligned)
		 */
		dyld_order(arch, member, object);
	    }
	    else{
		/*
		 * This is a relocatable object file either the output of the
		 * assembler or output of ld(1) with -r.  For the output of
		 * the assembler:
		 * The order of the symbolic info is:
		 * 	relocation entries (by section)
		 *	indirect symbol table
		 *	symbol table
		 *		local symbols (in order as appeared in stabs)
		 *		defined external symbols (sorted by name)
		 *		undefined symbols (sorted by name)
		 *	string table
		 *		strings for external symbols
		 *		strings for local symbols
		 * With this order the symbol table can be replaced and the
		 * relocation entries and the indirect symbol table entries
		 * can be updated in the file and not moved.
		 * For the output of ld -r:
		 * The order of the symbolic info is:
		 * 	relocation entries (by section)
		 *	symbol table
		 *		local symbols (in order as appeared in stabs)
		 *		defined external symbols (sorted by name)
		 *		undefined symbols (sorted by name)
		 *	indirect symbol table
		 *	string table
		 *		strings for external symbols
		 *		strings for local symbols
		 *		code signature
		 */
		symbol_string_at_end(arch, member, object);
	    }
	}
}

static
void
dyld_order(
struct arch *arch,
struct member *member,
struct object *object)
{
    uint64_t offset, rounded_offset;
    uint32_t isym;

	if(object->mh != NULL){
	    if(object->seg_linkedit == NULL)
		fatal_arch(arch, member, "malformed file (no " SEG_LINKEDIT
		    " segment): ");
	    if(object->seg_linkedit->filesize != 0 &&
	       object->seg_linkedit->fileoff +
	       object->seg_linkedit->filesize != object->object_size)
		fatal_arch(arch, member, "the " SEG_LINKEDIT " segment "
		    "does not cover the end of the file (can't "
		    "be processed) in: ");

	    offset = object->seg_linkedit->fileoff;

	    if(object->seg_bitcode != NULL){
		if(object->seg_bitcode->filesize < sizeof(struct xar_header))
		    fatal_arch(arch, member, "the __LLVM segment too small "
			       "(less than sizeof(struct xar_header)) in: ");
		if(object->seg_bitcode->fileoff +
		   object->seg_bitcode->filesize != offset)
		    fatal_arch(arch, member, "the __LLVM segment not directly "
			       "before the "  SEG_LINKEDIT " segment in: ");
		if(object->seg_bitcode->vmaddr +
		   object->seg_bitcode->vmsize != object->seg_linkedit->vmaddr)
		    fatal_arch(arch, member, "the __LLVM segment's vmaddr plus "
			       "vmsize not directly before the vmaddr of the "
			       SEG_LINKEDIT " segment in: ");
	    }
	}
	else{
	    if(object->seg_linkedit64 == NULL)
		fatal_arch(arch, member, "malformed file (no " SEG_LINKEDIT
		    " segment): ");
	    if(object->seg_linkedit64->filesize != 0 &&
	       object->seg_linkedit64->fileoff +
	       object->seg_linkedit64->filesize != object->object_size)
		fatal_arch(arch, member, "the " SEG_LINKEDIT " segment "
		    "does not cover the end of the file (can't "
		    "be processed) in: ");

	    offset = object->seg_linkedit64->fileoff;

	    if(object->seg_bitcode64 != NULL){
		if(object->seg_bitcode64->filesize < sizeof(struct xar_header))
		    fatal_arch(arch, member, "the __LLVM segment too small "
			       "(less than sizeof(struct xar_header)) in: ");
		if(object->seg_bitcode64->fileoff +
		   object->seg_bitcode64->filesize != offset)
		    fatal_arch(arch, member, "the __LLVM segment not directly "
			       "before the "  SEG_LINKEDIT " segment in: ");
		if(object->seg_bitcode64->vmaddr +
		   object->seg_bitcode64->vmsize !=
						object->seg_linkedit64->vmaddr)
		    fatal_arch(arch, member, "the __LLVM segment's vmaddr plus "
			       "vmsize not directly before the vmaddr of the "
			       SEG_LINKEDIT " segment in: ");
	    }
	}

	if(object->dyld_info != NULL){
	    /* dyld_info starts at beginning of __LINKEDIT */
	    if (object->dyld_info->rebase_off != 0){
		if (object->dyld_info->rebase_off != offset)
		    order_error(arch, member, "dyld_info "
			"out of place");
	    }
	    else if (object->dyld_info->bind_off != 0){
		if (object->dyld_info->bind_off != offset)
		    order_error(arch, member, "dyld_info "
			"out of place");
	    }
	    else if(object->dyld_info->export_off != 0){
		if(object->dyld_info->export_off != offset &&
		   object->dyld_info->weak_bind_size != 0 &&
		   object->dyld_info->lazy_bind_size != 0)
		    order_error(arch, member, "dyld_info "
			"out of place");
	    }
	    /* update offset to end of dyld_info contents */
	    if (object->dyld_info->export_size != 0)
		offset = object->dyld_info->export_off + 
			    object->dyld_info->export_size;
	    else if (object->dyld_info->lazy_bind_size != 0)
		offset = object->dyld_info->lazy_bind_off + 
			    object->dyld_info->lazy_bind_size;
	    else if (object->dyld_info->weak_bind_size != 0)
		offset = object->dyld_info->weak_bind_off + 
			    object->dyld_info->weak_bind_size;
	    else if (object->dyld_info->bind_size != 0)
		offset = object->dyld_info->bind_off + 
			    object->dyld_info->bind_size;
	    else if (object->dyld_info->rebase_size != 0)
		offset = object->dyld_info->rebase_off + 
			    object->dyld_info->rebase_size;
	}
	if(object->dyld_chained_fixups != NULL){
	    /* dyld_chained_fixups starts at beginning of __LINKEDIT */
	    if (object->dyld_chained_fixups->dataoff != offset)
		    order_error(arch, member, "dyld chained fixups "
			"out of place");
	    offset = object->dyld_chained_fixups->dataoff +
	    		object->dyld_chained_fixups->datasize;
	}
	if(object->dyld_exports_trie != NULL){
	    if (object->dyld_exports_trie->dataoff != offset)
		    order_error(arch, member, "dyld exports trie "
			"out of place");
	    offset = object->dyld_exports_trie->dataoff +
	    		object->dyld_exports_trie->datasize;
	}
	if(object->dyst->nlocrel != 0){
	    if(object->dyst->locreloff != offset)
		order_error(arch, member, "local relocation entries "
		    "out of place");
	    offset += object->dyst->nlocrel *
		      sizeof(struct relocation_info);
	}
	if(object->split_info_cmd != NULL){
	    if(object->split_info_cmd->dataoff != 0 &&
	       object->split_info_cmd->dataoff != offset)
		order_error(arch, member, "split info data out of place");
	    offset += object->split_info_cmd->datasize;
	}
	if(object->func_starts_info_cmd != NULL){
	    if(object->func_starts_info_cmd->dataoff != 0 &&
	       object->func_starts_info_cmd->dataoff != offset)
		order_error(arch, member, "function starts data out of place");
	    offset += object->func_starts_info_cmd->datasize;
	}
	if(object->data_in_code_cmd != NULL){
	    if(object->data_in_code_cmd->dataoff != 0 &&
	       object->data_in_code_cmd->dataoff != offset)
		order_error(arch, member, "data in code info out of place");
	    offset += object->data_in_code_cmd->datasize;
	}
	if(object->code_sign_drs_cmd != NULL){
	    if(object->code_sign_drs_cmd->dataoff != 0 &&
	       object->code_sign_drs_cmd->dataoff != offset)
		order_error(arch, member, "code signing DRs info out of place");
	    offset += object->code_sign_drs_cmd->datasize;
	}
	if(object->link_opt_hint_cmd != NULL){
	    if(object->link_opt_hint_cmd->dataoff != 0 &&
	       object->link_opt_hint_cmd->dataoff != offset)
		order_error(arch, member, "linker optimization hint info out "
					  "of place");
	    offset += object->link_opt_hint_cmd->datasize;
	}
	if(object->st->nsyms != 0){
	    if(object->st->symoff != offset)
		order_error(arch, member, "symbol table out of place");
	    if(object->mh != NULL)
		offset += object->st->nsyms * sizeof(struct nlist);
	    else
		offset += object->st->nsyms * sizeof(struct nlist_64);
	}
	isym = 0;
	if(object->dyst->nlocalsym != 0){
	    if(object->dyst->ilocalsym != isym)
		order_error(arch, member, "local symbols out of place");
	    isym += object->dyst->nlocalsym;
	}
	if(object->dyst->nextdefsym != 0){
	    if(object->dyst->iextdefsym != isym)
		order_error(arch, member, "externally defined symbols out of "
			    "place");
	    isym += object->dyst->nextdefsym;
	}
	if(object->dyst->nundefsym != 0){
	    if(object->dyst->iundefsym != isym)
		order_error(arch, member, "undefined symbols out of place");
	    isym += object->dyst->nundefsym;
	}
	if(object->hints_cmd != NULL && object->hints_cmd->nhints != 0){
	    if(object->hints_cmd->offset != offset)
		order_error(arch, member, "hints table out of place");
	    offset += object->hints_cmd->nhints * sizeof(struct twolevel_hint);
	}
	if(object->dyst->nextrel != 0){
	    if(object->dyst->extreloff != offset)
		order_error(arch, member, "external relocation entries"
		    " out of place");
	    offset += object->dyst->nextrel *
		      sizeof(struct relocation_info);
	}
	if(object->dyst->nindirectsyms != 0){
	    if(object->dyst->indirectsymoff != offset)
		order_error(arch, member, "indirect symbol table "
		    "out of place");
	    offset += object->dyst->nindirectsyms *
		      sizeof(uint32_t);
	}

	/*
	 * If this is a 64-bit Mach-O file and has an odd number of indirect
	 * symbol table entries the next offset MAYBE rounded to a multiple of
	 * 8 or MAY NOT BE. This should done to keep all the tables aligned but
	 * was not done for 64-bit Mach-O in Mac OS X 10.4.
	 */
 	object->input_indirectsym_pad = 0;
	if(object->mh64 != NULL &&
	   (object->dyst->nindirectsyms % 2) != 0){
	    rounded_offset = rnd(offset, 8);
	}
	else{
	    rounded_offset = offset;
	}

	if(object->dyst->ntoc != 0){
	    if(object->dyst->tocoff != offset &&
	       object->dyst->tocoff != rounded_offset)
		order_error(arch, member, "table of contents out of place");
	    if(object->dyst->tocoff == offset){
		offset += object->dyst->ntoc *
			  sizeof(struct dylib_table_of_contents);
		rounded_offset = offset;
	    }
	    else if(object->dyst->tocoff == rounded_offset){
		object->input_indirectsym_pad = (uint32_t)(rounded_offset -
							   offset);
		rounded_offset += object->dyst->ntoc *
			          sizeof(struct dylib_table_of_contents);
		offset = rounded_offset;
	    }
	}
	if(object->dyst->nmodtab != 0){
	    if(object->dyst->modtaboff != offset &&
	       object->dyst->modtaboff != rounded_offset)
		order_error(arch, member, "module table out of place");
	    if(object->mh != NULL){
		offset += object->dyst->nmodtab *
			  sizeof(struct dylib_module);
		rounded_offset = offset;
	    }
	    else{
		if(object->dyst->modtaboff == offset){
		    offset += object->dyst->nmodtab *
			      sizeof(struct dylib_module_64);
		    rounded_offset = offset;
		}
		else if(object->dyst->modtaboff == rounded_offset){
		    object->input_indirectsym_pad = (uint32_t)(rounded_offset -
							       offset);
		    rounded_offset += object->dyst->nmodtab *
				      sizeof(struct dylib_module_64);
		    offset = rounded_offset;
		}
	    }
	}
	if(object->dyst->nextrefsyms != 0){
	    if(object->dyst->extrefsymoff != offset &&
	       object->dyst->extrefsymoff != rounded_offset)
		order_error(arch, member, "reference table out of place");
	    if(object->dyst->extrefsymoff == offset){
		offset += object->dyst->nextrefsyms *
			  sizeof(struct dylib_reference);
		rounded_offset = offset;
	    }
	    else if(object->dyst->extrefsymoff == rounded_offset){
		object->input_indirectsym_pad = (uint32_t)(rounded_offset -
							   offset);
		rounded_offset += object->dyst->nextrefsyms *
			          sizeof(struct dylib_reference);
		offset = rounded_offset;
	    }
	}
	if(object->st->strsize != 0){
	    if(object->st->stroff != offset &&
	       object->st->stroff != rounded_offset)
		order_error(arch, member, "string table out of place");
	    if(object->st->stroff == offset){
		offset += object->st->strsize;
		rounded_offset = offset;
	    }
	    else if(object->st->stroff == rounded_offset){
		object->input_indirectsym_pad = (uint32_t)(rounded_offset -
							   offset);
		rounded_offset += object->st->strsize;
		offset = rounded_offset;
	    }
	}
	if(object->code_sig_cmd != NULL){
	    rounded_offset = rnd(rounded_offset, 16);
	    if(object->code_sig_cmd->dataoff != rounded_offset)
		order_error(arch, member, "code signature data out of place");
	    rounded_offset += object->code_sig_cmd->datasize;
	    offset = rounded_offset;
	}
	if(offset != object->object_size &&
	   rounded_offset != object->object_size)
	    order_error(arch, member, "link edit information does not fill the "
			SEG_LINKEDIT " segment");
}

static
void
order_error(
struct arch *arch,
struct member *member,
char *reason)
{
	fatal_arch(arch, member, "file not in an order that can be processed "
		   "(%s): ", reason);
}

static
void
symbol_string_at_end(
struct arch *arch,
struct member *member,
struct object *object)
{
    uint32_t end, sigend, strend, rounded_strend;
    uint32_t indirectend, rounded_indirectend;

	if(object->st != NULL && object->st->nsyms != 0){
	    end = object->object_size;
	    if(object->code_sig_cmd != NULL){
		sigend = object->code_sig_cmd->dataoff +
			 object->code_sig_cmd->datasize;
		if(sigend != end)
		    fatal_arch(arch, member, "code signature not at the end "
			"of the file (can't be processed) in file: ");
		/*
		 * The code signature starts at a 16 byte offset.  So if the
		 * string table end rouned to 16 bytes is the offset where the 
		 * code signature starts then just back up the current "end" to
		 * the end of the string table.
		 */
		end = object->code_sig_cmd->dataoff;
		if(object->st->strsize != 0){
		    strend = object->st->stroff + object->st->strsize;
		    rounded_strend = (uint32_t)rnd(strend, 16);
		    if(object->code_sig_cmd->dataoff == rounded_strend)
		       end = strend;
		}
	    }
	    if(object->st->strsize != 0){
		strend = object->st->stroff + object->st->strsize;
		/*
		 * Since archive member sizes are now rounded to 8 bytes the
		 * string table may not be exactly at the end of the
		 * object_size due to rounding.
		 */
		rounded_strend = (uint32_t)rnd(strend, 8);
		if(strend != end && rounded_strend != end)
		    fatal_arch(arch, member, "string table not at the end "
			"of the file (can't be processed) in file: ");
		/*
		 * To make the code work that assumes the end of string table is
		 * at the end of the object file change the object_size to be
		 * the end of the string table here.  This could be done at the
		 * end of this routine but since all the later checks are fatal
		 * we'll just do this here.  If there is a code signature after
		 * string table don't do this.
		 */
		if(rounded_strend != strend && object->code_sig_cmd == NULL)
		    object->object_size = strend;
		end = object->st->stroff;
	    }
	    if(object->dyst != NULL &&
	       object->dyst->nindirectsyms != 0 &&
	       object->st->nsyms != 0 &&
	       object->dyst->indirectsymoff > object->st->symoff){

		indirectend = object->dyst->indirectsymoff + 
		    object->dyst->nindirectsyms * sizeof(uint32_t);

		/*
		 * If this is a 64-bit Mach-O file and has an odd number of
		 * indirect symbol table entries the next offset MAYBE rounded
		 * to a multiple of 8 or MAY NOT BE. This should done to keep
		 * all the tables aligned but was not done for 64-bit Mach-O in
		 * Mac OS X 10.4.
		 */
		if(object->mh64 != NULL &&
		   (object->dyst->nindirectsyms % 2) != 0){
		    rounded_indirectend = (uint32_t)rnd(indirectend, 8);
		}
		else{
		    rounded_indirectend = indirectend;
		}

		if(indirectend != end && rounded_indirectend != end){
		    fatal_arch(arch, member, "indirect symbol table does not "
			"directly preceed the string table (can't be "
			"processed) in file: ");
		}
		object->input_indirectsym_pad = end - indirectend;
		end = object->dyst->indirectsymoff;
		if(object->mh != NULL){
		    if(object->st->symoff +
		       object->st->nsyms * sizeof(struct nlist) != end)
			fatal_arch(arch, member, "symbol table does not "
			    "directly preceed the indirect symbol table (can't "
			    "be processed) in file: ");
		}
		else{
		    if(object->st->symoff +
		       object->st->nsyms * sizeof(struct nlist_64) != end)
			fatal_arch(arch, member, "symbol table does not "
			    "directly preceed the indirect symbol table (can't "
			    "be processed) in file: ");
		}
	    }
	    else{
		if(object->mh != NULL){
		    if(object->st->symoff +
		       object->st->nsyms * sizeof(struct nlist) != end)
			fatal_arch(arch, member, "symbol table and string "
			    "table not at the end of the file (can't be "
			    "processed) in file: ");
		}
		else{
		    if(object->st->symoff +
		       object->st->nsyms * sizeof(struct nlist_64) != end)
			fatal_arch(arch, member, "symbol table and string "
			    "table not at the end of the file (can't be "
			    "processed) in file: ");
		}
	    }
	    if(object->seg_linkedit != NULL &&
	       (object->seg_linkedit->flags & SG_FVMLIB) != SG_FVMLIB &&
	       object->seg_linkedit->filesize != 0){
		if(object->seg_linkedit->fileoff +
		   object->seg_linkedit->filesize != object->object_size)
		    fatal_arch(arch, member, "the " SEG_LINKEDIT " segment "
			"does not cover the symbol and string table (can't "
			"be processed) in file: ");
	    }
	}
}
#endif /* !defined(RLD) */
