#as: -mm9s12xg
#source: xgate-link.s
#ld: --relax -mm68hc12elf -defsym var1=0xfeed -defsym var2=0xdeaf -defsym var3=0xa1b2 -defsym var4=0x3456 -defsym var5=0xfa -defsym var6=0x20fe
#objdump: -d --prefix-addresses -r -mm9s12xg

tmpdir/dump:     file format elf32-m68hc12


Disassembly of section .text:
00008000 <_start> ldl R1, #0xed
00008002 <_start\+0x2> ldh R1, #0xfe
00008004 <_start\+0x4> addl R5, #0xaf
00008006 <_start\+0x6> addh R5, #0xde
00008008 <_start\+0x8> ldl R2, #0x56
0000800a <_start\+0xa> ldh R2, #0x34
0000800c <_start\+0xc> ldl R3, #0x21
0000800e <_start\+0xe> ldh R6, #0xfa
00008010 <_start\+0x10> cmpl R1, #0xcd
00008012 <_start\+0x12> cpch R1, #0xab
00008014 <_start\+0x14> cmpl R2, #0xb2
00008016 <_start\+0x16> cpch R2, #0xa1
00008018 <_start\+0x18> ldl R1, #0xfe
0000801a <_start\+0x1a> ldh R1, #0x20
0000801c <_start\+0x1c> ldl R2, #0x02
0000801e <_start\+0x1e> ldh R2, #0x22
