#objdump: -d
#name:    
#source:  psh-pul.s


.*:     file format elf32-s12z


Disassembly of section .text:

00000000 <here>:
   0:	04 00       	psh ALL
   2:	04 40       	psh ALL16b
   4:	04 3c       	psh cch, ccl, d0, d1
   6:	04 43       	psh x, y

00000008 <there>:
   8:	04 80       	pul ALL
   a:	04 c0       	pul ALL16b
   c:	04 bc       	pul cch, ccl, d0, d1
   e:	04 c3       	pul x, y
