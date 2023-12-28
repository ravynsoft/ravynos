#PROG: objcopy
#objdump: -h
#objcopy: --set-section-flags foo=contents,alloc,load,code
#name: copy with setting section flags 2
#source: copytest.s
# Many formats do not allow arbitrary section flags, just run for ELF and PE.
#target: [is_elf_format] [is_pecoff_format]
#xfail: h8300-*-*

.*: +file format .*

Sections:
Idx.*
#...
  [0-9]* foo.*
                  CONTENTS, ALLOC, LOAD, CODE
#...
