#source: fdpic-gotoffi20-shared.s
#as: --isa=sh2a -big --fdpic
#ld: -EB -mshelf_fd -shared
#objdump: -dsR -j.text -j.data -j.got
#target: sh*-*-uclinux*

.*:[ \t]+file format elf32-shbig-fdpic

Contents of section \.text:
 [0-9a-f]+ 00f0fffc 01ce[ \t]+.*
Contents of section \.data:
 [0-9a-f]+ 00000001[ \t]+.*
Contents of section \.got:
 [0-9a-f]+ 00000000 00000000 00000000[ \t]+.*

Disassembly of section \.text:

[0-9a-f]+ <f>:
 [0-9a-f]+:[ \t]+00 f0 ff fc[ \t]+movi20[ \t]+#-4,r0
 [0-9a-f]+:[ \t]+01 ce[ \t]+mov\.l[ \t]+@\(r0,r12\),r1

Disassembly of section \.data:

[0-9a-f]+ <foo>:
[ \t]+[0-9a-f]+:[ \t]+00 00 00 01[ \t]+\.\.\.\.

Disassembly of section \.got:

[0-9a-f]+ <\.got>:
[ \t]+\.\.\.
