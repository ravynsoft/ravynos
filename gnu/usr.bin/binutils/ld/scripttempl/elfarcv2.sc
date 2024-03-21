#
# Unusual variables checked by this code:
#	NOP - four byte opcode for no-op (defaults to 0)
#	NO_SMALL_DATA - no .sbss/.sbss2/.sdata/.sdata2 sections if not
#		empty.
#	OTHER_READONLY_SECTIONS - other than .text .init .rodata ...
#		(e.g., .PARISC.milli)
# When adding sections, do note that the names of some sections are used
# when specifying the start address of the next.
#
test -z "$ENTRY" && ENTRY=start
test -z "${BIG_OUTPUT_FORMAT}" && BIG_OUTPUT_FORMAT=${OUTPUT_FORMAT}
test -z "${LITTLE_OUTPUT_FORMAT}" && LITTLE_OUTPUT_FORMAT=${OUTPUT_FORMAT}
# If we request a big endian toolchain, give a big endian linker
test -z "$GOT" && GOT=".got          ${RELOCATING-0} : {${RELOCATING+ *(.got.plt)} *(.got) } ${RELOCATING+ > ${DATA_MEMORY}}"
test "${ARC_ENDIAN}" = "big" && OUTPUT_FORMAT=${BIG_OUTPUT_FORMAT}
if [ -z "$MACHINE" ]; then OUTPUT_ARCH=${ARCH}; else OUTPUT_ARCH=${ARCH}:${MACHINE}; fi
test -z "${ELFSIZE}" && ELFSIZE=32
test -z "${ALIGNMENT}" && ALIGNMENT="${ELFSIZE} / 8"
test "$LD_FLAG" = "N" && DATA_ADDR=.

CTOR=".ctors        ${CONSTRUCTING-0} :
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

    KEEP (*crtbegin*.o(.ctors))

    /* We don't want to include the .ctor section from
       from the crtend.o file until after the sorted ctors.
       The .ctor section from the crtend file contains the
       end of ctors marker and it must be last */

    KEEP (*(EXCLUDE_FILE (*crtend*.o $OTHER_EXCLUDE_FILES) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    ${CONSTRUCTING+${CTOR_END}}
  } ${RELOCATING+ > ${DATA_MEMORY}}"
DTOR=".dtors        ${CONSTRUCTING-0} :
  {
    ${CONSTRUCTING+${DTOR_START}}
    KEEP (*crtbegin*.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend*.o $OTHER_EXCLUDE_FILES) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    ${CONSTRUCTING+${DTOR_END}}
  } ${RELOCATING+ > ${DATA_MEMORY}}"

IVT="
 /* If the 'ivtbase_addr' symbol is defined, it indicates  the base address of
    the interrupt vectors.  See description of INT_VECTOR_BASE register.  */

 .ivt DEFINED (ivtbase_addr) ? ivtbase_addr : 0x00 :
 {
   ${RELOCATING+ PROVIDE (__ivtbase_addr = .); }
   KEEP (*(.ivt));
 } ${RELOCATING+ > ${STARTUP_MEMORY}}"

if test -z "${NO_SMALL_DATA}"; then
  SBSS=".sbss         ${RELOCATING-0} :
  {
    ${RELOCATING+PROVIDE (__sbss_start = .);}
    ${RELOCATING+PROVIDE (___sbss_start = .);}
    *(.dynsbss)
    *(.sbss${RELOCATING+ .sbss.* .gnu.linkonce.sb.*})
    *(.scommon)
    ${RELOCATING+PROVIDE (__sbss_end = .);}
    ${RELOCATING+PROVIDE (___sbss_end = .);}
  } ${RELOCATING+ > ${SDATA_MEMORY}}"
  SBSS2=".sbss2        ${RELOCATING-0} : { *(.sbss2${RELOCATING+ .sbss2.* .gnu.linkonce.sb2.*}) } ${RELOCATING+ > ${SDATA_MEMORY}}"
  SDATA="/* We want the small data sections together, so single-instruction offsets
     can access them all, and initialized data all before uninitialized, so
     we can shorten the on-disk segment size.  */
  .sdata        ${RELOCATING-0} :
  {
    ${RELOCATING+${SDATA_START_SYMBOLS}}
    *(.sdata${RELOCATING+ .sdata.* .gnu.linkonce.s.*})

    ${RELOCATING+_edata  =  .;}
    ${RELOCATING+PROVIDE (edata = .);}
  } ${RELOCATING+ > ${SDATA_MEMORY}}"
  SDATA2=".sdata2       ${RELOCATING-0} : { *(.sdata2${RELOCATING+ .sdata2.* .gnu.linkonce.s2.*}) } ${RELOCATING+ > ${SDATA_MEMORY}}"
  REL_SDATA=".rel.sdata    ${RELOCATING-0} : { *(.rel.sdata${RELOCATING+ .rel.sdata.* .rel.gnu.linkonce.s.*}) }
  .rela.sdata   ${RELOCATING-0} : { *(.rela.sdata${RELOCATING+ .rela.sdata.* .rela.gnu.linkonce.s.*}) }"
  REL_SBSS=".rel.sbss     ${RELOCATING-0} : { *(.rel.sbss${RELOCATING+ .rel.sbss.* .rel.gnu.linkonce.sb.*}) }
  .rela.sbss    ${RELOCATING-0} : { *(.rela.sbss${RELOCATING+ .rela.sbss.* .rela.gnu.linkonce.sb.*}) }"
  REL_SDATA2=".rel.sdata2   ${RELOCATING-0} : { *(.rel.sdata2${RELOCATING+ .rel.sdata2.* .rel.gnu.linkonce.s2.*}) }
  .rela.sdata2  ${RELOCATING-0} : { *(.rela.sdata2${RELOCATING+ .rela.sdata2.* .rela.gnu.linkonce.s2.*}) }"
  REL_SBSS2=".rel.sbss2    ${RELOCATING-0} : { *(.rel.sbss2${RELOCATING+ .rel.sbss2.* .rel.gnu.linkonce.sb2.*}) }
  .rela.sbss2   ${RELOCATING-0} : { *(.rela.sbss2${RELOCATING+ .rela.sbss2.* .rela.gnu.linkonce.sb2.*}) }"
fi

#
# We provide two emulations: a fixed on that defines some memory banks
# and a configurable one that includes a user provided memory definition.
#
case $GENERIC_BOARD in
  yes|1|YES)
	MEMORY_DEF="
