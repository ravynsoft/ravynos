	.section .mbind.data,"adw",%progbits,unique,0
	.byte 1

	.section .mbind.data,"adw",%progbits,0x3,unique,1
	.byte 2

	.section .mbind.text,"adx",%progbits,unique,2
	.byte 3

	.section .mbind.text,"adx",%progbits,0x3,unique,3
	.byte 4

	.section .mbind.bss,"adw",%nobits,unique,4
	.zero 5

	.section .mbind.bss,"adw",%nobits,0x3,unique,5
	.zero 6

	.section .mbind.rodata,"adG",%progbits,.foo_group,comdat,0x2,unique,6
	.byte 7

	.section .mbind.data,"adGw",%progbits,.foo_group,comdat,unique,7
	.byte 8

	.section .mbind.data,"adGw",%progbits,.foo_group,comdat,0x3,unique,8
	.byte 9

	# Check that .pushsection works as well.
	.pushsection .mbind.text,"adGx",%progbits,.foo_group,comdat,0x3,unique,9
	.byte 10

	.popsection
	.byte 11
