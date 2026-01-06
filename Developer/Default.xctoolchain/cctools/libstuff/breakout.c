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
#include <stdlib.h>
#include <string.h>
#include "stuff/ofile.h"
#include "stuff/breakout.h"
#include "stuff/allocate.h"
#include "stuff/errors.h"
#include "stuff/rnd.h"
#include "stuff/crc32.h"
#ifdef LTO_SUPPORT
#include "stuff/lto.h"
#endif /* LTO_SUPPORT */

static void breakout_internal(
    char *filename,
    struct arch **archs,
    uint32_t *narchs,
    enum bool calculate_input_prebind_cksum,
    struct ofile *ofile);
static void breakout_loop_through_archive(
    char *filename,
    struct arch *arch,
    struct ofile *ofile);
static void cksum_object(
    struct arch *arch,
    enum bool calculate_input_prebind_cksum);
static struct arch *new_arch(
    struct arch **archs,
    uint32_t *narchs);
static struct member *new_member(
    struct arch *arch);

__private_extern__
struct ofile *
breakout_mem(
void *membuf,
uint32_t length,
char *filename,
struct arch **archs,
uint32_t *narchs,
enum bool calculate_input_prebind_cksum)
{
    struct ofile *ofile;
    uint32_t previous_errors;

	*archs = NULL;
	*narchs = 0;
	ofile = allocate(sizeof(struct ofile));
	
	/*
	 * If the file_name is NULL, we will use a dummy file name so 
	 * that error reporting, etc. works.
	 */
	if(filename == NULL)
	    filename = "(broken out from memory)";
	
	/*
	 * Rely on the ofile_*() routines to do all the checking and only
	 * return valid ofiles files broken out.
	 */
	if(ofile_map_from_memory((char *)membuf, length, filename, 0,NULL, NULL,
				 ofile, FALSE) == FALSE){
	    free(ofile);
	    return(NULL);
	}

	previous_errors = errors;
	breakout_internal(filename, archs, narchs,
			  calculate_input_prebind_cksum, ofile);
	errors += previous_errors;
	if(errors != 0){
	    free(ofile);
	    return(NULL);
	}
	return(ofile);
}

__private_extern__
struct ofile *
breakout(
char *filename,
struct arch **archs,
uint32_t *narchs,
enum bool calculate_input_prebind_cksum)
{
    struct ofile *ofile;
    uint32_t previous_errors;
	
	*archs = NULL;
	*narchs = 0;
	ofile = allocate(sizeof(struct ofile));
	/*
	 * Rely on the ofile_*() routines to do all the checking and only
	 * return valid ofiles files broken out.
	 */
	if(ofile_map(filename, NULL, NULL, ofile, FALSE) == FALSE){
	    free(ofile);
	    return(NULL);
	}

	previous_errors = errors;
	breakout_internal(filename, archs, narchs, 
			  calculate_input_prebind_cksum, ofile);
	errors += previous_errors;
	if(errors != 0){
	    free(ofile);
	    return(NULL);
	}
	return(ofile);
}

static 
void 
breakout_internal(
char *filename,
struct arch **archs,
uint32_t *narchs,
enum bool calculate_input_prebind_cksum,
struct ofile *ofile)
{
    struct arch *arch;

