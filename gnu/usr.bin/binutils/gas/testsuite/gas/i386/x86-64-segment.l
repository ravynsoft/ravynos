   1              	.psize 0
   2              	.text
   3              	# test segment reg insns with memory operand
   4 0000 8C18     		movw	%ds,\(%rax\)
   5 0002 8C18     		mov	%ds,\(%rax\)
   6 0004 8E18     		movw	\(%rax\),%ds
   7 0006 8E18     		mov	\(%rax\),%ds
   8              	# test segment reg insns with avoided REX
   9 0008 8CD8     		mov	%ds,%rax
  10 000a 8CD8     		movq	%ds,%rax
  11 000c 8ED8     		mov	%rax,%ds
  12 000e 8ED8     		movq	%rax,%ds
  13              	# test segment reg insns with REX
  14 0010 418CD8   		mov	%ds,%r8
  15 0013 418CD8   		movq	%ds,%r8
  16 0016 418ED8   		mov	%r8,%ds
  17 0019 418ED8   		movq	%r8,%ds
  18              		# Force a good alignment.
  19 001c 0+ +	*\.p2align	4,0
