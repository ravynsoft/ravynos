/*

  Copyright (C) 2005 Silicon Graphics, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2.1 of the GNU Lesser General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it would be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  Further, this software is distributed without any warranty that it is
  free of the rightful claim of any third person regarding infringement
  or the like.  Any license provided herein, whether implied or
  otherwise, applies only to this software file.  Patent licenses, if
  any, provided herein do not apply to combinations of this program with
  other software, or any other product whatsoever.

  You should have received a copy of the GNU Lesser General Public
  License along with this program; if not, write the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307,
  USA.

  Contact information:  Silicon Graphics, Inc., 1500 Crittenden Lane,
  Mountain View, CA 94043, or:

  http://www.sgi.com

  For further information regarding this notice, see:

  http://oss.sgi.com/projects/GenInfo/NoticeExplan

*/



/* malloc_check.c For checking dealloc completeness. 

   This code is as simple as possible and works ok for
   reasonable size allocation counts.

   It treats allocation as global, and so will not
   work very well if an application opens more than one
   Dwarf_Debug.

*/

#include <stdio.h>
#include <stdlib.h>		/* for exit() and various malloc
				   prototypes */
#include "config.h"
#include "dwarf_incl.h"
#include "malloc_check.h"
#ifdef  WANT_LIBBDWARF_MALLOC_CHECK

/* To turn off printing every entry, just change the define
   to set PRINT_MALLOC_DETAILS 0.
*/
#define PRINT_MALLOC_DETAILS 0

#define MC_TYPE_UNKNOWN 0
#define MC_TYPE_ALLOC 1
#define MC_TYPE_DEALLOC 2

struct mc_data_s {
    struct mc_data_s *mc_prev;
    unsigned long mc_address;	/* Assumes this is large enough to hold 
				   a pointer! */

    long mc_alloc_number;	/* Assigned in order by when record
				   created. */
    unsigned char mc_alloc_code;	/* Allocation code, libdwarf. */
    unsigned char mc_type;
    unsigned char mc_dealloc_noted;	/* Used on an ALLOC node. */
    unsigned char mc_dealloc_noted_count;	/* Used on an ALLOC
						   node. */
};

/* 
   
   
*/
#define HASH_TABLE_SIZE 10501
static struct mc_data_s *mc_data_hash[HASH_TABLE_SIZE];
static long mc_data_list_size = 0;

static char *alloc_type_name[MAX_DW_DLA + 1] = {
    "",
    "DW_DLA_STRING",
    "DW_DLA_LOC",
    "DW_DLA_LOCDESC",
    "DW_DLA_ELLIST",
    "DW_DLA_BOUNDS",
    "DW_DLA_BLOCK",
    "DW_DLA_DEBUG",
    "DW_DLA_DIE",
    "DW_DLA_LINE",
    "DW_DLA_ATTR",
    "DW_DLA_TYPE",
    "DW_DLA_SUBSCR",
    "DW_DLA_GLOBAL",
    "DW_DLA_ERROR",
    "DW_DLA_LIST",
    "DW_DLA_LINEBUF",
    "DW_DLA_ARANGE",
    "DW_DLA_ABBREV",
    "DW_DLA_FRAME_OP",
    "DW_DLA_CIE",
    "DW_DLA_FDE",
    "DW_DLA_LOC_BLOCK",
    "DW_DLA_FRAME_BLOCK",
    "DW_DLA_FUNC",
    "DW_DLA_TYPENAME",
    "DW_DLA_VAR",
    "DW_DLA_WEAK",
    "DW_DLA_ADDR",
    "DW_DLA_ABBREV_LIST",
    "DW_DLA_CHAIN",
    "DW_DLA_CU_CONTEXT",
    "DW_DLA_FRAME",
    "DW_DLA_GLOBAL_CONTEXT",
    "DW_DLA_FILE_ENTRY",
    "DW_DLA_LINE_CONTEXT",
    "DW_DLA_LOC_CHAIN",
    "DW_DLA_HASH_TABLE",
    "DW_DLA_FUNC_CONTEXT",
    "DW_DLA_TYPENAME_CONTEXT",
    "DW_DLA_VAR_CONTEXT",
    "DW_DLA_WEAK_CONTEXT",
    "DW_DLA_PUBTYPES_CONTEXT"
	/* Don't forget to expand this list if the list of codes
	   expands. */
};

static unsigned
hash_address(unsigned long addr)
{
    unsigned long a = addr >> 2;

    return a % HASH_TABLE_SIZE;
}

#if PRINT_MALLOC_DETAILS
static void
print_alloc_dealloc_detail(unsigned long addr,
			   int code, char *whichisit)
{
    fprintf(stderr,
	    "%s  addr 0x%lx code %d (%s) entry %ld\n",
	    whichisit, addr, code, alloc_type_name[code],
	    mc_data_list_size);
}
#else
#define  print_alloc_dealloc_detail(a,b,c)	/* nothing */
#endif

/* Create a zeroed struct or die. */
static void *
newone(void)
{
    struct mc_data_s *newd = malloc(sizeof(struct mc_data_s));

    if (newd == 0) {
	fprintf(stderr, "out of memory , # %ld\n", mc_data_list_size);
	exit(1);
    }
    memset(newd, 0, sizeof(struct mc_data_s));
    return newd;
}

