#source: ./r-bra.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <bar-0x1>:
       0:	03                            	nop

00000001 <bar>:
       1:	03                            	nop
       2:	03                            	nop
       3:	04 .. .. ..                   	bra\.a	.*
			4: R_RX_DIR24S_PCREL	fred
       7:	04 7b 81 00                   	bra\.a	8182 <barney>
       b:	38 8e 00                      	bra\.w	99 <grill>
       e:	2e f3                         	bra\.b	1 <bar>
      10:	0e                            	bra\.s	16 <foo>
      11:	03                            	nop
      12:	2e ef                         	bra\.b	1 <bar>
      14:	03                            	nop
      15:	03                            	nop

00000016 <foo>:
      16:	03                            	nop
	\.\.\.

00000099 <grill>:
      99:	03                            	nop
	\.\.\.

00008182 <barney>:
    8182:	03                            	nop
