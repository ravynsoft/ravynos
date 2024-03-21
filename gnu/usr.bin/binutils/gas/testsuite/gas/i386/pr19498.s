      	.text
	.globl callmefirst
callmefirst:
	nop
	.globl callmesecond
callmesecond:
	jmp	callmealias.lto_priv.0
	.weakref callmealias.lto_priv.0,callmefirst
