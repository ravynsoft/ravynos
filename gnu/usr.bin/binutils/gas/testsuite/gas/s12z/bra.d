#objdump: -d
#name:    
#source:  bra.s

.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <L1>:
   0:	20 80 0f    	bra L4

00000003 <L2>:
   3:	21 ff fd    	bsr L1
   6:	22 ff fa    	bhi L1

00000009 <L3>:
   9:	23 80 00    	bls L3
   c:	24 ff f4    	bcc L1

0000000f <L4>:
   f:	25 ff f4    	bcs L2
  12:	26 ff f7    	bne L3
  15:	27 ff fa    	beq L4
  18:	28 ff f7    	bvc L4
  1b:	29 ff e8    	bvs L2
  1e:	2a ff e2    	bpl L1
  21:	2b ff e2    	bmi L2
  24:	2c ff dc    	bge L1
  27:	2d ff e8    	blt L4
  2a:	2e ff df    	bgt L3
  2d:	2f ff d3    	ble L1
  30:	24 ff d3    	bcc L2
  33:	25 ff d0    	bcs L2
  36:	20 02       	bra \*\+2
  38:	20 7c       	bra \*-4
