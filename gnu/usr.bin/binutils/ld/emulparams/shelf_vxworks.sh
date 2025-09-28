# If you change this file, please also look at files which source this one:
# shlelf_vxworks.sh

SCRIPT_NAME=elf
NO_REL_RELOCS=yes
BIG_OUTPUT_FORMAT="elf32-sh-vxworks"
LITTLE_OUTPUT_FORMAT="elf32-shl-vxworks"
OUTPUT_FORMAT="$BIG_OUTPUT_FORMAT"
TEXT_START_ADDR=0x1000
MAXPAGESIZE='CONSTANT (MAXPAGESIZE)'
ARCH=sh
MACHINE=
TEMPLATE_NAME=elf
GENERATE_SHLIB_SCRIPT=yes
ENTRY=__start
SYMPREFIX=_

GOT=".got          ${RELOCATING-0} : {
  ${RELOCATING+PROVIDE(__GLOBAL_OFFSET_TABLE_ = .);
  *(.got.plt) }*(.got) }"
source_sh ${srcdir}/emulparams/vxworks.sh
