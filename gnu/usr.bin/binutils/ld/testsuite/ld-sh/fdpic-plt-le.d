#source: fdpic-plt.s
#as: --isa=sh4a -little --fdpic
#ld: -EL -mshlelf_fd -shared
#objdump: -dsR -j.plt -j.text -j.got
#target: sh*-*-uclinux*

.*:[ \t]+file format elf32-sh-fdpic

Contents of section \.plt:
 [0-9a-f]+ 02d0ce01 04702b41 ce0c0900 f0ffffff[ \t]+.*
 [0-9a-f]+ 00000000 c2602b40 c1530900 02d0ce01[ \t]+.*
 [0-9a-f]+ 04702b41 ce0c0900 f8ffffff 0c000000[ \t]+.*
 [0-9a-f]+ c2602b40 c1530900[ \t]+.*
Contents of section \.text:
 [0-9a-f]+ 00d001d1 c4ffffff dcffffff[ \t]+.*
Contents of section \.got:
 [0-9a-f]+ 3c020000 00000000 58020000 00000000[ \t]+.*
 [0-9a-f]+ 00000000 00000000 00000000[ \t]+.*

Disassembly of section \.plt:

[0-9a-f]+ <foo@plt>:
 [0-9a-f]+:[ \t]+02 d0[ \t]+mov\.l[ \t]+[0-9a-f]+ <foo@plt\+0xc>,r0[ \t]+! fffffff0
 [0-9a-f]+:[ \t]+ce 01[ \t]+mov\.l[ \t]+@\(r0,r12\),r1
 [0-9a-f]+:[ \t]+04 70[ \t]+add[ \t]+#4,r0
 [0-9a-f]+:[ \t]+2b 41[ \t]+jmp[ \t]+@r1
 [0-9a-f]+:[ \t]+ce 0c[ \t]+mov\.l[ \t]+@\(r0,r12\),r12
 [0-9a-f]+:[ \t]+09 00[ \t]+nop[ \t]+
 [0-9a-f]+:[ \t]+f0 ff[ \t]+fadd[ \t]+fr15,fr15
 [0-9a-f]+:[ \t]+ff ff[ \t]+\.word 0xffff
 [0-9a-f]+:[ \t]+00 00[ \t]+\.word 0x0000
 [0-9a-f]+:[ \t]+00 00[ \t]+\.word 0x0000
 [0-9a-f]+:[ \t]+c2 60[ \t]+mov\.l[ \t]+@r12,r0
 [0-9a-f]+:[ \t]+2b 40[ \t]+jmp[ \t]+@r0
 [0-9a-f]+:[ \t]+c1 53[ \t]+mov\.l[ \t]+@\(4,r12\),r3
 [0-9a-f]+:[ \t]+09 00[ \t]+nop[ \t]+

[0-9a-f]+ <bar@plt>:
 [0-9a-f]+:[ \t]+02 d0[ \t]+mov\.l[ \t]+[0-9a-f]+ <bar@plt\+0xc>,r0[ \t]+! fffffff8
 [0-9a-f]+:[ \t]+ce 01[ \t]+mov\.l[ \t]+@\(r0,r12\),r1
 [0-9a-f]+:[ \t]+04 70[ \t]+add[ \t]+#4,r0
 [0-9a-f]+:[ \t]+2b 41[ \t]+jmp[ \t]+@r1
 [0-9a-f]+:[ \t]+ce 0c[ \t]+mov\.l[ \t]+@\(r0,r12\),r12
 [0-9a-f]+:[ \t]+09 00[ \t]+nop[ \t]+
 [0-9a-f]+:[ \t]+f8 ff[ \t]+fmov[ \t]+@r15,fr15
 [0-9a-f]+:[ \t]+ff ff[ \t]+\.word 0xffff
 [0-9a-f]+:[ \t]+0c 00[ \t]+mov\.b[ \t]+@\(r0,r0\),r0
 [0-9a-f]+:[ \t]+00 00[ \t]+\.word 0x0000
 [0-9a-f]+:[ \t]+c2 60[ \t]+mov\.l[ \t]+@r12,r0
 [0-9a-f]+:[ \t]+2b 40[ \t]+jmp[ \t]+@r0
 [0-9a-f]+:[ \t]+c1 53[ \t]+mov\.l[ \t]+@\(4,r12\),r3
 [0-9a-f]+:[ \t]+09 00[ \t]+nop[ \t]+

Disassembly of section \.text:

[0-9a-f]+ <f>:
 [0-9a-f]+:[ \t]+00 d0[ \t]+mov\.l[ \t]+[0-9a-f]+ <f\+0x4>,r0[ \t]+! ffffffc4
 [0-9a-f]+:[ \t]+01 d1[ \t]+mov\.l[ \t]+[0-9a-f]+ <f\+0x8>,r1[ \t]+! ffffffdc
 [0-9a-f]+:[ \t]+c4 ff[ \t]+fcmp/eq[ \t]+fr12,fr15
 [0-9a-f]+:[ \t]+ff ff[ \t]+\.word 0xffff
 [0-9a-f]+:[ \t]+dc ff[ \t]+fmov[ \t]+fr13,fr15
 [0-9a-f]+:[ \t]+ff ff[ \t]+\.word 0xffff

Disassembly of section \.got:

[0-9a-f]+ <\.got>:
[ \t]+[0-9a-f]+:[ \t]+3c 02[ \t]+mov.b[ \t]+@\(r0,r3\),r2
[ \t]+[0-9a-f]+: R_SH_FUNCDESC_VALUE[ \t]+foo
[ \t]+[0-9a-f]+:[ \t]+00 00[ \t]+\.word 0x0000
[ \t]+[0-9a-f]+:[ \t]+00 00[ \t]+\.word 0x0000
[ \t]+[0-9a-f]+:[ \t]+00 00[ \t]+\.word 0x0000
[ \t]+[0-9a-f]+:[ \t]+58 02[ \t]+\.word 0x0258
[ \t]+[0-9a-f]+: R_SH_FUNCDESC_VALUE[ \t]+bar
[ \t]+\.\.\.
