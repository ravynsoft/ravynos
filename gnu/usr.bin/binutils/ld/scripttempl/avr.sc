# Copyright (C) 2014-2023 Free Software Foundation, Inc.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.

# RODATA_PM_OFFSET
#         If empty, .rodata sections will be part of .data.  This is for
#         devices where it is not possible to use LD* instructions to read
#         from flash.
#
#         If non-empty, .rodata is not part of .data and the .rodata
#         objects are assigned addresses at an offest of RODATA_PM_OFFSET.
#         This is for devices that feature reading from flash by means of
#         LD* instructions, provided the addresses are offset by
#         __RODATA_PM_OFFSET__ (which defaults to RODATA_PM_OFFSET).

cat <<EOF
/* Copyright (C) 2014-2023 Free Software Foundation, Inc.

   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */

OUTPUT_FORMAT("${OUTPUT_FORMAT}","${OUTPUT_FORMAT}","${OUTPUT_FORMAT}")
OUTPUT_ARCH(${ARCH})
EOF

test -n "${RELOCATING}" && cat <<EOF
__TEXT_REGION_LENGTH__ = DEFINED(__TEXT_REGION_LENGTH__) ? __TEXT_REGION_LENGTH__ : $TEXT_LENGTH;
__DATA_REGION_ORIGIN__ = DEFINED(__DATA_REGION_ORIGIN__) ? __DATA_REGION_ORIGIN__ : $DATA_ORIGIN;
__DATA_REGION_LENGTH__ = DEFINED(__DATA_REGION_LENGTH__) ? __DATA_REGION_LENGTH__ : $DATA_LENGTH;

${EEPROM_LENGTH+__EEPROM_REGION_LENGTH__ = DEFINED(__EEPROM_REGION_LENGTH__) ? __EEPROM_REGION_LENGTH__ : $EEPROM_LENGTH;}
__FUSE_REGION_LENGTH__ = DEFINED(__FUSE_REGION_LENGTH__) ? __FUSE_REGION_LENGTH__ : $FUSE_LENGTH;
__LOCK_REGION_LENGTH__ = DEFINED(__LOCK_REGION_LENGTH__) ? __LOCK_REGION_LENGTH__ : $LOCK_LENGTH;
__SIGNATURE_REGION_LENGTH__ = DEFINED(__SIGNATURE_REGION_LENGTH__) ? __SIGNATURE_REGION_LENGTH__ : $SIGNATURE_LENGTH;
${USER_SIGNATURE_LENGTH+__USER_SIGNATURE_REGION_LENGTH__ = DEFINED(__USER_SIGNATURE_REGION_LENGTH__) ? __USER_SIGNATURE_REGION_LENGTH__ : $USER_SIGNATURE_LENGTH;}
${RODATA_PM_OFFSET+__RODATA_PM_OFFSET__ = DEFINED(__RODATA_PM_OFFSET__) ? __RODATA_PM_OFFSET__ : $RODATA_PM_OFFSET;}
MEMORY
{
  text   (rx)   : ORIGIN = 0, LENGTH = __TEXT_REGION_LENGTH__
  data   (rw!x) : ORIGIN = __DATA_REGION_ORIGIN__, LENGTH = __DATA_REGION_LENGTH__
${EEPROM_LENGTH+  eeprom (rw!x) : ORIGIN = 0x810000, LENGTH = __EEPROM_REGION_LENGTH__}
  $FUSE_NAME      (rw!x) : ORIGIN = 0x820000, LENGTH = __FUSE_REGION_LENGTH__
  lock      (rw!x) : ORIGIN = 0x830000, LENGTH = __LOCK_REGION_LENGTH__
  signature (rw!x) : ORIGIN = 0x840000, LENGTH = __SIGNATURE_REGION_LENGTH__
${USER_SIGNATURE_LENGTH+  user_signatures (rw!x) : ORIGIN = 0x850000, LENGTH = __USER_SIGNATURE_REGION_LENGTH__}
}
EOF