/* Get memory banks definition from some user configuration file.
   This file must be located in some linker directory (search path
   with -L<dir>). See fixed memory banks emulation script.  */
INCLUDE memory.x;
"
	;;
  *)
MEMORY_DEF="
/* Fixed definition of the available memory banks.
   See generic emulation script for a user defined configuration.  */
MEMORY
{
    ICCM : ORIGIN = 0x00000000, LENGTH = ${ICCM_SIZE}
    DCCM : ORIGIN = ${RAM_START_ADDR}, LENGTH = ${RAM_SIZE}
}
"
	;;
esac

cat <<EOF
OUTPUT_FORMAT("${OUTPUT_FORMAT}", "${BIG_OUTPUT_FORMAT}", "${LITTLE_OUTPUT_FORMAT}")
OUTPUT_ARCH(${OUTPUT_ARCH})
${RELOCATING+ENTRY(${ENTRY})}

${RELOCATING+${LIB_SEARCH_DIRS}}
${RELOCATING+${EXECUTABLE_SYMBOLS}}
${RELOCATING+${MEMORY_DEF}}

SECTIONS
{
  ${RELOCATING+${IVT}}

  /* Read-only sections, merged into text segment: */
  ${TEXT_DYNAMIC+${DYNAMIC}}
  .hash        ${RELOCATING-0} : { *(.hash)		}
  .dynsym      ${RELOCATING-0} : { *(.dynsym)		}
  .dynstr      ${RELOCATING-0} : { *(.dynstr)		}
  .gnu.version ${RELOCATING-0} : { *(.gnu.version)	}
  .gnu.version_d ${RELOCATING-0} : { *(.gnu.version_d)	}
  .gnu.version_r ${RELOCATING-0} : { *(.gnu.version_r)	}

  .rel.init     ${RELOCATING-0} : { *(.rel.init) }
  .rela.init    ${RELOCATING-0} : { *(.rela.init) }
  .rel.text     ${RELOCATING-0} : { *(.rel.text${RELOCATING+ .rel.text.* .rel.gnu.linkonce.t.*}) }
  .rela.text    ${RELOCATING-0} : { *(.rela.text${RELOCATING+ .rela.text.* .rela.gnu.linkonce.t.*}) }
  .rel.fini     ${RELOCATING-0} : { *(.rel.fini) }
  .rela.fini    ${RELOCATING-0} : { *(.rela.fini) }
  .rel.rodata   ${RELOCATING-0} : { *(.rel.rodata${RELOCATING+ .rel.rodata.* .rel.gnu.linkonce.r.*}) }
  .rela.rodata  ${RELOCATING-0} : { *(.rela.rodata${RELOCATING+ .rela.rodata.* .rela.gnu.linkonce.r.*}) }
  .rel.data     ${RELOCATING-0} : { *(.rel.data${RELOCATING+ .rel.data.* .rel.gnu.linkonce.d.*}) }
  .rela.data    ${RELOCATING-0} : { *(.rela.data${RELOCATING+ .rela.data.* .rela.gnu.linkonce.d.*}) }
  .rel.tdata	${RELOCATING-0} : { *(.rel.tdata${RELOCATING+ .rel.tdata.* .rel.gnu.linkonce.td.*}) }
  .rela.tdata	${RELOCATING-0} : { *(.rela.tdata${RELOCATING+ .rela.tdata.* .rela.gnu.linkonce.td.*}) }
  .rel.tbss	${RELOCATING-0} : { *(.rel.tbss${RELOCATING+ .rel.tbss.* .rel.gnu.linkonce.tb.*}) }
  .rela.tbss	${RELOCATING-0} : { *(.rela.tbss${RELOCATING+ .rela.tbss.* .rela.gnu.linkonce.tb.*}) }
  .rel.ctors    ${RELOCATING-0} : { *(.rel.ctors) }
  .rela.ctors   ${RELOCATING-0} : { *(.rela.ctors) }
  .rel.dtors    ${RELOCATING-0} : { *(.rel.dtors) }
  .rela.dtors   ${RELOCATING-0} : { *(.rela.dtors) }
  .rel.got      ${RELOCATING-0} : { *(.rel.got) }
  .rela.got     ${RELOCATING-0} : { *(.rela.got) }
  ${REL_SDATA}
  ${REL_SBSS}
  ${REL_SDATA2}
  ${REL_SBSS2}
  .rel.bss      ${RELOCATING-0} : { *(.rel.bss${RELOCATING+ .rel.bss.* .rel.gnu.linkonce.b.*}) }
  .rela.bss     ${RELOCATING-0} : { *(.rela.bss${RELOCATING+ .rela.bss.* .rela.gnu.linkonce.b.*}) }

  .text         ${RELOCATING-0} :
  {
    ${RELOCATING+${TEXT_START_SYMBOLS}}

    ${RELOCATING+ . = ALIGN(4);}
    ${RELOCATING+${INIT_START}}
    KEEP (*(SORT_NONE(.init)))
    ${RELOCATING+${INIT_END}}

    /* Start here after reset.  */
    ${RELOCATING+ . = ALIGN(4);}
    KEEP (*crt0.o(.text.__startup))

    /* Remaining code.  */
    ${RELOCATING+ . = ALIGN(4);}
    *(.text .stub${RELOCATING+ .text.* .gnu.linkonce.t.*})
    /* .gnu.warning sections are handled specially by elf.em.  */
    *(.gnu.warning)

    ${RELOCATING+${OTHER_TEXT_SECTIONS}}

  } ${RELOCATING+ > ${TEXT_MEMORY}} =${NOP-0}

  .fini         ${RELOCATING-0} :
  {
    ${RELOCATING+${FINI_START}}
    KEEP (*(SORT_NONE(.fini)))
    ${RELOCATING+${FINI_END}}

    ${RELOCATING+PROVIDE (__etext = .);}
    ${RELOCATING+PROVIDE (_etext = .);}
    ${RELOCATING+PROVIDE (etext = .);}
  } ${RELOCATING+ > ${TEXT_MEMORY}} =${NOP-0}

  .jcr ${RELOCATING-0} :
  {
    KEEP (*(.jcr))
  } ${RELOCATING+> ${TEXT_MEMORY}}

  .eh_frame ${RELOCATING-0} :
  {
    KEEP (*(.eh_frame))
  } ${RELOCATING+> ${TEXT_MEMORY}}

  .gcc_except_table ${RELOCATING-0} :
  {
    *(.gcc_except_table) *(.gcc_except_table.*)
  } ${RELOCATING+> ${TEXT_MEMORY}}

  .plt ${RELOCATING-0} :
  {
    *(.plt)
  } ${RELOCATING+> ${TEXT_MEMORY}}

  .jlitab ${RELOCATING-0} :
  {
    ${RELOCATING+${JLI_START_TABLE}}
     jlitab*.o:(.jlitab*) *(.jlitab*)
  } ${RELOCATING+> ${TEXT_MEMORY}}

  .rodata ${RELOCATING-0} :
  {
    *(.rodata) ${RELOCATING+*(.rodata.*)} ${RELOCATING+*(.gnu.linkonce.r.*)}
  } ${RELOCATING+> ${TEXT_MEMORY}}

  .rodata1      ${RELOCATING-0} : { *(.rodata1) } ${RELOCATING+> ${TEXT_MEMORY}}

  ${RELOCATING+${OTHER_READONLY_SECTIONS}}

  /* Start of the data section image in ROM.  */
  ${RELOCATING+__data_image = .;}
  ${RELOCATING+PROVIDE (__data_image = .);}

  .data	${RELOCATING-0} :
  {
    ${RELOCATING+ PROVIDE (__data_start = .) ; }
    /* --gc-sections will delete empty .data. This leads to wrong start
       addresses for subsequent sections because -Tdata= from the command
       line will have no effect, see PR13697.  Thus, keep .data  */
    KEEP (*(.data))
    ${RELOCATING+${DATA_START_SYMBOLS}}
    ${RELOCATING+*(.data.* .gnu.linkonce.d.*)}
    ${CONSTRUCTING+SORT(CONSTRUCTORS)}

  } ${RELOCATING+ > ${DATA_MEMORY}}

  ${GOT}
  ${RELOCATING+${CTOR}}
  ${RELOCATING+${DTOR}}

  ${RELOCATING+${SDATA}}
  ${RELOCATING+${SDATA2}}
  ${RELOCATING+${SBSS}}
  ${RELOCATING+${SBSS2}}
  .bss          ${RELOCATING-0} :
  {
    ${RELOCATING+*(.dynbss)}
    *(.bss${RELOCATING+ .bss.* .gnu.linkonce.b.*})
    ${RELOCATING+*(COMMON)
    /* Align here to ensure that the .bss section occupies space up to
       _end.  Align after .bss to ensure correct alignment even if the
       .bss section disappears because there are no input sections.  */
    . = ALIGN(${ALIGNMENT});}
   ${RELOCATING+_end = .;}
   ${RELOCATING+PROVIDE (end = .);}
  } ${RELOCATING+ > ${DATA_MEMORY}}

  /* Global data not cleared after reset.  */
  .noinit ${RELOCATING-0}:
  {
    *(.noinit${RELOCATING+ .noinit.* .gnu.linkonce.n.*})
    ${RELOCATING+. = ALIGN(${ALIGNMENT});}
    ${RELOCATING+ PROVIDE (__start_heap = .) ; }
  } ${RELOCATING+ > ${DATA_MEMORY}}

  ${RELOCATING+ PROVIDE (__stack_top = (ORIGIN (${DATA_MEMORY}) + LENGTH (${DATA_MEMORY}) - 1) & -4);}
  ${RELOCATING+ PROVIDE (__end_heap = ORIGIN (${DATA_MEMORY}) + LENGTH (${DATA_MEMORY}) - 1);}

  .note.gnu.build-id : { *(.note.gnu.build-id) }
EOF

source_sh $srcdir/scripttempl/misc-sections.sc
source_sh $srcdir/scripttempl/DWARF.sc

cat <<EOF
  /* ARC Extension Sections */
  .arcextmap	  0 : { *(.arcextmap.*) }
}
EOF
