SCRIPT_NAME=elf
OUTPUT_FORMAT="elf32-csky-little"
BIG_OUTPUT_FORMAT="elf32-csky-big"
LITTLE_OUTPUT_FORMAT="elf32-csky-little"
NO_REL_RELOCS=yes
TARGET_PAGE_SIZE=0x400
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"
TEXT_START_ADDR=0x60000000
DATA_ADDR=0x20000000

# The default address spaces
# RAM:        0x0 - 0x1FFFFFFF
# RAM: 0x20000000 - 0x3FFFFFFF
# RAM: 0x60000000 - 0x7FFFFFFF
#
# There are some symbols used in the crt.S
# __stack: The stack top
# __heap_start: The heap start
# __heap_end: The heap end
# __csky_exit: Be used by qemu to check exit

OTHER_SYMBOLS='
  PROVIDE (__stack =  0x01000000 - 0x8);
  PROVIDE (__heap_start = 0x00000100);
  PROVIDE (__heap_end = 0x00900000);
  PROVIDE (__csky_exit = 0x10002000);
'
CHECK_RELOCS_AFTER_OPEN_INPUT=yes
NONPAGED_TEXT_START_ADDR=0
ATTRS_SECTIONS='.csky.attributes 0 : { KEEP (*(.csky.attributes)) KEEP (*(.csky.attributes)) }'
ARCH=csky
EMBEDDED=yes
EXTRA_EM_FILE=cskyelf

# There is a problem with the NOP value - it must work for both
# big endian and little endian systems.  Unfortunately there is
# no symmetrical mcore opcode that functions as a noop.  The
# chosen solution is to use "tst r0, r14".  This is a symmetrical
# value, and apart from the corruption of the C bit, it has no other
# side effects.  Since the carry bit is never tested without being
# explicitly set first, and since the NOP code is only used as a
# fill value between independently viable pieces of code, it should
# not matter.
NOP=0

ENTRY=__start
OTHER_BSS_SYMBOLS="__sbss__ = . ;"
OTHER_BSS_END_SYMBOLS="__ebss__ = . ;"

# This sets the stack to the top of the simulator memory (2^19 bytes).
# STACK_ADDR=0x80000

TEMPLATE_NAME=elf
