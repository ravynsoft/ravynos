#objdump: -d
#name:    
#source:  db-tb-cc-opr.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	0b dc 82 00 	dble.b \(3,d4\), \*\+1
   4:	03 01 
   6:	0b cd 83 00 	dbgt.w \(23,d5\), \*\+18
   a:	17 12 
   c:	0b de 84 00 	dble.p \(2,d0\), \*-19
  10:	02 6d 
  12:	0b bf fb ff 	dbmi.l \(-s\), \*-137
  16:	77 
  17:	0b 3c e4 22 	tbmi.b \[34,s\], \*\+43
  1b:	2b 
  1c:	0b 4d ff 5f 	tbgt.w \(s\+\), \*-33
  20:	0b 1e c4 ea 	tbeq.p \[234,x\], \*-134
  24:	ff 7a 
  26:	0b 5f f0 22 	tble.l \(34,p\), \*\+331
  2a:	81 4b 
  2c:	0b 0c 5e 81 	tbne.b \(14,y\), \*\+431
  30:	af 
  31:	0b 2e 4e 80 	tbpl.p \(14,x\), \*\+231
  35:	e7 