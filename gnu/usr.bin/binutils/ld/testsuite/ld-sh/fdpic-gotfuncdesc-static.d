#source: fdpic-gotfuncdesc-static.s
#as: --isa=sh2a -big --fdpic
#ld: -EB -mshelf_fd
#objdump: -ds
#target: sh*-*-uclinux*

.*:[ \t]+file format elf32-shbig-fdpic

Contents of section \.text:
 400094 d00001ce 0000000c 00090009[ \t]+.*
Contents of section \.rofixup:
 4000a0 004100b0 004100b4 004100c4 004100b8[ \t]+.*
Contents of section \.got:
 4100b0 0040009c 004100b8 00000000 00000000[ \t]+.*
 4100c0 00000000 004100b0[ \t]+.*

Disassembly of section \.text:

00400094 <_start>:
[ \t]+400094:[ \t]+d0 00[ \t]+mov\.l[ \t]+400098 <_start\+0x4>,r0[ \t]+! c
[ \t]+400096:[ \t]+01 ce[ \t]+mov\.l[ \t]+@\(r0,r12\),r1
[ \t]+400098:[ \t]+00 00 00 0c[ \t]+movi20[ \t]+#12,r0

0040009c <foo>:
[ \t]+40009c:[ \t]+00 09[ \t]+nop[ \t]+
[ \t]+40009e:[ \t]+00 09[ \t]+nop[ \t]+
