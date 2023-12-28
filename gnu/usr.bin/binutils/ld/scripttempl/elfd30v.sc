# Copyright (C) 2014-2023 Free Software Foundation, Inc.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.

CTOR=".ctors ${CONSTRUCTING-0} :
  {
    ${CONSTRUCTING+ __CTOR_LIST__ = .; }
    /* gcc uses crtbegin.o to find the start of
       the constructors, so we make sure it is
       first.  Because this is a wildcard, it
       doesn't matter if the user does not
       actually link against crtbegin.o; the
       linker won't look for a file to match a
       wildcard.  The wildcard also means that it
       doesn't matter which directory crtbegin.o
       is in.  */

    KEEP (*crtbegin.o(.ctors))
    KEEP (*crtbegin?.o(.ctors))

    /* We don't want to include the .ctor section from
       the crtend.o file until after the sorted ctors.
       The .ctor section from the crtend file contains the
       end of ctors marker and it must be last */

    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    ${CONSTRUCTING+ __CTOR_END__ = .; }
  } ${RELOCATING+ > ${DATA_MEMORY}}"

DTOR="  .dtors	${CONSTRUCTING-0} :
  {
    ${CONSTRUCTING+ __DTOR_LIST__ = .; }
    KEEP (*crtbegin.o(.dtors))
    KEEP (*crtbegin?.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    ${CONSTRUCTING+ __DTOR_END__ = .; }
  } ${RELOCATING+ > ${DATA_MEMORY}}"

cat <<EOF
/* Copyright (C) 2014-2023 Free Software Foundation, Inc.

   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */

OUTPUT_FORMAT("${OUTPUT_FORMAT}")
OUTPUT_ARCH(${ARCH})

EOF

test -n "${RELOCATING}" && cat <<EOF
MEMORY
{
  text ${TEXT_DEF_SECTION} : ORIGIN = ${TEXT_START_ADDR}, LENGTH = ${TEXT_SIZE}
  data ${DATA_DEF_SECTION} : ORIGIN = ${DATA_START_ADDR}, LENGTH = ${DATA_SIZE}
  emem ${EMEM_DEF_SECTION} : ORIGIN = ${EMEM_START_ADDR}, LENGTH = ${EMEM_SIZE}
  eit			   : ORIGIN = ${EIT_START_ADDR},  LENGTH = ${EIT_SIZE}
}

EOF

cat <<EOF
SECTIONS
{
  /* Read-only sections, merged into text segment: */
  ${TEXT_DYNAMIC+${DYNAMIC}}
  .hash			${RELOCATING-0} : { *(.hash) }
  .dynsym		${RELOCATING-0} : { *(.dynsym) }
  .dynstr		${RELOCATING-0} : { *(.dynstr) }
  .gnu.version		${RELOCATING-0} : { *(.gnu.version) }
  .gnu.version_d	${RELOCATING-0} : { *(.gnu.version_d) }
  .gnu.version_r	${RELOCATING-0} : { *(.gnu.version_r) }

  .rel.text		${RELOCATING-0} : { *(.rel.text${RELOCATING+ .rel.gnu.linkonce.t*}) }
  .rela.text		${RELOCATING-0} : { *(.rela.text${RELOCATING+ .rela.gnu.linkonce.t*}) }
  .rel.data		${RELOCATING-0} : { *(.rel.data${RELOCATING+ .rel.gnu.linkonce.d*}) }
  .rela.data		${RELOCATING-0} : { *(.rela.data${RELOCATING+ .rela.gnu.linkonce.d*}) }
  .rel.rodata		${RELOCATING-0} : { *(.rel.rodata${RELOCATING+ .rel.gnu.linkonce.r*}) }
  .rela.rodata		${RELOCATING-0} : { *(.rela.rodata${RELOCATING+ .rela.gnu.linkonce.r*}) }
  .rel.stext		${RELOCATING-0} : { *(.rel.stest) }
  .rela.stext		${RELOCATING-0} : { *(.rela.stest) }
  .rel.etext		${RELOCATING-0} : { *(.rel.etest) }
  .rela.etext		${RELOCATING-0} : { *(.rela.etest) }
  .rel.sdata		${RELOCATING-0} : { *(.rel.sdata) }
  .rela.sdata		${RELOCATING-0} : { *(.rela.sdata) }
  .rel.edata		${RELOCATING-0} : { *(.rel.edata) }
  .rela.edata		${RELOCATING-0} : { *(.rela.edata) }
  .rel.eit_v		${RELOCATING-0} : { *(.rel.eit_v) }
  .rela.eit_v		${RELOCATING-0} : { *(.rela.eit_v) }
  .rel.sbss		${RELOCATING-0} : { *(.rel.sbss) }
  .rela.sbss		${RELOCATING-0} : { *(.rela.sbss) }
  .rel.ebss		${RELOCATING-0} : { *(.rel.ebss) }
  .rela.ebss		${RELOCATING-0} : { *(.rela.ebss) }
  .rel.srodata		${RELOCATING-0} : { *(.rel.srodata) }
  .rela.srodata		${RELOCATING-0} : { *(.rela.srodata) }
  .rel.erodata		${RELOCATING-0} : { *(.rel.erodata) }
  .rela.erodata		${RELOCATING-0} : { *(.rela.erodata) }
  .rel.got		${RELOCATING-0} : { *(.rel.got) }
  .rela.got		${RELOCATING-0} : { *(.rela.got) }
  .rel.ctors		${RELOCATING-0} : { *(.rel.ctors) }
  .rela.ctors		${RELOCATING-0} : { *(.rela.ctors) }
  .rel.dtors		${RELOCATING-0} : { *(.rel.dtors) }
  .rela.dtors		${RELOCATING-0} : { *(.rela.dtors) }
  .rel.init		${RELOCATING-0} : { *(.rel.init) }
  .rela.init		${RELOCATING-0} : { *(.rela.init) }
  .rel.fini		${RELOCATING-0} : { *(.rel.fini) }
  .rela.fini		${RELOCATING-0} : { *(.rela.fini) }
  .rel.bss		${RELOCATING-0} : { *(.rel.bss) }
  .rela.bss		${RELOCATING-0} : { *(.rela.bss) }
  .rel.plt		${RELOCATING-0} : { *(.rel.plt) }
  .rela.plt		${RELOCATING-0} : { *(.rela.plt) }

  .init			${RELOCATING-0} : { KEEP (*(SORT_NONE(.init))) } =${NOP-0}
  ${DATA_PLT-${PLT}}

  /* Internal text space */
  .stext	${RELOCATING-0} : { *(.stext) }		${RELOCATING+ > text}

  /* Internal text space or external memory */
  .text :
  {
    *(.text)
    ${RELOCATING+*(.gnu.linkonce.t*)
    KEEP (*(SORT_NONE(.fini)))
    _etext = . ;}
  } ${RELOCATING+ > ${TEXT_MEMORY}}

  /* Internal data space */
  .srodata	${RELOCATING-0} : { *(.srodata) }	${RELOCATING+ > data}
  .sdata	${RELOCATING-0} : { *(.sdata) }		${RELOCATING+ > data}

  /* Internal data space or external memory */
  .rodata	${RELOCATING-0} : { *(.rodata) }	${RELOCATING+ > ${DATA_MEMORY}}

  /* C++ exception support.  */
  .eh_frame	${RELOCATING-0} : { KEEP (*(.eh_frame)) }	${RELOCATING+ > ${DATA_MEMORY}}
  .gcc_except_table ${RELOCATING-0} : { *(.gcc_except_table) }	${RELOCATING+ > ${DATA_MEMORY}}

  /* Java class registration support.  */
  .jcr		${RELOCATING-0} : { KEEP (*(.jcr)) }	${RELOCATING+ >${DATA_MEMORY}}

  ${RELOCATING+${CTOR}}
  ${RELOCATING+${DTOR}}

  .data		${RELOCATING-0} :
  {
    *(.data)
    ${RELOCATING+*(.gnu.linkonce.d*)}
    ${CONSTRUCTING+CONSTRUCTORS}
    ${RELOCATING+ _edata = . ; }
  } ${RELOCATING+ > ${DATA_MEMORY}}

  /* External memory */
  .etext	${RELOCATING-0} :
  {
    ${RELOCATING+ PROVIDE (__etext_start = .) ; }
    *(.etext)
    ${RELOCATING+ PROVIDE (__etext_end = .) ; }
  } ${RELOCATING+ > emem}

  .erodata	${RELOCATING-0} : { *(.erodata) }	${RELOCATING+ > emem}
  .edata	${RELOCATING-0} : { *(.edata) }		${RELOCATING+ > emem}

  .sbss		${RELOCATING-0} :
  {
    ${RELOCATING+ PROVIDE (__sbss_start = .) ; }
    *(.sbss)
    ${RELOCATING+ PROVIDE (__sbss_end = .) ;  }
  } ${RELOCATING+ > data}

  .ebss		${RELOCATING-0} :
  {
    ${RELOCATING+ PROVIDE (__ebss_start = .) ; }
    *(.ebss)
    ${RELOCATING+ PROVIDE (__ebss_end = .) ;  }
  } ${RELOCATING+ > data}

  .bss		${RELOCATING-0} :
  {
    ${RELOCATING+ PROVIDE (__bss_start = .) ; }
    *(.bss)
    ${RELOCATING+*(COMMON)}
    ${RELOCATING+ PROVIDE (__bss_end = .) ; }
    ${RELOCATING+ _end = . ;  }
  } ${RELOCATING+ > ${DATA_MEMORY}}

  .eit_v	${RELOCATING-0} :
  {
    ${RELOCATING+ PROVIDE (__eit_start = .) ; }
    *(.eit_v)
    ${RELOCATING+ PROVIDE (__eit_end = .) ; }
  } ${RELOCATING+ > eit}

EOF

source_sh $srcdir/scripttempl/misc-sections.sc
source_sh $srcdir/scripttempl/DWARF.sc

cat <<EOF
  ${RELOCATING+PROVIDE (__stack = ${STACK_START_ADDR});}
}
EOF



