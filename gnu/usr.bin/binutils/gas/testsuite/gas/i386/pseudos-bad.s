	.text
pseudos:
	{vex} nop
	{vex} nop %eax

	{vex3} nop
	{vex3} nop %eax

	{evex} nop
	{evex} nop %eax

	{evex} vzeroall
	{evex} vmovmskps %xmm0, %eax
