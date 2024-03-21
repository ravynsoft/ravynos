# Linker script for Alpha VMS systems.
# Tristan Gingold <gingold@adacore.com>.
#
# Copyright (C) 2014-2023 Free Software Foundation, Inc.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.

PAGESIZE=0x10000

cat <<EOF
/* Copyright (C) 2014-2023 Free Software Foundation, Inc.

   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */

OUTPUT_FORMAT("${OUTPUT_FORMAT}")
${LIB_SEARCH_DIRS}

SECTIONS
{
  ${RELOCATING+. = ${PAGESIZE};}

  /* RW initialized data.  */
  \$DATA\$ ALIGN (${PAGESIZE}) : {
    *(\$DATA\$)
  }
  /* RW data unmodified (zero-initialized).  */
  \$BSS\$ ALIGN (${PAGESIZE}) : {
    *(\$BSS\$)
  }
  /* RO, executable code.  */
  \$CODE\$ ALIGN (${PAGESIZE}) : {
    *(\$CODE\$${RELOCATING+ *\$CODE*})
  }
  /* RO initialized data.  */
  \$LITERAL\$ ALIGN (${PAGESIZE}) : {
    ${RELOCATING+*(\$LINK\$)}
    *(\$LITERAL\$)
    ${RELOCATING+*(\$READONLY\$)
    *(\$READONLY_ADDR\$)
    *(eh_frame)
    *(jcr)
    *(ctors)
    *(dtors)
    *(gcc_except_table)

    /* LIB$INITIALIZE stuff.  */
    *(LIB\$INITIALIZDZ)	/* Start marker.  */
    *(LIB\$INITIALIZD_)	/* Hi priority.  */
    *(LIB\$INITIALIZE)	/* User.  */
    *(LIB\$INITIALIZE$)	/* End marker.  */}
  }

  \$DWARF\$ ALIGN (${PAGESIZE}) : {
    ${RELOCATING+\$dwarf2.debug_pubtypes = .;
    *(debug_pubtypes)
    \$dwarf2.debug_ranges = .;
    *(debug_ranges)

    \$dwarf2.debug_abbrev = .;
    *(debug_abbrev)
    \$dwarf2.debug_aranges = .;
    *(debug_aranges)
    \$dwarf2.debug_frame = .;
    *(debug_frame)
    \$dwarf2.debug_info = .;
    *(debug_info)
    \$dwarf2.debug_line = .;
    *(debug_line)
    \$dwarf2.debug_loc = .;
    *(debug_loc)
    \$dwarf2.debug_macinfo = .;
    *(debug_macinfo)
    \$dwarf2.debug_macro = .;
    *(debug_macro)
    \$dwarf2.debug_pubnames = .;
    *(debug_pubnames)
    \$dwarf2.debug_str = .;
    *(debug_str)
    \$dwarf2.debug_zzzzzz = .;}
  }

  \$DST\$ 0 : {
    *(\$DST\$)
  }
}
EOF
