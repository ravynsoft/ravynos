	.globl  bar1
	.set    bar1,(%eax+1)
	mov bar1,%eax
	.set    bar2,(%eax+1)
	mov bar2,%eax
	.globl  bar2
	.set    bar3,(%eax+1)
	mov bar3,%eax
