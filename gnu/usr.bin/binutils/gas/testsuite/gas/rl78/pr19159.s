	.text

	mov a, [de]
	mov a, [de + 0]
	mov a, [de + 1]
	mov [de], a
	mov [de + 0], a
	mov [de + 1], a
	movw ax, [de]
	movw ax, [de + 0]
	movw ax, [de + 1]

	.end
