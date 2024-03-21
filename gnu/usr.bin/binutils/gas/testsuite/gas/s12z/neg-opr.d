#objdump: -d
#name:    
#source:  neg-opr.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	df fe 12 34 	neg.l \[1193046\]
   4:	56 
   5:	dc c4 04    	neg.b \[4,x\]
   8:	dd b8       	neg.w d2
   a:	dc dc       	neg.b \[d0,y\]
   c:	dc e4 0c    	neg.b \[12,s\]
   f:	dc 63       	neg.b \(3,s\)
  11:	dc e1 85    	neg.b \(-123,s\)
  14:	df f2 12 3a 	neg.l \(1194684,p\)
  18:	bc 
  19:	dd c7       	neg.w \(x-\)
  1b:	dd d7       	neg.w \(y-\)
  1d:	dc e7       	neg.b \(x\+\)
  1f:	dc f7       	neg.b \(y\+\)
  21:	dc ff       	neg.b \(s\+\)
  23:	df e3       	neg.l \(\+x\)
  25:	dd f3       	neg.w \(\+y\)
  27:	df c3       	neg.l \(-x\)
  29:	dd d3       	neg.w \(-y\)
  2b:	dc fb       	neg.b \(-s\)
  2d:	df 8d       	neg.l \(d1,x\)
  2f:	df 98       	neg.l \(d2,y\)
  31:	df a9       	neg.l \(d3,s\)
