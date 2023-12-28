.text
	.globl foo
foo:
	lgrl	%r1,bar@GOTENT
	lg	%r1,bar@GOT(%r12)
	lrl	%r1,bar@GOTENT
	l	%r1,bar@GOT(%r12)
	ly	%r1,bar@GOT(%r12)
	lgrl	%r1,_GLOBAL_OFFSET_TABLE_@GOTENT
	lgrl	%r1,misaligned_sym@GOTENT

.data
.globl	bar
bar:	.quad	0x123

.globl  misaligned_sym
	.byte	1
misaligned_sym:
	.quad	42
