#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS64r6 n64 instructions
#as: -64
#source: r6-64.s

# Check MIPSR6 64 instructions

.*: +file format .*mips.*

Disassembly of section .text:
0+0000 <[^>]*> 0064109c 	dmul	v0,v1,a0
0+0004 <[^>]*> 006410dc 	dmuh	v0,v1,a0
0+0008 <[^>]*> 0064109e 	ddiv	v0,v1,a0
0+000c <[^>]*> 0064109d 	dmulu	v0,v1,a0
0+0010 <[^>]*> 006410dd 	dmuhu	v0,v1,a0
0+0014 <[^>]*> 006410de 	dmod	v0,v1,a0
0+0018 <[^>]*> 0064109f 	ddivu	v0,v1,a0
0+001c <[^>]*> 006410df 	dmodu	v0,v1,a0
0+0020 <[^>]*> 00641015 	dlsa	v0,v1,a0,0x1
0+0024 <[^>]*> 006410d5 	dlsa	v0,v1,a0,0x4
0+0028 <[^>]*> 00601052 	dclz	v0,v1
0+002c <[^>]*> 00601053 	dclo	v0,v1
0+0030 <[^>]*> 7c628037 	lld	v0,-256\(v1\)
0+0034 <[^>]*> 7c627fb7 	lld	v0,255\(v1\)
0+0038 <[^>]*> 7c628027 	scd	v0,-256\(v1\)
0+003c <[^>]*> 7c627fa7 	scd	v0,255\(v1\)
0+0040 <[^>]*> 7c432224 	dalign	a0,v0,v1,0
0+0044 <[^>]*> 7c432264 	dalign	a0,v0,v1,1
0+0048 <[^>]*> 7c4322a4 	dalign	a0,v0,v1,2
0+004c <[^>]*> 7c4322e4 	dalign	a0,v0,v1,3
0+0050 <[^>]*> 7c432324 	dalign	a0,v0,v1,4
0+0054 <[^>]*> 7c432364 	dalign	a0,v0,v1,5
0+0058 <[^>]*> 7c4323a4 	dalign	a0,v0,v1,6
0+005c <[^>]*> 7c4323e4 	dalign	a0,v0,v1,7
0+0060 <[^>]*> 7c022024 	dbitswap	a0,v0
0+0064 <[^>]*> 7443ffff 	daui	v1,v0,0xffff
0+0068 <[^>]*> 0466ffff 	dahi	v1,v1,0xffff
0+006c <[^>]*> 047effff 	dati	v1,v1,0xffff
0+0070 <[^>]*> ec900000 	lwupc	a0,0+0000070 <[^>]*>
[	]*70: R_MIPS_PC19_S2	\.L1.*1
[	]*70: R_MIPS_NONE	\*ABS\*
[	]*70: R_MIPS_NONE	\*ABS\*
0+0074 <[^>]*> ec900000 	lwupc	a0,0+0000074 <[^>]*>
[	]*74: R_MIPS_PC19_S2	L0.*-0x100000
[	]*74: R_MIPS_NONE	\*ABS\*-0x100000
[	]*74: R_MIPS_NONE	\*ABS\*-0x100000
0+0078 <[^>]*> ec900000 	lwupc	a0,0+0000078 <[^>]*>
[	]*78: R_MIPS_PC19_S2	L0.*\+0xffffc
[	]*78: R_MIPS_NONE	\*ABS\*\+0xffffc
[	]*78: R_MIPS_NONE	\*ABS\*\+0xffffc
0+007c <[^>]*> ec940000 	lwupc	a0,f+ff0007c <[^>]*>
0+0080 <[^>]*> ec93ffff 	lwupc	a0,0+010007c <[^>]*>
0+0084 <[^>]*> ec980000 	ldpc	a0,0+0000080 <[^>]*>
[	]*84: R_MIPS_PC18_S3	.L1.*1
[	]*84: R_MIPS_NONE	\*ABS\*
[	]*84: R_MIPS_NONE	\*ABS\*
0+0088 <[^>]*> ec980000 	ldpc	a0,0+0000088 <[^>]*>
[	]*88: R_MIPS_PC18_S3	.L1.*1
[	]*88: R_MIPS_NONE	\*ABS\*
[	]*88: R_MIPS_NONE	\*ABS\*
0+008c <[^>]*> 00000000 	nop
0+0090 <[^>]*> ec980000 	ldpc	a0,0+0000090 <[^>]*>
[	]*90: R_MIPS_PC18_S3	.L3.*1-0x100000
[	]*90: R_MIPS_NONE	\*ABS\*-0x100000
[	]*90: R_MIPS_NONE	\*ABS\*-0x100000
0+0094 <[^>]*> ec980000 	ldpc	a0,0+0000090 <[^>]*>
[	]*94: R_MIPS_PC18_S3	.L3.*1-0x100000
[	]*94: R_MIPS_NONE	\*ABS\*-0x100000
[	]*94: R_MIPS_NONE	\*ABS\*-0x100000
0+0098 <[^>]*> ec980000 	ldpc	a0,0+0000098 <[^>]*>
[	]*98: R_MIPS_PC18_S3	.L3.*2\+0xffff8
[	]*98: R_MIPS_NONE	\*ABS\*\+0xffff8
[	]*98: R_MIPS_NONE	\*ABS\*\+0xffff8
0+009c <[^>]*> ec980000 	ldpc	a0,0+0000098 <[^>]*>
[	]*9c: R_MIPS_PC18_S3	.L3.*2\+0xffff8
[	]*9c: R_MIPS_NONE	\*ABS\*\+0xffff8
[	]*9c: R_MIPS_NONE	\*ABS\*\+0xffff8
0+00a0 <[^>]*> ec9a0000 	ldpc	a0,f+ff000a0 <[^>]*>
0+00a4 <[^>]*> ec9a0000 	ldpc	a0,f+ff000a0 <[^>]*>
0+00a8 <[^>]*> ec99ffff 	ldpc	a0,0+01000a0 <[^>]*>
0+00ac <[^>]*> ec99ffff 	ldpc	a0,0+01000a0 <[^>]*>
0+00b0 <[^>]*> 7cc52077 	lldp	a1,a0,a2
0+00b4 <[^>]*> 7cc52067 	scdp	a1,a0,a2
	\.\.\.
