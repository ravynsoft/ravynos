source_sh ${srcdir}/emulparams/elf32bmipn32-defs.sh

OUTPUT_FORMAT="elf32-nlittlemips"
BIG_OUTPUT_FORMAT="elf32-nbigmips"
LITTLE_OUTPUT_FORMAT="elf32-nlittlemips"

TEXT_START_ADDR=0x0100000
ARCH=mips:5900
MACHINE=
MAXPAGESIZE=128
EMBEDDED=yes
DYNAMIC_LINK=false

OTHER_TEXT_SECTIONS='*(.mips16.fn.*) *(.mips16.call.*)'
OTHER_SECTIONS="
  .gptab.sdata : {${RELOCATING+ *(.gptab.data)} *(.gptab.sdata) }
  .gptab.sbss : {${RELOCATING+ *(.gptab.bss)} *(.gptab.sbss) }
"

unset DATA_ADDR
SHLIB_TEXT_START_ADDR=0
unset GENERATE_SHLIB_SCRIPT

