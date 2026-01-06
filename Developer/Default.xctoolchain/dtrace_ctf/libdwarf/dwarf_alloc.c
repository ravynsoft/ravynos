/*

  Copyright (C) 2000,2002,2004,2005 Silicon Graphics, Inc.  All Rights Reserved.

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
#undef  DEBUG

#include "config.h"
#include "dwarf_incl.h"
#include <sys/types.h>

#include <stdlib.h>
#include <stdio.h>
#include "malloc_check.h"

/*
    These files are included to get the sizes
    of structs to set the ah_bytes_one_struct field
    of the Dwarf_Alloc_Hdr_s structs for each
    allocation type.
*/
#include "dwarf_line.h"
#include "dwarf_global.h"
#include "dwarf_arange.h"
#include "dwarf_abbrev.h"
#include "dwarf_die_deliv.h"
#include "dwarf_frame.h"
#include "dwarf_loc.h"
#include "dwarf_funcs.h"
#include "dwarf_types.h"
#include "dwarf_vars.h"
#include "dwarf_weaks.h"

static void _dwarf_free_special_error(Dwarf_Ptr space);

#ifdef DWARF_SIMPLE_MALLOC
static void _dwarf_simple_malloc_add_to_list(Dwarf_Debug dbg,
					     Dwarf_Ptr addr,
					     unsigned long size,
					     short alloc_type);
static void _dwarf_simple_malloc_delete_from_list(Dwarf_Debug dbg,
						  Dwarf_Ptr space,
						  short alloc_type);
void _dwarf_simple_malloc_botch(int err);

#endif /* DWARF_SIMPLE_MALLOC */




/*
    This macro adds the size of a pointer to the size of a
    struct that is given to it.  It rounds up the size to
    be a multiple of the size of a pointer.  This is done
    so that every struct returned by _dwarf_get_alloc()
    can be preceded by a pointer to the chunk it came from.
    Before allocating, it checks if the size of struct is less than
    the size of a pointer.  If yes, it returns the size
    of 2 pointers.  The returned size should be at least
    the size of 2 pointers, since the first points to the
    chunk the struct was allocated from, and the second
    is used to link the free list.

    We want DW_RESERVE to be at least the size of
    a long long and at least the size of a pointer because
    our struct has a long long and we want that aligned right.
    Now Standard C defines long long as 8 bytes, so lets
    make that standard. It will become unworkable when
    long long or pointer grows beyound 8 bytes.
    Unclear what to do with wierd requirements, like
    36 bit pointers.


*/
#define DW_RESERVE 8

/* Round size up to the next multiple of DW_RESERVE bytes 
*/
#define ROUND_SIZE(inputsize)                 \
        (((inputsize) % (DW_RESERVE)) == 0 ? \
            (inputsize):                      \
            ((inputsize)  +                   \
               (DW_RESERVE) - ((inputsize) % (DW_RESERVE)) ))

#define ROUND_SIZE_WITH_POINTER(i_size) (ROUND_SIZE(i_size) + DW_RESERVE)

/* SMALL_ALLOC is for trivia where allocation is a waste.
   Things that should be removed, really. */
#define SMALL_ALLOC 2

/* BASE_ALLOC is where a basic allocation makes sense, but 'not too large'. 
   No thorough evaluation of this value has been done, though
   it was found wasteful of memory to have BASE_ALLOC be as large as
   BIG_ALLOC. */
#define BASE_ALLOC 64

/* BIG_ALLOC is where a larger-than-BASE_ALLOC 
   allocation makes sense, but still 'not too large'.
   No thorough evaluation of this value has been done. */
#define BIG_ALLOC  128

/* This translates into de_alloc_hdr index 
** the 0,1,1 entries are special: they don't use the
** table values at all.
** Rearranging the DW_DLA values would break binary compatibility
** so that is not an option.
*/
struct ial_s {
    int ia_al_num;		/* Index into de_alloc_hdr table. */

    /* In bytes, one struct instance. This does not account for extra
       space needed per block, but that (DW_RESERVE) will be added in
       later where it is needed (DW_RESERVE space never added in here). 
     */
    int ia_struct_size;


    /* Number of instances per alloc block. MUST be > 0. */
    int ia_base_count;

    int (*specialconstructor) (Dwarf_Debug, void *);
    void (*specialdestructor) (void *);
};

