# Check %axl / %cxl aren't permitted as accumulator / shift count

	.text
reg:
	add	$1, %axl
	div	%bl, %axl
	in	%dx, %axl
	lods	(%rsi), %axl
	movabs	-1, %axl
	shl	%cxl, %eax
	test	$1, %axl
