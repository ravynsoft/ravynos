source_sh ${srcdir}/emulparams/elf32ppccommon.sh
source_sh ${srcdir}/emulparams/plt_unwind.sh
EXTRA_EM_FILE=ppc32elf
OUTPUT_FORMAT="elf32-powerpc-vxworks"
source_sh ${srcdir}/emulparams/vxworks.sh
