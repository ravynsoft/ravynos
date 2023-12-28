#source: fdpic-plt.s
#as: --isa=sh2a -big --fdpic
#ld: -EB -mshelf_fd -shared
#objdump: -dsR -j.plt -j.text -j.got
#target: sh*-*-uclinux*

.*:[ \t]+file format elf32-shbig-fdpic

Contents of section \.plt:
 [0-9a-f]+ 00f0fff0 01ce7004 412b0cce 00000000[ \t]+.*
 [0-9a-f]+ 60c2402b 53c10009 00f0fff8 01ce7004[ \t]+.*
 [0-9a-f]+ 412b0cce 0000000c 60c2402b 53c10009[ \t]+.*
Contents of section \.text:
 [0-9a-f]+ d000d101 ffffffcc ffffffe0[ \t]+.*
Contents of section \.got:
 [0-9a-f]+ 00000238 00000000 00000250 00000000[ \t]+.*
 [0-9a-f]+ 00000000 00000000 00000000[ \t]+.*

Disassembly of section \.plt:

[0-9a-f]+ <foo@plt>:
 [0-9a-f]+:[ \t]+00 f0 ff f0[ \t]+movi20[ \t]+#-16,r0
 [0-9a-f]+:[ \t]+01 ce[ \t]+mov\.l[ \t]+@\(r0,r12\),r1
 [0-9a-f]+:[ \t]+70 04[ \t]+add[ \t]+#4,r0
 [0-9a-f]+:[ \t]+41 2b[ \t]+jmp[ \t]+@r1
 [0-9a-f]+:[ \t]+0c ce[ \t]+mov\.l[ \t]+@\(r0,r12\),r12
 [0-9a-f]+:[ \t]+00 00 00 00[ \t]+movi20[ \t]+#0,r0
 [0-9a-f]+:[ \t]+60 c2[ \t]+mov\.l[ \t]+@r12,r0
 [0-9a-f]+:[ \t]+40 2b[ \t]+jmp[ \t]+@r0
 [0-9a-f]+:[ \t]+53 c1[ \t]+mov\.l[ \t]+@\(4,r12\),r3
 [0-9a-f]+:[ \t]+00 09[ \t]+nop[ \t]+

[0-9a-f]+ <bar@plt>:
 [0-9a-f]+:[ \t]+00 f0 ff f8[ \t]+movi20[ \t]+#-8,r0
 [0-9a-f]+:[ \t]+01 ce[ \t]+mov\.l[ \t]+@\(r0,r12\),r1
 [0-9a-f]+:[ \t]+70 04[ \t]+add[ \t]+#4,r0
 [0-9a-f]+:[ \t]+41 2b[ \t]+jmp[ \t]+@r1
 [0-9a-f]+:[ \t]+0c ce[ \t]+mov\.l[ \t]+@\(r0,r12\),r12
 [0-9a-f]+:[ \t]+00 00 00 0c[ \t]+movi20[ \t]+#12,r0
 [0-9a-f]+:[ \t]+60 c2[ \t]+mov\.l[ \t]+@r12,r0
 [0-9a-f]+:[ \t]+40 2b[ \t]+jmp[ \t]+@r0
 [0-9a-f]+:[ \t]+53 c1[ \t]+mov\.l[ \t]+@\(4,r12\),r3
 [0-9a-f]+:[ \t]+00 09[ \t]+nop[ \t]+

Disassembly of section \.text:

[0-9a-f]+ <f>:
 [0-9a-f]+:[ \t]+d0 00[ \t]+mov\.l[ \t]+[0-9a-f]+ <f\+0x4>,r0[ \t]+! ffffffcc
 [0-9a-f]+:[ \t]+d1 01[ \t]+mov\.l[ \t]+[0-9a-f]+ <f\+0x8>,r1[ \t]+! ffffffe0
 [0-9a-f]+:[ \t]+ff ff[ \t]+\.word 0xffff
 [0-9a-f]+:[ \t]+ff cc[ \t]+fmov[ \t]+fr12,fr15
 [0-9a-f]+:[ \t]+ff ff[ \t]+\.word 0xffff
 [0-9a-f]+:[ \t]+ff e0[ \t]+fadd[ \t]+fr14,fr15

Disassembly of section \.got:

[0-9a-f]+ <\.got>:
[ \t]+[0-9a-f]+:[ \t]+00 00 02 38[ \t]+movi20[ \t]+#568,r0
[ \t]+[0-9a-f]+: R_SH_FUNCDESC_VALUE[ \t]+foo
[ \t]+[0-9a-f]+:[ \t]+00 00 00 00[ \t]+movi20[ \t]+#0,r0
[ \t]+[0-9a-f]+:[ \t]+00 00 02 50[ \t]+movi20[ \t]+#592,r0
[ \t]+[0-9a-f]+: R_SH_FUNCDESC_VALUE[ \t]+bar
[ \t]+\.\.\.
