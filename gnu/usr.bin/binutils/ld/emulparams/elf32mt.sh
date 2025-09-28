MACHINE=
SCRIPT_NAME=elf
TEMPLATE_NAME=generic
EXTRA_EM_FILE=genelf
OUTPUT_FORMAT="elf32-mt"
# See also `include/elf/mt.h'
TEXT_START_ADDR=0x2000
ARCH=mt
ENTRY=_start
EMBEDDED=yes
ELFSIZE=32
MAXPAGESIZE=256

STACK_ADDR="(DEFINED(__stack) ? __stack : 0x007FFFF0)"
STACK_SENTINEL="LONG(0xdeaddead)"
# We do not need .stack for shared library.
test -n "$CREATE_SHLIB" && unset STACK_ADDR
