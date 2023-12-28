# If you change this file, please also look at files which source this one:
# elf64ltsmip.sh

source_sh ${srcdir}/emulparams/elf64bmip-defs.sh
OUTPUT_FORMAT="elf64-tradbigmips"
BIG_OUTPUT_FORMAT="elf64-tradbigmips"
LITTLE_OUTPUT_FORMAT="elf64-tradlittlemips"

# Magic sections.
OTHER_TEXT_SECTIONS='*(.mips16.fn.*) *(.mips16.call.*)'
OTHER_SECTIONS="
  .gptab.sdata : {${RELOCATING+ *(.gptab.data)} *(.gptab.sdata) }
  .gptab.sbss : {${RELOCATING+ *(.gptab.bss)} *(.gptab.sbss) }
"

TEXT_START_ADDR="0x120000000"
