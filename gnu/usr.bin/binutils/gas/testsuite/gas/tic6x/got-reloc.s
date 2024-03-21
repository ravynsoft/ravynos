# Test that no GOT relocs with an addend are produced.
	.section	.const.str1.1,"aMS",@progbits,1
.LC0:
	.string	"foo"
.LC1:
	.string	"bar"
.text
.nocmp
.globl f
f:
	ldw .d2t2 *+B14($GOT(.LC1)), B0
	mvkl .s2 $DPR_GOT(.LC1), B1
	mvkh .s2 $DPR_GOT(.LC1), B1


