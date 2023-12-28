# Test SIZE64 relocation
	.text
	movq	$xxx@SIZE, %r15
	movq	$xxx@SIZE - 8, %r15
	movq	$xxx@SIZE + 8, %r15
	.comm	zzz,4294967290,32
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
	.quad	xxx@SIZE - 1
	.quad	yyy@SIZE + 200
	.quad	zzz@SIZE
