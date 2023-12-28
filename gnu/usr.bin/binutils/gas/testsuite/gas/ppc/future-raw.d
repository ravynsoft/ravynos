#as: -mfuture
#objdump: -dr -Mfuture -Mraw
#name: Future tests - raw disassembly

.*


Disassembly of section \.text:

0+0 <_start>:
.*:	(90 58 4c 7d|7d 4c 58 90) 	subfus  r10,0,r12,r11
.*:	(91 58 4c 7d|7d 4c 58 91) 	subfus\. r10,0,r12,r11
.*:	(90 ac 96 7e|7e 96 ac 90) 	subfus  r20,1,r22,r21
.*:	(91 ac 96 7e|7e 96 ac 91) 	subfus\. r20,1,r22,r21
#pass
