#source: fdpic-gotofffuncdesci20-static.s
#as: --isa=sh2a -big --fdpic
#ld: -EB -mshelf_fd
#objdump: -ds
#target: sh*-*-uclinux*

.*:[ \t]+file format elf32-shbig-fdpic

Contents of section \.text:
 400094 01f0fff8 31cc0009[ \t]+.*
Contents of section \.rofixup:
 40009c 004100a8 004100ac 004100b0[ \t]+.*
Contents of section \.got:
 4100a8 0040009a 004100b0 00000000 00000000[ \t]+.*
 4100b8 00000000[ \t]+.*

Disassembly of section \.text:

00400094 <_start>:
[ \t]+400094:[ \t]+01 f0 ff f8[ \t]+movi20[ \t]+#-8,r1
[ \t]+400098:[ \t]+31 cc[ \t]+add[ \t]+r12,r1

0040009a <foo>:
[ \t]+40009a:[ \t]+00 09[ \t]+nop[ \t]+
