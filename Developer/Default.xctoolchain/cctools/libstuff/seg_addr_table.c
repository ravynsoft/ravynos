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
/* NOTE: This is no longer compiled into libstuff as of 2/27/2019 */
/* cctools-port: We still use this. */
#ifndef RLD
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <mach/mach.h>
#include <libc.h>

#include "stuff/bool.h"
#include "stuff/errors.h"
#include "stuff/allocate.h"
#include "stuff/guess_short_name.h"
#include "stuff/seg_addr_table.h"

/*
 * parse_default_seg_addr_table() simply calls parse_seg_addr_table() with
 * the default file that contains the segment address table.
 */
struct seg_addr_table *
parse_default_seg_addr_table(
char **seg_addr_table_name,
uint32_t *table_size)
{
#ifdef __GONZO_BUNSEN_BEAKER__
	*seg_addr_table_name = "/Local/Developer/seg_addr_table";
#else
	*seg_addr_table_name = "/AppleInternal/Developer/seg_addr_table";
#endif
	return(parse_seg_addr_table(*seg_addr_table_name,
				"default", "segment address table", table_size));
}

/*
 * parse_seg_addr_table() opens the file_name passed to it and parses it as a
 * segment address table.  The flag and argument parameters are used for
 * error messages if there is a problem parsing the file.  The file should
 * contains lines of the form:
 *	<hex address> <install name>
 * or
 *	<hex address> <hex address> <install name>
 * The first form with one <hex address> specifies the -seg1addr of the
 * dynamic shared library with the specified <install name>.  The second form
 * specifies the -segs_read_only_addr and the -segs_read_write_addr addresses
 * of the dynamic shared library.  The fields are to be separated by spaces or
 * tabs.  Comment lines starting with a '#' character are ignored as well as
 * lines with just spaces and tabs.  This routine returns the parsed table as an
 * array of seg_addr_table structs.  The last entry in the table has a NULL
 * install_name.  For lines with two addresses the field split will be TRUE
 * otherwise it will be FALSE.
 */
