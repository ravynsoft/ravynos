start:	mov	$ind,r0
        jsr     pc,(r0)
	jsr	pc,@(r0)
	jsr	pc,@0(r0)
	jsr	pc,@2(r0)
	halt


ind:	.WORD	dest
	.WORD	dest2

dest:	rts	pc

dest2:	rts	pc
	
	.END
	
