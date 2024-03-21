# The PLT-agnostic parts of a generic 32-bit ELF PowerPC target.  Included by:
# elf32ppc.sh elf32ppcvxworks.sh elf64ppc.sh
source_sh ${srcdir}/emulparams/dynamic_undefined_weak.sh

TEMPLATE_NAME=elf
GENERATE_SHLIB_SCRIPT=yes
GENERATE_PIE_SCRIPT=yes
SCRIPT_NAME=elf
OUTPUT_FORMAT="elf32-powerpc"
NO_REL_RELOCS=yes
TEXT_START_ADDR=0x01800000
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"
COMMONPAGESIZE="CONSTANT (COMMONPAGESIZE)"
ARCH=powerpc:common
MACHINE=
EXECUTABLE_SYMBOLS='PROVIDE (__stack = 0); PROVIDE (___stack = 0);'
if test -z "${CREATE_SHLIB}"; then
  SBSS_START_SYMBOLS="PROVIDE (__sbss_start = .); PROVIDE (___sbss_start = .);"
  SBSS_END_SYMBOLS="PROVIDE (__sbss_end = .); PROVIDE (___sbss_end = .);"
else
  unset SDATA_START_SYMBOLS
  unset SDATA2_START_SYMBOLS
  unset SBSS_START_SYMBOLS
  unset SBSS_END_SYMBOLS
fi
OTHER_END_SYMBOLS="${CREATE_SHLIB+PROVIDE (}__end = .${CREATE_SHLIB+)};"
OTHER_RELRO_SECTIONS="
  .fixup        ${RELOCATING-0} : { *(.fixup) }
  .got1         ${RELOCATING-0} : { *(.got1) }
  .got2         ${RELOCATING-0} : { *(.got2) }
"
OTHER_GOT_RELOC_SECTIONS="
  .rela.got1         ${RELOCATING-0} : { *(.rela.got1) }
  .rela.got2         ${RELOCATING-0} : { *(.rela.got2) }
"

# Treat a host that matches the target with the possible exception of "64"
# in the name as if it were native.
if test `echo "$host" | sed -e s/64//` = `echo "$target" | sed -e s/64//`; then
  case " $EMULATION_LIBPATH " in
    *" ${EMULATION_NAME} "*)
      NATIVE=yes
      ;;
  esac
fi

# Look for 64 bit target libraries in /lib64, /usr/lib64 etc., first.
# Similarly, look for 32 bit libraries in /lib32, /usr/lib32 etc.
case `echo "$target" | sed -e 's/-.*//'`:"$EMULATION_NAME" in
  *le:*64lppc*) LIBPATH_SUFFIX=64 ;;
  *le:*32lppc*) LIBPATH_SUFFIX=32 ;;
  *le:*64*) LIBPATH_SUFFIX=64be ;;
  *le:*32*) LIBPATH_SUFFIX=32be ;;
  *:*64lppc*) LIBPATH_SUFFIX=64le ;;
  *:*32lppc*) LIBPATH_SUFFIX=32le ;;
  *:*64*) LIBPATH_SUFFIX=64 ;;
  *:*32*) LIBPATH_SUFFIX=32 ;;
esac
