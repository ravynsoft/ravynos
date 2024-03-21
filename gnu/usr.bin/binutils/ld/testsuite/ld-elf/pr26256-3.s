	.text
	.global _start
_start:
	.long 0x33333333
	.long 0x33333333
	.long 0x33333333
	.long 0x33333333

	.section .rosection,"a"
	.byte 9

	.section .text.bar,"a",%progbits
	.long 0x22222222
	.long 0x22222222
	.long 0x22222222
	.long 0x22222222
	.section .text.foo,"a",%progbits
	.long 0x11111111
	.long 0x11111111
	.long 0x11111111
	.long 0x11111111
	.section .rodata.foo,"ao",%progbits,.text.foo
	.byte 1
	.section .rodata.bar,"a",%progbits
	.byte 2
	.section .rodata.bar,"ao",%progbits,.text.bar
	.byte 3
