#source: fdpic-gotofffuncdesc-static.s
#as: --isa=sh2a -big --fdpic
#ld: -EB -mshelf_fd
#objdump: -ds
#target: sh*-*-uclinux*

.*:[ \t]+file format elf32-shbig-fdpic

Contents of section \.text:
 400094 d10031cc fffffff8 00090009[ \t]+.*
Contents of section \.rofixup:
 4000a0 004100ac 004100b0 004100b4[ \t]+.*
Contents of section \.got:
 4100ac 0040009c 004100b4 00000000 00000000[ \t]+.*
 4100bc 00000000[ \t]+.*

Disassembly of section \.text:

00400094 <_start>:
[ \t]+400094:[ \t]+d1 00[ \t]+mov\.l[ \t]+400098 <_start\+0x4>,r1[ \t]+! fffffff8
[ \t]+400096:[ \t]+31 cc[ \t]+add[ \t]+r12,r1
[ \t]+400098:[ \t]+ff ff[ \t]+\.word 0xffff
[ \t]+40009a:[ \t]+ff f8[ \t]+fmov[ \t]+@r15,fr15

0040009c <foo>:
[ \t]+40009c:[ \t]+00 09[ \t]+nop[ \t]+
[ \t]+40009e:[ \t]+00 09[ \t]+nop[ \t]+
