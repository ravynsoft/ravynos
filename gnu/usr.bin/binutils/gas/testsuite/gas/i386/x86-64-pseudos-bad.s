	.text
pseudos:
	{vex} vmovaps %xmm0, %xmm30
	{vex3} vmovaps %xmm30, %xmm0
	{rex} vmovaps %xmm7,%xmm2
	{rex} vmovaps %xmm17,%xmm2
	{rex} rorx $7,%eax,%ebx
