# If you change this file, please also look at files which source this one:
# elf32b4300.sh elf32bsmip.sh elf32btsmip.sh elf32ebmip.sh elf32lmip.sh
# elf32ebmipvxworks.sh elf32elmipvxworks.sh

SCRIPT_NAME=elf
OUTPUT_FORMAT="elf32-bigmips"
BIG_OUTPUT_FORMAT="elf32-bigmips"
LITTLE_OUTPUT_FORMAT="elf32-littlemips"
TEXT_START_ADDR=0x0400000
test -n "${EMBEDDED}" || DATA_ADDR=0x10000000
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"
COMMONPAGESIZE="CONSTANT (COMMONPAGESIZE)"
SHLIB_TEXT_START_ADDR=0x5ffe0000
TEXT_DYNAMIC=
INITIAL_READONLY_SECTIONS=
if test -z "${CREATE_SHLIB}"; then
  INITIAL_READONLY_SECTIONS=".interp       ${RELOCATING-0} : { *(.interp) }"
fi
INITIAL_READONLY_SECTIONS="${INITIAL_READONLY_SECTIONS}
  .MIPS.abiflags ${RELOCATING-0} : { *(.MIPS.abiflags) }
  .reginfo       ${RELOCATING-0} : { *(.reginfo) }
  .MIPS.xhash    ${RELOCATING-0} : { *(.MIPS.xhash) }
"
OTHER_TEXT_SECTIONS='*(.mips16.fn.*) *(.mips16.call.*)'
# Unlike most targets, the MIPS backend puts all dynamic relocations
# in a single dynobj section, which it also calls ".rel.dyn".  It does
# this so that it can easily sort all dynamic relocations before the
# output section has been populated.
OTHER_GOT_RELOC_SECTIONS="
  .rel.dyn      ${RELOCATING-0} : { *(.rel.dyn) }
"
# If the output has a GOT section, there must be exactly 0x7ff0 bytes
# between .got and _gp.
OTHER_GOT_SYMBOLS='HIDDEN (_gp = ALIGN (16) + 0x7ff0);'

# .got.plt is only used for the PLT psABI extension.  It should not be
# included in the .sdata block with .got, as there is no need to access
# the section from _gp.  Note that the traditional:
#
#      . = .
#      _gp = ALIGN (16) + 0x7ff0;
#      .got : { *(.got.plt) *(.got) }
#
# would set _gp to the wrong value; _gp - 0x7ff0 must point to the start
# of *(.got).
GOT=".got          ${RELOCATING-0} : { *(.got) }"
unset OTHER_READWRITE_SECTIONS
unset OTHER_RELRO_SECTIONS
if test -n "$RELRO_NOW"; then
  OTHER_RELRO_SECTIONS=".got.plt      ${RELOCATING-0} : { *(.got.plt) }"
else
  OTHER_READWRITE_SECTIONS=".got.plt      ${RELOCATING-0} : { *(.got.plt) }"
fi

OTHER_SDATA_SECTIONS="
  .lit8         ${RELOCATING-0} : { *(.lit8) }
  .lit4         ${RELOCATING-0} : { *(.lit4) }
"
TEXT_START_SYMBOLS="${CREATE_SHLIB+PROVIDE (}_ftext = .${CREATE_SHLIB+)};"
DATA_START_SYMBOLS="${CREATE_SHLIB+PROVIDE (}_fdata = .${CREATE_SHLIB+)};"
OTHER_BSS_SYMBOLS="${CREATE_SHLIB+PROVIDE (}_fbss = .${CREATE_SHLIB+)};"
OTHER_SECTIONS="
  .gptab.sdata : {${RELOCATING+ *(.gptab.data)} *(.gptab.sdata) }
  .gptab.sbss : {${RELOCATING+ *(.gptab.bss)} *(.gptab.sbss) }
  .mdebug.abi32 0 : { KEEP(*(.mdebug.abi32)) }
  .mdebug.abiN32 0 : { KEEP(*(.mdebug.abiN32)) }
  .mdebug.abi64 0 : { KEEP(*(.mdebug.abi64)) }
  .mdebug.abiO64 0 : { KEEP(*(.mdebug.abiO64)) }
  .mdebug.eabi32 0 : { KEEP(*(.mdebug.eabi32)) }
  .mdebug.eabi64 0 : { KEEP(*(.mdebug.eabi64)) }
  .gcc_compiled_long32 0 : { KEEP(*(.gcc_compiled_long32)) }
  .gcc_compiled_long64 0 : { KEEP(*(.gcc_compiled_long64)) }
"
ARCH=mips
MACHINE=
TEMPLATE_NAME=elf
EXTRA_EM_FILE=mipself
GENERATE_SHLIB_SCRIPT=yes
GENERATE_PIE_SCRIPT=yes
