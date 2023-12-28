#as: -maltivec -mspe -mppc64 
#objdump: -d -Maltivec -Mppc64
#name: Check that ISA extensions can be specified before CPU selection

.*

Disassembly of section \.text:

0+00 <.*>:
   0:	(7e 00 06 6c|6c 06 00 7e) 	dssall
   4:	(7d 00 83 a6|a6 83 00 7d) 	mtspr   512,r8
   8:	(4c 00 00 24|24 00 00 4c) 	rfid
#pass
