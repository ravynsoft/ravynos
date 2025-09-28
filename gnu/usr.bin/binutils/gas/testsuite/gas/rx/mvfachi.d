#source: ./mvfachi.s
#objdump: -dr

.*:     file format .*


Disassembly of section \.text:

00000000 <\.text>:
   0:	fd 1f 00                      	mvfachi	#0, a0, r0
   3:	fd 1f 0f                      	mvfachi	#0, a0, r15
   6:	fd 1f 00                      	mvfachi	#0, a0, r0
   9:	fd 1f 0f                      	mvfachi	#0, a0, r15
   c:	fd 1f 40                      	mvfachi	#1, a0, r0
   f:	fd 1f 4f                      	mvfachi	#1, a0, r15
  12:	fd 1e 00                      	mvfachi	#2, a0, r0
  15:	fd 1e 0f                      	mvfachi	#2, a0, r15
  18:	fd 1f 80                      	mvfachi	#0, a1, r0
  1b:	fd 1f 8f                      	mvfachi	#0, a1, r15
  1e:	fd 1f c0                      	mvfachi	#1, a1, r0
  21:	fd 1f cf                      	mvfachi	#1, a1, r15
  24:	fd 1e 80                      	mvfachi	#2, a1, r0
  27:	fd 1e 8f                      	mvfachi	#2, a1, r15