static const
struct ial_s index_into_allocated[ALLOC_AREA_INDEX_TABLE_MAX] = {
    {0, 1, 1, 0, 0},		/* none */
    {0, 1, 1, 0, 0},		/* 1 DW_DLA_STRING */
    {1, sizeof(Dwarf_Loc), BASE_ALLOC, 0, 0}
    ,				/* 2 DW_DLA_LOC */
    {2, sizeof(Dwarf_Locdesc), BASE_ALLOC, 0, 0}
    ,				/* 3 DW_DLA_LOCDESC */
    {0, 1, 1, 0, 0}
    ,				/* not used *//* 4 DW_DLA_ELLIST */
    {0, 1, 1, 0, 0}
    ,				/* not used *//* 5 DW_DLA_BOUNDS */
    {3, sizeof(Dwarf_Block), BASE_ALLOC, 0, 0}
    ,				/* 6 DW_DLA_BLOCK */
    {0, 1, 1, 0, 0}
    ,				/* the actual dwarf_debug structure *//* 7 DW_DLA_DEBUG */
    {4, sizeof(struct Dwarf_Die_s), BIG_ALLOC, 0, 0},	/* 8 DW_DLA_DIE 
							 */
    {5, sizeof(struct Dwarf_Line_s), BIG_ALLOC, 0, 0},	/* 9
							   DW_DLA_LINE */
    {6, sizeof(struct Dwarf_Attribute_s), BIG_ALLOC * 2, 0, 0},
    /* 10 DW_DLA_ATTR */
    {0, 1, 1, 0, 0},		/* not used *//* 11 DW_DLA_TYPE */
    {0, 1, 1, 0, 0},		/* not used *//* 12 DW_DLA_SUBSCR */
    {7, sizeof(struct Dwarf_Global_s), BASE_ALLOC, 0, 0},	/* 13
								   DW_DLA_GLOBAL 
								 */
    {8, sizeof(struct Dwarf_Error_s), BASE_ALLOC, 0, 0},	/* 14
								   DW_DLA_ERROR 
								 */
    {0, 1, 1, 0, 0},		/* 15 DW_DLA_LIST */
    {0, 1, 1, 0, 0},		/* not used *//* 16 DW_DLA_LINEBUF */
    {9, sizeof(struct Dwarf_Arange_s), BASE_ALLOC, 0, 0},	/* 17
								   DW_DLA_ARANGE 
								 */
    {10, sizeof(struct Dwarf_Abbrev_s), BIG_ALLOC, 0, 0},	/* 18
								   DW_DLA_ABBREV 
								 */
    {11, sizeof(Dwarf_Frame_Op), BIG_ALLOC, 0, 0}
    ,				/* 19 DW_DLA_FRAME_OP */
    {12, sizeof(struct Dwarf_Cie_s), BASE_ALLOC, 0, 0},	/* 20
							   DW_DLA_CIE */
    {13, sizeof(struct Dwarf_Fde_s), BASE_ALLOC, 0, 0},	/* 21
							   DW_DLA_FDE */
    {0, 1, 1, 0, 0},		/* 22 DW_DLA_LOC_BLOCK */
    {0, 1, 1, 0, 0},		/* 23 DW_DLA_FRAME_BLOCK */
    {14, sizeof(struct Dwarf_Global_s), BASE_ALLOC, 0, 0},	/* 24
								   DW_DLA_FUNC 
								   UNUSED */
    {15, sizeof(struct Dwarf_Global_s), BASE_ALLOC, 0, 0},	/* 25
								   DW_DLA_TYPENAME
								   UNUSED */
    {16, sizeof(struct Dwarf_Global_s), BASE_ALLOC, 0, 0},	/* 26
								   DW_DLA_VAR 
								   UNUSED */
    {17, sizeof(struct Dwarf_Global_s), BASE_ALLOC, 0, 0},	/* 27
								   DW_DLA_WEAK 
								   UNUSED */
    {0, 1, 1, 0, 0},		/* 28 DW_DLA_ADDR */
    {18, sizeof(struct Dwarf_Abbrev_List_s), BIG_ALLOC, 0, 0},
    /* 29 DW_DLA_ABBREV_LIST */

    {19, sizeof(struct Dwarf_Chain_s), BIG_ALLOC, 0, 0},	/* 30
								   DW_DLA_CHAIN 
								 */
    {20, sizeof(struct Dwarf_CU_Context_s), BASE_ALLOC, 0, 0},
    /* 31 DW_DLA_CU_CONTEXT */
    {21, sizeof(struct Dwarf_Frame_s), BASE_ALLOC,
     _dwarf_frame_constructor,
     _dwarf_frame_destructor},	/* 32 DW_DLA_FRAME */
    {22, sizeof(struct Dwarf_Global_Context_s), BASE_ALLOC, 0, 0},
    /* 33 DW_DLA_GLOBAL_CONTEXT */
    {23, sizeof(struct Dwarf_File_Entry_s), BASE_ALLOC, 0, 0},	/* 34 */
    /* 34 DW_DLA_FILE_ENTRY */
    {24, sizeof(struct Dwarf_Line_Context_s), BASE_ALLOC, 0, 0},
    /* 35 DW_DLA_LINE_CONTEXT */
    {25, sizeof(struct Dwarf_Loc_Chain_s), BASE_ALLOC, 0, 0},	/* 36 */
    /* 36 DW_DLA_LOC_CHAIN */

    /* See use of ABBREV_HASH_TABLE_SIZE below for final dealloc. */
    {26, ABBREV_HASH_TABLE_SIZE * sizeof(struct Dwarf_Hash_Table_s),
     BASE_ALLOC, 0, 0},		/* 37 */


    /* 37 DW_DLA_HASH_TABLE */

/* The following really use Global struct: used to be unique struct
   per type, but now merged (11/99).  The opaque types
   are visible in the interface. The types  for
   DW_DLA_FUNC,
   DW_DLA_TYPENAME, DW_DLA_VAR, DW_DLA_WEAK also use
   the global types.
  
*/
    {27, sizeof(struct Dwarf_Global_Context_s), BASE_ALLOC, 0, 0},
    /* 38 DW_DLA_FUNC_CONTEXT */
    {28, sizeof(struct Dwarf_Global_Context_s), BASE_ALLOC, 0, 0},
    /* 39 DW_DLA_TYPENAME_CONTEXT */
    {29, sizeof(struct Dwarf_Global_Context_s), BASE_ALLOC, 0, 0},
    /* 40 DW_DLA_VAR_CONTEXT */
    {30, sizeof(struct Dwarf_Global_Context_s), BASE_ALLOC, 0, 0},
    /* 41 DW_DLA_WEAK_CONTEXT */
    {31, sizeof(struct Dwarf_Global_Context_s), BASE_ALLOC, 0, 0},
    /* 42 DW_DLA_PUBTYPES_CONTEXT DWARF3 */

};

