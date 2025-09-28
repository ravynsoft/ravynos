# Test SIZE32 relocations against global TLS symbols
	.text
	movl	$xxx@SIZE, %eax
	movl	$xxx@SIZE - 8, %eax
	movl	$xxx@SIZE + 8, %eax
	movl	$yyy@SIZE, %eax
	movl	$yyy@SIZE - 16, %eax
	movl	$yyy@SIZE + 16, %eax
	.section .tbss,"awT",%nobits
	.global	yyy
	.type	yyy,%object
	.size	yyy,30
yyy:
	.zero	30
	.section .tdata,"awT",%progbits
	.global	xxx
	.type	xxx,%object
	.size	xxx,80
xxx:
	.zero	80
	.long	xxx@SIZE - 1
	.long	yyy@SIZE + 2
