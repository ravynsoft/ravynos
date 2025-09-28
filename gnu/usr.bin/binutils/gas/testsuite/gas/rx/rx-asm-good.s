
	.INCLUDE ..FILE@.inc
	
	.SECTION P,CODE,ALIGN

	.ORG 100H
	
	.GLB mem
mem:
	.word three, seven
	
	; mac1 r2
	; mac1 r3,r4
	; 
	; mac2
	; 
	; mac3 L,r0,H,[a0]
	
	.MREPEAT 3
	mov.l #..MACREP,r0
	.ENDR

	.ALIGN

	.STK 2
	bsr glbsub
	bsr localsub
	.STK -2

	.IF TYPE==0
	.byte "harry"
	.ELIF TYPE>0
	.byte "jim"
	.ELSE
	.byte "bert"
	.ENDIF

?:
	bra ?+
	bra ?-
?:
	bra ?-

	
	.SECTION D_1,DATA
	.GLB dmem
dmem:
	size .EQU 2
	
	.ADDR 1
	.ADDR "dat"
	.ADDR dmem+1

	.BLKA 1
	.BLKA size

	.BLKB 1
	.BLKB size

	.BLKD 1
	.BLKD size

	.BLKF 1
	.BLKF size

	.BLKL 1
	.BLKL size+1

	.BLKW 1
	.BLKW size

	.BYTE 1
	.BYTE "data"
	.BYTE dmem+1

	.DOUBLE 5e2
	.FIXED  1.234
	.FLOAT  5e2

	.LWORD 1
	.LWORD "two"
	.LWORD 3+4
	
	.WORD	3
	
	