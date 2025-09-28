# RV64 code using LP64D ABI.
# ABI not in emulation name to avoid breaking backward compatibility.
source_sh ${srcdir}/emulparams/elf64lriscv-defs.sh
OUTPUT_FORMAT="elf64-littleriscv"

# On Linux, first look for 64 bit LP64D target libraries in /lib64/lp64d as per
# the glibc ABI, and then /lib64 for backward compatility.
case "$target" in
  riscv64*-linux*)
    case "$EMULATION_NAME" in
      *64*)
	LIBPATH_SUFFIX="64/lp64d 64";;
    esac
    ;;
esac
