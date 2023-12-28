# name: Neon logic insns with two and three operands including imm. values
# as: -mfpu=neon
# objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*


Disassembly of section \.text:
00000000 <.text> f387015f 	vorr.i32	q0, #255	@ 0x000000ff
00000004 <.text\+0x4> f387015f 	vorr.i32	q0, #255	@ 0x000000ff
00000008 <.text\+0x8> f2220154 	vorr	q0, q1, q2
0000000c <.text\+0xc> f2200152 	vorr	q0, q0, q1
00000010 <.text\+0x10> f387011f 	vorr.i32	d0, #255	@ 0x000000ff
00000014 <.text\+0x14> f387011f 	vorr.i32	d0, #255	@ 0x000000ff
00000018 <.text\+0x18> f2210112 	vorr	d0, d1, d2
0000001c <.text\+0x1c> f2200111 	vorr	d0, d0, d1