#ifndef DWARF_SIMPLE_MALLOC

/*
    This function is given a pointer to the header
    structure that is used to allocate 1 struct of
    the type given by alloc_type.  It first checks
    if a struct is available in its free list.  If
    not, it checks if 1 is available in its blob, 
    which is a chunk of memory that is reserved for
    its use.  If not, it malloc's a chunk.  The
    initial part of it is used to store the end
    address of the chunk, and also to keep track 
    of the number of free structs in that chunk.
    This information is used for freeing the chunk
    when all the structs in it are free.

    Assume all input arguments have been validated.

    This function can be used only to allocate 1
    struct of the given type.

    It returns a pointer to the struct that the
    user can use.  It returns NULL only when it
    is out of free structs, and cannot malloc 
    any more.  The struct returned is zero-ed.

    A pointer to the chunk that the struct belongs
    to is stored in the bytes preceding the
    returned address.  Since this pointer it
    never overwritten, when a struct is allocated
    from the free_list this pointer does not
    have to be written.  In the 2 other cases,
    where the struct is allocated from a new
    chunk, or the blob, a pointer to the chunk
    is written.
*/
static Dwarf_Ptr
_dwarf_find_memory(Dwarf_Alloc_Hdr alloc_hdr)
{
    /* Pointer to the struct allocated. */
    Dwarf_Small *ret_mem = 0;

    /* Pointer to info about chunks allocated. */
    Dwarf_Alloc_Area alloc_area;

    /* Size of chunk malloc'ed when no free structs left. */
    Dwarf_Signed mem_block_size;

    /* Pointer to block malloc'ed. */
    Dwarf_Small *mem_block;

    /* 
       Check the alloc_area from which the last allocation was made
       (most recent new block). If that is not successful, then search
       the list of alloc_area's from alloc_header. */
    alloc_area = alloc_hdr->ah_last_alloc_area;
    if (alloc_area == NULL || alloc_area->aa_free_structs_in_chunk == 0)
	for (alloc_area = alloc_hdr->ah_alloc_area_head;
	     alloc_area != NULL; alloc_area = alloc_area->aa_next) {

	    if (alloc_area->aa_free_structs_in_chunk > 0) {
		break;		/* found a free entry! */
	    }

	}

    if (alloc_area != NULL) {
	alloc_area->aa_free_structs_in_chunk--;

	if (alloc_area->aa_free_list != NULL) {
	    ret_mem = alloc_area->aa_free_list;

	    /* 
	       Update the free list.  The initial part of the struct is 
	       used to hold a pointer to the next struct on the free
	       list.  In this way, the free list chain is maintained at
	       0 memory cost. */
	    alloc_area->aa_free_list =
		((Dwarf_Free_List) ret_mem)->fl_next;
	} else if (alloc_area->aa_blob_start < alloc_area->aa_blob_end) {
	    ret_mem = alloc_area->aa_blob_start;

	    /* 
	       Store pointer to chunk this struct belongs to in the
	       first few bytes.  Return pointer to bytes after this
	       pointer storage. */
	    *(Dwarf_Alloc_Area *) ret_mem = alloc_area;
	    ret_mem += DW_RESERVE;

	    alloc_area->aa_blob_start += alloc_hdr->ah_bytes_one_struct;
	} else {
	    /* else fall thru , though it should be impossible to fall
	       thru. And represents a disastrous programming error if
	       we get here. */
#ifdef DEBUG
	    fprintf(stderr, "libdwarf Internal error start %x end %x\n",
		    (int) alloc_area->aa_blob_start,
		    (int) alloc_area->aa_blob_end);
#endif
	}
    }

    /* New memory has to malloc'ed since there are no free structs. */
    if (ret_mem == 0) {
	Dwarf_Word rounded_area_hdr_size;

	alloc_hdr->ah_chunks_allocated++;

	{			/* this nonsense avoids a warning */
	    /* CONSTCOND would be better */
	    unsigned long v = sizeof(struct Dwarf_Alloc_Area_s);

	    rounded_area_hdr_size = ROUND_SIZE(v);
	}

	/* 
	   Allocate memory to contain the required number of structs
	   and the Dwarf_Alloc_Area_s to control it. */
	mem_block_size = alloc_hdr->ah_bytes_malloc_per_chunk +
	    rounded_area_hdr_size;

	mem_block = malloc(mem_block_size);
	if (mem_block == NULL) {
	    return (NULL);
	}


	/* 
	   Attach the Dwarf_Alloc_Area_s struct to the list of chunks
	   malloc'ed for this struct type. Also initialize the fields
	   of the Dwarf_Alloc_Area_s. */
	alloc_area = (Dwarf_Alloc_Area) mem_block;
	alloc_area->aa_prev = 0;
	if (alloc_hdr->ah_alloc_area_head != NULL) {
	    alloc_hdr->ah_alloc_area_head->aa_prev = alloc_area;
	}
	alloc_area->aa_free_list = 0;
	alloc_area->aa_next = alloc_hdr->ah_alloc_area_head;
	alloc_hdr->ah_alloc_area_head = alloc_area;

	alloc_area->aa_alloc_hdr = alloc_hdr;
	alloc_area->aa_free_structs_in_chunk =
	    (Dwarf_Sword) alloc_hdr->ah_structs_per_chunk - 1;
	if (alloc_area->aa_free_structs_in_chunk < 1) {
	    /* If we get here, there is a disastrous programming error
	       somewhere. */
#ifdef DEBUG
	    fprintf(stderr,
		    "libdwarf Internal error: free structs in chunk %d\n",
		    (int) alloc_area->aa_free_structs_in_chunk);
#endif
	    return NULL;
	}

	/* 
	   The struct returned begins immediately after the
	   Dwarf_Alloc_Area_s struct. */
	ret_mem = mem_block + rounded_area_hdr_size;
	alloc_area->aa_blob_start =
	    ret_mem + alloc_hdr->ah_bytes_one_struct;
	alloc_area->aa_blob_end = mem_block + mem_block_size;

	/* 
	   Store pointer to chunk this struct belongs to in the first
	   few bytes.  Return pointer to bytes after this pointer
	   storage. */
	*(Dwarf_Alloc_Area *) ret_mem = alloc_area;
	ret_mem += DW_RESERVE;
    }

    alloc_hdr->ah_last_alloc_area = alloc_area;
    alloc_hdr->ah_struct_user_holds++;
    memset(ret_mem, 0, alloc_hdr->ah_bytes_one_struct - DW_RESERVE);
    return (ret_mem);
}

