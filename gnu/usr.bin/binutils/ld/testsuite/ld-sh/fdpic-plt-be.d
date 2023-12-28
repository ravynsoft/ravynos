#source: fdpic-plt.s
#as: --isa=sh4a -big --fdpic
#ld: -EB -mshelf_fd -shared
#objdump: -dsR -j.plt -j.text -j.got
#target: sh*-*-uclinux*

.*:[ \t]+file format elf32-shbig-fdpic

Contents of section \.plt:
 [0-9a-f]+ d00201ce 7004412b 0cce0009 fffffff0[ \t]+.*
 [0-9a-f]+ 00000000 60c2402b 53c10009 d00201ce[ \t]+.*
 [0-9a-f]+ 7004412b 0cce0009 fffffff8 0000000c[ \t]+.*
 [0-9a-f]+ 60c2402b 53c10009[ \t]+.*
Contents of section \.text:
 [0-9a-f]+ d000d101 ffffffc4 ffffffdc[ \t]+.*
Contents of section \.got:
 [0-9a-f]+ 0000023c 00000000 00000258 00000000[ \t]+.*
 [0-9a-f]+ 00000000 00000000 00000000[ \t]+.*

Disassembly of section \.plt:

[0-9a-f]+ <foo@plt>:
 [0-9a-f]+:[ \t]+d0 02[ \t]+mov\.l[ \t]+[0-9a-f]+ <foo@plt\+0xc>,r0[ \t]+! fffffff0
 [0-9a-f]+:[ \t]+01 ce[ \t]+mov\.l[ \t]+@\(r0,r12\),r1
 [0-9a-f]+:[ \t]+70 04[ \t]+add[ \t]+#4,r0
 [0-9a-f]+:[ \t]+41 2b[ \t]+jmp[ \t]+@r1
 [0-9a-f]+:[ \t]+0c ce[ \t]+mov\.l[ \t]+@\(r0,r12\),r12
 [0-9a-f]+:[ \t]+00 09[ \t]+nop[ \t]+
 [0-9a-f]+:[ \t]+ff ff[ \t]+\.word 0xffff
 [0-9a-f]+:[ \t]+ff f0[ \t]+fadd[ \t]+fr15,fr15
 [0-9a-f]+:[ \t]+00 00[ \t]+\.word 0x0000
 [0-9a-f]+:[ \t]+00 00[ \t]+\.word 0x0000
 [0-9a-f]+:[ \t]+60 c2[ \t]+mov\.l[ \t]+@r12,r0
 [0-9a-f]+:[ \t]+40 2b[ \t]+jmp[ \t]+@r0
 [0-9a-f]+:[ \t]+53 c1[ \t]+mov\.l[ \t]+@\(4,r12\),r3
 [0-9a-f]+:[ \t]+00 09[ \t]+nop[ \t]+

[0-9a-f]+ <bar@plt>:
 [0-9a-f]+:[ \t]+d0 02[ \t]+mov\.l[ \t]+[0-9a-f]+ <bar@plt\+0xc>,r0[ \t]+! fffffff8
 [0-9a-f]+:[ \t]+01 ce[ \t]+mov\.l[ \t]+@\(r0,r12\),r1
 [0-9a-f]+:[ \t]+70 04[ \t]+add[ \t]+#4,r0
 [0-9a-f]+:[ \t]+41 2b[ \t]+jmp[ \t]+@r1
 [0-9a-f]+:[ \t]+0c ce[ \t]+mov\.l[ \t]+@\(r0,r12\),r12
 [0-9a-f]+:[ \t]+00 09[ \t]+nop[ \t]+
 [0-9a-f]+:[ \t]+ff ff[ \t]+\.word 0xffff
 [0-9a-f]+:[ \t]+ff f8[ \t]+fmov[ \t]+@r15,fr15
 [0-9a-f]+:[ \t]+00 00[ \t]+\.word 0x0000
 [0-9a-f]+:[ \t]+00 0c[ \t]+mov\.b[ \t]+@\(r0,r0\),r0
 [0-9a-f]+:[ \t]+60 c2[ \t]+mov\.l[ \t]+@r12,r0
 [0-9a-f]+:[ \t]+40 2b[ \t]+jmp[ \t]+@r0
 [0-9a-f]+:[ \t]+53 c1[ \t]+mov\.l[ \t]+@\(4,r12\),r3
 [0-9a-f]+:[ \t]+00 09[ \t]+nop[ \t]+

Disassembly of section \.text:

[0-9a-f]+ <f>:
 [0-9a-f]+:[ \t]+d0 00[ \t]+mov\.l[ \t]+[0-9a-f]+ <f\+0x4>,r0[ \t]+! ffffffc4
 [0-9a-f]+:[ \t]+d1 01[ \t]+mov\.l[ \t]+[0-9a-f]+ <f\+0x8>,r1[ \t]+! ffffffdc
 [0-9a-f]+:[ \t]+ff ff[ \t]+\.word 0xffff
 [0-9a-f]+:[ \t]+ff c4[ \t]+fcmp/eq[ \t]+fr12,fr15
 [0-9a-f]+:[ \t]+ff ff[ \t]+\.word 0xffff
 [0-9a-f]+:[ \t]+ff dc[ \t]+fmov[ \t]+fr13,fr15

Disassembly of section \.got:

[0-9a-f]+ <\.got>:
[ \t]+[0-9a-f]+:[ \t]+00 00[ \t]+\.word 0x0000
[ \t]+[0-9a-f]+: R_SH_FUNCDESC_VALUE[ \t]+foo
[ \t]+[0-9a-f]+:[ \t]+02 3c[ \t]+mov.b[ \t]+@\(r0,r3\),r2
[ \t]+[0-9a-f]+:[ \t]+00 00[ \t]+\.word 0x0000
[ \t]+[0-9a-f]+:[ \t]+00 00[ \t]+\.word 0x0000
[ \t]+[0-9a-f]+:[ \t]+00 00[ \t]+\.word 0x0000
[ \t]+[0-9a-f]+: R_SH_FUNCDESC_VALUE[ \t]+bar
[ \t]+[0-9a-f]+:[ \t]+02 58[ \t]+\.word 0x0258
[ \t]+\.\.\.
