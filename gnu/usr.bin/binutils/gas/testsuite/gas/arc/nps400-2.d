#as: -mcpu=arc700 -mnps400
#objdump: -dr

.*: +file format .*arc.*

Disassembly of section .text:

[0-9a-f]+ <.*>:
   0:	3e6f 7004           	schd\.rw
   4:	3e6f 7084           	schd\.rd
   8:	3e6f 7044           	schd\.wft
   c:	3e6f 7144           	schd\.wft\.ie1
  10:	3e6f 7244           	schd\.wft\.ie2
  14:	3e6f 7344           	schd\.wft\.ie12
  18:	3e6f 703f           	sync\.rd
  1c:	3e6f 707f           	sync\.wr
  20:	3a6f 10bf           	hwschd\.off	r10
  24:	3e6f 7503           	hwschd\.restore	0,r20