#endif /* ndef DWARF_SIMPLE_MALLOC */

/*
    This function returns a pointer to a region
    of memory.  For alloc_types that are not
    strings or lists of pointers, only 1 struct
    can be requested at a time.  This is indicated
    by an input count of 1.  For strings, count 
    equals the length of the string it will
    contain, i.e it the length of the string
    plus 1 for the terminating null.  For lists
    of pointers, count is equal to the number of
    pointers.  For DW_DLA_FRAME_BLOCK, and
    DW_DLA_LOC_BLOCK allocation types also, count
    is the count of the number of structs needed.

    This function cannot be used to allocate a 
    Dwarf_Debug_s struct.

*/
Dwarf_Ptr
_dwarf_get_alloc(Dwarf_Debug dbg,
		 Dwarf_Small alloc_type, Dwarf_Unsigned count)
{
    Dwarf_Alloc_Hdr alloc_hdr;

    Dwarf_Ptr ret_mem;

    Dwarf_Signed size = 0;
    unsigned int index;
    unsigned int type = alloc_type;

    if (dbg == NULL) {
	return (NULL);
    }

    if (type >= ALLOC_AREA_INDEX_TABLE_MAX) {
	/* internal error */
	return NULL;
    }
    index = index_into_allocated[type].ia_al_num;
    /* zero also illegal but not tested for */

    /* If the Dwarf_Debug is not fully set up, we will get index 0 for
       any type and must do something.  'Not fully set up' can only
       happen for DW_DLA_ERROR, I (davea) believe, and for that we call 
       special code here.. */

    if (index == 0) {
	if (alloc_type == DW_DLA_STRING) {
	    size = count;
	} else if (alloc_type == DW_DLA_LIST) {
	    size = count * sizeof(Dwarf_Ptr);
	} else if (alloc_type == DW_DLA_FRAME_BLOCK) {
	    size = count * sizeof(Dwarf_Frame_Op);
	} else if (alloc_type == DW_DLA_LOC_BLOCK) {
	    size = count * sizeof(Dwarf_Loc);
	} else if (alloc_type == DW_DLA_ADDR) {
	    size = count *
		(sizeof(Dwarf_Addr) > sizeof(Dwarf_Off) ?
		 sizeof(Dwarf_Addr) : sizeof(Dwarf_Off));
	} else if (alloc_type == DW_DLA_ERROR) {
	    void *m = _dwarf_special_no_dbg_error_malloc();

	    dwarf_malloc_check_alloc_data(m, DW_DLA_ERROR);
	    return m;

	} else {
	    /* If we get here, there is a disastrous programming error
	       somewhere. */
#ifdef DEBUG
	    fprintf(stderr,
		    "libdwarf Internal error: type %d  unexpected\n",
		    (int) type);
#endif
	}
    } else {
	alloc_hdr = &dbg->de_alloc_hdr[index];
	if (alloc_hdr->ah_bytes_one_struct > 0) {
#ifdef DWARF_SIMPLE_MALLOC
	    size = alloc_hdr->ah_bytes_one_struct;
#else
	    {
		void *m = _dwarf_find_memory(alloc_hdr);

		dwarf_malloc_check_alloc_data(m, type);
		if (index_into_allocated[type].specialconstructor) {
		    int res =
			index_into_allocated[type].
			specialconstructor(dbg, m);
		    if (res != DW_DLV_OK) {
			/* We leak what we allocated in
			   _dwarf_find_memory when constructor fails. */
			return NULL;
		    }
		}
		return m;
	    }
#endif

	} else {
	    /* Special case: should not really happen at all. */
	    if (type == DW_DLA_ERROR) {
		/* dwarf_init failure. Because dbg is incomplete we
		   won't use it to record the malloc. */
		void *m = _dwarf_special_no_dbg_error_malloc();

		dwarf_malloc_check_alloc_data(m, DW_DLA_ERROR);
		return m;
	    } else {
		/* If we get here, there is a disastrous programming
		   error somewhere. */
#ifdef DWARF_SIMPLE_MALLOC
		_dwarf_simple_malloc_botch(3);
#endif
#ifdef DEBUG
		fprintf(stderr,
			"libdwarf Internal error: Type %d  unexpected\n",
			(int) type);
#endif
	    }
	}
    }

    ret_mem = malloc(size);
#ifdef DWARF_SIMPLE_MALLOC
    _dwarf_simple_malloc_add_to_list(dbg, ret_mem, (unsigned long) size,
				     type);
#endif
    if (ret_mem != NULL)
	memset(ret_mem, 0, size);

    dwarf_malloc_check_alloc_data(ret_mem, type);
    if (index_into_allocated[type].specialconstructor) {
	int res =
	    index_into_allocated[type].specialconstructor(dbg, ret_mem);
	if (res != DW_DLV_OK) {
	    /* We leak what we allocated in _dwarf_find_memory when
	       constructor fails. */
	    return NULL;
	}
    }

    return (ret_mem);
}



