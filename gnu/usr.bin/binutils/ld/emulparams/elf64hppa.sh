SCRIPT_NAME=elf64hppa
ELFSIZE=64
OUTPUT_FORMAT="elf64-hppa"
NO_REL_RELOCS=yes
TEXT_START_ADDR=0x4000000000001000
DATA_ADDR=0x8000000000001000
TARGET_PAGE_SIZE=4096
MAXPAGESIZE="CONSTANT (MAXPAGESIZE)"
LIB_PATH="=/usr/lib/pa20_64:=/opt/langtools/lib/pa20_64"

# The HP dynamic linker actually requires you set the start of text and
# data to some reasonable value.  Of course nobody knows what reasoanble
# really is, so we just use the same values that HP's linker uses.
SHLIB_TEXT_START_ADDR=0x4000000000001000
SHLIB_DATA_ADDR=0x8000000000001000

ARCH=hppa
MACHINE=hppa2.0w
ENTRY="main"
TEMPLATE_NAME=elf
GENERATE_SHLIB_SCRIPT=yes

# We really want multiple .stub sections, one for each input .text section,
# but for now this is good enough.
OTHER_READONLY_SECTIONS="
  .PARISC.unwind ${RELOCATING-0} : { *(.PARISC.unwind) }"

# The PA64 ELF port treats .plt sections differently than most.  We also have
# to create a .opd section.  What most systems call the .got, we call the .dlt
OTHER_READWRITE_SECTIONS="
  .PARISC.pfa_count ${RELOCATING-0} : { *(.PARISC.pfa_count) }
  .PARISC.global ${RELOCATING-0} : { *(.PARISC.global) }
  .opd          ${RELOCATING-0} : { *(.opd) }
  ${RELOCATING+PROVIDE (__gp = .);}
  .plt          ${RELOCATING-0} : { *(.plt) }
  .dlt          ${RELOCATING-0} : { *(.dlt) }"

# The PA64 ELF port has an additional huge bss section.
OTHER_BSS_SECTIONS="
  .PARISC.ansi.common ${RELOCATING-0} : { *(.PARISC.ansi.common) }
  .PARISC.huge.common ${RELOCATING-0} : { *(.PARISC.huge.common) }
  .hbss         ${RELOCATING-0} : { *(.hbss) }
  .tbss         ${RELOCATING-0} : { *(.tbss) }"

#OTHER_SYMBOLS='PROVIDE (__TLS_SIZE = SIZEOF (.tbss));'
OTHER_SYMBOLS='
  PROVIDE (__TLS_SIZE = 0);
  PROVIDE (__TLS_INIT_SIZE = 0);
  PROVIDE (__TLS_INIT_START = 0);
  PROVIDE (__TLS_INIT_A = 0);
  PROVIDE (__TLS_PREALLOC_DTV_A = 0);'

# HPs use .dlt where systems use .got.  Sigh.
OTHER_GOT_RELOC_SECTIONS="
  .rela.dlt     ${RELOCATING-0} : { *(.rela.dlt) }
  .rela.opd     ${RELOCATING-0} : { *(.rela.opd) }"

# We're not actually providing a symbol anymore (due to the inability to be
# safe in regards to shared libraries). So we just allocate the hunk of space
# unconditionally, but do not mess around with the symbol table.
DATA_START_SYMBOLS='. += 16;'

DATA_PLT=
PLT_BEFORE_GOT=

# .dynamic should be at the start of the .text segment.
TEXT_DYNAMIC=

# The linker is required to define these two symbols.
OTHER_SYMBOLS='PROVIDE (__SYSTEM_ID = 0x214); PROVIDE (_FPU_STATUS = 0x0);'
# The PA64 ELF port needs two additional initializer sections and also wants
# a start/end symbol pair for the .init and .fini sections.
INIT_START='KEEP (*(.HP.init)); PROVIDE (__preinit_start = .); KEEP (*(.preinit)); PROVIDE (__preinit_end = .); PROVIDE (__init_start = .);'
INIT_END='PROVIDE (__init_end = .);'
FINI_START='PROVIDE (__fini_start = .);'
FINI_END='PROVIDE (__fini_end = .);'
