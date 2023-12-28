#objdump: -d
#name:    
#source:  bit-manip.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	ec 1c       	bclr d0, #3
   2:	ed 25       	bset d1, #4
   4:	ee 28       	btgl d2, #5
   6:	ec b1 b9    	bclr d3, d5
   9:	ed e1 ba    	bset d4, d6
   c:	ee f1 bb    	btgl d5, d7
   f:	ec a0 c0 22 	bclr.b \(34,x\), #2
  13:	ec c3 ff    	bclr.w \(s\+\), #12
  16:	ec fd e0 38 	bclr.l \(56,s\), d7
  1a:	ed d0 c4 22 	bset.b \[34,x\], #5
  1e:	ed db fb    	bset.l \(-s\), #29
  21:	ed f5 c0 9c 	bset.w \(156,x\), d7
  25:	ee d0 c4 22 	btgl.b \[34,x\], #5
  29:	ee f3 fb    	btgl.w \(-s\), #15
  2c:	ee fd f0 0f 	btgl.l \(15,p\), d7