/*
    This function is used to deallocate a region of memory
    that was obtained by a call to _dwarf_get_alloc.  Note
    that though dwarf_dealloc() is a public function, 
    _dwarf_get_alloc() isn't.  

    For lists, typically arrays of pointers, it is assumed
    that the space was allocated by a direct call to malloc,
    and so a straight free() is done.  This is also the case
    for variable length blocks such as DW_DLA_FRAME_BLOCK
    and DW_DLA_LOC_BLOCK.

    For strings, the pointer might point to a string in 
    .debug_info or .debug_string.  After this is checked,
    and if found not to be the case, a free() is done,
    again on the assumption that a malloc was used to
    obtain the space.

    For other types of structs, a pointer to the chunk that
    the struct was allocated out of, is present in the bytes
    preceding the pointer passed in.  For this chunk it is 
    checked whether all the structs in that chunk are now free.  
    If so, the entire chunk is free_ed.  Otherwise, the space 
    is added to the free list for that chunk, and the free count
    incremented.

    This function does not return anything.
*/
void
dwarf_dealloc(Dwarf_Debug dbg,
	      Dwarf_Ptr space, Dwarf_Unsigned alloc_type)
{
    Dwarf_Alloc_Hdr alloc_hdr;
    Dwarf_Alloc_Area alloc_area;
    unsigned int type = alloc_type;
    unsigned int index;

    if (space == NULL) {
	return;
    }
    if (type == DW_DLA_ERROR) {
	/* Get pointer to Dwarf_Alloc_Area this struct came from. See
	   dwarf_alloc.h ROUND_SIZE_WITH_POINTER stuff */
	alloc_area =
	    *(Dwarf_Alloc_Area *) ((char *) space - DW_RESERVE);
	if (alloc_area == 0) {
	    /* This is the special case of a failed dwarf_init(). Also
	       (and more signficantly) there are a variety of other
	       situations where libdwarf does not *know* what dbg is
	       involved (because of a libdwarf-caller-error) so
	       libdwarf uses NULL as the dbg. Those too wind up here. */
	    _dwarf_free_special_error(space);
	    dwarf_malloc_check_dealloc_data(space, type);
	    return;
	}

    }
    if (dbg == NULL) {
	/* App error, or an app that failed to succeed in a
	   dwarf_init() call. */
	return;
    }
    if (type >= ALLOC_AREA_INDEX_TABLE_MAX) {
	/* internal or user app error */
	return;
    }

    index = index_into_allocated[type].ia_al_num;
    /* 
       A string pointer may point into .debug_info or .debug_string.
       Otherwise, they are directly malloc'ed. */
    dwarf_malloc_check_dealloc_data(space, type);
    if (index == 0) {
	if (type == DW_DLA_STRING) {
	    if ((Dwarf_Small *) space >= dbg->de_debug_info &&
		(Dwarf_Small *) space <
		dbg->de_debug_info + dbg->de_debug_info_size)
		return;

	    if (dbg->de_debug_line != NULL &&
		(Dwarf_Small *) space >= dbg->de_debug_line &&
		(Dwarf_Small *) space <
		dbg->de_debug_line + dbg->de_debug_line_size)
		return;

	    if (dbg->de_debug_pubnames != NULL &&
		(Dwarf_Small *) space >= dbg->de_debug_pubnames &&
		(Dwarf_Small *) space <
		dbg->de_debug_pubnames + dbg->de_debug_pubnames_size)
		return;

	    if (dbg->de_debug_frame != NULL &&
		(Dwarf_Small *) space >= dbg->de_debug_frame &&
		(Dwarf_Small *) space <
		dbg->de_debug_frame + dbg->de_debug_frame_size)
		return;

	    if (dbg->de_debug_str != NULL &&
		(Dwarf_Small *) space >= dbg->de_debug_str &&
		(Dwarf_Small *) space <
		dbg->de_debug_str + dbg->de_debug_str_size)
		return;

	    if (dbg->de_debug_funcnames != NULL &&
		(Dwarf_Small *) space >= dbg->de_debug_funcnames &&
		(Dwarf_Small *) space <
		dbg->de_debug_funcnames + dbg->de_debug_funcnames_size)
		return;

	    if (dbg->de_debug_typenames != NULL &&
		(Dwarf_Small *) space >= dbg->de_debug_typenames &&
		(Dwarf_Small *) space <
		dbg->de_debug_typenames + dbg->de_debug_typenames_size)
		return;
	    if (dbg->de_debug_pubtypes != NULL &&
		(Dwarf_Small *) space >= dbg->de_debug_pubtypes &&
		(Dwarf_Small *) space <
		dbg->de_debug_pubtypes + dbg->de_debug_pubtypes_size)
		return;

	    if (dbg->de_debug_varnames != NULL &&
		(Dwarf_Small *) space >= dbg->de_debug_varnames &&
		(Dwarf_Small *) space <
		dbg->de_debug_varnames + dbg->de_debug_varnames_size)
		return;

	    if (dbg->de_debug_weaknames != NULL &&
		(Dwarf_Small *) space >= dbg->de_debug_weaknames &&
		(Dwarf_Small *) space <
		dbg->de_debug_weaknames + dbg->de_debug_weaknames_size)
		return;

	    free(space);
	    return;
	}

	if (type == DW_DLA_LIST ||
	    type == DW_DLA_FRAME_BLOCK ||
	    type == DW_DLA_LOC_BLOCK || type == DW_DLA_ADDR) {

	    free(space);
	    return;
	}
	/* else is an alloc type that is not used */
	/* app or internal error */
#ifdef DWARF_SIMPLE_MALLOC
	_dwarf_simple_malloc_botch(4);
#endif
	return;

    }
    if (index_into_allocated[type].specialdestructor) {
	index_into_allocated[type].specialdestructor(space);
    }
#ifdef DWARF_SIMPLE_MALLOC
    _dwarf_simple_malloc_delete_from_list(dbg, space, type);
    free(space);
#else /* !DWARF_SIMPLE_MALLOC */
    alloc_hdr = &dbg->de_alloc_hdr[index];

    /* Get pointer to Dwarf_Alloc_Area this struct came from. See
       dwarf_alloc.h ROUND_SIZE_WITH_POINTER stuff */
    alloc_area = *(Dwarf_Alloc_Area *) ((char *) space - DW_RESERVE);

    /* ASSERT: alloc_area != NULL If NULL we could abort, let it
       coredump below, or return, pretending all is well. We go on,
       letting program crash. Is caller error. */

    /* 
       Check that the alloc_hdr field of the alloc_area we have is
       pointing to the right alloc_hdr.  This is used to catch use of
       incorrect deallocation code by the user. */
    if (alloc_area->aa_alloc_hdr != alloc_hdr) {
	/* If we get here, the user has called dwarf_dealloc wrongly or 
	   there is some other disastrous error. By leaking mem here we 
	   try to be safe... */
#ifdef DEBUG
	fprintf(stderr,
		"libdwarf Internal error: type %d hdr mismatch %lx %lx "
		"area ptr %lx\n",
		(int) type,
		(long) alloc_area->aa_alloc_hdr,
		(long) alloc_hdr, (long) alloc_area);
#endif
	return;
    }

    alloc_hdr->ah_struct_user_holds--;
    alloc_area->aa_free_structs_in_chunk++;

    /* 
       Give chunk back to malloc only when every struct is freed */
    if (alloc_area->aa_free_structs_in_chunk ==
	alloc_hdr->ah_structs_per_chunk) {
	if (alloc_area->aa_prev != NULL) {
	    alloc_area->aa_prev->aa_next = alloc_area->aa_next;
	} else {
	    alloc_hdr->ah_alloc_area_head = alloc_area->aa_next;
	}

	if (alloc_area->aa_next != NULL) {
	    alloc_area->aa_next->aa_prev = alloc_area->aa_prev;
	}

	alloc_hdr->ah_chunks_allocated--;

	if (alloc_area == alloc_hdr->ah_last_alloc_area) {
	    alloc_hdr->ah_last_alloc_area = NULL;
	}
	memset(alloc_area, 0, sizeof(*alloc_area));
	free(alloc_area);
    }

    else {
	((Dwarf_Free_List) space)->fl_next = alloc_area->aa_free_list;
	alloc_area->aa_free_list = space;
    }
#endif /* !DWARF_SIMPLE_MALLOC */
}


