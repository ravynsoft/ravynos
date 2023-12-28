# This is an ELF platform.
SCRIPT_NAME=elf
ARCH=riscv
NO_REL_RELOCS=yes

TEMPLATE_NAME=elf
EXTRA_EM_FILE=riscvelf

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
  riscv*-elf)
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

DATA_START_SYMBOLS="${CREATE_SHLIB-__DATA_BEGIN__ = .;}"

SDATA_START_SYMBOLS="${CREATE_SHLIB-__SDATA_BEGIN__ = .;}
    *(.srodata.cst16) *(.srodata.cst8) *(.srodata.cst4) *(.srodata.cst2) *(.srodata .srodata.*)"

INITIAL_READONLY_SECTIONS=".interp         : { *(.interp) } ${CREATE_PIE-${INITIAL_READONLY_SECTIONS}}"
INITIAL_READONLY_SECTIONS="${RELOCATING+${CREATE_SHLIB-${INITIAL_READONLY_SECTIONS}}}"

# We must cover as much of sdata as possible if it exists.  If sdata+bss is
# smaller than 0x1000 then we should start from bss end to cover as much of
# the program as possible.  But we can't allow gp to cover any of rodata, as
# the address of variables in rodata may change during relaxation, so we start
# from data in that case.
OTHER_END_SYMBOLS="${CREATE_SHLIB-__BSS_END__ = .;
    __global_pointer$ = MIN(__SDATA_BEGIN__ + 0x800,
		            MAX(__DATA_BEGIN__ + 0x800, __BSS_END__ - 0x800));}"
