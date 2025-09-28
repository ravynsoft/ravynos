.set norelax
_main:	br start
trap:
	br abort	
.globl _main
abort: 	movui r3, 0x0
	movui  r2, 0x1
	
end: movui r3, 0x0
	movui r2, 0x0
	br exit

start:	
		addi r2, r2, -4		# test for ve numbers
		movui r11, 0x1		
		ori r5, r0, %lo(0x0)	# r5 = 0x0
		ori r6, r0, %lo(0x0)	# r6 = 0x0
		br ldst	

ldst:
		movui r2, 0xF00C
		movui r20, 0xFACE
		stw r20,(r2)
		ldw r21, (r2)		
		br end
		

exit:	br exit
