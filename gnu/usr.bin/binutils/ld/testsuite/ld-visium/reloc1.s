	.data
foo:
	.byte	0
	.byte	data1-foo
	.word	data1-foo
	.long	data1-foo
	.long	text1
	.long	text2
	.word	abs1
	.word	abs1+0x10
	.text
bar:
	nop
	brr	tr,text1
	moviq	r2,text1-bar+8
	movil	r2,%l text2-bar+16
	moviu	r2,%u text2-bar+16
	moviq	r2,%u text2
	subi	r2,%l text2
	addi	r2,%u text2
	movil	r2, text2	; with movil, the %l may be omitted
	moviu	r2,%u text2
	moviq	r2,abs1
	.end
