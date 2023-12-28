# Test SIZE32 relocations against local symbols
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
	movl	$.text@SIZE, %eax
	movl	$.data@SIZE + 4, %eax
	movl	$.bss@SIZE - 0x10000000, %eax
	.local	zzz
	.comm	zzz,429496720,32
	.bss
	.type	yyy,%object
	.size	yyy,30
yyy:
	.zero	30
	.data
	.type	xxx,%object
	.size	xxx,80
xxx:
	.zero	80
	.long	xxx@SIZE - 1
	.long	yyy@SIZE + 2
	.long	zzz@SIZE
