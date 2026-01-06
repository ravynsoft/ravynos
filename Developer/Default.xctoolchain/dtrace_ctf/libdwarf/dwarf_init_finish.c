/*

  Copyright (C) 2000,2002,2003,2004,2005 Silicon Graphics, Inc.  All Rights Reserved.

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



#include "config.h"
#include "dwarf_incl.h"
#ifdef HAVE_ELF_H
#include <sys/elf.h>
#endif
#ifdef __SGI_FAST_LIBELF
#include <libelf_sgi.h>
#else
#ifdef HAVE_LIBELF_H
#include <libelf.h>
#endif
#endif /* !defined(__SGI_FAST_LIBELF) */

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#include "dwarf_incl.h"
#include "malloc_check.h"

#define DWARF_DBG_ERROR(dbg,errval,retval) \
     _dwarf_error(dbg, error, errval); return(retval);

#define FALSE	0
#define TRUE	1

#ifdef __SGI_FAST_LIBELF
#else
#ifdef HAVE_ELF64_GETEHDR
extern Elf64_Ehdr *elf64_getehdr(Elf *);
#endif
#ifdef HAVE_ELF64_GETSHDR
extern Elf64_Shdr *elf64_getshdr(Elf_Scn *);
#endif
#endif /* !defined(__SGI_FAST_LIBELF) */


/* This static is copied to the dbg on dbg init
   so that the static need not be referenced at
   run time, preserving better locality of
   reference.
   Value is 0 means do the string check.
   Value non-zero means do not do the check.
*/
static Dwarf_Small _dwarf_assume_string_bad;


int
dwarf_set_stringcheck(int newval)
{
    int oldval = _dwarf_assume_string_bad;

    _dwarf_assume_string_bad = newval;
    return oldval;
}

#ifdef __SGI_FAST_LIBELF
/*
	This function translates an elf_sgi error code into a libdwarf
	code.
 */
static int
_dwarf_error_code_from_elf_sgi_error_code(enum elf_sgi_error_type val)
{
    switch (val) {
    case ELF_SGI_ERROR_OK:
	return DW_DLE_NE;
    case ELF_SGI_ERROR_BAD_ALLOC:
	return DW_DLE_MAF;
    case ELF_SGI_ERROR_FORMAT:
	return DW_DLE_MDE;
    case ELF_SGI_ERROR_ERRNO:
	return DW_DLE_IOF;
    case ELF_SGI_ERROR_TOO_BIG:
	return DW_DLE_MOF;
    default:
	return DW_DLE_LEE;
    }
}
#endif

