#as: -mm9s12xg
#source: xgate1.s
#source: xgate2.s
#ld: --relax -mm68hc12elf
#objdump: -d --prefix-addresses -r -mm9s12xg

tmpdir/dump:     file format elf32-m68hc12


Disassembly of section .text:
00008000 <_start> ldl R1, \#0x00
00008002 <_start\+0x2> ldh R1, \#0x11
00008004 <_start\+0x4> sub R0, R1, R0
00008006 <_start\+0x6> beq 0x0+8010 <linked_ad1>
00008008 <_start\+0x8> sub R0, R2, R0
0000800a <_start\+0xa> beq 0x0+800e <the_end>
0000800c <_start\+0xc> bra 0x0+8018 <linked_ad2>
0000800e <the_end> rts
00008010 <linked_ad1> cmpl R4, \#0x01
00008012 <linked_ad1\+0x2> bne 0x0+8018 <linked_ad2>
00008014 <label1> nop
00008016 <label1\+0x2> par R5
00008018 <linked_ad2> csem \#0x2
0000801a <linked_ad2\+0x2> rts
