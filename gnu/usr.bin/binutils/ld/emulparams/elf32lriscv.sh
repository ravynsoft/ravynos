# RV32 code using ILP32D ABI.
# ABI not in emulation name to avoid breaking backward compatibility.
source_sh ${srcdir}/emulparams/elf32lriscv-defs.sh
OUTPUT_FORMAT="elf32-littleriscv"

# On Linux, first look for 32 bit ILP32D target libraries in /lib/ilp32d as per
# the glibc ABI.
case "$target" in
  riscv32*-linux*)
    case "$EMULATION_NAME" in
      *32*)
	LIBPATH_SUFFIX="32/ilp32d 32" ;;
    esac
    ;;
esac
