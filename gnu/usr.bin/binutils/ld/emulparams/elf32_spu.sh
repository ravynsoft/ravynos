SCRIPT_NAME=elf
TEMPLATE_NAME=elf
EXTRA_EM_FILE=spuelf
OUTPUT_FORMAT="elf32-spu"
NO_REL_RELOCS=yes
ARCH=spu
MACHINE=
ALIGNMENT=16
TEXT_START_ADDR=0
INITIAL_READONLY_SECTIONS='.interrupt : { KEEP(*(.interrupt)) }'
if test -z "${CREATE_SHLIB}"; then
  INITIAL_READONLY_SECTIONS="${INITIAL_READONLY_SECTIONS}
  .interp       ${RELOCATING-0} : { *(.interp) }"
fi
OTHER_END_SYMBOLS='PROVIDE (__stack = 0x3fff0);'
NO_SMALL_DATA=true
EMBEDDED=true
MAXPAGESIZE=0x80
DATA_ADDR="ALIGN(${MAXPAGESIZE})"
OTHER_BSS_SECTIONS=".toe ALIGN(128) : { *(.toe) } = 0"
OTHER_SECTIONS=".note.spu_name 0 : { KEEP(*(.note.spu_name)) }
  ._ea 0 : { KEEP(*(._ea))${RELOCATING+ KEEP(*(._ea.*))} }"
OTHER_READONLY_SECTIONS="
  .fixup ${RELOCATING-0} : {
    ${RELOCATING+PROVIDE (__fixup_start = .);}
    KEEP(*(.fixup))
  }"
