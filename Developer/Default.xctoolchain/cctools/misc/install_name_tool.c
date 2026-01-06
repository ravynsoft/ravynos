/*
 * Copyright (c) 2001 Apple Computer, Inc. All rights reserved.
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
#include <unistd.h>
#include <fcntl.h>
#if defined(__APPLE__) && __aarch64__ /* cctools-port: copy before modification */
#include <copyfile.h>
#endif
#include "stuff/port.h" /* cctools-port: fake signing */
#include "stuff/errors.h"
#include "stuff/breakout.h"
#include "stuff/rnd.h"
#include "stuff/allocate.h"
#include "stuff/write64.h"

/* used by error routines as the name of the program */
char *progname = NULL;

static void usage(
    void);

static void process(
    struct arch *archs,
    uint32_t narchs);

static void write_on_input(
    struct arch *archs,
    uint32_t narchs,
    char *input);

static void update_load_commands(
    struct arch *arch,
    uint32_t *header_size);

/* the argument to the -id option */
static char *id = NULL;

/* the arguments to the -change options */
struct changes {
    char *old;
    char *new;
};
static struct changes *changes = NULL;
static uint32_t nchanges = 0;

/* the arguments to the -rpath options */
struct rpaths {
    char *old;
    char *new;
    enum bool found;
};
static struct rpaths *rpaths = NULL;
static uint32_t nrpaths = 0;

/* the arguments to the -add_rpath options */
struct add_rpaths {
    char *new;
};
static struct add_rpaths *add_rpaths = NULL;
static uint32_t nadd_rpaths = 0;

/* the arguments to the -delete_rpath options */
struct delete_rpaths {
    char *old;
    enum bool found;
};
static struct delete_rpaths *delete_rpaths = NULL;
static uint32_t ndelete_rpaths = 0;

/*
 * This is a pointer to an array of the original header sizes (mach header and
 * load commands) for each architecture which is used when we are writing on the
 * input file.
 */
static uint32_t *arch_header_sizes = NULL;

/* apple_version is created by the libstuff/Makefile */
extern char apple_version[];
char *version = apple_version;

/*
 * The install_name_tool allow the dynamic shared library install names of a
 * Mach-O binary to be changed.  For this tool to work when the install names
 * are larger the binary should be built with the ld(1)
 * -headerpad_max_install_names option.
 *
 *    Usage: install_name_tool [-change old new] ... [-rpath old new] ...
 * 	[-add_rpath new] ... [-delete_rpath old] ... [-id name] input
 *
 * The "-change old new" option changes the "old" install name to the "new"
 * install name if found in the binary.
 * 
 * The "-rpath old new" option changes the "old" path name in the rpath to
 * the "new" path name in an LC_RPATH load command in the binary.
 *
 * The "-add_rpath new" option adds an LC_RPATH load command.
 *
 * The "-delete_rpath old" option deletes the LC_RPATH load command with the
 * "old" path name in the binary.
 *
 * The "-id name" option changes the install name in the LC_ID_DYLIB load
 * command for a dynamic shared library.
 */