struct seg_addr_table *
parse_seg_addr_table(
char *file_name,/* file name of the seg_addr_table file */
char *flag,	/* "-seg_addr_table" or "default" */
char *argument, /* -seg_addr_table argument or "segment address table" */
uint32_t *table_size)
{
    int fd;
    struct stat stat_buf;
    uint32_t j, k, file_size, seg_addr_table_size, line;
    char *file_addr, *endp;
    struct seg_addr_table *new_seg_addr_table;

	if((fd = open(file_name, O_RDONLY, 0)) == -1)
	    system_fatal("Can't open: %s for %s %s",
		    file_name, flag, argument);
	if(fstat(fd, &stat_buf) == -1)
	    system_fatal("Can't stat file: %s for %s %s",
		    file_name, flag, argument);
	/*
	 * For some reason mapping files with zero size fails
	 * so it has to be handled specially. Also, deal with files that are
	 * too large to be valid seg_addr tables.
	 */
	file_addr = NULL;
	if (stat_buf.st_size > 0xFFFFFFFF) {
	    fatal("File too large (%llu): %s for %s %s", stat_buf.st_size,
		  file_name, flag, argument);
	}
	else if (stat_buf.st_size == 0) {
	    fatal("Empty file: %s for %s %s", file_name, flag, argument);
	}
	else {
	    file_addr = mmap(0, stat_buf.st_size, PROT_READ|PROT_WRITE,
			     MAP_FILE|MAP_PRIVATE, fd, 0);
	    if((intptr_t)file_addr == -1)
		system_error("can't map file: %s for %s %s", file_name, flag,
			     argument);
	}
	close(fd);
	file_size = (uint32_t)stat_buf.st_size;

	/*
	 * Got the file mapped now parse it.
	 */
	if(file_addr[file_size - 1] != '\n')
	    fatal("file: %s for %s %s does not end in new line",
		  file_name, flag, argument);
	seg_addr_table_size = 0;
	for(j = 1; j < file_size; j++){
	    if(file_addr[j] == '\n'){
		seg_addr_table_size++;
	    }
	}
	seg_addr_table_size++;
	new_seg_addr_table = allocate(sizeof(struct seg_addr_table) *
				      seg_addr_table_size);
	k = 0;
	line = 1;
	for(j = 0; j < file_size; /* no increment expression */ ){
	    /* Skip lines that start with '#' */
	    if(file_addr[j] == '#'){
		j++;
		while(file_addr[j] != '\n')
		    j++;
		continue;
	    }
	    /* Skip blank lines */
	    while(file_addr[j] == ' ' || file_addr[j] == '\t')
		j++;
	    if(file_addr[j] == '\n'){
		j++;
		line++;
		continue;
	    }
	    new_seg_addr_table[k].seg1addr =
		(uint32_t)strtoul(file_addr + j, &endp, 16);
	    if(endp == NULL)
		fatal("improper hexadecimal number on line %u in "
		      "file: %s for %s %s", j, file_name, flag, argument);
	    j = (uint32_t)(endp - file_addr);
	    if(j == file_size)
		fatal("missing library install name on line %u in file: "
		      "%s for %s %s", j, file_name, flag, argument);
	    /*
	     * Since we checked to see the file ends in a '\n' we can
	     * be assured this won't run off the end of the file.
	     */
	    while(file_addr[j] == ' ' || file_addr[j] == '\t')
		j++;
	    if(file_addr[j] == '\n')
		fatal("missing library install name on line %u in file: "
		      "%s for %s %s", j, file_name, flag, argument);

	    new_seg_addr_table[k].segs_read_write_addr =
		(uint32_t)strtoul(file_addr + j, &endp, 16);
	    if(endp == NULL || endp == file_addr + j){
		new_seg_addr_table[k].split = FALSE;
		new_seg_addr_table[k].segs_read_write_addr = UINT_MAX;
	    }
	    else{
		j = (uint32_t)(endp - file_addr);
		new_seg_addr_table[k].split = TRUE;
		new_seg_addr_table[k].segs_read_only_addr =
		    new_seg_addr_table[k].seg1addr;
		new_seg_addr_table[k].seg1addr =  UINT_MAX;
		while(file_addr[j] == ' ' || file_addr[j] == '\t')
		    j++;
	    }

	    new_seg_addr_table[k].install_name = file_addr + j;
	    new_seg_addr_table[k].line = line;
	    k++;
	    while(file_addr[j] != '\n')
		j++;
	    file_addr[j] = '\0';
	    line++;
	    j++;
	}

	new_seg_addr_table[k].install_name = NULL;
	new_seg_addr_table[k].seg1addr = 0;
	new_seg_addr_table[k].split = FALSE;
	new_seg_addr_table[k].segs_read_only_addr = UINT_MAX;
	new_seg_addr_table[k].segs_read_write_addr = UINT_MAX;
	new_seg_addr_table[k].line = line;

	*table_size = k;
	return(new_seg_addr_table);
}

/*
 * search_seg_addr_table() searches the specified segment address table for the
 * specified name and returns the entry to it if it is found.  If it is not
 * found NULL is returned.
 */
struct seg_addr_table *
search_seg_addr_table(
struct seg_addr_table *seg_addr_table,
char *install_name)
{
    struct seg_addr_table *p;

	if(seg_addr_table == NULL)
	    return(NULL);

	for(p = seg_addr_table; p->install_name != NULL; p++){
	    if(strcmp(p->install_name, install_name) == 0)
		return(p);
	}
	return(NULL);
}

/*
 * process_seg_addr_table() is used to create an new segment address table from
 * an existing segment address table.  It opens and parses "file_name" as a
 * segment address table.  It copies the comment lines that don't start with
 * "comment_prefix" to "out_fp".  For entries in the file it calls the function
 * processor() pass to it and passes it the parsed entry from the file, the
 * "out_fp" and the "cookie" passed to process_seg_addr_table().
 */
