#as:  -march=score3 -I${srcdir}/${subdir}
#objdump:  -d
#source:  syscontrol_32.s

.*:     file format .*

Disassembly of section .text:

00000000 <.text>:
   0:	0020      	sdbbp!		0
   2:	003f      	sdbbp!		31
   4:	0020      	sdbbp!		0
   6:	0020      	sdbbp!		0
   8:	0020      	sdbbp!		0
   a:	0020      	sdbbp!		0
   c:	0020      	sdbbp!		0
   e:	0020      	sdbbp!		0
  10:	0020      	sdbbp!		0
  12:	0020      	sdbbp!		0
	...
#pass
