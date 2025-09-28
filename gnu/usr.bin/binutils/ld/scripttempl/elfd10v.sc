# Copyright (C) 2014-2023 Free Software Foundation, Inc.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.

test -z "$ENTRY" && ENTRY=_start
test -z "${BIG_OUTPUT_FORMAT}" && BIG_OUTPUT_FORMAT=${OUTPUT_FORMAT}
test -z "${LITTLE_OUTPUT_FORMAT}" && LITTLE_OUTPUT_FORMAT=${OUTPUT_FORMAT}
if [ -z "$MACHINE" ]; then OUTPUT_ARCH=${ARCH}; else OUTPUT_ARCH=${ARCH}:${MACHINE}; fi
test "$LD_FLAG" = "N" && DATA_ADDR=.
INTERP=".interp   ${RELOCATING-0} : { *(.interp) }"
PLT=".plt    ${RELOCATING-0} : { *(.plt) }"


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
    KEEP (*crtbegin?.o(.ctors))

    /* We don't want to include the .ctor section from
       the crtend.o file until after the sorted ctors.
       The .ctor section from the crtend file contains the
       end of ctors marker and it must be last */

    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    ${CONSTRUCTING+${CTOR_END}}
  }"

DTOR=" .dtors       ${CONSTRUCTING-0} :
  {
    ${CONSTRUCTING+${DTOR_START}}
    KEEP (*crtbegin.o(.dtors))
    KEEP (*crtbegin?.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    ${CONSTRUCTING+${DTOR_END}}
  }"

STACK=" .stack : { _stack = .; *(.stack) } >STACK "

# if this is for an embedded system, don't add SIZEOF_HEADERS.
if [ -z "$EMBEDDED" ]; then
   test -z "${READONLY_BASE_ADDRESS}" && READONLY_BASE_ADDRESS="${READONLY_START_ADDR} + SIZEOF_HEADERS"
else
   test -z "${READONLY_BASE_ADDRESS}" && READONLY_BASE_ADDRESS="${READONLY_START_ADDR}"
fi

cat <<EOF
/* Copyright (C) 2014-2023 Free Software Foundation, Inc.

   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */

OUTPUT_FORMAT("${OUTPUT_FORMAT}", "${BIG_OUTPUT_FORMAT}",
	      "${LITTLE_OUTPUT_FORMAT}")
OUTPUT_ARCH(${OUTPUT_ARCH})
EOF

test -n "${RELOCATING}" && cat <<EOF
ENTRY(${ENTRY})

${LIB_SEARCH_DIRS}
/* Do we need any of these for elf?
   __DYNAMIC = 0; ${STACKZERO+${STACKZERO}} ${SHLIB_PATH+${SHLIB_PATH}}  */
${EXECUTABLE_SYMBOLS}

MEMORY
{
  /* These are the values for the D10V-TS3 board.
     There are other memory regions available on
     the TS3 (eg ROM, FLASH, etc) but these are not
     used by this script.  */

  INSN       : org = 0x01000000, len = 256K
  DATA       : org = 0x02000000, len = 48K

  /* This is a fake memory region at the top of the
     on-chip RAM, used as the start of the
     (descending) stack.  */

  STACK      : org = 0x0200BFFC, len = 4
}

EOF

cat <<EOF
SECTIONS
{
  .text ${RELOCATING+${TEXT_START_ADDR}} :
  {
    ${RELOCATING+${TEXT_START_SYMBOLS}
    KEEP (*(SORT_NONE(.init)))
    KEEP (*(SORT_NONE(.init.*)))
    KEEP (*(SORT_NONE(.fini)))
    KEEP (*(SORT_NONE(.fini.*)))}
    *(.text)
    ${RELOCATING+*(.text.*)}
    /* .gnu.warning sections are handled specially by elf.em.  */
    *(.gnu.warning)
    ${RELOCATING+*(.gnu.linkonce.t*)
    _etext = .;
    PROVIDE (etext = .);}
  } ${RELOCATING+ >INSN} =${NOP-0}

  .rodata ${RELOCATING+${READONLY_START_ADDR}} : {
    *(.rodata)
    ${RELOCATING+*(.gnu.linkonce.r*)
    *(.rodata.*)}
  } ${RELOCATING+ >DATA}

  .rodata1 ${RELOCATING-0} : {
    *(.rodata1)
    ${RELOCATING+*(.rodata1.*)}
   } ${RELOCATING+ >DATA}

  .data  ${RELOCATING-0} :
  {
    ${RELOCATING+${DATA_START_SYMBOLS}}
    *(.data)
    ${RELOCATING+*(.data.*)
    *(.gnu.linkonce.d*)}
    ${CONSTRUCTING+CONSTRUCTORS}
  } ${RELOCATING+ >DATA}

  .data1 ${RELOCATING-0} : {
    *(.data1)
    ${RELOCATING+*(.data1.*)}
  } ${RELOCATING+ >DATA}

  ${RELOCATING+${CTOR} >DATA}
  ${RELOCATING+${DTOR} >DATA}

  /* We want the small data sections together, so single-instruction offsets
     can access them all, and initialized data all before uninitialized, so
     we can shorten the on-disk segment size.  */
  .sdata   ${RELOCATING-0} : {
    *(.sdata)
    ${RELOCATING+*(.sdata.*)}
  } ${RELOCATING+ >DATA}

  ${RELOCATING+_edata = .;}
  ${RELOCATING+PROVIDE (edata = .);}
  ${RELOCATING+__bss_start = .;}
  .sbss    ${RELOCATING-0} : { *(.sbss)${RELOCATING+ *(.scommon)} } ${RELOCATING+ >DATA}
  .bss     ${RELOCATING-0} :
  {
   ${RELOCATING+*(.dynbss)
   *(.dynbss.*)}
   *(.bss)
   ${RELOCATING+*(.bss.*)
   *(COMMON)}
  } ${RELOCATING+ >DATA}

  ${RELOCATING+_end = . ;}
  ${RELOCATING+PROVIDE (end = .);}

  ${RELOCATING+$STACK}

EOF

source_sh $srcdir/scripttempl/misc-sections.sc
source_sh $srcdir/scripttempl/DWARF.sc

cat <<EOF
}
EOF
