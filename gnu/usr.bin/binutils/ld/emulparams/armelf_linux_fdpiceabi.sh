source_sh ${srcdir}/emulparams/armelf_linux_eabi.sh

OUTPUT_FORMAT="elf32-littlearm-fdpic"
BIG_OUTPUT_FORMAT="elf32-bigarm-fdpic"
LITTLE_OUTPUT_FORMAT="elf32-littlearm-fdpic"

OTHER_READONLY_SECTIONS="${OTHER_READONLY_SECTIONS}
  .rofixup : {
	${RELOCATING+__ROFIXUP_LIST__ = .;}
	*(.rofixup)
	${RELOCATING+__ROFIXUP_END__ = .;}
}"
