#source: ./mvfc.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 6a 00                      	mvfc	psw, r0
   3:	fd 6a 0f                      	mvfc	psw, r15
   6:	fd 6a 30                      	mvfc	fpsw, r0
   9:	fd 6a 3f                      	mvfc	fpsw, r15
   c:	fd 6a 20                      	mvfc	usp, r0
   f:	fd 6a 2f                      	mvfc	usp, r15
  12:	fd 6a a0                      	mvfc	isp, r0
  15:	fd 6a af                      	mvfc	isp, r15
  18:	fd 6a c0                      	mvfc	intb, r0
  1b:	fd 6a cf                      	mvfc	intb, r15
  1e:	fd 6a 80                      	mvfc	bpsw, r0
  21:	fd 6a 8f                      	mvfc	bpsw, r15
  24:	fd 6a 90                      	mvfc	bpc, r0
  27:	fd 6a 9f                      	mvfc	bpc, r15
  2a:	fd 6a b0                      	mvfc	fintv, r0
  2d:	fd 6a bf                      	mvfc	fintv, r15
  30:	fd 6a d0                      	mvfc	extb, r0
  33:	fd 6a df                      	mvfc	extb, r15

