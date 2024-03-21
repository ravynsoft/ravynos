source_sh ${srcdir}/emulparams/armelf.sh
source_sh ${srcdir}/emulparams/elf_fbsd.sh

TEXT_START_ADDR=0x00010000

TARGET2_TYPE=got-rel
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"
GENERATE_PIE_SCRIPT=yes

unset STACK_ADDR
unset EMBEDDED