cat <<EOF
SECTIONS
{
  /* Read-only sections, merged into text segment: */
  ${TEXT_DYNAMIC+${DYNAMIC}}
  .hash        ${RELOCATING-0} : { *(.hash)		}
  .dynsym      ${RELOCATING-0} : { *(.dynsym)		}
  .dynstr      ${RELOCATING-0} : { *(.dynstr)		}
  .gnu.version ${RELOCATING-0} : { *(.gnu.version)	}
  .gnu.version_d ${RELOCATING-0} : { *(.gnu.version_d)	}
  .gnu.version_r ${RELOCATING-0} : { *(.gnu.version_r)	}

  .rel.init    ${RELOCATING-0} : { *(.rel.init)		}
  .rela.init   ${RELOCATING-0} : { *(.rela.init)	}
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
  .rel.fini    ${RELOCATING-0} : { *(.rel.fini)		}
  .rela.fini   ${RELOCATING-0} : { *(.rela.fini)	}
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
  .rel.ctors   ${RELOCATING-0} : { *(.rel.ctors)	}
  .rela.ctors  ${RELOCATING-0} : { *(.rela.ctors)	}
  .rel.dtors   ${RELOCATING-0} : { *(.rel.dtors)	}
  .rela.dtors  ${RELOCATING-0} : { *(.rela.dtors)	}
  .rel.got     ${RELOCATING-0} : { *(.rel.got)		}
  .rela.got    ${RELOCATING-0} : { *(.rela.got)		}
  .rel.bss     ${RELOCATING-0} : { *(.rel.bss)		}
  .rela.bss    ${RELOCATING-0} : { *(.rela.bss)		}
  .rel.plt     ${RELOCATING-0} : { *(.rel.plt)		}
  .rela.plt    ${RELOCATING-0} : { *(.rela.plt)		}

  /* Internal text space or external memory.  */
  .text ${RELOCATING-0} :
  {
    ${RELOCATING+*(.vectors)
    KEEP(*(.vectors))

    /* For data that needs to reside in the lower 64k of progmem.  */
    *(.progmem.gcc*)

    /* PR 13812: Placing the trampolines here gives a better chance
       that they will be in range of the code that uses them.  */
    . = ALIGN(2);
    __trampolines_start = . ;
    /* The jump trampolines for the 16-bit limited relocs will reside here.  */
    *(.trampolines)
    *(.trampolines*)
    __trampolines_end = . ;

    /* avr-libc expects these data to reside in lower 64K. */
    *libprintf_flt.a:*(.progmem.data)
    *libc.a:*(.progmem.data)

    *(.progmem.*)

    . = ALIGN(2);

    /* For code that needs to reside in the lower 128k progmem.  */
    *(.lowtext)
    *(.lowtext*)}

    ${CONSTRUCTING+ __ctors_start = . ; }
    ${CONSTRUCTING+ *(.ctors) }
    ${CONSTRUCTING+ __ctors_end = . ; }
    ${CONSTRUCTING+ __dtors_start = . ; }
    ${CONSTRUCTING+ *(.dtors) }
    ${CONSTRUCTING+ __dtors_end = . ; }
    ${RELOCATING+KEEP(SORT(*)(.ctors))
    KEEP(SORT(*)(.dtors))

    /* From this point on, we do not bother about whether the insns are
       below or above the 16 bits boundary.  */
    *(.init0)  /* Start here after reset.  */
    KEEP (*(.init0))
    *(.init1)
    KEEP (*(.init1))
    *(.init2)  /* Clear __zero_reg__, set up stack pointer.  */
    KEEP (*(.init2))
    *(.init3)
    KEEP (*(.init3))
    *(.init4)  /* Initialize data and BSS.  */
    KEEP (*(.init4))
    *(.init5)
    KEEP (*(.init5))
    *(.init6)  /* C++ constructors.  */
    KEEP (*(.init6))
    *(.init7)
    KEEP (*(.init7))
    *(.init8)
    KEEP (*(.init8))
    *(.init9)  /* Call main().  */
    KEEP (*(.init9))}
    *(.text)
    ${RELOCATING+. = ALIGN(2);
    *(.text.*)
    . = ALIGN(2);
    *(.fini9)  /* _exit() starts here.  */
    KEEP (*(.fini9))
    *(.fini8)
    KEEP (*(.fini8))
    *(.fini7)
    KEEP (*(.fini7))
    *(.fini6)  /* C++ destructors.  */
    KEEP (*(.fini6))
    *(.fini5)
    KEEP (*(.fini5))
    *(.fini4)
    KEEP (*(.fini4))
    *(.fini3)
    KEEP (*(.fini3))
    *(.fini2)
    KEEP (*(.fini2))
    *(.fini1)
    KEEP (*(.fini1))
    *(.fini0)  /* Infinite loop after program termination.  */
    KEEP (*(.fini0))

    /* For code that needs not to reside in the lower progmem.  */
    *(.hightext)
    *(.hightext*)

    *(.progmemx.*)

    . = ALIGN(2);

    /* For tablejump instruction arrays.  We do not relax
       JMP / CALL instructions within these sections.  */
    *(.jumptables)
    *(.jumptables*)

    _etext = . ;}
  } ${RELOCATING+ > text}
EOF

# Devices like ATtiny816 allow to read from flash memory by means of LD*
# instructions provided we add an offset of __RODATA_PM_OFFSET__ to the
# flash addresses.

if test -n "$RODATA_PM_OFFSET"; then
    cat <<EOF
  .rodata ${RELOCATING+ ADDR(.text) + SIZEOF (.text) + __RODATA_PM_OFFSET__ } ${RELOCATING-0} :
  {
    *(.rodata)
    ${RELOCATING+ *(.rodata*)
    *(.gnu.linkonce.r*)}
  } ${RELOCATING+AT> text}
EOF
fi

cat <<EOF
  .data        ${RELOCATING-0} :
  {
    ${RELOCATING+ PROVIDE (__data_start = .) ; }
    *(.data)
    ${RELOCATING+ *(.data*)
    *(.gnu.linkonce.d*)}
EOF

# Classical devices that don't show flash memory in the SRAM address space
# need .rodata to be part of .data because the compiler will use LD*
# instructions and LD* cannot access flash.

if test -z "$RODATA_PM_OFFSET" && test -n "${RELOCATING}"; then
    cat <<EOF
    *(.rodata)  /* We need to include .rodata here if gcc is used */
    *(.rodata*) /* with -fdata-sections.  */
    *(.gnu.linkonce.r*)
EOF
fi

cat <<EOF
    ${RELOCATING+. = ALIGN(2);}
    ${RELOCATING+ _edata = . ; }
    ${RELOCATING+ PROVIDE (__data_end = .) ; }
  } ${RELOCATING+ > data ${RELOCATING+AT> text}}

  .bss ${RELOCATING+ ADDR(.data) + SIZEOF (.data)} ${RELOCATING-0} :${RELOCATING+ AT (ADDR (.bss))}
  {
    ${RELOCATING+ PROVIDE (__bss_start = .) ; }
    *(.bss)
    ${RELOCATING+ *(.bss*)}
    ${RELOCATING+ *(COMMON)}
    ${RELOCATING+ PROVIDE (__bss_end = .) ; }
  } ${RELOCATING+ > data}

  ${RELOCATING+ __data_load_start = LOADADDR(.data); }
  ${RELOCATING+ __data_load_end = __data_load_start + SIZEOF(.data); }

  /* Global data not cleared after reset.  */
  .noinit ${RELOCATING+ ADDR(.bss) + SIZEOF (.bss)} ${RELOCATING-0}: ${RELOCATING+ AT (ADDR (.noinit))}
  {
    ${RELOCATING+ PROVIDE (__noinit_start = .) ; }
    *(.noinit${RELOCATING+ .noinit.* .gnu.linkonce.n.*})
    ${RELOCATING+ PROVIDE (__noinit_end = .) ; }
    ${RELOCATING+ _end = . ;  }
    ${RELOCATING+ PROVIDE (__heap_start = .) ; }
  } ${RELOCATING+ > data}
EOF

if test -n "${EEPROM_LENGTH}"; then
cat <<EOF

  .eeprom ${RELOCATING-0}:
  {
    /* See .data above...  */
    KEEP(*(.eeprom*))
    ${RELOCATING+ __eeprom_end = . ; }
  } ${RELOCATING+ > eeprom}
EOF
fi

if test "$FUSE_NAME" = "fuse" ; then
cat <<EOF

  .fuse ${RELOCATING-0}:
  {
    KEEP(*(.fuse))
    ${RELOCATING+KEEP(*(.lfuse))
    KEEP(*(.hfuse))
    KEEP(*(.efuse))}
  } ${RELOCATING+ > fuse}
EOF
fi

cat <<EOF

  .lock ${RELOCATING-0}:
  {
    KEEP(*(.lock*))
  } ${RELOCATING+ > lock}

  .signature ${RELOCATING-0}:
  {
    KEEP(*(.signature*))
  } ${RELOCATING+ > signature}
EOF

if test "$FUSE_NAME" = "config" ; then
cat <<EOF

  .config ${RELOCATING-0}:
  {
    KEEP(*(.config*))
  } ${RELOCATING+ > config}
EOF
fi

source_sh $srcdir/scripttempl/misc-sections.sc

cat <<EOF
  .note.gnu.build-id ${RELOCATING-0} : { *(.note.gnu.build-id) }
EOF

source_sh $srcdir/scripttempl/DWARF.sc

cat <<EOF
}
EOF
