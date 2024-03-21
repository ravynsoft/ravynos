# If you change this file, please also look at files which source this one:
# shelf_linux.sh shelf_fd.sh shlelf_fd.sh

SCRIPT_NAME=elf
OUTPUT_FORMAT="elf32-sh-linux"
NO_REL_RELOCS=yes
TEXT_START_ADDR=0x400000
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"
COMMONPAGESIZE="CONSTANT (COMMONPAGESIZE)"
ARCH=sh
MACHINE=
TEMPLATE_NAME=elf
GENERATE_SHLIB_SCRIPT=yes
GENERATE_PIE_SCRIPT=yes

DATA_START_SYMBOLS='PROVIDE (__data_start = .);';

OTHER_READWRITE_SECTIONS="
  .note.ABI-tag ${RELOCATING-0} : { *(.note.ABI-tag) }"
