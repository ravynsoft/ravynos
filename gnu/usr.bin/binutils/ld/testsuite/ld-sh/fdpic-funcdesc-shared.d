#source: fdpic-funcdesc-shared.s
#as: --isa=sh2a -big --fdpic
#ld: -EB -mshelf_fd -shared
#objdump: -dsR -j.text -j.data -j.got
#target: sh*-*-uclinux*

.*:[ \t]+file format elf32-shbig-fdpic

Contents of section \.text:
 [0-9a-f]+ 0009[ \t]+.*
Contents of section \.data:
 [0-9a-f]+ 00000000[ \t]+.*
Contents of section \.got:
 [0-9a-f]+ 00000000 00000000 00000000 00000000[ \t]+.*
 [0-9a-f]+ 00000000[ \t]+.*

Disassembly of section \.text:

[0-9a-f]+ <foo>:
 [0-9a-f]+:[ \t]+00 09[ \t]+nop[ \t]+

Disassembly of section \.data:

[0-9a-f]+ <bar>:
[ \t]+[0-9a-f]+:[ \t]+00 00 00 00[ \t]+\.\.\.\.
[ \t]+[0-9a-f]+: R_SH_DIR32[ \t]+\.got

Disassembly of section \.got:

[0-9a-f]+ <\.got>:
[ \t]+\.\.\.
[ \t]+[0-9a-f]+: R_SH_FUNCDESC_VALUE[ \t]+\.text
