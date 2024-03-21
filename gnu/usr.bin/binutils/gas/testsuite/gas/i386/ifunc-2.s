	.section .text.1,"ax",@progbits

start1:
	.long	bar1-foo1
	.long	bar2-foo2
	.long	bar1-bar2
	.long	bar2-bar1
	.long	start1-bar1
	.long	start1-bar2
	.long	bar1-abs1
	.long	abs1-bar1
	.long	.-bar1

	.type	foo1,%gnu_indirect_function
foo1:
	ret
	.size	foo1,.-foo1

	.long	bar1-foo1
	.long	bar2-foo2
	.long	bar1-bar2
	.long	bar2-bar1
	.long	start1-bar1
	.long	start1-bar2
	.long	bar1-abs1
	.long	abs1-bar1
	.long	.-bar1

	.type	bar1,%gnu_indirect_function
bar1:
	ret
	.size	bar1,.-bar1

	.long	bar1-foo1
	.long	bar2-foo2
	.long	bar1-bar2
	.long	bar2-bar1
	.long	start1-bar1
	.long	start1-bar2
	.long	bar1-abs1
	.long	abs1-bar1
	.long	.-bar1

	.long	abs1-abs2
	.long	abs2-abs1

	.equ	abs1,0x11223300
	.type	abs1,%gnu_indirect_function

	.long	abs1-abs2
	.long	abs2-abs1

	.equ	abs2,0x11223380
	.type	abs2,%gnu_indirect_function

	.long	abs1-abs2
	.long	abs2-abs1

	.section .text.2,"ax",@progbits

start2:
	.long	bar1-foo1
	.long	bar2-foo2
	.long	bar1-bar2
	.long	bar2-bar1
	.long	start2-bar1
	.long	start2-bar2
	.long	bar2-abs1
	.long	abs1-bar2
	.long	.-bar2

	.type	foo2,%gnu_indirect_function
foo2:
	ret
	.size	foo2,.-foo2

	.long	bar1-foo1
	.long	bar2-foo2
	.long	bar1-bar2
	.long	bar2-bar1
	.long	start2-bar1
	.long	start2-bar2
	.long	bar2-abs1
	.long	abs1-bar2
	.long	.-bar2

	.type	bar2,%gnu_indirect_function
bar2:
	ret
	.size	bar2,.-bar2

	.long	bar1-foo1
	.long	bar2-foo2
	.long	bar1-bar2
	.long	bar2-bar1
	.long	start2-bar1
	.long	start2-bar2
	.long	bar2-abs1
	.long	abs1-bar2
	.long	.-bar2
