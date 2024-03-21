	.section .mbind.data,"adw",%progbits
	.byte 1

	.section .mbind.data,"adw",%progbits,0x3
	.byte 2

	.section .mbind.text,"adx",%progbits
	.byte 3

	.section .mbind.text,"adx",%progbits,0x3
	.byte 4

	.section .mbind.bss,"adw",%nobits
	.zero 5

	.section .mbind.bss,"adw",%nobits,0x3
	.zero 6

	.section .mbind.rodata,"adG",%progbits,.foo_group,comdat,0x2
	.byte 7

	.section .mbind.data,"adGw",%progbits,.foo_group,comdat
	.byte 8

	.section .mbind.data,"adGw",%progbits,.foo_group,comdat,0x3
	.byte 9

	# Check that .pushsection works as well.
	.pushsection .mbind.text,"adGx",%progbits,.foo_group,comdat,0x3
	.byte 10

	.popsection
	.byte 11
