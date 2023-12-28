#objdump: -dr --show-raw-insn
#name: MCU for MIPS32r2
#as: -32
#source: mcu.s

.*: +file format .*mips.*

Disassembly of section \.text:

[0-9a-f]+ <foo>:
[ 0-9a-f]+:	42000038 	c0	0x38
[ 0-9a-f]+:	04070000 	.word	0x4070000
[ 0-9a-f]+:	04070000 	.word	0x4070000
[ 0-9a-f]+:	04070000 	.word	0x4070000
[ 0-9a-f]+:	04071000 	.word	0x4071000
[ 0-9a-f]+:	04072000 	.word	0x4072000
[ 0-9a-f]+:	04073000 	.word	0x4073000
[ 0-9a-f]+:	04074000 	.word	0x4074000
[ 0-9a-f]+:	04075000 	.word	0x4075000
[ 0-9a-f]+:	04076000 	.word	0x4076000
[ 0-9a-f]+:	04077000 	.word	0x4077000
[ 0-9a-f]+:	04477000 	.word	0x4477000
[ 0-9a-f]+:	07e77000 	.word	0x7e77000
[ 0-9a-f]+:	07e777ff 	.word	0x7e777ff
[ 0-9a-f]+:	07e77800 	.word	0x7e77800
[ 0-9a-f]+:	27e10800 	addiu	at,ra,2048
[ 0-9a-f]+:	04277000 	.word	0x4277000
[ 0-9a-f]+:	27e1f7ff 	addiu	at,ra,-2049
[ 0-9a-f]+:	04277000 	.word	0x4277000
[ 0-9a-f]+:	27e17fff 	addiu	at,ra,32767
[ 0-9a-f]+:	04277000 	.word	0x4277000
[ 0-9a-f]+:	27e18000 	addiu	at,ra,-32768
[ 0-9a-f]+:	04277000 	.word	0x4277000
[ 0-9a-f]+:	3c010001 	lui	at,0x1
[ 0-9a-f]+:	00240821 	addu	at,at,a0
[ 0-9a-f]+:	04277fff 	.word	0x4277fff
[ 0-9a-f]+:	3c010001 	lui	at,0x1
[ 0-9a-f]+:	00240821 	addu	at,at,a0
[ 0-9a-f]+:	04277000 	.word	0x4277000
[ 0-9a-f]+:	3c01ffff 	lui	at,0xffff
[ 0-9a-f]+:	00240821 	addu	at,at,a0
[ 0-9a-f]+:	04277000 	.word	0x4277000
[ 0-9a-f]+:	24818000 	addiu	at,a0,-32768
[ 0-9a-f]+:	04277000 	.word	0x4277000
[ 0-9a-f]+:	3c01ffff 	lui	at,0xffff
[ 0-9a-f]+:	00240821 	addu	at,at,a0
[ 0-9a-f]+:	04277001 	.word	0x4277001
[ 0-9a-f]+:	24818001 	addiu	at,a0,-32767
[ 0-9a-f]+:	04277000 	.word	0x4277000
[ 0-9a-f]+:	3c01f000 	lui	at,0xf000
[ 0-9a-f]+:	00240821 	addu	at,at,a0
[ 0-9a-f]+:	04277000 	.word	0x4277000
[ 0-9a-f]+:	04877fff 	.word	0x4877fff
[ 0-9a-f]+:	3c011234 	lui	at,0x1234
[ 0-9a-f]+:	34215000 	ori	at,at,0x5000
[ 0-9a-f]+:	00240821 	addu	at,at,a0
[ 0-9a-f]+:	04277678 	.word	0x4277678
[ 0-9a-f]+:	24610000 	addiu	at,v1,0
[ 	]*[0-9a-f]+: R_MIPS_LO16	foo
[ 0-9a-f]+:	04271000 	.word	0x4271000
[ 0-9a-f]+:	24610000 	addiu	at,v1,0
[ 	]*[0-9a-f]+: R_MIPS_LO16	foo
[ 0-9a-f]+:	04279000 	.word	0x4279000
[ 0-9a-f]+:	04078000 	.word	0x4078000
[ 0-9a-f]+:	04078000 	.word	0x4078000
[ 0-9a-f]+:	04078000 	.word	0x4078000
[ 0-9a-f]+:	04079000 	.word	0x4079000
[ 0-9a-f]+:	0407a000 	.word	0x407a000
[ 0-9a-f]+:	0407b000 	.word	0x407b000
[ 0-9a-f]+:	0407c000 	.word	0x407c000
[ 0-9a-f]+:	0407d000 	.word	0x407d000
[ 0-9a-f]+:	0407e000 	.word	0x407e000
[ 0-9a-f]+:	0407f000 	.word	0x407f000
[ 0-9a-f]+:	0447f000 	.word	0x447f000
[ 0-9a-f]+:	07e7f000 	.word	0x7e7f000
[ 0-9a-f]+:	07e7f7ff 	.word	0x7e7f7ff
[ 0-9a-f]+:	07e7f800 	.word	0x7e7f800
[ 0-9a-f]+:	27e10800 	addiu	at,ra,2048
[ 0-9a-f]+:	0427f000 	.word	0x427f000
[ 0-9a-f]+:	27e1f7ff 	addiu	at,ra,-2049
[ 0-9a-f]+:	0427f000 	.word	0x427f000
[ 0-9a-f]+:	27e17fff 	addiu	at,ra,32767
[ 0-9a-f]+:	0427f000 	.word	0x427f000
[ 0-9a-f]+:	27e18000 	addiu	at,ra,-32768
[ 0-9a-f]+:	0427f000 	.word	0x427f000
[ 0-9a-f]+:	3c010001 	lui	at,0x1
[ 0-9a-f]+:	00240821 	addu	at,at,a0
[ 0-9a-f]+:	0427ffff 	.word	0x427ffff
[ 0-9a-f]+:	3c010001 	lui	at,0x1
[ 0-9a-f]+:	00240821 	addu	at,at,a0
[ 0-9a-f]+:	0427f000 	.word	0x427f000
[ 0-9a-f]+:	3c01ffff 	lui	at,0xffff
[ 0-9a-f]+:	00240821 	addu	at,at,a0
[ 0-9a-f]+:	0427f000 	.word	0x427f000
[ 0-9a-f]+:	24818000 	addiu	at,a0,-32768
[ 0-9a-f]+:	0427f000 	.word	0x427f000
[ 0-9a-f]+:	3c01ffff 	lui	at,0xffff
[ 0-9a-f]+:	00240821 	addu	at,at,a0
[ 0-9a-f]+:	0427f001 	.word	0x427f001
[ 0-9a-f]+:	24818001 	addiu	at,a0,-32767
[ 0-9a-f]+:	0427f000 	.word	0x427f000
[ 0-9a-f]+:	3c01f000 	lui	at,0xf000
[ 0-9a-f]+:	00240821 	addu	at,at,a0
[ 0-9a-f]+:	0427f000 	.word	0x427f000
[ 0-9a-f]+:	0487ffff 	.word	0x487ffff
[ 0-9a-f]+:	3c011234 	lui	at,0x1234
[ 0-9a-f]+:	34215000 	ori	at,at,0x5000
[ 0-9a-f]+:	00240821 	addu	at,at,a0
[ 0-9a-f]+:	0427f678 	.word	0x427f678
	\.\.\.
