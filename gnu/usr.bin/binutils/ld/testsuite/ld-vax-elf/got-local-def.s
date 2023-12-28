	.data
	.globl	bar_hidden
	.type	bar_hidden, @object
	.hidden	bar_hidden
bar_hidden:
	.byte	0
	.size	bar_hidden, . - bar_hidden
	.globl	bar_visible
	.type	bar_visible, @object
bar_visible:
	.byte	0
	.size	bar_visible, . - bar_visible
