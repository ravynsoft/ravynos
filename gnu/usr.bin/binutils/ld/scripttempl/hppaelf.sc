# Copyright (C) 2014-2023 Free Software Foundation, Inc.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.

DATA_ADDR=0x40000000
test "$LD_FLAG" = "N" && DATA_ADDR=.

cat <<EOF
/* Copyright (C) 2014-2023 Free Software Foundation, Inc.

   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */

OUTPUT_FORMAT("${OUTPUT_FORMAT}")
OUTPUT_ARCH(${ARCH})
${RELOCATING+ENTRY("\$START\$")}
${RELOCATING+${LIB_SEARCH_DIRS}}
SECTIONS
{
  .text 0x1000 ${RELOCATING++${TEXT_START_ADDR}}:
  {
    ${RELOCATING+__text_start = .;
    CREATE_OBJECT_SYMBOLS}
    *(.PARISC.stubs)
    *(.text)
    ${RELOCATING+etext = .;
    _etext = .;}
  }
  ${RELOCATING+. = ${DATA_ADDR};}
  .data :
  {
    ${RELOCATING+. = . + 0x1000;
    __data_start = .;}
    *(.data)
    ${CONSTRUCTING+CONSTRUCTORS}
    ${RELOCATING+edata = .;
    _edata = .;}
  }
  ${RELOCATING+. = ${DATA_ADDR} + SIZEOF(.data);}
  .bss :
  {
   *(.bss)
   ${RELOCATING+*(COMMON)
   end = .;
   _end = .;}
  }
}
EOF
