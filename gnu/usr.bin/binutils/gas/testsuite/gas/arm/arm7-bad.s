	.text
	.cpu cortex-a8
	.syntax unified
	.arm
	cbnz r0, .+6
	.thumb
	cbnz r0, .+6
