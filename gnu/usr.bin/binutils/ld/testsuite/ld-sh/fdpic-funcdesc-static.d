#source: fdpic-funcdesc-static.s
#as: --isa=sh2a -big --fdpic
#ld: -EB -mshelf_fd
#objdump: -ds
#target: sh*-*-uclinux*

.*:[ \t]+file format elf32-shbig-fdpic

Contents of section \.text:
 400094 00090009[ \t]+.*
Contents of section \.rofixup:
 400098 004100ac 004100b0 004100a8 004100b4[ \t]+.*
Contents of section \.data:
 4100a8 004100ac[ \t]+.*
Contents of section \.got:
 4100ac 00400094 004100b4 00000000 00000000[ \t]+.*
 4100bc 00000000[ \t]+.*

Disassembly of section \.text:

00400094 <foo>:
[ \t]+400094:[ \t]+00 09[ \t]+nop[ \t]+

00400096 <_start>:
[ \t]+400096:[ \t]+00 09[ \t]+nop[ \t]+
