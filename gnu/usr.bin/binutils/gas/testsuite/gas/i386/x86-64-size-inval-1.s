# Test SIZE32 relocation overflow
	.local	yyy
	.comm	yyy,80,32
	.text
	movl	$xxx@SIZE + 100, %eax
	movl	$yyy@SIZE - 100, %eax
	.local	xxx
	.comm	xxx,4294967290,32
	.data
	.long	xxx@SIZE + 100
	.long	yyy@SIZE - 100
