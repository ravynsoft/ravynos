/*

  Copyright (C) 2000 Silicon Graphics, Inc.  All Rights Reserved.

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



/* This structure is used to read an arange into. */
struct Dwarf_Arange_s {

    /* Starting address of the arange, ie low-pc. */
    Dwarf_Addr ar_address;

    /* Length of the arange. */
    Dwarf_Unsigned ar_length;

    /* 
       Offset into .debug_info of the start of the compilation-unit
       containing this set of aranges. */
    Dwarf_Off ar_info_offset;

    /* Corresponding Dwarf_Debug. */
    Dwarf_Debug ar_dbg;
};



int
  _dwarf_get_aranges_addr_offsets(Dwarf_Debug dbg,
				  Dwarf_Addr ** addrs,
				  Dwarf_Off ** offsets,
				  Dwarf_Signed * count,
				  Dwarf_Error * error);
