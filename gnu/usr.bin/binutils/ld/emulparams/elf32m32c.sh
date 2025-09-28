MACHINE=
SCRIPT_NAME=elf
OUTPUT_FORMAT="elf32-m32c"
# See also `include/elf/m32c.h'
TEXT_START_ADDR=0x2000
ARCH=m32c
ENTRY=_start
EMBEDDED=yes
TEMPLATE_NAME=elf
EXTRA_EM_FILE=needrelax
ELFSIZE=32
MAXPAGESIZE=256

STACK_ADDR="(DEFINED(__stack) ? __stack : 0x7fc)"
STACK_SENTINEL="LONG(0xdeaddead)"
# We do not need .stack for shared library.
test -n "$CREATE_SHLIB" && unset STACK_ADDR
