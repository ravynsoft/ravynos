# If you change this file, please also look at files which source this one:
# elf32microblazeel.sh
SCRIPT_NAME=elfmicroblaze
BIG_OUTPUT_FORMAT="elf32-microblaze"
LITTLE_OUTPUT_FORMAT="elf32-microblazeel"
OUTPUT_FORMAT=$BIG_OUTPUT_FORMAT
#TEXT_START_ADDR=0
NONPAGED_TEXT_START_ADDR=0x28
ALIGNMENT=4
MAXPAGESIZE=4
ARCH=microblaze
EMBEDDED=yes

NOP=0x80000000

# Hmmm, there's got to be a better way.  This sets the stack to the
# top of the simulator memory (2^19 bytes).
#DATA_ADDR=0x10000
#OTHER_RELOCATING_SECTIONS='.stack 0x7000 : { _stack = .; *(.stack) }'
#$@{RELOCATING+ PROVIDE (__stack = 0x7000);@}
#OTHER_RELOCATING_SECTIONS='PROVIDE (_stack = _end + 0x1000);'

TEMPLATE_NAME=elf
#GENERATE_SHLIB_SCRIPT=yes


