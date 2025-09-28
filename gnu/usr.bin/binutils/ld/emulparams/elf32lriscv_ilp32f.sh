# RV32 code using ILP32F ABI.
source_sh ${srcdir}/emulparams/elf32lriscv-defs.sh
OUTPUT_FORMAT="elf32-littleriscv"

# On Linux, first look for 32 bit ILP32F target libraries in /lib/ilp32f as per
# the glibc ABI.
case "$target" in
  riscv32*-linux*)
    case "$EMULATION_NAME" in
      *32*)
	LIBPATH_SUFFIX="32/ilp32f 32" ;;
    esac
    ;;
esac
