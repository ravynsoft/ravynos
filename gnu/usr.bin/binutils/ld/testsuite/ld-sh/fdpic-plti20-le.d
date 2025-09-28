#source: fdpic-plt.s
#as: --isa=sh2a -little --fdpic
#ld: -EL -mshlelf_fd -shared
#objdump: -dsR -j.plt -j.text -j.got
#target: sh*-*-uclinux*

.*:[ \t]+file format elf32-sh-fdpic

Contents of section \.plt:
 [0-9a-f]+ f000f0ff ce010470 2b41ce0c 00000000[ \t]+.*
 [0-9a-f]+ c2602b40 c1530900 f000f8ff ce010470[ \t]+.*
 [0-9a-f]+ 2b41ce0c 0c000000 c2602b40 c1530900[ \t]+.*
Contents of section \.text:
 [0-9a-f]+ 00d001d1 ccffffff e0ffffff[ \t]+.*
Contents of section \.got:
 [0-9a-f]+ 38020000 00000000 50020000 00000000[ \t]+.*
 [0-9a-f]+ 00000000 00000000 00000000[ \t]+.*

Disassembly of section \.plt:

[0-9a-f]+ <foo@plt>:
 [0-9a-f]+:[ \t]+f0 00 f0 ff[ \t]+movi20[ \t]+#-16,r0
 [0-9a-f]+:[ \t]+ce 01[ \t]+mov\.l[ \t]+@\(r0,r12\),r1
 [0-9a-f]+:[ \t]+04 70[ \t]+add[ \t]+#4,r0
 [0-9a-f]+:[ \t]+2b 41[ \t]+jmp[ \t]+@r1
 [0-9a-f]+:[ \t]+ce 0c[ \t]+mov\.l[ \t]+@\(r0,r12\),r12
 [0-9a-f]+:[ \t]+00 00 00 00[ \t]+movi20[ \t]+#0,r0
 [0-9a-f]+:[ \t]+c2 60[ \t]+mov\.l[ \t]+@r12,r0
 [0-9a-f]+:[ \t]+2b 40[ \t]+jmp[ \t]+@r0
 [0-9a-f]+:[ \t]+c1 53[ \t]+mov\.l[ \t]+@\(4,r12\),r3
 [0-9a-f]+:[ \t]+09 00[ \t]+nop[ \t]+

[0-9a-f]+ <bar@plt>:
 [0-9a-f]+:[ \t]+f0 00 f8 ff[ \t]+movi20[ \t]+#-8,r0
 [0-9a-f]+:[ \t]+ce 01[ \t]+mov\.l[ \t]+@\(r0,r12\),r1
 [0-9a-f]+:[ \t]+04 70[ \t]+add[ \t]+#4,r0
 [0-9a-f]+:[ \t]+2b 41[ \t]+jmp[ \t]+@r1
 [0-9a-f]+:[ \t]+ce 0c[ \t]+mov\.l[ \t]+@\(r0,r12\),r12
 [0-9a-f]+:[ \t]+0c 00[ \t]+mov\.b[ \t]+@\(r0,r0\),r0
 [0-9a-f]+:[ \t]+00 00 c2 60[ \t]+movi20[ \t]+#24770,r0
 [0-9a-f]+:[ \t]+2b 40[ \t]+jmp[ \t]+@r0
 [0-9a-f]+:[ \t]+c1 53[ \t]+mov\.l[ \t]+@\(4,r12\),r3
 [0-9a-f]+:[ \t]+09 00[ \t]+nop[ \t]+

Disassembly of section \.text:

[0-9a-f]+ <f>:
 [0-9a-f]+:[ \t]+00 d0[ \t]+mov\.l[ \t]+[0-9a-f]+ <f\+0x4>,r0[ \t]+! ffffffcc
 [0-9a-f]+:[ \t]+01 d1[ \t]+mov\.l[ \t]+[0-9a-f]+ <f\+0x8>,r1[ \t]+! ffffffe0
 [0-9a-f]+:[ \t]+cc ff[ \t]+fmov[ \t]+fr12,fr15
 [0-9a-f]+:[ \t]+ff ff[ \t]+\.word 0xffff
 [0-9a-f]+:[ \t]+e0 ff[ \t]+fadd[ \t]+fr14,fr15
 [0-9a-f]+:[ \t]+ff ff[ \t]+\.word 0xffff

Disassembly of section \.got:

[0-9a-f]+ <\.got>:
[ \t]+[0-9a-f]+:[ \t]+38 02[ \t]+\.word[ \t]+0x0238
[ \t]+[0-9a-f]+: R_SH_FUNCDESC_VALUE[ \t]+foo
[ \t]+[0-9a-f]+:[ \t]+00 00 00 00[ \t]+movi20[ \t]+#0,r0
[ \t]+[0-9a-f]+:[ \t]+00 00 50 02[ \t]+movi20[ \t]+#592,r0
[ \t]+[0-9a-f]+: R_SH_FUNCDESC_VALUE[ \t]+bar
[ \t]+\.\.\.
