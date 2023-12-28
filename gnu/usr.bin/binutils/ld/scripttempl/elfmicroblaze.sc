# Adapted from mips.sc
#
# Copyright (C) 2014-2023 Free Software Foundation, Inc.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.
#
# These variables may be overridden by the emulation file.  The
# defaults are appropriate for a DECstation running Ultrix.

test -z "$ENTRY" && ENTRY=_start

#test -z "$TEXT_START_ADDR" && TEXT_START_ADDR="0x0"

CTOR=".ctors ${CONSTRUCTING-0} :
  {
    ${CONSTRUCTING+${CTOR_START}}
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

    /* We don't want to include the .ctor section from
       from the crtend.o file until after the sorted ctors.
       The .ctor section from the crtend file contains the
       end of ctors marker and it must be last */

    KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    ${CONSTRUCTING+${CTOR_END}}
  }"

DTOR=" .dtors       ${CONSTRUCTING-0} :
  {
    ${CONSTRUCTING+${DTOR_START}}
    KEEP (*crtbegin.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    ${CONSTRUCTING+${DTOR_END}}
  }"

cat <<EOF
/* Copyright (C) 2014-2023 Free Software Foundation, Inc.

   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */

OUTPUT_FORMAT("${OUTPUT_FORMAT}", "${BIG_OUTPUT_FORMAT}",
	      "${LITTLE_OUTPUT_FORMAT}")
/*${LIB_SEARCH_DIRS}*/
${RELOCATING+${LIB_SEARCH_DIRS}}

${RELOCATING+ENTRY (${ENTRY})}

${RELOCATING+_TEXT_START_ADDR = DEFINED(_TEXT_START_ADDR) ? _TEXT_START_ADDR : 0x50;
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x0;
_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x400;}

SECTIONS
{
  .vectors.reset 0x0 : { KEEP (*(.vectors.reset)) } = 0
  .vectors.sw_exception 0x8 : { KEEP (*(.vectors.sw_exception)) } = 0
  .vectors.interrupt 0x10 : { KEEP (*(.vectors.interrupt)) } = 0
  .vectors.debug_sw_break 0x18 : { KEEP (*(.vectors.debug_sw_break)) } = 0
  .vectors.hw_exception 0x20 : { KEEP (*(.vectors.hw_exception)) } = 0

  ${RELOCATING+. = _TEXT_START_ADDR;}

  ${RELOCATING+ _ftext  =  .;}
  .text : {
    *(.text)
    ${RELOCATING+*(.text.*)}
    ${RELOCATING+*(.gnu.linkonce.t.*)}
  }
  ${RELOCATING+ _etext  =  .;}

  .init : { KEEP (*(SORT_NONE(.init)))	} =0
  .fini : { KEEP (*(SORT_NONE(.fini)))	} =0

  ${RELOCATING+PROVIDE (__CTOR_LIST__ = .);}
  ${RELOCATING+PROVIDE (___CTOR_LIST__ = .);}
  ${RELOCATING+${CTOR}}
  ${RELOCATING+PROVIDE (__CTOR_END__ = .);}
  ${RELOCATING+PROVIDE (___CTOR_END__ = .);}

  ${RELOCATING+PROVIDE (__DTOR_LIST__ = .);}
  ${RELOCATING+PROVIDE (___DTOR_LIST__ = .);}
  ${RELOCATING+${DTOR}}
  ${RELOCATING+PROVIDE (__DTOR_END__ = .);}
  ${RELOCATING+PROVIDE (___DTOR_END__ = .);}

  ${RELOCATING+ . = ALIGN(4);}
   ${RELOCATING+ _frodata = . ;}
  .rodata : {
    *(.rodata)
    ${RELOCATING+*(.rodata.*)}
    ${RELOCATING+*(.gnu.linkonce.r.*)}
    ${CONSTRUCTING+CONSTRUCTORS;} /* Is this needed? */
  }
  ${RELOCATING+ _erodata = .;}

  /* Alignments by 8 to ensure that _SDA2_BASE_ on a word boundary */
  /* Note that .sdata2 and .sbss2 must be contiguous */
  ${RELOCATING+. = ALIGN(8);}
  ${RELOCATING+ _ssrw = .;}
  .sdata2 : {
    *(.sdata2)
    ${RELOCATING+*(.sdata2.*)}
    ${RELOCATING+*(.gnu.linkonce.s2.*)}
  }
  ${RELOCATING+. = ALIGN(4);}
  .sbss2 : {
    ${RELOCATING+PROVIDE (__sbss2_start = .);}
    *(.sbss2)
    ${RELOCATING+*(.sbss2.*)}
    ${RELOCATING+*(.gnu.linkonce.sb2.*)}
    ${RELOCATING+PROVIDE (__sbss2_end = .);}
  }
  ${RELOCATING+. = ALIGN(8);}
  ${RELOCATING+ _essrw = .;}
  ${RELOCATING+ _ssrw_size = _essrw - _ssrw;}
  ${RELOCATING+ PROVIDE (_SDA2_BASE_ = _ssrw + (_ssrw_size / 2 ));}

  ${RELOCATING+ . = ALIGN(4);}
  ${RELOCATING+ _fdata = .;}
  .data : {
    *(.data)
    ${RELOCATING+*(.data.*)}
    ${RELOCATING+*(.gnu.linkonce.d.*)}
    ${CONSTRUCTING+CONSTRUCTORS;} /* Is this needed? */
  }
  ${RELOCATING+ _edata = . ;}

   /* Added to handle pic code */
  .got : {
    *(.got)
  }

  .got1 : {
    *(.got1)
  }

  .got2 : {
    *(.got2)
  }

  /* Added by Sathya to handle C++ exceptions */
  .eh_frame : {
    *(.eh_frame)
  }

  .jcr : {
    *(.jcr)
  }

  .gcc_except_table : {
    *(.gcc_except_table)
  }

  /* Alignments by 8 to ensure that _SDA_BASE_ on a word boundary */
  /* Note that .sdata and .sbss must be contiguous */
  ${RELOCATING+. = ALIGN(8);}
  ${RELOCATING+ _ssro = .;}
  .sdata : {
    *(.sdata)
    ${RELOCATING+*(.sdata.*)}
    ${RELOCATING+*(.gnu.linkonce.s.*)}
  }
  ${RELOCATING+. = ALIGN(4);}
  .sbss : {
    ${RELOCATING+PROVIDE (__sbss_start = .);}
    *(.sbss)
    ${RELOCATING+*(.sbss.*)}
    ${RELOCATING+*(.gnu.linkonce.sb.*)}
    ${RELOCATING+PROVIDE (__sbss_end = .);}
  }
  ${RELOCATING+. = ALIGN(8);}
  ${RELOCATING+ _essro = .;}
  ${RELOCATING+ _ssro_size = _essro - _ssro;}
  ${RELOCATING+PROVIDE (_SDA_BASE_ = _ssro + (_ssro_size / 2 ));}

  ${RELOCATING+ . = ALIGN(4);}
  ${RELOCATING+ _fbss = .;}
  .bss : {
    ${RELOCATING+PROVIDE (__bss_start = .);}
    *(.bss)
    ${RELOCATING+*(.bss.*)}
    ${RELOCATING+*(.gnu.linkonce.b.*)}
    ${RELOCATING+*(COMMON)}
    ${RELOCATING+. = ALIGN(. != 0 ? 4 : 1);}

    ${RELOCATING+PROVIDE (__bss_end = .);}
  }

  ${RELOCATING+ . = ALIGN(4);}

  .heap : {
    ${RELOCATING+ _heap = .;}
    ${RELOCATING+ _heap_start = .;}
    ${RELOCATING+ . += _HEAP_SIZE;}
    ${RELOCATING+ _heap_end = .;}
  }

  ${RELOCATING+ . = ALIGN(4);}

  .stack : {
    ${RELOCATING+ _stack_end = .;}
    ${RELOCATING+ . += _STACK_SIZE;}
    ${RELOCATING+ . = ALIGN(. != 0 ? 8 : 1);}
    ${RELOCATING+ _stack = .;}
    ${RELOCATING+ _end = .;}
  }

  .tdata : {
    *(.tdata)
    ${RELOCATING+*(.tdata.*)}
    ${RELOCATING+*(.gnu.linkonce.td.*)}
  }
  .tbss : {
    *(.tbss)
    ${RELOCATING+*(.tbss.*)}
    ${RELOCATING+*(.gnu.linkonce.tb.*)}
  }
EOF

source_sh $srcdir/scripttempl/misc-sections.sc
source_sh $srcdir/scripttempl/DWARF.sc

cat <<EOF
}
EOF
