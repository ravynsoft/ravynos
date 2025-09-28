# Copyright (C) 2014-2023 Free Software Foundation, Inc.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.

# MMO is not a relocateable format, and we don't want to require an
# explicit (e.g.) "-m elf64mmix" when -r is used.

test -z $RELOCATEABLE_OUTPUT_FORMAT && RELOCATEABLE_OUTPUT_FORMAT=$OUTPUT_FORMAT
test -z ${RELOCATING+0} && OUTPUT_FORMAT=$RELOCATEABLE_OUTPUT_FORMAT

cat <<EOF
/* Copyright (C) 2014-2023 Free Software Foundation, Inc.

   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */

OUTPUT_FORMAT("$OUTPUT_FORMAT")
OUTPUT_ARCH(mmix)
${RELOCATING+ENTRY(Main)}
SECTIONS
{
  .text ${RELOCATING+ ${TEXT_START_ADDR}}:
  {
    /* FIXME: Move .init, .fini, .ctors and .dtors to their own sections.  */
    ${RELOCATING+ PROVIDE (_init_start = .);}
    ${RELOCATING+ PROVIDE (_init = .);}
    ${RELOCATING+ KEEP (*(SORT_NONE(.init)))}
    ${RELOCATING+ PROVIDE (_init_end = .);}

    *(.text)
    ${RELOCATING+*(.text.*)}
    ${RELOCATING+*(.gnu.linkonce.t*)}
    ${RELOCATING+*(.rodata)}
    ${RELOCATING+*(.rodata.*)}
    ${RELOCATING+*(.gnu.linkonce.r*)}

    ${RELOCATING+ PROVIDE (_fini_start = .);}
    ${RELOCATING+ PROVIDE (_fini = .);}
    ${RELOCATING+ KEEP (*(SORT_NONE(.fini)))}
    ${RELOCATING+ PROVIDE (_fini_end = .);}

    /* FIXME: Align ctors, dtors, ehframe.  */
    ${RELOCATING+ PROVIDE (_ctors_start = .);}
    ${RELOCATING+ PROVIDE (__ctors_start = .);}
    ${RELOCATING+ PROVIDE (_ctors = .);}
    ${RELOCATING+ PROVIDE (__ctors = .);}
    ${RELOCATING+ KEEP (*crtbegin.o(.ctors))}
    ${RELOCATING+ KEEP (*crtbegin?.o(.ctors))}
    ${RELOCATING+ KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o) .ctors))}
    ${RELOCATING+ KEEP (*(SORT(.ctors.*)))}
    ${RELOCATING+ KEEP (*(.ctors))}
    ${RELOCATING+ PROVIDE (_ctors_end = .);}
    ${RELOCATING+ PROVIDE (__ctors_end = .);}

    ${RELOCATING+ PROVIDE (_dtors_start = .);}
    ${RELOCATING+ PROVIDE (__dtors_start = .);}
    ${RELOCATING+ PROVIDE (_dtors = .);}
    ${RELOCATING+ PROVIDE (__dtors = .);}
    ${RELOCATING+ KEEP (*crtbegin.o(.dtors))}
    ${RELOCATING+ KEEP (*crtbegin?.o(.dtors))}
    ${RELOCATING+ KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o) .dtors))}
    ${RELOCATING+ KEEP (*(SORT(.dtors.*)))}
    ${RELOCATING+ KEEP (*(.dtors))}
    ${RELOCATING+ PROVIDE (_dtors_end = .);}
    ${RELOCATING+ PROVIDE (__dtors_end = .);}

    ${RELOCATING+KEEP (*(.jcr))}
    ${RELOCATING+KEEP (*(.eh_frame))}
    ${RELOCATING+*(.gcc_except_table)}

    ${RELOCATING+Main = DEFINED (Main) ? Main : (DEFINED (_start) ? _start : ADDR (.text));}
  }

  /* The following NOP assignment and those after .data and .bss, are
     necessary to get orphan sections adopted by the .text inserted before
     the following end-section symbols.  An output section would also serve
     this purpose, but we can't do that.  */
  . = .;
  ${RELOCATING+ PROVIDE(etext = .);}
  ${RELOCATING+ PROVIDE(_etext = .);}
  ${RELOCATING+ PROVIDE(__etext = .);}

  .data ${RELOCATING+ ${DATA_ADDR}}:
  {
    ${RELOCATING+ PROVIDE(__Sdata = .);}

    *(.data);
    ${RELOCATING+*(.data.*)}
    ${RELOCATING+*(.gnu.linkonce.d*)}
  }
  . = .;
  ${RELOCATING+ PROVIDE(__Edata = .);}
  /* Deprecated, use __Edata.  */
  ${RELOCATING+ PROVIDE(edata = .);}
  ${RELOCATING+ PROVIDE(_edata = .);}
  ${RELOCATING+ PROVIDE(__edata = .);}

  /* At the moment, although perhaps we should, we can't map sections
     without contents to sections *with* contents due to FIXME: a BFD bug.
     Anyway, the mmo back-end ignores sections without contents when
     writing out sections, so this works fine.   */
  .bss :
  {
    ${RELOCATING+ PROVIDE(__Sbss = .);}
    ${RELOCATING+ PROVIDE(__bss_start = .);}
    ${RELOCATING+ *(.sbss);}
    ${RELOCATING+ *(.bss);}
    ${RELOCATING+*(.bss.*)}
    ${RELOCATING+ *(COMMON);}
  }
  . = .;
  ${RELOCATING+ PROVIDE(__Ebss = .);}

  /* Deprecated, use __Ebss or __Eall as appropriate.  */
  ${RELOCATING+ PROVIDE(end = .);}
  ${RELOCATING+ PROVIDE(_end = .);}
  ${RELOCATING+ PROVIDE(__end = .);}
  ${RELOCATING+ PROVIDE(__Eall = .);}

  .stab 0 : { *(.stab) }
  .stabstr 0 : { *(.stabstr) }
  .stab.excl 0 : { *(.stab.excl) }
  .stab.exclstr 0 : { *(.stab.exclstr) }
  .stab.index 0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
EOF

source_sh $srcdir/scripttempl/DWARF.sc

cat <<EOF
  .MMIX.reg_contents :
  {
    /* Note that this section always has a fixed VMA - that of its
       first register * 8.  */
    *(.MMIX.reg_contents.linker_allocated);
    *(.MMIX.reg_contents);
  }

  ${RELOCATING+/* By default, put the high end of the stack where the register stack
     begins.  They grow in opposite directions.  */
  PROVIDE (__Stack_start = 0x6000000000000000);}

  /* Unfortunately, stabs are not mappable from ELF to MMO.
     It can probably be fixed with some amount of work.  */
  /DISCARD/ :
  { ${RELOCATING+ *(.gnu.warning.*);} }

  .gnu.attributes 0 : { KEEP (*(.gnu.attributes)) }
}
EOF
