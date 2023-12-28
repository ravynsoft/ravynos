#name: movem-offset
#objdump: -d
#as: 

.*:     file format .*

Disassembly of section \.text:

0+ <.text>:
   0:	4cee 047c ffe8 	moveml %fp@\(-24\),%d2-%d6/%a2
   6:	48ee 047c 0010 	moveml %d2-%d6/%a2,%fp@\(16\)
   c:	4cee 03ff ffe8 	moveml %fp@\(-24\),%d0-%a1
  12:	48ee 03ff 0010 	moveml %d0-%a1,%fp@\(16\)
	\.\.\.
