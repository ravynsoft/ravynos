	.text
start:
	cmovl	%eax, %ecx

	.arch default
	cmovnl	%eax, %ecx

	.arch generic32
	cmovg	%eax, %ecx

	.arch default
	cmovng	%eax, %ecx

	.end
