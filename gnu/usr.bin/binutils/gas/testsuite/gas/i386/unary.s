	.text
unary:
	mov	+1(%eax), %eax
	mov	-1(%eax), %eax
	mov	!1(%eax), %eax
	mov	~1(%eax), %eax

	mov	[+1](%eax), %eax
	mov	[-1](%eax), %eax
	mov	[!1](%eax), %eax
	mov	[~1](%eax), %eax
