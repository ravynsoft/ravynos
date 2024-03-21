#source: ./dpopm.s
#objdump: -dr

.*:     file format .*


Disassembly of section .*:

00000000 <.*>:
   0:	75 b8 17                      	dpopm.d	dr1-dr8
   3:	75 b8 1d                      	dpopm.d	dr1-dr14
   6:	75 b8 71                      	dpopm.d	dr7-dr8
   9:	75 b8 77                      	dpopm.d	dr7-dr14
   c:	75 a8 03                      	dpopm.l	dpsw-depc
   f:	75 a8 02                      	dpopm.l	dpsw-decnt
  12:	75 a8 12                      	dpopm.l	dcmr-depc
  15:	75 a8 11                      	dpopm.l	dcmr-decnt
