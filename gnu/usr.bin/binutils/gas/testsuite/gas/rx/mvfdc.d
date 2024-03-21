#source: ./mvfdc.s
#objdump: -dr

.*:     file format .*


Disassembly of section .*:

00000000 <.*>:
   0:	fd 75 80 04                   	mvfdc	dpsw, r0
   4:	fd 75 8f 04                   	mvfdc	dpsw, r15
   8:	fd 75 80 14                   	mvfdc	dcmr, r0
   c:	fd 75 8f 14                   	mvfdc	dcmr, r15
  10:	fd 75 80 24                   	mvfdc	decnt, r0
  14:	fd 75 8f 24                   	mvfdc	decnt, r15
  18:	fd 75 80 34                   	mvfdc	depc, r0
  1c:	fd 75 8f 34                   	mvfdc	depc, r15
