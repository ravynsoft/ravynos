#source: fdpic-gotfuncdesci20-static.s
#as: --isa=sh2a -big --fdpic
#ld: -EB -mshelf_fd
#objdump: -ds
#target: sh*-*-uclinux*

.*:[ \t]+file format elf32-shbig-fdpic

Contents of section \.text:
 400094 0000000c 01ce0009[ \t]+.*
Contents of section \.rofixup:
 40009c 004100ac 004100b0 004100c0 004100b4[ \t]+.*
Contents of section \.got:
 4100ac 0040009a 004100b4 00000000 00000000[ \t]+.*
 4100bc 00000000 004100ac[ \t]+.*

Disassembly of section \.text:

00400094 <_start>:
[ \t]+400094:[ \t]+00 00 00 0c[ \t]+movi20[ \t]+#12,r0
[ \t]+400098:[ \t]+01 ce[ \t]+mov\.l[ \t]+@\(r0,r12\),r1

0040009a <foo>:
[ \t]+40009a:[ \t]+00 09[ \t]+nop[ \t]+
