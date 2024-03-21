# Linker script for Alpha systems.
# Ian Lance Taylor <ian@cygnus.com>.
# These variables may be overridden by the emulation file.  The
# defaults are appropriate for an Alpha running OSF/1.
#
# Copyright (C) 2014-2023 Free Software Foundation, Inc.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.

test -z "$ENTRY" && ENTRY=__start
test -z "$TEXT_START_ADDR" && TEXT_START_ADDR="0x120000000 + SIZEOF_HEADERS"
if test "x$LD_FLAG" = "xn" -o "x$LD_FLAG" = "xN"; then
  DATA_ADDR=.
else
  test -z "$DATA_ADDR" && DATA_ADDR=0x140000000
fi
cat <<EOF
/* Copyright (C) 2014-2023 Free Software Foundation, Inc.

   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */

OUTPUT_FORMAT("${OUTPUT_FORMAT}")
${LIB_SEARCH_DIRS}

${RELOCATING+ENTRY (${ENTRY})}

SECTIONS
{
  ${RELOCATING+. = ${TEXT_START_ADDR};}
  .text : {
    ${RELOCATING+ _ftext = .;}
    ${RELOCATING+ __istart = .;}
    ${RELOCATING+ KEEP (*(SORT_NONE(.init)))}
    ${RELOCATING+ LONG (0x6bfa8001)}
    ${RELOCATING+ eprol = .;}
    *(.text)
    ${RELOCATING+ __fstart = .;}
    ${RELOCATING+ KEEP (*(SORT_NONE(.fini)))}
    ${RELOCATING+ LONG (0x6bfa8001)}
    ${RELOCATING+ _etext = .;}
  }
  .rdata : {
    *(.rdata)
  }
  .rconst : {
    *(.rconst)
  }
  .pdata : {
    ${RELOCATING+ _fpdata = .;}
    *(.pdata)
  }
  ${RELOCATING+. = ${DATA_ADDR};}
  .data : {
    ${RELOCATING+ _fdata = .;}
    *(.data)
    ${CONSTRUCTING+CONSTRUCTORS}
  }
  .xdata : {
    *(.xdata)
  }
  ${RELOCATING+ _gp = ALIGN (16) + 0x8000;}
  .lit8 : {
    *(.lit8)
  }
  .lita : {
    *(.lita)
  }
  .sdata : {
    *(.sdata)
  }
  ${RELOCATING+ _EDATA  =  .;}
  ${RELOCATING+ _FBSS = .;}
  .sbss : {
    *(.sbss)
    ${RELOCATING+*(.scommon)}
  }
  .bss : {
    *(.bss)
    ${RELOCATING+*(COMMON)}
  }
  ${RELOCATING+ _end = .;}
}
EOF
