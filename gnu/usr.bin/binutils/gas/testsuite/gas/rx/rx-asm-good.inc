	; mac1	.MACRO f1,f2
	; 	.IF    ..MACPARA == 2
	; 	add  f1,f2
	; 	.ELSE
	; 	add	r0,f1
	; 	.EXITM
	; 	.ENDIF
	; 	.ENDM
	; 
	; mac2	.MACRO
	; 	.LOCAL f1
	; f1:
	; 	nop
	; 	bra f1
	; 	.ENDM
	; 	
	; 
	; mac3	.MACRO p1,src,p2,dest
	; 	MOV@p1@p2	src, dest
	; 	.ENDM
