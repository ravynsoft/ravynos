#objdump: -d
#name:    
#source:  adc-opr.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <.text>:
   0:	1b 64 c5 21 	adc d0, \[-223,x\]
   4:	1b 65 e2 ff 	adc d1, \(-2000,s\)
   8:	f8 30 
   a:	1b 60 c7    	adc d2, \(x-\)
   d:	1b 61 f3    	adc d3, \(\+y\)
  10:	1b 62 bb    	adc d4, d5
  13:	1b 63 3e ce 	adc d5, 16078
  17:	1b 66 fe 01 	adc d6, \[73056\]
  1b:	1d 60 
  1d:	1b 67 8a    	adc d7, \(d4,x\)
