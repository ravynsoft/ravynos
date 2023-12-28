#source: ./mvfacgu.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 1f 30                      	mvfacgu	#0, a0, r0
   3:	fd 1f 3f                      	mvfacgu	#0, a0, r15
   6:	fd 1f 70                      	mvfacgu	#1, a0, r0
   9:	fd 1f 7f                      	mvfacgu	#1, a0, r15
   c:	fd 1e 30                      	mvfacgu	#2, a0, r0
   f:	fd 1e 3f                      	mvfacgu	#2, a0, r15
  12:	fd 1f b0                      	mvfacgu	#0, a1, r0
  15:	fd 1f bf                      	mvfacgu	#0, a1, r15
  18:	fd 1f f0                      	mvfacgu	#1, a1, r0
  1b:	fd 1f ff                      	mvfacgu	#1, a1, r15
  1e:	fd 1e b0                      	mvfacgu	#2, a1, r0
  21:	fd 1e bf                      	mvfacgu	#2, a1, r15
