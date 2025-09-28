	.syntax unified
	.section .code.1, "ax", %progbits
	.arm
	# Fit in RAML
	.global code1
	.type code1, %function
code1:
	nop
	nop
	bl code2

	.section .code.2, "ax", %progbits
	# Fit in RAML
	.global code2
	.type code2, %function
code2:
	nop
	nop
	bl code3

	.section .code.3, "ax", %progbits
	# Fit in RAMU
	.global code3
	.type code3, %function
code3:
	nop
	bl code4

	.section .code.4, "ax", %progbits
	# Fit in RAMZ
	.global code4
	.type code4, %function
code4:
$a:
	.fill 20, 4, 0xe1a00000
