	.text
	.globl _start
_start:
	la.local $r20,_start
	jirl $r1, $r20, 0
