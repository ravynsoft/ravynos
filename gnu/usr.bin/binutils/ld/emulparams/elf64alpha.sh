ENTRY=_start
SCRIPT_NAME=elf
ELFSIZE=64
TEMPLATE_NAME=elf
EXTRA_EM_FILE=alphaelf
OUTPUT_FORMAT="elf64-alpha"
NO_REL_RELOCS=yes
TEXT_START_ADDR="0x120000000"
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"
COMMONPAGESIZE="CONSTANT (COMMONPAGESIZE)"
ARCH=alpha
MACHINE=
GENERATE_SHLIB_SCRIPT=yes
GENERATE_PIE_SCRIPT=yes

# Yes, we want duplicate .plt sections.  The linker chooses the
# appropriate one magically in alpha_after_open.
PLT=".plt          ${RELOCATING-0} : SPECIAL { *(.plt) }"
DATA_PLT=yes
TEXT_PLT=yes

# Note that the number is always big-endian, thus we have to
# reverse the digit string.
NOP=0x0000fe2f1f04ff47		# unop; nop

OTHER_READONLY_SECTIONS="
  .reginfo      ${RELOCATING-0} : { *(.reginfo) }"
