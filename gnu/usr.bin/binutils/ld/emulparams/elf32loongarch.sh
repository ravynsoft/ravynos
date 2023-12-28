source_sh ${srcdir}/emulparams/elf32loongarch-defs.sh
OUTPUT_FORMAT="elf32-loongarch"

case "$target" in
  loongarch32*-linux*)
    case "$EMULATION_NAME" in
      *32*)
	LIBPATH_SUFFIX="32" ;;
    esac
    ;;
esac
