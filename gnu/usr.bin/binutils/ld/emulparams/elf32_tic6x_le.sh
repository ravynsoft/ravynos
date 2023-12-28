SCRIPT_NAME=elf
TEMPLATE_NAME=elf
OUTPUT_FORMAT="elf32-tic6x-le"
BIG_OUTPUT_FORMAT="elf32-tic6x-be"
EXTRA_EM_FILE=tic6xdsbt
GENERATE_SHLIB_SCRIPT=yes
# This address is an arbitrary value expected to be suitable for
# semihosting simulator use, but not on hardware where it is expected
# to be overridden.
case ${target} in
    *-elf)
	TEXT_START_ADDR=0x8000
	;;
    *-uclinux)
	TEXT_START_ADDR=0x0
	GOT="
.got ${RELOCATING-0} : {
  ${RELOCATING+*(.dsbt)
  *(.got.plt) *(.igot.plt) }*(.got)${RELOCATING+ *(.igot)}
}"
	;;
esac
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"
ARCH=tic6x
OTHER_GOT_SYMBOLS="PROVIDE_HIDDEN (__c6xabi_DSBT_BASE = .);"
# ".bss" is near (small) BSS, ".far" is far (normal) BSS, ".const" is
# far read-only data, ".rodata" is near read-only data.  ".neardata"
# is near (small) data, ".fardata" is (along with .data) far data.
RODATA_NAME="const"
SDATA_NAME="neardata"
SBSS_NAME="bss"
BSS_NAME="far"
OTHER_READONLY_SECTIONS="
  .c6xabi.extab ${RELOCATING-0} : { *(.c6xabi.extab${RELOCATING+* .gnu.linkonce.c6xabiextab.*}) }
  .c6xabi.exidx ${RELOCATING-0} :
    {
      ${RELOCATING+PROVIDE_HIDDEN (__exidx_start = .);}
      *(.c6xabi.exidx${RELOCATING+* .gnu.linkonce.c6xabiexidx.*})
      ${RELOCATING+PROVIDE_HIDDEN (__exidx_end = .);}
    }"
OTHER_SDATA_SECTIONS=".rodata ${RELOCATING-0} : { *(.rodata${RELOCATING+ .rodata.*}) }"
OTHER_READONLY_RELOC_SECTIONS="
  .rel.rodata   ${RELOCATING-0} : { *(.rel.rodata${RELOCATING+ .rel.rodata.*}) }
  .rela.rodata  ${RELOCATING-0} : { *(.rela.rodata${RELOCATING+ .rela.rodata.*}) }"
OTHER_READWRITE_SECTIONS=".fardata ${RELOCATING-0} : { *(.fardata${RELOCATING+ .fardata.*}) }"
OTHER_READWRITE_RELOC_SECTIONS="
  .rel.fardata     ${RELOCATING-0} : { *(.rel.fardata${RELOCATING+ .rel.fardata.*}) }
  .rela.fardata    ${RELOCATING-0} : { *(.rela.fardata${RELOCATING+ .rela.fardata.*}) }"
# For relocating operation, skip OTHER_BSS_SECTIONS, or will cause multiple definition.
if [ ${RELOCATING-0} ]; then
  OTHER_BSS_SECTIONS="";
else
  case ${target} in

    *-elf)
	OTHER_BSS_SECTIONS="
  .heap :
  {
    . = ALIGN(4);
    _HEAP_START = .;
    . += 0x2000000;
    _HEAP_MAX = .;
  }
  .stack :
  {
    . +=  0x100000;
    _STACK_START = .;
  }"
	;;
  esac
fi
ATTRS_SECTIONS='.c6xabi.attributes 0 : { KEEP (*(.c6xabi.attributes)) KEEP (*(.gnu.attributes)) }'
