#PROG: objcopy
#objdump: -h
#objcopy: --set-section-flags .text=alloc,data
#name: copy with setting section flags 3
#source: bintest.s
# The .text section in most formats has a fixed set of flags which
# cannot be changed, just run for ELF.
#target: [is_elf_format]
#xfail: rx-*-*

.*: +file format .*

Sections:
Idx.*
#...
  [0-9]* .text.*
                  CONTENTS, ALLOC, LOAD, RELOC, DATA
#...
