MACHINE=
SCRIPT_NAME=elf
OUTPUT_FORMAT="elf32-littlearm"
BIG_OUTPUT_FORMAT="elf32-bigarm"
LITTLE_OUTPUT_FORMAT="elf32-littlearm"
TEXT_START_ADDR=0x00100000
TEMPLATE_NAME=elf
EXTRA_EM_FILE=armelf
OTHER_TEXT_SECTIONS='*(.glue_7t) *(.glue_7) *(.vfp11_veneer) *(.v4_bx)'
OTHER_BSS_SYMBOLS="${CREATE_SHLIB+PROVIDE (}__bss_start__ = .${CREATE_SHLIB+)};"
OTHER_BSS_END_SYMBOLS="${CREATE_SHLIB+PROVIDE (}_bss_end__ = .${CREATE_SHLIB+)}; ${CREATE_SHLIB+PROVIDE (}__bss_end__ = .${CREATE_SHLIB+)};"
OTHER_END_SYMBOLS="${CREATE_SHLIB+PROVIDE (}__end__ = .${CREATE_SHLIB+)};"

DATA_START_SYMBOLS="${CREATE_SHLIB+PROVIDE (}__data_start = .${CREATE_SHLIB+)};"

GENERATE_SHLIB_SCRIPT=yes

ARCH=arm
MACHINE=
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"

ENTRY=_start

# This sets the stack to the top of the simulator memory (2^19 bytes).
STACK_ADDR=0x80000

# ARM does not support .s* sections.
NO_SMALL_DATA=yes
