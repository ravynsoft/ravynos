	.text
start:
	cmovl	%eax, %ecx

	.arch push
	.arch default
	cmovnl	%eax, %ecx

	.arch pop
	cmovg	%eax, %ecx

	.arch push
	.arch .cmov
	cmovng	%eax, %ecx

	.arch pop
	cmovz	%eax, %ecx

	.arch push
	.code16
	.arch pop
	.code32
	.arch pop

	.code16gcc
	.arch push
	.code32
	.arch pop
	.code16gcc
	.arch pop

	.arch pop

	.end
