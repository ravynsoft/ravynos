/*

  Copyright (C) 2000,2003,2004 Silicon Graphics, Inc.  All Rights Reserved.

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



/*
    Decodes unsigned leb128 encoded numbers.
    Make sure ptr is a pointer to a 1-byte type.  
    In 2003 and earlier this was a hand-inlined
    version of _dwarf_decode_u_leb128() which did
    not work correctly if Dwarf_Word was 64 bits.
*/
#define DECODE_LEB128_UWORD(ptr, value) \
    { \
       Dwarf_Word uleblen; \
	value = _dwarf_decode_u_leb128(ptr,&uleblen); \
        ptr += uleblen; \
    }

/*
    Decodes signed leb128 encoded numbers.
    Make sure ptr is a pointer to a 1-byte type.
    In 2003 and earlier this was a hand-inlined
    version of _dwarf_decode_s_leb128() which did
    not work correctly if Dwarf_Word was 64 bits.

*/
#define DECODE_LEB128_SWORD(ptr, value) \
    { \
       Dwarf_Word sleblen; \
	value = _dwarf_decode_s_leb128(ptr,&sleblen); \
        ptr += sleblen; \
    }


/*
    Skips leb128_encoded numbers that are guaranteed 
    to be no more than 4 bytes long.  Same for both
    signed and unsigned numbers.
*/
#define SKIP_LEB128_WORD(ptr) \
    if ((*(ptr++) & 0x80) != 0) { \
        if ((*(ptr++) & 0x80) != 0) { \
            if ((*(ptr++) & 0x80) != 0) { \
	        if ((*(ptr++) & 0x80) != 0) { \
	        } \
	    } \
        } \
    }


#define CHECK_DIE(die, error_ret_value) \
    if (die == NULL) { \
	_dwarf_error(NULL, error, DW_DLE_DIE_NULL); \
	return(error_ret_value); \
    } \
    if (die->di_cu_context == NULL) { \
	_dwarf_error(NULL, error, DW_DLE_DIE_NO_CU_CONTEXT); \
	return(error_ret_value); \
    } \
    if (die->di_cu_context->cc_dbg == NULL) { \
	_dwarf_error(NULL, error, DW_DLE_DBG_NULL); \
	return(error_ret_value); \
    }


/* 
   Reads 'source' for 'length' bytes from unaligned addr.

   Avoids any constant-in-conditional warnings and
   avoids a test in the generated code (for non-const cases,
	which are in the majority.)
   Uses a temp to avoid the test.
   The decl here should avoid any problem of size in the temp.
   This code is ENDIAN DEPENDENT
   The memcpy args are the endian issue.
*/
typedef Dwarf_Unsigned BIGGEST_UINT;

#ifdef WORDS_BIGENDIAN
#define READ_UNALIGNED(dbg,dest,desttype, source, length) \
    { \
      BIGGEST_UINT _ltmp = 0;  \
      dbg->de_copy_word( (((char *)(&_ltmp)) + sizeof(_ltmp) - length), \
			source, length) ; \
      dest = (desttype)_ltmp;  \
    }


/*
    This macro sign-extends a variable depending on the length.
    It fills the bytes between the size of the destination and
    the length with appropriate padding.
    This code is ENDIAN DEPENDENT but dependent only
    on host endianness, not object file endianness.
    The memcpy args are the issue.
*/
#define SIGN_EXTEND(dest, length) \
    if (*(Dwarf_Sbyte *)((char *)&dest + sizeof(dest) - length) < 0) \
	memcpy((char *)&dest, "\xff\xff\xff\xff\xff\xff\xff\xff", \
	    sizeof(dest) - length)
#else /* LITTLE ENDIAN */

#define READ_UNALIGNED(dbg,dest,desttype, source, length) \
    { \
      BIGGEST_UINT _ltmp = 0;  \
      dbg->de_copy_word( (char *)(&_ltmp) , \
                        source, length) ; \
      dest = (desttype)_ltmp;  \
    }


/*
    This macro sign-extends a variable depending on the length.
    It fills the bytes between the size of the destination and
    the length with appropriate padding.
    This code is ENDIAN DEPENDENT but dependent only
    on host endianness, not object file endianness.
    The memcpy args are the issue.
*/
#define SIGN_EXTEND(dest, length) \
    if (*(Dwarf_Sbyte *)((char *)&dest + (length-1)) < 0) \
        memcpy((char *)&dest+length,    \
                "\xff\xff\xff\xff\xff\xff\xff\xff", \
            sizeof(dest) - length)

#endif /* ! LITTLE_ENDIAN */