	errors = 0;
	if(ofile->file_type == OFILE_FAT && errors == 0){
	    /* loop through the fat architectures (can't have zero archs) */
	    (void)ofile_first_arch(ofile);
	    do{
		if(errors != 0)
		    break;
		arch = new_arch(archs, narchs);
		arch->file_name = savestr(filename);
		arch->type = ofile->arch_type;
		if(ofile->fat_header->magic == FAT_MAGIC_64){
		    arch->fat_arch64 = ofile->fat_archs64 + ofile->narch;
		    arch->fat_arch = NULL;
		}
		else{
		    arch->fat_arch = ofile->fat_archs + ofile->narch;
		    arch->fat_arch64 = NULL;
		}
		arch->fat_arch_name = savestr(ofile->arch_flag.name);

		if(ofile->arch_type == OFILE_ARCHIVE){
		    breakout_loop_through_archive(filename, arch, ofile);
		}
		else if(ofile->arch_type == OFILE_Mach_O){
		    arch->object = allocate(sizeof(struct object));
		    memset(arch->object, '\0', sizeof(struct object));
		    arch->object->object_addr = ofile->object_addr;
		    arch->object->object_size = ofile->object_size;
		    arch->object->object_byte_sex = ofile->object_byte_sex;
		    arch->object->mh64 = ofile->mh64;
		    arch->object->mh = ofile->mh;
		    arch->object->mh_filetype = ofile->mh_filetype;
		    arch->object->mh_cputype = ofile->mh_cputype;
		    arch->object->mh_cpusubtype = ofile->mh_cpusubtype;
		    arch->object->load_commands = ofile->load_commands;
		    cksum_object(arch, calculate_input_prebind_cksum);
		}
#ifdef LTO_SUPPORT
		else if(ofile->arch_type == OFILE_LLVM_BITCODE){
		    arch->lto = ofile->lto;
		    if(ofile->fat_header->magic == FAT_MAGIC_64){
			arch->unknown_addr = ofile->file_addr +
					     arch->fat_arch64->offset;
			arch->unknown_size = arch->fat_arch64->size;
		    }
		    else{
			arch->unknown_addr = ofile->file_addr +
					     arch->fat_arch->offset;
			arch->unknown_size = arch->fat_arch->size;
		    }
		}
#endif /* LTO_SUPPORT */
		else{ /* ofile->arch_type == OFILE_UNKNOWN */
		    if(ofile->fat_header->magic == FAT_MAGIC_64){
			arch->unknown_addr = ofile->file_addr +
					     arch->fat_arch64->offset;
			arch->unknown_size = arch->fat_arch64->size;
		    }
		    else{
			arch->unknown_addr = ofile->file_addr +
					     arch->fat_arch->offset;
			arch->unknown_size = arch->fat_arch->size;
		    }
		}
	    }while(ofile_next_arch(ofile) == TRUE);
	}
	else if(ofile->file_type == OFILE_ARCHIVE && errors == 0){
	    arch = new_arch(archs, narchs);
	    arch->file_name = savestr(filename);
	    arch->type = ofile->file_type;

	    breakout_loop_through_archive(filename, arch, ofile);
	}
	else if(ofile->file_type == OFILE_Mach_O && errors == 0){
	    arch = new_arch(archs, narchs);
	    arch->file_name = savestr(filename);
	    arch->type = ofile->file_type;
	    arch->object = allocate(sizeof(struct object));
	    memset(arch->object, '\0', sizeof(struct object));
	    arch->object->object_addr = ofile->object_addr;
	    arch->object->object_size = ofile->object_size;
	    arch->object->object_byte_sex = ofile->object_byte_sex;
	    arch->object->mh64 = ofile->mh64;
	    arch->object->mh = ofile->mh;
	    arch->object->mh_filetype = ofile->mh_filetype;
	    arch->object->mh_cputype = ofile->mh_cputype;
	    arch->object->mh_cpusubtype = ofile->mh_cpusubtype;
	    arch->object->load_commands = ofile->load_commands;
	    cksum_object(arch, calculate_input_prebind_cksum);
	}
#ifdef LTO_SUPPORT
	else if(ofile->file_type == OFILE_LLVM_BITCODE && errors == 0){
	    arch = new_arch(archs, narchs);
	    arch->file_name = savestr(filename);
	    arch->type = ofile->file_type;
	    arch->lto = ofile->lto;
	    arch->unknown_addr = ofile->file_addr;
	    arch->unknown_size = ofile->file_size;
	}
#endif /* LTO_SUPPORT */
	else if(errors == 0){ /* ofile->file_type == OFILE_UNKNOWN */
	    arch = new_arch(archs, narchs);
	    arch->file_name = savestr(filename);
	    arch->type = ofile->file_type;
	    arch->unknown_addr = ofile->file_addr;
	    arch->unknown_size = ofile->file_size;
	}
	if(errors != 0){
	    free_archs(*archs, *narchs);
	    *archs = NULL;
	    *narchs = 0;
	}
}

static
void
breakout_loop_through_archive(
char *filename,
struct arch *arch,
struct ofile *ofile)
{
    struct member *member;
    enum bool flag;
    struct ar_hdr *ar_hdr;
    uint64_t size, ar_name_size;
    char ar_name_buf[sizeof(ofile->member_ar_hdr->ar_name) + 1];
    char ar_size_buf[sizeof(ofile->member_ar_hdr->ar_size) + 1];

