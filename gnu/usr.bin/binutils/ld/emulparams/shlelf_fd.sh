# If you change this file, please also look at files which source this one:
# shelf_fd.sh

source_sh ${srcdir}/emulparams/shlelf_linux.sh
OUTPUT_FORMAT="elf32-sh-fdpic"
GOT=".got          ${RELOCATING-0} : {${RELOCATING+ *(.got.funcdesc) *(.got.plt)} *(.got) }"
OTHER_GOT_RELOC_SECTIONS="
  .rela.got.funcdesc      ${RELOCATING-0} : { *(.rela.got.funcdesc) }
"
OTHER_READONLY_SECTIONS="
  .rofixup        : {
    ${RELOCATING+__ROFIXUP_LIST__ = .;}
    *(.rofixup)
    ${RELOCATING+__ROFIXUP_END__ = .;}
  }
"
