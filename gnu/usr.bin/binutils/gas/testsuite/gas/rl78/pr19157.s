	.text

	mov a, [sp]
	mov a, [sp + 0]
	mov a, [sp + 1]
	movw ax, [sp]
	movw ax, [sp + 0]
	movw ax, [sp + 2]
	mov [sp], # 9
	mov [sp + 0], # 9
	mov [sp + 1], # 9

	.end
