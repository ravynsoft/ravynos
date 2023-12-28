source_sh ${srcdir}/emulparams/plt_unwind.sh
source_sh ${srcdir}/emulparams/extern_protected_data.sh
source_sh ${srcdir}/emulparams/dynamic_undefined_weak.sh
source_sh ${srcdir}/emulparams/call_nop.sh
SCRIPT_NAME=elf
OUTPUT_FORMAT="elf32-iamcu"
NO_RELA_RELOCS=yes
TEXT_START_ADDR=0x08048000
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"
COMMONPAGESIZE="CONSTANT (COMMONPAGESIZE)"
ARCH=iamcu
MACHINE=
TEMPLATE_NAME=elf
EXTRA_EM_FILE="elf-x86"
GENERATE_SHLIB_SCRIPT=yes
GENERATE_PIE_SCRIPT=yes
NO_SMALL_DATA=yes
SEPARATE_GOTPLT="SIZEOF (.got.plt) >= 12 ? 12 : 0"
IREL_IN_PLT=
# These sections are placed right after .plt section.
OTHER_PLT_SECTIONS="
.plt.got      ${RELOCATING-0} : { *(.plt.got) }
"

# Linux modify the default library search path to first include
# a 32-bit specific directory.
case "$target" in
  x86_64*-linux* | i[3-7]86*-linux*)
    case "$EMULATION_NAME" in
      *i386*)
	LIBPATH_SUFFIX=32
	LIBPATH_SUFFIX_SKIP=64
	;;
    esac
    ;;
esac