/* Notify checker that get_alloc has allocated user data. */
void
dwarf_malloc_check_alloc_data(void *addr_in, unsigned char code)
{
    struct mc_data_s *newd = newone();
    unsigned long addr = (unsigned long) addr_in;
    struct mc_data_s **base = &mc_data_hash[hash_address(addr)];

    print_alloc_dealloc_detail(addr, code, "alloc   ");
    newd->mc_address = addr;
    newd->mc_alloc_code = code;
    newd->mc_type = MC_TYPE_ALLOC;
    newd->mc_alloc_number = mc_data_list_size;
    newd->mc_prev = *base;
    *base = newd;
    newd->mc_alloc_number = mc_data_list_size;
    mc_data_list_size += 1;
}

static void
print_entry(char *msg, struct mc_data_s *data)
{
    fprintf(stderr,
	    "%s: 0x%08lx code %2d (%s) type %s dealloc noted %u ct %u\n",
	    msg,
	    (long) data->mc_address,
	    data->mc_alloc_code,
	    alloc_type_name[data->mc_alloc_code],
	    (data->mc_type == MC_TYPE_ALLOC) ? "alloc  " :
	    (data->mc_type == MC_TYPE_DEALLOC) ? "dealloc" : "unknown",
	    (unsigned) data->mc_dealloc_noted,
	    (unsigned) data->mc_dealloc_noted_count);
}

/* newd is a 'dealloc'.
*/
static long
balanced_by_alloc_p(struct mc_data_s *newd,
		    long *addr_match_num,
		    struct mc_data_s **addr_match,
		    struct mc_data_s *base)
{
    struct mc_data_s *cur = base;

    for (; cur; cur = cur->mc_prev) {
	if (cur->mc_address == newd->mc_address) {
	    if (cur->mc_type == MC_TYPE_ALLOC) {
		if (cur->mc_alloc_code == newd->mc_alloc_code) {
		    *addr_match = cur;
		    *addr_match_num = cur->mc_alloc_number;
		    return cur->mc_alloc_number;
		} else {
		    /* code mismatch */
		    *addr_match = cur;
		    *addr_match_num = cur->mc_alloc_number;
		    return -1;
		}
	    } else {
		/* Unbalanced new/del */
		*addr_match = cur;
		*addr_match_num = cur->mc_alloc_number;
		return -1;
	    }
	}
    }
    return -1;
}

/*  A dealloc is to take place. Ensure it balances an alloc.
*/
void
dwarf_malloc_check_dealloc_data(void *addr_in, unsigned char code)
{
    struct mc_data_s *newd = newone();
    long prev;
    long addr_match_num = -1;
    struct mc_data_s *addr_match = 0;
    unsigned long addr = (unsigned long) addr_in;
    struct mc_data_s **base = &mc_data_hash[hash_address(addr)];


    print_alloc_dealloc_detail(addr, code, "dealloc ");
    newd->mc_address = (unsigned long) addr;
    newd->mc_alloc_code = code;
    newd->mc_type = MC_TYPE_DEALLOC;
    newd->mc_prev = *base;
    prev =
	balanced_by_alloc_p(newd, &addr_match_num, &addr_match, *base);
    if (prev < 0) {
	fprintf(stderr,
		"Unbalanced dealloc at index %ld\n", mc_data_list_size);
	print_entry("new", newd);
	fprintf(stderr, "addr-match_num? %ld\n", addr_match_num);
	if (addr_match) {
	    print_entry("prev entry", addr_match);
	    if (addr_match->mc_dealloc_noted > 1) {
		fprintf(stderr, "Above is Duplicate dealloc!\n");
	    }
	}
	abort();
	exit(3);
    }
    addr_match->mc_dealloc_noted = 1;
    addr_match->mc_dealloc_noted_count += 1;
    if (addr_match->mc_dealloc_noted_count > 1) {
	fprintf(stderr, "Double dealloc entry %ld\n", addr_match_num);
	print_entry("new dealloc entry", newd);
	print_entry("bad alloc entry", addr_match);
    }
    *base = newd;
    mc_data_list_size += 1;
}

/* Final check for leaks.
*/
void
dwarf_malloc_check_complete(char *msg)
{
    long i = 0;
    long total = mc_data_list_size;
    long hash_slots_used = 0;
    long max_chain_length = 0;

    fprintf(stderr, "Run complete, %s. %ld entries\n", msg, total);
    for (; i < HASH_TABLE_SIZE; ++i) {
	struct mc_data_s *cur = mc_data_hash[i];
	long cur_chain_length = 0;

	if (cur == 0)
	    continue;
	++hash_slots_used;
	for (; cur; cur = cur->mc_prev) {
	    ++cur_chain_length;
	    if (cur->mc_type == MC_TYPE_ALLOC) {
		if (cur->mc_dealloc_noted) {
		    if (cur->mc_dealloc_noted > 1) {
			fprintf(stderr,
				" Duplicate dealloc! entry %ld\n",
				cur->mc_alloc_number);
			print_entry("duplicate dealloc", cur);

		    }
		    continue;
		} else {
		    fprintf(stderr, "malloc no dealloc, entry %ld\n",
			    cur->mc_alloc_number);
		    print_entry("dangle", cur);
		}
	    } else {
		/* mc_type is MC_TYPE_DEALLOC, already checked */

	    }
	}
	if (cur_chain_length > max_chain_length) {
	    max_chain_length = cur_chain_length;
	}
    }
    fprintf(stderr, "mc hash table slots=%ld, "
	    "used=%ld,  maxchain=%ld\n",
	    (long) HASH_TABLE_SIZE, hash_slots_used, max_chain_length);
    return;
}

#endif /* WANT_LIBBDWARF_MALLOC_CHECK */
