	.globl main
	.globl _main
	.globl start
	.globl _start
	.globl __start
	.text
main:
_main:
start:
_start:
__start:
	.byte 0
	.globl	var3
	.section	.data.var3,"aw",%progbits
	.p2align 2
	.type	var3, %object
	.size	var3, 4
var3:
	.zero	4
	.globl	var2
	.section	.data.var2,"aw",%progbits
	.p2align 4
	.type	var2, %object
	.size	var2, 16
var2:
	.zero	16
	.globl	var1
	.section	.data.var1,"aw",%progbits
	.p2align 2
	.type	var1, %object
	.size	var1, 4
var1:
	.zero	4
