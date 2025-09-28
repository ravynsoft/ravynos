# Copyright (C) 2014-2023 Free Software Foundation, Inc.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.

HEAP_SECTION_MSP430=" "
HEAP_MEMORY_MSP430=" "

if test ${GOT_HEAP_MSP-0} -ne 0
then
HEAP_SECTION_MSP430=".heap ${RELOCATING-0} :
  {
    ${RELOCATING+ PROVIDE (__heap_data_start = .) ; }
    *(.heap*)
    ${RELOCATING+ PROVIDE (_heap_data_end = .) ; }
    ${RELOCATING+. = ALIGN(2);}
    ${RELOCATING+ PROVIDE (__heap_bottom = .) ; }
    ${RELOCATING+ PROVIDE (__heap_top = ${HEAP_START} + ${HEAP_LENGTH}) ; }
  } ${RELOCATING+ > heap}"
HEAP_MEMORY_MSP430="heap(rwx)		: ORIGIN = $HEAP_START,	LENGTH = $HEAP_LENGTH"
fi


cat <<EOF
/* Copyright (C) 2014-2023 Free Software Foundation, Inc.

   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */

OUTPUT_FORMAT("${OUTPUT_FORMAT}","${OUTPUT_FORMAT}","${OUTPUT_FORMAT}")
OUTPUT_ARCH(${ARCH})

EOF

test -n "${RELOCATING}" && cat <<EOF
MEMORY
{
  text   (rx)		: ORIGIN = $ROM_START,  LENGTH = $ROM_SIZE
  data   (rwx)		: ORIGIN = $RAM_START,	LENGTH = $RAM_SIZE
  vectors (rw)		: ORIGIN = 0xffe0,      LENGTH = 0x20
  bootloader(rx)	: ORIGIN = 0x0c00,	LENGTH = 1K
  infomem(rx)		: ORIGIN = 0x1000,	LENGTH = 256
  infomemnobits(rx)	: ORIGIN = 0x1000,      LENGTH = 256
  ${HEAP_MEMORY_MSP430}
}

EOF

