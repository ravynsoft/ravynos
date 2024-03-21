#objdump: -d -r --show-raw-insn
#as: -32
#name: MIPS-OCTEON octeon_saa_saad

.*: +file format .*mips.*

Disassembly of section .text:

[0-9a-f]+ <foo>:
.*:	70450018 	saa	a1,\(v0\)
.*:	70860019 	saad	a2,\(a0\)
.*:	00000000 	nop
.*:	70450018 	saa	a1,\(v0\)
.*:	70860019 	saad	a2,\(a0\)
.*:	00000000 	nop
.*:	3c010000 	lui	at,0x0
			18: R_MIPS_HI16	.text
.*:	24210000 	addiu	at,at,0
			1c: R_MIPS_LO16	.text
.*:	70250018 	saa	a1,\(at\)
.*:	3c010000 	lui	at,0x0
			24: R_MIPS_HI16	.text
.*:	24210000 	addiu	at,at,0
			28: R_MIPS_LO16	.text
.*:	70220019 	saad	v0,\(at\)
.*:	00000000 	nop
.*:	3c011234 	lui	at,0x1234
.*:	24215678 	addiu	at,at,22136
.*:	70240018 	saa	a0,\(at\)
.*:	3c011234 	lui	at,0x1234
.*:	24215678 	addiu	at,at,22136
.*:	70240019 	saad	a0,\(at\)
.*:	00000000 	nop
.*:	24811234 	addiu	at,a0,4660
.*:	70250018 	saa	a1,\(at\)
.*:	2401003c 	li	at,60
.*:	70260019 	saad	a2,\(at\)
.*:	00000000 	nop
.*:	3c010012 	lui	at,0x12
.*:	00240821 	addu	at,at,a0
.*:	24213456 	addiu	at,at,13398
.*:	70250018 	saa	a1,\(at\)
.*:	24c11234 	addiu	at,a2,4660
.*:	70260018 	saa	a2,\(at\)
.*:	00000000 	nop
.*:	24a15678 	addiu	at,a1,22136
.*:	70240019 	saad	a0,\(at\)
.*:	3c010056 	lui	at,0x56
.*:	00250821 	addu	at,at,a1
.*:	24217891 	addiu	at,at,30865
.*:	70250019 	saad	a1,\(at\)
.*:	00000000 	nop
.*:	24a10000 	addiu	at,a1,0
			9c: R_MIPS_LO16	.text
.*:	70240018 	saa	a0,\(at\)
.*:	24a10000 	addiu	at,a1,0
			a4: R_MIPS_LO16	.text
.*:	70240019 	saad	a0,\(at\)
.*:	00000000 	nop
