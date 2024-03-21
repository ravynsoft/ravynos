; Just something defining 7 variables @ 4 bytes and requesting GOT
;  entries for each of them.

	.text
	.global	got7fn
	.type	got7fn,@function
got7fn:
	.irpc	n,1234567
	move.d	got7var\n:GOT,$r10
	.endr
.Lfe:
	.size	got7fn,.Lfe-got7fn

	.irpc	n,1234567
	.data
	.global got7var\n
	.type	got7var\n,@object
	.size	got7var\n,4
got7var\n:
	.dword 0
	.endr