	/* loop through archive (can be empty) */
	if((flag = ofile_first_member(ofile)) == TRUE && errors == 0){
	    /*
	     * If the first member is a table of contents then skip
	     * it as it is always rebuilt (so to get the time to
	     * match the modtime so it won't appear out of date).
	     * Also force it to be a long name so members can be 8 byte
	     * aligned.
	     */
	    if(ofile->member_ar_hdr != NULL &&
	       strncmp(ofile->member_name, SYMDEF,
		       sizeof(SYMDEF) - 1) == 0){
		arch->toc_long_name = TRUE;
		flag = ofile_next_member(ofile);
	    }
	    while(flag == TRUE && errors == 0){
		member = new_member(arch);
		member->type = ofile->member_type;
		member->member_name = ofile->member_name;
		/*
		 * Determine the size this member will have in the library which
		 * includes the padding as a result of rounding the size of the
		 * member.  To get all members on an 8 byte boundary (so that 
		 * mapping in object files can be used directly) the size of the
		 * member is CHANGED to reflect this padding.  In the UNIX
		 * definition of archives the size of the member is never
		 * changed but the offset to the next member is defined to be
		 * the offset of the previous member plus the size of the
		 * previous member rounded to 2.  So to get 8 byte boundaries
		 * without breaking the UNIX definition of archives the size is
		 * changed here.  As with the UNIX ar(1) program the padded
		 * bytes will be set to the character '\n'.
		 */
		if(ofile->mh != NULL || ofile->mh64 != NULL)
		    size = rnd(ofile->object_size, 8);
		else
		    size = rnd(ofile->member_size, 8);
		/*
		 * We will force the use of long names so we can make sure the
		 * size of the name and the size of struct ar_hdr are rounded to
		 * 8 bytes.  And that rounded size is what will be in the
		 * ar_name with the AR_EFMT1 string.  To avoid growing the size
		 * of names first trim the name size before rounding up.
		 */
		member->member_long_name = TRUE;
		for(ar_name_size = ofile->member_name_size;
		    ar_name_size > 1 ;
		    ar_name_size--){
		    if(ofile->member_name[ar_name_size - 1] != '\0')
		       break;
		}
		member->member_name_size = (uint32_t)ar_name_size;
		ar_name_size = rnd(ar_name_size, 8) +
			       (rnd(sizeof(struct ar_hdr), 8) -
				sizeof(struct ar_hdr));
		size += ar_name_size;
		/*
		 * Now with the output sizes of the long member name and rounded
		 * size of the member the offset to this member can be set and
		 * then left incremented for the next member's offset.
		 */
		member->offset = arch->library_size;
		arch->library_size += sizeof(struct ar_hdr) + size;
		/*
		 * Since we are rounding the member size and forcing a the use
		 * of a long name make a new ar_hdr with this information.
		 * Note the code in writeout() will do the padding with '\n'
		 * characters as needed.
		 */
		ar_hdr = allocate(sizeof(struct ar_hdr));
		*ar_hdr = *(ofile->member_ar_hdr);
		sprintf(ar_name_buf, "%s%-*lu", AR_EFMT1, 
			(int)(sizeof(ar_hdr->ar_name) - (sizeof(AR_EFMT1) - 1)),
		        (long unsigned int)ar_name_size);
		memcpy(ar_hdr->ar_name, ar_name_buf,
		      sizeof(ar_hdr->ar_name));
		sprintf(ar_size_buf, "%-*ld",
			(int)sizeof(ar_hdr->ar_size), (long unsigned int)size);
		memcpy(ar_hdr->ar_size, ar_size_buf,
		      sizeof(ar_hdr->ar_size));

		member->ar_hdr = ar_hdr;
		member->input_ar_hdr = ofile->member_ar_hdr;
		member->input_file_name = filename;

		if(ofile->member_type == OFILE_Mach_O){
		    member->object = allocate(sizeof(struct object));
		    memset(member->object, '\0', sizeof(struct object));
		    member->object->object_addr = ofile->object_addr;
		    member->object->object_size = ofile->object_size;
		    member->object->object_byte_sex = ofile->object_byte_sex;
		    member->object->mh64 = ofile->mh64;
		    member->object->mh = ofile->mh;
		    member->object->mh_filetype = ofile->mh_filetype;
		    member->object->mh_cputype = ofile->mh_cputype;
		    member->object->mh_cpusubtype = ofile->mh_cpusubtype;
		    member->object->load_commands = ofile->load_commands;
		}
#ifdef LTO_SUPPORT
		else if(ofile->member_type == OFILE_LLVM_BITCODE){
		    member->lto = ofile->lto;
		    member->unknown_addr = ofile->member_addr;
		    member->unknown_size = ofile->member_size;
		}
#endif /* LTO_SUPPORT */
		else{ /* ofile->member_type == OFILE_UNKNOWN */
		    member->unknown_addr = ofile->member_addr;
		    member->unknown_size = ofile->member_size;
		}
		flag = ofile_next_member(ofile);
	    }
	}
}

/*
 * cksum_object() is called to set the pointer to the LC_PREBIND_CKSUM load
 * command in the object struct for the specified arch.  If the parameter
 * calculate_input_prebind_cksum is TRUE then calculate the value
 * of the check sum for the input object if needed, set that into the
 * the calculated_input_prebind_cksum field of the object struct for the
 * specified arch.  This is needed for prebound files where the original
 * checksum (or zero) is recorded in the LC_PREBIND_CKSUM load command.
 * Only redo_prebinding operations sets the value of the cksum field to
 * non-zero and only if previously zero.  All other operations will set this
 * field to zero indicating a new original prebound file.
 */
