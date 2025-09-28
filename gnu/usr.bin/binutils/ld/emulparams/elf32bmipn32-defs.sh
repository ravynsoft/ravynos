# If you change this file, please also look at files which source this one:
# elf64bmip.sh elf64btsmip.sh elf32btsmipn32.sh elf32bmipn32.sh

# This is an ELF platform.
SCRIPT_NAME=elf

# Handle both big- and little-ended 32-bit MIPS objects.
ARCH=mips
OUTPUT_FORMAT="elf32-bigmips"
BIG_OUTPUT_FORMAT="elf32-bigmips"
LITTLE_OUTPUT_FORMAT="elf32-littlemips"

TEMPLATE_NAME=elf
EXTRA_EM_FILE=mipself

# Note: use "x$var" not x"$var" in case directive in order to work around bug in bash 4.2
case "x$EMULATION_NAME" in
xelf32*n32*) ELFSIZE=32 ;;
xelf64*) ELFSIZE=64 ;;
x) ;;
*) echo $0: unhandled emulation $EMULATION_NAME >&2; exit 1 ;;
esac

if test `echo "$host" | sed -e s/64//` = `echo "$target" | sed -e s/64//`; then
  case " $EMULATION_LIBPATH " in
    *" ${EMULATION_NAME} "*)
      NATIVE=yes
      ;;
  esac
fi

# Look for 64 bit target libraries in /lib64, /usr/lib64 etc., first.
LIBPATH_SUFFIX=$ELFSIZE

GENERATE_SHLIB_SCRIPT=yes
GENERATE_PIE_SCRIPT=yes

TEXT_START_ADDR=0x10000000
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"
ENTRY=__start

# Unlike most targets, the MIPS backend puts all dynamic relocations
# in a single dynobj section, which it also calls ".rel.dyn".  It does
# this so that it can easily sort all dynamic relocations before the
# output section has been populated.
OTHER_GOT_RELOC_SECTIONS="
  .rel.dyn      ${RELOCATING-0} : { *(.rel.dyn) }
"
# GOT-related settings.
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
  .srdata       ${RELOCATING-0} : { *(.srdata) }
"

# Magic symbols.
TEXT_START_SYMBOLS="${CREATE_SHLIB+PROVIDE (}_ftext = .${CREATE_SHLIB+)};"
DATA_START_SYMBOLS="${CREATE_SHLIB+PROVIDE (}_fdata = .${CREATE_SHLIB+)};"
OTHER_BSS_SYMBOLS="${CREATE_SHLIB+PROVIDE (}_fbss = .${CREATE_SHLIB+)};"

INITIAL_READONLY_SECTIONS=
if test -z "${CREATE_SHLIB}"; then
  INITIAL_READONLY_SECTIONS=".interp       ${RELOCATING-0} : { *(.interp) }"
fi
INITIAL_READONLY_SECTIONS="${INITIAL_READONLY_SECTIONS}
  .MIPS.abiflags      ${RELOCATING-0} : { *(.MIPS.abiflags) }
  .MIPS.xhash      ${RELOCATING-0} : { *(.MIPS.xhash) }
  .reginfo      ${RELOCATING-0} : { *(.reginfo) }"
# Discard any .MIPS.content* or .MIPS.events* sections.  The linker
# doesn't know how to adjust them.
OTHER_SECTIONS="/DISCARD/ : { *(.MIPS.content*) *(.MIPS.events*) }"

TEXT_DYNAMIC=
