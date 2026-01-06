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
#ifndef RLD
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stuff/symbol_list.h>
#include <stuff/allocate.h>
#include <stuff/errors.h>

static int cmp_qsort_name(
    const struct symbol_list *sym1,
    const struct symbol_list *sym2);

/*
 * This is called to setup a symbol list from a file.  It reads the file with
 * the strings in it and places them in an array of symbol_list structures and
 * then sorts them by name.
 *
 * The file that contains the symbol names must have symbol names one per line,
 * leading and trailing white space is removed and lines starting with a '#'
 * and lines with only white space are ignored.
 */
__private_extern__
void
setup_symbol_list(
char *file,
struct symbol_list **list,
uint32_t *size)
{
    int fd;
    uint32_t i, j, strings_size;
    size_t len;
    struct stat stat_buf;
    char *strings, *p, *line;

	if((fd = open(file, O_RDONLY)) < 0){
	    system_error("can't open: %s", file);
	    return;
	}
	if(fstat(fd, &stat_buf) == -1){
	    system_error("can't stat: %s", file);
	    close(fd);
	    return;
	}
	strings_size = (uint32_t)stat_buf.st_size;
	strings = (char *)allocate(strings_size + 2);
	strings[strings_size] = '\n';
	strings[strings_size + 1] = '\0';
	if(read(fd, strings, strings_size) != (int)strings_size){
	    system_error("can't read: %s", file);
	    close(fd);
	    return;
	}
	/*
	 * Change the newlines to '\0' and count the number of lines with
	 * symbol names.  Lines starting with '#' are comments and lines
	 * contain all space characters do not contain symbol names.
	 */
	p = strings;
	line = p;
	for(i = 0; i < strings_size + 1; i++){
	    if(*p == '\n' || *p == '\r'){
		*p = '\0';
		if(*line != '#'){
		    while(*line != '\0' && isspace(*line))
			line++;
		    if(*line != '\0')
			(*size)++;
		}
		p++;
		line = p;
	    }
	    else{
		p++;
	    }
	}
	*list = (struct symbol_list *)
		allocate((*size) * sizeof(struct symbol_list));

	/*
	 * Place the strings in the list trimming leading and trailing spaces
	 * from the lines with symbol names.
	 */
	p = strings;
	line = p;
	for(i = 0; i < (*size); ){
	    p += strlen(p) + 1;
	    if(*line != '#' && *line != '\0'){
		while(*line != '\0' && isspace(*line))
		    line++;
		if(*line != '\0'){
		    (*list)[i].name = line;
		    (*list)[i].seen = FALSE;
		    i++;
		    len = strlen(line);
		    j = (uint32_t)(len - 1);
		    while(j > 0 && isspace(line[j])){
			j--;
		    }
		    if(j > 0 && j + 1 < len && isspace(line[j+1]))
			line[j+1] = '\0';
		}
	    }
	    line = p;
	}

	qsort(*list, *size, sizeof(struct symbol_list),
	      (int (*)(const void *, const void *))cmp_qsort_name);

	/* remove duplicates on the list */
	for(i = 0; i < (*size); i++){
	    if(i + 1 < (*size)){
		if(strcmp((*list)[i].name, (*list)[i+1].name) == 0){
		    for(j = 1; j < ((*size) - i - 1); j++){
			(*list)[i + j].name = (*list)[i + j + 1].name;
		    }
		    *size = *size - 1;
		    /*
		     * Since there may be more than two of the same name
		     * check this one again against the next one in the
		     * list before moving on.
		     */
                    i--; 
		}
	    }
	}

#ifdef DEBUG
	printf("symbol list:\n");
	for(i = 0; i < (*size); i++){
	    printf("0x%p name = %s\n", &((*list)[i]),(*list)[i].name);
	}
#endif /* DEBUG */
}

/*
 * Function for qsort for comparing symbol list names.
 */
static
int
cmp_qsort_name(
const struct symbol_list *sym1,
const struct symbol_list *sym2)
{
	return(strcmp(sym1->name, sym2->name));
}

/*
 * Function for bsearch for finding a symbol name.
 */
__private_extern__
int
symbol_list_bsearch(
const char *name,
const struct symbol_list *sym)
{
	return(strcmp(name, sym->name));
}
#endif /* !defined(RLD) */
