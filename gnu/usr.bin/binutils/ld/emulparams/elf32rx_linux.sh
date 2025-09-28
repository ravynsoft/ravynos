MACHINE=
SCRIPT_NAME=elf
OUTPUT_FORMAT="elf32-rx-linux"
# See also `include/elf/rx.h'
TEXT_START_ADDR=0x10000000
ARCH=rx
ENTRY=start
EMBEDDED=yes
TEMPLATE_NAME=elf
EXTRA_EM_FILE=rxlinux
ELFSIZE=32
MAXPAGESIZE=256

STACK_ADDR="(DEFINED(__stack) ? __stack : 0xbffffffc)"
STACK_SENTINEL="LONG(0xdeaddead)"
# We do not need .stack for shared library.
test -n "$CREATE_SHLIB" && unset STACK_ADDR
