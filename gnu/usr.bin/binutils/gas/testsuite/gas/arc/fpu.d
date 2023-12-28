#as: -mcpu=archs
#objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arc.*


Disassembly of section .text:
0x[0-9a-f]+ 3208 0100           	fcvt32	r0,r2,r4
0x[0-9a-f]+ 3209 0100           	fcvt32_64	r0,r2,r4
0x[0-9a-f]+ 3238 0100           	fcvt64	r0,r2,r4
0x[0-9a-f]+ 3239 0100           	fcvt64_32	r0,r2,r4
0x[0-9a-f]+ 3231 0100           	fdadd	r0,r2,r4
0x[0-9a-f]+ 3033 8080           	fdcmp	r0,r2
0x[0-9a-f]+ 3034 8080           	fdcmpf	r0,r2
0x[0-9a-f]+ 3237 0100           	fddiv	r0,r2,r4
0x[0-9a-f]+ 3235 0100           	fdmadd	r0,r2,r4
0x[0-9a-f]+ 3236 0100           	fdmsub	r0,r2,r4
0x[0-9a-f]+ 3230 0100           	fdmul	r0,r2,r4
0x[0-9a-f]+ 302f 0081           	fdsqrt	r0,r2
0x[0-9a-f]+ 3232 0100           	fdsub	r0,r2,r4
0x[0-9a-f]+ 3201 0100           	fsadd	r0,r2,r4
0x[0-9a-f]+ 3003 8080           	fscmp	r0,r2
0x[0-9a-f]+ 3004 8080           	fscmpf	r0,r2
0x[0-9a-f]+ 3207 0100           	fsdiv	r0,r2,r4
0x[0-9a-f]+ 3205 0100           	fsmadd	r0,r2,r4
0x[0-9a-f]+ 3206 0100           	fsmsub	r0,r2,r4
0x[0-9a-f]+ 3200 0100           	fsmul	r0,r2,r4
0x[0-9a-f]+ 302f 0080           	fssqrt	r0,r2
0x[0-9a-f]+ 3202 0100           	fssub	r0,r2,r4
