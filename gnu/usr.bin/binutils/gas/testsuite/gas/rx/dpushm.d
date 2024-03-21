#source: ./dpushm.s
#objdump: -dr

.*:     file format .*


Disassembly of section .*:

00000000 <.*>:
   0:	75 b0 17                      	dpushm.d	dr1-dr8
   3:	75 b0 1d                      	dpushm.d	dr1-dr14
   6:	75 b0 71                      	dpushm.d	dr7-dr8
   9:	75 b0 77                      	dpushm.d	dr7-dr14
   c:	75 a0 03                      	dpushm.l	dpsw-depc
   f:	75 a0 02                      	dpushm.l	dpsw-decnt
  12:	75 a0 12                      	dpushm.l	dcmr-depc
  15:	75 a0 11                      	dpushm.l	dcmr-decnt
