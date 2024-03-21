	.section .text.1
	j.l	.Lfar, a8
	.section .text.2
.Lfar:
	j.l	.Lnear, a9
	.word	0
.Lnear:
