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
#include <sys/types.h>
#include <sys/stat.h>
#include "stuff/errors.h"
#include "stuff/breakout.h"
#include "stuff/rnd.h"

/* used by error routines as the name of the program */
char *progname = NULL;

char *segname = NULL;

static void usage(
    void);

static void process(
    struct arch *archs,
    uint32_t narchs);

static void hack_seg(
    struct object *object,
    struct load_command *lc,
    struct segment_command *sg);

/* apple_version is created by the libstuff/Makefile */
extern char apple_version[];
char *version = apple_version;

/*
 * The seg_hack(1) program changes all segments names to the one specified on
 * the command line:
 *	seg_hack NEWSEGNAME input -o output
 * except for S_DEBUG sections.
 */
int
main(
int argc,
char **argv,
char **envp)
{
    int i;
    char *input, *output;
    struct arch *archs;
    uint32_t narchs;
    struct stat stat_buf;

	progname = argv[0];
	input = NULL;
	output = NULL;
	archs = NULL;
	narchs = 0;
	if(argc < 3){
	    usage();
	    return(EXIT_FAILURE);
	}
	segname = argv[1];
	for(i = 2; i < argc; i++){
	    if(strcmp(argv[i], "-o") == 0){
		if(i + 1 == argc){
		    error("missing argument(s) to: %s option", argv[i]);
		    usage();
		}
		if(output != NULL){
		    error("more than one: %s option specified", argv[i]);
		    usage();
		}
		output = argv[i+1];
		i++;
	    }
	    else{
		if(input != NULL){
		    error("more than one input file specified (%s and %s)",
			  argv[i], input);
		    usage();
		}
		input = argv[i];
	    }
	}
	if(input == NULL || output == NULL)
	    usage();

	breakout(input, &archs, &narchs, FALSE);
	if(errors)
	    return(EXIT_FAILURE);

	process(archs, narchs);

	/* create the output file */
	if(stat(input, &stat_buf) == -1)
	    system_error("can't stat input file: %s", input);
	writeout(archs, narchs, output, stat_buf.st_mode & 0777,
		     TRUE, FALSE, FALSE, FALSE, NULL);

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
	fprintf(stderr, "Usage: %s NEWSEGNAME input -o output\n", progname);
	exit(EXIT_FAILURE);
}

static
void
process(
struct arch *archs,
uint32_t narchs)
{
    uint32_t i, j, k, offset, size;
    struct object *object;
    struct load_command *lc;
    struct segment_command *sg;
    struct ar_hdr h;
    char size_buf[sizeof(h.ar_size) + 1];

	for(i = 0; i < narchs; i++){
	    if(archs[i].type == OFILE_ARCHIVE){
		for(j = 0; j < archs[i].nmembers; j++){
		    if(archs[i].members[j].type == OFILE_Mach_O){
			object = archs[i].members[j].object;
			lc = object->load_commands;
			for(k = 0; k < object->mh->ncmds; k++){
			    if(lc->cmd == LC_SYMTAB){
				object->st = (struct symtab_command *)lc;
			    }
			    else if(lc->cmd == LC_SEGMENT){
				sg = (struct segment_command *)lc;
				hack_seg(object, lc, sg);
			    }
			    else if(lc->cmd == LC_DYSYMTAB){
				error_arch(archs + i, archs[i].members + j,
				    "can't process -dynamic object: ");
				return;
			    }
			    lc = (struct load_command *)((char *)lc +
							 lc->cmdsize);
			}
			if(object->st != NULL && object->st->nsyms != 0){
			    object->output_symbols
				    = (struct nlist *)(object->object_addr +
						       object->st->symoff);
			    if(object->object_byte_sex != get_host_byte_sex())
				swap_nlist(object->output_symbols,
					   object->st->nsyms,
					   get_host_byte_sex());
			    object->output_nsymbols = object->st->nsyms;
			    object->output_strings =
				object->object_addr + object->st->stroff;
			    object->output_strings_size = object->st->strsize;
			    object->input_sym_info_size =
				object->st->nsyms * sizeof(struct nlist) +
				object->st->strsize;
			    object->output_sym_info_size =
				object->input_sym_info_size;
			}
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
		object = archs[i].object;
		lc = object->load_commands;
		for(j = 0; j < object->mh->ncmds; j++){
		    if(lc->cmd == LC_SYMTAB){
			object->st = (struct symtab_command *)lc;
		    }
		    else if(lc->cmd == LC_SEGMENT){
			sg = (struct segment_command *)lc;
			hack_seg(object, lc, sg);
		    }
		    else if(lc->cmd == LC_DYSYMTAB){
			error_arch(archs + i, NULL, "can't process -dynamic "
			    "object: ");
			return;
		    }
		    lc = (struct load_command *)((char *)lc +
						 lc->cmdsize);
		}
		if(object->st != NULL && object->st->nsyms != 0){
		    object->output_symbols
			    = (struct nlist *)(object->object_addr +
					       object->st->symoff);
		    if(object->object_byte_sex != get_host_byte_sex()){
			swap_nlist(object->output_symbols, object->st->nsyms,
				   get_host_byte_sex());
		    }
		    object->output_nsymbols = object->st->nsyms;
		    object->output_strings =
			object->object_addr + object->st->stroff;
		    object->output_strings_size = object->st->strsize;
		    object->input_sym_info_size =
			object->st->nsyms * sizeof(struct nlist) +
			object->st->strsize;
		    object->output_sym_info_size =
			object->input_sym_info_size;
		}
		/*
		 * Always clear the prebind checksum if any when creating a new
		 * file.
		 */
		if(object->cs != NULL)
		    object->cs->cksum = 0;
	    }
	}
}

/*
 * hack_seg() changes the segment names of the segment command and the following
 * section structs to the new segment name.
 */
static
void
hack_seg(
struct object *object,
struct load_command *lc,
struct segment_command *sg)
{
    struct section *s;
    uint32_t i;

	if(strcmp(sg->segname, SEG_PAGEZERO) != 0 &&
	   strcmp(sg->segname, SEG_LINKEDIT) != 0){
	    if(object->mh->filetype != MH_OBJECT){
		memset(sg->segname, '\0', sizeof(sg->segname));
		strncpy(sg->segname, segname, sizeof(sg->segname));
	    }
	    s = (struct section *)((char *)lc + sizeof(struct segment_command));
	    for(i = 0; i < sg->nsects; i++){
	        if (! (s->flags & S_ATTR_DEBUG)){
		  memset(s->segname, '\0', sizeof(s->segname));
		  strncpy(s->segname, segname, sizeof(s->segname));
		}
		s++;
	    }
	}
}
