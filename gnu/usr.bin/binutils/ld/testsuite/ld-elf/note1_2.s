
	.section .gnu.linkonce.t.thunk.ax,"ax",%progbits
	.globl  thunk.ax
	.hidden thunk.ax
	.p2align 4
	.type   thunk.ax,%function
thunk.ax:
	.dc.l	0
	.size thunk.ax, . - thunk.ax

	.p2align 4
	.globl bar
	.type  bar,%function
bar:
	.dc.a	thunk.ax
	.dc.l	0
	.size bar, . - bar