int
main(
int argc,
char **argv,
char **envp)
{
    int i, j;
    struct arch *archs;
    uint32_t narchs;
    char *input;
    char *output;

	output = NULL;
	progname = argv[0];
	input = NULL;
	archs = NULL;
	narchs = 0;
	for(i = 1; i < argc; i++){
	    if(strcmp(argv[i], "-id") == 0){
		if(i + 1 == argc){
		    error("missing argument to: %s option", argv[i]);
		    usage();
		}
		if(id != NULL){
		    error("more than one: %s option specified", argv[i]);
		    usage();
		}
		id = argv[i+1];
		i++;
	    }
	    else if(strcmp(argv[i], "-change") == 0){
		if(i + 2 >= argc){
		    error("missing argument(s) to: %s option", argv[i]);
		    usage();
		}
		changes = reallocate(changes,
				     sizeof(struct changes) * (nchanges + 1));
		changes[nchanges].old = argv[i+1];
		changes[nchanges].new = argv[i+2];
		nchanges += 1;
		i += 2;
	    }
	    else if(strcmp(argv[i], "-rpath") == 0){
		if(i + 2 >= argc){
		    error("missing argument(s) to: %s option", argv[i]);
		    usage();
		}
		for(j = 0; j < nrpaths; j++){
		    if(strcmp(rpaths[j].old, argv[i+1]) == 0){
		        if(strcmp(rpaths[j].new, argv[i+2]) == 0){
			    error("\"-rpath %s %s\" specified more than once",
				   argv[i+1], argv[i+2]);
			    usage();
			}
			error("can't specify both \"-rpath %s %s\" and "
			      "\"-rpath %s %s\"", rpaths[j].old, rpaths[j].new,
			      argv[i+1], argv[i+2]);
			usage();
		    }
		    if(strcmp(rpaths[j].new, argv[i+1]) == 0 ||
		       strcmp(rpaths[j].old, argv[i+2]) == 0 ||
		       strcmp(rpaths[j].new, argv[i+2]) == 0){
			error("can't specify both \"-rpath %s %s\" and "
			      "\"-rpath %s %s\"", rpaths[j].old, rpaths[j].new,
			      argv[i+1], argv[i+2]);
			usage();
		    }
		}
		for(j = 0; j < nadd_rpaths; j++){
		    if(strcmp(add_rpaths[j].new, argv[i+1]) == 0 ||
		       strcmp(add_rpaths[j].new, argv[i+2]) == 0){
			error("can't specify both \"-add_rpath %s\" "
			      "and \"-rpath %s %s\"", add_rpaths[j].new,
			      argv[i+1], argv[i+2]);
			usage();
		    }
		}
		for(j = 0; j < ndelete_rpaths; j++){
		    if(strcmp(delete_rpaths[j].old, argv[i+1]) == 0 ||
		       strcmp(delete_rpaths[j].old, argv[i+2]) == 0){
			error("can't specify both \"-delete_rpath %s\" "
			      "and \"-rpath %s %s\"", delete_rpaths[j].old,
			      argv[i+1], argv[i+2]);
			usage();
		    }
		}
		rpaths = reallocate(rpaths,
				    sizeof(struct rpaths) * (nrpaths + 1));
		rpaths[nrpaths].old = argv[i+1];
		rpaths[nrpaths].new = argv[i+2];
		rpaths[nrpaths].found = FALSE;
		nrpaths += 1;
		i += 2;
	    }
	    else if(strcmp(argv[i], "-add_rpath") == 0){
		if(i + 1 == argc){
		    error("missing argument(s) to: %s option", argv[i]);
		    usage();
		}
		for(j = 0; j < nadd_rpaths; j++){
		    if(strcmp(add_rpaths[j].new, argv[i+1]) == 0){
			error("\"-add_rpath %s\" specified more than once",
			      add_rpaths[j].new);
			usage();
		    }
		}
		for(j = 0; j < nrpaths; j++){
		    if(strcmp(rpaths[j].old, argv[i+1]) == 0 ||
		       strcmp(rpaths[j].new, argv[i+1]) == 0){
			error("can't specify both \"-rpath %s %s\" and "
			      "\"-add_rpath %s\"", rpaths[j].old, rpaths[j].new,
			      argv[i+1]);
			usage();
		    }
		}
		for(j = 0; j < ndelete_rpaths; j++){
		    if(strcmp(delete_rpaths[j].old, argv[i+1]) == 0){
			error("can't specify both \"-delete_rpath %s\" "
			      "and \"-add_rpath %s\"", delete_rpaths[j].old,
			      argv[i+1]);
			usage();
		    }
		}
		add_rpaths = reallocate(add_rpaths,
				sizeof(struct add_rpaths) * (nadd_rpaths + 1));
		add_rpaths[nadd_rpaths].new = argv[i+1];
		nadd_rpaths += 1;
		i += 1;
	    }
	    else if(strcmp(argv[i], "-delete_rpath") == 0){
		if(i + 1 == argc){
		    error("missing argument(s) to: %s option", argv[i]);
		    usage();
		}
		for(j = 0; j < ndelete_rpaths; j++){
		    if(strcmp(delete_rpaths[j].old, argv[i+1]) == 0){
			error("\"-delete_rpath %s\" specified more than once",
			      delete_rpaths[j].old);
			usage();
		    }
		}
		for(j = 0; j < nrpaths; j++){
		    if(strcmp(rpaths[j].old, argv[i+1]) == 0 ||
		       strcmp(rpaths[j].new, argv[i+1]) == 0){
			error("can't specify both \"-rpath %s %s\" and "
			      "\"-delete_rpath %s\"", rpaths[j].old,
			      rpaths[j].new, argv[i+1]);
			usage();
		    }
		}
		for(j = 0; j < nadd_rpaths; j++){
		    if(strcmp(add_rpaths[j].new, argv[i+1]) == 0){
			error("can't specify both \"-add_rpath %s\" "
			      "and \"-delete_rpath %s\"", add_rpaths[j].new,
			      argv[i+1]);
			usage();
		    }
		}
		delete_rpaths = reallocate(delete_rpaths,
				sizeof(struct delete_rpaths) *
					(ndelete_rpaths + 1));
		delete_rpaths[ndelete_rpaths].old = argv[i+1];
		delete_rpaths[ndelete_rpaths].found = FALSE;
		ndelete_rpaths += 1;
		i += 1;
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
	if(input == NULL || (id == NULL && nchanges == 0 && nrpaths == 0 &&
	   nadd_rpaths == 0 && ndelete_rpaths == 0))
	    usage();

	breakout(input, &archs, &narchs, FALSE);
	if(errors)
	    exit(EXIT_FAILURE);

	checkout(archs, narchs);
	if(errors)
	    exit(EXIT_FAILURE);

	arch_header_sizes = allocate(narchs * sizeof(uint32_t));
	process(archs, narchs);
	if(errors)
	    exit(EXIT_FAILURE);

	if(output != NULL)
	    writeout(archs, narchs, output, 0777, TRUE, FALSE, FALSE, FALSE,
		     NULL);
	else {
	    write_on_input(archs, narchs, input);
        output = input;
    }

	FAKE_SIGN_ARM_BINARY(archs, narchs, input); /* cctools-port */

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
	fprintf(stderr, "Usage: %s [-change old new] ... [-rpath old new] ... "
			"[-add_rpath new] ... [-delete_rpath old] ... "
			"[-id name] input"
		"\n", progname);
	exit(EXIT_FAILURE);
}

static
void
process(
struct arch *archs,
uint32_t narchs)
{
    uint32_t i;
    struct object *object;

	for(i = 0; i < narchs; i++){
	    if(archs[i].type == OFILE_Mach_O){
		object = archs[i].object;
		if(object->mh_filetype == MH_DYLIB_STUB)
		    fatal("input file: %s is Mach-O dynamic shared library stub"
			  " file and can't be changed", archs[i].file_name);
		update_load_commands(archs + i, arch_header_sizes + i);
	    }
	    else{
		error("input file: %s is not a Mach-O file",archs[i].file_name);
		return;
	    }
	}
}

/*
 * write_on_input() takes the modified archs and writes the load commands
 * directly into the input file.
 */
static
void
write_on_input(
struct arch *archs,
uint32_t narchs,
char *input)
{
    int fd;
    uint32_t i, size, headers_size;
    off_t offset;
    char *headers;
    struct mach_header *mh;
    struct mach_header_64 *mh64;
    struct load_command *lc;
    enum byte_sex host_byte_sex;

#if defined(__APPLE__) && __aarch64__ /* cctools-port: copy before modification */
        char tempfile[PATH_MAX];

	snprintf(tempfile, PATH_MAX, "%sXXXXXX", input);
	mktemp(tempfile);

	if (copyfile(input, tempfile, NULL, COPYFILE_ALL | COPYFILE_MOVE) == -1)
	  system_error("failed to copy %s for installation", input);

	fd = open(tempfile, O_WRONLY, 0);
#else
	fd = open(input, O_WRONLY, 0);
#endif

	host_byte_sex = get_host_byte_sex();

	if(fd == -1)
	    system_error("can't open input file: %s for writing", input);

	for(i = 0; i < narchs; i++){
	    if(archs[i].fat_arch64 != NULL)
		offset = archs[i].fat_arch64->offset;
	    else if(archs[i].fat_arch != NULL)
		offset = archs[i].fat_arch->offset;
	    else
		offset = 0;
	    if(lseek(fd, offset, SEEK_SET) == -1)
		system_error("can't lseek to offset: %lld in file: %s for "
			     "writing", offset, input);
	    /*
	     * Since the new headers may be smaller than the old headers and
	     * we want to make sure any old unused bytes are zero in the file
	     * we allocate the size of the original headers into a buffer and
	     * zero it out. Then copy the new headers into the buffer and write
	     * out the size of the original headers to the file.
	     */
	    if(archs[i].object->mh != NULL){
		headers_size = sizeof(struct mach_header) +
			       archs[i].object->mh->sizeofcmds;
	    }
	    else{
		headers_size = sizeof(struct mach_header_64) +
			       archs[i].object->mh64->sizeofcmds;
	    }
	    if(arch_header_sizes[i] > headers_size)
		size = arch_header_sizes[i];
	    else
		size = headers_size;
	    headers = allocate(size);
	    memset(headers, '\0', size);

	    if(archs[i].object->mh != NULL){
		mh = (struct mach_header *)headers;
		lc = (struct load_command *)(headers +
					     sizeof(struct mach_header));
		*mh = *(archs[i].object->mh);
		memcpy(lc, archs[i].object->load_commands, mh->sizeofcmds);
		if(archs[i].object->object_byte_sex != host_byte_sex)
		    if(swap_object_headers(mh, lc) == FALSE)
			fatal("internal error: swap_object_headers() failed");
	    }
	    else{
		mh64 = (struct mach_header_64 *)headers;
		lc = (struct load_command *)(headers +
					     sizeof(struct mach_header_64));
		*mh64 = *(archs[i].object->mh64);
		memcpy(lc, archs[i].object->load_commands, mh64->sizeofcmds);
		if(archs[i].object->object_byte_sex != host_byte_sex)
		    if(swap_object_headers(mh64, lc) == FALSE)
			fatal("internal error: swap_object_headers() failed");
	    }

	    if(write64(fd, headers, size) != (ssize_t)size)
		system_error("can't write new headers in file: %s", input);

	    free(headers);
	}
	if(close(fd) == -1)
	    system_error("can't close written on input file: %s", input);

#if defined(__APPLE__) && __aarch64__ /* cctools-port: copy before modification */
	if (rename(tempfile, input) == -1)
	  system_error("can't rename tempfile to %s", input);
#endif
}

/*
 * update_load_commands() changes the install names the LC_LOAD_DYLIB,
 * LC_LOAD_WEAK_DYLIB, LC_REEXPORT_DYLIB, LC_LOAD_UPWARD_DYLIB and
 * LC_PREBOUND_DYLIB commands for the specified arch.
 */
static
void
update_load_commands(
struct arch *arch,
uint32_t *header_size)
{
    uint32_t i, j, new_sizeofcmds, new_size, linked_modules_size, ncmds,
	     sizeof_mach_header, cmd_round;
    uint64_t low_fileoff;
    struct load_command *lc1, *lc2, *new_load_commands;
    struct dylib_command *dl_load1, *dl_load2, *dl_id1, *dl_id2;
    struct prebound_dylib_command *pbdylib1, *pbdylib2;
    char *dylib_name1, *dylib_name2, *arch_name, *linked_modules1,
	 *linked_modules2, *path1, *path2;
    struct segment_command *sg;
    struct segment_command_64 *sg64;
    struct section *s;
    struct section_64 *s64;
    struct arch_flag arch_flag;
    struct rpath_command *rpath1, *rpath2;
    enum bool delete;

	for(i = 0; i < nrpaths; i++)
	    rpaths[i].found = FALSE;
	for(i = 0; i < ndelete_rpaths; i++)
	    delete_rpaths[i].found = FALSE;

	/*
	 * Make a pass through the load commands and figure out what the new
	 * size of the the commands needs to be and how much room there is for
	 * them.
	 */
	if(arch->object->mh != NULL){
	    new_sizeofcmds = arch->object->mh->sizeofcmds;
	    ncmds = arch->object->mh->ncmds;
	    sizeof_mach_header = sizeof(struct mach_header);
	    cmd_round = 4;
	    arch_flag.cputype = arch->object->mh->cputype;
	    arch_flag.cpusubtype = arch->object->mh->cpusubtype;
	}
	else{
	    new_sizeofcmds = arch->object->mh64->sizeofcmds;
	    ncmds = arch->object->mh64->ncmds;
	    sizeof_mach_header = sizeof(struct mach_header_64);
	    cmd_round = 8;
	    arch_flag.cputype = arch->object->mh64->cputype;
	    arch_flag.cpusubtype = arch->object->mh64->cpusubtype;
	}
	set_arch_flag_name(&arch_flag);
	arch_name = arch_flag.name;

	low_fileoff = ULLONG_MAX;
	lc1 = arch->object->load_commands;
	for(i = 0; i < ncmds; i++){
	    switch(lc1->cmd){
	    case LC_ID_DYLIB:
		dl_id1 = (struct dylib_command *)lc1;
		dylib_name1 = (char *)dl_id1 + dl_id1->dylib.name.offset;
		if(id != NULL){
		    new_size = sizeof(struct dylib_command) +
			       rnd32((uint32_t)strlen(id) + 1, cmd_round);
		    new_sizeofcmds += (new_size - dl_id1->cmdsize);
		}
		break;

	    case LC_LOAD_DYLIB:
	    case LC_LOAD_WEAK_DYLIB:
	    case LC_REEXPORT_DYLIB:
	    case LC_LOAD_UPWARD_DYLIB:
	    case LC_LAZY_LOAD_DYLIB:
		dl_load1 = (struct dylib_command *)lc1;
		dylib_name1 = (char *)dl_load1 + dl_load1->dylib.name.offset;
		for(j = 0; j < nchanges; j++){
		    if(strcmp(changes[j].old, dylib_name1) == 0){
			new_size = sizeof(struct dylib_command) +
				   rnd32((int)strlen(changes[j].new) + 1,
					 cmd_round);
			new_sizeofcmds += (new_size - dl_load1->cmdsize);
			break;
		    }
		}
		break;

	    case LC_PREBOUND_DYLIB:
		pbdylib1 = (struct prebound_dylib_command *)lc1;
		dylib_name1 = (char *)pbdylib1 + pbdylib1->name.offset;
		for(j = 0; j < nchanges; j++){
		    if(strcmp(changes[j].old, dylib_name1) == 0){
			linked_modules_size = pbdylib1->cmdsize - (
				sizeof(struct prebound_dylib_command) +
				rnd32((int)strlen(dylib_name1) + 1, cmd_round));
			new_size = sizeof(struct prebound_dylib_command) +
				   rnd32((int)strlen(changes[j].new) + 1,
					 cmd_round) +
				   linked_modules_size;
			new_sizeofcmds += (new_size - pbdylib1->cmdsize);
			break;
		    }
		}
		break;

	    case LC_SEGMENT:
		sg = (struct segment_command *)lc1;
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
		break;

	    case LC_SEGMENT_64:
		sg64 = (struct segment_command_64 *)lc1;
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
			low_fileoff = sg64->fileoff;
		}
		break;

	    case LC_RPATH:
		rpath1 = (struct rpath_command *)lc1;
		path1 = (char *)rpath1 + rpath1->path.offset;
		for(j = 0; j < nadd_rpaths; j++){
		    if(strcmp(add_rpaths[j].new, path1) == 0){
			error("for: %s (for architecture %s) option "
			      "\"-add_rpath %s\" would duplicate path, file "
			      "already has LC_RPATH for: %s", arch->file_name,
			      arch_name, add_rpaths[j].new, path1);
		    }
		}
		for(j = 0; j < nrpaths; j++){
		    if(strcmp(rpaths[j].old, path1) == 0){
			if(rpaths[j].found == TRUE)
			    break;
			rpaths[j].found = TRUE;
			new_size = rnd32(sizeof(struct rpath_command) +
				         (int)strlen(rpaths[j].new) + 1,
					 cmd_round);
			new_sizeofcmds += (new_size - rpath1->cmdsize);
			break;
		    }
		}
		for(j = 0; j < ndelete_rpaths; j++){
		    if(strcmp(delete_rpaths[j].old, path1) == 0){
			if(delete_rpaths[j].found == TRUE)
			    break;
			delete_rpaths[j].found = TRUE;
			new_sizeofcmds -= rpath1->cmdsize;
			break;
		    }
		}
		break;
	    }
	    lc1 = (struct load_command *)((char *)lc1 + lc1->cmdsize);
	}

	for(i = 0; i < ndelete_rpaths; i++){
	    if(delete_rpaths[i].found == FALSE){
		error("no LC_RPATH load command with path: %s found in: "
		      "%s (for architecture %s), required for specified option "
		      "\"-delete_rpath %s\"", delete_rpaths[i].old,
		      arch->file_name, arch_name, delete_rpaths[i].old);
	    }
	    delete_rpaths[i].found = FALSE;
	}
	for(i = 0; i < nrpaths; i++){
	    if(rpaths[i].found == FALSE){
		error("no LC_RPATH load command with path: %s found in: "
		      "%s (for architecture %s), required for specified option "
		      "\"-rpath %s %s\"", rpaths[i].old, arch->file_name,
		      arch_name, rpaths[i].old, rpaths[i].new);
	    }
	    rpaths[i].found = FALSE;
	}

	for(i = 0; i < nadd_rpaths; i++){
	    new_size = rnd32(sizeof(struct rpath_command) +
			     (int)strlen(add_rpaths[i].new) + 1, cmd_round);
	    new_sizeofcmds += new_size;
	}

	if(new_sizeofcmds + sizeof_mach_header > low_fileoff){
	    error("changing install names or rpaths can't be redone for: %s "
		  "(for architecture %s) because larger updated load commands "
		  "do not fit (the program must be relinked, and you may need "
		  "to use -headerpad or -headerpad_max_install_names)",
		  arch->file_name, arch_name);
	    return;
	}

	/*
	 * Allocate space for the new load commands and zero it out so any holes
	 * will be zero bytes.  Note this may be smaller than the original size
	 * of the load commands.
	 */
	new_load_commands = allocate(new_sizeofcmds);
	memset(new_load_commands, '\0', new_sizeofcmds);

	/*
	 * Fill in the new load commands by copying in the non-modified
	 * commands and updating ones with install name changes.
	 */
	lc1 = arch->object->load_commands;
	lc2 = new_load_commands;
	for(i = 0; i < ncmds; i++){
	    delete = FALSE;
	    switch(lc1->cmd){
	    case LC_ID_DYLIB:
		if(id != NULL){
		    memcpy(lc2, lc1, sizeof(struct dylib_command));
		    dl_id2 = (struct dylib_command *)lc2;
		    dl_id2->cmdsize = sizeof(struct dylib_command) +
				     rnd32((int)strlen(id) + 1, cmd_round);
		    dl_id2->dylib.name.offset = sizeof(struct dylib_command);
		    dylib_name2 = (char *)dl_id2 + dl_id2->dylib.name.offset;
		    strcpy(dylib_name2, id);
		}
		else{
		    memcpy(lc2, lc1, lc1->cmdsize);
		}
		break;

	    case LC_LOAD_DYLIB:
	    case LC_LOAD_WEAK_DYLIB:
	    case LC_REEXPORT_DYLIB:
	    case LC_LOAD_UPWARD_DYLIB:
	    case LC_LAZY_LOAD_DYLIB:
		dl_load1 = (struct dylib_command *)lc1;
		dylib_name1 = (char *)dl_load1 + dl_load1->dylib.name.offset;
		for(j = 0; j < nchanges; j++){
		    if(strcmp(changes[j].old, dylib_name1) == 0){
			memcpy(lc2, lc1, sizeof(struct dylib_command));
			dl_load2 = (struct dylib_command *)lc2;
			dl_load2->cmdsize = sizeof(struct dylib_command) +
					    rnd32((int)strlen(changes[j].new)+1,
						  cmd_round);
			dl_load2->dylib.name.offset =
			    sizeof(struct dylib_command);
			dylib_name2 = (char *)dl_load2 +
				      dl_load2->dylib.name.offset;
			strcpy(dylib_name2, changes[j].new);
			break;
		    }
		}
		if(j >= nchanges){
		    memcpy(lc2, lc1, lc1->cmdsize);
		}
		break;

	    case LC_PREBOUND_DYLIB:
		pbdylib1 = (struct prebound_dylib_command *)lc1;
		dylib_name1 = (char *)pbdylib1 + pbdylib1->name.offset;
		for(j = 0; j < nchanges; j++){
		    if(strcmp(changes[j].old, dylib_name1) == 0){
			memcpy(lc2, lc1, sizeof(struct prebound_dylib_command));
			pbdylib2 = (struct prebound_dylib_command *)lc2;
			linked_modules_size = pbdylib1->cmdsize - (
			    sizeof(struct prebound_dylib_command) +
			    rnd32((int)strlen(dylib_name1) + 1, cmd_round));
			pbdylib2->cmdsize =
			    sizeof(struct prebound_dylib_command) +
			    rnd32((int)strlen(changes[j].new) + 1, cmd_round) +
			    linked_modules_size;

			pbdylib2->name.offset =
			    sizeof(struct prebound_dylib_command);
			dylib_name2 = (char *)pbdylib2 +
				      pbdylib2->name.offset;
			strcpy(dylib_name2, changes[j].new);
			
			pbdylib2->linked_modules.offset = 
			    sizeof(struct prebound_dylib_command) +
			    rnd32((int)strlen(changes[j].new) + 1, cmd_round);
			linked_modules1 = (char *)pbdylib1 +
					  pbdylib1->linked_modules.offset;
			linked_modules2 = (char *)pbdylib2 +
					  pbdylib2->linked_modules.offset;
			memcpy(linked_modules2, linked_modules1,
			       linked_modules_size);
			break;
		    }
		}
		if(j >= nchanges){
		    memcpy(lc2, lc1, lc1->cmdsize);
		}
		break;

	    case LC_RPATH:
		rpath1 = (struct rpath_command *)lc1;
		path1 = (char *)rpath1 + rpath1->path.offset;
		for(j = 0; j < ndelete_rpaths; j++){
		    if(strcmp(delete_rpaths[j].old, path1) == 0){
			if(delete_rpaths[j].found == TRUE)
			    break;
			delete_rpaths[j].found = TRUE;
			delete = TRUE;
			break;
		    }
		}
		if(delete == TRUE)
		    break;
		for(j = 0; j < nrpaths; j++){
		    if(strcmp(rpaths[j].old, path1) == 0){
			if(rpaths[j].found == TRUE){
			    memcpy(lc2, lc1, lc1->cmdsize);
			    break;
			}
			rpaths[j].found = TRUE;
			memcpy(lc2, lc1, sizeof(struct rpath_command));
			rpath2 = (struct rpath_command *)lc2;
			rpath2->cmdsize = rnd32(sizeof(struct rpath_command) +
					        (int)strlen(rpaths[j].new) + 1,
						cmd_round);
			rpath2->path.offset = sizeof(struct rpath_command);
			path2 = (char *)rpath2 + rpath2->path.offset;
			strcpy(path2, rpaths[j].new);
			break;
		    }
		}
		if(j >= nrpaths){
		    memcpy(lc2, lc1, lc1->cmdsize);
		}
		break;

	    default:
		memcpy(lc2, lc1, lc1->cmdsize);
		break;
	    }
	    lc1 = (struct load_command *)((char *)lc1 + lc1->cmdsize);
	    if(delete == FALSE)
		lc2 = (struct load_command *)((char *)lc2 + lc2->cmdsize);
	}
	/*
	 * Add the new rpath load commands.
	 */
	for(i = 0; i < nadd_rpaths; i++){
	    rpath2 = (struct rpath_command *)lc2;
	    rpath2->cmd = LC_RPATH;
	    rpath2->cmdsize = rnd32(sizeof(struct rpath_command) +
				    (int)strlen(add_rpaths[i].new) + 1,
				    cmd_round);
	    rpath2->path.offset = sizeof(struct rpath_command);
	    path2 = (char *)rpath2 + rpath2->path.offset;
	    strcpy(path2, add_rpaths[i].new);
	    lc2 = (struct load_command *)((char *)lc2 + lc2->cmdsize);
	}
	ncmds += nadd_rpaths;
	ncmds -= ndelete_rpaths;

	/*
	 * Finally copy the updated load commands over the existing load
	 * commands. Since the headers could be smaller we save away the old
	 * header_size (for use when writing on the input) and also put zero
	 * bytes on the part that is no longer used for headers.
	 */
	if(arch->object->mh != NULL){
	    *header_size = sizeof(struct mach_header) +
			   arch->object->mh->sizeofcmds;
	    if(new_sizeofcmds < arch->object->mh->sizeofcmds){
		memset(((char *)arch->object->load_commands) + new_sizeofcmds,
		       '\0', arch->object->mh->sizeofcmds - new_sizeofcmds);
	    }
	    memcpy(arch->object->load_commands, new_load_commands,
		   new_sizeofcmds);
	    arch->object->mh->sizeofcmds = new_sizeofcmds;
	    arch->object->mh->ncmds = ncmds;
	}
	else{
	    *header_size = sizeof(struct mach_header_64) +
			   arch->object->mh64->sizeofcmds;
	    if(new_sizeofcmds < arch->object->mh64->sizeofcmds){
		memset(((char *)arch->object->load_commands) + new_sizeofcmds,
		       '\0', arch->object->mh64->sizeofcmds - new_sizeofcmds);
	    }
	    memcpy(arch->object->load_commands, new_load_commands,
		   new_sizeofcmds);
	    arch->object->mh64->sizeofcmds = new_sizeofcmds;
	    arch->object->mh64->ncmds = ncmds;
	}

	free(new_load_commands);

	/* reset the pointers into the load commands */
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
	    case LC_SEGMENT_64:
		sg64 = (struct segment_command_64 *)lc1;
		if(strcmp(sg64->segname, SEG_LINKEDIT) == 0)
		    arch->object->seg_linkedit64 = sg64;
		break;
	    case LC_CODE_SIGNATURE:
		arch->object->code_sig_cmd =
		    (struct linkedit_data_command *)lc1;
		break;
	    case LC_SEGMENT_SPLIT_INFO:
		arch->object->split_info_cmd =
		    (struct linkedit_data_command *)lc1;
		break;
	    case LC_FUNCTION_STARTS:
		arch->object->func_starts_info_cmd =
		    (struct linkedit_data_command *)lc1;
		break;
	    case LC_DATA_IN_CODE:
		arch->object->data_in_code_cmd =
		    (struct linkedit_data_command *)lc1;
		break;
	    case LC_DYLIB_CODE_SIGN_DRS:
		arch->object->code_sign_drs_cmd =
		    (struct linkedit_data_command *)lc1;
		break;
	    case LC_LINKER_OPTIMIZATION_HINT:
		arch->object->link_opt_hint_cmd =
		    (struct linkedit_data_command *)lc1;
		break;
	    case LC_DYLD_CHAINED_FIXUPS:
		arch->object->dyld_chained_fixups =
		    (struct linkedit_data_command *)lc1;
		break;
	    case LC_DYLD_EXPORTS_TRIE:
		arch->object->dyld_exports_trie =
		    (struct linkedit_data_command *)lc1;
		break;
	    }
	    lc1 = (struct load_command *)((char *)lc1 + lc1->cmdsize);
	}
}