/*
    Given an Elf ptr, set up dbg with pointers
    to all the Dwarf data sections.
    Return NULL on error.

    This function is also responsible for determining
    whether the given object contains Dwarf information
    or not.  The test currently used is that it contains
    either a .debug_info or a .debug_frame section.  If 
    not, it returns DW_DLV_NO_ENTRY causing dwarf_init() also to 
    return DW_DLV_NO_ENTRY.  Earlier, we had thought of using only 
    the presence/absence of .debug_info to test, but we 
    added .debug_frame since there could be stripped objects 
    that have only a .debug_frame section for exception 
    processing.
    DW_DLV_NO_ENTRY or DW_DLV_OK or DW_DLV_ERROR
*/
static int
_dwarf_setup(Dwarf_Debug dbg, dwarf_elf_handle elf, Dwarf_Error * error)
{
#ifdef __SGI_FAST_LIBELF
    Elf64_Ehdr ehdr;
    Elf64_Shdr shdr;
    enum elf_sgi_error_type sres;
    unsigned char const *ehdr_ident;
#else
    Elf32_Ehdr *ehdr32;

#ifdef HAVE_ELF64_GETEHDR
    Elf64_Ehdr *ehdr64;
#endif
    Elf32_Shdr *shdr32;

#ifdef HAVE_ELF64_GETSHDR
    Elf64_Shdr *shdr64;
#endif
    Elf_Scn *scn;
    char *ehdr_ident;
#endif /* !defined(__SGI_FAST_LIBELF) */
    Dwarf_Half machine;
    char *scn_name;
    int is_64bit;
    int foundDwarf;

    Dwarf_Unsigned section_size;
    Dwarf_Unsigned section_count;
    Dwarf_Half section_index;

    foundDwarf = FALSE;
    dbg->de_elf = elf;

    dbg->de_assume_string_in_bounds = _dwarf_assume_string_bad;

#ifdef __SGI_FAST_LIBELF
    sres = elf_sgi_ehdr(elf, &ehdr);
    if (sres != ELF_SGI_ERROR_OK) {
	DWARF_DBG_ERROR(dbg,
			_dwarf_error_code_from_elf_sgi_error_code(sres),
			DW_DLV_ERROR);
    }
    ehdr_ident = ehdr.e_ident;
    section_count = ehdr.e_shnum;
    machine = ehdr.e_machine;
#else
    if ((ehdr_ident = elf_getident(elf, NULL)) == NULL) {
	DWARF_DBG_ERROR(dbg, DW_DLE_ELF_GETIDENT_ERROR, DW_DLV_ERROR);
    }
#endif

    is_64bit = (ehdr_ident[EI_CLASS] == ELFCLASS64);


    dbg->de_same_endian = 1;
    dbg->de_copy_word = memcpy;
#ifdef WORDS_BIGENDIAN
    dbg->de_big_endian_object = 1;
    if (ehdr_ident[EI_DATA] == ELFDATA2LSB) {
	dbg->de_same_endian = 0;
	dbg->de_big_endian_object = 0;
	dbg->de_copy_word = _dwarf_memcpy_swap_bytes;
    }
#else /* little endian */
    dbg->de_big_endian_object = 0;
    if (ehdr_ident[EI_DATA] == ELFDATA2MSB) {
	dbg->de_same_endian = 0;
	dbg->de_big_endian_object = 1;
	dbg->de_copy_word = _dwarf_memcpy_swap_bytes;
    }
#endif /* !WORDS_BIGENDIAN */

    /* The following de_length_size is Not Too Significant. Only used
       one calculation, and an approximate one at that. */
    dbg->de_length_size = is_64bit ? 8 : 4;
    dbg->de_pointer_size = is_64bit ? 8 : 4;


#ifdef __SGI_FAST_LIBELF
    /* We've already loaded the ELF header, so there's nothing to do
       here */
#else
#ifdef HAVE_ELF64_GETEHDR
    if (is_64bit) {
	ehdr64 = elf64_getehdr(elf);
	if (ehdr64 == NULL) {
	    DWARF_DBG_ERROR(dbg, DW_DLE_ELF_GETEHDR_ERROR,
			    DW_DLV_ERROR);
	}
	section_count = ehdr64->e_shnum;
	machine = ehdr64->e_machine;
    } else
#endif
    {
	ehdr32 = elf32_getehdr(elf);
	if (ehdr32 == NULL) {
	    DWARF_DBG_ERROR(dbg, DW_DLE_ELF_GETEHDR_ERROR,
			    DW_DLV_ERROR);
	}
	section_count = ehdr32->e_shnum;
	machine = ehdr32->e_machine;
    }
#endif /* !defined(__SGI_FAST_LIBELF) */

    if (is_64bit) {
	/* MIPS/IRIX makes pointer size and length size 8 for -64.
	   Other platforms make length 4 always. */
	/* 4 here supports 32bit-offset dwarf2, as emitted by cygnus
	   tools, and the dwarfv2.1 64bit extension setting. */
	dbg->de_length_size = 4;
    }

    /* We start at index 1 to skip the initial empty section. */
    for (section_index = 1; section_index < section_count;
	 ++section_index) {

#ifdef __SGI_FAST_LIBELF
	sres = elf_sgi_shdr(elf, section_index, &shdr);
	if (sres != ELF_SGI_ERROR_OK) {
	    DWARF_DBG_ERROR(dbg,
			    _dwarf_error_code_from_elf_sgi_error_code
			    (sres), DW_DLV_ERROR);
	}

	section_size = shdr.sh_size;

	sres =
	    elf_sgi_string(elf, ehdr.e_shstrndx, shdr.sh_name,
			   (char const **) &scn_name);
	if (sres != ELF_SGI_ERROR_OK) {
	    DWARF_DBG_ERROR(dbg,
			    _dwarf_error_code_from_elf_sgi_error_code
			    (sres), DW_DLV_ERROR);
	}
#else /* !defined(__SGI_FAST_LIBELF) */
	scn = elf_getscn(elf, section_index);
	if (scn == NULL) {
	    DWARF_DBG_ERROR(dbg, DW_DLE_MDE, DW_DLV_ERROR);
	}
#ifdef HAVE_ELF64_GETSHDR
	if (is_64bit) {
	    shdr64 = elf64_getshdr(scn);
	    if (shdr64 == NULL) {
		DWARF_DBG_ERROR(dbg, DW_DLE_ELF_GETSHDR_ERROR,
				DW_DLV_ERROR);
	    }

	    section_size = shdr64->sh_size;

	    if ((scn_name = elf_strptr(elf, ehdr64->e_shstrndx,
				       shdr64->sh_name))
		== NULL) {
		DWARF_DBG_ERROR(dbg, DW_DLE_ELF_STRPTR_ERROR,
				DW_DLV_ERROR);
	    }
	} else
#endif /* HAVE_ELF64_GETSHDR */
	{
	    if ((shdr32 = elf32_getshdr(scn)) == NULL) {
		DWARF_DBG_ERROR(dbg, DW_DLE_ELF_GETSHDR_ERROR, 0);
	    }

	    section_size = shdr32->sh_size;

	    if ((scn_name = elf_strptr(elf, ehdr32->e_shstrndx,
				       shdr32->sh_name)) == NULL) {
		DWARF_DBG_ERROR(dbg, DW_DLE_ELF_STRPTR_ERROR,
				DW_DLV_ERROR);
	    }
	}
#endif /* !defined(__SGI_FAST_LIBELF) */

	if (strncmp(scn_name, ".debug_", 7)
	    && strcmp(scn_name, ".eh_frame")
	    )
	    continue;

	else if (strcmp(scn_name, ".debug_info") == 0) {
	    if (dbg->de_debug_info != NULL) {
		DWARF_DBG_ERROR(dbg, DW_DLE_DEBUG_INFO_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if (section_size == 0) {
		/* Know no reason to allow empty debug_info section */
		DWARF_DBG_ERROR(dbg, DW_DLE_DEBUG_INFO_NULL,
				DW_DLV_ERROR);
	    }
	    foundDwarf = TRUE;
	    dbg->de_debug_info_index = section_index;
	    dbg->de_debug_info_size = section_size;
	}

	else if (strcmp(scn_name, ".debug_abbrev") == 0) {
	    if (dbg->de_debug_abbrev != NULL) {
		DWARF_DBG_ERROR(dbg, DW_DLE_DEBUG_ABBREV_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if (section_size == 0) {
		/* Know no reason to allow empty debug_abbrev section */
		DWARF_DBG_ERROR(dbg, DW_DLE_DEBUG_ABBREV_NULL,
				DW_DLV_ERROR);
	    }
	    dbg->de_debug_abbrev_index = section_index;
	    dbg->de_debug_abbrev_size = section_size;
	}

	else if (strcmp(scn_name, ".debug_aranges") == 0) {
	    if (dbg->de_debug_aranges_index != 0) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_ARANGES_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if (section_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_aranges_index = section_index;
	    dbg->de_debug_aranges_size = section_size;
	}

	else if (strcmp(scn_name, ".debug_line") == 0) {
	    if (dbg->de_debug_line_index != 0) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_LINE_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if (section_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_line_index = section_index;
	    dbg->de_debug_line_size = section_size;
	}

	else if (strcmp(scn_name, ".debug_frame") == 0) {
	    if (dbg->de_debug_frame_index != 0) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_FRAME_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if (section_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_frame_index = section_index;
	    dbg->de_debug_frame_size = section_size;
	    foundDwarf = TRUE;
	} else if (strcmp(scn_name, ".eh_frame") == 0) {
	    /* gnu egcs-1.1.2 data */
	    if (dbg->de_debug_frame_eh_gnu_index != 0) {
#if defined(__APPLE__)
		continue;
#else
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_FRAME_DUPLICATE,
				DW_DLV_ERROR);
#endif
	    }
	    if (section_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_frame_eh_gnu_index = section_index;
	    dbg->de_debug_frame_size_eh_gnu = section_size;
	    foundDwarf = TRUE;
	}

	else if (strcmp(scn_name, ".debug_loc") == 0) {
	    if (dbg->de_debug_loc_index != 0) {
		DWARF_DBG_ERROR(dbg, DW_DLE_DEBUG_LOC_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if (section_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_loc_index = section_index;
	    dbg->de_debug_loc_size = section_size;
	}


	else if (strcmp(scn_name, ".debug_pubnames") == 0) {
	    if (dbg->de_debug_pubnames_index != 0) {
		DWARF_DBG_ERROR(dbg, DW_DLE_DEBUG_PUBNAMES_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if (section_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_pubnames_index = section_index;
	    dbg->de_debug_pubnames_size = section_size;
	}

	else if (strcmp(scn_name, ".debug_str") == 0) {
	    if (dbg->de_debug_str_index != 0) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_STR_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if (section_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_str_index = section_index;
	    dbg->de_debug_str_size = section_size;
	}

	else if (strcmp(scn_name, ".debug_funcnames") == 0) {
	    if (dbg->de_debug_funcnames_index != 0) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_FUNCNAMES_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if (section_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_funcnames_index = section_index;
	    dbg->de_debug_funcnames_size = section_size;
	}

	else if (strcmp(scn_name, ".debug_typenames") == 0) {
	    /* SGI IRIX-only, created years before DWARF3. Content
	       essentially identical to .debug_pubtypes.  */
	    if (dbg->de_debug_typenames_index != 0) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_TYPENAMES_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if (section_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_typenames_index = section_index;
	    dbg->de_debug_typenames_size = section_size;
	} else if (strcmp(scn_name, ".debug_pubtypes") == 0) {
	    /* Section new in DWARF3.  */
	    if (dbg->de_debug_pubtypes_index != 0) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_PUBTYPES_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if (section_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_pubtypes_index = section_index;
	    dbg->de_debug_pubtypes_size = section_size;
	}

	else if (strcmp(scn_name, ".debug_varnames") == 0) {
	    if (dbg->de_debug_varnames_index != 0) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_VARNAMES_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if (section_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_varnames_index = section_index;
	    dbg->de_debug_varnames_size = section_size;
	}

	else if (strcmp(scn_name, ".debug_weaknames") == 0) {
	    if (dbg->de_debug_weaknames_index != 0) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_WEAKNAMES_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if (section_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_weaknames_index = section_index;
	    dbg->de_debug_weaknames_size = section_size;
	} else if (strcmp(scn_name, ".debug_macinfo") == 0) {
	    if (dbg->de_debug_macinfo_index != 0) {
		DWARF_DBG_ERROR(dbg,
				DW_DLE_DEBUG_MACINFO_DUPLICATE,
				DW_DLV_ERROR);
	    }
	    if (section_size == 0) {
		/* a zero size section is just empty. Ok, no error */
		continue;
	    }
	    dbg->de_debug_macinfo_index = section_index;
	    dbg->de_debug_macinfo_size = section_size;
	}
    }
    if (foundDwarf) {
	return DW_DLV_OK;
    }

    return (DW_DLV_NO_ENTRY);
}


/*
    The basic dwarf initializer function for consumers.
    Return NULL on error.
*/
int
dwarf_init(int fd,
	   Dwarf_Unsigned access,
	   Dwarf_Handler errhand,
	   Dwarf_Ptr errarg, Dwarf_Debug * ret_dbg, Dwarf_Error * error)
{
    Dwarf_Debug dbg;
    struct stat fstat_buf;
    dwarf_elf_handle elf;
    int res;

#ifdef __SGI_FAST_LIBELF
    enum elf_sgi_error_type sres;
#else
    Elf_Cmd what_kind_of_elf_read;
#endif

    dbg = _dwarf_get_debug();
    if (dbg == NULL) {
	DWARF_DBG_ERROR(dbg, DW_DLE_DBG_ALLOC, DW_DLV_ERROR);
    }
    dbg->de_errhand = errhand;
    dbg->de_errarg = errarg;
    dbg->de_frame_rule_initial_value = DW_FRAME_REG_INITIAL_VALUE;
    dbg->de_frame_reg_rules_entry_count = DW_FRAME_LAST_REG_NUM;


    if (fstat(fd, &fstat_buf) != 0) {
	DWARF_DBG_ERROR(dbg, DW_DLE_FSTAT_ERROR, DW_DLV_ERROR);
    }
    if (!S_ISREG(fstat_buf.st_mode)) {
	DWARF_DBG_ERROR(dbg, DW_DLE_FSTAT_MODE_ERROR, DW_DLV_ERROR);
    }

    if (access != DW_DLC_READ) {
	DWARF_DBG_ERROR(dbg, DW_DLE_INIT_ACCESS_WRONG, DW_DLV_ERROR);
    }
    dbg->de_access = access;

#ifdef __SGI_FAST_LIBELF
    elf = elf_sgi_new();
    if (elf == NULL) {
	DWARF_DBG_ERROR(dbg, DW_DLE_MAF, DW_DLV_ERROR);
    }

    sres = elf_sgi_begin_fd(elf, fd, 0);
    if (sres != ELF_SGI_ERROR_OK) {
	elf_sgi_free(elf);
	DWARF_DBG_ERROR(dbg,
			_dwarf_error_code_from_elf_sgi_error_code(sres),
			DW_DLV_ERROR);
    }
#else /* ! __SGI_FAST_LIBELF */
    elf_version(EV_CURRENT);
    /* changed to mmap request per bug 281217. 6/95 */
#ifdef HAVE_ELF_C_READ_MMAP
    /* ELF_C_READ_MMAP is an SGI IRIX specific enum value from IRIX
       libelf.h meaning read but use mmap */
    what_kind_of_elf_read = ELF_C_READ_MMAP;
#else /* !HAVE_ELF_C_READ_MMAP */
    /* ELF_C_READ is a portable value */
    what_kind_of_elf_read = ELF_C_READ;
#endif /* HAVE_ELF_C_READ_MMAP */

    if ((elf = elf_begin(fd, what_kind_of_elf_read, 0)) == NULL) {
	DWARF_DBG_ERROR(dbg, DW_DLE_ELF_BEGIN_ERROR, DW_DLV_ERROR);
    }
#endif /* !defined(__SGI_FAST_LIBELF) */

    dbg->de_elf_must_close = 1;
    if ((res = _dwarf_setup(dbg, elf, error)) != DW_DLV_OK) {
#ifdef __SGI_FAST_LIBELF
	elf_sgi_free(elf);
#else
	elf_end(elf);
#endif
	free(dbg);
	return (res);
    }

    /* call cannot fail: no malloc or free involved */
    _dwarf_setup_debug(dbg);

    *ret_dbg = dbg;
    return (DW_DLV_OK);
}


/*
    The alternate dwarf setup call for consumers
*/
int
dwarf_elf_init(dwarf_elf_handle elf_file_pointer,
	       Dwarf_Unsigned access,
	       Dwarf_Handler errhand,
	       Dwarf_Ptr errarg,
	       Dwarf_Debug * ret_dbg, Dwarf_Error * error)
{
    Dwarf_Debug dbg;
    int res;

    dbg = _dwarf_get_debug();
    if (dbg == NULL) {
	DWARF_DBG_ERROR(dbg, DW_DLE_DBG_ALLOC, DW_DLV_ERROR);
    }
    dbg->de_errhand = errhand;
    dbg->de_errarg = errarg;
    dbg->de_frame_rule_initial_value = DW_FRAME_REG_INITIAL_VALUE;
    dbg->de_frame_reg_rules_entry_count = DW_FRAME_LAST_REG_NUM;

    if (access != DW_DLC_READ) {
	DWARF_DBG_ERROR(dbg, DW_DLE_INIT_ACCESS_WRONG, DW_DLV_ERROR);
    }
    dbg->de_access = access;

    dbg->de_elf_must_close = 0;
    if ((res = _dwarf_setup(dbg, elf_file_pointer, error)) != DW_DLV_OK) {
	free(dbg);
	return (res);
    }

    /* this call cannot fail: allocates nothing, releases nothing */
    _dwarf_setup_debug(dbg);

    *ret_dbg = dbg;
    return (DW_DLV_OK);
}


/*
	Frees all memory that was not previously freed
	by dwarf_dealloc.
	Aside from certain categories.
*/
int
dwarf_finish(Dwarf_Debug dbg, Dwarf_Error * error)
{
    int res = DW_DLV_OK;

    if (dbg->de_elf_must_close) {
	/* Must do this *before* _dwarf_free_all_of_one_debug() as that 
	   zeroes out dbg contents */
#ifdef __SGI_FAST_LIBELF
	elf_sgi_free(dbg->de_elf);
#else
	elf_end(dbg->de_elf);
#endif
    }

    res = _dwarf_free_all_of_one_debug(dbg);
    if (res == DW_DLV_ERROR) {
	DWARF_DBG_ERROR(dbg, DW_DLE_DBG_ALLOC, DW_DLV_ERROR);
    }
    dwarf_malloc_check_complete("After Final free");

    return res;


}


/*
    This function returns the Elf * pointer
    associated with a Dwarf_Debug.
*/
int
dwarf_get_elf(Dwarf_Debug dbg, dwarf_elf_handle * elf,
	      Dwarf_Error * error)
{
    if (dbg == NULL) {
	_dwarf_error(NULL, error, DW_DLE_DBG_NULL);
	return (DW_DLV_ERROR);
    }

    *elf = dbg->de_elf;
    return (DW_DLV_OK);
}


/*
	Load the ELF section with the specified index and set the
	pointer pointed to by section_data to the memory where it
	was loaded.
 */
int
_dwarf_load_section(Dwarf_Debug dbg,
		    Dwarf_Half section_index,
		    Dwarf_Small ** section_data, Dwarf_Error * error)
{
    if (section_index == 0) {
	return DW_DLV_NO_ENTRY;
    }

    /* check to see if the section is already loaded */
    if (*section_data != NULL) {
	return DW_DLV_OK;
    }

    {
#ifdef __SGI_FAST_LIBELF
	enum elf_sgi_error_type sres;

	sres = elf_sgi_section(dbg->de_elf,
			       section_index, (void **) section_data);
	if (sres != ELF_SGI_ERROR_OK) {
	    DWARF_DBG_ERROR(dbg,
			    _dwarf_error_code_from_elf_sgi_error_code
			    (sres), DW_DLV_ERROR);
	}
#else
	Elf_Scn *scn;
	Elf_Data *data;

	scn = elf_getscn(dbg->de_elf, section_index);
	if (scn == NULL) {
	    _dwarf_error(dbg, error, DW_DLE_MDE);
	    return DW_DLV_ERROR;
	}

	/* 
	   When using libelf as a producer, section data may be stored
	   in multiple buffers. In libdwarf however, we only use libelf
	   as a consumer (there is a dwarf producer API, but it doesn't
	   use libelf). Because of this, this single call to elf_getdata
	   will retrieve the entire section in a single contiguous
	   buffer. */
	data = elf_getdata(scn, NULL);
	if (data == NULL) {
	    _dwarf_error(dbg, error, DW_DLE_MDE);
	    return DW_DLV_ERROR;
	}

	*section_data = data->d_buf;
#endif /* !defined(__SGI_FAST_LIBELF) */
    }

    return DW_DLV_OK;
}

/* This is a hack so clients can verify offsets.
   Added April 2005 so that debugger can detect broken offsets
   (which happened in an IRIX  -64 executable larger than 2GB
    using MIPSpro 7.3.1.3 compilers. A couple .debug_pubnames
    offsets were wrong.).
*/
int
dwarf_get_section_max_offsets(Dwarf_Debug dbg,
			      Dwarf_Unsigned * debug_info_size,
			      Dwarf_Unsigned * debug_abbrev_size,
			      Dwarf_Unsigned * debug_line_size,
			      Dwarf_Unsigned * debug_loc_size,
			      Dwarf_Unsigned * debug_aranges_size,
			      Dwarf_Unsigned * debug_macinfo_size,
			      Dwarf_Unsigned * debug_pubnames_size,
			      Dwarf_Unsigned * debug_str_size,
			      Dwarf_Unsigned * debug_frame_size,
			      Dwarf_Unsigned * debug_ranges_size,
			      Dwarf_Unsigned * debug_typenames_size)
{
    *debug_info_size = dbg->de_debug_info_size;
    *debug_abbrev_size = dbg->de_debug_abbrev_size;
    *debug_line_size = dbg->de_debug_line_size;
    *debug_loc_size = dbg->de_debug_loc_size;
    *debug_aranges_size = dbg->de_debug_aranges_size;
    *debug_macinfo_size = dbg->de_debug_macinfo_size;
    *debug_pubnames_size = dbg->de_debug_pubnames_size;
    *debug_str_size = dbg->de_debug_str_size;
    *debug_frame_size = dbg->de_debug_frame_size;
    *debug_ranges_size = 0;	/* Not yet supported. */
    *debug_typenames_size = dbg->de_debug_typenames_size;


    return DW_DLV_OK;
}
