source_sh ${srcdir}/emulparams/elf64_ia64.sh
TEXT_START_ADDR="0x2000000000000000"
unset DATA_ADDR
unset SMALL_DATA_CTOR
unset SMALL_DATA_DTOR
source_sh ${srcdir}/emulparams/elf_fbsd.sh
