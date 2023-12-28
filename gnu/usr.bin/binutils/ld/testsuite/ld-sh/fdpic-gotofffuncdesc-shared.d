#source: fdpic-gotofffuncdesc-shared.s
#as: --isa=sh2a -big --fdpic
#ld: -EB -mshelf_fd -shared
#objdump: -dsR -j.text -j.got
#target: sh*-*-uclinux*

.*:[ \t]+file format elf32-shbig-fdpic

Contents of section \.text:
 [0-9a-f]+ d10031cc fffffff8 00090009[ \t]+.*
Contents of section \.got:
 [0-9a-f]+ 00000008 00000000 00000000 00000000[ \t]+.*
 [0-9a-f]+ 00000000[ \t]+.*

Disassembly of section \.text:

[0-9a-f]+ <f>:
 [0-9a-f]+:[ \t]+d1 00[ \t]+mov\.l[ \t]+[0-9a-f]+ <f\+0x4>,r1[ \t]+! fffffff8
 [0-9a-f]+:[ \t]+31 cc[ \t]+add[ \t]+r12,r1
 [0-9a-f]+:[ \t]+ff ff[ \t]+\.word 0xffff
 [0-9a-f]+:[ \t]+ff f8[ \t]+fmov[ \t]+@r15,fr15

[0-9a-f]+ <foo>:
 [0-9a-f]+:[ \t]+00 09[ \t]+nop[ \t]+
 [0-9a-f]+:[ \t]+00 09[ \t]+nop[ \t]+

Disassembly of section \.got:

[0-9a-f]+ <\.got>:
[ \t]+[0-9a-f]+:[ \t]+00 00 00 08[ \t]+movi20[ \t]+#8,r0
[ \t]+[0-9a-f]+: R_SH_FUNCDESC_VALUE[ \t]+\.text
[ \t]+\.\.\.
