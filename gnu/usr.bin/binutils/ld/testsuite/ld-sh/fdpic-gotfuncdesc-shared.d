#source: fdpic-gotfuncdesc-shared.s
#as: --isa=sh2a -big --fdpic
#ld: -EB -mshelf_fd -shared
#objdump: -dsR -j.text -j.got
#target: sh*-*-uclinux*

.*:[ \t]+file format elf32-shbig-fdpic

Contents of section \.text:
 [0-9a-f]+ d00001ce 0000000c[ \t]+.*
Contents of section \.got:
 [0-9a-f]+ 00000000 00000000 00000000 00000000[ \t]+.*

Disassembly of section \.text:

[0-9a-f]+ <f>:
 [0-9a-f]+:[ \t]+d0 00[ \t]+mov\.l[ \t]+[0-9a-f]+ <f\+0x4>,r0[ \t]+! c
 [0-9a-f]+:[ \t]+01 ce[ \t]+mov\.l[ \t]+@\(r0,r12\),r1
 [0-9a-f]+:[ \t]+00 00 00 0c[ \t]+movi20[ \t]+#12,r0

Disassembly of section \.got:

[0-9a-f]+ <\.got>:
[ \t]+\.\.\.
[ \t]+[0-9a-f]+: R_SH_FUNCDESC[ \t]+foo
