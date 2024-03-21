# Test SIZE32 relocations against global symbols
	.text
	movl	$xxx@SIZE, %eax
	movl	$xxx@SIZE - 8, %eax
	movl	$xxx@SIZE + 8, %eax
	movl	$yyy@SIZE, %eax
	movl	$yyy@SIZE - 16, %eax
	movl	$yyy@SIZE + 16, %eax
	movl	$zzz@SIZE, %eax
	movl	$zzz@SIZE - 32, %eax
	movl	$zzz@SIZE + 32, %eax
	.comm	zzz,429496729,32
	.bss
	.global	yyy
	.type	yyy,%object
	.size	yyy,30
yyy:
	.zero	30
	.data
	.global	xxx
	.type	xxx,%object
	.size	xxx,80
xxx:
	.zero	80
	.long	xxx@SIZE - 1
	.long	yyy@SIZE + 2
	.long	zzz@SIZE