/*
   READ_AREA LENGTH reads the length (the older way
   of pure 32 or 64 bit
   or the new proposed dwarfv2.1 64bit-extension way)

   It reads the bits from where rw_src_data_p  points to 
   and updates the rw_src_data_p to point past what was just read.

   It updates w_length_size and w_exten_size (which
	are really issues only for the dwarfv2.1  64bit extension).

   r_dbg is just the current dbg pointer.
   w_target is the output length field.
   r_targtype is the output type. Always Dwarf_Unsigned so far.
  
*/
/* This one handles the v2.1 64bit extension  
   and 32bit (and   MIPS fixed 64  bit via the
	dwarf_init-set r_dbg->de_length_size)..
   It does not recognize any but the one distingushed value
   (the only one with defined meaning).
   It assumes that no CU will have a length
	0xffffffxx  (32bit length)
	or
	0xffffffxx xxxxxxxx (64bit length)
   which makes possible auto-detection of the extension.

   This depends on knowing that only a non-zero length
   is legitimate (AFAICT), and for IRIX non-standard -64 
   dwarf that the first 32 bits of the 64bit offset will be
   zero (because the compiler could not handle a truly large 
   value as of Jan 2003 and because no app has that much debug 
   info anyway (yet)).

   At present not testing for '64bit elf' here as that
   does not seem necessary (none of the 64bit length seems 
   appropriate unless it's  ident[EI_CLASS] == ELFCLASS64).
   Might be a good idea though.

*/
#   define    READ_AREA_LENGTH(r_dbg,w_target,r_targtype,         \
	rw_src_data_p,w_length_size,w_exten_size)                 \
    READ_UNALIGNED(r_dbg,w_target,r_targtype,                     \
                rw_src_data_p, ORIGINAL_DWARF_OFFSET_SIZE);       \
    if(w_target == DISTINGUISHED_VALUE) {                         \
	     /* dwarf3 64bit extension */                         \
             w_length_size  = DISTINGUISHED_VALUE_OFFSET_SIZE;    \
             rw_src_data_p += ORIGINAL_DWARF_OFFSET_SIZE;         \
             w_exten_size   = ORIGINAL_DWARF_OFFSET_SIZE;         \
             READ_UNALIGNED(r_dbg,w_target,r_targtype,            \
                  rw_src_data_p, DISTINGUISHED_VALUE_OFFSET_SIZE);\
             rw_src_data_p += DISTINGUISHED_VALUE_OFFSET_SIZE;    \
    } else {                                                      \
	if(w_target == 0 && r_dbg->de_big_endian_object) {        \
	     /* IRIX 64 bit, big endian */                        \
             READ_UNALIGNED(r_dbg,w_target,r_targtype,            \
                rw_src_data_p, DISTINGUISHED_VALUE_OFFSET_SIZE);  \
	     w_length_size  = DISTINGUISHED_VALUE_OFFSET_SIZE;    \
	     rw_src_data_p += DISTINGUISHED_VALUE_OFFSET_SIZE;    \
	     w_exten_size = 0;                                    \
	} else {                                                  \
	     /* standard 32 bit dwarf2/dwarf3 */                  \
	     w_exten_size   = 0;                                  \
             w_length_size  = ORIGINAL_DWARF_OFFSET_SIZE;         \
             rw_src_data_p += w_length_size;                      \
	}                                                         \
    }



Dwarf_Unsigned
_dwarf_decode_u_leb128(Dwarf_Small * leb128,
		       Dwarf_Word * leb128_length);

Dwarf_Signed
_dwarf_decode_s_leb128(Dwarf_Small * leb128,
		       Dwarf_Word * leb128_length);

Dwarf_Unsigned
_dwarf_get_size_of_val(Dwarf_Debug dbg,
		       Dwarf_Unsigned form,
		       Dwarf_Small * val_ptr, int v_length_size);

/*
    This struct is used to build a hash table for the
    abbreviation codes for a compile-unit.  
*/
struct Dwarf_Hash_Table_s {
    Dwarf_Abbrev_List at_head;
    Dwarf_Abbrev_List at_tail;
};

Dwarf_Abbrev_List
_dwarf_get_abbrev_for_code(Dwarf_CU_Context cu_context,
			   Dwarf_Word code);


/* return 1 if string ends before 'endptr' else
** return 0 meaning string is not properly terminated.
** Presumption is the 'endptr' pts to end of some dwarf section data.
*/
int _dwarf_string_valid(void *startptr, void *endptr);

Dwarf_Unsigned _dwarf_length_of_cu_header(Dwarf_Debug,
					  Dwarf_Unsigned offset);
Dwarf_Unsigned _dwarf_length_of_cu_header_simple(Dwarf_Debug);

int _dwarf_load_debug_info(Dwarf_Debug dbg, Dwarf_Error *error);
