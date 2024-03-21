	.syntax unified
	.text
	.global end
	.type end, %function
end:
	.fnstart
	.save {r4, lr}
	bx lr
	.fnend
