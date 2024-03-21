source_sh ${srcdir}/emulparams/elf_i386.sh
source_sh ${srcdir}/emulparams/elf_haiku.sh
TEXT_START_ADDR=0x200000
NONPAGED_TEXT_START_ADDR=0x200000
MAXPAGESIZE=0x1000
NOP=0x90909090
OUTPUT_FORMAT="elf32-i386"
