 	.text
	nop
	{disp32}
	nop
	{disp32} movb (%bp),%al
	{disp16} movb (%ebp),%al
	{disp16} jmp .
	.code16
	{disp32} jmp .
	.p2align 4,0
