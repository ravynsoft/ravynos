#objdump: -d -Mcom
#as: -mcom -be
#name: BE only instructions

.*

Disassembly of section \.text:

0+00 <start>:
.*:	ba 8a 00 10 	lmw     r20,16\(r10\)
.*:	7d 4b 0c aa 	lswi    r10,r11,1
.*:	7d 8b 04 aa 	lswi    r12,r11,32
.*:	7d 4b 64 2a 	lswx    r10,r11,r12
.*:	be 8a 00 10 	stmw    r20,16\(r10\)
.*:	7d 4b 0d aa 	stswi   r10,r11,1
.*:	7d 4b 05 aa 	stswi   r10,r11,32
.*:	7d 4b 65 2a 	stswx   r10,r11,r12
