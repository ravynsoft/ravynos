# This is an ELF platform.
SCRIPT_NAME=elf
ARCH=loongarch
NO_REL_RELOCS=yes

TEMPLATE_NAME=elf
EXTRA_EM_FILE=loongarchelf

ELFSIZE=32

if test `echo "$host" | sed -e s/64//` = `echo "$target" | sed -e s/64//`; then
  case " $EMULATION_LIBPATH " in
    *" ${EMULATION_NAME} "*)
      NATIVE=yes
      ;;
  esac
fi

# Enable shared library support for everything except an embedded elf target.
case "$target" in
  loongarch*-elf)
    ;;
  *)
    GENERATE_SHLIB_SCRIPT=yes
    GENERATE_PIE_SCRIPT=yes
    ;;
esac

IREL_IN_PLT=
TEXT_START_ADDR=0x10000
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"
COMMONPAGESIZE="CONSTANT (COMMONPAGESIZE)"

SEPARATE_GOTPLT=0
INITIAL_READONLY_SECTIONS=".interp         : { *(.interp) } ${CREATE_PIE-${INITIAL_READONLY_SECTIONS}}"
INITIAL_READONLY_SECTIONS="${RELOCATING+${CREATE_SHLIB-${INITIAL_READONLY_SECTIONS}}}"
