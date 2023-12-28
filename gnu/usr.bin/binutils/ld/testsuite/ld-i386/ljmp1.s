 .text
.code32
 .global _start
_start:
	movl	%eax, %cr0
	ljmp	$seg1, $foo
foo:
	ljmpw	$seg2, $0
 .p2align 4,0x90
