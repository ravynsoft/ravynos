#objdump: -dr --show-raw-insn
#name: MCU for MIPS32r2
#as: -32
#source: mcu.s

.*: +file format .*mips.*

Disassembly of section \.text:

[0-9a-f]+ <foo>:
[ 0-9a-f]+:	0000 d37c 	iret
[ 0-9a-f]+:	2000 b000 	aclr	0x0,0\(zero\)
[ 0-9a-f]+:	2000 b000 	aclr	0x0,0\(zero\)
[ 0-9a-f]+:	2000 b000 	aclr	0x0,0\(zero\)
[ 0-9a-f]+:	2020 b000 	aclr	0x1,0\(zero\)
[ 0-9a-f]+:	2040 b000 	aclr	0x2,0\(zero\)
[ 0-9a-f]+:	2060 b000 	aclr	0x3,0\(zero\)
[ 0-9a-f]+:	2080 b000 	aclr	0x4,0\(zero\)
[ 0-9a-f]+:	20a0 b000 	aclr	0x5,0\(zero\)
[ 0-9a-f]+:	20c0 b000 	aclr	0x6,0\(zero\)
[ 0-9a-f]+:	20e0 b000 	aclr	0x7,0\(zero\)
[ 0-9a-f]+:	20e2 b000 	aclr	0x7,0\(v0\)
[ 0-9a-f]+:	20ff b000 	aclr	0x7,0\(ra\)
[ 0-9a-f]+:	20ff b7ff 	aclr	0x7,2047\(ra\)
[ 0-9a-f]+:	20ff b800 	aclr	0x7,-2048\(ra\)
[ 0-9a-f]+:	303f 0800 	addiu	at,ra,2048
[ 0-9a-f]+:	20e1 b000 	aclr	0x7,0\(at\)
[ 0-9a-f]+:	303f f7ff 	addiu	at,ra,-2049
[ 0-9a-f]+:	20e1 b000 	aclr	0x7,0\(at\)
[ 0-9a-f]+:	303f 7fff 	addiu	at,ra,32767
[ 0-9a-f]+:	20e1 b000 	aclr	0x7,0\(at\)
[ 0-9a-f]+:	303f 8000 	addiu	at,ra,-32768
[ 0-9a-f]+:	20e1 b000 	aclr	0x7,0\(at\)
[ 0-9a-f]+:	41a1 0001 	lui	at,0x1
[ 0-9a-f]+:	0081 0950 	addu	at,at,a0
[ 0-9a-f]+:	20e1 bfff 	aclr	0x7,-1\(at\)
[ 0-9a-f]+:	41a1 0001 	lui	at,0x1
[ 0-9a-f]+:	0081 0950 	addu	at,at,a0
[ 0-9a-f]+:	20e1 b000 	aclr	0x7,0\(at\)
[ 0-9a-f]+:	41a1 ffff 	lui	at,0xffff
[ 0-9a-f]+:	0081 0950 	addu	at,at,a0
[ 0-9a-f]+:	20e1 b000 	aclr	0x7,0\(at\)
[ 0-9a-f]+:	3024 8000 	addiu	at,a0,-32768
[ 0-9a-f]+:	20e1 b000 	aclr	0x7,0\(at\)
[ 0-9a-f]+:	41a1 ffff 	lui	at,0xffff
[ 0-9a-f]+:	0081 0950 	addu	at,at,a0
[ 0-9a-f]+:	20e1 b001 	aclr	0x7,1\(at\)
[ 0-9a-f]+:	3024 8001 	addiu	at,a0,-32767
[ 0-9a-f]+:	20e1 b000 	aclr	0x7,0\(at\)
[ 0-9a-f]+:	41a1 f000 	lui	at,0xf000
[ 0-9a-f]+:	0081 0950 	addu	at,at,a0
[ 0-9a-f]+:	20e1 b000 	aclr	0x7,0\(at\)
[ 0-9a-f]+:	20e4 bfff 	aclr	0x7,-1\(a0\)
[ 0-9a-f]+:	41a1 1234 	lui	at,0x1234
[ 0-9a-f]+:	5021 5000 	ori	at,at,0x5000
[ 0-9a-f]+:	0081 0950 	addu	at,at,a0
[ 0-9a-f]+:	20e1 b678 	aclr	0x7,1656\(at\)
[ 0-9a-f]+:	3023 0000 	addiu	at,v1,0
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	foo
[ 0-9a-f]+:	2021 b000 	aclr	0x1,0\(at\)
[ 0-9a-f]+:	3023 0000 	addiu	at,v1,0
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	foo
[ 0-9a-f]+:	2021 3000 	aset	0x1,0\(at\)
[ 0-9a-f]+:	2000 3000 	aset	0x0,0\(zero\)
[ 0-9a-f]+:	2000 3000 	aset	0x0,0\(zero\)
[ 0-9a-f]+:	2000 3000 	aset	0x0,0\(zero\)
[ 0-9a-f]+:	2020 3000 	aset	0x1,0\(zero\)
[ 0-9a-f]+:	2040 3000 	aset	0x2,0\(zero\)
[ 0-9a-f]+:	2060 3000 	aset	0x3,0\(zero\)
[ 0-9a-f]+:	2080 3000 	aset	0x4,0\(zero\)
[ 0-9a-f]+:	20a0 3000 	aset	0x5,0\(zero\)
[ 0-9a-f]+:	20c0 3000 	aset	0x6,0\(zero\)
[ 0-9a-f]+:	20e0 3000 	aset	0x7,0\(zero\)
[ 0-9a-f]+:	20e2 3000 	aset	0x7,0\(v0\)
[ 0-9a-f]+:	20ff 3000 	aset	0x7,0\(ra\)
[ 0-9a-f]+:	20ff 37ff 	aset	0x7,2047\(ra\)
[ 0-9a-f]+:	20ff 3800 	aset	0x7,-2048\(ra\)
[ 0-9a-f]+:	303f 0800 	addiu	at,ra,2048
[ 0-9a-f]+:	20e1 3000 	aset	0x7,0\(at\)
[ 0-9a-f]+:	303f f7ff 	addiu	at,ra,-2049
[ 0-9a-f]+:	20e1 3000 	aset	0x7,0\(at\)
[ 0-9a-f]+:	303f 7fff 	addiu	at,ra,32767
[ 0-9a-f]+:	20e1 3000 	aset	0x7,0\(at\)
[ 0-9a-f]+:	303f 8000 	addiu	at,ra,-32768
[ 0-9a-f]+:	20e1 3000 	aset	0x7,0\(at\)
[ 0-9a-f]+:	41a1 0001 	lui	at,0x1
[ 0-9a-f]+:	0081 0950 	addu	at,at,a0
[ 0-9a-f]+:	20e1 3fff 	aset	0x7,-1\(at\)
[ 0-9a-f]+:	41a1 0001 	lui	at,0x1
[ 0-9a-f]+:	0081 0950 	addu	at,at,a0
[ 0-9a-f]+:	20e1 3000 	aset	0x7,0\(at\)
[ 0-9a-f]+:	41a1 ffff 	lui	at,0xffff
[ 0-9a-f]+:	0081 0950 	addu	at,at,a0
[ 0-9a-f]+:	20e1 3000 	aset	0x7,0\(at\)
[ 0-9a-f]+:	3024 8000 	addiu	at,a0,-32768
[ 0-9a-f]+:	20e1 3000 	aset	0x7,0\(at\)
[ 0-9a-f]+:	41a1 ffff 	lui	at,0xffff
[ 0-9a-f]+:	0081 0950 	addu	at,at,a0
[ 0-9a-f]+:	20e1 3001 	aset	0x7,1\(at\)
[ 0-9a-f]+:	3024 8001 	addiu	at,a0,-32767
[ 0-9a-f]+:	20e1 3000 	aset	0x7,0\(at\)
[ 0-9a-f]+:	41a1 f000 	lui	at,0xf000
[ 0-9a-f]+:	0081 0950 	addu	at,at,a0
[ 0-9a-f]+:	20e1 3000 	aset	0x7,0\(at\)
[ 0-9a-f]+:	20e4 3fff 	aset	0x7,-1\(a0\)
[ 0-9a-f]+:	41a1 1234 	lui	at,0x1234
[ 0-9a-f]+:	5021 5000 	ori	at,at,0x5000
[ 0-9a-f]+:	0081 0950 	addu	at,at,a0
[ 0-9a-f]+:	20e1 3678 	aset	0x7,1656\(at\)
	\.\.\.
