source_sh ${srcdir}/emulparams/plt_unwind.sh
source_sh ${srcdir}/emulparams/extern_protected_data.sh
source_sh ${srcdir}/emulparams/dynamic_undefined_weak.sh
source_sh ${srcdir}/emulparams/reloc_overflow.sh
source_sh ${srcdir}/emulparams/call_nop.sh
source_sh ${srcdir}/emulparams/cet.sh
source_sh ${srcdir}/emulparams/x86-report-relative.sh
source_sh ${srcdir}/emulparams/x86-64-level.sh
source_sh ${srcdir}/emulparams/static.sh
source_sh ${srcdir}/emulparams/dt-relr.sh
SCRIPT_NAME=elf
ELFSIZE=32
OUTPUT_FORMAT="elf32-x86-64"
NO_REL_RELOCS=yes
TEXT_START_ADDR=0x400000
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"
COMMONPAGESIZE="CONSTANT (COMMONPAGESIZE)"
ARCH="i386:x64-32"
MACHINE=
TEMPLATE_NAME=elf
EXTRA_EM_FILE="elf-x86"
GENERATE_SHLIB_SCRIPT=yes
GENERATE_PIE_SCRIPT=yes
NO_SMALL_DATA=yes
LARGE_SECTIONS=yes
LARGE_BSS_AFTER_BSS=
SEPARATE_GOTPLT="SIZEOF (.got.plt) >= 24 ? 24 : 0"
IREL_IN_PLT=
# These sections are placed right after .plt section.
OTHER_PLT_SECTIONS="
.plt.got      ${RELOCATING-0} : { *(.plt.got) }
.plt.sec      ${RELOCATING-0} : { *(.plt.sec) }
"

if [ "x${host}" = "x${target}" ]; then
  case " $EMULATION_LIBPATH " in
    *" ${EMULATION_NAME} "*)
      NATIVE=yes
  esac
fi

# Linux modifies the default library search path to first include
# a 32-bit specific directory.
case "$target" in
  x86_64*-linux*|i[3-7]86-*-linux-*)
    case "$EMULATION_NAME" in
      *32*)
	LIBPATH_SUFFIX=x32
	LIBPATH_SUFFIX_SKIP=64
	;;
      *64*)
	LIBPATH_SUFFIX=64
	;;
    esac
    ;;
esac
