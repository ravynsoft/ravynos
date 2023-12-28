foo .req x0 ; bar .req x1
.unreq foo ; .unreq bar
.cpu generic ; .arch armv8-a ; .arch_extension crc ; .word 0
	
