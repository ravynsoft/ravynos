	.text
	.type start,"function"
	.global start
start:
	.type _start,"function"
	.global _start
_start:
	.type __start,"function"
	.global __start
__start:
	.type main,"function"
	.global main
main:
	.type _main,"function"
	.global _main
_main:
	.long 0
	.section .foo1,"e", %progbits
	.byte 0,0,0,0
