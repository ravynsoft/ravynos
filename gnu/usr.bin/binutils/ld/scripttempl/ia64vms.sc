# Linker script for Itanium VMS systems.
# Tristan Gingold <gingold@adacore.com>.
#
# Copyright (C) 2014-2023 Free Software Foundation, Inc.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.

# Using an empty script for ld -r is better than mashing together
# sections.  This hack likely leaves ld -Ur broken.
test -n "${RELOCATING}" || exit 0

PAGESIZE=0x10000
BLOCKSIZE=0x200

cat <<EOF
/* Copyright (C) 2014-2023 Free Software Foundation, Inc.

   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */

OUTPUT_FORMAT("${OUTPUT_FORMAT}")
${LIB_SEARCH_DIRS}
ENTRY(__entry)

SECTIONS
{
  /* RW segment.  */
  ${RELOCATING+. = ${PAGESIZE};}

  \$DATA\$ ALIGN (${BLOCKSIZE}) : {
    *(\$DATA\$ .data .data.*)
    *(\$BSS\$ .bss .bss.*)
  }

  /* Code segment.  Note: name must be \$CODE\$ */
  ${RELOCATING+. = ALIGN (${PAGESIZE});}

  \$CODE\$ ALIGN (${BLOCKSIZE}) : {
    *(\$CODE\$ .text .text.*)
  }
  .plt ALIGN (8) : {
    *(.plt)
  }

  /* RO segment.  */
  ${RELOCATING+. = ALIGN (${PAGESIZE});}

  /* RO initialized data.  */
  \$LITERAL\$ ALIGN (${BLOCKSIZE}) : {
    *(\$LITERAL\$)
    *(\$READONLY\$ .rodata .rodata.*)
    *(.jcr)
    *(.ctors)
    *(.dtors)
    *(.opd)
    *(.gcc_except_table)

    /* LIB$INITIALIZE stuff.  */
    *(LIB\$INITIALIZDZ)	/* Start marker.  */
    *(LIB\$INITIALIZD_)	/* Hi priority.  */
    *(LIB\$INITIALIZE)	/* User.  */
    *(LIB\$INITIALIZE$)	/* End marker.  */
  }

  /* Short segment.  */
  ${RELOCATING+. = ALIGN (${PAGESIZE});}

  .srodata : {
    *(.srodata .srodata.*)
  }
  .got ALIGN (8) : {
    *(.got)
  }
  .IA_64.pltoff ALIGN (16) : {
    *(.IA_64.pltoff)
  }
  \$TFR\$ ALIGN (16) : {
    /* Transfer vector.  */
    __entry = .;
    *(.transfer)
  }

  ${RELOCATING+. = ALIGN (${PAGESIZE});}

  \$RW_SHORT\$ ALIGN (${BLOCKSIZE}) : {
    *(.sdata .sdata.*)
    *(.sbss .sbss.*)
  }

  ${RELOCATING+. = ALIGN (${PAGESIZE});}

  .IA_64.unwind ALIGN (${BLOCKSIZE}) : {
    *(.IA_64.unwind .IA_64.unwind.*)
  }

  .IA_64.unwind_info ALIGN (8) : {
    *(.IA_64.unwind_info .IA_64.unwind_info.*)
  }

  ${RELOCATING+. = ALIGN (${PAGESIZE});}

  .dynamic /* \$DYNAMIC\$ */ ALIGN (${BLOCKSIZE}) : {
    *(.dynamic)
    *(.vmsdynstr)
    *(.fixups)
  }

  ${RELOCATING+. = ALIGN (${PAGESIZE});}

  .dynstr : { *(.dynstr) }

  .dynsym       ${RELOCATING-0} : { *(.dynsym) }
  .rela.got : { *(.rela.got) }
  .got.plt : { *(.got.plt) }
  .gnu.version_d : { *(.gnu.version_d) }
  .gnu.version : { *(.gnu.version) }
  .gnu.version_r : { *(.gnu.version_r) }
  .rela.IA_64.pltoff : { *(.rela.IA_64.pltoff) }

EOF

source_sh $srcdir/scripttempl/DWARF.sc

cat <<EOF
  .note : { *(.vms.note) }

  /DISCARD/ : { *(.note) *(.vms_display_name_info) }
}
EOF