/*
    Allocates space for a Dwarf_Debug_s struct,
    since one does not exist.
*/
Dwarf_Debug
_dwarf_get_debug(void
    )
{
    Dwarf_Debug dbg;

    dbg = (Dwarf_Debug) malloc(sizeof(struct Dwarf_Debug_s));
    if (dbg == NULL)
	return (NULL);
    else
	memset(dbg, 0, sizeof(struct Dwarf_Debug_s));

    return (dbg);
}


/*
    Sets up the Dwarf_Debug_s struct for all the
    allocation types currently defined.  
    Allocation types DW_DLA_STRING, DW_DLA_LIST,
    DW_DLA_FRAME_BLOCK, DW_DLA_LOC_BLOCK are 
    malloc'ed directly.

    This routine should be called after _dwarf_setup(),
    so that information about the sizes of the Dwarf
    sections can be used to decide the number of
    structs of each type malloc'ed.

    Also DW_DLA_ELLIST, DW_DLA_BOUNDS, DW_DLA_TYPE,
    DW_DLA_SUBSCR, DW_DLA_LINEBUF allocation types
    are currently not used.
    The ah_bytes_one_struct and ah_structs_per_chunk fields for
    these types have been set to 1 for efficiency
    in dwarf_get_alloc().
 
    Ah_alloc_num should be greater than 1 for all
    types that are currently being used.

    Therefore, for these allocation types the
    ah_bytes_one_struct, and ah_structs_per_chunk fields do not
    need to be initialized.

    Being an internal routine, assume proper dbg.




*/
/* 
**  Set up all the Dwarf_Alloc_Hdr records.
*/