cat <<EOF
SECTIONS
{
  /* Bootloader.  */
  .bootloader ${RELOCATING-0} :
  {
    ${RELOCATING+ PROVIDE (__boot_start = .) ; }
    *(.bootloader)
    ${RELOCATING+. = ALIGN(2);}
    ${RELOCATING+*(.bootloader.*)}
  } ${RELOCATING+ > bootloader}

  /* Information memory.  */
  .infomem ${RELOCATING-0} :
  {
    *(.infomem)
    ${RELOCATING+. = ALIGN(2);}
    ${RELOCATING+*(.infomem.*)}
  } ${RELOCATING+ > infomem}

  /* Information memory (not loaded into MPU).  */
  .infomemnobits ${RELOCATING-0} :
  {
    *(.infomemnobits)
    ${RELOCATING+. = ALIGN(2);}
    ${RELOCATING+*(.infomemnobits.*)}
  } ${RELOCATING+ > infomemnobits}

  /* Read-only sections, merged into text segment.  */
  ${TEXT_DYNAMIC+${DYNAMIC}}
  .hash        ${RELOCATING-0} : { *(.hash)             }
  .dynsym      ${RELOCATING-0} : { *(.dynsym)           }
  .dynstr      ${RELOCATING-0} : { *(.dynstr)           }
  .gnu.version ${RELOCATING-0} : { *(.gnu.version)      }
  .gnu.version_d ${RELOCATING-0} : { *(.gnu.version_d)  }
  .gnu.version_r ${RELOCATING-0} : { *(.gnu.version_r)  }

  .rel.init    ${RELOCATING-0} : { *(.rel.init) }
  .rela.init   ${RELOCATING-0} : { *(.rela.init) }
  .rel.text    ${RELOCATING-0} :
    {
      *(.rel.text)
      ${RELOCATING+*(.rel.text.*)}
      ${RELOCATING+*(.rel.gnu.linkonce.t*)}
    }
  .rela.text   ${RELOCATING-0} :
    {
      *(.rela.text)
      ${RELOCATING+*(.rela.text.*)}
      ${RELOCATING+*(.rela.gnu.linkonce.t*)}
    }
  .rel.fini    ${RELOCATING-0} : { *(.rel.fini) }
  .rela.fini   ${RELOCATING-0} : { *(.rela.fini) }
  .rel.rodata  ${RELOCATING-0} :
    {
      *(.rel.rodata)
      ${RELOCATING+*(.rel.rodata.*)}
      ${RELOCATING+*(.rel.gnu.linkonce.r*)}
    }
  .rela.rodata ${RELOCATING-0} :
    {
      *(.rela.rodata)
      ${RELOCATING+*(.rela.rodata.*)}
      ${RELOCATING+*(.rela.gnu.linkonce.r*)}
    }
  .rel.data    ${RELOCATING-0} :
    {
      *(.rel.data)
      ${RELOCATING+*(.rel.data.*)}
      ${RELOCATING+*(.rel.gnu.linkonce.d*)}
    }
  .rela.data   ${RELOCATING-0} :
    {
      *(.rela.data)
      ${RELOCATING+*(.rela.data.*)}
      ${RELOCATING+*(.rela.gnu.linkonce.d*)}
    }
  .rel.ctors   ${RELOCATING-0} : { *(.rel.ctors)        }
  .rela.ctors  ${RELOCATING-0} : { *(.rela.ctors)       }
  .rel.dtors   ${RELOCATING-0} : { *(.rel.dtors)        }
  .rela.dtors  ${RELOCATING-0} : { *(.rela.dtors)       }
  .rel.got     ${RELOCATING-0} : { *(.rel.got)          }
  .rela.got    ${RELOCATING-0} : { *(.rela.got)         }
  .rel.bss     ${RELOCATING-0} : { *(.rel.bss)          }
  .rela.bss    ${RELOCATING-0} : { *(.rela.bss)         }
  .rel.plt     ${RELOCATING-0} : { *(.rel.plt)          }
  .rela.plt    ${RELOCATING-0} : { *(.rela.plt)         }

  /* Internal text space.  */
  .text ${RELOCATING-0} :
  {
    ${RELOCATING+. = ALIGN(2);
    *(SORT_NONE(.init))
    *(SORT_NONE(.init0))  /* Start here after reset.  */
    *(SORT_NONE(.init1))
    *(SORT_NONE(.init2))  /* Copy data loop  */
    *(SORT_NONE(.init3))
    *(SORT_NONE(.init4))  /* Clear bss  */
    *(SORT_NONE(.init5))
    *(SORT_NONE(.init6))  /* C++ constructors.  */
    *(SORT_NONE(.init7))
    *(SORT_NONE(.init8))
    *(SORT_NONE(.init9))  /* Call main().  */}

    ${CONSTRUCTING+ __ctors_start = . ; }
    ${CONSTRUCTING+ *(.ctors) }
    ${CONSTRUCTING+ __ctors_end = . ; }
    ${CONSTRUCTING+ __dtors_start = . ; }
    ${CONSTRUCTING+ *(.dtors) }
    ${CONSTRUCTING+ __dtors_end = . ; }

    ${RELOCATING+. = ALIGN(2);
    *(.lower.text.* .lower.text)

    . = ALIGN(2);}
    *(.text)
    ${RELOCATING+. = ALIGN(2);
    *(.text.*)
    . = ALIGN(2);
    *(.text:*)

    *(.either.text.* .either.text)

    *(.upper.text.* .upper.text)

    . = ALIGN(2);
    *(SORT_NONE(.fini9))
    *(SORT_NONE(.fini8))
    *(SORT_NONE(.fini7))
    *(SORT_NONE(.fini6))  /* C++ destructors.  */
    *(SORT_NONE(.fini5))
    *(SORT_NONE(.fini4))
    *(SORT_NONE(.fini3))
    *(SORT_NONE(.fini2))
    *(SORT_NONE(.fini1))
    *(SORT_NONE(.fini0))  /* Infinite loop after program termination.  */
    *(SORT_NONE(.fini))

    _etext = .;}
  } ${RELOCATING+ > text}

  .rodata ${RELOCATING-0} :
  {
    ${RELOCATING+. = ALIGN(2);
    *(.lower.rodata.* .lower.rodata)

    . = ALIGN(2);
    *(.plt)}
    *(.rodata${RELOCATING+ .rodata.* .gnu.linkonce.r.* .const .const:*})
    ${RELOCATING+*(.rodata1)

    *(.either.rodata.*) *(.either.rodata)

    *(.upper.rodata.* .upper.rodata)

    *(.eh_frame_hdr)
    KEEP (*(.eh_frame))

    KEEP (*(.gcc_except_table)) *(.gcc_except_table.*)

    . = ALIGN(2);
    PROVIDE (__preinit_array_start = .);
    KEEP (*(.preinit_array))
    PROVIDE (__preinit_array_end = .);

    . = ALIGN(2);
    PROVIDE (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array))
    PROVIDE (__init_array_end = .);

    . = ALIGN(2);
    PROVIDE (__fini_array_start = .);
    KEEP (*(.fini_array))
    KEEP (*(SORT(.fini_array.*)))
    PROVIDE (__fini_array_end = .);

    /* gcc uses crtbegin.o to find the start of the constructors, so
       we make sure it is first.  Because this is a wildcard, it
       doesn't matter if the user does not actually link against
       crtbegin.o; the linker won't look for a file to match a
       wildcard.  The wildcard also means that it doesn't matter which
       directory crtbegin.o is in.  */
    KEEP (*crtbegin*.o(.ctors))

    /* We don't want to include the .ctor section from from the
       crtend.o file until after the sorted ctors.  The .ctor section
       from the crtend file contains the end of ctors marker and it
       must be last */
    KEEP (*(EXCLUDE_FILE (*crtend*.o ) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))

    KEEP (*crtbegin*.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend*.o ) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))}
  } ${RELOCATING+ > text}

  .vectors ${RELOCATING-0} :
  {
    ${RELOCATING+ PROVIDE (__vectors_start = .) ; }
    *(.vectors${RELOCATING+*})
    ${RELOCATING+ _vectors_end = . ; }
  } ${RELOCATING+ > vectors}

  .data ${RELOCATING-0} :
  {
    ${RELOCATING+ PROVIDE (__data_start = .) ; }
    ${RELOCATING+ PROVIDE (__datastart = .) ; }
    ${RELOCATING+. = ALIGN(2);

    KEEP (*(.jcr))
    *(.data.rel.ro.local) *(.data.rel.ro*)
    *(.dynamic)

    . = ALIGN(2);
    *(.lower.data.* .lower.data)}

    *(.data)
    ${RELOCATING+*(.data.*)
    *(.gnu.linkonce.d*)
    KEEP (*(.gnu.linkonce.d.*personality*))
    *(.data1)

    *(.either.data.* .either.data)

    *(.upper.data.* .upper.data)

    *(.got.plt) *(.got)
    . = ALIGN(2);
    *(.sdata .sdata.* .gnu.linkonce.s.*)
    . = ALIGN(2);
    _edata = .;}
  } ${RELOCATING+ > data AT> text}

  ${RELOCATING+__romdatastart = LOADADDR(.data);
  __romdatacopysize = SIZEOF(.data);}

  .bss ${RELOCATING-0}${RELOCATING+SIZEOF(.data) + ADDR(.data)} :
  {
    ${RELOCATING+. = ALIGN(2);}
    ${RELOCATING+ PROVIDE (__bss_start = .); }
    ${RELOCATING+ PROVIDE (__bssstart = .);
    *(.lower.bss.* .lower.bss)
    . = ALIGN(2);}
    *(.bss)
    ${RELOCATING+*(.either.bss.* .either.bss)
    *(.upper.bss.* .upper.bss)
    *(COMMON)
    PROVIDE (__bss_end = .);}
  } ${RELOCATING+ > data}
  ${RELOCATING+ PROVIDE (__bsssize = SIZEOF(.bss)); }

  /* This section contains data that is not initialized during load,
     or during the application's initialization sequence.  */
  .noinit ${RELOCATING-0}${RELOCATING+SIZEOF(.bss) + ADDR(.bss)} :
  {
    ${RELOCATING+. = ALIGN(2);}
    ${RELOCATING+ PROVIDE (__noinit_start = .) ; }
    *(.noinit${RELOCATING+ .noinit.* .gnu.linkonce.n.*})
    ${RELOCATING+. = ALIGN(2);}
    ${RELOCATING+ PROVIDE (__noinit_end = .) ; }
  } ${RELOCATING+ > data}

  /* This section contains data that is initialized during load,
     but not during the application's initialization sequence.  */
  .persistent ${RELOCATING-0}${RELOCATING+SIZEOF(.noinit) + ADDR(.noinit)} :
  {
    ${RELOCATING+. = ALIGN(2);}
    ${RELOCATING+ PROVIDE (__persistent_start = .) ; }
    *(.persistent${RELOCATING+ .persistent.* .gnu.linkonce.p.*})
    ${RELOCATING+. = ALIGN(2);}
    ${RELOCATING+ PROVIDE (__persistent_end = .) ; }
  } ${RELOCATING+ > data}

  ${RELOCATING+ _end = . ;  }
  ${HEAP_SECTION_MSP430}

  /* Stabs for profiling information*/
  .profiler 0 : { *(.profiler) }

EOF

source_sh $srcdir/scripttempl/misc-sections.sc
source_sh $srcdir/scripttempl/DWARF.sc

test -n "${RELOCATING}" && cat <<EOF
  .MSP430.attributes 0 :
  {
    KEEP (*(.MSP430.attributes))
    KEEP (*(.gnu.attributes))
    KEEP (*(__TI_build_attributes))
  }

  PROVIDE (__stack = ${STACK}) ;
  PROVIDE (__data_start_rom = _etext) ;
  PROVIDE (__data_end_rom   = _etext + SIZEOF (.data)) ;
  PROVIDE (__noinit_start_rom = _etext + SIZEOF (.data)) ;
  PROVIDE (__noinit_end_rom = _etext + SIZEOF (.data) + SIZEOF (.noinit)) ;
  PROVIDE (__subdevice_has_heap = ${GOT_HEAP_MSP-0}) ;
EOF

cat <<EOF
}
EOF
