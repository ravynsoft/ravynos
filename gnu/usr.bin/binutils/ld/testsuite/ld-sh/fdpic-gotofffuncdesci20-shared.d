#source: fdpic-gotofffuncdesci20-shared.s
#as: --isa=sh2a -big --fdpic
#ld: -EB -mshelf_fd -shared
#objdump: -dsR -j.text -j.got
#target: sh*-*-uclinux*

.*:[ \t]+file format elf32-shbig-fdpic

Contents of section \.text:
 [0-9a-f]+ 01f0fff8 31cc0009[ \t]+.*
Contents of section \.got:
 [0-9a-f]+ 00000006 00000000 00000000 00000000[ \t]+.*
 [0-9a-f]+ 00000000[ \t]+.*

Disassembly of section \.text:

[0-9a-f]+ <f>:
 [0-9a-f]+:[ \t]+01 f0 ff f8[ \t]+movi20[ \t]+#-8,r1
 [0-9a-f]+:[ \t]+31 cc[ \t]+add[ \t]+r12,r1

[0-9a-f]+ <foo>:
 [0-9a-f]+:[ \t]+00 09[ \t]+nop[ \t]+

Disassembly of section \.got:

[0-9a-f]+ <\.got>:
[ \t]+[0-9a-f]+:[ \t]+00 00 00 06[ \t]+movi20[ \t]+#6,r0
[ \t]+[0-9a-f]+: R_SH_FUNCDESC_VALUE[ \t]+\.text
[ \t]+\.\.\.