Dwarf_Debug
_dwarf_setup_debug(Dwarf_Debug dbg)
{
    int i;

    for (i = 1; i <= MAX_DW_DLA; i++) {
	const struct ial_s *ialp = &index_into_allocated[i];
	unsigned int hdr_index = ialp->ia_al_num;
	Dwarf_Word str_size = ialp->ia_struct_size;
	Dwarf_Word str_count = ialp->ia_base_count;
	Dwarf_Word rnded_size = ROUND_SIZE_WITH_POINTER(str_size);

	Dwarf_Alloc_Hdr alloc_hdr = &dbg->de_alloc_hdr[hdr_index];

	alloc_hdr->ah_bytes_one_struct = (Dwarf_Half) rnded_size;

	/* ah_structs_per_chunk must be >0 else we are in trouble */
	alloc_hdr->ah_structs_per_chunk = str_count;
	alloc_hdr->ah_bytes_malloc_per_chunk = rnded_size * str_count;
    }
    return (dbg);
}

/*
    This function prints out the statistics
    collected on allocation of memory chunks.
*/
void
dwarf_print_memory_stats(Dwarf_Debug dbg)
{
    Dwarf_Alloc_Hdr alloc_hdr;
    Dwarf_Shalf i;

    /* 
       Alloc types start at 1, not 0. Hence, the first NULL string, and 
       also a size of MAX_DW_DLA + 1. */
    char *alloc_type_name[MAX_DW_DLA + 1] = {
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
	"DW_DLA_WEAK_CONTEXT" "DW_DLA_PUBTYPES_CONTEXT"
    };

    if (dbg == NULL)
	return;

    printf("Size of Dwarf_Debug        %4ld bytes\n",
	   (long) sizeof(*dbg));
    printf("Size of Dwarf_Alloc_Hdr_s  %4ld bytes\n",
	   (long) sizeof(struct Dwarf_Alloc_Hdr_s));
    printf("size of Dwarf_Alloc_Area_s %4ld bytes\n",
	   (long) sizeof(struct Dwarf_Alloc_Area_s));

    printf("   Alloc Type                   Curr  Structs byt   str\n");
    printf("   ----------                   ----  ------- per   per\n");
    for (i = 1; i <= MAX_DW_DLA; i++) {
	int indx = index_into_allocated[i].ia_al_num;

	alloc_hdr = &dbg->de_alloc_hdr[indx];
	if (alloc_hdr->ah_bytes_one_struct != 1) {
	    printf("%2d %-25s   %6d %8d %6d %6d\n",
		   (int) i,
		   alloc_type_name[i],
		   (int) alloc_hdr->ah_chunks_allocated,
		   (int) alloc_hdr->ah_struct_user_holds,
		   (int) alloc_hdr->ah_bytes_malloc_per_chunk,
		   (int) alloc_hdr->ah_structs_per_chunk);
	}
    }
}


#ifndef DWARF_SIMPLE_MALLOC
/*
    This function is used to recursively
    free the chunks still allocated, and
    forward chained through the aa_next
    pointer.
*/
static void
_dwarf_recursive_free(Dwarf_Alloc_Area alloc_area)
{
    if (alloc_area->aa_next != NULL) {
	_dwarf_recursive_free(alloc_area->aa_next);
    }

    alloc_area->aa_next = 0;
    alloc_area->aa_prev = 0;
    free(alloc_area);
}
#endif

/*
    Used to free all space allocated for this Dwarf_Debug.
    The caller should assume that the Dwarf_Debug pointer 
    itself is no longer valid upon return from this function.

    In case of difficulty, this function simply returns quietly.
*/
int
_dwarf_free_all_of_one_debug(Dwarf_Debug dbg)
{
    Dwarf_Alloc_Hdr alloc_hdr;
    Dwarf_Shalf i;
    Dwarf_CU_Context context = 0;
    Dwarf_CU_Context nextcontext = 0;

    if (dbg == NULL)
	return (DW_DLV_ERROR);

    /* To do complete validation that we have no surprising missing or
       erroneous deallocs it is advisable to do the dwarf_deallocs here 
       that are not things the user can otherwise request.
       Housecleaning.  */

    for (context = dbg->de_cu_context_list;
	 context; context = nextcontext) {
	Dwarf_Hash_Table hash_table = context->cc_abbrev_hash_table;

	/* A Hash Table is an array with ABBREV_HASH_TABLE_SIZE struct 
	   Dwarf_Hash_Table_s entries in the array. */
	int hashnum = 0;

	for (; hashnum < ABBREV_HASH_TABLE_SIZE; ++hashnum) {
	    struct Dwarf_Abbrev_List_s *abbrev = 0;
	    struct Dwarf_Abbrev_List_s *nextabbrev = 0;

	    abbrev = hash_table[hashnum].at_head;
	    for (; abbrev; abbrev = nextabbrev) {
		nextabbrev = abbrev->ab_next;
		dwarf_dealloc(dbg, abbrev, DW_DLA_ABBREV_LIST);
	    }
	}
	nextcontext = context->cc_next;
	dwarf_dealloc(dbg, hash_table, DW_DLA_HASH_TABLE);
	dwarf_dealloc(dbg, context, DW_DLA_CU_CONTEXT);
    }

    /* Housecleaning done. Now really free all the space. */

#ifdef DWARF_SIMPLE_MALLOC
    if (dbg->de_simple_malloc_base) {
	struct simple_malloc_record_s *smp = dbg->de_simple_malloc_base;

	while (smp) {
	    int j;
	    struct simple_malloc_record_s *prev_smp = 0;

	    for (j = 0; j < smp->sr_used; ++j) {
		struct simple_malloc_entry_s *cur;

		cur = &smp->sr_entry[j];
		if (cur->se_addr != 0) {
		    free(cur->se_addr);
		    cur->se_addr = 0;
		}
	    }
	    prev_smp = smp;
	    smp = smp->sr_next;
	    free(prev_smp);
	}
	dbg->de_simple_malloc_base = 0;
	dbg->de_simple_malloc_current = 0;
    }
#else
    for (i = 1; i < ALLOC_AREA_REAL_TABLE_MAX; i++) {
	int indx = i;

	alloc_hdr = &dbg->de_alloc_hdr[indx];
	if (alloc_hdr->ah_alloc_area_head != NULL) {
	    _dwarf_recursive_free(alloc_hdr->ah_alloc_area_head);
	}
    }

#endif

    memset(dbg, 0, sizeof(*dbg));	/* prevent accidental use later 
					 */
    free(dbg);
    return (DW_DLV_OK);
}

