MACHINE=
SCRIPT_NAME=elf
OUTPUT_FORMAT="elf32-littlearm"
BIG_OUTPUT_FORMAT="elf32-bigarm"
LITTLE_OUTPUT_FORMAT="elf32-littlearm"
TEXT_START_ADDR=0x8000
TEMPLATE_NAME=elf
EXTRA_EM_FILE=armelf
OTHER_PLT_SECTIONS="
  .gnu.sgstubs    : { *(.gnu.sgstubs*) }"
OTHER_TEXT_SECTIONS='*(.glue_7t) *(.glue_7) *(.vfp11_veneer) *(.v4_bx)'
OTHER_BSS_SYMBOLS="${CREATE_SHLIB+PROVIDE (}__bss_start__ = .${CREATE_SHLIB+)};"
OTHER_BSS_END_SYMBOLS="${CREATE_SHLIB+PROVIDE (}_bss_end__ = .${CREATE_SHLIB+)}; ${CREATE_SHLIB+PROVIDE (}__bss_end__ = .${CREATE_SHLIB+)};"
OTHER_END_SYMBOLS="${CREATE_SHLIB+PROVIDE (}__end__ = .${CREATE_SHLIB+)};"
OTHER_SECTIONS='.note.gnu.arm.ident 0 : { KEEP (*(.note.gnu.arm.ident)) }'
ATTRS_SECTIONS='.ARM.attributes 0 : { KEEP (*(.ARM.attributes)) KEEP (*(.gnu.attributes)) }'
OTHER_READONLY_SECTIONS="
  .ARM.extab ${RELOCATING-0} : { *(.ARM.extab${RELOCATING+* .gnu.linkonce.armextab.*}) }
  .ARM.exidx ${RELOCATING-0} :
    {
      ${RELOCATING+PROVIDE_HIDDEN (__exidx_start = .);}
      *(.ARM.exidx${RELOCATING+* .gnu.linkonce.armexidx.*})
      ${RELOCATING+PROVIDE_HIDDEN (__exidx_end = .);}
    }"

DATA_START_SYMBOLS="${CREATE_SHLIB+PROVIDE (}__data_start = .${CREATE_SHLIB+)};"

GENERATE_SHLIB_SCRIPT=yes
GENERATE_PIE_SCRIPT=yes

ARCH=arm
MACHINE=
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"
ENTRY=_start
EMBEDDED=yes

# This sets the stack to the top of the simulator memory (2^19 bytes).
STACK_ADDR=0x80000

# ARM does not support .s* sections.
NO_SMALL_DATA=yes

# ARM supports the .noinit and .persistent sections.
HAVE_NOINIT=yes
HAVE_PERSISTENT=yes
