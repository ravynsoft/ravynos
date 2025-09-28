#source: ./mvfacmi.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 1f 20                      	mvfacmi	#0, a0, r0
   3:	fd 1f 2f                      	mvfacmi	#0, a0, r15
   6:	fd 1f 20                      	mvfacmi	#0, a0, r0
   9:	fd 1f 2f                      	mvfacmi	#0, a0, r15
   c:	fd 1f 60                      	mvfacmi	#1, a0, r0
   f:	fd 1f 6f                      	mvfacmi	#1, a0, r15
  12:	fd 1e 20                      	mvfacmi	#2, a0, r0
  15:	fd 1e 2f                      	mvfacmi	#2, a0, r15
  18:	fd 1f a0                      	mvfacmi	#0, a1, r0
  1b:	fd 1f af                      	mvfacmi	#0, a1, r15
  1e:	fd 1f e0                      	mvfacmi	#1, a1, r0
  21:	fd 1f ef                      	mvfacmi	#1, a1, r15
  24:	fd 1e a0                      	mvfacmi	#2, a1, r0
  27:	fd 1e af                      	mvfacmi	#2, a1, r15