/* A special case: we have no dbg, no alloc header etc.
   So create something out of thin air that we can recognize
   in dwarf_dealloc.
   Something with the prefix (prefix space hidden from caller).

   Only applies to DW_DLA_ERROR, making up an error record.
*/

struct Dwarf_Error_s *
_dwarf_special_no_dbg_error_malloc(void)
{
    /* the union unused things are to guarantee proper alignment */
    union u {
	Dwarf_Alloc_Area ptr_not_used;
	struct Dwarf_Error_s base_not_used;
	char data_space[sizeof(struct Dwarf_Error_s) +
			(DW_RESERVE * 2)];
    };
    char *mem;

    mem = malloc(sizeof(union u));

    if (mem == 0) {
	return 0;

    }
    memset(mem, 0, sizeof(union u));
    mem += DW_RESERVE;
    return (struct Dwarf_Error_s *) mem;
}

/* The free side of  _dwarf_special_no_dbg_error_malloc()
*/
static void
_dwarf_free_special_error(Dwarf_Ptr space)
{
    char *mem = (char *) space;

    mem -= DW_RESERVE;
    free(mem);
}


#ifdef DWARF_SIMPLE_MALLOC
/* here solely for planting a breakpoint. */
/* ARGSUSED */
void
_dwarf_simple_malloc_botch(int err)
{
}
static void
_dwarf_simple_malloc_add_to_list(Dwarf_Debug dbg,
				 Dwarf_Ptr addr,
				 unsigned long size, short alloc_type)
{
    struct simple_malloc_record_s *cur;
    struct simple_malloc_entry_s *newentry;

    if (!dbg->de_simple_malloc_current) {
	/* First entry to this routine. */
	dbg->de_simple_malloc_current =
	    malloc(sizeof(struct simple_malloc_record_s));
	if (!dbg->de_simple_malloc_current) {
	    return;		/* no memory, give up */
	}
	memset(dbg->de_simple_malloc_current,
	       0, sizeof(struct simple_malloc_record_s));
	dbg->de_simple_malloc_base = dbg->de_simple_malloc_current;
    }
    cur = dbg->de_simple_malloc_current;

    if (cur->sr_used >= DSM_BLOCK_COUNT) {
	/* better not be > than as that means chaos */

	/* Create a new block to link at the head. */

	struct simple_malloc_record_s *newblock =
	    malloc(sizeof(struct simple_malloc_record_s));
	if (!newblock) {
	    return;		/* Can do nothing, out of memory */
	}
	memset(newblock, 0, sizeof(struct simple_malloc_record_s));
	/* Link the new block at the head of the chain, and make it
	   'current' */
	dbg->de_simple_malloc_current = newblock;
	newblock->sr_next = cur;
	cur = newblock;
    }
    newentry = &cur->sr_entry[cur->sr_used];
    newentry->se_addr = addr;
    newentry->se_size = size;
    newentry->se_type = alloc_type;
    ++cur->sr_used;
}

/*
   DWARF_SIMPLE_MALLOC is for testing the hypothesis that the existing
   complex malloc scheme in libdwarf is pointless complexity.

   DWARF_SIMPLE_MALLOC also makes it easy for a malloc-tracing
   tool to verify libdwarf malloc has no botches (though of course
   such does not test the complicated standard-libdwarf-alloc code).

   To properly answer the question, the simple-malloc allocate
   and delete should be something other than a simple list.
   Perhaps a heap, or perhaps a red-black tree.

*/
static void
_dwarf_simple_malloc_delete_from_list(Dwarf_Debug dbg,
				      Dwarf_Ptr space, short alloc_type)
{
    if (space == 0) {
	_dwarf_simple_malloc_botch(6);
    }
    if (dbg->de_simple_malloc_base) {
	struct simple_malloc_record_s *smp = dbg->de_simple_malloc_base;

	while (smp) {
	    int i;

	    for (i = 0; i < smp->sr_used; ++i) {
		struct simple_malloc_entry_s *cur;

		cur = &smp->sr_entry[i];
		if (cur->se_addr == space) {
		    if (cur->se_type != alloc_type) {
			_dwarf_simple_malloc_botch(0);
		    }
		    cur->se_addr = 0;
		    return;
		}
	    }
	    smp = smp->sr_next;
	}
    }
    /* Never found the space */
    _dwarf_simple_malloc_botch(1);
    return;

}
#endif
