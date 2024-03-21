#source: ./mvfaclo.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 1f 10                      	mvfaclo	#0, a0, r0
   3:	fd 1f 1f                      	mvfaclo	#0, a0, r15
   6:	fd 1f 10                      	mvfaclo	#0, a0, r0
   9:	fd 1f 1f                      	mvfaclo	#0, a0, r15
   c:	fd 1f 50                      	mvfaclo	#1, a0, r0
   f:	fd 1f 5f                      	mvfaclo	#1, a0, r15
  12:	fd 1e 10                      	mvfaclo	#2, a0, r0
  15:	fd 1e 1f                      	mvfaclo	#2, a0, r15
  18:	fd 1f 90                      	mvfaclo	#0, a1, r0
  1b:	fd 1f 9f                      	mvfaclo	#0, a1, r15
  1e:	fd 1f d0                      	mvfaclo	#1, a1, r0
  21:	fd 1f df                      	mvfaclo	#1, a1, r15
  24:	fd 1e 90                      	mvfaclo	#2, a1, r0
  27:	fd 1e 9f                      	mvfaclo	#2, a1, r15
