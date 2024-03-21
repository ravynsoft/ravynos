#as: -mpower8
#objdump: -dr -Mpower8
#name: Hardware Transactional Memory (HTM) tests

.*

Disassembly of section \.text:

0+00 <htm>:
   0:	(7c 05 07 1d|1d 07 05 7c) 	tabort\. r5
   4:	(7c e8 86 1d|1d 86 e8 7c) 	tabortwc\. 7,r8,r16
   8:	(7e 8b 56 5d|5d 56 8b 7e) 	tabortdc\. 20,r11,r10
   c:	(7e 2a 9e 9d|9d 9e 2a 7e) 	tabortwci\. 17,r10,-13
  10:	(7f a3 de dd|dd de a3 7f) 	tabortdci\. 29,r3,-5
  14:	(7c 00 05 1d|1d 05 00 7c) 	tbegin\.
  18:	(7f 80 05 9c|9c 05 80 7f) 	tcheck  cr7
  1c:	(7c 00 05 5d|5d 05 00 7c) 	tend\.
  20:	(7c 00 05 5d|5d 05 00 7c) 	tend\.
  24:	(7e 00 05 5d|5d 05 00 7e) 	tendall\.
  28:	(7e 00 05 5d|5d 05 00 7e) 	tendall\.
  2c:	(7c 18 07 5d|5d 07 18 7c) 	treclaim\. r24
  30:	(7c 00 07 dd|dd 07 00 7c) 	trechkpt\.
  34:	(7c 00 05 dd|dd 05 00 7c) 	tsuspend\.
  38:	(7c 00 05 dd|dd 05 00 7c) 	tsuspend\.
  3c:	(7c 20 05 dd|dd 05 20 7c) 	tresume\.
  40:	(7c 20 05 dd|dd 05 20 7c) 	tresume\.
#pass
