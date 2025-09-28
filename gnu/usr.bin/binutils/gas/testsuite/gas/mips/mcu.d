#objdump: -dr --show-raw-insn
#name: MCU for MIPS32r2
#as: -32
#source: mcu.s

.*: +file format .*mips.*

Disassembly of section \.text:

[0-9a-f]+ <foo>:
[ 0-9a-f]+:	42000038 	iret
[ 0-9a-f]+:	04070000 	aclr	0x0,0\(zero\)
[ 0-9a-f]+:	04070000 	aclr	0x0,0\(zero\)
[ 0-9a-f]+:	04070000 	aclr	0x0,0\(zero\)
[ 0-9a-f]+:	04071000 	aclr	0x1,0\(zero\)
[ 0-9a-f]+:	04072000 	aclr	0x2,0\(zero\)
[ 0-9a-f]+:	04073000 	aclr	0x3,0\(zero\)
[ 0-9a-f]+:	04074000 	aclr	0x4,0\(zero\)
[ 0-9a-f]+:	04075000 	aclr	0x5,0\(zero\)
[ 0-9a-f]+:	04076000 	aclr	0x6,0\(zero\)
[ 0-9a-f]+:	04077000 	aclr	0x7,0\(zero\)
[ 0-9a-f]+:	04477000 	aclr	0x7,0\(v0\)
[ 0-9a-f]+:	07e77000 	aclr	0x7,0\(ra\)
[ 0-9a-f]+:	07e777ff 	aclr	0x7,2047\(ra\)
[ 0-9a-f]+:	07e77800 	aclr	0x7,-2048\(ra\)
[ 0-9a-f]+:	27e10800 	addiu	at,ra,2048
[ 0-9a-f]+:	04277000 	aclr	0x7,0\(at\)
[ 0-9a-f]+:	27e1f7ff 	addiu	at,ra,-2049
[ 0-9a-f]+:	04277000 	aclr	0x7,0\(at\)
[ 0-9a-f]+:	27e17fff 	addiu	at,ra,32767
[ 0-9a-f]+:	04277000 	aclr	0x7,0\(at\)
[ 0-9a-f]+:	27e18000 	addiu	at,ra,-32768
[ 0-9a-f]+:	04277000 	aclr	0x7,0\(at\)
[ 0-9a-f]+:	3c010001 	lui	at,0x1
[ 0-9a-f]+:	00240821 	addu	at,at,a0
[ 0-9a-f]+:	04277fff 	aclr	0x7,-1\(at\)
[ 0-9a-f]+:	3c010001 	lui	at,0x1
[ 0-9a-f]+:	00240821 	addu	at,at,a0
[ 0-9a-f]+:	04277000 	aclr	0x7,0\(at\)
[ 0-9a-f]+:	3c01ffff 	lui	at,0xffff
[ 0-9a-f]+:	00240821 	addu	at,at,a0
[ 0-9a-f]+:	04277000 	aclr	0x7,0\(at\)
[ 0-9a-f]+:	24818000 	addiu	at,a0,-32768
[ 0-9a-f]+:	04277000 	aclr	0x7,0\(at\)
[ 0-9a-f]+:	3c01ffff 	lui	at,0xffff
[ 0-9a-f]+:	00240821 	addu	at,at,a0
[ 0-9a-f]+:	04277001 	aclr	0x7,1\(at\)
[ 0-9a-f]+:	24818001 	addiu	at,a0,-32767
[ 0-9a-f]+:	04277000 	aclr	0x7,0\(at\)
[ 0-9a-f]+:	3c01f000 	lui	at,0xf000
[ 0-9a-f]+:	00240821 	addu	at,at,a0
[ 0-9a-f]+:	04277000 	aclr	0x7,0\(at\)
[ 0-9a-f]+:	04877fff 	aclr	0x7,-1\(a0\)
[ 0-9a-f]+:	3c011234 	lui	at,0x1234
[ 0-9a-f]+:	34215000 	ori	at,at,0x5000
[ 0-9a-f]+:	00240821 	addu	at,at,a0
[ 0-9a-f]+:	04277678 	aclr	0x7,1656\(at\)
[ 0-9a-f]+:	24610000 	addiu	at,v1,0
[ 	]*[0-9a-f]+: R_MIPS_LO16	foo
[ 0-9a-f]+:	04271000 	aclr	0x1,0\(at\)
[ 0-9a-f]+:	24610000 	addiu	at,v1,0
[ 	]*[0-9a-f]+: R_MIPS_LO16	foo
[ 0-9a-f]+:	04279000 	aset	0x1,0\(at\)
[ 0-9a-f]+:	04078000 	aset	0x0,0\(zero\)
[ 0-9a-f]+:	04078000 	aset	0x0,0\(zero\)
[ 0-9a-f]+:	04078000 	aset	0x0,0\(zero\)
[ 0-9a-f]+:	04079000 	aset	0x1,0\(zero\)
[ 0-9a-f]+:	0407a000 	aset	0x2,0\(zero\)
[ 0-9a-f]+:	0407b000 	aset	0x3,0\(zero\)
[ 0-9a-f]+:	0407c000 	aset	0x4,0\(zero\)
[ 0-9a-f]+:	0407d000 	aset	0x5,0\(zero\)
[ 0-9a-f]+:	0407e000 	aset	0x6,0\(zero\)
[ 0-9a-f]+:	0407f000 	aset	0x7,0\(zero\)
[ 0-9a-f]+:	0447f000 	aset	0x7,0\(v0\)
[ 0-9a-f]+:	07e7f000 	aset	0x7,0\(ra\)
[ 0-9a-f]+:	07e7f7ff 	aset	0x7,2047\(ra\)
[ 0-9a-f]+:	07e7f800 	aset	0x7,-2048\(ra\)
[ 0-9a-f]+:	27e10800 	addiu	at,ra,2048
[ 0-9a-f]+:	0427f000 	aset	0x7,0\(at\)
[ 0-9a-f]+:	27e1f7ff 	addiu	at,ra,-2049
[ 0-9a-f]+:	0427f000 	aset	0x7,0\(at\)
[ 0-9a-f]+:	27e17fff 	addiu	at,ra,32767
[ 0-9a-f]+:	0427f000 	aset	0x7,0\(at\)
[ 0-9a-f]+:	27e18000 	addiu	at,ra,-32768
[ 0-9a-f]+:	0427f000 	aset	0x7,0\(at\)
[ 0-9a-f]+:	3c010001 	lui	at,0x1
[ 0-9a-f]+:	00240821 	addu	at,at,a0
[ 0-9a-f]+:	0427ffff 	aset	0x7,-1\(at\)
[ 0-9a-f]+:	3c010001 	lui	at,0x1
[ 0-9a-f]+:	00240821 	addu	at,at,a0
[ 0-9a-f]+:	0427f000 	aset	0x7,0\(at\)
[ 0-9a-f]+:	3c01ffff 	lui	at,0xffff
[ 0-9a-f]+:	00240821 	addu	at,at,a0
[ 0-9a-f]+:	0427f000 	aset	0x7,0\(at\)
[ 0-9a-f]+:	24818000 	addiu	at,a0,-32768
[ 0-9a-f]+:	0427f000 	aset	0x7,0\(at\)
[ 0-9a-f]+:	3c01ffff 	lui	at,0xffff
[ 0-9a-f]+:	00240821 	addu	at,at,a0
[ 0-9a-f]+:	0427f001 	aset	0x7,1\(at\)
[ 0-9a-f]+:	24818001 	addiu	at,a0,-32767
[ 0-9a-f]+:	0427f000 	aset	0x7,0\(at\)
[ 0-9a-f]+:	3c01f000 	lui	at,0xf000
[ 0-9a-f]+:	00240821 	addu	at,at,a0
[ 0-9a-f]+:	0427f000 	aset	0x7,0\(at\)
[ 0-9a-f]+:	0487ffff 	aset	0x7,-1\(a0\)
[ 0-9a-f]+:	3c011234 	lui	at,0x1234
[ 0-9a-f]+:	34215000 	ori	at,at,0x5000
[ 0-9a-f]+:	00240821 	addu	at,at,a0
[ 0-9a-f]+:	0427f678 	aset	0x7,1656\(at\)
	\.\.\.
