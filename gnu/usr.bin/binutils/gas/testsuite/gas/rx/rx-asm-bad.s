	.text

	.ID "id-is-jane"

	.INITSCT a_section,data, align
	.OPTJ OFF,JMPW,JSRW	
	.INSF glbfunc, G, 0
	.CALL glbsub, G
	.CALL localsub, S
	.EINSF
	.FB 80H
	ldc #80H,FB
	.FBSYM george

	.LBBA 1000h
	ldr #1000h,LBBA
	lda 1000h,0,1200h,1

	.SB 80H
	ldc #80,SB

	.SBBIT	lance
	bclr	lance

	.SBSYM	sym
	mov.B	#0, sym

	.SBSYM16 sym2
	mov.B	 #0.sym2
	
	.RVECTOR 12,timer

	.BTGLB bit1

	fred .DEFINE "#01H,mem"

	bit1 .BTEQU 1,dmem

	three .equ .INSTR {"albert","bert",1}
	seven .equ .LEN {"albert"}
	.byte .SUBSTR {"albert",3,3}
		
	.ASSERT "assertion message"

	.FORM 20,80
	
	.LIST ON

	nop
	
	.PAGE "new page"

	.VER "ver"

	
	.END

	This text should just be emitted and not assembled