void
process_seg_addr_table(
char *file_name,
FILE *out_fp,
char *comment_prefix,
void (*processor)(struct seg_addr_table *entry, FILE *out_fp, void *cookie),
void *cookie)
{
    int fd;
    struct stat stat_buf;
    uint32_t i, file_size, line;
    size_t comment_prefix_length;
    char *file_addr, *endp;
    struct seg_addr_table entry;

	if((fd = open(file_name, O_RDONLY, 0)) == -1)
	    system_fatal("can't open file: %s", file_name);
	if(fstat(fd, &stat_buf) == -1)
	    system_fatal("can't stat file: %s", file_name);
	file_addr = NULL;
	if (stat_buf.st_size > 0xFFFFFFFF) {
	    fatal("File too large (%llu): %s", stat_buf.st_size,
		  file_name);
	}
	else if (stat_buf.st_size == 0) {
	    fatal("Empty file: %s", file_name);
	}
	else {
	    file_addr = mmap(0, stat_buf.st_size, PROT_READ|PROT_WRITE,
			     MAP_FILE|MAP_PRIVATE, fd, 0);
	    if((intptr_t)file_addr == -1)
		system_error("can't map file: %s", file_name);
	}
	close(fd);
	file_size = (uint32_t)stat_buf.st_size;

	/*
	 * Got the file mapped now parse and process it.
	 */
	if(file_addr[file_size - 1] != '\n')
	    fatal("file: %s does not end in new line", file_name);

	line = 1;
	comment_prefix_length = strlen(comment_prefix);
	for(i = 0; i < file_size; /* no increment expression */ ){
	    /* Copy comment lines that start with '#' */
	    if(file_addr[i] == '#'){
		if(strncmp(comment_prefix, file_addr + i + 1,
			   comment_prefix_length) == 0){
		    i++;
		    while(file_addr[i] != '\n')
			i++;
		    if(file_addr[i] == '\n'){
			i++;
			line++;
		    }
		    continue;
		}
		fputc(file_addr[i], out_fp);
		i++;
		while(file_addr[i] != '\n'){
		    fputc(file_addr[i], out_fp);
		    i++;
		}
		continue;
	    }
	    /* Copy blank lines */
	    while(file_addr[i] == ' ' || file_addr[i] == '\t'){
		fputc(file_addr[i], out_fp);
		i++;
	    }
	    if(file_addr[i] == '\n'){
		fputc(file_addr[i], out_fp);
		i++;
		line++;
		continue;
	    }
	    entry.seg1addr = (uint32_t)strtoul(file_addr + i, &endp, 16);
	    if(endp == NULL)
		fatal("improper hexadecimal number on line %u in file: %s",
		      line, file_name);
	    i = (uint32_t)(endp - file_addr);
	    if(i == file_size)
		fatal("missing library install name on line %u in file: %s",
		      line, file_name);
	    /*
	     * Since we checked to see the file ends in a '\n' we can
	     * be assured this won't run off the end of the file.
	     */
	    while(file_addr[i] == ' ' || file_addr[i] == '\t')
		i++;
	    if(file_addr[i] == '\n')
		fatal("missing library install name on line %u in file: %s",
		      line, file_name);

	    entry.segs_read_write_addr = (uint32_t)strtoul(file_addr + i, &endp, 16);
	    if(endp == NULL || endp == file_addr + i){
		entry.split = FALSE;
		entry.segs_read_write_addr = UINT_MAX;
	    }
	    else{
		i = (uint32_t)(endp - file_addr);
		entry.split = TRUE;
		entry.segs_read_only_addr = entry.seg1addr;
		entry.seg1addr = UINT_MAX;
		while(file_addr[i] == ' ' || file_addr[i] == '\t')
		    i++;
	    }

	    entry.install_name = file_addr + i;
	    entry.line = line;
	    while(file_addr[i] != '\n')
		i++;
	    file_addr[i] = '\0';

	    processor(&entry, out_fp, cookie);

	    line++;
	    i++;
	}
}
#endif /* !defined(RLD) */
