	.section .text.1,"ax",@progbits

start1:
	.long	bar1-.
	.long	bar2-.
	.long	bar1-start1
	.long	bar2-start1
	.long	bar1-base

	.type	bar1,%gnu_indirect_function
	.size	bar1,.Lendbar1-bar1
bar1:
	ret
.Lendbar1:
	.align	4

	.long	bar1-.
	.long	bar2-.
	.long	bar1-start1
	.long	bar2-start1
	.long	bar1-base

	.long	abs1-.
	.long	abs1-start1
	.long	abs1-base

	.equ	abs1,0x11223300
	.type	abs1,%gnu_indirect_function

	.long	abs1-.
	.long	abs1-start1
	.long	abs1-base

	.section .text.2,"ax",@progbits

start2:
	.long	bar1-.
	.long	bar2-.
	.long	bar1-start2
	.long	bar2-start2
	.long	bar2-base

	.type	bar2,%gnu_indirect_function
bar2:
	ret
	.size	bar2,.-bar2
	.align	4

	.long	bar1-.
	.long	bar2-.
	.long	bar1-start2
	.long	bar2-start2
	.long	bar2-base

	.equ	base,0xabc0
