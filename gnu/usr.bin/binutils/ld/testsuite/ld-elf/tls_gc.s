	.globl main
	.globl start
	.globl _start
	.globl __start
	.text
main:
start:
_start:
__start:
	.byte 0
	.globl var
	.section	.tbss.var,"awT",%nobits
	.type	var,%object
	.size	var,1
var:
	.zero	1