static
void
cksum_object(
struct arch *arch,
enum bool calculate_input_prebind_cksum)
{
    uint32_t i, buf_size, ncmds;
    struct load_command *lc;
    enum byte_sex host_byte_sex;
    char *buf;

	arch->object->cs = NULL;
	lc = arch->object->load_commands;
	if(arch->object->mh != NULL)
	    ncmds = arch->object->mh->ncmds;
	else
	    ncmds = arch->object->mh64->ncmds;
	for(i = 0;
	    i < ncmds && arch->object->cs == NULL;
	    i++){
	    if(lc->cmd == LC_PREBIND_CKSUM)
		arch->object->cs = (struct prebind_cksum_command *)lc;
	    lc = (struct load_command *)((char *)lc + lc->cmdsize);
	}

	/*
	 * If we don't want to calculate the input check sum, or there is no
	 * LC_PREBIND_CKSUM load command or there is one and the check sum is
	 * not zero then return.
	 */
	if(calculate_input_prebind_cksum == FALSE ||
	   arch->object->cs == NULL ||
	   arch->object->cs->cksum != 0)
	    return;


	host_byte_sex = get_host_byte_sex();
	buf_size = 0;
	buf = NULL;
	if(arch->object->object_byte_sex != host_byte_sex){
	    if(arch->object->mh != NULL){
		buf_size = sizeof(struct mach_header) +
			   arch->object->mh->sizeofcmds;
		buf = allocate(buf_size);
		memcpy(buf, arch->object->mh, buf_size);
		if(swap_object_headers(arch->object->mh,
				       arch->object->load_commands) == FALSE)
		    return;
	    }
	    else{
		buf_size = sizeof(struct mach_header_64) +
			   arch->object->mh64->sizeofcmds;
		buf = allocate(buf_size);
		memcpy(buf, arch->object->mh64, buf_size);
		if(swap_object_headers(arch->object->mh64,
				       arch->object->load_commands) == FALSE)
		    return;
	    }
	}

	arch->object->calculated_input_prebind_cksum =
	    crc32(arch->object->object_addr, arch->object->object_size);

	if(arch->object->object_byte_sex != host_byte_sex){
	    if(arch->object->mh != NULL)
		memcpy(arch->object->mh, buf, buf_size);
	    else
		memcpy(arch->object->mh64, buf, buf_size);
	    free(buf);
	}
}

__private_extern__
void
free_archs(
struct arch *archs,
uint32_t narchs)
{
    uint32_t i, j;

	for(i = 0; i < narchs; i++){
	    if(archs[i].type == OFILE_ARCHIVE){
		for(j = 0; j < archs[i].nmembers; j++){
		    if(archs[i].members[j].type == OFILE_Mach_O){
			if(archs[i].members[j].object->ld_r_ofile != NULL)
			   ofile_unmap(archs[i].members[j].object->ld_r_ofile);
			free(archs[i].members[j].object);
		    }
#ifdef LTO_SUPPORT
		    else if(archs[i].members[j].type == OFILE_LLVM_BITCODE){
			if(archs[i].members[j].lto != NULL)
			    lto_free(archs[i].members[j].lto);
		    }
#endif /* LTO_SUPPORT */
		}
		if(archs[i].nmembers > 0 && archs[i].members != NULL)
		    free(archs[i].members);
	    }
	    else if(archs[i].type == OFILE_Mach_O){
		if(archs[i].object->ld_r_ofile != NULL)
		    ofile_unmap(archs[i].object->ld_r_ofile);
		free(archs[i].object);
	    }
#ifdef LTO_SUPPORT
	    else if(archs[i].type == OFILE_LLVM_BITCODE){
		if(archs[i].lto != NULL)
		    lto_free(archs[i].lto);
	    }
#endif /* LTO_SUPPORT */
	}
	if(narchs > 0 && archs != NULL)
	    free(archs);
}

static
struct arch *
new_arch(
struct arch **archs,
uint32_t *narchs)
{
    struct arch *arch;

	*archs = reallocate(*archs, (*narchs + 1) * sizeof(struct arch));
	arch = *archs + *narchs;
	*narchs = *narchs + 1;
	memset(arch, '\0', sizeof(struct arch));
	return(arch);
}

static
struct member *
new_member(
struct arch *arch)
{
    struct member *member;

	arch->members = reallocate(arch->members,
				  (arch->nmembers + 1) * sizeof(struct member));
	member = arch->members + arch->nmembers;
	arch->nmembers++;
	memset(member, '\0', sizeof(struct member));
	return(member);
}
#endif /* !defined(RLD) */
