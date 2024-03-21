# RV32 code using ILP32 ABI.
source_sh ${srcdir}/emulparams/elf32lriscv-defs.sh
OUTPUT_FORMAT="elf32-littleriscv"

# On Linux, first look for 32 bit ILP32 target libraries in /lib/ilp32 as per
# the glibc ABI.
case "$target" in
  riscv32*-linux*)
    case "$EMULATION_NAME" in
      *32*)
	LIBPATH_SUFFIX="32/ilp32 32" ;;
    esac
    ;;
esac
