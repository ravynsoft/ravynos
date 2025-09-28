	.text
	.org 0x100
	.global note1.s
note1.s:
	.dc.l 0
	
	.pushsection .gnu.build.attributes, "0x100000", %note
	.balign 4
	.dc.l 8
	.dc.l 16
	.dc.l 0x100
	.asciz "GA$3p1"
	.8byte 0x100
	.8byte 0x104

	.dc.l 14
	.dc.l 0
	.dc.l 0x100
	.asciz "GA$gcc 7.0.1"
	.dc.b 0,0
	
	.dc.l 5
	.dc.l 0
	.dc.l 0x100
	.dc.b 0x47, 0x41, 0x2b, 0x2, 0
	.dc.b 0,0,0

	.dc.l 6
	.dc.l 0
	.dc.l 0x100
	.dc.b 0x47, 0x41, 0x2a, 0x7, 0, 0
	.dc.b 0,0

	.dc.l 6
	.dc.l 0
	.dc.l 0x100
	.dc.b 0x47, 0x41, 0x2a, 0x6, 0, 0
	.dc.b 0,0

	.dc.l 6
	.dc.l 0
	.dc.l 0x100
	.dc.b 0x47, 0x41, 0x2a, 0x7, 0, 0
	.dc.b 0,0

	.popsection


	.global note2.s
note2.s:
	.global func1
	.type func1, STT_FUNC
func1:	
	.dc.l 0x100

	.pushsection .gnu.build.attributes, "0x100000", %note
	.dc.l 8
	.dc.l 16		
	.dc.l 0x100	
	.asciz "GA$3p1"
	.8byte 0x104	
	.8byte 0x108	

	.dc.l 14 	
	.dc.l 0		
	.dc.l 0x100	
	.asciz "GA$gcc 7.0.1"	
	.dc.b 0,0
	
	.dc.l 5		
	.dc.l 0		
	.dc.l 0x100	
	.dc.b 0x47, 0x41, 0x21, 0x2, 0
	.dc.b 0,0,7 	

	.dc.l 6
	.dc.l 16
	.dc.l 0x101	
	.dc.b 0x47, 0x41, 0x2a, 0x7, 1, 0
	.dc.b 0,0
	.8byte 0x104	
	.8byte 0x106	

	.dc.l 6
	.dc.l 0		
	.dc.l 0x100	
	.dc.b 0x47, 0x41, 0x2a, 0x6, 0, 0
	.dc.b 0,0
	.popsection
	

	.global note3.s
note3.s:
	.dc.l 0x100
	
	.pushsection .gnu.build.attributes, "0x100000", %note
	.dc.l 8	
	.dc.l 16	
	.dc.l 0x100	
	.asciz "GA$3p1"
	.8byte 0x108
	.8byte 0x10b

	.dc.l 14 	
	.dc.l 0		
	.dc.l 0x100	
	.asciz "GA$gcc 7.0.1"
	.dc.b 0,0

	.popsection
