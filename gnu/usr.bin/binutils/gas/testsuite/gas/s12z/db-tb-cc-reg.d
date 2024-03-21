#objdump: -d
#name:    
#source:  db-tb-cc-reg.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	0b 84 0c    	dbne d0, \*\+12
   3:	0b 95 0d    	dbeq d1, \*\+13
   6:	0b a8 72    	dbpl x, \*-14
   9:	0b b9 0f    	dbmi y, \*\+15
   c:	0b c1 80 7b 	dbgt d3, \*\+123
  10:	0b d7 fb 1f 	dble d7, \*-1249
  14:	0b 04 16    	tbne d0, \*\+22
  17:	0b 15 17    	tbeq d1, \*\+23
  1a:	0b 28 18    	tbpl x, \*\+24
  1d:	0b 39 19    	tbmi y, \*\+25
  20:	0b 41 80 df 	tbgt d3, \*\+223
  24:	0b 57 88 c9 	tble d7, \*\+2249
  28:	0b 39 28    	tbmi y, \*\+40
  2b:	0b 84 ff 75 	dbne d0, \*-139
  2f:	0b a5 04    	dbpl d1, \*\+4
  32:	0b c2 ff 76 	dbgt d4, \*-138
  36:	0b 90 29    	dbeq d2, \*\+41
  39:	0b 06 04    	tbne d6, \*\+4
  3c:	0b 27 ff 78 	tbpl d7, \*-136
  40:	0b 18 28    	tbeq x, \*\+40
