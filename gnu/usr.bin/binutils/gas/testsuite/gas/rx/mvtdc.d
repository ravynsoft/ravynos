#source: ./mvtdc.s
#objdump: -dr

.*:     file format .*


Disassembly of section .*:

00000000 <.*>:
   0:	fd 77 80 04                   	mvtdc	r0, dpsw
   4:	fd 77 81 04                   	mvtdc	r0, dcmr
   8:	fd 77 82 04                   	mvtdc	r0, decnt
   c:	fd 77 83 04                   	mvtdc	r0, depc
  10:	fd 77 80 f4                   	mvtdc	r15, dpsw
  14:	fd 77 81 f4                   	mvtdc	r15, dcmr
  18:	fd 77 82 f4                   	mvtdc	r15, decnt
  1c:	fd 77 83 f4                   	mvtdc	r15, depc

